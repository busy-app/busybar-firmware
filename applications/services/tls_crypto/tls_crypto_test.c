#include <cli/cli_ansi.h>
#include <cli/cli_command.h>

#include <containers/pipe.h>

#include <furi_hal_random.h>

#include <tls_crypto/tls_crypto.h>

#define INPUT_DATA_LEN (TLS_CRYPTO_DATA_LEN_MAX)

void cli_command_tls_crypto_test(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    TlsCrypto* tls_crypto = furi_record_open(RECORD_TLS_CRYPTO);

    uint8_t input_data[INPUT_DATA_LEN];
    TlsCryptoSignature signature;

    while(!cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
        furi_hal_random_fill_buf(input_data, sizeof(input_data));

        const TlsCryptoStatus status = tls_crypto_sign(
            tls_crypto, TlsCryptoKeyIdDevice, input_data, sizeof(input_data), &signature);

        if(status != TlsCryptoStatusOk) {
            printf(ANSI_FG_RED "Failed to sign data: %d\r\n" ANSI_RESET, status);
        }
    }

    furi_record_close(RECORD_TLS_CRYPTO);
}
