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
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif

#include "SPI.h"
#include "rsi_ccp_common.h"
#include "rsi_debug.h"
#include "rsi_rom_clks.h"
#include "rsi_common_apis.h"
#include "UDMA.h"
#include "rsi_rom_udma_wrapper.h"
#include "sl_cpc_drv_secondary_spi_config.h"
#include "rsi_driver.h"
#include "sli_cpc.h"
#include "sli_cpc_drv.h"
#include "sli_cpc_hdlc.h"
#include "sli_cpc_crc.h"
#include "sli_cpc_debug.h"
#include "sl_cpc_config.h"
#include "em_device.h"

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/
#if defined(SLI_SI91X_MCU_INTERFACE)
#if ((RTE_SSI_SLAVE_RX_DMA == 0) && (RTE_SSI_SLAVE_TX_DMA == 0))
#error \
  "CPC SPI driver needs both RTE_SSI_SLAVE_RX_DMA and RTE_SSI_SLAVE_TX_DMA, enable the DMA channels in RTE_Device_917.h"
#endif
#endif
#define DMA_MAX_XFER_LEN         1024
#define DMA_MAX_XFER_PAYLOAD_LEN 1015
#define SET_IRQ_HIGH             1u
#define SET_IRQ_LOW              0u
typedef struct {
  sl_slist_node_t node;
  sl_cpc_buffer_handle_t *handle;
  uint16_t payload_len;
} sli_buf_entry_t;

enum header_situation { HEADER_CORRUPTED, HEADER_NULL, HEADER_VALID };

typedef enum {
  SPI_CPC_IDLE = 0,
  SPI_CPC_TX_HEADER,
  SPI_CPC_TX_FLUSH,
  SPI_CPC_TX_PAYLOAD,
  SPI_CPC_RX_HEADER,
  SPI_CPC_RX_PAYLOAD,
  SPI_CPC_RX_HEADER_PROCESS,
  SPI_CPC_FLUSH,
  SPI_CPC_END // Add any new enum's on top of this
} CPC_STATE_T;

typedef enum {
  IRQ_IDLE = 0,
  IRQ_TX_HEADER,
  IRQ_TX_PAYLOAD,
  IRQ_RX_HEADER,
  IRQ_RX_PAYLOAD,
  IRQ_END // Add any new enum's on top of this
} IRQ_STATE_T;

#if defined(DEBUG_LOG)
typedef enum { tx_header_idx = 0, tx_payload_idx, rx_header_idx, rx_payload_idx, tx_rx_idx_max } debug_idx_t;

typedef struct {
  //debug_idx_t init_index;
  CPC_STATE_T init_state;
  CPC_STATE_T runtime_state;
  bool no_rx_payload_triggered;
  bool no_rx_payload_cleared;
  uint8_t no_rx_payload_clearedcount;
  uint8_t no_rx_payload_setcount;
  uint32_t calling_count;
  uint32_t response_count;
} debug_status_t;
#endif
/*******************************************************************************
 *******************   LOGIC ANALYZER MACROS   *********************************
 ******************************************************************************/

#ifdef LOGIC_ANALYZER_TRACES
#define GPIO_SPI_CB_DEBUG_PAD_SEL 2
#define GPIO_SPI_CB_DEBUG_PORT    0 // Port selection
#define GPIO_SPI_CB_DEBUG_PIN     7 // Pin number to connect

#define GPIO_SPI_FI_DEBUG_PAD_SEL 1
#define GPIO_SPI_FI_DEBUG_PORT    0 // Port selection
#define GPIO_SPI_FI_DEBUG_PIN     6 // Pin number to connect

#define LOGIC_ANALYZER_TRACE_PIN_INIT                                                              \
  RSI_EGPIO_PadSelectionEnable(GPIO_SPI_CB_DEBUG_PAD_SEL);                                         \
  RSI_EGPIO_SetPinMux(EGPIO, GPIO_SPI_CB_DEBUG_PORT, GPIO_SPI_CB_DEBUG_PIN, EGPIO_PIN_MUX_MODE0);  \
  RSI_EGPIO_SetDir(EGPIO, GPIO_SPI_CB_DEBUG_PORT, GPIO_SPI_CB_DEBUG_PIN, EGPIO_CONFIG_DIR_OUTPUT); \
  RSI_EGPIO_SetPin(EGPIO, GPIO_SPI_CB_DEBUG_PORT, GPIO_SPI_CB_DEBUG_PIN, 0u);                      \
                                                                                                   \
  RSI_EGPIO_PadSelectionEnable(GPIO_SPI_FI_DEBUG_PAD_SEL);                                         \
  RSI_EGPIO_SetPinMux(EGPIO, GPIO_SPI_FI_DEBUG_PORT, GPIO_SPI_FI_DEBUG_PIN, EGPIO_PIN_MUX_MODE0);  \
  RSI_EGPIO_SetDir(EGPIO, GPIO_SPI_FI_DEBUG_PORT, GPIO_SPI_FI_DEBUG_PIN, EGPIO_CONFIG_DIR_OUTPUT); \
  RSI_EGPIO_SetPin(EGPIO, GPIO_SPI_FI_DEBUG_PORT, GPIO_SPI_FI_DEBUG_PIN, 0u);

#define LOGIC_ANALYZER_TRACE_SPI_DMA_CB_START RSI_EGPIO_SetPin(EGPIO, GPIO_SPI_CB_DEBUG_PORT, GPIO_SPI_CB_DEBUG_PIN, 1u)
#define LOGIC_ANALYZER_TRACE_SPI_DMA_CB_END   RSI_EGPIO_SetPin(EGPIO, GPIO_SPI_CB_DEBUG_PORT, GPIO_SPI_CB_DEBUG_PIN, 0u)
#define LOGIC_ANALYZER_TRACE_SPI_FI_START     RSI_EGPIO_SetPin(EGPIO, GPIO_SPI_FI_DEBUG_PORT, GPIO_SPI_FI_DEBUG_PIN, 1u)
#define LOGIC_ANALYZER_TRACE_SPI_FI_END       RSI_EGPIO_SetPin(EGPIO, GPIO_SPI_FI_DEBUG_PORT, GPIO_SPI_FI_DEBUG_PIN, 0u)

#define LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_START  (void)0
#define LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_END    (void)0
#define LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_START (void)0
#define LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_END   (void)0
#define LOGIC_ANALYZER_TRACE_TX_DMA_ARMED               (void)0
#define LOGIC_ANALYZER_TRACE_TX_FLUSHED                 (void)0

#else

#define LOGIC_ANALYZER_TRACE_SPI_DMA_CB_START (void)0
#define LOGIC_ANALYZER_TRACE_SPI_DMA_CB_END   (void)0
#define LOGIC_ANALYZER_TRACE_SPI_FI_START     (void)0
#define LOGIC_ANALYZER_TRACE_SPI_FI_END       (void)0
#define LOGIC_ANALYZER_TRACE_PIN_INIT         (void)0

#define LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_START  (void)0
#define LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_END    (void)0
#define LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_START (void)0
#define LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_END   (void)0
#define LOGIC_ANALYZER_TRACE_TX_DMA_ARMED               (void)0
#define LOGIC_ANALYZER_TRACE_TX_FLUSHED                 (void)0

#endif

/*******************************************************************************
 ***************************  SPI CONFIG VARIABLES   ***************************
 ******************************************************************************/
#define SLI_CPC_DRV_SPI_TYPE ARM_SPI_MODE_SLAVE
#define M4_GPIO_PIN
#define SPI_IRQ_PAD_SEL 3
#define SPI_IRQ_PORT    0 // Port selection
#define SPI_IRQ_PIN     8 // Pin number to connect

#define SPI_CS_GPIO_PAD_SEL 6
#define SPI_CS_GPIO_PORT    0  // Port selection
#define SPI_CS_GPIO_PIN     11 // Pin number to connect
/* Pin  interrupt number(0 to 7)               */
#define SPI_CS_GPIO_PIN_RINT_CH 5
/* Pin  interrupt number(0 to 7)               */
#define SPI_CS_GPIO_PIN_FINT_CH 4
/* GPIO interrupt NVIC interrupt name          */
#define SPI_CS_GPIO_PIN_RINT_NVIC_NAME EGPIO_PIN_5_IRQn
/* GPIO interrupt NVIC interrupt name          */
#define SPI_CS_GPIO_PIN_FINT_NVIC_NAME EGPIO_PIN_4_IRQn
/* GPIO interrupt IRQ function name            */
#define SPI_CS_GPIO_PIN_RINT_IRQ_HANDLER IRQ057_Handler
/* GPIO interrupt IRQ function name            */
#define SPI_CS_GPIO_PIN_FINT_IRQ_HANDLER IRQ056_Handler

/*******************************************************************************
 ***************************  SYSTEM CLOCK CONFIG VARIABLES   ******************
 ******************************************************************************/
#define INTF_PLL_CLK            180000000
#define INTF_PLL_REF_CLK        40000000
#define SOC_PLL_CLK             20000000
#define SOC_PLL_REF_CLK         40000000
#define INTF_PLL_500_CTRL_VALUE 0xD900
#define SOC_PLL_MM_COUNT_LIMIT  0xA4

/* SPI Driver */
extern ARM_DRIVER_SPI Driver_SSI_SLAVE;
static ARM_DRIVER_SPI *SPIdrv_slave = &Driver_SSI_SLAVE;

// List of "sli_buf_entry_t" which have an EMPTY "sl_cpc_buffer_handle_t" attached to them.
// Those are entries that are available for receiving a frame. When arming a reception,
// the driver picks the first one and configures the DMA to write the data into its
// attached buffer.
static sl_slist_node_t *rx_free_list_head;

// List of "sli_buf_entry_t" which DO NOT have a "sl_cpc_buffer_handle_t" attached to them.
// Those are entries that were previously received frames that the core read. When the core
// gets notified of a RX frame event, it reads entries from the "rx_pending_list_head" below,
// detaches and take ownership of the "sl_cpc_buffer_handle_t" buffer to process it and
// give back to the driver the dangling entry and put it in this list. Those entries
// are waiting for a "sl_cpc_buffer_handle_t" to be available and re-attached to them
// and be put back into the "rx_free_list_head" list.
static sl_slist_node_t *rx_free_no_buf_list_head;

// List of "sli_buf_entry_t" which have a "sl_cpc_buffer_handle_t" attached to them.
// Those are entries that have fully received frame written in the attached
// "sl_cpc_buffer_handle_t". They are waiting for the core to pick them up,
// detach the filled buffer and being put in the "rx_free_no_buf_list_head" list.
static sl_slist_node_t *rx_pending_list_head;

// List of "sli_buf_entry_t" which DO NOT have a "sl_cpc_buffer_handle_t" attached to them.
// Those are available entries waiting for the core to attach a filled up "sl_cpc_buffer_handle_t"
// to them. Then, they are placed in the "tx_submitted_list_head" .
static sl_slist_node_t *tx_free_list_head;

// List of "sli_buf_entry_t" which have a FILLED "sl_cpc_buffer_handle_t" attached to them.
// Those are entries that are waiting for its buffer to be sent on the wire.
static sl_slist_node_t *tx_submitted_list_head;

static sli_buf_entry_t rx_buf_entries_tbl[SL_CPC_DRV_SPI_RX_QUEUE_SIZE];
static sli_buf_entry_t tx_buf_entries_tbl[SL_CPC_DRV_SPI_TX_QUEUE_SIZE];

// When not null, points to the first entry in "rx_free_list_head" when the driver
// Set itself up to receive a new frame. During the transaction, it is in this entry's
// buffer that the DMA writes the data.
static sli_buf_entry_t *currently_receiving_rx_entry = NULL;

// When not null,
static sli_buf_entry_t *currently_transmiting_tx_entry = NULL;

static bool pending_late_header                            = false;
static IRQ_STATE_T irq_state                               = IRQ_IDLE;
static volatile uint32_t tx_frame_complete                 = 0;
static uint8_t tx_payload_data[DMA_MAX_XFER_LEN]           = { 0 };
static uint8_t header_buffer[SLI_CPC_HDLC_HEADER_RAW_SIZE] = { 0 };
static volatile bool is_header_tx_initiated                = FALSE;
static volatile bool is_header_rx_initiated                = FALSE;
static volatile bool rx_started                            = FALSE;
static volatile uint16_t tx_payload_len                    = 0;
static CPC_STATE_T cpc_tx_rx_state                         = SPI_CPC_IDLE;
#if defined(DEBUG_LOG)
static debug_status_t debug_buffer[tx_rx_idx_max] = { 0 };
#endif
// IRQ flags used for Tx
static uint8_t irq_header_tx_initiated  = 1;
static uint8_t irq_payload_tx_initiated = 0;
static uint8_t irq_header_tx_done       = 0;
static uint8_t irq_payload_tx_done      = 0;
// IRQ flags used for Rx
static uint8_t irq_header_rx_initiated    = 0;
static uint8_t irq_payload_rx_initiated   = 0;
static uint8_t irq_header_rx_done         = 0;
static uint8_t irq_payload_rx_done        = 0;
static volatile bool is_tx_flush_required = FALSE, no_tx_payload = FALSE, no_rx_payload = FALSE;
static enum header_situation received_header_situation;
/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/
static void flush_rx(void);
static void flush_tx(void);
static void end_of_header_xfer(void);
static void end_of_payload_xfer(void);
static void prime_dma_for_transmission(void);
static void Init_Interface_PLL(void);
static void prime_for_rx_header(uint8_t gpio_state_val);

/**
 * @fn       void SPI_CS_GPIO_PIN_RINT_IRQ_HANDLER(void)
 * @brief    SPI GPIO raising edge interrupt
 * @param    None
 * @return   None
 */
void SPI_CS_GPIO_PIN_RINT_IRQ_HANDLER(void)
{
  uint8_t gintStatus;
  {
    /*get interrupt status*/
    gintStatus = RSI_EGPIO_GetIntStat(EGPIO, SPI_CS_GPIO_PIN_RINT_CH);

    if (gintStatus & EGPIO_PIN_INT_CLR_RISING) {
      /*clear interrupt*/
      RSI_EGPIO_IntClr(EGPIO, SPI_CS_GPIO_PIN_RINT_CH, (EGPIO_PIN_INT_CLR_RISING | INTERRUPT_STATUS_CLR));
    } else {
      RSI_EGPIO_IntMask(EGPIO, SPI_CS_GPIO_PIN_RINT_CH);
    }
  }
}

/**
 * @fn       void SPI_CS_GPIO_PIN_FINT_IRQ_HANDLER(void)
 * @brief    SPI GPIO falling edge interrupt
 * @param    None
 * @return   None
 */
void SPI_CS_GPIO_PIN_FINT_IRQ_HANDLER(void)
{
  uint8_t gintStatus;

  /*get interrupt status*/
  gintStatus = RSI_EGPIO_GetIntStat(EGPIO, SPI_CS_GPIO_PIN_FINT_CH);

  if (gintStatus & EGPIO_PIN_INT_CLR_FALLING) {
    /*clear interrupt*/
    RSI_EGPIO_IntClr(EGPIO, SPI_CS_GPIO_PIN_FINT_CH, (INTERRUPT_STATUS_CLR | EGPIO_PIN_INT_CLR_FALLING));
#if defined(DEBUG_LOG)
    if (debug_buffer[rx_header_idx].no_rx_payload_triggered == TRUE) {
      debug_buffer[rx_header_idx].no_rx_payload_triggered = FALSE;
      debug_buffer[rx_header_idx].no_rx_payload_cleared   = no_rx_payload;
      debug_buffer[rx_header_idx].no_rx_payload_clearedcount++;
    }
#endif
    /* Flag used for detecting Rx start on secondary */
    if (irq_header_rx_initiated) {
      rx_started = TRUE;
    }

    /* 1. Check if payload Rx is ongoing, if so pull the IRQ line high
       * 2. Check if payload Tx is ongoing, if so pull the IRQ line high
       * 3. Check if no payload Tx is required, if so pull the IRQ line high
       * 4. Check if no payload Rx is required, if so pull the IRQ line high*/
    if ((irq_header_rx_done && irq_payload_rx_initiated) || (irq_header_tx_done && irq_payload_tx_initiated)
        || (no_tx_payload == TRUE) || (no_rx_payload == TRUE)) {
      LOGIC_ANALYZER_TRACE_SPI_FI_START;
      irq_header_rx_done = 0;
      irq_header_tx_done = 0;
      no_tx_payload      = FALSE;
      no_rx_payload      = FALSE;
      RSI_EGPIO_SetPin(EGPIO, SPI_IRQ_PORT, SPI_IRQ_PIN, 1u);
      LOGIC_ANALYZER_TRACE_SPI_FI_END;
    }

  } else {
    RSI_EGPIO_IntMask(EGPIO, SPI_CS_GPIO_PIN_FINT_CH);
  }
}

/**
 * @fn       void sli_cpc_spi_drv_callback_slave(uint32_t event)
 * @brief    SPI callback handler
 * @param    [in]event SPI transmit and receive events
 * @return   None
 */
void sli_cpc_spi_drv_callback_slave(uint32_t event)
{
  // Clearing the instance number to evaluate the event
  event &= SSI_INSTANCE_MASK;
  switch (event) {
    case ARM_SPI_EVENT_TRANSFER_COMPLETE: {
      // debug purpose
      LOGIC_ANALYZER_TRACE_SPI_DMA_CB_START;
      switch (irq_state) {
        case IRQ_IDLE:
          break;
        case IRQ_TX_HEADER: {
          /* Secondary transmitted header */
          if (tx_payload_len != 0) {
            // tx_payload_len is available, jump to payload tx
            cpc_tx_rx_state = SPI_CPC_TX_PAYLOAD;
            /* Set the IRQ line high*/
            RSI_EGPIO_SetPin(EGPIO, SPI_IRQ_PORT, SPI_IRQ_PIN, 1u);
          } else {
            // nothing to send in payload, jump to tx flush
            no_tx_payload     = TRUE;
            tx_frame_complete = 1;
            cpc_tx_rx_state   = SPI_CPC_TX_FLUSH;
          }
          irq_header_tx_done = 1;
#if defined(DEBUG_LOG)
          debug_buffer[tx_header_idx].response_count++;
#endif
        } break;
        case IRQ_TX_PAYLOAD: {
          /* Secondary transmitted payload */
          tx_payload_len           = 0;
          irq_payload_tx_initiated = 0;
          irq_payload_tx_done      = 1;
#if defined(DEBUG_LOG)
          debug_buffer[tx_payload_idx].response_count++;
#endif
          tx_frame_complete = 1;
          /* jump to tx flush */
          cpc_tx_rx_state = SPI_CPC_FLUSH;
        } break;
        case IRQ_RX_HEADER: {
          /* Secondary received header */
          irq_header_rx_initiated = 0;
          irq_header_rx_done      = 1;
#if defined(DEBUG_LOG)
          debug_buffer[rx_header_idx].response_count++;
#endif
          /* process received header */
          cpc_tx_rx_state = SPI_CPC_RX_HEADER_PROCESS;
        } break;
        case IRQ_RX_PAYLOAD: {
          /* Secondary received payload */
          irq_payload_rx_initiated = 0;
          irq_payload_rx_done      = 1;
#if defined(DEBUG_LOG)
          debug_buffer[rx_payload_idx].response_count++;
#endif
          /* process received payload */
          cpc_tx_rx_state = SPI_CPC_FLUSH;
        } break;
        default:
          break;
      }
      // reset the irq_state flag
      irq_state = IRQ_IDLE;
      // debug purpose
      LOGIC_ANALYZER_TRACE_SPI_DMA_CB_END;
    } break;
    case ARM_SPI_EVENT_DATA_LOST:
      /*  Occurs in slave mode when data is requested/sent by master
            but send/receive/transfer operation has not been started
            and indicates that data is lost. Occurs also in master mode
            when driver cannot transfer data fast enough. */
      DEBUGOUT("\r\n ARM_SPI_EVENT_DATA_LOST \r\n");
      break;
    case ARM_SPI_EVENT_MODE_FAULT:
      /*  Occurs in master mode when Slave Select is deactivated and
            indicates Master Mode Fault. */
      DEBUGOUT("\r\n ARM_SPI_EVENT_MODE_FAULT \r\n");
      break;

    case ARM_SPI_EVENT_SEND_COMPLETE:
      /* Tx call back, added to only process Rx call back */
      break;
  }
}

/***************************************************************************/ /**
 * Initiate SPI Hardware.
 ******************************************************************************/
sl_status_t sli_cpc_drv_init(void)
{
  // Initialize the lists
  {
    sl_slist_init(&rx_free_list_head);
    sl_slist_init(&rx_free_no_buf_list_head);
    sl_slist_init(&rx_pending_list_head);
    sl_slist_init(&tx_free_list_head);
    sl_slist_init(&tx_submitted_list_head);

    for (uint32_t buf_cnt = 0; buf_cnt < SL_CPC_DRV_SPI_RX_QUEUE_SIZE; buf_cnt++) {
      sli_buf_entry_t *entry = &rx_buf_entries_tbl[buf_cnt];
      if (sli_cpc_get_buffer_handle_for_rx(&entry->handle) != SL_STATUS_OK) {
        EFM_ASSERT(false);
        return SL_STATUS_ALLOCATION_FAILED;
      }
      sli_cpc_push_buffer_handle(&rx_free_list_head, &entry->node, entry->handle);
    }

    for (uint32_t buf_cnt = 0; buf_cnt < SL_CPC_DRV_SPI_TX_QUEUE_SIZE; buf_cnt++) {
      sli_buf_entry_t *entry = &tx_buf_entries_tbl[buf_cnt];
      sl_slist_push(&tx_free_list_head, &entry->node);
    }
  }

  //Initialize the system clock to 180MHz
  Init_Interface_PLL();

  // Set GPIO pin 8 as IRQ line
  {
    // Enable pad selection for gpio pins
    RSI_EGPIO_PadSelectionEnable(SPI_IRQ_PAD_SEL);

    // REN enable
    RSI_EGPIO_PadReceiverEnable(SPI_IRQ_PIN);

    // Set pin 8 in GPIO mode
    RSI_EGPIO_SetPinMux(EGPIO, SPI_IRQ_PORT, SPI_IRQ_PIN, EGPIO_PIN_MUX_MODE0);

    // Set output direction
    RSI_EGPIO_SetDir(EGPIO, SPI_IRQ_PORT, SPI_IRQ_PIN, EGPIO_CONFIG_DIR_OUTPUT);

    // Initialize IRQ pin, initial value is HIGH (no frame to send)
    RSI_EGPIO_SetPin(EGPIO, SPI_IRQ_PORT, SPI_IRQ_PIN, 1u);
  }

  // Set GPIO pin 11, which mimics chip select pin behavior
  {
    // Enable pad selection for gpio pins
    RSI_EGPIO_PadSelectionEnable(SPI_CS_GPIO_PAD_SEL);

    // Set pin 11 in GPIO mode
    RSI_EGPIO_SetPinMux(EGPIO, SPI_CS_GPIO_PORT, SPI_CS_GPIO_PIN, EGPIO_PIN_MUX_MODE0);

    // Set the direction
    RSI_EGPIO_SetDir(EGPIO, SPI_CS_GPIO_PORT, SPI_CS_GPIO_PIN, EGPIO_CONFIG_DIR_INPUT);

    // REN enable
    RSI_EGPIO_PadReceiverEnable(SPI_CS_GPIO_PIN);

    RSI_EGPIO_PinIntSel(EGPIO, (SPI_CS_GPIO_PIN_RINT_CH), SPI_CS_GPIO_PORT, SPI_CS_GPIO_PIN);

    RSI_EGPIO_SetIntRiseEdgeEnable(EGPIO, SPI_CS_GPIO_PIN_RINT_CH);

    RSI_EGPIO_PinIntSel(EGPIO, (SPI_CS_GPIO_PIN_FINT_CH), SPI_CS_GPIO_PORT, SPI_CS_GPIO_PIN);

    RSI_EGPIO_SetIntFallEdgeEnable(EGPIO, SPI_CS_GPIO_PIN_FINT_CH);

    // Unmask the  interrupt
    RSI_EGPIO_IntUnMask(EGPIO, (SPI_CS_GPIO_PIN_RINT_CH));

    // Unmask the  interrupt
    RSI_EGPIO_IntUnMask(EGPIO, (SPI_CS_GPIO_PIN_FINT_CH));

    // NVIC enable rising edge interrupt
    NVIC_EnableIRQ(SPI_CS_GPIO_PIN_RINT_NVIC_NAME);

    // NVIC enable falling edge interrupt
    NVIC_EnableIRQ(SPI_CS_GPIO_PIN_FINT_NVIC_NAME);
  }

  // Initialize 91x SPI driver
  {
    int32_t status = SPIdrv_slave->Initialize(sli_cpc_spi_drv_callback_slave);
    EFM_ASSERT(status == 0);

    //Set the CS pin state to high(Pullup)
    RSI_SPI_Slave_Set_CS_Init_State();

    // Power up the SPI peripheral
    status = SPIdrv_slave->PowerControl(ARM_POWER_FULL);
    EFM_ASSERT(status == 0);

    // Configure the SPI to Slave
    status = SPIdrv_slave->Control(
      ARM_SPI_MODE_SLAVE | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_SS_SLAVE_HW | ARM_SPI_DATA_BITS(SPI_BIT_WIDTH),
      SPI_BAUD);
    EFM_ASSERT(status == 0);
  }

#if defined(DEBUG_LOG)
  debug_buffer[tx_header_idx].init_state                 = SPI_CPC_TX_HEADER;
  debug_buffer[tx_payload_idx].init_state                = SPI_CPC_TX_PAYLOAD;
  debug_buffer[rx_header_idx].init_state                 = SPI_CPC_RX_HEADER;
  debug_buffer[rx_payload_idx].init_state                = SPI_CPC_RX_PAYLOAD;
  debug_buffer[rx_header_idx].no_rx_payload_triggered    = FALSE;
  debug_buffer[rx_header_idx].no_rx_payload_cleared      = FALSE;
  debug_buffer[rx_header_idx].no_rx_payload_clearedcount = 0;
  debug_buffer[rx_header_idx].no_rx_payload_setcount     = 0;
#endif
  LOGIC_ANALYZER_TRACE_PIN_INIT;

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif

  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 * Gets CPC driver capabilities.
 ******************************************************************************/
void sli_cpc_drv_get_capabilities(sli_cpc_drv_capabilities_t *capabilities)
{
  if (capabilities == NULL) {
    return;
  }

  *capabilities = (sli_cpc_drv_capabilities_t){ .use_raw_rx_buffer      = false,
                                                .preprocess_hdlc_header = true,
                                                .uart_flowcontrol       = false };
}

/***************************************************************************/ /**
 * Read bytes from SPI.
 ******************************************************************************/
sl_status_t sli_cpc_drv_read_data(sl_cpc_buffer_handle_t **buffer_handle, uint16_t *payload_rx_len)
{
  sl_status_t status;
  MCU_DECLARE_IRQ_STATE;

  MCU_ENTER_ATOMIC();
  sli_buf_entry_t *entry = (sli_buf_entry_t *)SLI_CPC_POP_BUFFER_HANDLE_LIST(&rx_pending_list_head, sli_buf_entry_t);
  if (entry == NULL) {
    MCU_EXIT_ATOMIC();
    return SL_STATUS_EMPTY;
  }
  MCU_EXIT_ATOMIC();

  *buffer_handle  = entry->handle;
  *payload_rx_len = SLI_CPC_RX_DATA_MAX_LENGTH;

  MCU_ENTER_ATOMIC();
  status = sli_cpc_get_buffer_handle_for_rx(&entry->handle);
  if (status == SL_STATUS_OK) {
    sli_cpc_push_buffer_handle(&rx_free_list_head, &entry->node, entry->handle);
  } else {
    sl_slist_push(&rx_free_no_buf_list_head, &entry->node);
  }
  MCU_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 * Write bytes to SPI.
 ******************************************************************************/
sl_status_t sli_cpc_drv_transmit_data(sl_cpc_buffer_handle_t *buffer_handle, uint16_t payload_tx_len)
{
  sli_buf_entry_t *entry;
  MCU_DECLARE_IRQ_STATE;

  MCU_ENTER_ATOMIC();
  entry = (sli_buf_entry_t *)sl_slist_pop(&tx_free_list_head);
  MCU_EXIT_ATOMIC();

  if (entry == NULL) {
    return SL_STATUS_NOT_READY;
  }

  entry->handle      = buffer_handle;
  entry->payload_len = payload_tx_len;

  MCU_ENTER_ATOMIC();
  sli_cpc_push_back_buffer_handle(&tx_submitted_list_head, &entry->node, entry->handle);

  prime_dma_for_transmission();

  MCU_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 * Checks if driver is ready to transmit.
 ******************************************************************************/
bool sli_cpc_drv_is_transmit_ready(void)
{
  sl_slist_node_t *head;

  MCU_ATOMIC_LOAD(head, tx_free_list_head);

  return (head != NULL);
}

/***************************************************************************/ /**
 * Get currently configured bus bitrate
 ******************************************************************************/
uint32_t sli_cpc_drv_get_bus_bitrate(void)
{
  // Stub function : In the case of the SPI driver, this value is meaningless
  return 0;
}

/***************************************************************************/ /**
 * Get maximum bus bitrate
 ******************************************************************************/
uint32_t sli_cpc_drv_get_bus_max_bitrate(void)
{
  uint32_t max_bitrate;

#if defined(SL_CPC_DRV_SPI_IS_USART)
  // The max speed of the USART in mode secondary is the peripheral clock feeding the USART / 6
  max_bitrate = CMU_ClockFreqGet(cmuClock_USART0)
                / 10; // Use cmuClock_USART0 for any USART as they are all connected to the same PCLK

#elif defined(SL_CPC_DRV_SPI_IS_EUSART)
  // The max bitrate of the EUSART in secondary mode is 10MHz no matter what.
  max_bitrate = 10000000;
#elif defined(SLI_SI91X_MCU_INTERFACE)
  max_bitrate = SPI_BAUD;
#endif

  return max_bitrate;
}

/**************************************************************************/ /**
 * Notification when RX buffer becomes free.
 ******************************************************************************/
void sli_cpc_memory_on_rx_buffer_free(void)
{
  sl_status_t status;
  MCU_DECLARE_IRQ_STATE;

  MCU_ENTER_ATOMIC();
  if (rx_free_no_buf_list_head == NULL) {
    MCU_EXIT_ATOMIC();
    return;
  }

  do {
    sli_buf_entry_t *entry = (sli_buf_entry_t *)rx_free_no_buf_list_head;

    status = sli_cpc_get_buffer_handle_for_rx(&entry->handle);
    if (status == SL_STATUS_OK) {
      (void)sl_slist_pop(&rx_free_no_buf_list_head);

      sli_cpc_push_buffer_handle(&rx_free_list_head, &entry->node, entry->handle);
    }
  } while (status == SL_STATUS_OK && rx_free_no_buf_list_head != NULL);
  MCU_EXIT_ATOMIC();
}

/**
 * @fn       void Device_Reset(void)
 * @brief    resets the Si91x as a whole which includes reseting of NWP core and
 *           M4 core respectively.
 * @param    None
 * @return   None
 */
void Device_Reset(void)
{
  /* NWP core */
  uint32_t status = RSI_SUCCESS;

  //TODO: Handle NWP reset
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\nWireless deinit failed, Error Code : 0x%1X\r\n", status);
  } else {
    /* M4 Reset */
    NVIC_SystemReset();
  }
}

/**
 * @fn       sl_cpc_system_status_t Read_reset_reason(void)
 * @brief    resets the Si91x as a whole which includes reseting of NWP core and
 *           M4 core respectively.
 * @param    None
 * @return   reset reason
 */
sl_cpc_system_status_t Read_reset_reason(void)
{
  //TODO: implement the reset reason read

  sl_cpc_system_status_t reset_reason = STATUS_RESET_UNKNOWN;
  uint32_t wakeup_status              = MCU_FSM->MCU_FSM_WAKEUP_STATUS_REG;

  if (wakeup_status & BIT(16)) {
    reset_reason = STATUS_RESET_POWER_ON;
  } else if (wakeup_status & BIT(17)) {
    reset_reason = STATUS_RESET_SOFTWARE;
  } else if (wakeup_status & BIT(0)) {
    reset_reason = STATUS_RESET_UNKNOWN;
  } else if (wakeup_status & BIT(1)) {
    reset_reason = STATUS_RESET_UNKNOWN;
  } else if (wakeup_status & BIT(2)) {
    reset_reason = STATUS_RESET_UNKNOWN;
  } else if (wakeup_status & BIT(3)) {
    reset_reason = STATUS_RESET_UNKNOWN;
  } else if (wakeup_status & BIT(4)) {
    reset_reason = STATUS_RESET_UNKNOWN;
  } else if (wakeup_status & BIT(5)) {
    reset_reason = STATUS_RESET_UNKNOWN;
  } else if (wakeup_status & BIT(6)) {
    reset_reason = STATUS_RESET_UNKNOWN;
  } else if (wakeup_status & BIT(7)) {
    reset_reason = STATUS_RESET_UNKNOWN;
  } else if (wakeup_status & BIT(8)) {
    reset_reason = STATUS_RESET_UNKNOWN;
  } else if (wakeup_status & BIT(9)) {
    reset_reason = STATUS_RESET_UNKNOWN;
  } else if (wakeup_status & BIT(10)) {
    reset_reason = STATUS_RESET_UNKNOWN;
  } else {
    //nothing
    reset_reason = STATUS_RESET_UNKNOWN;
  }
  return reset_reason;
}

/**
 * @fn       void sl_cpc_process_tx_rx(void)
 * @brief    Handles the secondary tx and rx logic based on cpc_tx_rx_state
 *           M4 core respectively.
 * @param    None
 * @return   None
 */
void sl_cpc_process_tx_rx(void)
{
  int32_t status = 0;
  switch (cpc_tx_rx_state) {
    case SPI_CPC_IDLE:
      break;
    case SPI_CPC_TX_HEADER: {
      if ((no_tx_payload == FALSE) && (tx_submitted_list_head)) {
        prime_dma_for_transmission();
      } else {
        cpc_tx_rx_state = SPI_CPC_TX_HEADER;
      }
    } break;
    case SPI_CPC_TX_FLUSH: {
      flush_tx();
      if (tx_submitted_list_head) {
        cpc_tx_rx_state = SPI_CPC_TX_HEADER;
      } else {
        cpc_tx_rx_state = SPI_CPC_RX_HEADER;
      }
    } break;
    case SPI_CPC_TX_PAYLOAD: {
      // Header is successfully tx'd prepare for payload tx
      is_tx_flush_required = TRUE;
      status               = SPIdrv_slave->Send(tx_payload_data, tx_payload_len);
      EFM_ASSERT(status == 0);
      RSI_EGPIO_SetPin(EGPIO, SPI_IRQ_PORT, SPI_IRQ_PIN, 0u);
#if defined(DEBUG_LOG)
      debug_buffer[tx_payload_idx].runtime_state = cpc_tx_rx_state;
      debug_buffer[tx_payload_idx].calling_count++;
#endif
      irq_payload_tx_initiated = 1;
      irq_state                = IRQ_TX_PAYLOAD;
      cpc_tx_rx_state          = SPI_CPC_IDLE;
    } break;
    case SPI_CPC_RX_HEADER: {
      // Prepare for header rx
      if ((no_tx_payload == FALSE)) {
        prime_for_rx_header(SET_IRQ_HIGH);
      } else {
        cpc_tx_rx_state = SPI_CPC_RX_HEADER;
      }
    } break;
    case SPI_CPC_RX_HEADER_PROCESS: {
      // Process received header data
      end_of_header_xfer();
    } break;
    case SPI_CPC_FLUSH: {
      // Rx is completed inform CPC core
      end_of_payload_xfer();
      // Reset the flag so that if we have something to send we can send
      rx_started = FALSE;

      // Nothing in the tx submitted list, prepare for rx
      if (!tx_submitted_list_head) {
        prime_for_rx_header(SET_IRQ_HIGH);
      }
    } break;

    default:
      break;
  }
}

/***************************************************************************/ /**
 * Reconfigure only what needs to be reconfigured in the RX DMA descriptor chain
 * and start the RX DMA channel
 ******************************************************************************/
static void prime_dma_for_reception(size_t payload_size, enum header_situation last_header_situation)
{
  int32_t status      = 0;
  bool init_header_rx = FALSE;
  if (last_header_situation == HEADER_NULL || last_header_situation == HEADER_CORRUPTED) {
    // We received a header full of 0s or a corrupted header.
    // its not worth retrieving a rx_buffer_entry just yet.
    // Ignore this and prepare for next Rx
    init_header_rx = TRUE;
    goto start_transfer;
  }

  // Since we received a valid header, we are going to need a rx_entry to place it into
  if (currently_receiving_rx_entry == NULL) {
    currently_receiving_rx_entry =
      (sli_buf_entry_t *)SLI_CPC_POP_BUFFER_HANDLE_LIST(&rx_free_list_head, sli_buf_entry_t);
  }

  if (currently_receiving_rx_entry == NULL) {
    // running out of buffer. We will drop this frame.
    // Configure the reception chain to skip the payload.
    // Having currently_receiving_rx_entry == NULL when flushing_rx will drop this frame

    // TODO: Handle the error case
    while (1)
      ;
  }

  // Copy the valid header from the static header buffer into the buffer_handle
  memcpy(currently_receiving_rx_entry->handle->hdlc_header, &header_buffer[0], SLI_CPC_HDLC_HEADER_RAW_SIZE);

  if (payload_size == 0) {
    // No payload to receive, prepare for next header rx
    init_header_rx = TRUE;
    rx_started     = FALSE;
  }

#if (SLI_CPC_RX_DATA_MAX_LENGTH <= DMA_MAX_XFER_LEN) // Non-large-buffer
  {
    // Configure the length of the payload to receive
  }
#else // Large-buffer
  {
#error "This driver only supports 1K data transaction SLI_CPC_RX_DATA_MAX_LENGTH should be set not more than 1024"
  }
#endif

  start_transfer :

      if ((init_header_rx == TRUE))
  {
    // initiate header rx
    memset(header_buffer, 0, SLI_CPC_HDLC_HEADER_RAW_SIZE);
    //Inform cpc core only if the valid header data is received
    if (last_header_situation == HEADER_VALID) {
      flush_rx();
    }

    //Initiate cpc header receive
    prime_for_rx_header(SET_IRQ_LOW);
    no_rx_payload = TRUE;
#if defined(DEBUG_LOG)
    debug_buffer[rx_header_idx].no_rx_payload_triggered = no_rx_payload;
    debug_buffer[rx_header_idx].no_rx_payload_setcount++;
#endif
  }
  else
  {
    // header is received so initiate payload rx
    status = SPIdrv_slave->Receive(currently_receiving_rx_entry->handle->data, payload_size);
    EFM_ASSERT(status == 0);
    // Inform primary that we are ready to receive payload
    RSI_EGPIO_SetPin(EGPIO, SPI_IRQ_PORT, SPI_IRQ_PIN, 0u);
#if defined(DEBUG_LOG)
    debug_buffer[rx_payload_idx].runtime_state = cpc_tx_rx_state;
    debug_buffer[rx_payload_idx].calling_count++;
#endif
    // Set corresponding flags
    irq_payload_rx_initiated = 1;
    irq_state                = IRQ_RX_PAYLOAD;
    cpc_tx_rx_state          = SPI_CPC_IDLE;
  }
}

/***************************************************************************/ /**
 * Reconfigure only what needs to be reconfigured in the TX DMA descriptor chain
 * and start the TX DMA channel.
 *
 * We assume the TX DMA channel is not running and we are in interrupt context
 ******************************************************************************/
static void prime_dma_for_transmission(void)
{

  int32_t status = 0;

  if ((currently_transmiting_tx_entry) || (rx_started == TRUE)) {
    // There is already a frame programmed to be sent. Do nothing now, when the
    // frame currently programmed in the TX DMA chain gets tx_flushed(), the
    // frame for which this function was called will be cocked in.
    return;
  }

  // Flush SPI Tx and Rx fifo buffer
  RSI_SPI_Slave_Disable();
  //init here since if there are back to back Tx initiated, then we will loose
  //previous tx payload length.
  tx_payload_len          = 0;
  irq_header_rx_initiated = 0;
  // Pick the next frame to send
  currently_transmiting_tx_entry = (sli_buf_entry_t *)tx_submitted_list_head;

  // Bug if this function is called but there is nothing in the submitted list
  EFM_ASSERT(currently_transmiting_tx_entry != NULL);

#if defined(CPC_TEST_SPI_DRIVER_CRC_ERROR_INJECTION)
  {
    // De-phase a bit
    static size_t count = CPC_TEST_SPI_DRIVER_CRC_ERROR_INJECTION_FREQUENCY / 4;
    static uint64_t bad_crc_header;

    count++;

    if (count % CPC_TEST_SPI_DRIVER_CRC_ERROR_INJECTION_FREQUENCY == 0) {
      count = 0;

      printf("Invalidated the transmit header CRC\n");

      // Make a copy of the good header we are going to transmit
      // We have to make a copy, because otherwise if we modify the good header itself,
      // we will retransmit a bad header indefinitely.
      bad_crc_header = *(uint64_t *)currently_transmiting_tx_entry->handle->hdlc_header;

      // Invert a byte to corrupt the CRC
      ((uint8_t *)&bad_crc_header)[4] = ~((uint8_t *)&bad_crc_header)[4];

      // set header source address to the bad header
      tx_desc_xfer_header.xfer.srcAddr = (uint32_t)&bad_crc_header;
    } else {
      tx_desc_xfer_header.xfer.srcAddr = (uint32_t)currently_transmiting_tx_entry->handle->hdlc_header;
    }
  }
#else
// set header source address
#endif

  if (currently_transmiting_tx_entry->payload_len == 0) {
    // Keep indent small, we are done so go to the end of this function directly
    goto start_transfer;
  }

#if (SLI_CPC_RX_DATA_MAX_LENGTH <= DMA_MAX_XFER_LEN) //Non-large-buffer
  {
    // Copy the payload
    memcpy(&tx_payload_data[0],
           currently_transmiting_tx_entry->handle->data,
           (currently_transmiting_tx_entry->payload_len));

    tx_payload_len += currently_transmiting_tx_entry->payload_len;
#if (SL_CPC_ENDPOINT_SECURITY_ENABLED == 1)
    {
      if (currently_transmiting_tx_entry->handle->security_tag) {
        tx_desc_xfer_payload.xfer.linkAddr = LINKABS_ADDR(&tx_desc_xfer_tag);
        tx_desc_xfer_tag.xfer.srcAddr      = (uint32_t)currently_transmiting_tx_entry->handle->security_tag;
      } else {
        tx_desc_xfer_payload.xfer.linkAddr = LINKABS_ADDR(&tx_desc_xfer_checksum);
      }
      tx_desc_xfer_checksum.xfer.srcAddr = (uint32_t)currently_transmiting_tx_entry->handle->fcs;
    }
#endif
  }
#else // Large-buffer
#error "This driver only supports 1K data transaction SLI_CPC_RX_DATA_MAX_LENGTH should be set not more than 1024"
#endif

  // Finally load the checksum
  memcpy(&tx_payload_data[currently_transmiting_tx_entry->payload_len],
         currently_transmiting_tx_entry->handle->fcs,
         2u);
  tx_payload_len += 2u;

start_transfer:

  // Prepare for header Tx
  status = SPIdrv_slave->Send(currently_transmiting_tx_entry->handle->hdlc_header, SLI_CPC_HDLC_HEADER_RAW_SIZE);
  EFM_ASSERT(status == 0);
  // Inform primary that we are ready with Tx header data
  RSI_EGPIO_SetPin(EGPIO, SPI_IRQ_PORT, SPI_IRQ_PIN, 0u);
#if defined(DEBUG_LOG)
  debug_buffer[tx_header_idx].runtime_state = cpc_tx_rx_state;
  debug_buffer[tx_header_idx].calling_count++;
#endif
  // Update the flags.
  is_header_tx_initiated  = TRUE;
  irq_header_tx_initiated = 1;
  irq_state               = IRQ_TX_HEADER;
  cpc_tx_rx_state         = SPI_CPC_IDLE;
  LOGIC_ANALYZER_TRACE_TX_DMA_ARMED;
}

/**
 * @fn       static void end_of_header_xfer(void)
 * @brief    Process received header buffer
 * @param    None
 * @return   None
 */
static void end_of_header_xfer(void)
{
  size_t rx_payload_length                        = 0;
  uint8_t null_buff[SLI_CPC_HDLC_HEADER_RAW_SIZE] = { 0 };
  rsi_reg_flags_t flags;
  flags = RSI_CRITICAL_SECTION_ENTRY();
  LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_START;

#if defined(CPC_TEST_SPI_DRIVER_CRC_ERROR_INJECTION)
  {
    static size_t count = 0;

    if (header_buffer != 0) {
      count++;

      if (count % CPC_TEST_SPI_DRIVER_CRC_ERROR_INJECTION_FREQUENCY == 0) {
        count = 0;

        printf("Invalidated the CRC\n");

        // Mess with the CRC by inverting a byte
        ((uint8_t *)&header_buffer)[4] = ~((uint8_t *)&header_buffer)[4];
      }
    }
  }
#endif

  // Check for rx header validity
  if (memcmp(header_buffer, null_buff, SLI_CPC_HDLC_HEADER_RAW_SIZE) == 0) {
    received_header_situation = HEADER_NULL;
  } else {
    bool valid_header = sli_cpc_validate_crc_sw((void *)&header_buffer[0],
                                                SLI_CPC_HDLC_HEADER_SIZE,
                                                sli_cpc_hdlc_get_hcs((void *)&header_buffer[0]));
    if (valid_header) {
      received_header_situation = HEADER_VALID;
      rx_payload_length         = sli_cpc_hdlc_get_length((void *)&header_buffer[0]);
    } else {
      received_header_situation = HEADER_CORRUPTED;
    }
  }

  // Prepare for either header/payload rx
  prime_dma_for_reception(rx_payload_length, received_header_situation);

  LOGIC_ANALYZER_TRACE_HEADER_TRANSFER_ISR_END;

  RSI_CRITICAL_SECTION_EXIT(flags);
}

/**
 * @fn       static void end_of_payload_xfer(void)
 * @brief    Notifies the core when a frame has been sent on the wire.
 * @param    None
 * @return   None
 */
static void end_of_payload_xfer(void)
{
  rsi_reg_flags_t flags;
  flags = RSI_CRITICAL_SECTION_ENTRY();

  LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_START;

  // Check if we had a late header transmission situation
  if (currently_transmiting_tx_entry) {
    // At this point, a header have been exchanged and we realize a TX entry is registered for transmission
    // We need to know is this "currently_transmiting_tx_entry" had the chance to have its header clocked in the header
    // exchange that just happened
    if (tx_frame_complete == 0) {
      // This "currently_transmiting_tx_entry" has arrived lated. Yes it is at this point the current active TX frame, but
      // at the moment the primary started clocking the header, its header did't go through. Mark this current
      // TX entry transmission as a "pending_late_header" so that we don't try to flush this "currently_transmiting_tx_entry"
      // as having been sent on the wire.
      pending_late_header = true;
    }
  }

  // Reset that variable which is set to 1 by the TX DMA chain
  tx_frame_complete = 0;

  flush_rx();
  flush_tx();

  // Check if we have something to send
  if (tx_submitted_list_head) {
    if (pending_late_header) {
      pending_late_header = false;
    } else {
      // Prepare for tx
      prime_dma_for_transmission();
    }
  }

  LOGIC_ANALYZER_TRACE_PAYLOAD_TRANSFER_ISR_END;

  RSI_CRITICAL_SECTION_EXIT(flags);
}

/**
 * @fn       static void flush_rx(void)
 * @brief    Notifies the core when a frame has been sent on the wire.
 * @param    None
 * @return   None
 */
static void flush_rx(void)
{
  if (currently_receiving_rx_entry && received_header_situation == HEADER_VALID) {
#if defined(CPC_TEST_SPI_DRIVER_CRC_ERROR_INJECTION)
    {
      // Start the payload CRC error injection out of phase with the header error
      static size_t count = CPC_TEST_SPI_DRIVER_CRC_ERROR_INJECTION_FREQUENCY / 2;

      if (sli_cpc_hdlc_get_length((void *)&header_buffer) != 0) {
        count++;

        if (count % CPC_TEST_SPI_DRIVER_CRC_ERROR_INJECTION_FREQUENCY == 0) {
          count = 0;

          printf("Invalidated the payload CRC\n");

          //Mess with the first byte of the payload CRC by inverting it
          ((uint8_t *)currently_receiving_rx_entry->handle->data)[0] =
            ~((uint8_t *)currently_receiving_rx_entry->handle->data)[0];
        }
      }
    }
#endif

    sli_cpc_push_back_buffer_handle(&rx_pending_list_head,
                                    &currently_receiving_rx_entry->node,
                                    currently_receiving_rx_entry->handle);

    currently_receiving_rx_entry = NULL;

    sli_cpc_drv_notify_rx_data();
  }
}

/**
 * @fn       static void flush_tx(void)
 * @brief    Notifies the core when a frame has been sent on the wire.
 * @param    None
 * @return   None
 */
static void flush_tx(void)
{
  if (currently_transmiting_tx_entry == NULL) {
    // Nothing to notify the core about if there is no current TX entry
    return;
  }

  if (pending_late_header) {
    // There is a TX entry, but we realized the header couldn't be sent in time
    return;
  }

  // Remove the first entry from the TX submitted list.
  sli_buf_entry_t *entry = (sli_buf_entry_t *)SLI_CPC_POP_BUFFER_HANDLE_LIST(&tx_submitted_list_head, sli_buf_entry_t);

  // Paranoia. The first entry in the TX submitted list NEEDS to be the currently_transmiting_tx_entry
  EFM_ASSERT(entry == currently_transmiting_tx_entry);

  // Notify the core that this entry has been sent on the wire. It will detach its buffer
  sli_cpc_drv_notify_tx_complete(entry->handle);

  sl_slist_push(&tx_free_list_head, &entry->node);

  // Important to set this back to NULL
  currently_transmiting_tx_entry = NULL;

  LOGIC_ANALYZER_TRACE_TX_FLUSHED;
}

/**
 * @fn       void prime_for_rx_header(uint8_t gpio_state_val)
 * @brief    Prepares for receive header
 * @param    gpio_state_val - state of the GPIO pin (high/low).
 * @return   None
 */
static void prime_for_rx_header(uint8_t gpio_state_val)
{
  int32_t status = 0;
  status         = SPIdrv_slave->Receive(header_buffer, SLI_CPC_HDLC_HEADER_RAW_SIZE);
  EFM_ASSERT(status == 0);
  //TODO: - added to set IRQ high when the rx payload length is 0 in previous transaction
  RSI_EGPIO_SetPin(EGPIO, SPI_IRQ_PORT, SPI_IRQ_PIN, gpio_state_val);
  is_header_rx_initiated  = TRUE;
  irq_header_rx_initiated = 1;
  irq_state               = IRQ_RX_HEADER;
#if defined(DEBUG_LOG)
  debug_buffer[rx_header_idx].runtime_state = cpc_tx_rx_state;
  debug_buffer[rx_header_idx].calling_count++;
#endif
  cpc_tx_rx_state = SPI_CPC_IDLE;
}

/**
 * @fn       void Init_Interface_PLL(void)
 * @brief    Configure system clock to 180MHz
 * @param    None
 * @return   None
 */
static void Init_Interface_PLL(void)
{

  uint32_t status = 0;

  // program intf pll to 180Mhz
  SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG9) = INTF_PLL_500_CTRL_VALUE;
  status                                  = RSI_CLK_SetIntfPllFreq(M4CLK, INTF_PLL_CLK, INTF_PLL_REF_CLK);
  EFM_ASSERT(status == RSI_OK);

  // Configure m4 soc to 180Mhz
  status = RSI_CLK_M4SocClkConfig(M4CLK, M4_INTFPLLCLK, 0);
  EFM_ASSERT(status == RSI_OK);

  // Configure socpll to 20Mhz
  RSI_CLK_SocPllLockConfig(1, 1, SOC_PLL_MM_COUNT_LIMIT);
  status = RSI_CLK_SetSocPllFreq(M4CLK, SOC_PLL_CLK, SOC_PLL_REF_CLK);
  EFM_ASSERT(status == RSI_OK);

  status = ROMAPI_M4SS_CLK_API->clk_qspi_clk_config(M4CLK, QSPI_INTFPLLCLK, 0, 0, 0);
  EFM_ASSERT(status == RSI_OK);
  // Enable clock for EGPIO module
  RSI_CLK_PeripheralClkEnable(M4CLK, EGPIO_CLK, ENABLE_STATIC_CLK);
}
