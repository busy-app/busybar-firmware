#include <furi_hal_interrupt.h>
#include <furi_hal_os.h>

#include <furi.h>
#include <FreeRTOSConfig.h>

#include <si91x_device.h>

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
} FuriHalIterrupt;

static FuriHalIterrupt furi_hal_interrupt;

const IRQn_Type furi_hal_interrupt_irqn[FuriHalInterruptIdMax] = {
    [FuriHalInterruptIdTimer0] = TIMER0_IRQn,
    [FuriHalInterruptIdTimer1] = TIMER1_IRQn,
    [FuriHalInterruptIdTimer2] = TIMER2_IRQn,
    [FuriHalInterruptIdTimer3] = TIMER3_IRQn,
    [FuriHalInterruptIdQEI] = QEI_IRQn,
};

void furi_hal_interrupt_init(void) {
    NVIC_SetPriority(SVCall_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));

    FURI_LOG_I(TAG, "Init OK");
}

const char* furi_hal_interrupt_get_name(uint8_t exception_number) {
    const int32_t id = (int32_t)exception_number - 16;

    switch(id) {
    // Standard Cortex-M interrupts
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
    // Si917-specific interrupts
    case 2:
        return "TIMER0";
    case 3:
        return "TIMER1";
    case 4:
        return "TIMER2";
    case 5:
        return "TIMER3";
    case 6:
        return "CAP_SENSOR";
    case 7:
        return "COMP2";
    case 8:
        return "COMP1";
    case 10:
        return "UDMA1";
    case 11:
        return "ADC";
    case 12:
        return "ULPSS_UART";
    case 13:
        return "I2C2";
    case 14:
        return "I2S1";
    case 15:
        return "IR_DECODER";
    case 16:
        return "SSI2";
    case 17:
        return "FIM";
    case 18:
        return "ULP_EGPIO_PIN";
    case 19:
        return "ULP_EGPIO_GROUP";
    case 20:
        return "NPSS_TO_MCU_WDT_INTR";
    case 21:
        return "NPSS_TO_MCU_GPIO_INTR";
#ifdef SLI_SI917B0
    case 22:
        return "NPSS_TO_MCU_SYSRTC_INTR";
#else
    case 22:
        return "NPSS_TO_MCU_CMP_RF_WKP_INTR";
#endif
    case 23:
        return "NPSS_TO_MCU_BOD_INTR";
    case 24:
        return "NPSS_TO_MCU_BUTTON_INTR";
    case 25:
        return "NPSS_TO_MCU_SDC_INTR";
    case 26:
        return "NPSS_TO_MCU_WIRELESS_INTR";
    case 27:
        return "NPSS_MCU_INTR";
    case 28:
        return "MCU_CAL_ALARM";
    case 29:
        return "MCU_CAL_RTC";
    case 31:
        return "GPDMA";
    case 33:
        return "UDMA0";
    case 34:
        return "CT";
    case 35:
        return "HIF0";
    case 36:
        return "HIF1";
    case 37:
        return "SIO";
    case 38:
        return "USART0";
    case 39:
        return "UART1";
    case 41:
        return "EGPIO_WAKEUP";
    case 42:
        return "I2C0";
    case 44:
        return "SSISlave";
    case 46:
        return "GSPI0";
    case 47:
        return "SSI0";
    case 48:
        return "MCPWM";
    case 49:
        return "QEI";
    case 50:
        return "EGPIO_GROUP_0";
    case 51:
        return "EGPIO_GROUP_1";
    case 52:
        return "EGPIO_PIN_0";
    case 53:
        return "EGPIO_PIN_1";
    case 54:
        return "EGPIO_PIN_2";
    case 55:
        return "EGPIO_PIN_3";
    case 56:
        return "EGPIO_PIN_4";
    case 57:
        return "EGPIO_PIN_5";
    case 58:
        return "EGPIO_PIN_6";
    case 59:
        return "EGPIO_PIN_7";
    case 60:
        return "QSPI";
    case 61:
        return "I2C1";
#ifdef SLI_SI917B0
    case 62:
        return "MVP";
    case 63:
        return "MVP_WAKEUP";
#endif
    case 64:
        return "I2S0";
    case 69:
        return "PLL_CLOCK";
    case 74:
        return "TASS_P2P";
    default:
        return NULL;
    }
}

FURI_ALWAYS_INLINE static void furi_hal_interrupt_call(FuriHalInterruptId index) {
    const FuriHalInterruptISRPair* isr_descr = &furi_hal_interrupt.isr[index];
    furi_check(isr_descr->isr);

    FURI_HAL_INTERRUPT_ACCOUNT_START();
    isr_descr->isr(isr_descr->context);
    FURI_HAL_INTERRUPT_ACCOUNT_END();
}

FURI_ALWAYS_INLINE static void
    furi_hal_interrupt_enable(FuriHalInterruptId index, uint16_t priority) {
    NVIC_SetPriority(
        furi_hal_interrupt_irqn[index],
        NVIC_EncodePriority(NVIC_GetPriorityGrouping(), priority, 0));
    NVIC_EnableIRQ(furi_hal_interrupt_irqn[index]);
}

FURI_ALWAYS_INLINE static void furi_hal_interrupt_clear_pending(FuriHalInterruptId index) {
    NVIC_ClearPendingIRQ(furi_hal_interrupt_irqn[index]);
}

// FURI_ALWAYS_INLINE static void furi_hal_interrupt_get_pending(FuriHalInterruptId index) {
//     NVIC_GetPendingIRQ(furi_hal_interrupt_irqn[index]);
// }
//
// FURI_ALWAYS_INLINE static void furi_hal_interrupt_set_pending(FuriHalInterruptId index) {
//     NVIC_SetPendingIRQ(furi_hal_interrupt_irqn[index]);
// }

FURI_ALWAYS_INLINE static void furi_hal_interrupt_disable(FuriHalInterruptId index) {
    NVIC_DisableIRQ(furi_hal_interrupt_irqn[index]);
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

    FuriHalInterruptISRPair* isr_descr = &furi_hal_interrupt.isr[index];
    if(isr) {
        // Pre ISR set
        furi_check(isr_descr->isr == NULL);
    } else {
        // Pre ISR clear
        furi_hal_interrupt_disable(index);
        furi_hal_interrupt_clear_pending(index);
    }

    isr_descr->isr = isr;
    isr_descr->context = context;
    __DMB();

    if(isr) {
        // Post ISR set
        furi_hal_interrupt_clear_pending(index);
        furi_hal_interrupt_enable(index, real_priority);
    } else {
        // Post ISR clear
    }
}

uint32_t furi_hal_interrupt_get_time_in_isr_total(void) {
    return furi_hal_interrupt.counter_time_in_isr_total;
}

void NMI_Handler(void) {
    furi_crash("NMIHandler");
}

void HardFault_Handler(void) {
    furi_crash("HardFault");
}

void MemManage_Handler(void) {
    if(FURI_BIT(SCB->CFSR, SCB_CFSR_MMARVALID_Pos)) {
        uint32_t memfault_address = SCB->MMFAR;
        if(memfault_address < (1024 * 1024)) {
            // from 0x00 to 1MB, see FuriHalMpuRegionNULL
            furi_crash("NULL pointer dereference");
        } else {
            // write or read of MPU region 1 (FuriHalMpuRegionThreadStack)
            furi_crash("MPU fault, possibly stack overflow");
        }
    } else if(FURI_BIT(SCB->CFSR, SCB_CFSR_MSTKERR_Pos)) {
        // push to stack on MPU region 1 (FuriHalMpuRegionThreadStack)
        furi_crash("MemManage fault, possibly stack overflow");
    }

    furi_crash("MemManage");
}

void BusFault_Handler(void) {
    furi_crash("BusFault");
}

void UsageFault_Handler(void) {
    furi_crash("UsageFault");
}

void DebugMon_Handler(void) {
}

void SysTick_Handler(void) {
    FURI_HAL_INTERRUPT_ACCOUNT_START();
    furi_hal_os_tick();
    FURI_HAL_INTERRUPT_ACCOUNT_END();
}

void IRQ002_Handler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdTimer0);
}

void IRQ003_Handler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdTimer1);
}

void IRQ004_Handler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdTimer2);
}

void IRQ005_Handler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdTimer3);
}

// 6: ULP Processor Interrupt6
void IRQ006_Handler(void) {
}

// 7: ULP Processor Interrupt7
void IRQ007_Handler(void) {
}

// 8: ULP Processor Interrupt8
void IRQ008_Handler(void) {
}

// 9: ULP Processor Interrupt9
void IRQ009_Handler(void) {
}

// 11: ULP Processor Interrupt11
void IRQ011_Handler(void) {
}

// 13: ULP Processor Interrupt13
void IRQ013_Handler(void) {
}

// 14: ULP Processor Interrupt14
void IRQ014_Handler(void) {
}

// 15: ULP Processor Interrupt15
void IRQ015_Handler(void) {
}

// 16: ULP Processor Interrupt16
void IRQ016_Handler(void) {
}

// 17: ULP Processor Interrupt17
void IRQ017_Handler(void) {
}

// 20: UULP Interrupt0
void IRQ020_Handler(void) {
}

// 22: UULP Interrupt2
void IRQ022_Handler(void) {
}

// 23: UULP Interrupt3
void IRQ023_Handler(void) {
}

// 24: UULP Interrupt4
void IRQ024_Handler(void) {
}

// 25: UULP Interrupt5
void IRQ025_Handler(void) {
}

// 26: UULP Interrupt6
void IRQ026_Handler(void) {
}

// 27: UULP Interrupt7
void IRQ027_Handler(void) {
}

// 28: UULP Interrupt8
void IRQ028_Handler(void) {
}

// 29: UULP Interrupt9
void IRQ029_Handler(void) {
}

// 34: SCT interrupt
void IRQ034_Handler(void) {
}

// 35: HIF Interrupt1
void HIF1_IRQHandler(void) {
}

// 36: HIF Interrupt2
void HIF2_IRQHandler(void) {
}

// 37: SIO Interrupt
void IRQ037_Handler(void) {
}

// 41: GPIO Wakeup Interrupt
void IRQ041_Handler(void) {
}

// 42: I2C1 Interrupt
void IRQ042_Handler(void) {
}

// 44: SSI Slave Interrupt
void IRQ044_Handler(void) {
}

// 46: GSPI Master 1 Interrupt
void IRQ046_Handler(void) {
}

// 48: MCPWM Interrupt
void IRQ048_Handler(void) {
}

// 49: QEI Interrupt
void IRQ049_Handler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdQEI);
}

// 60: QSPI Interrupt
void IRQ060_Handler(void) {
}

// 61: I2C2 Interrupt
void IRQ061_Handler(void) {
}

// 62: Ethernet Interrupt
void IRQ062_Handler(void) {
}

// 64: I2S master Interrupt
void IRQ064_Handler(void) {
}

// 66: Can 1 Interrupt
void IRQ066_Handler(void) {
}

// 68: SDMEM Interrupt
void IRQ068_Handler(void) {
}

// 69: PLL clock ind Interrupt
void IRQ069_Handler(void) {
}

// 71: CCI system Interrupt Out
void IRQ071_Handler(void) {
}

// 72: FPU exception
void IRQ072_Handler(void) {
}

//   73: USB INTR
void IRQ073_Handler(void) {
}
