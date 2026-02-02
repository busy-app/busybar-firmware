#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stm32u5xx_safe.h>
#include "stdbool.h"

typedef enum {
    /* AHB1 GRP1 */
    FuriHalBusAHB1_GRP1,
    FuriHalBusGPDMA1,
    FuriHalBusCORDIC,
    FuriHalBusFMAC,
    FuriHalBusMDF1,
    FuriHalBusFLASH,
    FuriHalBusCRC,
#if defined(JPEG)
    FuriHalBusJPEG,
#endif
    FuriHalBusTSC,
    FuriHalBusRAMCFG,
#if defined(DMA2D)
    FuriHalBusDMA2D,
#endif
#if defined(GFXMMU)
    FuriHalBusGFXMMU,
#endif
#if defined(GPU2D)
    FuriHalBusGPU2D,
#endif
#if defined(DCACHE2)
    FuriHalBusDCACHE2,
#endif
    FuriHalBusGTZC1,
    FuriHalBusBKPSRAM,
    FuriHalBusICACHE1,
    FuriHalBusDCACHE1,
    FuriHalBusSRAM1,

    /* AHB2 GRP1 */
    FuriHalBusAHB2_GRP1,
    FuriHalBusGPIOA,
    FuriHalBusGPIOB,
    FuriHalBusGPIOC,
    FuriHalBusGPIOD,
    FuriHalBusGPIOE,
#if defined(GPIOF)
    FuriHalBusGPIOF,
#endif
    FuriHalBusGPIOG,
    FuriHalBusGPIOH,
#if defined(GPIOI)
    FuriHalBusGPIOI,
#endif
#if defined(GPIOJ)
    FuriHalBusGPIOJ,
#endif
    FuriHalBusADC12,
    FuriHalBusDCMI_PSSI,
#if defined(USB_OTG_FS)
    FuriHalBusOTG_FS,
    FuriHalBusUSBFS,
#elif defined(USB_OTG_HS)
    FuriHalBusOTG_HS,
    FuriHalBusUSBHS,
#endif
#if defined(RCC_AHB2ENR1_USBPHYCEN)
    FuriHalBusUSBPHY,
#endif
#if defined(AES)
    FuriHalBusAES,
#endif
#if defined(HASH)
    FuriHalBusHASH,
#endif
    FuriHalBusRNG,
    FuriHalBusPKA,
#if defined(SAES)
    FuriHalBusSAES,
#endif
#if defined(OCTOSPIM)
    FuriHalBusOCTOSPIM,
#endif
    FuriHalBusOTFDEC1,
#if defined(OTFDEC2)
    FuriHalBusOTFDEC2,
#endif
    FuriHalBusSDMMC1,
#if defined(SDMMC2)
    FuriHalBusSDMMC2,
#endif
    FuriHalBusSRAM2,
#if defined(SRAM3_BASE)
    FuriHalBusSRAM3,
#endif

    /* AHB3 GRP1 */
    FuriHalBusAHB3_GRP1,
    FuriHalBusLPGPIO1,
    FuriHalBusPWR,
    FuriHalBusADC4,
    FuriHalBusDAC1,
    FuriHalBusLPDMA1,
    FuriHalBusADF1,
    FuriHalBusGTZC2,
    FuriHalBusSRAM4,

    /* AHB2 GRP2 */
    FuriHalBusAHB2_GRP2,
#if defined(FMC_BASE)
    FuriHalBusFSMC,
#endif
    FuriHalBusOCTOSPI1,
#if defined(OCTOSPI2)
    FuriHalBusOCTOSPI2,
#endif
#if defined(HSPI1)
    FuriHalBusHSPI1,
#endif
#if defined(SRAM6_BASE)
    FuriHalBusSRAM6,
#endif
#if defined(SRAM5_BASE)
    FuriHalBusSRAM5,
#endif

    /* APB1 GRP1 */
    FuriHalBusAPB1_GRP1,
    FuriHalBusTIM2,
    FuriHalBusTIM3,
    FuriHalBusTIM4,
    FuriHalBusTIM5,
    FuriHalBusTIM6,
    FuriHalBusTIM7,
    FuriHalBusWWDG,
    FuriHalBusSPI2,
#if defined(USART2)
    FuriHalBusUSART2,
#endif
    FuriHalBusUSART3,
    FuriHalBusUART4,
    FuriHalBusUART5,
    FuriHalBusI2C1,
    FuriHalBusI2C2,
    FuriHalBusCRS,
#if defined(USART6)
    FuriHalBusUSART6,
#endif

    /* APB1 GRP2 */
    FuriHalBusAPB1_GRP2,
    FuriHalBusI2C4,
    FuriHalBusLPTIM2,
    FuriHalBusFDCAN1,
#if defined(UCPD1)
    FuriHalBusUCPD1,
#endif
#if defined(I2C5)
    FuriHalBusI2C5,
#endif
#if defined(I2C6)
    FuriHalBusI2C6,
#endif

    /* APB2 GRP1 */
    FuriHalBusAPB2_GRP1,
    FuriHalBusTIM1,
    FuriHalBusSPI1,
    FuriHalBusTIM8,
    FuriHalBusUSART1,
    FuriHalBusTIM15,
    FuriHalBusTIM16,
    FuriHalBusTIM17,
    FuriHalBusSAI1,
#if defined(SAI2)
    FuriHalBusSAI2,
#endif
#if defined(USB_DRD_FS)
    FuriHalBusUSB_FS,
#endif
#if defined(GFXTIM)
    FuriHalBusGFXTIM,
#endif
#if defined(LTDC)
    FuriHalBusLTDC,
#endif
#if defined(DSI)
    FuriHalBusDSI,
#endif

    /* APB3 GRP1 */
    FuriHalBusAPB3_GRP1,
    FuriHalBusSYSCFG,
    FuriHalBusSPI3,
    FuriHalBusLPUART1,
    FuriHalBusI2C3,
    FuriHalBusLPTIM1,
    FuriHalBusLPTIM3,
    FuriHalBusLPTIM4,
    FuriHalBusOPAMP,
    FuriHalBusCOMP,
    FuriHalBusVREF,
    FuriHalBusRTCAPB,

    // TODO: special case for SRDAMR GRP1
    /* SRDAMR GRP1 */
    // FuriHalBusSRDAMR_GRP1,
    // FuriHalBusSRD_SPI3,
    // FuriHalBusSRD_LPUART1,
    // FuriHalBusSRD_I2C3,
    // FuriHalBusSRD_LPTIM1,
    // FuriHalBusSRD_LPTIM3,
    // FuriHalBusSRD_LPTIM4,
    // FuriHalBusSRD_OPAMP,
    // FuriHalBusSRD_COMP,
    // FuriHalBusSRD_VREF,
    // FuriHalBusSRD_RTCAPB,
    // FuriHalBusSRD_ADC4,
    // FuriHalBusSRD_LPGPIO1,
    // FuriHalBusSRD_DAC1,
    // FuriHalBusSRD_LPDMA1,
    // FuriHalBusSRD_ADF1,
    // FuriHalBusSRD_SRAM4,

    FuriHalBusMAX,
} FuriHalBus;

/** Early initialization */
void furi_hal_bus_init_early(void);

/** Early de-initialization */
void furi_hal_bus_deinit_early(void);

/**
 * Enable a peripheral by turning the clocking on and deasserting the reset.
 * @param [in] bus Peripheral to be enabled.
 * @warning Peripheral must be in disabled state in order to be enabled.
 */
void furi_hal_bus_enable(FuriHalBus bus);

/**
 * Reset a peripheral by sequentially asserting and deasserting the reset.
 * @param [in] bus Peripheral to be reset.
 * @warning Peripheral must be in enabled state in order to be reset.
 */
void furi_hal_bus_reset(FuriHalBus bus);

/**
 * Disable a peripheral by turning the clocking off and asserting the reset.
 * @param [in] bus Peripheral to be disabled.
 * @warning Peripheral must be in enabled state in order to be disabled.
 */
void furi_hal_bus_disable(FuriHalBus bus);

/** Check if peripheral is enabled
 *
 * @param[in]  bus   The peripheral to check
 *
 * @return     true if enabled or always enabled, false otherwise
 */
bool furi_hal_bus_is_enabled(FuriHalBus bus);

#ifdef __cplusplus
}
#endif
