##
# Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
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
