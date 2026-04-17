/*******************************************************************************
 * @file  efx32_ncp_host.c
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "sli_wifi_utility.h"
#include "sl_wifi_constants.h"
#include "sl_si91x_host_interface.h"
#include "sl_board_configuration.h"
#include "sl_status.h"
#include "em_eusart.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "cmsis_os2.h"
#include "dmadrv.h"
#include "gpiointerrupt.h"
#include "sl_si91x_status.h"
#include "sl_rsi_utility.h"
#include "sl_constants.h"
#include <stdbool.h>
#include <string.h>

#define SLI_LDMA_MAX_TRANSFER_LENGTH     4096
#define SLI_LDMA_DESCRIPTOR_ARRAY_LENGTH (SLI_LDMA_MAX_TRANSFER_LENGTH / 2048)
#define PACKET_PENDING_INT_PRI           3
#define SLI_SPI_BIT_RATE                 10000000

static bool sli_dma_callback(unsigned int channel, unsigned int sequenceNo, void *userParam);

unsigned int rx_ldma_channel;
unsigned int tx_ldma_channel;
osMutexId_t spi_transfer_mutex = NULL;

static uint32_t dummy_buffer;
static uint8_t host_initialized = 0;

uint8_t dummy_buffer_test[2500];
// LDMA descriptor and transfer configuration structures for EUSART TX channel
LDMA_Descriptor_t ldmaTXDescriptor[SLI_LDMA_DESCRIPTOR_ARRAY_LENGTH];
LDMA_TransferCfg_t ldmaTXConfig;

// LDMA descriptor and transfer configuration structures for EUSART RX channel
LDMA_Descriptor_t ldmaRXDescriptor[SLI_LDMA_DESCRIPTOR_ARRAY_LENGTH];
LDMA_TransferCfg_t ldmaRXConfig;

static osSemaphoreId_t transfer_done_semaphore = NULL;

static bool sli_dma_callback(unsigned int channel, unsigned int sequenceNo, void *userParam)
{
  UNUSED_PARAMETER(channel);
  UNUSED_PARAMETER(sequenceNo);
  UNUSED_PARAMETER(userParam);
  osSemaphoreRelease(transfer_done_semaphore);
  return false;
}

static void gpio_interrupt(uint8_t interrupt_number)
{
  UNUSED_PARAMETER(interrupt_number);
  sli_wifi_set_event(SL_SI91X_NCP_HOST_BUS_RX_EVENT);
  //  GPIO_IntClear(0xAAAA);
}

void sl_si91x_host_set_sleep_indicator(void)
{
  GPIO_PinOutSet(SLEEP_CONFIRM_PIN.port, SLEEP_CONFIRM_PIN.pin);
}

void sl_si91x_host_clear_sleep_indicator(void)
{
  GPIO_PinOutClear(SLEEP_CONFIRM_PIN.port, SLEEP_CONFIRM_PIN.pin);
}

uint32_t sl_si91x_host_get_wake_indicator(void)
{
  return GPIO_PinInGet(WAKE_INDICATOR_PIN.port, WAKE_INDICATOR_PIN.pin);
}

sl_status_t sl_si91x_host_init(const sl_si91x_host_init_configuration_t *config)
{
  UNUSED_PARAMETER(config);
  if (!host_initialized) {
    // Enable clock (not needed on xG21)
    CMU_ClockEnable(cmuClock_GPIO, true);

    // Configure SPI bus pins
    GPIO_PinModeSet(SPI_MISO_PIN.port, SPI_MISO_PIN.pin, gpioModeInput, 0);
    GPIO_PinModeSet(SPI_MOSI_PIN.port, SPI_MOSI_PIN.pin, gpioModePushPull, 0);
    GPIO_PinModeSet(SPI_CLOCK_PIN.port, SPI_CLOCK_PIN.pin, gpioModePushPullAlternate, 0);
    GPIO_PinModeSet(SPI_CS_PIN.port, SPI_CS_PIN.pin, gpioModePushPull, 1);
    // Enable clock (not needed on xG21)
    CMU_ClockEnable(SPI_EUSART_CMU_CLOCK, true);

    // Default asynchronous initializer (master mode, 1 Mbps, 8-bit data)
    EUSART_SpiAdvancedInit_TypeDef adv = EUSART_SPI_ADVANCED_INIT_DEFAULT;

    adv.msbFirst = true; // SPI standard MSB first

    EUSART_SpiInit_TypeDef init = EUSART_SPI_MASTER_INIT_DEFAULT_HF;

    init.bitRate          = SLI_SPI_BIT_RATE; // 10 MHz shift clock
    init.advancedSettings = &adv;             // Advanced settings structure

    /*
   * Route EUSART RX, TX, and CLK to the specified pins.  Note that CS is
   * not controlled by EUSART so there is no write to the corresponding
   * USARTROUTE register to do this.
   */
    GPIO->EUSARTROUTE[SPI_EUSART_ROUTE_INDEX].RXROUTE = (SPI_MISO_PIN.port << _GPIO_EUSART_RXROUTE_PORT_SHIFT)
                                                        | (SPI_MISO_PIN.pin << _GPIO_EUSART_RXROUTE_PIN_SHIFT);
    GPIO->EUSARTROUTE[SPI_EUSART_ROUTE_INDEX].TXROUTE = (SPI_MOSI_PIN.port << _GPIO_EUSART_TXROUTE_PORT_SHIFT)
                                                        | (SPI_MOSI_PIN.pin << _GPIO_EUSART_TXROUTE_PIN_SHIFT);
    GPIO->EUSARTROUTE[SPI_EUSART_ROUTE_INDEX].SCLKROUTE = (SPI_CLOCK_PIN.port << _GPIO_EUSART_SCLKROUTE_PORT_SHIFT)
                                                          | (SPI_CLOCK_PIN.pin << _GPIO_EUSART_SCLKROUTE_PIN_SHIFT);
    GPIO->EUSARTROUTE[SPI_EUSART_ROUTE_INDEX].CSROUTE = (SPI_CS_PIN.port << _GPIO_EUSART_CSROUTE_PORT_SHIFT)
                                                        | (SPI_CS_PIN.pin << _GPIO_EUSART_CSROUTE_PIN_SHIFT);

    // Enable USART interface pins
    GPIO->EUSARTROUTE[SPI_EUSART_ROUTE_INDEX].ROUTEEN = GPIO_EUSART_ROUTEEN_RXPEN | // MISO
                                                        GPIO_EUSART_ROUTEEN_TXPEN | // MOSI
                                                        GPIO_EUSART_ROUTEEN_SCLKPEN | GPIO_EUSART_ROUTEEN_CSPEN;

    // Set slew rate for alternate usage pins
    GPIO_SlewrateSet(SPI_CLOCK_PIN.port, 7, 7);

    EUSART1->TIMINGCFG |= /*USART_TIMING_TXDELAY_ONE | USART_TIMING_CSSETUP_ONE |*/ EUSART_TIMINGCFG_CSHOLD_ONE;

    EUSART_SpiInit(SPI_EUSART, &init);

    if (transfer_done_semaphore == NULL) {
      transfer_done_semaphore = osSemaphoreNew(1, 0, NULL);
    }

    if (spi_transfer_mutex == NULL) {
      spi_transfer_mutex = osMutexNew(NULL);
    }

    DMADRV_Init();
    DMADRV_AllocateChannel(&rx_ldma_channel, NULL);
    DMADRV_AllocateChannel(&tx_ldma_channel, NULL);

    // Start reset line low
    GPIO_PinModeSet(RESET_PIN.port, RESET_PIN.pin, gpioModeWiredAnd, 0);

    // configure packet pending interrupt priority
    NVIC_SetPriority(GPIO_ODD_IRQn, PACKET_PENDING_INT_PRI);

    // Configure interrupt, sleep and wake confirmation pins
    GPIOINT_CallbackRegister(INTERRUPT_PIN.pin, gpio_interrupt);
    GPIO_PinModeSet(INTERRUPT_PIN.port, INTERRUPT_PIN.pin, gpioModeInputPullFilter, 0);
    GPIO_ExtIntConfig(INTERRUPT_PIN.port, INTERRUPT_PIN.pin, INTERRUPT_PIN.pin, true, false, true);
    GPIO_PinModeSet(SLEEP_CONFIRM_PIN.port, SLEEP_CONFIRM_PIN.pin, gpioModeWiredOrPullDown, 1);
    GPIO_PinModeSet(WAKE_INDICATOR_PIN.port, WAKE_INDICATOR_PIN.pin, gpioModeWiredOrPullDown, 0);
    host_initialized = 1;
  } else {
    if (spi_transfer_mutex == NULL) {
      spi_transfer_mutex = osMutexNew(NULL);
    }
  }

  return SL_STATUS_OK;
}

sl_status_t sl_si91x_host_deinit(void)
{
  if (spi_transfer_mutex != NULL) {
    osMutexDelete(spi_transfer_mutex);
    spi_transfer_mutex = NULL;
  }
  return SL_STATUS_OK;
}

void sl_si91x_host_enable_high_speed_bus()
{
}

__WEAK void sl_si91x_host_spi_cs_assert()
{
  EUSART_BaudrateSet(SPI_EUSART, 0, SLI_SPI_BIT_RATE);
  GPIO_PinOutClear(SPI_CS_PIN.port, SPI_CS_PIN.pin);
}

__WEAK void sl_si91x_host_spi_cs_deassert()
{
  GPIO_PinOutSet(SPI_CS_PIN.port, SPI_CS_PIN.pin);
}

/*==================================================================*/
/**
 * @fn         sl_status_t sl_si91x_host_spi_transfer(const void *tx_buffer, void *rx_buffer, uint16_t buffer_length)
 * @param[in]  uint8_t *tx_buff, pointer to the buffer with the data to be transferred
 * @param[in]  uint8_t *rx_buff, pointer to the buffer to store the data received
 * @param[in]  uint16_t transfer_length, Number of bytes to send and receive
 * @param[in]  uint8_t mode, To indicate mode 8 BIT/32 BIT mode transfers.
 * @param[out] None
 * @return     0, 0=success
 * @section description
 * This API is used to transfer/receive data to the Wi-Fi module through the SPI interface.
 */
sl_status_t sl_si91x_host_spi_transfer(const void *tx_buffer, void *rx_buffer, uint16_t buffer_length)
{
  int i;
  osMutexAcquire(spi_transfer_mutex, 0xFFFFFFFFUL);

  if (buffer_length <= 2048) {
    if (tx_buffer == NULL) {
      dummy_buffer = 0;
      ldmaTXDescriptor[0] =
        (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_P2P_BYTE(&dummy_buffer, &(SPI_EUSART->TXDATA), buffer_length);
    } else {
      ldmaTXDescriptor[0] =
        (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(tx_buffer, &(SPI_EUSART->TXDATA), buffer_length);
    }
    if (rx_buffer == NULL) {
      ldmaRXDescriptor[0] =
        (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_P2P_BYTE(&(SPI_EUSART->RXDATA), &dummy_buffer, buffer_length);
    } else {
      ldmaRXDescriptor[0] =
        (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&(SPI_EUSART->RXDATA), rx_buffer, buffer_length);
    }
  } else {
    if (tx_buffer == NULL) {
      tx_buffer = (uint8_t *)&dummy_buffer_test;
    } else if (rx_buffer == NULL) {
      rx_buffer = (uint8_t *)&dummy_buffer_test;
    }
    //Transfer length is more than 2048 bytes. Initialize multiple LDMA Tx descriptor.
    for (i = 0; i < (SLI_LDMA_DESCRIPTOR_ARRAY_LENGTH - 1); i++) {
      ldmaRXDescriptor[i] =
        (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(&(SPI_EUSART->RXDATA), (rx_buffer + (2048 * i)), 2048, 1);
      ldmaTXDescriptor[i] =
        (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_M2P_BYTE((tx_buffer + (2048 * i)), &(SPI_EUSART->TXDATA), 2048, 1);
    }
    ldmaRXDescriptor[i] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&(SPI_EUSART->RXDATA),
                                                                             (rx_buffer + (2048 * i)),
                                                                             (buffer_length - (2048 * i)));
    ldmaTXDescriptor[i] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_M2P_BYTE((tx_buffer + (2048 * i)),
                                                                             &(SPI_EUSART->TXDATA),
                                                                             (buffer_length - (2048 * i)));
  }

  // Transfer a byte on free space in the USART buffer
  ldmaTXConfig = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(SPI_EUSART_LDMA_TX);

  // Transfer a byte on receive data valid
  ldmaRXConfig = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(SPI_EUSART_LDMA_RX);

  // Start both channels
  DMADRV_LdmaStartTransfer(rx_ldma_channel, &ldmaRXConfig, ldmaRXDescriptor, sli_dma_callback, NULL);
  DMADRV_LdmaStartTransfer(tx_ldma_channel, &ldmaTXConfig, ldmaTXDescriptor, NULL, NULL);

  if (osSemaphoreAcquire(transfer_done_semaphore, 1000) != osOK) {
    osMutexRelease(spi_transfer_mutex);
    return SL_STATUS_BUS_ERROR;
  }

  osMutexRelease(spi_transfer_mutex);

  return SL_STATUS_OK;
}

void sl_si91x_host_hold_in_reset(void)
{
  GPIO_PinOutClear(RESET_PIN.port, RESET_PIN.pin);
}

void sl_si91x_host_release_from_reset(void)
{
  GPIO_PinOutSet(RESET_PIN.port, RESET_PIN.pin);
}

void sl_si91x_host_enable_bus_interrupt(void)
{
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
}

void sl_si91x_host_disable_bus_interrupt(void)
{
  NVIC_DisableIRQ(GPIO_ODD_IRQn);
}

bool sl_si91x_host_is_in_irq_context(void)
{
  return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0U;
}
