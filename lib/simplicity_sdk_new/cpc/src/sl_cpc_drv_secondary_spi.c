/***************************************************************************/ /**
 * @file
 * @brief CPC SPI SECONDARY implementation.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifdef CPC_TEST_SPI_DRIVER_CRC_ERROR_INJECTION
#include "stdio.h"
#endif

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif
#include "sl_status.h"
#include "sl_atomic.h"
#include "sl_slist.h"
#include "sl_common.h"
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif
#include "sl_device_peripheral.h"
#include "sl_clock_manager.h"

#if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
#include "sl_cpc_drv_secondary_spi_hw_crc_config.h"
#endif
#include "dmadrv.h"

#include "sli_cpc.h"
#include "sli_cpc_assert.h"
#include "sli_cpc_drv.h"
#include "sli_cpc_hdlc.h"
#include "sli_cpc_crc.h"
#include "sli_cpc_debug.h"
#include "sl_cpc_config.h"
#include "sl_cpc_drv_secondary_spi_config.h"

#include "sl_cpc_instance_handles.h"

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

#if !defined(SL_CPC_DRV_SPI_IS_EUSART) && !defined(SL_CPC_DRV_SPI_IS_USART) && !defined(SL_CPC_DRV_SPI_IS_HSPI)
#error "SPI driver needs to be configured for EUSART, USART, or HSPI"
#endif

#if !defined(_SILICON_LABS_32B_SERIES_2) && !defined(_SILICON_LABS_32B_SERIES_3)
#error "This driver is only compatible with Series 2 & 3"
#endif

#if defined(SL_CPC_DRV_SPI_IS_HSPI)
#define HSPI_TARGET_MODE_MAX_BITRATE (100000000U)  // 100 MHz SDR for target mode
#endif

#if (SL_CPC_DRV_SPI_RX_BUFFER_MAX_COUNT < 3)
#error  Invalid configuration SL_CPC_DRV_SPI_RX_BUFFER_MAX_COUNT must be at least 3
#endif

// Series 2 compatibility layer
#if defined(_SILICON_LABS_32B_SERIES_2)
#include "em_ldma.h"
#include "em_gpio.h"
#include "em_prs.h"
#if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
#include "em_gpcrc.h"
#endif

#if defined(SL_CPC_DRV_SPI_IS_EUSART)

#define PRS_SIGNAL_EXTI(cs_pin_no)          SL_CONCAT_PASTER_2(prsSignalGPIO_PIN, cs_pin_no)
#define PRS_SIGNAL_SPI(periph_no, signal) \
  SL_CONCAT_PASTER_4(prsSignalEUSART, periph_no, _, signal)
#define SPI_PERIPHERAL(periph_no)           SL_CONCAT_PASTER_2(SL_PERIPHERAL_EUSART, periph_no)
#define LDMA_SIGNAL_TX(periph_no)           SL_CONCAT_PASTER_3(dmadrvPeripheralSignal_EUSART, periph_no, _TXBL)
#define LDMA_SIGNAL_RX(periph_no)           SL_CONCAT_PASTER_3(dmadrvPeripheralSignal_EUSART, periph_no, _RXDATAV)
#define GPIO_PORT_SHIFT(route)              SL_CONCAT_PASTER_3(_GPIO_EUSART_, route, _PORT_SHIFT)
#define GPIO_PIN_SHIFT(route)               SL_CONCAT_PASTER_3(_GPIO_EUSART_, route, _PIN_SHIFT)
#define GPIO_ROUTEEN                        (GPIO_EUSART_ROUTEEN_TXPEN | GPIO_EUSART_ROUTEEN_RXPEN | GPIO_EUSART_ROUTEEN_SCLKPEN | GPIO_EUSART_ROUTEEN_CSPEN)
#define SPI_ROUTE                           EUSARTROUTE
#define EUSART_TX_IRQHandler(periph_no)     SL_CONCAT_PASTER_3(EUSART, periph_no, _TX_IRQHandler)
#define EUSART_TX_IRQn(periph_no)           SL_CONCAT_PASTER_3(EUSART, periph_no, _TX_IRQn)

#elif defined(SL_CPC_DRV_SPI_IS_USART) // EUSART

#define PRS_SIGNAL_SPI(periph, signal) \
  SL_CONCAT_PASTER_4(prsSignalUSART, periph, _, signal)
#define SPI_PERIPHERAL(periph_no)           SL_CONCAT_PASTER_2(SL_PERIPHERAL_USART, periph_no)
#define LDMA_SIGNAL_TX(periph_no)           SL_CONCAT_PASTER_3(dmadrvPeripheralSignal_USART, periph_no, _TXBL)
#define LDMA_SIGNAL_RX(periph_no)           SL_CONCAT_PASTER_3(dmadrvPeripheralSignal_USART, periph_no, _RXDATAV)
#define GPIO_PORT_SHIFT(route)              SL_CONCAT_PASTER_3(_GPIO_USART_, route, _PORT_SHIFT)
#define GPIO_PIN_SHIFT(route)               SL_CONCAT_PASTER_3(_GPIO_USART_, route, _PIN_SHIFT)
#define GPIO_ROUTEEN                        (GPIO_USART_ROUTEEN_TXPEN | GPIO_USART_ROUTEEN_RXPEN | GPIO_USART_ROUTEEN_CLKPEN | GPIO_USART_ROUTEEN_CSPEN)
#define SPI_ROUTE                           USARTROUTE

#endif // USART

// PRS
typedef PRS_Signal_t prs_signal_t;
#define PRS_ASYNC_CONNECT_PRODUCER(ch, sig) PRS_ConnectSignal(ch, prsTypeAsync, sig)
#define PRS_SIGNAL_NONE                     prsSignalNone

// LDMA
#define LDMA_PERIPH                         LDMA
typedef LDMA_TransferCfg_t ldma_transfert_cfg_t;
#define LDMA_TRANSFER_CFG_DBGHALT           ldmaDbgHalt
#define LDMA_TRANSFER_CFG_LOOP_COUNT        ldmaLoopCnt
typedef LDMA_Descriptor_t ldma_descriptor_t;
#define LDMA_DESCRIPTOR_XFER_CNT            xferCnt
#define LDMA_DESCRIPTOR_DONE_IFS            doneIfs
#define LDMA_DESCRIPTOR_SRC_ADDR            srcAddr
#define LDMA_DESCRIPTOR_DST_ADDR            dstAddr
#define LDMA_DESCRIPTOR_LINK_ADDR           linkAddr
#define LDMA_DESCRIPTOR_LINK_MODE           linkMode
#define LDMA_DESCRIPTOR_LINK_MODE_ABS       ldmaLinkModeAbs
#define LDMA_DESCRIPTOR_DEC_LOOP_CNT        decLoopCnt
void LDMA_CLEAR_CH_IRQ(uint8_t channel)
{
  /* Clear the interrupt flag. */
#if defined (LDMA_HAS_SET_CLEAR)
  LDMA->IF_CLR = (1 << channel);
#else
  LDMA->IFC = (1 << channel);
#endif
}

// GPIO
#define GPIO_MODE_PUSH_PULL                 gpioModePushPull
#define GPIO_MODE_INPUT                     gpioModeInput
#define GPIO_MODE_INPUT_PULL                gpioModeInputPull
#define GPIO_SET_PIN_MODE                   GPIO_PinModeSet
#define GPIO_CLR_OUT_PIN                    GPIO_PinOutClear
#define GPIO_SET_OUT_PIN                    GPIO_PinOutSet
#define GPIO_CONFIGURE_EXT_INT(port,        \
                               pin,         \
                               intNo,       \
                               risingEdge,  \
                               fallingEdge) \
  GPIO_ExtIntConfig(port, pin, intNo, risingEdge, fallingEdge, false)

#endif // Series 2

// Series 3 compatibility layer
#if defined(_SILICON_LABS_32B_SERIES_3)
#include "sl_hal_ldma.h"
#include "sl_hal_gpio.h"
#include "sl_hal_prs.h"
#if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
#include "sl_hal_gpcrc.h"
#endif
#if defined(SL_CPC_DRV_SPI_IS_HSPI)
#include "sl_hal_hspi.h"
#define SPI_PERIPHERAL(periph_no)           SL_CONCAT_PASTER_2(SL_PERIPHERAL_HSPI, periph_no)
#else
#define SPI_PERIPHERAL(periph_no)           SL_CONCAT_PASTER_2(SL_PERIPHERAL_EUSART, periph_no)
#endif

#define SPI_ROUTE                           EUSARTROUTE
#define EUSART_TX_IRQHandler(periph_no)     SL_CONCAT_PASTER_3(EUSART, periph_no, _TX_IRQHandler)
#define EUSART_TX_IRQn(periph_no)           SL_CONCAT_PASTER_3(EUSART, periph_no, _TX_IRQn)

// PRS
typedef sl_hal_prs_sync_producer_signal_t prs_signal_t;
#define PRS_ASYNC_CONNECT_PRODUCER          sl_hal_prs_async_connect_channel_producer
#define PRS_SIGNAL_NONE                     SL_HAL_PRS_ASYNC_NONE
#define PRS_SIGNAL_EXTI(cs_pin_no)          SL_CONCAT_PASTER_2(SL_HAL_PRS_ASYNC_GPIO_PIN, cs_pin_no)
#if defined(SL_CPC_DRV_SPI_IS_HSPI)
#define PRS_SIGNAL_SPI(periph_no, signal)   SL_CONCAT_PASTER_4(SL_HAL_PRS_ASYNC_HSPI, periph_no, L_, signal)
#else
#define PRS_SIGNAL_SPI(periph_no, signal)   SL_CONCAT_PASTER_4(SL_HAL_PRS_ASYNC_EUSART, periph_no, L_, signal)
#endif
#define PRS_TYPE_ASYNC                      SL_HAL_PRS_TYPE_ASYNC

// LDMA
#define LDMA_PERIPH                         LDMA(0)
#if defined(SL_CPC_DRV_SPI_IS_HSPI)
#define LDMA_SIGNAL_TX(periph_no)           SL_CONCAT_PASTER_3(dmadrvPeripheralSignal_HSPI, periph_no, _TX)
#define LDMA_SIGNAL_RX(periph_no)           SL_CONCAT_PASTER_3(dmadrvPeripheralSignal_HSPI, periph_no, _RX)
#else
#define LDMA_SIGNAL_TX(periph_no)           SL_CONCAT_PASTER_3(dmadrvPeripheralSignal_EUSART, periph_no, _TXBL)
#define LDMA_SIGNAL_RX(periph_no)           SL_CONCAT_PASTER_3(dmadrvPeripheralSignal_EUSART, periph_no, _RXDATAV)
#endif
typedef sl_hal_ldma_transfer_config_t ldma_transfert_cfg_t;
#define LDMA_TRANSFER_CFG_PERIPHERAL        SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL
#define LDMA_TRANSFER_CFG_MEMORY            SL_HAL_LDMA_TRANSFER_CFG_MEMORY
typedef sl_hal_ldma_descriptor_t ldma_descriptor_t;
#define LDMA_TRANSFER_CFG_DBGHALT           debug_halt_en
#define LDMA_TRANSFER_CFG_LOOP_COUNT        loop_count
#define LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR  SL_HAL_LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR
#define LDMA_DESCRIPTOR_LINKABS_SYNC        SL_HAL_LDMA_DESCRIPTOR_LINKABS_SYNC
#define LDMA_DESCRIPTOR_LINKABS_WRITE       SL_HAL_LDMA_DESCRIPTOR_LINKABS_WRITE
#define LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(src, dest, cnt, link_jmp)  SL_HAL_LDMA_DESCRIPTOR_LINKREL_M2P(SL_HAL_LDMA_CTRL_SIZE_BYTE, src, dest, cnt, link_jmp)
#define LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(src, dest, cnt, link_jmp)  SL_HAL_LDMA_DESCRIPTOR_LINKREL_P2M(SL_HAL_LDMA_CTRL_SIZE_BYTE, src, dest, cnt, link_jmp)
#define LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(src, dest, cnt)             SL_HAL_LDMA_DESCRIPTOR_SINGLE_M2P(SL_HAL_LDMA_CTRL_SIZE_BYTE, src, dest, cnt)
#define LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(src, dest, cnt)             SL_HAL_LDMA_DESCRIPTOR_SINGLE_P2M(SL_HAL_LDMA_CTRL_SIZE_BYTE, src, dest, cnt)
#define LDMA_DESCRIPTOR_LINKABS_M2M_BYTE(src, dest, cnt)            SL_HAL_LDMA_DESCRIPTOR_LINKABS_M2M(SL_HAL_LDMA_CTRL_SIZE_BYTE, src, dest, cnt)
#define LDMA_DESCRIPTOR_MAX_XFER_SIZE       SL_HAL_LDMA_DESCRIPTOR_MAX_XFER_SIZE
#define LDMA_DESCRIPTOR_XFER_CNT            xfer_count
#define LDMA_DESCRIPTOR_DONE_IFS            done_ifs
#define LDMA_DESCRIPTOR_SRC_ADDR            src_addr
#define LDMA_DESCRIPTOR_DST_ADDR            dst_addr
#define LDMA_DESCRIPTOR_LINK_ADDR           link_addr
#define LDMA_DESCRIPTOR_LINK_MODE           link_mode
#define LDMA_DESCRIPTOR_LINK_MODE_ABS       SL_HAL_LDMA_LINK_MODE_ABS
#define LDMA_DESCRIPTOR_DEC_LOOP_CNT        dec_loop_count
#define LDMA_CLEAR_CH_IRQ(ch)               sl_hal_ldma_clear_interrupts(LDMA_PERIPH, (1 << ch))

// GPIO
#define GPIO_PORT_SHIFT(route)              SL_CONCAT_PASTER_3(_GPIO_EUSART_, route, _PORT_SHIFT)
#define GPIO_PIN_SHIFT(route)               SL_CONCAT_PASTER_3(_GPIO_EUSART_, route, _PIN_SHIFT)
#define GPIO_ROUTEEN                        (GPIO_EUSART_ROUTEEN_TXPEN     \
                                             | GPIO_EUSART_ROUTEEN_RXPEN   \
                                             | GPIO_EUSART_ROUTEEN_SCLKPEN \
                                             | GPIO_EUSART_ROUTEEN_CSPEN)
#define GPIO_MODE_PUSH_PULL                 SL_GPIO_MODE_PUSH_PULL
#define GPIO_MODE_INPUT                     SL_GPIO_MODE_INPUT
#define GPIO_MODE_INPUT_PULL                SL_GPIO_MODE_INPUT_PULL
static inline void GPIO_SET_PIN_MODE(uint8_t port, uint8_t pin, uint8_t mode, bool val)
{
  sl_gpio_t gpio = {
    .port = port,
    .pin = pin
  };
  sl_hal_gpio_set_pin_mode(&gpio, (sl_gpio_mode_t)mode, val);
}
static inline void GPIO_SET_OUT_PIN(uint8_t port, uint8_t pin)
{
  sl_gpio_t gpio = {
    .port = port,
    .pin = pin
  };
  sl_hal_gpio_set_pin(&gpio);
}
static inline void GPIO_CLR_OUT_PIN(uint8_t port, uint8_t pin)
{
  sl_gpio_t gpio = {
    .port = port,
    .pin = pin
  };
  sl_hal_gpio_clear_pin(&gpio);
}

static inline void GPIO_CONFIGURE_EXT_INT(uint8_t port,
                                          uint8_t pin,
                                          int intNo,
                                          bool risingEdge,
                                          bool fallingEdge)
{
  sl_gpio_t gpio = {
    .pin = pin,
    .port = port
  };
  sl_gpio_interrupt_flag_t flags = SL_GPIO_INTERRUPT_NO_EDGE;

  if (risingEdge && fallingEdge) {
    flags = SL_GPIO_INTERRUPT_RISING_FALLING_EDGE;
  } else if (risingEdge) {
    flags = SL_GPIO_INTERRUPT_RISING_EDGE;
  } else if (fallingEdge) {
    flags = SL_GPIO_INTERRUPT_FALLING_EDGE;
  }

  sl_hal_gpio_configure_external_interrupt(&gpio,
                                           intNo,
                                           flags);
}

#endif // Series 3
#define IRQ_PIN_SET_MASK              (1 << SL_CPC_DRV_SPI_IRQ_PIN)
#define TX_READY_WINDOW_TRIG_BIT_MASK (1 << SL_CPC_DRV_SPI_TX_AVAILABILITY_SYNCTRIG_CH)
#if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
#define GPCRC_SYNC_BIT_MASK           (1 << SL_CPC_DRV_SPI_GPCRC_SYNC_BIT_CH)
#endif

#define IRQ_GPIO_SET_REG_ADDR    &GPIO->P_SET[SL_CPC_DRV_SPI_IRQ_PORT].DOUT
#define IRQ_GPIO_CLR_REG_ADDR    &GPIO->P_CLR[SL_CPC_DRV_SPI_IRQ_PORT].DOUT
#define LDMA_IEN_SET_REG_ADDR    &LDMA_PERIPH->IEN_SET
#define LDMA_IEN_CLR_REG_ADDR    &LDMA_PERIPH->IEN_CLR
#define LDMA_SYNCSW_SET_REG_ADDR &LDMA_PERIPH->SYNCSWSET
#define LDMA_SYNCSW_CLR_REG_ADDR &LDMA_PERIPH->SYNCSWCLR

/*******************************************************************************
 *******************   LOGIC ANALYZER MACROS   *********************************
 ******************************************************************************/

#ifdef LOGIC_ANALYZER_TRACES

#define LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_START                \
  GPIO_SET_OUT_PIN(LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_PIN_PORT, \
                   LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_PIN_PIN)

#define LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_END                  \
  GPIO_CLR_OUT_PIN(LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_PIN_PORT, \
                   LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_PIN_PIN)

#define LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_START                \
  GPIO_SET_OUT_PIN(LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_PIN_PORT, \
                   LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_PIN_PIN)

#define LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_END                  \
  GPIO_CLR_OUT_PIN(LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_PIN_PORT, \
                   LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_PIN_PIN)

#define LOGIC_ANALYZER_TRACE_TX_DMA_ARMED                        \
  do {                                                           \
    GPIO_SET_OUT_PIN(LOGIC_ANALYZER_TRACE_TX_DMA_ARMED_PIN_PORT, \
                     LOGIC_ANALYZER_TRACE_TX_DMA_ARMED_PIN_PIN); \
    __NOP();                                                     \
    __NOP();                                                     \
    __NOP();                                                     \
    __NOP();                                                     \
    GPIO_CLR_OUT_PIN(LOGIC_ANALYZER_TRACE_TX_DMA_ARMED_PIN_PORT, \
                     LOGIC_ANALYZER_TRACE_TX_DMA_ARMED_PIN_PIN); \
  } while (0)

#define LOGIC_ANALYZER_TRACE_TX_FLUSHED                        \
  do {                                                         \
    GPIO_SET_OUT_PIN(LOGIC_ANALYZER_TRACE_TX_FLUSHED_PIN_PORT, \
                     LOGIC_ANALYZER_TRACE_TX_FLUSHED_PIN_PIN); \
    __NOP();                                                   \
    __NOP();                                                   \
    __NOP();                                                   \
    __NOP();                                                   \
    GPIO_CLR_OUT_PIN(LOGIC_ANALYZER_TRACE_TX_FLUSHED_PIN_PORT, \
                     LOGIC_ANALYZER_TRACE_TX_FLUSHED_PIN_PIN); \
  } while (0)

#define LOGIC_ANALYZER_TRACE_PIN_INIT                                   \
  GPIO_SET_PIN_MODE(LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_PIN_PORT,  \
                    LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_PIN_PIN,   \
                    GPIO_MODE_PUSH_PULL, 0);                            \
  GPIO_SET_PIN_MODE(LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_PIN_PORT, \
                    LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_PIN_PIN,  \
                    GPIO_MODE_PUSH_PULL, 0);                            \
  GPIO_SET_PIN_MODE(LOGIC_ANALYZER_TRACE_TX_DMA_ARMED_PIN_PORT,         \
                    LOGIC_ANALYZER_TRACE_TX_DMA_ARMED_PIN_PIN,          \
                    GPIO_MODE_PUSH_PULL, 0);                            \
  GPIO_SET_PIN_MODE(LOGIC_ANALYZER_TRACE_TX_FLUSHED_PIN_PORT,           \
                    LOGIC_ANALYZER_TRACE_TX_FLUSHED_PIN_PIN,            \
                    GPIO_MODE_PUSH_PULL, 0);

#else

#define LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_START  (void)0
#define LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_END    (void)0
#define LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_START (void)0
#define LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_END   (void)0
#define LOGIC_ANALYZER_TRACE_TX_DMA_ARMED               (void)0
#define LOGIC_ANALYZER_TRACE_TX_FLUSHED                 (void)0
#define LOGIC_ANALYZER_TRACE_PIN_INIT                   (void)0

#endif

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

static sli_cpc_instance_t * driver_instance;

static unsigned int rx_dma_channel;
static unsigned int tx_dma_channel;
#if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
static unsigned int gpcrc_dma_channel;
#endif

static ldma_transfert_cfg_t rx_dma_config;
static ldma_transfert_cfg_t tx_dma_config;
#if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
static ldma_transfert_cfg_t gpcrc_dma_config;
#endif

// List of "sli_buf_entry_t" which have an EMPTY "sl_cpc_buffer_handle_t" attached to them.
// Those are entries that are available for receiving a frame. When arming a reception,
// the driver picks the first one and configures the DMA to write the data into its
// attached buffer.
static sl_slist_node_t *rx_free_list_head;

// List of "sli_buf_entry_t" which have a "sl_cpc_buffer_handle_t" attached to them.
// Those are entries that have fully received frame written in the attached
// "sl_cpc_buffer_handle_t". They are waiting for the core to pick them up.
static sl_slist_node_t *rx_pending_list_head;

// List of "sli_buf_entry_t" which have a FILLED "sl_cpc_buffer_handle_t" attached to them.
// Those are entries that are waiting for its buffer to be sent on the wire.
static sl_slist_node_t *tx_submitted_list_head;

static uint16_t tx_buf_available_count = SL_CPC_DRV_SPI_TX_QUEUE_SIZE;

// When not null, points to the first entry in "rx_free_list_head" when the driver
// Set itself up to receive a new frame. During the transaction, it is in this entry's
// buffer that the DMA writes the data.
static sl_cpc_buffer_handle_t *currently_receiving_rx_buffer_handle = NULL;

// When not null, points to the buffer handle currently being transmitted and configured in the active
// TX DMA chain.
static sl_cpc_buffer_handle_t *currently_transmiting_buffer_handle = NULL;

// Static buffer for the DMA to receive the header to postpone as late as possible having to fetch a buffer handle
static uint64_t header_buffer = 0;

// Variable set to 1 by the DMA as a synchronization flag and cleared by software
static volatile uint32_t tx_frame_complete = 0;

static volatile bool need_rx_buffer_handle = false;

// Debug variable to help keep track of the DMA IRQ state machine
static volatile int dma_irq_seq_no = 0;

/***************************************************************************//**
 * DMA descriptors
 ******************************************************************************/

// Reception descriptors
static ldma_descriptor_t rx_desc_wait_cs_high_after_header;
static ldma_descriptor_t rx_desc_wait_cs_low_before_payload;
static ldma_descriptor_t rx_desc_set_irq_high;
static ldma_descriptor_t rx_desc_recv_payload;
#if (SLI_CPC_DRV_SPI_RX_DATA_MAX_LENGTH > LDMA_DESCRIPTOR_MAX_XFER_SIZE)
static ldma_descriptor_t rx_desc_recv_payload_large_buf;
#endif
static ldma_descriptor_t rx_desc_rxblocken;
static ldma_descriptor_t rx_desc_wait_cs_high_after_payload; // Generates the end-of-payload interrupt

// rx_desc_throw_away_extra_bytes is a loop descriptor. This kind of descriptor
// jumps to the next descriptor in memory when loopcnt == 0. In order to
// guarantee that the order between these two is correct, put them in a struct.
static struct {
  ldma_descriptor_t rx_desc_throw_away_extra_bytes;
  ldma_descriptor_t rx_desc_set_availability_sync_bit; // When HWCRC enabled, also sets the GPCRC launch bit
} rx_desc_group;

static ldma_descriptor_t rx_desc_rxblockdis;
static ldma_descriptor_t rx_desc_wait_cs_low_before_header;
static ldma_descriptor_t rx_desc_clear_availability_sync_bit;
static ldma_descriptor_t rx_desc_set_irq_high_after_header_cs_low;
static ldma_descriptor_t rx_desc_recv_header;
#if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
static ldma_descriptor_t rx_desc_wait_gpcrc_sync_bit;
static ldma_descriptor_t rx_desc_throw_header_in_gpcrc;
static ldma_descriptor_t rx_desc_recv_header_crc;
#endif

#if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
static ldma_descriptor_t gpcrc_desc_wait_for_gpcrc_sync_bit;
static ldma_descriptor_t gpcrc_desc_clear_gpcrc_sync_bit;
static ldma_descriptor_t gpcrc_desc_write_rx_payload_words_in_gpcrc;
static ldma_descriptor_t gpcrc_desc_write_rx_remaining_payload_bytes_in_gpcrc;
static ldma_descriptor_t gpcrc_desc_save_rx_gpcrc_computed_crc;
static ldma_descriptor_t gpcrc_desc_set_rxchain_ien_bit;
static ldma_descriptor_t gpcrc_desc_set_gpcrc_sync_bit;
#endif

// Transmission descriptors
static ldma_descriptor_t tx_desc_wait_availability_sync_bit;
static ldma_descriptor_t tx_desc_set_irq_low;
static ldma_descriptor_t tx_desc_xfer_header;
static ldma_descriptor_t tx_desc_wait_header_tranfered;
static ldma_descriptor_t tx_desc_set_tx_frame_complete_variable_after_header;
static ldma_descriptor_t tx_desc_xfer_payload;
#if (SLI_CPC_DRV_SPI_RX_DATA_MAX_LENGTH > LDMA_DESCRIPTOR_MAX_XFER_SIZE)
static ldma_descriptor_t tx_desc_xfer_payload_large_buf;
#endif
#if (SL_CPC_ENDPOINT_SECURITY_ENABLED == 1)
static ldma_descriptor_t tx_desc_xfer_tag;
#endif
static ldma_descriptor_t tx_desc_xfer_checksum;

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

static void init_clocks(void);
static void flush_rx(void);
static void flush_tx(void);

static bool rx_dma_callback(unsigned int channel, unsigned int sequenceNo, void *userParam);

static void end_of_header_xfer(void);
static bool end_of_payload_xfer(void);

static void prime_dma_for_transmission(void);
static bool prime_dma_for_reception(size_t payload_size, bool received_valid_header);

static sl_status_t spi_drv_hw_init(sli_cpc_drv_t *drv);
static sl_status_t spi_drv_init(sli_cpc_drv_t *drv, sli_cpc_instance_t *inst);
static sl_status_t spi_drv_get_capabilities(sli_cpc_drv_t *drv,
                                            sli_cpc_drv_capabilities_t *capabilities);
static sl_status_t spi_drv_start_rx(sli_cpc_drv_t *drv);
static sl_status_t spi_drv_read_data(sli_cpc_drv_t *drv, sl_cpc_buffer_handle_t **buffer_handle);
static sl_status_t spi_drv_transmit_data(sli_cpc_drv_t *drv, sl_cpc_buffer_handle_t *buffer_handle);
static bool spi_drv_is_transmit_ready(sli_cpc_drv_t *drv);
static uint32_t spi_drv_get_bus_bitrate(sli_cpc_drv_t *drv);
static uint32_t spi_drv_get_bus_max_bitrate(sli_cpc_drv_t *drv);
static void spi_drv_on_rx_buffer_free(sli_cpc_drv_t *drv);

sli_cpc_drv_t spi_driver = {
  .ops = {
    .hw_init = spi_drv_hw_init,
    .init = spi_drv_init,
    .get_capabilities = spi_drv_get_capabilities,
    .start_rx = spi_drv_start_rx,
    .read = spi_drv_read_data,
    .write = spi_drv_transmit_data,
    .is_transmit_ready = spi_drv_is_transmit_ready,
    .get_bus_bitrate = spi_drv_get_bus_bitrate,
    .get_bus_max_bitrate = spi_drv_get_bus_max_bitrate,
    .on_rx_buffer_free = spi_drv_on_rx_buffer_free,
  }
};

/*******************************************************************************
 ***************************   WEAK FUNCTIONS   ********************************
 ******************************************************************************/
void sli_cpc_drv_wake_gpio_init(void);
void sli_cpc_drv_wake_host_gpio(bool active);

/***************************************************************************/ /**
 * Initialize only the SPI peripheral to be used in a standalone manner
 * (during the bootloader poking). On the secondary (this) side, the initialization
 * split is useless and the second init function below will be called right after
 * this one
 ******************************************************************************/
static sl_status_t spi_drv_hw_init(sli_cpc_drv_t *drv)
{
  (void)drv;

  init_clocks();

  // Set pin modes and drive characteristics for the SPI mode 0
  GPIO_SET_PIN_MODE(SL_CPC_DRV_SPI_COPI_PORT, SL_CPC_DRV_SPI_COPI_PIN, GPIO_MODE_INPUT, 0);       // The E/USART's TX labeled pin becomes the input when configured as a slave
  GPIO_SET_PIN_MODE(SL_CPC_DRV_SPI_CIPO_PORT, SL_CPC_DRV_SPI_CIPO_PIN, GPIO_MODE_PUSH_PULL, 0);    // The E/USART's RX labeled pin becomes the output when configured as a slave
  GPIO_SET_PIN_MODE(SL_CPC_DRV_SPI_CLK_PORT, SL_CPC_DRV_SPI_CLK_PIN, GPIO_MODE_INPUT, 0);
  GPIO_SET_PIN_MODE(SL_CPC_DRV_SPI_CS_PORT, SL_CPC_DRV_SPI_CS_PIN, GPIO_MODE_INPUT_PULL, 1);   // Pull up to give a idle high state to the input Chip Select signal
  GPIO_SET_PIN_MODE(SL_CPC_DRV_SPI_IRQ_PORT, SL_CPC_DRV_SPI_IRQ_PIN, GPIO_MODE_PUSH_PULL, 1);  // Initial value of IRQ signal is HIGH (no frame to send)

  sli_cpc_drv_wake_gpio_init();

  // Configure the GPIO routing to the SPI peripheral
  {
  #if defined(_GPIO_USART_ROUTEEN_MASK) || defined(_GPIO_EUSART_ROUTEEN_MASK)
    // Route MISO to the EUSART
    GPIO->SPI_ROUTE[SL_CPC_DRV_SPI_PERIPHERAL_NO].TXROUTE =
      ((uint32_t)SL_CPC_DRV_SPI_COPI_PORT << GPIO_PORT_SHIFT(TXROUTE))
      | ((uint32_t)SL_CPC_DRV_SPI_COPI_PIN  << GPIO_PIN_SHIFT(TXROUTE));

    // Route MOSI to the EUSART
    GPIO->SPI_ROUTE[SL_CPC_DRV_SPI_PERIPHERAL_NO].RXROUTE =
      ((uint32_t)SL_CPC_DRV_SPI_CIPO_PORT << GPIO_PORT_SHIFT(RXROUTE))
      | ((uint32_t)SL_CPC_DRV_SPI_CIPO_PIN  << GPIO_PIN_SHIFT(RXROUTE));

    // Route SCLK to the EUSART
  #if defined(SL_CPC_DRV_SPI_IS_USART)
    GPIO->SPI_ROUTE[SL_CPC_DRV_SPI_PERIPHERAL_NO].CLKROUTE =
      ((uint32_t)SL_CPC_DRV_SPI_CLK_PORT << GPIO_PORT_SHIFT(CLKROUTE))
      | ((uint32_t)SL_CPC_DRV_SPI_CLK_PIN  << GPIO_PIN_SHIFT(CLKROUTE));
  #elif defined(SL_CPC_DRV_SPI_IS_EUSART)
    GPIO->SPI_ROUTE[SL_CPC_DRV_SPI_PERIPHERAL_NO].SCLKROUTE =
      ((uint32_t)SL_CPC_DRV_SPI_CLK_PORT << GPIO_PORT_SHIFT(SCLKROUTE))
      | ((uint32_t)SL_CPC_DRV_SPI_CLK_PIN  << GPIO_PIN_SHIFT(SCLKROUTE));
  #endif

    // Route CS to the EUSART
    GPIO->SPI_ROUTE[SL_CPC_DRV_SPI_PERIPHERAL_NO].CSROUTE =
      ((uint32_t)SL_CPC_DRV_SPI_CS_PORT << GPIO_PORT_SHIFT(CSROUTE))
      | ((uint32_t)SL_CPC_DRV_SPI_CS_PIN  << GPIO_PIN_SHIFT(CSROUTE));

    // Activate the routes to the EUSART
    GPIO->SPI_ROUTE[SL_CPC_DRV_SPI_PERIPHERAL_NO].ROUTEEN = GPIO_ROUTEEN;
  #endif
  }

  // Init of the E/USART
  {
  #if defined(SL_CPC_DRV_SPI_IS_EUSART)
    // MSB first, EUSART in SPI mode
    SL_CPC_DRV_SPI_PERIPHERAL->CFG0 = EUSART_CFG0_MSBF | EUSART_CFG0_SYNC;

    // TX and RX FIFO level interrupt of 1.
    SL_CPC_DRV_SPI_PERIPHERAL->CFG1 = EUSART_CFG1_RXFIW_ONEFRAME | EUSART_CFG1_TXFIW_ONEFRAME;

    // Slave mode 0 (cpol 0, cpha 0)
    SL_CPC_DRV_SPI_PERIPHERAL->CFG2 = EUSART_CFG2_FORCELOAD | EUSART_CFG2_CLKPHA_SAMPLELEADING | EUSART_CFG2_CLKPOL_IDLELOW | _EUSART_CFG2_MASTER_SLAVE;

    // 8 bits per byte
    SL_CPC_DRV_SPI_PERIPHERAL->FRAMECFG = EUSART_FRAMECFG_DATABITS_EIGHT;

    // 0s sent when nothing to send
    SL_CPC_DRV_SPI_PERIPHERAL->DTXDATCFG = EUSART_DTXDATCFG_DTXDAT_DEFAULT;

    // Default setup window of 5 is recommended when bitrate < 5Mhz
    SL_CPC_DRV_SPI_PERIPHERAL->TIMINGCFG = EUSART_TIMINGCFG_SETUPWINDOW_DEFAULT;

    // Enable EUSART
    SL_CPC_DRV_SPI_PERIPHERAL->EN = EUSART_EN_EN;

    // Enable RX and TX
    SL_CPC_DRV_SPI_PERIPHERAL->CMD = EUSART_CMD_RXEN | EUSART_CMD_TXEN;

    // Activate Load Error and TX overflow interrupts for monitoring
    SL_CPC_DRV_SPI_PERIPHERAL->IEN = EUSART_IF_LOADERR | EUSART_IF_TXOF;

    // Wait until all operations are synch'ed
    while (SL_CPC_DRV_SPI_PERIPHERAL->SYNCBUSY & _EUSART_SYNCBUSY_MASK) ;

  #elif defined(SL_CPC_DRV_SPI_IS_USART)

    #if defined(USART_EN_EN)
    SL_CPC_DRV_SPI_PERIPHERAL->EN_SET = USART_EN_EN;
    #endif

    SL_CPC_DRV_SPI_PERIPHERAL->CTRL = USART_CTRL_SYNC | USART_CTRL_CLKPOL_IDLELOW | USART_CTRL_CLKPHA_SAMPLELEADING | USART_CTRL_MSBF;

    SL_CPC_DRV_SPI_PERIPHERAL->FRAME = USART_FRAME_DATABITS_EIGHT;

    SL_CPC_DRV_SPI_PERIPHERAL->CMD = USART_CMD_RXEN | USART_CMD_TXEN;

  #elif defined(SL_CPC_DRV_SPI_IS_HSPI)

    // HSPI Initialization
    sl_hal_hspi_init_t hspi_cfg = SL_HAL_HSPI_SPI_TARGET_INIT_DEFAULT_HF;
    sl_hal_hspi_advanced_init_t hspi_adv_cfg = SL_HAL_HSPI_SPI_ADVANCED_INIT_DEFAULT;

    // Enable HSPI bus clock
    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_HSPI0);

    // Configure HSPI for SPI Mode 0 (clock idle low, sample on rising edge)
    hspi_cfg.clock_mode = SL_HAL_HSPI_CLOCK_MODE_0;
    hspi_adv_cfg.msb_first = true;  // MSB first transmission
    hspi_cfg.advanced_config = &hspi_adv_cfg;

    // Initialize and enable HSPI
    sl_hal_hspi_init(SL_CPC_DRV_SPI_PERIPHERAL, &hspi_cfg);
    sl_hal_hspi_enable(SL_CPC_DRV_SPI_PERIPHERAL);
    sl_hal_hspi_enable_rx(SL_CPC_DRV_SPI_PERIPHERAL);
    sl_hal_hspi_enable_tx(SL_CPC_DRV_SPI_PERIPHERAL);
    sl_hal_hspi_wait_sync(SL_CPC_DRV_SPI_PERIPHERAL, _HSPI_SYNCBUSY_MASK);

    // Route HSPI GPIO pins
    if ((SL_CPC_DRV_SPI_PERIPHERAL->MODECTRL & _HSPI_MODECTRL_MASK) == _HSPI_MODECTRL_SPIMODE_SIGNLEMODE) {
      GPIO->HSPI0PRI1ROUTEPEN = GPIO_HSPI0PRI1ROUTEPEN_HSPI0PRI1DATA0ROUTEPEN
                                | GPIO_HSPI0PRI1ROUTEPEN_HSPI0PRI1DATA1ROUTEPEN
                                | GPIO_HSPI0PRI1ROUTEPEN_HSPI0PRI1CSROUTEPEN
                                | GPIO_HSPI0PRI1ROUTEPEN_HSPI0PRI1CLKROUTEPEN;
    } else {
      GPIO->HSPI0PRI1ROUTEPEN = GPIO_HSPI0PRI1ROUTEPEN_HSPI0PRI1DATA0ROUTEPEN
                                | GPIO_HSPI0PRI1ROUTEPEN_HSPI0PRI1DATA1ROUTEPEN
                                | GPIO_HSPI0PRI1ROUTEPEN_HSPI0PRI1DATA2ROUTEPEN
                                | GPIO_HSPI0PRI1ROUTEPEN_HSPI0PRI1DATA3ROUTEPEN
                                | GPIO_HSPI0PRI1ROUTEPEN_HSPI0PRI1CSROUTEPEN
                                | GPIO_HSPI0PRI1ROUTEPEN_HSPI0PRI1CLKROUTEPEN;
    }

  #endif
  }

  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 * Initialize the rest of the driver after the SPI peripheral has been
 * initialized in spi_drv_hw_init.
 ******************************************************************************/
static sl_status_t spi_drv_init(sli_cpc_drv_t *drv, sli_cpc_instance_t *inst)
{
  (void)drv;

  // Initialize the lists
  {
    sl_slist_init(&rx_free_list_head);
    sl_slist_init(&rx_pending_list_head);
    sl_slist_init(&tx_submitted_list_head);

    driver_instance = inst;

    for (uint32_t buf_cnt = 0; buf_cnt < SL_CPC_DRV_SPI_RX_QUEUE_SIZE; buf_cnt++) {
      sl_cpc_buffer_handle_t *buffer_handle;
      if (SL_BRANCH_UNLIKELY(sli_cpc_get_buffer_handle_for_rx(inst, &buffer_handle, true) != SL_STATUS_OK)) {
        SLI_CPC_ASSERT(0);
        return SL_STATUS_ALLOCATION_FAILED;
      }
      sli_cpc_push_driver_buffer_handle(&rx_free_list_head, buffer_handle);
    }

    tx_buf_available_count = SL_CPC_DRV_SPI_TX_QUEUE_SIZE;
  }

  // Init the DMA and allocate two channels
  {
    Ecode_t ret;
    ret = DMADRV_Init();
    if (SL_BRANCH_UNLIKELY(ret != ECODE_EMDRV_DMADRV_OK && ret != ECODE_EMDRV_DMADRV_ALREADY_INITIALIZED)) {
      SLI_CPC_ASSERT(0);
      return SL_STATUS_INITIALIZATION;
    }

    ret = DMADRV_AllocateChannel(&rx_dma_channel, NULL);
    if (SL_BRANCH_UNLIKELY(ret != ECODE_EMDRV_DMADRV_OK)) {
      SLI_CPC_ASSERT(0);
      return SL_STATUS_ALLOCATION_FAILED;
    }

    ret = DMADRV_AllocateChannel(&tx_dma_channel, NULL);
    if (SL_BRANCH_UNLIKELY(ret != ECODE_EMDRV_DMADRV_OK)) {
      SLI_CPC_ASSERT(0);
      return SL_STATUS_ALLOCATION_FAILED;
    }

#if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
    ret = DMADRV_AllocateChannel(&gpcrc_dma_channel, NULL);
    if (SL_BRANCH_UNLIKELY(ret != ECODE_EMDRV_DMADRV_OK)) {
      SLI_CPC_ASSERT(0);
      return SL_STATUS_ALLOCATION_FAILED;
    }
#endif

    //The DMA channel that serves receive buffer should have higher priority than the DMA channel that serves transmit buffer.
    if (SL_BRANCH_UNLIKELY(rx_dma_channel > tx_dma_channel)) {
      // A lower number DMA channel is higher priority. If the allocated DMA channels from SPIDRV init gave us
      // a inverted priority, switch those channels. They have been allocated, its safe to just switch them.
      unsigned int tmp = rx_dma_channel;
      rx_dma_channel = tx_dma_channel;
      tx_dma_channel = tmp;
    }
  }

  // Configure the LDMA SYNCTRIG mechanism
  {
    // Driver relies on the SYNCTRIG mechanism to synchronize the RX and TX DMA chains. There is one
    // sync bit that follows the chip select: it is set on a rising edge of the chip select and
    // cleared on a falling edge.
    //
    // There are two main parts to set that up:
    //  - Set the Chip Select of (E)USART as signal producer for a PRS channel
    //  - Set the LDMA SYNCHW config to have a syncbit set and cleared based on that PRS channel
    //
    // The behaviour is not clearly defined when the PRS channel value is high and then the syncbit
    // is configured to be set on a rising edge. For some devices, that will set the syncbit and on
    // others it will not.
    //
    // To get deterministic behaviour, it's important to configure the LDMA first and only then
    // connect the chip select to the PRS channel. If chip select was high, this will make the LDMA
    // see a rising edge for all devices, and consequently the sync bit will be set.

    {
      // Enable the "usart_txc_prs_channel" PRS channel (a.k.a the USART TXC signal) to
      // be able to SET its corresponding SYNCTRIG[] bit
      LDMA_PERIPH->SYNCHWEN_SET = (1UL << SL_CPC_DRV_SPI_TXC_SYNCTRIG_PRS_CH) << _LDMA_SYNCHWEN_SYNCSETEN_SHIFT;
      // The "usart_txc_prs_channel" SYNCTRIG bit don't need SYNCHWSEL modification (rising-edge to SET by default)

      // Enable the "usart_cs_prs_channel" PRS channel (a.k.a the USART CS signal) to
      // be able to SET and CLEAR its corresponding SYNCTRIG[] bit
      LDMA_PERIPH->SYNCHWEN_SET =   ((1UL << SL_CPC_DRV_SPI_CS_SYNCTRIG_PRS_CH) << _LDMA_SYNCHWEN_SYNCSETEN_SHIFT)  // CS will SET its SYNC bit
                                  | ((1UL << SL_CPC_DRV_SPI_CS_SYNCTRIG_PRS_CH) << _LDMA_SYNCHWEN_SYNCCLREN_SHIFT); // CS will [also] CLEAR its SYNC bit.

      // Configure how USART CS will set/clear its SYNC bit
      // With this configuration, the SYNCTRIG[usart_txc_prs_channel] bit will reflect the state of the USART CS line :
      // == 1 when CS is high, and == 0 when CS is low
      LDMA_PERIPH->SYNCHWSEL_SET =   ((_LDMA_SYNCHWSEL_SYNCSETEDGE_RISE << SL_CPC_DRV_SPI_CS_SYNCTRIG_PRS_CH) << _LDMA_SYNCHWSEL_SYNCSETEDGE_SHIFT)  // CS will SET its SYNC bit on rising edge
                                   | ((_LDMA_SYNCHWSEL_SYNCSETEDGE_FALL << SL_CPC_DRV_SPI_CS_SYNCTRIG_PRS_CH) << _LDMA_SYNCHWSEL_SYNCCLREDGE_SHIFT); // CS will CLR its SYNC bit on falling edge
    }
  }

  // Setup a PRS channel to connect the E/USART TXC signal
  {
    prs_signal_t signal;

    // Make sure the SYNCTRIG/PRS combination is within bound
    SLI_CPC_ASSERT(SL_CPC_DRV_SPI_TXC_SYNCTRIG_PRS_CH <= 7);

    // Retrieve the connected signal
    #if defined(_PRS_ASYNC_CH_CTRL_SOURCESEL_MASK)
    signal = (prs_signal_t) (PRS->ASYNC_CH[SL_CPC_DRV_SPI_TXC_SYNCTRIG_PRS_CH].CTRL
                             & (_PRS_ASYNC_CH_CTRL_SOURCESEL_MASK | _PRS_ASYNC_CH_CTRL_SIGSEL_MASK));
    #else
    signal = (prs_signal_t) (PRS->CH[SL_CPC_DRV_SPI_TXC_SYNCTRIG_PRS_CH].CTRL
                             & (_PRS_CH_CTRL_SOURCESEL_MASK | _PRS_CH_CTRL_SIGSEL_MASK));
    #endif

    // The configured TXC SYNCTRIG bit works with its corresponding PRS Channel number. Make sure that PRS Channel is not used.
    SLI_CPC_ASSERT(signal == PRS_SIGNAL_NONE);

    // The PRS Channel was free, now configure it
    PRS_ASYNC_CONNECT_PRODUCER(
      SL_CPC_DRV_SPI_TXC_SYNCTRIG_PRS_CH,
      PRS_SIGNAL_SPI(SL_CPC_DRV_SPI_PERIPHERAL_NO, TXC));
  }

  // This driver needs to route the incoming Chip Select signal to a PRS channel.
  // Setup a PRS channel to connect the UART Chip Select signal
  {
    prs_signal_t signal;

    // Make sure the SYNCTRIG/PRS combination is within bound
    SLI_CPC_ASSERT(SL_CPC_DRV_SPI_CS_SYNCTRIG_PRS_CH <= 7);

    #if defined(_PRS_ASYNC_CH_CTRL_SOURCESEL_MASK)
    signal = (prs_signal_t) (PRS->ASYNC_CH[SL_CPC_DRV_SPI_CS_SYNCTRIG_PRS_CH].CTRL
                             & (_PRS_ASYNC_CH_CTRL_SOURCESEL_MASK | _PRS_ASYNC_CH_CTRL_SIGSEL_MASK));
    #else
    signal = (prs_signal_t) (PRS->CH[SL_CPC_DRV_SPI_CS_SYNCTRIG_PRS_CH].CTRL
                             & (_PRS_CH_CTRL_SOURCESEL_MASK | _PRS_CH_CTRL_SIGSEL_MASK));
    #endif

    // The configured CS SYNCTRIG bit works with its corresponding PRS Channel number. Make sure that PRS Channel is not used.
    SLI_CPC_ASSERT(signal == PRS_SIGNAL_NONE);

#if defined(SL_CPC_DRV_SPI_IS_EUSART) && defined(_SILICON_LABS_32B_SERIES_2)
    {
      // EUSART has a quirk that needs to be dealt with differently then USART
      // The only way of routing the incoming CS signal to a PRS channel is
      // via the External Interrupt route.

      // We need to chose an EXTI interrupt number for the CS pin.
      // The limitation is the following :
      // pins 0-3   (interrupt number 0-3)
      // pins 4-7   (interrupt number 4-7)

#if (SL_CPC_DRV_SPI_CS_EXTI_NUMBER >= 0 && SL_CPC_DRV_SPI_CS_EXTI_NUMBER <= 3)
#if !(SL_CPC_DRV_SPI_CS_PIN >= 0 && SL_CPC_DRV_SPI_CS_PIN <= 3)
#error "For an EXTI0..3, only pin Px0..3 can be used as CS"
#endif
#elif (SL_CPC_DRV_SPI_CS_EXTI_NUMBER >= 4 && SL_CPC_DRV_SPI_CS_EXTI_NUMBER <= 7)
#if !(SL_CPC_DRV_SPI_CS_PIN >= 4 && SL_CPC_DRV_SPI_CS_PIN <= 7)
#error "For an EXTI4..7, only pin Px4..7 can be used as CS"
#endif
#else
#error "Only EXTI0..7 can be used because the PRS only support those as inputs"
#endif

      GPIO_CONFIGURE_EXT_INT(SL_CPC_DRV_SPI_CS_PORT, SL_CPC_DRV_SPI_CS_PIN,
                             SL_CPC_DRV_SPI_CS_EXTI_NUMBER,
                             false, // don't care about rising edge
                             false);
      PRS_ASYNC_CONNECT_PRODUCER(
        SL_CPC_DRV_SPI_CS_SYNCTRIG_PRS_CH,
        PRS_SIGNAL_EXTI(SL_CPC_DRV_SPI_CS_EXTI_NUMBER));
    }
#else
    {
      // The CS signal can be retrieved via PRS.
      PRS_ASYNC_CONNECT_PRODUCER(
        SL_CPC_DRV_SPI_CS_SYNCTRIG_PRS_CH,
        PRS_SIGNAL_SPI(SL_CPC_DRV_SPI_PERIPHERAL_NO, CS));
    }
#endif
  }

  // SPI DMA configuration
  {
    // Create DMA configs for TX/RX from the DMA-REQ signals of the E/USART
    rx_dma_config = (ldma_transfert_cfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(LDMA_SIGNAL_RX(SL_CPC_DRV_SPI_PERIPHERAL_NO));
    tx_dma_config = (ldma_transfert_cfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(LDMA_SIGNAL_TX(SL_CPC_DRV_SPI_PERIPHERAL_NO));
    #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
    gpcrc_dma_config = (ldma_transfert_cfg_t)LDMA_TRANSFER_CFG_MEMORY();
    #endif

    rx_dma_config.LDMA_TRANSFER_CFG_DBGHALT = true;
    tx_dma_config.LDMA_TRANSFER_CFG_DBGHALT = true;
    #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
    gpcrc_dma_config.LDMA_TRANSFER_CFG_DBGHALT = true;
    #endif

    // This loop count is used by the rx_desc_throw_away_extra_bytes to loop
    // over itself. Setting the loop count to 1 means it's going to do two
    // iterations. As this structure is passed as argument when starting
    // transfers, that means the loop count will be reinitialized to one every
    // time a new transfer is started, which is the expect behavior.
    rx_dma_config.LDMA_TRANSFER_CFG_LOOP_COUNT = 1;
  }

  // Prepare the reception DMA descriptor chain.
  {
    #define ENABLE_SYNC_ON_CS_PRS_CHANNEL     (1 << SL_CPC_DRV_SPI_CS_SYNCTRIG_PRS_CH)
    #define CLEAR_CS_PRS_CHANNEL_UPON_LOAD    (1 << SL_CPC_DRV_SPI_CS_SYNCTRIG_PRS_CH)
    #define SYNC_ON_CS_PRS_CHANNEL_EQUAL_ONE  (1 << SL_CPC_DRV_SPI_CS_SYNCTRIG_PRS_CH)
    #define SYNC_ON_CS_PRS_CHANNEL_EQUAL_ZERO (0 << SL_CPC_DRV_SPI_CS_SYNCTRIG_PRS_CH)

    // Sync descriptor to wait for the CS high between the header and the payload
    {
      rx_desc_wait_cs_high_after_header = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_SYNC(
        0x0,
        0x0,
        SYNC_ON_CS_PRS_CHANNEL_EQUAL_ONE,
        ENABLE_SYNC_ON_CS_PRS_CHANNEL);

      // Fixed branching skipping the 3 following descriptors used for the SERIES_2_CONFIG_5 hack
      rx_desc_wait_cs_high_after_header.sync.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_wait_cs_low_before_payload);
    }

    // Sync descriptor to wait for the CS low before payload clocking
    {
      rx_desc_wait_cs_low_before_payload = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_SYNC(
        0x0,
        0x0,
        SYNC_ON_CS_PRS_CHANNEL_EQUAL_ZERO,
        ENABLE_SYNC_ON_CS_PRS_CHANNEL);

      rx_desc_wait_cs_low_before_payload.sync.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_set_irq_high);
    }

    // WRITE descriptor to set the IRQ pin high after the falling edge of CS of payload
    {
      rx_desc_set_irq_high = (ldma_descriptor_t)LDMA_DESCRIPTOR_LINKABS_WRITE(
        IRQ_PIN_SET_MASK,
        IRQ_GPIO_SET_REG_ADDR);

      // This descriptor's branching address is computed before each time this chain is armed
      // If there is a payload to receive, it branches to the payload descriptor
      // If there is NO payload to receive, it branches to the sync descriptor that sets RXBLOCKEN
    }

    // Transfer descriptor to receive the payload
    {
      rx_desc_recv_payload = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(
        &(SL_CPC_DRV_SPI_PERIPHERAL->RXDATA),
        0, // place holder for data pointer
        0, // place holder for length
        1); // Will be overridden below

      // The macro LDMA_DESCRIPTOR_LINKABS_P2M_BYTE does not exist, force this descriptor to use absolute linking
      rx_desc_recv_payload.xfer.LDMA_DESCRIPTOR_LINK_MODE = LDMA_DESCRIPTOR_LINK_MODE_ABS;

      // Override .LDMA_DESCRIPTOR_DONE_IFS; the interrupts are generated somewhere else
      rx_desc_recv_payload.xfer.LDMA_DESCRIPTOR_DONE_IFS = 0;

      // When NOT large buffer, this descriptor's branching remains fixed
      rx_desc_recv_payload.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_rxblocken);

      // When large buffer, the branching is computed before each arming
    }

    #if (SLI_CPC_DRV_SPI_RX_DATA_MAX_LENGTH > LDMA_DESCRIPTOR_MAX_XFER_SIZE)
    // Transfer descriptor to receive the large buffer payload
    {
      rx_desc_recv_payload_large_buf = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(
        &(SL_CPC_DRV_SPI_PERIPHERAL->RXDATA),
        0, // place holder for payload pointer
        0, // place holder for length
        1); // Will be overridden below

      // The macro LDMA_DESCRIPTOR_LINKABS_P2M_BYTE does not exist, force this descriptor to use absolute linking
      rx_desc_recv_payload_large_buf.xfer.LDMA_DESCRIPTOR_LINK_MODE = LDMA_DESCRIPTOR_LINK_MODE_ABS;

      // Override .LDMA_DESCRIPTOR_DONE_IFS; the interrupts are generated somewhere else
      rx_desc_recv_payload_large_buf.xfer.LDMA_DESCRIPTOR_DONE_IFS = 0;

      rx_desc_recv_payload_large_buf.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_rxblocken);
    }
    #endif

    // Write descriptor to enable the RXBLOCK of the E/USART to block incoming bytes
    {
      #if defined(SL_CPC_DRV_SPI_IS_EUSART)
        #define RXBLOCKEN_CMD  EUSART_CMD_RXBLOCKEN
      #elif defined(SL_CPC_DRV_SPI_IS_USART)
        #define RXBLOCKEN_CMD  USART_CMD_RXBLOCKEN
      #elif defined(SL_CPC_DRV_SPI_IS_HSPI)
        #define RXBLOCKEN_CMD  HSPI_CMD_RXBLOCKEN
      #endif

      rx_desc_rxblocken = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_WRITE(
        RXBLOCKEN_CMD,
        &SL_CPC_DRV_SPI_PERIPHERAL->CMD_SET);


      // Fixed branching
      rx_desc_rxblocken.wri.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_wait_cs_high_after_payload);
    }

    // Sync descriptor to wait for the rising edge of the UART CS following the payload.
    {
      rx_desc_wait_cs_high_after_payload = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_SYNC(
        0x0,
        0x0,
        SYNC_ON_CS_PRS_CHANNEL_EQUAL_ONE,
        ENABLE_SYNC_ON_CS_PRS_CHANNEL);

      // The payload interrupt is generated when this descriptor unblocks.
      // With HWCRC, since the IEN bit of this channel is voluntarily cleared, the IF bit might be pending
      // for a bit until the GPCRC DMA chain sets the IEN bit
      rx_desc_wait_cs_high_after_payload.sync.LDMA_DESCRIPTOR_DONE_IFS = 1;

      // Fixed branching to the descriptor following
      rx_desc_wait_cs_high_after_payload.sync.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_group.rx_desc_throw_away_extra_bytes);
    }

    // Transfer descriptor to throw away two bytes of data away. This is needed
    // when operating at high speed because the RXBLOCKEN command is not applied
    // immediately and up to two bytes might slip through in the RX FIFO.
    {
      // Dummy byte to throw away the bytes from the RX FIFO
      static uint8_t dummy_byte;

      rx_desc_group.rx_desc_throw_away_extra_bytes = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_M2M_BYTE(
        &(SL_CPC_DRV_SPI_PERIPHERAL->RXDATA),
        &dummy_byte,
        1);

      // Fixed branching
      rx_desc_group.rx_desc_throw_away_extra_bytes.xfer.LDMA_DESCRIPTOR_DEC_LOOP_CNT = 1;
      rx_desc_group.rx_desc_throw_away_extra_bytes.xfer.LDMA_DESCRIPTOR_LINK_ADDR =
        LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_group.rx_desc_throw_away_extra_bytes);
    }

    // Sync descriptor to set the TX availability bit
    {
      #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
      // When HC CRC is enabled, take advantage of this descriptor to set another sync trig bit that
      // will enable the 3rd DMA chain to compute the RX'ed data's CRC
      const uint8_t sync_trig_bits_to_set = TX_READY_WINDOW_TRIG_BIT_MASK | GPCRC_SYNC_BIT_MASK;
      #else
      const uint8_t sync_trig_bits_to_set = TX_READY_WINDOW_TRIG_BIT_MASK;
      #endif

      rx_desc_group.rx_desc_set_availability_sync_bit = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_SYNC(
        sync_trig_bits_to_set,
        0,
        0,
        0);

      // Fixed branching
      rx_desc_group.rx_desc_set_availability_sync_bit.sync.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_rxblockdis);
    }

    // Write descriptor to disable the RXBLOCK of the E/USART to accept incoming bytes
    {
      #if defined(SL_CPC_DRV_SPI_IS_EUSART)
        #define RXBLOCKDIS_CMD  EUSART_CMD_RXBLOCKDIS
      #elif defined(SL_CPC_DRV_SPI_IS_USART)
        #define RXBLOCKDIS_CMD  USART_CMD_RXBLOCKDIS
      #elif defined(SL_CPC_DRV_SPI_IS_HSPI)
        #define RXBLOCKDIS_CMD  HSPI_CMD_RXBLOCKDIS
      #endif

      #if defined(SL_CPC_DRV_SPI_IS_HSPI)
      rx_desc_rxblockdis = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_WRITE(
        RXBLOCKDIS_CMD,
        &SL_CPC_DRV_SPI_PERIPHERAL->CMD_SET);
      #else
      rx_desc_rxblockdis = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_WRITE(
        RXBLOCKDIS_CMD,
        &SL_CPC_DRV_SPI_PERIPHERAL->CMD);
      #endif

      rx_desc_rxblockdis.wri.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_wait_cs_low_before_header);
    }

    // Sync descriptor to wait for the falling edge of the UART CS of the next header.
    {
      rx_desc_wait_cs_low_before_header = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_SYNC(
        0,
        0x0,
        SYNC_ON_CS_PRS_CHANNEL_EQUAL_ZERO,
        ENABLE_SYNC_ON_CS_PRS_CHANNEL);

      rx_desc_wait_cs_low_before_header.sync.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_clear_availability_sync_bit);
    }

    // Sync descriptor to clear the SYNCTRIG[7] bit
    {
      rx_desc_clear_availability_sync_bit = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_SYNC(
        0x0,
        TX_READY_WINDOW_TRIG_BIT_MASK, // Clears TX READY WINDOW trig bit when loaded
        0x0,
        0x0);

      // Fixed branching to the descriptor following
      rx_desc_clear_availability_sync_bit.sync.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_set_irq_high_after_header_cs_low);
    }

    // Immediate write to set the IRQ pin high.
    {
      rx_desc_set_irq_high_after_header_cs_low = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_WRITE(
        IRQ_PIN_SET_MASK,
        IRQ_GPIO_SET_REG_ADDR);

      // Fixed branching to the descriptor following
      rx_desc_set_irq_high_after_header_cs_low.wri.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_recv_header);
    }

    // Transfer the received header. Because its a _SINGLE_ descriptor, the
    // .LDMA_DESCRIPTOR_DONE_IFS bit is set and an interrupt will fire after it gets executed
    {
      rx_desc_recv_header = (ldma_descriptor_t)LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(
        &(SL_CPC_DRV_SPI_PERIPHERAL->RXDATA),
        &header_buffer,
      #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
        // In case the CRC HW acceleration is enabled, only transfer to memory the first 5 bytes of the header
        // which are the effective header data. The remaining 2, the CRC, will be transfered after and if anything
        // they will accumulate in the E/USART's FIFO which is 2 + 1 (for the shift register) = 3 in the worst case (USART)
        SLI_CPC_HDLC_HEADER_SIZE);
      #else
        // In case the CRC HW acceleration is not enabled, transfer the full 7 bytes header like normal
        SLI_CPC_HDLC_HEADER_RAW_SIZE);
      #endif

      // For Series 2 without CRC HW acceleration, the chain ends here (_SINGLE_ => .link = 0)

      #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
      // In case of Series 1 and/or CRC HW acceleration, this descriptor is not the end of the chain and needs to
      // branch to the following one. Undo the _SINGLE_
      rx_desc_recv_header.xfer.link = 1;

      // The macro LDMA_DESCRIPTOR_LINKABS_P2M_BYTE does not exist for whatever reason.
      // Force this descriptor to use absolute linking
      rx_desc_recv_header.xfer.LDMA_DESCRIPTOR_LINK_MODE = LDMA_DESCRIPTOR_LINK_MODE_ABS;

      // Override .LDMA_DESCRIPTOR_DONE_IFS : Don't trigger an interrupt at the end of this descriptor
      rx_desc_recv_header.xfer.LDMA_DESCRIPTOR_DONE_IFS = 0;
      #endif

      #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
      // Fixed branching to the descriptor following
      rx_desc_recv_header.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_wait_gpcrc_sync_bit);
      #endif
    }

    #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
    // Sync descriptor to wait until the payload GPCRC calculation is done
    {
      rx_desc_wait_gpcrc_sync_bit = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_SYNC(
        0,
        0,
        GPCRC_SYNC_BIT_MASK,  // Wait for the GPCRC launch bit to be 1
        GPCRC_SYNC_BIT_MASK); // Wait for the GPCRC launch bit

      rx_desc_wait_gpcrc_sync_bit.sync.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_throw_header_in_gpcrc);
    }

    // Memory-to-Memory descriptor to throw the 5 header byte into the GPCRC
    {
      // Doing M2M single byte transfers with a DMA when the data in on hand is not optimal, but
      // here since its only 5 bytes its okay. The alternative would've been 1 1-word descriptor
      // chaining to 1 1-byte descriptor. The act of fetching a second 4-word descriptor for the
      // remaining byte should be about the same extra bus usage in the end.
      rx_desc_throw_header_in_gpcrc = (ldma_descriptor_t)LDMA_DESCRIPTOR_LINKABS_M2M_BYTE(
        &header_buffer,
        &GPCRC->INPUTDATABYTE,
        SLI_CPC_HDLC_HEADER_SIZE);

      // Because this is a M2M descriptor, the source increments (which is what we want) but the
      // destination as well. Here we send the header bytes to the same register, so override the
      // source increment to stay the same.
      rx_desc_throw_header_in_gpcrc.xfer.dstInc = ldmaCtrlSrcIncNone;

      rx_desc_throw_header_in_gpcrc.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_recv_header_crc);
    }

    // P2M descriptor to receive the remaining 2 CRC bytes of the header from the E/USART
    {
      rx_desc_recv_header_crc = (ldma_descriptor_t)LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(
        &(SL_CPC_DRV_SPI_PERIPHERAL->RXDATA),
        &((uint8_t*)&header_buffer)[SLI_CPC_HDLC_HEADER_SIZE], // Append to the already received 5 bytes of header
        SLI_CPC_HDLC_FCS_SIZE);
    }
    #endif
  }

  #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
  // Prepare the GPCRC descriptor chain
  {
    // Sync descriptor to wait until the payload was received to start computing the CRC, which is set by the RX DMA chain
    {
      gpcrc_desc_wait_for_gpcrc_sync_bit = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_SYNC(
        0,
        0,
        GPCRC_SYNC_BIT_MASK,  // Wait for the GPCRC sync bit to be 1
        GPCRC_SYNC_BIT_MASK); // Wait for the GPCRC sync bit

      // Fixed branching
      gpcrc_desc_wait_for_gpcrc_sync_bit.sync.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&gpcrc_desc_clear_gpcrc_sync_bit);
    }

    // Sync descriptor to clear the launch sync bit
    {
      gpcrc_desc_clear_gpcrc_sync_bit = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_SYNC(
        0,
        GPCRC_SYNC_BIT_MASK, // Immediately clear the GPCRC sync bit
        0,
        0);

      // The branching is computed each time the header interrupt occurs
    }

    // Memory-to-Memory descriptor to write payload into the GPCRC
    // This descriptor takes care of the entire 4-byte words
    // To decrease the time it takes for the DMA to write the data in the GPCRC and decreases the number
    // of transaction on the memory bus, we do as many 4-byte transfers as possible.
    // If the payload length is not divisible by 4, the remaining 1,2 or 3 bytes will be handled by
    // the descriptors that follow
    {
      gpcrc_desc_write_rx_payload_words_in_gpcrc = (ldma_descriptor_t)LDMA_DESCRIPTOR_LINKABS_M2M_WORD(
        NULL,              // Placeholder for the payload pointer
        &GPCRC->INPUTDATA, // The WORD input register
        0);                // Placeholder for the length

      // Because this is a M2M descriptor, the source increments (which is what we want) but the
      // destination as well. Here we send the payload bytes to the same register, so override the
      // source increment to stay the same.
      gpcrc_desc_write_rx_payload_words_in_gpcrc.xfer.dstInc = ldmaCtrlSrcIncNone;

      // The branching will be computed each time the header interrupt happens in function of the payload_len % 4 result
    }

    // M2M descriptor to potentially transfer remaining 1-to-3 bytes of payload into the GPCRC engine (modulo 4 of payload length)
    {
      gpcrc_desc_write_rx_remaining_payload_bytes_in_gpcrc = (ldma_descriptor_t)LDMA_DESCRIPTOR_LINKABS_M2M_BYTE(
        NULL,                  // Placeholder for the payload pointer
        &GPCRC->INPUTDATABYTE, // The BYTE input register
        0);                    // Placeholder, will be 1-to-3

      // Because this is a M2M descriptor, the source increments (which is what we want) but the
      // destination as well. Here we send the payload bytes to the same register, so override the
      // source increment to stay the same.
      gpcrc_desc_write_rx_remaining_payload_bytes_in_gpcrc.xfer.dstInc = ldmaCtrlSrcIncNone;

      // Fixed to the descriptor below
      gpcrc_desc_write_rx_remaining_payload_bytes_in_gpcrc.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&gpcrc_desc_save_rx_gpcrc_computed_crc);
    }

    // M2M descriptor to transfer the single half-word GPCRC result in the buffer_handle .fcs field
    {
      gpcrc_desc_save_rx_gpcrc_computed_crc = (ldma_descriptor_t)LDMA_DESCRIPTOR_LINKABS_M2M_HALF(
        &GPCRC->DATAREV,  // Read the CRC from the engine
        NULL,             // Placeholder for the buffer_handle .fcs pointer
        1);               // Only one half-word

      gpcrc_desc_save_rx_gpcrc_computed_crc.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&gpcrc_desc_set_rxchain_ien_bit);
    }

    // Write descriptor to set RX DMA chain IEN bit to trigger an interrupt
    {
      gpcrc_desc_set_rxchain_ien_bit = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_WRITE(
        (1 << rx_dma_channel),
        LDMA_IEN_SET_REG_ADDR);

      gpcrc_desc_set_rxchain_ien_bit.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&gpcrc_desc_set_gpcrc_sync_bit);
    }

    // Sync descriptor to set the launch sync bit
    {
      gpcrc_desc_set_gpcrc_sync_bit = (ldma_descriptor_t) LDMA_DESCRIPTOR_SINGLE_SYNC(
        GPCRC_SYNC_BIT_MASK, // Immediately set the GPCRC bit
        0,
        0,
        0);

      // Override .LDMA_DESCRIPTOR_DONE_IFS, do not generate interrupt
      gpcrc_desc_set_gpcrc_sync_bit.sync.LDMA_DESCRIPTOR_DONE_IFS = 0;
    }
  }
  #endif

  // Prepare the transmission DMA descriptor chain
  {
    #define ENABLE_SYNC_ON_TX_READY_WINDOW     TX_READY_WINDOW_TRIG_BIT_MASK
    #define SYNC_ON_TX_READY_WINDOW_EQUAL_ONE  TX_READY_WINDOW_TRIG_BIT_MASK

    // Sync descriptor to wait to be in the TX_AVAILABLE region
    {
      tx_desc_wait_availability_sync_bit = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_SYNC(
        0x0,
        0x0,
        SYNC_ON_TX_READY_WINDOW_EQUAL_ONE,
        ENABLE_SYNC_ON_TX_READY_WINDOW);

      // Fixed branching
      tx_desc_wait_availability_sync_bit.sync.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&tx_desc_set_irq_low);
    }

    // Set the IRQ pin LOW
    {
      tx_desc_set_irq_low = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_WRITE(
        IRQ_PIN_SET_MASK,
        IRQ_GPIO_CLR_REG_ADDR);

      tx_desc_set_irq_low.wri.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&tx_desc_xfer_header);
    }

    // Send header
    {
      tx_desc_xfer_header = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(
        NULL, //Place holder for header buffer address
        &(SL_CPC_DRV_SPI_PERIPHERAL->TXDATA),
        SLI_CPC_HDLC_HEADER_RAW_SIZE,
        1); // Overridden below

      // The macro LDMA_DESCRIPTOR_LINKABS_M2P_BYTE does not exist for whatever reason.
      // Force this descriptor to use absolute linking
      tx_desc_xfer_header.xfer.LDMA_DESCRIPTOR_LINK_MODE = LDMA_DESCRIPTOR_LINK_MODE_ABS;

      // Override .LDMA_DESCRIPTOR_DONE_IFS : Don't trigger an interrupt at the end of this descriptor
      tx_desc_xfer_header.xfer.LDMA_DESCRIPTOR_DONE_IFS = 0;

      tx_desc_xfer_header.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&tx_desc_wait_header_tranfered);
    }

    // Wait until the header has been sent on the wire
    {
      #define ENABLE_SYNC_ON_TXC_PRS_CHANNEL     (1 << SL_CPC_DRV_SPI_TXC_SYNCTRIG_PRS_CH)
      #define SYNC_ON_TXC_PRS_CHANNEL_EQUAL_ONE  (1 << SL_CPC_DRV_SPI_TXC_SYNCTRIG_PRS_CH)
      #define CLEAR_TXC_PRS_CHANNEL_UPON_LOAD    (1 << SL_CPC_DRV_SPI_TXC_SYNCTRIG_PRS_CH)

      tx_desc_wait_header_tranfered = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_SYNC(
        0x0,
        CLEAR_TXC_PRS_CHANNEL_UPON_LOAD,
        SYNC_ON_TXC_PRS_CHANNEL_EQUAL_ONE,
        ENABLE_SYNC_ON_TXC_PRS_CHANNEL);

      // Fixed branching
      tx_desc_wait_header_tranfered.sync.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&tx_desc_set_tx_frame_complete_variable_after_header);
    }

    // Write descriptor to set the "tx_frame_complete" variable to 1.
    {
      tx_desc_set_tx_frame_complete_variable_after_header = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKABS_WRITE(
        1,
        &tx_frame_complete);

      // The link bit will be set or cleared depending on whether there is a payload to send

      tx_desc_set_tx_frame_complete_variable_after_header.wri.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&tx_desc_xfer_payload);
    }

    // Branch if there is a payload

    // Transfer descriptor for the payload
    {
      tx_desc_xfer_payload = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(
        NULL, // Place holder for the payload address
        &(SL_CPC_DRV_SPI_PERIPHERAL->TXDATA),
        0, // Place holder for the payload length
        1); // Overridden below

      // The macro LDMA_DESCRIPTOR_LINKABS_M2P_BYTE does not exist for whatever reason.
      // Force this descriptor to use absolute linking
      tx_desc_xfer_payload.xfer.LDMA_DESCRIPTOR_LINK_MODE = LDMA_DESCRIPTOR_LINK_MODE_ABS;

      // Override .LDMA_DESCRIPTOR_DONE_IFS : Don't trigger an interrupt at the end of this descriptor
      tx_desc_xfer_payload.xfer.LDMA_DESCRIPTOR_DONE_IFS = 0;

      // In the case of NO large buffer and NO security, this descriptor branches  to the
      // send checksum descriptor no matter what, else its link address is computed each
      // time the chain is arms. In the case it never changes, set it right now
      tx_desc_xfer_payload.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&tx_desc_xfer_checksum);
    }

    #if (SLI_CPC_DRV_SPI_RX_DATA_MAX_LENGTH > LDMA_DESCRIPTOR_MAX_XFER_SIZE)
    // Transfer descriptor for the large buffer payload
    {
      tx_desc_xfer_payload_large_buf = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(
        NULL, // Place holder for the payload address
        &(SL_CPC_DRV_SPI_PERIPHERAL->TXDATA),
        0, // Place holder for the payload length
        1);

      // The macro LDMA_DESCRIPTOR_LINKABS_P2M_BYTE does not exist, force this descriptor to use absolute linking
      tx_desc_xfer_payload_large_buf.xfer.LDMA_DESCRIPTOR_LINK_MODE = LDMA_DESCRIPTOR_LINK_MODE_ABS;

      // Override .LDMA_DESCRIPTOR_DONE_IFS : Don't trigger an interrupt at the end of this descriptor
      tx_desc_xfer_payload_large_buf.xfer.LDMA_DESCRIPTOR_DONE_IFS = 0;

      // In the case of large buffer and NO security, this descriptor branches  to the
      // send checksum descriptor no matter what, else its link address is computed each
      // time the chain is arms. In the case it never changes, set it right now
      tx_desc_xfer_payload_large_buf.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&tx_desc_xfer_checksum);
    }
    #endif

    #if (SL_CPC_ENDPOINT_SECURITY_ENABLED == 1)
    // If the security is enabled and there is a security tag to send, send it before the payload checksum.
    {
      tx_desc_xfer_tag = (ldma_descriptor_t) LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(
        NULL,                                /* Place holder for the tag address */
        &(SL_CPC_DRV_SPI_PERIPHERAL->TXDATA),
        SLI_SECURITY_TAG_LENGTH_BYTES,
        1u);

      // The macro LDMA_DESCRIPTOR_LINKABS_P2M_BYTE does not exist, force this descriptor to use absolute linking
      tx_desc_xfer_tag.xfer.LDMA_DESCRIPTOR_LINK_MODE = LDMA_DESCRIPTOR_LINK_MODE_ABS;

      // Override .LDMA_DESCRIPTOR_DONE_IFS : Don't trigger an interrupt at the end of this descriptor
      tx_desc_xfer_tag.xfer.LDMA_DESCRIPTOR_DONE_IFS = 0;

      // Fixed branching
      tx_desc_xfer_tag.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&tx_desc_xfer_checksum);
    }
    #endif

    // Send the checksum
    {
      tx_desc_xfer_checksum = (ldma_descriptor_t) LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(
        NULL, // Placeholder for the checksum
        &(SL_CPC_DRV_SPI_PERIPHERAL->TXDATA),
        SLI_CPC_HDLC_FCS_SIZE);

      // Override .LDMA_DESCRIPTOR_DONE_IFS : TX DMA chain doesn't trigger interrupts
      tx_desc_xfer_checksum.xfer.LDMA_DESCRIPTOR_DONE_IFS = 0;
    }
  }

  LOGIC_ANALYZER_TRACE_PIN_INIT;

  #if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
  #endif

  #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
  // GPCRC hardware initialization
  {
    GPCRC_Init_TypeDef init;

    init.crcPoly          = 0x1021; // CRC-16 XMODEM 16 bits polynomial
    init.initValue        = 0x0000; // CRC-16 XMODEM 16 bits init value
    init.reverseByteOrder = false;  // Important: The bytes in the full-word writes are already in the right order
    init.reverseBits      = true;   // Important: Our software CRC implementation has bit order reversed, to match the host we must reverse the bits.
    init.enableByteMode   = false;  // Important: We use the full word-size INPUT capability
    init.autoInit         = true;   // Important: Re-start the GPCRC automatically when the DMA reads the DATA register
    init.enable           = true;

    GPCRC_Init(GPCRC, &init);

    // Manually load the INIT value in the DATA register to initialize the first CRC computation
    // After that, it will be done automatically when reading the DATA register (thanks to the .autoInit=true)
    GPCRC_Start(GPCRC);
  }
  #endif

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Start reception
 ******************************************************************************/
static sl_status_t spi_drv_start_rx(sli_cpc_drv_t *drv)
{
  (void)drv;

  // Due to the nature of the DMA chain, the start descriptor for the initial
  // DMA priming is in the middle of the chain, as opposed to when the DMA
  // is armed in the header interrupt
  ldma_descriptor_t *start_descriptor = &rx_desc_wait_cs_low_before_header;

  // The initial reception priming is special because we prime the RX DMA channel from
  // a descriptor in the middle of the chain.

  *((__IOM uint32_t*) LDMA_SYNCSW_SET_REG_ADDR) = TX_READY_WINDOW_TRIG_BIT_MASK;

  #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
  *((__IOM uint32_t*) LDMA_SYNCSW_SET_REG_ADDR) = GPCRC_SYNC_BIT_MASK;
  #endif

  Ecode_t ecode = DMADRV_LdmaStartTransfer(rx_dma_channel,
                                           &rx_dma_config,
                                           start_descriptor,
                                           rx_dma_callback,
                                           (void*)true); // Give a special "initial_pass" = true parameter.
  SLI_CPC_ASSERT(ecode == ECODE_EMDRV_DMADRV_OK);

  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 * Gets CPC driver capabilities.
 ******************************************************************************/
static sl_status_t spi_drv_get_capabilities(sli_cpc_drv_t *drv,
                                            sli_cpc_drv_capabilities_t *capabilities)
{
  (void)drv;

  if (capabilities == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  *capabilities = (sli_cpc_drv_capabilities_t){.preprocess_hdlc_header = true,
                                               .uart_flowcontrol = false };
  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 * Read bytes from SPI.
 ******************************************************************************/
static sl_status_t spi_drv_read_data(sli_cpc_drv_t *drv,
                                     sl_cpc_buffer_handle_t **buffer_handle)
{
  sl_status_t status;
  MCU_DECLARE_IRQ_STATE;

  (void)drv;

  sl_cpc_buffer_handle_t *new_buffer_handle;

  MCU_ENTER_ATOMIC();
  sl_cpc_buffer_handle_t *pending_buffer_handle = sli_cpc_pop_driver_buffer_handle(&rx_pending_list_head);
  if (SL_BRANCH_UNLIKELY(pending_buffer_handle == NULL)) {
    MCU_EXIT_ATOMIC();
    return SL_STATUS_EMPTY;
  }
  MCU_EXIT_ATOMIC();

  *buffer_handle = pending_buffer_handle;

  MCU_ENTER_ATOMIC();
  status = sli_cpc_get_buffer_handle_for_rx(driver_instance, &new_buffer_handle, true);
  if (SL_BRANCH_LIKELY(status == SL_STATUS_OK)) {
    sli_cpc_push_driver_buffer_handle(&rx_free_list_head, new_buffer_handle);
  }
  MCU_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 * Write bytes to SPI.
 ******************************************************************************/
static sl_status_t spi_drv_transmit_data(sli_cpc_drv_t *drv,
                                         sl_cpc_buffer_handle_t *buffer_handle)
{
  MCU_DECLARE_IRQ_STATE;

  (void)drv;

  MCU_ENTER_ATOMIC();
  if (SL_BRANCH_UNLIKELY(tx_buf_available_count == 0)) {
    MCU_EXIT_ATOMIC();
    return SL_STATUS_NOT_READY;
  }
  tx_buf_available_count--;
  sli_cpc_push_back_driver_buffer_handle(&tx_submitted_list_head, buffer_handle);
  sli_cpc_drv_wake_host_gpio(true);
  prime_dma_for_transmission();

  MCU_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 * Checks if driver is ready to transmit.
 ******************************************************************************/
static bool spi_drv_is_transmit_ready(sli_cpc_drv_t *drv)
{
  uint16_t tx_free;

  (void)drv;

  MCU_ATOMIC_LOAD(tx_free, tx_buf_available_count);
  return (tx_free > 0);
}

/***************************************************************************//**
 * Get currently configured bus bitrate
 ******************************************************************************/
static uint32_t spi_drv_get_bus_bitrate(sli_cpc_drv_t *drv)
{
  (void)drv;

  // Stub function : In the case of the SPI driver, this value is meaningless
  return 0;
}

/***************************************************************************//**
 * Get maximum bus bitrate
 ******************************************************************************/
static uint32_t spi_drv_get_bus_max_bitrate(sli_cpc_drv_t *drv)
{
  uint32_t max_bitrate;

  (void)drv;

#if defined(SL_CPC_DRV_SPI_IS_USART)
  sl_status_t status;
  // The max speed of the USART in mode secondary is the peripheral clock feeding the USART / 10
  sl_clock_branch_t spi_clock_branch = sl_device_peripheral_get_clock_branch(SPI_PERIPHERAL(SL_CPC_DRV_SPI_PERIPHERAL_NO));
  status = sl_clock_manager_get_clock_branch_frequency(spi_clock_branch,
                                                       &max_bitrate);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
  max_bitrate /= 10;

  if (max_bitrate > 6000000) {
    // USART shows issues when running at frequencies higher than 6MHz.
    max_bitrate = 6000000;
  }
#elif defined(SL_CPC_DRV_SPI_IS_EUSART)
  // The max bitrate of the EUSART in secondary mode is 10MHz no matter what.
  max_bitrate = 10000000;
#elif defined(SL_CPC_DRV_SPI_IS_HSPI)
  // The max bitrate of the HSPI in target mode is 100MHz SDR.
  max_bitrate = HSPI_TARGET_MODE_MAX_BITRATE;
#endif

  return max_bitrate;
}

/**************************************************************************/ /**
 * Notification when RX buffer handle becomes free.
 ******************************************************************************/
static void spi_drv_on_rx_buffer_free(sli_cpc_drv_t *drv)
{
  MCU_DECLARE_IRQ_STATE;
  sl_status_t status;

  (void)drv;

  MCU_ENTER_ATOMIC();
  do {
    sl_cpc_buffer_handle_t *buffer_handle;

    status = sli_cpc_get_buffer_handle_for_rx(driver_instance, &buffer_handle, true);
    if (SL_BRANCH_LIKELY(status == SL_STATUS_OK)) {
      sli_cpc_push_driver_buffer_handle(&rx_free_list_head, buffer_handle);
      if (need_rx_buffer_handle) {
        // Driver was out of RX buffer handle. Resume the reception.
        need_rx_buffer_handle = false;
        end_of_header_xfer();
        SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] Got RX buffer handle, resume RX.", __LINE__);
      }
    }
  } while (status == SL_STATUS_OK);
  MCU_EXIT_ATOMIC();
}

/***************************************************************************//**
 * Initialize peripheral clocks.
 ******************************************************************************/
static void init_clocks(void)
{
  sl_status_t status;
  // Enable the clocks of all the peripheral used in this driver
  status = sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
  status = sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
  sl_bus_clock_t spi_bus_clock = sl_device_peripheral_get_bus_clock(SPI_PERIPHERAL(SL_CPC_DRV_SPI_PERIPHERAL_NO));
  status = sl_clock_manager_enable_bus_clock(spi_bus_clock);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
#if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
  status = sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPCRC0);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
#endif
}

/***************************************************************************//**
 * Reconfigure only what needs to be reconfigured in the RX DMA descriptor chain
 * and start the RX DMA channel.
 * Returns if the DMA was successfully started for reception.
 ******************************************************************************/
static bool prime_dma_for_reception(size_t payload_size, bool received_valid_header)
{
  Ecode_t ecode;
  #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT) // With HW CRC
  const uint16_t effective_payload_size = payload_size - SLI_CPC_HDLC_FCS_SIZE;
  const uint16_t remaining_bytes = effective_payload_size % 4;
  bool skip_gpcrc = false;
  #endif

  if (received_valid_header == false) {
    // We received a header full of 0s or a corrupted header, skip the payload(s) descriptors
    rx_desc_set_irq_high.wri.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_rxblocken);

    #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
    skip_gpcrc = true;
    #endif

    goto start_transfer;
  }

  // Since we received a valid header, we are going to need a rx_entry to place it into
  currently_receiving_rx_buffer_handle = sli_cpc_pop_driver_buffer_handle(&rx_free_list_head);

  if (SL_BRANCH_UNLIKELY(currently_receiving_rx_buffer_handle == NULL)) {
    // Ran out of RX buffer. Hold the IRQ line in order to block communication
    // on the primary. Communication will be resumed on next call to
    // spi_drv_on_rx_buffer_free.
    need_rx_buffer_handle = true;
    SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] Out of RX buffers, hold IRQ.", __LINE__);
    return false;
  }

  // Here we have a proper buffer_handle on hand to receive the frame.

  // Copy the valid header from the static header buffer into the buffer_handle
  memcpy(currently_receiving_rx_buffer_handle->hdlc_header, &header_buffer, SLI_CPC_HEADER_SIZE);

  if (payload_size == 0 || SL_BRANCH_UNLIKELY(payload_size > SLI_CPC_DRV_SPI_RX_DATA_MAX_LENGTH)) {
    // Either received a frame with no payload, or a frame with an oversized payload.
    // Either way, skip over payload reception.
    if (SL_BRANCH_UNLIKELY(payload_size > SLI_CPC_DRV_SPI_RX_DATA_MAX_LENGTH)) {
      // Received a valid header from the bus, but with an invalid payload length
      currently_receiving_rx_buffer_handle->reason = SL_CPC_REJECT_ERROR;
      SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] Invalid RX buffer length, discarding payload. Length:", payload_size);
    }

    rx_desc_set_irq_high.wri.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_rxblocken);

    #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
    skip_gpcrc = true;
    #endif

    goto start_transfer;
  }

  // Now that we expect to receive a payload, configure the reception descriptor chain to include the payload reception descriptor
  rx_desc_set_irq_high.wri.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_recv_payload);

  // Set the RX buffer address in the payload descriptor
  rx_desc_recv_payload.xfer.LDMA_DESCRIPTOR_DST_ADDR = (uint32_t) currently_receiving_rx_buffer_handle->data;

  #if (SLI_CPC_DRV_SPI_RX_DATA_MAX_LENGTH <= LDMA_DESCRIPTOR_MAX_XFER_SIZE) // Non-large-buffer
  {
    // Receive the payload + CRC in memory
    rx_desc_recv_payload.xfer.LDMA_DESCRIPTOR_XFER_CNT = payload_size - 1;

    #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT) // With HW CRC
    {
      if (SL_BRANCH_UNLIKELY(effective_payload_size <= 3)) { // In the real world, it will be unlikely that payloads will be that small
        // Since the payload is not even one word long, skip the GPCRC full-word transfer descriptor and branch to the one that transfers single bytes
        gpcrc_desc_clear_gpcrc_sync_bit.sync.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&gpcrc_desc_write_rx_remaining_payload_bytes_in_gpcrc);

        // The "rx_desc_recv_payload" took the payload from the E/USART and wrote it to memory
        // Now take what has been written to memory and write it to the GPCRC;
        gpcrc_desc_write_rx_remaining_payload_bytes_in_gpcrc.xfer.LDMA_DESCRIPTOR_SRC_ADDR = (uint32_t) currently_receiving_rx_buffer_handle->data;

        gpcrc_desc_write_rx_remaining_payload_bytes_in_gpcrc.xfer.LDMA_DESCRIPTOR_XFER_CNT = effective_payload_size - 1;
      } else { // payload length of one full word or more
        // Branch the payload descriptor to the descriptor that transfers full words to the GPCRC
        gpcrc_desc_clear_gpcrc_sync_bit.sync.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&gpcrc_desc_write_rx_payload_words_in_gpcrc);

        // The "rx_desc_recv_payload" took the payload from the E/USART and wrote it to memory
        // Now take what has been written to memory and write it to the GPCRC;
        gpcrc_desc_write_rx_payload_words_in_gpcrc.xfer.LDMA_DESCRIPTOR_SRC_ADDR = (uint32_t) currently_receiving_rx_buffer_handle->data;

        // Unlike the "rx_desc_recv_payload" descriptor who's length was in bytes, this descriptor moves full words around
        // in order to be as efficient and fast as possible.
        gpcrc_desc_write_rx_payload_words_in_gpcrc.xfer.LDMA_DESCRIPTOR_XFER_CNT = (effective_payload_size / 4) - 1;

        if (SL_BRANCH_UNLIKELY(remaining_bytes == 0)) { // With random payload size distribution, having a payload size % 4 == 0 is 1/4
          // The effective payload length was an integer multiple of word-size, skip the descriptor that transfer the remainder
          gpcrc_desc_write_rx_payload_words_in_gpcrc.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&gpcrc_desc_save_rx_gpcrc_computed_crc);
        } else {
          // If 3,2 or 1 bytes of payload remain after full word transfers, link to the byte-transfer descriptor
          gpcrc_desc_write_rx_payload_words_in_gpcrc.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&gpcrc_desc_write_rx_remaining_payload_bytes_in_gpcrc);

          gpcrc_desc_write_rx_remaining_payload_bytes_in_gpcrc.xfer.LDMA_DESCRIPTOR_SRC_ADDR = (uint32_t) ((uintptr_t)currently_receiving_rx_buffer_handle->data + effective_payload_size - remaining_bytes);

          gpcrc_desc_write_rx_remaining_payload_bytes_in_gpcrc.xfer.LDMA_DESCRIPTOR_XFER_CNT = remaining_bytes - 1;
        }
      }
    }
    #endif
  }
  #else // Large-buffer
  {
    #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT) // With HW CRC
    {
      if (SL_BRANCH_LIKELY(payload_size <= LDMA_DESCRIPTOR_MAX_XFER_SIZE)) { // Using only one payload descriptor
        rx_desc_recv_payload.xfer.LDMA_DESCRIPTOR_XFER_CNT = payload_size - 1;

        // Skip over the large buffer descriptor
        rx_desc_recv_payload.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_rxblocken);

        if (SL_BRANCH_UNLIKELY(effective_payload_size <= 3)) { // In the real world, it will be unlikely that payloads will be that small
          // Since the payload is not even one word long, skip the GPCRC full-word transfer descriptor and branch to the one that transfers single bytes
          gpcrc_desc_clear_gpcrc_sync_bit.sync.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&gpcrc_desc_write_rx_remaining_payload_bytes_in_gpcrc);

          // The "rx_desc_recv_payload" took the payload from the E/USART and wrote it to memory
          // Now take what has been written to memory and write it to the GPCRC;
          gpcrc_desc_write_rx_remaining_payload_bytes_in_gpcrc.xfer.LDMA_DESCRIPTOR_SRC_ADDR = (uint32_t) currently_receiving_rx_buffer_handle->data;

          gpcrc_desc_write_rx_remaining_payload_bytes_in_gpcrc.xfer.LDMA_DESCRIPTOR_XFER_CNT = effective_payload_size - 1;

          goto start_transfer;
        } else { // payload length of one full word or more
          // Branch the payload descriptor to the descriptor that transfers full words to the GPCRC
          gpcrc_desc_clear_gpcrc_sync_bit.sync.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&gpcrc_desc_write_rx_payload_words_in_gpcrc);

          // The "rx_desc_recv_payload" took the payload from the E/USART and wrote it to memory
          // Now take what has been written to memory and write it to the GPCRC;
          gpcrc_desc_write_rx_payload_words_in_gpcrc.xfer.LDMA_DESCRIPTOR_SRC_ADDR = (uint32_t) currently_receiving_rx_buffer_handle->data;

          // Unlike the "rx_desc_recv_payload" descriptor who's length was in bytes, this descriptor moves full words around
          // in order to be as efficient and fast as possible.
          gpcrc_desc_write_rx_payload_words_in_gpcrc.xfer.LDMA_DESCRIPTOR_XFER_CNT = (effective_payload_size / 4) - 1;
        }
      } else { // Using second payload descriptor
        // We have large buffer compiled, and the payload spans the two payload descriptors
        // load the first descriptor to the max, and link to the one right under it
        rx_desc_recv_payload.xfer.LDMA_DESCRIPTOR_XFER_CNT = LDMA_DESCRIPTOR_MAX_XFER_SIZE - 1;
        rx_desc_recv_payload.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_recv_payload_large_buf);

        // Fill the second payload descriptor with the remaining data
        rx_desc_recv_payload_large_buf.xfer.LDMA_DESCRIPTOR_DST_ADDR = (uint32_t) &((uint8_t*)currently_receiving_rx_buffer_handle->data)[LDMA_DESCRIPTOR_MAX_XFER_SIZE];
        rx_desc_recv_payload_large_buf.xfer.LDMA_DESCRIPTOR_XFER_CNT = (payload_size - LDMA_DESCRIPTOR_MAX_XFER_SIZE) - 1;

        // Branch the payload descriptor to the descriptor that transfers full words to the GPCRC
        gpcrc_desc_clear_gpcrc_sync_bit.sync.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&gpcrc_desc_write_rx_payload_words_in_gpcrc);

        // The "rx_desc_recv_payload" and "rx_desc_recv_payload_large_buf" took the payload from the E/USART and
        // wrote it to memory. Now take what has been written to memory and write it to the GPCRC;
        gpcrc_desc_write_rx_payload_words_in_gpcrc.xfer.LDMA_DESCRIPTOR_SRC_ADDR = (uint32_t) currently_receiving_rx_buffer_handle->data;

        // Unlike the "rx_desc_recv_payload" descriptor who's length was in bytes, this descriptor moves full words around
        // in order to be as efficient and fast as possible.
        gpcrc_desc_write_rx_payload_words_in_gpcrc.xfer.LDMA_DESCRIPTOR_XFER_CNT = (effective_payload_size / 4) - 1;
      }

      // Take care of the remaining %4 bytes
      if (SL_BRANCH_UNLIKELY(remaining_bytes == 0)) { // With random payload size distribution, having a payload size % 4 == 0 is 1/4
        // The effective payload length was an integer multiple of word-size, skip the descriptor that transfer the remainder
        gpcrc_desc_write_rx_payload_words_in_gpcrc.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&gpcrc_desc_save_rx_gpcrc_computed_crc);
      } else {
        // If 3,2 or 1 bytes of payload remain after full word transfers, link to the byte-transfer descriptor
        gpcrc_desc_write_rx_payload_words_in_gpcrc.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&gpcrc_desc_write_rx_remaining_payload_bytes_in_gpcrc);

        gpcrc_desc_write_rx_remaining_payload_bytes_in_gpcrc.xfer.LDMA_DESCRIPTOR_SRC_ADDR = (uint32_t) ((uintptr_t)currently_receiving_rx_buffer_handle->data + effective_payload_size - remaining_bytes);

        gpcrc_desc_write_rx_remaining_payload_bytes_in_gpcrc.xfer.LDMA_DESCRIPTOR_XFER_CNT = remaining_bytes - 1;
      }
    }
    #else // Without HW CRC
    {
      if (SL_BRANCH_LIKELY(payload_size <= LDMA_DESCRIPTOR_MAX_XFER_SIZE)) {
        // We have large buffer compiled, but this payload doesn't span 2 descriptors.
        // load the first descriptor, and jump over the large-buffer reception descriptor.
        rx_desc_recv_payload.xfer.LDMA_DESCRIPTOR_XFER_CNT = payload_size - 1;
        rx_desc_recv_payload.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_rxblocken);
      } else {
        // We have large buffer compiled, and the payload spans the two payload descriptors
        // load the first descriptor to the max, and link to the one right under it
        rx_desc_recv_payload.xfer.LDMA_DESCRIPTOR_XFER_CNT = LDMA_DESCRIPTOR_MAX_XFER_SIZE - 1;
        rx_desc_recv_payload.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&rx_desc_recv_payload_large_buf);

        // We have large buffer compiled, and the payload spans the two payload descriptors
        // load the first descriptor to the max, and link to the one right under it
        rx_desc_recv_payload_large_buf.xfer.LDMA_DESCRIPTOR_DST_ADDR = (uint32_t) &((uint8_t*)currently_receiving_rx_buffer_handle->data)[LDMA_DESCRIPTOR_MAX_XFER_SIZE];
        rx_desc_recv_payload_large_buf.xfer.LDMA_DESCRIPTOR_XFER_CNT = (payload_size - LDMA_DESCRIPTOR_MAX_XFER_SIZE) - 1;
      }
    }
    #endif
  }
  #endif

  start_transfer:

  #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
  {
    // Store the GPCRC's computed CRC in the buffer_handle when it is done computing
    gpcrc_desc_save_rx_gpcrc_computed_crc.xfer.LDMA_DESCRIPTOR_DST_ADDR = (uint32_t) &currently_receiving_rx_buffer_handle->fcs;
  }
  #endif

  ecode = DMADRV_LdmaStartTransfer(rx_dma_channel,
                                   &rx_dma_config,
                                   &rx_desc_wait_cs_high_after_header,
                                   rx_dma_callback,
                                   NULL);
  SLI_CPC_ASSERT(ecode == ECODE_EMDRV_DMADRV_OK);

  #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
  {
    if (skip_gpcrc == false) {
      // By clearing the IEN bit, the RX DMA chain will still set its interrupt flag then the payload is done
      // receiving, but it ensures the interrupt is not triggered until the GPCRC chain sets back the RX chain IEN bit.
      // This is to ensure the RX DMA chain interrupt bit set is not lost to the header interrupt.
      *((volatile uint32_t*) LDMA_IEN_CLR_REG_ADDR) = (1 << rx_dma_channel);

      // Ensures that the launch SYNC bit that was set at the end of the last transaction is clear so that the GPCRC chain
      // starts by waiting against the RX chain
      *((volatile uint32_t*) LDMA_SYNCSW_CLR_REG_ADDR) = GPCRC_SYNC_BIT_MASK;

      ecode = DMADRV_LdmaStartTransfer(gpcrc_dma_channel,
                                       &gpcrc_dma_config,
                                       &gpcrc_desc_wait_for_gpcrc_sync_bit,
                                       NULL,
                                       NULL);
      SLI_CPC_ASSERT(ecode == ECODE_EMDRV_DMADRV_OK);
    }
  }
  #endif

  return true;
}

/***************************************************************************//**
 * Reconfigure only what needs to be reconfigured in the TX DMA descriptor chain
 * and start the TX DMA channel.
 *
 * We assume the TX DMA channel is not running and we are in interrupt context
 ******************************************************************************/
static void prime_dma_for_transmission(void)
{
  if (currently_transmiting_buffer_handle) {
    // There is already a frame programmed to be sent. Do nothing now, when the
    // frame currently programmed in the TX DMA chain gets tx_flushed(), the
    // frame for which this function was called will be cocked in.
    return;
  }

  // Bug if this function is called but there is nothing in the submitted list
  SLI_CPC_ASSERT(!sl_slist_is_empty(tx_submitted_list_head));

  // Pick the next frame to send
  currently_transmiting_buffer_handle = SL_SLIST_ENTRY(tx_submitted_list_head, sl_cpc_buffer_handle_t, driver_node);

  #if defined(CPC_TEST_SPI_DRIVER_CRC_ERROR_INJECTION)
  {
    // De-phase a bit
    static size_t count = CPC_TEST_SPI_DRIVER_CRC_ERROR_INJECTION_FREQUENCY / 4;
    static uint64_t bad_crc_header;

    count++;

    if ( count % CPC_TEST_SPI_DRIVER_CRC_ERROR_INJECTION_FREQUENCY == 0) {
      count = 0;

      printf("Invalidated the transmit header CRC\n");

      // Make a copy of the good header we are going to transmit
      // We have to make a copy, because otherwise if we modify the good header itself,
      // we will retransmit a bad header indefinitely.
      bad_crc_header = *(uint64_t*)currently_transmiting_buffer_handle->hdlc_header;

      // Invert a byte to corrupt the CRC
      ((uint8_t*)&bad_crc_header)[4] = ~((uint8_t*)&bad_crc_header)[4];

      // set header source address to the bad header
      tx_desc_xfer_header.xfer.LDMA_DESCRIPTOR_SRC_ADDR = (uint32_t) &bad_crc_header;
    } else {
      tx_desc_xfer_header.xfer.LDMA_DESCRIPTOR_SRC_ADDR = (uint32_t) currently_transmiting_buffer_handle->hdlc_header;
    }
  }
  #else
  // set header source address
  tx_desc_xfer_header.xfer.LDMA_DESCRIPTOR_SRC_ADDR = (uint32_t) currently_transmiting_buffer_handle->hdlc_header;
  #endif

  if (currently_transmiting_buffer_handle->data_length == 0) {
    // Turn off branching to payload descriptor
    tx_desc_set_tx_frame_complete_variable_after_header.wri.link = 0;

    goto start_transfer;
  }

  // Turn on branching to payload descriptor
  tx_desc_set_tx_frame_complete_variable_after_header.wri.link = 1;

  // Set the payload source address
  tx_desc_xfer_payload.xfer.LDMA_DESCRIPTOR_SRC_ADDR = (uint32_t) currently_transmiting_buffer_handle->data;

  #if (SLI_CPC_DRV_SPI_RX_DATA_MAX_LENGTH <= LDMA_DESCRIPTOR_MAX_XFER_SIZE) //Non-large-buffer
  {
    tx_desc_xfer_payload.xfer.LDMA_DESCRIPTOR_XFER_CNT = currently_transmiting_buffer_handle->data_length - 1;

    #if (SL_CPC_ENDPOINT_SECURITY_ENABLED == 1)
    {
      if (SL_BRANCH_LIKELY(currently_transmiting_buffer_handle->security_tag)) {
        tx_desc_xfer_payload.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&tx_desc_xfer_tag);
        tx_desc_xfer_tag.xfer.LDMA_DESCRIPTOR_SRC_ADDR = (uint32_t) currently_transmiting_buffer_handle->security_tag;
      } else {
        tx_desc_xfer_payload.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&tx_desc_xfer_checksum);
      }
      tx_desc_xfer_checksum.xfer.LDMA_DESCRIPTOR_SRC_ADDR = (uint32_t) &currently_transmiting_buffer_handle->fcs;
    }
    #endif
  }
  #else // Large-buffer
  {
    if (SL_BRANCH_LIKELY(currently_transmiting_buffer_handle->data_length <= LDMA_DESCRIPTOR_MAX_XFER_SIZE)) {
      // We have large buffer compiled, but this payload doesn't span 2 descriptors.
      // load the first descriptor, and jump over the one that follows.
      tx_desc_xfer_payload.xfer.LDMA_DESCRIPTOR_XFER_CNT = currently_transmiting_buffer_handle->data_length - 1;

      #if (SL_CPC_ENDPOINT_SECURITY_ENABLED == 1)
      {
        if (currently_transmiting_buffer_handle->security_tag) {
          tx_desc_xfer_payload.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&tx_desc_xfer_tag);
          tx_desc_xfer_tag.xfer.LDMA_DESCRIPTOR_SRC_ADDR = (uint32_t) currently_transmiting_buffer_handle->security_tag;
        } else {
          tx_desc_xfer_payload.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&tx_desc_xfer_checksum);
        }
      }
      #else
      {
        tx_desc_xfer_payload.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&tx_desc_xfer_checksum);
      }
      #endif
    } else {
      // We have large buffer compiled, and the payload spans the two payload descriptors
      // load the first payload descriptor to the max, and link it to the large buffer payload descriptor
      tx_desc_xfer_payload.xfer.LDMA_DESCRIPTOR_XFER_CNT = LDMA_DESCRIPTOR_MAX_XFER_SIZE - 1;
      tx_desc_xfer_payload.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&tx_desc_xfer_payload_large_buf);

      // Load the large buffer payload descriptor with the remaining data
      tx_desc_xfer_payload_large_buf.xfer.LDMA_DESCRIPTOR_SRC_ADDR = (uint32_t) &((uint8_t*)currently_transmiting_buffer_handle->data)[LDMA_DESCRIPTOR_MAX_XFER_SIZE];
      tx_desc_xfer_payload_large_buf.xfer.LDMA_DESCRIPTOR_XFER_CNT = (currently_transmiting_buffer_handle->data_length - LDMA_DESCRIPTOR_MAX_XFER_SIZE) - 1;

      #if (SL_CPC_ENDPOINT_SECURITY_ENABLED == 1)
      {
        if (currently_transmiting_buffer_handle->security_tag) {
          tx_desc_xfer_payload_large_buf.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&tx_desc_xfer_tag);
          tx_desc_xfer_tag.xfer.LDMA_DESCRIPTOR_SRC_ADDR = (uint32_t) currently_transmiting_buffer_handle->security_tag;
        } else {
          tx_desc_xfer_payload_large_buf.xfer.LDMA_DESCRIPTOR_LINK_ADDR = LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&tx_desc_xfer_checksum);
        }
      }
      #endif
    }
  }
  #endif

  // Finally load the checksum descriptor
  tx_desc_xfer_checksum.xfer.LDMA_DESCRIPTOR_SRC_ADDR = (uint32_t) &currently_transmiting_buffer_handle->fcs;

  Ecode_t ecode;

  start_transfer:

  ecode = DMADRV_LdmaStartTransfer(tx_dma_channel,
                                   &tx_dma_config,
                                   &tx_desc_wait_availability_sync_bit,
                                   NULL,
                                   NULL);
  SLI_CPC_ASSERT(ecode == ECODE_EMDRV_DMADRV_OK);

  LOGIC_ANALYZER_TRACE_TX_DMA_ARMED;
}

/***************************************************************************//**
 * This is the callback called by the DMADRV IRQ handler when the RX DMA channel
 * fires an interrupt. Note that thanks to the synchronization between TX and RX
 * channels, only the RX channel triggers interrupts, not the TX channel; it
 * silently finishes its descriptor chain
 ******************************************************************************/
static bool rx_dma_callback(unsigned int channel, unsigned int sequenceNo, void *userParam)
{
  SLI_CPC_ASSERT(channel == rx_dma_channel);
  SLI_CPC_ASSERT(sequenceNo < 3);
  dma_irq_seq_no = sequenceNo;

  // A full frame transmission is the transmission of a header, then a payload.
  // But when it comes to the RX DMA descriptor chain, it is not "in phase" with
  // the flow of a transmission. The DMA descriptor chain is armed during the header
  // interrupt, so the beginning of the chain is actually dealing with the payload, then
  // the header of the NEXT frame.
  // For this one RX DMA chain, the interrupt bit is set in two places, hence generating
  // two interrupts : one for the header, and one for the payload.
  // The way the DMADRV addresses the possibility of a channel raising multiple time an interrupt
  // during one DMA chain is with the "sequenceNo". When a channel is armed, the driver resets the
  // sequence to 0, and each time this callback is called, it is increased.
  // Whether the "sequenceNo" is 1 or 2 determines if this callback services the first or second
  // interrupt bit set event. In this case, whether the interrupt is for the header or payload.
  // Counter intuitively (because header is transfered before payload) :
  // sequenceNo == 1 -> payload interrupt
  // sequenceNo == 2 -> header interrupt

  // Because the very first time the RX DMA chain is armed during the init and not during the header interrupt,
  // the chain is not armed from the start, but rather from the middle, resulting in  the "sequenceNo"  swapped.
  // Unlike during normal operation, the initial arming of the RX DMA chain passed "true" as the user parameter.
  bool initial_pass = (bool) userParam;
  if (SL_BRANCH_UNLIKELY(initial_pass)) {
    sequenceNo = 2;
  }

  if (sequenceNo == 2) {
    end_of_header_xfer();
  } else { // sequenceNo == 1
    bool piggy_back = end_of_payload_xfer();

    if (SL_BRANCH_UNLIKELY(piggy_back)) {
      // There must have been a lot of interrupt latency and "end_of_payload_xfer" was delayed so long
      // that it was detected at the end of the servicing of the payload that the header of a next frame
      // was already fully received and ready to be serviced. This resulted in the LDMA interrupt bit for
      // this channel to be set while it was already set and not yet serviced, effectively "loosing" the
      // second interrupt bit set event. If we do not piggy-back the servicing of the header, that event
      // would be lost. Luckily we were able to logically detect that in the "end_of_payload_xfer" routine
      end_of_header_xfer();
    }
  }

  // The return value has no effect since the channel is not configured as "ping-pong"
  return false;
}

static void end_of_header_xfer(void)
{
  size_t rx_payload_length = 0;
  bool rx_dma_primed;
  bool received_valid_header = false;

  LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_START;

  #if defined(CPC_TEST_SPI_DRIVER_CRC_ERROR_INJECTION)
  {
    static size_t count = 0;

    if (header_buffer != 0) {
      count++;

      if ( count % CPC_TEST_SPI_DRIVER_CRC_ERROR_INJECTION_FREQUENCY == 0) {
        count = 0;

        printf("Invalidated the CRC\n");

        // Mess with the CRC by inverting a byte
        ((uint8_t*)&header_buffer)[4] = ~((uint8_t*)&header_buffer)[4];
      }
    }
  }
  #endif

#if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
  // GPCRC's autoInit is enabled, having read the GPCRC->DATA register restarted the GPCRC
  uint16_t computed_crc = (uint16_t) GPCRC->DATAREV;
#endif

  if (header_buffer != 0x0000000000000000) {
    #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
    received_valid_header = computed_crc == sli_cpc_hdlc_get_hcs((void*)&header_buffer);
    #else
    received_valid_header = sli_cpc_validate_crc_sw((uint8_t*)&header_buffer,
                                                    SLI_CPC_HDLC_HEADER_SIZE,
                                                    sli_cpc_hdlc_get_hcs((void*)&header_buffer));
    #endif

    if (SL_BRANCH_LIKELY(received_valid_header)) {
      rx_payload_length = sli_cpc_hdlc_get_length((void*)&header_buffer);
      if (SL_BRANCH_UNLIKELY(rx_payload_length > SLI_CPC_DRV_SPI_RX_DATA_MAX_LENGTH)) {
        SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] Received header for oversized payload, discarding payload.", __LINE__);
        received_valid_header = false;
        rx_payload_length = 0;
      }
    } else {
      SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] Invalid header checksum, discarding payload.", __LINE__);
      SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] Invalid header[63:32]", (uint32_t)(header_buffer >> 32));
      SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] Invalid header[31:0]", (uint32_t)(header_buffer & 0xFFFFFFFF));
    }
  }

  rx_dma_primed = prime_dma_for_reception(rx_payload_length, received_valid_header);

  if (SL_BRANCH_LIKELY(rx_dma_primed)) {
    // Ready for payload reception.
    // Clear the IRQ line to send the signal to the primary that we are done with
    // this interrupt critical section. The primary is now free to start clocking
    // us data.
    GPIO_CLR_OUT_PIN(SL_CPC_DRV_SPI_IRQ_PORT, SL_CPC_DRV_SPI_IRQ_PIN);
  } else {
    // Out of RX buffer handles. IRQ will be held until
    // spi_drv_on_rx_buffer_free is called.
    #if defined (LDMA_HAS_SET_CLEAR)
    LDMA_PERIPH->IF_CLR = (1 << rx_dma_channel);
    #else
    LDMA_PERIPH->IFC = (1 << rx_dma_channel);;
    #endif
  }

  LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_END;
}

/***************************************************************************/ /**
 * Notifies the core when a frame has been sent on the wire.
 *
 * @return true  : "end_of_header_xfer" needs to be tail chained to this routine
 *         false : The DMA callback can return after this function
 ******************************************************************************/
static bool end_of_payload_xfer(void)
{
  bool tfer_done;
  Ecode_t ecode;
  // Used to keep track of whether the buffer held in "currently_transmiting_buffer_handle" did go out on the wire
  // after the last transaction completion or not.
  bool pending_late_header = false;

  LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_START;

  // Check if we had a late header transmission situation
  if (currently_transmiting_buffer_handle) {
    // At this point, a header have been exchanged and we realize a TX entry is registered for transmission
    // We need to know if this "currently_transmiting_buffer_handle" had the chance to have its header clocked in the header
    // exchange that just happened
    ecode = DMADRV_TransferDone(tx_dma_channel, &tfer_done);
    SLI_CPC_ASSERT(ecode == ECODE_EMDRV_DMADRV_OK);

    if (SL_BRANCH_UNLIKELY(tx_frame_complete == 0 || !tfer_done)) {
      // This "currently_transmiting_buffer_handle" has arrived lated. Yes it is at this point the current active TX frame, but
      // at the moment the primary started clocking the header, its header did't go through. Mark this current
      // TX entry transmission as a "pending_late_header" so that we don't try to flush this "currently_transmiting_buffer_handle"
      // as having been sent on the wire.
      SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] Submitted late TX frame, will be sent on next transfer.", __LINE__);
      pending_late_header = true;
    }
  }

  flush_rx();

  if (SL_BRANCH_LIKELY(pending_late_header == false)) {
    flush_tx();

    if (tx_submitted_list_head) {
      prime_dma_for_transmission();
    } else {
      sli_cpc_drv_wake_host_gpio(false);
    }
  }

  LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_END;
  ecode = DMADRV_TransferDone(rx_dma_channel, &tfer_done);
  SLI_CPC_ASSERT(ecode == ECODE_EMDRV_DMADRV_OK);
  if (SL_BRANCH_UNLIKELY(tfer_done)) {
    // If the RX DMA channel is already done here, it means as we just finished dealing with this frame,
    // the header of the next frame was received. This happens when this interrupt suffered a high interrupt
    // latency and its execution was delayed so much that the header interrupt of the next header is also pending.
    // Since there is one interrupt flag for this channel and its been set twice (once by this payload interrupt
    // descriptor, and once by the next header descriptor) the two event blended into one. If we were to just return
    // from the interrupt, the next header interrupt would be lost. Return true so that the higher callback can
    // call directly the header interrupt routine.
    SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] Received header before previous RX'd frame was processed (could indicate high IRQ latency, or a quick primary).", __LINE__);

    // Make sure to clear the DMA IRQ flag here. If the header arrived during the
    // execution of the handler, the IRQ flag will be set from the RX header descriptor
    // completing. If this happens, the handler will execute once more as though
    // a payload was received, when in fact it was the header that caused the flag
    // to be set. In other words, in order to avoid IRQ handler re-entry and an
    // eventual de-sync, make sure we clear the IRQ flag, because the header
    // reception has already been handled.
    LDMA_CLEAR_CH_IRQ(rx_dma_channel);
    return true;
  } else {
    return false;
  }
}

/***************************************************************************/ /**
 * Notifies the core when a frame has been sent on the wire.
 ******************************************************************************/
static void flush_rx(void)
{
  if (currently_receiving_rx_buffer_handle == NULL) {
    // Noting to flush to the core
    return;
  }

  #if defined(CPC_TEST_SPI_DRIVER_CRC_ERROR_INJECTION)
  {
    // Start the payload CRC error injection out of phase with the header error
    static size_t count = CPC_TEST_SPI_DRIVER_CRC_ERROR_INJECTION_FREQUENCY / 2;

    if (sli_cpc_hdlc_get_length((void*)&header_buffer) != 0) {
      count++;

      if ( count % CPC_TEST_SPI_DRIVER_CRC_ERROR_INJECTION_FREQUENCY == 0) {
        count = 0;

        printf("Invalidated the payload CRC\n");

        // Mess with the first byte of the payload CRC by inverting it
        ((uint8_t*)currently_receiving_rx_buffer_handle->data)[0] = ~((uint8_t*)currently_receiving_rx_buffer_handle->data)[0];
      }
    }
  }
  #endif

  currently_receiving_rx_buffer_handle->data_length = sli_cpc_hdlc_get_length(currently_receiving_rx_buffer_handle->hdlc_header);
  sli_cpc_push_back_driver_buffer_handle(&rx_pending_list_head, currently_receiving_rx_buffer_handle);

  currently_receiving_rx_buffer_handle = NULL;

  sli_cpc_notify_rx_data_from_drv(driver_instance);
}

/***************************************************************************/ /**
 * Notifies the core when a frame has been sent on the wire.
 ******************************************************************************/
static void flush_tx(void)
{
  if (currently_transmiting_buffer_handle == NULL) {
    // Nothing to notify the core about if there is no current TX entry
    return;
  }

  // Reset that variable which is set to 1 by the TX DMA chain
  tx_frame_complete = 0;

  // Remove the first entry from the TX submitted list.
  sl_cpc_buffer_handle_t *buffer_handle = sli_cpc_pop_driver_buffer_handle(&tx_submitted_list_head);

  // Paranoia. The first entry in the TX submitted list NEEDS to be the currently_transmiting_buffer_handle
  SLI_CPC_ASSERT(buffer_handle == currently_transmiting_buffer_handle);

  // Notify the core that this entry has been sent on the wire. It will detach its buffer
  sli_cpc_notify_tx_data_by_drv(buffer_handle);

  ++tx_buf_available_count;
  SLI_CPC_ASSERT(tx_buf_available_count <= SL_CPC_DRV_SPI_TX_QUEUE_SIZE);

  // Important to set this back to NULL
  currently_transmiting_buffer_handle = NULL;

  LOGIC_ANALYZER_TRACE_TX_FLUSHED;
}

/***************************************************************************//**
 * @brief Initializes the GPIO wake pin used to wake up the host/primary device.
 *
 ******************************************************************************/
SL_WEAK void sli_cpc_drv_wake_gpio_init(void)
{
  // User implementation for initializing the GPIO wake pin.
}

/***************************************************************************//**
 * @brief Sets the GPIO wake pin to the specified active state.
 *
 * @note This function is called during an IRQ context.
 *
 * @param[in] active  If true, sets the GPIO pin to active state;
 *                    otherwise, sets it to inactive state.
 ******************************************************************************/
SL_WEAK void sli_cpc_drv_wake_host_gpio(bool active)
{
  (void)active;
}
