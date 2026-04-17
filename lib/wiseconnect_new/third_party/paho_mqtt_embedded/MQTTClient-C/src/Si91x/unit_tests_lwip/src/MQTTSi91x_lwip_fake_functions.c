/*******************************************************************************
 * @file
 * @brief Fake/stub functions for MQTTSi91x_lwip unit tests
 ******************************************************************************/
#include "MQTTSi91x_lwip_fake_functions.h"

DEFINE_FFF_GLOBALS;

DEFINE_FAKE_VALUE_FUNC(uint32_t, osKernelGetTickCount);
DEFINE_FAKE_VOID_FUNC1(mqtt_fake_disconnect, Network *);

// Socket and BSD-like functions
DEFINE_FAKE_VALUE_FUNC5(int, setsockopt, int, int, int, const void *, socklen_t);
DEFINE_FAKE_VALUE_FUNC4(ssize_t, recv, int, void *, size_t, int);
DEFINE_FAKE_VALUE_FUNC4(int, send, int, const void *, size_t, int);
DEFINE_FAKE_VALUE_FUNC3(int, socket, int, int, int);
DEFINE_FAKE_VALUE_FUNC3(int, bind, int, const struct sockaddr *, socklen_t);
DEFINE_FAKE_VALUE_FUNC3(int, connect, int, const struct sockaddr *, socklen_t);

// Fakes for TLS and time functions
DEFINE_FAKE_VALUE_FUNC0(uint32_t, sys_now);

DEFINE_FAKE_VALUE_FUNC5(int, lwip_select, int, fd_set *, fd_set *, fd_set *, struct timeval *);
DEFINE_FAKE_VALUE_FUNC3(ssize_t, lwip_read, int, void *, size_t);
DEFINE_FAKE_VALUE_FUNC3(ssize_t, lwip_write, int, const void *, size_t);
DEFINE_FAKE_VALUE_FUNC1(int, lwip_close, int);
DEFINE_FAKE_VALUE_FUNC3(int, lwip_socket, int, int, int);
DEFINE_FAKE_VALUE_FUNC3(int, lwip_bind, int, const struct sockaddr *, socklen_t);
DEFINE_FAKE_VALUE_FUNC3(int, lwip_connect, int, const struct sockaddr *, socklen_t);
DEFINE_FAKE_VALUE_FUNC1(uint16_t, lwip_htons, uint16_t);
DEFINE_FAKE_VALUE_FUNC4(ssize_t, lwip_recv, int, void *, size_t, int);
DEFINE_FAKE_VALUE_FUNC4(ssize_t, lwip_send, int, const void *, size_t, int);

// mbedTLS functions (required by MQTTSi91x_lwip.c)
DEFINE_FAKE_VOID_FUNC1(mbedtls_ssl_config_init, mbedtls_ssl_config *);
DEFINE_FAKE_VOID_FUNC1(mbedtls_x509_crt_init, mbedtls_x509_crt *);
DEFINE_FAKE_VOID_FUNC1(mbedtls_ssl_init, mbedtls_ssl_context *);
DEFINE_FAKE_VOID_FUNC1(mbedtls_entropy_init, mbedtls_entropy_context *);
DEFINE_FAKE_VOID_FUNC1(mbedtls_ctr_drbg_init, mbedtls_ctr_drbg_context *);
DEFINE_FAKE_VOID_FUNC1(mbedtls_ssl_free, mbedtls_ssl_context *);
DEFINE_FAKE_VOID_FUNC1(mbedtls_ssl_config_free, mbedtls_ssl_config *);
DEFINE_FAKE_VOID_FUNC1(mbedtls_x509_crt_free, mbedtls_x509_crt *);
DEFINE_FAKE_VOID_FUNC1(mbedtls_entropy_free, mbedtls_entropy_context *);
DEFINE_FAKE_VOID_FUNC1(mbedtls_ctr_drbg_free, mbedtls_ctr_drbg_context *);
DEFINE_FAKE_VOID_FUNC3(mbedtls_ssl_conf_dbg, mbedtls_ssl_config *, mbedtls_ssl_debug_cb, void *);
DEFINE_FAKE_VOID_FUNC1(mbedtls_debug_set_threshold, int);
DEFINE_FAKE_VALUE_FUNC5(int,
                        mbedtls_entropy_add_source,
                        mbedtls_entropy_context *,
                        mbedtls_entropy_f_source_ptr,
                        void *,
                        size_t,
                        int);
DEFINE_FAKE_VALUE_FUNC5(int,
                        mbedtls_ctr_drbg_seed,
                        mbedtls_ctr_drbg_context *,
                        mbedtls_ctr_drbg_entropy_cb,
                        void *,
                        const unsigned char *,
                        size_t);
DEFINE_FAKE_VALUE_FUNC4(int, mbedtls_ssl_config_defaults, mbedtls_ssl_config *, int, int, int);
DEFINE_FAKE_VOID_FUNC2(mbedtls_ssl_conf_authmode, mbedtls_ssl_config *, int);
DEFINE_FAKE_VOID_FUNC3(mbedtls_ssl_conf_rng, mbedtls_ssl_config *, mbedtls_ctr_drbg_entropy_cb, void *);
DEFINE_FAKE_VOID_FUNC3(mbedtls_ssl_conf_handshake_timeout, mbedtls_ssl_config *, uint32_t, uint32_t);
DEFINE_FAKE_VOID_FUNC2(mbedtls_ssl_conf_session_tickets, mbedtls_ssl_config *, int);
DEFINE_FAKE_VOID_FUNC2(mbedtls_ssl_conf_renegotiation, mbedtls_ssl_config *, int);
DEFINE_FAKE_VALUE_FUNC2(int, mbedtls_ssl_setup, mbedtls_ssl_context *, const mbedtls_ssl_config *);
DEFINE_FAKE_VALUE_FUNC2(int, mbedtls_ssl_set_hostname, mbedtls_ssl_context *, const char *);
DEFINE_FAKE_VOID_FUNC5(mbedtls_ssl_set_bio,
                       mbedtls_ssl_context *,
                       void *,
                       mbedtls_ssl_send_t *,
                       mbedtls_ssl_recv_t *,
                       mbedtls_ssl_recv_timeout_t *);
DEFINE_FAKE_VALUE_FUNC1(int, mbedtls_ssl_handshake, mbedtls_ssl_context *);
DEFINE_FAKE_VOID_FUNC2(mbedtls_ssl_conf_read_timeout, mbedtls_ssl_config *, uint32_t);
DEFINE_FAKE_VALUE_FUNC3(int, mbedtls_ssl_read, mbedtls_ssl_context *, unsigned char *, size_t);
DEFINE_FAKE_VALUE_FUNC3(int, mbedtls_ssl_write, mbedtls_ssl_context *, const unsigned char *, size_t);
DEFINE_FAKE_VALUE_FUNC3(int, mbedtls_x509_crt_parse, mbedtls_x509_crt *, const unsigned char *, size_t);
DEFINE_FAKE_VALUE_FUNC3(int, mbedtls_entropy_func, void *, unsigned char *, size_t);
DEFINE_FAKE_VALUE_FUNC3(int, mbedtls_ctr_drbg_random, void *, unsigned char *, size_t);
