#include "tls_config.h"

#include <core/check.h>

TlsConfigValidationStatus tls_config_validate(const TlsConfig* tls_config) {
    furi_check(tls_config);

    TlsConfigValidationStatus status = TlsConfigValidationStatusOk;

    const TlsClientCertInfo* client_cert_info = &tls_config->client_cert_info;
    const TlsClientCertType client_cert_type = client_cert_info->type;

    if((client_cert_type == TlsClientCertTypeNone) ||
       (client_cert_type == TlsClientCertTypeDevice)) {
        /* Nothing */
    } else if(client_cert_type == TlsClientCertTypeCustom) {
        const TlsClientCertPaths* client_cert_paths = &client_cert_info->paths;

        if((client_cert_paths->certificate == NULL) || (client_cert_paths->private_key == NULL) ||
           (strlen(client_cert_paths->certificate) == 0) ||
           (strlen(client_cert_paths->private_key) == 0)) {
            status = TlsConfigValidationStatusMissingPaths;
        }

    } else {
        status = TlsConfigValidationStatusInvalidType;
    }

    return status;
}
