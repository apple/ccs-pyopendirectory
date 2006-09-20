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
import sys
                  
if sys.platform in ["darwin", "macosx"]: 

    """
    On Mac OS X we build the actual Python module linking to the
    DirectoryService.framework.
    """

    try:
        from Pyrex.Distutils import build_ext
        DSModule = Extension("DirectoryServices/DirectoryService",
                             ["DirectoryServices/DirectoryService.pyx"],
                             extra_link_args = ['-framework', 
                                                'DirectoryService'])

        cmdClass = {'build_ext': build_ext}
        
    except ImportError:
        DSModule = Extension("DirectoryServices/DirectoryService",
                             extra_link_args = ['-framework', 
                                                'DirectoryService'],
                             sources = ["DirectoryServices/DirectoryService.c"])

        cmdClass = {}

    setup (
        name = 'DirectoryServices',
        version = '2.0',
        description = 'This is a high-level interface to Directory Services',
        ext_modules = [DSModule],
        packages = ["DirectoryServices"],
        cmdclass = cmdClass
    )

# else:
#     """
#     On other OS's we simply include a stub file of prototypes.
#     Eventually we should build the proper module and link
#     with appropriate local ldap etc libraries.
#     """

#     setup (
#         name = 'opendirectory',
#         version = '1.0',
#         description = 'This is a high-level interface to the Kerberos.framework',
#         package_dir={'': 'pysrc'},
#         packages=['']
#     )
