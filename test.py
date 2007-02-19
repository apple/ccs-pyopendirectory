##
# Copyright (c) 2006-2007 Apple Inc. All rights reserved.
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

	def listUsers():
		d = opendirectory.listAllRecordsWithAttributes(ref, dsattributes.kDSStdRecordTypeUsers,
													   [dsattributes.kDS1AttrGeneratedUID, dsattributes.kDS1AttrDistinguishedName,])
		if d is None:
			print "Failed to list users"
		else:
			names = [v for v in d.iterkeys()]
			names.sort()
			print "\nlistUsers number of results = %d" % (len(names),)
			for n in names:
				print "Name: %s" % n
				print "dict: %s" % str(d[n])
	
	def listGroups():
		d = opendirectory.listAllRecordsWithAttributes(ref, dsattributes.kDSStdRecordTypeGroups,
													   [dsattributes.kDS1AttrGeneratedUID, dsattributes.kDSNAttrGroupMembers,])
		if d is None:
			print "Failed to list groups"
		else:
			names = [v for v in d.iterkeys()]
			names.sort()
			print "\nlistGroups number of results = %d" % (len(names),)
			for n in names:
				print "Name: %s" % n
				print "dict: %s" % str(d[n])
	
	def listComputers():
		d = opendirectory.listAllRecordsWithAttributes(ref, dsattributes.kDSStdRecordTypeComputers,
													   [dsattributes.kDS1AttrGeneratedUID, dsattributes.kDS1AttrXMLPlist,])
		if d is None:
			print "Failed to list computers"
		else:
			names = [v for v in d.iterkeys()]
			names.sort()
			print "\nlistComputers number of results = %d" % (len(names),)
			for n in names:
				print "Name: %s" % n
				print "dict: %s" % str(d[n])
	
	def query(title, dict, matchType, casei, allmatch, recordType, attrs):
		d = opendirectory.queryRecordsWithAttributes(
		    ref,
		    dict,
		    matchType,
		    casei,
		    allmatch,
			recordType,
			attrs
		)
		if d is None:
			print "Failed to query users"
		else:
			names = [v for v in d.iterkeys()]
			names.sort()
			print "\n%s number of results = %d" % (title, len(names),)
			for n in names:
				print "Name: %s" % n
				print "dict: %s" % str(d[n])
		
	def queryUsers():
		query(
			"queryUsers",
		    {dsattributes.kDS1AttrFirstName: "cyrus",},
		    dsattributes.eDSExact,
		    True,
		    True,
			dsattributes.kDSStdRecordTypeUsers,
			[dsattributes.kDS1AttrGeneratedUID, dsattributes.kDS1AttrDistinguishedName,]
		)
		
	def queryUsersCompoundOr():
		query(
			"queryUsersCompoundOr",
		    {dsattributes.kDS1AttrFirstName: "chris", dsattributes.kDS1AttrLastName: "roy",},
		    dsattributes.eDSContains,
		    True,
		    False,
			dsattributes.kDSStdRecordTypeUsers,
			[dsattributes.kDS1AttrGeneratedUID, dsattributes.kDS1AttrDistinguishedName,]
		)
		
	def queryUsersCompoundOrExact():
		query(
			"queryUsersCompoundOrExact",
		    {dsattributes.kDS1AttrFirstName: "chris", dsattributes.kDS1AttrLastName: "roy",},
		    dsattributes.eDSExact,
		    True,
		    False,
			dsattributes.kDSStdRecordTypeUsers,
			[dsattributes.kDS1AttrGeneratedUID, dsattributes.kDS1AttrDistinguishedName,]
		)
		
	def queryUsersCompoundAnd():
		query(
			"queryUsersCompoundAnd",
		    {dsattributes.kDS1AttrFirstName: "chris", dsattributes.kDS1AttrLastName: "roy",},
		    dsattributes.eDSContains,
		    True,
		    True,
			dsattributes.kDSStdRecordTypeUsers,
			[dsattributes.kDS1AttrGeneratedUID, dsattributes.kDS1AttrDistinguishedName,]
		)
		
	def authentciateBasic():
		if opendirectory.authenticateUserBasic(ref, "gooeyed", "test", "test"):
			print "Authenticated user"
		else:
			print "Failed to authenticate user"
	
	#listUsers()
	#listGroups()
	listComputers()
	#queryUsers()
	#queryUsersCompoundOr()
	#queryUsersCompoundOrExact()
	#queryUsersCompoundAnd()
	authentciateBasic()

	ref = None
except opendirectory.ODError, ex:
	print ex

print "Done."
