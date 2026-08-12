#pragma once

#include <mongoose.h>

typedef enum {
    MongooseTlsClientCertTypeNone,
    MongooseTlsClientCertTypeDevice,
    MongooseTlsClientCertTypeCustom,
} MongooseTlsClientCertType;

typedef struct {
    const char* cert;
    const char* key;
} MongooseTlsCustomPath;

typedef struct {
    const char* server_url;
    MongooseTlsCustomPath custom_path;
    MongooseTlsClientCertType client_cert_type;
    bool ignore_server_cert;
} MongooseTlsConfig;

bool mongoose_tls_init(struct mg_connection* conn, const MongooseTlsConfig* config);

void mongoose_tls_deinit(struct mg_connection* conn);
