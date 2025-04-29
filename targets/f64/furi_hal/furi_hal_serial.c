#include <furi_hal_serial.h>
#include <furi_hal_serial_types_i.h>

#include <furi_hal_resources.h>
#include <furi_hal_cortex.h>
#include <furi_hal_bus.h>
#include <furi_hal_dma.h>

#include <si91x_device.h>

#define FRAC_BITS       (6UL)
#define FRAC_MULTIPLIER (1UL << FRAC_BITS)
#define FRAC_MASK       (FRAC_MULTIPLIER - 1UL)

#define LCR_DLAB_POS (7)
#define LCR_DLAB_SET (1U << LCR_DLAB_POS)
#define LCR_DLS_POS  (0)
#define LCR_DLS_8BIT (3UL << LCR_DLS_POS)

#define FCR_FIFOE_POS      (0)
#define FCR_FIFOE_SET      (1UL << FCR_FIFOE_POS)
#define FCR_FIFOE_CLR      (0UL << FCR_FIFOE_POS)
#define FCR_RFIFOR_POS     (1)
#define FCR_RFIFOR_SET     (1UL << FCR_RFIFOR_POS)
#define FCR_XFIFOR_POS     (2)
#define FCR_XFIFOR_SET     (1UL << FCR_XFIFOR_POS)
#define FCR_DMAM_POS       (3)
#define FCR_DMAM_SET       (1UL << FCR_DMAM_POS)
#define FCR_DMAM_CLR       (0UL << FCR_DMAM_POS)
#define FCR_RT_POS         (6)
#define FCR_RT_ONE_CHAR    (0UL << FCR_RT_POS)
#define FCR_RT_HALF_FULL   (2UL << FCR_RT_POS)
#define FCR_RT_ALMOST_FULL (3UL << FCR_RT_POS)

#define IER_ERBFI_POS (0)
#define IER_ERBFI_SET (1UL << IER_ERBFI_POS)
#define IER_ELSI_POS  (2)
#define IER_ELSI_SET  (1UL << IER_ELSI_POS)
#define IER_CLEAR_ALL (0UL)

#define LSR_OE_POS (1)
#define LSR_OE_SET (1UL << LSR_OE_POS)
#define LSR_FE_POS (3)
#define LSR_FE_SET (1UL << LSR_FE_POS)
#define LSR_BI_POS (4)
#define LSR_BI_SET (1UL << LSR_BI_POS)

#define MCR_RTS_POS  (1)
#define MCR_RTS_SET  (1UL << MCR_RTS_POS)
#define MCR_RTS_CLR  (0UL << MCR_RTS_POS)
#define MCR_AFCE_POS (5)
#define MCR_AFCE_SET (1UL << MCR_AFCE_POS)
#define MCR_AFCE_CLR (0UL << MCR_AFCE_POS)

#define IIR_IID_MASK         (0xFUL)
#define IIR_IID_MODEM_STATUS (0x0)
#define IIR_IID_NO_INTERRUPT (0x1)
#define IIR_IID_THR_EMPTY    (0x2)
#define IIR_IID_RX_AVAILABLE (0x4)
#define IIR_IID_LINE_STATUS  (0x6)
#define IIR_IID_BUSY_DETECT  (0x7)
#define IIR_IID_CHAR_TIMEOUT (0xC)

typedef struct {
    FuriHalSerialHandle* handle;
    FuriHalSerialRxCallback rx_callback;
    FuriHalSerialTxCallback tx_callback;
    void* callback_context;
} FuriHalSerial;

typedef struct {
    USART0_Type* periph;
    uint16_t dma_rx_channel;
    uint16_t dma_tx_channel;
    IRQn_Type irqn;
} FuriHalSerialResources;

static const FuriHalSerialResources furi_hal_serial_resources[FuriHalSerialIdMax] = {
    [FuriHalSerialIdUsart0] =
        {
            .periph = UART0,
            .dma_rx_channel = FuriHalDmaChannelSrcUsart0,
            .dma_tx_channel = FuriHalDmaChannelDstUsart0,
            .irqn = USART0_IRQn,
        },
    [FuriHalSerialIdUart1] =
        {
            .periph = UART1,
            .dma_rx_channel = FuriHalDmaChannelSrcUart1,
            .dma_tx_channel = FuriHalDmaChannelSrcUart1,
            .irqn = UART1_IRQn,
        },
    [FuriHalSerialIdUlpuart] =
        {
            .periph = ULP_UART,
            .dma_rx_channel = UINT16_MAX, // Not yet implemented
            .dma_tx_channel = UINT16_MAX, // Not yet implemented
            .irqn = ULPSS_UART_IRQn,
        },
};

static FuriHalSerial furi_hal_serial[FuriHalSerialIdMax];

static void furi_hal_serial_enable_fifo(FuriHalSerialHandle* handle) {
    USART0_Type* periph = furi_hal_serial_resources[handle->id].periph;
    periph->FCR = FCR_RT_ONE_CHAR | FCR_DMAM_SET | FCR_FIFOE_SET;
}

static void furi_hal_serial_disable_fifo(FuriHalSerialHandle* handle) {
    USART0_Type* periph = furi_hal_serial_resources[handle->id].periph;
    periph->FCR_b.FIFOE = 0;
}

void furi_hal_serial_init(FuriHalSerialHandle* handle, uint32_t baud) {
    furi_check(handle);

    FuriHalSerial* serial = &furi_hal_serial[handle->id];
    furi_check(serial->handle == NULL);

    serial->handle = handle;

    if(handle->id == FuriHalSerialIdUsart0) {
        furi_hal_bus_enable(FuriHalBusUSART1_PCLK);
        furi_hal_bus_enable(FuriHalBusUSART1_SCLK);

        // TODO: Prettify clock selection
        // Select SOC PLL clock
        M4CLK->CLK_CONFIG_REG2_b.USART1_SCLK_SEL = 0x01;
        // Wait for the switch to complete
        while((M4CLK->PLL_STAT_REG_b.USART1_SCLK_SWITCHED) != 1)
            ;
        // No clock division
        M4CLK->CLK_CONFIG_REG2_b.USART1_SCLK_DIV_FAC = 0;

        // Init main pins
        furi_hal_gpio_init_ex(
            &gpio_usart0_rx, GpioModeInput, GpioPullUp, GpioSpeedHigh, GpioAltFn2USART0_RX);
        furi_hal_gpio_init_ex(
            &gpio_usart0_tx,
            GpioModeOutputPushPull,
            GpioPullNo,
            GpioSpeedHigh,
            GpioAltFn2USART0_TX);

    } else if(handle->id == FuriHalSerialIdUart1) {
        furi_hal_bus_enable(FuriHalBusUSART2_PCLK);
        furi_hal_bus_enable(FuriHalBusUSART2_SCLK);

        // TODO: Prettify clock selection
        // Select SOC PLL clock
        M4CLK->CLK_CONFIG_REG2_b.USART2_SCLK_SEL = 0x01;
        // Wait for the switch to complete
        while((M4CLK->PLL_STAT_REG_b.USART2_SCLK_SWITCHED) != 1)
            ;
        // No clock division
        M4CLK->CLK_CONFIG_REG2_b.USART2_SCLK_DIV_FAC = 0;

        // Init main pins
        furi_hal_gpio_init_ex(
            &gpio_uart1_rx,
            GpioModeInput,
            GpioPullUp,
            GpioSpeedHigh,
            GpioAltFn6SOCPERH_ON_ULP_GPIO_8);
        furi_hal_gpio_init_ex(
            &gpio_uart1_tx,
            GpioModeOutputPushPull,
            GpioPullNo,
            GpioSpeedHigh,
            GpioAltFn6SOCPERH_ON_ULP_GPIO_11);
        // Init virtual (multiplexed) pins
        furi_hal_gpio_init_ex(
            &gpio_i_uart1_rx, GpioModeInput, GpioPullNo, GpioSpeedHigh, GpioAltFn6UART1_RX);
        furi_hal_gpio_init_ex(
            &gpio_i_uart1_tx,
            GpioModeOutputPushPull,
            GpioPullNo,
            GpioSpeedHigh,
            GpioAltFn9UART1_TX);

    } else if(handle->id == FuriHalSerialIdUlpuart) {
        furi_hal_bus_enable(FuriHalBusUlpPCLK_UART);
        furi_hal_bus_enable(FuriHalBusUlpSCLK_UART);

        // TODO: This should be elsewhere (not here)
        // Enable ULP clock from HP domain
        furi_hal_bus_enable(FuriHalBusULPSS_CLK);

        // TODO: Prettify clock selection
        // Select HP to ULP clock
        ULPCLK->ULP_UART_CLK_GEN_REG_b.ULP_UART_CLK_SEL = 6;
        // No clock division
        M4CLK->CLK_CONFIG_REG4_b.ULPSS_CLK_DIV_FAC = 0;

        // Init main pins
        furi_hal_gpio_init_ex(
            &gpio_ulp_uart_rx,
            GpioModeInput,
            GpioPullUp,
            GpioSpeedHigh,
            GpioAltFn9ULPPERH_ON_SOC_GPIO_2);
        furi_hal_gpio_init_ex(
            &gpio_ulp_uart_tx,
            GpioModeOutputPushPull,
            GpioPullNo,
            GpioSpeedHigh,
            GpioAltFn9ULPPERH_ON_SOC_GPIO_3);
        // Init virtual (multiplexed) pins
        furi_hal_gpio_enable_ulp_on_hp(&gpio_ulp_2, GpioAltFn3ULP_UART_RX);
        furi_hal_gpio_enable_ulp_on_hp(&gpio_ulp_i_3, GpioAltFn3ULP_UART_TX);

    } else {
        furi_crash("Invalid serial id");
    }

    furi_hal_serial_set_baud_rate(handle, baud);
    furi_hal_serial_enable_fifo(handle);
    furi_hal_serial_set_hw_flow_control(handle, FuriHalSerialHwFlowControlNone);
    furi_hal_serial_clear(handle, FuriHalSerialDirectionTxRx);
}

void furi_hal_serial_deinit(FuriHalSerialHandle* handle) {
    furi_check(handle);
    furi_hal_serial_disable_fifo(handle);

    if(handle->id == FuriHalSerialIdUsart0) {
        furi_hal_bus_disable(FuriHalBusUSART1_PCLK);
        furi_hal_bus_disable(FuriHalBusUSART1_SCLK);

    } else if(handle->id == FuriHalSerialIdUart1) {
        furi_hal_bus_disable(FuriHalBusUSART2_PCLK);
        furi_hal_bus_disable(FuriHalBusUSART2_SCLK);

    } else if(handle->id == FuriHalSerialIdUlpuart) {
        furi_hal_bus_disable(FuriHalBusULPSS_CLK);
        furi_hal_bus_disable(FuriHalBusUlpSCLK_UART);
        furi_hal_bus_disable(FuriHalBusUlpPCLK_UART);

    } else {
        furi_crash("Invalid serial id");
    }
    furi_hal_serial[handle->id].handle = NULL;
}

bool furi_hal_serial_is_baud_rate_supported(FuriHalSerialHandle* handle, uint32_t baud_rate) {
    furi_check(handle);
    return baud_rate >= 9600UL && baud_rate <= 11250000UL;
}

void furi_hal_serial_set_baud_rate(FuriHalSerialHandle* handle, uint32_t baud_rate) {
    furi_check(handle);

    USART0_Type* periph = furi_hal_serial_resources[handle->id].periph;

    /*
     * Integer part:
     *   divisor = PCLK / (baud * 16)
     * Fractional part:
     *   6 bits (1/64...63/64)
     * Multiply both sides by 64:
     *   divisor_64 = (PCLK * 4) / baud
     */

    const uint32_t divisor_64 = (SystemCoreClock * (FRAC_MULTIPLIER / 16)) / baud_rate;
    const uint32_t divisor = divisor_64 >> FRAC_BITS;

    // Enable divisor modification
    periph->LCR = LCR_DLAB_SET;
    // Divisor low 8 bits
    periph->DLL = divisor & 0xFF;
    // Divisor high 8 bits
    periph->DLH = divisor >> 8;
    // Fractional part 6 bits
    periph->DLF = divisor_64 & FRAC_MASK;
    // Disable divisor modification and use 8bit per character
    periph->LCR = LCR_DLS_8BIT;
}

void furi_hal_serial_set_hw_flow_control(
    FuriHalSerialHandle* handle,
    FuriHalSerialHwFlowControl flow_control) {
    furi_check(handle);

    const FuriHalSerialId serial_id = handle->id;
    USART0_Type* periph = furi_hal_serial_resources[serial_id].periph;

    if(flow_control == FuriHalSerialHwFlowControlNone) {
        periph->MCR = MCR_RTS_CLR | MCR_AFCE_CLR;

        if(serial_id == FuriHalSerialIdUsart0) {
            furi_hal_gpio_init_simple(&gpio_usart0_cts, GpioModeInput);
            furi_hal_gpio_init_simple(&gpio_usart0_rts, GpioModeInput);

        } else if(serial_id == FuriHalSerialIdUart1) {
            // No pins defined for hardware flow control
        } else if(serial_id == FuriHalSerialIdUlpuart) {
            // No pins defined for hardware flow control
        } else {
            furi_crash();
        }

    } else if(flow_control == FuriHalSerialHwFlowControlRtsCts) {
        periph->MCR = MCR_RTS_SET | MCR_AFCE_SET;

        if(serial_id == FuriHalSerialIdUsart0) {
            furi_hal_gpio_init_ex(
                &gpio_usart0_cts, GpioModeInput, GpioPullUp, GpioSpeedHigh, GpioAltFn2USART0_CTS);
            furi_hal_gpio_init_ex(
                &gpio_usart0_rts,
                GpioModeOutputPushPull,
                GpioPullNo,
                GpioSpeedHigh,
                GpioAltFn2USART0_RTS);

        } else if(serial_id == FuriHalSerialIdUart1) {
            // No pins defined for hardware flow control
        } else if(serial_id == FuriHalSerialIdUlpuart) {
            // No pins defined for hardware flow control
        } else {
            furi_crash();
        }

    } else {
        furi_crash();
    }
}

void furi_hal_serial_set_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialTxCallback tx_callback,
    FuriHalSerialRxCallback rx_callback,
    void* context) {
    furi_check(handle);

    FuriHalSerial* serial = &furi_hal_serial[handle->id];

    serial->tx_callback = tx_callback;
    serial->rx_callback = rx_callback;
    serial->callback_context = context;
}

void furi_hal_serial_suspend(FuriHalSerialHandle* handle) {
    furi_check(handle);
}

void furi_hal_serial_resume(FuriHalSerialHandle* handle) {
    furi_check(handle);
}

void furi_hal_serial_tx(FuriHalSerialHandle* handle, const uint8_t* buffer, size_t buffer_size) {
    furi_check(handle);
    furi_check(buffer);
    furi_check(buffer_size);

    USART0_Type* periph = furi_hal_serial_resources[handle->id].periph;

    while(buffer_size > 0) {
        while(!periph->USR_b.TFNF)
            ;

        periph->THR = *buffer;

        ++buffer;
        --buffer_size;
    }
}

bool furi_hal_serial_tx_wait_complete(FuriHalSerialHandle* handle, uint32_t timeout) {
    furi_check(handle);

    bool success = false;

    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(timeout * 1000);

    while(!furi_hal_cortex_timer_is_expired(timer)) {
        if(furi_hal_serial_resources[handle->id].periph->USR_b.TFE) {
            success = true;
            break;
        }
    }

    return success;
}

bool furi_hal_serial_rx_available(FuriHalSerialHandle* handle) {
    furi_check(handle->id < FuriHalSerialIdMax);
    return furi_hal_serial_resources[handle->id].periph->USR_b.RFNE;
}

uint8_t furi_hal_serial_rx(FuriHalSerialHandle* handle) {
    furi_check(handle->id < FuriHalSerialIdMax);
    return furi_hal_serial_resources[handle->id].periph->RBR;
}

void furi_hal_serial_async_rx_start(FuriHalSerialHandle* handle, bool report_errors) {
    furi_check(handle);

    const FuriHalSerialResources* resources = &furi_hal_serial_resources[handle->id];
    resources->periph->IER = report_errors ? (IER_ELSI_SET | IER_ERBFI_SET) : IER_ERBFI_SET;

    NVIC_SetPriority(resources->irqn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 10, 0));
    NVIC_EnableIRQ(resources->irqn);
}

void furi_hal_serial_async_rx_stop(FuriHalSerialHandle* handle) {
    furi_check(handle);

    const FuriHalSerialResources* resources = &furi_hal_serial_resources[handle->id];
    resources->periph->IER = IER_CLEAR_ALL;

    NVIC_DisableIRQ(resources->irqn);
}

static void furi_hal_serial_dma_tx_irq_callback(void* context) {
    FuriHalSerialHandle* handle = context;
    FuriHalSerial* serial = &furi_hal_serial[handle->id];

    if(serial->tx_callback) {
        serial->tx_callback(handle, FuriHalSerialTxEventComplete, serial->callback_context);
    }
}

void furi_hal_serial_dma_tx(FuriHalSerialHandle* handle, const uint8_t* buffer, size_t buffer_size) {
    furi_check(handle);
    furi_check(buffer);
    furi_check(buffer_size);
    furi_check(buffer_size <= FURI_HAL_DMA_MAX_TRANSFER_COUNT);

    const FuriHalSerialResources* resources = &furi_hal_serial_resources[handle->id];

    furi_hal_dma_set_callback(
        resources->dma_tx_channel, furi_hal_serial_dma_tx_irq_callback, handle);

    const FuriHalDmaTransfer transfer = {
        .src_address = (uint32_t)buffer,
        .dst_address = (uint32_t)&resources->periph->THR,
        .count = buffer_size,
        .type = FuriHalDmaTransferTypeSimple,
        .src_width = FuriHalDmaDataWidth8,
        .src_increment = FuriHalDmaAddressIncrement8,
        .dst_width = FuriHalDmaDataWidth8,
        .dst_increment = FuriHalDmaAddressIncrementNone,
    };

    furi_hal_dma_init_channel(resources->dma_tx_channel, &transfer);
}

static void furi_hal_serial_dma_rx_irq_callback(void* context) {
    FuriHalSerialHandle* handle = context;
    FuriHalSerial* serial = &furi_hal_serial[handle->id];

    if(serial->rx_callback) {
        serial->rx_callback(handle, FuriHalSerialRxEventData, serial->callback_context);
    }
}

void furi_hal_serial_dma_rx_start(FuriHalSerialHandle* handle, uint8_t* buffer, size_t buffer_size) {
    furi_check(handle);
    furi_check(buffer);
    furi_check(buffer_size);
    furi_check(buffer_size <= FURI_HAL_DMA_MAX_TRANSFER_COUNT);

    const FuriHalSerialResources* resources = &furi_hal_serial_resources[handle->id];

    furi_hal_dma_set_callback(
        resources->dma_rx_channel, furi_hal_serial_dma_rx_irq_callback, handle);

    const FuriHalDmaTransfer transfer = {
        .src_address = (uint32_t)&resources->periph->RBR,
        .dst_address = (uint32_t)buffer,
        .count = buffer_size,
        .type = FuriHalDmaTransferTypeSimple,
        .src_width = FuriHalDmaDataWidth8,
        .src_increment = FuriHalDmaAddressIncrementNone,
        .dst_width = FuriHalDmaDataWidth8,
        .dst_increment = FuriHalDmaAddressIncrement8,
    };

    furi_hal_dma_init_channel(resources->dma_rx_channel, &transfer);
}

void furi_hal_serial_dma_rx_stop(FuriHalSerialHandle* handle) {
    furi_check(handle);
    furi_hal_dma_deinit_channel(furi_hal_serial_resources[handle->id].dma_rx_channel);
}

void furi_hal_serial_clear(FuriHalSerialHandle* handle, FuriHalSerialDirection dir) {
    furi_check(handle);

    USART0_Type* periph = furi_hal_serial_resources[handle->id].periph;

    if(dir & FuriHalSerialDirectionTx) {
        periph->FCR |= FCR_XFIFOR_SET;
        while(!periph->USR_b.TFE)
            ;
    }

    if(dir & FuriHalSerialDirectionRx) {
        periph->FCR |= FCR_RFIFOR_SET;
        while(periph->USR_b.RFNE)
            ;
    }
}

FURI_ALWAYS_INLINE static void furi_hal_serial_irq_handler(FuriHalSerialId serial_id) {
    FuriHalSerial* serial = &furi_hal_serial[serial_id];
    USART0_Type* periph = furi_hal_serial_resources[serial_id].periph;

    for(;;) {
        FuriHalSerialRxEvent event = 0;
        const uint32_t iid_value = periph->IIR & IIR_IID_MASK;

        if(iid_value == IIR_IID_NO_INTERRUPT) {
            // No interrupts left to process, exit handler
            break;
        } else if(iid_value == IIR_IID_RX_AVAILABLE) {
            event = FuriHalSerialRxEventData;
        } else if(iid_value == IIR_IID_CHAR_TIMEOUT) {
            event = FuriHalSerialRxEventIdle;
        } else if(iid_value == IIR_IID_LINE_STATUS) {
            const uint32_t lsr_value = periph->LSR;

            if(lsr_value & LSR_OE_SET) {
                event |= FuriHalSerialRxEventOverrunError;
            }
            if(lsr_value & LSR_FE_SET) {
                event |= FuriHalSerialRxEventFrameError;
            }
            if(lsr_value & LSR_BI_SET) {
                event |= FuriHalSerialRxEventBreak;
            }
        } else {
            furi_crash();
        }

        serial->rx_callback(serial->handle, event, serial->callback_context);
    }
}

void IRQ012_Handler(void) {
    furi_hal_serial_irq_handler(FuriHalSerialIdUlpuart);
}

void IRQ038_Handler(void) {
    furi_hal_serial_irq_handler(FuriHalSerialIdUsart0);
}

void IRQ039_Handler(void) {
    furi_hal_serial_irq_handler(FuriHalSerialIdUart1);
}
