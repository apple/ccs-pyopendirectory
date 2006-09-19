cdef extern from "DirectoryService/DirectoryService.h":    
    ctypedef enum tDirStatus:
        eDSNoErr = 0
        eDSNodeNotFound = -14008

    ctypedef unsigned long tDirReference

    ctypedef void * tClientData
    ctypedef void * tBuffer
    ctypedef void * tContextData
    ctypedef void * tDataBufferPtr
    ctypedef void * tDataListPtr    

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


# DirectoryNode definitions
include "DirectoryNode.pyx"

class DirectoryServicesError(Exception):
    def __init__(self, errno):
        self.errno = errno

    def __repr__(self):
        return "<DirectoryServicesError: %d>" % (self.errno,)


def raiseOnError(dirStatus):
    if dirStatus != eDSNoErr:
        print "Oh no:", dirStatus
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
        dataBuffer = NULL

        done = False

        cdef tDirStatus dirStatus
        dirStatus = eDSNoErr

        cdef unsigned long bufferCount
        bufferCount = 0

        cdef tContextData context
        context = NULL
        
        cdef tDataListPtr nodeName
        nodeName = NULL

        cdef unsigned long index

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
