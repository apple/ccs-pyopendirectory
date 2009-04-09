##
# Copyright (c) 2006-2009 Apple Inc. All rights reserved.
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
##

import opendirectory
import dsattributes
from dsquery import expression, match

try:
	ref = opendirectory.odInit("/Search")
	if ref is None:
		print "Failed odInit"
	else:
		print "OK odInit"

	guids = {}
	for recordType in (
		dsattributes.kDSStdRecordTypeUsers,
		dsattributes.kDSStdRecordTypeGroups,
		dsattributes.kDSStdRecordTypePlaces,
		dsattributes.kDSStdRecordTypeResources):
		
		d = opendirectory.listAllRecordsWithAttributes(ref, recordType,
													   (
													   	dsattributes.kDS1AttrGeneratedUID,
													   ))

		for name, attrs in d.iteritems():
			name = "%s/%s" % (recordType, name,)
			try:
				guid = attrs[dsattributes.kDS1AttrGeneratedUID]
				if guid in guids:
					print "Duplicate GUIDs for %s and %s: %s" % (guids[guid], name, guid,)
				else:
					guids[guid] = name
			except KeyError:
				print "No GUID for %s" % (name,)

	ref = None
except opendirectory.ODError, ex:
	print ex
except Exception, e:
	print e

print "Done."
