#!/usr/bin/env python
import sys
sys.path.insert(0, 'build/lib.macosx-10.4-fat-2.4/')

from DirectoryServices import DirectoryService
from DirectoryServices import kDSRecordsAll

def listNodes(ds):
    i = 1
    for node in ds.getNodeList():
        print "%d: %r" % (i, node)
        i += 1

def openNode(ds, path):
    print "Getting node: %s" % (path,)
    node = ds.getNode(path)
    print "Got node: %s" % (node,)
    print "Opening node."
    node.open()
    print "Listing records."
    listRecords(node)
    print "Closing node."
    node.close()

def listRecords(node):
    recList = node.listRecords()

    for x in recList:
        print x


if __name__ == '__main__':
    ds = DirectoryService()
    listNodes(ds)

    openNode(ds, '/LDAPv3/webboserver.apple.com')

#     openNode(ds, '/LDAPv3/od.apple.com')
    
#     openNode(ds, '/Search')
    
#     #openNode(ds, '/BSD/local')
