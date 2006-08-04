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
