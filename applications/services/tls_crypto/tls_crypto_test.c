#include <cli/cli_ansi.h>
#include <cli/cli_command.h>

#include <containers/pipe.h>

#include <furi_hal_random.h>

#include <tls_crypto/tls_crypto.h>

#define INPUT_DATA_LEN (TLS_CRYPTO_DATA_LEN_MAX)

#define ERROR_COUNT_MAX (64) // An arbitrary round number
#define RETRY_COUNT_MAX (10)

void cli_command_tls_crypto_test(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    TlsCrypto* tls_crypto = furi_record_open(RECORD_TLS_CRYPTO);

    uint8_t input_data[INPUT_DATA_LEN];
    TlsCryptoSignature signature;

    uint32_t error_count = 0;
    uint32_t retry_count = 0;

    uint32_t start_time = furi_get_tick();

    while(retry_count < RETRY_COUNT_MAX && !cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
        furi_hal_random_fill_buf(input_data, sizeof(input_data));

        const TlsCryptoStatus status = tls_crypto_sign(
            tls_crypto, TlsCryptoKeyIdDevice, input_data, sizeof(input_data), &signature);

        if(status != TlsCryptoStatusOk) {
            printf(ANSI_FG_RED "Failed to sign data\r\n" ANSI_RESET);
            ++error_count;
        } else {
            if(error_count) {
                printf(ANSI_FG_GREEN "Recovered after %lu errors\r\n" ANSI_RESET, error_count);
            }
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

    furi_record_close(RECORD_TLS_CRYPTO);
}
