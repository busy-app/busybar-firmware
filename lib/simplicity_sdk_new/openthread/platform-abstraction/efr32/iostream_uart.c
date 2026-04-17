/*
 *  Copyright (c) 2023, The OpenThread Authors.
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
 *   This file implements the OpenThread platform abstraction for UART communication
 *   using the iostream APIs
 *
 */

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#include "uart.h"
#include <openthread-system.h>
#include <stddef.h>
#include <string.h>
#include "utils/code_utils.h"
#include "utils/uart.h"

#if defined(SL_CATALOG_IOSTREAM_EUSART_PRESENT)
#define IOSTREAM_VCOM_CONFIG_HEADER "sl_iostream_eusart_vcom_config.h"
#define IOSTREAM_INSTANCE_HEADER "sl_iostream_init_eusart_instances.h"
#define IOSTREAM_RX_GPIO_PORT SL_IOSTREAM_EUSART_VCOM_RX_PORT
#define IOSTREAM_RX_GPIO_PIN SL_IOSTREAM_EUSART_VCOM_RX_PIN
#define IOSTREAM_RX_BUFFER_SIZE SL_IOSTREAM_EUSART_VCOM_RX_BUFFER_SIZE
#elif defined(SL_CATALOG_IOSTREAM_USART_PRESENT)
#define IOSTREAM_VCOM_CONFIG_HEADER "sl_iostream_usart_vcom_config.h"
#define IOSTREAM_INSTANCE_HEADER "sl_iostream_init_usart_instances.h"
#define IOSTREAM_RX_GPIO_PORT SL_IOSTREAM_USART_VCOM_RX_PORT
#define IOSTREAM_RX_GPIO_PIN SL_IOSTREAM_USART_VCOM_RX_PIN
#define IOSTREAM_RX_BUFFER_SIZE SL_IOSTREAM_USART_VCOM_RX_BUFFER_SIZE
#endif

#include "sl_gpio.h"
#include IOSTREAM_VCOM_CONFIG_HEADER
#include IOSTREAM_INSTANCE_HEADER

#ifdef SL_CATALOG_KERNEL_PRESENT
#include "sl_ot_rtos_adaptation.h"
#endif // SL_CATALOG_KERNEL_PRESENT

#ifndef TESTING
#define STATIC static
#else
#define STATIC
#endif

#if (defined(SL_CATALOG_POWER_MANAGER_PRESENT) || defined(SL_CATALOG_KERNEL_PRESENT))
#define ENABLE_UART_RX_INTERRUPT
#endif

#if (defined ENABLE_UART_RX_INTERRUPT) && (!OPENTHREAD_RADIO)
#define WAIT_FOR_UART_RX_READY
#endif

static uint8_t       sReceiveBuffer[IOSTREAM_RX_BUFFER_SIZE];
static volatile bool sTransmitDone = false;
static volatile bool sRxDataReady  = false;

#ifdef ENABLE_UART_RX_INTERRUPT
static unsigned int sGpioIntContext = IOSTREAM_RX_GPIO_PIN;

STATIC void gpioSerialWakeupCallback(uint8_t interrupt_no, void *context)
{
    unsigned int *pin = (unsigned int *)context;

    (void)interrupt_no;

    if (*pin == IOSTREAM_RX_GPIO_PIN)
    {
        sRxDataReady = true;
#ifdef SL_CATALOG_KERNEL_PRESENT
        sl_ot_rtos_set_pending_event(SL_OT_RTOS_EVENT_SERIAL);
#endif
        otSysEventSignalPending();
    }
}

static otError gpioSerialRxInterruptEnable(void)
{
    unsigned int intNo = SL_GPIO_INTERRUPT_UNAVAILABLE;
    sl_gpio_t    gpio;
    gpio.port          = IOSTREAM_RX_GPIO_PORT;
    gpio.pin           = IOSTREAM_RX_GPIO_PIN;
    sl_status_t status = sl_gpio_configure_external_interrupt(&gpio,
                                                              (int32_t *)&intNo,
                                                              SL_GPIO_INTERRUPT_FALLING_EDGE,
                                                              (sl_gpio_irq_callback_t)gpioSerialWakeupCallback,
                                                              &sGpioIntContext);

    return (status == SL_STATUS_OK) ? OT_ERROR_NONE : OT_ERROR_FAILED;
}
#endif // ENABLE_UART_RX_INTERRUPT

bool efr32UartIsDataReady(void)
{
    return sRxDataReady || sTransmitDone;
}

static void processReceive(void)
{
    sl_status_t status;
    size_t      bytes_read = 0;

#ifndef WAIT_FOR_UART_RX_READY
    // Set ready value to true before check if configured to not wait for data ready interrupt
    sRxDataReady = true;
#endif

    otEXPECT(sRxDataReady);

    memset(sReceiveBuffer, 0, IOSTREAM_RX_BUFFER_SIZE);

#ifdef SL_CATALOG_KERNEL_PRESENT
    // Set Read API to non-blocking mode
    sl_iostream_uart_set_read_block((sl_iostream_uart_t *)sl_iostream_uart_vcom_handle, false);
#endif // SL_CATALOG_KERNEL_PRESENT

    status = sl_iostream_read(sl_iostream_vcom_handle, &sReceiveBuffer, sizeof(sReceiveBuffer), &bytes_read);

    sRxDataReady = false;

    if (status == SL_STATUS_OK)
    {
        otPlatUartReceived(sReceiveBuffer, bytes_read);
    }

    otSysEventSignalPending();

exit:
    return;
}

static void processTransmit(void)
{
    (void)otPlatUartFlush();
}

void efr32UartInit(void)
{
    memset(sReceiveBuffer, 0, IOSTREAM_RX_BUFFER_SIZE);
    sTransmitDone = false;
    sRxDataReady  = false;
}

void efr32UartProcess(void)
{
    processReceive();
    processTransmit();
}

otError otPlatUartFlush(void)
{
    otError error = OT_ERROR_NONE;
    otEXPECT_ACTION(sTransmitDone, error = OT_ERROR_INVALID_STATE);

    sTransmitDone = false;

    otPlatUartSendDone();
    otSysEventSignalPending();
exit:
    return error;
}

otError otPlatUartEnable(void)
{
    otError error = OT_ERROR_NONE;

#ifdef ENABLE_UART_RX_INTERRUPT
    error = gpioSerialRxInterruptEnable();
#endif

    return error;
}

otError otPlatUartDisable(void)
{
    return OT_ERROR_NOT_IMPLEMENTED;
}

otError otPlatUartSend(const uint8_t *aBuf, uint16_t aBufLength)
{
    otError     error = OT_ERROR_FAILED;
    sl_status_t status;

    otEXPECT(aBuf != NULL && aBufLength > 0);

    if (sTransmitDone)
    {
        (void)otPlatUartFlush();
    }

    status = sl_iostream_write(sl_iostream_vcom_handle, aBuf, aBufLength);
    if (status == SL_STATUS_OK)
    {
        sTransmitDone = true;
        error         = OT_ERROR_NONE;
    }

#ifdef SL_CATALOG_KERNEL_PRESENT
    sl_ot_rtos_set_pending_event(SL_OT_RTOS_EVENT_SERIAL);
#endif // SL_CATALOG_KERNEL_PRESENT
    otSysEventSignalPending();

exit:
    return error;
}

/* Weak Function Definitions - Exclude from code coverage */

// GCOV_EXCL_START
OT_TOOL_WEAK void otPlatUartReceived(const uint8_t *aBuf, uint16_t aBufLength)
{
    OT_UNUSED_VARIABLE(aBuf);
    OT_UNUSED_VARIABLE(aBufLength);
    // do nothing
}

OT_TOOL_WEAK void otPlatUartSendDone(void)
{
    // do nothing
}
// GCOV_EXCL_STOP
