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
from dsquery import expression, match

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
	
	def querySimple(title, attr, value, matchType, casei, recordType, attrs):
		d = opendirectory.queryRecordsWithAttribute(
		    ref,
		    attr,
		    value,
		    matchType,
		    casei,
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
		
	def queryCompound(title, compound, casei, recordType, attrs):
		d = opendirectory.queryRecordsWithAttributes(
		    ref,
		    compound,
		    casei,
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
		querySimple(
			"queryUsers",
		    dsattributes.kDS1AttrFirstName,
		    "cyrus",
		    dsattributes.eDSExact,
		    True,
			dsattributes.kDSStdRecordTypeUsers,
			[dsattributes.kDS1AttrGeneratedUID, dsattributes.kDS1AttrDistinguishedName,]
		)
		
	def queryUsersCompoundOr():
		queryCompound(
			"queryUsersCompoundOr",
		    expression(expression.OR,
					   (match(dsattributes.kDS1AttrFirstName, "chris", dsattributes.eDSContains),
					    match(dsattributes.kDS1AttrLastName, "roy", dsattributes.eDSContains))).generate(),
		    False,
			dsattributes.kDSStdRecordTypeUsers,
			[dsattributes.kDS1AttrGeneratedUID, dsattributes.kDS1AttrDistinguishedName,]
		)
		
	def queryUsersCompoundOrExact():
		queryCompound(
			"queryUsersCompoundOrExact",
		    expression(expression.OR,
					   (match(dsattributes.kDS1AttrFirstName, "chris", dsattributes.eDSExact),
					    match(dsattributes.kDS1AttrLastName, "roy", dsattributes.eDSExact))).generate(),
		    False,
			dsattributes.kDSStdRecordTypeUsers,
			[dsattributes.kDS1AttrGeneratedUID, dsattributes.kDS1AttrDistinguishedName,]
		)
		
	def queryUsersCompoundAnd():
		queryCompound(
			"queryUsersCompoundAnd",
		    expression(expression.AND,
					   (match(dsattributes.kDS1AttrFirstName, "chris", dsattributes.eDSContains),
					    match(dsattributes.kDS1AttrLastName, "roy", dsattributes.eDSContains))).generate(),
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
	#listComputers()
	queryUsers()
	queryUsersCompoundOr()
	queryUsersCompoundOrExact()
	queryUsersCompoundAnd()
	#authentciateBasic()

	ref = None
except opendirectory.ODError, ex:
	print ex

print "Done."
