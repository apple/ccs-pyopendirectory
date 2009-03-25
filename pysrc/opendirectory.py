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

def listNodes(obj):
    """
    List all the nodes current configured in Open Directory.

    @param obj: C{object} the object obtained from an odInit call.
    @return: C{list} containing a C{str} for each configured node.
    """
    
def getNodeAttributes(obj, nodename, attributes):
    """
    Return key attributes for the specified directory node. The attributes
    can be a C{str} for the attribute name, or a C{tuple} or C{list} where the first C{str}
    is the attribute name, and the second C{str} is an encoding type, either "str" or "base64".

    @param obj: C{object} the object obtained from an odInit call.
    @param nodename: C{str} containing the OD nodename to query.
    @param attributes: C{list} or C{tuple} containing the attributes to return for each record.
    @return: C{dict} of attributes found.
    """

def listAllRecordsWithAttributes(obj, recordType, attributes, count=0):
    """
    List records in Open Directory, and return key attributes for each one.
    The attributes can be a C{str} for the attribute name, or a C{tuple} or C{list} where the first C{str}
    is the attribute name, and the second C{str} is an encoding type, either "str" or "base64".
    
    @param obj: C{object} the object obtained from an odInit call.
    @param recordType: C{str}, C{tuple} or C{list} containing the OD record types to lookup.
    @param attributes: C{list} or C{tuple} containing the attributes to return for each record.
    @param count: C{int} maximum number of records to return (zero returns all).
    @return: C{dict} containing a C{dict} of attributes for each record found,  
        or C{None} otherwise.
    """

def queryRecordsWithAttribute(obj, attr, value, matchType, casei, recordType, attributes, count=0):
    """
    List records in Open Directory matching specified attribute/value, and return key attributes for each one.
    The attributes can be a C{str} for the attribute name, or a C{tuple} or C{list} where the first C{str}
    is the attribute name, and the second C{str} is an encoding type, either "str" or "base64".
    
    @param obj: C{object} the object obtained from an odInit call.
    @param attr: C{str} containing the attribute to search.
    @param value: C{str} containing the value to search for.
    @param matchType: C{int} DS match type to use when searching.
    @param casei: C{True} to do case-insenstive match, C{False} otherwise.
    @param recordType: C{str}, C{tuple} or C{list} containing the OD record types to lookup.
    @param attributes: C{list} or C{tuple} containing the attributes to return for each record.
    @param count: C{int} maximum number of records to return (zero returns all).
    @return: C{dict} containing a C{dict} of attributes for each record found,  
        or C{None} otherwise.
    """

def queryRecordsWithAttributes(obj, compound, casei, recordType, attributes, count=0):
    """
    List records in Open Directory matching specified criteria, and return key attributes for each one.
    The attributes can be a C{str} for the attribute name, or a C{tuple} or C{list} where the first C{str}
    is the attribute name, and the second C{str} is an encoding type, either "str" or "base64".
    
    @param obj: C{object} the object obtained from an odInit call.
    @param compound: C{str} containing the compound search query to use.
    @param casei: C{True} to do case-insenstive match, C{False} otherwise.
    @param recordType: C{str}, C{tuple} or C{list} containing the OD record types to lookup.
    @param attributes: C{list} or C{tuple} containing the attributes to return for each record.
    @param count: C{int} maximum number of records to return (zero returns all).
    @return: C{dict} containing a C{dict} of attributes for each record found,  
        or C{None} otherwise.
    """

def listAllRecordsWithAttributes_list(obj, recordType, attributes, count=0):
    """
    List records in Open Directory, and return key attributes for each one.
    The attributes can be a C{str} for the attribute name, or a C{tuple} or C{list} where the first C{str}
    is the attribute name, and the second C{str} is an encoding type, either "str" or "base64".
    
    @param obj: C{object} the object obtained from an odInit call.
    @param recordType: C{str}, C{tuple} or C{list} containing the OD record types to lookup.
    @param attributes: C{list} or C{tuple} containing the attributes to return for each record.
    @param count: C{int} maximum number of records to return (zero returns all).
    @return: C{list} containing a C{list} of C{str} (record name) and C{dict} attributes 
        for each record found, or C{None} otherwise.
    """

def queryRecordsWithAttribute_list(obj, attr, value, matchType, casei, recordType, attributes, count=0):
    """
    List records in Open Directory matching specified attribute/value, and return key attributes for each one.
    The attributes can be a C{str} for the attribute name, or a C{tuple} or C{list} where the first C{str}
    is the attribute name, and the second C{str} is an encoding type, either "str" or "base64".
    
    @param obj: C{object} the object obtained from an odInit call.
    @param attr: C{str} containing the attribute to search.
    @param value: C{str} containing the value to search for.
    @param matchType: C{int} DS match type to use when searching.
    @param casei: C{True} to do case-insenstive match, C{False} otherwise.
    @param recordType: C{str}, C{tuple} or C{list} containing the OD record types to lookup.
    @param attributes: C{list} or C{tuple} containing the attributes to return for each record.
    @param count: C{int} maximum number of records to return (zero returns all).
    @return: C{list} containing a C{list} of C{str} (record name) and C{dict} attributes 
        for each record found, or C{None} otherwise.
    """

def queryRecordsWithAttributes_list(obj, compound, casei, recordType, attributes, count=0):
    """
    List records in Open Directory matching specified criteria, and return key attributes for each one.
    The attributes can be a C{str} for the attribute name, or a C{tuple} or C{list} where the first C{str}
    is the attribute name, and the second C{str} is an encoding type, either "str" or "base64".
    
    @param obj: C{object} the object obtained from an odInit call.
    @param compound: C{str} containing the compound search query to use.
    @param casei: C{True} to do case-insenstive match, C{False} otherwise.
    @param recordType: C{str}, C{tuple} or C{list} containing the OD record types to lookup.
    @param attributes: C{list} or C{tuple} containing the attributes to return for each record.
    @param count: C{int} maximum number of records to return (zero returns all).
    @return: C{list} containing a C{list} of C{str} (record name) and C{dict} attributes 
        for each record found, or C{None} otherwise.
    """

def authenticateUserBasic(obj, nodename, user, pswd):
    """
    Authenticate a user with a password to Open Directory.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param nodename: C{str} the directory nodename for the record to check.
    @param user: C{str} the user identifier/directory record name to check.
    @param pswd: C{str} containing the password to check.
    @return: C{True} if the user was found, C{False} otherwise.
    """

def authenticateUserDigest(obj, nodename, user, challenge, response, method):
    """
    Authenticate using HTTP Digest credentials to Open Directory.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param nodename: C{str} the directory nodename for the record to check.
    @param user: C{str} the user identifier/directory record name to check.
    @param challenge: C{str} the HTTP challenge sent to the client.
    @param response: C{str} the HTTP response sent from the client.
    @param method: C{str} the HTTP method being used.
    @return: C{True} if the user was found, C{False} otherwise.
    """

class ODError(Exception):
    """
    Exceptions from DirectoryServices errors.
    """
