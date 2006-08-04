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
