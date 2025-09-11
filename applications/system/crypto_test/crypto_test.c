#include "crypto_test.h"

#include <furi.h>
#include <strint.h>

#include "helpers/crypto_aes.h"
#include "helpers/crypto_sha.h"
#include "helpers/crypto_ecdsa.h"
#include "helpers/crypto_hmac.h"
#include "helpers/crypto_mbedtls_edsa.h"
#include "helpers/crypto_csr.h"

#include <cli/args.h>
#include <cli/cli_command.h>
#include <cli/shell/cli_shell.h>
#include <cli/cli_ansi.h>

#include <wifi/wifi_common.h>

#define TAG "CryptoTestApp"

struct CryptoTestApp {
    FuriString* msg;
    CliShell* shell;
};

void crypto_test_app_stop(void* app_handle);

static CryptoTestApp* crypto_test_app_instance = NULL;

void* crypto_test_app_start(CliShell* shell) {
    FURI_LOG_I(TAG, "Starting");

    furi_record_open(RECORD_WIFI);

    crypto_test_app_instance = malloc(sizeof(CryptoTestApp));
    crypto_test_app_instance->msg = furi_string_alloc();
    crypto_test_app_instance->shell = shell;

    return crypto_test_app_instance;
}

void crypto_test_app_stop(void* app_handle) {
    furi_check(app_handle);
    FURI_LOG_I(TAG, "Stopping");
    CryptoTestApp* instance = (CryptoTestApp*)app_handle;

    if(instance) {
        furi_string_free(instance->msg);
        free(instance);
        instance = NULL;
    }

    furi_record_close(RECORD_WIFI);
}

static void crypto_test_motd(void* context) {
    UNUSED(context);
    printf("\r\n+-------------------------------+\r\n");
    printf("| Welcome to crypto test shell! |\r\n");
    printf("+-------------------------------+\r\n\r\n");
}

void crypto_test_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    CliRegistry* registry = cli_registry_alloc();
    cli_registry_add_command(registry, "aes", CliCommandFlagDefault, crypto_aes_command, NULL);
    cli_registry_add_command(registry, "ecdsa", CliCommandFlagDefault, crypto_ecdsa_command, NULL);
    cli_registry_add_command(registry, "hmac", CliCommandFlagDefault, crypto_hmac_command, NULL);
    cli_registry_add_command(registry, "sha", CliCommandFlagDefault, crypto_sha_command, NULL);
    cli_registry_add_command(
        registry, "mbedtls_edsa", CliCommandFlagDefault, crypto_mbedtls_edsa_command, NULL);
    cli_registry_add_command(registry, "csr", CliCommandFlagDefault, crypto_csr_command, NULL);

    CliShell* shell = cli_shell_alloc(crypto_test_motd, NULL, pipe, registry, NULL);
    cli_shell_set_prompt(shell, "crypto_test");

    CryptoTestApp* app = crypto_test_app_start(shell);
    cli_shell_start(shell);
    cli_shell_join(shell);
    crypto_test_app_stop(app);

    cli_shell_free(shell);
    cli_registry_free(registry);
}
