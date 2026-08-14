#include "tls_config.h"

#include <core/check.h>

static TlsConfigValidationStatus
    tls_config_validate_client_cert_paths(const TlsClientCertPaths* client_cert_paths) {
    TlsConfigValidationStatus status;

    do {
        if((client_cert_paths->certificate == NULL) ||
           (strlen(client_cert_paths->certificate) == 0)) {
            status = TlsConfigValidationStatusClientCertNotSpecified;
            break;
        }

        if((client_cert_paths->private_key == NULL) ||
           (strlen(client_cert_paths->private_key) == 0)) {
            status = TlsConfigValidationStatusPrivateKeyNotSpecified;
            break;
        }

        status = TlsConfigValidationStatusOk;
    } while(false);

    return status;
}

void tls_config_init(TlsConfig* tls_config) {
    furi_check(tls_config);

    memset(tls_config, 0, sizeof(TlsConfig));
    tls_config->client_cert_info.type = TlsClientCertTypeNone;
}

TlsConfigValidationStatus tls_config_validate(const TlsConfig* tls_config) {
    furi_check(tls_config);

    TlsConfigValidationStatus status = TlsConfigValidationStatusOk;

    const TlsClientCertInfo* client_cert_info = &tls_config->client_cert_info;
    const TlsClientCertType client_cert_type = client_cert_info->type;

    if((client_cert_type == TlsClientCertTypeNone) ||
       (client_cert_type == TlsClientCertTypeDevice)) {
        /* Assumed valid */
    } else if(client_cert_type == TlsClientCertTypeCustom) {
        status = tls_config_validate_client_cert_paths(&client_cert_info->paths);
    } else {
        status = TlsConfigValidationStatusInvalidType;
    }

    return status;
}
