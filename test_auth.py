#!/usr/bin/env python
##
# Copyright (c) 2006-2008 Apple Inc. All rights reserved.
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
##

from getpass import getpass
import opendirectory
import dsattributes
import md5
import sha
import shlex

algorithms = {
    'md5': md5.new,
    'md5-sess': md5.new,
    'sha': sha.new,
}

# DigestCalcHA1
def calcHA1(
    pszAlg,
    pszUserName,
    pszRealm,
    pszPassword,
    pszNonce,
    pszCNonce,
    preHA1=None
):
    """
    @param pszAlg: The name of the algorithm to use to calculate the digest.
        Currently supported are md5 md5-sess and sha.

    @param pszUserName: The username
    @param pszRealm: The realm
    @param pszPassword: The password
    @param pszNonce: The nonce
    @param pszCNonce: The cnonce

    @param preHA1: If available this is a str containing a previously
        calculated HA1 as a hex string. If this is given then the values for
        pszUserName, pszRealm, and pszPassword are ignored.
    """

    if (preHA1 and (pszUserName or pszRealm or pszPassword)):
        raise TypeError(("preHA1 is incompatible with the pszUserName, "
                         "pszRealm, and pszPassword arguments"))

    if preHA1 is None:
        # We need to calculate the HA1 from the username:realm:password
        m = algorithms[pszAlg]()
        m.update(pszUserName)
        m.update(":")
        m.update(pszRealm)
        m.update(":")
        m.update(pszPassword)
        HA1 = m.digest()
    else:
        # We were given a username:realm:password
        HA1 = preHA1.decode('hex')

    if pszAlg == "md5-sess":
        m = algorithms[pszAlg]()
        m.update(HA1)
        m.update(":")
        m.update(pszNonce)
        m.update(":")
        m.update(pszCNonce)
        HA1 = m.digest()

    return HA1.encode('hex')

# DigestCalcResponse
def calcResponse(
    HA1,
    algo,
    pszNonce,
    pszNonceCount,
    pszCNonce,
    pszQop,
    pszMethod,
    pszDigestUri,
    pszHEntity,
):
    m = algorithms[algo]()
    m.update(pszMethod)
    m.update(":")
    m.update(pszDigestUri)
    if pszQop == "auth-int" or pszQop == "auth-conf":
        m.update(":")
        m.update(pszHEntity)
    HA2 = m.digest().encode('hex')

    m = algorithms[algo]()
    m.update(HA1)
    m.update(":")
    m.update(pszNonce)
    m.update(":")
    if pszNonceCount and pszCNonce and pszQop:
        m.update(pszNonceCount)
        m.update(":")
        m.update(pszCNonce)
        m.update(":")
        m.update(pszQop)
        m.update(":")
    m.update(HA2)
    respHash = m.digest().encode('hex')
    return respHash


attempts = 100


def doAuthDigest(username, password, qop, algorithm):
    failures = 0
    
    realm = "host.example.com"
    nonce = "128446648710842461101646794502"
    nc = "00000001"
    cnonce = "/rrD6TqPA3lHRmg+fw/vyU6oWoQgzK7h9yWrsCmv/lE="
    uri = "http://host.example.com"
    method = "GET"
    entity = "00000000000000000000000000000000"
    cipher = "rc4"
    maxbuf = "65536"

    result = opendirectory.queryRecordsWithAttribute_list(
        od,
        dsattributes.kDSNAttrRecordName,
        username,
        dsattributes.eDSExact,
        False,
        dsattributes.kDSStdRecordTypeUsers,
        [dsattributes.kDSNAttrMetaNodeLocation])
    if not result:
        print "Failed to get record for user: %s" % (username,)
        return
    nodename = result[0][1][dsattributes.kDSNAttrMetaNodeLocation]
    
    print( '    User node= "%s"' % nodename)
    adUser = nodename.startswith("/Active Directory/")
        
    for _ignore_x in xrange(attempts):
        if adUser:
    
            challenge = opendirectory.getDigestMD5ChallengeFromActiveDirectory(od, nodename)
            if not challenge:
                print "Failed to get Active Directory challenge for user: %s" % (username,)
                return
            # parse challenge
            
            l = shlex.shlex(challenge)
            l.wordchars = l.wordchars + "_-"
            l.whitespace = l.whitespace + "=,"
            auth = {}
            while 1:
                k = l.get_token()
                if not k: 
                    break
                v = l.get_token()
                if not v: 
                    break
                v = v.strip('"') # this strip is kind of a hack, should remove matched leading and trailing double quotes
                       
                auth[k.strip()] = v.strip()
                    
            # get expected response parameters from challenge
            nonce = auth["nonce"]
            #nonce = "+Upgraded+v17fa28b0e0cb4c483144a0d568259ca0102de13e7b48ff9261cfa9748b93f83cc09d8ee50638c6d9794e1b4f8485a7dee"
        
            if auth.get("digest-uri", False):
                uri = auth["digest-uri"]
                
            qopstr = auth.get("qop", False)
            if qopstr:
                qops = qopstr.split(",")
                if "auth-conf" in qops:
                    qop = "auth-conf"
                elif "auth-int" in qops:
                    qop = "auth-int"
                elif "quth" in qops:
                    qop = "auth"
                else:
                    qop = qops[0]
                    
            if auth.get("realm", False):
                realm = auth["realm"]
            if auth.get("algorithm", False):
                algorithm = auth["algorithm"]
            
            cipherstr = auth.get("cipher", False)
            if cipherstr:
                ciphers = cipherstr.split(",")
                if "rc4" in ciphers:
                    cipher = "rc4"
                else:
                    cipher = ciphers[0]
            
            if auth.get("maxbuf", False):
                maxbuf = auth["maxbuf"]
                
            method = "AUTHENTICATE"
                
        else:
            
            if qop:
                challenge = 'realm="%s", nonce="%s", algorithm=%s, qop="%s"' % (realm, nonce, algorithm, qop,)
            else:
                challenge = 'realm="%s", nonce="%s", algorithm=%s' % (realm, nonce, algorithm,)
        
        
        expected = calcResponse(
                    calcHA1(algorithm, username, realm, password, nonce, cnonce),
                    algorithm, nonce, nc, cnonce, qop, method, uri, entity
                )
        
        if qop:
            response = ('username="%s",realm="%s",algorithm=%s,'
                    'nonce="%s",cnonce="%s",nc=%s,qop=%s,'
                    'cipher=%s,maxbuf=%s,digest-uri="%s",response=%s' % (username, realm, algorithm,
                                                                              nonce, cnonce, nc, qop, 
                                                                              cipher, maxbuf, uri, expected ))
        else:
            response = ('Digest username="%s", realm="%s", '
                    'nonce="%s", digest-uri="%s", '
                    'response=%s, algorithm=%s' % (username, realm, nonce, uri, expected, algorithm, ))
        
        print "    Challenge: %s" % (challenge,)
        print "    Response:  %s" % (response, )

    
        if adUser:
            success = opendirectory.authenticateUserDigestToActiveDirectory(
                od, 
                nodename,
                username,
                response,
            )
        else:
            success = opendirectory.authenticateUserDigest(
                od, 
                nodename,
                username,
                challenge,
                response,
                method
            )
             
        if not success:
            failures += 1
    
    print "\n%d failures out of %d attempts for Digest.\n\n" % (failures, attempts)

def doAuthBasic(username, password):
    failures = 0
    
    result = opendirectory.queryRecordsWithAttribute_list(
        od,
        dsattributes.kDSNAttrRecordName,
        username,
        dsattributes.eDSExact,
        False,
        dsattributes.kDSStdRecordTypeUsers,
        [dsattributes.kDSNAttrMetaNodeLocation])
    if not result:
        print "Failed to get record for user: %s" % (username,)
        return
    nodename = result[0][1][dsattributes.kDSNAttrMetaNodeLocation]
    
    for _ignore_x in xrange(attempts):
        success = opendirectory.authenticateUserBasic(
            od, 
            nodename,
            username,
            password,
        )
    
        if not success:
            failures += 1
    
    print "\n%d failures out of %d attempts for Basic.\n\n" % (failures, attempts)
"""
search = raw_input("DS search path: ")
user = raw_input("User: ")
pswd = getpass("Password: ")
attempts = int(raw_input("Number of attempts: "))
"""

# to test, bind your client to Active Directory that contains the user specified below

search = "/Search"
user = "servicetest"
pswd = "pass"
attempts = 10

od = opendirectory.odInit(search)

doAuthBasic(user, pswd)
doAuthDigest(user, pswd, None, "md5")

# to test, bind your client to an Open Directory master that contains the user specified below

user = "testuser"
pswd = "test"
doAuthBasic(user, pswd)
doAuthDigest(user, pswd, None, "md5")
doAuthDigest(user, pswd, "auth-int", "md5")
doAuthDigest(user, pswd, "auth-int", "md5-sess")
doAuthDigest(user, pswd, "auth-conf", "md5-sess")

