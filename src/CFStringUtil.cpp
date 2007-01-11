/**
 * A class that wraps CFString.
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

#include "CFStringUtil.h"

// Construct with c-string.
//
// @param cstr: c-string to create CFString from (mustr be UTF-8 encoded).
//
CFStringUtil::CFStringUtil(const char* cstr)
{
	mRef = ::CFStringCreateWithCString(kCFAllocatorDefault, cstr, kCFStringEncodingUTF8);
	mTemp = NULL;
}

// Construct with existing CFStringRef.
//
// @param ref: CFStringRef to use - this is retained.
//
CFStringUtil::CFStringUtil(CFStringRef ref)
{
	mRef = ref;
	if (mRef != NULL)
		::CFRetain(mRef);
	mTemp = NULL;
}

CFStringUtil::~CFStringUtil()
{
	if (mRef != NULL)
	{
		::CFRelease(mRef);
		mRef = NULL;
	}
	if (mTemp != NULL)
	{
		::free((void*)mTemp);
		mTemp = NULL;
	}
}

CFStringUtil& CFStringUtil::operator=(const CFStringUtil& copy)
{
	mRef = copy.get();
	if (mRef != NULL)
		::CFRetain(mRef);
	mTemp = NULL;
	
	return *this;
}


// Return a new c-string from the CFString data.
//
// @return: c-string created from CFString - this must be deallocated (free) by caller.
//
char* CFStringUtil::c_str() const
{
	const char* bytes = (mRef != NULL) ? CFStringGetCStringPtr(mRef, kCFStringEncodingUTF8) : "";
	
	if (bytes == NULL)
	{
		char localBuffer[256];
		localBuffer[0] = 0;
		Boolean success = ::CFStringGetCString(mRef, localBuffer, 256, kCFStringEncodingUTF8);
		if (!success)
			localBuffer[0] = 0;
		
		return ::strdup(localBuffer);
	}
	else
	{
		return ::strdup(bytes);
	}
}

// Return a temporary c-string from the CFString data.
//
// @return: c-string created from CFString - this must NOT be deallocated by caller as this object owns it.
//
const char* CFStringUtil::temp_str() const
{
	if (mTemp != NULL)
	{
		::free((void*)mTemp);
		mTemp = NULL;
	}
	mTemp = c_str();
	return mTemp;
}

// Reset with existing CFStringRef. Any existing CFStringRef is released before the new one is used.
//
// @param ref: CFStringRef to use - this is retained.
//
void CFStringUtil::reset(CFStringRef ref)
{
	if (mRef != NULL)
	{
		::CFRelease(mRef);
		mRef = NULL;
	}
	mRef = ref;
	if (mRef != NULL)
		::CFRetain(mRef);
}	
