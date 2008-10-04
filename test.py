##
# Copyright (c) 2006-2008 Apple Inc. All rights reserved.
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

	def listUsers():
		d = opendirectory.listAllRecordsWithAttributes(ref, dsattributes.kDSStdRecordTypeUsers,
													   (
													   	dsattributes.kDS1AttrGeneratedUID,
													    dsattributes.kDS1AttrDistinguishedName,
													    ("dsAttrTypeStandard:JPEGPhoto", "base64"),
													   ))
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
		
	def listUsers_list():
		d = opendirectory.listAllRecordsWithAttributes_list(ref, dsattributes.kDSStdRecordTypeUsers,
													   [dsattributes.kDS1AttrGeneratedUID, dsattributes.kDS1AttrDistinguishedName,])
		if d is None:
			print "Failed to list users"
		else:
			d.sort(cmp=lambda x, y: x[0] < y[0])
			print "\nlistUsers_list number of results = %d" % (len(d),)
			for name, record in d:
				print "Name: %s" % name
				print "dict: %s" % str(record)
	
	def listGroups_list():
		d = opendirectory.listAllRecordsWithAttributes_list(ref, dsattributes.kDSStdRecordTypeGroups,
													   [dsattributes.kDS1AttrGeneratedUID, dsattributes.kDSNAttrGroupMembers,])
		if d is None:
			print "Failed to list groups"
		else:
			d.sort(cmp=lambda x, y: x[0] < y[0])
			print "\nlistGroups_list number of results = %d" % (len(d),)
			for name, record in d:
				print "Name: %s" % name
				print "dict: %s" % str(record)
	
	def listComputers_list():
		d = opendirectory.listAllRecordsWithAttributes_list(ref, dsattributes.kDSStdRecordTypeComputers,
													   [dsattributes.kDS1AttrGeneratedUID, dsattributes.kDS1AttrXMLPlist,])
		if d is None:
			print "Failed to list computers"
		else:
			d.sort(cmp=lambda x, y: x[0] < y[0])
			print "\nlistComputers_list number of results = %d" % (len(d),)
			for name, record in d:
				print "Name: %s" % name
				print "dict: %s" % str(record)
	
	def querySimple_list(title, attr, value, matchType, casei, recordType, attrs):
		d = opendirectory.queryRecordsWithAttribute_list(
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
			d.sort(cmp=lambda x, y: x[0] < y[0])
			print "\n%s number of results = %d" % (title, len(d),)
			for name, record in d:
				print "Name: %s" % name
				print "dict: %s" % str(record)
		
	def queryCompound_list(title, compound, casei, recordType, attrs):
		d = opendirectory.queryRecordsWithAttributes_list(
		    ref,
		    compound,
		    casei,
			recordType,
			attrs
		)
		if d is None:
			print "Failed to query users"
		else:
			d.sort(cmp=lambda x, y: x[0] < y[0])
			print "\n%s number of results = %d" % (title, len(d),)
			for name, record in d:
				print "Name: %s" % name
				print "dict: %s" % str(record)
		
	def queryUsers_list():
		querySimple_list(
			"queryUsers_list",
		    dsattributes.kDS1AttrFirstName,
		    "cyrus",
		    dsattributes.eDSExact,
		    True,
			dsattributes.kDSStdRecordTypeUsers,
			[dsattributes.kDS1AttrGeneratedUID, dsattributes.kDS1AttrDistinguishedName,]
		)
		
	def queryUsersCompoundOr_list():
		queryCompound_list(
			"queryUsersCompoundOr_list",
		    expression(expression.OR,
					   (match(dsattributes.kDS1AttrFirstName, "chris", dsattributes.eDSContains),
					    match(dsattributes.kDS1AttrLastName, "roy", dsattributes.eDSContains))).generate(),
		    False,
			dsattributes.kDSStdRecordTypeUsers,
			[dsattributes.kDS1AttrGeneratedUID, dsattributes.kDS1AttrDistinguishedName,]
		)
		
	def queryUsersCompoundOrExact_list():
		queryCompound_list(
			"queryUsersCompoundOrExact_list",
		    expression(expression.OR,
					   (match(dsattributes.kDS1AttrFirstName, "chris", dsattributes.eDSExact),
					    match(dsattributes.kDS1AttrLastName, "roy", dsattributes.eDSExact))).generate(),
		    False,
			dsattributes.kDSStdRecordTypeUsers,
			[dsattributes.kDS1AttrGeneratedUID, dsattributes.kDS1AttrDistinguishedName,]
		)
		
	def queryUsersCompoundAnd_list():
		queryCompound_list(
			"queryUsersCompoundAnd_list",
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
	
	listUsers()
	listGroups()
	listComputers()
	queryUsers()
	queryUsersCompoundOr()
	queryUsersCompoundOrExact()
	queryUsersCompoundAnd()
	listUsers_list()
	listGroups_list()
	listComputers_list()
	queryUsers_list()
	queryUsersCompoundOr_list()
	queryUsersCompoundOrExact_list()
	queryUsersCompoundAnd_list()
	#authentciateBasic()

	ref = None
except opendirectory.ODError, ex:
	print ex
except Exception, e:
	print e

print "Done."
