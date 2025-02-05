#include <furi_hal_serial.h>
#include <furi_hal_serial_types_i.h>

#include <furi_hal_resources.h>
#include <furi_hal_interrupt.h>
#include <furi_hal_bus.h>
#include <furi_hal_dma.h>

#include <stm32u5xx_ll_rcc.h>
#include <stm32u5xx_ll_usart.h>
#include <stm32u5xx_ll_dma.h>

typedef struct {
    FuriHalSerialHandle* handle;
    USART_TypeDef* periph_ptr;
    FuriHalSerialRxCallback rx_callback;
    FuriHalSerialTxCallback tx_callback;
    void* callback_context;
    uint32_t dma_rx_channel;
    uint32_t dma_tx_channel;
} FuriHalSerial;

typedef struct {
    USART_TypeDef* periph;
    GpioAltFn alt_fn;
    const GpioPin* gpio[FuriHalSerialPinMax];
    FuriHalInterruptId irq;
    uint32_t tx_dma_request;
    uint32_t rx_dma_request;
} FuriHalSerialResources;

static const FuriHalSerialResources furi_hal_serial_resources[FuriHalSerialIdMax] = {
    [FuriHalSerialIdUsart1] =
        {
            .periph = USART1,
            .alt_fn = GpioAltFn7USART1,
            .gpio =
                {
                    [FuriHalSerialPinTx] = &gpio_usart1_tx,
                    [FuriHalSerialPinRx] = &gpio_usart1_rx,
                    [FuriHalSerialPinRts] = &gpio_usart1_rts,
                    [FuriHalSerialPinCts] = &gpio_usart1_cts,
                },
            .irq = FuriHalInterruptIdUsart1,
            .tx_dma_request = LL_GPDMA1_REQUEST_USART1_TX,
            .rx_dma_request = LL_GPDMA1_REQUEST_USART1_RX,
        },
    [FuriHalSerialIdUsart2] =
        {
            .periph = USART2,
            .alt_fn = GpioAltFn7USART2,
            .gpio =
                {
                    [FuriHalSerialPinTx] = &gpio_usart2_tx,
                    [FuriHalSerialPinRx] = &gpio_usart2_rx,
                    [FuriHalSerialPinRts] = NULL,
                    [FuriHalSerialPinCts] = NULL,
                },
            .irq = FuriHalInterruptIdUsart2,
            .tx_dma_request = LL_GPDMA1_REQUEST_USART2_TX,
            .rx_dma_request = LL_GPDMA1_REQUEST_USART2_RX,
        },
    [FuriHalSerialIdUsart3] =
        {
            .periph = USART3,
            .alt_fn = GpioAltFn7USART3,
            .gpio =
                {
                    [FuriHalSerialPinTx] = NULL,
                    [FuriHalSerialPinRx] = NULL,
                    [FuriHalSerialPinRts] = NULL,
                    [FuriHalSerialPinCts] = NULL,
                },
            .irq = FuriHalInterruptIdUsart3,
            .tx_dma_request = LL_GPDMA1_REQUEST_USART3_TX,
            .rx_dma_request = LL_GPDMA1_REQUEST_USART3_RX,
        },
    [FuriHalSerialIdUart4] =
        {
            .periph = UART4,
            .alt_fn = GpioAltFn8UART4,
            .gpio =
                {
                    [FuriHalSerialPinTx] = NULL,
                    [FuriHalSerialPinRx] = NULL,
                    [FuriHalSerialPinRts] = NULL,
                    [FuriHalSerialPinCts] = NULL,
                },
            .irq = FuriHalInterruptIdUart4,
            .tx_dma_request = LL_GPDMA1_REQUEST_UART4_TX,
            .rx_dma_request = LL_GPDMA1_REQUEST_UART4_RX,
        },
    [FuriHalSerialIdUart5] =
        {
            .periph = UART5,
            .alt_fn = GpioAltFn8UART5,
            .gpio =
                {
                    [FuriHalSerialPinTx] = NULL,
                    [FuriHalSerialPinRx] = NULL,
                    [FuriHalSerialPinRts] = NULL,
                    [FuriHalSerialPinCts] = NULL,
                },
            .irq = FuriHalInterruptIdUart5,
            .tx_dma_request = LL_GPDMA1_REQUEST_UART5_TX,
            .rx_dma_request = LL_GPDMA1_REQUEST_UART5_RX,
        },
    [FuriHalSerialIdUsart6] =
        {
            .periph = USART6,
            .alt_fn = GpioAltFn7USART6,
            .gpio =
                {
                    [FuriHalSerialPinTx] = &gpio_log_usart_tx,
                    [FuriHalSerialPinRx] = &gpio_log_usart_rx,
                    [FuriHalSerialPinRts] = NULL,
                    [FuriHalSerialPinCts] = NULL,
                },
            .irq = FuriHalInterruptIdUsart6,
            .tx_dma_request = LL_GPDMA1_REQUEST_USART6_TX,
            .rx_dma_request = LL_GPDMA1_REQUEST_USART6_RX,
        },
};

static FuriHalSerial* furi_hal_serial[FuriHalSerialIdMax];

static void
    furi_hal_serial_set_transfer_direction(FuriHalSerialHandle* handle, FuriHalSerialDirection dir);

static bool furi_hal_serial_is_enabled(FuriHalSerialHandle* handle);

static void furi_hal_serial_set_config(
    FuriHalSerialHandle* handle,
    FuriHalSerialConfigDataBits data_bits,
    FuriHalSerialConfigParity parity,
    FuriHalSerialConfigStopBits stop_bits);

static inline void furi_hal_serial_check(FuriHalSerialHandle* handle) {
    furi_check(handle, "Serial: handle is NULL");
    furi_assert(furi_hal_serial[handle->id], "Serial: handle is not initialized");
}

static void furi_hal_serial_irq_callback(void* context) {
    FuriHalSerialHandle* handle = context;

    FuriHalSerial* serial = furi_hal_serial[handle->id];
    USART_TypeDef* periph = furi_hal_serial_resources[handle->id].periph;

    uint32_t events = 0;

    if(LL_USART_IsActiveFlag_RXNE_RXFNE(periph)) {
        events |= FuriHalSerialRxEventData;
    }
    if(LL_USART_IsActiveFlag_FE(periph)) {
        LL_USART_ClearFlag_FE(periph);
        events |= FuriHalSerialRxEventFrameError;
    }
    if(LL_USART_IsActiveFlag_NE(periph)) {
        LL_USART_ClearFlag_NE(periph);
        events |= FuriHalSerialRxEventFrameError;
    }
    if(LL_USART_IsActiveFlag_PE(periph)) {
        LL_USART_ClearFlag_PE(periph);
        events |= FuriHalSerialRxEventParityError;
    }
    if(LL_USART_IsActiveFlag_ORE(periph)) {
        LL_USART_ClearFlag_ORE(periph);
        events |= FuriHalSerialRxEventOverrunError;
    }
    if(serial->rx_callback) {
        serial->rx_callback(handle, events, serial->callback_context);
    }
}

static void furi_hal_serial_dma_irq_callback(void* context) {
    FuriHalSerialHandle* handle = context;
    FuriHalSerial* serial = furi_hal_serial[handle->id];

    const uint32_t dma_rx_channel = serial->dma_rx_channel;
    const uint32_t dma_tx_channel = serial->dma_tx_channel;

    if(LL_DMA_IsActiveFlag_TC(GPDMA1, dma_rx_channel)) {
        LL_DMA_ClearFlag_TC(GPDMA1, dma_rx_channel);
        LL_DMA_DisableChannel(GPDMA1, dma_rx_channel);

        if(serial->rx_callback) {
            serial->rx_callback(handle, FuriHalSerialRxEventData, serial->callback_context);
        }
    }

    if(LL_DMA_IsActiveFlag_TC(GPDMA1, dma_tx_channel)) {
        LL_DMA_ClearFlag_TC(GPDMA1, dma_tx_channel);
        LL_DMA_DisableChannel(GPDMA1, dma_tx_channel);

        if(serial->tx_callback) {
            serial->tx_callback(handle, FuriHalSerialTxEventComplete, serial->callback_context);
        }
    }
}

static void furi_hal_serial_dma_tx_init(FuriHalSerialHandle* handle) {
    furi_assert(handle);

    FuriHalSerialId serial_id = handle->id;
    FuriHalSerial* serial = furi_hal_serial[serial_id];
    furi_assert(serial);

    furi_check(furi_hal_dma_allocate_gpdma_channel(&serial->dma_tx_channel));

    const uint32_t dma_channel = serial->dma_tx_channel;
    const uint32_t dma_request = furi_hal_serial_resources[serial_id].tx_dma_request;

    LL_DMA_InitTypeDef dma_init_struct = {
        .SrcAddress = 0,
        .DestAddress = LL_USART_DMA_GetRegAddr(serial->periph_ptr, LL_USART_DMA_REG_DATA_TRANSMIT),
        .BlkDataLength = 0,
        .Request = dma_request,

        .Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH,
        .BlkHWRequest = LL_DMA_HWREQUEST_SINGLEBURST,
        .DataAlignment = LL_DMA_DATA_ALIGN_ZEROPADD,

        .SrcAllocatedPort = LL_DMA_SRC_ALLOCATED_PORT1,
        .SrcBurstLength = 1,
        .SrcIncMode = LL_DMA_SRC_INCREMENT,
        .SrcDataWidth = LL_DMA_SRC_DATAWIDTH_BYTE,

        .DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT0,
        .DestBurstLength = 1,
        .DestIncMode = LL_DMA_DEST_FIXED,
        .DestDataWidth = LL_DMA_DEST_DATAWIDTH_BYTE,

        .TriggerMode = LL_DMA_TRIGM_BLK_TRANSFER,
        .TriggerPolarity = LL_DMA_TRIG_POLARITY_MASKED,
        .TriggerSelection = 0,

        .TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER,

        .Priority = LL_DMA_LOW_PRIORITY_MID_WEIGHT,
        .LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT1,
        .LinkStepMode = LL_DMA_LSM_FULL_EXECUTION,
        .LinkedListBaseAddr = 0,
        .LinkedListAddrOffset = 0,
    };

    LL_DMA_Init(GPDMA1, dma_channel, &dma_init_struct);

    furi_hal_interrupt_set_isr(
        furi_hal_dma_get_gpdma_interrupt_id(dma_channel),
        furi_hal_serial_dma_irq_callback,
        handle);

    LL_DMA_EnableIT_TC(GPDMA1, dma_channel);
}

static void furi_hal_serial_dma_rx_init(FuriHalSerialHandle* handle) {
    furi_assert(handle);

    FuriHalSerialId serial_id = handle->id;
    FuriHalSerial* serial = furi_hal_serial[serial_id];
    furi_assert(serial);

    furi_check(furi_hal_dma_allocate_gpdma_channel(&serial->dma_rx_channel));

    const uint32_t dma_channel = serial->dma_rx_channel;
    const uint32_t dma_request = furi_hal_serial_resources[serial_id].rx_dma_request;

    LL_DMA_InitTypeDef dma_init_struct = {
        .SrcAddress = LL_USART_DMA_GetRegAddr(serial->periph_ptr, LL_USART_DMA_REG_DATA_RECEIVE),
        .DestAddress = 0,
        .BlkDataLength = 0,
        .Request = dma_request,

        .Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY,
        .BlkHWRequest = LL_DMA_HWREQUEST_SINGLEBURST,
        .DataAlignment = LL_DMA_DATA_ALIGN_ZEROPADD,

        .SrcAllocatedPort = LL_DMA_SRC_ALLOCATED_PORT0,
        .SrcBurstLength = 1,
        .SrcIncMode = LL_DMA_SRC_FIXED,
        .SrcDataWidth = LL_DMA_SRC_DATAWIDTH_BYTE,

        .DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT1,
        .DestBurstLength = 1,
        .DestIncMode = LL_DMA_DEST_INCREMENT,
        .DestDataWidth = LL_DMA_DEST_DATAWIDTH_BYTE,

        .TriggerMode = LL_DMA_TRIGM_BLK_TRANSFER,
        .TriggerPolarity = LL_DMA_TRIG_POLARITY_MASKED,
        .TriggerSelection = 0,

        .TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER,

        .Priority = LL_DMA_LOW_PRIORITY_MID_WEIGHT,
        .LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT1,
        .LinkStepMode = LL_DMA_LSM_FULL_EXECUTION,
        .LinkedListBaseAddr = 0,
        .LinkedListAddrOffset = 0,
    };

    LL_DMA_Init(GPDMA1, dma_channel, &dma_init_struct);

    furi_hal_interrupt_set_isr(
        furi_hal_dma_get_gpdma_interrupt_id(dma_channel),
        furi_hal_serial_dma_irq_callback,
        handle);

    LL_DMA_EnableIT_TC(GPDMA1, dma_channel);
}

void furi_hal_serial_init(FuriHalSerialHandle* handle, uint32_t baud_rate) {
    furi_check(handle);

    const FuriHalSerialId serial_id = handle->id;
    furi_check(furi_hal_serial[serial_id] == NULL);

    furi_hal_serial[serial_id] = malloc(sizeof(FuriHalSerial));

    FuriHalSerial* serial = furi_hal_serial[serial_id];
    USART_TypeDef* periph = furi_hal_serial_resources[serial_id].periph;

    serial->handle = handle;
    serial->periph_ptr = periph;

    // TODO: This should not be called by default
    furi_hal_serial_dma_tx_init(handle);
    furi_hal_serial_dma_rx_init(handle);

    switch(handle->id) {
    case FuriHalSerialIdUsart1:
        LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_SYSCLK);
        furi_hal_bus_enable(FuriHalBusUSART1);
        break;
    case FuriHalSerialIdUsart2:
        LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_SYSCLK);
        furi_hal_bus_enable(FuriHalBusUSART2);
        break;
    case FuriHalSerialIdUsart3:
        LL_RCC_SetUSARTClockSource(LL_RCC_USART3_CLKSOURCE_SYSCLK);
        furi_hal_bus_enable(FuriHalBusUSART3);
        break;
    case FuriHalSerialIdUart4:
        LL_RCC_SetUARTClockSource(LL_RCC_UART4_CLKSOURCE_SYSCLK);
        furi_hal_bus_enable(FuriHalBusUART4);
        break;
    case FuriHalSerialIdUart5:
        LL_RCC_SetUARTClockSource(LL_RCC_UART5_CLKSOURCE_SYSCLK);
        furi_hal_bus_enable(FuriHalBusUART5);
        break;
    case FuriHalSerialIdUsart6:
        LL_RCC_SetUSARTClockSource(LL_RCC_USART6_CLKSOURCE_SYSCLK);
        furi_hal_bus_enable(FuriHalBusUSART6);
        break;
    default:
        furi_crash("Invalid serial id");
        break;
    }

    furi_hal_serial_set_transfer_direction(handle, FuriHalSerialDirectionTxRx);

    furi_hal_serial_set_config(
        handle,
        FuriHalSerialConfigDataBits8,
        FuriHalSerialConfigParityNone,
        FuriHalSerialConfigStopBits_1);

    furi_hal_serial_set_baud_rate(handle, baud_rate);
    furi_hal_serial_set_hw_flow_control(handle, FuriHalSerialHwFlowControlNone);

    LL_USART_SetTXFIFOThreshold(periph, LL_USART_FIFOTHRESHOLD_1_8);
    LL_USART_SetRXFIFOThreshold(periph, LL_USART_FIFOTHRESHOLD_1_8);
    LL_USART_EnableFIFO(periph);

    LL_USART_Enable(periph);

    while(!LL_USART_IsActiveFlag_TEACK(periph) || !LL_USART_IsActiveFlag_REACK(periph))
        ;

    furi_hal_serial_clear(handle, FuriHalSerialDirectionTxRx);
}

static void furi_hal_serial_dma_tx_deinit(FuriHalSerialHandle* handle) {
    furi_assert(handle);

    FuriHalSerial* serial = furi_hal_serial[handle->id];
    LL_USART_DisableDMAReq_TX(serial->periph_ptr);

    LL_DMA_DisableChannel(GPDMA1, serial->dma_tx_channel);

    furi_hal_interrupt_set_isr(
        furi_hal_dma_get_gpdma_interrupt_id(serial->dma_tx_channel), NULL, NULL);

    furi_hal_dma_free_gpdma_channel(serial->dma_tx_channel);
}

static void furi_hal_serial_dma_rx_deinit(FuriHalSerialHandle* handle) {
    furi_assert(handle);

    FuriHalSerial* serial = furi_hal_serial[handle->id];
    LL_USART_DisableDMAReq_RX(serial->periph_ptr);

    LL_DMA_DisableChannel(GPDMA1, serial->dma_rx_channel);

    furi_hal_interrupt_set_isr(
        furi_hal_dma_get_gpdma_interrupt_id(serial->dma_rx_channel), NULL, NULL);

    furi_hal_dma_free_gpdma_channel(serial->dma_rx_channel);
}

void furi_hal_serial_deinit(FuriHalSerialHandle* handle) {
    furi_check(handle);

    FuriHalSerialId serial_id = handle->id;
    FuriHalSerial* serial = furi_hal_serial[serial_id];
    // TODO: deinit() should NOT be called before init()
    if(serial == NULL) return;

    LL_USART_Disable(serial->periph_ptr);

    furi_hal_serial_dma_rx_deinit(handle);
    furi_hal_serial_dma_tx_deinit(handle);

    furi_hal_serial_set_transfer_direction(handle, FuriHalSerialDirectionNone);

    switch(handle->id) {
    case FuriHalSerialIdUsart1:
        furi_hal_bus_disable(FuriHalBusUSART1);
        break;
    case FuriHalSerialIdUsart2:
        furi_hal_bus_disable(FuriHalBusUSART2);
        break;
    case FuriHalSerialIdUsart3:
        furi_hal_bus_disable(FuriHalBusUSART3);
        break;
    case FuriHalSerialIdUart4:
        furi_hal_bus_disable(FuriHalBusUART4);
        break;
    case FuriHalSerialIdUart5:
        furi_hal_bus_disable(FuriHalBusUART5);
        break;
    case FuriHalSerialIdUsart6:
        furi_hal_bus_disable(FuriHalBusUSART6);
        break;

    default:
        furi_crash("Invalid serial id");
        break;
    }

    free(serial);

    furi_hal_serial[serial_id] = NULL;
}

inline void furi_hal_serial_suspend(FuriHalSerialHandle* handle) {
    furi_hal_serial_check(handle);

    USART_TypeDef* periph = furi_hal_serial[handle->id]->periph_ptr;

    LL_USART_Disable(periph);

    while(LL_USART_IsActiveFlag_TEACK(periph) && LL_USART_IsActiveFlag_REACK(periph))
        ;
}

inline void furi_hal_serial_resume(FuriHalSerialHandle* handle) {
    furi_hal_serial_check(handle);

    USART_TypeDef* periph = furi_hal_serial[handle->id]->periph_ptr;

    LL_USART_Enable(periph);

    while(!LL_USART_IsActiveFlag_TEACK(periph) || !LL_USART_IsActiveFlag_REACK(periph))
        ;
}

bool furi_hal_serial_is_baud_rate_supported(FuriHalSerialHandle* handle, uint32_t baud_rate) {
    UNUSED(handle);
    return baud_rate >= 10 && baud_rate <= 20000000;
}

void furi_hal_serial_set_baud_rate(FuriHalSerialHandle* handle, uint32_t baud_rate) {
    furi_check(handle);
    furi_check(furi_hal_serial_is_baud_rate_supported(handle, baud_rate));

    FuriHalSerial* serial = furi_hal_serial[handle->id];
    furi_check(serial);

    uint32_t divisor = (SystemCoreClock / baud_rate);
    uint32_t prescaler = 0;
    uint32_t over_sampling = 0;

    if(baud_rate > 10000000) {
        over_sampling = LL_USART_OVERSAMPLING_8;
        divisor = (divisor / 8) >> 12;
    } else {
        over_sampling = LL_USART_OVERSAMPLING_16;
        divisor = (divisor / 16) >> 12;
    }

    if(divisor < 1) {
        prescaler = LL_USART_PRESCALER_DIV1;
    } else if(divisor < 2) {
        prescaler = LL_USART_PRESCALER_DIV2;
    } else if(divisor < 4) {
        prescaler = LL_USART_PRESCALER_DIV4;
    } else if(divisor < 6) {
        prescaler = LL_USART_PRESCALER_DIV6;
    } else if(divisor < 8) {
        prescaler = LL_USART_PRESCALER_DIV8;
    } else if(divisor < 10) {
        prescaler = LL_USART_PRESCALER_DIV10;
    } else if(divisor < 12) {
        prescaler = LL_USART_PRESCALER_DIV12;
    } else if(divisor < 16) {
        prescaler = LL_USART_PRESCALER_DIV16;
    } else if(divisor < 32) {
        prescaler = LL_USART_PRESCALER_DIV32;
    } else if(divisor < 64) {
        prescaler = LL_USART_PRESCALER_DIV64;
    } else if(divisor < 128) {
        prescaler = LL_USART_PRESCALER_DIV128;
    } else {
        prescaler = LL_USART_PRESCALER_DIV256;
    }

    USART_TypeDef* periph = serial->periph_ptr;

    LL_USART_SetOverSampling(periph, over_sampling);
    LL_USART_SetPrescaler(periph, prescaler);
    LL_USART_SetBaudRate(periph, SystemCoreClock, prescaler, over_sampling, baud_rate);
}

void furi_hal_serial_set_hw_flow_control(
    FuriHalSerialHandle* handle,
    FuriHalSerialHwFlowControl flow_control) {
    furi_check(handle);

    const FuriHalSerialResources* resources = &furi_hal_serial_resources[handle->id];

    const GpioPin* gpio_rts = resources->gpio[FuriHalSerialPinRts];
    const GpioPin* gpio_cts = resources->gpio[FuriHalSerialPinCts];
    const GpioAltFn alt_fn = resources->alt_fn;

    if(gpio_rts == NULL || gpio_cts == NULL) {
        // Assuming that both pins must be defined
        return;
    }

    uint32_t hw_flow_reg_value;

    if(flow_control == FuriHalSerialHwFlowControlNone) {
        hw_flow_reg_value = LL_USART_HWCONTROL_NONE;
        furi_hal_gpio_init_simple(gpio_rts, GpioModeAnalog);
        furi_hal_gpio_init_simple(gpio_cts, GpioModeAnalog);

    } else if(flow_control == FuriHalSerialHwFlowControlRts) {
        hw_flow_reg_value = LL_USART_HWCONTROL_RTS;
        furi_hal_gpio_init_ex(
            gpio_rts, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedLow, alt_fn);
        furi_hal_gpio_init_simple(gpio_cts, GpioModeAnalog);

    } else if(flow_control == FuriHalSerialHwFlowControlCts) {
        hw_flow_reg_value = LL_USART_HWCONTROL_CTS;
        furi_hal_gpio_init_simple(gpio_rts, GpioModeAnalog);
        furi_hal_gpio_init_ex(gpio_cts, GpioModeInput, GpioPullUp, GpioSpeedLow, alt_fn);

    } else if(flow_control == FuriHalSerialHwFlowControlRtsCts) {
        hw_flow_reg_value = LL_USART_HWCONTROL_RTS_CTS;
        furi_hal_gpio_init_ex(
            gpio_rts, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedLow, alt_fn);
        furi_hal_gpio_init_ex(
            gpio_cts, GpioModeAltFunctionPushPull, GpioPullUp, GpioSpeedLow, alt_fn);

    } else {
        furi_crash();
    }

    const bool is_enabled = furi_hal_serial_is_enabled(handle);
    if(is_enabled) {
        furi_hal_serial_suspend(handle);
    }

    LL_USART_SetHWFlowCtrl(resources->periph, hw_flow_reg_value);

    if(is_enabled) {
        furi_hal_serial_resume(handle);
    }
}

void furi_hal_serial_set_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialTxCallback tx_callback,
    FuriHalSerialRxCallback rx_callback,
    void* context) {
    furi_check(handle);

    FuriHalSerial* serial = furi_hal_serial[handle->id];
    furi_check(serial);

    serial->tx_callback = tx_callback;
    serial->rx_callback = rx_callback;
    serial->callback_context = context;
}

void furi_hal_serial_tx(FuriHalSerialHandle* handle, const uint8_t* buffer, size_t buffer_size) {
    furi_check(handle);

    USART_TypeDef* periph = furi_hal_serial_resources[handle->id].periph;

    while(buffer_size > 0) {
        while(!LL_USART_IsActiveFlag_TXE_TXFNF(periph))
            ;

        LL_USART_TransmitData8(periph, *buffer);

        buffer++;
        buffer_size--;
    }
}

void furi_hal_serial_tx_wait_complete(FuriHalSerialHandle* handle) {
    furi_check(handle);
    while(!LL_USART_IsActiveFlag_TXFE(furi_hal_serial_resources[handle->id].periph))
        ;
}

bool furi_hal_serial_rx_available(FuriHalSerialHandle* handle) {
    furi_check(handle);
    return LL_USART_IsActiveFlag_RXNE_RXFNE(furi_hal_serial_resources[handle->id].periph);
}

uint8_t furi_hal_serial_rx(FuriHalSerialHandle* handle) {
    furi_check(handle);
    return LL_USART_ReceiveData8(furi_hal_serial_resources[handle->id].periph);
}

void furi_hal_serial_async_rx_start(FuriHalSerialHandle* handle, bool report_errors) {
    furi_check(handle);
    const FuriHalSerialResources* resources = &furi_hal_serial_resources[handle->id];
    USART_TypeDef* periph = resources->periph;

    FURI_CRITICAL_ENTER();
    furi_hal_interrupt_set_isr(resources->irq, furi_hal_serial_irq_callback, handle);
    LL_USART_EnableIT_RXNE_RXFNE(periph);
    if(report_errors) {
        LL_USART_EnableIT_ERROR(periph);
    }
    FURI_CRITICAL_EXIT();
}

void furi_hal_serial_async_rx_stop(FuriHalSerialHandle* handle) {
    furi_check(handle);
    const FuriHalSerialResources* resources = &furi_hal_serial_resources[handle->id];
    USART_TypeDef* periph = resources->periph;

    FURI_CRITICAL_ENTER();
    LL_USART_DisableIT_RXNE_RXFNE(periph);
    LL_USART_DisableIT_ERROR(resources->periph);
    furi_hal_interrupt_set_isr(resources->irq, NULL, NULL);
    FURI_CRITICAL_EXIT();
}

void furi_hal_serial_dma_tx(FuriHalSerialHandle* handle, const uint8_t* buffer, size_t buffer_size) {
    furi_check(handle);

    FuriHalSerial* serial = furi_hal_serial[handle->id];
    furi_check(serial);

    USART_TypeDef* periph = serial->periph_ptr;
    const uint32_t dma_channel = serial->dma_tx_channel;

    FURI_CRITICAL_ENTER();
    LL_USART_DisableDMAReq_TX(periph);

    LL_DMA_DisableChannel(GPDMA1, dma_channel);
    LL_DMA_SetBlkDataLength(GPDMA1, dma_channel, buffer_size);
    LL_DMA_SetSrcAddress(GPDMA1, dma_channel, (uint32_t)buffer);
    LL_DMA_EnableChannel(GPDMA1, dma_channel);

    LL_USART_EnableDMAReq_TX(periph);
    FURI_CRITICAL_EXIT();
}

void furi_hal_serial_dma_rx_start(FuriHalSerialHandle* handle, uint8_t* buffer, size_t buffer_size) {
    furi_check(handle);
    furi_check(buffer);
    furi_check(buffer_size);

    FuriHalSerial* serial = furi_hal_serial[handle->id];
    furi_check(serial);

    USART_TypeDef* periph = serial->periph_ptr;
    const uint32_t dma_channel = serial->dma_rx_channel;

    FURI_CRITICAL_ENTER();
    LL_USART_DisableDMAReq_RX(periph);

    LL_DMA_DisableChannel(GPDMA1, dma_channel);
    LL_DMA_SetBlkDataLength(GPDMA1, dma_channel, buffer_size);
    LL_DMA_SetDestAddress(GPDMA1, dma_channel, (uint32_t)buffer);
    LL_DMA_EnableChannel(GPDMA1, dma_channel);

    LL_USART_EnableDMAReq_RX(periph);
    FURI_CRITICAL_EXIT();
}

void furi_hal_serial_dma_rx_stop(FuriHalSerialHandle* handle) {
    furi_check(handle);
    // TODO: Implement
}

void furi_hal_serial_clear(FuriHalSerialHandle* handle, FuriHalSerialDirection dir) {
    furi_check(handle);

    USART_TypeDef* periph = furi_hal_serial_resources[handle->id].periph;

    if(dir & FuriHalSerialDirectionTx) {
        LL_USART_RequestTxDataFlush(periph);
        while(!LL_USART_IsActiveFlag_TXFE(periph))
            ;
    }

    if(dir & FuriHalSerialDirectionRx) {
        LL_USART_RequestRxDataFlush(periph);
        while(LL_USART_IsActiveFlag_RXNE_RXFNE(periph))
            ;
    }
}

void furi_hal_serial_set_transfer_direction(
    FuriHalSerialHandle* handle,
    FuriHalSerialDirection dir) {
    furi_hal_serial_check(handle);

    const FuriHalSerialResources* resources = &furi_hal_serial_resources[handle->id];

    const GpioPin* gpio_tx = resources->gpio[FuriHalSerialPinTx];
    const GpioPin* gpio_rx = resources->gpio[FuriHalSerialPinRx];
    const GpioAltFn alt_fn = resources->alt_fn;

    uint32_t direction;

    switch(dir) {
    case FuriHalSerialDirectionNone:
        direction = LL_USART_DIRECTION_NONE;
        furi_hal_gpio_init(gpio_tx, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
        furi_hal_gpio_init(gpio_rx, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
        break;

    case FuriHalSerialDirectionTx:
        direction = LL_USART_DIRECTION_TX;
        furi_hal_gpio_init_ex(
            gpio_tx, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedHigh, alt_fn);
        furi_hal_gpio_init(gpio_rx, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
        break;

    case FuriHalSerialDirectionRx:
        direction = LL_USART_DIRECTION_RX;
        furi_hal_gpio_init(gpio_tx, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
        furi_hal_gpio_init_ex(
            gpio_rx, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedHigh, alt_fn);
        break;

    case FuriHalSerialDirectionTxRx:
        direction = LL_USART_DIRECTION_TX_RX;
        furi_hal_gpio_init_ex(
            gpio_tx, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedHigh, alt_fn);
        furi_hal_gpio_init_ex(
            gpio_rx, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedHigh, alt_fn);
        break;

    default:
        furi_crash("Serial: Invalid transfer direction");
        break;
    }

    LL_USART_SetTransferDirection(furi_hal_serial[handle->id]->periph_ptr, direction);
}

const GpioPin* furi_hal_serial_gpio_get_pin(FuriHalSerialHandle* handle, FuriHalSerialPin pin) {
    furi_hal_serial_check(handle);
    return furi_hal_serial_resources[handle->id].gpio[pin];
}

void furi_hal_serial_set_config(
    FuriHalSerialHandle* handle,
    FuriHalSerialConfigDataBits data_bits,
    FuriHalSerialConfigParity parity,
    FuriHalSerialConfigStopBits stop_bits) {
    furi_hal_serial_check(handle);
    uint32_t data_width = LL_USART_DATAWIDTH_8B;
    uint32_t parity_mode = LL_USART_PARITY_NONE;
    uint32_t stop_bits_mode = LL_USART_STOPBITS_1;
    bool is_enabled = furi_hal_serial_is_enabled(handle);

    switch(data_bits) {
    case FuriHalSerialConfigDataBits7:
        data_width = LL_USART_DATAWIDTH_7B;
        break;
    case FuriHalSerialConfigDataBits8:
        data_width = LL_USART_DATAWIDTH_8B;
        break;
    case FuriHalSerialConfigDataBits9:
        data_width = LL_USART_DATAWIDTH_9B;
        break;
    default:
        furi_crash("Serial: Invalid data bits");
        break;
    }

    switch(parity) {
    case FuriHalSerialConfigParityNone:
        parity_mode = LL_USART_PARITY_NONE;
        break;
    case FuriHalSerialConfigParityEven:
        parity_mode = LL_USART_PARITY_EVEN;
        break;
    case FuriHalSerialConfigParityOdd:
        parity_mode = LL_USART_PARITY_ODD;
        break;
    default:
        furi_crash("Serial: Invalid parity");
        break;
    }

    switch(stop_bits) {
    case FuriHalSerialConfigStopBits_0_5:
        stop_bits_mode = LL_USART_STOPBITS_0_5;
        break;
    case FuriHalSerialConfigStopBits_1:
        stop_bits_mode = LL_USART_STOPBITS_1;
        break;
    case FuriHalSerialConfigStopBits_1_5:
        stop_bits_mode = LL_USART_STOPBITS_1_5;
        break;
    case FuriHalSerialConfigStopBits_2:
        stop_bits_mode = LL_USART_STOPBITS_2;
        break;
    default:
        furi_crash("Serial: Invalid stop bits");
        break;
    }

    if(is_enabled) {
        furi_hal_serial_suspend(handle);
    }

    if(parity != FuriHalSerialConfigParityNone) {
        LL_USART_EnableIT_PE(furi_hal_serial[handle->id]->periph_ptr);
    } else {
        LL_USART_DisableIT_PE(furi_hal_serial[handle->id]->periph_ptr);
    }

    LL_USART_ConfigCharacter(
        furi_hal_serial[handle->id]->periph_ptr, data_width, parity_mode, stop_bits_mode);
    if(is_enabled) {
        furi_hal_serial_resume(handle);
    }
}

void furi_hal_serial_tx_rx_swap(FuriHalSerialHandle* handle, bool enable) {
    furi_hal_serial_check(handle);

    bool is_enabled = furi_hal_serial_is_enabled(handle);
    if(is_enabled) {
        furi_hal_serial_suspend(handle);
    }

    if(enable) {
        LL_USART_SetTXRXSwap(furi_hal_serial[handle->id]->periph_ptr, LL_USART_TXRX_SWAPPED);
    } else {
        LL_USART_SetTXRXSwap(furi_hal_serial[handle->id]->periph_ptr, LL_USART_TXRX_STANDARD);
    }

    if(is_enabled) {
        furi_hal_serial_resume(handle);
    }
}

void furi_hal_serial_rx_level_inverted(FuriHalSerialHandle* handle, bool enable) {
    furi_hal_serial_check(handle);

    bool is_enabled = furi_hal_serial_is_enabled(handle);
    if(is_enabled) {
        furi_hal_serial_suspend(handle);
    }

    if(enable) {
        LL_USART_SetRXPinLevel(
            furi_hal_serial[handle->id]->periph_ptr, LL_USART_RXPIN_LEVEL_INVERTED);
    } else {
        LL_USART_SetRXPinLevel(
            furi_hal_serial[handle->id]->periph_ptr, LL_USART_RXPIN_LEVEL_STANDARD);
    }

    if(is_enabled) {
        furi_hal_serial_resume(handle);
    }
}

void furi_hal_serial_tx_level_inverted(FuriHalSerialHandle* handle, bool enable) {
    furi_hal_serial_check(handle);

    bool is_enabled = furi_hal_serial_is_enabled(handle);
    if(is_enabled) {
        furi_hal_serial_suspend(handle);
    }

    if(enable) {
        LL_USART_SetTXPinLevel(
            furi_hal_serial[handle->id]->periph_ptr, LL_USART_TXPIN_LEVEL_INVERTED);
    } else {
        LL_USART_SetTXPinLevel(
            furi_hal_serial[handle->id]->periph_ptr, LL_USART_TXPIN_LEVEL_STANDARD);
    }

    if(is_enabled) {
        furi_hal_serial_resume(handle);
    }
}

void furi_hal_serial_set_transfer_bit_order(
    FuriHalSerialHandle* handle,
    FuriHalSerialTransferBitOrder bit_order) {
    furi_hal_serial_check(handle);

    bool is_enabled = furi_hal_serial_is_enabled(handle);
    if(is_enabled) {
        furi_hal_serial_suspend(handle);
    }

    switch(bit_order) {
    case FuriHalSerialTransferBitOrderLsbFirst:
        LL_USART_SetTransferBitOrder(
            furi_hal_serial[handle->id]->periph_ptr, LL_USART_BITORDER_LSBFIRST);
        break;
    case FuriHalSerialTransferBitOrderMsbFirst:
        LL_USART_SetTransferBitOrder(
            furi_hal_serial[handle->id]->periph_ptr, LL_USART_BITORDER_MSBFIRST);
        break;

    default:
        furi_crash("Serial: Invalid transfer bit order");
        break;
    }

    if(is_enabled) {
        furi_hal_serial_resume(handle);
    }
}

void furi_hal_serial_set_binary_data_logic(
    FuriHalSerialHandle* handle,
    FuriHalSerialBinaryDataLogic binary_data_logic) {
    furi_hal_serial_check(handle);

    bool is_enabled = furi_hal_serial_is_enabled(handle);
    if(is_enabled) {
        furi_hal_serial_suspend(handle);
    }

    switch(binary_data_logic) {
    case FuriHalSerialBinaryDataLogicPositive:
        LL_USART_SetBinaryDataLogic(
            furi_hal_serial[handle->id]->periph_ptr, LL_USART_BINARY_LOGIC_POSITIVE);
        break;
    case FuriHalSerialBinaryDataLogicNegative:
        LL_USART_SetBinaryDataLogic(
            furi_hal_serial[handle->id]->periph_ptr, LL_USART_BINARY_LOGIC_NEGATIVE);
        break;

    default:
        furi_crash("Serial: Invalid binary data logic");
        break;
    }

    if(is_enabled) {
        furi_hal_serial_resume(handle);
    }
}

inline bool furi_hal_serial_is_enabled(FuriHalSerialHandle* handle) {
    furi_hal_serial_check(handle);
    return LL_USART_IsEnabled(furi_hal_serial[handle->id]->periph_ptr);
}
