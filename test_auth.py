#!/usr/bin/env python

from getpass import getpass
import opendirectory
import dsattributes
import md5
import sha

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
    if pszQop == "auth-int":
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

realm = "/Search"
nonce = "128446648710842461101646794502"
nc = "00000001"
cnonce = "0a4f113b12345"
uri = "/principals/"
method = "GET"

def doAuthDigest(username, password, qop, algorithm):
    failures = 0
    
    result = opendirectory.queryRecordsWithAttribute(
        od,
        dsattributes.kDSNAttrRecordName,
        username,
        dsattributes.eDSExact,
        False,
        dsattributes.kDSStdRecordTypeUsers,
        [dsattributes.kDS1AttrGeneratedUID])
    guid = result[username][dsattributes.kDS1AttrGeneratedUID]
    
    expected = calcResponse(
                calcHA1(algorithm, username, realm, password, nonce, cnonce),
                algorithm, nonce, nc, cnonce, qop, method, uri, None
            )
    #print expected
    
    if qop:
        challenge = 'Digest realm="%s", nonce="%s", algorithm=%s, qop="%s"' % (realm, nonce, algorithm, qop,)
    else:
        challenge = 'Digest realm="%s", nonce="%s", algorithm=%s' % (realm, nonce, algorithm,)
    if qop:
        response = ('Digest username="%s", realm="%s", '
                'nonce="%s", digest-uri="%s", '
                'response=%s, algorithm=%s, cnonce="%s", qop=%s, nc=%s' % (username, realm, nonce, uri, expected, algorithm, cnonce, qop, nc, ))
    else:
        response = ('Digest username="%s", realm="%s", '
                'nonce="%s", digest-uri="%s", '
                'response=%s, algorithm=%s' % (username, realm, nonce, uri, expected, algorithm, ))
    
    print "    Challenge: %s" % (challenge,)
    print "    Response:  %s" % (response, )
    
    for x in xrange(attempts):
        success = opendirectory.authenticateUserDigest(
            od, 
            guid,
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
    
    result = opendirectory.queryRecordsWithAttribute(
        od,
        dsattributes.kDSNAttrRecordName,
        username,
        dsattributes.eDSExact,
        False,
        dsattributes.kDSStdRecordTypeUsers,
        [dsattributes.kDS1AttrGeneratedUID])
    guid = result[username][dsattributes.kDS1AttrGeneratedUID]
    
    for x in xrange(attempts):
        success = opendirectory.authenticateUserBasic(
            od, 
            guid,
            username,
            password,
        )
    
        if not success:
            failures += 1
    
    print "\n%d failures out of %d attempts for Basic.\n\n" % (failures, attempts)

search = raw_input("DS search path: ")
user = raw_input("User: ")
pswd = getpass("Password: ")

od = opendirectory.odInit(search)

doAuthBasic(user, pswd)
doAuthDigest(user, pswd, None, "md5")
#doAuth(user, pswd, "auth", "md5")
#doAuth(user, pswd, "auth", "md5-sess")

