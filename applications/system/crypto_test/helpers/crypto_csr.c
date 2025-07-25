#include "crypto_csr.h"

#include <mbedtls/x509_crt.h>
#include <mbedtls/x509_csr.h>
#include <psa/crypto.h>

#include <furi_hal_crypto.h>
#include <cli/cli_ansi.h>

#define TAG "Crypto_CSR"

static const uint8_t private_key[FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256] = {
    0xb4, 0x0f, 0x54, 0xf1, 0x50, 0x15, 0xc3, 0x3c, 0xfd, 0xea, 0xa6,
    0xc2, 0x4d, 0x58, 0x0f, 0x1b, 0x80, 0x56, 0xbe, 0xcf, 0xf9, 0xdd,
    0x1a, 0x07, 0x1c, 0x0f, 0x76, 0x86, 0x92, 0x37, 0xcb, 0xab};

void crypto_csr_wrap(uint8_t* key, size_t key_size, uint8_t* wrapped_key) {
    furi_check(key_size == FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256);
    furi_hal_crypto_wrap_key(FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256, key, wrapped_key);
}

void crypto_csr_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    psa_key_id_t key_id;
    mbedtls_pk_context key_ctx;
    mbedtls_x509write_csr csr_ctx;

    size_t max_size = 2048;
    uint8_t* buffer = malloc(max_size);
    psa_key_attributes_t key_attr;

    /* psa crypto library initialization */
    psa_status_t psa_status = psa_crypto_init();
    if(psa_status != PSA_SUCCESS) {
        printf(
            ANSI_FG_RED
            "PSA crypto library initialization failed with error: 0x%08lX\r\n" ANSI_RESET,
            psa_status);
    } else {
        printf(ANSI_FG_GREEN "PSA crypto library initialization Success\r\n" ANSI_RESET);
    }

    // import key
    key_attr = psa_key_attributes_init();
    psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&key_attr, FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256_BITS);
    psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(
        &key_attr,
        PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_SIGN_MESSAGE |
            PSA_KEY_USAGE_VERIFY_MESSAGE);

    // Import a private key
    psa_status = psa_import_key(&key_attr, private_key, sizeof(private_key), &key_id);
    if(psa_status != PSA_SUCCESS) {
        printf(
            ANSI_FG_RED "Import Key failed with error: status 0x%08lX\r\n" ANSI_RESET, psa_status);
    } else {
        printf(ANSI_FG_GREEN "Import Key success\r\n" ANSI_RESET);
    }

    // Generate CSR
    printf("Generating CSR...\r\n");
    // 1 initialize CSR context
    mbedtls_x509write_csr_init(&csr_ctx);

    // 2 Subject name
    char subject_name[512];
#if MATTER_X509_EXTENSIONS
    snprintf(
        subject_name,
        sizeof(subject_name),
        "CN=%s,VID=%04X,PID=%04X",
        common_name,
        vendor_id,
        product_id);
#else
    snprintf(
        subject_name,
        sizeof(subject_name),
        "CN=%s,serialNumber=%s,commonName=%s",
        "test",
        "12345",
        "11test111");
#endif

    int err = mbedtls_x509write_csr_set_subject_name(&csr_ctx, subject_name);
    if(err != 0) {
        printf(ANSI_FG_RED "Failed to set subject name: %d\r\n" ANSI_RESET, err);
    } else {
        printf(ANSI_FG_GREEN "Subject name set to: %s\r\n" ANSI_RESET, subject_name);
    }

    // 3 set algorithm
    mbedtls_x509write_csr_set_md_alg(&csr_ctx, MBEDTLS_MD_SHA256);

    // 4 set key
    mbedtls_pk_init(&key_ctx);
    err = mbedtls_pk_setup_opaque(&key_ctx, key_id);
    if(err != 0) {
        printf(ANSI_FG_RED "Failed to setup key context: %d\r\n" ANSI_RESET, err);
    } else {
        printf(ANSI_FG_GREEN "Key context setup successful\r\n" ANSI_RESET);
    }

    // Signing key
    mbedtls_x509write_csr_set_key(&csr_ctx, &key_ctx);

    // 5 Generate CSR format PEM
    err = mbedtls_x509write_csr_pem(&csr_ctx, (uint8_t*)buffer, max_size, NULL, NULL);
    if(err != 0) {
        printf(ANSI_FG_RED "Failed to write CSR in PEM format: %d\r\n" ANSI_RESET, err);
    } else {
        printf(ANSI_FG_GREEN "CSR in PEM format generated successfully\r\n" ANSI_RESET);
        printf("CSR in PEM format:\r\n\r\n%s\r\n", buffer);
    }

    // 5 Generate CSR format DER
    int len_or_err = mbedtls_x509write_csr_der(&csr_ctx, (uint8_t*)buffer, max_size, NULL, NULL);
    if(len_or_err < 0) {
        printf(ANSI_FG_RED "Failed to write CSR in DER format: %d\r\n" ANSI_RESET, len_or_err);
    } else {
        printf(ANSI_FG_GREEN "CSR in DER format generated successfully\r\n" ANSI_RESET);
        printf("CSR in DER format, size: %d bytes\r\n\r\n", len_or_err);
        for(int i = 0; i < len_or_err; i++) {
            printf("%02X", *(buffer + (max_size - len_or_err) + i));
            if((i + 1) % 16 == 0) {
                printf("\r\n");
            } else {
                printf(" ");
            }
        }
        printf("\r\n");
    }

    // 6 free resources
    mbedtls_x509write_csr_free(&csr_ctx);
    mbedtls_pk_free(&key_ctx);
    psa_destroy_key(key_id);

    free(buffer);
}
