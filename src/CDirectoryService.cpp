/**
 * A class that wraps high-level Directory Service calls needed by the
 * CalDAV server.
 **
 * Copyright (c) 2006-2007 Apple Inc. All rights reserved.
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
 *
 * DRI: Cyrus Daboo, cdaboo@apple.com
 **/

#include "CDirectoryService.h"

#include "CFStringUtil.h"

#include <Python.h>

#include <stdlib.h>
#include <string.h>
#include <memory>

extern PyObject* ODException_class;

# define ThrowIfDSErr(x) { long dirStatus = x; if (dirStatus != eDSNoErr) throw dirStatus; }
# define ThrowIfNULL(x) { if (x == NULL) throw -1L; }

// This is copied from WhitePages
#define		kDSStdRecordTypeResources				"dsRecTypeStandard:Resources"

// Calendar attribute.
#define		kDS1AttrCalendarPrincipalURI			"dsAttrTypeStandard:CalendarPrincipalURI"

const int cBufferSize = 32 * 1024;		// 32K buffer for Directory Services operations

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
// @return: CFMutableDictionaryRef composed of CFMutableDictionaryRef of CFStringRef key and value entries
//			for each attribute/value requested in the record indexed by uid,
//		    or NULL if it fails.
//
CFMutableDictionaryRef CDirectoryService::ListAllRecordsWithAttributes(const char* recordType, CFArrayRef attributes)
{
	try
	{
		// Get attribute map
		return _ListAllRecordsWithAttributes(recordType, NULL, attributes);
	}
	catch(long dserror)
	{
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices Error", dserror));		
		return NULL;
	}
	catch(...)
	{
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "Unknown Error", -1));		
		return NULL;
	}
}

// AuthenticateUserBasic
// 
// Authenticate a user to the directory using plain text credentials.
//
// @param user: the uid of the user.
// @param pswd: the plain text password to authenticate with.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryService::AuthenticateUserBasic(const char* user, const char* pswd, bool& result)
{
	try
	{
		result = NativeAuthenticationBasic(user, pswd);
		return true;
	}
	catch(long dserror)
	{
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices Error", dserror));		
		return false;
	}
	catch(...)
	{
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "Unknown Error", -1));		
		return false;
	}
}

// AuthenticateUserDigest
// 
// Authenticate a user to the directory using HTTP DIGEST credentials.
//
// @param challenge: HTTP challenge sent by server.
// @param response: HTTP response sent by client.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryService::AuthenticateUserDigest(const char* user, const char* challenge, const char* response, const char* method, bool& result)
{
	try
	{
		result = NativeAuthenticationDigest(user, challenge, response, method);
		return true;
	}
	catch(long dserror)
	{
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices Error", dserror));		
		return false;
	}
	catch(...)
	{
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "Unknown Error", -1));		
		return false;
	}
}

#pragma mark -----Private API

// _ListAllRecordsWithAttributes
// 
// Get specific attributes for records of a specified type in the directory.
//
// @param type: the record type to check.
// @param attrs: a list of attributes to return.
// @return: CFMutableDictionaryRef composed of CFMutableDictionaryRef of CFStringRef key and value entries
//			for each attribute/value requested in the record indexed by uid,
//		    or NULL if it fails.
//
CFMutableDictionaryRef CDirectoryService::_ListAllRecordsWithAttributes(const char* type, CFArrayRef names, CFArrayRef attrs)
{
	CFMutableDictionaryRef result = NULL;
	CFMutableDictionaryRef vresult = NULL;
	CFMutableArrayRef values = NULL;
	tDataListPtr recNames = NULL;
	tDataListPtr recTypes = NULL;
	tDataListPtr attrTypes = NULL;
	tContextData context = NULL;
	tAttributeListRef attrListRef = 0L;
	tRecordEntry* pRecEntry = NULL;
	
	// Must have attributes
	if (::CFArrayGetCount(attrs) == 0)
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
		BuildStringDataList(attrs, attrTypes);
		
		result = ::CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		
		do
		{
			// List all the appropriate records
			unsigned long recCount = 0;
			tDirStatus err;
			do
			{
				err = ::dsGetRecordList(mNode, mData, recNames, eDSExact, recTypes, attrTypes, false, &recCount, &context);
				if (err == eDSBufferTooSmall)
					ReallocBuffer();
			} while(err == eDSBufferTooSmall);
			ThrowIfDSErr(err);
			for(unsigned long i = 1; i <= recCount; i++)
			{
				// Get the record entry
				ThrowIfDSErr(::dsGetRecordEntry(mNode, mData, i, &attrListRef, &pRecEntry));
				
				// Get the entry's name
				char* temp = NULL;
				ThrowIfDSErr(::dsGetRecordNameFromEntry(pRecEntry, &temp));
				std::auto_ptr<char> recname(temp);
				
				// Create a dictionary for the values
				vresult = ::CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
				
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
						
						if (attributeInfoPtr->fAttributeValueCount > 1)
						{
							values = ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

							for(unsigned long k = 1; k <= attributeInfoPtr->fAttributeValueCount; k++)
							{
								// Get the attribute value and store in results
								tAttributeValueEntryPtr attributeValue = NULL;
								ThrowIfDSErr(::dsGetAttributeValue(mNode, mData, k, attributeValueListRef, &attributeValue));
								std::auto_ptr<char> data(CStringFromBuffer(&attributeValue->fAttributeValueData));
								CFStringUtil strvalue(data.get());
								::CFArrayAppendValue(values, strvalue.get());
								::dsDeallocAttributeValueEntry(mDir, attributeValue);
								attributeValue = NULL;
							}
							::CFDictionarySetValue(vresult, cfattrname.get(), values);
							::CFRelease(values);
							values = NULL;
						}
						else
						{
							// Get the attribute value and store in results
							tAttributeValueEntryPtr attributeValue = NULL;
							ThrowIfDSErr(::dsGetAttributeValue(mNode, mData, 1, attributeValueListRef, &attributeValue));
							std::auto_ptr<char> data(CStringFromBuffer(&attributeValue->fAttributeValueData));
							CFStringUtil strvalue(data.get());
							::CFDictionarySetValue(vresult, cfattrname.get(), strvalue.get());
							::dsDeallocAttributeValueEntry(mDir, attributeValue);
							attributeValue = NULL;
						}
					}
					
					::dsCloseAttributeValueList(attributeValueListRef);
					attributeValueListRef = NULL;
					::dsDeallocAttributeEntry(mDir, attributeInfoPtr);
					attributeInfoPtr = NULL;
				}
				
				// Add dictionary of values to result array
				CFStringUtil str(recname.get());
				::CFDictionarySetValue(result, str.get(), vresult);
				::CFRelease(vresult);
				vresult = NULL;
				
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
	catch(long dsStatus)
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
		if (vresult != NULL)
		{
			::CFRelease(vresult);
			vresult = NULL;
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

// AuthenticationGetNode
// 
// Authenticate a user to the directory.
//
// @param user: the uid of the user.
// @return: CFStringUtil for node name.
//
CFStringRef CDirectoryService::AuthenticationGetNode(const char* user)
{
	// We need to find the 'native' node for the specifies user as the current node
	// made not support this user's authentication directly.
	CFStringRef result = NULL;

	CFMutableArrayRef users = ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	CFStringUtil cfuser(user);
	::CFArrayAppendValue(users, cfuser.get());
	
	CFMutableArrayRef attrs = ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	CFStringUtil cfattr(kDSNAttrMetaNodeLocation);
	::CFArrayAppendValue(attrs, cfattr.get());
	
	// First list the record for the current user and get its node.
	CFMutableDictionaryRef found = _ListAllRecordsWithAttributes(kDSStdRecordTypeUsers, users, attrs);
	::CFRelease(users);
	::CFRelease(attrs);
	if (found == NULL)
		return result;

	// Now extract the returned data.
	CFDictionaryRef dictvalue = (CFDictionaryRef)CFDictionaryGetValue(found, cfuser.get());
	if ((dictvalue == NULL) || (::CFDictionaryGetCount(dictvalue) == 0))
	{
		::CFRelease(found);
		return result;
	}
	const void* value = ::CFDictionaryGetValue(dictvalue, cfattr.get());

	// The dictionary value may be a string or a list
	if (::CFGetTypeID((CFTypeRef)value) == ::CFStringGetTypeID())
	{
		result = (CFStringRef)value;
	}
	else if(::CFGetTypeID((CFTypeRef)value) == ::CFArrayGetTypeID())
	{
		CFArrayRef arrayvalue = (CFArrayRef)value;
		if (::CFArrayGetCount(arrayvalue) == 0)
			return result;
		result = (CFStringRef)CFArrayGetValueAtIndex(arrayvalue, 0);
	}

	::CFRetain(result);
	::CFRelease(found);
	return result;
}

// NativeAuthenticationBasic
// 
// Authenticate a user to the directory.
//
// @param user: the uid of the user.
// @param pswd: the plain text password to authenticate with.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryService::NativeAuthenticationBasic(const char* user, const char* pswd)
{
	// We need to find the 'native' node for the specifies user as the current node
	// made not support this user's authentication directly.
	
	CFStringRef result = AuthenticationGetNode(user);
	if (result == NULL)
		return false;
	CFStringUtil cfvalue(result);
	::CFRelease(result);
	return NativeAuthenticationBasicToNode(cfvalue.temp_str(), user, pswd);
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
			throw eDSNullDataBuff;
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

// NativeAuthenticationDigest
// 
// Authenticate a user to the directory.
//
// @param user: the uid of the user.
// @param challenge: the server challenge.
// @param response: the client response.
// @param method: the HTTP method.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryService::NativeAuthenticationDigest(const char* user, const char* challenge, const char* response, const char* method)
{
	// We need to find the 'native' node for the specifies user as the current node
	// made not support this user's authentication directly.
	
	CFStringRef result = AuthenticationGetNode(user);
	if (result == NULL)
		return false;
	CFStringUtil cfvalue(result);
	::CFRelease(result);
	return NativeAuthenticationDigestToNode(cfvalue.temp_str(), user, challenge, response, method);
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
		//			  <length><challenge>
		//            <length><response>
		//			  <length><method>
		//   Receive: success or failure.
		long aDataBufSize = sizeof(long) + ::strlen(user) +
							sizeof(long) + ::strlen(challenge) +
							sizeof(long) + ::strlen(response) +
							sizeof(long) + ::strlen(method);
		authData = ::dsDataBufferAllocate(mDir, aDataBufSize);
		if (authData == NULL)
			throw eDSNullDataBuff;
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
			throw dirStatus;
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
			throw dirStatus;
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
			throw eDSNullDataBuff;
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
		throw eDSNullDataBuff;
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
