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

import opendirectory
import dsattributes
import time

try:
	ref = opendirectory.odInit("/Search")
	if ref is None:
		print "Failed odInit"
	else:
		print "OK odInit"
	
	d = opendirectory.listAllRecordsWithAttributes(ref, dsattributes.kDSStdRecordTypeUsers,
												   [dsattributes.kDS1AttrGeneratedUID, dsattributes.kDS1AttrDistinguishedName,])
	if d is None:
		print "Failed to list users"
	else:
		names = [v for v in d.iterkeys()]
		names.sort()
		for n in names:
			print "Name: %s" % n
			print "dict: %s" % str(d[n])
	
	d = opendirectory.listAllRecordsWithAttributes(ref, dsattributes.kDSStdRecordTypeGroups,
												   [dsattributes.kDS1AttrGeneratedUID, dsattributes.kDSNAttrGroupMembers,])
	if d is None:
		print "Failed to list groups"
	else:
		names = [v for v in d.iterkeys()]
		names.sort()
		for n in names:
			print "Name: %s" % n
			print "dict: %s" % str(d[n])
	
	if opendirectory.authenticateUserBasic(ref, "test", "test"):
		print "Authenticated user"
	else:
		print "Failed to authenticate user"
	
	ref = None
except opendirectory.ODError, ex:
	print ex

print "Done."
