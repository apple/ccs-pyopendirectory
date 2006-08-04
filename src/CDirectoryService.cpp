/**
 * A class that wraps high-level Directory Service calls needed by the
 * CalDAV server.
 **
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#include <stdlib.h>
#include <string.h>
#include <memory>

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

// ListUsers
// 
// List all users in the directory returning a list consisting of elements that contain attribute values
// for each user. The attributes are in order: uid, guid, last-modified, calendar-principal-uri.
//
// @return: CFMutableArrayRef composed of values which are CFMutableArrayRef which contain CFStringRef for each attribute,
//		    or NULL if it fails.
//
CFMutableArrayRef CDirectoryService::ListUsers()
{
	try
	{
		return ListRecords(kDSStdRecordTypeUsers);
	}
	catch(...)
	{
		return NULL;
	}
}

// ListGroups
// 
// List all groups in the directory returning a list consisting of elements that contain attribute values
// for each group. The attributes are in order: uid, guid, last-modified, calendar-principal-uri.
//
// @return: CFMutableArrayRef composed of values which are CFMutableArrayRef which contain CFStringRef for each attribute,
//		    or NULL if it fails.
//
CFMutableArrayRef CDirectoryService::ListGroups()
{
	try
	{
		return ListRecords(kDSStdRecordTypeGroups);
	}
	catch(...)
	{
		return NULL;
	}
}

// ListResources
// 
// List all resources in the directory returning a list consisting of elements that contain attribute values
// for each resource. The attributes are in order: uid, guid, last-modified, calendar-principal-uri.
//
// @return: CFMutableArrayRef composed of values which are CFMutableArrayRef which contain CFStringRef for each attribute,
//		    or NULL if it fails.
//
CFMutableArrayRef CDirectoryService::ListResources()
{
	try
	{
		return ListRecords(kDSStdRecordTypeResources);
	}
	catch(...)
	{
		return NULL;
	}
}

// CheckUser
// 
// Check whether the specified user exists in the directory.
//
// @param user: the uid of the user.
// @return: true if user is found, false otherwise.
//
bool CDirectoryService::CheckUser(const char* user)
{
	try
	{
		return HasRecord(kDSStdRecordTypeUsers, user);
	}
	catch(...)
	{
		return false;
	}
}

// CheckGroup
// 
// Check whether the specified group exists in the directory.
//
// @param user: the uid of the group.
// @return: true if group is found, false otherwise.
//
bool CDirectoryService::CheckGroup(const char* grp)
{
	try
	{
		return HasRecord(kDSStdRecordTypeGroups, grp);
	}
	catch(...)
	{
		return false;
	}
}

// CheckResource
// 
// Check whether the specified resource exists in the directory.
//
// @param user: the uid of the resource.
// @return: true if resource is found, false otherwise.
//
bool CDirectoryService::CheckResource(const char* rsrc)
{
	try
	{
		return HasRecord(kDSStdRecordTypeResources, rsrc);
	}
	catch(...)
	{
		return false;
	}
}


// Set of attributes to be looked up for a user record.
CFStringRef userattrs[] = {
	CFSTR(kDS1AttrDistinguishedName),
	CFSTR(kDS1AttrGeneratedUID),
	CFSTR(kDS1AttrModificationTimestamp),
	CFSTR(kDS1AttrCalendarPrincipalURI),
	NULL};

// ListUsersWithAttributes
// 
// Get specific attributes for one or more user records in the directory.
//
// @param users: a list of uids of the users.
// @return: CFMutableDictionaryRef composed of CFMutableDictionaryRef of CFStringRef key and value entries
//			for each attribute/value requested in the record indexed by uid,
//		    or NULL if it fails.
//
CFMutableDictionaryRef CDirectoryService::ListUsersWithAttributes(CFArrayRef users)
{
	CFMutableDictionaryRef result = NULL;
	CFMutableArrayRef attrs = NULL;
	try
	{
		// Build array of required attributes
		attrs = ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
		if (attrs == NULL)
			throw -1L;
		CFStringRef* str = userattrs;
		while(*str != NULL)
			::CFArrayAppendValue(attrs, *str++);
		
		// Get attribute map
		result = ListRecordsWithAttributes(kDSStdRecordTypeUsers, users, attrs);
		::CFRelease(attrs);
		return result;
	}
	catch(...)
	{
		if (attrs != NULL)
			::CFRelease(attrs);
		return NULL;
	}
}

// Set of attributes to be looked up for a group record.
CFStringRef grpattrs[] = {
	CFSTR(kDS1AttrDistinguishedName),
	CFSTR(kDS1AttrGeneratedUID),
	CFSTR(kDS1AttrModificationTimestamp),
	CFSTR(kDS1AttrCalendarPrincipalURI),
	CFSTR(kDSNAttrGroupMembers),
	NULL};

// ListGroupsWithAttributes
// 
// Get specific attributes for one or more group records in the directory.
//
// @param grps: a list of uids of the groups.
// @return: CFMutableDictionaryRef composed of CFMutableDictionaryRef of CFStringRef key and value entries
//			for each attribute/value requested in the record indexed by uid,
//		    or NULL if it fails.
//
CFMutableDictionaryRef CDirectoryService::ListGroupsWithAttributes(CFArrayRef grps)
{
	CFMutableDictionaryRef result = NULL;
	CFMutableArrayRef attrs = NULL;
	try
	{
		// Build array of required attributes
		attrs = ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
		if (attrs == NULL)
			throw -1L;
		CFStringRef* str = grpattrs;
		while(*str != NULL)
			::CFArrayAppendValue(attrs, *str++);
		
		// Get attribute map
		result = ListRecordsWithAttributes(kDSStdRecordTypeGroups, grps, attrs);
		::CFRelease(attrs);
		return result;
	}
	catch(...)
	{
		if (attrs != NULL)
			::CFRelease(attrs);
		return NULL;
	}
}

// Set of attributes to be looked up for a resource record.
CFStringRef rsrcattrs[] = {
	CFSTR(kDS1AttrDistinguishedName),
	CFSTR(kDS1AttrGeneratedUID),
	CFSTR(kDS1AttrModificationTimestamp),
	CFSTR(kDS1AttrCalendarPrincipalURI),
	NULL};

// ListResourcesWithAttributes
// 
// Get specific attributes for one or more resource records in the directory.
//
// @param rsrc: a list of uids of the resources.
// @return: CFMutableDictionaryRef composed of CFMutableDictionaryRef of CFStringRef key and value entries
//			for each attribute/value requested in the record indexed by uid,
//		    or NULL if it fails.
//
CFMutableDictionaryRef CDirectoryService::ListResourcesWithAttributes(CFArrayRef rsrcs)
{
	CFMutableDictionaryRef result = NULL;
	CFMutableArrayRef attrs = NULL;
	try
	{
		// Build array of required attributes
		attrs = ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
		if (attrs == NULL)
			throw -1L;
		CFStringRef* str = rsrcattrs;
		while(*str != NULL)
			::CFArrayAppendValue(attrs, *str++);
		
		// Get attribute map
		result = ListRecordsWithAttributes(kDSStdRecordTypeResources, rsrcs, attrs);
		::CFRelease(attrs);
		return result;
	}
	catch(...)
	{
		if (attrs != NULL)
			::CFRelease(attrs);
		return NULL;
	}
}

// AuthenticateUser
// 
// Authenticate a user to the directory.
//
// @param user: the uid of the user.
// @param pswd: the plain text password to authenticate with.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryService::AuthenticateUser(const char* user, const char* pswd)
{
	try
{
	return NativeAuthentication(user, pswd);
}
catch(...)
{
	return false;
}
}

#pragma mark -----Private API

// ListRecords
// 
// List all records of the specified type in the directory returning a list consisting of elements that contain attribute values
// for each user. The attributes are in order: uid, guid, last-modified, calendar-principal-uri.
//
// @return: CFMutableArrayRef composed of values which are CFMutableArrayRef which contain CFStringRef for each attribute,
//          or NULL if it fails.
//
CFMutableArrayRef CDirectoryService::ListRecords(const char* type)
{
	CFMutableArrayRef result = NULL;
	CFMutableArrayRef vresult = NULL;
	tDataListPtr recNames = NULL;
	tDataListPtr recTypes = NULL;
	tDataListPtr attrTypes = NULL;
	tContextData context = NULL;
	tAttributeListRef attrListRef = 0L;
	tRecordEntry* pRecEntry = NULL;
	
	try
	{
		// Make sure we have a valid directory service
		OpenService();
		
		// Open the node we want to query
		OpenNode();

		// We need a buffer for what comes next
		CreateBuffer();
		
		result = ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
		
		recNames = ::dsDataListAllocate(mDir);
		ThrowIfNULL(recNames);
		recTypes = ::dsDataListAllocate(mDir);
		ThrowIfNULL(recTypes);
		attrTypes = ::dsDataListAllocate(mDir);
		ThrowIfNULL(attrTypes);
		ThrowIfDSErr(::dsBuildListFromStringsAlloc(mDir, recNames,  kDSRecordsAll, NULL));
		ThrowIfDSErr(::dsBuildListFromStringsAlloc(mDir, recTypes,  type, NULL));
		ThrowIfDSErr(::dsBuildListFromStringsAlloc(mDir, attrTypes,  kDS1AttrGeneratedUID, kDS1AttrModificationTimestamp, kDS1AttrCalendarPrincipalURI, NULL));

		do
		{
			// List all the appropriate records
			unsigned long recCount = 0;
			ThrowIfDSErr(::dsGetRecordList(mNode, mData, recNames, eDSExact, recTypes, attrTypes, false, &recCount, &context));
			for(unsigned long i = 1; i <= recCount; i++)
			{
				// Get the record entry
				ThrowIfDSErr(::dsGetRecordEntry(mNode, mData, i, &attrListRef, &pRecEntry));

				// Get the entry's name
				char* temp = NULL;
				ThrowIfDSErr(::dsGetRecordNameFromEntry(pRecEntry, &temp));
				std::auto_ptr<char> recname(temp);

				// Create an array for the values
				vresult = ::CFArrayCreateMutable(kCFAllocatorDefault, 4, &kCFTypeArrayCallBacks);
				for(unsigned long j = 0; j < 4; j++)
				{
					CFStringUtil strvalue("");
					::CFArrayAppendValue(vresult, strvalue.get());
				}

				// Build a CFString from this name and add to results
				CFStringUtil str(recname.get());
				::CFArraySetValueAtIndex(vresult, 0, str.get());
				
				// Look at each requested attribute and get one value
				for(unsigned long j = 1; j <= pRecEntry->fRecordAttributeCount; j++)
				{
					tAttributeValueListRef attributeValueListRef = 0L;
					tAttributeEntryPtr attributeInfoPtr = NULL;

					ThrowIfDSErr(::dsGetAttributeEntry(mNode, mData, attrListRef, j, &attributeValueListRef, &attributeInfoPtr));
					
					if (attributeInfoPtr->fAttributeValueCount > 0)
					{
						// Determine what the attribute is and where in the result list it should be put
						std::auto_ptr<char> attrname(CStringFromBuffer(&attributeInfoPtr->fAttributeSignature));
						unsigned long attrindex = 0xFFFFFFFF;
						if (::strcmp(attrname.get(), kDS1AttrGeneratedUID) == 0)
							attrindex = 1;
						else if (::strcmp(attrname.get(), kDS1AttrModificationTimestamp) == 0)
							attrindex = 2;
						else if (::strcmp(attrname.get(), kDS1AttrCalendarPrincipalURI) == 0)
							attrindex = 3;
							
						// Get the attribute value and store in results
						tAttributeValueEntryPtr attributeValue = NULL;
						ThrowIfDSErr(::dsGetAttributeValue(mNode, mData, 1, attributeValueListRef, &attributeValue));
						std::auto_ptr<char> data(CStringFromBuffer(&attributeValue->fAttributeValueData));
						CFStringUtil strvalue(data.get());
						::CFArraySetValueAtIndex(vresult, attrindex, strvalue.get());
						::dsDeallocAttributeValueEntry(mDir, attributeValue);
						attributeValue = NULL;
					}

					::dsCloseAttributeValueList(attributeValueListRef);
					attributeValueListRef = NULL;
					::dsDeallocAttributeEntry(mDir, attributeInfoPtr);
					attributeInfoPtr = NULL;
				}

				// Add array of values to result array
				::CFArrayAppendValue(result, vresult);
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
	catch(...)
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

// HasRecord
// 
// Check whether the specified record with the specified type exists in the directory.
//
// @param type: the record type to check.
// @param name: the uid of the record to check.
// @return: true if record is found, false otherwise.
//
bool CDirectoryService::HasRecord(const char* type, const char* name)
{
	bool result = false;
	tDataNodePtr recName = NULL;
    tDataNodePtr recType = NULL;
	tRecordReference recRef = NULL;

	try
	{
		// Make sure we have a valid directory service
		OpenService();
		
		// Open the node we want to query
		OpenNode();
		
		recName = ::dsDataNodeAllocateString(mDir, name);
		if (recName == NULL)
			throw eDSBadDataNodeLength;
		recType = ::dsDataNodeAllocateString(mDir, type);
		if (recType == NULL)
			throw eDSBadDataNodeLength;
		
		long dirStatus = ::dsOpenRecord(mNode, recType, recName, &recRef);
		if (dirStatus == eDSNoErr)
		{
			result = true;
			::dsCloseRecord(recRef);
			recRef = NULL;
		}
		else if (dirStatus == eDSRecordNotFound)
			result = false;
		else
			throw dirStatus;
		
		// Cleanup
		::dsDataNodeDeAllocate(mDir, recType);
		recType = NULL;
		::dsDataNodeDeAllocate(mDir, recName);
		recName = NULL;
		CloseNode();
		CloseService();
	}
	catch(...)
	{
		// Cleanup
		if (recType != NULL)
			::dsDataNodeDeAllocate(mDir, recType);
		if (recName != NULL)
			::dsDataNodeDeAllocate(mDir, recName);
		CloseNode();
		CloseService();
		
		throw;
	}
	
	return result;
}

// ListRecordsWithAttributes
// 
// Get specific attributes for records of a specified type in the directory.
//
// @param type: the record type to check.
// @param names: the uids of the records to check.
// @param attrs: a list of attributes to return.
// @return: CFMutableDictionaryRef composed of CFMutableDictionaryRef of CFStringRef key and value entries
//			for each attribute/value requested in the record indexed by uid,
//		    or NULL if it fails.
//
CFMutableDictionaryRef CDirectoryService::ListRecordsWithAttributes(const char* type, CFArrayRef names, CFArrayRef attrs)
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
	
	// Must have names and attributes
	if ((::CFArrayGetCount(names) == 0) || (::CFArrayGetCount(attrs) == 0))
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
		BuildStringDataList(names, recNames);
		
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
			ThrowIfDSErr(::dsGetRecordList(mNode, mData, recNames, eDSExact, recTypes, attrTypes, false, &recCount, &context));
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
	catch(...)
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

// NativeAuthentication
// 
// Authenticate a user to the directory.
//
// @param user: the uid of the user.
// @param pswd: the plain text password to authenticate with.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryService::NativeAuthentication(const char* user, const char* pswd)
{
	// We need to find the 'native' node for the specifies user as the current node
	// made not support this user's authentication directly.
	
	CFMutableArrayRef users = ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	CFStringUtil cfuser(user);
	::CFArrayAppendValue(users, cfuser.get());
	
	CFMutableArrayRef attrs = ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	CFStringUtil cfattr(kDSNAttrMetaNodeLocation);
	::CFArrayAppendValue(attrs, cfattr.get());
	
	// First list the record for the current user and get its node.
	CFMutableDictionaryRef found = ListRecordsWithAttributes(kDSStdRecordTypeUsers, users, attrs);
	::CFRelease(users);
	::CFRelease(attrs);
	if (found == NULL)
		return false;

	// Now extract the returned data.
	CFDictionaryRef dictvalue = (CFDictionaryRef)CFDictionaryGetValue(found, cfuser.get());
	if ((dictvalue == NULL) || (::CFDictionaryGetCount(dictvalue) == 0))
	{
		::CFRelease(found);
		return false;
	}
	const void* value = ::CFDictionaryGetValue(dictvalue, cfattr.get());

	// The dictionary value may be a string or a list
	CFStringRef strvalue = NULL;
	if (::CFGetTypeID((CFTypeRef)value) == ::CFStringGetTypeID())
	{
		strvalue = (CFStringRef)value;
	}
	else if(::CFGetTypeID((CFTypeRef)value) == ::CFArrayGetTypeID())
	{
		CFArrayRef arrayvalue = (CFArrayRef)value;
		if (::CFArrayGetCount(arrayvalue) == 0)
			return false;
		strvalue = (CFStringRef)CFArrayGetValueAtIndex(arrayvalue, 0);
	}

	CFStringUtil cfvalue(strvalue);
	::CFRelease(found);
	return NativeAuthenticationToNode(cfvalue.temp_str(), user, pswd);
}

// NativeAuthenticationToNode
// 
// Authenticate a user to the directory.
//
// @param nodename: the node to authenticate to.
// @param user: the uid of the user.
// @param pswd: the plain text password to authenticate with.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryService::NativeAuthenticationToNode(const char* nodename, const char* user, const char* pswd)
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
		authType = ::dsDataNodeAllocateString(mDir, kDSStdAuthNodeNativeClearTextOK);
		/* authType = ::dsDataNodeAllocate(mDir, kDSStdAuthNodeNativeNoClearText); */

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
