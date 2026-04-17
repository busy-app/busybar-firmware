/*
 *  Copyright (c) 2025, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file implements the OpenThread platform abstraction for SPI communication.
 *
 */

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#include "sl_core.h"

#include "sl_device_gpio.h"
#include "sl_gpio.h"
#include "sl_hal_gpio.h"
#include "sl_hal_ldma.h"

#include "spidrv.h"

#include "sl_clock_manager.h"
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif

#include "sl_ncp_spidrv_eusart_config.h"

#include "platform-efr32.h"
#include <openthread-system.h>
#include <openthread/error.h>
#include <openthread/platform/spi-slave.h>
#include "common/code_utils.hpp"
#include "common/debug.hpp"

#ifdef SL_CATALOG_KERNEL_PRESENT
#include "sl_ot_rtos_adaptation.h"
#endif // SL_CATALOG_KERNEL_PRESENT

// DEFINES
#define MAX_DMA_DESCRIPTOR_TRANSFER_COUNT ((_LDMA_CH_CTRL_XFERCNT_MASK >> _LDMA_CH_CTRL_XFERCNT_SHIFT) + 1U)

#define SL_OT_SPIDRV_SPI_CONCAT_PASTER(first, second, third) first##second##third

#define SL_OT_SPIDRV_SPI_LDMA_RX_PERIPH_TRIGGER(periph_nbr) \
    SL_OT_SPIDRV_SPI_CONCAT_PASTER(SL_HAL_LDMA_PERIPHERAL_SIGNAL_EUSART, periph_nbr, _RXFL)
#define SL_OT_SPIDRV_SPI_LDMA_TX_PERIPH_TRIGGER(periph_nbr) \
    SL_OT_SPIDRV_SPI_CONCAT_PASTER(SL_HAL_LDMA_PERIPHERAL_SIGNAL_EUSART, periph_nbr, _TXFL)

//  MEMBER VARIABLES
static volatile bool should_process_transaction = false;

// DMA
static sl_hal_ldma_transfer_config_t rx_dma_transfer_config;
static sl_hal_ldma_transfer_config_t tx_dma_transfer_config;

static uint8_t                           default_tx_value;
static volatile sl_hal_ldma_descriptor_t tx_descriptor[2];

static volatile sl_hal_ldma_descriptor_t rx_descriptor;

// TRANSACTION EVENT'S CALLBACK
static volatile otPlatSpiSlaveTransactionCompleteCallback complete_callback;
static volatile otPlatSpiSlaveTransactionProcessCallback  process_callback;
static volatile void                                     *context;

// SPI Peripheral
static volatile SPIDRV_HandleData_t sl_spidrv_handle_data;

static void rcp_spidrv_set_host_request(void)
{
#if defined(SL_NCP_SPIDRV_EUSART_HOST_INT_PORT) && defined(SL_NCP_SPIDRV_EUSART_HOST_INT_PIN)
    const sl_gpio_t host_int_gpio = {.port = SL_NCP_SPIDRV_EUSART_HOST_INT_PORT,
                                     .pin  = SL_NCP_SPIDRV_EUSART_HOST_INT_PIN};
    sl_hal_gpio_clear_pin(&host_int_gpio);
#else
    int empty = 0;
    (void)empty;
#endif
}

static void rcp_spidrv_deassert_host_request(void)
{
#if defined(SL_NCP_SPIDRV_EUSART_HOST_INT_PORT) && defined(SL_NCP_SPIDRV_EUSART_HOST_INT_PIN)
    const sl_gpio_t host_int_gpio = {.port = SL_NCP_SPIDRV_EUSART_HOST_INT_PORT,
                                     .pin  = SL_NCP_SPIDRV_EUSART_HOST_INT_PIN};
    sl_hal_gpio_set_pin(&host_int_gpio);
#else
    int empty = 0;
    (void)empty;
#endif
}

static void clearEusartFifos(EUSART_TypeDef *eusart)
{
    // EUSART_CMD_CLEARTX reportedly only affects UART, not SPI mode,
    // and there is no EUSART_CMD_CLEARRX. Only way to clear the
    // FIFOs is via the big hammer of disabling then reenabling it.
#if defined(_SILICON_LABS_32B_SERIES_2)
    EUSART_Enable(eusart, eusartDisable);
    EUSART_Enable(eusart, eusartEnable);
#else
    sl_hal_eusart_disable_rx(eusart);
    sl_hal_eusart_disable_tx(eusart);
    sl_hal_eusart_disable(eusart);
    while (eusart->EN & _EUSART_EN_DISABLING_MASK);

    sl_hal_eusart_enable(eusart);
    sl_hal_eusart_enable_rx(eusart);
    sl_hal_eusart_enable_tx(eusart);
    sl_hal_eusart_wait_sync(eusart, _EUSART_SYNCBUSY_MASK);
#endif
}

static void rcp_spidrv_spi_transaction_end_interrupt(uint8_t intNo, void *ctx)
{
    OT_UNUSED_VARIABLE(ctx);

    if (intNo == SL_NCP_SPIDRV_EUSART_CS_FALLING_EDGE_INT_NO)
    {
        return;
    }
    else if (intNo == SL_NCP_SPIDRV_EUSART_CS_RISING_EDGE_INT_NO)
    {
        // Must be done before calling the "complete_callback" since
        // this callback will use otPlatSpiSlavePrepareTransaction who
        // would not setup the buffers if a transaction is ongoing.
        rcp_spidrv_deassert_host_request();
    }

    uint32_t tx_transaction_size = 0U;

    sl_hal_ldma_stop_transfer(LDMA0, sl_spidrv_handle_data.txDMACh);
    sl_hal_ldma_stop_transfer(LDMA0, sl_spidrv_handle_data.rxDMACh);

    // Clear the FIFOs if there are more bytes to transmit than expected LDMA tx xferCnt.
    if (sl_spidrv_handle_data.peripheral.eusartPort->STATUS & _EUSART_STATUS_TXFCNT_MASK)
    {
        clearEusartFifos(sl_spidrv_handle_data.peripheral.eusartPort);
    }

    uint32_t tx_dma_channel_nb = sl_spidrv_handle_data.txDMACh;

    // Use the current loader DMA transfer struct to asses the number of transfered bytes.
    uint32_t tx_dma_channel_remaining_xfercnt =
        (LDMA0->CH[tx_dma_channel_nb].CTRL & _LDMA_CH_CTRL_XFERCNT_MASK) >> _LDMA_CH_CTRL_XFERCNT_SHIFT;
    tx_dma_channel_remaining_xfercnt += 1U;
    tx_dma_channel_remaining_xfercnt +=
        (sl_spidrv_handle_data.peripheral.eusartPort->STATUS & _EUSART_STATUS_TXFCNT_MASK)
        >> _EUSART_STATUS_TXFCNT_SHIFT;

    uint32_t current_tx_descritor_link =
        (LDMA0->CH[tx_dma_channel_nb].LINK & _LDMA_CH_LINK_LINK_MASK) >> _LDMA_CH_LINK_LINK_SHIFT;

    uint8_t *old_tx_buffer      = (uint8_t *)tx_descriptor[0].xfer.src_addr;
    uint16_t old_tx_buffer_size = tx_descriptor[0].xfer.xfer_count + 1U;

    uint8_t *old_rx_buffer      = (uint8_t *)rx_descriptor.xfer.dst_addr;
    uint16_t old_rx_buffer_size = rx_descriptor.xfer.xfer_count + 1U;

    if (current_tx_descritor_link == 0U)
    {
        // Since the link bit is not set in the tx dma channel descriptor the second tx descriptor was loaded.
        tx_transaction_size = tx_descriptor[0].xfer.xfer_count + 1U;
        tx_transaction_size += (tx_descriptor[1].xfer.xfer_count + 1U) - tx_dma_channel_remaining_xfercnt;
    }
    else
    {
        tx_transaction_size = (tx_descriptor[0].xfer.xfer_count - tx_dma_channel_remaining_xfercnt) + 1U;
    }

    // call's otPlatSpiSlavePrepareTransaction in the background, the DMA buffer's will be ready after this call.
    if (complete_callback((void *)context,
                          (uint8_t *)old_tx_buffer,
                          old_tx_buffer_size,
                          (uint8_t *)old_rx_buffer,
                          old_rx_buffer_size,
                          tx_transaction_size))
    {
        should_process_transaction = true;
#ifdef SL_CATALOG_KERNEL_PRESENT
        sl_ot_rtos_set_pending_event(SL_OT_RTOS_EVENT_SERIAL);
#endif
        otSysEventSignalPending();
    }
}

otError otPlatSpiSlaveEnable(otPlatSpiSlaveTransactionCompleteCallback aCompleteCallback,
                             otPlatSpiSlaveTransactionProcessCallback  aProcessCallback,
                             void                                     *aContext)
{
    CORE_DECLARE_IRQ_STATE;
    otError error = OT_ERROR_NONE;

    // If driver was already configured, an error is returned.
    VerifyOrExit(complete_callback == NULL, error = OT_ERROR_ALREADY);
    VerifyOrExit(process_callback == NULL, error = OT_ERROR_ALREADY);
    VerifyOrExit(context == NULL, error = OT_ERROR_ALREADY);

    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);

    SPIDRV_Init_t init_data = (SPIDRV_Init_t){
        SL_NCP_SPIDRV_EUSART_PERIPHERAL, ///< The EUSART used for SPI.
        SL_NCP_SPIDRV_EUSART_TX_PORT,    ///< Tx port.
        SL_NCP_SPIDRV_EUSART_RX_PORT,    ///< Rx port.
        SL_NCP_SPIDRV_EUSART_SCLK_PORT,  ///< Clock port.
        SL_NCP_SPIDRV_EUSART_CS_PORT,    ///< Chip select port.
        SL_NCP_SPIDRV_EUSART_TX_PIN,     ///< Tx pin.
        SL_NCP_SPIDRV_EUSART_RX_PIN,     ///< Rx pin.
        SL_NCP_SPIDRV_EUSART_SCLK_PIN,   ///< Clock pin.
        SL_NCP_SPIDRV_EUSART_CS_PIN,     ///< Chip select pin.
        0U,                              ///< An SPI bitrate.
        8,                               ///< An SPI framelength, valid numbers are 4..16
        0,                               ///< The value to transmit when using SPI receive API functions.
        spidrvSlave,                     ///< An SPI type, slave.
        SL_NCP_SPIDRV_EUSART_BIT_ORDER,  ///< A bit order on the SPI bus, MSB or LSB first.
        SL_NCP_SPIDRV_EUSART_CLOCK_MODE, ///< SPI mode, CLKPOL/CLKPHASE setting.
        spidrvCsControlAuto,             ///< A select master mode chip select (CS) control scheme.
        spidrvSlaveStartImmediate,       ///< A slave mode transfer start scheme.
    };

    VerifyOrExit(SPIDRV_Init((SPIDRV_HandleData_t *)&sl_spidrv_handle_data, &init_data) == ECODE_EMDRV_SPIDRV_OK,
                 error = OT_ERROR_FAILED);

    // Client callback functions.
    complete_callback = aCompleteCallback;
    process_callback  = aProcessCallback;
    context           = aContext;

    // Client complete callback request foreground processing.
    should_process_transaction = false;

    // TX default value.
    default_tx_value = 0xFFU;

    // DMA transfer structures.
    rx_dma_transfer_config = (sl_hal_ldma_transfer_config_t)SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(
        SL_OT_SPIDRV_SPI_LDMA_RX_PERIPH_TRIGGER(SL_NCP_SPIDRV_EUSART_PERIPHERAL_NO));
    tx_dma_transfer_config = (sl_hal_ldma_transfer_config_t)SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(
        SL_OT_SPIDRV_SPI_LDMA_TX_PERIPH_TRIGGER(SL_NCP_SPIDRV_EUSART_PERIPHERAL_NO));

    rx_descriptor = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_SINGLE_P2M(
        SL_HAL_LDMA_CTRL_SIZE_BYTE,
        &(sl_spidrv_handle_data.peripheral.eusartPort->RXDATA),
        NULL,
        1U);
    rx_descriptor.xfer.done_ifs = 0U;

    tx_descriptor[0] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_LINKREL_M2P(
        SL_HAL_LDMA_CTRL_SIZE_BYTE,
        &default_tx_value,
        &(sl_spidrv_handle_data.peripheral.eusartPort->TXDATA),
        1,
        1);
    tx_descriptor[0].xfer.done_ifs = 0U;

    tx_descriptor[1] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_SINGLE_M2P(
        SL_HAL_LDMA_CTRL_SIZE_BYTE,
        &default_tx_value,
        &(sl_spidrv_handle_data.peripheral.eusartPort->TXDATA),
        MAX_DMA_DESCRIPTOR_TRANSFER_COUNT);
    tx_descriptor[1].xfer.src_inc  = SL_HAL_LDMA_CTRL_SRC_INC_NONE;
    tx_descriptor[1].xfer.done_ifs = 0U;

    // Configuring Host INT line. Active low
#if defined(SL_NCP_SPIDRV_EUSART_HOST_INT_PORT) && defined(SL_NCP_SPIDRV_EUSART_HOST_INT_PIN)
    const sl_gpio_t host_int_gpio = {.port = SL_NCP_SPIDRV_EUSART_HOST_INT_PORT,
                                     .pin  = SL_NCP_SPIDRV_EUSART_HOST_INT_PIN};
    sl_hal_gpio_set_pin_mode(&host_int_gpio, SL_GPIO_MODE_PUSH_PULL, 1U);
#endif

    const sl_gpio_t cs_gpio = {.port = SL_NCP_SPIDRV_EUSART_CS_PORT, .pin = SL_NCP_SPIDRV_EUSART_CS_PIN};
    sl_hal_gpio_set_pin_mode(&cs_gpio, SL_GPIO_MODE_INPUT_PULL_FILTER, 1);

    CORE_ENTER_ATOMIC();

    // Initialization during transaction is not supported.
    VerifyOrExit(sl_hal_gpio_get_pin_input(&cs_gpio), error = OT_ERROR_FAILED);

    int32_t     intNo;
    sl_status_t status;
    intNo  = SL_NCP_SPIDRV_EUSART_CS_RISING_EDGE_INT_NO;
    status = sl_gpio_configure_external_interrupt(&cs_gpio,
                                                  &intNo,
                                                  SL_GPIO_INTERRUPT_RISING_EDGE,
                                                  rcp_spidrv_spi_transaction_end_interrupt,
                                                  NULL);
    OT_ASSERT(status == SL_STATUS_OK);
    intNo  = SL_NCP_SPIDRV_EUSART_CS_FALLING_EDGE_INT_NO;
    status = sl_gpio_configure_external_interrupt(&cs_gpio,
                                                  &intNo,
                                                  SL_GPIO_INTERRUPT_FALLING_EDGE,
                                                  rcp_spidrv_spi_transaction_end_interrupt,
                                                  NULL);
    OT_ASSERT(status == SL_STATUS_OK);

    // Clear the eusart RX/TX FIFO before configuring the dma transfers.
    clearEusartFifos(sl_spidrv_handle_data.peripheral.eusartPort);

    // Load the default value descriptor.
    sl_hal_ldma_init_transfer(LDMA0,
                              sl_spidrv_handle_data.txDMACh,
                              (sl_hal_ldma_transfer_config_t *)&tx_dma_transfer_config,
                              (sl_hal_ldma_descriptor_t *)&tx_descriptor[1]);
    sl_hal_ldma_start_transfer(LDMA0, sl_spidrv_handle_data.txDMACh);

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif

    CORE_EXIT_ATOMIC();

exit:
    return error;
}

void otPlatSpiSlaveDisable(void)
{
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_ATOMIC();

    // Disable CS GPIO IRQ.
    IgnoreReturnValue(sl_gpio_deconfigure_external_interrupt(SL_NCP_SPIDRV_EUSART_CS_RISING_EDGE_INT_NO));
    IgnoreReturnValue(sl_gpio_deconfigure_external_interrupt(SL_NCP_SPIDRV_EUSART_CS_FALLING_EDGE_INT_NO));

    CORE_EXIT_ATOMIC();

    // Stop any reception DMA transfers.
    sl_hal_ldma_stop_transfer(LDMA0, sl_spidrv_handle_data.txDMACh);
    sl_hal_ldma_stop_transfer(LDMA0, sl_spidrv_handle_data.rxDMACh);

    // Host INT line.
    rcp_spidrv_deassert_host_request();
#if defined(SL_NCP_SPIDRV_EUSART_HOST_INT_PORT) && defined(SL_NCP_SPIDRV_EUSART_HOST_INT_PIN)
    const sl_gpio_t host_int_gpio = {.port = SL_NCP_SPIDRV_EUSART_HOST_INT_PORT,
                                     .pin  = SL_NCP_SPIDRV_EUSART_HOST_INT_PIN};
    sl_hal_gpio_set_pin_mode(&host_int_gpio, SL_GPIO_MODE_INPUT, 0U);
#endif

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif

    should_process_transaction = false;

    complete_callback = NULL;
    process_callback  = NULL;
    context           = NULL;
}

otError otPlatSpiSlavePrepareTransaction(uint8_t *aOutputBuf,
                                         uint16_t aOutputBufLen,
                                         uint8_t *aInputBuf,
                                         uint16_t aInputBufLen,
                                         bool     aRequestTransactionFlag)
{
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_ATOMIC();

    otError error = OT_ERROR_NONE;

    VerifyOrExit(aOutputBufLen <= MAX_DMA_DESCRIPTOR_TRANSFER_COUNT, error = OT_ERROR_FAILED);
    VerifyOrExit(aInputBufLen <= MAX_DMA_DESCRIPTOR_TRANSFER_COUNT, error = OT_ERROR_FAILED);

    uint32_t tx_dma_channel_number = sl_spidrv_handle_data.txDMACh;
    uint32_t rx_dma_channel_number = sl_spidrv_handle_data.rxDMACh;

    // Check the CS pin if SPI transactions are in progress.
    const sl_gpio_t cs_gpio = {.port = SL_NCP_SPIDRV_EUSART_CS_PORT, .pin = SL_NCP_SPIDRV_EUSART_CS_PIN};
    VerifyOrExit(sl_hal_gpio_get_pin_input(&cs_gpio), error = OT_ERROR_BUSY);

    if (aOutputBuf)
    {
        sl_hal_ldma_stop_transfer(LDMA0, tx_dma_channel_number);
    }

    if (aInputBuf)
    {
        // Clear the rxFifo only if it is not empty.
        if (sl_spidrv_handle_data.peripheral.eusartPort->STATUS & _EUSART_STATUS_RXFL_MASK)
        {
            clearEusartFifos(sl_spidrv_handle_data.peripheral.eusartPort);
        }

        sl_hal_ldma_stop_transfer(LDMA0, rx_dma_channel_number);
    }

    VerifyOrExit(sl_hal_gpio_get_pin_input(&cs_gpio), error = OT_ERROR_BUSY);

    if (aOutputBuf != NULL)
    {
        // Clear the txFifo only if it is not empty.
        if (sl_spidrv_handle_data.peripheral.eusartPort->STATUS & _EUSART_STATUS_TXFCNT_MASK)
        {
            clearEusartFifos(sl_spidrv_handle_data.peripheral.eusartPort);
        }

        tx_descriptor[0].xfer.xfer_count = aOutputBufLen - 1U;
        tx_descriptor[0].xfer.src_addr   = (uint32_t)aOutputBuf;
    }

    if (aInputBuf != NULL)
    {
        rx_descriptor.xfer.xfer_count = aInputBufLen - 1U;
        rx_descriptor.xfer.dst_addr   = (uint32_t)aInputBuf;
    }

    VerifyOrExit(sl_hal_gpio_get_pin_input(&cs_gpio), error = OT_ERROR_BUSY);

    if (aOutputBuf != NULL)
    {
        sl_hal_ldma_init_transfer(LDMA0,
                                  tx_dma_channel_number,
                                  (sl_hal_ldma_transfer_config_t *)&tx_dma_transfer_config,
                                  (sl_hal_ldma_descriptor_t *)&(tx_descriptor[0]));
        sl_hal_ldma_start_transfer(LDMA0, tx_dma_channel_number);
    }

    if (aInputBuf != NULL)
    {
        sl_hal_ldma_init_transfer(LDMA0,
                                  rx_dma_channel_number,
                                  (sl_hal_ldma_transfer_config_t *)&rx_dma_transfer_config,
                                  (sl_hal_ldma_descriptor_t *)&rx_descriptor);
        sl_hal_ldma_start_transfer(LDMA0, rx_dma_channel_number);
    }

    if (aRequestTransactionFlag)
    {
        rcp_spidrv_set_host_request();
    }
    else
    {
        rcp_spidrv_deassert_host_request();
    }

exit:

    CORE_EXIT_ATOMIC();
    return error;
}

void efr32SpiProcess(void)
{
    if (should_process_transaction)
    {
        if (context)
        {
            process_callback((void *)context);
        }

        should_process_transaction = false;
    }
}
