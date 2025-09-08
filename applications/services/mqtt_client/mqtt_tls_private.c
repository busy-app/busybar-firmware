#include "mqtt_i.h"
#include <mbedtls/library/pk_wrap.h>
#include <tls_crypto/tls_crypto_client.h>

#define TAG "MqttTls"

#define TLS_DEBUG_LEVEL 2
#define TLS_KEY_SLOT    0

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

#define TLS_CRYPTO_DATA_SIZE_MAX 128

typedef enum {
    TlsCryptoSignRequest,
    TlsCryptoSignResponse,
    TlsCryptoError = 0xFFFFFFFF, /**< Special value for error handling */
} TlsCryptoCmd;

typedef struct FURI_PACKED {
    TlsCryptoCmd cmd; /**< Command type */
    uint8_t key_slot;
    size_t data_size;
    uint8_t data[TLS_CRYPTO_DATA_SIZE_MAX];
} TlsCryptoSignMessage;

static const char priv_key[] = "-----BEGIN EC PRIVATE KEY-----\n"
                               "MHcCAQEEIB7FooM2FmMvHm2eO/bkpAf+4cOpu/pledOBmU7IuF+5oAoGCCqGSM49\n"
                               "AwEHoUQDQgAEV06XdA1WR0mDHEGd4EVl+aBcO8uKG51nawvLS+JWlBpvJyLs1wC3\n"
                               "gCI0cTPazzzx5PhoH7rzE44q9v4MvI6QIg==\n"
                               "-----END EC PRIVATE KEY-----\n";

static bool tls_sign(
    uint8_t key_slot,
    const uint8_t* hash,
    size_t hash_len,
    uint8_t* sign_buf,
    size_t sign_buf_size,
    size_t* sign_len) {
    if(key_slot != TLS_KEY_SLOT) {
        return false;
    }

    int ret = 0;
    bool success = false;
    mbedtls_pk_context sign_pk;
    mbedtls_pk_init(&sign_pk);
    do {
        ret = mbedtls_pk_parse_key(
            &sign_pk, (uint8_t*)priv_key, sizeof(priv_key), NULL, 0, tls_random, NULL);
        if(ret != 0) {
            FURI_LOG_E(TAG, "Key load err -%04X", -ret);
            break;
        }

        ret = mbedtls_pk_sign(
            &sign_pk,
            MBEDTLS_MD_SHA256,
            hash,
            hash_len,
            sign_buf,
            sign_buf_size,
            sign_len,
            tls_random,
            NULL);

        if(ret != 0) {
            FURI_LOG_E(TAG, "Sign err -%04X", -ret);
            break;
        }
        success = true;
    } while(0);
    mbedtls_pk_free(&sign_pk);
    return success;
}

static int tls_pk_sign_wrap(
    mbedtls_pk_context* pk,
    mbedtls_md_type_t md_alg,
    const unsigned char* hash,
    size_t hash_len,
    unsigned char* sig,
    size_t sig_size,
    size_t* sig_len,
    int (*f_rng)(void*, unsigned char*, size_t),
    void* p_rng) {
    UNUSED(pk);
    UNUSED(f_rng);
    UNUSED(p_rng);
    if(md_alg != MBEDTLS_MD_SHA256) {
        return MBEDTLS_ERR_SSL_FEATURE_UNAVAILABLE;
    }
    // bool success = tls_crypto_client_sign(TLS_KEY_SLOT, hash, hash_len, sig, sig_size, sig_len);
    bool success = tls_sign(TLS_KEY_SLOT, hash, hash_len, sig, sig_size, sig_len);
    return (success ? 0 : MBEDTLS_ERR_SSL_INTERNAL_ERROR);
}

static size_t tls_pk_get_bitlen(mbedtls_pk_context* pk) {
    UNUSED(pk);
    return 256;
}

static int tls_pk_can_do(mbedtls_pk_type_t type) {
    return (type == MBEDTLS_PK_ECKEY || type == MBEDTLS_PK_ECDSA);
}

static const mbedtls_pk_info_t tls_pk_wrap = {
    .type = MBEDTLS_PK_ECDSA,
    .name = "ECDSA_917",
    .get_bitlen = tls_pk_get_bitlen,
    .can_do = tls_pk_can_do,
    .verify_func = NULL,
    .sign_func = tls_pk_sign_wrap,
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

static bool tls_load_cert(struct mg_str str, mbedtls_x509_crt* p) {
    if(str.buf == NULL || str.buf[0] == '\0' || str.buf[0] == '*') return true;
    if(str.buf[0] == '-') str.len++; // PEM, include trailing NUL
    int ret = mbedtls_x509_crt_parse(p, (uint8_t*)str.buf, str.len);
    if(ret != 0) {
        FURI_LOG_E(TAG, "Cert load err -%04X", -ret);
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
    if(n == MG_IO_WAIT) return MBEDTLS_ERR_SSL_WANT_WRITE;
    if(n == MG_IO_RESET) return MBEDTLS_ERR_NET_CONN_RESET;
    if(n == MG_IO_ERR) return MBEDTLS_ERR_NET_RECV_FAILED;
    return (int)n;
}

void mqtt_tls_init(struct mg_connection* conn, const struct mg_tls_opts* opts) {
    struct mg_tls* tls = (struct mg_tls*)calloc(1, sizeof(*tls));
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

        if(opts->ca.len == 0 || mg_strcmp(opts->ca, mg_str("*")) == 0) {
            // NOTE: MBEDTLS_SSL_VERIFY_NONE is not supported for TLS1.3 on client side
            // See https://github.com/Mbed-TLS/mbedtls/issues/7075
            mbedtls_ssl_conf_authmode(&tls->conf, MBEDTLS_SSL_VERIFY_NONE);
        } else {
            if(tls_load_cert(opts->ca, &tls->ca) == false) {
                break;
            }
            mbedtls_ssl_conf_ca_chain(&tls->conf, &tls->ca, NULL);
            if(conn->is_client && opts->name.buf != NULL && opts->name.buf[0] != '\0') {
                char* host = mg_mprintf("%.*s", opts->name.len, opts->name.buf);
                mbedtls_ssl_set_hostname(&tls->ssl, host);
                free(host);
            }
            mbedtls_ssl_conf_authmode(&tls->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
        }
        if(!tls_load_cert(opts->cert, &tls->cert)) {
            break;
        }
        tls->pk.pk_info = &tls_pk_wrap;

        ret = mbedtls_ssl_conf_own_cert(&tls->conf, &tls->cert, &tls->pk);
        if(tls->cert.version && ret != 0) {
            mg_error(conn, "Config own cert -%04X", -ret);
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

        return;
    } while(0);

    mg_tls_free(conn);
}
