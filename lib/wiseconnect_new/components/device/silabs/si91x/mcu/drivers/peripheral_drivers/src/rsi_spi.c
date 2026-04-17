/******************************************************************************
* @file  rsi_spi.c
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* SPDX-License-Identifier: Zlib
*
* The licensor of this software is Silicon Laboratories Inc.
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
******************************************************************************/

/**
 * Includes
 */

#include "rsi_ccp_user_config.h"
#include "SPI.h"
#include "rsi_rom_egpio.h"
#include "rsi_rom_ulpss_clk.h"
#include "rsi_power_save.h"
#include "rsi_spi.h"
#ifdef SL_SI91X_SSI_DMA
#include "sl_si91x_dma.h"
#if (defined(SSI_ULP_MASTER_RX_DMA_Instance) && (SSI_ULP_MASTER_RX_DMA_Instance == 1))
#define DMA_INSTANCE 1
#else
#define DMA_INSTANCE 0
#endif
#else
#include "rsi_rom_udma_wrapper.h"
#endif

#include "sl_si91x_driver_gpio.h"
#include "sl_gpio_board.h"

#define SSI_4_BIT_DATA_WIDTH    4    // 4-bit data frame width
#define SSI_8_BIT_DATA_WIDTH    8    // 8-bit data frame width
#define SSI_16_BIT_DATA_WIDTH   16   // 16-bit data frame width
#define SSI_32_BIT_DATA_WIDTH   32   // 32-bit data frame width
#define BYTES_FOR_8_DATA_WIDTH  1    // Number of bytes for 8 bit data frame
#define BYTES_FOR_16_DATA_WIDTH 2    // Number of bytes for 16 bit data frame
#define BYTES_FOR_32_DATA_WIDTH 4    // Number of bytes for 32 bit data frame
#define DUMMY_DATA              0xA5 // Dummy data to be written in receive only mode by master
#define SER_DISABLED            0x00 // Disabled all slave.
#define MODE_FAULT_STATUS_SET   1U   // The status of mode fault setting.
#define MODE_FAULT_STATUS_CLR   0U   // The status of mode fault clearing.
#define SSI_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MIN \
  46 // Minimum pin number for specific range of HP pins to act as ULP pins
#define SSI_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MAX \
  49 // Maximum pin number for specific range of HP pins to act as ULP pins
/*When use the ULP Master instance with DMA enabled, it is advisable ulp memory used for low power modes */
#define ULP_SSI_DUMMY_DATA \
  (ULP_SRAM_START_ADDR + (1 * 800)) // Dummy data to be written in receive only mode by master while in PS2 state
#if (defined(SSI_ULP_MASTER_RX_DMA_Instance) && (SSI_ULP_MASTER_RX_DMA_Instance == 1))
#define ULP_SSI_MASTER_BANK_OFFSET 0x800 // ULP Memory bank offset value.
#define ULP_SSI_MASTER_BUF_MEMORY  (ULP_SRAM_START_ADDR + (1 * ULP_SSI_MASTER_BANK_OFFSET))
#endif
static uint8_t data_width_in_bytes = 0; // variable to store data width in bytes for current transfer
static uint8_t ssi_slave_number    = 0; // variable to store current slave number
#if !(defined(SSI_ULP_MASTER_RX_DMA_Instance) && (SSI_ULP_MASTER_RX_DMA_Instance == 1))
#ifndef SI917_MEMLCD
static uint8_t cs0_gpio_mode_pin = 0; // ULP_SSI primary CS0 pin number store in this variable.
static uint8_t cs1_gpio_mode_pin = 0; // ULP_SSI primary CS1 pin number store in this variable.
static uint8_t cs2_gpio_mode_pin = 0; // ULP_SSI primary CS2 pin number store in this variable.
static uint8_t cs0_gpio_port     = 0; // ULP_SSI primary CS0 port number store in this variable.
static uint8_t cs1_gpio_port     = 0; // ULP_SSI primary CS1 port number store in this variable.
static uint8_t cs2_gpio_port     = 0; // ULP_SSI primary CS2 port number store in this variable.
static sl_status_t SPI_Configure_CS_Pin(const SPI_PIN *cs_pin, uint8_t *cs_gpio_mode_pin, uint8_t *cs_gpio_port);
static sl_status_t SPI_Set_CS_Pin_State(uint8_t cs_gpio_mode_pin, uint8_t cs_gpio_port, uint8_t state);
#endif
#endif
static void SPI_Convert_Data_Width_To_Bytes(uint16_t data_width);
static sl_status_t SPI_Set_Mode_Fault_Event(const SPI_RESOURCES *spi);
#ifdef SSI_DUAL_QUAD_COMPONENT
static int32_t SPI_Configure_Dma_Transfer(const SPI_RESOURCES *spi,
                                          uint32_t num,
                                          uint16_t data_width,
                                          uint16_t xfer_type);
#endif
STATIC INLINE void spi_configure_gpio_pin(SPI_PIN *spi_pin);
#ifndef SSI_ROMDRIVER_PRESENT
#include "rsi_rom_clks.h"
#include "rsi_spi.h"

STATIC INLINE void spi_configure_gpio_pin(SPI_PIN *spi_pin)
{
  // If the pin is a ULP_GPIO, configure it for direct ULP_GPIO mode.
  if (spi_pin->pin > 64) {
    RSI_EGPIO_UlpPadReceiverEnable(spi_pin->pin - 64);
    RSI_EGPIO_SetPinMux(EGPIO1, 0, (spi_pin->pin - 64), EGPIO_PIN_MUX_MODE6);
  }
  // Set up the standard GPIO pin multiplexing for the signal
  RSI_EGPIO_SetPinMux(EGPIO, spi_pin->port, spi_pin->pin, spi_pin->mode);
  if (spi_pin->pad_sel != 0) {
    RSI_EGPIO_PadSelectionEnable(spi_pin->pad_sel);
  }
  // Activate the pad receiver for the signal pin
  RSI_EGPIO_PadReceiverEnable(spi_pin->pin);
  if (spi_pin->pin >= 25 && spi_pin->pin <= 30) {
    RSI_EGPIO_HostPadsGpioModeEnable(spi_pin->pin);
  }
}

/** @addtogroup SOC15
 * @{
 */
/**
 * @fn           int32_t SPI_Initialize(ARM_SPI_SignalEvent_t cb_event, const SPI_RESOURCES *spi, UDMA_RESOURCES *udma, RSI_UDMA_DESC_T *UDMA_Table, RSI_UDMA_HANDLE_T *udmaHandle, uint32_t *mem)
 * @brief        Initialize SPI Interface.
 * @param[in]    cb_event: Pointer to the ARM_SPI_SignalEvent
 * @param[in]    spi: Pointer to the spi resources
 * @param[in]    udma: Pointer to the udma resources
 * @param[in]    UDMA_Table: This pointer points to UDMA table 
 * @param[in]    udmaHandle: This pointer points to UDMA Handle
 * @param[in]    mem: This pointer stores UDMA starting memory location
 * @return 		   excecution status
 */

int32_t SPI_Initialize(ARM_SPI_SignalEvent_t cb_event,
                       const SPI_RESOURCES *spi,
                       UDMA_RESOURCES *udma,
                       RSI_UDMA_DESC_T *UDMA_Table,
                       RSI_UDMA_HANDLE_T *udmaHandle,
                       uint32_t *mem)
{
#ifdef SL_SI91X_SSI_DMA
  //Added to suppress unused variable warning
  (void)udma;
  (void)udmaHandle;
  (void)UDMA_Table;
  (void)mem;
#endif
  if (spi->info->state & SPI_INITIALIZED) {
    return ARM_DRIVER_OK;
  }

  // Initialize SPI Run-Time Resources
  spi->info->cb_event          = cb_event;
  spi->info->status.busy       = 0U;
  spi->info->status.data_lost  = 0U;
  spi->info->status.mode_fault = 0U;

  // Clear transfer information
  memset(spi->xfer, 0, sizeof(SPI_TRANSFER_INFO));
  if ((spi->instance_mode == SPI_MASTER_MODE) || (spi->instance_mode == SPI_SLAVE_MODE)) {
    //CLK
    if (spi->io.sck != NULL) {
      spi_configure_gpio_pin(spi->io.sck);
    }

    //CS0
    if (spi->io.cs0 != NULL) {
      spi_configure_gpio_pin(spi->io.cs0);
    }
    //CS1
    if (spi->io.cs1 != NULL) {
      spi_configure_gpio_pin(spi->io.cs1);
    }
    //CS2
    if (spi->io.cs2 != NULL) {
      spi_configure_gpio_pin(spi->io.cs2);
    }
    //CS3
    if (spi->io.cs3 != NULL) {
      spi_configure_gpio_pin(spi->io.cs3);
    }
    //MOSI/DATA0
    if (spi->io.mosi != NULL) {
      spi_configure_gpio_pin(spi->io.mosi);
    }
    //MISO/DATA1
    if (spi->io.miso != NULL) {
      spi_configure_gpio_pin(spi->io.miso);
    }
#if (defined(SPI_QUAD_MODE) && (SPI_QUAD_MODE == 1))
    //DATA2 - Configure pin for the 3rd data line (IO2) in quad SPI mode
    if (spi->io.data2 != NULL) {
      spi_configure_gpio_pin(spi->io.data2);
    }

    //DATA3 - Configure pin for the 4th data line (IO3) in quad SPI mode
    if (spi->io.data3 != NULL) {
      spi_configure_gpio_pin(spi->io.data3);
    }
#endif
  } else if (spi->instance_mode == SPI_ULP_MASTER_MODE) {
    //clock
    //if the pin is ULP_GPIO then set the pin mode for direct ULP_GPIO.
    if (spi->io.sck->pin >= GPIO_MAX_PIN) {
      RSI_EGPIO_UlpPadReceiverEnable(spi->io.sck->pin - GPIO_MAX_PIN);
      RSI_EGPIO_SetPinMux(EGPIO1, spi->io.sck->port, (spi->io.sck->pin - GPIO_MAX_PIN), spi->io.sck->mode);
    } else { // if the pin is SoC GPIO then set the HP GPIO mode to ULP_PERI_ON_SOC_PIN_MODE.
      RSI_EGPIO_SetPinMux(EGPIO, spi->io.sck->port, spi->io.sck->pin, EGPIO_PIN_MUX_MODE9);
      if (spi->io.sck->pad_sel != 0) {
        RSI_EGPIO_PadSelectionEnable(spi->io.sck->pad_sel);
      }
      RSI_EGPIO_SetPinMux(EGPIO1, spi->io.sck->port, spi->io.sck->pin, 0);
      if (spi->io.sck->pin >= SSI_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MIN
          && spi->io.sck->pin <= SSI_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MAX) {
        RSI_EGPIO_UlpSocGpioMode(ULPCLK, (spi->io.sck->pin - 38), spi->io.sck->mode);
      } else {
        RSI_EGPIO_UlpSocGpioMode(ULPCLK, (spi->io.sck->pin - 6), spi->io.sck->mode);
      }
    }

#if (defined(SSI_ULP_MASTER_RX_DMA_Instance) && (SSI_ULP_MASTER_RX_DMA_Instance == 1))
    //CS Selection
    if (spi->io.cs0 != NULL) {
      //if the pin is ULP_GPIO then set the pin mode for direct ULP_GPIO.
      if (spi->io.cs0->pin > GPIO_MAX_PIN) {
        RSI_EGPIO_UlpPadReceiverEnable(spi->io.cs0->pin - GPIO_MAX_PIN);
        RSI_EGPIO_SetPinMux(EGPIO1, spi->io.cs0->port, (spi->io.cs0->pin - GPIO_MAX_PIN), spi->io.cs0->mode);
      } else { // if the pin is SoC GPIO then set the HP GPIO mode to ULP_PERI_ON_SOC_PIN_MODE.
        RSI_EGPIO_SetPinMux(EGPIO, spi->io.cs0->port, spi->io.cs0->pin, EGPIO_PIN_MUX_MODE9);
        if (spi->io.cs0->pad_sel != 0) {
          RSI_EGPIO_PadSelectionEnable(spi->io.cs0->pad_sel);
        }
        RSI_EGPIO_SetPinMux(EGPIO1, spi->io.cs0->port, spi->io.cs0->pin, 0);
        if (spi->io.cs0->pin >= SSI_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MIN
            && spi->io.cs0->pin <= SSI_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MAX) {
          RSI_EGPIO_UlpSocGpioMode(ULPCLK, (spi->io.cs0->pin - 38), spi->io.cs0->mode);
        } else {
          RSI_EGPIO_UlpSocGpioMode(ULPCLK, (spi->io.cs0->pin - 6), spi->io.cs0->mode);
        }
      }
    }
    if (spi->io.cs1 != NULL) {
      //if the pin is ULP_GPIO then set the pin mode for direct ULP_GPIO.
      if (spi->io.cs1->pin > GPIO_MAX_PIN) {
        RSI_EGPIO_UlpPadReceiverEnable(spi->io.cs1->pin - GPIO_MAX_PIN);
        RSI_EGPIO_SetPinMux(EGPIO1, spi->io.cs1->port, (spi->io.cs1->pin - GPIO_MAX_PIN), spi->io.cs1->mode);
      } else { // if the pin is SoC GPIO then set the HP GPIO mode to ULP_PERI_ON_SOC_PIN_MODE.
        RSI_EGPIO_SetPinMux(EGPIO, spi->io.cs1->port, spi->io.cs1->pin, EGPIO_PIN_MUX_MODE9);
        if (spi->io.cs1->pad_sel != 0) {
          RSI_EGPIO_PadSelectionEnable(spi->io.cs1->pad_sel);
        }
        RSI_EGPIO_SetPinMux(EGPIO1, spi->io.cs1->port, spi->io.cs1->pin, 0);
        if (spi->io.cs1->pin >= SSI_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MIN
            && spi->io.cs1->pin <= SSI_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MAX) {
          RSI_EGPIO_UlpSocGpioMode(ULPCLK, (spi->io.cs1->pin - 38), spi->io.cs1->mode);
        } else {
          RSI_EGPIO_UlpSocGpioMode(ULPCLK, (spi->io.cs1->pin - 6), spi->io.cs1->mode);
        }
      }
    }
    if (spi->io.cs2 != NULL) {
      //if the pin is ULP_GPIO then set the pin mode for direct ULP_GPIO.
      if (spi->io.cs2->pin > GPIO_MAX_PIN) {
        RSI_EGPIO_UlpPadReceiverEnable(spi->io.cs2->pin - GPIO_MAX_PIN);
        RSI_EGPIO_SetPinMux(EGPIO1, spi->io.cs2->port, (spi->io.cs2->pin - GPIO_MAX_PIN), spi->io.cs2->mode);
      } else { // if the pin is SoC GPIO then set the HP GPIO mode to ULP_PERI_ON_SOC_PIN_MODE.
        RSI_EGPIO_SetPinMux(EGPIO, spi->io.cs2->port, spi->io.cs2->pin, EGPIO_PIN_MUX_MODE9);
        if (spi->io.cs2->pad_sel != 0) {
          RSI_EGPIO_PadSelectionEnable(spi->io.cs2->pad_sel);
        }
        RSI_EGPIO_SetPinMux(EGPIO1, spi->io.cs2->port, spi->io.cs2->pin, 0);
        if (spi->io.cs2->pin >= SSI_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MIN
            && spi->io.cs2->pin <= SSI_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MAX) {
          RSI_EGPIO_UlpSocGpioMode(ULPCLK, (spi->io.cs2->pin - 38), spi->io.cs2->mode);
        } else {
          RSI_EGPIO_UlpSocGpioMode(ULPCLK, (spi->io.cs2->pin - 6), spi->io.cs2->mode);
        }
      }
    }
#else // For ULP_SSI primary non DMA case, configure CS as normal gpio pin to control manually.
#ifndef SI917_MEMLCD
    sl_status_t status;
    // Initialize the GPIOs by clearing all interrupts initially
    status = sl_gpio_driver_init();
    if (status != SL_STATUS_OK) {
      return ARM_DRIVER_ERROR;
    }
    if (spi->io.cs0 != NULL) {
      //Configured CS0 pin
      status = SPI_Configure_CS_Pin(spi->io.cs0, &cs0_gpio_mode_pin, &cs0_gpio_port);
      if (status != SL_STATUS_OK) {
        return ARM_DRIVER_ERROR;
      }
    }
    if (spi->io.cs1 != NULL) {
      //Configured CS1 pin
      status = SPI_Configure_CS_Pin(spi->io.cs1, &cs1_gpio_mode_pin, &cs1_gpio_port);
      if (status != SL_STATUS_OK) {
        return ARM_DRIVER_ERROR;
      }
    }
    if (spi->io.cs2 != NULL) {
      //Configured CS2 pin
      status = SPI_Configure_CS_Pin(spi->io.cs2, &cs2_gpio_mode_pin, &cs2_gpio_port);
      if (status != SL_STATUS_OK) {
        return ARM_DRIVER_ERROR;
      }
    }
#endif
#endif
    //MOSI
    //if the pin is ULP_GPIO then set the pin mode for direct ULP_GPIO.
    if (spi->io.mosi->pin > GPIO_MAX_PIN) {
      RSI_EGPIO_UlpPadReceiverEnable(spi->io.mosi->pin - GPIO_MAX_PIN);
      RSI_EGPIO_SetPinMux(EGPIO1, spi->io.mosi->port, (spi->io.mosi->pin - GPIO_MAX_PIN), spi->io.mosi->mode);
    } else { // if the pin is SoC GPIO then set the HP GPIO mode to ULP_PERI_ON_SOC_PIN_MODE.
      RSI_EGPIO_SetPinMux(EGPIO, spi->io.mosi->port, spi->io.mosi->pin, EGPIO_PIN_MUX_MODE9);
      if (spi->io.mosi->pad_sel != 0) {
        RSI_EGPIO_PadSelectionEnable(spi->io.mosi->pad_sel);
      }
      RSI_EGPIO_SetPinMux(EGPIO1, spi->io.mosi->port, spi->io.mosi->pin, 0);
      if (spi->io.mosi->pin >= SSI_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MIN
          && spi->io.mosi->pin <= SSI_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MAX) {
        RSI_EGPIO_UlpSocGpioMode(ULPCLK, (spi->io.mosi->pin - 38), spi->io.mosi->mode);
      } else {
        RSI_EGPIO_UlpSocGpioMode(ULPCLK, (spi->io.mosi->pin - 6), spi->io.mosi->mode);
      }
    }

    //MISO
    //if the pin is ULP_GPIO then set the pin mode for direct ULP_GPIO.
    if (spi->io.miso->pin > GPIO_MAX_PIN) {
      RSI_EGPIO_UlpPadReceiverEnable(spi->io.miso->pin - GPIO_MAX_PIN);
      RSI_EGPIO_SetPinMux(EGPIO1, spi->io.miso->port, (spi->io.miso->pin - GPIO_MAX_PIN), spi->io.miso->mode);
    } else { // if the pin is SoC GPIO then set the HP GPIO mode to ULP_PERI_ON_SOC_PIN_MODE.
      RSI_EGPIO_SetPinMux(EGPIO, spi->io.miso->port, spi->io.miso->pin, EGPIO_PIN_MUX_MODE9);
      if (spi->io.miso->pad_sel != 0) {
        RSI_EGPIO_PadSelectionEnable(spi->io.miso->pad_sel);
      }
      RSI_EGPIO_PadReceiverEnable(spi->io.miso->pin);
      RSI_EGPIO_SetPinMux(EGPIO1, spi->io.miso->port, spi->io.miso->pin, 0);
      if (spi->io.miso->pin >= SSI_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MIN
          && spi->io.miso->pin <= SSI_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MAX) {
        RSI_EGPIO_UlpSocGpioMode(ULPCLK, (spi->io.miso->pin - 38), spi->io.miso->mode);
      } else {
        RSI_EGPIO_UlpSocGpioMode(ULPCLK, (spi->io.miso->pin - 6), spi->io.miso->mode);
      }
    }
  }
  if ((spi->rx_dma != NULL) || (spi->tx_dma != NULL)) {
// Enable DMA instance
// DMA0 used for SSI_MASTER and SSI_SLAVE
#ifdef SL_SI91X_SSI_DMA
    sl_dma_init_t dma_init;
    dma_init.dma_number = DMA_INSTANCE;
    if (sl_si91x_dma_init(&dma_init)) {
      return ARM_DRIVER_ERROR;
    }
    *udmaHandle = sl_si91x_get_dma_handle(dma_init.dma_number);
#else
    *udmaHandle = UDMAx_Initialize(udma, UDMA_Table, udmaHandle, mem);
#endif
  }
  spi->info->state = SPI_INITIALIZED;

  return ARM_DRIVER_OK;
}

/**
 * @fn           int32_t SPI_Uninitialize(const SPI_RESOURCES *spi, UDMA_RESOURCES *udma)
 * @brief        DeInitialize SPI Interface
 * @param[in]    spi       : Pointer to the spi resources
 * @param[in]    udma      : Pointer to the udma resources
 * @return 		   excecution status 
 */
int32_t SPI_Uninitialize(const SPI_RESOURCES *spi, UDMA_RESOURCES *udma)
{
#ifdef SL_SI91X_SSI_DMA
  //Added to suppress unused variable warning
  (void)udma;
#endif
  spi->reg->SSIENR = SSI_DISABLE;
  // Clear SPI state
  spi->info->state = 0U;

  if ((spi->rx_dma != NULL) || (spi->tx_dma != NULL)) {
// Diasable DMA instance
#ifdef SL_SI91X_SSI_DMA
    if (sl_si91x_dma_unregister_callbacks(DMA_INSTANCE,
                                          (spi->tx_dma->channel + 1),
                                          SL_DMA_TRANSFER_DONE_CB | SL_DMA_ERROR_CB)) {
      return ARM_DRIVER_ERROR;
    }
    if (sl_si91x_dma_unregister_callbacks(DMA_INSTANCE,
                                          (spi->tx_dma->channel + 1),
                                          SL_DMA_TRANSFER_DONE_CB | SL_DMA_ERROR_CB)) {
      return ARM_DRIVER_ERROR;
    }
#else
    UDMAx_Uninitialize(udma);
#endif
  }

  return ARM_DRIVER_OK;
}

/**
 * @fn           int32_t SPI_PowerControl(ARM_POWER_STATE state, const SPI_RESOURCES *spi)
 * @brief        Control SPI Interface Power.
 * @param[in]    state     : Power state
 * @param[in]    spi       : Pointer to the spi resources
 * @return 		   excecution status 
 */
int32_t SPI_PowerControl(ARM_POWER_STATE state, const SPI_RESOURCES *spi)
{
  switch (state) {
    case ARM_POWER_OFF:

      NVIC_DisableIRQ(spi->irq_num); // Disable SPI IRQ in NVIC
      spi->reg->SSIENR = SSI_DISABLE;

      // Reset SPI Run-Time Resources
      spi->info->status.busy       = 0U;
      spi->info->status.data_lost  = 0U;
      spi->info->status.mode_fault = 0U;

      // Clear pending SPI interrupts in NVIC
      NVIC_ClearPendingIRQ(spi->irq_num);

      // Clear transfer information
      memset(spi->xfer, 0, sizeof(SPI_TRANSFER_INFO));

      spi->info->state &= (uint8_t)(~SPI_POWERED); // SPI is not powered

      break;

    case ARM_POWER_FULL:

      if ((spi->info->state & SPI_INITIALIZED) == 0U) {
        return ARM_DRIVER_ERROR;
      }
      if ((spi->info->state & SPI_POWERED) != 0U) {
        return ARM_DRIVER_OK;
      }

      // Connect SPI BASE clock
      /* Static Clock gating is Enabled for M4 SOC Other Clock*/
      if (spi->instance_mode == SPI_MASTER_MODE) {
        RSI_CLK_SsiMstClkConfig(M4CLK, ENABLE_STATIC_CLK, spi->clock.spi_clk_src, spi->clock.divfact);
      } else if (spi->instance_mode == SPI_SLAVE_MODE) {
        RSI_CLK_PeripheralClkEnable(M4CLK, SSISLAVE_CLK, ENABLE_STATIC_CLK);
      } else if (spi->instance_mode == SPI_ULP_MASTER_MODE) {
        RSI_ULPSS_UlpSsiClkConfig(ULPCLK, ENABLE_STATIC_CLK, spi->clock.ulp_spi_clk_src, (uint16_t)spi->clock.divfact);
      }

      // Reset SPI Run-Time Resources
      spi->info->status.busy       = 0U;
      spi->info->status.data_lost  = 0U;
      spi->info->status.mode_fault = 0U;
      spi->info->state |= SPI_POWERED; // SPI is powered

      NVIC_ClearPendingIRQ(spi->irq_num);
      NVIC_EnableIRQ(spi->irq_num); // Enable SPI IRQ in NVIC
      break;

    case ARM_POWER_LOW:
    default:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  return ARM_DRIVER_OK;
}

/**
 * @fn          int32_t SPI_Transfer(const void *data_out,
                     void *data_in,
                     uint32_t num,
                     const SPI_RESOURCES *spi,
                     UDMA_RESOURCES *udma,
                     UDMA_Channel_Info *chnl_info,
                     RSI_UDMA_HANDLE_T udmaHandle)
 * @brief        Start sending/receiving data to/from SPI transmitter/receiver.
 * @param[in]    data_out    : Pointer to buffer with data to send to SPI transmitter
 * @param[in]    data_in     : Pointer to buffer for data to receive from SPI receiver
 * @param[in]    num         : Number of data items to transfer
 * @param[in]    spi         : Pointer to the SPI resources
 * @param[in]	   udma        : Pointer to the udma resources
 * @param[in]	   chnl_info   : Pointer for channel info
 * @param[in]	   udmaHandle  : Pointer to the UDMA Handle
 * @return 		   excecution status
 */
int32_t SPI_Transfer(const void *data_out,
                     void *data_in,
                     uint32_t num,
                     const SPI_RESOURCES *spi,
                     UDMA_RESOURCES *udma,
                     UDMA_Channel_Info *chnl_info,
                     RSI_UDMA_HANDLE_T udmaHandle)
{
#ifdef SL_SI91X_SSI_DMA
  //Added to suppress unused variable warning
  (void)udma;
  (void)udmaHandle;
  (void)chnl_info;
  sl_status_t status;
#else
  volatile int32_t stat;
#endif
  RSI_UDMA_CHA_CONFIG_DATA_T control = { 0 };
  uint16_t data_width;

  if ((data_out == NULL) || (data_in == NULL) || (num == 0U)) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }
  if (!(spi->info->state & SPI_CONFIGURED)) {
    return ARM_DRIVER_ERROR;
  }
  if (spi->info->status.busy) {
    return ARM_DRIVER_ERROR_BUSY;
  }
  // Set the mode fault event if slave select is deactivated in master/ulp_master instance mode.
  if (SPI_Set_Mode_Fault_Event(spi)) {
    return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  spi->info->status.busy       = 1U;
  spi->info->status.data_lost  = 0U;
  spi->info->status.mode_fault = 0U;

  spi->xfer->rx_buf = (uint8_t *)data_in;
  spi->xfer->tx_buf = (uint8_t *)data_out;

  spi->xfer->rx_cnt = 0U;
  spi->xfer->tx_cnt = 0U;

  spi->reg->CTRLR0_b.TMOD = TRANSMIT_AND_RECEIVE;

  spi->reg->SSIENR = SSI_ENABLE;

  if ((spi->instance_mode == SPI_MASTER_MODE) || (spi->instance_mode == SPI_ULP_MASTER_MODE)) {
    data_width = spi->reg->CTRLR0_b.DFS_32;
  } else {
    data_width = spi->reg->CTRLR0_b.DFS;
  }

  if ((spi->rx_dma != NULL) || (spi->tx_dma != NULL)) {
    if (spi->rx_dma != NULL) {
      control.transferType = UDMA_MODE_BASIC;
      control.nextBurst    = 0;
      if (num < 1024) {
        control.totalNumOfDMATrans = (unsigned int)((num - 1) & 0x03FF);
      } else {
        control.totalNumOfDMATrans = 0x3FF;
      }
      control.rPower      = ARBSIZE_1;
      control.srcProtCtrl = 0x0;
      control.dstProtCtrl = 0x0;
      control.srcInc      = SRC_INC_NONE;
      if (data_width <= (8U - 1U)) {
        // 8-bit data frame
        control.srcSize = SRC_SIZE_8;
        control.dstSize = DST_SIZE_8;
        control.dstInc  = DST_INC_8;
      } else if (data_width <= (16U - 1U)) {
        // 16-bit data frame
        control.srcSize = SRC_SIZE_16;
        control.dstSize = DST_SIZE_16;
        control.dstInc  = DST_INC_16;
      } else {
        // 32-bit data frame
        control.srcSize = SRC_SIZE_32;
        control.dstSize = DST_SIZE_32;
        control.dstInc  = DST_INC_32;
      }
      spi->reg->DMACR_b.RDMAE    = 1;
      spi->reg->DMARDLR_b.DMARDL = 0;
#if SL_SI91X_SSI_DMA
      sl_dma_xfer_t dma_transfer_rx = { 0 };
      uint32_t channel              = spi->rx_dma->channel + 1;
      uint32_t channel_priority     = spi->rx_dma->chnl_cfg.channelPrioHigh;
      sl_dma_callback_t spi_rx_callback;
      //Initialize sl_dma callback structure
      spi_rx_callback.transfer_complete_cb = ssi_transfer_complete_callback;
      spi_rx_callback.error_cb             = ssi_error_callback;
      //Initialize sl_dma transfer structure
      dma_transfer_rx.src_addr       = (uint32_t *)((uint32_t) & (spi->reg->DR));
      dma_transfer_rx.dest_addr      = (uint32_t *)((uint32_t)(spi->xfer->rx_buf));
      dma_transfer_rx.src_inc        = control.srcInc;
      dma_transfer_rx.dst_inc        = control.dstInc;
      dma_transfer_rx.xfer_size      = control.dstSize;
      dma_transfer_rx.transfer_count = num;
      dma_transfer_rx.transfer_type  = SL_DMA_PERIPHERAL_TO_MEMORY;
      dma_transfer_rx.dma_mode       = control.transferType;
      dma_transfer_rx.signal         = (uint8_t)spi->rx_dma->chnl_cfg.periAck;

      //Allocate DMA channel for Tx
      status = sl_si91x_dma_allocate_channel(DMA_INSTANCE, &channel, channel_priority);
      if (status && (status != SL_STATUS_DMA_CHANNEL_ALLOCATED)) {
        return ARM_DRIVER_ERROR;
      }
      //Register transfer complete and error callback
      if (sl_si91x_dma_register_callbacks(DMA_INSTANCE, channel, &spi_rx_callback)) {
        return ARM_DRIVER_ERROR;
      }
      //Configure the channel for DMA transfer
      if (sl_si91x_dma_transfer(DMA_INSTANCE, channel, &dma_transfer_rx)) {
        return ARM_DRIVER_ERROR;
      }
      sl_si91x_dma_channel_enable(DMA_INSTANCE, spi->rx_dma->channel + 1);
#else
      // Initialize and start SPI TX DMA Stream
      stat = UDMAx_ChannelConfigure(udma,
                                    spi->rx_dma->channel,
                                    (uint32_t) & (spi->reg->DR),
                                    (uint32_t)(spi->xfer->rx_buf),
                                    num,
                                    control,
                                    &spi->rx_dma->chnl_cfg,
                                    spi->rx_dma->cb_event,
                                    chnl_info,
                                    udmaHandle);
      if (stat == -1) {
        return ARM_DRIVER_ERROR;
      }
      UDMAx_ChannelEnable(spi->rx_dma->channel, udma, udmaHandle);
#endif
    }
    if (spi->tx_dma != NULL) {
      control.transferType = UDMA_MODE_BASIC;
      control.nextBurst    = 0;
      if (num < 1024) {
        control.totalNumOfDMATrans = (unsigned int)((num - 1) & 0x03FF);
      } else {
        control.totalNumOfDMATrans = 0x3FF;
      }
      control.rPower      = ARBSIZE_1;
      control.srcProtCtrl = 0x0;
      control.dstProtCtrl = 0x0;
      control.dstInc      = DST_INC_NONE;
      if (data_width <= (8U - 1U)) {
        // 8-bit data frame
        control.srcSize = SRC_SIZE_8;
        control.srcInc  = SRC_INC_8;
        control.dstSize = DST_SIZE_8;
      } else if (data_width <= (16U - 1U)) {
        // 16-bit data frame
        control.srcSize = SRC_SIZE_16;
        control.srcInc  = SRC_INC_16;
        control.dstSize = DST_SIZE_16;
      } else {
        // 32-bit data frame
        control.srcSize = SRC_SIZE_32;
        control.srcInc  = SRC_INC_32;
        control.dstSize = DST_SIZE_32;
      }
      spi->reg->DMACR_b.TDMAE    = 1;
      spi->reg->DMATDLR_b.DMATDL = 1;
// Initialize and start SPI TX DMA Stream
#if SL_SI91X_SSI_DMA
      sl_dma_xfer_t dma_transfer_tx = { 0 };
      uint32_t channel              = spi->tx_dma->channel + 1;
      uint32_t channel_priority     = spi->tx_dma->chnl_cfg.channelPrioHigh;
      sl_dma_callback_t spi_tx_callback;
      //Initialize sl_dma callback structure
      spi_tx_callback.transfer_complete_cb = ssi_transfer_complete_callback;
      spi_tx_callback.error_cb             = ssi_error_callback;
      //Initialize sl_dma transfer structure
      dma_transfer_tx.src_addr       = (uint32_t *)((uint32_t)(spi->xfer->tx_buf));
      dma_transfer_tx.dest_addr      = (uint32_t *)((uint32_t) & (spi->reg->DR));
      dma_transfer_tx.src_inc        = control.srcInc;
      dma_transfer_tx.dst_inc        = control.dstInc;
      dma_transfer_tx.xfer_size      = control.dstSize;
      dma_transfer_tx.transfer_count = num;
      dma_transfer_tx.transfer_type  = SL_DMA_MEMORY_TO_PERIPHERAL;
      dma_transfer_tx.dma_mode       = control.transferType;
      dma_transfer_tx.signal         = (uint8_t)spi->tx_dma->chnl_cfg.periAck;

      //Allocate DMA channel for Tx
      status = sl_si91x_dma_allocate_channel(DMA_INSTANCE, &channel, channel_priority);
      if (status && (status != SL_STATUS_DMA_CHANNEL_ALLOCATED)) {
        return ARM_DRIVER_ERROR;
      }
      //Register transfer complete and error callback
      if (sl_si91x_dma_register_callbacks(DMA_INSTANCE, channel, &spi_tx_callback)) {
        return ARM_DRIVER_ERROR;
      }
      //Configure the channel for DMA transfer
      if (sl_si91x_dma_transfer(DMA_INSTANCE, channel, &dma_transfer_tx)) {
        return ARM_DRIVER_ERROR;
      }
      sl_si91x_dma_channel_enable(DMA_INSTANCE, spi->tx_dma->channel + 1);
#else
      stat = UDMAx_ChannelConfigure(udma,
                                    spi->tx_dma->channel,
                                    (uint32_t)(spi->xfer->tx_buf),
                                    (uint32_t) & (spi->reg->DR),
                                    num,
                                    control,
                                    &spi->tx_dma->chnl_cfg,
                                    spi->tx_dma->cb_event,
                                    chnl_info,
                                    udmaHandle);
      if (stat == -1) {
        return ARM_DRIVER_ERROR;
      }
      UDMAx_ChannelEnable(spi->tx_dma->channel, udma, udmaHandle);
#endif
    }
#if SL_SI91X_SSI_DMA
    sl_si91x_dma_enable(DMA_INSTANCE);
#else
    UDMAx_DMAEnable(udma, udmaHandle);
#endif
  } else {
    // Converts the data width into number of bytes to send/receive at once
    SPI_Convert_Data_Width_To_Bytes(data_width);
    // Total number of transfers according to the configured data width
    spi->xfer->num = num * data_width_in_bytes;

    // Interrupt mode
    /* spi->reg->IMR |= TXEIM | RXFIM; */
    /*enabled below bits
       Transmit FIFO Empty Interrupt Mask
       Transmit FIFO Overflow Interrupt Mask
       Receive FIFO Underflow Interrupt Mask
       Receive FIFO Overflow Interrupt Mask
       Receive FIFO Full Interrupt Mask
       */
#if !(defined(SSI_ULP_MASTER_RX_DMA_Instance) && (SSI_ULP_MASTER_RX_DMA_Instance == 1))
    // Down the CS pin for ULP primary to transfer the data.
#ifndef SI917_MEMLCD
    if (spi->instance_mode == SPI_ULP_MASTER_MODE) {
      // Set CS0 pin state
      if (spi->io.cs0 != NULL) {
        SPI_Set_CS_Pin_State(cs0_gpio_mode_pin, cs0_gpio_port, CLR);
      }
      // Set CS1 pin state
      if (spi->io.cs1 != NULL) {
        SPI_Set_CS_Pin_State(cs1_gpio_mode_pin, cs1_gpio_port, CLR);
      }
      // Set CS2 pin state
      if (spi->io.cs2 != NULL) {
        SPI_Set_CS_Pin_State(cs2_gpio_mode_pin, cs2_gpio_port, CLR);
      }
    }
#endif
#endif
    spi->reg->IMR |= (TXEIM | TXOIM | RXUIM | RXOIM | RXFIM);
  }
  return ARM_DRIVER_OK;
}

/**
 * @fn          uint32_t SPI_GetDataCount(const SPI_RESOURCES *spi)
 * @brief		     Get transferred data count.
 * @param[in]	   SPI        : Pointer to the SPI resources
 * @return 		   number of data items transferred
 */
uint32_t SPI_GetDataCount(const SPI_RESOURCES *spi)
{
  if (!(spi->info->state & SPI_CONFIGURED)) {
    return 0U;
  }

  return spi->xfer->rx_cnt;
}

/**
 * @fn          int32_t SPI_Control(uint32_t control, uint32_t arg, const SPI_RESOURCES *spi, uint32_t base_clock, uint8_t slavenumber)
 * @brief		     Control SPI Interface
 * @param[in]    control     : Operation
 * @param[in]    arg         : Argument of operation(optional)
 * @param[in]	   spi         : Pointer to the SPI resources
 * @param[in]	   base_clock  : Pointer for the base clock
 * @param[in]	   slavenumber : Pointer for slave number
 * @return 		   excecution status
 */
int32_t SPI_Control(uint32_t control, uint32_t arg, const SPI_RESOURCES *spi, uint32_t base_clock, uint8_t slavenumber)
{
  uint32_t sck_divisor;
  uint32_t data_bits;
  uint32_t val   = 0;
  uint8_t cs_pin = 0;
  volatile uint32_t icr;
  ssi_slave_number = slavenumber;
  if (!(spi->info->state & SPI_POWERED)) {
    return ARM_DRIVER_ERROR;
  }
  if ((spi->reg == SSISlave) && ((control & ARM_SPI_CONTROL_Msk) == ARM_SPI_MODE_MASTER)) {
    return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  if (((spi->reg == SSI0) || (spi->reg == SSI2)) && ((control & ARM_SPI_CONTROL_Msk) == ARM_SPI_MODE_SLAVE)) {
    return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  if ((control & ARM_SPI_CONTROL_Msk) == ARM_SPI_ABORT_TRANSFER) {
    icr = spi->reg->ICR; // Disable SPI interrupts
    (void)icr;
    memset(spi->xfer, 0, sizeof(SPI_TRANSFER_INFO));
    spi->info->status.busy = 0U;
    return ARM_DRIVER_OK;
  }
  if (spi->info->status.busy) {
    return ARM_DRIVER_ERROR_BUSY;
  }

  spi->reg->SSIENR = SSI_DISABLE;

  switch (control & ARM_SPI_CONTROL_Msk) {
    case ARM_SPI_MODE_INACTIVE:
      spi->info->mode &= ~ARM_SPI_CONTROL_Msk;
      spi->info->mode |= ARM_SPI_MODE_INACTIVE;
      spi->info->state &= (uint8_t)(~SPI_CONFIGURED);
      return ARM_DRIVER_OK;

    case ARM_SPI_MODE_MASTER:
      spi->info->mode &= ~ARM_SPI_CONTROL_Msk;
      spi->info->mode |= ARM_SPI_MODE_MASTER;
      spi->info->state |= SPI_CONFIGURED;
      goto set_speed;

    case ARM_SPI_MODE_SLAVE:
      spi->info->mode &= ~ARM_SPI_CONTROL_Msk;
      spi->info->mode |= ARM_SPI_MODE_SLAVE;
      spi->info->state |= SPI_CONFIGURED;
      break;

    case ARM_SPI_MODE_MASTER_SIMPLEX:
      return ARM_SPI_ERROR_MODE;

    case ARM_SPI_MODE_SLAVE_SIMPLEX:
      return ARM_SPI_ERROR_MODE;

    case ARM_SPI_SET_BUS_SPEED:
set_speed:
      sck_divisor             = (base_clock / arg);
      spi->reg->BAUDR_b.SCKDV = (uint8_t)sck_divisor;
      break;

    case ARM_SPI_GET_BUS_SPEED:
      return (int32_t)(base_clock / spi->reg->BAUDR_b.SCKDV);
    case ARM_SPI_SET_DEFAULT_TX_VALUE: // Set default Transmit value; arg = value
      spi->xfer->def_val = (uint16_t)(arg & 0xFFFF);
      return ARM_DRIVER_OK;

    case ARM_SPI_CONTROL_SS: // Control Slave Select; arg = 0:inactive, 1:active
      if (((spi->info->mode & ARM_SPI_CONTROL_Msk) != ARM_SPI_MODE_MASTER)) {
        return ARM_DRIVER_ERROR;
      }
      val = (spi->info->mode & ARM_SPI_SS_MASTER_MODE_Msk);
      if (arg == ARM_SPI_SS_INACTIVE) {
        if (val == ARM_SPI_SS_MASTER_HW_OUTPUT) {
          if ((spi->instance_mode == SPI_MASTER_MODE) || (spi->instance_mode == SPI_ULP_MASTER_MODE)) {
            /*cs*/
            spi->reg->SER &= ~(BIT(slavenumber));
          } else if (spi->instance_mode == SPI_SLAVE_MODE) {
            spi->reg->SER = 0 << 0;
          }
        } else {
          spi->reg->SER &= ~(BIT(slavenumber));
          switch (slavenumber) {
            case SPI_CS0:
              cs_pin = spi->io.cs0->pin;
              break;
            case SPI_CS1:
              cs_pin = spi->io.cs1->pin;
              break;
            case SPI_CS2:
              cs_pin = spi->io.cs2->pin;
              break;
            case SPI_CS3:
              cs_pin = spi->io.cs3->pin;
              break;
          }
          RSI_EGPIO_SetPin(EGPIO, 0, cs_pin, 1);
        }

      } else { //ARM_SPI_SS_ACTIVE
        if (val == ARM_SPI_SS_MASTER_HW_OUTPUT) {
          if ((spi->instance_mode == SPI_MASTER_MODE) || (spi->instance_mode == SPI_ULP_MASTER_MODE)) {
            spi->reg->SER |= (BIT(slavenumber));
          } else if (spi->instance_mode == SPI_SLAVE_MODE) {
            spi->reg->SER = 0x1 << 0;
          }
        } else {
          spi->reg->SER |= (BIT(slavenumber));
          switch (slavenumber) {
            case SPI_CS0:
              cs_pin = spi->io.cs0->pin;
              break;
            case SPI_CS1:
              cs_pin = spi->io.cs1->pin;
              break;
            case SPI_CS2:
              cs_pin = spi->io.cs2->pin;
              break;
            case SPI_CS3:
              cs_pin = spi->io.cs3->pin;
              break;
          }
          RSI_EGPIO_SetPin(EGPIO, 0, cs_pin, 0);
        }
      }
      return ARM_DRIVER_OK;
  }

  if ((spi->info->mode & ARM_SPI_CONTROL_Msk) == ARM_SPI_MODE_MASTER) {
    switch (control & ARM_SPI_SS_MASTER_MODE_Msk) {
      case ARM_SPI_SS_MASTER_UNUSED: // SPI Slave Select when Master: Not used (default)
        spi->reg->SER = 0;
        spi->info->mode &= ~ARM_SPI_SS_MASTER_MODE_Msk;
        spi->info->mode |= ARM_SPI_SS_MASTER_UNUSED;
        break;

      case ARM_SPI_SS_MASTER_HW_INPUT: // SPI Slave Select when Master: Hardware monitored Input
        spi->info->mode &= ~ARM_SPI_SS_MASTER_MODE_Msk;
        return ARM_SPI_ERROR_SS_MODE;

      case ARM_SPI_SS_MASTER_SW: // SPI Slave Select when Master: Software controlled
        spi->info->mode &= ~ARM_SPI_SS_MASTER_MODE_Msk;

        switch (slavenumber) {
          case SPI_CS0:
            cs_pin = spi->io.cs0->pin;
            if (spi->io.cs0->pad_sel != 0) {
              RSI_EGPIO_PadSelectionEnable(spi->io.cs0->pad_sel);
            }
            break;
          case SPI_CS1:
            cs_pin = spi->io.cs1->pin;
            if (spi->io.cs1->pad_sel != 0) {
              RSI_EGPIO_PadSelectionEnable(spi->io.cs1->pad_sel);
            }
            break;
          case SPI_CS2:
            cs_pin = spi->io.cs2->pin;
            if (spi->io.cs2->pad_sel != 0) {
              RSI_EGPIO_PadSelectionEnable(spi->io.cs2->pad_sel);
            }
            break;
          case SPI_CS3:
            cs_pin = spi->io.cs3->pin;
            if (spi->io.cs3->pad_sel != 0) {
              RSI_EGPIO_PadSelectionEnable(spi->io.cs3->pad_sel);
            }
            break;
        }
        if (cs_pin != (int)NULL) {
          if (cs_pin > 64) {
            RSI_EGPIO_SetPinMux(EGPIO1, 0, (cs_pin - 64), EGPIO_PIN_MUX_MODE6);
          }
          RSI_EGPIO_SetPinMux(EGPIO, 0, cs_pin, 0);

          RSI_EGPIO_SetDir(EGPIO, 0, cs_pin, 0);
          RSI_EGPIO_SetPin(EGPIO, 0, cs_pin, 1);

          spi->info->mode |= ARM_SPI_SS_MASTER_SW;
        } else {
          return ARM_SPI_ERROR_SS_MODE;
        }
        break;

      case ARM_SPI_SS_MASTER_HW_OUTPUT: // SPI Slave Select when Master: Hardware controlled Output
        spi->info->mode &= ~ARM_SPI_SS_MASTER_MODE_Msk;
        if ((spi->io.cs0->pin != (int)NULL) || (spi->io.cs1->pin != (int)NULL) || (spi->io.cs2->pin != (int)NULL)
            || (spi->io.cs3->pin != (int)NULL)) {
          spi->info->mode |= ARM_SPI_SS_MASTER_HW_OUTPUT;
        } else {
          return ARM_SPI_ERROR_SS_MODE;
        }
        break;
      default:
        break;
    }
  }
  if ((spi->info->mode & ARM_SPI_CONTROL_Msk) == ARM_SPI_MODE_SLAVE) {
    switch (control & ARM_SPI_SS_SLAVE_MODE_Msk) {
      case ARM_SPI_SS_SLAVE_HW: // SPI Slave Select when Slave: Hardware monitored (default)
        spi->info->mode &= ~ARM_SPI_SS_SLAVE_MODE_Msk;
        if ((spi->io.cs0->pin != (int)NULL) || (spi->io.cs1->pin != (int)NULL) || (spi->io.cs2->pin != (int)NULL)
            || (spi->io.cs3->pin != (int)NULL)) {
          spi->info->mode |= ARM_SPI_SS_SLAVE_HW;
        } else {
          return ARM_SPI_ERROR_SS_MODE;
        }
        break;

      case ARM_SPI_SS_SLAVE_SW: // SPI Slave Select when Slave: Software controlled
        spi->info->mode &= ~ARM_SPI_SS_SLAVE_MODE_Msk;
        return ARM_SPI_ERROR_SS_MODE;
      default:
        break;
    }
  }

  // Frame format:
  switch (control & ARM_SPI_FRAME_FORMAT_Msk) {
    case ARM_SPI_CPOL0_CPHA0:
      spi->reg->CTRLR0_b.SCPOL = 0;
      spi->reg->CTRLR0_b.SCPH  = 0;
      break;

    case ARM_SPI_CPOL0_CPHA1:
      spi->reg->CTRLR0_b.SCPOL = 0;
      spi->reg->CTRLR0_b.SCPH  = 1;
      break;

    case ARM_SPI_CPOL1_CPHA0:
      spi->reg->CTRLR0_b.SCPOL = 1;
      spi->reg->CTRLR0_b.SCPH  = 0;
      break;

    case ARM_SPI_CPOL1_CPHA1:
      spi->reg->CTRLR0_b.SCPOL = 1;
      spi->reg->CTRLR0_b.SCPH  = 1;
      break;

    case ARM_SPI_TI_SSI:
      spi->reg->CTRLR0_b.FRF = TEXAS_INSTRUMENTS_SSP;
      break;

    case ARM_SPI_MICROWIRE:
      spi->reg->CTRLR0_b.FRF = NATIONAL_SEMICONDUCTORS_MICROWIRE;
      break;

    default:
      return ARM_SPI_ERROR_FRAME_FORMAT;
  }

  if ((spi->info->mode & ARM_SPI_CONTROL_Msk) == ARM_SPI_MODE_MASTER) {
    switch (control & SPI_TRANSFER_MODE_Msk) {
      case SPI_TRANSFER_MODE_STANDARD:
        spi->reg->CTRLR0_b.SPI_FRF = STANDARD_SPI_FORMAT;
        break;
      case SPI_TRANSFER_MODE_DUAL:
        spi->reg->CTRLR0_b.SPI_FRF = DUAL_SPI_FORMAT;
        break;
      case SPI_TRANSFER_MODE_QUAD:
        spi->reg->CTRLR0_b.SPI_FRF = QUAD_SPI_FORMAT;
        break;
    }
  } else {
    spi->reg->CTRLR0_b.SPI_FRF = STANDARD_SPI_FORMAT;
  }

  // Configure Number of Data Bits
  data_bits = ((control & ARM_SPI_DATA_BITS_Msk) >> ARM_SPI_DATA_BITS_Pos);
  if ((data_bits < SSI_4_BIT_DATA_WIDTH) || (data_bits > SSI_32_BIT_DATA_WIDTH)) {
    return ARM_SPI_ERROR_DATA_BITS;
  }
  if ((spi->instance_mode == SPI_MASTER_MODE) || (spi->instance_mode == SPI_ULP_MASTER_MODE)) {
    // For Dual/Quad SPI, the data bits should be a multiple of 2/4.
    if ((spi->reg->CTRLR0_b.SPI_FRF == DUAL_SPI_FORMAT) && ((data_bits % 2) != 0U)) {
      return ARM_SPI_ERROR_DATA_BITS;
    } else if ((spi->reg->CTRLR0_b.SPI_FRF == QUAD_SPI_FORMAT) && ((data_bits % 4) != 0U)) {
      return ARM_SPI_ERROR_DATA_BITS;
    }
    spi->reg->CTRLR0_b.DFS_32 = (unsigned int)((data_bits - 1U) & 0x1F);
  } else {
    if ((data_bits >= SSI_4_BIT_DATA_WIDTH) && (data_bits <= SSI_16_BIT_DATA_WIDTH)) {
      spi->reg->CTRLR0_b.DFS = (unsigned int)((data_bits - 1U) & 0x0F);
    } else {
      return ARM_SPI_ERROR_DATA_BITS;
    }
  }
  // Configure Bit Order
  if ((control & ARM_SPI_BIT_ORDER_Msk) == ARM_SPI_LSB_MSB) {
    return ARM_SPI_ERROR_BIT_ORDER;
  }

  spi->reg->CTRLR0_b.SRL = 0;
  spi->reg->CTRLR0_b.FRF = MOTOROLA_SPI;
  spi->reg->TXFTLR_b.TFT = 0;
  spi->reg->RXFTLR_b.RFT = 0;

  if ((spi->instance_mode == SPI_MASTER_MODE) || (spi->instance_mode == SPI_ULP_MASTER_MODE)) {
    spi->reg->IMR &= (uint32_t)(~(0x3F)); // Disable SPI interrupts
  } else {
    spi->reg->IMR &= (uint32_t)(~(0x1F)); // Disable SPI interrupts
  }

  return ARM_DRIVER_OK;
}

#ifdef SSI_DUAL_QUAD_COMPONENT
/**
 * @fn          int32_t SPI_Command_Configure(const SPI_RESOURCES *spi, uint32_t inst_len, uint32_t addr_len, sl_ssi_frf_t spi_frf, sl_ssi_xfer_type_t xfer_type)
 * @brief       Configure the SPI controller for command phase operations.
 * @param[in]   spi       : Pointer to the SPI resources
 * @param[in]   inst_len  : Instruction length
 * @param[in]   addr_len  : Address length
 * @param[in]   spi_frf   : SPI frame format (SPI_FRF_STANDARD, SPI_FRF_DUAL, SPI_FRF_QUAD)
 * @param[in]   xfer_type : Transfer type (instruction and address phase configuration)
 * @return      int32_t   : ARM_DRIVER_OK on success, error code otherwise
 */
int32_t SPI_Command_Configure(const SPI_RESOURCES *spi,
                              uint32_t inst_len,
                              uint32_t addr_len,
                              sl_ssi_frf_t spi_frf,
                              sl_ssi_xfer_type_t xfer_type)
{
  if ((spi->reg == SSISlave) || (spi->reg == SSI2)) {
    return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  // Disable SSI before configuration
  spi->reg->SSIENR_b.SSI_EN = 0;

  // Set SPI frame format (standard/dual/quad) and transfer type
  spi->reg->CTRLR0_b.SPI_FRF        = spi_frf;
  spi->reg->SPI_CTRLR0_b.TRANS_TYPE = xfer_type;

  // Set instruction length and address length fields
  spi->reg->SPI_CTRLR0_b.INST_L = inst_len;
  spi->reg->SPI_CTRLR0_b.ADDR_L = addr_len;

  return ARM_DRIVER_OK;
}
/**
 * @fn          int32_t SPI_Command_Send(const SPI_RESOURCES *spi, uint32_t instruction, uint32_t address)
 * @brief       Send command sequence to a SPI device (instruction, address)
 * @param[in]   spi        : Pointer to the SPI resources
 * @param[in]   instruction: The instruction value to send in the command phase
 * @param[in]   address    : The address value to send in the command phase
 * @return      int32_t    : ARM_DRIVER_OK on success, error code otherwise
 */
int32_t SPI_Command_Send(const SPI_RESOURCES *spi, uint32_t instruction, uint32_t address)
{
  uint32_t inst_len = spi->reg->SPI_CTRLR0_b.INST_L;
  uint32_t addr_len = spi->reg->SPI_CTRLR0_b.ADDR_L;
  uint32_t spi_frf  = spi->reg->CTRLR0_b.SPI_FRF;
  // Enable SSI after configuration
  spi->reg->SSIENR_b.SSI_EN = 1;

  // Write instruction (if non-zero length)
  if (inst_len) {
    spi->reg->DR = instruction;
  }

  // Write address (if non-zero length)
  if (addr_len) {
    if (spi_frf == SSI_FRF_STANDARD) {
      // Write address bytes MSB first for standard SPI
      // This loop breaks the multi-byte address into individual bytes and sends them
      // starting from the most significant byte (MSB) down to the least significant byte (LSB)
      // Example: For a 3-byte address (addr_len=6), this sends bytes at positions 16, 8, 0
      for (int i = ((addr_len * 4) - SSI_8_BIT_DATA_WIDTH); i >= 0; i -= SSI_8_BIT_DATA_WIDTH) {
        // Extract and write one byte at bit position 'i'
        // The mask 0xFF ensures only the lower 8 bits are sent
        spi->reg->DR = (address >> i) & 0xFF;
      }
    } else {
      // For dual/quad, write the address as a single value
      spi->reg->DR = address;
    }
  }
  return ARM_DRIVER_OK;
}
#endif

/**
 * @fn          ARM_SPI_STATUS SPI_GetStatus(const SPI_RESOURCES *spi)
 * @brief		    Get SPI status.
 * @param[in]	  spi        : Pointer to the SPI resources
 * @return 		  ARM_SPI_STATUS
 */
ARM_SPI_STATUS SPI_GetStatus(const SPI_RESOURCES *spi)
{
  ARM_SPI_STATUS spi_status;

  spi_status.busy       = (uint32_t)(spi->info->status.busy & 0x01);
  spi_status.data_lost  = (uint32_t)(spi->info->status.data_lost & 0x01);
  spi_status.mode_fault = (uint32_t)(spi->info->status.mode_fault & 0x01);

  return spi_status;
}

/**
 * @fn          void SPI_IRQHandler(const SPI_RESOURCES *spi)
 * @brief		    SPI IRQ Handler
 * @param[in]	  spi        : Pointer to the SPI resources
 * @return 		  none
 */
void SPI_IRQHandler(const SPI_RESOURCES *spi)
{
  typedef volatile uint32_t vuint32_t __attribute__((may_alias));
  uint32_t data = 0U;
  uint32_t event;
  uint32_t isr;
  uint32_t sr;
  volatile uint32_t ovrrunclr;
  volatile uint32_t icr;
  event = 0U;

  isr = spi->reg->ISR;
  sr  = spi->reg->SR;
  icr = spi->reg->ICR;
  (void)icr;

  if ((isr & SPI_ISR_TX_FIFO_OVERFLOW) || (isr & SPI_ISR_RX_FIFO_OVERFLOW)
      || (isr & SPI_ISR_RX_FIFO_UNDERFLOW)) { // checking Tx FIFO Over Flow, Rx Overflow and Rx FIFO underflow
    // Overrun flag is set
    // Clear Overrun flag,
    ovrrunclr = spi->reg->TXOICR; /* Clear Transmit FIFO Overflow */
    ovrrunclr = spi->reg->RXOICR; /* Clear Receive FIFO Overflow Interrupt */
    ovrrunclr = spi->reg->RXUICR; /* Clear Receive FIFO Underflow Interrupt */
    (void)ovrrunclr;
    spi->info->status.data_lost = 1U;
    event |= ARM_SPI_EVENT_DATA_LOST;
  } else if ((sr & BIT(3)) && (isr & BIT(4))) {
    // FIFO contains one or more entries and receive full interrupt is triggered
    // Receive Buffer Not Empty
    if (spi->xfer->rx_buf) {
      // If rx_buf should not be null when receive full interrupt is triggered
      if (spi->xfer->rx_cnt < spi->xfer->num) {
        // If rx count is less than num transmission is still pending
        if (data_width_in_bytes == 1) {
          // For data width less than 8
          data                                       = *(volatile uint8_t *)(&spi->reg->DR);
          *(spi->xfer->rx_buf + spi->xfer->rx_cnt++) = (uint8_t)(data);
        } else if (data_width_in_bytes == 2) {
          // For data width 8-16
          data                                       = *(vuint32_t *)(&spi->reg->DR);
          *(spi->xfer->rx_buf + spi->xfer->rx_cnt++) = (uint8_t)(data);
          *(spi->xfer->rx_buf + spi->xfer->rx_cnt++) = (uint8_t)(data >> 8);
        } else if (data_width_in_bytes == 4) {
          // For data width 16-32
          data                                       = *(vuint32_t *)(&spi->reg->DR);
          *(spi->xfer->rx_buf + spi->xfer->rx_cnt++) = (uint8_t)(data);
          *(spi->xfer->rx_buf + spi->xfer->rx_cnt++) = (uint8_t)(data >> 8);
          *(spi->xfer->rx_buf + spi->xfer->rx_cnt++) = (uint8_t)(data >> 16);
          *(spi->xfer->rx_buf + spi->xfer->rx_cnt++) = (uint8_t)(data >> 24);
        }
        if (spi->xfer->rx_cnt >= spi->xfer->num) {
          // Disable RX Buffer Not Empty Interrupt
          spi->reg->IMR &= (uint32_t)(~RXFIM);
          // Clear busy flag
          spi->info->status.busy = 0U;
          // Transfer completed
          event |= ARM_SPI_EVENT_TRANSFER_COMPLETE;
        } else {
          // Only meant for master to write dummy byte in case of half duplex mode
          if (((spi->instance_mode == SPI_MASTER_MODE) || (spi->instance_mode == SPI_ULP_MASTER_MODE))
              && (spi->xfer->tx_buf == NULL)) {
            // If master mode or ulp master mode and tx buffer is null means receive only conditon where dummy byte needs to be generated
            *(vuint32_t *)(&spi->reg->DR) = DUMMY_DATA; // dummy data
                                                        // Waiting till the busy flag is cleared
            while (spi->reg->SR & BIT(0))
              ;
          }
        }
      } else {
        // Unexpected transfer, data lost
        event |= ARM_SPI_EVENT_DATA_LOST;
      }
    }
  } else if ((sr & BIT(2)) && (isr & BIT(0))) {
    // Transmit fifo contains one or more empty locations and transmit empty fifo interrupt is triggered
    if (spi->xfer->tx_buf) {
      // If tx_buf should not be null when transmit empty interrupt is triggered
      if (spi->xfer->tx_cnt < spi->xfer->num) {
        // If tx count is less than num transmission is still pending
        if (data_width_in_bytes == 1) {
          // For data width less than 8
          data                                 = *(spi->xfer->tx_buf + spi->xfer->tx_cnt++);
          *(volatile uint8_t *)(&spi->reg->DR) = data;
        } else if (data_width_in_bytes == 2) {
          // For data width 8-16
          data = *(spi->xfer->tx_buf + spi->xfer->tx_cnt++);
          data |= *(spi->xfer->tx_buf + spi->xfer->tx_cnt++) << 8;
          *(vuint32_t *)(&spi->reg->DR) = (uint16_t)data;
        } else if (data_width_in_bytes == 4) {
          // For data width 16-32
          data = *(spi->xfer->tx_buf + spi->xfer->tx_cnt++);
          data |= *(spi->xfer->tx_buf + spi->xfer->tx_cnt++) << 8;
          data |= *(spi->xfer->tx_buf + spi->xfer->tx_cnt++) << 16;
          data |= *(spi->xfer->tx_buf + spi->xfer->tx_cnt++) << 24;
          *(vuint32_t *)(&spi->reg->DR) = data;
        }
        // Waiting till the busy flag is cleared
        if ((spi->instance_mode == SPI_MASTER_MODE) || (spi->instance_mode == SPI_ULP_MASTER_MODE)) {
          while (spi->reg->SR & BIT(0))
            ;
        }
        if (spi->xfer->tx_cnt >= spi->xfer->num) {
          // All data sent, disable TX Buffer Empty Interrupt
          spi->reg->IMR &= (uint32_t)(~TXEIM);
          // Clear busy flag
          if (spi->reg->CTRLR0_b.TMOD == TRANSMIT_ONLY) {
            // Transfer completed
            event |= ARM_SPI_EVENT_TRANSFER_COMPLETE;
            spi->info->status.busy = 0U;
          }
        }
      } else {
        // Unexpected transfer, data lost
        event |= ARM_SPI_EVENT_DATA_LOST;
      }
    }
  }

  // Send event
  if (event && spi->info->cb_event) {
    uint8_t ssi_instance = 0;
    // Validate the SSI instance that triggered this event
    if (spi->reg == SSI0) {
      // Assigning instance number to the callback variable
      ssi_instance = SSI_MASTER_INSTANCE;
    } else if (spi->reg == SSISlave) {
      // Assigning instance number to the callback variable
      ssi_instance = SSI_SLAVE_INSTANCE;
    } else if (spi->reg == SSI2) {
      // Assigning instance number to the callback variable
      ssi_instance = SSI_ULP_MASTER_INSTANCE;
    }
    // Appending the instance value in the callback event variable
    event |= (ssi_instance << SSI_INSTANCE_BIT);
    spi->info->cb_event(event);
#if !(defined(SSI_ULP_MASTER_RX_DMA_Instance) && (SSI_ULP_MASTER_RX_DMA_Instance == 1))
#ifndef SI917_MEMLCD
    // CS pin setting high after transferred all the bytes of data.
    if (spi->instance_mode == SPI_ULP_MASTER_MODE) {
      // Set CS0 pin state to SET
      if (spi->io.cs0 != NULL) {
        SPI_Set_CS_Pin_State(cs0_gpio_mode_pin, cs0_gpio_port, SET);
      }
      // Set CS1 pin state to SET
      if (spi->io.cs1 != NULL) {
        SPI_Set_CS_Pin_State(cs1_gpio_mode_pin, cs1_gpio_port, SET);
      }
      // Set CS2 pin state to SET
      if (spi->io.cs2 != NULL) {
        SPI_Set_CS_Pin_State(cs2_gpio_mode_pin, cs2_gpio_port, SET);
      }
    }
#endif
#endif
    if (spi->info->status.busy) {
      spi->info->status.busy = 0U;
    }
  }
}
#else
typedef int dummy; // To remove empty translation unit warning.
#endif // SSI_ROMDRIVER_PRESENT

#ifdef SSI_DUAL_QUAD_COMPONENT
/**
 * @brief Configures DMA for SSI/SPI data transfer
 *
 * @details
 * This function configures the DMA controller for SSI/SPI data transfers. It handles both
 * transmit and receive operations with appropriate settings based on data width and transfer
 * direction. The function configures DMA channel parameters, hardware triggers, and callback
 * functions required for the transfer.
 *
 * @param[in] spi        Pointer to SPI resources structure
 * @param[in] num        Number of data items to transfer (1-1024)
 * @param[in] data_width Width of each data item in bits (1-32)
 * @param[in] xfer_type  Transfer direction (TRANSMIT_ONLY or RECEIVE_ONLY)
 *
 * @return Status code
 *         - SL_STATUS_OK: DMA configured successfully
 *         - ARM_DRIVER_ERROR_PARAMETER: Invalid transfer type
 *         - ARM_DRIVER_ERROR: DMA setup failed
 */
static int32_t SPI_Configure_Dma_Transfer(const SPI_RESOURCES *spi,
                                          uint32_t num,
                                          uint16_t data_width,
                                          uint16_t xfer_type)
{
  int32_t status                          = SL_STATUS_OK;
  RSI_UDMA_CHA_CONFIG_DATA_T control_data = { 0 };
  sl_dma_xfer_t dma_transfer              = { 0 };
  uint32_t channel                        = 0;
  uint32_t channel_priority               = 0;
  sl_dma_callback_t spi_callback;

  // Parameter validation - only single-direction transfers supported
  if ((xfer_type != TRANSMIT_ONLY) && (xfer_type != RECEIVE_ONLY)) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }
  // Configure basic DMA transfer parameters common to both TX and RX
  control_data.transferType = UDMA_MODE_BASIC; // Basic mode
  control_data.nextBurst    = 0;               // Disable burst transfers

  // Set transfer count with 10-bit limit (max 1024 items per DMA operation)
  if (num < 1024) {
    control_data.totalNumOfDMATrans = (unsigned int)((num - 1) & 0x03FF);
  } else {
    control_data.totalNumOfDMATrans = 0x3FF;
  }

  control_data.rPower      = ARBSIZE_1;
  control_data.srcProtCtrl = 0x0;
  control_data.dstProtCtrl = 0x0;
  if (xfer_type == TRANSMIT_ONLY) {
    control_data.dstInc = DST_INC_NONE;
    if (data_width <= (8U - 1U)) {
      // 8-bit data frame
      control_data.srcSize = SRC_SIZE_8;
      control_data.srcInc  = SRC_INC_8;
      control_data.dstSize = DST_SIZE_8;
    } else if (data_width <= (16U - 1U)) {
      // 16-bit data frame
      control_data.srcSize = SRC_SIZE_16;
      control_data.srcInc  = SRC_INC_16;
      control_data.dstSize = DST_SIZE_16;
    } else {
      // 32-bit data frame
      control_data.srcSize = SRC_SIZE_32;
      control_data.srcInc  = SRC_INC_32;
      control_data.dstSize = DST_SIZE_32;
    }
    spi->reg->DMATDLR_b.DMATDL = 1;
    spi->reg->DMACR_b.TDMAE    = 1;
    channel                    = spi->tx_dma->channel + 1;
    channel_priority           = spi->tx_dma->chnl_cfg.channelPrioHigh;

    //Initialize sl_dma callback structure
    spi_callback.transfer_complete_cb = ssi_transfer_complete_callback;
    spi_callback.error_cb             = ssi_error_callback;
    //Initialize sl_dma transfer structure
    dma_transfer.src_addr      = (uint32_t *)((uint32_t)(spi->xfer->tx_buf));
    dma_transfer.dest_addr     = (uint32_t *)((uint32_t) & (spi->reg->DR));
    dma_transfer.transfer_type = SL_DMA_MEMORY_TO_PERIPHERAL;
    dma_transfer.signal        = (uint8_t)spi->tx_dma->chnl_cfg.periAck;
  } else {
    control_data.srcInc = SRC_INC_NONE;
    if (data_width <= (8U - 1U)) {
      // 8-bit data frame
      control_data.srcSize = SRC_SIZE_8;
      control_data.dstSize = DST_SIZE_8;
      control_data.dstInc  = DST_INC_8;
    } else if (data_width <= (16U - 1U)) {
      // 16-bit data frame
      control_data.srcSize = SRC_SIZE_16;
      control_data.dstSize = DST_SIZE_16;
      control_data.dstInc  = DST_INC_16;
    } else {
      // 32-bit data frame
      control_data.srcSize = SRC_SIZE_32;
      control_data.dstSize = DST_SIZE_32;
      control_data.dstInc  = DST_INC_32;
    }
    spi->reg->DMARDLR_b.DMARDL = 0;
    spi->reg->DMACR_b.RDMAE    = 1;

    channel          = spi->rx_dma->channel + 1;
    channel_priority = spi->rx_dma->chnl_cfg.channelPrioHigh;
    //Initialize sl_dma callback structure
    spi_callback.transfer_complete_cb = ssi_transfer_complete_callback;
    spi_callback.error_cb             = ssi_error_callback;
    //Initialize sl_dma transfer structure
    dma_transfer.src_addr      = (uint32_t *)((uint32_t) & (spi->reg->DR));
    dma_transfer.dest_addr     = (uint32_t *)((uint32_t)(spi->xfer->rx_buf));
    dma_transfer.transfer_type = SL_DMA_PERIPHERAL_TO_MEMORY;
    dma_transfer.signal        = (uint8_t)spi->rx_dma->chnl_cfg.periAck;
  }

  dma_transfer.dma_mode       = control_data.transferType;
  dma_transfer.transfer_count = num;
  dma_transfer.src_inc        = control_data.srcInc;
  dma_transfer.dst_inc        = control_data.dstInc;
  dma_transfer.xfer_size      = control_data.dstSize;

  //Allocate DMA channel for Tx
  status = sl_si91x_dma_allocate_channel(DMA_INSTANCE, &channel, channel_priority);
  if (status && (status != SL_STATUS_DMA_CHANNEL_ALLOCATED)) {
    return ARM_DRIVER_ERROR;
  }
  //Register transfer complete and error callback
  if (sl_si91x_dma_register_callbacks(DMA_INSTANCE, channel, &spi_callback)) {
    return ARM_DRIVER_ERROR;
  }
  //Configure the channel for DMA transfer
  if (sl_si91x_dma_transfer(DMA_INSTANCE, channel, &dma_transfer)) {
    return ARM_DRIVER_ERROR;
  }

  return status;
}
#endif

/// @cond PRIVATE
static volatile uint32_t dummy_data = 0;
/// @endcond
/**
 * @fn           int32_t SPI_Send(const void *data, uint32_t num, const SPI_RESOURCES *spi, UDMA_RESOURCES *udma, UDMA_Channel_Info *chnl_info, RSI_UDMA_HANDLE_T udmaHandle)
 * @brief        Start sending data to SPI transmitter.
 * @param[in]    data     : Pointer to buffer with data to send to SPI transmitter
 * @param[in]    num      : Number of data items to send
 * @param[in]    spi       : Pointer to the spi resources
 * @param[in]    udma       : Pointer to the udma resources
 * @param[in]    chnl_info  : Pointer for channel info
 * @param[in]    udmaHandle : Pointer to the UDMA Handle
 * @return 		   excecution status 
 */

int32_t SPI_Send(const void *data,
                 uint32_t num,
                 const SPI_RESOURCES *spi,
                 UDMA_RESOURCES *udma,
                 UDMA_Channel_Info *chnl_info,
                 RSI_UDMA_HANDLE_T udmaHandle)
{
#ifdef SL_SI91X_SSI_DMA
  //Added to suppress unused variable warning
  (void)udma;
  (void)udmaHandle;
  (void)chnl_info;
  sl_status_t status;
#else
  volatile int32_t stat;
#endif
  uint16_t data_width;
  RSI_UDMA_CHA_CONFIG_DATA_T control = { 0 };
  dummy_data                         = 0;

  if ((data == NULL) || (num == 0U)) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }
  if (!(spi->info->state & SPI_CONFIGURED)) {
    return ARM_DRIVER_ERROR;
  }
  if (spi->info->status.busy) {
    return ARM_DRIVER_ERROR_BUSY;
  }
  // Set the mode fault event if slave select is deactivated in master/ulp_master instance mode.
  if (SPI_Set_Mode_Fault_Event(spi)) {
    return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  spi->info->status.busy       = 1U;
  spi->info->status.data_lost  = 0U;
  spi->info->status.mode_fault = 0U;

  spi->xfer->tx_buf = (uint8_t *)data;
  spi->xfer->rx_buf = NULL;
  spi->xfer->rx_cnt = 0U;
  spi->xfer->tx_cnt = 0U;

  spi->reg->CTRLR0_b.TMOD = TRANSMIT_ONLY;

  spi->reg->SPI_CTRLR0_b.INST_L = 0;
  spi->reg->SPI_CTRLR0_b.ADDR_L = 0;

  if ((spi->instance_mode == SPI_MASTER_MODE) || (spi->instance_mode == SPI_ULP_MASTER_MODE)) {
    data_width = spi->reg->CTRLR0_b.DFS_32;
  } else {
    data_width = spi->reg->CTRLR0_b.DFS;
  }

  spi->reg->SSIENR = SSI_ENABLE;

  if (spi->tx_dma != NULL) {
    control.transferType = UDMA_MODE_BASIC;
    control.nextBurst    = 0;
    if (num < 1024) {
      control.totalNumOfDMATrans = (unsigned int)((num - 1) & 0x03FF);
    } else {
      control.totalNumOfDMATrans = 0x3FF;
    }
    control.rPower      = ARBSIZE_1;
    control.srcProtCtrl = 0x0;
    control.dstProtCtrl = 0x0;
    control.dstInc      = DST_INC_NONE;
    if (data_width <= (8U - 1U)) {
      // 8-bit data frame
      control.srcSize = SRC_SIZE_8;
      control.srcInc  = SRC_INC_8;
      control.dstSize = DST_SIZE_8;
    } else if (data_width <= (16U - 1U)) {
      // 16-bit data frame
      control.srcSize = SRC_SIZE_16;
      control.srcInc  = SRC_INC_16;
      control.dstSize = DST_SIZE_16;
    } else {
      // 32-bit data frame
      control.srcSize = SRC_SIZE_32;
      control.srcInc  = SRC_INC_32;
      control.dstSize = DST_SIZE_32;
    }
    spi->reg->DMACR_b.TDMAE    = 1;
    spi->reg->DMATDLR_b.DMATDL = 1;
#if SL_SI91X_SSI_DMA
    sl_dma_xfer_t dma_transfer_tx = { 0 };
    uint32_t channel              = spi->tx_dma->channel + 1;
    uint32_t channel_priority     = spi->tx_dma->chnl_cfg.channelPrioHigh;
    sl_dma_callback_t spi_tx_callback;
    //Initialize sl_dma callback structure
    spi_tx_callback.transfer_complete_cb = ssi_transfer_complete_callback;
    spi_tx_callback.error_cb             = ssi_error_callback;
    //Initialize sl_dma transfer structure
    dma_transfer_tx.src_addr       = (uint32_t *)((uint32_t)(spi->xfer->tx_buf));
    dma_transfer_tx.dest_addr      = (uint32_t *)((uint32_t) & (spi->reg->DR));
    dma_transfer_tx.src_inc        = control.srcInc;
    dma_transfer_tx.dst_inc        = control.dstInc;
    dma_transfer_tx.xfer_size      = control.dstSize;
    dma_transfer_tx.transfer_count = num;
    dma_transfer_tx.transfer_type  = SL_DMA_MEMORY_TO_PERIPHERAL;
    dma_transfer_tx.dma_mode       = control.transferType;
    dma_transfer_tx.signal         = (uint8_t)spi->tx_dma->chnl_cfg.periAck;

    //Allocate DMA channel for Tx
    status = sl_si91x_dma_allocate_channel(DMA_INSTANCE, &channel, channel_priority);
    if (status && (status != SL_STATUS_DMA_CHANNEL_ALLOCATED)) {
      return ARM_DRIVER_ERROR;
    }
    //Register transfer complete and error callback
    if (sl_si91x_dma_register_callbacks(DMA_INSTANCE, channel, &spi_tx_callback)) {
      return ARM_DRIVER_ERROR;
    }
    //Configure the channel for DMA transfer
    if (sl_si91x_dma_transfer(DMA_INSTANCE, channel, &dma_transfer_tx)) {
      return ARM_DRIVER_ERROR;
    }
    sl_si91x_dma_channel_enable(DMA_INSTANCE, spi->tx_dma->channel + 1);
    sl_si91x_dma_enable(DMA_INSTANCE);
#else
    // Initialize and start SPI TX DMA Stream
    stat = UDMAx_ChannelConfigure(udma,
                                  spi->tx_dma->channel,
                                  (uint32_t)(spi->xfer->tx_buf),
                                  (uint32_t) & (spi->reg->DR),
                                  num,
                                  control,
                                  &spi->tx_dma->chnl_cfg,
                                  spi->tx_dma->cb_event,
                                  chnl_info,
                                  udmaHandle);
    if (stat == -1) {
      return ARM_DRIVER_ERROR;
    }
    UDMAx_ChannelEnable(spi->tx_dma->channel, udma, udmaHandle);
    UDMAx_ChannelEnable(spi->rx_dma->channel, udma, udmaHandle);
    UDMAx_DMAEnable(udma, udmaHandle);
#endif
  } else {
    // Converts the data width into number of bytes to send/receive at once
    SPI_Convert_Data_Width_To_Bytes(data_width);
    // Total number of transfers according to the configured data width
    spi->xfer->num = num * data_width_in_bytes;

    /* spi->reg->IMR |= (TXEIM | RXFIM); */
    /*enabled below bits 
     Transmit FIFO Empty Interrupt Mask
     Transmit FIFO Overflow Interrupt Mask
     Receive FIFO Underflow Interrupt Mask
     Receive FIFO Overflow Interrupt Mask
     Receive FIFO Full Interrupt Mask
     */
#if !(defined(SSI_ULP_MASTER_RX_DMA_Instance) && (SSI_ULP_MASTER_RX_DMA_Instance == 1))
#ifndef SI917_MEMLCD
    // Down the CS pin for ULP primary to send the data.
    if (spi->instance_mode == SPI_ULP_MASTER_MODE) {
      // Set CS0 pin state
      if (spi->io.cs0 != NULL) {
        SPI_Set_CS_Pin_State(cs0_gpio_mode_pin, cs0_gpio_port, CLR);
      }
      // Set CS1 pin state
      if (spi->io.cs1 != NULL) {
        SPI_Set_CS_Pin_State(cs1_gpio_mode_pin, cs1_gpio_port, CLR);
      }
      // Set CS2 pin state
      if (spi->io.cs2 != NULL) {
        SPI_Set_CS_Pin_State(cs2_gpio_mode_pin, cs2_gpio_port, CLR);
      }
    }
#endif
#endif
    spi->reg->IMR |= (TXEIM | TXOIM);
  }
  return ARM_DRIVER_OK;
}
#ifdef SSI_DUAL_QUAD_COMPONENT
/**
 * @fn          int32_t SPI_Send_Command_Data(const void *data, uint32_t num, uint32_t instruction, uint32_t address, 
 *                                     const SPI_RESOURCES *spi, UDMA_RESOURCES *udma,
 *                                     UDMA_Channel_Info *chnl_info, RSI_UDMA_HANDLE_T udmaHandle)
 * @brief       Advanced SPI data transmission with instruction and address phases
 * @param[in]   data       : Pointer to data buffer to send
 * @param[in]   num        : Number of data items to send
 * @param[in]   instruction: Instruction value for command phase
 * @param[in]   address    : Address value for command phase
 * @param[in]   spi        : Pointer to SPI resources
 * @param[in]   udma       : Pointer to UDMA resources (used for DMA transfers)
 * @param[in]   chnl_info  : Pointer to UDMA channel information
 * @param[in]   udmaHandle : UDMA handle for the transfer
 * @return      int32_t    : ARM_DRIVER_OK on success, error code otherwise
 */
int32_t SPI_Send_Command_Data(const void *data,
                              uint32_t num,
                              uint32_t instruction,
                              uint32_t address,
                              const SPI_RESOURCES *spi,
                              UDMA_RESOURCES *udma,
                              UDMA_Channel_Info *chnl_info,
                              RSI_UDMA_HANDLE_T udmaHandle)
{
  //Added to suppress unused variable warning
  (void)udma;
  (void)chnl_info;
  int32_t status;
  uint16_t data_width;
  if ((data == NULL) || (num == 0U)) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }
  if (!(spi->info->state & SPI_CONFIGURED)) {
    return ARM_DRIVER_ERROR;
  }
  if (spi->info->status.busy) {
    return ARM_DRIVER_ERROR_BUSY;
  }
  // Set the mode fault event if slave select is deactivated in master/ulp_master instance mode.
  if (SPI_Set_Mode_Fault_Event(spi)) {
    return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  spi->info->status.busy       = 1U;
  spi->info->status.mode_fault = 0U;
  spi->info->status.data_lost  = 0U;
  spi->xfer->rx_buf            = NULL;
  spi->xfer->tx_buf            = (uint8_t *)data;
  spi->xfer->tx_cnt            = 0U;
  spi->xfer->rx_cnt            = 0U;
  spi->xfer->num               = num;
  spi->reg->CTRLR0_b.TMOD      = TRANSMIT_ONLY;
  // Get data width from CTRLR0 register
  data_width = spi->reg->CTRLR0_b.DFS_32;
  // Converts the data width into number of bytes to send/receive at once
  SPI_Convert_Data_Width_To_Bytes(data_width);

  if (spi->tx_dma != NULL) {
    RSI_UDMA_DATACONTEXT_T *pDrv = (RSI_UDMA_DATACONTEXT_T *)udmaHandle;
    uint32_t inst_len            = spi->reg->SPI_CTRLR0_b.INST_L;
    uint32_t addr_len            = spi->reg->SPI_CTRLR0_b.ADDR_L;
    uint32_t spi_frf             = spi->reg->CTRLR0_b.SPI_FRF;
    status                       = SPI_Configure_Dma_Transfer(spi, spi->xfer->num, data_width, TRANSMIT_ONLY);
    if (status == ARM_DRIVER_ERROR) {
      return status;
    }
    sl_si91x_dma_enable(DMA_INSTANCE);

    // Enable SSI after configuration
    spi->reg->SSIENR_b.SSI_EN = 1;

    // Write instruction (if non-zero length)
    if (inst_len) {
      spi->reg->DR = instruction;
    }
    // Write address
    if (spi_frf == SSI_FRF_STANDARD) {
      // Write address bytes MSB first for standard SPI
      // This loop breaks the multi-byte address into individual bytes and sends them
      // starting from the most significant byte (MSB) down to the least significant byte (LSB)
      // Example: For a 3-byte address (addr_len=6), this sends bytes at positions 16, 8, 0
      for (int i = ((addr_len * 4) - SSI_8_BIT_DATA_WIDTH); i >= 0; i -= SSI_8_BIT_DATA_WIDTH) {
        // Extract and write one byte at bit position 'i'
        // The mask 0xFF ensures only the lower 8 bits are sent
        spi->reg->DR = (address >> i) & 0xFF;
      }
      // Enable the DMA channel
      pDrv->base->CHNL_ENABLE_SET = SET_BIT(spi->tx_dma->channel);
    } else if (addr_len) {
      // Enable the DMA channel
      pDrv->base->CHNL_ENABLE_SET = SET_BIT(spi->tx_dma->channel);
      // For dual/quad, write the address as a single value
      spi->reg->DR = address;
    }
  } else {
    return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  return ARM_DRIVER_OK;
}
#endif
/**
 * @fn          int32_t SPI_Receive(void *data, uint32_t num, const SPI_RESOURCES *spi, UDMA_RESOURCES *udma, UDMA_Channel_Info *chnl_info, RSI_UDMA_HANDLE_T udmaHandle)
 * @brief        Start receiving data from SPI receiver.
 * @param[in]    data        : Pointer to buffer for data to receive from SPI receiver
 * @param[in]    num         : Number of data items to receive
 * @param[in]    spi         : Pointer to the SPI resources
 * @param[in]	   udma        : Pointer to the udma resources
 * @param[in]	   chnl_info   : Pointer to channel info
 * @param[in]	   udmaHandle  : Pointer to UDMA Handle
 * @return 		   excecution status
 */
int32_t SPI_Receive(void *data,
                    uint32_t num,
                    const SPI_RESOURCES *spi,
                    UDMA_RESOURCES *udma,
                    UDMA_Channel_Info *chnl_info,
                    RSI_UDMA_HANDLE_T udmaHandle)
{
#ifdef SL_SI91X_SSI_DMA
  //Added to suppress unused variable warning
  (void)udma;
  (void)udmaHandle;
  (void)chnl_info;
  sl_status_t status;
#else
  volatile int32_t stat;
#endif
  RSI_UDMA_CHA_CONFIG_DATA_T control = { 0 };
  uint16_t data_width;
  dummy_data = 0;

  if ((data == NULL) || (num == 0U)) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }
  if (!(spi->info->state & SPI_CONFIGURED)) {
    return ARM_DRIVER_ERROR;
  }
  if (spi->info->status.busy) {
    return ARM_DRIVER_ERROR_BUSY;
  }
  // Set the mode fault event if slave select is deactivated in master/ulp_master instance mode.
  if (SPI_Set_Mode_Fault_Event(spi)) {
    return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  spi->info->status.busy       = 1U;
  spi->info->status.data_lost  = 0U;
  spi->info->status.mode_fault = 0U;

  spi->xfer->tx_buf = NULL;
  spi->xfer->rx_buf = (uint8_t *)data;
  spi->xfer->rx_cnt = 0U;
  spi->xfer->tx_cnt = 0U;

#ifdef SSI_INSTANCE_CONFIG
#if (SL_SSI_PRIMARY_DMA_CONFIG_ENABLE) || (SL_SSI_SECONDARY_DMA_CONFIG_ENABLE) || (SL_SSI_ULP_PRIMARY_DMA_CONFIG_ENABLE)
  spi->reg->CTRLR0_b.TMOD = TRANSMIT_AND_RECEIVE;
#else
  spi->reg->CTRLR0_b.TMOD = RECEIVE_ONLY;
#endif
#elif SSI_CONFIG
#if (SL_SSI_MASTER_DMA_CONFIG_ENABLE) || (SL_SSI_SLAVE_DMA_CONFIG_ENABLE) || (SL_SSI_ULP_MASTER_DMA_CONFIG_ENABLE)
  spi->reg->CTRLR0_b.TMOD = TRANSMIT_AND_RECEIVE;
#else
  spi->reg->CTRLR0_b.TMOD = RECEIVE_ONLY;
#endif
#endif

  if ((spi->instance_mode == SPI_MASTER_MODE) || (spi->instance_mode == SPI_ULP_MASTER_MODE)) {
    data_width = spi->reg->CTRLR0_b.DFS_32;
  } else {
    data_width = spi->reg->CTRLR0_b.DFS;
  }

  spi->reg->SSIENR = SSI_ENABLE;

  if ((spi->rx_dma != NULL) || (spi->tx_dma != NULL)) {
    if (spi->rx_dma != NULL) {
      control.transferType = UDMA_MODE_BASIC;
      control.nextBurst    = 0;
      if (num < 1024) {
        control.totalNumOfDMATrans = (unsigned int)((num - 1) & 0x03FF);
      } else {
        control.totalNumOfDMATrans = 0x3FF;
      }
      control.rPower      = ARBSIZE_1;
      control.srcProtCtrl = 0x0;
      control.dstProtCtrl = 0x0;
      control.srcInc      = SRC_INC_NONE;
      if (data_width <= (8U - 1U)) {
        // 8-bit data frame
        control.srcSize = SRC_SIZE_8;
        control.dstSize = DST_SIZE_8;
        control.dstInc  = DST_INC_8;
      } else if (data_width <= (16U - 1U)) {
        // 16-bit data frame
        control.srcSize = SRC_SIZE_16;
        control.dstSize = DST_SIZE_16;
        control.dstInc  = DST_INC_16;
      } else {
        // 32-bit data frame
        control.srcSize = SRC_SIZE_32;
        control.dstSize = DST_SIZE_32;
        control.dstInc  = DST_INC_32;
      }
      spi->reg->DMACR_b.RDMAE    = 1;
      spi->reg->DMARDLR_b.DMARDL = 0;
#if SL_SI91X_SSI_DMA
      sl_dma_xfer_t dma_transfer_rx = { 0 };
      uint32_t channel              = spi->rx_dma->channel + 1;
      uint32_t channel_priority     = spi->rx_dma->chnl_cfg.channelPrioHigh;
      sl_dma_callback_t spi_rx_callback;
      //Initialize sl_dma callback structure
      spi_rx_callback.transfer_complete_cb = ssi_transfer_complete_callback;
      spi_rx_callback.error_cb             = ssi_error_callback;
      //Initialize sl_dma transfer structure
      dma_transfer_rx.src_addr       = (uint32_t *)((uint32_t) & (spi->reg->DR));
      dma_transfer_rx.dest_addr      = (uint32_t *)((uint32_t)(spi->xfer->rx_buf));
      dma_transfer_rx.src_inc        = control.srcInc;
      dma_transfer_rx.dst_inc        = control.dstInc;
      dma_transfer_rx.xfer_size      = control.dstSize;
      dma_transfer_rx.transfer_count = num;
      dma_transfer_rx.transfer_type  = SL_DMA_PERIPHERAL_TO_MEMORY;
      dma_transfer_rx.dma_mode       = control.transferType;
      dma_transfer_rx.signal         = (uint8_t)spi->rx_dma->chnl_cfg.periAck;

      //Allocate DMA channel for Tx
      status = sl_si91x_dma_allocate_channel(DMA_INSTANCE, &channel, channel_priority);
      if (status && (status != SL_STATUS_DMA_CHANNEL_ALLOCATED)) {
        return ARM_DRIVER_ERROR;
      }
      //Register transfer complete and error callback
      if (sl_si91x_dma_register_callbacks(DMA_INSTANCE, channel, &spi_rx_callback)) {
        return ARM_DRIVER_ERROR;
      }
      if (sl_si91x_dma_transfer(DMA_INSTANCE, channel, &dma_transfer_rx)) {
        return ARM_DRIVER_ERROR;
      }
      sl_si91x_dma_channel_enable(DMA_INSTANCE, spi->rx_dma->channel + 1);
#else
      // Initialize and start SPI RX DMA Stream
      stat = UDMAx_ChannelConfigure(udma,
                                    spi->rx_dma->channel,
                                    (uint32_t) & (spi->reg->DR),
                                    (uint32_t)(spi->xfer->rx_buf),
                                    num,
                                    control,
                                    &spi->rx_dma->chnl_cfg,
                                    spi->rx_dma->cb_event,
                                    chnl_info,
                                    udmaHandle);
      if (stat == -1) {
        return ARM_DRIVER_ERROR;
      }
      UDMAx_ChannelEnable(spi->rx_dma->channel, udma, udmaHandle);
#endif
    }
    if ((spi->instance_mode == SPI_MASTER_MODE) || (spi->instance_mode == SPI_ULP_MASTER_MODE)) {
      if (spi->tx_dma != NULL) {
        control.transferType = UDMA_MODE_BASIC;
        control.nextBurst    = 0;
        if (num < 1024) {
          control.totalNumOfDMATrans = (unsigned int)((num - 1) & 0x03FF);
        } else {
          control.totalNumOfDMATrans = 0x3FF;
        }
        control.rPower      = ARBSIZE_1;
        control.srcProtCtrl = 0x0;
        control.dstProtCtrl = 0x0;
        control.dstInc      = DST_INC_NONE;
        if (data_width <= (8U - 1U)) {
          // 8-bit data frame
          control.srcSize = SRC_SIZE_8;
          control.srcInc  = SRC_SIZE_8;
          control.dstSize = DST_SIZE_8;
        } else if (data_width <= (16U - 1U)) {
          // 16-bit data frame
          control.srcSize = SRC_SIZE_16;
          control.srcInc  = SRC_SIZE_16;
          control.dstSize = DST_SIZE_16;
        } else {
          // 32-bit data frame
          control.srcSize = SRC_SIZE_32;
          control.srcInc  = SRC_SIZE_32;
          control.dstSize = DST_SIZE_32;
        }
        spi->reg->DMACR_b.TDMAE    = 1;
        spi->reg->DMATDLR_b.DMATDL = 1;
        // Initialize and start SPI TX DMA Stream
#if SL_SI91X_SSI_DMA
        sl_dma_xfer_t dma_transfer_tx = { 0 };
        uint32_t channel              = spi->tx_dma->channel + 1;
        uint32_t channel_priority     = spi->tx_dma->chnl_cfg.channelPrioHigh;
        sl_dma_callback_t spi_tx_callback;
        //Initialize sl_dma callback structure
        spi_tx_callback.transfer_complete_cb = ssi_transfer_complete_callback;
        spi_tx_callback.error_cb             = ssi_error_callback;
        //Initialize sl_dma transfer structure
        if (RSI_PS_IsPS2State()) {
          dma_transfer_tx.src_addr = (uint32_t *)((uint32_t)(ULP_SSI_DUMMY_DATA));
        } else {
          dma_transfer_tx.src_addr = (uint32_t *)((uint32_t) & (dummy_data));
        }
        dma_transfer_tx.dest_addr      = (uint32_t *)((uint32_t) & (spi->reg->DR));
        dma_transfer_tx.src_inc        = control.srcInc;
        dma_transfer_tx.dst_inc        = control.dstInc;
        dma_transfer_tx.xfer_size      = control.dstSize;
        dma_transfer_tx.transfer_count = num;
        dma_transfer_tx.transfer_type  = SL_DMA_MEMORY_TO_PERIPHERAL;
        dma_transfer_tx.dma_mode       = control.transferType;
        dma_transfer_tx.signal         = (uint8_t)spi->tx_dma->chnl_cfg.periAck;

        //Allocate DMA channel for Tx
        status = sl_si91x_dma_allocate_channel(DMA_INSTANCE, &channel, channel_priority);
        if (status && (status != SL_STATUS_DMA_CHANNEL_ALLOCATED)) {
          return ARM_DRIVER_ERROR;
        }
        //Register transfer complete and error callback
        if (sl_si91x_dma_register_callbacks(DMA_INSTANCE, channel, &spi_tx_callback)) {
          return ARM_DRIVER_ERROR;
        }
        //Configure the channel for DMA transfer
        if (sl_si91x_dma_transfer(DMA_INSTANCE, channel, &dma_transfer_tx)) {
          return ARM_DRIVER_ERROR;
        }
        sl_si91x_dma_channel_enable(DMA_INSTANCE, spi->tx_dma->channel + 1);
#else
        stat = UDMAx_ChannelConfigure(udma,
                                      spi->tx_dma->channel,
#if (defined(SSI_ULP_MASTER_RX_DMA_Instance) && (SSI_ULP_MASTER_RX_DMA_Instance == 1))
                                      (uint32_t)(ULP_SSI_MASTER_BUF_MEMORY),
#else
                                      (uint32_t) & (dummy_data),
#endif
                                      (uint32_t) & (spi->reg->DR),
                                      num,
                                      control,
                                      &spi->tx_dma->chnl_cfg,
                                      spi->tx_dma->cb_event,
                                      chnl_info,
                                      udmaHandle);
        if (stat == -1) {
          return ARM_DRIVER_ERROR;
        }
        UDMAx_ChannelEnable(spi->tx_dma->channel, udma, udmaHandle);
#endif // SL_SI91X_SSI_DMA
      }
    }
#if SL_SI91X_SSI_DMA
    sl_si91x_dma_enable(DMA_INSTANCE);
#else
    UDMAx_DMAEnable(udma, udmaHandle);
#endif
  } else {
    // Converts the data width into number of bytes to send/receive at once
    SPI_Convert_Data_Width_To_Bytes(data_width);
    // Total number of transfers according to the configured data width
    spi->xfer->num = num * data_width_in_bytes;

    // Interrupt mode
    // RX Buffer not empty interrupt enable
    /* spi->reg->IMR |= (TXEIM | RXFIM); */
    /*enabled below bits 
     Transmit FIFO Empty Interrupt Mask
     Transmit FIFO Overflow Interrupt Mask
     Receive FIFO Underflow Interrupt Mask
     Receive FIFO Overflow Interrupt Mask
     Receive FIFO Full Interrupt Mask
     */
#if !(defined(SSI_ULP_MASTER_RX_DMA_Instance) && (SSI_ULP_MASTER_RX_DMA_Instance == 1))
#ifndef SI917_MEMLCD
    // Down the CS pin for ULP primary to receive the data.
    if (spi->instance_mode == SPI_ULP_MASTER_MODE) {
      // Set CS0 pin state
      if (spi->io.cs0 != NULL) {
        SPI_Set_CS_Pin_State(cs0_gpio_mode_pin, cs0_gpio_port, CLR);
      }
      // Set CS1 pin state
      if (spi->io.cs1 != NULL) {
        SPI_Set_CS_Pin_State(cs1_gpio_mode_pin, cs1_gpio_port, CLR);
      }
      // Set CS2 pin state
      if (spi->io.cs2 != NULL) {
        SPI_Set_CS_Pin_State(cs2_gpio_mode_pin, cs2_gpio_port, CLR);
      }
    }
#endif
#endif
    spi->reg->IMR |= (RXUIM | RXOIM | RXFIM);
    if ((spi->instance_mode == SPI_MASTER_MODE) || (spi->instance_mode == SPI_ULP_MASTER_MODE)) {
      *(volatile uint8_t *)(&spi->reg->DR) = DUMMY_DATA; // dummy data
      while (spi->reg->SR & BIT(0))
        ;
    }
  }
  return ARM_DRIVER_OK;
}

#ifdef SSI_DUAL_QUAD_COMPONENT
/**
 * @fn          int32_t SPI_Receive_Command_Data(void *data, uint32_t num, uint32_t instruction, uint32_t address, 
 *                                        uint32_t wait_cycles, const SPI_RESOURCES *spi, UDMA_RESOURCES *udma,
 *                                        UDMA_Channel_Info *chnl_info, RSI_UDMA_HANDLE_T udmaHandle)
 * @brief       Advanced SPI data reception with instruction, address, and wait phases
 * @param[out]  data       : Pointer to data buffer for received data
 * @param[in]   num        : Number of data items to receive
 * @param[in]   instruction: Instruction value for command phase
 * @param[in]   address    : Address value for command phase
 * @param[in]   wait_cycles: Number of dummy/wait cycles between command and data phase
 * @param[in]   spi        : Pointer to SPI resources
 * @param[in]   udma       : Pointer to UDMA resources (used for DMA transfers)
 * @param[in]   chnl_info  : Pointer to UDMA channel information
 * @param[in]   udmaHandle : UDMA handle for the transfer
 * @return      int32_t    : ARM_DRIVER_OK on success, error code otherwise
 */
int32_t SPI_Receive_Command_Data(void *data,
                                 uint32_t num,
                                 uint32_t instruction,
                                 uint32_t address,
                                 uint32_t wait_cycles,
                                 const SPI_RESOURCES *spi,
                                 UDMA_RESOURCES *udma,
                                 UDMA_Channel_Info *chnl_info,
                                 RSI_UDMA_HANDLE_T udmaHandle)
{
  //Added to suppress unused variable warning
  (void)udma;
  (void)udmaHandle;
  (void)chnl_info;
  int32_t status;

  uint16_t data_width;

  if ((num == 0U) || (data == NULL)) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }
  if (!(SPI_CONFIGURED & spi->info->state)) {
    return ARM_DRIVER_ERROR;
  }
  if (spi->info->status.busy) {
    return ARM_DRIVER_ERROR_BUSY;
  }
  // Set the mode fault event if slave select is deactivated in master/ulp_master instance mode.
  if (SPI_Set_Mode_Fault_Event(spi)) {
    return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  spi->info->status.mode_fault = 0U;
  spi->info->status.busy       = 1U;
  spi->info->status.data_lost  = 0U;
  spi->xfer->rx_buf            = (uint8_t *)data;
  spi->xfer->rx_cnt            = 0U;
  spi->xfer->tx_cnt            = 0U;
  spi->xfer->tx_buf            = NULL;

  if (spi->reg->CTRLR0_b.SPI_FRF == STANDARD_SPI_FORMAT) {
    spi->reg->CTRLR0_b.TMOD = 3; //RECEIVE_ONLY;
  } else {
    spi->reg->CTRLR0_b.TMOD = RECEIVE_ONLY;
  }
  data_width = spi->reg->CTRLR0_b.DFS_32;

  // Set wait/dummy cycles if supported (optional, hardware dependent)
  if (wait_cycles) {
    spi->reg->SPI_CTRLR0_b.WAIT_CYCLES = wait_cycles;
  }

  if (num != 0) {
    SSI0->CTRLR1_b.NDF = num - 1;
  }

  if (spi->rx_dma != NULL) {
    status = SPI_Configure_Dma_Transfer(spi, num, data_width, RECEIVE_ONLY);
    if (status == ARM_DRIVER_ERROR) {
      return status;
    }

    if (spi->reg->CTRLR0_b.SPI_FRF == STANDARD_SPI_FORMAT) {
      SPI_Command_Send(spi, instruction, address);
    }

    sl_si91x_dma_channel_enable(DMA_INSTANCE, spi->rx_dma->channel + 1);
    sl_si91x_dma_enable(DMA_INSTANCE);
    if (spi->reg->CTRLR0_b.SPI_FRF != STANDARD_SPI_FORMAT) {
      // Send command to SPI peripheral
      SPI_Command_Send(spi, instruction, address);
    }
  } else {
    // non-DMA mode not supported for dual and quad modes
    return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  return ARM_DRIVER_OK;
}
#endif

/**
 * @fn          void SPI_UDMA_Tx_Event(uint32_t event, uint8_t dmaCh, SPI_RESOURCES *spi)
 * @brief        SPI UDMA transfer event Handler
 * @param[in]    event       : UDMA Event
 * @param[in]    dmaCh       : UDMA channel
 * @param[in]    spi        : Pointer to the SPI resources
 * @return       none
 */
void SPI_UDMA_Tx_Event(uint32_t event, uint8_t dmaCh, SPI_RESOURCES *spi)
{
  (void)dmaCh;
  uint32_t status_reg  = 0;
  uint32_t ssi_event   = 0;
  uint8_t ssi_instance = 0;
  // Validate the SSI instance that triggered this event
  if (spi->reg == SSI0) {
    // Assigning instance number to the callback variable
    ssi_instance = SSI_MASTER_INSTANCE;
  } else if (spi->reg == SSISlave) {
    // Assigning instance number to the callback variable
    ssi_instance = SSI_SLAVE_INSTANCE;
  } else if (spi->reg == SSI2) {
    // Assigning instance number to the callback variable
    ssi_instance = SSI_ULP_MASTER_INSTANCE;
  }
  switch (event) {
    case UDMA_EVENT_XFER_DONE:
      // Update TX buffer info
      spi->xfer->tx_cnt = spi->xfer->num;
      // Waiting till the busy flag is cleared
      while (spi->reg->SR & BIT(0))
        ;
      // Clear error status by reading the register
      status_reg             = spi->reg->SR;
      spi->info->status.busy = 0U;
      (void)status_reg;
      // Appending the instance value in the callback event variable
      ssi_event = ARM_SPI_EVENT_TRANSFER_COMPLETE | (ssi_instance << SSI_INSTANCE_BIT);
      if ((spi->info->cb_event != NULL) && (spi->xfer->rx_buf == NULL)) {
        spi->info->cb_event(ssi_event);
      }
      break;
    case UDMA_EVENT_ERROR:
      break;
  }
}
/**
 * @fn          void SPI_UDMA_Rx_Event(uint32_t event, uint8_t dmaCh, SPI_RESOURCES *spi)
 * @brief        SPI UDMA transfer event Handler
 * @param[in]    event       : UDMA Event
 * @param[in]    dmaCh       : UDMA channel
 * @param[in]    spi        : Pointer to the SPI resources
 * @return       none
 */
void SPI_UDMA_Rx_Event(uint32_t event, uint8_t dmaCh, SPI_RESOURCES *spi)
{
  (void)dmaCh;
  uint32_t status_reg  = 0;
  uint32_t ssi_event   = 0;
  uint8_t ssi_instance = 0;
  // Validate the SSI instance that triggered this event
  if (spi->reg == SSI0) {
    // Assigning instance number to the callback variable
    ssi_instance = SSI_MASTER_INSTANCE;
  } else if (spi->reg == SSISlave) {
    // Assigning instance number to the callback variable
    ssi_instance = SSI_SLAVE_INSTANCE;
  } else if (spi->reg == SSI2) {
    // Assigning instance number to the callback variable
    ssi_instance = SSI_ULP_MASTER_INSTANCE;
  }
  switch (event) {
    case UDMA_EVENT_XFER_DONE:
      spi->xfer->rx_cnt      = spi->xfer->num;
      spi->info->status.busy = 0U;
      if ((spi->instance_mode == SPI_MASTER_MODE) || (spi->instance_mode == SPI_ULP_MASTER_MODE)) {
        // Disables the slave after the transfer is completed
        spi->reg->SER_b.SER &= ~(1 << ssi_slave_number);
      }
      // Clear error status by reading the register
      status_reg = spi->reg->SR;
      (void)status_reg;
      // Appending the instance value in the callback event variable
      ssi_event = ARM_SPI_EVENT_TRANSFER_COMPLETE | (ssi_instance << SSI_INSTANCE_BIT);
      break;
    case UDMA_EVENT_ERROR:
      break;
  }
  if (spi->info->cb_event != NULL) {
    spi->info->cb_event(ssi_event);
  }
}

/**
 * @fn          void SPI_Clear_SSI_Enable_State(const SPI_RESOURCES *spi)
 * @brief       Write into SSI Enable Register
 * @param[in]   spi        : Pointer to the SPI resources
 * @return      none
 */
void SPI_Clear_SSI_Enable_State(const SPI_RESOURCES *spi)
{
  // Writing 0 in SSIENR shall flush both Tx and Rx FIFO buffers
  spi->reg->SSIENR = SSI_DISABLE;
}

/**
 * @fn          void SPI_Slave_Set_CS_Init_State(const SPI_RESOURCES *spi)
 * @brief       Set CS pin init state to high
 * @param[in]   spi        : Pointer to the SPI resources
 * @return      none
 */
void SPI_Slave_Set_CS_Init_State(const SPI_RESOURCES *spi)
{
  // CS0 pin shall be pulled up.
  RSI_EGPIO_PadDriverDisableState(spi->io.cs0->pin, Pullup);
}

/**
 * @fn          void SPI_Convert_Data_Width_To_Bytes(uint16_t data_width)
 * @brief       Update the data width and number of bytes in global static variable
 * @param[in]   data_width        : Number of data-bits configured
 * @return      none
 */
static void SPI_Convert_Data_Width_To_Bytes(uint16_t data_width)
{
  if (data_width <= (8U - 1U)) {
    // For 8-bit data frame number of bytes is 1
    data_width_in_bytes = BYTES_FOR_8_DATA_WIDTH;
  } else if (data_width <= (16U - 1U)) {
    // For 16-bit data frame number of bytes is 2
    data_width_in_bytes = BYTES_FOR_16_DATA_WIDTH;
  } else {
    // For 32-bit data frame number of bytes is 4
    data_width_in_bytes = BYTES_FOR_32_DATA_WIDTH;
  }
}

/**
 * @fn          void SPI_Set_Mode_Fault_Event(const SPI_RESOURCES *spi)
 * @brief       Set the callback event as mode fault if Slave enable register is not enabled.
 * @param[in]   spi        : Pointer to the SPI resources
 * @return      none
 */
static sl_status_t SPI_Set_Mode_Fault_Event(const SPI_RESOURCES *spi)
{
  sl_status_t status = SL_STATUS_OK;
  uint32_t event     = 0;
  if ((spi->instance_mode == SPI_MASTER_MODE) || (spi->instance_mode == SPI_ULP_MASTER_MODE)) {
    // Condition to check slave enable register if no slave enabled set the mode fault event.
    if (spi->reg->SER == SER_DISABLED) {
      spi->info->status.mode_fault = MODE_FAULT_STATUS_SET;
      status                       = SL_STATUS_FAIL;
      event |= ARM_SPI_EVENT_MODE_FAULT;
      if (event && spi->info->cb_event) {
        uint8_t ssi_instance = 0;
        // Validate the SSI instance that triggered this event
        if (spi->reg == SSI0) {
          // Assigning instance number to the callback variable
          ssi_instance = SSI_MASTER_INSTANCE;
        } else if (spi->reg == SSI2) {
          // Assigning instance number to the callback variable
          ssi_instance = SSI_ULP_MASTER_INSTANCE;
        }
        // Appending the instance value in the callback event variable
        event |= (ssi_instance << SSI_INSTANCE_BIT);
        spi->info->cb_event(event);
        if (spi->info->status.mode_fault) {
          spi->info->status.mode_fault = MODE_FAULT_STATUS_CLR;
        }
      }
    }
  }
  return status;
}
#if !(defined(SSI_ULP_MASTER_RX_DMA_Instance) && (SSI_ULP_MASTER_RX_DMA_Instance == 1))
#ifndef SI917_MEMLCD
/**
 * @fn          static sl_status_t SPI_Configure_CS_Pin(const SPI_PIN *cs_pin, uint8_t *cs_gpio_mode_pin, uint8_t *cs_gpio_port)
 * @brief       Configure the CS pin
 * @param[in]   cs_pin            : Pointer to the CS pin configuration
 * @param[in]   cs_gpio_mode_pin  : Pointer to the CS GPIO mode pin
 * @param[in]   cs_gpio_port      : Pointer to the CS GPIO port
 * @return      sl_status_t       : Status of the configuration
 */
static sl_status_t SPI_Configure_CS_Pin(const SPI_PIN *cs_pin, uint8_t *cs_gpio_mode_pin, uint8_t *cs_gpio_port)
{
  sl_si91x_gpio_pin_config_t sl_gpio_pin_config;
  sl_status_t status;

  if (cs_pin == NULL) {
    return SL_STATUS_FAIL;
  }

  if (cs_pin->pin > 64) {
    sl_gpio_pin_config.port_pin.pin  = (cs_pin->pin - 64);
    sl_gpio_pin_config.port_pin.port = SL_GPIO_ULP_PORT;
  } else {
    sl_gpio_pin_config.port_pin.pin  = cs_pin->pin;
    sl_gpio_pin_config.port_pin.port = SL_GPIO_PORT_A;
  }

  sl_gpio_pin_config.direction = GPIO_OUTPUT;

  // Configure CS pin using GPIO driver pin configuration API
  status = sl_gpio_set_configuration(sl_gpio_pin_config);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Set CS line as high by default
  *cs_gpio_mode_pin = sl_gpio_pin_config.port_pin.pin;
  *cs_gpio_port     = sl_gpio_pin_config.port_pin.port;

  if (*cs_gpio_port == SL_GPIO_ULP_PORT) {
    ULP_GPIO->PIN_CONFIG[*cs_gpio_mode_pin].BIT_LOAD_REG = SET;
    while (!(ULP_GPIO->PIN_CONFIG[*cs_gpio_mode_pin].BIT_LOAD_REG == SET))
      ;
  } else {
    GPIO->PIN_CONFIG[*cs_gpio_mode_pin].BIT_LOAD_REG = SET;
    while (!(GPIO->PIN_CONFIG[*cs_gpio_mode_pin].BIT_LOAD_REG == SET))
      ;
  }

  return SL_STATUS_OK;
}

/**
 * @fn          static sl_status_t SPI_Set_CS_Pin_State(uint8_t cs_gpio_mode_pin, uint8_t cs_gpio_port, uint8_t state)
 * @brief       Set the state of the CS pin
 * @param[in]   cs_gpio_mode_pin  : CS GPIO mode pin
 * @param[in]   cs_gpio_port      : CS GPIO port
 * @param[in]   state             : State to set
 * @return      sl_status_t       : Status of the operation
 */
static sl_status_t SPI_Set_CS_Pin_State(uint8_t cs_gpio_mode_pin, uint8_t cs_gpio_port, uint8_t state)
{
  if (cs_gpio_port == SL_GPIO_ULP_PORT) {
    ULP_GPIO->PIN_CONFIG[cs_gpio_mode_pin].BIT_LOAD_REG = state;
    while (!(ULP_GPIO->PIN_CONFIG[cs_gpio_mode_pin].BIT_LOAD_REG == state))
      ;
  } else {
    GPIO->PIN_CONFIG[cs_gpio_mode_pin].BIT_LOAD_REG = state;
    while (!(GPIO->PIN_CONFIG[cs_gpio_mode_pin].BIT_LOAD_REG == state))
      ;
  }
  return SL_STATUS_OK;
}
#endif
#endif
/** @} */
