/******************************************************************************
* @file  rsi_gspi.c
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

// Include Files

#include "rsi_ccp_user_config.h"

#ifndef GSPI_ROMDRIVER_PRESENT
#include "rsi_rom_clks.h"
#include "rsi_rom_egpio.h"
#include "GSPI.h"
#ifndef SL_SI91X_GSPI_DMA
#include "rsi_rom_udma_wrapper.h"
#endif
#ifdef SL_SI91X_GSPI_DMA
#include "sl_si91x_dma.h"
#include "rsi_gspi.h"
#define DMA_INSTANCE 0
#endif
#include "rsi_gspi.h"

#define MAX_BAUDRATE_FOR_DYNAMIC_CLOCK   116000000 // Maximum baudrate for dynamic clock division factor
#define MAX_BAUDRATE_FOR_POS_EDGE_SAMPLE 40000000  // Maximum baudrate for positive edge sample
#define STATIC_CLOCK_DIV_FACTOR          1         // Static clock divison factor
#define HALF_CLOCK_DIV_FACTOR            2         // To make the clock division factor half
#define DUMMY_DATA                       (0x5AA5)
#define MAX_DATA_WIDTH                   16 // Maximum data width gspi supports
#define DATA_WIDTH_8                     8  // Data width 8 for differentiating DMA Transfers
#define DATA_WIDTH_16                    16 // Data width 16 for differentiating DMA Transfers

typedef uint32_t __attribute__((__may_alias__)) aliased_uint32_t;

static uint8_t data_width_in_bytes = 0; // variable to store data width in bytes for current transfer
static void GSPI_Convert_Data_Width_To_Bytes(uint16_t data_width);
/*==============================================*/
/**
 * @fn          int32_t GSPI_Initialize(ARM_SPI_SignalEvent_t cb_event,
 *                       const GSPI_RESOURCES *gspi,
 *                       UDMA_RESOURCES *udma,
 *                       RSI_UDMA_DESC_T *UDMA_Table,
 *                       RSI_UDMA_HANDLE_T *udmaHandle,
 *                       uint32_t *mem)
 * @brief		     This API is used to Initialize the GSPI interface.
 * note: By default GSPI initialize, transfer, send and receive APIs use rsi_udma_wrapper.c
 * drivers. SL_DMA driver can be enabled by defining SL_SI91X_GSPI_DMA to 1.
 * rsi_udma_wrapper drivers can be executed from ROM. SL_DMA drivers cannot be executed
 * from ROM.
 * @param[in]	   cb_event    : Pointer to the ARM_SPI_SignalEvent_t
 * @param[in]	   gspi        : Pointer to the GSPI resources
 * @param[in]    udma        : Pointer to the UDMA resources
 * @param[in]    UDMA_Table  : Pointer to the RSI_UDMA_DESC_T
 * @param[in]    mem         : Pointer to the memory Buffer
 * @return 		   excecution status
 */
int32_t GSPI_Initialize(ARM_SPI_SignalEvent_t cb_event,
                        const GSPI_RESOURCES *gspi,
                        UDMA_RESOURCES *udma,
                        RSI_UDMA_DESC_T *UDMA_Table,
                        RSI_UDMA_HANDLE_T *udmaHandle,
                        uint32_t *mem)
{
#ifdef SL_SI91X_GSPI_DMA
  //Added to suppress unused variable warning
  (void)udma;
  (void)udmaHandle;
  (void)UDMA_Table;
  (void)mem;
#endif
#if defined(SLI_SI917)
  RSI_PS_M4ssPeriPowerUp(M4SS_PWRGATE_ULP_EFUSE_PERI);
#else
  RSI_PS_M4ssPeriPowerUp(M4SS_PWRGATE_ULP_PERI2);
#endif

  if (gspi->info->state & SPI_INITIALIZED) {
    return ARM_DRIVER_OK;
  }

  // Initialize SPI Run-Time Resources
  gspi->info->cb_event          = cb_event;
  gspi->info->status.busy       = 0U;
  gspi->info->status.data_lost  = 0U;
  gspi->info->status.mode_fault = 0U;

  // Clear transfer information
  memset(gspi->xfer, 0, sizeof(GSPI_TRANSFER_INFO));

  gspi->info->state = SPI_INITIALIZED;

  // Configure the M4 Clock
  RSI_CLK_PeripheralClkEnable3(M4CLK, M4_SOC_CLK_FOR_OTHER_ENABLE);

  // Configure the GSPI Clocks
  RSI_CLK_GspiClkConfig(M4CLK, gspi->clock_source);

  // Pin Mux
  if ((gspi->reg == GSPI0)) {
    //Configure clock pin
    if (gspi->io.clock->pin > 63) {
      RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(gspi->io.clock->pin - 64));
      RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(gspi->io.clock->pin - 64), 6);
    }
    if (gspi->io.clock->pad_sel != 0) {
      RSI_EGPIO_PadSelectionEnable(gspi->io.clock->pad_sel);
    }
    if (gspi->io.clock->pin >= 25 && gspi->io.clock->pin <= 30) {
      RSI_EGPIO_HostPadsGpioModeEnable(gspi->io.clock->pin);
    }
    RSI_EGPIO_PadReceiverEnable(gspi->io.clock->pin);
    RSI_EGPIO_SetPinMux(EGPIO, gspi->io.clock->port, gspi->io.clock->pin, gspi->io.clock->mode);

    if (gspi->io.cs0 != NULL) {
      //configure cs0 pin
      if (gspi->io.cs0->pin > 63) {
        RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(gspi->io.cs0->pin - 64));
        RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(gspi->io.cs0->pin - 64), 6);
      }
      if (gspi->io.cs0->pad_sel != 0) {
        RSI_EGPIO_PadSelectionEnable(gspi->io.cs0->pad_sel);
      }
      if (gspi->io.cs0->pin >= 25 && gspi->io.cs0->pin <= 30) {
        RSI_EGPIO_HostPadsGpioModeEnable(gspi->io.cs0->pin);
      }
      RSI_EGPIO_PadReceiverEnable(gspi->io.cs0->pin);
      RSI_EGPIO_SetPinMux(EGPIO, gspi->io.cs0->port, gspi->io.cs0->pin, gspi->io.cs0->mode);
    }
    if (gspi->io.cs1 != NULL) {
      //configure cs1 pin
      if (gspi->io.cs1->pin > 63) {
        RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(gspi->io.cs1->pin - 64));
        RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(gspi->io.cs1->pin - 64), 6);
      }
      if (gspi->io.cs1->pad_sel != 0) {
        RSI_EGPIO_PadSelectionEnable(gspi->io.cs1->pad_sel);
      }
      if (gspi->io.cs1->pin >= 25 && gspi->io.cs1->pin <= 30) {
        RSI_EGPIO_HostPadsGpioModeEnable(gspi->io.cs1->pin);
      }
      RSI_EGPIO_PadReceiverEnable(gspi->io.cs1->pin);
      RSI_EGPIO_SetPinMux(EGPIO, gspi->io.cs1->port, gspi->io.cs1->pin, gspi->io.cs1->mode);
    }

    if (gspi->io.cs2 != NULL) {
      //configure cs2 pin
      if (gspi->io.cs2->pin > 63) {
        RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(gspi->io.cs2->pin - 64));
        RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(gspi->io.cs2->pin - 64), 6);
      }
      if (gspi->io.cs2->pad_sel != 0) {
        RSI_EGPIO_PadSelectionEnable(gspi->io.cs2->pad_sel);
      }
      if (gspi->io.cs2->pin >= 25 && gspi->io.cs2->pin <= 30) {
        RSI_EGPIO_HostPadsGpioModeEnable(gspi->io.cs2->pin);
      }
      RSI_EGPIO_PadReceiverEnable(gspi->io.cs2->pin);
      RSI_EGPIO_SetPinMux(EGPIO, gspi->io.cs2->port, gspi->io.cs2->pin, gspi->io.cs2->mode);
    }

    //configure MOSI pin
    if (gspi->io.mosi->pin > 63) {
      RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(gspi->io.mosi->pin - 64));
      RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(gspi->io.mosi->pin - 64), 6);
    }
    if (gspi->io.mosi->pad_sel != 0) {
      RSI_EGPIO_PadSelectionEnable(gspi->io.mosi->pad_sel);
    }
    if (gspi->io.mosi->pin >= 25 && gspi->io.mosi->pin <= 30) {
      RSI_EGPIO_HostPadsGpioModeEnable(gspi->io.mosi->pin);
    }
    RSI_EGPIO_PadReceiverEnable(gspi->io.mosi->pin);
    RSI_EGPIO_SetPinMux(EGPIO, gspi->io.mosi->port, gspi->io.mosi->pin, gspi->io.mosi->mode);

    //configure MISO pin
    if (gspi->io.miso->pin > 63) {
      RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(gspi->io.miso->pin - 64));
      RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(gspi->io.miso->pin - 64), 6);
    }
    if (gspi->io.miso->pad_sel != 0) {
      RSI_EGPIO_PadSelectionEnable(gspi->io.miso->pad_sel);
    }
    if (gspi->io.miso->pin >= 25 && gspi->io.miso->pin <= 30) {
      RSI_EGPIO_HostPadsGpioModeEnable(gspi->io.miso->pin);
    }
    RSI_EGPIO_PadReceiverEnable(gspi->io.miso->pin);
    RSI_EGPIO_SetPinMux(EGPIO, gspi->io.miso->port, gspi->io.miso->pin, gspi->io.miso->mode);
  }
  // DMA Initialize
  if (gspi->tx_dma || gspi->rx_dma) {
    if ((gspi->rx_dma != NULL) || (gspi->tx_dma != NULL)) {
      // Enable DMA instance
      if ((gspi->reg == GSPI0)) {
#ifdef SL_SI91X_GSPI_DMA
        sl_dma_init_t dma_init;
        dma_init.dma_number = DMA_INSTANCE;
        if (sl_si91x_dma_init(&dma_init)) {
          return ARM_DRIVER_ERROR;
        }
        sl_si91x_dma_enable(DMA_INSTANCE);
#else
        //if using uart debug init in application this UDMA0_Uninitialize is power gating the uart peri power,then disabled this  ,
        // if other issue occures enable UDMA0_Uninitialize and try
        // DMA0
        *udmaHandle = UDMAx_Initialize(udma, UDMA_Table, udmaHandle, mem);
#endif
      }
    }
  }
  gspi->info->state = SPI_INITIALIZED;

  return ARM_DRIVER_OK;
}

/*==============================================*/
/**
 * @fn          int32_t GSPI_Uninitialize(const GSPI_RESOURCES *gspi, UDMA_RESOURCES *udma)
 * @brief		     This API is used to UnInitialize the GSPI interface.
 * @param[in]	   gspi        : Pointer to the GSPI resources
 * @param[in]    udma        : Pointer to the UDMA resources
 * @return 		   excecution status
 */
int32_t GSPI_Uninitialize(const GSPI_RESOURCES *gspi, UDMA_RESOURCES *udma)
{
#ifdef SL_SI91X_GSPI_DMA
  //Added to suppress unused variable warning
  (void)udma;
#endif
  // Clear SPI state
  gspi->info->state = 0U;

  if ((gspi->rx_dma != NULL) || (gspi->tx_dma != NULL)) {
    // Diasable DMA instance
    if ((gspi->reg == GSPI0)) {
#ifdef SL_SI91X_GSPI_DMA
      if (sl_si91x_dma_unregister_callbacks(DMA_INSTANCE,
                                            (gspi->tx_dma->channel + 1),
                                            SL_DMA_TRANSFER_DONE_CB | SL_DMA_ERROR_CB)) {
        return ARM_DRIVER_ERROR;
      }
      if (sl_si91x_dma_unregister_callbacks(DMA_INSTANCE,
                                            (gspi->rx_dma->channel + 1),
                                            SL_DMA_TRANSFER_DONE_CB | SL_DMA_ERROR_CB)) {
        return ARM_DRIVER_ERROR;
      }
#else
      UDMAx_Uninitialize(udma);
#endif
    }
  }
  return ARM_DRIVER_OK;
}

/*==============================================*/
/**
 * @fn          int32_t GSPI_PowerControl(ARM_POWER_STATE state, const GSPI_RESOURCES *gspi)
 * @brief		     This API is used control the GSPI interface power.
 * @param[in]    state       : Power State
 * @param[in]	   gspi        : Pointer to the GSPI resources
 * @return 		   excecution status
 */
int32_t GSPI_PowerControl(ARM_POWER_STATE state, const GSPI_RESOURCES *gspi)
{
  switch (state) {
    case ARM_POWER_OFF:

      NVIC_DisableIRQ(gspi->irq_num); // Disable SPI IRQ in NVIC

      // Reset SPI Run-Time Resources
      gspi->info->status.busy       = 0U;
      gspi->info->status.data_lost  = 0U;
      gspi->info->status.mode_fault = 0U;

      // Clear pending SPI interrupts in NVIC
      NVIC_ClearPendingIRQ(gspi->irq_num);

      // Clear transfer information
      memset(gspi->xfer, 0, sizeof(GSPI_TRANSFER_INFO));

      gspi->info->state &= (uint8_t)~SPI_POWERED; // SPI is not powered

// power down
#if defined(SLI_SI917)
      RSI_PS_M4ssPeriPowerDown(M4SS_PWRGATE_ULP_EFUSE_PERI);
#else
      RSI_PS_M4ssPeriPowerDown(M4SS_PWRGATE_ULP_PERI2);
#endif

      break;

    case ARM_POWER_FULL:

      if ((gspi->info->state & SPI_INITIALIZED) == 0U) {
        return ARM_DRIVER_ERROR;
      }
      if ((gspi->info->state & SPI_POWERED) != 0U) {
        return ARM_DRIVER_OK;
      }

      gspi->reg->GSPI_BUS_MODE_b.SPI_HIGH_PERFORMANCE_EN = 0x1;

      // Reset SPI Run-Time Resources
      gspi->info->status.busy       = 0U;
      gspi->info->status.data_lost  = 0U;
      gspi->info->status.mode_fault = 0U;
      gspi->info->state |= SPI_POWERED; // SPI is powered

      NVIC_ClearPendingIRQ(gspi->irq_num);
      NVIC_EnableIRQ(gspi->irq_num); // Enable SPI IRQ in NVIC
      break;

    case ARM_POWER_LOW:
    default:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  return ARM_DRIVER_OK;
}

/*==============================================*/
/**
 * @fn          void GSPI_Write_Dummy_Byte(const GSPI_RESOURCES *gspi)
 * @brief        This API is used to write GSPI 1byte dummy data into buffer.
 * @param[in]    gspi        : Pointer to the GSPI resources
 * @return       None
 */
void GSPI_Write_Dummy_Byte(const GSPI_RESOURCES *gspi)
{
  // Save original configuration values
  uint8_t originalManualWrite    = gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_WR;
  uint8_t originalFullDuplex     = gspi->reg->GSPI_CONFIG1_b.SPI_FULL_DUPLEX_EN;
  uint8_t originalEmptyThreshold = gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AEMPTY_THRLD;
  uint8_t originalFullThreshold  = gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AFULL_THRLD;

  GSPI0->GSPI_CONFIG1_b.GSPI_MANUAL_CSN = 0;
  while ((GSPI0->GSPI_STATUS & GSPI_MAN_CSN))
    ;

  // Set direction and high for each CS pin
  if (gspi->reg == GSPI0) {
    if (gspi->io.cs0 != NULL) {
      RSI_EGPIO_SetDir(EGPIO, gspi->io.cs0->port, gspi->io.cs0->pin, 0);    // Set CS pin direction as output
      RSI_EGPIO_SetPin(EGPIO, gspi->io.cs0->port, gspi->io.cs0->pin, 1);    // Set pin high
      RSI_EGPIO_SetPinMux(EGPIO, gspi->io.cs0->port, gspi->io.cs0->pin, 0); // set pin mode to mode0
      if (gspi->io.cs0->pin >= GPIO_MAX_PIN) {
        RSI_EGPIO_SetPinMux(EGPIO1,
                            gspi->io.cs0->port,
                            (uint8_t)(gspi->io.cs0->pin - GPIO_MAX_PIN),
                            0); // set pin mode to mode0
        RSI_EGPIO_SetDir(EGPIO1,
                         gspi->io.cs0->port,
                         (uint8_t)(gspi->io.cs0->pin - GPIO_MAX_PIN),
                         0); // Set CS pin direction as output
        RSI_EGPIO_SetPin(EGPIO1, gspi->io.cs0->port, (uint8_t)(gspi->io.cs0->pin - GPIO_MAX_PIN), 1); // Set pin high
      }
    }
  }

  gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_WR     = ENABLE; // Write enable
  gspi->reg->GSPI_CONFIG1_b.SPI_FULL_DUPLEX_EN = ENABLE; // Enable Full duplex mode

  // Configure the FIFO thresholds in interrupt mode
  gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AEMPTY_THRLD = 0x1;
  gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AFULL_THRLD  = 0xC;
  // enable GSPI interrupt
  gspi->reg->GSPI_INTR_UNMASK |= GSPI_INTR_UNMASK_BIT;
  // Write dummy data into write fifo
  *(volatile aliased_uint32_t *)(gspi->reg->GSPI_WRITE_FIFO) = DUMMY_DATA;
  while (gspi->reg->GSPI_STATUS & GSPI_BUSY_F)
    ;
  uint8_t flush_data = *(volatile aliased_uint32_t *)(gspi->reg->GSPI_READ_FIFO);
  (void)flush_data;
  gspi->reg->GSPI_INTR_MASK |= (GSPI_INTR_MASK_BIT);
  GSPI0->GSPI_CONFIG1_b.GSPI_MANUAL_CSN = 1;
  while (!(GSPI0->GSPI_STATUS & GSPI_MAN_CSN))
    ;
  // Pin mux configuration after all operations
  if (gspi->reg == GSPI0) {
    if (gspi->io.cs0 != NULL) {
      // Set the pin mode to the stored default mode
      RSI_EGPIO_SetPinMux(EGPIO, gspi->io.cs0->port, gspi->io.cs0->pin, gspi->io.cs0->mode);
      if (gspi->io.cs0->pin >= GPIO_MAX_PIN) {
        // Set the pin mode to the stored default mode
        RSI_EGPIO_SetPinMux(EGPIO1,
                            gspi->io.cs0->port,
                            (uint8_t)(gspi->io.cs0->pin - GPIO_MAX_PIN),
                            gspi->io.cs0->mode);
      }
    }
  }
  // Restore original configuration values
  gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_WR       = originalManualWrite;
  gspi->reg->GSPI_CONFIG1_b.SPI_FULL_DUPLEX_EN   = originalFullDuplex;
  gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AEMPTY_THRLD = originalEmptyThreshold;
  gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AFULL_THRLD  = originalFullThreshold;
}

/*==============================================*/
/**
 * @fn          int32_t GSPI_Control(uint32_t control, uint32_t arg, const GSPI_RESOURCES *gspi, uint32_t base_clock)
 * @brief		     This API is used control the GSPI interface.
 * @param[in]    control     : Operation
 * @param[in]    arg         : Argument of operation (optional)
 * @param[in]	   gspi        : Pointer to the GSPI resources
 * @param[in]    base_clock  : GSPI clock
 * @return 		   excecution status
 */
int32_t GSPI_Control(uint32_t control,
                     uint32_t arg,
                     const GSPI_RESOURCES *gspi,
                     uint32_t base_clock,
                     uint8_t slavenumber)
{
  uint32_t clk_div_factor = 0;
  uint32_t val            = 0;
  uint16_t data_bits      = 0;
  uint8_t cs_pin          = 0;

  if (!(gspi->info->state & SPI_POWERED)) {
    return ARM_DRIVER_ERROR;
  }
  if ((gspi->reg == GSPI0) && ((control & ARM_SPI_CONTROL_Msk) == ARM_SPI_MODE_SLAVE)) {
    return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  if ((control & ARM_SPI_CONTROL_Msk) == ARM_SPI_ABORT_TRANSFER) {
    memset(gspi->xfer, 0, sizeof(GSPI_TRANSFER_INFO));
    gspi->info->status.busy = 0U;
    return ARM_DRIVER_OK;
  }
  if (gspi->info->status.busy) {
    return ARM_DRIVER_ERROR_BUSY;
  }
  switch (control & ARM_SPI_CONTROL_Msk) {
    case ARM_SPI_MODE_INACTIVE:
      gspi->info->mode &= ~ARM_SPI_CONTROL_Msk;
      gspi->info->mode |= ARM_SPI_MODE_INACTIVE;
      gspi->info->state &= (uint8_t)~SPI_CONFIGURED;
      return ARM_DRIVER_OK;

    case ARM_SPI_MODE_MASTER:
      gspi->info->mode &= ~ARM_SPI_CONTROL_Msk;
      gspi->info->mode |= ARM_SPI_MODE_MASTER;
      gspi->info->state |= SPI_CONFIGURED;
      gspi->info->mode |= ARM_SPI_MODE_MASTER;
      goto set_speed;

    case ARM_SPI_MODE_SLAVE:
      return ARM_SPI_ERROR_MODE;

    case ARM_SPI_MODE_MASTER_SIMPLEX:
      return ARM_SPI_ERROR_MODE;

    case ARM_SPI_MODE_SLAVE_SIMPLEX:
      return ARM_SPI_ERROR_MODE;

    case ARM_SPI_SET_BUS_SPEED:
set_speed:
      if (arg > MAX_BAUDRATE_FOR_DYNAMIC_CLOCK) {
        // Clock division factor should be 1 for baudrate more than 110 MHz
        clk_div_factor = STATIC_CLOCK_DIV_FACTOR;
      } else {
        clk_div_factor = (base_clock / arg);
        // NOTE : division factor is multiples of 2
        clk_div_factor = (clk_div_factor / HALF_CLOCK_DIV_FACTOR);
      }

      if (clk_div_factor < 1) {
        gspi->reg->GSPI_CLK_CONFIG_b.GSPI_CLK_EN = 0x1;

        gspi->reg->GSPI_CLK_CONFIG_b.GSPI_CLK_SYNC = 0x1;
      }
      if (arg > MAX_BAUDRATE_FOR_POS_EDGE_SAMPLE) {
        // Enabling negative edge sampling for baudrate more than 40 MHz
        gspi->reg->GSPI_BUS_MODE_b.GSPI_DATA_SAMPLE_EDGE = ENABLE;
      }
      // Update the clock rate to hardware
      gspi->reg->GSPI_CLK_DIV_b.GSPI_CLK_DIV_FACTOR = (uint8_t)clk_div_factor;
      break;

    case ARM_SPI_GET_BUS_SPEED: // Get Bus Speed in bps
      return gspi->reg->GSPI_CLK_DIV_b.GSPI_CLK_DIV_FACTOR;

    case ARM_SPI_SET_DEFAULT_TX_VALUE: // Set default Transmit value; arg = value
      gspi->xfer->def_val = (uint16_t)(arg & 0xFFFF);
      return ARM_DRIVER_OK;

    case ARM_SPI_CONTROL_SS:

      if (((gspi->info->mode & ARM_SPI_CONTROL_Msk) != ARM_SPI_MODE_MASTER)) {
        return ARM_DRIVER_ERROR;
      }
      val = (gspi->info->mode & ARM_SPI_SS_MASTER_MODE_Msk);

      if (arg == ARM_SPI_SS_INACTIVE) {
        if (val == ARM_SPI_SS_MASTER_HW_OUTPUT) {
          // Assert selected CS line
          gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_CSN_SELECT = (uint32_t)(slavenumber & 0x03);
          gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_CSN        = 0x1;
          while (!(gspi->reg->GSPI_STATUS & GSPI_MAN_CSN))
            ;
        } else {

          switch (slavenumber) {
            case GSPI_CS0:
              if (gspi->io.cs0 != NULL) {
                cs_pin = gspi->io.cs0->pin;
              } else {
                return ARM_DRIVER_ERROR_PARAMETER;
              }
              break;
            case GSPI_CS1:
              if (gspi->io.cs1 != NULL) {
                cs_pin = gspi->io.cs1->pin;
              } else {
                return ARM_DRIVER_ERROR_PARAMETER;
              }
              break;
            case GSPI_CS2:
              if (gspi->io.cs2 != NULL) {
                cs_pin = gspi->io.cs2->pin;
              } else {
                return ARM_DRIVER_ERROR_PARAMETER;
              }
              break;
            default:
              return ARM_DRIVER_ERROR_PARAMETER;
              break;
          }
          RSI_EGPIO_SetPin(EGPIO, 0, cs_pin, 1);
          // Disabling the manual cs pin
          gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_CSN = 0;
          while ((gspi->reg->GSPI_STATUS & GSPI_MAN_CSN))
            ;
        }
      } else {
        if (val == ARM_SPI_SS_MASTER_HW_OUTPUT) {
          // Assert selected CS line
          gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_CSN_SELECT = (uint32_t)(slavenumber & 0x03);
          gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_CSN        = 0;
          while ((gspi->reg->GSPI_STATUS & GSPI_MAN_CSN))
            ;
        } else {
          switch (slavenumber) {
            case GSPI_CS0:
              if (gspi->io.cs0 != NULL) {
                cs_pin = gspi->io.cs0->pin;
              } else {
                return ARM_DRIVER_ERROR_PARAMETER;
              }
              break;
            case GSPI_CS1:
              if (gspi->io.cs1 != NULL) {
                cs_pin = gspi->io.cs1->pin;
              } else {
                return ARM_DRIVER_ERROR_PARAMETER;
              }
              break;
            case GSPI_CS2:
              if (gspi->io.cs1 != NULL) {
                cs_pin = gspi->io.cs2->pin;
              } else {
                return ARM_DRIVER_ERROR_PARAMETER;
              }
              break;
            default:
              return ARM_DRIVER_ERROR_PARAMETER;
              break;
          }
          RSI_EGPIO_SetPin(EGPIO, 0, cs_pin, 0);
        }
      }
      return ARM_DRIVER_OK;
  }

  if ((gspi->info->mode & ARM_SPI_CONTROL_Msk) == ARM_SPI_MODE_MASTER) {
    switch (control & ARM_SPI_SS_MASTER_MODE_Msk) {
      case ARM_SPI_SS_MASTER_UNUSED: // SPI Slave Select when Master: Not used (default)
                                     //unconfigure the slave selct pin
        RSI_EGPIO_SetPinMux(EGPIO, 0, cs_pin, 0);
        gspi->info->mode &= ~ARM_SPI_SS_MASTER_MODE_Msk;
        gspi->info->mode |= ARM_SPI_SS_MASTER_UNUSED;
        break;

      case ARM_SPI_SS_MASTER_HW_INPUT: // SPI Slave Select when Master: Hardware monitored Input
        gspi->info->mode &= ~ARM_SPI_SS_MASTER_MODE_Msk;
        return ARM_SPI_ERROR_SS_MODE;

      case ARM_SPI_SS_MASTER_SW: // SPI Slave Select when Master: Software controlled
        //configure Gpio pin output mode
        //configure cs pin
        //!pad selection,host pad gpio mode enables for host pad gpios are already pogrammed in initialization
        switch (slavenumber) {
          case GSPI_CS0:
            if (gspi->io.cs0 != NULL) {
              cs_pin = gspi->io.cs0->pin;
              if (gspi->io.cs0->pad_sel != 0) {
                RSI_EGPIO_PadSelectionEnable(gspi->io.cs0->pad_sel);
              }
            } else {
              return ARM_DRIVER_ERROR_PARAMETER;
            }
            break;
          case GSPI_CS1:
            if (gspi->io.cs1 != NULL) {
              cs_pin = gspi->io.cs1->pin;
              if (gspi->io.cs1->pad_sel != 0) {
                RSI_EGPIO_PadSelectionEnable(gspi->io.cs1->pad_sel);
              }
            } else {
              return ARM_DRIVER_ERROR_PARAMETER;
            }
            break;
          case GSPI_CS2:
            if (gspi->io.cs2 != NULL) {
              cs_pin = gspi->io.cs2->pin;
              if (gspi->io.cs2->pad_sel != 0) {
                RSI_EGPIO_PadSelectionEnable(gspi->io.cs2->pad_sel);
              }
            } else {
              return ARM_DRIVER_ERROR_PARAMETER;
            }
            break;
          default:
            return ARM_DRIVER_ERROR_PARAMETER;
            break;
        }
        if (cs_pin > 63) {
          RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(cs_pin - 64), 6);
        }
        RSI_EGPIO_SetPinMux(EGPIO, 0, cs_pin, 0); //gpio mode
        RSI_EGPIO_SetDir(EGPIO, 0, cs_pin, 0);    //set output direction
        RSI_EGPIO_SetPin(EGPIO, 0, cs_pin, 1);    //set pin value to 1
        gspi->info->mode |= ARM_SPI_SS_MASTER_SW;
        break;
      case ARM_SPI_SS_MASTER_HW_OUTPUT: // SPI Slave Select when Master: Hardware controlled Output
        gspi->info->mode &= ~ARM_SPI_SS_MASTER_MODE_Msk;
        if ((gspi->io.cs0->pin != (int)NULL) || (gspi->io.cs1->pin != (int)NULL) || (gspi->io.cs2->pin != (int)NULL)) {
          gspi->info->mode |= ARM_SPI_SS_MASTER_HW_OUTPUT;
        } else {
          return ARM_SPI_ERROR_SS_MODE;
        }
        break;
      default:
        break;
    }
  }
  // Frame format:
  switch (control & ARM_SPI_FRAME_FORMAT_Msk) {
    case ARM_SPI_CPOL0_CPHA0:
      gspi->reg->GSPI_BUS_MODE_b.GSPI_CLK_MODE_CSN0 = 0; //mode 0
      break;

    case ARM_SPI_CPOL0_CPHA1:
      return ARM_SPI_ERROR_FRAME_FORMAT;

    case ARM_SPI_CPOL1_CPHA0:
      return ARM_SPI_ERROR_FRAME_FORMAT;

    case ARM_SPI_CPOL1_CPHA1:
      gspi->reg->GSPI_BUS_MODE_b.GSPI_CLK_MODE_CSN0 = 0x1; //mode 3
      break;

    case ARM_SPI_TI_SSI:
      return ARM_SPI_ERROR_FRAME_FORMAT;

    case ARM_SPI_MICROWIRE:
      return ARM_SPI_ERROR_FRAME_FORMAT;

    default:
      return ARM_SPI_ERROR_FRAME_FORMAT;
  }
  // Swap the read data inside the GSPI controller it-self.
  gspi->reg->GSPI_CONFIG2 &= ~BIT(4);

  // Configure Number of Data Bits
  data_bits = ((control & ARM_SPI_DATA_BITS_Msk) >> ARM_SPI_DATA_BITS_Pos);
  if (data_bits > MAX_DATA_WIDTH) {
    return ARM_SPI_ERROR_DATA_BITS;
  } else {
    // Update the number of Data Bits
    gspi->reg->GSPI_WRITE_DATA2_b.GSPI_MANUAL_WRITE_DATA2 = (uint32_t)(data_bits & 0x0F);
  }

  return ARM_DRIVER_OK;
}

/*==============================================*/
/**
 * @fn          int32_t GSPI_Send(const void *data,
 *                               uint32_t num,
 *								const GSPI_RESOURCES *gspi,
 *								UDMA_RESOURCES *udma,
 *								DMA_Channel_Info *chnl_info,
 *								RSI_UDMA_HANDLE_T udmaHandle)
 * @brief		     This API Starts sending data to GSPI transmitter.
 * @param[in]    data        : Pointer to buffer with data to send to SPI transmitter
 * @param[in]    num         : Number of data items to send
 * @param[in]	   gspi        : Pointer to the GSPI resources
 * @param[in]	   udma        : Pointer to the UDMA resources
 * @param[in]	   CHNL_INFO   : Pointer to the UDMA channel info
 * @param[in]    udmaHandle  : Pointer to UDMA Handle
 * @return 		   excecution status
 */
int32_t GSPI_Send(const void *data,
                  uint32_t num,
                  const GSPI_RESOURCES *gspi,
                  UDMA_RESOURCES *udma,
                  UDMA_Channel_Info *chnl_info,
                  RSI_UDMA_HANDLE_T udmaHandle)
{
#ifdef SL_SI91X_GSPI_DMA
  //Added to suppress unused variable warning
  (void)udma;
  (void)udmaHandle;
  (void)chnl_info;
  sl_status_t status;
#else
  volatile int32_t stat = 0;
#endif
  RSI_UDMA_CHA_CONFIG_DATA_T control = { 0 };

  uint16_t data_bits;
  uint16_t data_16bit;

  if ((data == NULL) || (num == 0U)) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }
  if (!(gspi->info->state & SPI_CONFIGURED)) {
    return ARM_DRIVER_ERROR;
  }
  if (gspi->info->status.busy) {
    return ARM_DRIVER_ERROR_BUSY;
  }
  gspi->info->status.busy       = 1U;
  gspi->info->status.data_lost  = 0U;
  gspi->info->status.mode_fault = 0U;
  gspi->xfer->rx_buf            = NULL;
  gspi->xfer->rx_cnt            = 0U;
  gspi->xfer->tx_cnt            = 0U;
  gspi->xfer->tx_buf            = (uint8_t *)data;

  // Read the number of data bits; if the value is 0, it means 16 bits are valid
  data_bits = gspi->reg->GSPI_WRITE_DATA2_b.GSPI_MANUAL_WRITE_DATA2;
  if (data_bits == 0) {
    data_bits = DATA_WIDTH_16; // 0 indicates all 16 bits are valid
  }

  gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_WR      = ENABLE; //write enable
  gspi->reg->GSPI_WRITE_DATA2_b.USE_PREV_LENGTH = true;

  // Disable Full duplex mode while only Sending
  gspi->reg->GSPI_CONFIG1_b.SPI_FULL_DUPLEX_EN = DISABLE;

  if (gspi->tx_dma) {
    // Configure the FIFO thresholds
    gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AEMPTY_THRLD = (uint32_t)(gspi->threshold->gspi_aempty_threshold & 0x0F);
    gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AFULL_THRLD  = (uint32_t)(gspi->threshold->gspi_afull_threshold & 0x0F);

    control.transferType       = UDMA_MODE_BASIC;
    control.nextBurst          = 0;
    control.totalNumOfDMATrans = (num < 1024) ? (num - 1) : (0x03FF);
    control.rPower             = (uint32_t)(gspi->threshold->txdma_arb_size & 0x0F);
    control.srcProtCtrl        = 0x0;
    control.dstProtCtrl        = 0x0;
    control.dstInc             = DST_INC_NONE;
    if (data_bits <= DATA_WIDTH_8) {
      //8-bit data frame
      control.srcSize = SRC_SIZE_8;
      control.srcInc  = SRC_INC_8;
      control.dstSize = DST_SIZE_8;
    } else {
      //16-bit data frame
      control.srcSize = SRC_SIZE_16;
      control.srcInc  = SRC_INC_16;
      control.dstSize = DST_SIZE_16;
    }
#ifdef SL_SI91X_GSPI_DMA
    sl_dma_xfer_t dma_transfer_tx = { 0 };
    uint32_t channel              = gspi->tx_dma->channel + 1;
    uint32_t channel_priority     = gspi->tx_dma->chnl_cfg.channelPrioHigh;
    sl_dma_callback_t gspi_tx_callback;
    //Initialize sl_dma callback structure
    gspi_tx_callback.transfer_complete_cb = gspi_transfer_complete_callback;
    gspi_tx_callback.error_cb             = gspi_error_callback;
    //Initialize sl_dma transfer structure
    dma_transfer_tx.src_addr       = (uint32_t *)((uint32_t)(gspi->xfer->tx_buf));
    dma_transfer_tx.dest_addr      = (uint32_t *)((uint32_t) & (gspi->reg->GSPI_WRITE_FIFO));
    dma_transfer_tx.src_inc        = control.srcInc;
    dma_transfer_tx.dst_inc        = control.dstInc;
    dma_transfer_tx.xfer_size      = control.dstSize;
    dma_transfer_tx.transfer_count = num;
    dma_transfer_tx.transfer_type  = SL_DMA_MEMORY_TO_PERIPHERAL;
    dma_transfer_tx.dma_mode       = control.transferType;
    dma_transfer_tx.signal         = (uint8_t)gspi->tx_dma->chnl_cfg.periAck;

    //Allocate DMA channel for Tx
    status = sl_si91x_dma_allocate_channel(DMA_INSTANCE, &channel, channel_priority);
    if (status && (status != SL_STATUS_DMA_CHANNEL_ALLOCATED)) {
      return ARM_DRIVER_ERROR;
    }
    //Register transfer complete and error callback
    if (sl_si91x_dma_register_callbacks(DMA_INSTANCE, channel, &gspi_tx_callback)) {
      return ARM_DRIVER_ERROR;
    }
    //Configure the channel for DMA transfer
    if (sl_si91x_dma_transfer(DMA_INSTANCE, channel, &dma_transfer_tx)) {
      return ARM_DRIVER_ERROR;
    }
    sl_si91x_dma_channel_enable(DMA_INSTANCE, gspi->tx_dma->channel + 1);
#else
    // Initialize and start GSPI TX DMA Stream
    stat = UDMAx_ChannelConfigure(udma,
                                  gspi->tx_dma->channel,
                                  (uint32_t)(gspi->xfer->tx_buf),
                                  (uint32_t) & (gspi->reg->GSPI_WRITE_FIFO),
                                  num,
                                  control,
                                  &gspi->tx_dma->chnl_cfg,
                                  gspi->tx_dma->cb_event,
                                  chnl_info,
                                  udmaHandle);
    if (stat == -1) {
      return ARM_DRIVER_ERROR;
    }
    UDMAx_ChannelEnable(gspi->tx_dma->channel, udma, udmaHandle);
    UDMAx_DMAEnable(udma, udmaHandle);
#endif
  } else {
    // Converts the data width into number of bytes to send/receive at once
    GSPI_Convert_Data_Width_To_Bytes(data_bits);
    gspi->xfer->num = num * data_width_in_bytes;

    // Configure the FIFO thresholds in interrupt mode
    gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AEMPTY_THRLD = 0x1;
    gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AFULL_THRLD  = 0xC;
    // enable GSPI interrupt
    gspi->reg->GSPI_INTR_UNMASK |= GSPI_INTR_UNMASK_BIT;

    // write first index data
    if (data_width_in_bytes == 1) {
      *(volatile aliased_uint32_t *)(gspi->reg->GSPI_WRITE_FIFO) =
        (uint32_t) * (gspi->xfer->tx_buf + gspi->xfer->tx_cnt++);
    } else if (data_width_in_bytes == 2) {
      data_16bit = *(gspi->xfer->tx_buf + gspi->xfer->tx_cnt++);
      data_16bit |= (uint16_t)(*(gspi->xfer->tx_buf + gspi->xfer->tx_cnt++) << 8U);
      *(volatile aliased_uint32_t *)(gspi->reg->GSPI_WRITE_FIFO) = (uint32_t)data_16bit;
    }
    while (gspi->reg->GSPI_STATUS & GSPI_BUSY_F)
      ;
  }
  return ARM_DRIVER_OK;
}

/*==============================================*/
/**
 * @fn          int32_t GSPI_Receive( void *data,
 *                               uint32_t num,
 *								const GSPI_RESOURCES *gspi,
 *								UDMA_RESOURCES *udma,
 *								UDMA_Channel_Info *chnl_info,
 *								RSI_UDMA_HANDLE_T udmaHandle)
 * @brief		     Start receiving data from GSPI receiver.
 * @param[in]    data        : Pointer to buffer for data to receive from GSPI receiver
 * @param[in]    num         : Number of data items to receive
 * @param[in]	   gspi        : Pointer to the GSPI resources
 * @param[in]	   udma        : Pointer to the UDMA resources
 * @param[in]	   CHNL_INFO   : Pointer to the UDMA channel info
 * @param[in]    udmaHandle  : Pointer to UDMA Handle
 * @return 		   excecution status
 */
int32_t GSPI_Receive(void *data,
                     uint32_t num,
                     const GSPI_RESOURCES *gspi,
                     UDMA_RESOURCES *udma,
                     UDMA_Channel_Info *chnl_info,
                     RSI_UDMA_HANDLE_T udmaHandle)
{
#ifdef SL_SI91X_GSPI_DMA
  //Added to suppress unused variable warning
  (void)udma;
  (void)udmaHandle;
  (void)chnl_info;
  sl_status_t status;
#else
  volatile int32_t stat = 0;
#endif
  RSI_UDMA_CHA_CONFIG_DATA_T control = { 0 };
  uint32_t dummy_data                = 0x5AA5;
  uint16_t data_bits;

  if ((data == NULL) || (num == 0U)) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }
  if (!(gspi->info->state & SPI_CONFIGURED)) {
    return ARM_DRIVER_ERROR;
  }

  if (gspi->info->status.busy) {
    return ARM_DRIVER_ERROR_BUSY;
  }
  gspi->info->status.busy       = 1U;
  gspi->info->status.data_lost  = 0U;
  gspi->info->status.mode_fault = 0U;

  gspi->xfer->rx_buf = (uint8_t *)data;
  gspi->xfer->tx_buf = NULL;

  gspi->xfer->rx_cnt = 0U;
  gspi->xfer->tx_cnt = 0U;

  // Read the number of data bits; if the value is 0, it means 16 bits are valid
  data_bits = gspi->reg->GSPI_WRITE_DATA2_b.GSPI_MANUAL_WRITE_DATA2;
  if (data_bits == 0) {
    data_bits = DATA_WIDTH_16; // 0 indicates all 16 bits are valid
  }

  gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_WR      = ENABLE; //write enable
  gspi->reg->GSPI_WRITE_DATA2_b.USE_PREV_LENGTH = true;

  // Enable Full duplex mode
  gspi->reg->GSPI_CONFIG1_b.SPI_FULL_DUPLEX_EN = ENABLE;

  if ((gspi->rx_dma) || (gspi->tx_dma)) {
    // Configure the FIFO thresholds
    gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AEMPTY_THRLD = (uint32_t)(gspi->threshold->gspi_aempty_threshold & 0x0F);
    gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AFULL_THRLD  = (uint32_t)(gspi->threshold->gspi_afull_threshold & 0x0F);

    if ((gspi->tx_dma)) {
      control.transferType       = UDMA_MODE_BASIC;
      control.nextBurst          = 0;
      control.totalNumOfDMATrans = (num < 1024) ? (num - 1) : (0x03FF);
      control.rPower             = (uint32_t)(gspi->threshold->rxdma_arb_size & 0x0F);
      control.srcProtCtrl        = 0x0;
      control.dstProtCtrl        = 0x0;
      control.dstInc             = DST_INC_NONE;
      if (data_bits <= DATA_WIDTH_8) {
        //8-bit data frame
        control.srcSize = SRC_SIZE_8;
        control.srcInc  = SRC_INC_8;
        control.dstSize = DST_SIZE_8;
      } else {
        //16-bit data frame
        control.srcSize = SRC_SIZE_16;
        control.srcInc  = SRC_INC_16;
        control.dstSize = DST_SIZE_16;
      }
#ifdef SL_SI91X_GSPI_DMA
      sl_dma_xfer_t dma_transfer_tx = { 0 };
      uint32_t channel              = gspi->tx_dma->channel + 1;
      uint32_t channel_priority     = gspi->tx_dma->chnl_cfg.channelPrioHigh;
      sl_dma_callback_t gspi_tx_callback;
      //Initialize sl_dma callback structure
      gspi_tx_callback.transfer_complete_cb = gspi_transfer_complete_callback;
      gspi_tx_callback.error_cb             = gspi_error_callback;
      //Initialize sl_dma transfer structure
      dma_transfer_tx.src_addr       = (uint32_t *)((uint32_t)&dummy_data);
      dma_transfer_tx.dest_addr      = (uint32_t *)((uint32_t) & (gspi->reg->GSPI_WRITE_FIFO));
      dma_transfer_tx.src_inc        = control.srcInc;
      dma_transfer_tx.dst_inc        = control.dstInc;
      dma_transfer_tx.xfer_size      = control.dstSize;
      dma_transfer_tx.transfer_count = num;
      dma_transfer_tx.transfer_type  = SL_DMA_MEMORY_TO_PERIPHERAL;
      dma_transfer_tx.dma_mode       = control.transferType;
      dma_transfer_tx.signal         = (uint8_t)gspi->tx_dma->chnl_cfg.periAck;

      //Allocate DMA channel for Tx
      status = sl_si91x_dma_allocate_channel(DMA_INSTANCE, &channel, channel_priority);
      if (status && (status != SL_STATUS_DMA_CHANNEL_ALLOCATED)) {
        return ARM_DRIVER_ERROR;
      }
      //Register transfer complete and error callback
      if (sl_si91x_dma_register_callbacks(DMA_INSTANCE, channel, &gspi_tx_callback)) {
        return ARM_DRIVER_ERROR;
      }
      //Configure the channel for DMA transfer
      if (sl_si91x_dma_transfer(DMA_INSTANCE, channel, &dma_transfer_tx)) {
        return ARM_DRIVER_ERROR;
      }
#else
      // Initialize and start GSPI TX DMA Stream
      stat = UDMAx_ChannelConfigure(udma,
                                    gspi->tx_dma->channel,
                                    (dummy_data),
                                    (uint32_t) & (gspi->reg->GSPI_WRITE_FIFO),
                                    num,
                                    control,
                                    &gspi->tx_dma->chnl_cfg,
                                    gspi->tx_dma->cb_event,
                                    chnl_info,
                                    udmaHandle);
      if (stat == -1) {
        return ARM_DRIVER_ERROR;
      }
      UDMAx_ChannelEnable(gspi->tx_dma->channel, udma, udmaHandle);
#endif
    } else {
      return ARM_DRIVER_ERROR;
    }
    if ((gspi->rx_dma)) {
      control.transferType = UDMA_MODE_BASIC;
      control.nextBurst    = 0;
      if (num < 1024) {
        control.totalNumOfDMATrans = (uint32_t)((num - 1) & 0x03FF);
      } else {
        control.totalNumOfDMATrans = 0x3FF;
      }
      control.rPower      = ARBSIZE_1;
      control.srcProtCtrl = 0x0;
      control.dstProtCtrl = 0x0;
      control.srcInc      = SRC_INC_NONE;
      if (data_bits <= DATA_WIDTH_8) {
        //8-bit data frame
        control.srcSize = SRC_SIZE_8;
        control.dstSize = DST_SIZE_8;
        control.dstInc  = DST_INC_8;
      } else {
        // 16-bit data frame
        control.srcSize = SRC_SIZE_16;
        control.dstSize = DST_SIZE_16;
        control.dstInc  = DST_INC_16;
      }
#ifdef SL_SI91X_GSPI_DMA
      sl_dma_xfer_t dma_transfer_rx = { 0 };
      uint32_t channel              = gspi->rx_dma->channel + 1;
      uint32_t channel_priority     = gspi->rx_dma->chnl_cfg.channelPrioHigh;
      sl_dma_callback_t gspi_rx_callback;
      //Initialize sl_dma callback structure
      gspi_rx_callback.transfer_complete_cb = gspi_transfer_complete_callback;
      gspi_rx_callback.error_cb             = gspi_error_callback;
      //Initialize sl_dma transfer structure
      dma_transfer_rx.src_addr       = (uint32_t *)((uint32_t) & (gspi->reg->GSPI_READ_FIFO));
      dma_transfer_rx.dest_addr      = (uint32_t *)((uint32_t)(gspi->xfer->rx_buf));
      dma_transfer_rx.src_inc        = control.srcInc;
      dma_transfer_rx.dst_inc        = control.dstInc;
      dma_transfer_rx.xfer_size      = control.dstSize;
      dma_transfer_rx.transfer_count = num;
      dma_transfer_rx.transfer_type  = SL_DMA_PERIPHERAL_TO_MEMORY;
      dma_transfer_rx.dma_mode       = control.transferType;
      dma_transfer_rx.signal         = (uint8_t)gspi->rx_dma->chnl_cfg.periAck;

      //Allocate DMA channel for Rx
      status = sl_si91x_dma_allocate_channel(DMA_INSTANCE, &channel, channel_priority);
      if (status && (status != SL_STATUS_DMA_CHANNEL_ALLOCATED)) {
        return ARM_DRIVER_ERROR;
      }
      //Register transfer complete and error callback
      if (sl_si91x_dma_register_callbacks(DMA_INSTANCE, channel, &gspi_rx_callback)) {
        return ARM_DRIVER_ERROR;
      }
      //Configure the channel for DMA transfer
      if (sl_si91x_dma_transfer(DMA_INSTANCE, channel, &dma_transfer_rx)) {
        return ARM_DRIVER_ERROR;
      }
#else
      // Initialize and start GSPI RX DMA Stream
      stat = UDMAx_ChannelConfigure(udma,
                                    gspi->rx_dma->channel,
                                    (uint32_t) & (gspi->reg->GSPI_READ_FIFO),
                                    (uint32_t)(gspi->xfer->rx_buf),
                                    num,
                                    control,
                                    &gspi->rx_dma->chnl_cfg,
                                    gspi->rx_dma->cb_event,
                                    chnl_info,
                                    udmaHandle);
      if (stat == -1) {
        return ARM_DRIVER_ERROR;
      }
      UDMAx_ChannelEnable(gspi->rx_dma->channel, udma, udmaHandle);
#endif
      gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_RD = ENABLE;
    } else {
      return ARM_DRIVER_ERROR;
    }
#ifdef SL_SI91X_GSPI_DMA
    sl_si91x_dma_channel_enable(DMA_INSTANCE, gspi->tx_dma->channel + 1);
    sl_si91x_dma_channel_enable(DMA_INSTANCE, gspi->rx_dma->channel + 1);
#else
    UDMAx_DMAEnable(udma, udmaHandle);
#endif
  } else {
    // Converts the data width into number of bytes to send/receive at once
    GSPI_Convert_Data_Width_To_Bytes(data_bits);
    gspi->xfer->num = num * data_width_in_bytes;

    // Configure the FIFO thresholds in interrupt mode
    gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AEMPTY_THRLD = 0x1;
    gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AFULL_THRLD  = 0xC;
    // enable GSPI interrupt
    gspi->reg->GSPI_INTR_UNMASK |= GSPI_INTR_UNMASK_BIT;

    // write dummy for reading first index data
    if (data_width_in_bytes == 1) {
      *(volatile aliased_uint32_t *)(gspi->reg->GSPI_WRITE_FIFO) = (uint32_t)DUMMY_DATA;
    } else if (data_width_in_bytes == 2) {
      *(volatile aliased_uint32_t *)(gspi->reg->GSPI_WRITE_FIFO) = (uint32_t)DUMMY_DATA;
    }
    while (gspi->reg->GSPI_STATUS & GSPI_BUSY_F)
      ;
  }
  return ARM_DRIVER_OK;
}

/*==============================================*/
/**
 * @fn          int32_t GSPI_Transfer(const void *data_out,
 *								void *data_in,
 *								uint32_t num,
 *								const GSPI_RESOURCES *gspi,
 *								UDMA_RESOURCES *udma,
 *								UDMA_Channel_Info *chnl_info,
 *								RSI_UDMA_HANDLE_T udmaHandle)
 * @brief		     Start sending/receiving data to/from GSPI transmitter/receiver.
 * @param[in]    data_out    : Pointer to buffer with data to send to GSPI transmitter
 * @param[in]    data_in     : Pointer to buffer for data to receive from GSPI receiver
 * @param[in]    num         : Number of data items to transfer
 * @param[in]	   gspi        : Pointer to the GSPI resources
 * @param[in]	   udma        : Pointer to the UDMA resources
 * @param[in]	   CHNL_INFO   : Pointer to the UDMA channel info
 * @param[in]    udmaHandle  : Pointer to UDMA Handle
 * @return 		   excecution status
 */
int32_t GSPI_Transfer(const void *data_out,
                      void *data_in,
                      uint32_t num,
                      const GSPI_RESOURCES *gspi,
                      UDMA_RESOURCES *udma,
                      UDMA_Channel_Info *chnl_info,
                      RSI_UDMA_HANDLE_T udmaHandle)
{
#ifdef SL_SI91X_GSPI_DMA
  //Added to suppress unused variable warning
  (void)udma;
  (void)udmaHandle;
  (void)chnl_info;
  sl_status_t status;
#else
  volatile int32_t stat = 0;
#endif
  RSI_UDMA_CHA_CONFIG_DATA_T control = { 0 };
  uint16_t data_bits;
  uint16_t data_16bit;

  if ((data_out == NULL) || (data_in == NULL) || (num == 0U)) {
    return ARM_DRIVER_ERROR_PARAMETER;
  }
  if (!(gspi->info->state & SPI_CONFIGURED)) {
    return ARM_DRIVER_ERROR;
  }
  if (gspi->info->status.busy) {
    return ARM_DRIVER_ERROR_BUSY;
  }

  gspi->info->status.busy       = 1U;
  gspi->info->status.data_lost  = 0U;
  gspi->info->status.mode_fault = 0U;

  gspi->xfer->rx_buf = (uint8_t *)data_in;
  gspi->xfer->tx_buf = (uint8_t *)data_out;

  gspi->xfer->rx_cnt = 0U;
  gspi->xfer->tx_cnt = 0U;

  // Read the number of data bits; if the value is 0, it means 16 bits are valid
  data_bits = gspi->reg->GSPI_WRITE_DATA2_b.GSPI_MANUAL_WRITE_DATA2;
  if (data_bits == 0) {
    data_bits = DATA_WIDTH_16; // 0 indicates all 16 bits are valid
  }

  gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_WR      = ENABLE; //write enable
  gspi->reg->GSPI_WRITE_DATA2_b.USE_PREV_LENGTH = true;

  // Enable Full duplex mode
  gspi->reg->GSPI_CONFIG1_b.SPI_FULL_DUPLEX_EN = ENABLE;

  if ((gspi->rx_dma) || (gspi->tx_dma)) {
    // Configure the FIFO thresholds
    gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AEMPTY_THRLD = (uint32_t)(gspi->threshold->gspi_aempty_threshold & 0x0F);
    gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AFULL_THRLD  = (uint32_t)(gspi->threshold->gspi_afull_threshold & 0x0F);
    if (gspi->tx_dma) {
      control.transferType       = UDMA_MODE_BASIC;
      control.nextBurst          = 0;
      control.totalNumOfDMATrans = (num < 1024) ? (num - 1) : (0x03FF);
      control.rPower             = (uint32_t)(gspi->threshold->txdma_arb_size & 0x0F);
      control.srcProtCtrl        = 0x0;
      control.dstProtCtrl        = 0x0;
      control.dstInc             = DST_INC_NONE;
      if (data_bits <= DATA_WIDTH_8) {
        //8-bit data frame
        control.srcSize = SRC_SIZE_8;
        control.srcInc  = SRC_INC_8;
        control.dstSize = DST_SIZE_8;
      } else {
        //16-bit data frame
        control.srcSize = SRC_SIZE_16;
        control.srcInc  = SRC_INC_16;
        control.dstSize = DST_SIZE_16;
      }
#ifdef SL_SI91X_GSPI_DMA
      sl_dma_xfer_t dma_transfer_tx = { 0 };
      uint32_t channel              = gspi->tx_dma->channel + 1;
      uint32_t channel_priority     = gspi->tx_dma->chnl_cfg.channelPrioHigh;
      sl_dma_callback_t gspi_tx_callback;
      //Initialize sl_dma callback structure
      gspi_tx_callback.transfer_complete_cb = gspi_transfer_complete_callback;
      gspi_tx_callback.error_cb             = gspi_error_callback;
      //Initialize sl_dma transfer structure
      dma_transfer_tx.src_addr       = (uint32_t *)((uint32_t)(gspi->xfer->tx_buf));
      dma_transfer_tx.dest_addr      = (uint32_t *)((uint32_t) & (gspi->reg->GSPI_WRITE_FIFO));
      dma_transfer_tx.src_inc        = control.srcInc;
      dma_transfer_tx.dst_inc        = control.dstInc;
      dma_transfer_tx.xfer_size      = control.dstSize;
      dma_transfer_tx.transfer_count = num;
      dma_transfer_tx.transfer_type  = SL_DMA_MEMORY_TO_PERIPHERAL;
      dma_transfer_tx.dma_mode       = control.transferType;
      dma_transfer_tx.signal         = (uint8_t)gspi->tx_dma->chnl_cfg.periAck;

      //Allocate DMA channel for Tx
      status = sl_si91x_dma_allocate_channel(DMA_INSTANCE, &channel, channel_priority);
      if (status && (status != SL_STATUS_DMA_CHANNEL_ALLOCATED)) {
        return ARM_DRIVER_ERROR;
      }
      //Register transfer complete and error callback
      if (sl_si91x_dma_register_callbacks(DMA_INSTANCE, channel, &gspi_tx_callback)) {
        return ARM_DRIVER_ERROR;
      }
      //Configure the channel for DMA transfer
      if (sl_si91x_dma_transfer(DMA_INSTANCE, channel, &dma_transfer_tx)) {
        return ARM_DRIVER_ERROR;
      }
#else
      // Initialize and start GSPI TX DMA Stream
      stat = UDMAx_ChannelConfigure(udma,
                                    gspi->tx_dma->channel,
                                    (uint32_t)(gspi->xfer->tx_buf),
                                    (uint32_t) & (gspi->reg->GSPI_WRITE_FIFO),
                                    num,
                                    control,
                                    &gspi->tx_dma->chnl_cfg,
                                    gspi->tx_dma->cb_event,
                                    chnl_info,
                                    udmaHandle);
      if (stat == -1) {
        return ARM_DRIVER_ERROR;
      }
      UDMAx_ChannelEnable(gspi->tx_dma->channel, udma, udmaHandle);
#endif
    }
    if (gspi->rx_dma) {
      control.transferType       = UDMA_MODE_BASIC;
      control.nextBurst          = 0;
      control.totalNumOfDMATrans = (num < 1024) ? (num - 1) : (0x03FF);
      control.rPower             = (uint32_t)(gspi->threshold->rxdma_arb_size & 0x0F);
      control.srcProtCtrl        = 0x0;
      control.dstProtCtrl        = 0x0;
      control.srcInc             = SRC_INC_NONE;
      if (data_bits <= DATA_WIDTH_8) {
        //8-bit data frame
        control.srcSize = SRC_SIZE_8;
        control.dstSize = DST_SIZE_8;
        control.dstInc  = DST_INC_8;
      } else {
        //16-bit data frame
        control.srcSize = SRC_SIZE_16;
        control.dstSize = DST_SIZE_16;
        control.dstInc  = DST_INC_16;
      }
#ifdef SL_SI91X_GSPI_DMA
      sl_dma_xfer_t dma_transfer_rx = { 0 };
      uint32_t channel              = gspi->rx_dma->channel + 1;
      uint32_t channel_priority     = gspi->rx_dma->chnl_cfg.channelPrioHigh;
      sl_dma_callback_t gspi_rx_callback;
      //Initialize sl_dma callback structure
      gspi_rx_callback.transfer_complete_cb = gspi_transfer_complete_callback;
      gspi_rx_callback.error_cb             = gspi_error_callback;
      //Initialize sl_dma transfer structure
      dma_transfer_rx.src_addr       = (uint32_t *)((uint32_t) & (gspi->reg->GSPI_READ_FIFO));
      dma_transfer_rx.dest_addr      = (uint32_t *)((uint32_t)(gspi->xfer->rx_buf));
      dma_transfer_rx.src_inc        = control.srcInc;
      dma_transfer_rx.dst_inc        = control.dstInc;
      dma_transfer_rx.xfer_size      = control.dstSize;
      dma_transfer_rx.transfer_count = num;
      dma_transfer_rx.transfer_type  = SL_DMA_PERIPHERAL_TO_MEMORY;
      dma_transfer_rx.dma_mode       = control.transferType;
      dma_transfer_rx.signal         = (uint8_t)gspi->rx_dma->chnl_cfg.periAck;

      //Allocate DMA channel for Rx
      status = sl_si91x_dma_allocate_channel(DMA_INSTANCE, &channel, channel_priority);
      if (status && (status != SL_STATUS_DMA_CHANNEL_ALLOCATED)) {
        return ARM_DRIVER_ERROR;
      }
      //Register transfer complete and error callback
      if (sl_si91x_dma_register_callbacks(DMA_INSTANCE, channel, &gspi_rx_callback)) {
        return ARM_DRIVER_ERROR;
      }
      //Configure the channel for DMA transfer
      if (sl_si91x_dma_transfer(DMA_INSTANCE, channel, &dma_transfer_rx)) {
        return ARM_DRIVER_ERROR;
      }
#else
      // Initialize and start GSPI RX DMA Stream
      stat = UDMAx_ChannelConfigure(udma,
                                    gspi->rx_dma->channel,
                                    (uint32_t) & (gspi->reg->GSPI_READ_FIFO),
                                    (uint32_t)(gspi->xfer->rx_buf),
                                    num,
                                    control,
                                    &gspi->rx_dma->chnl_cfg,
                                    gspi->rx_dma->cb_event,
                                    chnl_info,
                                    udmaHandle);
      if (stat == -1) {
        return ARM_DRIVER_ERROR;
      }
      UDMAx_ChannelEnable(gspi->rx_dma->channel, udma, udmaHandle);
#endif
      gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_RD = ENABLE;
#ifdef SL_SI91X_GSPI_DMA
      sl_si91x_dma_channel_enable(DMA_INSTANCE, gspi->rx_dma->channel + 1);
      sl_si91x_dma_channel_enable(DMA_INSTANCE, gspi->tx_dma->channel + 1);
#else
      UDMAx_DMAEnable(udma, udmaHandle);
#endif
    }
  } else {
    // Converts the data width into number of bytes to send/receive at once
    GSPI_Convert_Data_Width_To_Bytes(data_bits);
    gspi->xfer->num = num * data_width_in_bytes;

    // Configure the FIFO thresholds in interrupt mode
    gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AEMPTY_THRLD = 0x1;
    gspi->reg->GSPI_FIFO_THRLD_b.FIFO_AFULL_THRLD  = 0xC;
    // enable GSPI interrupt
    gspi->reg->GSPI_INTR_UNMASK |= GSPI_INTR_UNMASK_BIT;

    // write first index data
    if (data_width_in_bytes == 1) {
      *(volatile aliased_uint32_t *)(gspi->reg->GSPI_WRITE_FIFO) =
        (uint32_t) * (gspi->xfer->tx_buf + gspi->xfer->tx_cnt++);
    } else if (data_width_in_bytes == 2) {
      data_16bit = *(gspi->xfer->tx_buf + gspi->xfer->tx_cnt++);
      data_16bit |= (uint16_t)(*(gspi->xfer->tx_buf + gspi->xfer->tx_cnt++) << 8U);
      *(volatile aliased_uint32_t *)(gspi->reg->GSPI_WRITE_FIFO) = (uint32_t)data_16bit;
    }
    while (gspi->reg->GSPI_STATUS & GSPI_BUSY_F)
      ;
  }
  return ARM_DRIVER_OK;
}

/*==============================================*/
/**
 * @fn          uint32_t GSPI_GetDataCount(const GSPI_RESOURCES *gspi)
 * @brief		     Get transferred data count.
 * @param[in]	   gspi        : Pointer to the GSPI resources
 * @return 		   number of data items transferred
 */
uint32_t GSPI_GetDataCount(const GSPI_RESOURCES *gspi)
{
  if (!(gspi->info->state & SPI_CONFIGURED)) {
    return 0U;
  }

  return gspi->xfer->rx_cnt;
}
/*==============================================*/
/**
 * @fn          void GSPI_UDMA_Tx_Event(uint32_t event, uint8_t dmaCh, GSPI_RESOURCES *gspi)
 * @brief		     GSPI UDMA transfer event Handler
 * @param[in]    event       : UDMA Event
 * @param[in]    dmaCh       : UDMA channel
 * @param[in]	   gspi        : Pointer to the GSPI resources
 * @return 		   none
 */
void GSPI_UDMA_Tx_Event(uint32_t event, uint8_t dmaCh, GSPI_RESOURCES *gspi)
{
  (void)dmaCh;
  switch (event) {
    case UDMA_EVENT_XFER_DONE:
      gspi->info->status.busy = 0U;
      // Update TX buffer info
      gspi->xfer->tx_cnt = gspi->xfer->num;
      if (gspi->xfer->rx_buf == NULL) {
        if (gspi->info->cb_event != NULL) {
#ifdef SL_SI91X_GSPI_DMA
          sl_si91x_dma_channel_disable(DMA_INSTANCE, gspi->tx_dma->channel + 1);
#endif
          gspi->info->cb_event(ARM_SPI_EVENT_TRANSFER_COMPLETE);
        }
      }
      break;
    case UDMA_EVENT_ERROR:
      break;
  }
}

/*==============================================*/
/**
 * @fn          void GSPI_UDMA_Rx_Event(uint32_t event, uint8_t dmaCh, GSPI_RESOURCES *gspi)
 * @brief		     GSPI UDMA rx event Handler
 * @param[in]    event       : UDMA Event
 * @param[in]    dmaCh       : UDMA channel
 * @param[in]	   gspi        : Pointer to the GSPI resources
 * @return 		   none
 */
void GSPI_UDMA_Rx_Event(uint32_t event, uint8_t dmaCh, GSPI_RESOURCES *gspi)
{
  (void)dmaCh;
  switch (event) {
    case UDMA_EVENT_XFER_DONE:
      //gspi->xfer->rx_cnt    = gspi->xfer->num;
      gspi->info->status.busy = 0U;
      break;
    case UDMA_EVENT_ERROR:
      break;
  }
  if (gspi->info->cb_event != NULL) {
#ifdef SL_SI91X_GSPI_DMA
    sl_si91x_dma_channel_disable(DMA_INSTANCE, gspi->rx_dma->channel + 1);
#endif
    gspi->info->cb_event(ARM_SPI_EVENT_TRANSFER_COMPLETE);
  }
}

/*==============================================*/
/**
 * @fn          void GSPI_IRQHandler(const GSPI_RESOURCES *gspi)
 * @brief		     GSPI IRQ Handler
 * @param[in]	   gspi        : Pointer to the GSPI resources
 * @return 		   none
 */
void GSPI_IRQHandler(const GSPI_RESOURCES *gspi)
{
  uint32_t event = 0;
  uint32_t gspi_status;
  uint8_t data_8bit;
  uint16_t data_16bit;

  gspi_status = gspi->reg->GSPI_STATUS;

  gspi->reg->GSPI_INTR_ACK_b.GSPI_INTR_ACK = ENABLE;

  if (!(gspi_status & GSPI_FIFO_EMPTY_RFIFO_S)) {
    if (gspi->xfer->rx_buf) {
      if (gspi->xfer->rx_cnt < gspi->xfer->num) {
        // read the next data available in the FIFO into rx_buf
        if (data_width_in_bytes == 1) {
          //8bit
          data_8bit                                    = *(volatile aliased_uint32_t *)(gspi->reg->GSPI_READ_FIFO);
          *(gspi->xfer->rx_buf + gspi->xfer->rx_cnt++) = data_8bit;
        } else if (data_width_in_bytes == 2) {
          //16bit
          data_16bit                                   = *(volatile aliased_uint32_t *)(gspi->reg->GSPI_READ_FIFO);
          *(gspi->xfer->rx_buf + gspi->xfer->rx_cnt++) = (uint8_t)data_16bit;
          *(gspi->xfer->rx_buf + gspi->xfer->rx_cnt++) = (uint8_t)(data_16bit >> 8U);
        }
        // If all bytes are received, clear the interrupt
        if (gspi->xfer->rx_cnt >= gspi->xfer->num) {
          gspi->reg->GSPI_INTR_MASK |= (GSPI_INTR_MASK_BIT);
          // Clear busy flag
          gspi->info->status.busy = 0U;
          // Transfer completed
          event |= ARM_SPI_EVENT_TRANSFER_COMPLETE;
        } else if (!(gspi->xfer->tx_buf)) { // Only meant to write dummy data in case of half-duplex mode "Receive-only"
          if (data_width_in_bytes == 1) {
            *(volatile aliased_uint32_t *)(gspi->reg->GSPI_WRITE_FIFO) = (uint32_t)DUMMY_DATA;
          } else if (data_width_in_bytes == 2) {
            *(volatile aliased_uint32_t *)(gspi->reg->GSPI_WRITE_FIFO) = (uint32_t)DUMMY_DATA;
          }
        }
      }
    } else {
      // Unanticipated receive, flush the fifo
      uint32_t flush_data;
      flush_data = *(volatile aliased_uint32_t *)(gspi->reg->GSPI_READ_FIFO);
      (void)flush_data;
    }
  }

  if (!(gspi_status & GSPI_FIFO_AFULL_WFIFO_S)) {
    if (gspi->xfer->tx_buf) {
      // verify if there are still some bytes to transmit
      if (gspi->xfer->tx_cnt < gspi->xfer->num) {
        if (data_width_in_bytes == 1) {
          data_8bit                                                  = *(gspi->xfer->tx_buf + gspi->xfer->tx_cnt++);
          *(volatile aliased_uint32_t *)(gspi->reg->GSPI_WRITE_FIFO) = (uint32_t)data_8bit;
        } else if (data_width_in_bytes == 2) {
          data_16bit = *(gspi->xfer->tx_buf + gspi->xfer->tx_cnt++);
          data_16bit |= (uint16_t)(*(gspi->xfer->tx_buf + gspi->xfer->tx_cnt++) << 8);
          *(volatile aliased_uint32_t *)(gspi->reg->GSPI_WRITE_FIFO) = (uint32_t)data_16bit;
        }
        while (gspi->reg->GSPI_STATUS_b.GSPI_BUSY)
          ;
      } else if (!(gspi->xfer->rx_buf)) {
        // if all bytes are transmitted, clear the interrupt only in case of "Send-only"
        gspi->reg->GSPI_INTR_MASK |= (GSPI_INTR_MASK_BIT);
        gspi->info->status.busy = 0U;
        event |= ARM_SPI_EVENT_TRANSFER_COMPLETE;
      }
    }
  }
  // Send callback event
  if (event && gspi->info->cb_event) {
    gspi->info->cb_event(event);
  }
}
/**
 * @fn          void GSPI_Convert_Data_Width_To_Bytes(uint16_t data_width)
 * @brief       Update the number of bytes w.r.t. data width in a global static variable
 * @param[in]   data_width        : Number of data-bits configured
 * @return      none
 */
static void GSPI_Convert_Data_Width_To_Bytes(uint16_t data_width)
{
  if (data_width <= DATA_WIDTH_8) {
    // For 8-bit data frame number of bytes is 1
    data_width_in_bytes = 1;
  } else if (data_width <= DATA_WIDTH_16) {
    // For 16-bit data frame number of bytes is 2
    data_width_in_bytes = 2;
  }
}
/**
 * @fn          ARM_SPI_STATUS GSPI_GetStatus(const GSPI_RESOURCES *gspi)
 * @brief		    Get GSPI status.
 * @param[in]	  gspi        : Pointer to the GSPI resources
 * @return 		  ARM_SPI_STATUS
 */
ARM_SPI_STATUS GSPI_GetStatus(const GSPI_RESOURCES *gspi)
{
  ARM_SPI_STATUS gspi_status;
  GSPI_STATUS status = gspi->info->status;

  gspi_status.busy       = status.busy;
  gspi_status.data_lost  = status.data_lost;
  gspi_status.mode_fault = status.mode_fault;

  return gspi_status;
}
#endif //GSPI_ROMDRIVER_PRESENT
