#include <cli/cli_ansi.h>
#include <cli/cli_command.h>

#include <containers/pipe.h>

#include <furi_hal_random.h>

#include <tls_crypto/tls_crypto_client.h>

void cli_command_tls_crypto_test(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    TlsCryptoClient* tls = furi_record_open(RECORD_TLS_CRYPTO_CLIENT);

    uint8_t hash[800];
    uint8_t buf[128];
    size_t sign_len;

    while(!cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
        furi_hal_random_fill_buf(hash, sizeof(hash));

        if(!tls_crypto_client_sign(tls, 1, hash, sizeof(hash), buf, sizeof(buf), &sign_len)) {
            printf(ANSI_FG_RED "Failed to sign data\r\n" ANSI_RESET);
        }
    }

    furi_record_close(RECORD_TLS_CRYPTO_CLIENT);
}
