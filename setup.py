##
# Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
#
# This file contains Original Code and/or Modifications of Original Code
# as defined in and that are subject to the Apple Public Source License
# Version 2.0 (the 'License'). You may not use this file except in
# compliance with the License. Please obtain a copy of the License at
# http://www.opensource.apple.com/apsl/ and read it before using this
# file.
# 
# The Original Code and all software distributed under the License are
# distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
# INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
# Please see the License for the specific language governing rights and
# limitations under the License.
#
# DRI: Cyrus Daboo, cdaboo@apple.com
##

from distutils.core import setup, Extension

module1 = Extension(
    'opendirectory',
    extra_link_args = ['-framework', 'DirectoryService', "-framework", "CoreFoundation"],
    sources = ['src/PythonWrapper.cpp', 'src/CDirectoryService.cpp', 'src/CFStringUtil.cpp'],
)

setup (
    name = 'opendirectory',
    version = '1.0',
    description = 'This is a high-level interface to Open Directory for operations specific to a CalDAV server.',
    ext_modules = [module1],
    py_modules = ['dsattributes']
)
