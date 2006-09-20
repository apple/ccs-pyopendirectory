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

include "dsHeader.pyx"
include "DirectoryNode.pyx"
include "DirectoryRecord.pyx"

class DirectoryServicesError(Exception):
    def __init__(self, errno):
        self.errno = errno

    def __str__(self):
        return "<DirectoryServicesError: %d>" % (self.errno,)

    def __repr__(self):
        return self.__str__()


cdef raiseIfNull(void *ptr):
    if ptr == NULL:
        raise ValueError("Null pointer")


def raiseOnError(dirStatus):
    if dirStatus != eDSNoErr:
        raise DirectoryServicesError(dirStatus)

        
cdef class DirectoryService:
    cdef tDirReference gDirRef

    def __new__(self):
        raiseOnError(dsOpenDirService(&self.gDirRef))

    def __dealloc__(self):
        raiseOnError(dsCloseDirService(self.gDirRef))

    def getNodeCount(self):
        cdef unsigned long nodeCount
        nodeCount = 0

        raiseOnError(dsGetDirNodeCount(self.gDirRef, &nodeCount))

        return nodeCount

    def getNodeList(self):
        cdef tDataBufferPtr dataBuffer
        cdef unsigned long bufferCount
        cdef tContextData context
        cdef tDataListPtr nodeName
        cdef unsigned long index

        done = False

        dataBuffer = dsDataBufferAllocate(self.gDirRef, 32 * 1024)

        nodes = []
        try:
            if dataBuffer != NULL:
                while done == False:
                    raiseOnError(
                        dsGetDirNodeList(self.gDirRef, dataBuffer, 
                                         &bufferCount, &context))

                    for index from 1 <= index < bufferCount:
                        raiseOnError(
                            dsGetDirNodeName(self.gDirRef,
                                             dataBuffer,
                                             index,
                                             &nodeName))

                        nodes.append(nodeFromName(self.gDirRef, nodeName))
                        
                    done = (context == NULL)
        finally:
            if context != NULL:
                raiseOnError(
                    dsReleaseContinueData(self.gDirRef, context))

            raiseOnError(dsDataBufferDeAllocate(self.gDirRef, dataBuffer))

            dataBuffer = NULL

        return nodes

    def getNode(object self, char * nodePath):
        cdef tDataListPtr nodeName

        nodeName = dsDataListAllocate(self.gDirRef)
        raiseOnError(dsBuildListFromPathAlloc(self.gDirRef,
                                              nodeName,
                                              <char *>nodePath,
                                              <char *>"/"))

        return nodeFromName(self.gDirRef, nodeName)

