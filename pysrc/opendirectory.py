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

"""
PyOpenDirectory Function Description.
"""
 
def odInit(nodename):
    """
    Create an Open Directory object to operate on the specified directory service node name.
 
    @param nodename: C{str} containing the node name.
    @return: C{object} an object to be passed to all subsequent functions on success,
        C{None} on failure.
    """

def listUsers(obj):
    """
    List users in Open Directory, and return key attributes for each user.
    The attributes in the tuple are (uid, guid, last-modified, calendar-principal-uri).
    
    @param obj: C{object} the object obtained from an odInit call.
    @return: C{list} containing a C{tuple} of C{str} for each user found,
        or C{None} on failure.
    """

def listGroups(obj):
    """
    List groups in Open Directory, and return key attributes for each group.
    The attributes in the tuple are (uid, guid, last-modified, calendar-principal-uri).
    
    @param obj: C{object} the object obtained from an odInit call.
    @return: C{list} containg a C{tuple} of C{str} for each group found,
        or C{None} on failure.
    """

def listResources(obj):
    """
    List resources in Open Directory, and return key attributes for each resource.
    The attributes in the tuple are (uid, guid, last-modified, calendar-principal-uri).
    
    @param obj: C{object} the object obtained from an odInit call.
    @return: C{list} containg a C{tuple} of C{str} for each resource found,
        or C{None} on failure.
    """

def checkUser(obj, user):
    """
    Check that a user exists in Open Directory.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param user: C{str} containing the user to check.
    @return: C{True} if the user was found, C{False} otherwise.
    """

def checkGroup(obj, group):
    """
    Check that a group exists in Open Directory.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param group: C{str} containing the group to check.
    @return: C{True} if the group was found, C{False} otherwise.
    """

def checkResource(obj, resource):
    """
    Check that a resource exists in Open Directory.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param resource: C{str} containing the resource to check.
    @return: C{True} if the resource was found, C{False} otherwise.
    """

def listUsersWithAttributes(obj, user):
    """
    Get user attributes relevant to CalDAV from Open Directory.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param users: C{list} containing C{str}'s for each user to get attributes for.
    @return: C{dict} containing a C{dict} of attributes for each requested user, 
        or C{None} otherwise.
    """

def listGroupsWithAttributes(obj, grp):
    """
    Get group attributes relevant to CalDAV from Open Directory.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param grp: C{str} containing the group to get attributes for.
    @return: C{dict} containing a C{dict} of attributes for each requested group, 
        or C{None} otherwise.
    """

def listResourcesWithAttributes(obj, rsrc):
    """
    Get resource attributes relevant to CalDAV from Open Directory.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param rsrc: C{str} containing the resource to get attributes for.
    @return: C{dict} containing a C{dict} of attributes for each requested resource, 
        or C{None} otherwise.
    """

def authenticateUser(obj, user, pswd):
    """
    Authenticate a user with a password to Open Directory.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param user: C{str} container the user to check.
    @param pswd: C{str} containing the password to check.
    @return: C{True} if the user was found, C{False} otherwise.
    """
