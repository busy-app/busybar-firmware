/** TRNG self-tests sample
 *
 * Sample showing how to use Secure-IC TRNG with self-tests. The self-tests are run
 * before the TRNG initialization. If no error occurs, the initializing the TRNG
 * is executed.
 * The sample will output 4 random bytes, as a result of the successful self-tests
 * and TRNG initialization.
  *
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include <sxsymcrypt/trng.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/version.h>
#include "env/io.h"

#define TRNG_CHUNK_SZ 4

SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 5);

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int result;
    char rnd_bytes[TRNG_CHUNK_SZ];
    struct sx_trng ctx;

    result = sx_trng_init_with_self_tests(&ctx, NULL, default_selftests);
    if (result != SX_OK)
        return result;

    do {
        result = sx_trng_get(&ctx, rnd_bytes, sizeof(rnd_bytes));
        if (result == SX_ERR_HW_PROCESSING)
            continue;
        if (result != SX_OK)
            return result;
        writedata(rnd_bytes, sizeof(rnd_bytes));
    } while (result == SX_ERR_HW_PROCESSING);

    return 0;
}
