/**
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
 *
 * DRI: Cyrus Daboo, cdaboo@apple.com
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <CoreFoundation/CoreFoundation.h>
#include <DirectoryService/DirectoryService.h>

#include "CDirectoryService.h"
#include "CFStringUtil.h"

tDirReference gDirRef = NULL;

void ListNodes ( void );
void FindNodes ( char* inNodePath );
void NodeInfo ( const tDirNodeReference nodeRef );
long MyOpenDirNode ( tDirNodeReference *outNodeRef, char* inNodePath );
long GetRecordList ( const tDirNodeReference nodeRef );
char* CStringFromCFString(CFStringRef str);
void PrintDictionaryDictionary(const void* key, const void* value, void* ref);
void PrintDictionary(const void* key, const void* value, void* ref);
void PrintArrayArray(CFMutableArrayRef list);
void PrintArray(CFMutableArrayRef list);
void PrintNodeName ( tDataListPtr inNode );
void CheckUser(CDirectoryService* dir, const char* user);
void CheckGroup(CDirectoryService* dir, const char* grp);
void CheckResource(CDirectoryService* dir, const char* rsrc);
void UserAttributes(CDirectoryService* dir, const char* user);
void AuthenticateUser(CDirectoryService* dir, const char* user, const char* pswd);

int main (int argc, const char * argv[]) {
#if 0
    // insert code here...
	long dirStatus = eDSNoErr;
	tDirNodeReference nodeRef = NULL;
    dirStatus = dsOpenDirService( &gDirRef );
    if ( dirStatus == eDSNoErr )
    {
        //ListNodes();
		//FindNodes("/LDAPv3/webboserver.apple.com");
		
        dirStatus = MyOpenDirNode( &nodeRef, "/LDAPv3/webboserver.apple.com" );
        if ( dirStatus == eDSNoErr )
        {
			//	NodeInfo(nodeRef);
			GetRecordList( nodeRef );
            dsCloseDirNode( nodeRef );
        }
    }
    if ( gDirRef != NULL )
    {
        dirStatus = dsCloseDirService( gDirRef );
    }
#else
    
	CDirectoryService* dir = new CDirectoryService("/LDAPv3/webboserver.apple.com");
	CFMutableArrayRef list = dir->ListResources();
	if (list != NULL)
	{
		printf("\n*** Users: %d ***\n", CFArrayGetCount(list));
		//PrintArrayArray(list);
		//CFRelease(list);
	}
	else
	{
		printf("No Users returned");
	}

	CFMutableArrayRef users = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	for(CFIndex i = 0; i < CFArrayGetCount(list); i++)
		CFArrayAppendValue(users, CFArrayGetValueAtIndex((CFArrayRef)CFArrayGetValueAtIndex(list, i), 0));

	CFMutableDictionaryRef dict = dir->ListResourcesWithAttributes(users);
	if (dict != NULL)
	{
		printf("\n*** Users: %d ***\n", CFDictionaryGetCount(dict));
		CFDictionaryApplyFunction(dict, PrintDictionaryDictionary, NULL);
		CFRelease(dict);
	}
	else
	{
		printf("No Users returned");
	}
#if 0
	list = dir->ListGroups();
	if (list != NULL)
	{
		printf("\n*** Groups ***\n");
		PrintArrayArray(list);
		CFRelease(list);
	}
	else
	{
		printf("No Groups returned");
	}
	list = dir->ListResources();
	if (list != NULL)
	{
		printf("\n*** Resources ***\n");
		PrintArrayArray(list);
		CFRelease(list);
	}
	else
	{
		printf("No Resources returned");
	}
	
	CheckUser(dir, "cyrusdaboo");
	CheckUser(dir, "chris");
	CheckGroup(dir, "sangriafest");
	CheckGroup(dir, "cyrusdaboo");
	CheckResource(dir, "Attitude Adjuster");
	CheckResource(dir, "cyrusdaboo");
	
	AuthenticateUser(dir, "cyrusdaboo", "54321");
	CFMutableArrayRef list = dir->ListUsers();
	PrintArrayArray(list);
	CFRelease(list);
	UserAttributes(dir, "cyrusdaboo");
	UserAttributes(dir, "oliverdaboo");
#endif
	
#if 0
	unsigned long total = 10;
	time_t start = time(NULL);
	for(unsigned long i = 0; i < total; i++)
		//dir->UserAttributes("cyrusdaboo");
		dir->ListUsers();
	time_t end = time(NULL);
	float diff = (end - start);
	printf("Total time: %f, average time: %f", diff, diff/total);
	delete dir;
#endif
	
#endif
	
	return 0;
}

void AuthenticateUser(CDirectoryService* dir, const char* user, const char* pswd)
{
	if (dir->AuthenticateUser(user, pswd))
		printf("Authenticated user: %s\n", user);
	else
		printf("Not Authenticated user: %s\n", user);
}

void CheckUser(CDirectoryService* dir, const char* user)
{
	if (dir->CheckUser(user))
		printf("Found user: %s\n", user);
	else
		printf("Not Found user: %s\n", user);
}

void CheckGroup(CDirectoryService* dir, const char* grp)
{
	if (dir->CheckGroup(grp))
		printf("Found user: %s\n", grp);
	else
		printf("Not Found user: %s\n", grp);
}

void CheckResource(CDirectoryService* dir, const char* rsrc)
{
	if (dir->CheckResource(rsrc))
		printf("Found user: %s\n", rsrc);
	else
		printf("Not Found user: %s\n", rsrc);
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

void UserAttributes(CDirectoryService* dir, const char* user)
{
	CFMutableDictionaryRef dict = dir->UserAttributes(user);
	if (dict != NULL)
	{
		printf("\nAttributes for %s\n", user);
		CFDictionaryApplyFunction(dict, CFDictionaryIterator, NULL);
		CFRelease(dict);
	}
}

void ListNodes ( void ) {
    bool done = false;
    long dirStatus = eDSNoErr;
    unsigned long index = 0;
    unsigned long nodeCount = 0;
    unsigned long bufferCount = 0;
    tDataBufferPtr dataBuffer = NULL;
    tDataListPtr nodeName = NULL;
    tContextData context = NULL;
	
    dirStatus = dsGetDirNodeCount( gDirRef, &nodeCount );
    printf( "Registered node count is: %lu\n", nodeCount  );
    if ( (dirStatus == eDSNoErr) && (nodeCount != 0) )
    {
        //Allocate a 32k buffer.
        dataBuffer = dsDataBufferAllocate( gDirRef, 32 * 1024 );
        if ( dataBuffer != NULL )
        {
            while ( (dirStatus == eDSNoErr) && (done ==  false) )
            {
                dirStatus = dsGetDirNodeList( gDirRef, dataBuffer,  &bufferCount,  &context );
                if ( dirStatus == eDSNoErr )
                {
                    for ( index = 1; index <= bufferCount; index++  )
                    {
                        dirStatus = dsGetDirNodeName( gDirRef, dataBuffer,  index,  &nodeName );
                        if ( dirStatus == eDSNoErr )
                        {
                            printf( "#%4ld ", index );
                            PrintNodeName( nodeName );
                            //Deallocate the data list containing  the node name.
                            dirStatus = dsDataListDeallocate( gDirRef,  nodeName );
                            free(nodeName);
                        }
                        else
                        {
                            printf("dsGetDirNodeName error  = %ld\n", dirStatus );
                        }
                    }
                }
                done = (context == NULL);
            }
            if (context != NULL)
            {
                dsReleaseContinueData( gDirRef, context );
            }
            dsDataBufferDeAllocate( gDirRef, dataBuffer );
            dataBuffer = NULL;
        }
    }
} // ListNodes

void FindNodes ( char* inNodePath ){
    bool done = false;
    long dirStatus = eDSNoErr;
    unsigned long index = 0;
    unsigned long bufferCount = 0;
    tDataBufferPtr dataBuffer = NULL;
    tDataListPtr nodeName = NULL;
    tContextData context = NULL;
    nodeName = dsBuildFromPath( gDirRef, inNodePath, "/");
    if ( nodeName != NULL )
    {
        //Allocate a 32k buffer.
        dataBuffer = dsDataBufferAllocate( gDirRef, 32 * 1024 );
        if ( dataBuffer != NULL )
        {
            while ( (dirStatus == eDSNoErr) && (done ==  false) )
            {
                dirStatus = dsFindDirNodes( gDirRef, dataBuffer,  nodeName,  eDSStartsWith, &bufferCount, &context );
                if ( dirStatus == eDSNoErr )
                {
                    for ( index = 1; index <= bufferCount; index++  )
                    {
                        dirStatus = dsGetDirNodeName( gDirRef, dataBuffer,  index,  &nodeName );
                        if ( dirStatus == eDSNoErr )
                        {
                            printf( "#%4ld ", index );
                            PrintNodeName( nodeName );
                            //Deallocate the nodes.
                            dirStatus = dsDataListDeallocate( gDirRef,  nodeName );
                            free(nodeName);
                        }
                        else
                        {
                            printf("dsGetDirNodeName error  = %ld\n", dirStatus );
                        }
                    }
                }
                done = (context == NULL);
            }
            dirStatus = dsDataBufferDeAllocate( gDirRef, dataBuffer  );
            dataBuffer = NULL;
        }
    }
} // FindNodes

void NodeInfo ( const tDirNodeReference nodeRef ){
    bool done = false;
    long dirStatus = eDSNoErr;
    unsigned long index = 0;
    unsigned long bufferCount = 0;
    tDataBufferPtr dataBuffer = NULL;
    tDataListPtr nodeName = NULL;
    tAttributeListRef attrListRef = NULL;
    unsigned long count = 0;
    tDataList attrTypes;
    tContextData context = NULL;
    {
        //Allocate a 32k buffer.
        dataBuffer = dsDataBufferAllocate( gDirRef, 32 * 1024 );
        if ( dataBuffer != NULL )
        {
            while ( (dirStatus == eDSNoErr) && (done ==  false) )
            {
				dirStatus = dsBuildListFromStringsAlloc ( gDirRef, &attrTypes,  kDSNAttrRecordType, NULL );
				dirStatus = dsGetDirNodeInfo(nodeRef, &attrTypes, dataBuffer, false, &count, &attrListRef, &context);
                if ( dirStatus == eDSNoErr )
                {
                    for ( index = 1; index <= count; index++  )
                    {
						tAttributeValueListRef 	valueRef		= 0;
						tAttributeEntryPtr 		pAttrEntry		= NULL;
						tAttributeValueEntryPtr	pValueEntry		= NULL;
						dirStatus = dsGetAttributeEntry(nodeRef, dataBuffer, attrListRef, index, &valueRef, &pAttrEntry);
						if ( dirStatus != eDSNoErr )
							break;
                    }
                }
                done = (context == NULL);
            }
			dsDataListDeallocate ( gDirRef, &attrTypes );
            dirStatus = dsDataBufferDeAllocate( gDirRef, dataBuffer  );
            dataBuffer = NULL;
        }
    }
} // FindNodes

long MyOpenDirNode ( tDirNodeReference *outNodeRef, char* inNodePath )
{
    long dirStatus = eDSNoErr;
    char nodeName[ 256 ] = "\0";
    tDataListPtr nodePath = NULL;
    strncpy( nodeName, inNodePath, 256 );
    printf( "Opening: %s.\n", nodeName );
    nodePath = dsBuildFromPath( gDirRef, nodeName, "/"  );
    if ( nodePath != NULL )
    {
        dirStatus = dsOpenDirNode( gDirRef, nodePath, outNodeRef  );
        if ( dirStatus == eDSNoErr )
        {
            printf( "Open succeeded. Node Reference = %lu\n",  (unsigned  long)outNodeRef );
        }
        else
        {
            printf( "Open node failed. Err = %ld\n", dirStatus  );
        }
    }
    dsDataListDeallocate( gDirRef, nodePath );
    free( nodePath );
    return( dirStatus );
} // MyOpenDirNode

long GetRecordList ( const tDirNodeReference nodeRef )
{
    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long k = 0;
    long dirStatus = eDSNoErr;
    unsigned long recCount = 0; // Get all records.
    tDataBufferPtr dataBuffer = NULL;
    tContextData context = NULL;
    tAttributeListRef attrListRef = NULL;
    tAttributeValueListRef valueRef = NULL;
    tRecordEntry *pRecEntry = NULL;
    tAttributeEntry *pAttrEntry = NULL;
    tAttributeValueEntry *pValueEntry = NULL;
    tDataList recNames;
    tDataList recTypes;
    tDataList attrTypes;
    dataBuffer = dsDataBufferAllocate( gDirRef, 2 * 1024 ); // allocate  a 2k buffer
    if ( dataBuffer != NULL )
    {
		CFMutableArrayRef list;
		
		list = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);
		
        // For readability, the sample code does not check dirStatus  after 
        // each call, but 
        // your code should.
        dirStatus = dsBuildListFromStringsAlloc ( gDirRef, &recNames,  kDSRecordsAll, NULL );
        dirStatus = dsBuildListFromStringsAlloc ( gDirRef, &recTypes,  kDSStdRecordTypeUsers, NULL );
        dirStatus = dsBuildListFromStringsAlloc ( gDirRef, &attrTypes,  kDSNAttrRecordName, NULL );
        do
        {
            dirStatus = dsGetRecordList( nodeRef, dataBuffer, &recNames,  eDSExact,  &recTypes, &attrTypes, false, &recCount, &context  );
            for ( i = 1; i <= recCount; i++ )
            {
				char* recname = NULL;
				CFStringRef str;
				
                dirStatus = dsGetRecordEntry( nodeRef, dataBuffer,  i, &attrListRef,  &pRecEntry );
				dirStatus = dsGetRecordNameFromEntry(pRecEntry, &recname);
				str = CFStringCreateWithCString(kCFAllocatorDefault, recname, kCFStringEncodingUTF8);
				free(recname);
				if (str != NULL)
				{
					CFArrayAppendValue(list, str);
					//CFRelease(str);
				}
                for ( j = 1; j <= pRecEntry->fRecordAttributeCount;  j++ )
                {
                    dirStatus = dsGetAttributeEntry( nodeRef, dataBuffer,  attrListRef, j, &valueRef, &pAttrEntry );
                    for ( k = 1; k <= pAttrEntry->fAttributeValueCount;  k++ )
                    {
                        dirStatus = dsGetAttributeValue( nodeRef,  dataBuffer, k,  valueRef, &pValueEntry );
                        printf( "%s\t- %lu\n", pValueEntry->fAttributeValueData.fBufferData, pValueEntry->fAttributeValueID  );
                        dirStatus = dsDeallocAttributeValueEntry(  gDirRef,  pValueEntry );
                        pValueEntry = NULL;
                        // Deallocate pAttrEntry, pValueEntry, and  pRecEntry
                        // by calling dsDeallocAttributeEntry,
                        // dsDeallocAttributeValueEntry, and
                        // dsDeallocRecordEntry, respectively.
                    }
                    dirStatus = dsCloseAttributeValueList( valueRef  );
                    valueRef = NULL;
                    dirStatus = dsDeallocAttributeEntry( gDirRef,  pAttrEntry);
                    pAttrEntry = NULL;
                }
                dirStatus = dsCloseAttributeList( attrListRef );
                attrListRef = NULL;
                dirStatus = dsDeallocRecordEntry( gDirRef, pRecEntry  );
                pRecEntry = NULL;
            }
        } while (context != NULL); // Loop until all data has been  obtained.
								   // Call dsDataListDeallocate to deallocate recNames, recTypes,  and
								   // attrTypes.
								   // Deallocate dataBuffer by calling dsDataBufferDeAllocate.
		dsDataListDeallocate ( gDirRef, &recNames );
		dsDataListDeallocate ( gDirRef, &recTypes );
		dsDataListDeallocate ( gDirRef, &attrTypes );
		dsDataBufferDeAllocate ( gDirRef, dataBuffer );
		dataBuffer = NULL;
		
    }
    return dirStatus;
} // GetRecordList

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
	CFStringUtil strvalue((CFStringRef)value);

	printf("Key: \"%s\"; Value: \"%s\"\n", strkey.temp_str(), strvalue.temp_str());
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
		printf("Index: %d\n", i);
		PrintArray(array);
		printf("\n");
	}
}

void PrintArray(CFMutableArrayRef list)
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
			printf("%d: %s\n", i, localBuffer);
		}
		else
		{
			printf("%d: %s\n", i, (const char*)bytes);
		}
	}
}

void PrintNodeName ( tDataListPtr inNode ) {
    char* pPath;
    pPath = dsGetPathFromList( gDirRef, inNode, "/" );
    printf( "%s\n", pPath );
    if ( pPath != NULL )
    {
        free( pPath );
        pPath = NULL;
    }
} // PrintNodeName
