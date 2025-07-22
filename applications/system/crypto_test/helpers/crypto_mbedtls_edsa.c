#include "crypto_mbedtls_edsa.h"
#include "crypto_common.h"

#include <cli/cli_ansi.h>
//top#include <sl_mbedtls.h>
#include "psa/crypto.h"
#include "sl_si91x_psa_wrap.h"

#define TAG "Crypto_MbedTLS_EDSA"

#define PUBLIC_KEY_SIZE_P192R1       49
#define PUBLIC_KEY_SIZE_P224R1       57
#define PUBLIC_KEY_SIZE_P256R1       65
#define PRIVATE_KEY_SIZE_P192R1      24
#define PRIVATE_KEY_SIZE_P224R1      28
#define PRIVATE_KEY_SIZE_P256R1      32
#define SIGNATURE_SIZE_P192R1        48
#define SIGNATURE_SIZE_P224R1        56
#define SIGNATURE_SIZE_P256R1        64
#define PRIVATE_KEY_SIZE_P192R1_BITS 192
#define PRIVATE_KEY_SIZE_P224R1_BITS 224
#define PRIVATE_KEY_SIZE_P256R1_BITS 256

#define WRAP_INPUT_KEYS     1 // Enable this if the input private key needs to be wrapped before use
#define IMPORT_WRAPPED_KEYS 0 // Enable this if the input key is wrapped

static const unsigned char input_data[] = {0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
                                           0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
                                           0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
                                           0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1};

// static const unsigned char private_key[] = {0x95, 0xCD, 0x3A, 0x36, 0x25, 0xD6, 0xF6, 0x06,
//                                             0xBD, 0xC8, 0x64, 0x77, 0x8D, 0x4A, 0xA6, 0x50,
//                                             0xC2, 0xD7, 0x9A, 0x05, 0x94, 0xDD, 0x10, 0xCF,
//                                             0x4C, 0x47, 0x4B, 0x83, 0xD2, 0x87, 0x0D, 0x1A};

static const uint8_t private_key[] = {0x41, 0x9c, 0x9c, 0x80, 0x33, 0x6c, 0x40, 0x1f,
                                      0xd2, 0x06, 0x49, 0x59, 0x8b, 0xb6, 0x5f, 0xb3,
                                      0xd8, 0xd8, 0xef, 0xd5, 0xeb, 0x4a, 0xe1, 0xe8,
                                      0x8a, 0x63, 0x36, 0x81, 0xcb, 0x0a, 0x21, 0x07};
static const uint8_t public_key_check[] = {
    0x04, 0xb6, 0xcd, 0x40, 0x84, 0x9a, 0xf6, 0xc4, 0xc2, 0x2b, 0x57, 0x99, 0x86,
    0xa7, 0x7d, 0xfa, 0x19, 0x61, 0xaa, 0xe2, 0x6e, 0x60, 0xe6, 0x83, 0x82, 0x11,
    0xeb, 0xe5, 0xd1, 0x40, 0x79, 0x22, 0x25, 0xe4, 0x12, 0x40, 0xfe, 0x30, 0xec,
    0x63, 0x88, 0xab, 0x35, 0xaf, 0xb6, 0x34, 0xd8, 0x76, 0x03, 0xef, 0x81, 0xb8,
    0x11, 0x7d, 0x90, 0x43, 0xf6, 0x7e, 0x0a, 0x73, 0x01, 0xbd, 0x48, 0x5e, 0x7f};

void crypto_mbedtls_edsa_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    //sl_mbedtls_init();

    uint8_t public_key[PUBLIC_KEY_SIZE_P256R1]; // Uncompressed point format
    size_t pubkey_len;
    uint8_t signature_buf[SIGNATURE_SIZE_P256R1]; // DER format
    size_t signature_len;

    psa_status_t ret;
    psa_key_id_t key_id;
    psa_key_attributes_t key_attr;

    /* psa crypto library initialization */
    ret = psa_crypto_init();
    if(ret != PSA_SUCCESS) {
        printf(
            ANSI_FG_RED "PSA crypto library initialization failed with error: %ld\r\n" ANSI_RESET,
            ret);
    } else {
        printf(ANSI_FG_GREEN "PSA crypto library initialization Success\r\n" ANSI_RESET);
    }

    /* Import (private key) and Generate (public key) ECC key pair */

    // Set up attributes for a volatile private key
    key_attr = psa_key_attributes_init();
    psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(
        &key_attr, PRIVATE_KEY_SIZE_P256R1_BITS); // Set PRIVATE_KEY_SIZE_P192R1_BITS for secp192r1
    psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE);
    psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));

    // Import a private key
    ret = psa_import_key(&key_attr, private_key, sizeof(private_key), &key_id);
    if(ret != PSA_SUCCESS) {
        printf(ANSI_FG_RED "Import Key failed with error: status %ld\r\n" ANSI_RESET, ret);
    } else {
        printf(ANSI_FG_GREEN "Import Key success\r\n" ANSI_RESET);
    }

    // Export a public key from a volatile private key
    ret = psa_export_public_key(key_id, public_key, sizeof(public_key), &pubkey_len);
    if(ret != PSA_SUCCESS) {
        printf(
            ANSI_FG_RED
            "Exporting a Public Key from Private key Failed with error: %ld\r\n" ANSI_RESET,
            ret);
    } else {
        printf(ANSI_FG_GREEN "Export Public Key from Private Key Success\r\n" ANSI_RESET);
    }

    // Destroy the private key
    ret = psa_destroy_key(key_id);
    if(ret != PSA_SUCCESS) {
        printf(ANSI_FG_RED "Destroy Key failed with error : %ld\r\n" ANSI_RESET, ret);
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
        &key_attr, PRIVATE_KEY_SIZE_P256R1_BITS); // Set PRIVATE_KEY_SIZE_P192R1_BITS for secp192r1
    psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE);
    psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
#if WRAP_INPUT_KEYS
    psa_set_key_lifetime(
        &key_attr,
        PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(
            PSA_KEY_PERSISTENCE_VOLATILE, PSA_KEY_VOLATILE_PERSISTENT_WRAP_IMPORT));
#elif IMPORT_WRAPPED_KEYS
    psa_set_key_lifetime(
        &key_attr,
        PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(
            PSA_KEY_PERSISTENCE_VOLATILE, PSA_KEY_VOLATILE_PERSISTENT_WRAPPED));
#endif

    // Import a private key
    ret = psa_import_key(&key_attr, private_key, sizeof(private_key), &key_id);
    if(ret != PSA_SUCCESS) {
        printf(ANSI_FG_RED "Import Key failed with error: status %ld\r\n" ANSI_RESET, ret);
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
        printf(ANSI_FG_RED "Sign Message with Private key Failed with error: %ld\r\n" ANSI_RESET, ret);
    } else {
        printf(ANSI_FG_GREEN "Sign Message with Private Key Success\r\n" ANSI_RESET);
    }

    // Destroy the wrapped/plain private key
    ret = psa_destroy_key(key_id);
    if(ret != PSA_SUCCESS) {
        printf(ANSI_FG_RED "Destroy Key failed with error : %ld\r\n" ANSI_RESET, ret);
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
        printf(ANSI_FG_RED "Import Public Key failed with error : %ld\r\n" ANSI_RESET, ret);
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
        printf(ANSI_FG_RED "Signature Verification with Public Key failed with error: %ld\r\n" ANSI_RESET, ret);
    } else {
        printf(ANSI_FG_GREEN "Signature Verification with Public Key Success\r\n" ANSI_RESET);
    }

    // Destroy public key
    ret = psa_destroy_key(key_id);
    if(ret != PSA_SUCCESS) {
        printf(ANSI_FG_RED "Destroy Key failed with error : %ld\r\n" ANSI_RESET, ret);
    } else {
        printf(ANSI_FG_GREEN "Destroy Key Success\r\n" ANSI_RESET);
    }

    printf("Crypto MbedTLS ECDSA done\r\n");
}
