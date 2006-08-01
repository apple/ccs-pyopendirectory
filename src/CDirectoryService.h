/**
 * A class that wraps high-level Directory Service calls needed by the
 * CalDAV server.
 **
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * DRI: Cyrus Daboo, cdaboo@apple.com
 **/

#pragma once

#include <CoreFoundation/CoreFoundation.h>
#include <DirectoryService/DirectoryService.h>

class CDirectoryService
{
public:
	CDirectoryService(const char* nodename);
	~CDirectoryService();
	
	CFMutableArrayRef ListUsers();
	CFMutableArrayRef ListGroups();
	CFMutableArrayRef ListResources();
	
	bool CheckUser(const char* user);
	bool CheckGroup(const char* grp);
	bool CheckResource(const char* rsrc);
	
	CFMutableDictionaryRef ListUsersWithAttributes(CFArrayRef users);
	CFMutableDictionaryRef ListGroupsWithAttributes(CFArrayRef grps);
	CFMutableDictionaryRef ListResourcesWithAttributes(CFArrayRef rsrcs);

	bool AuthenticateUser(const char* user, const char* pswd);
	
private:
	const char*			mNodeName;
	tDirReference		mDir;
	tDirNodeReference	mNode;
	tDataBufferPtr		mData;
	
	CFMutableArrayRef ListRecords(const char* type);
	CFMutableDictionaryRef ListRecordsWithAttributes(const char* type, CFArrayRef names, CFArrayRef attrs);

	bool HasRecord(const char* type, const char* name);

	bool NativeAuthentication(const char* user, const char* pswd);
	bool NativeAuthenticationToNode(const char* nodename, const char* user, const char* pswd);
	
	void OpenService();
	void CloseService();
	
	void OpenNode();
	tDirNodeReference OpenNamedNode(const char* nodename);
	void CloseNode();
	
	void CreateBuffer();
	void RemoveBuffer();

	void BuildStringDataList(CFArrayRef strs, tDataListPtr data);

	char* CStringFromBuffer(tDataBufferPtr data);
	char* CStringFromData(const char* data, size_t len);
};
