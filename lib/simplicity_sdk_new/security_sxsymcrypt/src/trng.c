/*
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include "../include/sxsymcrypt/trng.h"
#include "../include/sxsymcrypt/statuscodes.h"
#include "crypmasterregs.h"
#include "hw.h"
#include "ba431regs.h"
#include "cmdma.h"

#ifndef RNG_CLKDIV
#define RNG_CLKDIV (241)
#endif

#ifndef RNG_BLENDING_LEVEL
    #define RNG_BLENDING_LEVEL 0
#else
    #if (RNG_BLENDING_LEVEL > 3)
        #error "RNG_BLENDING_LEVEL to big, maximum value is 3"
    #endif
#endif

#define RNG_OFF_TIMER_VAL (0)
#define RNG_INIT_WAIT_VAL (512)
#define RNG_NB_128BIT_BLOCKS (4)
#define RNG_FIFOLEVEL_GRANULARITY (4)
#define RNG_NO_OF_COND_KEYS (4)
/** Number of bytes per word used for test data, Version 0.
 * Value is fixed in this version. */
#define RNG_BYTES_PER_WORD_V0 (4u)

#define RNG_RAW_MODE (BA431_FLD_Control_AIS31Bypass_MASK | \
                      BA431_FLD_Control_HealthTestBypass_MASK | \
                      BA431_FLD_Control_Conditioning_Bypass_MASK)

/** TRNG self-tests
 *
 * This structure is used with sx_trng_init_with_self_tests().
 */
struct sx_trng_selftest {
     int (*run)(struct sx_trng* ctx, const struct sx_trng_selftest* test);
     const char *key;
     const size_t keysz;
     const char *input;
     const size_t inputsz;
     const char *expected;
     const size_t expectedsz;
};

static int sx_trng_selftest_conditioning(struct sx_trng *ctx,
    const struct sx_trng_selftest *test);
static int sx_trng_selftest_repetition(struct sx_trng *ctx,
    const struct sx_trng_selftest *test);
static int sx_trng_selftest_proportion(struct sx_trng *ctx,
    const struct sx_trng_selftest *test);

const struct sx_trng_selftest default_selftests_array[] = {
     {sx_trng_selftest_conditioning,
      "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c",
      16,
      "\x6b\xc0\xbc\xe1\x2a\x45\x99\x91\xe1\x34\x74\x1a\x7f\x9e\x19\x25"
      "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51"
      "\x30\xc8\x1c\x46\xa3\x5c\xe4\x11\xe5\xfb\xc1\x19\x1a\x0a\x52\xef"
      "\xf6\x9f\x24\x45\xdf\x4f\x9b\x17\xad\x2b\x41\x7b\xe6\x6c\x37\x10",
      64,
      "\x3f\xf1\xca\xa1\x68\x1f\xac\x09\x12\x0e\xca\x30\x75\x86\xe1\xa7",
      16
     },
     {sx_trng_selftest_repetition,
      "", 0, "", 0, "", 0
     },
     {sx_trng_selftest_proportion,
      "", 0, "", 0, "", 0
     },
     { 0 }
};

const struct sx_trng_selftest *default_selftests = default_selftests_array;

static int ba431_check_state(struct sx_trng *ctx)
{
    uint32_t state = sx_rdreg(ctx->regs, BA431_REG_Status_OFST);
    state = (state & BA431_FLD_Status_State_MASK) >> BA431_FLD_Status_State_LSB;

    if (BA431_STATE_ERROR == state){
        return SX_ERR_RESET_NEEDED;
    } else if ((BA431_STATE_RESET == state) ||
             (BA431_STATE_STARTUP == state)) {
        return SX_ERR_HW_PROCESSING;
    }

    return SX_OK;
}


static void sx_trng_enable(struct sx_trng *ctx)
{
    uint32_t ctrl_reg = sx_rdreg(ctx->regs, BA431_REG_Control_OFST);

    ctrl_reg |= BA431_FLD_Control_Enable_MASK;
    sx_wrreg(ctx->regs, BA431_REG_Control_OFST, ctrl_reg);
}


static void sx_trng_flush(struct sx_trng *ctx)
{
    uint32_t ctrl_reg = sx_rdreg(ctx->regs, BA431_REG_Control_OFST);

    sx_wrreg(ctx->regs, BA431_REG_Control_OFST,
             ctrl_reg | BA431_FLD_Control_SoftRst_MASK);
    sx_wrreg(ctx->regs, BA431_REG_Control_OFST,
             ctrl_reg & ~BA431_FLD_Control_SoftRst_MASK);
}


void sx_trng_restart(struct sx_trng *ctx)
{
    sx_trng_flush(ctx);
    sx_trng_enable(ctx);
}


static int sx_trng_get_hw_resources(struct sx_trng *ctx)
{
    unsigned int compatible;
    int i;

    *ctx = (struct sx_trng){0};
    compatible = sx_cmdma_list_compatible(REG_HW_PRESENT_BA431);
    if (!compatible)
        return SX_ERR_INCOMPATIBLE_HW;
    for (i = 0; !(compatible & 1); i++, compatible >>= 1) {
    }
    ctx->regs = sx_hw_find_trng_regs(i);
    if (!ctx->regs)
        return SX_ERR_INCOMPATIBLE_HW;

    return SX_OK;
}


static int sx_trng_init_config(struct sx_trng *ctx,
    const struct sx_trng_config *config)
{
    uint32_t control;
    uint32_t fifo_wakeup_level;
    uint32_t rng_off_timer_val = RNG_OFF_TIMER_VAL;
    uint32_t rng_clkdiv = RNG_CLKDIV;
    uint32_t rng_init_wait_val = RNG_INIT_WAIT_VAL;
    uint32_t ctrlbitmask = 0;

    /* Trigger warm reset. This will have an effect if the engine was previously
     * initialized. It will clear any data generated and stored in FIFOs, it will
     * stop the ring oscillators and it will reset the internal state machine to
     * "Reset" state.
     */
    sx_trng_flush(ctx);

    fifo_wakeup_level = sx_rdreg(ctx->regs, BA431_REG_FIFODepth_OFST) /
        RNG_FIFOLEVEL_GRANULARITY - 1;
    if (config) {
        if (config->wakeup_level) {
            if (config->wakeup_level > fifo_wakeup_level)
                return SX_ERR_TOO_BIG;
            fifo_wakeup_level = config->wakeup_level;
        }
        if (config->off_time_delay)
            rng_off_timer_val = config->off_time_delay;
        if (config->init_wait)
            rng_init_wait_val = config->init_wait;
        if (config->sample_clock_div)
            rng_clkdiv = config->sample_clock_div;
        ctrlbitmask = config->control_bitmask;
    }

    /* Program powerdown and clock settings */
    sx_wrreg(ctx->regs, BA431_REG_FIFOThreshold_OFST, fifo_wakeup_level);
    sx_wrreg(ctx->regs, BA431_REG_SwOffTmrVal_OFST, rng_off_timer_val);
    sx_wrreg(ctx->regs, BA431_REG_ClkDiv_OFST, rng_clkdiv);
    sx_wrreg(ctx->regs, BA431_REG_InitWaitVal_OFST, rng_init_wait_val);

    /* Configure the control register and set the enable it */
    control = (RNG_NB_128BIT_BLOCKS << BA431_FLD_Control_Nb128BitBlocks_LSB);
    control |= ctrlbitmask;
    control |= BA431_FLD_Control_Enable_MASK;
    control |= RNG_BLENDING_LEVEL << BA431_FLD_Control_BlendingMethod_LSB;
    sx_wrreg(ctx->regs, BA431_REG_Control_OFST, control);

    return SX_OK;
}


int sx_trng_init(struct sx_trng *ctx, const struct sx_trng_config *config)
{
    int status;

    status = sx_trng_get_hw_resources(ctx);
    if (status != SX_OK)
        return status;

    return sx_trng_init_config(ctx, config);
}


static int ba431_setup_conditioning_key(struct sx_trng *ctx)
{
    uint32_t key;
    uint32_t level = sx_rdreg(ctx->regs, BA431_REG_FIFOLevel_OFST);

    /* FIFO level must be 4 (4 * 32bit Word) */
    if (level < RNG_NO_OF_COND_KEYS)
        return SX_ERR_HW_PROCESSING;

    for (size_t i = 0; i < RNG_NO_OF_COND_KEYS; i++) {
        key = sx_rdreg(ctx->regs, BA431_REG_FIFODATA_OFST);
        sx_wrreg(ctx->regs, BA431_REG_Key0_OFST + i * sizeof(key), key);
    }

    /* After the conditioning keys are written the FIFOs should be cleared
     * as that data generated up to this point used default keys. */
    sx_trng_restart(ctx);
    ctx->conditioning_key_set = 1;

    return SX_OK;
}


int sx_trng_get(struct sx_trng *ctx, char *dst, size_t size)
{
    int status = SX_OK;

    if (!ctx->regs)
        return SX_ERR_UNITIALIZED_OBJ;

    status = ba431_check_state(ctx);
    if (status != SX_OK)
        return status;

    /* Program random key for the conditioning function */
    if (!ctx->conditioning_key_set) {
        status = ba431_setup_conditioning_key(ctx);
        if (status != SX_OK)
            return status;
    }

    /* Block sizes above the FIFO wakeup level to guarantee that the
     * hardware will be able at some time to provide the requested bytes. */
    uint32_t wakeup_level = sx_rdreg(ctx->regs, BA431_REG_FIFOThreshold_OFST);
    if (size > (wakeup_level + 1) * 16)
        return SX_ERR_TOO_BIG;

    uint32_t level = sx_rdreg(ctx->regs, BA431_REG_FIFOLevel_OFST);
    /* FIFO level in 4-byte words */
    if (size > level * RNG_FIFOLEVEL_GRANULARITY)
        return SX_ERR_HW_PROCESSING;

    while (size) {
        uint32_t data;
        data = sx_rdreg(ctx->regs, BA431_REG_FIFODATA_OFST);
        for (size_t i = 0; (i < sizeof(data)) && (size); i++, size--) {
            *dst = (char)(data & 0xFF);
            dst++;
            data >>= 8;
        }
    }

    return status;
}


int sx_trng_save_state(struct sx_trng *ctx, struct sx_trng_state *state)
{
    if (!ctx->regs)
        return SX_ERR_UNITIALIZED_OBJ;

    state->conditioning_key_set = ctx->conditioning_key_set;

    if (ctx->conditioning_key_set)
        for (size_t i = 0; i < RNG_NO_OF_COND_KEYS; i++)
            state->cond_key[i] = sx_rdreg(ctx->regs,
                    BA431_REG_Key0_OFST + i * sizeof(state->cond_key[0]));

    return SX_OK;
}


int sx_trng_restore_state(struct sx_trng *ctx,
    const struct sx_trng_config *config, const struct sx_trng_state *state)
{
    int status;

    status = sx_trng_init(ctx, config);
    if (status != SX_OK)
        return status;

    if (state->conditioning_key_set) {
        for (size_t i = 0; i < RNG_NO_OF_COND_KEYS; i++)
            sx_wrreg(ctx->regs, BA431_REG_Key0_OFST + i * sizeof(state->cond_key[0]),
                    state->cond_key[i]);

        sx_trng_restart(ctx);
        ctx->conditioning_key_set = 1;
    }

    return SX_OK;
}


int sx_trng_init_unsafe(struct sx_trng *ctx, const struct sx_trng_config *config)
{
    struct sx_trng_config local_config = {0};

    if (config)
        local_config = *config;

    local_config.control_bitmask |= RNG_RAW_MODE;

    return sx_trng_init(ctx, &local_config);
}


static unsigned int sx_trng_get_bytes_per_test_word(struct sx_trng *ctx)
{
    uint32_t hw_version;
    uint32_t hw_config;
    uint32_t log2_number_shares;

    /* There are two versions of TRNGs.
     * Number of bytes per word:
     *     - fixed to 32 (HardwareVersion = 0)
     *     - from 8 to 32 (HardwareVersion = 1)
     *          - value is found in HW config register as 2^Log2NbOfShares.
     */
    hw_version = sx_rdreg(ctx->regs, BA431_REG_HwVersion_OFST);
    if (hw_version == 0)
        return RNG_BYTES_PER_WORD_V0;

    hw_config = sx_rdreg(ctx->regs, BA431_REG_HwConfig_OFST);
    log2_number_shares = (hw_config >> BA431_REG_HwConfig_Log2NumShares_LSB) &
            BA431_REG_HwConfig_Log2NumShares_MASK;

    return 1u << log2_number_shares;
}


static int sx_trng_selftest_write_testdata(struct sx_trng *ctx,
    const char *input, const size_t inputsz)
{
    uint32_t test_data = 0;
    unsigned int bytes_per_word;

    /* Write test data input
     * Based on number of shares, the input data must be written in chunks of
     * 8, 16 or 32 bit words. The data must be provided in little-endian format.
     */
    bytes_per_word = sx_trng_get_bytes_per_test_word(ctx);
    if (bytes_per_word > 4)
        return SX_ERR_INCOMPATIBLE_HW;

    for (unsigned int i = 0; i < inputsz; i += bytes_per_word) {
        test_data = 0;
        for (unsigned int j = 0; j < bytes_per_word; j++)
            test_data = (input[i + j] << 24) | (test_data >> 8);

        /* Wait until test data is processed */
        while (sx_rdreg(ctx->regs, BA431_REG_Status_OFST) &
                BA431_FLD_Status_TestDataBusy_MASK) {
        }
        sx_wrreg(ctx->regs, BA431_REG_TestData_OFST, test_data);
    }

    return SX_OK;
}


static int sx_trng_selftest_conditioning(struct sx_trng *ctx,
    const struct sx_trng_selftest *test)
{
    uint32_t i;
    uint32_t error = 0;
    uint32_t control;

    sx_trng_flush(ctx);

    /* Enable test mode */
    control = BA431_FLD_Control_TestEn_MASK;
    control |= RNG_NB_128BIT_BLOCKS << BA431_FLD_Control_Nb128BitBlocks_LSB;
    control |= BA431_FLD_Control_HealthTestBypass_MASK;
    control |= BA431_FLD_Control_Enable_MASK;
    sx_wrreg(ctx->regs, BA431_REG_Control_OFST, control);

    /* Write key */
    for (size_t i = 0; i < test->keysz; i += sizeof(uint32_t)) {
        uint32_t key = 0;
        for (size_t j = 0; j < sizeof(uint32_t); j++)
            key = (test->key[i + j] << 24) | (key >> 8);
        sx_wrreg(ctx->regs, BA431_REG_Key0_OFST + i, key);
    }

    if (sx_trng_selftest_write_testdata(ctx, test->input, test->inputsz) != SX_OK)
        return SX_ERR_INCOMPATIBLE_HW;

    /* Wait for conditioning test to end, wait for data to fill the FIFO */
    while (sx_rdreg(ctx->regs, BA431_REG_FIFOLevel_OFST) < 4) {
    }
    /* Clear control register */
    sx_wrreg(ctx->regs, BA431_REG_Control_OFST, 0x00);

    /* Compare results to known answer */
    for (i = 0; i < test->expectedsz / sizeof(uint32_t); i++) {
        uint32_t result = sx_rdreg(ctx->regs, BA431_REG_FIFODATA_OFST);
        error |= (result & 0xFF) ^ test->expected[i * 4];
        result >>= 8;
        error |= (result & 0xFF) ^ test->expected[i * 4 + 1];
        result >>= 8;
        error |= (result & 0xFF) ^ test->expected[i * 4 + 2];
        result >>= 8;
        error |= (result & 0xFF) ^ test->expected[i * 4 + 3];
    }
    if (error)
        return SX_ERR_SELF_TEST_FAILURE;

    /* Reset FIFOs */
    sx_trng_flush(ctx);

    return SX_OK;
}


/* The repetition health tests checks if a value is repeated consecutively.
 * The TRNG engine has two version in production that define a sample in a different
 * manner. HW V0 defines a sample as a bit, and works with bits, in contrast to
 * V1 where a sample has a byte granularity, and it works with bytes.
 * To simplify the test function, the cutoff register is overwritten with a
 * known value. The value is specific to each HW version. The initial cutoff
 * register value is restored after the self-test ends. For HW V0 the health
 * test counts how many consecutive '1' or '0' were generated and compares it
 * with the cutoff register value. For HW V1, the health test counts how many
 * consecutive bytes, per share, were generated and compares it with the cutoff
 * register value.
 */
static int sx_trng_generate_selftest_repetition(struct sx_trng *ctx)
{
    char data[sizeof(uint32_t)] = {0xFF, 0xFF, 0xFF, 0xFF};
    uint32_t cutoff_bkp = sx_rdreg(ctx->regs, BA431_REG_Repetition_Cutoff_OFST);
    uint32_t bytes_per_word = sx_trng_get_bytes_per_test_word(ctx);
    int status = SX_OK;
    /* The cutoff value for the repetition test for HW V0, in bits. */
    const int cutoff_v0 = 128;
    /* The cutoff value for the repetition test for HW V1, in bytes. */
    const int cutoff_v1 = 4;

    if (sx_rdreg(ctx->regs, BA431_REG_HwVersion_OFST) > 0)
        sx_wrreg(ctx->regs, BA431_REG_Repetition_Cutoff_OFST, cutoff_v1);
    else
        sx_wrreg(ctx->regs, BA431_REG_Repetition_Cutoff_OFST, cutoff_v0);

    for (uint32_t i = 0; i < cutoff_v1 * bytes_per_word;
            i += sizeof(uint32_t)) {
        status = sx_trng_selftest_write_testdata(ctx, data, sizeof(data));
        if (status != SX_OK)
            break;
    }

    /* Wait until test data is processed */
    while (sx_rdreg(ctx->regs, BA431_REG_Status_OFST) &
            BA431_FLD_Status_TestDataBusy_MASK) {
    }

    /* Restore the original cutoff value. */
    sx_wrreg(ctx->regs, BA431_REG_Repetition_Cutoff_OFST, cutoff_bkp);

    return status;
}


static int sx_trng_selftest_repetition(struct sx_trng *ctx,
    const struct sx_trng_selftest *test)
{
    uint32_t control, state;
    unsigned int shares_mask;

    sx_trng_flush(ctx);

    /* Enable test mode */
    control = BA431_FLD_Control_TestEn_MASK;
    control |= BA431_FLD_Control_Enable_MASK;
    sx_wrreg(ctx->regs, BA431_REG_Control_OFST, control);

    if (test->inputsz) {
        if (sx_trng_selftest_write_testdata(ctx, test->input, test->inputsz) != SX_OK)
            return SX_ERR_INCOMPATIBLE_HW;
    } else {
        if (sx_trng_generate_selftest_repetition(ctx) != SX_OK)
            return SX_ERR_INCOMPATIBLE_HW;
    }

    /* Check status, expected repetition test to fail. */
    state = sx_rdreg(ctx->regs, BA431_REG_Status_OFST);
    if (!(state & (1 << BA431_FLD_Status_Repetition_LSB)))
        return SX_ERR_SELF_TEST_FAILURE;

    /* Check the state register to see that the error occurred on all present
     * shares. bytes_per_word is calculated based on the number of shares and
     * also tells how many shares are present. This check applies only to hardware
     * version 1.*/
    if (sx_rdreg(ctx->regs, BA431_REG_HwVersion_OFST) > 0) {
        shares_mask = (1u << sx_trng_get_bytes_per_test_word(ctx)) - 1;
        state >>= BA431_FLD_Status_RepetitionPerShare_LSB;
        if ((state & shares_mask) != shares_mask)
             return SX_ERR_SELF_TEST_FAILURE;
    }

    return SX_OK;
}


/* The NIST continuous proportion health check looks if the first sample of the
 * current window does not occur too often in the window.
 * The self-test checks that the proportion health test is working properly.
 * TRNG HW V1 defines a sample as a byte and works with bytes. In contrast, V0
 * defines a sample as one bit and it works with bits.
 * The cutoff register tells how many times the first byte (V1), or bit (V0)
 * can occur in 512 bytes (V1), or 1024 bits (V0) window until the proportion
 * failure is triggered.
 * To trigger the proportion failure, the first byte/bit must occur, in the data set,
 * as many times as the cutoff register value. The data set size must be smaller,
 * or equal to the window size.
 * To simplify the test function, the cutoff register is overwritten with a
 * known value. The value is specific to each HW version. The initial cutoff
 * register value is restored after the self-test ends.
 * Both HW version work with 4 bytes words. For one share, the 4 bytes word is
 * written to the same share. For two shares, the first and the third byte are
 * written to one share and the second and fourth byte to the second share. For
 * four shares, each byte is written to a different share.
 * The index starts from 0 and goes up to 115, and the test data looks like:
 * index, ~index, index + 1, ~(index + 1). If we do just this, the error will
 * not be triggered. To force the error, we overwrite with 0xFF, 0xFE, 0xFD, OxFC,
 * when index % 'repeat_after' == 0 (including index = 0). By doing this we
 * fulfill the condition for V0 and V1 (all number of shares).
 */
static int sx_trng_generate_selftest_proportion(struct sx_trng *ctx)
{
    unsigned char data[4];
    uint32_t cutoff_bkp = sx_rdreg(ctx->regs, BA431_REG_Proportion_Cutoff_OFST);
    int status = SX_OK;
    /* The cutoff value for the proportion test for HW V1, in bits. */
    const int cutoff_v0 = 558;
    /* The cutoff value for the proportion test for HW V1, in bytes. */
    const int cutoff_v1 = 8;
    /* This value represents the number of bytes after which the bytes are repeated. */
    const int repeat_after = 16;
    /* This value was set to 116 bytes in order to check the exact limit of the
     * cutoff register. More to the point, to trigger the error with the word written,
     * and to be able to check that the error was not triggered earlier. This value
     * covers all HW versions and number of shares. */
    const int dataset_sz = 116;

    if (sx_rdreg(ctx->regs, BA431_REG_HwVersion_OFST) > 0)
        sx_wrreg(ctx->regs, BA431_REG_Proportion_Cutoff_OFST, cutoff_v1);
    else
        sx_wrreg(ctx->regs, BA431_REG_Proportion_Cutoff_OFST, cutoff_v0);

    for (unsigned char i = 0; i < dataset_sz; i += 4) {
        if (i % repeat_after == 0) {
            data[0] = 0xFF;
            data[1] = 0xFE;
            data[2] = 0xFD;
            data[3] = 0xFC;
        } else {
            data[0] = i;
            data[1] = ~i;
            data[2] = i + 1;
            data[3] = ~(i + 1);
        }

        /* Check status, proportion test should not have failed yet. */
        uint32_t state = sx_rdreg(ctx->regs, BA431_REG_Status_OFST);
        if (state & (1 << BA431_FLD_Status_Proportion_LSB)) {
            status = SX_ERR_SELF_TEST_FAILURE;
            break;
        }
        status = sx_trng_selftest_write_testdata(ctx, (char*) data, sizeof(data));
        if (status != SX_OK)
            break;
    }

    /* Wait until test data is processed */
    while (sx_rdreg(ctx->regs, BA431_REG_Status_OFST) &
            BA431_FLD_Status_TestDataBusy_MASK) {
    }

    /* Restore the original cutoff value. */
    sx_wrreg(ctx->regs, BA431_REG_Proportion_Cutoff_OFST, cutoff_bkp);

    return status;
}


static int sx_trng_selftest_proportion(struct sx_trng *ctx,
    const struct sx_trng_selftest *test)
{
    uint32_t control, state;
    unsigned int shares_mask;

    sx_trng_flush(ctx);

    /* Enable test mode */
    control = BA431_FLD_Control_TestEn_MASK;
    control |= BA431_FLD_Control_Enable_MASK;
    sx_wrreg(ctx->regs, BA431_REG_Control_OFST, control);

    if (test->inputsz) {
        if (sx_trng_selftest_write_testdata(ctx, test->input, test->inputsz) != SX_OK)
            return SX_ERR_INCOMPATIBLE_HW;
    } else {
        if (sx_trng_generate_selftest_proportion(ctx) != SX_OK)
            return SX_ERR_INCOMPATIBLE_HW;
    }

    /* Check status, expected repetition test to fail. */
    state = sx_rdreg(ctx->regs, BA431_REG_Status_OFST);
    if (!(state & (1 << BA431_FLD_Status_Proportion_LSB)))
        return SX_ERR_SELF_TEST_FAILURE;

    /* Check the state register to see that the error occurred on all present
     * shares. bytes_per_word is calculated based on the number of shares and
     * also tells how many shares are present. This check applies only to hardware
     * version 1.*/
    if (sx_rdreg(ctx->regs, BA431_REG_HwVersion_OFST) > 0) {
        shares_mask = (1u << sx_trng_get_bytes_per_test_word(ctx)) - 1;
        state >>= BA431_FLD_Status_ProportionPerShare_LSB;
        if ((state & shares_mask) != shares_mask)
             return SX_ERR_SELF_TEST_FAILURE;
    }

    return SX_OK;
}


int sx_trng_init_with_self_tests(struct sx_trng *ctx,
    const struct sx_trng_config *config,
    const struct sx_trng_selftest *tests)
{
    int status;
    const struct sx_trng_selftest *test = tests;

    status = sx_trng_get_hw_resources(ctx);
    if (status != SX_OK)
        return status;

    while (!test || test->run) {
        status = test->run(ctx, test);
        if (status != SX_OK)
            return status;

        test++;
    }

    return sx_trng_init_config(ctx, config);
}
