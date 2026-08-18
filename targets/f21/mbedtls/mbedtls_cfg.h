#pragma once

/**
* A subset of the mbedTLS configuration options that are relevant to the
* Busy Bar firmware and apps. They are built to "mbedtls" library you can
* link your apps with.
*
* If you need more features, either bring the full mbedtls library into your
* app using "fap_private_libs" or open an issue on GitHub to add them to the
* default configuration.
**/

#define MBEDTLS_HAVE_ASM

#define MBEDTLS_NO_UDBL_DIVISION
#define MBEDTLS_NO_64BIT_MULTIPLICATION

#define MBEDTLS_DEPRECATED_WARNING

#define MBEDTLS_AES_FEWER_TABLES
// #define MBEDTLS_CHECK_RETURN_WARNING

// #define MBEDTLS_CIPHER_MODE_CBC
// #define MBEDTLS_CIPHER_MODE_CFB
// #define MBEDTLS_CIPHER_MODE_CTR
// #define MBEDTLS_CIPHER_MODE_OFB
// #define MBEDTLS_CIPHER_MODE_XTS

// #define MBEDTLS_CIPHER_PADDING_PKCS7
// #define MBEDTLS_CIPHER_PADDING_ONE_AND_ZEROS
// #define MBEDTLS_CIPHER_PADDING_ZEROS_AND_LEN
// #define MBEDTLS_CIPHER_PADDING_ZEROS

/* Short Weierstrass curves (supporting ECP, ECDH, ECDSA) */
// #define MBEDTLS_ECP_DP_SECP192R1_ENABLED
// #define MBEDTLS_ECP_DP_SECP224R1_ENABLED
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECP_DP_SECP384R1_ENABLED
#define MBEDTLS_ECP_DP_SECP521R1_ENABLED
// #define MBEDTLS_ECP_DP_SECP192K1_ENABLED
// #define MBEDTLS_ECP_DP_SECP224K1_ENABLED
// #define MBEDTLS_ECP_DP_SECP256K1_ENABLED
// #define MBEDTLS_ECP_DP_BP256R1_ENABLED
// #define MBEDTLS_ECP_DP_BP384R1_ENABLED
// #define MBEDTLS_ECP_DP_BP512R1_ENABLED
/* Montgomery curves (supporting ECP) */
#define MBEDTLS_ECP_DP_CURVE25519_ENABLED
// #define MBEDTLS_ECP_DP_CURVE448_ENABLED

#define MBEDTLS_ECP_NIST_OPTIM

// #define MBEDTLS_GENPRIME
#define MBEDTLS_PKCS1_V15
#define MBEDTLS_PKCS1_V21

#define MBEDTLS_MD_C

#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_CAN_ECDH
#define MBEDTLS_PK_CAN_ECDSA_SIGN
#define MBEDTLS_PK_C

#define MBEDTLS_ASN1_PARSE_C
// #define MBEDTLS_ASN1_WRITE_C

#define MBEDTLS_PSA_CRYPTO_C

#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG
#define MBEDTLS_NO_PLATFORM_ENTROPY

#define MBEDTLS_BASE64_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_OID_C

// #define MBEDTLS_HMAC_DRBG_C
// #define MBEDTLS_ECDH_LEGACY_CONTEXT

#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_SSL_PROTO_TLS1_3

// Server only
// #define MBEDTLS_SSL_COOKIE_C
#define MBEDTLS_SSL_PROTO_DTLS            // Not required, but fails build
// #define MBEDTLS_SSL_DTLS_ANTI_REPLAY
// #define MBEDTLS_SSL_DTLS_HELLO_VERIFY
// #define MBEDTLS_SSL_EXPORT_KEYS
#define MBEDTLS_SSL_KEEP_PEER_CERTIFICATE // Required for TLS1.3
#define MBEDTLS_SSL_MAX_FRAGMENT_LENGTH
// #define MBEDTLS_SSL_ALPN

#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED

#define MBEDTLS_SSL_TLS1_3_COMPATIBILITY_MODE
#define MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_EPHEMERAL_ENABLED

#define MBEDTLS_SSL_DTLS_CONNECTION_ID
// #define MBEDTLS_SSL_DTLS_CONNECTION_ID_COMPAT 0

#define MBEDTLS_CIPHER_C
// #define MBEDTLS_DES_C
// #define MBEDTLS_DHM_C

#define MBEDTLS_ECDH_C
#define MBEDTLS_ECDSA_C
#define MBEDTLS_ECP_C
#define MBEDTLS_HKDF_C

#define MBEDTLS_RSA_C
#define MBEDTLS_GCM_C
#define MBEDTLS_CCM_C
#define MBEDTLS_AES_C
#define MBEDTLS_MD5_C

// #define MBEDTLS_SHA224_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_SHA1_C
#define MBEDTLS_SHA384_C
#define MBEDTLS_SHA512_C

//Hostname verification
#define MBEDTLS_X509_CHECK_HOSTNAME
//Certificate extension checks
#define MBEDTLS_X509_CHECK_EXTENDED_KEY_USAGE
#define MBEDTLS_X509_CHECK_KEY_USAGE
//Full PK support
// #define MBEDTLS_PK_WRITE_C
#define MBEDTLS_PK_PARSE_C
//Additional verification options
#define MBEDTLS_X509_CRL_PARSE_C // For checking certificate revocation lists
#define MBEDTLS_X509_CSR_PARSE_C // For parsing certificate requests
//Enable SNI (Server Name Indication)
#define MBEDTLS_SSL_SERVER_NAME_INDICATION

// #define MBEDTLS_DEBUG_C
// #define MBEDTLS_ERROR_C
#define MBEDTLS_ERROR_STRERROR_DUMMY
