#include <furi.h>

#include <mongoose.h>
#include <wifi/wifi.h>
#include <network/network.h>
#include "certs.h"

#define TAG "HttpsTest"

//#define HTTP_URL "https://www.example.com:443/"
//#define HTTP_URL "https://www.example.org/"
//#define HTTP_URL "https://192.168.10.2:443/"
#define HTTP_URL "https://portal6400.ru:443/"
//#define HTTP_URL "https://letsencrypt.org:443/"
typedef struct {
    Network* network;
    struct mg_mgr mgr;
    bool done;
} HttpTestApp;

static void http_test_mg_handler(struct mg_connection* connection, int event, void* event_data) {
    UNUSED(event_data);

    HttpTestApp* instance = connection->fn_data;
    furi_assert(instance);

    if(event == MG_EV_CONNECT) {
        const struct mg_str name = mg_url_host(HTTP_URL);
        const struct mg_str ca =
            mg_str_s("-----BEGIN CERTIFICATE-----\n"
                     "MIIELzCCAxegAwIBAgIUJlZAiy9xhYxQ0EpxMRkjmBIeW5gwDQYJKoZIhvcNAQEL\n"
                     "BQAwgaYxCzAJBgNVBAYTAlJVMRUwEwYDVQQIDAwxOTIuMTY4LjEwLjIxFTATBgNV\n"
                     "BAcMDDE5Mi4xNjguMTAuMjEVMBMGA1UECgwMMTkyLjE2OC4xMC4yMRUwEwYDVQQL\n"
                     "DAwxOTIuMTY4LjEwLjIxFTATBgNVBAMMDDE5Mi4xNjguMTAuMjEkMCIGCSqGSIb3\n"
                     "DQEJARYVMTkyLjE2OC4xMC4yQGdtYWlsLnJ1MB4XDTI1MDgyOTEzMDUxN1oXDTI1\n"
                     "MDkyODEzMDUxN1owgaYxCzAJBgNVBAYTAlJVMRUwEwYDVQQIDAwxOTIuMTY4LjEw\n"
                     "LjIxFTATBgNVBAcMDDE5Mi4xNjguMTAuMjEVMBMGA1UECgwMMTkyLjE2OC4xMC4y\n"
                     "MRUwEwYDVQQLDAwxOTIuMTY4LjEwLjIxFTATBgNVBAMMDDE5Mi4xNjguMTAuMjEk\n"
                     "MCIGCSqGSIb3DQEJARYVMTkyLjE2OC4xMC4yQGdtYWlsLnJ1MIIBIjANBgkqhkiG\n"
                     "9w0BAQEFAAOCAQ8AMIIBCgKCAQEApbB8XUL2AJHoHFQkGSslKxo1CKXkdTpkqg4p\n"
                     "CGzx+37Ubh608IKoiArMDZQeA3mCKv+Kb4ga3bPd7LHHnbiuocvdhKSVlOLN7PEy\n"
                     "d7wqbQykredDE/0ao3OBMufoiAH/cUhShKGKFhppUpsBEV3Liqvb/pkJZl0GMWc7\n"
                     "h7Cs+S2yeMH2BLFPkPRFi9l9DTheyxCAf0uia/pvj+sXwEW8S0V8Gn6D/VrC7xH4\n"
                     "r+lPy3A+w0mb5FcB32V1hY+hTYu7hmpuaCYDKFkuHDD2nGImmkPMekkj/pxkYT+h\n"
                     "zgd/gvXV3dSvR6edjmKSlwSFYHMERc2uK5lqkZHiiisxZpwIrQIDAQABo1MwUTAd\n"
                     "BgNVHQ4EFgQUEFD59xQdvmoaNQ773gHLLyeCVvkwHwYDVR0jBBgwFoAUEFD59xQd\n"
                     "vmoaNQ773gHLLyeCVvkwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOC\n"
                     "AQEARPFj3BeEhUrWhbjiUwmZ45GUleQKghmGcvTjw6rCNka5xRW5CN2JaIxQIhge\n"
                     "L4E+59bXl0fNBXSN4Th3dC5rb0ZHPIaI/rIR882qbRHdTqcyzANpaAD8GgLRVH0w\n"
                     "Ubp5UdaTutXNfgbfnTfm1UMfIRDuJp7kKvIiLosN2XS/F0uZaC9Hoc8+cfPqEU6F\n"
                     "Px4kw9nmszFvZK33DNLmQv8kXGtaPfqh1vbUtEp89cdHJhRd9lkhiZMFpfKaeRXq\n"
                     "VSjr+QQzboEBIwucuYuUVrxd+CNGklsLRLx8FFuTgcmQ2XVIs7HdfW+yBGaYSqpi\n"
                     "rB31QY551pp4i/SKRkq6oKhgmQ==\n"
                     "-----END CERTIFICATE-----\n");
        UNUSED(ca);
        //const struct mg_tls_opts opts = {.ca = ca, .name = name};
        const struct mg_tls_opts opts = {.name = name};      
        mg_tls_init(connection, &opts);

        mg_printf(
            connection,
            "GET %s HTTP/1.0\r\nHost: %.*s\r\n\r\n",
            mg_url_uri(HTTP_URL),
            name.len,
            name.buf);

    } else if(event == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = event_data;
        FURI_LOG_I(TAG, "Data received: %.*s", (int)hm->message.len, hm->message.buf);

        connection->is_draining = 1;
        instance->done = true;

    } else if(event == MG_EV_ERROR) {
        FURI_LOG_E(TAG, "Error occurred: %s", (char*)event_data);
        instance->done = true;
    } else if(event == MG_EV_TLS_HS) {
        FURI_LOG_I(TAG, "TLS handshake successful");
    }
}

static HttpTestApp* http_test_alloc(void) {
    HttpTestApp* instance = malloc(sizeof(HttpTestApp));

    instance->network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(instance->network);

    mg_log_set(MG_LL_VERBOSE);

    mg_mgr_init(&instance->mgr);
    mg_http_connect(&instance->mgr, HTTP_URL, http_test_mg_handler, instance);

    return instance;
}

static void http_test_free(HttpTestApp* instance) {
    mg_mgr_free(&instance->mgr);

    network_deinit_current_thread(instance->network);
    furi_record_close(RECORD_NETWORK);

    free(instance);
}

int https_test_app(void* arg) {
    UNUSED(arg);

    HttpTestApp* instance = http_test_alloc();

    while(!instance->done) {
        mg_mgr_poll(&instance->mgr, 1000);
    }

    http_test_free(instance);
    return 0;
}
