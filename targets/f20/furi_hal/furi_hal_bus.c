#include <furi_hal_bus.h>
#include <furi.h>

#include <stm32u5xx_ll_bus.h>

/* Bus bitmask definitions */
#define FURI_HAL_BUS_IGNORE (0x0U)

#define FURI_HAL_BUS_AHB1_GRP1   (LL_AHB1_GRP1_PERIPH_ALL)
#define FURI_HAL_BUS_AHB2_GRP1   (LL_AHB2_GRP1_PERIPH_ALL)
#define FURI_HAL_BUS_AHB3_GRP1   (LL_AHB3_GRP1_PERIPH_ALL)
#define FURI_HAL_BUS_AHB2_GRP2   (LL_AHB2_GRP2_PERIPH_ALL)
#define FURI_HAL_BUS_APB1_GRP1   (LL_APB1_GRP1_PERIPH_ALL)
#define FURI_HAL_BUS_APB1_GRP2   (LL_APB1_GRP2_PERIPH_ALL)
#define FURI_HAL_BUS_APB2_GRP1   (LL_APB2_GRP1_PERIPH_ALL)
#define FURI_HAL_BUS_APB3_GRP1   (LL_APB3_GRP1_PERIPH_ALL)
#define FURI_HAL_BUS_SRDAMR_GRP1 (LL_SRDAMR_GRP1_PERIPH_ALL)

/* Test macro definitions */
#define FURI_HAL_BUS_IS_ALL_CLEAR(reg, value) (READ_BIT((reg), (value)) == 0UL)
#define FURI_HAL_BUS_IS_ALL_SET(reg, value)   (READ_BIT((reg), (value)) == (value))

#define FURI_HAL_BUS_IS_CLOCK_ENABLED(bus, value, ...) \
    (FURI_HAL_BUS_IS_ALL_SET(RCC->bus##ENR##__VA_ARGS__, (value)))
#define FURI_HAL_BUS_IS_CLOCK_DISABLED(bus, value, ...) \
    (FURI_HAL_BUS_IS_ALL_CLEAR(RCC->bus##ENR##__VA_ARGS__, (value)))

#define FURI_HAL_BUS_IS_RESET_ASSERTED(bus, value, ...) \
    (FURI_HAL_BUS_IS_ALL_SET(RCC->bus##RSTR##__VA_ARGS__, (value)))
#define FURI_HAL_BUS_IS_RESET_DEASSERTED(bus, value, ...) \
    (FURI_HAL_BUS_IS_ALL_CLEAR(RCC->bus##RSTR##__VA_ARGS__, (value)))

#define FURI_HAL_BUS_IS_PERIPH_ENABLED(bus, value, ...)             \
    (FURI_HAL_BUS_IS_RESET_DEASSERTED(bus, (value), __VA_ARGS__) && \
     FURI_HAL_BUS_IS_CLOCK_ENABLED(bus, (value), __VA_ARGS__))

#define FURI_HAL_BUS_IS_PERIPH_DISABLED(bus, value, ...)          \
    (FURI_HAL_BUS_IS_CLOCK_DISABLED(bus, (value), __VA_ARGS__) && \
     FURI_HAL_BUS_IS_RESET_ASSERTED(bus, (value), __VA_ARGS__))

/* Control macro definitions */
#define FURI_HAL_BUS_RESET_ASSERT(bus, value, grp)   LL_##bus##_GRP##grp##_ForceReset(value)
#define FURI_HAL_BUS_RESET_DEASSERT(bus, value, grp) LL_##bus##_GRP##grp##_ReleaseReset(value)

#define FURI_HAL_BUS_CLOCK_ENABLE(bus, value, grp)  LL_##bus##_GRP##grp##_EnableClock(value)
#define FURI_HAL_BUS_CLOCK_DISABLE(bus, value, grp) LL_##bus##_GRP##grp##_DisableClock(value)

#define FURI_HAL_BUS_PERIPH_ENABLE(bus, value, grp) \
    FURI_HAL_BUS_CLOCK_ENABLE(bus, value, grp);     \
    FURI_HAL_BUS_RESET_DEASSERT(bus, value, grp)

#define FURI_HAL_BUS_PERIPH_DISABLE(bus, value, grp) \
    FURI_HAL_BUS_RESET_ASSERT(bus, value, grp);      \
    FURI_HAL_BUS_CLOCK_DISABLE(bus, value, grp)

#define FURI_HAL_BUS_PERIPH_RESET(bus, value, grp) \
    FURI_HAL_BUS_RESET_ASSERT(bus, value, grp);    \
    FURI_HAL_BUS_RESET_DEASSERT(bus, value, grp)

// TODO: static
const uint32_t furi_hal_bus[] = {
    [FuriHalBusAHB1_GRP1] = FURI_HAL_BUS_IGNORE,
    [FuriHalBusGPDMA1] = LL_AHB1_GRP1_PERIPH_GPDMA1,
    [FuriHalBusCORDIC] = LL_AHB1_GRP1_PERIPH_CORDIC,
    [FuriHalBusFMAC] = LL_AHB1_GRP1_PERIPH_FMAC,
    [FuriHalBusMDF1] = LL_AHB1_GRP1_PERIPH_MDF1,
    [FuriHalBusFLASH] = LL_AHB1_GRP1_PERIPH_FLASH,
    [FuriHalBusCRC] = LL_AHB1_GRP1_PERIPH_CRC,
#if defined(JPEG)
    [FuriHalBusJPEG] = LL_AHB1_GRP1_PERIPH_JPEG,
#endif
    [FuriHalBusTSC] = LL_AHB1_GRP1_PERIPH_TSC,
    [FuriHalBusRAMCFG] = LL_AHB1_GRP1_PERIPH_RAMCFG,
#if defined(DMA2D)
    [FuriHalBusDMA2D] = LL_AHB1_GRP1_PERIPH_DMA2D,
#endif
#if defined(GFXMMU)
    [FuriHalBusGFXMMU] = LL_AHB1_GRP1_PERIPH_GFXMMU,
#endif
#if defined(GPU2D)
    [FuriHalBusGPU2D] = LL_AHB1_GRP1_PERIPH_GPU2D,
#endif
#if defined(DCACHE2)
    [FuriHalBusDCACHE2] = LL_AHB1_GRP1_PERIPH_DCACHE2,
#endif
    [FuriHalBusGTZC1] = LL_AHB1_GRP1_PERIPH_GTZC1,
    [FuriHalBusBKPSRAM] = LL_AHB1_GRP1_PERIPH_BKPSRAM,
    [FuriHalBusICACHE1] = LL_AHB1_GRP1_PERIPH_ICACHE1,
    [FuriHalBusDCACHE1] = LL_AHB1_GRP1_PERIPH_DCACHE1,
    [FuriHalBusSRAM1] = LL_AHB1_GRP1_PERIPH_SRAM1,

    [FuriHalBusAHB2_GRP1] = FURI_HAL_BUS_IGNORE,
    [FuriHalBusGPIOA] = LL_AHB2_GRP1_PERIPH_GPIOA,
    [FuriHalBusGPIOB] = LL_AHB2_GRP1_PERIPH_GPIOB,
    [FuriHalBusGPIOC] = LL_AHB2_GRP1_PERIPH_GPIOC,
    [FuriHalBusGPIOD] = LL_AHB2_GRP1_PERIPH_GPIOD,
    [FuriHalBusGPIOE] = LL_AHB2_GRP1_PERIPH_GPIOE,
#if defined(GPIOF)
    [FuriHalBusGPIOF] = LL_AHB2_GRP1_PERIPH_GPIOF,
#endif
    [FuriHalBusGPIOG] = LL_AHB2_GRP1_PERIPH_GPIOG,
    [FuriHalBusGPIOH] = LL_AHB2_GRP1_PERIPH_GPIOH,
#if defined(GPIOI)
    [FuriHalBusGPIOI] = LL_AHB2_GRP1_PERIPH_GPIOI,
#endif
#if defined(GPIOJ)
    [FuriHalBusGPIOJ] = LL_AHB2_GRP1_PERIPH_GPIOJ,
#endif
    [FuriHalBusADC12] = LL_AHB2_GRP1_PERIPH_ADC12,
    [FuriHalBusDCMI_PSSI] = LL_AHB2_GRP1_PERIPH_DCMI_PSSI,
#if defined(USB_OTG_FS)
    [FuriHalBusOTG_FS] = LL_AHB2_GRP1_PERIPH_OTG_FS,
    [FuriHalBusUSBFS] = LL_AHB2_GRP1_PERIPH_USBFS,
#elif defined(USB_OTG_HS)
    [FuriHalBusOTG_HS] = LL_AHB2_GRP1_PERIPH_OTG_HS,
    [FuriHalBusUSBHS] = LL_AHB2_GRP1_PERIPH_USBHS,
#endif
#if defined(RCC_AHB2ENR1_USBPHYCEN)
    [FuriHalBusUSBPHY] = LL_AHB2_GRP1_PERIPH_USBPHY,
#endif
#if defined(AES)
    [FuriHalBusAES] = LL_AHB2_GRP1_PERIPH_AES,
#endif
#if defined(HASH)
    [FuriHalBusHASH] = LL_AHB2_GRP1_PERIPH_HASH,
#endif
    [FuriHalBusRNG] = LL_AHB2_GRP1_PERIPH_RNG,
#if defined(PKA)
    [FuriHalBusPKA] = LL_AHB2_GRP1_PERIPH_PKA,
#endif
#if defined(SAES)
    [FuriHalBusSAES] = LL_AHB2_GRP1_PERIPH_SAES,
#endif
#if defined(OCTOSPIM)
    [FuriHalBusOCTOSPIM] = LL_AHB2_GRP1_PERIPH_OCTOSPIM,
#endif
#if defined(OTFDEC1)
    [FuriHalBusOTFDEC1] = LL_AHB2_GRP1_PERIPH_OTFDEC1,
#endif
#if defined(OTFDEC2)
    [FuriHalBusOTFDEC2] = LL_AHB2_GRP1_PERIPH_OTFDEC2,
#endif
    [FuriHalBusSDMMC1] = LL_AHB2_GRP1_PERIPH_SDMMC1,
#if defined(SDMMC2)
    [FuriHalBusSDMMC2] = LL_AHB2_GRP1_PERIPH_SDMMC2,
#endif
    [FuriHalBusSRAM2] = LL_AHB2_GRP1_PERIPH_SRAM2,
#if defined(SRAM3_BASE)
    [FuriHalBusSRAM3] = LL_AHB2_GRP1_PERIPH_SRAM3,
#endif

    [FuriHalBusAHB3_GRP1] = FURI_HAL_BUS_IGNORE,
    [FuriHalBusLPGPIO1] = LL_AHB3_GRP1_PERIPH_LPGPIO1,
    [FuriHalBusPWR] = LL_AHB3_GRP1_PERIPH_PWR,
    [FuriHalBusADC4] = LL_AHB3_GRP1_PERIPH_ADC4,
    [FuriHalBusDAC1] = LL_AHB3_GRP1_PERIPH_DAC1,
    [FuriHalBusLPDMA1] = LL_AHB3_GRP1_PERIPH_LPDMA1,
    [FuriHalBusADF1] = LL_AHB3_GRP1_PERIPH_ADF1,
    [FuriHalBusGTZC2] = LL_AHB3_GRP1_PERIPH_GTZC2,
    [FuriHalBusSRAM4] = LL_AHB3_GRP1_PERIPH_SRAM4,

    [FuriHalBusAHB2_GRP2] = FURI_HAL_BUS_IGNORE,
#if defined(FMC_BASE)
    [FuriHalBusFSMC] = LL_AHB2_GRP2_PERIPH_FSMC,
#endif
    [FuriHalBusOCTOSPI1] = LL_AHB2_GRP2_PERIPH_OCTOSPI1,
#if defined(OCTOSPI2)
    [FuriHalBusOCTOSPI2] = LL_AHB2_GRP2_PERIPH_OCTOSPI2,
#endif
#if defined(HSPI1)
    [FuriHalBusHSPI1] = LL_AHB2_GRP2_PERIPH_HSPI1,
#endif
#if defined(SRAM6_BASE)
    [FuriHalBusSRAM6] = LL_AHB2_GRP2_PERIPH_SRAM6,
#endif
#if defined(SRAM5_BASE)
    [FuriHalBusSRAM5] = LL_AHB2_GRP2_PERIPH_SRAM5,
#endif

    [FuriHalBusAPB1_GRP1] = FURI_HAL_BUS_APB1_GRP1,
    [FuriHalBusTIM2] = LL_APB1_GRP1_PERIPH_TIM2,
    [FuriHalBusTIM3] = LL_APB1_GRP1_PERIPH_TIM3,
    [FuriHalBusTIM4] = LL_APB1_GRP1_PERIPH_TIM4,
    [FuriHalBusTIM5] = LL_APB1_GRP1_PERIPH_TIM5,
    [FuriHalBusTIM6] = LL_APB1_GRP1_PERIPH_TIM6,
    [FuriHalBusTIM7] = LL_APB1_GRP1_PERIPH_TIM7,
    [FuriHalBusWWDG] = LL_APB1_GRP1_PERIPH_WWDG,
    [FuriHalBusSPI2] = LL_APB1_GRP1_PERIPH_SPI2,
#if defined(USART2)
    [FuriHalBusUSART2] = LL_APB1_GRP1_PERIPH_USART2,
#endif
    [FuriHalBusUSART3] = LL_APB1_GRP1_PERIPH_USART3,
    [FuriHalBusUART4] = LL_APB1_GRP1_PERIPH_UART4,
    [FuriHalBusUART5] = LL_APB1_GRP1_PERIPH_UART5,
    [FuriHalBusI2C1] = LL_APB1_GRP1_PERIPH_I2C1,
    [FuriHalBusI2C2] = LL_APB1_GRP1_PERIPH_I2C2,
    [FuriHalBusCRS] = LL_APB1_GRP1_PERIPH_CRS,
#if defined(USART6)
    [FuriHalBusUSART6] = LL_APB1_GRP1_PERIPH_USART6,
#endif

    [FuriHalBusAPB1_GRP2] = FURI_HAL_BUS_APB1_GRP2,
    [FuriHalBusI2C4] = LL_APB1_GRP2_PERIPH_I2C4,
    [FuriHalBusLPTIM2] = LL_APB1_GRP2_PERIPH_LPTIM2,
    [FuriHalBusFDCAN1] = LL_APB1_GRP2_PERIPH_FDCAN1,
#if defined(UCPD1)
    [FuriHalBusUCPD1] = LL_APB1_GRP2_PERIPH_UCPD1,
#endif
#if defined(I2C5)
    [FuriHalBusI2C5] = LL_APB1_GRP2_PERIPH_I2C5,
#endif
#if defined(I2C6)
    [FuriHalBusI2C6] = LL_APB1_GRP2_PERIPH_I2C6,
#endif

    [FuriHalBusAPB2_GRP1] = FURI_HAL_BUS_IGNORE,
    [FuriHalBusTIM1] = LL_APB2_GRP1_PERIPH_TIM1,
    [FuriHalBusSPI1] = LL_APB2_GRP1_PERIPH_SPI1,
    [FuriHalBusTIM8] = LL_APB2_GRP1_PERIPH_TIM8,
    [FuriHalBusUSART1] = LL_APB2_GRP1_PERIPH_USART1,
    [FuriHalBusTIM15] = LL_APB2_GRP1_PERIPH_TIM15,
    [FuriHalBusTIM16] = LL_APB2_GRP1_PERIPH_TIM16,
    [FuriHalBusTIM17] = LL_APB2_GRP1_PERIPH_TIM17,
    [FuriHalBusSAI1] = LL_APB2_GRP1_PERIPH_SAI1,
#if defined(SAI2)
    [FuriHalBusSAI2] = LL_APB2_GRP1_PERIPH_SAI2,
#endif
#if defined(USB_DRD_FS)
    [FuriHalBusUSB_FS] = LL_APB2_GRP1_PERIPH_USB_FS,
#endif
#if defined(GFXTIM)
    [FuriHalBusGFXTIM] = LL_APB2_GRP1_PERIPH_GFXTIM,
#endif
#if defined(LTDC)
    [FuriHalBusLTDC] = LL_APB2_GRP1_PERIPH_LTDC,
#endif
#if defined(DSI)
    [FuriHalBusDSI] = LL_APB2_GRP1_PERIPH_DSI,
#endif

    [FuriHalBusAPB3_GRP1] = FURI_HAL_BUS_APB3_GRP1,
    [FuriHalBusSYSCFG] = LL_APB3_GRP1_PERIPH_SYSCFG,
    [FuriHalBusSPI3] = LL_APB3_GRP1_PERIPH_SPI3,
    [FuriHalBusLPUART1] = LL_APB3_GRP1_PERIPH_LPUART1,
    [FuriHalBusI2C3] = LL_APB3_GRP1_PERIPH_I2C3,
    [FuriHalBusLPTIM1] = LL_APB3_GRP1_PERIPH_LPTIM1,
    [FuriHalBusLPTIM3] = LL_APB3_GRP1_PERIPH_LPTIM3,
    [FuriHalBusLPTIM4] = LL_APB3_GRP1_PERIPH_LPTIM4,
    [FuriHalBusOPAMP] = LL_APB3_GRP1_PERIPH_OPAMP,
    [FuriHalBusCOMP] = LL_APB3_GRP1_PERIPH_COMP,
    [FuriHalBusVREF] = LL_APB3_GRP1_PERIPH_VREF,
    [FuriHalBusRTCAPB] = LL_APB3_GRP1_PERIPH_RTCAPB,

    // TODO: special case for SRDAMR GRP1
    // [FuriHalBusSRDAMR_GRP1] = FURI_HAL_BUS_SRDAMR_GRP1,
    // [FuriHalBusSRD_SPI3] = LL_SRDAMR_GRP1_PERIPH_SPI3,
    // [FuriHalBusSRD_LPUART1] = LL_SRDAMR_GRP1_PERIPH_LPUART1,
    // [FuriHalBusSRD_I2C3] = LL_SRDAMR_GRP1_PERIPH_I2C3,
    // [FuriHalBusSRD_LPTIM1] = LL_SRDAMR_GRP1_PERIPH_LPTIM1,
    // [FuriHalBusSRD_LPTIM3] = LL_SRDAMR_GRP1_PERIPH_LPTIM3,
    // [FuriHalBusSRD_LPTIM4] = LL_SRDAMR_GRP1_PERIPH_LPTIM4,
    // [FuriHalBusSRD_OPAMP] = LL_SRDAMR_GRP1_PERIPH_OPAMP,
    // [FuriHalBusSRD_COMP] = LL_SRDAMR_GRP1_PERIPH_COMP,
    // [FuriHalBusSRD_VREF] = LL_SRDAMR_GRP1_PERIPH_VREF,
    // [FuriHalBusSRD_RTCAPB] = LL_SRDAMR_GRP1_PERIPH_RTCAPB,
    // [FuriHalBusSRD_ADC4] = LL_SRDAMR_GRP1_PERIPH_ADC4,
    // [FuriHalBusSRD_LPGPIO1] = LL_SRDAMR_GRP1_PERIPH_LPGPIO1,
    // [FuriHalBusSRD_DAC1] = LL_SRDAMR_GRP1_PERIPH_DAC1,
    // [FuriHalBusSRD_LPDMA1] = LL_SRDAMR_GRP1_PERIPH_LPDMA1,
    // [FuriHalBusSRD_ADF1] = LL_SRDAMR_GRP1_PERIPH_ADF1,
    // [FuriHalBusSRD_SRAM4] = LL_SRDAMR_GRP1_PERIPH_SRAM4,
};

void furi_hal_bus_init_early(void) {
    FURI_CRITICAL_ENTER();

    // FURI_HAL_BUS_PERIPH_DISABLE(AHB1, FURI_HAL_BUS_AHB1_GRP1, 1);
    // FURI_HAL_BUS_PERIPH_DISABLE(AHB2, FURI_HAL_BUS_AHB2_GRP1, 1);
    // FURI_HAL_BUS_PERIPH_DISABLE(AHB3, FURI_HAL_BUS_AHB3_GRP1, 1);
    FURI_HAL_BUS_PERIPH_DISABLE(APB1, FURI_HAL_BUS_APB1_GRP1, 1);
    FURI_HAL_BUS_PERIPH_DISABLE(APB1, FURI_HAL_BUS_APB1_GRP2, 2);
    FURI_HAL_BUS_PERIPH_DISABLE(APB2, FURI_HAL_BUS_APB2_GRP1, 1);
    FURI_HAL_BUS_PERIPH_DISABLE(APB3, FURI_HAL_BUS_APB3_GRP1, 1);

    FURI_CRITICAL_EXIT();
}

void furi_hal_bus_deinit_early(void) {
    FURI_CRITICAL_ENTER();

    // FURI_HAL_BUS_PERIPH_ENABLE(AHB1, FURI_HAL_BUS_AHB1_GRP1, 1);
    // FURI_HAL_BUS_PERIPH_ENABLE(AHB2, FURI_HAL_BUS_AHB2_GRP1, 1);
    // FURI_HAL_BUS_PERIPH_ENABLE(AHB3, FURI_HAL_BUS_AHB3_GRP1, 1);
    FURI_HAL_BUS_PERIPH_ENABLE(APB1, FURI_HAL_BUS_APB1_GRP1, 1);
    FURI_HAL_BUS_PERIPH_ENABLE(APB1, FURI_HAL_BUS_APB1_GRP2, 2);
    FURI_HAL_BUS_PERIPH_ENABLE(APB2, FURI_HAL_BUS_APB2_GRP1, 1);
    FURI_HAL_BUS_PERIPH_ENABLE(APB3, FURI_HAL_BUS_APB3_GRP1, 1);

    FURI_CRITICAL_EXIT();
}

void furi_hal_bus_enable(FuriHalBus bus) {
    furi_check(bus < FuriHalBusMAX);
    const uint32_t value = furi_hal_bus[bus];
    if(!value) {
        return;
    }

    FURI_CRITICAL_ENTER();
    if(bus < FuriHalBusAHB2_GRP1) {
        // furi_check(FURI_HAL_BUS_IS_PERIPH_DISABLED(AHB1, value));
        FURI_HAL_BUS_PERIPH_ENABLE(AHB1, value, 1);
    } else if(bus < FuriHalBusAHB3_GRP1) {
        // furi_check(FURI_HAL_BUS_IS_PERIPH_DISABLED(AHB2, value));
        FURI_HAL_BUS_PERIPH_ENABLE(AHB2, value, 1);
    } else if(bus < FuriHalBusAHB2_GRP2) {
        // furi_check(FURI_HAL_BUS_IS_PERIPH_DISABLED(AHB3, value));
        FURI_HAL_BUS_PERIPH_ENABLE(AHB3, value, 1);
    } else if(bus < FuriHalBusAPB1_GRP1) {
        // furi_check(FURI_HAL_BUS_IS_PERIPH_DISABLED(AHB2, value, 2));
        FURI_HAL_BUS_PERIPH_ENABLE(AHB2, value, 2);
    } else if(bus < FuriHalBusAPB1_GRP2) {
        furi_check(FURI_HAL_BUS_IS_PERIPH_DISABLED(APB1, value, 1));
        FURI_HAL_BUS_PERIPH_ENABLE(APB1, value, 1);
    } else if(bus < FuriHalBusAPB2_GRP1) {
        furi_check(FURI_HAL_BUS_IS_PERIPH_DISABLED(APB1, value, 2));
        FURI_HAL_BUS_PERIPH_ENABLE(APB1, value, 2);
    } else if(bus < FuriHalBusAPB3_GRP1) {
        furi_check(FURI_HAL_BUS_IS_PERIPH_DISABLED(APB2, value));
        FURI_HAL_BUS_PERIPH_ENABLE(APB2, value, 1);
    } else {
        furi_check(FURI_HAL_BUS_IS_PERIPH_DISABLED(APB3, value));
        FURI_HAL_BUS_PERIPH_ENABLE(APB3, value, 1);
    }
    FURI_CRITICAL_EXIT();
}

void furi_hal_bus_reset(FuriHalBus bus) {
    furi_check(bus < FuriHalBusMAX);
    const uint32_t value = furi_hal_bus[bus];
    if(!value) {
        return;
    }

    FURI_CRITICAL_ENTER();
    if(bus < FuriHalBusAHB2_GRP1) {
        // furi_check(FURI_HAL_BUS_IS_PERIPH_ENABLED(AHB1, value));
        FURI_HAL_BUS_PERIPH_RESET(AHB1, value, 1);
    } else if(bus < FuriHalBusAHB3_GRP1) {
        // furi_check(FURI_HAL_BUS_IS_PERIPH_ENABLED(AHB2, value));
        FURI_HAL_BUS_PERIPH_RESET(AHB2, value, 1);
    } else if(bus < FuriHalBusAHB2_GRP2) {
        // furi_check(FURI_HAL_BUS_IS_PERIPH_ENABLED(AHB3, value));
        FURI_HAL_BUS_PERIPH_RESET(AHB3, value, 1);
    } else if(bus < FuriHalBusAPB1_GRP1) {
        // furi_check(FURI_HAL_BUS_IS_PERIPH_ENABLED(AHB2, value, 2));
        FURI_HAL_BUS_PERIPH_RESET(AHB2, value, 2);
    } else if(bus < FuriHalBusAPB1_GRP2) {
        furi_check(FURI_HAL_BUS_IS_PERIPH_ENABLED(APB1, value, 1));
        FURI_HAL_BUS_PERIPH_RESET(APB1, value, 1);
    } else if(bus < FuriHalBusAPB2_GRP1) {
        furi_check(FURI_HAL_BUS_IS_PERIPH_ENABLED(APB1, value, 2));
        FURI_HAL_BUS_PERIPH_RESET(APB1, value, 2);
    } else if(bus < FuriHalBusAPB3_GRP1) {
        furi_check(FURI_HAL_BUS_IS_PERIPH_ENABLED(APB2, value));
        FURI_HAL_BUS_PERIPH_RESET(APB2, value, 1);
    } else {
        furi_check(FURI_HAL_BUS_IS_PERIPH_ENABLED(APB3, value));
        FURI_HAL_BUS_PERIPH_RESET(APB3, value, 1);
    }
    FURI_CRITICAL_EXIT();
}

void furi_hal_bus_disable(FuriHalBus bus) {
    furi_check(bus < FuriHalBusMAX);
    const uint32_t value = furi_hal_bus[bus];
    if(!value) {
        return;
    }

    FURI_CRITICAL_ENTER();
    if(bus < FuriHalBusAHB2_GRP1) {
        // furi_check(FURI_HAL_BUS_IS_PERIPH_ENABLED(AHB1, value));
        FURI_HAL_BUS_PERIPH_DISABLE(AHB1, value, 1);
    } else if(bus < FuriHalBusAHB3_GRP1) {
        // furi_check(FURI_HAL_BUS_IS_PERIPH_ENABLED(AHB2, value));
        FURI_HAL_BUS_PERIPH_DISABLE(AHB2, value, 1);
    } else if(bus < FuriHalBusAHB2_GRP2) {
        // furi_check(FURI_HAL_BUS_IS_PERIPH_ENABLED(AHB3, value));
        FURI_HAL_BUS_PERIPH_DISABLE(AHB3, value, 1);
    } else if(bus < FuriHalBusAPB1_GRP1) {
        // furi_check(FURI_HAL_BUS_IS_PERIPH_ENABLED(AHB2, value, 2));
        FURI_HAL_BUS_PERIPH_DISABLE(AHB2, value, 2);
    } else if(bus < FuriHalBusAPB1_GRP2) {
        furi_check(FURI_HAL_BUS_IS_PERIPH_ENABLED(APB1, value, 1));
        FURI_HAL_BUS_PERIPH_DISABLE(APB1, value, 1);
    } else if(bus < FuriHalBusAPB2_GRP1) {
        furi_check(FURI_HAL_BUS_IS_PERIPH_ENABLED(APB1, value, 2));
        FURI_HAL_BUS_PERIPH_DISABLE(APB1, value, 2);
    } else if(bus < FuriHalBusAPB3_GRP1) {
        furi_check(FURI_HAL_BUS_IS_PERIPH_ENABLED(APB2, value));
        FURI_HAL_BUS_PERIPH_DISABLE(APB2, value, 1);
    } else {
        furi_check(FURI_HAL_BUS_IS_PERIPH_ENABLED(APB3, value));
        FURI_HAL_BUS_PERIPH_DISABLE(APB3, value, 1);
    }
    FURI_CRITICAL_EXIT();
}

bool furi_hal_bus_is_enabled(FuriHalBus bus) {
    furi_check(bus < FuriHalBusMAX);
    const uint32_t value = furi_hal_bus[bus];
    if(value == FURI_HAL_BUS_IGNORE) {
        return true;
    }

    bool ret = false;
    FURI_CRITICAL_ENTER();
    if(bus < FuriHalBusAHB2_GRP1) {
        ret = FURI_HAL_BUS_IS_PERIPH_ENABLED(AHB1, value);
    } else if(bus < FuriHalBusAHB3_GRP1) {
        ret = FURI_HAL_BUS_IS_PERIPH_ENABLED(AHB2, value, 1);
    } else if(bus < FuriHalBusAHB2_GRP2) {
        ret = FURI_HAL_BUS_IS_PERIPH_ENABLED(AHB3, value);
    } else if(bus < FuriHalBusAPB1_GRP1) {
        ret = FURI_HAL_BUS_IS_PERIPH_ENABLED(AHB2, value, 2);
    } else if(bus < FuriHalBusAPB1_GRP2) {
        ret = FURI_HAL_BUS_IS_PERIPH_ENABLED(APB1, value, 1);
    } else if(bus < FuriHalBusAPB2_GRP1) {
        ret = FURI_HAL_BUS_IS_PERIPH_ENABLED(APB1, value, 2);
    } else if(bus < FuriHalBusAPB3_GRP1) {
        ret = FURI_HAL_BUS_IS_PERIPH_ENABLED(APB2, value);
    } else {
        ret = FURI_HAL_BUS_IS_PERIPH_ENABLED(APB3, value);
    }
    FURI_CRITICAL_EXIT();

    return ret;
}
