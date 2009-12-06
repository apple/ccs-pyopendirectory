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

#include "CDirectoryServiceAuth.h"

#include "CDirectoryServiceException.h"

#ifndef kDSStdAuthSASLProxy
#define	kDSStdAuthSASLProxy	"dsAuthMethodStandard:dsAuthSASLProxy"
#endif
#define kSASLDIGESTMD5 "DIGEST-MD5"

#pragma mark -----Public API

CDirectoryServiceAuth::CDirectoryServiceAuth() :
	CDirectoryService("")
{
}

CDirectoryServiceAuth::~CDirectoryServiceAuth()
{
	// Clean-up
	CloseService();
}

// AuthenticateUserBasic
//
// Authenticate a user to the directory using plain text credentials.
//
// @param nodename: the directory nodename for the user record.
// @param user: the identifier/directory record name of the user.
// @param pswd: the plain text password to authenticate with.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryServiceAuth::AuthenticateUserBasic(const char* nodename, const char* user, const char* pswd, bool& result, bool using_python)
{
    try
    {
        StPythonThreadState threading(using_python);

        result = NativeAuthenticationBasicToNode(nodename, user, pswd);
        return true;
    }
    catch(CDirectoryServiceException& dserror)
    {
		if (using_python)
	        dserror.SetPythonException();
        return false;
    }
    catch(...)
    {
        CDirectoryServiceException dserror;
		if (using_python)
	        dserror.SetPythonException();
        return false;
    }
}

// AuthenticateUserDigest
//
// Authenticate a user to the directory using HTTP DIGEST credentials.
//
// @param nodename: the directory nodename for the user record.
// @param user: the identifier/directory record name of the user.
// @param challenge: HTTP challenge sent by server.
// @param response: HTTP response sent by client.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryServiceAuth::AuthenticateUserDigest(const char* nodename, const char* user, const char* challenge, const char* response, const char* method, bool& result, bool using_python)
{
    try
    {
        StPythonThreadState threading(using_python);

        result = NativeAuthenticationDigestToNode(nodename, user, challenge, response, method);
        return true;
    }
    catch(CDirectoryServiceException& dserror)
    {
		if (using_python)
	        dserror.SetPythonException();
        return false;
    }
    catch(...)
    {
        CDirectoryServiceException dserror;
		if (using_python)
	        dserror.SetPythonException();
        return false;
    }
}

// AuthenticateUserDigestToActiveDirectory
//
// Authenticate a user to the directory using SASL Digest credentials.
//
// @param nodename: the directory nodename for the user record.
// @param user: the identifier/directory record name of the user.
// @param response: HTTP response sent by client.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryServiceAuth::AuthenticateUserDigestToActiveDirectory(const char* nodename, const char* user, const char* response, bool& result, bool using_python)
{
    try
    {
        StPythonThreadState threading(using_python);
		
        result = NativeAuthenticationSASLDigestToNode(nodename, user, response);
        return true;
    }
    catch(CDirectoryServiceException& dserror)
    {
		if (using_python)
	        dserror.SetPythonException();
        return false;
    }
    catch(...)
    {
        CDirectoryServiceException dserror;
		if (using_python)
	        dserror.SetPythonException();
        return false;
    }
}


// GetDigestMD5ChallengeFromActiveDirectory
//
// Authenticate a user to the directory using SASL Digest credentials.
//
// @param nodename: the directory nodename for the user record.
// @return: challange as CFStringRef
//
CFStringRef CDirectoryServiceAuth::GetDigestMD5ChallengeFromActiveDirectory(const char* nodename, bool using_python)
{
    try
    {
        StPythonThreadState threading(using_python);
		CFStringRef challenge = NULL;
        (void) NativeAuthenticationSASLDigestToNode(nodename, "anonymous", "", &challenge);
        return challenge;
    }
    catch(CDirectoryServiceException& dserror)
    {
		if (using_python)
	        dserror.SetPythonException();
        return NULL;
    }
    catch(...)
    {
        CDirectoryServiceException dserror;
		if (using_python)
	        dserror.SetPythonException();
        return NULL;
    }
}


#pragma mark -----Private API

// NativeAuthenticationBasicToNode
//
// Authenticate a user to the directory.
//
// @param nodename: the node to authenticate to.
// @param user: the identifier/directory record name of the user.
// @param pswd: the plain text password to authenticate with.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryServiceAuth::NativeAuthenticationBasicToNode(const char* nodename, const char* user, const char* pswd)
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
        UInt32 aDataBufSize = sizeof(UInt32) + ::strlen(user) + sizeof(UInt32) + ::strlen(pswd);
        authData = ::dsDataBufferAllocate(mDir, aDataBufSize);
        if (authData == NULL)
            ThrowIfDSErr(eDSNullDataBuff);

		// Fill the buffer
		::dsFillAuthBuffer(authData, 2,
						   ::strlen(user), user,
						   ::strlen(pswd), pswd);

        // Do authentication
        tDirStatus dirStatus = ::dsDoDirNodeAuth(node, authType, true,  authData,  mData, &context);
        result = (dirStatus == eDSNoErr);

        // Cleanup
        ::dsDataBufferDeAllocate(mDir, authData);
        authData = NULL;
        ::dsDataNodeDeAllocate(mDir, authType);
        authType = NULL;
        RemoveBuffer();

		// If fatal error, force full reset
		if (not result and (dirStatus != eDSAuthFailed))
		{
			CloseService();
		}
    }
    catch(...)
    {
        // Cleanup
        if (authData != NULL)
            ::dsDataBufferDeAllocate(mDir, authData);
        if (authType != NULL)
            ::dsDataNodeDeAllocate(mDir, authType);
        RemoveBuffer();
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
// @param user: the identifier/directory record name of the user.
// @param challenge: the server challenge.
// @param response: the client response.
// @param method: the HTTP method.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryServiceAuth::NativeAuthenticationDigestToNode(const char* nodename, const char* user,
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
        //            <length><challenge>
        //            <length><response>
        //            <length><method>
        //   Receive: success or failure.
        UInt32 aDataBufSize = sizeof(UInt32) + ::strlen(user) +
                              sizeof(UInt32) + ::strlen(challenge) +
                              sizeof(UInt32) + ::strlen(response) +
                              sizeof(UInt32) + ::strlen(method);
        authData = ::dsDataBufferAllocate(mDir, aDataBufSize);
        if (authData == NULL)
            ThrowIfDSErr(eDSNullDataBuff);
		
		// Fill the buffer
		::dsFillAuthBuffer(authData, ::strlen(method)?4:3,
						   ::strlen(user), user,
						   ::strlen(challenge), challenge,
						   ::strlen(response), response,
						   ::strlen(method), method);

        // Do authentication
        tDirStatus dirStatus = ::dsDoDirNodeAuth(node, authType, true,  authData,  mData, &context);
        result = (dirStatus == eDSNoErr);

        // Cleanup
        ::dsDataBufferDeAllocate(mDir, authData);
        authData = NULL;
        ::dsDataNodeDeAllocate(mDir, authType);
        authType = NULL;
        RemoveBuffer();

		// If fatal error, force full reset
		if (not result and (dirStatus != eDSAuthFailed))
		{
			CloseService();
		}
    }
    catch(...)
    {
        // Cleanup
        if (authData != NULL)
            ::dsDataBufferDeAllocate(mDir, authData);
        if (authType != NULL)
            ::dsDataNodeDeAllocate(mDir, authType);
        RemoveBuffer();
        CloseService();

        throw;
    }

    return result;
}

// NativeAuthenticationSASLDigestToNode
//
// Authenticate a user to the directory.
//
// @param nodename: the node to authenticate to.
// @param user: the identifier/directory record name of the user.
// @param sasldata: the client response.
// @param saslResult: returned step data.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryServiceAuth::NativeAuthenticationSASLDigestToNode(const char* nodename, const char* user, const char* sasldata, CFStringRef* saslResult)
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
        authType = ::dsDataNodeAllocateString(mDir, kDSStdAuthSASLProxy);

        // Build input data
        //  Native authentication is a one step authentication scheme.
        //  Step 1
        //      Send: <length><user>
        //            <length><saslmechansim>
        //            <length><sasldata>
        //   Receive: success or failure and 
		//			step data:
		//				<length><saslresultdata>
        UInt32 aDataBufSize = sizeof(UInt32) + ::strlen(user) +
                              sizeof(UInt32) + ::strlen(kSASLDIGESTMD5) +
                              sizeof(UInt32) + ::strlen(sasldata);
        authData = ::dsDataBufferAllocate(mDir, aDataBufSize);
        if (authData == NULL)
            ThrowIfDSErr(eDSNullDataBuff);
		
		// Fill the buffer
		::dsFillAuthBuffer(authData, 3,
						   ::strlen(user), user,
						   ::strlen(kSASLDIGESTMD5), kSASLDIGESTMD5,
						   ::strlen(sasldata), sasldata);

        // Do authentication
        tDirStatus dirStatus = ::dsDoDirNodeAuth(node, authType, true,  authData,  mData, &context);
        result = (dirStatus == eDSNoErr);
		
		if (result && (NULL != saslResult))
		{		
			// get first step data in string (always a string for kSASLDIGESTMD5)
			*saslResult = CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8*)&mData->fBufferData[4], *(UInt32*)&mData->fBufferData[0],
													kCFStringEncodingUTF8,	false);
		}

        // Cleanup
        ::dsDataBufferDeAllocate(mDir, authData);
        authData = NULL;
        ::dsDataNodeDeAllocate(mDir, authType);
        authType = NULL;
        RemoveBuffer();

		// If fatal error, force full reset
		if (not result and (dirStatus != eDSAuthFailed))
		{
			CloseService();
		}
    }
    catch(...)
    {
        // Cleanup
        if (authData != NULL)
            ::dsDataBufferDeAllocate(mDir, authData);
        if (authType != NULL)
            ::dsDataNodeDeAllocate(mDir, authType);
        RemoveBuffer();
        CloseService();

        throw;
    }

    return result;
}

/*
// NativeGenerateClientResponse
//
// Authenticate a user to the directory.
//
// @param nodename: the node to authenticate to.
// @param user: the identifier/directory record name of the user.
// @param sasldata: the client response.
// @param saslResult: returned step data.
// @return: true if authentication succeeds, false otherwise.
//
bool CDirectoryServiceAuth::NativeGenerateClientResponse(const char* serverChallenge, CFStringRef* response)
{
    bool result = false;
    tDirNodeReference node = 0L;
    tDataNodePtr authType = NULL;
    tDataBufferPtr authData = NULL;
    tContextData context = NULL;

    try
    {
		sasl_conn = ::SASLClientNewContext( callbacks, &sasl_context );
	// Client hashes the digest and responds
	result = ::sasl_client_step(sasl_conn, (char *)[serverChal bytes], [serverChal length], NULL, &data, &len);
	if (result != SASL_CONTINUE) {
		printf("sasl_client_step = %d\n", result);
		goto done;
	}

    }
    catch(...)
    {
        // Cleanup
        if (authData != NULL)
            ::dsDataBufferDeAllocate(mDir, authData);
        if (authType != NULL)
            ::dsDataNodeDeAllocate(mDir, authType);
        RemoveBuffer();
        CloseService();

        throw;
    }

    return result;
}
*/


// CloseService
//
// Close the directory service if previously open.
// Also close any open/cached nodes.
//
void CDirectoryServiceAuth::CloseService()
{
    if (mDir != 0L)
    {
		// Close all open nodes
		for(TNodeMap::const_iterator iter = mNodeMap.begin(); iter != mNodeMap.end(); iter++)
		{
            ::dsCloseDirNode((*iter).second);
		}
		mNodeMap.clear();
    }
	
	CDirectoryService::CloseService();
}

// OpenNamedNode
//
// Open a named node in the directory.
//
// @param nodename: the name of the node to open.
// @return: node reference if success, NULL otherwise.
// @throw: yes
//
tDirNodeReference CDirectoryServiceAuth::OpenNamedNode(const char* nodename)
{
	// Check cache first
    tDirNodeReference result = NULL;
	TNodeMap::const_iterator found = mNodeMap.find(nodename);
	if (found != mNodeMap.end())
	{
		return (*found).second;
	}
	
	// Create a new one and cache
	result = CDirectoryService::OpenNamedNode(nodename);
	mNodeMap[nodename] = result;
    return result;
}
