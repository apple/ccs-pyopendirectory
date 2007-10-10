/**
 * Copyright (c) 2006-2007 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * DRI: Cyrus Daboo, cdaboo@apple.com
 **/

#include <CoreFoundation/CoreFoundation.h>
#include <Python.h>

#include "CDirectoryService.h"
#include "CFStringUtil.h"

#ifndef Py_RETURN_TRUE
#define Py_RETURN_TRUE return Py_INCREF(Py_True), Py_True
#endif
#ifndef Py_RETURN_FALSE
#define Py_RETURN_FALSE return Py_INCREF(Py_False), Py_False
#endif
#ifndef Py_RETURN_NONE
#define Py_RETURN_NONE return Py_INCREF(Py_None), Py_None
#endif

// Utility function - not exposed to Python
static PyObject* CFStringToPyStr(CFStringRef str)
{
	CFStringUtil s(str);
	return PyString_FromString(s.temp_str());
}

// Utility function - not exposed to Python
static PyObject* CFArrayToPyList(CFArrayRef list, bool sorted = false)
{
	CFIndex lsize = (list != NULL) ? CFArrayGetCount(list) : 0;
	if (sorted and (list != NULL))
		CFArraySortValues((CFMutableArrayRef)list, CFRangeMake(0, lsize), (CFComparatorFunction)CFStringCompare, NULL);
	
	PyObject* result = PyList_New(lsize);
	for(CFIndex i = 0; i < lsize; i++)
	{
		CFStringRef str = (CFStringRef)CFArrayGetValueAtIndex(list, i);
		PyObject* pystr = CFStringToPyStr(str);
		
		PyList_SetItem(result, i, pystr);
	}
	
	return result;
}

// Utility function - not exposed to Python
static CFArrayRef PyListToCFArray(PyObject* list)
{
	CFMutableArrayRef result = CFArrayCreateMutable(kCFAllocatorDefault, PyList_Size(list), &kCFTypeArrayCallBacks);
	for(int i = 0; i < PyList_Size(list); i++)
	{
		PyObject* str = PyList_GetItem(list, i);
		if ((str == NULL) || !PyString_Check(str))
		{
			CFRelease(result);
			return NULL;
		}
		const char* cstr = PyString_AsString(str);
		if (cstr == NULL)
		{
			CFRelease(result);
			return NULL;
		}
		CFStringUtil cfstr(cstr);
		CFArrayAppendValue(result, cfstr.get());
	}
	
	return result;
}

// Utility function - not exposed to Python
static void CFDictionaryIterator(const void* key, const void* value, void* ref)
{
	CFStringRef strkey = (CFStringRef)key;
	PyObject* dict = (PyObject*)ref;
	
	PyObject* pystrkey = CFStringToPyStr(strkey);

	// The dictionary value may be a string or a list
	if (CFGetTypeID((CFTypeRef)value) == CFStringGetTypeID())
	{
		CFStringRef strvalue = (CFStringRef)value;
		PyObject* pystrvalue = CFStringToPyStr(strvalue);
		PyDict_SetItem(dict, pystrkey, pystrvalue);
		Py_DECREF(pystrvalue);
	}
	else if(CFGetTypeID((CFTypeRef)value) == CFArrayGetTypeID())
	{
		CFArrayRef arrayvalue = (CFArrayRef)value;
		PyObject* pylistvalue = CFArrayToPyList(arrayvalue);
		PyDict_SetItem(dict, pystrkey, pylistvalue);
		Py_DECREF(pylistvalue);
	}
	Py_DECREF(pystrkey);
}

// Utility function - not exposed to Python
static PyObject* CFDictionaryToPyDict(CFDictionaryRef dict)
{
	PyObject* result = PyDict_New();
	if (dict != NULL)
		CFDictionaryApplyFunction(dict, CFDictionaryIterator, result);
	
	return result;
}

// Utility function - not exposed to Python
static void CFDictionaryDictionaryIterator(const void* key, const void* value, void* ref)
{
	CFStringRef strkey = (CFStringRef)key;
	CFDictionaryRef dictvalue = (CFDictionaryRef)value;
	PyObject* dict = (PyObject*)ref;
	
	PyObject* pystrkey = CFStringToPyStr(strkey);
	PyObject* pydictvalue = CFDictionaryToPyDict(dictvalue);
	
	PyDict_SetItem(dict, pystrkey, pydictvalue);
	Py_DECREF(pystrkey);
	Py_DECREF(pydictvalue);
}

// Utility function - not exposed to Python
static PyObject* CFDictionaryDictionaryToPyDict(CFDictionaryRef dict)
{
	PyObject* result = PyDict_New();
	if (dict != NULL)
		CFDictionaryApplyFunction(dict, CFDictionaryDictionaryIterator, result);
	
	return result;
}

// Utility function - not exposed to Python
static PyObject* CFArrayStringDictionaryToPyList(CFArrayRef list)
{
	CFIndex lsize = (list != NULL) ? CFArrayGetCount(list) : 0;
	if (lsize != 2)
		return NULL;
	
	PyObject* result = PyList_New(lsize);

	CFStringRef str = (CFStringRef)CFArrayGetValueAtIndex(list, 0);
	PyObject* pystr = CFStringToPyStr(str);
	PyList_SetItem(result, 0, pystr);
	
	CFDictionaryRef dict = (CFDictionaryRef)CFArrayGetValueAtIndex(list, 1);
	PyObject* pydict = CFDictionaryToPyDict(dict);
	PyList_SetItem(result, 1, pydict);
	
	return result;
}

// Utility function - not exposed to Python
static PyObject* CFArrayArrayDictionaryToPyList(CFArrayRef list)
{
	CFIndex lsize = (list != NULL) ? CFArrayGetCount(list) : 0;
	
	PyObject* result = PyList_New(lsize);
	for(CFIndex i = 0, j = 0; i < lsize; i++)
	{
		CFArrayRef nested = (CFArrayRef)CFArrayGetValueAtIndex(list, i);
		PyObject* pylist = CFArrayStringDictionaryToPyList(nested);
		if (pylist != NULL)
		{
			PyList_SetItem(result, j++, pylist);
		}
	}
	
	return result;
}

// Utility function - not exposed to Python
static void CFArrayStringDictionaryToPyDict(CFArrayRef list, PyObject* result)
{
	CFIndex lsize = (list != NULL) ? CFArrayGetCount(list) : 0;
	if (lsize != 2)
		return;
	
	CFStringRef str = (CFStringRef)CFArrayGetValueAtIndex(list, 0);
	PyObject* pystrkey = CFStringToPyStr(str);
	
	CFDictionaryRef dict = (CFDictionaryRef)CFArrayGetValueAtIndex(list, 1);
	PyObject* pydictvalue = CFDictionaryToPyDict(dict);
	
	PyDict_SetItem(result, pystrkey, pydictvalue);
	Py_DECREF(pystrkey);
	Py_DECREF(pydictvalue);
}

// Utility function - not exposed to Python
static PyObject* CFArrayArrayDictionaryToPyDict(CFArrayRef list)
{
	CFIndex lsize = (list != NULL) ? CFArrayGetCount(list) : 0;
	
	PyObject* result = PyDict_New();
	for(CFIndex i = 0; i < lsize; i++)
	{
		CFArrayRef nested = (CFArrayRef)CFArrayGetValueAtIndex(list, i);
		CFArrayStringDictionaryToPyDict(nested, result);
	}
	
	return result;
}

PyObject* ODException_class = NULL;

/*
 This is an automatic destructor for the object obtained by odInit. It is not directly
 exposed to Python, instead Python calls it automatically when reclaiming the object.
 */
extern "C" void odDestroy(void* obj)
{
	CDirectoryService* ds = static_cast<CDirectoryService*>(obj);
	delete ds;
}

/*
 def odInit(nodename):
	"""
	Create an Open Directory object to operate on the specified directory service node name.
 
	@param nodename: C{str} containing the node name.
	@return: C{object} an object to be passed to all subsequent functions on success,
		C{None} on failure.
	"""
 */
extern "C" PyObject* odInit(PyObject* self, PyObject* args)
{
    const char* nodename;
    if (!PyArg_ParseTuple(args, "s", &nodename))
    {
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices odInit: could not parse arguments", 0));		
        return NULL;
    }

	CDirectoryService* ds = new CDirectoryService(nodename);
	if (ds != NULL)
	{
		return PyCObject_FromVoidPtr(ds, odDestroy);
	}
	
	PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices odInit: could not initialize directory service", 0));		
	return NULL;
}

/*
def listAllRecordsWithAttributes(obj, recordType, attributes):
    """
    List records in Open Directory, and return key attributes for each one.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param recordType: C{str} containing the OD record type to lookup.
    @param attributes: C{list} containing the attributes to return for each record.
	@return: C{dict} containing a C{dict} of attributes for each record found,  
        or C{None} otherwise.
 */
extern "C" PyObject *listAllRecordsWithAttributes(PyObject *self, PyObject *args)
{
	PyObject* pyds;
	const char* recordType;
	PyObject* attributes;
    if (!PyArg_ParseTuple(args, "OsO", &pyds, &recordType, &attributes) || !PyCObject_Check(pyds) || !PyList_Check(attributes))
    {
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices listAllRecordsWithAttributes: could not parse arguments", 0));		
        return NULL;
    }
	
	// Convert list to CFArray of CFString
	CFArrayRef cfattributes = PyListToCFArray(attributes);
	if (cfattributes == NULL)
    {
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices listAllRecordsWithAttributes: could not parse attributes list", 0));		
        return NULL;
    }

	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		CFMutableArrayRef list = ds->ListAllRecordsWithAttributes(recordType, cfattributes);
		if (list != NULL)
		{
			PyObject* result = CFArrayArrayDictionaryToPyDict(list);
			CFRelease(list);
			CFRelease(cfattributes);
			
			return result;
		}
	}
	else
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices listAllRecordsWithAttributes: invalid directory service argument", 0));		
	
	CFRelease(cfattributes);
	return NULL;
}

/*
def queryRecordsWithAttribute(obj, attr, value, matchType, casei, recordType, attributes):
    """
    List records in Open Directory matching specified attribute and value, and return key attributes for each one.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param attr: C{str} for the attribute to query.
    @param value: C{str} for the attribute value to query.
    @param matchType: C{int} DS match type to use when searching.
    @param casei: C{True} to do case-insenstive match, C{False} otherwise.
    @param recordType: C{str} containing the OD record type to lookup.
    @param attributes: C{list} containing the attributes to return for each record.
	@return: C{dict} containing a C{dict} of attributes for each record found,  
        or C{None} otherwise.
    """
 */
extern "C" PyObject *queryRecordsWithAttribute(PyObject *self, PyObject *args)
{
	PyObject* pyds;
	const char* attr;
	const char* value;
	int matchType;
	PyObject* caseio;
	bool casei;
	const char* recordType;
	PyObject* attributes;
    if (!PyArg_ParseTuple(args, "OssiOsO", &pyds, &attr, &value, &matchType, &caseio, &recordType, &attributes) ||
    	!PyCObject_Check(pyds) || !PyBool_Check(caseio) || !PyList_Check(attributes))
    {
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices queryRecordsWithAttributes: could not parse arguments", 0));		
        return NULL;
    }

	casei = (caseio == Py_True);

	// Convert list to CFArray of CFString
	CFArrayRef cfattributes = PyListToCFArray(attributes);
	if (cfattributes == NULL)
    {
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices queryRecordsWithAttributes: could not parse attributes list", 0));		
        return NULL;
    }

	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		CFMutableArrayRef list = ds->QueryRecordsWithAttribute(attr, value, matchType, casei, recordType, cfattributes);
		if (list != NULL)
		{
			PyObject* result = CFArrayArrayDictionaryToPyDict(list);
			CFRelease(list);
			CFRelease(cfattributes);
			
			return result;
		}
	}
	else
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices queryRecordsWithAttributes: invalid directory service argument", 0));		
	
	CFRelease(cfattributes);
	return NULL;
}

/*
def queryRecordsWithAttributes(obj, query, casei, recordType, attributes):
    """
    List records in Open Directory matching specified compound query, and return key attributes for each one.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param query: C{str} the compound query string.
    @param casei: C{True} to do case-insenstive match, C{False} otherwise.
    @param recordType: C{str} containing the OD record type to lookup.
    @param attributes: C{list} containing the attributes to return for each record.
	@return: C{dict} containing a C{dict} of attributes for each record found,  
        or C{None} otherwise.
    """
 */
extern "C" PyObject *queryRecordsWithAttributes(PyObject *self, PyObject *args)
{
	PyObject* pyds;
	const char* query;
	PyObject* caseio;
	bool casei;
	const char* recordType;
	PyObject* attributes;
    if (!PyArg_ParseTuple(args, "OsOsO", &pyds, &query, &caseio, &recordType, &attributes) ||
    	!PyCObject_Check(pyds) || !PyBool_Check(caseio) || !PyList_Check(attributes))
    {
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices queryRecordsWithAttributes: could not parse arguments", 0));		
        return NULL;
    }

	casei = (caseio == Py_True);

	// Convert list to CFArray of CFString
	CFArrayRef cfattributes = PyListToCFArray(attributes);
	if (cfattributes == NULL)
    {
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices queryRecordsWithAttributes: could not parse attributes list", 0));		
        return NULL;
    }

	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		CFMutableArrayRef list = ds->QueryRecordsWithAttributes(query, casei, recordType, cfattributes);
		if (list != NULL)
		{
			PyObject* result = CFArrayArrayDictionaryToPyDict(list);
			CFRelease(list);
			CFRelease(cfattributes);
			
			return result;
		}
	}
	else
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices queryRecordsWithAttributes: invalid directory service argument", 0));		
	
	CFRelease(cfattributes);
	return NULL;
}

/*
def listAllRecordsWithAttributes_list(obj, recordType, attributes):
    """
    List records in Open Directory, and return key attributes for each one.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param recordType: C{str} containing the OD record type to lookup.
    @param attributes: C{list} containing the attributes to return for each record.
    @return: C{list} containing a C{list} of C{str} (record name) and C{dict} attributes 
         for each record found, or C{None} otherwise.
    """
 */
extern "C" PyObject *listAllRecordsWithAttributes_list(PyObject *self, PyObject *args)
{
	PyObject* pyds;
	const char* recordType;
	PyObject* attributes;
    if (!PyArg_ParseTuple(args, "OsO", &pyds, &recordType, &attributes) || !PyCObject_Check(pyds) || !PyList_Check(attributes))
    {
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices listAllRecordsWithAttributes: could not parse arguments", 0));		
        return NULL;
    }
	
	// Convert list to CFArray of CFString
	CFArrayRef cfattributes = PyListToCFArray(attributes);
	if (cfattributes == NULL)
    {
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices listAllRecordsWithAttributes: could not parse attributes list", 0));		
        return NULL;
    }

	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		CFMutableArrayRef list = ds->ListAllRecordsWithAttributes(recordType, cfattributes);
		if (list != NULL)
		{
			PyObject* result = CFArrayArrayDictionaryToPyList(list);
			CFRelease(list);
			CFRelease(cfattributes);
			
			return result;
		}
	}
	else
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices listAllRecordsWithAttributes: invalid directory service argument", 0));		
	
	CFRelease(cfattributes);
	return NULL;
}

/*
def queryRecordsWithAttribute_list(obj, attr, value, matchType, casei, recordType, attributes):
    """
    List records in Open Directory matching specified attribute and value, and return key attributes for each one.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param attr: C{str} for the attribute to query.
    @param value: C{str} for the attribute value to query.
    @param matchType: C{int} DS match type to use when searching.
    @param casei: C{True} to do case-insenstive match, C{False} otherwise.
    @param recordType: C{str} containing the OD record type to lookup.
    @param attributes: C{list} containing the attributes to return for each record.
    @return: C{list} containing a C{list} of C{str} (record name) and C{dict} attributes 
         for each record found, or C{None} otherwise.
    """
 */
extern "C" PyObject *queryRecordsWithAttribute_list(PyObject *self, PyObject *args)
{
	PyObject* pyds;
	const char* attr;
	const char* value;
	int matchType;
	PyObject* caseio;
	bool casei;
	const char* recordType;
	PyObject* attributes;
    if (!PyArg_ParseTuple(args, "OssiOsO", &pyds, &attr, &value, &matchType, &caseio, &recordType, &attributes) ||
    	!PyCObject_Check(pyds) || !PyBool_Check(caseio) || !PyList_Check(attributes))
    {
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices queryRecordsWithAttributes: could not parse arguments", 0));		
        return NULL;
    }

	casei = (caseio == Py_True);

	// Convert list to CFArray of CFString
	CFArrayRef cfattributes = PyListToCFArray(attributes);
	if (cfattributes == NULL)
    {
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices queryRecordsWithAttributes: could not parse attributes list", 0));		
        return NULL;
    }

	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		CFMutableArrayRef list = ds->QueryRecordsWithAttribute(attr, value, matchType, casei, recordType, cfattributes);
		if (list != NULL)
		{
			PyObject* result = CFArrayArrayDictionaryToPyList(list);
			CFRelease(list);
			CFRelease(cfattributes);
			
			return result;
		}
	}
	else
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices queryRecordsWithAttributes: invalid directory service argument", 0));		
	
	CFRelease(cfattributes);
	return NULL;
}

/*
def queryRecordsWithAttributes_list(obj, query, casei, recordType, attributes):
    """
    List records in Open Directory matching specified compound query, and return key attributes for each one.
    
    @param obj: C{object} the object obtained from an odInit call.
    @param query: C{str} the compound query string.
    @param casei: C{True} to do case-insenstive match, C{False} otherwise.
    @param recordType: C{str} containing the OD record type to lookup.
    @param attributes: C{list} containing the attributes to return for each record.
    @return: C{list} containing a C{list} of C{str} (record name) and C{dict} attributes 
         for each record found, or C{None} otherwise.
    """
 */
extern "C" PyObject *queryRecordsWithAttributes_list(PyObject *self, PyObject *args)
{
	PyObject* pyds;
	const char* query;
	PyObject* caseio;
	bool casei;
	const char* recordType;
	PyObject* attributes;
    if (!PyArg_ParseTuple(args, "OsOsO", &pyds, &query, &caseio, &recordType, &attributes) ||
    	!PyCObject_Check(pyds) || !PyBool_Check(caseio) || !PyList_Check(attributes))
    {
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices queryRecordsWithAttributes: could not parse arguments", 0));		
        return NULL;
    }

	casei = (caseio == Py_True);

	// Convert list to CFArray of CFString
	CFArrayRef cfattributes = PyListToCFArray(attributes);
	if (cfattributes == NULL)
    {
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices queryRecordsWithAttributes: could not parse attributes list", 0));		
        return NULL;
    }

	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		CFMutableArrayRef list = ds->QueryRecordsWithAttributes(query, casei, recordType, cfattributes);
		if (list != NULL)
		{
			PyObject* result = CFArrayArrayDictionaryToPyList(list);
			CFRelease(list);
			CFRelease(cfattributes);
			
			return result;
		}
	}
	else
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices queryRecordsWithAttributes: invalid directory service argument", 0));		
	
	CFRelease(cfattributes);
	return NULL;
}

/*
def authenticateUserBasic(obj, nodename, user, pswd):
	"""
	Authenticate a user with a password to Open Directory.
	
	@param obj: C{object} the object obtained from an odInit call.
    @param nodename: C{str} the directory nodename for the record to check.
    @param user: C{str} the user identifier/directory record name to check.
	@param pswd: C{str} containing the password to check.
	@return: C{True} if the user was found, C{False} otherwise.
	"""
 */
extern "C" PyObject *authenticateUserBasic(PyObject *self, PyObject *args)
{
	PyObject* pyds;
	const char* nodename;
	const char* user;
	const char* pswd;
    if (!PyArg_ParseTuple(args, "Osss", &pyds, &nodename, &user, &pswd) || !PyCObject_Check(pyds))
    {
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices authenticateUserBasic: could not parse arguments", 0));		
        return NULL;
    }
	
	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		bool result = false;
		if (ds->AuthenticateUserBasic(nodename, user, pswd, result))
		{
			if (result)
				Py_RETURN_TRUE;
			else
				Py_RETURN_FALSE;
		}
	}
	else
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices authenticateUserBasic: invalid directory service argument", 0));		
	
	return NULL;
}

/*
def authenticateUserDigest(obj, guid, user, challenge, response, method):
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
 */
extern "C" PyObject *authenticateUserDigest(PyObject *self, PyObject *args)
{
	PyObject* pyds;
	const char* nodename;
	const char* user;
	const char* challenge;
	const char* response;
	const char* method;
    if (!PyArg_ParseTuple(args, "Osssss", &pyds, &nodename, &user, &challenge, &response, &method) || !PyCObject_Check(pyds))
    {
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices authenticateUserDigest: could not parse arguments", 0));		
        return NULL;
    }
	
	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		bool result = false;
		if (ds->AuthenticateUserDigest(nodename, user, challenge, response, method, result))
		{
			if (result)
				Py_RETURN_TRUE;
			else
				Py_RETURN_FALSE;
		}
	}
	else
		PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", "DirectoryServices authenticateUserDigest: invalid directory service argument", 0));		
	
	return NULL;
}

static PyMethodDef ODMethods[] = {
    {"odInit",  odInit, METH_VARARGS,
		"Initialize the Open Directory system."},
    {"listAllRecordsWithAttributes",  listAllRecordsWithAttributes, METH_VARARGS,
		"List all records of the specified type in Open Directory, returning requested attributes."},
    {"queryRecordsWithAttribute",  queryRecordsWithAttribute, METH_VARARGS,
		"List records in Open Directory matching specified attribute/value, and return key attributes for each one."},
    {"queryRecordsWithAttributes",  queryRecordsWithAttributes, METH_VARARGS,
		"List records in Open Directory matching specified criteria, and return key attributes for each one."},
    {"listAllRecordsWithAttributes_list",  listAllRecordsWithAttributes_list, METH_VARARGS,
		"List all records of the specified type in Open Directory, returning requested attributes."},
    {"queryRecordsWithAttribute_list",  queryRecordsWithAttribute_list, METH_VARARGS,
		"List records in Open Directory matching specified attribute/value, and return key attributes for each one."},
    {"queryRecordsWithAttributes_list",  queryRecordsWithAttributes_list, METH_VARARGS,
		"List records in Open Directory matching specified criteria, and return key attributes for each one."},
    {"authenticateUserBasic",  authenticateUserBasic, METH_VARARGS,
		"Authenticate a user with a password to Open Directory using plain text authentication."},
    {"authenticateUserDigest",  authenticateUserDigest, METH_VARARGS,
		"Authenticate a user with a password to Open Directory using HTTP DIGEST authentication."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC initopendirectory(void)
{
    PyObject* m = Py_InitModule("opendirectory", ODMethods);

    PyObject* d = PyModule_GetDict(m);

    if (!(ODException_class = PyErr_NewException("opendirectory.ODError", NULL, NULL)))
        goto error;
    PyDict_SetItemString(d, "ODError", ODException_class);
    Py_INCREF(ODException_class);


error:
    if (PyErr_Occurred())
		PyErr_SetString(PyExc_ImportError, "opendirectory: init failed");
}
