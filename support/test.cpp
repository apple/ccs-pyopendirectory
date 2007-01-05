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

char* CStringFromCFString(CFStringRef str);
void PrintDictionaryDictionary(const void* key, const void* value, void* ref);
void PrintDictionary(const void* key, const void* value, void* ref);
void PrintArrayArray(CFMutableArrayRef list);
void PrintArray(CFArrayRef list);
void AuthenticateUser(CDirectoryService* dir, const char* user, const char* pswd);

int main (int argc, const char * argv[]) {
    
	CDirectoryService* dir = new CDirectoryService("/Search");

	CFStringRef strings[2];
	strings[0] = CFSTR(kDS1AttrDistinguishedName);
	strings[1] = CFSTR(kDS1AttrGeneratedUID);
	CFArrayRef array = CFArrayCreate(kCFAllocatorDefault, (const void **)strings, 2, &kCFTypeArrayCallBacks);
                        
	CFMutableDictionaryRef dict = dir->ListAllRecordsWithAttributes(kDSStdRecordTypeUsers, array);
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

	strings[0] = CFSTR(kDSNAttrGroupMembers);
	strings[1] = CFSTR(kDS1AttrGeneratedUID);
	array = CFArrayCreate(kCFAllocatorDefault, (const void **)strings, 2, &kCFTypeArrayCallBacks);
                        
	dict = dir->ListAllRecordsWithAttributes(kDSStdRecordTypeGroups, array);
	if (dict != NULL)
	{
		printf("\n*** Groups: %d ***\n", CFDictionaryGetCount(dict));
		CFDictionaryApplyFunction(dict, PrintDictionaryDictionary, NULL);
		CFRelease(dict);
	}
	else
	{
		printf("\nNo Groups returned\n");
	}
	CFRelease(array);

	AuthenticateUser(dir, "cdaboo", "appledav1234");
	AuthenticateUser(dir, "cdaboo", "appledav6585");
	
	return 0;
}

void AuthenticateUser(CDirectoryService* dir, const char* user, const char* pswd)
{
	if (dir->AuthenticateUserBasic(user, pswd))
		printf("Authenticated user: %s\n", user);
	else
		printf("Not Authenticated user: %s\n", user);
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
		printf("Index: %d\n", i);
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
			printf("%d: %s\n", i, localBuffer);
		}
		else
		{
			printf("%d: %s\n", i, (const char*)bytes);
		}
	}
}
