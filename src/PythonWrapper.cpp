/**
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * DRI: Cyrus Daboo, cdaboo@apple.com
 **/

#include <CoreFoundation/CoreFoundation.h>
#include <Python/Python.h>

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
	PyObject* pystr = NULL;
	const char* bytes = CFStringGetCStringPtr(str, kCFStringEncodingUTF8);
	
	if (bytes == NULL)
	{
		char localBuffer[256];
		Boolean success = CFStringGetCString(str, localBuffer, 256, kCFStringEncodingUTF8);
		if (!success)
			localBuffer[0] = 0;
		pystr = PyString_FromString(localBuffer);
	}
	else
	{
		pystr = PyString_FromString(bytes);
	}
	
	return pystr;
}

// Utility function - not exposed to Python
static PyObject* CFArrayToPyTuple(CFArrayRef list, bool sorted = false)
{
	CFIndex lsize = CFArrayGetCount(list);
	if (sorted)
		CFArraySortValues((CFMutableArrayRef)list, CFRangeMake(0, lsize), (CFComparatorFunction)CFStringCompare, NULL);
	
	PyObject* result = PyTuple_New(lsize);
	for(CFIndex i = 0; i < lsize; i++)
	{
		CFStringRef str = (CFStringRef)CFArrayGetValueAtIndex(list, i);
		PyObject* pystr = CFStringToPyStr(str);
		
		PyTuple_SetItem(result, i, pystr);
	}
	
	return result;
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
static CFComparisonResult CompareRecordListValues(const void *val1, const void *val2, void *context)
{
	CFMutableArrayRef l1 = (CFMutableArrayRef)val1;
	CFMutableArrayRef l2 = (CFMutableArrayRef)val2;
	CFIndex c1 = CFArrayGetCount(l1);
	CFIndex c2 = CFArrayGetCount(l2);
	if ((c1 > 0) && (c2 > 0))
	{
		return CFStringCompare((CFStringRef)CFArrayGetValueAtIndex(l1, 0), (CFStringRef)CFArrayGetValueAtIndex(l2, 0), NULL);
	}
	else if (c1 > 0)
		return kCFCompareGreaterThan;
	else if (c2 > 0)
		return kCFCompareLessThan;
	else
		return kCFCompareEqualTo;
}

// Utility function - not exposed to Python
static PyObject* CFArrayArrayToPyList(CFMutableArrayRef list, bool sorted = false)
{
	CFIndex lsize = (list != NULL) ? CFArrayGetCount(list) : 0;
	if (sorted and (list != NULL))
		CFArraySortValues(list, CFRangeMake(0, lsize), CompareRecordListValues, NULL);
	
	PyObject* result = PyList_New(lsize);
	for(CFIndex i = 0; i < lsize; i++)
	{
		CFMutableArrayRef array = (CFMutableArrayRef)CFArrayGetValueAtIndex(list, i);
		PyObject* pytuple = CFArrayToPyTuple(array);
		
		PyList_SetItem(result, i, pytuple);
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
	}
	else if(CFGetTypeID((CFTypeRef)value) == CFArrayGetTypeID())
	{
		CFArrayRef arrayvalue = (CFArrayRef)value;
		PyObject* pylistvalue = CFArrayToPyList(arrayvalue);
		PyDict_SetItem(dict, pystrkey, pylistvalue);
	}
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
}

// Utility function - not exposed to Python
static PyObject* CFDictionaryDictionaryToPyDict(CFDictionaryRef dict)
{
	PyObject* result = PyDict_New();
	if (dict != NULL)
		CFDictionaryApplyFunction(dict, CFDictionaryDictionaryIterator, result);
	
	return result;
}

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
    int result = 0;

    const char* nodename;
    if (!PyArg_ParseTuple(args, "s", &nodename))
        return NULL;
	
	CDirectoryService* ds = new CDirectoryService(nodename);
	PyObject* pyds;
	if (ds != NULL)
	{
		pyds = PyCObject_FromVoidPtr(ds, odDestroy);
		result = 1;
	}
	
	if (result == 1)
		return pyds;
	else
		Py_RETURN_NONE;
}

/*
 def listUsers(obj):
	"""
	List users in Open Directory, and return key attributes for each user.
	The attributes in the tuple are (uid, guid, last-modified, calendar-principal-uri).
	
	@param obj: C{object} the object obtained from an odInit call.
	@return: C{list} containing a C{tuple} of C{str} for each user found,
		or C{None} on failure.
	"""
 */
extern "C" PyObject *listUsers(PyObject *self, PyObject *args)
{
	PyObject* pyds;
	PyObject* result = NULL;
    if (!PyArg_ParseTuple(args, "O", &pyds) || !PyCObject_Check(pyds))
        return NULL;
	
	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		CFMutableArrayRef list = ds->ListUsers();
		if (list != NULL)
		{
			result = CFArrayArrayToPyList(list, true);
			CFRelease(list);
		}
		else
			result = PyList_New(0);
		
		return result;
	}
	else
		Py_RETURN_NONE;
}

/*
 def listGroups(obj):
	"""
	List groups in Open Directory, and return key attributes for each group.
	The attributes in the tuple are (uid, guid, last-modified, calendar-principal-uri).
	
	@param obj: C{object} the object obtained from an odInit call.
	@return: C{list} containg a C{tuple} of C{str} for each group found,
		or C{None} on failure.
	"""
 */
extern "C" PyObject *listGroups(PyObject *self, PyObject *args)
{
	PyObject* pyds;
	PyObject* result = NULL;
    if (!PyArg_ParseTuple(args, "O", &pyds) || !PyCObject_Check(pyds))
        return NULL;
	
	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		CFMutableArrayRef list = ds->ListGroups();
		if (list != NULL)
		{
			result = CFArrayArrayToPyList(list, true);
			CFRelease(list);
		}
		else
			result = PyList_New(0);
		
		return result;
	}
	else
		Py_RETURN_NONE;
}

/*
 def listResources(obj):
	"""
	List resources in Open Directory, and return key attributes for each resource.
	The attributes in the tuple are (uid, guid, last-modified, calendar-principal-uri).
	
	@param obj: C{object} the object obtained from an odInit call.
	@return: C{list} containg a C{tuple} of C{str} for each resource found,
		or C{None} on failure.
	"""
 */
extern "C" PyObject *listResources(PyObject *self, PyObject *args)
{
	PyObject* pyds;
	PyObject* result = NULL;
    if (!PyArg_ParseTuple(args, "O", &pyds) || !PyCObject_Check(pyds))
        return NULL;
	
	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		CFMutableArrayRef list = ds->ListResources();
		if (list != NULL)
		{
			result = CFArrayArrayToPyList(list, true);
			CFRelease(list);
		}
		else
			result = PyList_New(0);
		
		return result;
	}
	else
		Py_RETURN_NONE;
}

/*
 def checkUser(obj, user):
	"""
	Check that a user exists in Open Directory.
	
	@param obj: C{object} the object obtained from an odInit call.
	@param user: C{str} containing the user to check.
	@return: C{True} if the user was found, C{False} otherwise.
	"""
 */
extern "C" PyObject *checkUser(PyObject *self, PyObject *args)
{
	bool result = false;
	
	PyObject* pyds;
	const char* user;
    if (!PyArg_ParseTuple(args, "Os", &pyds, &user) || !PyCObject_Check(pyds))
        return NULL;

	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		result = ds->CheckUser(user);
	}
	
	if (result)
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

/*
 def checkGroup(obj, group):
	"""
	Check that a group exists in Open Directory.
	
	@param obj: C{object} the object obtained from an odInit call.
	@param group: C{str} containing the group to check.
	@return: C{True} if the group was found, C{False} otherwise.
	"""
 */
extern "C" PyObject *checkGroup(PyObject *self, PyObject *args)
{
	bool result = false;
	
	PyObject* pyds;
	const char* group;
    if (!PyArg_ParseTuple(args, "Os", &pyds, &group) || !PyCObject_Check(pyds))
        return NULL;
	
	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		result = ds->CheckGroup(group);
	}
	
	if (result)
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

/*
 def checkResource(obj, resource):
	"""
	Check that a resource exists in Open Directory.
	
	@param obj: C{object} the object obtained from an odInit call.
	@param resource: C{str} containing the resource to check.
	@return: C{True} if the resource was found, C{False} otherwise.
	"""
 */
extern "C" PyObject *checkResource(PyObject *self, PyObject *args)
{
	bool result = false;
	
	PyObject* pyds;
	const char* resource;
    if (!PyArg_ParseTuple(args, "Os", &pyds, &resource) || !PyCObject_Check(pyds))
        return NULL;
	
	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		result = ds->CheckResource(resource);
	}
	
	if (result)
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

/*
 def listUsersWithAttributes(obj, users):
	"""
	Get user attributes relevant to CalDAV from Open Directory.
	
	@param obj: C{object} the object obtained from an odInit call.
	@param users: C{list} containing C{str}'s for each user to get attributes for.
	@return: C{dict} of attributes if the user was found, C{None} otherwise.
	"""
 */
extern "C" PyObject *listUsersWithAttributes(PyObject *self, PyObject *args)
{
	PyObject* pyds;
	PyObject* users;
    if (!PyArg_ParseTuple(args, "OO", &pyds, &users) || !PyCObject_Check(pyds) || !PyList_Check(users))
        return NULL;
	
	// Convert list to CFArray of CFString
	CFArrayRef cfusers = PyListToCFArray(users);
	if (cfusers == NULL)
		return NULL;

	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		CFMutableDictionaryRef dict = ds->ListUsersWithAttributes(cfusers);
		if (dict != NULL)
		{
			PyObject* result = CFDictionaryDictionaryToPyDict(dict);
			CFRelease(dict);
			CFRelease(cfusers);
			
			return result;
		}
	}
	
	CFRelease(cfusers);
	Py_RETURN_NONE;
}

/*
 def listGroupsWithAttributes(obj, groups):
	"""
	Get group attributes relevant to CalDAV from Open Directory.
	
	@param obj: C{object} the object obtained from an odInit call.
	@param groups: C{list} containing C{str}'s for each group to get attributes for.
	@return: C{dict} of attributes if the group was found, C{None} otherwise.
	"""
 */
extern "C" PyObject *listGroupsWithAttributes(PyObject *self, PyObject *args)
{
	PyObject* pyds;
	PyObject* grps;
    if (!PyArg_ParseTuple(args, "OO", &pyds, &grps) || !PyCObject_Check(pyds) || !PyList_Check(grps))
        return NULL;
	
	// Convert list to CFArray of CFString
	CFArrayRef cfgrps = PyListToCFArray(grps);
	if (cfgrps == NULL)
		return NULL;
	
	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		CFMutableDictionaryRef dict = ds->ListGroupsWithAttributes(cfgrps);
		if (dict != NULL)
		{
			PyObject* result = CFDictionaryDictionaryToPyDict(dict);
			CFRelease(dict);
			CFRelease(cfgrps);
			
			return result;
		}
	}
	
	CFRelease(cfgrps);
	Py_RETURN_NONE;
}

/*
 def listResourcesWithAttributes(obj, rsrcs):
	"""
	Get resource attributes relevant to CalDAV from Open Directory.
	
	@param obj: C{object} the object obtained from an odInit call.
	@param rsrcs: C{list} containing C{str}'s for each resource to get attributes for.
	@return: C{dict} of attributes if the resource was found, C{None} otherwise.
	"""
 */
extern "C" PyObject *listResourcesWithAttributes(PyObject *self, PyObject *args)
{
	PyObject* pyds;
	PyObject* rsrcs;
    if (!PyArg_ParseTuple(args, "OO", &pyds, &rsrcs) || !PyCObject_Check(pyds) || !PyList_Check(rsrcs))
        return NULL;
	
	// Convert list to CFArray of CFString
	CFArrayRef cfrsrcs = PyListToCFArray(rsrcs);
	if (cfrsrcs == NULL)
		return NULL;
	
	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		CFMutableDictionaryRef dict = ds->ListResourcesWithAttributes(cfrsrcs);
		if (dict != NULL)
		{
			PyObject* result = CFDictionaryDictionaryToPyDict(dict);
			CFRelease(dict);
			CFRelease(cfrsrcs);
			
			return result;
		}
	}
	
	CFRelease(cfrsrcs);
	Py_RETURN_NONE;
}

/*
 def authenticateUser(obj, user, pswd):
	"""
	Authenticate a user with a password to Open Directory.
	
	@param obj: C{object} the object obtained from an odInit call.
	@param user: C{str} container the user to check.
	@param pswd: C{str} containing the password to check.
	@return: C{True} if the user was found, C{False} otherwise.
	"""
 */
extern "C" PyObject *authenticateUser(PyObject *self, PyObject *args)
{
	bool result = false;
	
	PyObject* pyds;
	const char* user;
	const char* pswd;
    if (!PyArg_ParseTuple(args, "Oss", &pyds, &user, &pswd) || !PyCObject_Check(pyds))
        return NULL;
	
	CDirectoryService* ds = static_cast<CDirectoryService*>(PyCObject_AsVoidPtr(pyds));
	if (ds != NULL)
	{
		result = ds->AuthenticateUser(user, pswd);
	}
	
	if (result)
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

static PyMethodDef ODMethods[] = {
    {"odInit",  odInit, METH_VARARGS,
		"Initialize the Open Directory system."},
    {"listUsers",  listUsers, METH_VARARGS,
		"List all users in Open Directory."},
    {"listGroups",  listGroups, METH_VARARGS,
		"List all groups in Open Directory."},
    {"listResources",  listResources, METH_VARARGS,
		"List all resources in Open Directory."},
    {"checkUser",  checkUser, METH_VARARGS,
		"Check that a user exists in Open Directory."},
    {"checkGroup",  checkGroup, METH_VARARGS,
		"Check that a group exists in Open Directory."},
    {"checkResource",  checkResource, METH_VARARGS,
		"Check that a resource exists in Open Directory."},
    {"listUsersWithAttributes",  listUsersWithAttributes, METH_VARARGS,
		"Get user attributes relevant to CalDAV from Open Directory."},
    {"listGroupsWithAttributes",  listGroupsWithAttributes, METH_VARARGS,
		"Get group attributes relevant to CalDAV from Open Directory."},
    {"listResourcesWithAttributes",  listResourcesWithAttributes, METH_VARARGS,
		"Get resource attributes relevant to CalDAV from Open Directory."},
    {"authenticateUser",  authenticateUser, METH_VARARGS,
		"Authenticate a user with a password to Open Directory."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC initopendirectory(void)
{
    (void) Py_InitModule("opendirectory", ODMethods);
}
