#include <furi_hal_random.h>
#include <furi_hal_bus.h>
#include <furi.h>

#include <stm32u5xx_ll_rng.h>
#include <stm32u5xx_ll_rcc.h>

#define TAG "FuriHalRandom"

static FuriMutex* furi_hal_rng_mutex = NULL;

static uint32_t furi_hal_random_read_rng(void) {
    while(LL_RNG_IsActiveFlag_CECS(RNG) || LL_RNG_IsActiveFlag_SECS(RNG) ||
          !LL_RNG_IsActiveFlag_DRDY(RNG)) {
        /* Error handling as described in RM0434, pg. 582-583 */
        if(LL_RNG_IsActiveFlag_CECS(RNG)) {
            /* Clock error occurred */
            LL_RNG_ClearFlag_CEIS(RNG);
        }

        if(LL_RNG_IsActiveFlag_SECS(RNG)) {
            /* Noise source error occurred */
            LL_RNG_ClearFlag_SEIS(RNG);

            for(uint32_t i = 0; i < 12; ++i) {
                const volatile uint32_t discard = LL_RNG_ReadRandData32(RNG);
                UNUSED(discard);
            }
        }
    }

    return LL_RNG_ReadRandData32(RNG);
}

static void furi_hal_random_enable(void) {
    furi_check(furi_mutex_acquire(furi_hal_rng_mutex, FuriWaitForever) == FuriStatusOk);
    LL_RNG_Enable(RNG);
}

static void furi_hal_random_disable(void) {
    LL_RNG_Disable(RNG);
    furi_check(furi_mutex_release(furi_hal_rng_mutex) == FuriStatusOk);
}

void furi_hal_random_init(void) {
    furi_hal_bus_enable(FuriHalBusRNG);
    LL_RCC_SetRNGClockSource(LL_RCC_RNG_CLKSOURCE_HSI48);
    furi_hal_rng_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    LL_RNG_Enable(RNG);
}

uint32_t furi_hal_random_get(void) {
    furi_hal_random_enable();
    const uint32_t random_val = furi_hal_random_read_rng();
    furi_hal_random_disable();

    return random_val;
}

void furi_hal_random_fill_buf(uint8_t* buf, uint32_t len) {
    furi_check(buf);
    furi_check(len);

    furi_hal_random_enable();

    for(uint32_t i = 0; i < len; i += 4) {
        const uint32_t random_val = furi_hal_random_read_rng();
        uint8_t len_cur = ((i + 4) < len) ? (4) : (len - i);
        memcpy(&buf[i], &random_val, len_cur);
    }

    furi_hal_random_disable();
}

void srand(unsigned seed) {
    UNUSED(seed);
}

int rand(void) {
    return furi_hal_random_get() & RAND_MAX;
}

long random(void) {
    return furi_hal_random_get() & RAND_MAX;
}
