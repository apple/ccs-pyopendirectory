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

#pragma once

#include <CoreFoundation/CoreFoundation.h>
#include <DirectoryService/DirectoryService.h>
#include <Python.h>

class CFStringUtil;

class CDirectoryService
{
public:
    CDirectoryService(const char* nodename);
    ~CDirectoryService();

    CFMutableArrayRef ListAllRecordsWithAttributes(CFArrayRef recordTypes, CFDictionaryRef attributes, UInt32 maxRecordCount=0, bool using_python=true);
    CFMutableArrayRef QueryRecordsWithAttribute(const char* attr, const char* value, int matchType, bool casei, CFArrayRef recordTypes, CFDictionaryRef attributes, UInt32 maxRecordCount=0, bool using_python=true);
    CFMutableArrayRef QueryRecordsWithAttributes(const char* query, bool casei, CFArrayRef recordTypes, CFDictionaryRef attributes, UInt32 maxRecordCount=0, bool using_python=true);

    bool AuthenticateUserBasic(const char* nodename, const char* user, const char* pswd, bool& result, bool using_python=true);
    bool AuthenticateUserDigest(const char* nodename, const char* user, const char* challenge, const char* response, const char* method, bool& result, bool using_python=true);

private:

    class StPythonThreadState
    {
    public:
        StPythonThreadState(bool using_python=true)
        {
			if (using_python)
				mSavedState = PyEval_SaveThread();
			else
				mSavedState = NULL;
		}

        ~StPythonThreadState()
        {
			if (mSavedState != NULL)
				PyEval_RestoreThread(mSavedState);
        }

    private:
        PyThreadState* mSavedState;
    };

    const char*           mNodeName;
    tDirReference         mDir;
    tDirNodeReference     mNode;
    tDataBufferPtr        mData;
    UInt32                mDataSize;

    CFMutableArrayRef _ListAllRecordsWithAttributes(CFArrayRef recordTypes, CFArrayRef names, CFDictionaryRef attrs, UInt32 maxRecordCount);
    CFMutableArrayRef _QueryRecordsWithAttributes(const char* attr, const char* value, int matchType, const char* compound, bool casei, CFArrayRef recordTypes, CFDictionaryRef attrs, UInt32 maxRecordCount);

    bool NativeAuthenticationBasicToNode(const char* nodename, const char* user, const char* pswd);
    bool NativeAuthenticationDigestToNode(const char* nodename, const char* user, const char* challenge, const char* response, const char* method);

    void OpenService();
    void CloseService();

    void OpenNode();
    tDirNodeReference OpenNamedNode(const char* nodename);
    void CloseNode();

    void CreateBuffer();
    void RemoveBuffer();
    void ReallocBuffer();

    void BuildStringDataList(CFArrayRef strs, tDataListPtr data);
    void BuildStringDataListFromKeys(CFDictionaryRef strs, tDataListPtr data);

    char* CStringFromBuffer(tDataBufferPtr data);
    char* CStringBase64FromBuffer(tDataBufferPtr data);
    char* CStringFromData(const char* data, size_t len);
};
