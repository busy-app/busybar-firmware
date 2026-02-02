#include <furi.h>
#include <furi_hal_random.h>

#include <psa/crypto.h>
#include <mbedtls/ssl.h>

#define TAG "mbedtls"

psa_status_t mbedtls_psa_external_get_random(
    mbedtls_psa_external_random_context_t* context,
    uint8_t* output,
    size_t output_size,
    size_t* output_length) {
    UNUSED(context);

    furi_hal_random_fill_buf(output, output_size);
    *output_length = output_size;

    return PSA_SUCCESS;
}
