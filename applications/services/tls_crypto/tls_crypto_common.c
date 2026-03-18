#include "tls_crypto_common_i.h"

#include <core/log.h>

static const char* request_type_names[TlsCryptoRequestTypeMax] = {
    [TlsCryptoRequestTypeGetCertificate] = "GetCertificate",
    [TlsCryptoRequestTypeSignMessage] = "SignMessage",
};

void tls_crypto_log_response_status(const TlsCryptoResponse* response) {
    const TlsCryptoStatus status = response->status;
    const TlsCryptoRequestType request_type = response->type;

    const char* request_name = request_type_names[request_type];

    if(status == TlsCryptoStatusOk) {
        FURI_LOG_D(TAG, "%s OK", request_name);
    } else {
        FURI_LOG_E(TAG, "%s failed with error: %d", request_name, status);
    }
}
