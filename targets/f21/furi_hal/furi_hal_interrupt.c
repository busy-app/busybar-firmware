#include <furi_hal.h>
#include <FreeRTOS.h>
#include <stm32u5xx_ll_cortex.h>
#include <stm32u5xx_ll_system.h>

#define TAG "FuriHalInterrupt"

#define FURI_HAL_INTERRUPT_DEFAULT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 5)

#ifdef FURI_RAM_EXEC
#define FURI_HAL_INTERRUPT_ACCOUNT_START()
#define FURI_HAL_INTERRUPT_ACCOUNT_END()
#else
#define FURI_HAL_INTERRUPT_ACCOUNT_START() const uint32_t _isr_start = DWT->CYCCNT;
#define FURI_HAL_INTERRUPT_ACCOUNT_END()                    \
    const uint32_t _time_in_isr = DWT->CYCCNT - _isr_start; \
    furi_hal_interrupt.counter_time_in_isr_total += _time_in_isr;
#endif

typedef struct {
    FuriHalInterruptISR isr;
    void* context;
} FuriHalInterruptISRPair;

typedef struct {
    FuriHalInterruptISRPair isr[FuriHalInterruptIdMax];
    uint32_t counter_time_in_isr_total;
} FuriHalInterrupt;

static FuriHalInterrupt furi_hal_interrupt = {};

const IRQn_Type furi_hal_interrupt_irqn[FuriHalInterruptIdMax] = {
    // SDMMC
    [FuriHalInterruptIdSdMmc1] = SDMMC1_IRQn,

    // GPDMA
    [FuriHalInterruptIdGPDMA1Channel0] = GPDMA1_Channel0_IRQn,
    [FuriHalInterruptIdGPDMA1Channel1] = GPDMA1_Channel1_IRQn,
    [FuriHalInterruptIdGPDMA1Channel2] = GPDMA1_Channel2_IRQn,
    [FuriHalInterruptIdGPDMA1Channel3] = GPDMA1_Channel3_IRQn,
    [FuriHalInterruptIdGPDMA1Channel4] = GPDMA1_Channel4_IRQn,
    [FuriHalInterruptIdGPDMA1Channel5] = GPDMA1_Channel5_IRQn,
    [FuriHalInterruptIdGPDMA1Channel6] = GPDMA1_Channel6_IRQn,
    [FuriHalInterruptIdGPDMA1Channel7] = GPDMA1_Channel7_IRQn,
    [FuriHalInterruptIdGPDMA1Channel8] = GPDMA1_Channel8_IRQn,
    [FuriHalInterruptIdGPDMA1Channel9] = GPDMA1_Channel9_IRQn,
    [FuriHalInterruptIdGPDMA1Channel10] = GPDMA1_Channel10_IRQn,
    [FuriHalInterruptIdGPDMA1Channel11] = GPDMA1_Channel11_IRQn,
    [FuriHalInterruptIdGPDMA1Channel12] = GPDMA1_Channel12_IRQn,
    [FuriHalInterruptIdGPDMA1Channel13] = GPDMA1_Channel13_IRQn,
    [FuriHalInterruptIdGPDMA1Channel14] = GPDMA1_Channel14_IRQn,
    [FuriHalInterruptIdGPDMA1Channel15] = GPDMA1_Channel15_IRQn,

    // LPDMA
    [FuriHalInterruptIdLPDMA1Channel0] = LPDMA1_Channel0_IRQn,
    [FuriHalInterruptIdLPDMA1Channel1] = LPDMA1_Channel1_IRQn,
    [FuriHalInterruptIdLPDMA1Channel2] = LPDMA1_Channel2_IRQn,
    [FuriHalInterruptIdLPDMA1Channel3] = LPDMA1_Channel3_IRQn,

    // GPU
    [FuriHalInterruptIdDMA2D] = DMA2D_IRQn,
    // [FuriHalInterruptIdGPU2D] = GPU2D_IRQn,
    // [FuriHalInterruptIdGPU2DError] = GPU2D_ER_IRQn,

    // LPUART
    [FuriHalInterruptIdLPUART1] = LPUART1_IRQn,

    // USART
    [FuriHalInterruptIdUsart1] = USART1_IRQn,
    [FuriHalInterruptIdUsart2] = USART2_IRQn,
    [FuriHalInterruptIdUsart3] = USART3_IRQn,
    [FuriHalInterruptIdUsart6] = USART6_IRQn,

    // UART
    [FuriHalInterruptIdUart4] = UART4_IRQn,
    [FuriHalInterruptIdUart5] = UART5_IRQn,

    // LPTIM
    [FuriHalInterruptIdLPTIM1] = LPTIM1_IRQn,
    [FuriHalInterruptIdLPTIM2] = LPTIM2_IRQn,
    [FuriHalInterruptIdLPTIM3] = LPTIM3_IRQn,
    [FuriHalInterruptIdLPTIM4] = LPTIM4_IRQn,

    // USB
    [FuriHalInterruptIdUSBHS] = OTG_HS_IRQn,

    // RCC
    [FuriHalInterruptIdRcc] = RCC_IRQn,

    // USB PD
    [FuriHalInterruptIdUCPD1] = UCPD1_IRQn,
};

__attribute__((always_inline)) static inline void
    furi_hal_interrupt_call(FuriHalInterruptId index) {
    FURI_HAL_INTERRUPT_ACCOUNT_START();
    furi_check(furi_hal_interrupt.isr[index].isr);
    furi_hal_interrupt.isr[index].isr(furi_hal_interrupt.isr[index].context);
    FURI_HAL_INTERRUPT_ACCOUNT_END();
}

__attribute__((always_inline)) static inline void
    furi_hal_interrupt_enable(FuriHalInterruptId index, uint16_t priority) {
    NVIC_SetPriority(
        furi_hal_interrupt_irqn[index],
        NVIC_EncodePriority(NVIC_GetPriorityGrouping(), priority, 0));
    NVIC_EnableIRQ(furi_hal_interrupt_irqn[index]);
}

__attribute__((always_inline)) static inline void
    furi_hal_interrupt_clear_pending(FuriHalInterruptId index) {
    NVIC_ClearPendingIRQ(furi_hal_interrupt_irqn[index]);
}

__attribute__((always_inline)) static inline void
    furi_hal_interrupt_get_pending(FuriHalInterruptId index) {
    NVIC_GetPendingIRQ(furi_hal_interrupt_irqn[index]);
}

__attribute__((always_inline)) static inline void
    furi_hal_interrupt_set_pending(FuriHalInterruptId index) {
    NVIC_SetPendingIRQ(furi_hal_interrupt_irqn[index]);
}

__attribute__((always_inline)) static inline void
    furi_hal_interrupt_disable(FuriHalInterruptId index) {
    NVIC_DisableIRQ(furi_hal_interrupt_irqn[index]);
}

void furi_hal_interrupt_init() {
    NVIC_SetPriority(TAMP_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(TAMP_IRQn);

    NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));

    NVIC_SetPriority(FPU_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
    NVIC_EnableIRQ(FPU_IRQn);

    LL_SYSCFG_DisableIT_FPU_IOC();
    LL_SYSCFG_DisableIT_FPU_DZC();
    LL_SYSCFG_DisableIT_FPU_UFC();
    LL_SYSCFG_DisableIT_FPU_OFC();
    LL_SYSCFG_DisableIT_FPU_IDC();
    LL_SYSCFG_DisableIT_FPU_IXC();

    LL_HANDLER_EnableFault(LL_HANDLER_FAULT_USG);
    LL_HANDLER_EnableFault(LL_HANDLER_FAULT_BUS);
    LL_HANDLER_EnableFault(LL_HANDLER_FAULT_MEM);

    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_interrupt_set_isr(FuriHalInterruptId index, FuriHalInterruptISR isr, void* context) {
    FuriHalInterruptPriority priority = furi_kernel_is_running() ?
                                            FuriHalInterruptPriorityNormal :
                                            FuriHalInterruptPriorityKamiSama;
    furi_hal_interrupt_set_isr_ex(index, priority, isr, context);
}

void furi_hal_interrupt_set_isr_ex(
    FuriHalInterruptId index,
    FuriHalInterruptPriority priority,
    FuriHalInterruptISR isr,
    void* context) {
    furi_check(index < FuriHalInterruptIdMax);
    furi_check(
        (priority >= FuriHalInterruptPriorityLowest &&
         priority <= FuriHalInterruptPriorityHighest) ||
        priority == FuriHalInterruptPriorityKamiSama);

    uint16_t real_priority = FURI_HAL_INTERRUPT_DEFAULT_PRIORITY - priority;

    if(isr) {
        // Pre ISR set
        furi_check(furi_hal_interrupt.isr[index].isr == NULL);
    } else {
        // Pre ISR clear
        furi_hal_interrupt_disable(index);
        furi_hal_interrupt_clear_pending(index);
    }

    furi_hal_interrupt.isr[index].isr = isr;
    furi_hal_interrupt.isr[index].context = context;
    __DMB();

    if(isr) {
        // Post ISR set
        furi_hal_interrupt_clear_pending(index);
        furi_hal_interrupt_enable(index, real_priority);
    } else {
        // Post ISR clear
    }
}

void SDMMC1_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdSdMmc1);
}

void GPDMA1_Channel0_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPDMA1Channel0);
}

void GPDMA1_Channel1_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPDMA1Channel1);
}

void GPDMA1_Channel2_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPDMA1Channel2);
}

void GPDMA1_Channel3_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPDMA1Channel3);
}

void GPDMA1_Channel4_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPDMA1Channel4);
}

void GPDMA1_Channel5_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPDMA1Channel5);
}

void GPDMA1_Channel6_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPDMA1Channel6);
}

void GPDMA1_Channel7_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPDMA1Channel7);
}

void GPDMA1_Channel8_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPDMA1Channel8);
}

void GPDMA1_Channel9_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPDMA1Channel9);
}

void GPDMA1_Channel10_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPDMA1Channel10);
}

void GPDMA1_Channel11_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPDMA1Channel11);
}

void GPDMA1_Channel12_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPDMA1Channel12);
}

void GPDMA1_Channel13_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPDMA1Channel13);
}

void GPDMA1_Channel14_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPDMA1Channel14);
}

void GPDMA1_Channel15_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPDMA1Channel15);
}

void LPDMA1_Channel0_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdLPDMA1Channel0);
}

void LPDMA1_Channel1_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdLPDMA1Channel1);
}

void LPDMA1_Channel2_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdLPDMA1Channel2);
}

void LPDMA1_Channel3_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdLPDMA1Channel3);
}

void DMA2D_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdDMA2D);
}

void LPUART1_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdLPUART1);
}

void USART1_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdUsart1);
}

void USART2_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdUsart2);
}

void USART3_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdUsart3);
}

void USART6_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdUsart6);
}

void UART4_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdUart4);
}

void UART5_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdUart5);
}

void TAMP_IRQHandler(void) {
    if(LL_RCC_LSE_IsCSSDetected()) {
        LL_RCC_LSE_DisableCSS();
        if(!LL_RCC_LSE_IsReady()) {
            FURI_LOG_E(TAG, "LSE CSS fired: resetting system");
            NVIC_SystemReset();
        } else {
            FURI_LOG_E(TAG, "LSE CSS fired: but LSE is alive");
            LL_RCC_LSE_EnableCSS(); // TODO: we really can recover from this?
        }
    }
}

void UCPD1_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdUCPD1);
}

void RCC_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdRcc);
}

void GPU2D_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPU2D);
}

void GPU2D_ER_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdGPU2DError);
}

void LPTIM1_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdLPTIM1);
}

void LPTIM2_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdLPTIM2);
}

void LPTIM3_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdLPTIM3);
}

void LPTIM4_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdLPTIM4);
}

void OTG_HS_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdUSBHS);
}

void NMI_Handler() {
    if(LL_RCC_IsActiveFlag_HSECSS()) {
        LL_RCC_ClearFlag_HSECSS();
        FURI_LOG_E(TAG, "HSE CSS fired: resetting system");
        NVIC_SystemReset();
    }
}

void HardFault_Handler() {
    furi_crash("HardFault");
}

void MemManage_Handler() {
    furi_log_puts("\r\n" _FURI_LOG_CLR_E "Mem fault:\r\n");
    if(FURI_BIT(SCB->CFSR, SCB_CFSR_MLSPERR_Pos)) {
        furi_log_puts(" - lazy stacking for exception entry\r\n");
    }

    if(FURI_BIT(SCB->CFSR, SCB_CFSR_MSTKERR_Pos)) {
        furi_log_puts(" - stacking for exception entry\r\n");
    }

    if(FURI_BIT(SCB->CFSR, SCB_CFSR_MUNSTKERR_Pos)) {
        furi_log_puts(" - unstacking for exception return\r\n");
    }

    if(FURI_BIT(SCB->CFSR, SCB_CFSR_DACCVIOL_Pos)) {
        furi_log_puts(" - data access violation\r\n");
    }

    if(FURI_BIT(SCB->CFSR, SCB_CFSR_IACCVIOL_Pos)) {
        furi_log_puts(" - instruction access violation\r\n");
    }

    if(FURI_BIT(SCB->CFSR, SCB_CFSR_MMARVALID_Pos)) {
        uint32_t memfault_address = SCB->MMFAR;
        furi_log_puts(" -- at 0x");
        furi_log_puthex32(memfault_address);
        furi_log_puts("\r\n");

        if(memfault_address < (1024 * 1024)) {
            furi_log_puts(" -- NULL pointer dereference");
        } else {
            // write or read of MPU region 1 (FuriHalMpuRegionStack)
            furi_log_puts(" -- MPU fault, possibly stack overflow");
        }
    }
    furi_log_puts(_FURI_LOG_CLR_RESET "\r\n");

    furi_crash("MemManage");
}

void BusFault_Handler() {
    furi_log_puts("\r\n" _FURI_LOG_CLR_E "Bus fault:\r\n");
    if(FURI_BIT(SCB->CFSR, SCB_CFSR_LSPERR_Pos)) {
        furi_log_puts(" - lazy stacking for exception entry\r\n");
    }

    if(FURI_BIT(SCB->CFSR, SCB_CFSR_STKERR_Pos)) {
        furi_log_puts(" - stacking for exception entry\r\n");
    }

    if(FURI_BIT(SCB->CFSR, SCB_CFSR_UNSTKERR_Pos)) {
        furi_log_puts(" - unstacking for exception return\r\n");
    }

    if(FURI_BIT(SCB->CFSR, SCB_CFSR_IMPRECISERR_Pos)) {
        furi_log_puts(" - imprecise data access\r\n");
    }

    if(FURI_BIT(SCB->CFSR, SCB_CFSR_PRECISERR_Pos)) {
        furi_log_puts(" - precise data access\r\n");
    }

    if(FURI_BIT(SCB->CFSR, SCB_CFSR_IBUSERR_Pos)) {
        furi_log_puts(" - instruction\r\n");
    }

    if(FURI_BIT(SCB->CFSR, SCB_CFSR_BFARVALID_Pos)) {
        uint32_t busfault_address = SCB->BFAR;
        furi_log_puts(" -- at 0x");
        furi_log_puthex32(busfault_address);
        furi_log_puts("\r\n");

        if(busfault_address == (uint32_t)NULL) {
            furi_log_puts(" -- NULL pointer dereference");
        }
    }
    furi_log_puts(_FURI_LOG_CLR_RESET "\r\n");

    furi_crash("BusFault");
}

void UsageFault_Handler() {
    furi_log_puts("\r\n" _FURI_LOG_CLR_E "Usage fault\r\n");
    if(FURI_BIT(SCB->CFSR, SCB_CFSR_DIVBYZERO_Pos)) {
        furi_log_puts(" - division by zero\r\n");
    }

    if(FURI_BIT(SCB->CFSR, SCB_CFSR_UNALIGNED_Pos)) {
        furi_log_puts(" - unaligned access\r\n");
    }

    if(FURI_BIT(SCB->CFSR, SCB_CFSR_STKOF_Pos)) {
        furi_log_puts(" - stack overflow\r\n");
    }

    if(FURI_BIT(SCB->CFSR, SCB_CFSR_NOCP_Pos)) {
        furi_log_puts(" - no coprocessor\r\n");
    }

    if(FURI_BIT(SCB->CFSR, SCB_CFSR_INVPC_Pos)) {
        furi_log_puts(" - invalid PC\r\n");
    }

    if(FURI_BIT(SCB->CFSR, SCB_CFSR_INVSTATE_Pos)) {
        furi_log_puts(" - invalid state\r\n");
    }

    if(FURI_BIT(SCB->CFSR, SCB_CFSR_UNDEFINSTR_Pos)) {
        furi_log_puts(" - undefined instruction\r\n");
    }
    furi_log_puts(_FURI_LOG_CLR_RESET);

    furi_crash("UsageFault");
}

void DebugMon_Handler() {
}

void FPU_IRQHandler() {
    furi_crash("FpuFault");
}

void FuriSysTick_Handler(void) {
    // FURI_HAL_INTERRUPT_ACCOUNT_START();
    furi_hal_os_tick();
    // FURI_HAL_INTERRUPT_ACCOUNT_END();
}

// Potential space-saver for updater build
const char* furi_hal_interrupt_get_name(uint8_t exception_number) {
    const IRQn_Type irqn = (IRQn_Type)((int32_t)exception_number - 16);

    switch(irqn) {
    case NonMaskableInt_IRQn:
        return "NMI";
    case HardFault_IRQn:
        return "HardFault";
    case MemoryManagement_IRQn:
        return "MemMgmt";
    case BusFault_IRQn:
        return "BusFault";
    case UsageFault_IRQn:
        return "UsageFault";
    case SecureFault_IRQn:
        return "SecureFault";
    case SVCall_IRQn:
        return "SVC";
    case DebugMonitor_IRQn:
        return "DebugMon";
    case PendSV_IRQn:
        return "PendSV";
    case SysTick_IRQn:
        return "SysTick";
    case WWDG_IRQn:
        return "WWDG";
    case PVD_PVM_IRQn:
        return "PVD_PVM";
    case RTC_IRQn:
        return "RTC";
    case RTC_S_IRQn:
        return "RTC_S";
    case TAMP_IRQn:
        return "TAMP";
    case RAMCFG_IRQn:
        return "RAMCFG";
    case FLASH_IRQn:
        return "FLASH";
    case FLASH_S_IRQn:
        return "FLASH_S";
    case GTZC_IRQn:
        return "GTZC";
    case RCC_IRQn:
        return "RCC";
    case RCC_S_IRQn:
        return "RCC_S";
    case EXTI0_IRQn:
        return "EXTI0";
    case EXTI1_IRQn:
        return "EXTI1";
    case EXTI2_IRQn:
        return "EXTI2";
    case EXTI3_IRQn:
        return "EXTI3";
    case EXTI4_IRQn:
        return "EXTI4";
    case EXTI5_IRQn:
        return "EXTI5";
    case EXTI6_IRQn:
        return "EXTI6";
    case EXTI7_IRQn:
        return "EXTI7";
    case EXTI8_IRQn:
        return "EXTI8";
    case EXTI9_IRQn:
        return "EXTI9";
    case EXTI10_IRQn:
        return "EXTI10";
    case EXTI11_IRQn:
        return "EXTI11";
    case EXTI12_IRQn:
        return "EXTI12";
    case EXTI13_IRQn:
        return "EXTI13";
    case EXTI14_IRQn:
        return "EXTI14";
    case EXTI15_IRQn:
        return "EXTI15";
    case IWDG_IRQn:
        return "IWDG";
    case GPDMA1_Channel0_IRQn:
        return "GPDMA1_Channel0";
    case GPDMA1_Channel1_IRQn:
        return "GPDMA1_Channel1";
    case GPDMA1_Channel2_IRQn:
        return "GPDMA1_Channel2";
    case GPDMA1_Channel3_IRQn:
        return "GPDMA1_Channel3";
    case GPDMA1_Channel4_IRQn:
        return "GPDMA1_Channel4";
    case GPDMA1_Channel5_IRQn:
        return "GPDMA1_Channel5";
    case GPDMA1_Channel6_IRQn:
        return "GPDMA1_Channel6";
    case GPDMA1_Channel7_IRQn:
        return "GPDMA1_Channel7";
    case ADC1_2_IRQn:
        return "ADC1_2";
    case DAC1_IRQn:
        return "DAC1";
    case FDCAN1_IT0_IRQn:
        return "FDCAN1_IT0";
    case FDCAN1_IT1_IRQn:
        return "FDCAN1_IT1";
    case TIM1_BRK_IRQn:
        return "TIM1_BRK";
    case TIM1_UP_IRQn:
        return "TIM1_UP";
    case TIM1_TRG_COM_IRQn:
        return "TIM1_TRG_COM";
    case TIM1_CC_IRQn:
        return "TIM1_CC";
    case TIM2_IRQn:
        return "TIM2";
    case TIM3_IRQn:
        return "TIM3";
    case TIM4_IRQn:
        return "TIM4";
    case TIM5_IRQn:
        return "TIM5";
    case TIM6_IRQn:
        return "TIM6";
    case TIM7_IRQn:
        return "TIM7";
    case TIM8_BRK_IRQn:
        return "TIM8_BRK";
    case TIM8_UP_IRQn:
        return "TIM8_UP";
    case TIM8_TRG_COM_IRQn:
        return "TIM8_TRG_COM";
    case TIM8_CC_IRQn:
        return "TIM8_CC";
    case I2C1_EV_IRQn:
        return "I2C1_EV";
    case I2C1_ER_IRQn:
        return "I2C1_ER";
    case I2C2_EV_IRQn:
        return "I2C2_EV";
    case I2C2_ER_IRQn:
        return "I2C2_ER";
    case SPI1_IRQn:
        return "SPI1";
    case SPI2_IRQn:
        return "SPI2";
    case USART1_IRQn:
        return "USART1";
    case USART2_IRQn:
        return "USART2";
    case USART3_IRQn:
        return "USART3";
    case UART4_IRQn:
        return "UART4";
    case UART5_IRQn:
        return "UART5";
    case LPUART1_IRQn:
        return "LPUART1";
    case LPTIM1_IRQn:
        return "LPTIM1";
    case LPTIM2_IRQn:
        return "LPTIM2";
    case TIM15_IRQn:
        return "TIM15";
    case TIM16_IRQn:
        return "TIM16";
    case TIM17_IRQn:
        return "TIM17";
    case COMP_IRQn:
        return "COMP";
    case OTG_HS_IRQn:
        return "OTG_HS";
    case CRS_IRQn:
        return "CRS";
    case FMC_IRQn:
        return "FMC";
    case OCTOSPI1_IRQn:
        return "OCTOSPI1";
    case PWR_S3WU_IRQn:
        return "PWR_S3WU";
    case SDMMC1_IRQn:
        return "SDMMC1";
    case SDMMC2_IRQn:
        return "SDMMC2";
    case GPDMA1_Channel8_IRQn:
        return "GPDMA1_Channel8";
    case GPDMA1_Channel9_IRQn:
        return "GPDMA1_Channel9";
    case GPDMA1_Channel10_IRQn:
        return "GPDMA1_Channel10";
    case GPDMA1_Channel11_IRQn:
        return "GPDMA1_Channel11";
    case GPDMA1_Channel12_IRQn:
        return "GPDMA1_Channel12";
    case GPDMA1_Channel13_IRQn:
        return "GPDMA1_Channel13";
    case GPDMA1_Channel14_IRQn:
        return "GPDMA1_Channel14";
    case GPDMA1_Channel15_IRQn:
        return "GPDMA1_Channel15";
    case I2C3_EV_IRQn:
        return "I2C3_EV";
    case I2C3_ER_IRQn:
        return "I2C3_ER";
    case SAI1_IRQn:
        return "SAI1";
    case SAI2_IRQn:
        return "SAI2";
    case TSC_IRQn:
        return "TSC";
    case RNG_IRQn:
        return "RNG";
    case FPU_IRQn:
        return "FPU";
    case HASH_IRQn:
        return "HASH";
    case LPTIM3_IRQn:
        return "LPTIM3";
    case SPI3_IRQn:
        return "SPI3";
    case I2C4_ER_IRQn:
        return "I2C4_ER";
    case I2C4_EV_IRQn:
        return "I2C4_EV";
    case MDF1_FLT0_IRQn:
        return "MDF1_FLT0";
    case MDF1_FLT1_IRQn:
        return "MDF1_FLT1";
    case MDF1_FLT2_IRQn:
        return "MDF1_FLT2";
    case MDF1_FLT3_IRQn:
        return "MDF1_FLT3";
    case UCPD1_IRQn:
        return "UCPD1";
    case ICACHE_IRQn:
        return "ICACHE";
    case LPTIM4_IRQn:
        return "LPTIM4";
    case DCACHE1_IRQn:
        return "DCACHE1";
    case ADF1_IRQn:
        return "ADF1";
    case ADC4_IRQn:
        return "ADC4";
    case LPDMA1_Channel0_IRQn:
        return "LPDMA1_Channel0";
    case LPDMA1_Channel1_IRQn:
        return "LPDMA1_Channel1";
    case LPDMA1_Channel2_IRQn:
        return "LPDMA1_Channel2";
    case LPDMA1_Channel3_IRQn:
        return "LPDMA1_Channel3";
    case DMA2D_IRQn:
        return "DMA2D";
    case DCMI_PSSI_IRQn:
        return "DCMI_PSSI";
    case OCTOSPI2_IRQn:
        return "OCTOSPI2";
    case MDF1_FLT4_IRQn:
        return "MDF1_FLT4";
    case MDF1_FLT5_IRQn:
        return "MDF1_FLT5";
    case CORDIC_IRQn:
        return "CORDIC";
    case FMAC_IRQn:
        return "FMAC";
    case LSECSSD_IRQn:
        return "LSECSSD";
    case USART6_IRQn:
        return "USART6";
    case I2C5_ER_IRQn:
        return "I2C5_ER";
    case I2C5_EV_IRQn:
        return "I2C5_EV";
    case I2C6_ER_IRQn:
        return "I2C6_ER";
    case I2C6_EV_IRQn:
        return "I2C6_EV";
    case HSPI1_IRQn:
        return "HSPI1";
    default:
        return NULL;
    }
}

uint32_t furi_hal_interrupt_get_time_in_isr_total(void) {
    return furi_hal_interrupt.counter_time_in_isr_total;
}

void furi_hal_interrupt_assert_valid_priority(void) {
    uint32_t ulCurrentInterrupt = __get_IPSR();

    const uint32_t exti_priority = NVIC_GetPriority(ulCurrentInterrupt - 16);
    uint32_t group_priority, sub_priority;
    NVIC_DecodePriority(exti_priority, NVIC_GetPriorityGrouping(), &group_priority, &sub_priority);

    furi_check(group_priority >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
}
