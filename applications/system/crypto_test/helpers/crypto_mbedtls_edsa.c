#include "crypto_mbedtls_edsa.h"
#include "crypto_common.h"

#include <cli/cli_ansi.h>
#include <furi_hal_crypto.h>
#include <psa/crypto.h>
#include <sl_si91x_psa_wrap.h>

#define TAG "Crypto_MbedTLS_EDSA"

#define WRAP_INPUT_KEYS     0 // Enable this if the input private key needs to be wrapped before use
#define IMPORT_WRAPPED_KEYS 1 // Enable this if the input key is wrapped

static const unsigned char input_data[] = {0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
                                           0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
                                           0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
                                           0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1};

static const uint8_t private_key_init[FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256] = {
    0xb4, 0x0f, 0x54, 0xf1, 0x50, 0x15, 0xc3, 0x3c, 0xfd, 0xea, 0xa6,
    0xc2, 0x4d, 0x58, 0x0f, 0x1b, 0x80, 0x56, 0xbe, 0xcf, 0xf9, 0xdd,
    0x1a, 0x07, 0x1c, 0x0f, 0x76, 0x86, 0x92, 0x37, 0xcb, 0xab};
static const uint8_t public_key_check[FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_256] = {
    0x04, 0x56, 0x76, 0x7d, 0xe5, 0xfc, 0x3e, 0x14, 0x5a, 0x4f, 0xc5, 0x83, 0xb5,
    0xbc, 0x18, 0x52, 0x0b, 0xed, 0xbd, 0x76, 0xea, 0x03, 0xcf, 0xca, 0x9c, 0xea,
    0x0f, 0xe2, 0x56, 0xa3, 0x81, 0x89, 0xd5, 0x17, 0x5f, 0x44, 0x10, 0xa7, 0x8c,
    0x4f, 0x57, 0x75, 0x77, 0xa8, 0x3e, 0x51, 0x1b, 0xa3, 0x07, 0xb5, 0x35, 0xb1,
    0x0c, 0xc2, 0x26, 0x72, 0x41, 0xe5, 0x4c, 0x25, 0x0d, 0x44, 0xaf, 0xa4, 0x0b};

void crypto_mbedtls_edsa_wrap(uint8_t* key, size_t key_size, uint8_t* wrapped_key) {
    furi_check(key_size == FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256);
    furi_hal_crypto_wrap_key(FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256, key, wrapped_key);
}

//sli_si91x_crypto_wrap_key(key, key_size, SL_SI91X_WRAP_IV_CBC_MODE, WRAP_IV);

void crypto_mbedtls_edsa_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    uint8_t private_key[FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256]; // Uncompressed point format
    uint8_t public_key[FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_256]; // Uncompressed point format
    size_t pubkey_len;
    uint8_t signature_buf[FURI_HAL_CRYPTO_ECDSA_MAX_SIGNATURE_SIZE]; // DER format
    size_t signature_len;

    psa_status_t ret;
    psa_key_id_t key_id;
    psa_key_attributes_t key_attr;

    memcpy(private_key, private_key_init, sizeof(private_key_init));

    /* psa crypto library initialization */
    ret = psa_crypto_init();
    if(ret != PSA_SUCCESS) {
        printf(
            ANSI_FG_RED
            "PSA crypto library initialization failed with error: 0x%08lX\r\n" ANSI_RESET,
            ret);
    } else {
        printf(ANSI_FG_GREEN "PSA crypto library initialization Success\r\n" ANSI_RESET);
    }

    /* Import (private key) and Generate (public key) ECC key pair */

    // Set up attributes for a volatile private key
    key_attr = psa_key_attributes_init();
    psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(
        &key_attr,
        FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256_BITS); // Set PRIVATE_KEY_SIZE_P192R1_BITS for secp192r1
    psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE);
    psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));

    // Import a private key
    ret = psa_import_key(&key_attr, private_key, sizeof(private_key), &key_id);
    if(ret != PSA_SUCCESS) {
        printf(ANSI_FG_RED "Import Key failed with error: status 0x%08lX\r\n" ANSI_RESET, ret);
    } else {
        printf(ANSI_FG_GREEN "Import Key success\r\n" ANSI_RESET);
    }

    // Export a public key from a volatile private key
    ret = psa_export_public_key(key_id, public_key, sizeof(public_key), &pubkey_len);
    if(ret != PSA_SUCCESS) {
        printf(
            ANSI_FG_RED
            "Exporting a Public Key from Private key Failed with error: 0x%08lX\r\n" ANSI_RESET,
            ret);
    } else {
        printf(ANSI_FG_GREEN "Export Public Key from Private Key Success\r\n" ANSI_RESET);
    }

    // Destroy the private key
    ret = psa_destroy_key(key_id);
    if(ret != PSA_SUCCESS) {
        printf(ANSI_FG_RED "Destroy Key failed with error : 0x%08lX\r\n" ANSI_RESET, ret);
    } else {
        printf(ANSI_FG_GREEN "Destroy Key Success\r\n" ANSI_RESET);
    }

    printf("Public Key: ");
    for(uint32_t i = 0; i < pubkey_len; i++) {
        printf("%02X ", public_key[i]);
    }
    printf("\r\n");

    if(memcmp(public_key, public_key_check, sizeof(public_key)) == 0) {
        printf(ANSI_FG_GREEN "Public Key generated and Public Key Check match\r\n" ANSI_RESET);
    } else {
        printf(ANSI_FG_RED "Public Key and Public Key Check do not match\r\n" ANSI_RESET);
    }

    /* Import private key and generate signature */

    // Set up attributes for a volatile private key
    key_attr = psa_key_attributes_init();
    psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(
        &key_attr,
        FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256_BITS); // Set PRIVATE_KEY_SIZE_P192R1_BITS for secp192r1
    psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE);
    psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
#if WRAP_INPUT_KEYS
    psa_set_key_lifetime(
        &key_attr,
        PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(
            PSA_KEY_PERSISTENCE_VOLATILE, PSA_KEY_VOLATILE_PERSISTENT_WRAP_IMPORT));

#elif IMPORT_WRAPPED_KEYS
    uint8_t wrapped_key[FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256];
    memset(wrapped_key, 0, sizeof(wrapped_key));
    crypto_mbedtls_edsa_wrap(private_key, sizeof(private_key), wrapped_key);
    memcpy(private_key, wrapped_key, sizeof(wrapped_key));

    printf("Wrapped Key: ");
    for(uint32_t i = 0; i < sizeof(private_key); i++) {
        printf("%02X ", private_key[i]);
    }
    printf("\r\n");

    psa_set_key_lifetime(
        &key_attr,
        PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(
            PSA_KEY_PERSISTENCE_VOLATILE, PSA_KEY_VOLATILE_PERSISTENT_WRAPPED));
#endif
    // Import a private key
    ret = psa_import_key(&key_attr, private_key, sizeof(private_key), &key_id);
    if(ret != PSA_SUCCESS) {
        printf(ANSI_FG_RED "Import Key failed with error: status 0x%08lX\r\n" ANSI_RESET, ret);
    } else {
        printf(ANSI_FG_GREEN "Import Key success\r\n" ANSI_RESET);
    }

    // Sign a message with a volatile private key
    ret = psa_sign_message(
        key_id,
        PSA_ALG_ECDSA(PSA_ALG_SHA_256),
        input_data,
        sizeof(input_data),
        signature_buf,
        sizeof(signature_buf),
        &signature_len);

    if(ret != PSA_SUCCESS) {
        printf(
            ANSI_FG_RED "Sign Message with Private key Failed with error: 0x%08lX\r\n" ANSI_RESET,
            ret);
    } else {
        printf(ANSI_FG_GREEN "Sign Message with Private Key Success\r\n" ANSI_RESET);
    }

    // Destroy the wrapped/plain private key
    ret = psa_destroy_key(key_id);
    if(ret != PSA_SUCCESS) {
        printf(ANSI_FG_RED "Destroy Key failed with error : 0x%08lX\r\n" ANSI_RESET, ret);
    } else {
        printf(ANSI_FG_GREEN "Destroy Key Success\r\n" ANSI_RESET);
    }

    /* Import public key and verify signature */

    // Set up attributes for the generated public key
    key_attr = psa_key_attributes_init();
    psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_PUBLIC_KEY(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_VERIFY_MESSAGE);
    psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));

    // Import public key
    ret = psa_import_key(&key_attr, public_key, sizeof(public_key), &key_id);
    if(ret != PSA_SUCCESS) {
        printf(ANSI_FG_RED "Import Public Key failed with error : 0x%08lX\r\n" ANSI_RESET, ret);
    } else {
        printf(ANSI_FG_GREEN "Import Public Key Success\r\n" ANSI_RESET);
    }

    // Verify signature with public key
    ret = psa_verify_message(
        key_id,
        PSA_ALG_ECDSA(PSA_ALG_SHA_256),
        input_data,
        sizeof(input_data),
        signature_buf,
        signature_len);
    if(ret != PSA_SUCCESS) {
        printf(
            ANSI_FG_RED
            "Signature Verification with Public Key failed with error: 0x%08lX\r\n" ANSI_RESET,
            ret);
    } else {
        printf(ANSI_FG_GREEN "Signature Verification with Public Key Success\r\n" ANSI_RESET);
    }

    // Destroy public key
    ret = psa_destroy_key(key_id);
    if(ret != PSA_SUCCESS) {
        printf(ANSI_FG_RED "Destroy Key failed with error : 0x%08lX\r\n" ANSI_RESET, ret);
    } else {
        printf(ANSI_FG_GREEN "Destroy Key Success\r\n" ANSI_RESET);
    }

    printf("Crypto MbedTLS ECDSA done\r\n");
}
