#include <cli/cli_ansi.h>
#include <cli/cli_command.h>

#include <containers/pipe.h>

#include <furi_hal_random.h>

#include <tls_crypto/tls_crypto_client.h>

#define INPUT_DATA_SIZE    (800)
#define SIGNATURE_BUF_SIZE (128)

#define ERROR_COUNT_MAX (64) // An arbitrary round number
#define RETRY_COUNT_MAX (10)

void cli_command_tls_crypto_test(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    TlsCryptoClient* tls = furi_record_open(RECORD_TLS_CRYPTO_CLIENT);

    uint8_t input_data[INPUT_DATA_SIZE];
    uint8_t signature_buf[SIGNATURE_BUF_SIZE];
    size_t signature_len;

    uint32_t error_count = 0;
    uint32_t retry_count = 0;

    uint32_t start_time = furi_get_tick();

    while(retry_count < RETRY_COUNT_MAX && !cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
        furi_hal_random_fill_buf(input_data, sizeof(input_data));

        if(!tls_crypto_client_sign(
               tls,
               1,
               input_data,
               sizeof(input_data),
               signature_buf,
               sizeof(signature_buf),
               &signature_len)) {
            printf(ANSI_FG_RED "Failed to sign data\r\n" ANSI_RESET);
            ++error_count;
        } else {
            error_count = 0;
        }

        if(error_count >= ERROR_COUNT_MAX) {
            const uint32_t now = furi_get_tick();
            const uint32_t end_time_s = (now - start_time) / 1000;

            start_time = now;
            error_count = 0;
            ++retry_count;

            printf(
                ANSI_FG_YELLOW "Max error count reached in %lu s, retrying ...\r\n" ANSI_RESET,
                end_time_s);
            furi_delay_ms(1000);
        }
    }

    furi_record_close(RECORD_TLS_CRYPTO_CLIENT);
}
