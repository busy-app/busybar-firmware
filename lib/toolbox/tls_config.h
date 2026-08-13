/**
 * @file tls_config.h
 */
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TlsClientCertTypeNone,
    TlsClientCertTypeDevice,
    TlsClientCertTypeCustom,
    TlsClientCertTypeInvalid,
} TlsClientCertType;

typedef struct {
    const char* certificate;
    const char* private_key;
} TlsClientCertPaths;

typedef struct {
    TlsClientCertType type;
    TlsClientCertPaths paths;
} TlsClientCertInfo;

typedef struct {
    TlsClientCertInfo client_cert_info;
    bool is_server_cert_ignored;
} TlsConfig;

typedef enum {
    TlsConfigValidationStatusOk,
    TlsConfigValidationStatusInvalidType,
    TlsConfigValidationStatusMissingPaths,
} TlsConfigValidationStatus;

TlsConfigValidationStatus tls_config_validate(const TlsConfig* tls_config);

#ifdef __cplusplus
}
#endif
