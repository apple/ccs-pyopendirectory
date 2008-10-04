/**
 * A class that wraps high-level Directory Service calls needed by the
 * CalDAV server.
 **
 * Copyright (c) 2006-2008 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

#include "CDirectoryService.h"

#include "CDirectoryServiceException.h"

#include "base64.h"
#include "CFStringUtil.h"

#include <Python.h>

#include <stdlib.h>
#include <string.h>
#include <memory>

extern PyObject* ODException_class;

// This is copied from WhitePages
#define        kDSStdRecordTypeResources                "dsRecTypeStandard:Resources"

// Calendar attribute.
#define        kDS1AttrCalendarPrincipalURI            "dsAttrTypeStandard:CalendarPrincipalURI"

const int cBufferSize = 32 * 1024;        // 32K buffer for Directory Services operations

#pragma mark -----Public API

CDirectoryService::CDirectoryService(const char* nodename)
{
    mNodeName = CStringFromData(nodename, ::strlen(nodename));
    mDir = 0L;
    mNode = 0L;
    mData = NULL;
    mDataSize = 0;
}

CDirectoryService::~CDirectoryService()
{
    // Clean-up any allocated objects
    if (mData != NULL)
    {
        assert(mDir != 0L);
        ::dsDataBufferDeAllocate(mDir, mData);
        mData = 0L;
    }

    if (mNode != 0L)
    {
        ::dsCloseDirNode(mNode);
        mNode = 0L;
    }

    if (mDir != 0L)
    {
        ::dsCloseDirService(mDir);
        mDir = 0L;
    }

    delete mNodeName;
    mNodeName = NULL;
}

// ListAllRecordsWithAttributes
//
// Get specific attributes for one or more user records in the directory.
//
// @param recordType: the record type to list.
// @param attributes: CFArray of CFString listing the attributes to return for each record.
// @return: CFMutableArrayRef composed of CFMutableArrayRef with a CFStringRef/CFMutableDictionaryRef tuple for
//          each record, where the CFStringRef is the record name and CFMutableDictionaryRef of CFStringRef key
//            and value entries for each attribute/value requested in the record indexed by uid,
//            or NULL if it fails.
//
CFMutableArrayRef CDirectoryService::ListAllRecordsWithAttributes(const char* recordType, CFDictionaryRef attributes, bool using_python)
{
    try
    {
        StPythonThreadState threading(using_python);

        // Get attribute map
        return _ListAllRecordsWithAttributes(recordType, NULL, attributes);
    }
    catch(CDirectoryServiceException& dserror)
    {
        dserror.SetPythonException();
        return NULL;
    }
    catch(...)
    {
        CDirectoryServiceException dserror;
        dserror.SetPythonException();
        return NULL;
    }
}

// QueryRecordsWithAttribute
//
// Get specific attributes for one or more user records with matching attribute/value in the directory.
//
// @param attr: the attribute to query.
// @param value: the value to query.
// @param matchType: the match type to use.
// @param casei: true if case-insensitive match is to be used, false otherwise.
// @param recordType: the record type to list.
// @param attributes: CFArray of CFString listing the attributes to return for each record.
// @return: CFMutableArrayRef composed of CFMutableArrayRef with a CFStringRef/CFMutableDictionaryRef tuple for
//          each record, where the CFStringRef is the record name and CFMutableDictionaryRef of CFStringRef key
//          and value entries for each attribute/value requested in the record indexed by uid,
//          or NULL if it fails.
//
CFMutableArrayRef CDirectoryService::QueryRecordsWithAttribute(const char* attr, const char* value, int matchType, bool casei, const char* recordType, CFDictionaryRef attributes, bool using_python)
{
    try
    {
        StPythonThreadState threading(using_python);

        // Get attribute map
        return _QueryRecordsWithAttributes(attr, value, matchType, NULL, casei, recordType, attributes);
    }
    catch(CDirectoryServiceException& dserror)
    {
        dserror.SetPythonException();
        return NULL;
    }
    catch(...)
    {
        CDirectoryServiceException dserror;
        dserror.SetPythonException();
        return NULL;
    }
}

// QueryRecordsWithAttributes
//
// Get specific attributes for one or more user records with matching attributes in the directory.
//
// @param query: the compund query string to use.
// @param casei: true if case-insensitive match is to be used, false otherwise.
// @param recordType: the record type to list.
// @param attributes: CFArray of CFString listing the attributes to return for each record.
// @return: CFMutableArrayRef composed of CFMutableArrayRef with a CFStringRef/CFMutableDictionaryRef tuple for
//          each record, where the CFStringRef is the record name and CFMutableDictionaryRef of CFStringRef key
//          and value entries for each attribute/value requested in the record indexed by uid,
//          or NULL if it fails.
//
CFMutableArrayRef CDirectoryService::QueryRecordsWithAttributes(const char* query, bool casei, const char* recordType, CFDictionaryRef attributes, bool using_python)
{
    try
    {
        StPythonThreadState threading(using_python);

        // Get attribute map
        return _QueryRecordsWithAttributes(NULL, NULL, 0, query, casei, recordType, attributes);
    }
    catch(CDirectoryServiceException& dserror)
    {
        dserror.SetPythonException();
        return NULL;
    }
    catch(...)
    {
        CDirectoryServiceException dserror;
        dserror.SetPythonException();
        return NULL;
    }
}

// AuthenticateUserBasic
//
// Authenticate a user to the directory using plain text credentials.
//
// @param nodename: the directory nodename for the user record.
// @param user: the uid of the user.
// @param pswd: the plain text password to authenticate with.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryService::AuthenticateUserBasic(const char* nodename, const char* user, const char* pswd, bool& result, bool using_python)
{
    try
    {
        StPythonThreadState threading(using_python);

        result = NativeAuthenticationBasicToNode(nodename, user, pswd);
        return true;
    }
    catch(CDirectoryServiceException& dserror)
    {
        dserror.SetPythonException();
        return false;
    }
    catch(...)
    {
        CDirectoryServiceException dserror;
        dserror.SetPythonException();
        return false;
    }
}

// AuthenticateUserDigest
//
// Authenticate a user to the directory using HTTP DIGEST credentials.
//
// @param nodename: the directory nodename for the user record.
// @param challenge: HTTP challenge sent by server.
// @param response: HTTP response sent by client.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryService::AuthenticateUserDigest(const char* nodename, const char* user, const char* challenge, const char* response, const char* method, bool& result, bool using_python)
{
    try
    {
        StPythonThreadState threading(using_python);

        result = NativeAuthenticationDigestToNode(nodename, user, challenge, response, method);
        return true;
    }
    catch(CDirectoryServiceException& dserror)
    {
        dserror.SetPythonException();
        return false;
    }
    catch(...)
    {
        CDirectoryServiceException dserror;
        dserror.SetPythonException();
        return false;
    }
}

#pragma mark -----Private API

// _ListAllRecordsWithAttributes
//
// Get specific attributes for records of a specified type in the directory.
//
// @param type: the record type to check.
// @param names: a list of record names to target if NULL all records are matched.
// @param attributes: a list of attributes to return.
// @return: CFMutableArrayRef composed of CFMutableArrayRef with a CFStringRef/CFMutableDictionaryRef tuple for
//          each record, where the CFStringRef is the record name and CFMutableDictionaryRef of CFStringRef key
//          and value entries for each attribute/value requested in the record indexed by uid,
//          or NULL if it fails.
//
CFMutableArrayRef CDirectoryService::_ListAllRecordsWithAttributes(const char* type, CFArrayRef names, CFDictionaryRef attributes)
{
    CFMutableArrayRef result = NULL;
    CFMutableArrayRef record_tuple = NULL;
    CFMutableDictionaryRef record = NULL;
    CFMutableArrayRef values = NULL;
    tDataListPtr recNames = NULL;
    tDataListPtr recTypes = NULL;
    tDataListPtr attrTypes = NULL;
    tContextData context = NULL;
    tAttributeListRef attrListRef = 0L;
    tRecordEntry* pRecEntry = NULL;

    // Must have attributes
    if (::CFDictionaryGetCount(attributes) == 0)
        return NULL;

    try
    {
        // Make sure we have a valid directory service
        OpenService();

        // Open the node we want to query
        OpenNode();

        // We need a buffer for what comes next
        CreateBuffer();

        // Build data list of names
        recNames = ::dsDataListAllocate(mDir);
        ThrowIfNULL(recNames);
        if (names != NULL)
            BuildStringDataList(names, recNames);
        else
            ThrowIfDSErr(::dsBuildListFromStringsAlloc(mDir, recNames,  kDSRecordsAll, NULL));

        // Build data list of types
        recTypes = ::dsDataListAllocate(mDir);
        ThrowIfNULL(recTypes);
        ThrowIfDSErr(::dsBuildListFromStringsAlloc(mDir, recTypes,  type, NULL));

        // Build data list of attributes
        attrTypes = ::dsDataListAllocate(mDir);
        ThrowIfNULL(attrTypes);
        BuildStringDataListFromKeys(attributes, attrTypes);

        result = ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

        do
        {
            // List all the appropriate records
            UInt32 recCount = 0;
            tDirStatus err;
            do
            {
                err = ::dsGetRecordList(mNode, mData, recNames, eDSExact, recTypes, attrTypes, false, &recCount, &context);
                if (err == eDSBufferTooSmall)
                    ReallocBuffer();
            } while(err == eDSBufferTooSmall);
            ThrowIfDSErr(err);
            for(UInt32 i = 1; i <= recCount; i++)
            {
                // Get the record entry
                ThrowIfDSErr(::dsGetRecordEntry(mNode, mData, i, &attrListRef, &pRecEntry));

                // Get the entry's name
                char* temp = NULL;
                ThrowIfDSErr(::dsGetRecordNameFromEntry(pRecEntry, &temp));
                std::auto_ptr<char> recname(temp);

                // Create a dictionary for the values
                record = ::CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

                // Look at each requested attribute and get one value
                for(unsigned long j = 1; j <= pRecEntry->fRecordAttributeCount; j++)
                {
                    tAttributeValueListRef attributeValueListRef = NULL;
                    tAttributeEntryPtr attributeInfoPtr = NULL;

                    ThrowIfDSErr(::dsGetAttributeEntry(mNode, mData, attrListRef, j, &attributeValueListRef, &attributeInfoPtr));

                    if (attributeInfoPtr->fAttributeValueCount > 0)
                    {
                        // Determine what the attribute is and where in the result list it should be put
                        std::auto_ptr<char> attrname(CStringFromBuffer(&attributeInfoPtr->fAttributeSignature));
                        CFStringUtil cfattrname(attrname.get());

						// Determine whether string/base64 encoding is needed
						bool base64 = false;
						CFStringRef encoding = (CFStringRef)::CFDictionaryGetValue(attributes, cfattrname.get());
						if (encoding && (::CFStringCompare(encoding, CFSTR("base64"), 0) == kCFCompareEqualTo))
							base64 = true;

                        if (attributeInfoPtr->fAttributeValueCount > 1)
                        {
                            values = ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

                            for(unsigned long k = 1; k <= attributeInfoPtr->fAttributeValueCount; k++)
                            {
                                // Get the attribute value and store in results
                                tAttributeValueEntryPtr attributeValue = NULL;
                                ThrowIfDSErr(::dsGetAttributeValue(mNode, mData, k, attributeValueListRef, &attributeValue));
								std::auto_ptr<char> data;
								if (base64)
									data.reset(CStringBase64FromBuffer(&attributeValue->fAttributeValueData));
								else
									data.reset(CStringFromBuffer(&attributeValue->fAttributeValueData));
                                CFStringUtil strvalue(data.get());
                                ::CFArrayAppendValue(values, strvalue.get());
                                ::dsDeallocAttributeValueEntry(mDir, attributeValue);
                                attributeValue = NULL;
                            }
                            ::CFDictionarySetValue(record, cfattrname.get(), values);
                            ::CFRelease(values);
                            values = NULL;
                        }
                        else
                        {
                            // Get the attribute value and store in results
                            tAttributeValueEntryPtr attributeValue = NULL;
                            ThrowIfDSErr(::dsGetAttributeValue(mNode, mData, 1, attributeValueListRef, &attributeValue));
                            std::auto_ptr<char> data;
							if (base64)
								data.reset(CStringBase64FromBuffer(&attributeValue->fAttributeValueData));
							else
								data.reset(CStringFromBuffer(&attributeValue->fAttributeValueData));
                            CFStringUtil strvalue(data.get());
							if (strvalue.get() != NULL)
							{
								::CFDictionarySetValue(record, cfattrname.get(), strvalue.get());
							}
							::dsDeallocAttributeValueEntry(mDir, attributeValue);
                            attributeValue = NULL;
                        }
                    }

                    ::dsCloseAttributeValueList(attributeValueListRef);
                    attributeValueListRef = NULL;
                    ::dsDeallocAttributeEntry(mDir, attributeInfoPtr);
                    attributeInfoPtr = NULL;
                }

                // Create tuple of record name and record values
                CFStringUtil str(recname.get());

                record_tuple = ::CFArrayCreateMutable(kCFAllocatorDefault, 2, &kCFTypeArrayCallBacks);
                ::CFArrayAppendValue(record_tuple, str.get());
                ::CFArrayAppendValue(record_tuple, record);
                ::CFRelease(record);
                record = NULL;

                // Append tuple to results array
                ::CFArrayAppendValue(result, record_tuple);
                ::CFRelease(record_tuple);
                record_tuple = NULL;

                // Clean-up
                ::dsCloseAttributeList(attrListRef);
                attrListRef = 0L;
                ::dsDeallocRecordEntry(mDir, pRecEntry);
                pRecEntry = NULL;
            }
        } while (context != NULL); // Loop until all data has been obtained.

        // Cleanup
        ::dsDataListDeallocate(mDir, recNames);
        ::dsDataListDeallocate(mDir, recTypes);
        ::dsDataListDeallocate(mDir, attrTypes);
        free(recNames);
        free(recTypes);
        free(attrTypes);
        RemoveBuffer();
        CloseNode();
        CloseService();
    }
    catch(CDirectoryServiceException& dsStatus)
    {
        // Cleanup
        if (context != NULL)
            ::dsReleaseContinueData(mDir, context);
        if (attrListRef != 0L)
            ::dsCloseAttributeList(attrListRef);
        if (pRecEntry != NULL)
            dsDeallocRecordEntry(mDir, pRecEntry);
        if (recNames != NULL)
        {
            ::dsDataListDeallocate(mDir, recNames);
            free(recNames);
            recNames = NULL;
        }
        if (recTypes != NULL)
        {
            ::dsDataListDeallocate(mDir, recTypes);
            free(recTypes);
            recTypes = NULL;
        }
        if (attrTypes != NULL)
        {
            ::dsDataListDeallocate(mDir, attrTypes);
            free(attrTypes);
            attrTypes = NULL;
        }
        RemoveBuffer();
        CloseNode();
        CloseService();

        if (values != NULL)
        {
            ::CFRelease(values);
            values = NULL;
        }
        if (record != NULL)
        {
            ::CFRelease(record);
            record = NULL;
        }
        if (record_tuple != NULL)
        {
            ::CFRelease(record_tuple);
            record_tuple = NULL;
        }
        if (result != NULL)
        {
            ::CFRelease(result);
            result = NULL;
        }
        throw;
    }

    return result;
}

// _QueryRecordsWithAttributes
//
// Get specific attributes for records of a specified type in the directory.
//
// @param attr: the attribute to query (NULL if compound is being used).
// @param value: the value to query (NULL if compound is being used).
// @param matchType: the match type to use (0 if compound is being used).
// @param attr: the compound query to use rather than single attribute/value (NULL if compound is not being used).
// @param casei: true if case-insensitive match is to be used, false otherwise.
// @param type: the record type to check.
// @param attributes: a list of attributes to return.
// @return: CFMutableArrayRef composed of CFMutableArrayRef with a CFStringRef/CFMutableDictionaryRef tuple for
//          each record, where the CFStringRef is the record name and CFMutableDictionaryRef of CFStringRef key
//          and value entries for each attribute/value requested in the record indexed by uid,
//          or NULL if it fails.
//
CFMutableArrayRef CDirectoryService::_QueryRecordsWithAttributes(const char* attr, const char* value, int matchType, const char* compound, bool casei, const char* type, CFDictionaryRef attributes)
{
    CFMutableArrayRef result = NULL;
    CFMutableArrayRef record_tuple = NULL;
    CFMutableDictionaryRef record = NULL;
    CFMutableArrayRef values = NULL;
    tDataNodePtr queryAttr = NULL;
    tDataNodePtr queryValue = NULL;
    tDataListPtr recTypes = NULL;
    tDataListPtr attrTypes = NULL;
    tContextData context = NULL;
    tAttributeListRef attrListRef = 0L;
    tRecordEntry* pRecEntry = NULL;

    // Must have attributes
    if (::CFDictionaryGetCount(attributes) == 0)
        return NULL;

    try
    {
        // Make sure we have a valid directory service
        OpenService();

        // Open the node we want to query
        OpenNode();

        // We need a buffer for what comes next
        CreateBuffer();

        if (compound == NULL)
        {
            // Determine attribute to search
            queryAttr = ::dsDataNodeAllocateString(mDir, attr);
            ThrowIfNULL(queryAttr);

            queryValue = ::dsDataNodeAllocateString(mDir, value);
            ThrowIfNULL(queryValue);

            if (casei)
                matchType |= 0x0100;
            else
                matchType &= 0xFEFF;
        }
        else
        {
            queryAttr = ::dsDataNodeAllocateString(mDir, kDS1AttrDistinguishedName);
            ThrowIfNULL(queryAttr);

            queryValue = ::dsDataNodeAllocateString(mDir, compound);
            ThrowIfNULL(queryValue);

            matchType = (casei) ? eDSiCompoundExpression : eDSCompoundExpression;
        }

        // Build data list of types
        recTypes = ::dsDataListAllocate(mDir);
        ThrowIfNULL(recTypes);
        ThrowIfDSErr(::dsBuildListFromStringsAlloc(mDir, recTypes,  type, NULL));

        // Build data list of attributes
        attrTypes = ::dsDataListAllocate(mDir);
        ThrowIfNULL(attrTypes);
        BuildStringDataListFromKeys(attributes, attrTypes);

        result = ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

        do
        {
            // List all the appropriate records
            UInt32 recCount = 0;
            tDirStatus err;
            do
            {
                err = ::dsDoAttributeValueSearchWithData(mNode, mData, recTypes, queryAttr, (tDirPatternMatch)matchType, queryValue, attrTypes, false, &recCount, &context);
                if (err == eDSBufferTooSmall)
                    ReallocBuffer();
            } while(err == eDSBufferTooSmall);
            ThrowIfDSErr(err);
            for(UInt32 i = 1; i <= recCount; i++)
            {
                // Get the record entry
                ThrowIfDSErr(::dsGetRecordEntry(mNode, mData, i, &attrListRef, &pRecEntry));

                // Get the entry's name
                char* temp = NULL;
                ThrowIfDSErr(::dsGetRecordNameFromEntry(pRecEntry, &temp));
                std::auto_ptr<char> recname(temp);

                // Create a dictionary for the values
                record = ::CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

                // Look at each requested attribute and get one value
                for(unsigned long j = 1; j <= pRecEntry->fRecordAttributeCount; j++)
                {
                    tAttributeValueListRef attributeValueListRef = NULL;
                    tAttributeEntryPtr attributeInfoPtr = NULL;

                    ThrowIfDSErr(::dsGetAttributeEntry(mNode, mData, attrListRef, j, &attributeValueListRef, &attributeInfoPtr));

                    if (attributeInfoPtr->fAttributeValueCount > 0)
                    {
                        // Determine what the attribute is and where in the result list it should be put
                        std::auto_ptr<char> attrname(CStringFromBuffer(&attributeInfoPtr->fAttributeSignature));
                        CFStringUtil cfattrname(attrname.get());

						// Determine whether string/base64 encoding is needed
						bool base64 = false;
						CFStringRef encoding = (CFStringRef)::CFDictionaryGetValue(attributes, cfattrname.get());
						if (encoding && (::CFStringCompare(encoding, CFSTR("base64"), 0) == kCFCompareEqualTo))
							base64 = true;

                        if (attributeInfoPtr->fAttributeValueCount > 1)
                        {
                            values = ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

                            for(unsigned long k = 1; k <= attributeInfoPtr->fAttributeValueCount; k++)
                            {
                                // Get the attribute value and store in results
                                tAttributeValueEntryPtr attributeValue = NULL;
                                ThrowIfDSErr(::dsGetAttributeValue(mNode, mData, k, attributeValueListRef, &attributeValue));
								std::auto_ptr<char> data;
								if (base64)
									data.reset(CStringBase64FromBuffer(&attributeValue->fAttributeValueData));
								else
									data.reset(CStringFromBuffer(&attributeValue->fAttributeValueData));
                                CFStringUtil strvalue(data.get());
								if (strvalue.get() != NULL)
									::CFArrayAppendValue(values, strvalue.get());
                                ::dsDeallocAttributeValueEntry(mDir, attributeValue);
                                attributeValue = NULL;
                            }
                            ::CFDictionarySetValue(record, cfattrname.get(), values);
                            ::CFRelease(values);
                            values = NULL;
                        }
                        else
                        {
                            // Get the attribute value and store in results
                            tAttributeValueEntryPtr attributeValue = NULL;
                            ThrowIfDSErr(::dsGetAttributeValue(mNode, mData, 1, attributeValueListRef, &attributeValue));
                            std::auto_ptr<char> data;
							if (base64)
								data.reset(CStringBase64FromBuffer(&attributeValue->fAttributeValueData));
							else
								data.reset(CStringFromBuffer(&attributeValue->fAttributeValueData));
                            CFStringUtil strvalue(data.get());
							if (strvalue.get() != NULL)
								::CFDictionarySetValue(record, cfattrname.get(), strvalue.get());
                            ::dsDeallocAttributeValueEntry(mDir, attributeValue);
                            attributeValue = NULL;
                        }
                    }

                    ::dsCloseAttributeValueList(attributeValueListRef);
                    attributeValueListRef = NULL;
                    ::dsDeallocAttributeEntry(mDir, attributeInfoPtr);
                    attributeInfoPtr = NULL;
                }

                // Create tuple of record name and record values
                CFStringUtil str(recname.get());

                record_tuple = ::CFArrayCreateMutable(kCFAllocatorDefault, 2, &kCFTypeArrayCallBacks);
                ::CFArrayAppendValue(record_tuple, str.get());
                ::CFArrayAppendValue(record_tuple, record);
                ::CFRelease(record);
                record = NULL;

                // Append tuple to results array
                ::CFArrayAppendValue(result, record_tuple);
                ::CFRelease(record_tuple);
                record_tuple = NULL;

                // Clean-up
                ::dsCloseAttributeList(attrListRef);
                attrListRef = 0L;
                ::dsDeallocRecordEntry(mDir, pRecEntry);
                pRecEntry = NULL;
            }
        } while (context != NULL); // Loop until all data has been obtained.

        // Cleanup
        ::dsDataListDeallocate(mDir, recTypes);
        ::dsDataListDeallocate(mDir, attrTypes);
        ::dsDataNodeDeAllocate(mDir, queryValue);
        ::dsDataNodeDeAllocate(mDir, queryAttr);
        free(recTypes);
        free(attrTypes);
        RemoveBuffer();
        CloseNode();
        CloseService();
    }
    catch(CDirectoryServiceException& dsStatus)
    {
        // Cleanup
        if (context != NULL)
            ::dsReleaseContinueData(mDir, context);
        if (attrListRef != 0L)
            ::dsCloseAttributeList(attrListRef);
        if (pRecEntry != NULL)
            dsDeallocRecordEntry(mDir, pRecEntry);
        if (recTypes != NULL)
        {
            ::dsDataListDeallocate(mDir, recTypes);
            free(recTypes);
            recTypes = NULL;
        }
        if (attrTypes != NULL)
        {
            ::dsDataListDeallocate(mDir, attrTypes);
            free(attrTypes);
            attrTypes = NULL;
        }
        if (queryValue != NULL)
        {
            ::dsDataNodeDeAllocate(mDir, queryValue);
            queryValue = NULL;
        }
        if (queryAttr != NULL)
        {
            ::dsDataNodeDeAllocate(mDir, queryAttr);
            queryAttr = NULL;
        }
        RemoveBuffer();
        CloseNode();
        CloseService();

        if (values != NULL)
        {
            ::CFRelease(values);
            values = NULL;
        }
        if (record != NULL)
        {
            ::CFRelease(record);
            record = NULL;
        }
        if (record_tuple != NULL)
        {
            ::CFRelease(record_tuple);
            record_tuple = NULL;
        }
        if (result != NULL)
        {
            ::CFRelease(result);
            result = NULL;
        }
        throw;
    }

    return result;
}

// NativeAuthenticationBasicToNode
//
// Authenticate a user to the directory.
//
// @param nodename: the node to authenticate to.
// @param user: the uid of the user.
// @param pswd: the plain text password to authenticate with.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryService::NativeAuthenticationBasicToNode(const char* nodename, const char* user, const char* pswd)
{
    bool result = false;
    tDirNodeReference node = 0L;
    tDataNodePtr authType = NULL;
    tDataBufferPtr authData = NULL;
    tContextData context = NULL;

    try
    {
        // Make sure we have a valid directory service
        OpenService();

        // Open the node we want to query
        node = OpenNamedNode(nodename);

        CreateBuffer();

        // First, specify the type of authentication.
        authType = ::dsDataNodeAllocateString(mDir, kDSStdAuthClearText);

        // Build input data
        //  Native authentication is a one step authentication scheme.
        //  Step 1
        //      Send: <length><recordname>
        //            <length><cleartextpassword>
        //   Receive: success or failure.
        long aDataBufSize = sizeof(long) + ::strlen(user) + sizeof(long) + ::strlen(pswd);
        authData = ::dsDataBufferAllocate(mDir, aDataBufSize);
        if (authData == NULL)
            ThrowIfDSErr(eDSNullDataBuff);
        long aCurLength = 0;
        long aTempLength = ::strlen(user);
        ::memcpy(&(authData->fBufferData[aCurLength]), &aTempLength,  sizeof(long));
        aCurLength += sizeof(long);

        ::memcpy(&(authData->fBufferData[aCurLength]), user,  aTempLength);
        aCurLength += aTempLength;

        aTempLength = ::strlen(pswd);
        ::memcpy(&(authData->fBufferData[aCurLength]), &aTempLength,  sizeof(long));
        aCurLength += sizeof(long);

        ::memcpy(&(authData->fBufferData[aCurLength]), pswd,  aTempLength);

        authData->fBufferLength = aDataBufSize;

        // Do authentication
        long dirStatus = ::dsDoDirNodeAuth(node, authType, true,  authData,  mData, &context);
        result = (dirStatus == eDSNoErr);

        // Cleanup
        ::dsDataBufferDeAllocate(mDir, authData);
        authData = NULL;
        ::dsDataNodeDeAllocate(mDir, authType);
        authType = NULL;
        RemoveBuffer();
        if (node != 0L)
        {
            ::dsCloseDirNode(node);
            node = 0L;
        }
        CloseService();
    }
    catch(...)
    {
        // Cleanup
        if (authData != NULL)
            ::dsDataBufferDeAllocate(mDir, authData);
        if (authType != NULL)
            ::dsDataNodeDeAllocate(mDir, authType);
        RemoveBuffer();
        if (node != 0L)
        {
            ::dsCloseDirNode(node);
            node = 0L;
        }
        CloseService();

        throw;
    }

    return result;
}

// NativeAuthenticationDigestToNode
//
// Authenticate a user to the directory.
//
// @param nodename: the node to authenticate to.
// @param user: the uid of the user.
// @param challenge: the server challenge.
// @param response: the client response.
// @param method: the HTTP method.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryService::NativeAuthenticationDigestToNode(const char* nodename, const char* user,
                                                         const char* challenge, const char* response, const char* method)
{
    bool result = false;
    tDirNodeReference node = 0L;
    tDataNodePtr authType = NULL;
    tDataBufferPtr authData = NULL;
    tContextData context = NULL;

    try
    {
        // Make sure we have a valid directory service
        OpenService();

        // Open the node we want to query
        node = OpenNamedNode(nodename);

        CreateBuffer();

        // First, specify the type of authentication.
        authType = ::dsDataNodeAllocateString(mDir, kDSStdAuthDIGEST_MD5);

        // Build input data
        //  Native authentication is a one step authentication scheme.
        //  Step 1
        //      Send: <length><user>
        //              <length><challenge>
        //            <length><response>
        //              <length><method>
        //   Receive: success or failure.
        long aDataBufSize = sizeof(long) + ::strlen(user) +
                            sizeof(long) + ::strlen(challenge) +
                            sizeof(long) + ::strlen(response) +
                            sizeof(long) + ::strlen(method);
        authData = ::dsDataBufferAllocate(mDir, aDataBufSize);
        if (authData == NULL)
            ThrowIfDSErr(eDSNullDataBuff);
        long aCurLength = 0;
        long aTempLength = ::strlen(user);
        ::memcpy(&(authData->fBufferData[aCurLength]), &aTempLength,  sizeof(long));
        aCurLength += sizeof(long);

        ::memcpy(&(authData->fBufferData[aCurLength]), user,  aTempLength);
        aCurLength += aTempLength;

        aTempLength = ::strlen(challenge);
        ::memcpy(&(authData->fBufferData[aCurLength]), &aTempLength,  sizeof(long));
        aCurLength += sizeof(long);

        ::memcpy(&(authData->fBufferData[aCurLength]), challenge,  aTempLength);
        aCurLength += aTempLength;

        aTempLength = ::strlen(response);
        ::memcpy(&(authData->fBufferData[aCurLength]), &aTempLength,  sizeof(long));
        aCurLength += sizeof(long);

        ::memcpy(&(authData->fBufferData[aCurLength]), response,  aTempLength);
        aCurLength += aTempLength;

        aTempLength = ::strlen(method);
        ::memcpy(&(authData->fBufferData[aCurLength]), &aTempLength,  sizeof(long));
        aCurLength += sizeof(long);

        ::memcpy(&(authData->fBufferData[aCurLength]), method,  aTempLength);

        authData->fBufferLength = aDataBufSize;

        // Do authentication
        long dirStatus = ::dsDoDirNodeAuth(node, authType, true,  authData,  mData, &context);
        result = (dirStatus == eDSNoErr);

        // Cleanup
        ::dsDataBufferDeAllocate(mDir, authData);
        authData = NULL;
        ::dsDataNodeDeAllocate(mDir, authType);
        authType = NULL;
        RemoveBuffer();
        if (node != 0L)
        {
            ::dsCloseDirNode(node);
            node = 0L;
        }
        CloseService();
    }
    catch(...)
    {
        // Cleanup
        if (authData != NULL)
            ::dsDataBufferDeAllocate(mDir, authData);
        if (authType != NULL)
            ::dsDataNodeDeAllocate(mDir, authType);
        RemoveBuffer();
        if (node != 0L)
        {
            ::dsCloseDirNode(node);
            node = 0L;
        }
        CloseService();

        throw;
    }

    return result;
}

// OpenService
//
// Open the directory service.
//
// @throw: yes
//
void CDirectoryService::OpenService()
{
    if (mDir == 0L)
    {
        long dirStatus = ::dsOpenDirService(&mDir);
        if (dirStatus != eDSNoErr)
        {
            mDir = 0L;
            ThrowIfDSErr(dirStatus);
        }
    }
}

// CloseService
//
// Close the directory service if previously open.
//
void CDirectoryService::CloseService()
{
    if (mDir != 0L)
    {
        ::dsCloseDirService(mDir);
        mDir = 0L;
    }
}

// OpenNode
//
// Open a node in the directory.
//
// @throw: yes
//
void CDirectoryService::OpenNode()
{
    mNode = OpenNamedNode(mNodeName);
}

// OpenNode
//
// Open a node in the directory.
//
// @param nodename: the name of the node to open.
// @return: node reference if success, NULL otherwise.
// @throw: yes
//
tDirNodeReference CDirectoryService::OpenNamedNode(const char* nodename)
{
    long dirStatus = eDSNoErr;
    tDataListPtr nodePath = NULL;
    tDirNodeReference result = NULL;

    try
    {
        nodePath = ::dsDataListAllocate(mDir);
        ThrowIfNULL(nodePath);
        ThrowIfDSErr(::dsBuildListFromPathAlloc(mDir, nodePath, nodename, "/"));
        dirStatus = ::dsOpenDirNode(mDir, nodePath, &result);
        if (dirStatus == eDSNoErr)
        {
            // OK
        }
        else
        {
            result = NULL;
            ThrowIfDSErr(dirStatus);
        }
        dirStatus = ::dsDataListDeallocate(mDir, nodePath);
        free(nodePath);
    }
    catch(...)
    {
        if (nodePath != NULL)
        {
            dirStatus = ::dsDataListDeallocate(mDir, nodePath);
            free(nodePath);
            nodePath = NULL;
        }
        throw;
    }

    return result;
}

// CloseNode
//
// Close the node if previously open.
//
void CDirectoryService::CloseNode()
{
    if (mNode != 0L)
    {
        ::dsCloseDirNode(mNode);
        mNode = 0L;
    }
}

// CreateBuffer
//
// Create a data buffer for use with directory service calls.
//
// @throw: yes
//
void CDirectoryService::CreateBuffer()
{
    if (mData == NULL)
    {
        mData = ::dsDataBufferAllocate(mDir, cBufferSize);
        if (mData == NULL)
        {
            ThrowIfDSErr(eDSNullDataBuff);
        }
        mDataSize = cBufferSize;
    }
}

// RemoveBuffer
//
// Destroy the data buffer.
//
void CDirectoryService::RemoveBuffer()
{
    if (mData != NULL)
    {
        ::dsDataBufferDeAllocate(mDir, mData);
        mData = NULL;
    }
}

// ReallocBuffer
//
// Destroy the data buffer, then re-create with double previous size.
//
void CDirectoryService::ReallocBuffer()
{
    RemoveBuffer();
    mData = ::dsDataBufferAllocate(mDir, 2 * mDataSize);
    if (mData == NULL)
    {
        ThrowIfDSErr(eDSNullDataBuff);
    }
    mDataSize *= 2;
}

void CDirectoryService::BuildStringDataList(CFArrayRef strs, tDataListPtr data)
{
    CFStringUtil add_cfname((CFStringRef)::CFArrayGetValueAtIndex(strs, 0));
    std::auto_ptr<char> add_name(add_cfname.c_str());
    ThrowIfDSErr(::dsBuildListFromStringsAlloc(mDir, data,  add_name.get(), NULL));
    for(CFIndex i = 1; i < ::CFArrayGetCount(strs); i++)
    {
        add_cfname.reset((CFStringRef)::CFArrayGetValueAtIndex(strs, i));
        add_name.reset(add_cfname.c_str());
        ThrowIfDSErr(::dsAppendStringToListAlloc(mDir, data,  add_name.get()));
    }
}

void CDirectoryService::BuildStringDataListFromKeys(CFDictionaryRef strs, tDataListPtr data)
{
	CFStringRef strings[::CFDictionaryGetCount(strs)];
	::CFDictionaryGetKeysAndValues(strs, (const void**)&strings, NULL);
    CFStringUtil add_cfname(strings[0]);
    std::auto_ptr<char> add_name(add_cfname.c_str());
    ThrowIfDSErr(::dsBuildListFromStringsAlloc(mDir, data,  add_name.get(), NULL));
    for(CFIndex i = 1; i < ::CFDictionaryGetCount(strs); i++)
    {
        add_cfname.reset((CFStringRef)strings[i]);
        add_name.reset(add_cfname.c_str());
        ThrowIfDSErr(::dsAppendStringToListAlloc(mDir, data,  add_name.get()));
    }
}

// CStringFromBuffer
//
// Convert data in a buffer into a c-string.
//
// @return: the converted string.
//
char* CDirectoryService::CStringFromBuffer(tDataBufferPtr data)
{
    char* result = new char[data->fBufferLength + 1];
    ::strncpy(result, data->fBufferData, data->fBufferLength);
    result[data->fBufferLength] = 0;
    return result;
}

// CStringBase64FromBuffer
//
// Convert data in a buffer into a base64 encoded c-string.
//
// @return: the converted string.
//
char* CDirectoryService::CStringBase64FromBuffer(tDataBufferPtr data)
{
	return ::base64_encode((const unsigned char*)data->fBufferData, data->fBufferLength);
}

// CStringFromData
//
// Convert data to a c-string.
//
// @return: the converted string.
//
char* CDirectoryService::CStringFromData(const char* data, size_t len)
{
    char* result = new char[len + 1];
    ::strncpy(result, data, len);
    result[len] = 0;
    return result;
}
