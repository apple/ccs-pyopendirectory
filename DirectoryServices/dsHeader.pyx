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

cdef extern from "sys/types.h": # 
    int kill(int i, int i)      #
                                #  For debugging only
cdef extern from "stdio.h":     #
    int printf(char *foo)       #

cdef extern from "DirectoryService/DirectoryService.h":    
    ctypedef enum tDirStatus:
        eDSNoErr = 0
        eDSNodeNotFound = -14008
        eDSRecordNotFound = -14136
        eDSBadDataNodeLength = -14255
        
    ctypedef enum tDirPatternMatch:
        eDSExact = 0x2001

    ctypedef unsigned long tDirReference
    ctypedef unsigned long tDirNodeReference
    ctypedef unsigned long tRecordReference
    ctypedef unsigned long tAttributeListRef
    ctypedef unsigned long dsBool
    
    ctypedef void * tClientData
    ctypedef void * tBuffer
    ctypedef void * tContextData
    ctypedef void * tDataBufferPtr
    ctypedef void * tDataListPtr    
    ctypedef void * tDataNodePtr
    ctypedef void * tRecordEntryPtr
    ctypedef void * tRecordEntry

    ctypedef struct tDataList:
        unsigned long fDataNodeCount
        tDataNodePtr fDataListHead

    tDirStatus dsOpenDirService(tDirReference *outDirReference)

    tDirStatus dsCloseDirService(tDirReference inDirReference)

    tDirStatus dsGetDirNodeCount(tDirReference inDirReference,
                                 unsigned long *outNodeCount)

    tDirStatus dsGetDirNodeName(tDirReference inDirReference,
                                tDataBufferPtr inOutDataBufferPtr,
                                unsigned long inDirNodeIndex,
                                tDataListPtr *inOutDataList)

    tDirStatus dsGetDirNodeList(tDirReference inDirReference,
                                tDataBufferPtr inOutDataBufferPtr,
                                unsigned long *outDirNodeCount,
                                tContextData *inOutCountinueData)

    tDataBufferPtr dsDataBufferAllocate(tDirReference inDirReference,
                                        unsigned long inBufferSize)

    char *dsGetPathFromList(tDirReference inDirReference,
                                 tDataListPtr *inDataList,
                                 char *inDelimiter)

    tDirStatus dsDataBufferDeAllocate(tDirReference inDirReference,
                                      tDataBufferPtr inDataBufferPtr)

    tDirStatus dsReleaseContinueData(tDirReference inDirReference,
                                     tContextData inContinueData)

    tDataListPtr dsDataListAllocate(tDirReference inDirReference)

    tDirStatus dsDataListDeallocate(tDirReference inDirReference,
                                    tDataListPtr inDataList)

    tDirStatus dsBuildListFromPathAlloc(tDirReference inDirReference,
                                        tDataListPtr inDataList,
                                        char *inPathCString,
                                        char *inPathSeparatorCString)

    tDirStatus dsOpenDirNode(tDirReference inDirReference,
                             tDataListPtr inDataList,
                             tDirNodeReference *outDirNodeReference)

    tDirStatus dsCloseDirNode(tDirNodeReference inDirNodeReference)

    tDirStatus dsOpenRecord(tDirNodeReference inDirNodeReference,
                            tDataNodePtr inRecordType,
                            tDataNodePtr inRecordName,
                            tRecordReference *outRecordReference)

    tDirStatus dsCloseRecord(tRecordReference inRecordReference)

    tDataNodePtr dsDataNodeAllocateString(tDirReference inDirReference,
                                        char *inCString)

    tDirStatus dsDataNodeDeAllocate(tDirReference inDirReference,
                                    tDataNodePtr inDataNodePtr)
    
    tDirStatus dsAppendStringToListAlloc(tDirReference inDirReference,
                                         tDataListPtr inDataList,
                                         char *inCString)

    tDirStatus dsGetRecordList(tDirNodeReference inDirNodeReference,
                               tDataBufferPtr inOutDataBuffer,
                               tDataListPtr inRecordNameList,
                               tDirPatternMatch inPatternMatchType,
                               tDataListPtr inRecordTypeList,
                               tDataListPtr inAttributeTypeList,
                               dsBool inAttributeInfoOnly,
                               unsigned long *inOutRecordEntryCount,
                               tContextData *inOutContinueData)

    tDirStatus dsGetRecordEntry(tDirNodeReference inDirNodeReference,
                                tDataBufferPtr inOutDataBuffer,
                                unsigned long inRecordEntryIndex,
                                tAttributeListRef *outAttributeListRef,
                                tRecordEntryPtr *outRecordEntryPtr)

    tDirStatus dsCloseAttributeList(tAttributeListRef inAttributeListRef)
    
    tDirStatus dsDeallocRecordEntry(tDirReference inDirRef, 
                                    tRecordEntryPtr inRecEntry)
    
    tDirStatus dsDataListGetNodeAlloc(tDirReference inDirReference,
                                      tDataList *inDataListPtr,
                                      unsigned long inNodeIndex,
                                      tDataNodePtr *outDataNode)
                                      
    unsigned long dsDataListGetNodeCount(tDataListPtr inDataListPtr)
