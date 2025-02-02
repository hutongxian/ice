//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceSSL/Config.h>

#include <IceSSL/SecureTransportEngineF.h>
#include <IceSSL/SecureTransportEngine.h>

#include <IceUtil/FileUtil.h>
#include <IceUtil/StringUtil.h>

#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <Ice/Communicator.h>
#include <Ice/Logger.h>
#include <Ice/LoggerUtil.h>

#include <IceSSL/SecureTransportTransceiverI.h>
#include <IceSSL/SecureTransportUtil.h>
#include <IceSSL/Plugin.h>
#include <IceSSL/SSLEngine.h>
#include <IceSSL/Util.h>

#include <regex.h>

// Disable deprecation warnings from SecureTransport APIs
#include <IceUtil/DisableWarnings.h>

using namespace std;
using namespace IceUtil;
using namespace Ice;
using namespace IceInternal;
using namespace IceSSL;
using namespace IceSSL::SecureTransport;

namespace
{

mutex staticMutex;

class RegExp
{
public:

    RegExp(const string&);
    ~RegExp();
    bool match(const string&);

private:

    regex_t _preg;
};
using RegExpPtr = shared_ptr<RegExp>;

RegExp::RegExp(const string& regexp)
{
    int err = regcomp(&_preg, regexp.c_str(), REG_EXTENDED | REG_NOSUB);
    if(err)
    {
        throw IceUtil::SyscallException(__FILE__, __LINE__, err);
    }
}

RegExp::~RegExp()
{
    regfree(&_preg);
}

bool
RegExp::match(const string& value)
{
    return regexec(&_preg, value.c_str(), 0, 0, 0) == 0;
}

struct CipherExpression
{
    bool negation;
    string cipher;
    RegExpPtr re;
};

class CiphersHelper
{
public:

    static void initialize();
    static SSLCipherSuite cipherForName(const string& name);
    static string cipherName(SSLCipherSuite cipher);
    static map<string, SSLCipherSuite> ciphers();

private:

    static map<string, SSLCipherSuite> _ciphers;
};

map<string, SSLCipherSuite> CiphersHelper::_ciphers;

//
// Initialize a dictionary with the names of ciphers
//
void
CiphersHelper::initialize()
{
    lock_guard sync(staticMutex);
    if(_ciphers.empty())
    {
        _ciphers["NULL_WITH_NULL_NULL"] = SSL_NULL_WITH_NULL_NULL;
        _ciphers["RSA_WITH_NULL_MD5"] = SSL_RSA_WITH_NULL_MD5;
        _ciphers["RSA_WITH_NULL_SHA"] = SSL_RSA_WITH_NULL_SHA;
        _ciphers["RSA_EXPORT_WITH_RC4_40_MD5"] = SSL_RSA_EXPORT_WITH_RC4_40_MD5;
        _ciphers["RSA_WITH_RC4_128_MD5"] = SSL_RSA_WITH_RC4_128_MD5;
        _ciphers["RSA_WITH_RC4_128_SHA"] = SSL_RSA_WITH_RC4_128_SHA;
        _ciphers["RSA_EXPORT_WITH_RC2_CBC_40_MD5"] = SSL_RSA_EXPORT_WITH_RC2_CBC_40_MD5;
        _ciphers["RSA_WITH_IDEA_CBC_SHA"] = SSL_RSA_WITH_IDEA_CBC_SHA;
        _ciphers["RSA_EXPORT_WITH_DES40_CBC_SHA"] = SSL_RSA_EXPORT_WITH_DES40_CBC_SHA;
        _ciphers["RSA_WITH_DES_CBC_SHA"] = SSL_RSA_WITH_DES_CBC_SHA;
        _ciphers["RSA_WITH_3DES_EDE_CBC_SHA"] = SSL_RSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["DH_DSS_EXPORT_WITH_DES40_CBC_SHA"] = SSL_DH_DSS_EXPORT_WITH_DES40_CBC_SHA;
        _ciphers["DH_DSS_WITH_DES_CBC_SHA"] = SSL_DH_DSS_WITH_DES_CBC_SHA;
        _ciphers["DH_DSS_WITH_3DES_EDE_CBC_SHA"] = SSL_DH_DSS_WITH_3DES_EDE_CBC_SHA;
        _ciphers["DH_RSA_EXPORT_WITH_DES40_CBC_SHA"] = SSL_DH_RSA_EXPORT_WITH_DES40_CBC_SHA;
        _ciphers["DH_RSA_WITH_DES_CBC_SHA"] = SSL_DH_RSA_WITH_DES_CBC_SHA;
        _ciphers["DH_RSA_WITH_3DES_EDE_CBC_SHA"] = SSL_DH_RSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["DHE_DSS_EXPORT_WITH_DES40_CBC_SHA"] = SSL_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA;
        _ciphers["DHE_DSS_WITH_DES_CBC_SHA"] = SSL_DHE_DSS_WITH_DES_CBC_SHA;
        _ciphers["DHE_DSS_WITH_3DES_EDE_CBC_SHA"] = SSL_DHE_DSS_WITH_3DES_EDE_CBC_SHA;
        _ciphers["DHE_RSA_EXPORT_WITH_DES40_CBC_SHA"] = SSL_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA;
        _ciphers["DHE_RSA_WITH_DES_CBC_SHA"] = SSL_DHE_RSA_WITH_DES_CBC_SHA;
        _ciphers["DHE_RSA_WITH_3DES_EDE_CBC_SHA"] = SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["DH_anon_EXPORT_WITH_RC4_40_MD5"] = SSL_DH_anon_EXPORT_WITH_RC4_40_MD5;
        _ciphers["DH_anon_WITH_RC4_128_MD5"] = SSL_DH_anon_WITH_RC4_128_MD5;
        _ciphers["DH_anon_EXPORT_WITH_DES40_CBC_SHA"] = SSL_DH_anon_EXPORT_WITH_DES40_CBC_SHA;
        _ciphers["DH_anon_WITH_DES_CBC_SHA"] = SSL_DH_anon_WITH_DES_CBC_SHA;
        _ciphers["DH_anon_WITH_3DES_EDE_CBC_SHA"] = SSL_DH_anon_WITH_3DES_EDE_CBC_SHA;
        _ciphers["FORTEZZA_DMS_WITH_NULL_SHA"] = SSL_FORTEZZA_DMS_WITH_NULL_SHA;
        _ciphers["FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA"] = SSL_FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA;

        //
        // TLS addenda using AES, per RFC 3268
        //
        _ciphers["RSA_WITH_AES_128_CBC_SHA"] = TLS_RSA_WITH_AES_128_CBC_SHA;
        _ciphers["DH_DSS_WITH_AES_128_CBC_SHA"] = TLS_DH_DSS_WITH_AES_128_CBC_SHA;
        _ciphers["DH_RSA_WITH_AES_128_CBC_SHA"] = TLS_DH_RSA_WITH_AES_128_CBC_SHA;
        _ciphers["DHE_DSS_WITH_AES_128_CBC_SHA"] = TLS_DHE_DSS_WITH_AES_128_CBC_SHA;
        _ciphers["DHE_RSA_WITH_AES_128_CBC_SHA"] = TLS_DHE_RSA_WITH_AES_128_CBC_SHA;
        _ciphers["DH_anon_WITH_AES_128_CBC_SHA"] = TLS_DH_anon_WITH_AES_128_CBC_SHA;
        _ciphers["RSA_WITH_AES_256_CBC_SHA"] = TLS_RSA_WITH_AES_256_CBC_SHA;
        _ciphers["DH_DSS_WITH_AES_256_CBC_SHA"] = TLS_DH_DSS_WITH_AES_256_CBC_SHA;
        _ciphers["DH_RSA_WITH_AES_256_CBC_SHA"] = TLS_DH_RSA_WITH_AES_256_CBC_SHA;
        _ciphers["DHE_DSS_WITH_AES_256_CBC_SHA"] = TLS_DHE_DSS_WITH_AES_256_CBC_SHA;
        _ciphers["DHE_RSA_WITH_AES_256_CBC_SHA"] = TLS_DHE_RSA_WITH_AES_256_CBC_SHA;
        _ciphers["DH_anon_WITH_AES_256_CBC_SHA"] = TLS_DH_anon_WITH_AES_256_CBC_SHA;

        //
        // ECDSA addenda, RFC 4492
        //
        _ciphers["ECDH_ECDSA_WITH_NULL_SHA"] = TLS_ECDH_ECDSA_WITH_NULL_SHA;
        _ciphers["ECDH_ECDSA_WITH_RC4_128_SHA"] = TLS_ECDH_ECDSA_WITH_RC4_128_SHA;
        _ciphers["ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA"] = TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["ECDH_ECDSA_WITH_AES_128_CBC_SHA"] = TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA;
        _ciphers["ECDH_ECDSA_WITH_AES_256_CBC_SHA"] = TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA;
        _ciphers["ECDHE_ECDSA_WITH_NULL_SHA"] = TLS_ECDHE_ECDSA_WITH_NULL_SHA;
        _ciphers["ECDHE_ECDSA_WITH_RC4_128_SHA"] = TLS_ECDHE_ECDSA_WITH_RC4_128_SHA;
        _ciphers["ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA"] = TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["ECDHE_ECDSA_WITH_AES_128_CBC_SHA"] = TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA;
        _ciphers["ECDHE_ECDSA_WITH_AES_256_CBC_SHA"] = TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA;
        _ciphers["ECDH_RSA_WITH_NULL_SHA"] = TLS_ECDH_RSA_WITH_NULL_SHA;
        _ciphers["ECDH_RSA_WITH_RC4_128_SHA"] = TLS_ECDH_RSA_WITH_RC4_128_SHA;
        _ciphers["ECDH_RSA_WITH_3DES_EDE_CBC_SHA"] = TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["ECDH_RSA_WITH_AES_128_CBC_SHA"] = TLS_ECDH_RSA_WITH_AES_128_CBC_SHA;
        _ciphers["ECDH_RSA_WITH_AES_256_CBC_SHA"] = TLS_ECDH_RSA_WITH_AES_256_CBC_SHA;
        _ciphers["ECDHE_RSA_WITH_NULL_SHA"] = TLS_ECDHE_RSA_WITH_NULL_SHA;
        _ciphers["ECDHE_RSA_WITH_RC4_128_SHA"] = TLS_ECDHE_RSA_WITH_RC4_128_SHA;
        _ciphers["ECDHE_RSA_WITH_3DES_EDE_CBC_SHA"] = TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["ECDHE_RSA_WITH_AES_128_CBC_SHA"] = TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA;
        _ciphers["ECDHE_RSA_WITH_AES_256_CBC_SHA"] = TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA;
        _ciphers["ECDH_anon_WITH_NULL_SHA"] = TLS_ECDH_anon_WITH_NULL_SHA;
        _ciphers["ECDH_anon_WITH_RC4_128_SHA"] = TLS_ECDH_anon_WITH_RC4_128_SHA;
        _ciphers["ECDH_anon_WITH_3DES_EDE_CBC_SHA"] = TLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA;
        _ciphers["ECDH_anon_WITH_AES_128_CBC_SHA"] = TLS_ECDH_anon_WITH_AES_128_CBC_SHA;
        _ciphers["ECDH_anon_WITH_AES_256_CBC_SHA"] = TLS_ECDH_anon_WITH_AES_256_CBC_SHA;

        //
        // TLS 1.2 addenda, RFC 5246
        //
        //_ciphers["NULL_WITH_NULL_NULL"] = TLS_NULL_WITH_NULL_NULL;

        //
        // Server provided RSA certificate for key exchange.
        //
        //_ciphers["RSA_WITH_NULL_MD5"] = TLS_RSA_WITH_NULL_MD5;
        //_ciphers["RSA_WITH_NULL_SHA"] = TLS_RSA_WITH_NULL_SHA;
        //_ciphers["RSA_WITH_RC4_128_MD5"] = TLS_RSA_WITH_RC4_128_MD5;
        //_ciphers["RSA_WITH_RC4_128_SHA"] = TLS_RSA_WITH_RC4_128_SHA;
        //_ciphers["RSA_WITH_3DES_EDE_CBC_SHA"] = TLS_RSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["RSA_WITH_NULL_SHA256"] = TLS_RSA_WITH_NULL_SHA256;
        _ciphers["RSA_WITH_AES_128_CBC_SHA256"] = TLS_RSA_WITH_AES_128_CBC_SHA256;
        _ciphers["RSA_WITH_AES_256_CBC_SHA256"] = TLS_RSA_WITH_AES_256_CBC_SHA256;

        //
        // Server-authenticated (and optionally client-authenticated) Diffie-Hellman.
        //
        //_ciphers["DH_DSS_WITH_3DES_EDE_CBC_SHA"] = TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA;
        //_ciphers["DH_RSA_WITH_3DES_EDE_CBC_SHA"] = TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA;
        //_ciphers["DHE_DSS_WITH_3DES_EDE_CBC_SHA"] = TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA;
        //_ciphers["DHE_RSA_WITH_3DES_EDE_CBC_SHA"] = TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["DH_DSS_WITH_AES_128_CBC_SHA256"] = TLS_DH_DSS_WITH_AES_128_CBC_SHA256;
        _ciphers["DH_RSA_WITH_AES_128_CBC_SHA256"] = TLS_DH_RSA_WITH_AES_128_CBC_SHA256;
        _ciphers["DHE_DSS_WITH_AES_128_CBC_SHA256"] = TLS_DHE_DSS_WITH_AES_128_CBC_SHA256;
        _ciphers["DHE_RSA_WITH_AES_128_CBC_SHA256"] = TLS_DHE_RSA_WITH_AES_128_CBC_SHA256;
        _ciphers["DH_DSS_WITH_AES_256_CBC_SHA256"] = TLS_DH_DSS_WITH_AES_256_CBC_SHA256;
        _ciphers["DH_RSA_WITH_AES_256_CBC_SHA256"] = TLS_DH_RSA_WITH_AES_256_CBC_SHA256;
        _ciphers["DHE_DSS_WITH_AES_256_CBC_SHA256"] = TLS_DHE_DSS_WITH_AES_256_CBC_SHA256;
        _ciphers["DHE_RSA_WITH_AES_256_CBC_SHA256"] = TLS_DHE_RSA_WITH_AES_256_CBC_SHA256;

        //
        // Completely anonymous Diffie-Hellman
        //
        //_ciphers["DH_anon_WITH_RC4_128_MD5"] = TLS_DH_anon_WITH_RC4_128_MD5;
        //_ciphers["DH_anon_WITH_3DES_EDE_CBC_SHA"] = TLS_DH_anon_WITH_3DES_EDE_CBC_SHA;
        _ciphers["DH_anon_WITH_AES_128_CBC_SHA256"] = TLS_DH_anon_WITH_AES_128_CBC_SHA256;
        _ciphers["DH_anon_WITH_AES_256_CBC_SHA256"] = TLS_DH_anon_WITH_AES_256_CBC_SHA256;

        //
        // Addendum from RFC 4279, TLS PSK
        //
        _ciphers["PSK_WITH_RC4_128_SHA"] = TLS_PSK_WITH_RC4_128_SHA;
        _ciphers["PSK_WITH_3DES_EDE_CBC_SHA"] = TLS_PSK_WITH_3DES_EDE_CBC_SHA;
        _ciphers["PSK_WITH_AES_128_CBC_SHA"] = TLS_PSK_WITH_AES_128_CBC_SHA;
        _ciphers["PSK_WITH_AES_256_CBC_SHA"] = TLS_PSK_WITH_AES_256_CBC_SHA;
        _ciphers["DHE_PSK_WITH_RC4_128_SHA"] = TLS_DHE_PSK_WITH_RC4_128_SHA;
        _ciphers["DHE_PSK_WITH_3DES_EDE_CBC_SHA"] = TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA;
        _ciphers["DHE_PSK_WITH_AES_128_CBC_SHA"] = TLS_DHE_PSK_WITH_AES_128_CBC_SHA;
        _ciphers["DHE_PSK_WITH_AES_256_CBC_SHA"] = TLS_DHE_PSK_WITH_AES_256_CBC_SHA;
        _ciphers["RSA_PSK_WITH_RC4_128_SHA"] = TLS_RSA_PSK_WITH_RC4_128_SHA;
        _ciphers["RSA_PSK_WITH_3DES_EDE_CBC_SHA"] = TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA;
        _ciphers["RSA_PSK_WITH_AES_128_CBC_SHA"] = TLS_RSA_PSK_WITH_AES_128_CBC_SHA;
        _ciphers["RSA_PSK_WITH_AES_256_CBC_SHA"] = TLS_RSA_PSK_WITH_AES_256_CBC_SHA;

        //
        // RFC 4785 - Pre-Shared Key (PSK) Ciphersuites with NULL Encryption
        //
        _ciphers["PSK_WITH_NULL_SHA"] = TLS_PSK_WITH_NULL_SHA;
        _ciphers["DHE_PSK_WITH_NULL_SHA"] = TLS_DHE_PSK_WITH_NULL_SHA;
        _ciphers["RSA_PSK_WITH_NULL_SHA"] = TLS_RSA_PSK_WITH_NULL_SHA;

        //
        // Addenda from rfc 5288 AES Galois Counter Mode (GCM) Cipher Suites for TLS.
        //
        _ciphers["RSA_WITH_AES_128_GCM_SHA256"] = TLS_RSA_WITH_AES_128_GCM_SHA256;
        _ciphers["RSA_WITH_AES_256_GCM_SHA384"] = TLS_RSA_WITH_AES_256_GCM_SHA384;
        _ciphers["DHE_RSA_WITH_AES_128_GCM_SHA256"] = TLS_DHE_RSA_WITH_AES_128_GCM_SHA256;
        _ciphers["DHE_RSA_WITH_AES_256_GCM_SHA384"] = TLS_DHE_RSA_WITH_AES_256_GCM_SHA384;
        _ciphers["DH_RSA_WITH_AES_128_GCM_SHA256"] = TLS_DH_RSA_WITH_AES_128_GCM_SHA256;
        _ciphers["DH_RSA_WITH_AES_256_GCM_SHA384"] = TLS_DH_RSA_WITH_AES_256_GCM_SHA384;
        _ciphers["DHE_DSS_WITH_AES_128_GCM_SHA256"] = TLS_DHE_DSS_WITH_AES_128_GCM_SHA256;
        _ciphers["DHE_DSS_WITH_AES_256_GCM_SHA384"] = TLS_DHE_DSS_WITH_AES_256_GCM_SHA384;
        _ciphers["DH_DSS_WITH_AES_128_GCM_SHA256"] = TLS_DH_DSS_WITH_AES_128_GCM_SHA256;
        _ciphers["DH_DSS_WITH_AES_256_GCM_SHA384"] = TLS_DH_DSS_WITH_AES_256_GCM_SHA384;
        _ciphers["DH_anon_WITH_AES_128_GCM_SHA256"] = TLS_DH_anon_WITH_AES_128_GCM_SHA256;
        _ciphers["DH_anon_WITH_AES_256_GCM_SHA384"] = TLS_DH_anon_WITH_AES_256_GCM_SHA384;

        //
        // RFC 5487 - PSK with SHA-256/384 and AES GCM
        //
        _ciphers["PSK_WITH_AES_128_GCM_SHA256"] = TLS_PSK_WITH_AES_128_GCM_SHA256;
        _ciphers["PSK_WITH_AES_256_GCM_SHA384"] = TLS_PSK_WITH_AES_256_GCM_SHA384;
        _ciphers["DHE_PSK_WITH_AES_128_GCM_SHA256"] = TLS_DHE_PSK_WITH_AES_128_GCM_SHA256;
        _ciphers["DHE_PSK_WITH_AES_256_GCM_SHA384"] = TLS_DHE_PSK_WITH_AES_256_GCM_SHA384;
        _ciphers["RSA_PSK_WITH_AES_128_GCM_SHA256"] = TLS_RSA_PSK_WITH_AES_128_GCM_SHA256;
        _ciphers["RSA_PSK_WITH_AES_256_GCM_SHA384"] = TLS_RSA_PSK_WITH_AES_256_GCM_SHA384;

        _ciphers["PSK_WITH_AES_128_CBC_SHA256"] = TLS_PSK_WITH_AES_128_CBC_SHA256;
        _ciphers["PSK_WITH_AES_256_CBC_SHA384"] = TLS_PSK_WITH_AES_256_CBC_SHA384;
        _ciphers["PSK_WITH_NULL_SHA256"] = TLS_PSK_WITH_NULL_SHA256;
        _ciphers["PSK_WITH_NULL_SHA384"] = TLS_PSK_WITH_NULL_SHA384;

        _ciphers["DHE_PSK_WITH_AES_128_CBC_SHA256"] = TLS_DHE_PSK_WITH_AES_128_CBC_SHA256;
        _ciphers["DHE_PSK_WITH_AES_256_CBC_SHA384"] = TLS_DHE_PSK_WITH_AES_256_CBC_SHA384;
        _ciphers["DHE_PSK_WITH_NULL_SHA256"] = TLS_DHE_PSK_WITH_NULL_SHA256;
        _ciphers["DHE_PSK_WITH_NULL_SHA384"] = TLS_DHE_PSK_WITH_NULL_SHA384;

        _ciphers["RSA_PSK_WITH_AES_128_CBC_SHA256"] = TLS_RSA_PSK_WITH_AES_128_CBC_SHA256;
        _ciphers["RSA_PSK_WITH_AES_256_CBC_SHA384"] = TLS_RSA_PSK_WITH_AES_256_CBC_SHA384;
        _ciphers["RSA_PSK_WITH_NULL_SHA256"] = TLS_RSA_PSK_WITH_NULL_SHA256;
        _ciphers["RSA_PSK_WITH_NULL_SHA384"] = TLS_RSA_PSK_WITH_NULL_SHA384;

        //
        // Addenda from rfc 5289  Elliptic Curve Cipher Suites with HMAC SHA-256/384.
        //
        _ciphers["ECDHE_ECDSA_WITH_AES_128_CBC_SHA256"] = TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256;
        _ciphers["ECDHE_ECDSA_WITH_AES_256_CBC_SHA384"] = TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384;
        _ciphers["ECDH_ECDSA_WITH_AES_128_CBC_SHA256"] = TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256;
        _ciphers["ECDH_ECDSA_WITH_AES_256_CBC_SHA384"] = TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384;
        _ciphers["ECDHE_RSA_WITH_AES_128_CBC_SHA256"] = TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256;
        _ciphers["ECDHE_RSA_WITH_AES_256_CBC_SHA384"] = TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384;
        _ciphers["ECDH_RSA_WITH_AES_128_CBC_SHA256"] = TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256;
        _ciphers["ECDH_RSA_WITH_AES_256_CBC_SHA384"] = TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384;

        //
        // Addenda from rfc 5289  Elliptic Curve Cipher Suites with SHA-256/384 and AES Galois Counter Mode (GCM)
        //
        _ciphers["ECDHE_ECDSA_WITH_AES_128_GCM_SHA256"] = TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256;
        _ciphers["ECDHE_ECDSA_WITH_AES_256_GCM_SHA384"] = TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384;
        _ciphers["ECDH_ECDSA_WITH_AES_128_GCM_SHA256"] = TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256;
        _ciphers["ECDH_ECDSA_WITH_AES_256_GCM_SHA384"] = TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384;
        _ciphers["ECDHE_RSA_WITH_AES_128_GCM_SHA256"] = TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256;
        _ciphers["ECDHE_RSA_WITH_AES_256_GCM_SHA384"] = TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384;
        _ciphers["ECDH_RSA_WITH_AES_128_GCM_SHA256"] = TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256;
        _ciphers["ECDH_RSA_WITH_AES_256_GCM_SHA384"] = TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384;

        //
        // RFC 5746 - Secure Renegotiation
        //
        _ciphers["EMPTY_RENEGOTIATION_INFO_SCSV"] = TLS_EMPTY_RENEGOTIATION_INFO_SCSV;

        //
        // Tags for SSL 2 cipher kinds that are not specified for SSL 3.
        //
        _ciphers["RSA_WITH_RC2_CBC_MD5"] = SSL_RSA_WITH_RC2_CBC_MD5;
        _ciphers["RSA_WITH_IDEA_CBC_MD5"] = SSL_RSA_WITH_IDEA_CBC_MD5;
        _ciphers["RSA_WITH_DES_CBC_MD5"] = SSL_RSA_WITH_DES_CBC_MD5;
        _ciphers["RSA_WITH_3DES_EDE_CBC_MD5"] = SSL_RSA_WITH_3DES_EDE_CBC_MD5;
        _ciphers["NO_SUCH_CIPHERSUITE"] = SSL_NO_SUCH_CIPHERSUITE;

        //
        // TLS 1.3 standard cipher suites
        //
        _ciphers["TLS_AES_128_GCM_SHA256"] = TLS_AES_128_GCM_SHA256;
        _ciphers["TLS_AES_256_GCM_SHA384"] = TLS_AES_256_GCM_SHA384;
        _ciphers["TLS_CHACHA20_POLY1305_SHA256"] = TLS_CHACHA20_POLY1305_SHA256;
        _ciphers["TLS_AES_128_CCM_SHA256"] = TLS_AES_128_CCM_SHA256;
        _ciphers["TLS_AES_128_CCM_8_SHA256"] = TLS_AES_128_CCM_8_SHA256;

    }
}

SSLCipherSuite
CiphersHelper::cipherForName(const string& name)
{
    map<string, SSLCipherSuite>::const_iterator i = _ciphers.find(name);
    if(i == _ciphers.end() || i->second == SSL_NO_SUCH_CIPHERSUITE)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: no such cipher " + name);
    }
    return i->second;
}

//
// Retrive the name of a cipher, SSLCipherSuite inlude duplicated values for TLS/SSL
// protocol ciphers, for example SSL_RSA_WITH_RC4_128_MD5/TLS_RSA_WITH_RC4_128_MD5
// are represeted by the same SSLCipherSuite value, the names return by this method
// doesn't include a protocol prefix.
//
string
CiphersHelper::cipherName(SSLCipherSuite cipher)
{
    switch(cipher)
    {
        case SSL_NULL_WITH_NULL_NULL:
            return "NULL_WITH_NULL_NULL";
        case SSL_RSA_WITH_NULL_MD5:
            return "RSA_WITH_NULL_MD5";
        case SSL_RSA_WITH_NULL_SHA:
            return "RSA_WITH_NULL_SHA";
        case SSL_RSA_EXPORT_WITH_RC4_40_MD5:
            return "RSA_EXPORT_WITH_RC4_40_MD5";
        case SSL_RSA_WITH_RC4_128_MD5:
            return "RSA_WITH_RC4_128_MD5";
        case SSL_RSA_WITH_RC4_128_SHA:
            return "RSA_WITH_RC4_128_SHA";
        case SSL_RSA_EXPORT_WITH_RC2_CBC_40_MD5:
            return "RSA_EXPORT_WITH_RC2_CBC_40_MD5";
        case SSL_RSA_WITH_IDEA_CBC_SHA:
            return "RSA_WITH_IDEA_CBC_SHA";
        case SSL_RSA_EXPORT_WITH_DES40_CBC_SHA:
            return "RSA_EXPORT_WITH_DES40_CBC_SHA";
        case SSL_RSA_WITH_DES_CBC_SHA:
            return "RSA_WITH_DES_CBC_SHA";
        case SSL_RSA_WITH_3DES_EDE_CBC_SHA:
            return "RSA_WITH_3DES_EDE_CBC_SHA";
        case SSL_DH_DSS_EXPORT_WITH_DES40_CBC_SHA:
            return "DH_DSS_EXPORT_WITH_DES40_CBC_SHA";
        case SSL_DH_DSS_WITH_DES_CBC_SHA:
            return "DH_DSS_WITH_DES_CBC_SHA";
        case SSL_DH_DSS_WITH_3DES_EDE_CBC_SHA:
            return "DH_DSS_WITH_3DES_EDE_CBC_SHA";
        case SSL_DH_RSA_EXPORT_WITH_DES40_CBC_SHA:
            return "DH_RSA_EXPORT_WITH_DES40_CBC_SHA";
        case SSL_DH_RSA_WITH_DES_CBC_SHA:
            return "DH_RSA_WITH_DES_CBC_SHA";
        case SSL_DH_RSA_WITH_3DES_EDE_CBC_SHA:
            return "DH_RSA_WITH_3DES_EDE_CBC_SHA";
        case SSL_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA:
            return "DHE_DSS_EXPORT_WITH_DES40_CBC_SHA";
        case SSL_DHE_DSS_WITH_DES_CBC_SHA:
            return "DHE_DSS_WITH_DES_CBC_SHA";
        case SSL_DHE_DSS_WITH_3DES_EDE_CBC_SHA:
            return "DHE_DSS_WITH_3DES_EDE_CBC_SHA";
        case SSL_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA:
            return "DHE_RSA_EXPORT_WITH_DES40_CBC_SHA";
        case SSL_DHE_RSA_WITH_DES_CBC_SHA:
            return "DHE_RSA_WITH_DES_CBC_SHA";
        case SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA:
            return "DHE_RSA_WITH_3DES_EDE_CBC_SHA";
        case SSL_DH_anon_EXPORT_WITH_RC4_40_MD5:
            return "DH_anon_EXPORT_WITH_RC4_40_MD5";
        case SSL_DH_anon_WITH_RC4_128_MD5:
            return "DH_anon_WITH_RC4_128_MD5";
        case SSL_DH_anon_EXPORT_WITH_DES40_CBC_SHA:
            return "DH_anon_EXPORT_WITH_DES40_CBC_SHA";
        case SSL_DH_anon_WITH_DES_CBC_SHA:
            return "DH_anon_WITH_DES_CBC_SHA";
        case SSL_DH_anon_WITH_3DES_EDE_CBC_SHA:
            return "DH_anon_WITH_3DES_EDE_CBC_SHA";
        case SSL_FORTEZZA_DMS_WITH_NULL_SHA:
            return "FORTEZZA_DMS_WITH_NULL_SHA";
        case SSL_FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA:
            return "FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA";

        //
        // TLS addenda using AES, per RFC 3268
        //
        case TLS_RSA_WITH_AES_128_CBC_SHA:
            return "RSA_WITH_AES_128_CBC_SHA";
        case TLS_DH_DSS_WITH_AES_128_CBC_SHA:
            return "DH_DSS_WITH_AES_128_CBC_SHA";
        case TLS_DH_RSA_WITH_AES_128_CBC_SHA:
            return "DH_RSA_WITH_AES_128_CBC_SHA";
        case TLS_DHE_DSS_WITH_AES_128_CBC_SHA:
            return "DHE_DSS_WITH_AES_128_CBC_SHA";
        case TLS_DHE_RSA_WITH_AES_128_CBC_SHA:
            return "DHE_RSA_WITH_AES_128_CBC_SHA";
        case TLS_DH_anon_WITH_AES_128_CBC_SHA:
            return "DH_anon_WITH_AES_128_CBC_SHA";
        case TLS_RSA_WITH_AES_256_CBC_SHA:
            return "RSA_WITH_AES_256_CBC_SHA";
        case TLS_DH_DSS_WITH_AES_256_CBC_SHA:
            return "DH_DSS_WITH_AES_256_CBC_SHA";
        case TLS_DH_RSA_WITH_AES_256_CBC_SHA:
            return "DH_RSA_WITH_AES_256_CBC_SHA";
        case TLS_DHE_DSS_WITH_AES_256_CBC_SHA:
            return "DHE_DSS_WITH_AES_256_CBC_SHA";
        case TLS_DHE_RSA_WITH_AES_256_CBC_SHA:
            return "DHE_RSA_WITH_AES_256_CBC_SHA";
        case TLS_DH_anon_WITH_AES_256_CBC_SHA:
            return "DH_anon_WITH_AES_256_CBC_SHA";

        //
        // ECDSA addenda, RFC 4492
        //
        case TLS_ECDH_ECDSA_WITH_NULL_SHA:
            return "ECDH_ECDSA_WITH_NULL_SHA";
        case TLS_ECDH_ECDSA_WITH_RC4_128_SHA:
            return "ECDH_ECDSA_WITH_RC4_128_SHA";
        case TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA:
            return "ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA";
        case TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA:
            return "ECDH_ECDSA_WITH_AES_128_CBC_SHA";
        case TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA:
            return "ECDH_ECDSA_WITH_AES_256_CBC_SHA";
        case TLS_ECDHE_ECDSA_WITH_NULL_SHA:
            return "ECDHE_ECDSA_WITH_NULL_SHA";
        case TLS_ECDHE_ECDSA_WITH_RC4_128_SHA:
            return "ECDHE_ECDSA_WITH_RC4_128_SHA";
        case TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA:
            return "ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA";
        case TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA:
            return "ECDHE_ECDSA_WITH_AES_128_CBC_SHA";
        case TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA:
            return "ECDHE_ECDSA_WITH_AES_256_CBC_SHA";
        case TLS_ECDH_RSA_WITH_NULL_SHA:
            return "ECDH_RSA_WITH_NULL_SHA";
        case TLS_ECDH_RSA_WITH_RC4_128_SHA:
            return "ECDH_RSA_WITH_RC4_128_SHA";
        case TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA:
            return "ECDH_RSA_WITH_3DES_EDE_CBC_SHA";
        case TLS_ECDH_RSA_WITH_AES_128_CBC_SHA:
            return "ECDH_RSA_WITH_AES_128_CBC_SHA";
        case TLS_ECDH_RSA_WITH_AES_256_CBC_SHA:
            return "ECDH_RSA_WITH_AES_256_CBC_SHA";
        case TLS_ECDHE_RSA_WITH_NULL_SHA:
            return "ECDHE_RSA_WITH_NULL_SHA";
        case TLS_ECDHE_RSA_WITH_RC4_128_SHA:
            return "ECDHE_RSA_WITH_RC4_128_SHA";
        case TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA:
            return "ECDHE_RSA_WITH_3DES_EDE_CBC_SHA";
        case TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA:
            return "ECDHE_RSA_WITH_AES_128_CBC_SHA";
        case TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA:
            return "ECDHE_RSA_WITH_AES_256_CBC_SHA";
        case TLS_ECDH_anon_WITH_NULL_SHA:
            return "ECDH_anon_WITH_NULL_SHA";
        case TLS_ECDH_anon_WITH_RC4_128_SHA:
            return "ECDH_anon_WITH_RC4_128_SHA";
        case TLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA:
            return "ECDH_anon_WITH_3DES_EDE_CBC_SHA";
        case TLS_ECDH_anon_WITH_AES_128_CBC_SHA:
            return "ECDH_anon_WITH_AES_128_CBC_SHA";
        case TLS_ECDH_anon_WITH_AES_256_CBC_SHA:
            return "ECDH_anon_WITH_AES_256_CBC_SHA";

        //
        // TLS 1.2 addenda, RFC 5246
        //
        //case TLS_NULL_WITH_NULL_NULL:
        //    return "NULL_WITH_NULL_NULL";

        //
        // Server provided RSA certificate for key exchange.
        //
        //case TLS_RSA_WITH_NULL_MD5:
        //    return "RSA_WITH_NULL_MD5";
        //case TLS_RSA_WITH_NULL_SHA:
        //    return "RSA_WITH_NULL_SHA";
        //case TLS_RSA_WITH_RC4_128_MD5:
        //    return "RSA_WITH_RC4_128_MD5";
        //case TLS_RSA_WITH_RC4_128_SHA:
        //    return "RSA_WITH_RC4_128_SHA";
        //case TLS_RSA_WITH_3DES_EDE_CBC_SHA:
        //    return "RSA_WITH_3DES_EDE_CBC_SHA";
        case TLS_RSA_WITH_NULL_SHA256:
            return "RSA_WITH_NULL_SHA256";
        case TLS_RSA_WITH_AES_128_CBC_SHA256:
            return "RSA_WITH_AES_128_CBC_SHA256";
        case TLS_RSA_WITH_AES_256_CBC_SHA256:
            return "RSA_WITH_AES_256_CBC_SHA256";

        //
        // Server-authenticated (and optionally client-authenticated) Diffie-Hellman.
        //
        //case TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA:
        //    return "DH_DSS_WITH_3DES_EDE_CBC_SHA";
        //case TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA:
        //    return "DH_RSA_WITH_3DES_EDE_CBC_SHA";
        //case TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA:
        //    return "DHE_DSS_WITH_3DES_EDE_CBC_SHA";
        //case TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA:
        //    return "DHE_RSA_WITH_3DES_EDE_CBC_SHA";
        case TLS_DH_DSS_WITH_AES_128_CBC_SHA256:
            return "DH_DSS_WITH_AES_128_CBC_SHA256";
        case TLS_DH_RSA_WITH_AES_128_CBC_SHA256:
            return "DH_RSA_WITH_AES_128_CBC_SHA256";
        case TLS_DHE_DSS_WITH_AES_128_CBC_SHA256:
            return "DHE_DSS_WITH_AES_128_CBC_SHA256";
        case TLS_DHE_RSA_WITH_AES_128_CBC_SHA256:
            return "DHE_RSA_WITH_AES_128_CBC_SHA256";
        case TLS_DH_DSS_WITH_AES_256_CBC_SHA256:
            return "DH_DSS_WITH_AES_256_CBC_SHA256";
        case TLS_DH_RSA_WITH_AES_256_CBC_SHA256:
            return "DH_RSA_WITH_AES_256_CBC_SHA256";
        case TLS_DHE_DSS_WITH_AES_256_CBC_SHA256:
            return "DHE_DSS_WITH_AES_256_CBC_SHA256";
        case TLS_DHE_RSA_WITH_AES_256_CBC_SHA256:
            return "DHE_RSA_WITH_AES_256_CBC_SHA256";

        //
        // Completely anonymous Diffie-Hellman
        //
        //case TLS_DH_anon_WITH_RC4_128_MD5:
        //    return "DH_anon_WITH_RC4_128_MD5";
        //case TLS_DH_anon_WITH_3DES_EDE_CBC_SHA:
        //    return "DH_anon_WITH_3DES_EDE_CBC_SHA";
        case TLS_DH_anon_WITH_AES_128_CBC_SHA256:
            return "DH_anon_WITH_AES_128_CBC_SHA256";
        case TLS_DH_anon_WITH_AES_256_CBC_SHA256:
            return "DH_anon_WITH_AES_256_CBC_SHA256";

        //
        // Addendum from RFC 4279, TLS PSK
        //
        case TLS_PSK_WITH_RC4_128_SHA:
            return "PSK_WITH_RC4_128_SHA";
        case TLS_PSK_WITH_3DES_EDE_CBC_SHA:
            return "PSK_WITH_3DES_EDE_CBC_SHA";
        case TLS_PSK_WITH_AES_128_CBC_SHA:
            return "PSK_WITH_AES_128_CBC_SHA";
        case TLS_PSK_WITH_AES_256_CBC_SHA:
            return "PSK_WITH_AES_256_CBC_SHA";
        case TLS_DHE_PSK_WITH_RC4_128_SHA:
            return "DHE_PSK_WITH_RC4_128_SHA";
        case TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA:
            return "DHE_PSK_WITH_3DES_EDE_CBC_SHA";
        case TLS_DHE_PSK_WITH_AES_128_CBC_SHA:
            return "DHE_PSK_WITH_AES_128_CBC_SHA";
        case TLS_DHE_PSK_WITH_AES_256_CBC_SHA:
            return "DHE_PSK_WITH_AES_256_CBC_SHA";
        case TLS_RSA_PSK_WITH_RC4_128_SHA:
            return "RSA_PSK_WITH_RC4_128_SHA";
        case TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA:
            return "RSA_PSK_WITH_3DES_EDE_CBC_SHA";
        case TLS_RSA_PSK_WITH_AES_128_CBC_SHA:
            return "RSA_PSK_WITH_AES_128_CBC_SHA";
        case TLS_RSA_PSK_WITH_AES_256_CBC_SHA:
            return "RSA_PSK_WITH_AES_256_CBC_SHA";

        //
        // RFC 4785 - Pre-Shared Key (PSK) Ciphersuites with NULL Encryption
        //
        case TLS_PSK_WITH_NULL_SHA:
            return "PSK_WITH_NULL_SHA";
        case TLS_DHE_PSK_WITH_NULL_SHA:
            return "DHE_PSK_WITH_NULL_SHA";
        case TLS_RSA_PSK_WITH_NULL_SHA:
            return "RSA_PSK_WITH_NULL_SHA";

        //
        // Addenda from rfc 5288 AES Galois Counter Mode (GCM) Cipher Suites for TLS.
        //
        case TLS_RSA_WITH_AES_128_GCM_SHA256:
            return "RSA_WITH_AES_128_GCM_SHA256";
        case TLS_RSA_WITH_AES_256_GCM_SHA384:
            return "RSA_WITH_AES_256_GCM_SHA384";
        case TLS_DHE_RSA_WITH_AES_128_GCM_SHA256:
            return "DHE_RSA_WITH_AES_128_GCM_SHA256";
        case TLS_DHE_RSA_WITH_AES_256_GCM_SHA384:
            return "DHE_RSA_WITH_AES_256_GCM_SHA384";
        case TLS_DH_RSA_WITH_AES_128_GCM_SHA256:
            return "DH_RSA_WITH_AES_128_GCM_SHA256";
        case TLS_DH_RSA_WITH_AES_256_GCM_SHA384:
            return "DH_RSA_WITH_AES_256_GCM_SHA384";
        case TLS_DHE_DSS_WITH_AES_128_GCM_SHA256:
            return "DHE_DSS_WITH_AES_128_GCM_SHA256";
        case TLS_DHE_DSS_WITH_AES_256_GCM_SHA384:
            return "DHE_DSS_WITH_AES_256_GCM_SHA384";
        case TLS_DH_DSS_WITH_AES_128_GCM_SHA256:
            return "DH_DSS_WITH_AES_128_GCM_SHA256";
        case TLS_DH_DSS_WITH_AES_256_GCM_SHA384:
            return "DH_DSS_WITH_AES_256_GCM_SHA384";
        case TLS_DH_anon_WITH_AES_128_GCM_SHA256:
            return "DH_anon_WITH_AES_128_GCM_SHA256";
        case TLS_DH_anon_WITH_AES_256_GCM_SHA384:
            return "DH_anon_WITH_AES_256_GCM_SHA384";

        //
        // RFC 5487 - PSK with SHA-256/384 and AES GCM
        //
        case TLS_PSK_WITH_AES_128_GCM_SHA256:
            return "PSK_WITH_AES_128_GCM_SHA256";
        case TLS_PSK_WITH_AES_256_GCM_SHA384:
            return "PSK_WITH_AES_256_GCM_SHA384";
        case TLS_DHE_PSK_WITH_AES_128_GCM_SHA256:
            return "DHE_PSK_WITH_AES_128_GCM_SHA256";
        case TLS_DHE_PSK_WITH_AES_256_GCM_SHA384:
            return "DHE_PSK_WITH_AES_256_GCM_SHA384";
        case TLS_RSA_PSK_WITH_AES_128_GCM_SHA256:
            return "RSA_PSK_WITH_AES_128_GCM_SHA256";
        case TLS_RSA_PSK_WITH_AES_256_GCM_SHA384:
            return "RSA_PSK_WITH_AES_256_GCM_SHA384";

        case TLS_PSK_WITH_AES_128_CBC_SHA256:
            return "PSK_WITH_AES_128_CBC_SHA256";
        case TLS_PSK_WITH_AES_256_CBC_SHA384:
            return "PSK_WITH_AES_256_CBC_SHA384";
        case TLS_PSK_WITH_NULL_SHA256:
            return "WITH_NULL_SHA256";
        case TLS_PSK_WITH_NULL_SHA384:
            return "PSK_WITH_NULL_SHA384";

        case TLS_DHE_PSK_WITH_AES_128_CBC_SHA256:
            return "DHE_PSK_WITH_AES_128_CBC_SHA256";
        case TLS_DHE_PSK_WITH_AES_256_CBC_SHA384:
            return "DHE_PSK_WITH_AES_256_CBC_SHA384";
        case TLS_DHE_PSK_WITH_NULL_SHA256:
            return "DHE_PSK_WITH_NULL_SHA256";
        case TLS_DHE_PSK_WITH_NULL_SHA384:
            return "DHE_PSK_WITH_NULL_SHA384";

        case TLS_RSA_PSK_WITH_AES_128_CBC_SHA256:
            return "RSA_PSK_WITH_AES_128_CBC_SHA256";
        case TLS_RSA_PSK_WITH_AES_256_CBC_SHA384:
            return "RSA_PSK_WITH_AES_256_CBC_SHA384";
        case TLS_RSA_PSK_WITH_NULL_SHA256:
            return "RSA_PSK_WITH_NULL_SHA256";
        case TLS_RSA_PSK_WITH_NULL_SHA384:
            return "RSA_PSK_WITH_NULL_SHA384";

        //
        // Addenda from rfc 5289  Elliptic Curve Cipher Suites with HMAC SHA-256/384.
        //
        case TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256:
            return "ECDHE_ECDSA_WITH_AES_128_CBC_SHA256";
        case TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384:
            return "ECDHE_ECDSA_WITH_AES_256_CBC_SHA384";
        case TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256:
            return "ECDH_ECDSA_WITH_AES_128_CBC_SHA256";
        case TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384:
            return "ECDH_ECDSA_WITH_AES_256_CBC_SHA384";
        case TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256:
            return "ECDHE_RSA_WITH_AES_128_CBC_SHA256";
        case TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384:
            return "ECDHE_RSA_WITH_AES_256_CBC_SHA384";
        case TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256:
            return "ECDH_RSA_WITH_AES_128_CBC_SHA256";
        case TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384:
            return "ECDH_RSA_WITH_AES_256_CBC_SHA384";

        //
        // Addenda from rfc 5289  Elliptic Curve Cipher Suites with SHA-256/384 and AES Galois Counter Mode (GCM)
        //
        case TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256:
            return "ECDHE_ECDSA_WITH_AES_128_GCM_SHA256";
        case TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384:
            return "ECDHE_ECDSA_WITH_AES_256_GCM_SHA384";
        case TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256:
            return "ECDH_ECDSA_WITH_AES_128_GCM_SHA256";
        case TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384:
            return "ECDH_ECDSA_WITH_AES_256_GCM_SHA384";
        case TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256:
            return "ECDHE_RSA_WITH_AES_128_GCM_SHA256";
        case TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384:
            return "ECDHE_RSA_WITH_AES_256_GCM_SHA384";
        case TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256:
            return "ECDH_RSA_WITH_AES_128_GCM_SHA256";
        case TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384:
            return "ECDH_RSA_WITH_AES_256_GCM_SHA384";

        //
        // RFC 5746 - Secure Renegotiation
        //
        case TLS_EMPTY_RENEGOTIATION_INFO_SCSV:
            return "EMPTY_RENEGOTIATION_INFO_SCSV";

        //
        // Tags for SSL 2 cipher kinds that are not specified for SSL 3.
        //
        case SSL_RSA_WITH_RC2_CBC_MD5:
            return "RSA_WITH_RC2_CBC_MD5";
        case SSL_RSA_WITH_IDEA_CBC_MD5:
            return "RSA_WITH_IDEA_CBC_MD5";
        case SSL_RSA_WITH_DES_CBC_MD5:
            return "RSA_WITH_DES_CBC_MD5";
        case SSL_RSA_WITH_3DES_EDE_CBC_MD5:
            return "RSA_WITH_3DES_EDE_CBC_MD5";

        //
        //TLS 1.3 standard cipher suites
        //
        case TLS_AES_128_GCM_SHA256:
            return "TLS_AES_128_GCM_SHA256";
        case TLS_AES_256_GCM_SHA384:
            return "TLS_AES_256_GCM_SHA384";
        case TLS_CHACHA20_POLY1305_SHA256:
            return "TLS_CHACHA20_POLY1305_SHA256";
        case TLS_AES_128_CCM_SHA256:
            return "TLS_AES_128_CCM_SHA256";
        case TLS_AES_128_CCM_8_SHA256:
            return "TLS_AES_128_CCM_8_SHA256";

        default:
            return "";
    }
}

map<string, SSLCipherSuite>
CiphersHelper::ciphers()
{
    return _ciphers;
}

SSLProtocol
parseProtocol(const string& p)
{
    const string prot = IceUtilInternal::toUpper(p);
    if(prot == "SSL3" || prot == "SSLV3")
    {
        return kSSLProtocol3;
    }
    else if(prot == "TLS" || prot == "TLS1" || prot == "TLSV1" || prot == "TLS1_0" || prot == "TLSV1_0")
    {
        return kTLSProtocol1;
    }
    else if(prot == "TLS1_1" || prot == "TLSV1_1")
    {
        return kTLSProtocol11;
    }
    else if(prot == "TLS1_2" || prot == "TLSV1_2")
    {
        return kTLSProtocol12;
    }
    else if(prot == "TLS1_3" || prot == "TLSV1_3")
    {
        return kTLSProtocol13;
    }
    else
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: unrecognized protocol `" + p + "'");
    }
}

}

IceSSL::SecureTransport::SSLEngine::SSLEngine(const Ice::CommunicatorPtr& communicator) :
    IceSSL::SSLEngine(communicator),
    _certificateAuthorities(0),
    _chain(0),
    _protocolVersionMax(kSSLProtocolUnknown),
    _protocolVersionMin(kSSLProtocolUnknown)
{
}

//
// Setup the engine.
//
void
IceSSL::SecureTransport::SSLEngine::initialize()
{
    lock_guard lock(_mutex);
    if(_initialized)
    {
        return;
    }

    IceSSL::SSLEngine::initialize();

    const PropertiesPtr properties = communicator()->getProperties();

    //
    // Check for a default directory. We look in this directory for
    // files mentioned in the configuration.
    //
    const string defaultDir = properties->getProperty("IceSSL.DefaultDir");

    //
    // Load the CA certificates used to authenticate peers into
    // _certificateAuthorities array.
    //
    try
    {
        string caFile = properties->getProperty("IceSSL.CAs");
        if(caFile.empty())
        {
            caFile = properties->getProperty("IceSSL.CertAuthFile");
        }
        if(!caFile.empty())
        {
            string resolved;
            if(!checkPath(caFile, defaultDir, false, resolved))
            {
                throw PluginInitializationException(__FILE__, __LINE__,
                                                    "IceSSL: CA certificate file not found:\n" + caFile);
            }
            _certificateAuthorities.reset(loadCACertificates(resolved));
        }
        else if(properties->getPropertyAsInt("IceSSL.UsePlatformCAs") <= 0)
        {
            // Setup an empty list of Root CAs to not use the system root CAs.
            _certificateAuthorities.reset(CFArrayCreate(0, 0, 0, 0));
        }
    }
    catch(const CertificateReadException& ce)
    {
        throw PluginInitializationException(__FILE__, __LINE__, ce.reason);
    }

    const string password = properties->getProperty("IceSSL.Password");
    const int passwordRetryMax = properties->getPropertyAsIntWithDefault("IceSSL.PasswordRetryMax", 3);
    PasswordPromptPtr passwordPrompt = getPasswordPrompt();

    string certFile = properties->getProperty("IceSSL.CertFile");
    string findCert = properties->getProperty("IceSSL.FindCert");
    string keychain = properties->getProperty("IceSSL.Keychain");
    string keychainPassword = properties->getProperty("IceSSL.KeychainPassword");

    if(!certFile.empty())
    {
        vector<string> files;
        if(!IceUtilInternal::splitString(certFile, IceUtilInternal::pathsep, files) || files.size() > 2)
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: invalid value for IceSSL.CertFile:\n" + certFile);
        }
        vector<string> keyFiles;
        {
            string keyFile = properties->getProperty("IceSSL.KeyFile");
            if(!keyFile.empty())
            {
                if(!IceUtilInternal::splitString(keyFile, IceUtilInternal::pathsep, keyFiles) || keyFiles.size() > 2)
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                                        "IceSSL: invalid value for IceSSL.KeyFile:\n" + keyFile);
                }
                if(files.size() != keyFiles.size())
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                                        "IceSSL: IceSSL.KeyFile does not agree with IceSSL.CertFile");
                }
            }
        }

        for(size_t i = 0; i < files.size(); ++i)
        {
            string file = files[i];
            string keyFile = keyFiles.empty() ? "" : keyFiles[i];
            string resolved;

            if(!checkPath(file, defaultDir, false, resolved))
            {
                throw PluginInitializationException(__FILE__, __LINE__,
                                                    "IceSSL: certificate file not found:\n" + file);
            }
            file = resolved;

            if(!keyFile.empty())
            {
                if(!checkPath(keyFile, defaultDir, false, resolved))
                {
                    throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: key file not found:\n" + keyFile);
                }
                keyFile = resolved;
            }

            try
            {
                _chain.reset(loadCertificateChain(file, keyFile, keychain, keychainPassword, password, passwordPrompt,
                                                  passwordRetryMax));
                break;
            }
            catch(const CertificateReadException& ce)
            {
                //
                // If this is the last certificate rethrow the exception as PluginInitializationException,
                // otherwise try the next certificate.
                //
                if(i == files.size() - 1)
                {
                    throw PluginInitializationException(__FILE__, __LINE__, ce.reason);
                }
            }
        }
    }
    else if(!findCert.empty())
    {
        _chain.reset(findCertificateChain(keychain, keychainPassword, findCert));
    }

    //
    // DiffieHellmanParams in DER format.
    //
#if defined(ICE_USE_SECURE_TRANSPORT_MACOS)
    string dhFile = properties->getProperty("IceSSL.DHParams");
    if(!dhFile.empty())
    {
        string resolved;
        if(!checkPath(dhFile, defaultDir, false, resolved))
        {
            throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: DH params file not found:\n" + dhFile);
        }

        readFile(resolved, _dhParams);
    }
#endif

    //
    // Establish the cipher list.
    //
    const string ciphers = properties->getProperty("IceSSL.Ciphers");
    CiphersHelper::initialize();

    if(!ciphers.empty())
    {
        parseCiphers(ciphers);
    }

    if(securityTraceLevel() >= 1)
    {
        ostringstream os;
        os << "enabling SSL ciphersuites:";

        if(_ciphers.empty())
        {
            map<string, SSLCipherSuite> enabled = CiphersHelper::ciphers();
            for(map<string, SSLCipherSuite>::const_iterator i = enabled.begin(); i != enabled.end(); ++i)
            {
                os << "\n " << i->first;
            }
        }
        else
        {
            for(vector<SSLCipherSuite>::const_iterator i = _ciphers.begin(); i != _ciphers.end(); ++i)
            {
                os << "\n " << getCipherName(*i);
            }
        }
        getLogger()->trace(securityTraceCategory(), os.str());
    }

    //
    // Parse protocols
    //
    const string protocolVersionMax = properties->getProperty("IceSSL.ProtocolVersionMax");
    if(!protocolVersionMax.empty())
    {
        _protocolVersionMax = parseProtocol(protocolVersionMax);
    }

    //
    // The default min protocol version is set to TLS1.0 to avoid security issues with SSLv3
    //
    const string protocolVersionMin = properties->getPropertyWithDefault("IceSSL.ProtocolVersionMin", "tls1_0");
    if(!protocolVersionMin.empty())
    {
        _protocolVersionMin = parseProtocol(protocolVersionMin);
    }
    _initialized = true;
}

//
// Destroy the engine.
//
void
IceSSL::SecureTransport::SSLEngine::destroy()
{
}

IceInternal::TransceiverPtr
IceSSL::SecureTransport::SSLEngine::createTransceiver(const InstancePtr& instance,
                                                      const IceInternal::TransceiverPtr& delegate,
                                                      const string& hostOrAdapterName,
                                                      bool incoming)
{
    return make_shared<IceSSL::SecureTransport::TransceiverI>(instance, delegate, hostOrAdapterName, incoming);
}

SSLContextRef
IceSSL::SecureTransport::SSLEngine::newContext(bool incoming)
{
    SSLContextRef ssl = SSLCreateContext(kCFAllocatorDefault, incoming ? kSSLServerSide : kSSLClientSide,
                                         kSSLStreamType);
    if(!ssl)
    {
        throw SecurityException(__FILE__, __LINE__, "IceSSL: unable to create SSL context");
    }

    OSStatus err = noErr;
    if(incoming)
    {
        switch(getVerifyPeer())
        {
            case 0:
            {
                SSLSetClientSideAuthenticate(ssl, kNeverAuthenticate);
                break;
            }
            case 1:
            {
                SSLSetClientSideAuthenticate(ssl, kTryAuthenticate);
                break;
            }
            case 2:
            {
                SSLSetClientSideAuthenticate(ssl, kAlwaysAuthenticate);
                break;
            }
            default:
            {
                assert(false);
                break;
            }
        }

#if defined(ICE_USE_SECURE_TRANSPORT_MACOS)
        if(!_dhParams.empty())
        {
            if((err = SSLSetDiffieHellmanParams(ssl, &_dhParams[0], _dhParams.size())))
            {
                throw SecurityException(__FILE__, __LINE__,
                                        "IceSSL: unable to create the trust object:\n" + sslErrorToString(err));
            }
        }
#endif
    }

    if(_chain && (err = SSLSetCertificate(ssl, _chain.get())))
    {
        throw SecurityException(__FILE__, __LINE__,
                                "IceSSL: error while setting the SSL context certificate:\n" + sslErrorToString(err));
    }

    if(!_ciphers.empty())
    {
        if((err = SSLSetEnabledCiphers(ssl, &_ciphers[0], _ciphers.size())))
        {
            throw SecurityException(__FILE__, __LINE__, "IceSSL: error while setting ciphers:\n" + sslErrorToString(err));
        }
    }

    if((err = SSLSetSessionOption(ssl, incoming ? kSSLSessionOptionBreakOnClientAuth :
                                                  kSSLSessionOptionBreakOnServerAuth,
                                  true)))
    {
        throw SecurityException(__FILE__, __LINE__, "IceSSL: error while setting SSL option:\n" + sslErrorToString(err));
    }

    if(_protocolVersionMax != kSSLProtocolUnknown)
    {
        if((err = SSLSetProtocolVersionMax(ssl, _protocolVersionMax)))
        {
            throw SecurityException(__FILE__, __LINE__,
                                    "IceSSL: error while setting SSL protocol version max:\n" + sslErrorToString(err));
        }
    }

    if(_protocolVersionMin != kSSLProtocolUnknown)
    {
        if((err = SSLSetProtocolVersionMin(ssl, _protocolVersionMin)))
        {
            throw SecurityException(__FILE__, __LINE__,
                                    "IceSSL: error while setting SSL protocol version min:\n" + sslErrorToString(err));
        }
    }

    return ssl;
}

CFArrayRef
IceSSL::SecureTransport::SSLEngine::getCertificateAuthorities() const
{
    return _certificateAuthorities.get();
}

string
IceSSL::SecureTransport::SSLEngine::getCipherName(SSLCipherSuite cipher) const
{
    return CiphersHelper::cipherName(cipher);
}

void
IceSSL::SecureTransport::SSLEngine::parseCiphers(const string& ciphers)
{
    vector<string> tokens;
    vector<CipherExpression> cipherExpressions;

    bool allCiphers = false;
    IceUtilInternal::splitString(ciphers, " \t", tokens);
    for(vector<string>::const_iterator i = tokens.begin(); i != tokens.end(); ++i)
    {
        string token(*i);
        if(token == "ALL")
        {
            if(i != tokens.begin())
            {
                throw PluginInitializationException(__FILE__, __LINE__,
                                                    "IceSSL: `ALL' must be first in cipher list `" + ciphers + "'");
            }
            allCiphers = true;
        }
        else if(token == "NONE")
        {
            if(i != tokens.begin())
            {
                throw PluginInitializationException(__FILE__, __LINE__,
                                                    "IceSSL: `NONE' must be first in cipher list `" + ciphers + "'");
            }
        }
        else
        {
            CipherExpression ce;
            if(token.find('!') == 0)
            {
                ce.negation = true;
                if(token.size() > 1)
                {
                    token = token.substr(1);
                }
                else
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                                        "IceSSL: invalid cipher expression `" + token + "'");
                }
            }
            else
            {
                ce.negation = false;
            }

            if(token.find('(') == 0)
            {
                if(token.rfind(')') != token.size() - 1)
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                                        "IceSSL: invalid cipher expression `" + token + "'");
                }

                try
                {
                    ce.re = make_shared<RegExp>(token.substr(1, token.size() - 2));
                }
                catch(const Ice::SyscallException&)
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                                        "IceSSL: invalid cipher expression `" + token + "'");
                }
            }
            else
            {
                ce.cipher = token;
            }

            cipherExpressions.push_back(ce);
        }
    }

    //
    // Context used to get the cipher list
    //
    UniqueRef<SSLContextRef> ctx(SSLCreateContext(kCFAllocatorDefault, kSSLServerSide, kSSLStreamType));
    size_t numSupportedCiphers = 0;
    SSLGetNumberSupportedCiphers(ctx.get(), &numSupportedCiphers);

    vector<SSLCipherSuite> supported;
    supported.resize(numSupportedCiphers);

    OSStatus err = SSLGetSupportedCiphers(ctx.get(), &supported[0], &numSupportedCiphers);
    if(err)
    {
        throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: unable to get supported ciphers list:\n" + sslErrorToString(err));
    }

    vector<SSLCipherSuite> enabled;
    if(allCiphers)
    {
        enabled = supported;
    }

    for(vector<CipherExpression>::const_iterator i = cipherExpressions.begin(); i != cipherExpressions.end(); ++i)
    {
        CipherExpression ce = *i;
        if(ce.negation)
        {
            for(vector<SSLCipherSuite>::iterator j = enabled.begin(); j != enabled.end();)
            {
                string name = CiphersHelper::cipherName(*j);
                if((ce.cipher.empty() && ce.re->match(name)) || ce.cipher == name)
                {
                    j = enabled.erase(j);
                }
                else
                {
                    ++j;
                }
            }
        }
        else
        {
            if(ce.cipher.empty())
            {
                for(vector<SSLCipherSuite>::const_iterator j = supported.begin(); j != supported.end(); ++j)
                {
                    SSLCipherSuite cipher = *j;
                    string name = CiphersHelper::cipherName(cipher);
                    if(ce.re->match(name))
                    {
                        vector<SSLCipherSuite>::const_iterator k = find(enabled.begin(), enabled.end(), cipher);
                        if(k == enabled.end())
                        {
                            enabled.push_back(cipher);
                        }
                    }
                }
            }
            else
            {
                SSLCipherSuite cipher = CiphersHelper::cipherForName(ce.cipher);
                vector<SSLCipherSuite>::const_iterator k = find(enabled.begin(), enabled.end(), cipher);
                if(k == enabled.end())
                {
                    enabled.push_back(cipher);
                }
            }
        }
    }
    _ciphers = enabled;

    if(_ciphers.empty())
    {
        throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: invalid value for IceSSL.Ciphers:\n" + ciphers +
                                            "\nThe result cipher list does not contain any entries");
    }
}
