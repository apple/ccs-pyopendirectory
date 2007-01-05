/**
 * A class that wraps CFString.
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

class CFStringUtil
{
public:
	CFStringUtil(const char* cstr);
	CFStringUtil(CFStringRef ref);
	~CFStringUtil();
	
	CFStringUtil& operator=(const CFStringUtil& copy);

	CFStringRef get() const
	{
		return mRef;
	}

	char* c_str() const;
	const char* temp_str() const;

	void reset(CFStringRef ref);

private:
	CFStringRef mRef;
	mutable const char* mTemp;
};
