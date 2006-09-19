cdef extern from "DirectoryService/DirectoryService.h":

    ctypedef unsigned long tDirNodeReference

    tDirStatus dsOpenDirNode(tDirReference inDirReference,
                             tDataListPtr inDataList,
                             tDirNodeReference *outDirNodeReference)

    tDirStatus dsCloseDirNode(tDirNodeReference inDirNodeReference)


cdef nodeFromName(tDirReference directory,
                  tDataListPtr nodeName):
    """Make a new Node from the given nodeName (we have to use this
    because PyRex doesn't support passing pointers to methods)
    """

    return DirectoryNode(directory,
                         <unsigned long>nodeName)


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

    def __repr__(self):
        return "<DirectoryNode: %s>" % (self.getPath())

    def __dealloc__(self):
        self.close()
        raiseOnError(dsDataListDeallocate(self.directory, self.nodeName))
