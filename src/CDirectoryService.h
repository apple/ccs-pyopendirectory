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
    
    CFMutableArrayRef ListAllRecordsWithAttributes(const char* recordType, CFArrayRef attributes);
    CFMutableArrayRef QueryRecordsWithAttribute(const char* attr, const char* value, int matchType, bool casei, const char* recordType, CFArrayRef attributes);
    CFMutableArrayRef QueryRecordsWithAttributes(const char* query, bool casei, const char* recordType, CFArrayRef attributes);

    bool AuthenticateUserBasic(const char* nodename, const char* user, const char* pswd, bool& result);
    bool AuthenticateUserDigest(const char* nodename, const char* user, const char* challenge, const char* response, const char* method, bool& result);
    
private:

    class StPythonThreadState
    {
    public:
        StPythonThreadState()
        {
            mSavedState = PyEval_SaveThread();
         }
        
        ~StPythonThreadState()
        {
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
    
    CFMutableArrayRef _ListAllRecordsWithAttributes(const char* type, CFArrayRef names, CFArrayRef attrs);
    CFMutableArrayRef _QueryRecordsWithAttributes(const char* attr, const char* value, int matchType, const char* compound, bool casei, const char* recordType, CFArrayRef attributes);

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

    char* CStringFromBuffer(tDataBufferPtr data);
    char* CStringFromData(const char* data, size_t len);
};
