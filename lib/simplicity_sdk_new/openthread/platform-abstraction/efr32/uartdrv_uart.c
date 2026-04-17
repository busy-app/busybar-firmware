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
 *   This file implements the OpenThread platform abstraction for UART communication.
 *
 */

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#include "sl_core.h"
#include "sl_uartdrv_instances.h"

#ifdef SL_CATALOG_UARTDRV_USART_PRESENT
#include "sl_uartdrv_usart_vcom_config.h"
#elif defined(SL_CATALOG_UARTDRV_EUSART_PRESENT)
#include "sl_uartdrv_eusart_vcom_config.h"
#endif
#include "uartdrv.h"
#include <openthread-core-config.h>
#include <openthread-system.h>
#include <stddef.h>
#include <string.h>
#include "utils/code_utils.h"
#include "utils/uart.h"

#include "ecode.h"
#include "platform-efr32.h"
#include "sl_sleeptimer.h"
#include "sl_status.h"

#ifdef SL_CATALOG_KERNEL_PRESENT
#include "sl_ot_rtos_adaptation.h"
#endif // SL_CATALOG_KERNEL_PRESENT

#define IRQ_CONCAT(type, instance, property) type##instance##property

#ifdef SL_CATALOG_UARTDRV_USART_PRESENT

#define UART_PERIPHERAL SL_UARTDRV_USART_VCOM_PERIPHERAL
#define UART_HANDLE sl_uartdrv_usart_vcom_handle

#define IRQ_LABEL_FORMAT(peripheral_no) IRQ_CONCAT(USART, peripheral_no, _RX_IRQn)
#define IRQ_HANDLER_FORMAT(peripheral_no) IRQ_CONCAT(USART, peripheral_no, _RX_IRQHandler)

#define UART_IRQ IRQ_LABEL_FORMAT(SL_UARTDRV_USART_VCOM_PERIPHERAL_NO)
#define UART_IRQHandler IRQ_HANDLER_FORMAT(SL_UARTDRV_USART_VCOM_PERIPHERAL_NO)
#define UART_IRQ_NAME USART_IF_RXDATAV
#define UART_IRQ_ENABLE USART_IntEnable
#define UART_IRQ_DISABLE USART_IntEnable

#define CLEAR_RX_IRQ() (void)USART_RxDataGet(UART_PERIPHERAL)

#elif defined(SL_CATALOG_UARTDRV_EUSART_PRESENT)

#define UART_PERIPHERAL SL_UARTDRV_EUSART_VCOM_PERIPHERAL
#define UART_HANDLE sl_uartdrv_eusart_vcom_handle

#define IRQ_LABEL_FORMAT(peripheral_no) IRQ_CONCAT(EUSART, peripheral_no, _RX_IRQn)
#define IRQ_HANDLER_FORMAT(peripheral_no) IRQ_CONCAT(EUSART, peripheral_no, _RX_IRQHandler)

#define UART_IRQ IRQ_LABEL_FORMAT(SL_UARTDRV_EUSART_VCOM_PERIPHERAL_NO)
#define UART_IRQHandler IRQ_HANDLER_FORMAT(SL_UARTDRV_EUSART_VCOM_PERIPHERAL_NO)
#define UART_IRQ_NAME EUSART_IF_RXFL

#if defined(_SILICON_LABS_32B_SERIES_2)

#define UART_IRQ_ENABLE EUSART_IntEnable
#define UART_IRQ_DISABLE EUSART_IntEnable

#define CLEAR_RX_IRQ() (void)EUSART_IntClear(UART_PERIPHERAL, EUSART_IF_RXFL)

#define EUSART_ENABLE() EUSART_Enable(UART_PERIPHERAL, eusartEnable)

#else //_SILICON_LABS_32B_SERIES_3

#define UART_IRQ_ENABLE sl_hal_eusart_enable_interrupts
#define UART_IRQ_DISABLE sl_hal_eusart_disable_interrupts

#define CLEAR_RX_IRQ() (void)sl_hal_eusart_clear_interrupts(UART_PERIPHERAL, EUSART_IF_RXFL)

#define EUSART_ENABLE()                       \
    sl_hal_eusart_enable(UART_PERIPHERAL);    \
    sl_hal_eusart_enable_rx(UART_PERIPHERAL); \
    sl_hal_eusart_enable_tx(UART_PERIPHERAL)
#endif //_SILICON_LABS_32B_SERIES_2 or _SILICON_LABS_32B_SERIES_3
#endif // SL_CATALOG_UARTDRV_USART_PRESENT or SL_CATALOG_UARTDRV_EUSART_PRESENT

enum
{
    kReceiveFifoSize = 128,
};

// In order to reduce the probability of data loss due to disabled interrupts, we use
// two duplicate receive buffers so we can always have one "active" receive request.
#define RECEIVE_BUFFER_SIZE 128
static uint8_t       sReceiveBuffer1[RECEIVE_BUFFER_SIZE];
static uint8_t       sReceiveBuffer2[RECEIVE_BUFFER_SIZE];
static uint8_t       lastCount    = 0;
static volatile bool sTxComplete  = false;
static volatile bool sRxDataReady = false;

typedef struct ReceiveFifo_t
{
    // The data buffer
    uint8_t mBuffer[kReceiveFifoSize];
    // The offset of the first item written to the list.
    volatile uint16_t mHead;
    // The offset of the next item to be written to the list.
    volatile uint16_t mTail;
} ReceiveFifo_t;

static ReceiveFifo_t sReceiveFifo;

static void processReceive(void);
static void processTransmit(void);

void UART_IRQHandler(void)
{
    sRxDataReady = true;
    CLEAR_RX_IRQ();
#ifdef SL_CATALOG_KERNEL_PRESENT
    sl_ot_rtos_set_pending_event(SL_OT_RTOS_EVENT_SERIAL);
#endif
    otSysEventSignalPending();
}

static void receiveDone(UARTDRV_Handle_t aHandle, Ecode_t aStatus, uint8_t *aData, UARTDRV_Count_t aCount)
{
    OT_UNUSED_VARIABLE(aStatus);

    // We can only write if incrementing mTail doesn't equal mHead
    if (sReceiveFifo.mHead != (sReceiveFifo.mTail + aCount - lastCount) % kReceiveFifoSize)
    {
        memcpy(sReceiveFifo.mBuffer + sReceiveFifo.mTail, aData + lastCount, aCount - lastCount);
        sReceiveFifo.mTail = (sReceiveFifo.mTail + aCount - lastCount) % kReceiveFifoSize;
        lastCount          = 0;
    }

    UARTDRV_Receive(aHandle, aData, aCount, receiveDone);

#ifdef SL_CATALOG_KERNEL_PRESENT
    sl_ot_rtos_set_pending_event(SL_OT_RTOS_EVENT_SERIAL); // Receive Done event
#endif
    otSysEventSignalPending();
}

static void transmitDone(UARTDRV_Handle_t aHandle, Ecode_t aStatus, uint8_t *aData, UARTDRV_Count_t aCount)
{
    OT_UNUSED_VARIABLE(aHandle);
    OT_UNUSED_VARIABLE(aStatus);
    OT_UNUSED_VARIABLE(aData);
    OT_UNUSED_VARIABLE(aCount);

    // This value will be used later in processTransmit() to call otPlatUartSendDone()
    sTxComplete = true;
#ifdef SL_CATALOG_KERNEL_PRESENT
    sl_ot_rtos_set_pending_event(SL_OT_RTOS_EVENT_SERIAL);
#endif
    otSysEventSignalPending();
}

static void processReceive(void)
{
    uint8_t        *aData;
    UARTDRV_Count_t aCount, remaining;

    otEXPECT(sRxDataReady);

    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_ATOMIC();

    sRxDataReady = false;
    UARTDRV_GetReceiveStatus(UART_HANDLE, &aData, &aCount, &remaining);
    if (aCount > lastCount)
    {
        memcpy(sReceiveFifo.mBuffer + sReceiveFifo.mTail, aData + lastCount, aCount - lastCount);
        sReceiveFifo.mTail = (sReceiveFifo.mTail + aCount - lastCount) % kReceiveFifoSize;
        lastCount          = aCount;
    }

    CORE_EXIT_ATOMIC();

    // Copy tail to prevent multiple reads
    uint16_t tail = sReceiveFifo.mTail;

    // If the data wraps around, process the first part
    if (sReceiveFifo.mHead > tail)
    {
        otPlatUartReceived(sReceiveFifo.mBuffer + sReceiveFifo.mHead, kReceiveFifoSize - sReceiveFifo.mHead);

        // Reset the buffer mHead back to zero.
        sReceiveFifo.mHead = 0;
    }

    // For any data remaining, process it
    if (sReceiveFifo.mHead != tail)
    {
        otPlatUartReceived(sReceiveFifo.mBuffer + sReceiveFifo.mHead, tail - sReceiveFifo.mHead);

        // Set mHead to the local tail we have cached
        sReceiveFifo.mHead = tail;
    }
exit:
    return;
}

static void processTransmit(void)
{
    // NOTE: This check needs to be done in here and cannot be done in transmitDone because the transmit may not be
    // fully complete when the transmitDone callback is called.
    otEXPECT(sTxComplete);
    sTxComplete = false;
    otPlatUartSendDone();
exit:
    return;
}

void efr32UartProcess(void)
{
    processReceive();
    processTransmit();
}

otError otPlatUartEnable(void)
{
    otError error = OT_ERROR_NONE;

    // Enable UART interrupt to wake OT task when data arrives
    NVIC_ClearPendingIRQ(UART_IRQ);
    NVIC_EnableIRQ(UART_IRQ);

    // Clear previous RX interrupts
    CLEAR_RX_IRQ();
#ifdef SL_CATALOG_UARTDRV_EUSART_PRESENT
    // Enable EUSART
    EUSART_ENABLE();
#endif

    UART_IRQ_ENABLE(UART_PERIPHERAL, UART_IRQ_NAME);

    sReceiveFifo.mHead = 0;
    sReceiveFifo.mTail = 0;

    // When one receive request is completed, the other buffer is used for a separate receive request, issued
    // immediately.
    UARTDRV_Receive(UART_HANDLE, sReceiveBuffer1, RECEIVE_BUFFER_SIZE, receiveDone);
    UARTDRV_Receive(UART_HANDLE, sReceiveBuffer2, RECEIVE_BUFFER_SIZE, receiveDone);

    return error;
}

otError otPlatUartDisable(void)
{
    return OT_ERROR_NOT_IMPLEMENTED;
}

static void flushTimeoutAlarmCallback(sl_sleeptimer_timer_handle_t *aHandle, void *aData)
{
    OT_UNUSED_VARIABLE(aHandle);
    *(bool *)aData = true;
}

otError otPlatUartFlush(void)
{
    otError                      error         = OT_ERROR_NONE;
    sl_status_t                  status        = SL_STATUS_OK;
    volatile bool                flushTimedOut = false;
    sl_sleeptimer_timer_handle_t flushTimer;

    // Start flush timeout timer
    status = sl_sleeptimer_start_timer_ms(&flushTimer,
                                          OPENTHREAD_CONFIG_EFR32_UART_TX_FLUSH_TIMEOUT_MS,
                                          flushTimeoutAlarmCallback,
                                          (void *)&flushTimedOut,
                                          0,
                                          SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
    otEXPECT_ACTION(status == SL_STATUS_OK, error = OT_ERROR_FAILED);

    // Block until DMA has finished transmitting every buffer in sUartTxQueue and becomes idle
    uint8_t transmitQueueDepth = 0;
    bool    uartFullyFlushed   = false;
    bool    uartIdle           = false;

    do
    {
        // Check both peripheral status and queue depth
        transmitQueueDepth = UARTDRV_GetTransmitDepth(UART_HANDLE);
        uartIdle           = (UARTDRV_GetPeripheralStatus(UART_HANDLE) & (UARTDRV_STATUS_TXIDLE | UARTDRV_STATUS_TXC));
        uartFullyFlushed   = uartIdle && (transmitQueueDepth == 0);
    } while (!uartFullyFlushed && !flushTimedOut);

    sl_sleeptimer_stop_timer(&flushTimer);

    if (flushTimedOut)
    {
        // Abort all transmits
        UARTDRV_Abort(UART_HANDLE, uartdrvAbortTransmit);
    }
    sTxComplete = false;
exit:
    return error;
}

OT_TOOL_WEAK void otPlatUartReceived(const uint8_t *aBuf, uint16_t aBufLength)
{
    OT_UNUSED_VARIABLE(aBuf);
    OT_UNUSED_VARIABLE(aBufLength);
    // do nothing
}

otError otPlatUartSend(const uint8_t *aBuf, uint16_t aBufLength)
{
    otError error  = OT_ERROR_NONE;
    Ecode_t status = ECODE_EMDRV_UARTDRV_OK;

    status = UARTDRV_Transmit(UART_HANDLE, (uint8_t *)aBuf, aBufLength, transmitDone);
    otEXPECT_ACTION(status == ECODE_EMDRV_UARTDRV_OK, error = OT_ERROR_FAILED);

exit:
    return error;
}

OT_TOOL_WEAK void otPlatUartSendDone(void)
{
    // do nothing
}
