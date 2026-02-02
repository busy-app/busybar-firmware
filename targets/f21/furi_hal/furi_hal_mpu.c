#include <core/check.h>
#include <core/log.h>
#include <furi_hal_mpu.h>
#include <stm32u5xx_ll_cortex.h>

#define TAG "MPU"

#define FURI_HAL_MPU_IS_VALID_ADDRESS(address) (((address) & ~MPU_RBAR_BASE_Msk) == 0)
#define FURI_HAL_MPU_IS_VALID_SIZE(size) \
    ((size) > 0 && ((size) & ~(MPU_RBAR_BASE_Msk & MPU_RLAR_LIMIT_Msk)) == 0)

#define FURI_HAL_MPU_STACK_REGION_SIZE (32)

#define FURI_HAL_MPU_NULL_REGION_ADDRESS (0x00000000)
#define FURI_HAL_MPU_NULL_REGION_SIZE    (1024 * 1024)

static_assert(FURI_HAL_MPU_IS_VALID_ADDRESS(FURI_HAL_MPU_NULL_REGION_ADDRESS));
static_assert(FURI_HAL_MPU_IS_VALID_SIZE(FURI_HAL_MPU_NULL_REGION_SIZE));

typedef enum {
    /* null pointer dereference protection */
    FuriHalMpuRegionNull = LL_MPU_REGION_NUMBER0,

    /* stack overflow protection */
    FuriHalMpuRegionStack = LL_MPU_REGION_NUMBER1,
} FuriHalMpuRegion;

typedef enum {
    /* null pointer dereference protection */
    FuriHalMpuAttributesIndexNull = LL_MPU_ATTRIBUTES_NUMBER0,

    /* stack overflow protection */
    FuriHalMpuAttributesIndexStack = LL_MPU_ATTRIBUTES_NUMBER1,
} FuriHalMpuAtt;

static void furi_hal_mpu_enable(void) {
    LL_MPU_Enable(LL_MPU_CTRL_PRIVILEGED_DEFAULT);
}

static void furi_hal_mpu_disable(void) {
    LL_MPU_Disable();
}

static void
    furi_hal_mpu_protect_no_access(FuriHalMpuRegion region, uint32_t address, uint32_t size) {
    /* any other bits won't be used by the MPU */
    furi_assert(FURI_HAL_MPU_IS_VALID_ADDRESS(address));
    furi_assert(FURI_HAL_MPU_IS_VALID_SIZE(size));

    uint32_t mpu_attributes = LL_MPU_INSTRUCTION_ACCESS_DISABLE | LL_MPU_ACCESS_NOT_SHAREABLE |
                              LL_MPU_REGION_PRIV_RO;

    furi_hal_mpu_disable();
    /* MPU's actual limit address is postfixed with 0x1F */
    LL_MPU_ConfigRegion(
        region, mpu_attributes, FuriHalMpuAttributesIndexNull, address, address + size - 1);
    furi_hal_mpu_enable();
}

static void
    furi_hal_mpu_protect_read_only(FuriHalMpuRegion region, uint32_t address, uint32_t size) {
    /* any other bits won't be used by the MPU */
    furi_assert(FURI_HAL_MPU_IS_VALID_ADDRESS(address));
    furi_assert(FURI_HAL_MPU_IS_VALID_SIZE(size));

    uint32_t mpu_attributes = LL_MPU_INSTRUCTION_ACCESS_ENABLE | LL_MPU_ACCESS_NOT_SHAREABLE |
                              LL_MPU_REGION_ALL_RO;

    furi_hal_mpu_disable();
    /* MPU's actual limit address is postfixed with 0x1F */
    LL_MPU_ConfigRegion(
        region, mpu_attributes, FuriHalMpuAttributesIndexStack, address, address + size - 1);
    furi_hal_mpu_enable();
}

static void furi_hal_mpu_protect_disable(FuriHalMpuRegion region) {
    furi_hal_mpu_disable();
    LL_MPU_DisableRegion(region);
    furi_hal_mpu_enable();
}

void furi_hal_mpu_set_stack_protection(uint32_t* stack) {
    uint32_t _stack = (uint32_t)stack;

    if(!FURI_HAL_MPU_IS_VALID_ADDRESS(_stack)) {
        /* align _stack upwards by MPU's granularity value */
        _stack = (_stack + (1 << MPU_RBAR_BASE_Pos)) & MPU_RBAR_BASE_Msk;
    }

    furi_hal_mpu_protect_read_only(FuriHalMpuRegionStack, _stack, FURI_HAL_MPU_STACK_REGION_SIZE);
}

void furi_hal_mpu_reset_stack_protection(void) {
    furi_hal_mpu_protect_disable(FuriHalMpuRegionStack);
}

void furi_hal_mpu_init(void) {
    LL_MPU_ConfigAttributes(FuriHalMpuAttributesIndexNull, LL_MPU_NO_ALLOCATE);
    LL_MPU_ConfigAttributes(FuriHalMpuAttributesIndexStack, LL_MPU_R_ALLOCATE);

    furi_hal_mpu_protect_no_access(
        FuriHalMpuRegionNull, FURI_HAL_MPU_NULL_REGION_ADDRESS, FURI_HAL_MPU_NULL_REGION_SIZE);

    FURI_LOG_I(TAG, "Initialization successful");
}
