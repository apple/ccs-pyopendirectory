/**
 * A class that wraps CFString.
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
