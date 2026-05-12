#include <mongoose.h>

#include <mbedtls/ssl.h>
#include <mbedtls/pk.h>

#include <pk_wrap.h>

#include <storage/storage.h>
#include <tls_crypto/tls_crypto.h>

#include "mqtt_config.h"

#define TAG "MqttTls"

#define TLS_DEBUG_LEVEL 0

// Intermediate cert slot (signing-ca.der)
#define TLS_KEY_SLOT_SIGN   TlsCryptoKeyIdIntermediate
// Device cert and key slot (device.der + device.key)
#define TLS_KEY_SLOT_DEVICE TlsCryptoKeyIdDevice

#define TLS_CUSTOM_CERT_DEVICE APP_ASSETS_PATH("device.crt")
#define TLS_CUSTOM_KEY         APP_ASSETS_PATH("device.key")

static const char* mqtt_alpn_list[] = {"mqtt", NULL};

static int tls_random(void* ctx, unsigned char* buf, size_t len) {
    UNUSED(ctx);
    mg_random(buf, len);
    return 0;
}

static void tls_debug_cb(void* ctx, int lev, const char* file, int line, const char* str) {
    UNUSED(file);
    UNUSED(line);
    size_t len = strlen(str) - 1;
    FURI_LOG_I(TAG, "%lu %d %.*s", ((struct mg_connection*)ctx)->id, lev, len, str);
}

static size_t tls_pk_get_bitlen(mbedtls_pk_context* pk) {
    UNUSED(pk);
    return 256;
}

static int tls_pk_can_do(mbedtls_pk_type_t type) {
    return (type == MBEDTLS_PK_ECKEY || type == MBEDTLS_PK_ECDSA);
}

static int tls_pk_sign_with_hw_crypto(
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

static const mbedtls_pk_info_t tls_pk_wrap_hw_crypto = {
    .type = MBEDTLS_PK_ECKEY,
    .name = "ECDSA_HW",
    .get_bitlen = tls_pk_get_bitlen,
    .can_do = tls_pk_can_do,
    .sign_message_func = tls_pk_sign_with_hw_crypto,
    .verify_func = NULL,
    .sign_func = NULL, // Using .sign_message_func instead
#if defined(MBEDTLS_ECDSA_C) && defined(MBEDTLS_ECP_RESTARTABLE)
    .verify_rs_func = NULL,
    .sign_rs_func = NULL,
#endif
    .decrypt_func = NULL,
    .encrypt_func = NULL,
    .check_pair_func = NULL,
    .ctx_alloc_func = NULL,
    .ctx_free_func = NULL,
#if defined(MBEDTLS_ECDSA_C) && defined(MBEDTLS_ECP_RESTARTABLE)
    .rs_alloc_func = NULL,
    .rs_free_func = NULL,
#endif
    .debug_func = NULL,
};

static bool tls_load_ca(struct mg_str str, mbedtls_x509_crt* p) {
    if(str.buf == NULL || str.buf[0] == '\0' || str.buf[0] == '*') return true;
    if(str.buf[0] == '-') str.len++; // PEM, include trailing NUL
    int ret = mbedtls_x509_crt_parse(p, (uint8_t*)str.buf, str.len);
    if(ret != 0) {
        FURI_LOG_E(TAG, "Cert parse error -0x%04X", -ret);
        return false;
    }
    return true;
}

static bool tls_load_cert_from_hw_crypto(uint8_t slot, mbedtls_x509_crt* crt) {
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

static bool tls_load_cert_from_file(char* path, mbedtls_x509_crt* crt) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    bool success = false;
    size_t cert_len = 0;
    uint8_t* cert_buf = NULL;

    do {
        if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Cert file error: %s", storage_file_get_error_desc(file));
            break;
        }

        cert_len = storage_file_size(file);
        cert_buf = malloc(cert_len + 1);

        if(storage_file_read(file, cert_buf, cert_len) != cert_len) {
            FURI_LOG_E(TAG, "Cert file read error");
            break;
        }

        const int parse_result = mbedtls_x509_crt_parse(crt, cert_buf, cert_len + 1);

        if(parse_result != 0) {
            FURI_LOG_E(TAG, "Cert parse error -0x%04X", -parse_result);
            break;
        }

        success = true;
    } while(0);

    if(cert_buf) {
        free(cert_buf);
    }

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return success;
}

static bool tls_load_key_from_file(char* path, mbedtls_pk_context* pk) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    bool success = false;
    size_t cert_len = 0;
    uint8_t* cert_buf = NULL;

    do {
        if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Key file error: %s", storage_file_get_error_desc(file));
            return false;
        }

        cert_len = storage_file_size(file);
        cert_buf = malloc(cert_len + 1);

        if(storage_file_read(file, cert_buf, cert_len) != cert_len) {
            FURI_LOG_E(TAG, "Key file read error");
            return false;
        }
        success = true;
    } while(0);

    storage_file_close(file);
    furi_record_close(RECORD_STORAGE);

    if(!success) {
        if(cert_buf) free(cert_buf);
        return false;
    }

    int ret = mbedtls_pk_parse_key(pk, cert_buf, cert_len + 1, NULL, 0, tls_random, 0);
    free(cert_buf);
    if(ret != 0) {
        FURI_LOG_E(TAG, "Key parse error -0x%04X", -ret);
        return false;
    }
    return true;
}

static int tls_net_send(void* ctx, const unsigned char* buf, size_t len) {
    long n = mg_io_send((struct mg_connection*)ctx, buf, len);
    if(n == MG_IO_WAIT) return MBEDTLS_ERR_SSL_WANT_WRITE;
    if(n == MG_IO_RESET) return MBEDTLS_ERR_NET_CONN_RESET;
    if(n == MG_IO_ERR) return MBEDTLS_ERR_NET_SEND_FAILED;
    return (int)n;
}

static int tls_net_recv(void* ctx, unsigned char* buf, size_t len) {
    long n = mg_io_recv((struct mg_connection*)ctx, buf, len);
    if(n == MG_IO_WAIT) return MBEDTLS_ERR_SSL_WANT_READ;
    if(n == MG_IO_RESET) return MBEDTLS_ERR_NET_CONN_RESET;
    if(n == MG_IO_ERR) return MBEDTLS_ERR_NET_RECV_FAILED;
    return (int)n;
}

static bool mqtt_tls_load_certificates(struct mg_tls* tls, MqttClientCertType cert_type) {
    bool success = false;

    do {
        if(cert_type == MqttClientCertTypeDefault) {
            if(!tls_load_cert_from_hw_crypto(TLS_KEY_SLOT_DEVICE, &tls->cert)) {
                break;
            }
            if(!tls_load_cert_from_hw_crypto(TLS_KEY_SLOT_SIGN, &tls->cert)) {
                break;
            }

            // Setup custom PK wrapper for private key operations
            mbedtls_pk_setup(&tls->pk, &tls_pk_wrap_hw_crypto);

        } else if(cert_type == MqttClientCertTypeCustom) {
            if(!tls_load_cert_from_file(TLS_CUSTOM_CERT_DEVICE, &tls->cert)) {
                break;
            }
            if(!tls_load_key_from_file(TLS_CUSTOM_KEY, &tls->pk)) {
                break;
            }

        } else {
            success = true;
            break;
        }

        const int ret = mbedtls_ssl_conf_own_cert(&tls->conf, &tls->cert, &tls->pk);

        if(tls->cert.version && ret != 0) {
            FURI_LOG_E(TAG, "mbedtls_ssl_conf_own_cert() failed: -%04X", -ret);
            break;
        }

        success = true;
    } while(false);

    return success;
}

static bool mqtt_tls_init_hostname(struct mg_tls* tls, const char* server_url) {
    bool success = false;

    do {
        const struct mg_str hostname = mg_url_host(server_url);

        if(hostname.buf == NULL || *hostname.buf == 0) {
            break;
        }

        FuriString* trimmed = furi_string_alloc_printf("%.*s", hostname.len, hostname.buf);
        mbedtls_ssl_set_hostname(&tls->ssl, furi_string_get_cstr(trimmed));
        furi_string_free(trimmed);

        success = true;
    } while(false);

    return success;
}

bool mqtt_tls_init(
    struct mg_connection* conn,
    const char* server_url,
    const char* ca_bundle,
    const MqttConfig* config) {
    bool success = false;

    struct mg_tls* tls = calloc(1, sizeof(*tls));
    conn->tls = tls;

    do {
        if(conn->is_listening) {
            break;
        }

        psa_crypto_init();

        mbedtls_ssl_init(&tls->ssl);
        mbedtls_ssl_config_init(&tls->conf);

        mbedtls_x509_crt_init(&tls->ca);
        mbedtls_x509_crt_init(&tls->cert);

        mbedtls_pk_init(&tls->pk);

        mbedtls_ssl_conf_dbg(&tls->conf, tls_debug_cb, conn);
        mbedtls_debug_set_threshold(TLS_DEBUG_LEVEL);

        int ret = mbedtls_ssl_config_defaults(
            &tls->conf,
            conn->is_client ? MBEDTLS_SSL_IS_CLIENT : MBEDTLS_SSL_IS_SERVER,
            MBEDTLS_SSL_TRANSPORT_STREAM,
            MBEDTLS_SSL_PRESET_DEFAULT);

        if(ret != 0) {
            mg_error(conn, "Config defaults -%04X", -ret);
            break;
        }

        mbedtls_ssl_conf_rng(&tls->conf, tls_random, conn);

        // Force TLS 1.3
        mbedtls_ssl_conf_min_tls_version(&tls->conf, MBEDTLS_SSL_VERSION_TLS1_3);
        mbedtls_ssl_conf_max_tls_version(&tls->conf, MBEDTLS_SSL_VERSION_TLS1_3);

        // ALPN
        mbedtls_ssl_conf_alpn_protocols(&tls->conf, mqtt_alpn_list);

        if(!tls_load_ca(mg_str(ca_bundle), &tls->ca)) {
            mg_error(conn, "Failed to load CA bundle");
            break;
        }

        mbedtls_ssl_conf_ca_chain(&tls->conf, &tls->ca, NULL);

        if(!mqtt_tls_init_hostname(tls, server_url)) {
            mg_error(conn, "Failed to parse hostname");
            break;
        }

        mbedtls_ssl_conf_authmode(
            &tls->conf,
            config->ignore_server_cert ? MBEDTLS_SSL_VERIFY_NONE : MBEDTLS_SSL_VERIFY_REQUIRED);

        if(!mqtt_tls_load_certificates(tls, config->client_cert_type)) {
            mg_error(conn, "Failed to load certificates");
            break;
        }

#ifdef MBEDTLS_SSL_SESSION_TICKETS
        mbedtls_ssl_conf_session_tickets_cb(
            &tls->conf,
            mbedtls_ssl_ticket_write,
            mbedtls_ssl_ticket_parse,
            &((struct mg_tls_ctx*)c->mgr->tls_ctx)->tickets);
#endif

        ret = mbedtls_ssl_setup(&tls->ssl, &tls->conf);

        if(ret != 0) {
            mg_error(conn, "Setup error -%04X", -ret);
            break;
        }

        conn->is_tls = 1;
        conn->is_tls_hs = 1;

        mbedtls_ssl_set_bio(&tls->ssl, conn, tls_net_send, tls_net_recv, 0);

        success = true;
    } while(false);

    if(!success) {
        mg_tls_free(conn);
    }

    return success;
}

void mqtt_tls_free_ca(struct mg_connection* c) {
    struct mg_tls* tls = (struct mg_tls*)c->tls;
    furi_assert(tls);
    mbedtls_x509_crt_free(&tls->ca);
}
