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

import opendirectory
import dsattributes
import time

ref = opendirectory.odInit("/LDAPv3/webboserver.apple.com")
if ref is None:
	print "Failed odInit"
else:
	print "OK odInit"

list = opendirectory.listUsers(ref)
if list is None:
	print "Failed listUsers"
else:
	for i in list:
		print i

def CheckUser(user):
	if opendirectory.checkUser(ref, user):
		print "Found User: %s" % (user,)
	else:
		print "Not Found User: %s" % (user,)

def CheckGroup(grp):
	if opendirectory.checkGroup(ref, grp):
		print "Found Group: %s" % (grp,)
	else:
		print "Not Found Group: %s" % (grp,)

def CheckResource(rsrc):
	if opendirectory.checkResource(ref, rsrc):
		print "Found Resource: %s" % (rsrc,)
	else:
		print "Not Found Resource: %s" % (rsrc,)

#CheckUser("cyrusdaboo");
#CheckUser("chris");
#CheckGroup("sangriafest");
#CheckGroup("cyrusdaboo");
#CheckResource("Attitude Adjuster");
#CheckResource("cyrusdaboo");
#
#CheckUser("steevef\xc3\xbchr")
#CheckUser("steevefu\xcc\x88hr")
#

dict = opendirectory.listGroupsWithAttributes(ref, [i[0] for i in list])
names = [v for v in dict.iterkeys()]
names.sort()
for n in names:
	print "Name: %s" % n
	print "dict: %s" % str(dict[n])

#
#dict = opendirectory.groupAttributes(ref, "admin")
#print dict
#
#print dsattributes.attrRealName

#if False:
#	t = time.time()
#	total = 10
#	for i in range(total):
#		opendirectory.userAttributes(ref, "cyrusdaboo")
#	t = time.time() - t
#	print "Total time: %f, average time: %f" % (t, t/total)

ref = None

print "Done."
