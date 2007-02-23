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

def listAllRecordsWithAttributes(obj, recordType, attributes):
    """
    List records in Open Directory, and return key attributes for each one.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param recordType: C{str} containing the OD record type to lookup.
    @param attributes: C{list} containing the attributes to return for each record.
    @return: C{dict} containing a C{dict} of attributes for each record found, 
        or C{None} otherwise.
    """

def queryRecordsWithAttribute(obj, attr, value, matchType, casei, recordType, attributes):
    """
    List records in Open Directory matching specified attribute/value, and return key attributes for each one.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param attr: C{str} containing the attribute to search.
    @param value: C{str} containing the value to search for.
    @param matchType: C{int} DS match type to use when searching.
    @param casei: C{True} to do case-insenstive match, C{False} otherwise.
    @param recordType: C{str} containing the OD record type to lookup.
    @param attributes: C{list} containing the attributes to return for each record.
    @return: C{dict} containing a C{dict} of attributes for each record found, 
        or C{None} otherwise.
    """

def queryRecordsWithAttributes(obj, compound, casei, recordType, attributes):
    """
    List records in Open Directory matching specified criteria, and return key attributes for each one.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param compound: C{str} containing the compound search query to use.
    @param casei: C{True} to do case-insenstive match, C{False} otherwise.
    @param recordType: C{str} containing the OD record type to lookup.
    @param attributes: C{list} containing the attributes to return for each record.
    @return: C{dict} containing a C{dict} of attributes for each record found, 
        or C{None} otherwise.
    """

def authenticateUserBasic(obj, guid, user, pswd):
    """
    Authenticate a user with a password to Open Directory.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param guid: C{str} the GUID for the record to check.
    @param user: C{str} the user identifier/directory record name to check.
    @param pswd: C{str} containing the password to check.
    @return: C{True} if the user was found, C{False} otherwise.
    """

def authenticateUserDigest(obj, guid, user, challenge, response, method):
    """
    Authenticate using HTTP Digest credentials to Open Directory.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param guid: C{str} the GUID for the record to check.
    @param user: C{str} the user identifier/directory record name to check.
    @param challenge: C{str} the HTTP challenge sent to the client.
    @param response: C{str} the HTTP response sent from the client.
    @param method: C{str} the HTTP method being used.
    @return: C{True} if the user was found, C{False} otherwise.
    """

class ODError(exception):
    """
    Exceptions from DirectoryServices errors.
    """
