#include <furi_hal_cortex.h>
#include <furi.h>

#include <si91x_device.h>

void furi_hal_cortex_init_early(void) {
    CoreDebug->DEMCR |= (CoreDebug_DEMCR_TRCENA_Msk | CoreDebug_DEMCR_MON_EN_Msk);
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0U;
}

FURI_NORETURN void furi_hal_cortex_system_reset(void) {
    NVIC_SystemReset();
}

void furi_hal_cortex_comp_enable(
    FuriHalCortexComp comp,
    FuriHalCortexCompFunction function,
    uint32_t value,
    uint32_t mask,
    FuriHalCortexCompSize size) {
    uint32_t function_reg = (uint32_t)function | ((uint32_t)size << 10);

    switch(comp) {
    case FuriHalCortexComp0:
        (DWT->COMP0) = value;
        (DWT->MASK0) = mask;
        (DWT->FUNCTION0) = function_reg;
        break;
    case FuriHalCortexComp1:
        (DWT->COMP1) = value;
        (DWT->MASK1) = mask;
        (DWT->FUNCTION1) = function_reg;
        break;
    case FuriHalCortexComp2:
        (DWT->COMP2) = value;
        (DWT->MASK2) = mask;
        (DWT->FUNCTION2) = function_reg;
        break;
    case FuriHalCortexComp3:
        (DWT->COMP3) = value;
        (DWT->MASK3) = mask;
        (DWT->FUNCTION3) = function_reg;
        break;
    default:
        furi_crash("Invalid parameter");
    }
}

void furi_hal_cortex_comp_reset(FuriHalCortexComp comp) {
    switch(comp) {
    case FuriHalCortexComp0:
        (DWT->COMP0) = 0;
        (DWT->MASK0) = 0;
        (DWT->FUNCTION0) = 0;
        break;
    case FuriHalCortexComp1:
        (DWT->COMP1) = 0;
        (DWT->MASK1) = 0;
        (DWT->FUNCTION1) = 0;
        break;
    case FuriHalCortexComp2:
        (DWT->COMP2) = 0;
        (DWT->MASK2) = 0;
        (DWT->FUNCTION2) = 0;
        break;
    case FuriHalCortexComp3:
        (DWT->COMP3) = 0;
        (DWT->MASK3) = 0;
        (DWT->FUNCTION3) = 0;
        break;
    default:
        furi_crash("Invalid parameter");
    }
}
