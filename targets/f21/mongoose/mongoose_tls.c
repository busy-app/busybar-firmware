#include "mongoose_tls.h"

#include <mbedtls/ssl.h>
#include <mbedtls/pk.h>

#include <pk_wrap.h>

#include <storage/storage.h>
#include <ca_storage/ca_storage.h>

#include <tls_crypto/tls_crypto.h>

#define TLS_DEBUG_LEVEL 0

// Intermediate cert slot (signing-ca.der)
#define TLS_KEY_SLOT_SIGN   TlsCryptoKeyIdIntermediate
// Device cert and key slot (device.der + device.key)
#define TLS_KEY_SLOT_DEVICE TlsCryptoKeyIdDevice

#define TAG "MongooseTls"

typedef bool (*MongooseTlsParseCallback)(void* out, const uint8_t* data, size_t data_len);

static int mongoose_tls_random(void* ctx, unsigned char* buf, size_t len) {
    UNUSED(ctx);
    mg_random(buf, len);
    return 0;
}

static void
    mongoose_tls_debug_cb(void* ctx, int lev, const char* file, int line, const char* str) {
    UNUSED(file);
    UNUSED(line);
    size_t len = strlen(str) - 1;
    FURI_LOG_I(TAG, "%lu %d %.*s", ((struct mg_connection*)ctx)->id, lev, len, str);
}

static size_t mongoose_tls_pk_get_bitlen(mbedtls_pk_context* pk) {
    UNUSED(pk);
    return 256;
}

static int mongoose_tls_pk_can_do(mbedtls_pk_type_t type) {
    return (type == MBEDTLS_PK_ECKEY || type == MBEDTLS_PK_ECDSA);
}

static int mongoose_tls_net_send(void* ctx, const unsigned char* buf, size_t len) {
    int result;

    const long bytes_sent = mg_io_send((struct mg_connection*)ctx, buf, len);

    if(bytes_sent == MG_IO_WAIT) {
        result = MBEDTLS_ERR_SSL_WANT_WRITE;
    } else if(bytes_sent == MG_IO_RESET) {
        result = MBEDTLS_ERR_NET_CONN_RESET;
    } else if(bytes_sent == MG_IO_ERR) {
        result = MBEDTLS_ERR_NET_SEND_FAILED;
    } else {
        result = bytes_sent;
    }

    return result;
}

static int mongoose_tls_net_recv(void* ctx, unsigned char* buf, size_t len) {
    int result;

    const long bytes_received = mg_io_recv((struct mg_connection*)ctx, buf, len);

    if(bytes_received == MG_IO_WAIT) {
        result = MBEDTLS_ERR_SSL_WANT_READ;
    } else if(bytes_received == MG_IO_RESET) {
        result = MBEDTLS_ERR_NET_CONN_RESET;
    } else if(bytes_received == MG_IO_ERR) {
        result = MBEDTLS_ERR_NET_RECV_FAILED;
    } else {
        result = bytes_received;
    }

    return result;
}

static void mongoose_tls_conf_ca_chain(mbedtls_ssl_config* conf) {
    CaStorage* ca_storage = furi_record_open(RECORD_CA_STORAGE);
    // NOTE: Assuming that the certificate chain will
    // only be read during the TLS context lifetime
    mbedtls_x509_crt* ca_chain = (mbedtls_x509_crt*)ca_storage_get_cert_chain(ca_storage);
    mbedtls_ssl_conf_ca_chain(conf, ca_chain, NULL);

    furi_record_close(RECORD_CA_STORAGE);
}

static bool mongoose_tls_load_cert_from_hw_crypto(uint8_t slot, mbedtls_x509_crt* crt) {
    bool success = false;

    do {
        TlsCrypto* tls_crypto = furi_record_open(RECORD_TLS_CRYPTO);

        TlsCryptoCertificate certificate = {0};
        const TlsCryptoStatus crypto_status =
            tls_crypto_get_certificate(tls_crypto, slot, &certificate);

        furi_record_close(RECORD_TLS_CRYPTO);

        if(crypto_status != TlsCryptoStatusOk) {
            if(crypto_status == TlsCryptoStatusErrorTimeout) {
                FURI_LOG_E(TAG, "Failed to get certificate from hw crypto: timeout");
            } else {
                FURI_LOG_E(TAG, "Failed to get certificate from hw crypto: internal error");
            }
            break;
        }

        const int parse_result =
            mbedtls_x509_crt_parse(crt, certificate.bytes, certificate.length);

        if(parse_result != 0) {
            FURI_LOG_E(TAG, "Cert parse error -0x%04X", -parse_result);
            break;
        }

        success = true;
    } while(false);

    return success;
}

static int mongoose_tls_pk_sign_with_hw_crypto(
    mbedtls_md_type_t md_alg,
    const unsigned char* data,
    size_t data_len,
    unsigned char* sig,
    size_t sig_size,
    size_t* sig_len) {
    int ret;

    do {
        if(md_alg != MBEDTLS_MD_SHA256) {
            FURI_LOG_E(TAG, "Unsupported MD algorithm 0x%02X", md_alg);
            ret = MBEDTLS_ERR_SSL_FEATURE_UNAVAILABLE;
            break;
        }

        TlsCrypto* tls_crypto = furi_record_open(RECORD_TLS_CRYPTO);

        TlsCryptoSignature signature;
        const TlsCryptoStatus crypto_status =
            tls_crypto_sign(tls_crypto, TLS_KEY_SLOT_DEVICE, data, data_len, &signature);

        furi_record_close(RECORD_TLS_CRYPTO);

        if(crypto_status != TlsCryptoStatusOk) {
            if(crypto_status == TlsCryptoStatusErrorTimeout) {
                FURI_LOG_E(TAG, "Failed to sign with hw crypto: timeout");
                ret = MBEDTLS_ERR_SSL_TIMEOUT;
            } else {
                FURI_LOG_E(TAG, "Failed to sign with hw crypto: internal error");
                ret = MBEDTLS_ERR_SSL_INTERNAL_ERROR;
            }
            break;
        }

        if(sig_size < signature.length) {
            ret = MBEDTLS_ERR_SSL_BUFFER_TOO_SMALL;
            break;
        }

        memcpy(sig, signature.bytes, signature.length);
        *sig_len = signature.length;

        ret = 0;

    } while(false);

    return ret;
}

static bool mongoose_tls_setup_pk_wrapper(struct mg_tls* tls) {
    bool success = true;

    static const mbedtls_pk_info_t tls_pk_wrap_hw_crypto = {
        .type = MBEDTLS_PK_ECKEY,
        .name = "ECDSA_HW",
        .get_bitlen = mongoose_tls_pk_get_bitlen,
        .can_do = mongoose_tls_pk_can_do,
        .sign_message_func = mongoose_tls_pk_sign_with_hw_crypto,
    };

    const int status = mbedtls_pk_setup(&tls->pk, &tls_pk_wrap_hw_crypto);

    if(status != 0) {
        success = false;
    }

    return success;
}

static bool mongoose_tls_load_device_certificates(struct mg_tls* tls) {
    bool success = false;

    do {
        if(!mongoose_tls_load_cert_from_hw_crypto(TLS_KEY_SLOT_DEVICE, &tls->cert)) {
            break;
        }

        if(!mongoose_tls_load_cert_from_hw_crypto(TLS_KEY_SLOT_SIGN, &tls->cert)) {
            break;
        }

        if(!mongoose_tls_setup_pk_wrapper(tls)) {
            break;
        }

        success = true;
    } while(false);

    return success;
}

static bool mongoose_tls_parse_cert_callback(void* out, const uint8_t* data, size_t data_len) {
    bool success = true;

    const int status = mbedtls_x509_crt_parse((mbedtls_x509_crt*)out, data, data_len);

    if(status != 0) {
        FURI_LOG_E(TAG, "Cert parse error -0x%04X", -status);
        success = false;
    }

    return success;
}

static bool mongoose_tls_parse_pk_callback(void* out, const uint8_t* data, size_t data_len) {
    bool success = true;

    const int status = mbedtls_pk_parse_key(
        (mbedtls_pk_context*)out, data, data_len, NULL, 0, mongoose_tls_random, 0);

    if(status != 0) {
        FURI_LOG_E(TAG, "Key parse error -0x%04X", -status);
        success = false;
    }

    return success;
}

static bool mongoose_tls_load_cert_from_file(
    const char* path,
    void* out,
    MongooseTlsParseCallback parse_callback) {
    bool success = false;
    uint8_t* data_buf = NULL;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(
                TAG, "Failed to open file for reading: %s", storage_file_get_error_desc(file));
            break;
        }

        const size_t data_len = storage_file_size(file);

        data_buf = malloc(data_len + 1);

        if(storage_file_read(file, data_buf, data_len) != data_len) {
            FURI_LOG_E(TAG, "Failed to read file: %s", storage_file_get_error_desc(file));
            break;
        }

        data_buf[data_len] = '\0';

        if(!parse_callback(out, data_buf, data_len + 1)) {
            break;
        }

        success = true;
    } while(0);

    if(data_buf) {
        free(data_buf);
    }

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return success;
}

static bool
    mongoose_tls_load_custom_certificates(struct mg_tls* tls, const TlsClientCertPaths* paths) {
    bool success = false;

    do {
        if(!mongoose_tls_load_cert_from_file(
               paths->certificate, &tls->cert, mongoose_tls_parse_cert_callback)) {
            break;
        }
        if(!mongoose_tls_load_cert_from_file(
               paths->private_key, &tls->pk, mongoose_tls_parse_pk_callback)) {
            break;
        }

        success = true;
    } while(false);

    return success;
}

static bool
    mongoose_tls_load_client_certificates(struct mg_tls* tls, const TlsClientCertInfo* cert_info) {
    bool success = false;

    do {
        const TlsClientCertType cert_type = cert_info->type;

        if(cert_type == TlsClientCertTypeNone) {
            success = true;
            break;

        } else if(cert_type == TlsClientCertTypeDevice) {
            if(!mongoose_tls_load_device_certificates(tls)) {
                break;
            }

        } else if(cert_type == TlsClientCertTypeCustom) {
            if(!mongoose_tls_load_custom_certificates(tls, &cert_info->paths)) {
                break;
            }

        } else {
            break;
        }

        const int mbedtls_status = mbedtls_ssl_conf_own_cert(&tls->conf, &tls->cert, &tls->pk);

        if((tls->cert.version != 0) && (mbedtls_status != 0)) {
            FURI_LOG_E(TAG, "mbedtls_ssl_conf_own_cert() failed: -%04X", -mbedtls_status);
            break;
        }

        success = true;
    } while(false);

    return success;
}

static bool mongoose_tls_init_hostname(struct mg_tls* tls, const char* server_url) {
    bool success = false;

    do {
        const struct mg_str hostname = mg_url_host(server_url);

        if((hostname.buf == NULL) || (hostname.len == 0)) {
            break;
        }

        FuriString* trimmed = furi_string_alloc_printf("%.*s", hostname.len, hostname.buf);
        mbedtls_ssl_set_hostname(&tls->ssl, furi_string_get_cstr(trimmed));
        furi_string_free(trimmed);

        success = true;
    } while(false);

    return success;
}

bool mongoose_tls_init(struct mg_connection* conn, const char* url, const TlsConfig* config) {
    bool success = false;

    struct mg_tls* tls = calloc(1, sizeof(*tls));
    conn->tls = tls;

    do {
        int mbedtls_status;

        if(conn->is_listening) {
            break;
        }

        psa_crypto_init();

        mbedtls_ssl_init(&tls->ssl);
        mbedtls_ssl_config_init(&tls->conf);

        mbedtls_x509_crt_init(&tls->ca);
        mbedtls_x509_crt_init(&tls->cert);

        mbedtls_pk_init(&tls->pk);

        mbedtls_ssl_conf_dbg(&tls->conf, mongoose_tls_debug_cb, conn);
        mbedtls_debug_set_threshold(TLS_DEBUG_LEVEL);

        const int endpoint = conn->is_client ? MBEDTLS_SSL_IS_CLIENT : MBEDTLS_SSL_IS_SERVER;

        mbedtls_status = mbedtls_ssl_config_defaults(
            &tls->conf, endpoint, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);

        if(mbedtls_status != 0) {
            FURI_LOG_E(TAG, "mbedtls_ssl_config_defaults() failed: -%04X", -mbedtls_status);
            break;
        }

        mbedtls_ssl_conf_rng(&tls->conf, mongoose_tls_random, conn);

        mbedtls_ssl_conf_min_tls_version(&tls->conf, MBEDTLS_SSL_VERSION_TLS1_2);
        mbedtls_ssl_conf_max_tls_version(&tls->conf, MBEDTLS_SSL_VERSION_TLS1_3);

        mongoose_tls_conf_ca_chain(&tls->conf);

        if(!mongoose_tls_init_hostname(tls, url)) {
            FURI_LOG_E(TAG, "Failed to determine hostname");
            break;
        }

        const int auth_mode = config->is_server_cert_ignored ? MBEDTLS_SSL_VERIFY_NONE :
                                                               MBEDTLS_SSL_VERIFY_REQUIRED;
        mbedtls_ssl_conf_authmode(&tls->conf, auth_mode);

        if(!mongoose_tls_load_client_certificates(tls, &config->client_cert_info)) {
            FURI_LOG_E(TAG, "Failed to load client certificates");
            break;
        }

        mbedtls_status = mbedtls_ssl_setup(&tls->ssl, &tls->conf);

        if(mbedtls_status != 0) {
            FURI_LOG_E(TAG, "mbedtls_ssl_setup() failed: -%04X", -mbedtls_status);
            break;
        }

        conn->is_tls = 1;
        conn->is_tls_hs = 1;

        mbedtls_ssl_set_bio(&tls->ssl, conn, mongoose_tls_net_send, mongoose_tls_net_recv, 0);

        success = true;
    } while(false);

    if(!success) {
        mg_tls_free(conn);
    }

    return success;
}
