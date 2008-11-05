/**
 * A class that wraps high-level Directory Service calls needed by the
 * CalDAV server.
 **
 * Copyright (c) 2006-2008 Apple Inc. All rights reserved.
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
 **/

#include "CDirectoryServiceException.h"

#include <Python.h>

#include <stdlib.h>
#include <string.h>

extern PyObject* ODException_class;

#pragma mark -----Public API

CDirectoryServiceException::CDirectoryServiceException()
{
    mDSError = eUndefinedError;
    ::snprintf(mDescription, 1024, "Unknown Error");
}

CDirectoryServiceException::CDirectoryServiceException(tDirStatus error, const char* file, long line)
{
    mDSError = error;
    ::snprintf(mDescription, 1024, "Exception raised in file %s at line %ld", file, line);
}

CDirectoryServiceException::~CDirectoryServiceException()
{
}

void CDirectoryServiceException::ThrowDSError(tDirStatus error, const char* file, long line)
{
    CDirectoryServiceException dirStatus(error, file, line);
    throw dirStatus;
}

void CDirectoryServiceException::SetPythonException()
{
    char error[1024];
    ::snprintf(error, 1024, "%s %s", "DirectoryServices Error:", mDescription);
    PyErr_SetObject(ODException_class, Py_BuildValue("((s:i))", error, mDSError));
}

