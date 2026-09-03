#include "ca_storage.h"

#include <mbedtls/psa_util.h>

#include <storage/storage.h>

#define CA_STORAGE_BUNDLE_PATH   SHARED_ASSETS_PATH("ca/cacert.pem")
#define CA_STORAGE_MAX_FILE_SIZE (350000LLU) // Limit max file size to 350KB

#define TAG "CaStorage"

struct CaStorage {
    mbedtls_x509_crt cert_chain_head;
};

static uint8_t* ca_storage_load_pem_bundle(size_t* data_size) {
    uint8_t* data = NULL;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, CA_STORAGE_BUNDLE_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Failed to open CA bundle file: %s", CA_STORAGE_BUNDLE_PATH);
            break;
        }

        const uint64_t file_size = storage_file_size(file);

        if(file_size == 0) {
            FURI_LOG_E(TAG, "CA bundle file is empty");
            break;
        }

        if(file_size > CA_STORAGE_MAX_FILE_SIZE) {
            FURI_LOG_E(
                TAG, "CA bundle file size exceeds %llu KB", CA_STORAGE_MAX_FILE_SIZE / 1000);
            break;
        }

        data = malloc(file_size + 1);

        const uint64_t read_size = storage_file_read(file, data, file_size);

        if(read_size != file_size) {
            FURI_LOG_E(
                TAG,
                "Failed to read CA bundle file: expected %llu, read %llu bytes",
                file_size,
                read_size);

            free(data);
            data = NULL;

            break;
        }

        data[file_size] = '\0';
        *data_size = file_size + 1;

        FURI_LOG_D(TAG, "Load CA bundle file OK");

    } while(false);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return data;
}

static CaStorage* ca_storage_alloc(void) {
    CaStorage* instance = malloc(sizeof(CaStorage));

    psa_crypto_init();

    mbedtls_x509_crt_init(&instance->cert_chain_head);

    size_t data_size;
    uint8_t* data = ca_storage_load_pem_bundle(&data_size);

    if(data != NULL) {
        const int parse_status =
            mbedtls_x509_crt_parse(&instance->cert_chain_head, data, data_size);

        if(parse_status == 0) {
            FURI_LOG_I(TAG, "Load CA bundle OK");
        } else if(parse_status > 0) {
            FURI_LOG_W(TAG, "Failed to parse %d certificates in the chain", parse_status);
        } else {
            FURI_LOG_E(TAG, "Failed to parse CA bundle file: -0x%04X", -parse_status);
        }

        free(data);
    }

    return instance;
}

const mbedtls_x509_crt* ca_storage_get_cert_chain(const CaStorage* instance) {
    furi_check(instance);
    return &instance->cert_chain_head;
}

void ca_storage_on_system_start(void) {
    CaStorage* instance = ca_storage_alloc();
    furi_record_create(RECORD_CA_STORAGE, instance);
}
