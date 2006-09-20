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
##

class DirectoryRecordListIter:
    def __init__(self, dr):
        self.dr = dr

    def next(self):
        return self.dr.next()

cdef class DirectoryRecordList:
    cdef tDirReference directory
    cdef tDirNodeReference node
    cdef tDataBufferPtr records
    cdef tDataListPtr recordTypes
    cdef tDataListPtr recordNames
    cdef tDataListPtr attributeTypes
    cdef tContextData context
    cdef unsigned long recordCount
    cdef unsigned long recordIndex    

    def __new__(self, tDirReference directory, 
                tDirNodeReference node, unsigned long recordList, 
                unsigned long context, unsigned long recordCount):
        self.directory = directory
        self.node = node

        self.records = <tDataBufferPtr>recordList
        raiseIfNull(self.records)

        self.context = <tContextData>context

        self.recordCount = recordCount
        self.recordIndex = 1

    def __dealloc__(self):
        dsReleaseContinueData(self.directory, self.context)

    def __len__(self):
        return self.recordCount

    def _getRecord(self, index):
        cdef tAttributeListRef attrListRef
        cdef tRecordEntryPtr pRecEntry

        raiseOnError(
            dsGetRecordEntry(self.node, self.records, index, 
                             &attrListRef, &pRecEntry))

        record = DirectoryRecord(attrListRef, <unsigned long>pRecEntry)
        
        dsCloseAttributeList(attrListRef)
        dsDeallocRecordEntry(self.directory, pRecEntry)

        return record

    def __getitem__(self, unsigned long index):
        self._getRecord(index)

    def __iter__(self):
        return DirectoryRecordListIter(self)

    def next(self):
        if self.context == NULL:
            raise StopIteration

        rec = self._getRecord(self.recordIndex)
        self.recordIndex = self.recordIndex + 1

        return rec


cdef class DirectoryRecord:
    def __new__(self, tAttributeListRef attrListRef, unsigned long pRecEntry):
        pass

    def __dealloc__(self):
        pass

    def __getitem__(self, key):
        pass

    def __setitem__(self, key, value):
        pass

    def flush(self):
        pass

    

