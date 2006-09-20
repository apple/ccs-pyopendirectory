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

from DirectoryServices import Constants

cdef nodeFromName(tDirReference directory,
                  tDataListPtr nodeName):
    """Make a new Node from the given nodeName (we have to use this
    because PyRex doesn't support passing pointers to methods)
    """

    return DirectoryNode(directory,
                         <unsigned long>nodeName)


cdef convertDataList(tDirReference directory,
                     object inList,
                     tDataListPtr outList):
    outList = dsDataListAllocate(directory)
        
    for item in inList:
        raiseOnError(
            dsAppendStringToListAlloc(directory, outList, item))


cdef class DirectoryNode:
    cdef tDirReference directory
    cdef tDirNodeReference node
    cdef tDataListPtr nodeName
    cdef char * pPath

    def __new__(object self,
                tDirReference directory,
                unsigned long nodeName):

        self.directory = directory
        self.nodeName = <tDataListPtr>nodeName

    def __repr__(self):
        return "<DirectoryNode: %s>" % (self.getPath())

    def __dealloc__(self):
        self.close()
        raiseOnError(dsDataListDeallocate(self.directory, self.nodeName))

    def open(self):
        raiseOnError(dsOpenDirNode(self.directory,
                                   self.nodeName, 
                                   &self.node))

    def close(self):
        if self.node != 0:
            raiseOnError(dsCloseDirNode(self.node))
            self.node = 0

    def getPath(self):
        if self.pPath == NULL:
            self.pPath = dsGetPathFromList(self.directory,
                                           <tDataListPtr *>self.nodeName,
                                           "/")

        return str(self.pPath)

    def hasRecord(self, recordType, recordName):
        cdef tDataNodePtr recType        
        cdef tDataNodePtr recName
        cdef tRecordReference recRef

        try:
            
            recName = dsDataNodeAllocateString(self.directory,
                                               recordName)
            if recName == NULL:
                raise DirectoryServicesError(eDSBadDataNodeLength)
            
            recType = dsDataNodeAllocateString(self.directory,
                                               recordType)
            if recType == NULL:
                raise DirectoryServicesError(eDSBadDataNodeLength)

            dirStatus = dsOpenRecord(self.node, recType, recName, &recRef)
            
            if dirStatus == eDSNoErr:
                result = true
                dsCloseRecord(recRef)

            elif dirStatus == eDSRecordNotFound:
                result = false

            else:
                raiseOnError(dirStatus)

        finally: 
            if recName != NULL:
                dsDataNodeDeAllocate(self.directory, recName)
            if recType != NULL:
                dsDataNodeDeAllocate(self.directory, recType)

        return result

    def listRecords(self, recordTypes=None, recordNames=None, 
                    attributeTypes=None, matchType=None):
        cdef tDataListPtr recTypes
        cdef tDataListPtr recNames
        cdef tDataListPtr attrTypes
        cdef tDataBufferPtr records
        cdef tContextData context
        context = NULL
        cdef tDirPatternMatch cMatchType
        cdef unsigned long recordCount

        recordList = None

        try:
            if not recordNames:
                recordNames = [Constants.kDSRecordsAll]
                
            if not recordTypes:
                recordTypes = [Constants.kDSStdRecordTypeAll]
                    
            if not attributeTypes:
                attributeTypes = [Constants.kDSAttributesAll]

            if not matchType:
                matchType = eDSExact

            cMatchType = matchType
            
            recNames = dsDataListAllocate(self.directory)
            raiseIfNull(recNames)
            for item in recordNames:
                raiseOnError(
                    dsAppendStringToListAlloc(self.directory, recNames, 
                                              item))

            assert (len(recordNames) == 
                    dsDataListGetNodeCount(recNames))

            recTypes = dsDataListAllocate(self.directory)
            raiseIfNull(recTypes)
            for item in recordTypes:
                raiseOnError(
                    dsAppendStringToListAlloc(self.directory, recTypes, 
                                              item))

            assert (len(recordTypes) == 
                    dsDataListGetNodeCount(recTypes))

            attrTypes = dsDataListAllocate(self.directory)
            raiseIfNull(attrTypes)
            for item in attributeTypes:
                raiseOnError(
                    dsAppendStringToListAlloc(self.directory, attrTypes, 
                                              item))

            assert (len(attributeTypes) == 
                    dsDataListGetNodeCount(attrTypes))

            records = dsDataBufferAllocate(self.directory, 128 * 1024)
            raiseIfNull(records)

            dirStatus = dsGetRecordList(self.node, records, recNames, 
                                        cMatchType, recTypes,
                                        attrTypes, False,
                                        &recordCount, &context)
            raiseOnError(dirStatus)
            
            recordList = DirectoryRecordList(self.directory,
                                             self.node,
                                             <unsigned long>records,
                                             <unsigned long>context,
                                             recordCount)

        finally:
            pass
# #             if recNames != NULL:
# #                 dsDataListDeallocate(self.directory, recNames)
# #             if recTypes != NULL:
# #                 dsDataListDeallocate(self.directory, recTypes)
# #             if attrTypes != NULL:
# #                 dsDataListDeallocate(self.directory, attrTypes)

        return recordList
