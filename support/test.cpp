/**
 * Copyright (c) 2006-2009 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <CoreFoundation/CoreFoundation.h>
#include <DirectoryService/DirectoryService.h>

#include "CDirectoryService.h"
#include "CDirectoryServiceAuth.h"
#include "CFStringUtil.h"

tDirReference gDirRef = NULL;

char* CStringFromCFString(CFStringRef str);
void PrintDictionaryDictionary(const void* key, const void* value, void* ref);
void PrintDictionary(const void* key, const void* value, void* ref);
void PrintArrayArray(CFMutableArrayRef list);
void PrintArray(CFArrayRef list);
void AuthenticateUser(CDirectoryServiceAuth* dir, const char* nodename, const char* user, const char* pswd);
void AuthenticateUserDigest(CDirectoryServiceAuth* dir, const char* nodename, const char* user, const char* challenge, const char* response, const char* method);
void AuthenticateUserDigestToActiveDirectory(CDirectoryServiceAuth* dir, const char* nodename, const char* user, const char* response);
void GetDigestMD5ChallengeFromActiveDirectory(CDirectoryServiceAuth* dir, const char* nodename);

void AuthenticateUserDigestODAD(CDirectoryServiceAuth* dir, const char* nodename, const char* user, const char* pswd, bool verbose = false);

CFStringRef GetClientResponseFromSASL( const char* username, const char* pswd, const char* serverchallenge );
CFStringRef GetDigestMD5ChallengeFromSASL( void );

#define		kDSStdRecordTypeResources					"dsRecTypeStandard:Resources"
#define		kDSNAttrServicesLocator						"dsAttrTypeStandard:ServicesLocator"
#define		kDSNAttrJPEGPhoto						    "dsAttrTypeStandard:JPEGPhoto"

void ListNodes(CDirectoryService* dir)
{
	CFMutableArrayRef data = dir->ListNodes(false);
	if (data != NULL)
	{
		printf("\n*** Nodes: %ld ***\n", CFArrayGetCount(data));
		for(CFIndex i = 0; i < CFArrayGetCount(data); i++)
		{
			CFStringRef str = (CFStringRef)CFArrayGetValueAtIndex(data, i);
			const char* bytes = CFStringGetCStringPtr(str, kCFStringEncodingUTF8);
			
			if (bytes == NULL)
			{
				char localBuffer[256];
				Boolean success;
				success = CFStringGetCString(str, localBuffer, 256, kCFStringEncodingUTF8);
				printf("%ld: %s\n", i, localBuffer);
			}
			else
			{
				printf("%ld: %s\n", i, (const char*)bytes);
			}
		}
		CFRelease(data);
	}
}

void GetNodeAttributes(CDirectoryService* dir)
{
	CFStringRef attrs[2];
	attrs[0] = CFSTR(kDS1AttrSearchPath);
	attrs[1] = CFSTR(kDS1AttrReadOnlyNode);
	
	CFStringRef types[2];
	types[0] = CFSTR("str");
	types[1] = CFSTR("str");
	CFDictionaryRef attrsdict = CFDictionaryCreate(kCFAllocatorDefault, (const void **)attrs, (const void **)types, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	CFMutableDictionaryRef nodeData = dir->GetNodeAttributes("/Search", attrsdict, false);
	if (nodeData != NULL)
	{
		printf("\n*** Node Attributes: %ld ***\n", CFDictionaryGetCount(nodeData));
		CFDictionaryApplyFunction(nodeData, PrintDictionary, NULL);
		CFRelease(nodeData);
	}
	CFRelease(attrsdict);
}

void GetUserRecordDetails(CDirectoryService* dir)
{
	CFStringRef attrs[3];
	attrs[0] = CFSTR(kDS1AttrDistinguishedName);
	attrs[1] = CFSTR(kDS1AttrGeneratedUID);
	attrs[2] = CFSTR(kDSNAttrJPEGPhoto);
	
	CFStringRef types[3];
	types[0] = CFSTR("str");
	types[1] = CFSTR("str");
	types[2] = CFSTR("base64");
	CFDictionaryRef attrsdict = CFDictionaryCreate(kCFAllocatorDefault, (const void **)attrs, (const void **)types, 3, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	CFStringRef rtypes[1];
	rtypes[0] = CFSTR(kDSStdRecordTypeUsers);
	CFArrayRef recordTypes = CFArrayCreate(kCFAllocatorDefault, (const void**)rtypes, 1, &kCFTypeArrayCallBacks);
	
	CFMutableArrayRef data = dir->ListAllRecordsWithAttributes(recordTypes, attrsdict, 0, false);
	if (data != NULL)
	{
		printf("\n*** Users: %ld ***\n", CFArrayGetCount(data));
		for(CFIndex i = 0; i < CFArrayGetCount(data); i++)
		{
			CFArrayRef tuple = (CFArrayRef)CFArrayGetValueAtIndex(data, i);
			CFStringRef str = (CFStringRef)CFArrayGetValueAtIndex(tuple, 0);
			const char* bytes = CFStringGetCStringPtr(str, kCFStringEncodingUTF8);
			
			if (bytes == NULL)
			{
				char localBuffer[256];
				Boolean success;
				success = CFStringGetCString(str, localBuffer, 256, kCFStringEncodingUTF8);
				printf("%ld: %s\n", i, localBuffer);
			}
			else
			{
				printf("%ld: %s\n", i, (const char*)bytes);
			}
		}
		CFRelease(data);
	}
	else
	{
		printf("\nNo Users returned\n");
	}
	CFRelease(attrsdict);
}

void GetGroupRecordDetails(CDirectoryService* dir)
{
#if 0
	CFStringRef strings[2];
	strings[0] = CFSTR(kDSNAttrGroupMembers);
	strings[1] = CFSTR(kDS1AttrGeneratedUID);
	CFArrayRef array = CFArrayCreate(kCFAllocatorDefault, (const void **)strings, 2, &kCFTypeArrayCallBacks);
	
	CFMutableArrayRef data = dir->ListAllRecordsWithAttributes(kDSStdRecordTypeGroups, array);
	if (data != NULL)
	{
		printf("\n*** Groups: %d ***\n", CFArrayGetCount(data));
		for(CFIndex i = 0; i < CFArrayGetCount(data); i++)
		{
			CFArrayRef tuple = (CFArrayRef)CFArrayGetValueAtIndex(data, i);
			CFStringRef str = (CFStringRef)CFArrayGetValueAtIndex(tuple, 0);
			const char* bytes = CFStringGetCStringPtr(str, kCFStringEncodingUTF8);
			
			if (bytes == NULL)
			{
				char localBuffer[256];
				Boolean success;
				success = CFStringGetCString(str, localBuffer, 256, kCFStringEncodingUTF8);
				printf("%ld: %s\n", i, localBuffer);
			}
			else
			{
				printf("%ld: %s\n", i, (const char*)bytes);
			}
		}
		CFRelease(data);
	}
	else
	{
		printf("\nNo Groups returned\n");
	}
	CFRelease(array);
#endif
}

void GetSpecificUserRecordDetails(CDirectoryService* dir)
{
#if 0
	CFStringRef keys[2];
	keys[0] = CFSTR(kDS1AttrFirstName);
	keys[1] = CFSTR(kDS1AttrLastName);
	CFStringRef values[2];
	values[0] = CFSTR("cyrus");
	values[1] = CFSTR("daboo");
	CFDictionaryRef kvdict = CFDictionaryCreate(kCFAllocatorDefault, (const void **)keys, (const void**)values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	CFStringRef strings[2];
	strings[0] = CFSTR(kDS1AttrDistinguishedName);
	strings[1] = CFSTR(kDS1AttrGeneratedUID);
	CFArrayRef array = CFArrayCreate(kCFAllocatorDefault, (const void **)strings, 2, &kCFTypeArrayCallBacks);
	
	CFMutableDictionaryRef dict = dir->QueryRecordsWithAttributes(kvdict, eDSContains, false, false, kDSStdRecordTypeUsers, array);
	if (dict != NULL)
	{
		printf("\n*** Users: %d ***\n", CFDictionaryGetCount(dict));
		CFDictionaryApplyFunction(dict, PrintDictionaryDictionary, NULL);
		CFRelease(dict);
	}
	else
	{
		printf("\nNo Users returned\n");
	}
	CFRelease(array);
#endif
}

void GetResourcesRecordDetails(CDirectoryService* dir)
{
#if 0
	CFStringRef strings[2];
	strings[0] = CFSTR(kDS1AttrDistinguishedName);
	strings[1] = CFSTR(kDS1AttrXMLPlist);
	CFArrayRef array = CFArrayCreate(kCFAllocatorDefault, (const void **)strings, 2, &kCFTypeArrayCallBacks);
	
	CFMutableDictionaryRef dict = dir->QueryRecordsWithAttribute(kDSNAttrServicesLocator, "D9A8E41B", eDSStartsWith, false, kDSStdRecordTypeResources, array);
	if (dict != NULL)
	{
		printf("\n*** Computers: %d ***\n", CFDictionaryGetCount(dict));
		CFDictionaryApplyFunction(dict, PrintDictionaryDictionary, NULL);
		CFRelease(dict);
	}
	else
	{
		printf("\nNo Users returned\n");
	}
	CFRelease(array);
#endif
}

void GetCompoundResourcesRecordDetails(CDirectoryService* dir)
{
#if 0
	const char* compoundtest = "(&(|(dsAttrTypeStandard:RealName=U2*)(dsAttrTypeStandard:RealName=X S*))(dsAttrTypeStandard:ServicesLocator=D9A8E41B-C591-4D6B-A1CA-B57FFB8EF2F5:F967C034-54B8-4E65-9B38-7A6CD2600268:calendar))";
	
	CFStringRef strings[2];
	strings[0] = CFSTR(kDS1AttrDistinguishedName);
	strings[1] = CFSTR(kDS1AttrXMLPlist);
	CFArrayRef array = CFArrayCreate(kCFAllocatorDefault, (const void **)strings, 2, &kCFTypeArrayCallBacks);
	
	CFMutableDictionaryRef dict = dir->QueryRecordsWithAttributes(compoundtest, true, kDSStdRecordTypeResources, array);
	if (dict != NULL)
	{
		printf("\n*** Computers: %d ***\n", CFDictionaryGetCount(dict));
		CFDictionaryApplyFunction(dict, PrintDictionaryDictionary, NULL);
		CFRelease(dict);
	}
	else
	{
		printf("\nNo Users returned\n");
	}
	CFRelease(array);
#endif
}

int main (int argc, const char * argv[]) {

	//CDirectoryService* dir = new CDirectoryService("/Search");
	CDirectoryServiceAuth* authdir = new CDirectoryServiceAuth();

	//AuthenticateUser(authdir, "/LDAPv3/127.0.0.1", "oliverdaboo", "oliver");
	AuthenticateUser(authdir, "/LDAPv3/127.0.0.1", "eleanordaboo", "eleanor");

	{
		const char* n = "/LDAPv3/127.0.0.1";
		//const char* u = "test";
		//const char* c = "nonce=\"1\", qop=\"auth\", realm=\"Test\", algorithm=\"md5\", opaque=\"1\"";
		//const char* r = "username=\"test\", nonce=\"1\", cnonce=\"1\", nc=\"1\", realm=\"Test\", algorithm=\"md5\", opaque=\"1\", qop=\"auth\", uri=\"/\", response=\"4241f31ffe6f9c99b891f88e9c41caa9\"";
		//const char* c = "WWW-Authenticate: digest nonce=\"1621696297327727918745238639\", opaque=\"121994e78694cbdff74f12cb32ee6f00-MTYyMTY5NjI5NzMyNzcyNzkxODc0NTIzODYzOSwxMjcuMC4wLjEsMTE2ODU2ODg5NQ==\", realm=\"Test Realm\", algorithm=\"md5\", qop=\"auth\"";
		//const char* r = "Authorization: Digest username=\"test\", realm=\"Test Realm\", nonce=\"1621696297327727918745238639\", uri=\"/principals/users/test/\", response=\"e260f13cffcc15572ddeec9c31de437b\", opaque=\"121994e78694cbdff74f12cb32ee6f00-MTYyMTY5NjI5NzMyNzcyNzkxODc0NTIzODYzOSwxMjcuMC4wLjEsMTE2ODU2ODg5NQ==\", algorithm=\"md5\", cnonce=\"70cbd8f04227d8d46c0193b290beaf0d\", nc=00000001, qop=\"auth\"";
		const char* u = "";
		const char* c = "DIGEST-MD5";
		const char* r = "";
		AuthenticateUserDigest(authdir, n, u, c, r, "GET");
	}

	GetDigestMD5ChallengeFromActiveDirectory(authdir, "/Active Directory/All Domains");

	{
		const char* n = "/Active Directory/All Domains";
		const char* u = "";
		const char* r = "";
		AuthenticateUserDigestToActiveDirectory(authdir, n, u, r);
	}
	

	AuthenticateUser(authdir, "/Active Directory/All Domains", "servicetest", "pass");
	AuthenticateUserDigestODAD(authdir, "/Active Directory/All Domains", "servicetest", "pass");

	//AuthenticateUser(authdir, "/LDAPv3/127.0.0.1", "eleanordaboo", "eleanor");
	//AuthenticateUserDigestODAD(authdir, "/LDAPv3/127.0.0.1", "eleanordaboo", "eleanor");

	return 0;
}

void AuthenticateUser(CDirectoryServiceAuth* dir, const char* nodename, const char* user, const char* pswd)
{
	bool result = false;
	if (dir->AuthenticateUserBasic(nodename, user, pswd, result, false))
	{
		if (result)
			printf("AuthenticateUserBasic() success; nodename:\"%s\", user:\"%s\", pswd:\"%s\"\n", nodename, user, pswd );
		else
			printf("AuthenticateUserBasic() success, but auth failed; nodename:\"%s\", user:\"%s\", pswd:\"%s\"\n", nodename, user, pswd );
	}
	else
			printf("AuthenticateUserBasic() failed; nodename:\"%s\", user:\"%s\", pswd:\"%s\"\n", nodename, user, pswd );
}

void AuthenticateUserDigest(CDirectoryServiceAuth* dir, const char* nodename, const char* user, const char* challenge, const char* response, const char* method)
{
	bool result = false;
	if (dir->AuthenticateUserDigest(nodename, user, challenge, response, method, result, false))
	{
		if (result)
			printf("AuthenticateUserDigest() success; nodename:\"%s\", user:\"%s\", challenge:\"%s\", response:\"%s\", method:\"%s\"\n", nodename, user, challenge, response, method );
		else
			printf("AuthenticateUserDigest() success, but auth failed; user:\"%s\", user:\"%s\", challenge:\"%s\", response:\"%s\", method:\"%s\"\n", nodename, user, challenge, response, method );
	}
	else
			printf("AuthenticateUserDigest() failed, nodename:\"%s\", user:\"%s\", challenge:\"%s\", response:\"%s\", method:\"%s\"\n", nodename, user, challenge, response, method );
}


void AuthenticateUserDigestToActiveDirectory(CDirectoryServiceAuth* dir, const char* nodename, const char* user, const char* response)
{

	bool result = false;
	if (dir->AuthenticateUserDigestToActiveDirectory(nodename, user, response, result, false))
	{
		if (result)
			printf("AuthenticateUserDigestToActiveDirectory() success; nodename:\"%s\", user:\"%s\", response:\"%s\"\n", nodename, user, response );
		else
			printf("AuthenticateUserDigestToActiveDirectory() success, but auth failed; nodename:\"%s\", user:\"%s\", response:\"%s\"\n", nodename, user, response );
	}
	else
			printf("AuthenticateUserDigestToActiveDirectory() failed; nodename:\"%s\", user:\"%s\", response:\"%s\"\n", nodename, user, response );
}

void GetDigestMD5ChallengeFromActiveDirectory(CDirectoryServiceAuth* dir, const char* nodename)
{

	CFStringRef result = dir->GetDigestMD5ChallengeFromActiveDirectory(nodename, false);
	if (NULL != result)
	{
	    CFStringUtil s(result);
		printf("GetDigestMD5ChallengeFromActiveDirectory() success; nodename:\"%s\", challenge:\"%s\"\n", nodename, s.temp_str());

		CFRelease( result );
	}
	else
		printf("GetDigestMD5ChallengeFromActiveDirectory() failed; nodename:\"%s\"\n", nodename);
}


void AuthenticateUserDigestODAD(CDirectoryServiceAuth* dir, const char* nodename, const char* user, const char* pswd, bool verbose)
{
	CFStringRef challengeRef = NULL;
	CFStringRef responseRef = NULL;
	bool activeDirectory = (0 == strncmp(nodename, "/Active Directory/", strlen("/Active Directory/")));

	// first get server challange
	if (activeDirectory)
	{
		// get server challenge from AD
		challengeRef = dir->GetDigestMD5ChallengeFromActiveDirectory(nodename, false);
		if (NULL != challengeRef)
		{
			if (verbose) {
				CFStringUtil s(challengeRef);
				printf("GetDigestMD5ChallengeFromActiveDirectory() success; nodename:\"%s\", challenge:\"%s\"\n", nodename, s.temp_str());
			}
		}
		else
			printf("GetDigestMD5ChallengeFromActiveDirectory() failed; nodename:\"%s\"\n", nodename);
	}
	else
	{
		// get server challenge from AD
		challengeRef = GetDigestMD5ChallengeFromSASL();
		if (NULL != challengeRef)
		{
			if (verbose) {
				CFStringUtil s(challengeRef);
				printf("GetDigestMD5ChallengeFromSASL() success; nodename:\"%s\", challenge:\"%s\"\n", nodename, s.temp_str());
			}
		}
		else
			printf("GetDigestMD5ChallengeFromSASL() failed; nodename:\"%s\"\n", nodename);
	}

	if (NULL != challengeRef)
	{
		
		CFStringUtil challengeStrUtil(challengeRef);
		const char* challenge = challengeStrUtil.temp_str();

		// now create response 
		responseRef = GetClientResponseFromSASL( user, pswd, challenge );
		CFStringUtil responseStrUtil(responseRef);
		const char* response = responseStrUtil.temp_str();

		if (NULL != responseRef)
		{
			if (verbose) {
				printf("GetClientResponseFromSASL() success; user:\"%s\", pswd:\"%s\", challenge:\"%s\", response:\"%s\"\n", user, pswd, challenge, response);
			}
		}
		else
		{
			printf("GetClientResponseFromSASL() failed; user:\"%s\", pswd:\"%s\", challenge:\"%s\"\n", user, pswd, challenge);
			goto exit;
		}
			
		if (activeDirectory)
		{
			// now auth
			bool result = false;
			if (dir->AuthenticateUserDigestToActiveDirectory(nodename, user, response, result, false))
			{
				if (result)
					printf("AuthenticateUserDigestToActiveDirectory() success; nodename:\"%s\", user:\"%s\", response:\"%s\"\n", nodename, user, response );
				else
					printf("AuthenticateUserDigestToActiveDirectory() success, but auth failed; nodename:\"%s\", user:\"%s\", response:\"%s\"\n", nodename, user, response );
			}
			else
				printf("AuthenticateUserDigestToActiveDirectory() failed; nodename:\"%s\", user:\"%s\", response:\"%s\"\n", nodename, user, response );
		}
		else 
		{
			bool result = false;
			const char* method = "";
			if (dir->AuthenticateUserDigest(nodename, user, challenge, response, method, result, false))
			{
				if (result)
					printf("AuthenticateUserDigest() success; nodename:\"%s\", user:\"%s\", challenge:\"%s\", response:\"%s\", method:\"%s\"\n", nodename, user, challenge, response, method );
				else
					printf("AuthenticateUserDigest() success, but auth failed; nodename:\"%s\", user:\"%s\", challenge:\"%s\", response:\"%s\", method:\"%s\"\n", nodename, user, challenge, response, method );
			}
			else
				printf("AuthenticateUserDigest() failed, nodename:\"%s\", user:\"%s\", challenge:\"%s\", response:\"%s\", method:\"%s\"\n", nodename, user, challenge, response, method );
		}
	}

exit:
	
	if (NULL != challengeRef)
		CFRelease( challengeRef );
	if (NULL != responseRef)
		CFRelease( responseRef );

}

void CFDictionaryIterator(const void* key, const void* value, void* ref)
{
	CFStringRef strkey = (CFStringRef)key;
	CFStringRef strvalue = (CFStringRef)value;

	char* pystrkey = CStringFromCFString(strkey);
	char* pystrvalue = CStringFromCFString(strvalue);


	printf("%s: %s\n", pystrkey, pystrvalue);

	free(pystrkey);
	free(pystrvalue);
}

char* CStringFromCFString(CFStringRef str)
{
	const char* bytes = CFStringGetCStringPtr(str, kCFStringEncodingUTF8);

	if (bytes == NULL)
	{
		char localBuffer[256];
		localBuffer[0] = 0;
		Boolean success = ::CFStringGetCString(str, localBuffer, 256, kCFStringEncodingUTF8);
		if (!success)
			localBuffer[0] = 0;
		return ::strdup(localBuffer);
	}
	else
	{
		return ::strdup(bytes);
	}
}

void PrintDictionaryDictionary(const void* key, const void* value, void* ref)
{
	CFStringUtil strkey((CFStringRef)key);
	CFDictionaryRef dictvalue = (CFDictionaryRef)value;

	printf("Dictionary Entry: \"%s\"\n", strkey.temp_str());
	CFDictionaryApplyFunction(dictvalue, PrintDictionary, NULL);
	printf("\n");
}

void PrintDictionary(const void* key, const void* value, void* ref)
{
	CFStringUtil strkey((CFStringRef)key);
	if (CFGetTypeID((CFTypeRef)value) == CFStringGetTypeID())
	{
		CFStringUtil strvalue((CFStringRef)value);

		printf("Key: \"%s\"; Value: \"%s\"\n", strkey.temp_str(), strvalue.temp_str());
	}
	else if(CFGetTypeID((CFTypeRef)value) == CFArrayGetTypeID())
	{
		CFArrayRef arrayvalue = (CFArrayRef)value;
		printf("Key: \"%s\"; Value: Array:\n", strkey.temp_str());
		PrintArray(arrayvalue);
		printf("---\n");
	}
}

CFComparisonResult CompareRecordListValues(const void *val1, const void *val2, void *context)
{
	CFMutableArrayRef l1 = (CFMutableArrayRef)val1;
	CFMutableArrayRef l2 = (CFMutableArrayRef)val2;
	CFIndex c1 = CFArrayGetCount(l1);
	CFIndex c2 = CFArrayGetCount(l2);
	if ((c1 > 0) && (c2 > 0))
	{
		return CFStringCompare((CFStringRef)CFArrayGetValueAtIndex(l1, 0), (CFStringRef)CFArrayGetValueAtIndex(l2, 0), NULL);
	}
	else if (c1 > 0)
		return kCFCompareGreaterThan;
	else if (c2 > 0)
		return kCFCompareLessThan;
	else
		return kCFCompareEqualTo;
}

void PrintArrayArray(CFMutableArrayRef list)
{
	CFArraySortValues(list, CFRangeMake(0, CFArrayGetCount(list)), (CFComparatorFunction)CompareRecordListValues, NULL);
	for(CFIndex i = 0; i < CFArrayGetCount(list); i++)
	{
		CFMutableArrayRef array = (CFMutableArrayRef)CFArrayGetValueAtIndex(list, i);
		printf("Index: %ld\n", i);
		PrintArray(array);
		printf("\n");
	}
}

void PrintArray(CFArrayRef list)
{
	//CFArraySortValues(list, CFRangeMake(0, CFArrayGetCount(list)), (CFComparatorFunction)CFStringCompare, NULL);
	for(CFIndex i = 0; i < CFArrayGetCount(list); i++)
	{
		CFStringRef str = (CFStringRef)CFArrayGetValueAtIndex(list, i);
		const char* bytes = CFStringGetCStringPtr(str, kCFStringEncodingUTF8);

		if (bytes == NULL)
		{
			char localBuffer[256];
			Boolean success;
			success = CFStringGetCString(str, localBuffer, 256, kCFStringEncodingUTF8);
			printf("%ld: %s\n", i, localBuffer);
		}
		else
		{
			printf("%ld: %s\n", i, (const char*)bytes);
		}
	}
}


#pragma mark ----- SASL calls

#include <sasl/sasl.h>

#define kSASLMinSecurityFactor		0
#define kSASLMaxSecurityFactor		65535
#define kSASLMaxBufferSize			65536
#define kSASLSecurityFlags			0
#define kSASLPropertyNames			(NULL)
#define kSASLPropertyValues			(NULL)

typedef struct saslContext {
	const char *user;
	const char *pass;
} saslContext;

typedef int sasl_cbproc();


int getrealm(void *context /*__attribute__((unused))*/, 
		    int cb_id,
		    const char **availrealms,
		    const char **result)
{
	#pragma unused (context)
	
    /* paranoia check */
    if (cb_id != SASL_CB_GETREALM) return SASL_BADPARAM;
    if (!result) return SASL_BADPARAM;

    if ( availrealms ) {
        *result = *availrealms;
    }
    
    return SASL_OK;
}

int simple(void *context /*__attribute__((unused))*/,
		  int cb_id,
		  const char **result,
		  unsigned *len)
{
	saslContext *text = (saslContext *)context;
	
    //syslog(LOG_INFO, "in simple\n");

    /* paranoia check */
    if ( result == NULL )
        return SASL_BADPARAM;
    	
    *result = NULL;
    
    switch (cb_id) {
        case SASL_CB_USER:
            *result = text->user;
			break;
		
		case SASL_CB_AUTHNAME:
            *result = text->user;
            break;
            
        default:
            return SASL_BADPARAM;
    }
    
    if (*result != NULL && len != NULL)
        *len = strlen(*result);
    
    return SASL_OK;
}


int
getsecret(sasl_conn_t *conn,
	  void *context /*__attribute__((unused))*/,
	  int cb_id,
	  sasl_secret_t **psecret)
{
	saslContext *text = (saslContext *)context;
	
    //syslog(LOG_INFO, "in getsecret\n");

    /* paranoia check */
    if (! conn || ! psecret || cb_id != SASL_CB_PASS)
        return SASL_BADPARAM;
	
	size_t pwdLen = strlen(text->pass);
    *psecret = (sasl_secret_t *) malloc( sizeof(sasl_secret_t) + pwdLen );
	(*psecret)->len = pwdLen;
	strcpy((char *)(*psecret)->data, text->pass);
    
    return SASL_OK;
}




//----------------------------------------------------------------------------------------
//	SASLClientNewContext
//
//	Returns: A SASL context, or NULL
//
//	<callbacks> must be an array with capacity for at least 5 items
//----------------------------------------------------------------------------------------

sasl_conn_t *SASLClientNewContext( sasl_callback_t *callbacks, saslContext *context )
{
	int result = 0;
	sasl_conn_t *sasl_conn = NULL;
	sasl_security_properties_t secprops = { kSASLMinSecurityFactor, kSASLMaxSecurityFactor,
											kSASLMaxBufferSize, kSASLSecurityFlags,
											kSASLPropertyNames, kSASLPropertyValues };
	
	result = sasl_client_init( NULL );
	//printf( "sasl_client_init = %d\n", result );
	if ( result != SASL_OK )
		return NULL;
	
	// callbacks we support
	callbacks[0].id = SASL_CB_GETREALM;
	callbacks[0].proc = (sasl_cbproc *)&getrealm;
	callbacks[0].context = context;
	
	callbacks[1].id = SASL_CB_USER;
	callbacks[1].proc = (sasl_cbproc *)&simple;
	callbacks[1].context = context;
	
	callbacks[2].id = SASL_CB_AUTHNAME;
	callbacks[2].proc = (sasl_cbproc *)&simple;
	callbacks[2].context = context;
	
	callbacks[3].id = SASL_CB_PASS;
	callbacks[3].proc = (sasl_cbproc *)&getsecret;
	callbacks[3].context = context;
	
	callbacks[4].id = SASL_CB_LIST_END;
	callbacks[4].proc = NULL;
	callbacks[4].context = NULL;
	
	result = sasl_client_new( "http", "servicetest.example.com", NULL, NULL, callbacks, 0, &sasl_conn );
	//printf( "sasl_client_new = %d\n", result );
	if ( result != SASL_OK )
		return NULL;
	
	result = sasl_setprop( sasl_conn, SASL_SEC_PROPS, &secprops );
	//printf( "sasl_setprop = %d\n", result );
	if ( result != SASL_OK ) {
		sasl_dispose( &sasl_conn );
		return NULL;
	}
	
	return sasl_conn;
}


CFStringRef GetClientResponseFromSASL( const char* username, const char* pswd, const char* serverchallenge )
{
	int result = 0;
	sasl_callback_t callbacks[5] = {{0}};
	saslContext sasl_context = { NULL, NULL };
	sasl_conn_t *sasl_conn = NULL;
	const char *data = NULL;
    unsigned int len = 0;
	const char *chosenmech = NULL;
	CFStringRef response = NULL;
	
	sasl_context.user = username;
	sasl_context.pass = pswd;
	
	// Client's first move
	sasl_conn = SASLClientNewContext( callbacks, &sasl_context );
	
	result = sasl_client_start( sasl_conn, "DIGEST-MD5", NULL, &data, &len, &chosenmech ); 
	//printf( "sasl_client_start = %d, len = %d\n", result, len );
	if ( result != SASL_CONTINUE )
		goto done;
	
	// Client hashes the digest and responds
	result = sasl_client_step(sasl_conn, serverchallenge, strlen(serverchallenge), NULL, &data, &len);
	if (result != SASL_CONTINUE) {
		printf("sasl_client_step = %d\n", result);
		goto done;
	}
		
	response = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8*)data, len, kCFStringEncodingUTF8, false);

	
done:
	if ( sasl_conn != NULL )
		sasl_dispose( &sasl_conn );

	return response;
}

//----------------------------------------------------------------------------------------
//	GetDigestMD5ChallengeFromSASL
//
//	Returns: A server challenge for DIGEST-MD5 authentication
//----------------------------------------------------------------------------------------

CFStringRef GetDigestMD5ChallengeFromSASL( void )
{
	int result = 0;
	sasl_conn_t *sasl_server_conn = NULL;
	const char *serverOut = NULL;
	unsigned int serverOutLen = 0;
	CFStringRef challenge = NULL;
	
	// Passing a minimum security factor of 2 or more triggers the latest digest-md5 specification.
	// algorithm=md5-sess,qop=auth-conf.
	sasl_security_properties_t secprops = { 2, kSASLMaxSecurityFactor,
										kSASLMaxBufferSize, kSASLSecurityFlags,
										kSASLPropertyNames, kSASLPropertyValues };
		
	// Get a challenge from SASL	
	result = sasl_server_init_alt( NULL, "AppName" );
	if ( result != SASL_OK ) {
		printf( "sasl_server_init_alt = %d\n", result );
		goto done;
	}
	
	//"127.0.0.1;80"
	result = sasl_server_new( "http", "servicetest.example.com", NULL, NULL, NULL, NULL, 0, &sasl_server_conn );
	if ( result != SASL_OK ) {
		printf( "sasl_server_init_alt = %d\n", result );
		goto done;
	}
	
	result = sasl_setprop( sasl_server_conn, SASL_SEC_PROPS, &secprops );
	
	result = sasl_server_start( sasl_server_conn, "DIGEST-MD5", NULL, 0, &serverOut, &serverOutLen );
	if ( result != SASL_CONTINUE ) {
		printf( "sasl_server_start = %d\n", result );
		goto done;
	}
	
	challenge = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8*)serverOut, serverOutLen, kCFStringEncodingUTF8, false);

done:
	if ( sasl_server_conn != NULL )
		sasl_dispose( &sasl_server_conn );
	
	return challenge;
}



