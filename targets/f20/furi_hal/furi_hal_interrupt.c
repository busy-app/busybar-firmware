#include <furi_hal.h>
#include <FreeRTOS.h>
#include <stm32u5xx_ll_cortex.h>
#include <stm32u5xx_ll_system.h>

#define TAG "FuriHalInterrupt"

#define FURI_HAL_INTERRUPT_DEFAULT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 5)

typedef struct {
    FuriHalInterruptISR isr;
    void* context;
} FuriHalInterruptISRPair;

FuriHalInterruptISRPair furi_hal_interrupt_isr[FuriHalInterruptIdMax] = {0};

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
    furi_check(furi_hal_interrupt_isr[index].isr);
    furi_hal_interrupt_isr[index].isr(furi_hal_interrupt_isr[index].context);
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
    furi_hal_interrupt_set_isr_ex(index, FuriHalInterruptPriorityNormal, isr, context);
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
        furi_check(furi_hal_interrupt_isr[index].isr == NULL);
    } else {
        // Pre ISR clear
        furi_hal_interrupt_disable(index);
        furi_hal_interrupt_clear_pending(index);
    }

    furi_hal_interrupt_isr[index].isr = isr;
    furi_hal_interrupt_isr[index].context = context;
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
    int32_t id = (int32_t)exception_number - 16;

    switch(id) {
    case -14:
        return "NMI";
    case -13:
        return "HardFault";
    case -12:
        return "MemMgmt";
    case -11:
        return "BusFault";
    case -10:
        return "UsageFault";
    case -5:
        return "SVC";
    case -4:
        return "DebugMon";
    case -2:
        return "PendSV";
    case -1:
        return "SysTick";
    case 0:
        return "WWDG";
    case 1:
        return "PVD_PVM";
    case 2:
        return "TAMP";
    case 3:
        return "RTC_WKUP";
    case 4:
        return "FLASH";
    case 5:
        return "RCC";
    case 6:
        return "EXTI0";
    case 7:
        return "EXTI1";
    case 8:
        return "EXTI2";
    case 9:
        return "EXTI3";
    case 10:
        return "EXTI4";
    case 11:
        return "DMA1_Ch1";
    case 12:
        return "DMA1_Ch2";
    case 13:
        return "DMA1_Ch3";
    case 14:
        return "DMA1_Ch4";
    case 15:
        return "DMA1_Ch5";
    case 16:
        return "DMA1_Ch6";
    case 17:
        return "DMA1_Ch7";
    case 18:
        return "ADC1";
    case 19:
        return "USB_HP";
    case 20:
        return "USB_LP";
    case 21:
        return "C2SEV_PWR_C2H";
    case 22:
        return "COMP";
    case 23:
        return "EXTI9_5";
    case 24:
        return "TIM1_BRK";
    case 25:
        return "TIM1_UP_TIM16";
    case 26:
        return "TIM1_TRG_COM_TIM17";
    case 27:
        return "TIM1_CC";
    case 28:
        return "TIM2";
    case 29:
        return "PKA";
    case 30:
        return "I2C1_EV";
    case 31:
        return "I2C1_ER";
    case 32:
        return "I2C3_EV";
    case 33:
        return "I2C3_ER";
    case 34:
        return "SPI1";
    case 35:
        return "SPI2";
    case 36:
        return "USART1";
    case 37:
        return "LPUART1";
    case 38:
        return "SAI1";
    case 39:
        return "TSC";
    case 40:
        return "EXTI15_10";
    case 41:
        return "RTC_Alarm";
    case 42:
        return "CRS";
    case 43:
        return "PWR_SOTF_BLE";
    case 44:
        return "IPCC_C1_RX";
    case 45:
        return "IPCC_C1_TX";
    case 46:
        return "HSEM";
    case 47:
        return "LPTIM1";
    case 48:
        return "LPTIM2";
    case 49:
        return "LCD";
    case 50:
        return "QUADSPI";
    case 51:
        return "AES1";
    case 52:
        return "AES2";
    case 53:
        return "RNG";
    case 54:
        return "FPU";
    case 55:
        return "DMA2_Ch1";
    case 56:
        return "DMA2_Ch2";
    case 57:
        return "DMA2_Ch3";
    case 58:
        return "DMA2_Ch4";
    case 59:
        return "DMA2_Ch5";
    case 60:
        return "DMA2_Ch6";
    case 61:
        return "DMA2_Ch7";
    case 62:
        return "DMAMUX1_OVR";
    default:
        return NULL;
    }
}

uint32_t furi_hal_interrupt_get_time_in_isr_total(void) {
    // return furi_hal_interrupt.counter_time_in_isr_total; // TODO
    return 0;
}

void furi_hal_interrupt_assert_valid_priority(void) {
    uint32_t ulCurrentInterrupt = __get_IPSR();

    const uint32_t exti_priority = NVIC_GetPriority(ulCurrentInterrupt - 16);
    uint32_t group_priority, sub_priority;
    NVIC_DecodePriority(exti_priority, NVIC_GetPriorityGrouping(), &group_priority, &sub_priority);

    furi_check(group_priority >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
}
