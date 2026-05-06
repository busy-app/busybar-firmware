#include "../unit_tests.h"
#include <furi.h>
#include <containers/pipe.h>
#include <containers/pipe_util.h>
#include <cli_intercom/cli_intercom.h>
#include <string.h>

#define PIPE_SIZE          512U
#define CLI_PROMPT         "\r\n917>: "
#define CRYPTO_TEST_PROMPT "\r\ncrypto_test>: "
#define TAG                "CryptoTest"

static bool expect(PipeSide* pipe, const char* expected) {
    FURI_LOG_D(TAG, "expecting: \"%s\"", expected);
    const char* terminators[3] = {
        expected,
        CLI_PROMPT,
        CRYPTO_TEST_PROMPT,
    };
    PipeDiscardUntilResult result = pipe_discard_until_either(pipe, terminators, 3);

    if(!result.success) {
        FURI_LOG_E(TAG, "broken pipe");
    } else if(result.found_idx != 0) {
        FURI_LOG_E(TAG, "recevied cli prompt");
    }

    return result.success && result.found_idx == 0;
}

static bool send_cmd(PipeSide* pipe, const char* cmd, const char* prompt) {
    if(!expect(pipe, prompt)) {
        return false;
    }

    FURI_LOG_D(TAG, "sending cmd: \"%s\"", cmd);
    bool success = false;
    do {
        size_t len = strlen(cmd);
        if(pipe_send(pipe, cmd, strlen(cmd)) != len) {
            break;
        }
        if(pipe_send(pipe, "\r", 1) != 1) {
            break;
        }
        success = true;
    } while(false);
    if(!success) {
        FURI_LOG_E(TAG, "error sending cmd");
    }
    return success;
}

typedef void (*CliTestFn)(PipeSide* pipe);

static void do_crypto_echo_test(PipeSide* pipe) {
    mu_check(send_cmd(pipe, "echo hello", CLI_PROMPT));
    if(expect(pipe, "\r\nhello")) {
        mu_check(send_cmd(pipe, "exit", CLI_PROMPT));
    } else {
        mu_check(send_cmd(pipe, "exit", ""));
        mu_fail("echo command failed");
    }
}

static void run_crypto_test_command(PipeSide* pipe, const char* cmd) {
    mu_check(send_cmd(pipe, "crypto_test", CLI_PROMPT));
    if(send_cmd(pipe, cmd, CRYPTO_TEST_PROMPT)) {
        if(expect(pipe, "\r\nSUCCESS")) {
            mu_check(send_cmd(pipe, "exit", CRYPTO_TEST_PROMPT));
            mu_check(send_cmd(pipe, "exit", CLI_PROMPT));
        } else {
            mu_check(send_cmd(pipe, "exit", ""));
            mu_check(send_cmd(pipe, "exit", CLI_PROMPT));
            mu_fail("command failed");
        }
    } else {
        mu_check(send_cmd(pipe, "exit", ""));
        mu_fail("command failed");
    }
}

static void do_crypto_aes_test(PipeSide* pipe) {
    run_crypto_test_command(pipe, "aes");
}

static void do_crypto_ecdsa_test(PipeSide* pipe) {
    run_crypto_test_command(pipe, "ecdsa");
}

static void do_crypto_sha_test(PipeSide* pipe) {
    run_crypto_test_command(pipe, "sha");
}

static void do_crypto_hmac_test(PipeSide* pipe) {
    run_crypto_test_command(pipe, "hmac");
}

static void do_crypto_csr_test(PipeSide* pipe) {
    run_crypto_test_command(pipe, "csr");
}

static void do_crypto_mbedtls_edsa_test(PipeSide* pipe) {
    run_crypto_test_command(pipe, "mbedtls_edsa");
}

static void run_cli_test(CliTestFn fn) {
    PipeSideBundle bundle = pipe_alloc(PIPE_SIZE, 1);
    PipeSide* my_pipe = bundle.alices_side;
    PipeSide* shell_pipe = bundle.bobs_side;

    CliIntercom* cli_intercom = furi_record_open(RECORD_CLI_INTERCOM);

    do {
        if(cli_intercom_spawn(cli_intercom, shell_pipe, true) != CliIntercomSpawnStatusOk) {
            pipe_free(shell_pipe);
            break;
        }

        fn(my_pipe);
    } while(false);
    cli_intercom_join(cli_intercom);
    pipe_free(my_pipe);

    furi_record_close(RECORD_CLI_INTERCOM);
}

MU_TEST(crypto_echo_test) {
    run_cli_test(do_crypto_echo_test);
}

MU_TEST(crypto_aes_test) {
    run_cli_test(do_crypto_aes_test);
}

MU_TEST(crypto_ecdsa_test) {
    run_cli_test(do_crypto_ecdsa_test);
}

MU_TEST(crypto_sha_test) {
    run_cli_test(do_crypto_sha_test);
}

MU_TEST(crypto_hmac_test) {
    run_cli_test(do_crypto_hmac_test);
}

MU_TEST(crypto_csr_test) {
    run_cli_test(do_crypto_csr_test);
}

MU_TEST(crypto_mbedtls_edsa_test) {
    run_cli_test(do_crypto_mbedtls_edsa_test);
}

MU_TEST_SUITE(crypto_test_suite) {
    MU_RUN_TEST(crypto_echo_test);
    MU_RUN_TEST(crypto_aes_test);
    MU_RUN_TEST(crypto_ecdsa_test);
    MU_RUN_TEST(crypto_sha_test);
    MU_RUN_TEST(crypto_hmac_test);
    MU_RUN_TEST(crypto_csr_test);
    MU_RUN_TEST(crypto_mbedtls_edsa_test);
}

int run_minunit_crypto_test(void) {
    MU_RUN_SUITE(crypto_test_suite);
    return MU_EXIT_CODE;
}
