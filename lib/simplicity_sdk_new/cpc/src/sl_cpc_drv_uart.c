/***************************************************************************/ /**
 * @file
 * @brief CPC UART Driver implementation.
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

#include "ecode.h"
#include "dmadrv.h"
#include "sl_atomic.h"
#include "sl_core.h"
#include "sl_cpc_config.h"
#include "sl_cpc_drv_uart_config.h"
#include "sl_cpc_instance_handles.h"
#include "sl_clock_manager.h"
#include "sl_device_peripheral.h"
#include "sl_sleeptimer.h"
#include "sl_slist.h"
#include "sl_status.h"

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif
#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#if defined(SLI_CPC_DRV_UART_RECOVERY_TRACE_ENABLED)
#define SL_CPC_RECOVERY_TRACE(msg, line) SL_CPC_JOURNAL_RECORD_DEBUG(msg, line)
#else
#define SL_CPC_RECOVERY_TRACE(msg, line) do {} while (0)
#endif

#if defined(_SILICON_LABS_32B_SERIES_2)
#include "em_gpio.h"
#if defined(SL_CPC_DRV_PERIPH_IS_EUSART)
#include "em_eusart.h"
#elif defined(SL_CPC_DRV_PERIPH_IS_USART)
#include "em_usart.h"
#else
#error Unsupported UART peripheral
#endif
#if !defined(SL_CATALOG_CLOCK_MANAGER_PRESENT)
#include "em_cmu.h"
#endif
#else // Series 3 and up
#include "sl_gpio.h"
#include "sl_hal_eusart.h"
#include "sl_hal_gpio.h"
#endif // Series 2-3 compatibility

#include "sli_cpc.h"
#include "sli_cpc_assert.h"
#include "sli_cpc_crc.h"
#include "sli_cpc_debug.h"
#include "sli_cpc_drv.h"
#include "sli_cpc_hdlc.h"
#include "sli_cpc_instance.h"
#include "sli_cpc_memory.h"
#include "sli_cpc_system_common.h"

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
#include "sl_cpc_primary_config.h"
#endif

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

// Generic (E)USART defines
#define CPC_UART_ISR_TX_HANDLER(periph_nbr) SL_CONCAT_PASTER_3(SL_CPC_DRV_PERIPH_NAME, periph_nbr, _TX_IRQHandler)
#define CPC_UART_TX_IRQn(periph_nbr)        SL_CONCAT_PASTER_3(SL_CPC_DRV_PERIPH_NAME, periph_nbr, _TX_IRQn)
#define CPC_UART_CLOCK                      SL_CONCAT_PASTER_3(SL_BUS_CLOCK_, SL_CPC_DRV_PERIPH_NAME, SL_CPC_DRV_UART_PERIPHERAL_NO)

// EUSART defines
#if defined(SL_CPC_DRV_PERIPH_IS_EUSART)
#define WITH_HWFC              eusartHwFlowControlCtsAndRts_D
#define WITHOUT_HWFC           eusartHwFlowControlNone_D
#define CPC_UART_IF_TXC        SL_CONCAT_PASTER_2(EUSART, _IF_TXC)
#define CPC_UART_CLEAR_CMD()   SL_CPC_DRV_UART_PERIPHERAL->CMD_SET = EUSART_CMD_CLEARTX
#else
// USART defines
#define WITH_HWFC              usartHwFlowControlCtsAndRts_D
#define WITHOUT_HWFC           usartHwFlowControlNone_D
#define CPC_UART_IF_TXC        SL_CONCAT_PASTER_2(USART, _IF_TXC)
#define CPC_UART_CLEAR_CMD()   SL_CPC_DRV_UART_PERIPHERAL->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX
#endif

// Generic LDMA defines
#define RX_DMA_DESCRIPTOR_QTY (2)

// Configuration validation
#if ((SLI_CPC_DRV_UART_RX_DATA_MAX_LENGTH > 2048) \
  && (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITHOUT_HWFC))
#error  Buffer larger than 2048 bytes without Hardware Flow Control is not supported
#endif

#if (SL_CPC_DRV_UART_RX_QUEUE_SIZE > SL_CPC_DRV_UART_RX_BUFFER_MAX_COUNT)
#error  Invalid configuration SL_CPC_RX_BUFFER_MAX_COUNT must be at least SL_CPC_DRV_UART_RX_QUEUE_SIZE
#endif

#if ((SL_CPC_DRV_UART_RX_BUFFER_MAX_COUNT <= 2) \
  && (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITHOUT_HWFC))
#error  Invalid configuration SL_CPC_RX_BUFFER_MAX_COUNT must be greater than 2 when no hardware flow control is provided
#endif

// Series 2/3 compatibility layer
#if defined(_SILICON_LABS_32B_SERIES_2)
// Series 2 uses EMLib
// UART
#if defined(SL_CPC_DRV_PERIPH_IS_EUSART)
// EUSART
#define cpc_uart_enable(peripheral)              EUSART_Enable(peripheral, eusartEnable)
#define cpc_uart_disable(peripheral)             EUSART_Enable(peripheral, eusartDisable)
#define cpc_uart_int_clear                       SL_CONCAT_PASTER_2(EUSART, _IntClear)
#define cpc_uart_int_enable                      SL_CONCAT_PASTER_2(EUSART, _IntEnable)
#define cpc_uart_int_disable                     SL_CONCAT_PASTER_2(EUSART, _IntDisable)
#define cpc_uart_int_get                         SL_CONCAT_PASTER_2(EUSART, _IntGet)
#define CPC_LDMA_RX_PERIPH_TRIGGER(periph_nbr)   SL_CONCAT_PASTER_3(SL_CONCAT_PASTER_2(ldmaPeripheralSignal_, SL_CPC_DRV_PERIPH_NAME), periph_nbr, _RXFL)
#define CPC_LDMA_TX_PERIPH_TRIGGER(periph_nbr)   SL_CONCAT_PASTER_3(SL_CONCAT_PASTER_2(ldmaPeripheralSignal_, SL_CPC_DRV_PERIPH_NAME), periph_nbr, _TXFL)
#else
// USART
#define cpc_uart_enable(peripheral)              USART_Enable(peripheral, usartEnable)
#define cpc_uart_disable(peripheral)             USART_Enable(peripheral, usartDisable)
#define cpc_uart_int_clear                       SL_CONCAT_PASTER_2(USART, _IntClear)
#define cpc_uart_int_enable                      SL_CONCAT_PASTER_2(USART, _IntEnable)
#define cpc_uart_int_disable                     SL_CONCAT_PASTER_2(USART, _IntDisable)
#define cpc_uart_int_get                         SL_CONCAT_PASTER_2(USART, _IntGet)
#define CPC_LDMA_RX_PERIPH_TRIGGER(periph_nbr)   SL_CONCAT_PASTER_3(SL_CONCAT_PASTER_2(ldmaPeripheralSignal_, SL_CPC_DRV_PERIPH_NAME), periph_nbr, _RXDATAV)
#define CPC_LDMA_TX_PERIPH_TRIGGER(periph_nbr)   SL_CONCAT_PASTER_3(SL_CONCAT_PASTER_2(ldmaPeripheralSignal_, SL_CPC_DRV_PERIPH_NAME), periph_nbr, _TXBL)
#endif

// LDMA
#define LDMA_PERIPH                                                    LDMA
typedef LDMA_Descriptor_t                                              cpc_ldma_descriptor_t;
#define CPC_LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(src, dest, cnt, link_jmp) LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(src, dest, cnt, link_jmp)
#define CPC_LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(src, dest, cnt, link_jmp) LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(src, dest, cnt, link_jmp)
#define CPC_LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(src, dest, cnt)            LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(src, dest, cnt)
#define CPC_LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(src, dest, cnt)            LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(src, dest, cnt)
#define CPC_LDMA_DESCRIPTOR_MAX_XFER_SIZE                              LDMA_DESCRIPTOR_MAX_XFER_SIZE
#define CPC_LDMA_DESCRIPTOR_LINKABS_LINKADDR_TO_ADDR                   LDMA_DESCRIPTOR_LINKABS_LINKADDR_TO_ADDR
#define CPC_LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR                   LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR
#define CPC_LDMA_DESCRIPTOR_XFER_CNT                                   xferCnt
#define CPC_LDMA_DESCRIPTOR_DONE_IFS                                   doneIfs
#define CPC_LDMA_DESCRIPTOR_DST_ADDR                                   dstAddr
#define CPC_LDMA_DESCRIPTOR_LINK_ADDR                                  linkAddr
#define CPC_LDMA_DESCRIPTOR_LINK_MODE                                  linkMode
#define CPC_LDMA_DESCRIPTOR_LINK_MODE_ABS                              ldmaLinkModeAbs
#define cpc_ldma_int_clear                                             LDMA_IntClear
#define cpc_ldma_int_enable                                            LDMA_IntEnable

// GPIO
#define cpc_gpio_set_pin_mode      GPIO_PinModeSet
#define CPC_GPIO_MODE_PUSH_PULL    gpioModePushPull
#define CPC_GPIO_MODE_INPUT_PULL   gpioModeInputPull

#else
// Series 3 uses sl_hal
// LDMA
#define LDMA_PERIPH                                                    LDMA(0)
#define CPC_LDMA_RX_PERIPH_TRIGGER(periph_nbr)                         SL_CONCAT_PASTER_3(dmadrvPeripheralSignal_EUSART, periph_nbr, _RXDATAV)
#define CPC_LDMA_TX_PERIPH_TRIGGER(periph_nbr)                         SL_CONCAT_PASTER_3(dmadrvPeripheralSignal_EUSART, periph_nbr, _TXBL)
typedef sl_hal_ldma_descriptor_t                                       cpc_ldma_descriptor_t;
#define CPC_LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(src, dest, cnt, link_jmp) SL_HAL_LDMA_DESCRIPTOR_LINKREL_M2P(SL_HAL_LDMA_CTRL_SIZE_BYTE, src, dest, cnt, link_jmp)
#define CPC_LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(src, dest, cnt, link_jmp) SL_HAL_LDMA_DESCRIPTOR_LINKREL_P2M(SL_HAL_LDMA_CTRL_SIZE_BYTE, src, dest, cnt, link_jmp)
#define CPC_LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(src, dest, cnt)            SL_HAL_LDMA_DESCRIPTOR_SINGLE_M2P(SL_HAL_LDMA_CTRL_SIZE_BYTE, src, dest, cnt)
#define CPC_LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(src, dest, cnt)            SL_HAL_LDMA_DESCRIPTOR_SINGLE_P2M(SL_HAL_LDMA_CTRL_SIZE_BYTE, src, dest, cnt)
#define CPC_LDMA_DESCRIPTOR_MAX_XFER_SIZE                              SL_HAL_LDMA_DESCRIPTOR_MAX_XFER_SIZE
#define CPC_LDMA_DESCRIPTOR_LINKABS_LINKADDR_TO_ADDR                   SL_HAL_LDMA_DESCRIPTOR_LINKABS_LINKADDR_TO_ADDR
#define CPC_LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR                   SL_HAL_LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR
#define CPC_LDMA_DESCRIPTOR_XFER_CNT                                   xfer_count
#define CPC_LDMA_DESCRIPTOR_DONE_IFS                                   done_ifs
#define CPC_LDMA_DESCRIPTOR_DST_ADDR                                   dst_addr
#define CPC_LDMA_DESCRIPTOR_LINK_ADDR                                  link_addr
#define CPC_LDMA_DESCRIPTOR_LINK_MODE                                  link_mode
#define CPC_LDMA_DESCRIPTOR_LINK_MODE_ABS                              SL_HAL_LDMA_LINK_MODE_ABS
#define cpc_ldma_int_clear(flags)                                      sl_hal_ldma_clear_interrupts(LDMA_PERIPH, flags)
#define cpc_ldma_int_enable(flags)                                     sl_hal_ldma_enable_interrupts(LDMA_PERIPH, flags)

// GPIO
static inline void cpc_gpio_set_pin_mode(uint8_t port, uint8_t pin, uint8_t mode, bool val)
{
  sl_gpio_t gpio = {
    .port = port,
    .pin = pin
  };
  sl_gpio_set_pin_mode(&gpio, mode, val);
}
#define CPC_GPIO_MODE_PUSH_PULL            SL_GPIO_MODE_PUSH_PULL
#define CPC_GPIO_MODE_INPUT_PULL           SL_GPIO_MODE_INPUT_PULL

// UART (Series 3 only supports EUSART)
#define CPC_UART_PERIPHERAL(periph_no)     SL_CONCAT_PASTER_2(SL_PERIPHERAL_EUSART, periph_no)
#define cpc_uart_int_clear                 sl_hal_eusart_clear_interrupts
#define cpc_uart_int_enable                sl_hal_eusart_enable_interrupts
#define cpc_uart_int_disable               sl_hal_eusart_disable_interrupts
#define cpc_uart_int_get                   sl_hal_eusart_get_pending_interrupts
#define cpc_uart_enable(peripheral)      \
  {                                      \
    sl_hal_eusart_enable(peripheral);    \
    sl_hal_eusart_enable_tx(peripheral); \
    sl_hal_eusart_enable_rx(peripheral); \
  }
#define cpc_uart_disable(peripheral)     \
  {                                      \
    sl_hal_eusart_disable_rx(peripheral) \
    sl_hal_eusart_disable_tx(peripheral) \
    sl_hal_eusart_disable(peripheral);   \
  }
#endif // Series 2/3 compatibility layer

#define SL_CPC_DRV_UART_RX_MAX_BUFFER_SIZE  (SL_MIN(SLI_CPC_DRV_UART_RX_FRAME_MAX_LENGTH, CPC_LDMA_DESCRIPTOR_MAX_XFER_SIZE))

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

static sli_cpc_instance_t * driver_instance;

// read/write channel and tx/rx config are non-static because the xmodem driver reuses those values
unsigned int read_channel;
unsigned int write_channel;
// Series LDMA variants
#if defined(_SILICON_LABS_32B_SERIES_2)
LDMA_TransferCfg_t rx_config;
LDMA_TransferCfg_t tx_config;
static cpc_ldma_descriptor_t rx_descriptor[RX_DMA_DESCRIPTOR_QTY];
static cpc_ldma_descriptor_t *rx_descriptor_head = NULL;

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
/* An extra DMA descriptor is needed for the security tag */
static cpc_ldma_descriptor_t tx_descriptor[5u];
#else
static cpc_ldma_descriptor_t tx_descriptor[4u];
#endif

#else // Series 3
sl_hal_ldma_transfer_config_t rx_config;
sl_hal_ldma_transfer_config_t tx_config;
static sl_hal_ldma_descriptor_t rx_descriptor[RX_DMA_DESCRIPTOR_QTY];
static sl_hal_ldma_descriptor_t *rx_descriptor_head = NULL;

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
/* An extra DMA descriptor is needed for the security tag */
static sl_hal_ldma_descriptor_t tx_descriptor[5u];
#else
static sl_hal_ldma_descriptor_t tx_descriptor[4u];
#endif
#endif // Series LDMA variants

static sl_slist_node_t *rx_free_buffer_handle_list_head;
static sl_slist_node_t *rx_pending_list_head;

static sl_slist_node_t *tx_submitted_list_head;
static sl_slist_node_t *tx_complete_list_head;

static uint16_t free_rx_buffer_handles = 0;
static uint16_t free_tx_items = 0;
static uint16_t next_rx_size = 0;
static bool header_expected_next = true;
static bool need_rx_buffer_handle = false;
static bool tx_ready = true;

#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITH_HWFC)
static bool need_rx_buffer = false;
#endif

#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITHOUT_HWFC)
static struct {
  uint8_t *spilled_buffer;
  void *active_buffer;
  bool out_of_sync;
  uint16_t offset;
  sl_cpc_buffer_handle_t *buffer_handle;
  uint16_t recovered_payloads;
  bool misaligned_payload;
  bool recovery_completed;
  uint8_t invalid_hdlc_header[SLI_CPC_HDLC_HEADER_RAW_SIZE];
  uint8_t hdlc_header_out_of_sync_sliding_window[SLI_CPC_HDLC_HEADER_RAW_SIZE * 2];
  uint8_t out_of_sync_extra_bytes[SLI_CPC_HDLC_HEADER_RAW_SIZE];
  uint8_t out_of_sync_extra_bytes_len;
} recovery_context;

static sl_cpc_dispatcher_handle_t recovery_dispatcher_handle;
#endif

static sl_cpc_dispatcher_handle_t dispatcher_handle;

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
static bool deinited_for_firmware_upgrade = false;
#endif

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/
static void init_clocks(void);
static sl_status_t prepare_next_tx(void);

#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITHOUT_HWFC)
static sl_status_t resize_current_dma_descriptor(uint16_t new_length);

static bool rx_dma_complete_no_hwfc(unsigned int channel,
                                    unsigned int sequenceNo,
                                    void *userParam);

static void recovery(void *data);

static void swap_descriptors(void);

static void update_dma_desc_link_abs(cpc_ldma_descriptor_t *current_dma_desc,
                                     cpc_ldma_descriptor_t *next_dma_desc,
                                     bool enable_link);

#else
static void resize_current_dma_descriptor(void *pre_loaded_rx_buffer, uint16_t offset, uint16_t new_length);

static bool rx_dma_complete_hwfc(unsigned int channel,
                                 unsigned int sequenceNo,
                                 void *userParam);
#endif

static void notify_core_error(sl_cpc_buffer_handle_t *buffer_handle, sl_cpc_reject_reason_t reason);

static void restart_dma(void);

static void push_free_rx_buffer_handle(sl_cpc_buffer_handle_t* buffer_handle);
static sl_cpc_buffer_handle_t* pop_free_rx_buffer_handle(void);

static sl_status_t uart_drv_hw_init(sli_cpc_drv_t *drv);
static sl_status_t uart_drv_init(sli_cpc_drv_t *drv, sli_cpc_instance_t *inst);
static sl_status_t uart_drv_get_capabilities(sli_cpc_drv_t *drv,
                                             sli_cpc_drv_capabilities_t *capabilities);
static sl_status_t uart_drv_start_rx(sli_cpc_drv_t *drv);
#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT) && (SL_CPC_PRIMARY_FIRMWARE_UPGRADE_SUPPORT_ENABLED >= 1)
static void uart_drv_deinit_for_firmware_upgrade(sli_cpc_drv_t *drv);
#endif
static sl_status_t uart_drv_read_data(sli_cpc_drv_t *drv, sl_cpc_buffer_handle_t **buffer_handle);
static sl_status_t uart_drv_transmit_data(sli_cpc_drv_t *drv, sl_cpc_buffer_handle_t *buffer_handle);
static bool uart_drv_is_transmit_ready(sli_cpc_drv_t *drv);
static uint32_t uart_drv_get_bus_bitrate(sli_cpc_drv_t *drv);
static uint32_t uart_drv_get_bus_max_bitrate(sli_cpc_drv_t *drv);
static void uart_drv_on_rx_buffer_handle_free(sli_cpc_drv_t *drv);
static void uart_drv_on_rx_buffer_free(sli_cpc_drv_t *drv);

sli_cpc_drv_t uart_driver = {
  .ops = {
    .hw_init = uart_drv_hw_init,
    .init = uart_drv_init,
    .get_capabilities = uart_drv_get_capabilities,
    .start_rx = uart_drv_start_rx,
#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT) && (SL_CPC_PRIMARY_FIRMWARE_UPGRADE_SUPPORT_ENABLED >= 1)
    .deinit = uart_drv_deinit_for_firmware_upgrade,
#endif
    .read = uart_drv_read_data,
    .write = uart_drv_transmit_data,
    .is_transmit_ready = uart_drv_is_transmit_ready,
    .get_bus_bitrate = uart_drv_get_bus_bitrate,
    .get_bus_max_bitrate = uart_drv_get_bus_max_bitrate,
    .on_rx_buffer_handle_free = uart_drv_on_rx_buffer_handle_free,
    .on_rx_buffer_free = uart_drv_on_rx_buffer_free,
  }
};

/*******************************************************************************
 ***************************   WEAK FUNCTIONS   ********************************
 ******************************************************************************/
void sli_cpc_drv_wake_gpio_init(void);
void sli_cpc_drv_wake_host_gpio(bool active);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
/***************************************************************************/ /**
 * Return a pointer to the UART driver instance.
 ******************************************************************************/
sli_cpc_drv_t* sli_cpc_drv_get_driver(void)
{
  return &uart_driver;
}
#endif

/***************************************************************************/ /**
 * Initialize only the UART peripheral to be used in a standalone manner
 * (during the bootloader poking)
 ******************************************************************************/
static sl_status_t uart_drv_hw_init(sli_cpc_drv_t *drv)
{
  (void)drv;

  // Initialize clocks for driver operation
  init_clocks();

  // Configure GPIO pin modes
  {
    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);

    cpc_gpio_set_pin_mode(SL_CPC_DRV_UART_TX_PORT, SL_CPC_DRV_UART_TX_PIN, CPC_GPIO_MODE_PUSH_PULL, 1);
    cpc_gpio_set_pin_mode(SL_CPC_DRV_UART_RX_PORT, SL_CPC_DRV_UART_RX_PIN, CPC_GPIO_MODE_INPUT_PULL, 1);
  #if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITH_HWFC)
    cpc_gpio_set_pin_mode(SL_CPC_DRV_UART_CTS_PORT, SL_CPC_DRV_UART_CTS_PIN, CPC_GPIO_MODE_INPUT_PULL, 0);
    cpc_gpio_set_pin_mode(SL_CPC_DRV_UART_RTS_PORT, SL_CPC_DRV_UART_RTS_PIN, CPC_GPIO_MODE_PUSH_PULL, 0);
  #endif
  }
  sli_cpc_drv_wake_gpio_init();

  // init the UART peripheral
  {
  #if defined(SL_CPC_DRV_PERIPH_IS_EUSART)
  #if defined(_SILICON_LABS_32B_SERIES_2)
    EUSART_UartInit_TypeDef init = EUSART_UART_INIT_DEFAULT_HF;
    EUSART_AdvancedInit_TypeDef advancedSettings = EUSART_ADVANCED_INIT_DEFAULT;
    init.enable = eusartEnable;
    #if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITHOUT_HWFC)
    advancedSettings.hwFlowControl = eusartHwFlowControlNone;
    #elif (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITH_HWFC)
    advancedSettings.hwFlowControl = eusartHwFlowControlCtsAndRts;
    #endif
    init.advancedSettings = &advancedSettings;
    init.baudrate = SL_CPC_DRV_UART_BAUDRATE;

    EUSART_UartInitHf(SL_CPC_DRV_UART_PERIPHERAL, &init);

    CPC_UART_CLEAR_CMD();
  #elif defined(_SILICON_LABS_32B_SERIES_3)
    sl_hal_eusart_uart_config_t init = SL_HAL_EUSART_UART_INIT_DEFAULT_HF;
    sl_hal_eusart_uart_advanced_config_t advancedSettings = SL_HAL_EUSART_UART_ADVANCED_INIT_DEFAULT;
    #if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITHOUT_HWFC)
    advancedSettings.hw_flow_control_mode = SL_HAL_EUSART_HW_FLOW_CONTROL_NONE;
    #elif (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITH_HWFC)
    advancedSettings.hw_flow_control_mode = SL_HAL_EUSART_HW_FLOW_CONTROL_CTS_RTS;
    #endif

    // Compute clock div for baudrate
    uint32_t freq;
    sl_clock_branch_t clock_branch = sl_device_peripheral_get_clock_branch(CPC_UART_PERIPHERAL(SL_CPC_DRV_UART_PERIPHERAL_NO));
    sl_status_t status = sl_clock_manager_get_clock_branch_frequency(clock_branch, &freq);
    SLI_CPC_ASSERT(status == SL_STATUS_OK);

    init.clock_div = sl_hal_eusart_uart_calculate_clock_div(freq,
                                                            SL_CPC_DRV_UART_BAUDRATE,
                                                            init.oversampling);

    init.advanced_config = &advancedSettings;
    sl_hal_eusart_init_uart_hf(SL_CPC_DRV_UART_PERIPHERAL, &init);

    // Enable peripheral
    sl_hal_eusart_enable(SL_CPC_DRV_UART_PERIPHERAL);
  #endif
  #else // Using USART
    USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
    init.enable = usartDisable;
    #if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITHOUT_HWFC)
    init.hwFlowControl = (USART_HwFlowControl_TypeDef)WITHOUT_HWFC;
    #elif (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITH_HWFC)
    init.hwFlowControl = (USART_HwFlowControl_TypeDef)WITH_HWFC;
    #endif
    init.baudrate = SL_CPC_DRV_UART_BAUDRATE;

    // Initialize an UART driver instance.
    USART_InitAsync(SL_CPC_DRV_UART_PERIPHERAL, &init);

    // Discard false frames and/or IRQs
    SL_CPC_DRV_UART_PERIPHERAL->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
  #endif
  }
  // Configure GPIO pin routes
  {
    #define UART_ROUTE          SL_CONCAT_PASTER_2(SL_CPC_DRV_PERIPH_NAME, ROUTE[SL_CPC_DRV_UART_PERIPHERAL_NO])
    #define GPIO_ROUTEEN_TXPEN  SL_CONCAT_PASTER_3(GPIO_, SL_CPC_DRV_PERIPH_NAME, _ROUTEEN_TXPEN)
    #define GPIO_ROUTEEN_RTSPEN SL_CONCAT_PASTER_3(GPIO_, SL_CPC_DRV_PERIPH_NAME, _ROUTEEN_RTSPEN)
    #define GPIO_PORT_SHIFT(route) SL_CONCAT_PASTER_4(_GPIO_, SL_CONCAT_PASTER_2(SL_CPC_DRV_PERIPH_NAME, _), route, _PORT_SHIFT)
    #define GPIO_PIN_SHIFT(route)  SL_CONCAT_PASTER_4(_GPIO_, SL_CONCAT_PASTER_2(SL_CPC_DRV_PERIPH_NAME, _), route, _PIN_SHIFT)

  #if defined(_USART_ROUTELOC0_MASK)
    SL_CPC_DRV_UART_PERIPHERAL->ROUTELOC0 =
      (SL_CPC_DRV_UART_TX_LOC << _USART_ROUTELOC0_TXLOC_SHIFT)
      | (SL_CPC_DRV_UART_RX_LOC << _USART_ROUTELOC0_RXLOC_SHIFT);

    SL_CPC_DRV_UART_PERIPHERAL->ROUTEPEN = USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_RXPEN;

  #if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITH_HWFC)
    SL_CPC_DRV_UART_PERIPHERAL->ROUTELOC1 =
      (SL_CPC_DRV_UART_CTS_LOC << _USART_ROUTELOC1_CTSLOC_SHIFT)
      | (SL_CPC_DRV_UART_RTS_LOC << _USART_ROUTELOC1_RTSLOC_SHIFT);
    SL_CPC_DRV_UART_PERIPHERAL->CTRLX    |= USART_CTRLX_CTSEN;
    SL_CPC_DRV_UART_PERIPHERAL->ROUTEPEN |= (USART_ROUTEPEN_CTSPEN | USART_ROUTEPEN_RTSPEN);
  #endif

  #elif defined(USART_ROUTE_TXPEN)

    SL_CPC_DRV_UART_PERIPHERAL->ROUTE =
      USART_ROUTE_TXPEN | USART_ROUTE_RXPEN
      | (SL_CPC_DRV_UART_RX_LOC << _USART_ROUTE_LOCATION_SHIFT);

  #elif defined(_GPIO_USART_ROUTEEN_MASK) || defined(_GPIO_EUSART_ROUTEEN_MASK)

    GPIO->UART_ROUTE.ROUTEEN = GPIO_ROUTEEN_TXPEN;
    GPIO->UART_ROUTE.TXROUTE =
      (SL_CPC_DRV_UART_TX_PORT << GPIO_PORT_SHIFT(TXROUTE))
      | (SL_CPC_DRV_UART_TX_PIN << GPIO_PIN_SHIFT(TXROUTE));
    GPIO->UART_ROUTE.RXROUTE =
      (SL_CPC_DRV_UART_RX_PORT << GPIO_PORT_SHIFT(RXROUTE))
      | (SL_CPC_DRV_UART_RX_PIN << GPIO_PIN_SHIFT(RXROUTE));

  #if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITH_HWFC)
    GPIO->UART_ROUTE.ROUTEEN |= GPIO_ROUTEEN_RTSPEN;
    GPIO->UART_ROUTE.RTSROUTE =
      (SL_CPC_DRV_UART_RTS_PORT << GPIO_PORT_SHIFT(RTSROUTE))
      | (SL_CPC_DRV_UART_RTS_PIN << GPIO_PIN_SHIFT(RTSROUTE));
    GPIO->UART_ROUTE.CTSROUTE =
      (SL_CPC_DRV_UART_CTS_PORT << GPIO_PORT_SHIFT(CTSROUTE))
      | (SL_CPC_DRV_UART_CTS_PIN << GPIO_PIN_SHIFT(CTSROUTE));

  #endif
  #endif
  }

  // Init DMA
  {
    Ecode_t ecode;

    // Init DMA
    ecode = DMADRV_Init();
    if (ecode != ECODE_EMDRV_DMADRV_OK && ecode != ECODE_EMDRV_DMADRV_ALREADY_INITIALIZED) {
      SLI_CPC_ASSERT(0);
    }

    // Allocate read and write channel
    ecode = DMADRV_AllocateChannel(&read_channel, NULL);
    if (ecode != ECODE_OK) {
      SLI_CPC_ASSERT(0);
    }

    ecode = DMADRV_AllocateChannel(&write_channel, NULL);
    if (ecode != ECODE_OK) {
      DMADRV_FreeChannel(read_channel);
      SLI_CPC_ASSERT(0);
    }

    // Configure DMA transfer
    #if defined(_SILICON_LABS_32B_SERIES_2)
    rx_config = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(CPC_LDMA_RX_PERIPH_TRIGGER(SL_CPC_DRV_UART_PERIPHERAL_NO));
    tx_config = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(CPC_LDMA_TX_PERIPH_TRIGGER(SL_CPC_DRV_UART_PERIPHERAL_NO));
    rx_config.ldmaDbgHalt = true;
    tx_config.ldmaDbgHalt = true;
    #else
    rx_config = (sl_hal_ldma_transfer_config_t)SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(CPC_LDMA_RX_PERIPH_TRIGGER(SL_CPC_DRV_UART_PERIPHERAL_NO));
    tx_config = (sl_hal_ldma_transfer_config_t)SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(CPC_LDMA_TX_PERIPH_TRIGGER(SL_CPC_DRV_UART_PERIPHERAL_NO));
    rx_config.debug_halt_en = true;
    tx_config.debug_halt_en = true;
    #endif
  }

#if defined(SL_CPC_PRIMARY_FIRMWARE_UPGRADE_RECOVERY_PINS_SUPPORT_ENABLED) && (SL_CPC_PRIMARY_FIRMWARE_UPGRADE_RECOVERY_PINS_SUPPORT_ENABLED >= 1)
  cpc_gpio_set_pin_mode(SL_CPC_DRV_UART_RESET_PORT, SL_CPC_DRV_UART_RESET_PIN, gpioModePushPull, 1);
  cpc_gpio_set_pin_mode(SL_CPC_DRV_UART_WAKE_PORT, SL_CPC_DRV_UART_WAKE_PORT, gpioModePushPull, 1);
#endif

  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 * Initialize the rest of the driver after the UART peripheral has been
 * initialized in uart_drv_hw_init.
 ******************************************************************************/
static sl_status_t uart_drv_init(sli_cpc_drv_t *drv, sli_cpc_instance_t *inst)
{
  uint32_t buf_cnt;
  sl_cpc_buffer_handle_t *buffer_handle;

  (void)drv;

  // keep a reference to this instance associated with that driver. That is
  // used in the dispatcher as well as for debug counters.
  driver_instance = inst;

  sl_slist_init(&rx_free_buffer_handle_list_head);
  sl_slist_init(&rx_pending_list_head);
  sl_slist_init(&tx_submitted_list_head);
  sl_slist_init(&tx_complete_list_head);

  sli_cpc_dispatcher_init_handle(&dispatcher_handle, inst);
#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITHOUT_HWFC)
  sli_cpc_dispatcher_init_handle(&recovery_dispatcher_handle, inst);
#endif

  for (buf_cnt = 0; buf_cnt < SL_CPC_DRV_UART_RX_QUEUE_SIZE; buf_cnt++) {
    if (sli_cpc_get_buffer_handle_for_rx(inst, &buffer_handle, false) != SL_STATUS_OK) {
      SLI_CPC_ASSERT(0);
      return SL_STATUS_ALLOCATION_FAILED;
    }
    push_free_rx_buffer_handle(buffer_handle);
  }

  free_tx_items = SL_CPC_DRV_UART_TX_QUEUE_SIZE;

  NVIC_EnableIRQ(CPC_UART_TX_IRQn(SL_CPC_DRV_UART_PERIPHERAL_NO));

  // Discard false frames and/or IRQs
  CPC_UART_CLEAR_CMD();

  cpc_uart_int_clear(SL_CPC_DRV_UART_PERIPHERAL, 0xFFFFFFFF);
  cpc_uart_int_enable(SL_CPC_DRV_UART_PERIPHERAL, CPC_UART_IF_TXC);

  cpc_uart_enable(SL_CPC_DRV_UART_PERIPHERAL);

#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITHOUT_HWFC)
  uint8_t * buffer_ptr;

  if (sli_cpc_get_raw_rx_buffer(driver_instance, &buffer_ptr) != SL_STATUS_OK) {
    SLI_CPC_ASSERT(0);
    return SL_STATUS_ALLOCATION_FAILED;
  }
  rx_descriptor[0] = (cpc_ldma_descriptor_t)CPC_LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&(SL_CPC_DRV_UART_PERIPHERAL->RXDATA), buffer_ptr, SL_MIN(SLI_CPC_DRV_UART_RX_FRAME_MAX_LENGTH, CPC_LDMA_DESCRIPTOR_MAX_XFER_SIZE));

  if (sli_cpc_get_raw_rx_buffer(driver_instance, &buffer_ptr) != SL_STATUS_OK) {
    SLI_CPC_ASSERT(0);
    return SL_STATUS_ALLOCATION_FAILED;
  }
  rx_descriptor[1] = (cpc_ldma_descriptor_t)CPC_LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&(SL_CPC_DRV_UART_PERIPHERAL->RXDATA), buffer_ptr, SL_MIN(SLI_CPC_DRV_UART_RX_FRAME_MAX_LENGTH, CPC_LDMA_DESCRIPTOR_MAX_XFER_SIZE));
  rx_descriptor[0].xfer.CPC_LDMA_DESCRIPTOR_DONE_IFS = 0;
  rx_descriptor[1].xfer.CPC_LDMA_DESCRIPTOR_DONE_IFS = 0;

  update_dma_desc_link_abs(&rx_descriptor[0], &rx_descriptor[1], false);
  update_dma_desc_link_abs(&rx_descriptor[1], &rx_descriptor[0], false);
  rx_descriptor_head = &rx_descriptor[0];

  memset(&recovery_context, 0x00, sizeof(recovery_context));

  // Get a receive queue item and don't give it back.
  // This will ensure that there is always one more RX buffer available for the
  // driver in the non-HWFC mode.
  // Non-HWFC requires one RX buffer for the active descriptor and one for the spill descriptor.
  sl_cpc_receive_queue_item_t *item;
  if (sli_cpc_get_receive_queue_item(inst, &item) != SL_STATUS_OK) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

#endif
  next_rx_size = SLI_CPC_HDLC_HEADER_RAW_SIZE;

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Start reception
 ******************************************************************************/
static sl_status_t uart_drv_start_rx(sli_cpc_drv_t *drv)
{
  (void)drv;

  restart_dma();
  return SL_STATUS_OK;
}

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT) && (SL_CPC_PRIMARY_FIRMWARE_UPGRADE_SUPPORT_ENABLED >= 1)
static void uart_drv_deinit_for_firmware_upgrade(sli_cpc_drv_t *drv)
{
  MCU_DECLARE_IRQ_STATE;

  (void)drv;

  MCU_ENTER_ATOMIC();

  {
    NVIC_DisableIRQ(CPC_UART_TX_IRQn(SL_CPC_DRV_UART_PERIPHERAL_NO));
    NVIC_ClearPendingIRQ(CPC_UART_TX_IRQn(SL_CPC_DRV_UART_PERIPHERAL_NO));

    cpc_uart_int_disable(SL_CPC_DRV_UART_PERIPHERAL, CPC_UART_IF_TXC);
    cpc_uart_int_clear(SL_CPC_DRV_UART_PERIPHERAL, CPC_UART_IF_TXC);
    cpc_uart_disable(SL_CPC_DRV_UART_PERIPHERAL);

    DMADRV_StopTransfer(read_channel);
    DMADRV_StopTransfer(write_channel);

#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITHOUT_HWFC)
    memset(&recovery_context, 0x00, sizeof(recovery_context));
    sli_cpc_dispatcher_cancel(&recovery_dispatcher_handle);
#endif
    sli_cpc_dispatcher_cancel(&dispatcher_handle);

    deinited_for_firmware_upgrade = true;
  }

  MCU_EXIT_ATOMIC();
}
#endif

/***************************************************************************/ /**
 * Gets CPC driver capabilities.
 ******************************************************************************/
static sl_status_t uart_drv_get_capabilities(sli_cpc_drv_t *drv,
                                             sli_cpc_drv_capabilities_t *capabilities)
{
  (void)drv;

  if (capabilities == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  *capabilities = (sli_cpc_drv_capabilities_t){ 0 };
  capabilities->preprocess_hdlc_header = true;
  capabilities->uart_flowcontrol = SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITH_HWFC;

  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 * Read bytes from UART.
 ******************************************************************************/
static sl_status_t uart_drv_read_data(sli_cpc_drv_t *drv,
                                      sl_cpc_buffer_handle_t **buffer_handle)
{
  MCU_DECLARE_IRQ_STATE;

  (void)drv;

  MCU_ENTER_ATOMIC();

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
  SLI_CPC_ASSERT(deinited_for_firmware_upgrade == false);
#endif
  SLI_CPC_ASSERT(buffer_handle != NULL);

  *buffer_handle = sli_cpc_pop_driver_buffer_handle(&rx_pending_list_head);

  if (*buffer_handle == NULL) {
    MCU_EXIT_ATOMIC();
    return SL_STATUS_EMPTY;
  }

  MCU_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 * Write bytes to UART.
 ******************************************************************************/
static sl_status_t uart_drv_transmit_data(sli_cpc_drv_t *drv,
                                          sl_cpc_buffer_handle_t *buffer_handle)
{
  sl_status_t status;
  MCU_DECLARE_IRQ_STATE;

  (void)drv;

  MCU_ENTER_ATOMIC();

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
  SLI_CPC_ASSERT(deinited_for_firmware_upgrade == false);
#endif

  if (free_tx_items == 0) {
    MCU_EXIT_ATOMIC();
    return SL_STATUS_NOT_READY;
  }

  --free_tx_items;

  SLI_CPC_ASSERT(buffer_handle->hdlc_header != NULL);

  sli_cpc_push_back_driver_buffer_handle(&tx_submitted_list_head, buffer_handle);
  sli_cpc_drv_wake_host_gpio(true);

  status = prepare_next_tx();
  MCU_EXIT_ATOMIC();

  return status;
}

/***************************************************************************/ /**
 * Checks if driver is ready to transmit.
 ******************************************************************************/
static bool uart_drv_is_transmit_ready(sli_cpc_drv_t *drv)
{
  bool ready;

  (void)drv;

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
  SLI_CPC_ASSERT(deinited_for_firmware_upgrade == false);
#endif

  MCU_ATOMIC_SECTION(ready = tx_ready && free_tx_items > 0; );
  return ready;
}

/***************************************************************************//**
 * Get currently configured bus bitrate
 ******************************************************************************/
static uint32_t uart_drv_get_bus_bitrate(sli_cpc_drv_t *drv)
{
  (void)drv;

  return SL_CPC_DRV_UART_BAUDRATE;
}

/***************************************************************************//**
 * Get maximum bus bitrate
 ******************************************************************************/
static uint32_t uart_drv_get_bus_max_bitrate(sli_cpc_drv_t *drv)
{
  (void)drv;

  return SL_CPC_DRV_UART_BAUDRATE;
}

/***************************************************************************/ /**
 * Initialize peripheral clocks
 ******************************************************************************/
static void init_clocks(void)
{
  sl_status_t status;
#if !defined(SL_CATALOG_CLOCK_MANAGER_PRESENT)
  // Missing CLOCK_MANAGER. Manually route clock branches with em_cmu
  #if defined(SL_CPC_DRV_PERIPH_IS_EUSART)
    #if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2) || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_7) || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_9)
      #if defined(EUART_PRESENT)
  if (SL_CPC_DRV_UART_PERIPHERAL_NO == 0) {
    CMU_CLOCK_SELECT_SET(EUART0CLK, EM01GRPACLK);
  }
      #elif defined(EUSART_PRESENT)
  if (SL_CPC_DRV_UART_PERIPHERAL_NO == 0) {
    CMU_CLOCK_SELECT_SET(EUSART0CLK, EM01GRPACLK);
  }
      #endif
    #elif (_SILICON_LABS_32B_SERIES_2_CONFIG >= 3)
  CMU_CLOCK_SELECT_SET(EM01GRPCCLK, HFRCODPLL);
  if (SL_CPC_DRV_UART_PERIPHERAL_NO == 0) {
    CMU_CLOCK_SELECT_SET(EUSART0CLK, EM01GRPCCLK);
  }
    #else
      #error Unsupported Series 2 board configuration.
    #endif
  #endif // SL_CPC_DRV_PERIPH_IS_EUSART
#endif // SL_CATALOG_CLOCK_MANAGER_PRESENT

  // Enable peripheral clocks
  status = sl_clock_manager_enable_bus_clock(CPC_UART_CLOCK);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
}

/***************************************************************************/ /**
 * Pop a RX buffer handle
 ******************************************************************************/
static sl_cpc_buffer_handle_t* pop_free_rx_buffer_handle(void)
{
  sl_cpc_buffer_handle_t *buffer_handle;
  MCU_DECLARE_IRQ_STATE;

  MCU_ENTER_ATOMIC();
  if (free_rx_buffer_handles == 0) {
    need_rx_buffer_handle = true;
    SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] need_rx_buffer_handle = true", __LINE__);
    SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] stopping DMA", __LINE__);
    DMADRV_StopTransfer(read_channel);
    MCU_EXIT_ATOMIC();
    return NULL;
  }
  buffer_handle = sli_cpc_pop_driver_buffer_handle(&rx_free_buffer_handle_list_head);
  if (buffer_handle == NULL) {
    SLI_CPC_ASSERT(0); // Should not have reached this point, free_rx_buffer_handles == 0
    MCU_EXIT_ATOMIC();
    return NULL;
  }
  --free_rx_buffer_handles;
  MCU_EXIT_ATOMIC();
  return buffer_handle;
}

/***************************************************************************/ /**
 * Push a unused RX buffer handle
 ******************************************************************************/
static void push_free_rx_buffer_handle(sl_cpc_buffer_handle_t* buffer_handle)
{
  MCU_DECLARE_IRQ_STATE;

  MCU_ENTER_ATOMIC();
  if (free_rx_buffer_handles >= SL_CPC_DRV_UART_RX_QUEUE_SIZE) {
    SLI_CPC_ASSERT(0);
    MCU_EXIT_ATOMIC();
    return;
  }
  sli_cpc_push_driver_buffer_handle(&rx_free_buffer_handle_list_head, buffer_handle);
  ++free_rx_buffer_handles;
  if (need_rx_buffer_handle) {
    need_rx_buffer_handle = false;
    SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] need_rx_buffer_handle = false", __LINE__);
#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITH_HWFC)
    if (!need_rx_buffer) {
      restart_dma();
    }
#else
    restart_dma();
#endif
  }
  MCU_EXIT_ATOMIC();
}

/***************************************************************************/ /**
 * Prepare for transmission of next buffer.
 ******************************************************************************/
static sl_status_t prepare_next_tx(void)
{
  sl_cpc_buffer_handle_t *buffer_handle;
  Ecode_t code;
  uint8_t idx = 0;
  uint16_t payload_len;
  MCU_DECLARE_IRQ_STATE;

  MCU_ENTER_ATOMIC();
  buffer_handle = SL_SLIST_ENTRY(tx_submitted_list_head, sl_cpc_buffer_handle_t, driver_node);
  if (buffer_handle == NULL) {
    MCU_EXIT_ATOMIC();
    return SL_STATUS_EMPTY;
  }

  payload_len = buffer_handle->data_length;

  if (buffer_handle->hdlc_header == NULL) {
    SLI_CPC_ASSERT(0);
    return SL_STATUS_FAIL;
  }

  if (payload_len > 0u) {
    /* First TX descriptor is for the header */
    tx_descriptor[idx++] = (cpc_ldma_descriptor_t)CPC_LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(
      buffer_handle->hdlc_header,
      &(SL_CPC_DRV_UART_PERIPHERAL->TXDATA),
      SLI_CPC_HDLC_HEADER_RAW_SIZE,
      1u);

    /* next descriptor(s) are for the payload */
    if (payload_len <= DMADRV_MAX_XFER_COUNT) {
      tx_descriptor[idx++] = (cpc_ldma_descriptor_t)CPC_LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(
        buffer_handle->data,
        &(SL_CPC_DRV_UART_PERIPHERAL->TXDATA),
        payload_len,
        1u);
    } else if (payload_len <= (DMADRV_MAX_XFER_COUNT * 2)) {
      tx_descriptor[idx++] = (cpc_ldma_descriptor_t)CPC_LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(
        buffer_handle->data,
        &(SL_CPC_DRV_UART_PERIPHERAL->TXDATA),
        DMADRV_MAX_XFER_COUNT,
        1u);
      tx_descriptor[idx++] = (cpc_ldma_descriptor_t)CPC_LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(
        &((uint8_t *)buffer_handle->data)[DMADRV_MAX_XFER_COUNT],
        &(SL_CPC_DRV_UART_PERIPHERAL->TXDATA),
        (payload_len - DMADRV_MAX_XFER_COUNT),
        1u);
    } else {
      MCU_EXIT_ATOMIC();
      return SL_STATUS_INVALID_PARAMETER;
    }

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
    /*
     * If the security is enabled and there is a security tag to send, send it
     * before the FCS.
     */
    if (buffer_handle->security_tag) {
      tx_descriptor[idx++] = (cpc_ldma_descriptor_t)CPC_LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(
        (uint8_t *)buffer_handle->security_tag,
        &(SL_CPC_DRV_UART_PERIPHERAL->TXDATA),
        SLI_SECURITY_TAG_LENGTH_BYTES,
        1u);
    }
#endif

    /* Caution: last descriptor, don't increment idx */
    tx_descriptor[idx] = (cpc_ldma_descriptor_t)CPC_LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(
      &buffer_handle->fcs,
      &(SL_CPC_DRV_UART_PERIPHERAL->TXDATA),
      SLI_CPC_HDLC_FCS_SIZE);
  } else {
    /*
     * This buffer has no payload, header only.
     * Caution: last descriptor, don't increment idx
     */
    tx_descriptor[idx] = (cpc_ldma_descriptor_t)CPC_LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(
      buffer_handle->hdlc_header,
      &(SL_CPC_DRV_UART_PERIPHERAL->TXDATA),
      SLI_CPC_HDLC_HEADER_RAW_SIZE);
  }

  /* Clear doneIfs */
  for (uint8_t doneIfIndex = 0; doneIfIndex <= idx; doneIfIndex++) {
    tx_descriptor[doneIfIndex].xfer.CPC_LDMA_DESCRIPTOR_DONE_IFS = 0u;
  }

  tx_ready = false;

  code = DMADRV_LdmaStartTransfer(write_channel,
                                  &tx_config,
                                  tx_descriptor,
                                  NULL,
                                  NULL);
  SLI_CPC_ASSERT(code == ECODE_OK);

  MCU_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 * Notification when RX buffer becomes free.
 ******************************************************************************/
static void uart_drv_on_rx_buffer_free(sli_cpc_drv_t *drv)
{
  MCU_DECLARE_IRQ_STATE;

  (void)drv;

  MCU_ENTER_ATOMIC();

  SL_CPC_JOURNAL_RECORD_DEBUG("uart_drv_on_rx_buffer_free", __LINE__);

#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITH_HWFC)
  // Restart DMA if we previously ran out of buffers
  if (need_rx_buffer && (rx_free_buffer_handle_list_head != NULL)) {
    need_rx_buffer = false;
    SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] need_rx_buffer = false", __LINE__);
    restart_dma();
  }
#endif

  MCU_EXIT_ATOMIC();
}

/***************************************************************************/ /**
 * Notification when RX buffer handle becomes free.
 ******************************************************************************/
static void uart_drv_on_rx_buffer_handle_free(sli_cpc_drv_t *drv)
{
  sl_status_t status = SL_STATUS_FAIL;
  MCU_DECLARE_IRQ_STATE;

  (void)drv;

  MCU_ENTER_ATOMIC();

  sl_cpc_buffer_handle_t *buffer_handle;

  SL_CPC_JOURNAL_RECORD_DEBUG("uart_drv_on_rx_buffer_handle_free", __LINE__);
  if (free_rx_buffer_handles < SL_CPC_DRV_UART_RX_QUEUE_SIZE) {
    status = sli_cpc_get_buffer_handle_for_rx(driver_instance, &buffer_handle, false);
    if (status == SL_STATUS_OK) {
      push_free_rx_buffer_handle(buffer_handle);
    }
  } else {
    SL_CPC_JOURNAL_RECORD_ERROR("Possible memory leak, free_rx_buffer_handles >= SL_CPC_DRV_UART_RX_QUEUE_SIZE", __LINE__);
    SLI_CPC_ASSERT(0);
  }

  MCU_EXIT_ATOMIC();
}

/***************************************************************************//**
 * Dispatcher function, to notify core that packets have been transmitted
 ******************************************************************************/
static void sli_cpc_drv_dispatch_function(void *data)
{
  sl_cpc_buffer_handle_t *buffer_handle;
  MCU_DECLARE_IRQ_STATE;

  (void)data;

  // When the buffer_handle is popped from tx_complete_list_head, it will decrease
  // entry->handle->ref_count. If an ack is received at this moment, the buffer
  // might get freed before sli_cpc_notify_tx_data_by_drv is called, meaning
  // that this function would do a use-after-free operation. To prevent that,
  // popping off the list and calling the notify function must be done atomically.
  MCU_ENTER_ATOMIC();

  buffer_handle = sli_cpc_pop_driver_buffer_handle(&tx_complete_list_head);

  while (buffer_handle != NULL) {
    sli_cpc_notify_tx_data_by_drv(buffer_handle);
    MCU_EXIT_ATOMIC();

    // release atomic section in case sli_cpc_notify_tx_data_by_drv was long

    // these two lines could be separated in two atomic sections as they are
    // not related to one another, but as they both are very short, group them
    // together for shortness and readability
    MCU_ENTER_ATOMIC();
    ++free_tx_items;
    buffer_handle = sli_cpc_pop_driver_buffer_handle(&tx_complete_list_head);
  }

  MCU_EXIT_ATOMIC();
}

/***************************************************************************//**
 * TX IRQ handler.
 ******************************************************************************/
void CPC_UART_ISR_TX_HANDLER(SL_CPC_DRV_UART_PERIPHERAL_NO)(void)
{
  MCU_DECLARE_IRQ_STATE;
  bool done;
  uint32_t flag = cpc_uart_int_get(SL_CPC_DRV_UART_PERIPHERAL);
  cpc_uart_int_clear(SL_CPC_DRV_UART_PERIPHERAL, flag & CPC_UART_IF_TXC);

  if (flag & CPC_UART_IF_TXC) {
    sl_cpc_buffer_handle_t *buffer_handle;

    DMADRV_TransferDone(write_channel, &done);
    SLI_CPC_ASSERT(done);

    tx_ready = true;

    // these two lines keep buffer_handle->ref_count equals to 1
    // and prevent it from being dropped by the core
    MCU_ENTER_ATOMIC();
    buffer_handle = sli_cpc_pop_driver_buffer_handle(&tx_submitted_list_head);
    SLI_CPC_ASSERT(buffer_handle != NULL);
    sli_cpc_push_back_driver_buffer_handle(&tx_complete_list_head, buffer_handle);
    MCU_EXIT_ATOMIC();

    // it's possible that this interrupt handler is called twice (or more)
    // before the dispatch function is called. In that case, the second call
    // to sli_cpc_dispatcher_push would return an error, that's why the return
    // code is ignored here.
    sli_cpc_dispatcher_push(&dispatcher_handle,
                            sli_cpc_drv_dispatch_function,
                            NULL);

    if (tx_submitted_list_head) {
      prepare_next_tx();
    } else {
      sli_cpc_drv_wake_host_gpio(false);
    }
  }
}

#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITHOUT_HWFC)
static sl_status_t get_already_received_cnt(uint16_t *already_recvd_cnt)
{
  uint32_t count = LDMA_PERIPH->CH[read_channel].DST - rx_descriptor_head->xfer.CPC_LDMA_DESCRIPTOR_DST_ADDR;

  if (count > SL_CPC_DRV_UART_RX_MAX_BUFFER_SIZE) {
    SL_CPC_JOURNAL_RECORD_ERROR("[DRV] get_already_received_cnt: count overflow", __LINE__);
    SLI_CPC_ASSERT(0); // Should never happen
    return SL_STATUS_FAIL;
  }

  *already_recvd_cnt = (uint16_t)count;
  return SL_STATUS_OK;
}

static void swap_descriptors(void)
{
  rx_descriptor_head = CPC_LDMA_DESCRIPTOR_LINKABS_LINKADDR_TO_ADDR(rx_descriptor_head->xfer.CPC_LDMA_DESCRIPTOR_LINK_ADDR);
}
#endif

static bool find_valid_header(uint8_t *buffer, uint16_t buffer_size, uint16_t *header_flag_position)
{
  uint16_t hcs = 0;

  SLI_CPC_ASSERT(buffer_size >= SLI_CPC_HDLC_HEADER_RAW_SIZE);

  for (uint16_t i = 0; i <= buffer_size - SLI_CPC_HDLC_HEADER_RAW_SIZE; i++) {
    if (buffer[i] == SLI_CPC_HDLC_FLAG_VAL) {
      hcs = sli_cpc_hdlc_get_hcs(&buffer[i]);
      if (sli_cpc_validate_crc_sw(&buffer[i], SLI_CPC_HDLC_HEADER_SIZE, hcs)) {
        if (header_flag_position != NULL) {
          *header_flag_position = i;
        }
        return true;
      }
    }
  }

  return false;
}

#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITHOUT_HWFC)
static void dispatch_recovery(void)
{
  uint32_t chMask = 1UL << (uint8_t)read_channel;
  sl_status_t dispatch_status = SL_STATUS_FAIL;

  dispatch_status = sli_cpc_dispatcher_push(&recovery_dispatcher_handle, recovery, NULL);
  SLI_CPC_ASSERT(dispatch_status == SL_STATUS_OK);

  LDMA_PERIPH->CH[read_channel].LINK &= ~LDMA_CH_LINK_LINK;

  // Disable interrupts
#ifdef _LDMA_CH_CTRL_DONEIFSEN_MASK
  LDMA_PERIPH->CH[read_channel].CTRL &= ~_LDMA_CH_CTRL_DONEIFSEN_MASK;
#else
  LDMA_PERIPH->CH[read_channel].CTRL &= ~_LDMA_CH_CTRL_DONEIEN_MASK;
#endif
  LDMA_PERIPH->IEN &= ~chMask;

  // Clear any pending ISR
  cpc_ldma_int_clear(chMask);
}

static void recovery(void *data)
{
  typedef enum {
    RECOVERY_FAILED = 0,
    RECOVERY_FOUND_HEADER,
    RECOVERY_FOUND_HEADER_NO_PAYLOAD,
    RECOVERY_FOUND_PAYLOAD
  } recovery_state_t;

  MCU_DECLARE_IRQ_STATE;
  (void)data;
  bool misaligned_payload = false;
  recovery_state_t recovery_state = RECOVERY_FAILED;
  uint8_t *rx_buffer_ptr;
  uint16_t already_recvd_cnt = 0;
  uint16_t header_flag_position = 0;
  sl_status_t status;
  bool spilled_data_fully_recovered = true;
  bool resynced = false;
  bool header_found = false;

  SL_CPC_RECOVERY_TRACE("[RECOVERY] Recovery function entry", __LINE__);
  MCU_ENTER_ATOMIC();

  do {
    SL_CPC_RECOVERY_TRACE("[RECOVERY] Start of main recovery loop", __LINE__);
    misaligned_payload = false;
    recovery_state = RECOVERY_FAILED;
    already_recvd_cnt = 0;
    header_flag_position = 0;
    spilled_data_fully_recovered = true;

    if (recovery_context.spilled_buffer == NULL) {
      SLI_CPC_ASSERT(0);
      goto exit;
    }

    // Allocate a RX entry if necessary
    if (recovery_context.buffer_handle == NULL && header_expected_next) {
      SL_CPC_RECOVERY_TRACE("[RECOVERY] Need to allocate buffer handle (NULL && header_expected_next)", __LINE__);
      recovery_context.buffer_handle = pop_free_rx_buffer_handle();

      if (recovery_context.buffer_handle == NULL) {
        SL_CPC_RECOVERY_TRACE("[RECOVERY] No buffer handle available - stopping and waiting", __LINE__);
        // Stop and wait for buffer
        recovery_context.spilled_buffer = NULL; // Will be reused by the ISR
        MCU_EXIT_ATOMIC();
        return;
      }
      SL_CPC_RECOVERY_TRACE("[RECOVERY] Buffer handle allocated successfully", __LINE__);
    } else {
      SL_CPC_RECOVERY_TRACE("[RECOVERY] Skip buffer allocation (buffer_handle != NULL || !header_expected_next)", __LINE__);
    }

    //===========================================================================
    //
    //  Driver is out of sync. Attempt a recovery by fetching potential HDLC headers
    //  and place them in a sliding window.
    //
    //===========================================================================
    if (recovery_context.out_of_sync) {
      SL_CPC_RECOVERY_TRACE("[RECOVERY] Driver is out of sync - starting resync procedure", __LINE__);
      do {
        SL_CPC_RECOVERY_TRACE("[RECOVERY] Resync loop - loading sliding window with ISR data", __LINE__);
        //===========================================================================
        //
        // First we load the sliding window with data received by the ISR and check if
        // any valid header is found
        //
        //  +-------------------+-------------------+
        //  | Previous ISR data |  Current ISR data |
        //  +-------------------+-------------------+
        //
        //===========================================================================
        // Slide and load the resync window with contents from the DMA complete ISR
        memmove(recovery_context.hdlc_header_out_of_sync_sliding_window, recovery_context.hdlc_header_out_of_sync_sliding_window + SLI_CPC_HDLC_HEADER_RAW_SIZE, SLI_CPC_HDLC_HEADER_RAW_SIZE);
        memcpy(recovery_context.hdlc_header_out_of_sync_sliding_window + SLI_CPC_HDLC_HEADER_RAW_SIZE, recovery_context.invalid_hdlc_header, SLI_CPC_HDLC_HEADER_RAW_SIZE);

        // Search for a header in these 14 bytes
        if (find_valid_header(recovery_context.hdlc_header_out_of_sync_sliding_window + 1, sizeof(recovery_context.hdlc_header_out_of_sync_sliding_window) - 1, &header_flag_position)) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Found valid header in sliding window - resync successful", __LINE__);
          header_flag_position += 1; // We skipped the first byte

          // Copy the contents of the recovered header
          memcpy(recovery_context.buffer_handle->hdlc_header, recovery_context.hdlc_header_out_of_sync_sliding_window + header_flag_position, SLI_CPC_HDLC_HEADER_RAW_SIZE);
          recovery_context.buffer_handle->data_length = sli_cpc_hdlc_get_length(recovery_context.buffer_handle->hdlc_header);

          // Remember the bytes at the end of the sliding window.
          // They are not part of the spill buffer and could be the begining of a payload or another header
          recovery_context.out_of_sync_extra_bytes_len = sizeof(recovery_context.hdlc_header_out_of_sync_sliding_window) - (header_flag_position + SLI_CPC_HDLC_HEADER_RAW_SIZE);
          memcpy(recovery_context.out_of_sync_extra_bytes, recovery_context.hdlc_header_out_of_sync_sliding_window + SLI_CPC_HDLC_HEADER_RAW_SIZE + header_flag_position, recovery_context.out_of_sync_extra_bytes_len);

          // Resync was a success, reset the sliding window
          memset(recovery_context.hdlc_header_out_of_sync_sliding_window, 0x00, sizeof(recovery_context.hdlc_header_out_of_sync_sliding_window));

          recovery_context.out_of_sync = false;
          resynced = true; // We will be using this header
          break;
        }
        SL_CPC_RECOVERY_TRACE("[RECOVERY] No valid header found in sliding window - trying extended search", __LINE__);

        //===========================================================================
        //
        // Did we receive a bit more data since the ISR?
        // If so, maybe that's our golden ticket to resync.
        //
        //  +-------------------+-------------------+--------------+
        //  | Previous ISR data |  Current ISR data | Spill Buffer |
        //  +-------------------+-------------------+--------------+
        //                                    ^------------^
        //                                    i ->
        //===========================================================================
        if (get_already_received_cnt(&already_recvd_cnt) != SL_STATUS_OK) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] get_already_received_cnt failed - recovery failed", __LINE__);
          recovery_state = RECOVERY_FAILED;
          goto exit;
        }
        if (already_recvd_cnt > 0 ) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Received additional data since ISR - searching for header candidates", __LINE__);
          uint8_t header_candidate[SLI_CPC_HDLC_HEADER_RAW_SIZE];
          for (uint8_t i = 1; i < SLI_CPC_HDLC_HEADER_RAW_SIZE; ++i) {
            if (i > already_recvd_cnt) {
              SL_CPC_RECOVERY_TRACE("[RECOVERY] Not enough bytes received to proceed with candidate search", __LINE__);
              // We did not receive enough bytes to proceed
              break;
            }
            // Look if the byte at `i` is a HDLC header
            if (recovery_context.hdlc_header_out_of_sync_sliding_window[(SLI_CPC_HDLC_HEADER_RAW_SIZE - 1) + i] == SLI_CPC_HDLC_FLAG_VAL) {
              SL_CPC_RECOVERY_TRACE("[RECOVERY] Found potential HDLC flag - validating header candidate", __LINE__);
              // Populate a temporary buffer and see if it's a valid HDLC header
              memset(header_candidate, 0, sizeof(header_candidate));

              // Copy part of the second half of the ISR data
              memcpy(header_candidate,
                     recovery_context.hdlc_header_out_of_sync_sliding_window + SLI_CPC_HDLC_HEADER_RAW_SIZE + i,
                     SLI_CPC_HDLC_HEADER_RAW_SIZE - i);

              // Copy part of the spill buffer
              memcpy(header_candidate + (SLI_CPC_HDLC_HEADER_RAW_SIZE - i),
                     recovery_context.spilled_buffer,
                     i);

              if (find_valid_header(header_candidate, sizeof(header_candidate), &header_flag_position)) {
                SL_CPC_RECOVERY_TRACE("[RECOVERY] Header candidate validated - resync successful", __LINE__);
                // Copy the contents of the recovered header
                memcpy(recovery_context.buffer_handle->hdlc_header, header_candidate + header_flag_position, SLI_CPC_HDLC_HEADER_RAW_SIZE);
                recovery_context.buffer_handle->data_length = sli_cpc_hdlc_get_length(recovery_context.buffer_handle->hdlc_header);

                // Remember the offset to skip bytes in the spill buffer
                recovery_context.offset = i;

                // Resync was a success
                memset(recovery_context.hdlc_header_out_of_sync_sliding_window, 0x00, sizeof(recovery_context.hdlc_header_out_of_sync_sliding_window));
                recovery_context.out_of_sync = false;
                resynced = true; // We will be using this header

                // There is no unused bytes in the resync window buffer
                recovery_context.out_of_sync_extra_bytes_len = 0;
                break;
              }
              SL_CPC_RECOVERY_TRACE("[RECOVERY] Header candidate validation failed", __LINE__);
            }
          }
          if (resynced) {
            SL_CPC_RECOVERY_TRACE("[RECOVERY] Resync completed - breaking out of resync superloop", __LINE__);
            break; // Break out of resync superloop
          }
        } else {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] No additional data received since ISR", __LINE__);
        }

        // We are still out of sync, 7 bytes is required
        if (resize_current_dma_descriptor(SLI_CPC_HDLC_HEADER_RAW_SIZE) == SL_STATUS_OK) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Still out of sync - resize successful, loading 7 more bytes", __LINE__);
          push_free_rx_buffer_handle(recovery_context.buffer_handle);
          recovery_context.spilled_buffer = NULL;
          recovery_context.offset = 0;
          recovery_context.buffer_handle = NULL;
          recovery_context.spilled_buffer = NULL;
          // We can exit here, another 7 bytes will be loaded by the DMA and an interrupt will be generated
          MCU_EXIT_ATOMIC();
          return;
        }
        SL_CPC_RECOVERY_TRACE("[RECOVERY] Resize failed - trying to scan spill buffer", __LINE__);

        //===========================================================================
        //
        // We are still out of sync, but there are 7 more bytes available in
        // the spill buffer, slide the window once more and load them.
        //
        //  +-------------------+-------------------+
        //  | Current ISR data  | Spill buffer data |
        //  +-------------------+-------------------+
        //
        //===========================================================================

        memcpy(recovery_context.hdlc_header_out_of_sync_sliding_window, recovery_context.hdlc_header_out_of_sync_sliding_window + SLI_CPC_HDLC_HEADER_RAW_SIZE, SLI_CPC_HDLC_HEADER_RAW_SIZE);
        memcpy(recovery_context.hdlc_header_out_of_sync_sliding_window + SLI_CPC_HDLC_HEADER_RAW_SIZE, recovery_context.spilled_buffer, SLI_CPC_HDLC_HEADER_RAW_SIZE);

        // Search for a header in these 14 bytes
        if (find_valid_header(recovery_context.hdlc_header_out_of_sync_sliding_window, sizeof(recovery_context.hdlc_header_out_of_sync_sliding_window), &header_flag_position)) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Found valid header in ISR+spill buffer window - resync successful", __LINE__);
          // Copy the contents of the recovered header
          memcpy(recovery_context.buffer_handle->hdlc_header, recovery_context.hdlc_header_out_of_sync_sliding_window + header_flag_position, SLI_CPC_HDLC_HEADER_RAW_SIZE);
          recovery_context.buffer_handle->data_length = sli_cpc_hdlc_get_length(recovery_context.buffer_handle->hdlc_header);

          // It's possible that the header began in the first part of the sliding window and ended in the second.
          // If that is the case, we need to add an offset to the spill buffer
          recovery_context.offset = header_flag_position;

          // Resync was a success, reset the sliding window
          memset(recovery_context.hdlc_header_out_of_sync_sliding_window, 0x00, sizeof(recovery_context.hdlc_header_out_of_sync_sliding_window));

          recovery_context.out_of_sync = false;
          resynced = true; // We will be using this header
          break;
        }
        SL_CPC_RECOVERY_TRACE("[RECOVERY] No valid header found in ISR+spill buffer window", __LINE__);

        // We are still out of sync, update the window and load another 7 bytes
        if (resize_current_dma_descriptor(SLI_CPC_HDLC_HEADER_RAW_SIZE) == SL_STATUS_OK) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Resize successful - saving last 7 bytes and loading more", __LINE__);
          // Remember the last 7 bytes - we must have at least 7 bytes at this point
          if (already_recvd_cnt < SLI_CPC_HDLC_HEADER_RAW_SIZE) {
            SLI_CPC_ASSERT(0);
            recovery_state = RECOVERY_FAILED;
            goto exit;
          }
          memcpy(recovery_context.hdlc_header_out_of_sync_sliding_window + SLI_CPC_HDLC_HEADER_RAW_SIZE,
                 recovery_context.spilled_buffer + (already_recvd_cnt - SLI_CPC_HDLC_HEADER_RAW_SIZE),
                 SLI_CPC_HDLC_HEADER_RAW_SIZE);

          push_free_rx_buffer_handle(recovery_context.buffer_handle);
          recovery_context.spilled_buffer = NULL;
          recovery_context.offset = 0;
          recovery_context.buffer_handle = NULL;
          recovery_context.spilled_buffer = NULL;

          // We can exit here, another 7 bytes will be loaded by the DMA and an interrupt will be generated
          MCU_EXIT_ATOMIC();
          return;
        }
        SL_CPC_RECOVERY_TRACE("[RECOVERY] Second resize failed - scanning entire spill buffer", __LINE__);

        //===========================================================================
        //
        // We are still out of sync, but there are 7 more bytes available in
        // the spill buffer. This means we can scan the remaining contents of
        // the spill buffer without worrying about any sliding window
        //
        //  +---------------------------------------+
        //  |          Spill buffer data            |
        //  +---------------------------------------+
        //
        //===========================================================================
        if (get_already_received_cnt(&already_recvd_cnt) != SL_STATUS_OK) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] get_already_received_cnt failed - recovery failed", __LINE__);
          recovery_state = RECOVERY_FAILED;
          goto exit;
        }
        if (find_valid_header(recovery_context.spilled_buffer, already_recvd_cnt, &header_flag_position)) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Found valid header in spill buffer - resync successful", __LINE__);
          // Copy the contents from the spilled_buffer with the offset found from the header_flag_position
          memcpy(recovery_context.buffer_handle->hdlc_header, recovery_context.spilled_buffer + header_flag_position, SLI_CPC_HDLC_HEADER_RAW_SIZE);
          recovery_context.buffer_handle->data_length = sli_cpc_hdlc_get_length(recovery_context.buffer_handle->hdlc_header);

          // Reset the sliding window
          memset(recovery_context.hdlc_header_out_of_sync_sliding_window, 0x00, sizeof(recovery_context.hdlc_header_out_of_sync_sliding_window));

          recovery_context.offset = SLI_CPC_HDLC_HEADER_RAW_SIZE + header_flag_position;
          recovery_context.out_of_sync = false;
          resynced = true;
          break;
        } else {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] No valid header found in spill buffer", __LINE__);
          // We are still out of sync and scanned the entire spill buffer, load another 7 bytes
          uint16_t resize_length = SLI_CPC_HDLC_HEADER_RAW_SIZE + already_recvd_cnt;
          if (resize_current_dma_descriptor(resize_length) == SL_STATUS_OK) {
            SL_CPC_RECOVERY_TRACE("[RECOVERY] Final resize successful - saving last 7 bytes and loading more", __LINE__);
            // Remember the last 7 bytes - we must have at least 7 bytes at this point
            if (already_recvd_cnt < SLI_CPC_HDLC_HEADER_RAW_SIZE) {
              SLI_CPC_ASSERT(0);
              recovery_state = RECOVERY_FAILED;
              goto exit;
            }
            memcpy(recovery_context.hdlc_header_out_of_sync_sliding_window + SLI_CPC_HDLC_HEADER_RAW_SIZE,
                   recovery_context.spilled_buffer + (already_recvd_cnt - SLI_CPC_HDLC_HEADER_RAW_SIZE),
                   SLI_CPC_HDLC_HEADER_RAW_SIZE);

            push_free_rx_buffer_handle(recovery_context.buffer_handle);
            recovery_context.spilled_buffer = NULL;
            recovery_context.offset = already_recvd_cnt;
            recovery_context.buffer_handle = NULL;
            recovery_context.spilled_buffer = NULL;
            // We can exit here, another 7 bytes will be loaded by the DMA and an interrupt will be generated
            MCU_EXIT_ATOMIC();
            return;
          } else {
            SL_CPC_RECOVERY_TRACE("[RECOVERY] Final resize failed - recovery failed", __LINE__);
            // If we end up here, we traversed through the entire spill buffer and did not resync.
            // We need to drop the spill buffer and restart the DMA
            recovery_state = RECOVERY_FAILED;
            goto exit;
          }
        }
      } while (false); // Used for breaking out of if(recovery_context.out_of_sync)
    } else {
      SL_CPC_RECOVERY_TRACE("[RECOVERY] Driver is in sync - skipping resync procedure", __LINE__);
    }
    if (recovery_context.out_of_sync_extra_bytes_len > SLI_CPC_HDLC_HEADER_RAW_SIZE) {
      SLI_CPC_ASSERT(0);
      goto exit;
    }

    //===========================================================================
    //
    //  Driver is in sync. Attempt to recover a header that has spilled
    //
    //===========================================================================
    // Do we need to re-align a payload ?
    if (recovery_context.misaligned_payload) {
      SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] RECOVERY: Processing misaligned payload from previous cycle", __LINE__);
      SL_CPC_RECOVERY_TRACE("[RECOVERY] Setting misaligned_payload flag", __LINE__);
      misaligned_payload = true;
      recovery_context.misaligned_payload = false;
    } else {
      SL_CPC_RECOVERY_TRACE("[RECOVERY] No misaligned payload to process", __LINE__);
    }

    if (header_expected_next) {
      SL_CPC_RECOVERY_TRACE("[RECOVERY] Expecting header next - processing header recovery", __LINE__);
      // Expect a header
      if (resynced) {
        SL_CPC_RECOVERY_TRACE("[RECOVERY] Using header from previous resync", __LINE__);
        // A resync previously occured, and a header was found. Use it now.
        resynced = false;
        header_found = true;
      } else {
        SL_CPC_RECOVERY_TRACE("[RECOVERY] No resync - checking for spilled header data", __LINE__);
        if (get_already_received_cnt(&already_recvd_cnt) != SL_STATUS_OK) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] get_already_received_cnt failed - recovery failed", __LINE__);
          recovery_state = RECOVERY_FAILED;
          goto exit;
        }
        if (recovery_context.out_of_sync_extra_bytes_len > 0) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Have out_of_sync_extra_bytes - checking if enough data received", __LINE__);
          if (already_recvd_cnt >= (uint8_t)(SLI_CPC_HDLC_HEADER_RAW_SIZE - recovery_context.out_of_sync_extra_bytes_len)) {
            SL_CPC_RECOVERY_TRACE("[RECOVERY] Enough data received - recovering header from extra bytes + spill buffer", __LINE__);
            // A resync previously occured, and there was extra bytes in the sliding window.
            // We need to recover them by prepending them to the spill buffer
            if (recovery_context.out_of_sync_extra_bytes_len >= SLI_CPC_HDLC_HEADER_RAW_SIZE) {
              SLI_CPC_ASSERT(0);
              goto exit;
            }

            memcpy(recovery_context.buffer_handle->hdlc_header,
                   recovery_context.out_of_sync_extra_bytes,
                   recovery_context.out_of_sync_extra_bytes_len);

            memcpy((uint8_t *)recovery_context.buffer_handle->hdlc_header + recovery_context.out_of_sync_extra_bytes_len,
                   recovery_context.spilled_buffer,
                   SLI_CPC_HDLC_HEADER_RAW_SIZE - recovery_context.out_of_sync_extra_bytes_len);

            recovery_context.offset += (SLI_CPC_HDLC_HEADER_RAW_SIZE - recovery_context.out_of_sync_extra_bytes_len);
            recovery_context.out_of_sync_extra_bytes_len = 0;

            if (find_valid_header(recovery_context.buffer_handle->hdlc_header, SLI_CPC_HDLC_HEADER_RAW_SIZE, NULL)) {
              SL_CPC_RECOVERY_TRACE("[RECOVERY] Header validation successful", __LINE__);
              recovery_context.buffer_handle->data_length = sli_cpc_hdlc_get_length(recovery_context.buffer_handle->hdlc_header);
              header_found = true;
            } else {
              SL_CPC_RECOVERY_TRACE("[RECOVERY] Header validation failed", __LINE__);
            }
          } else {
            SL_CPC_RECOVERY_TRACE("[RECOVERY] Not enough data received yet", __LINE__);
          }
        } else if (already_recvd_cnt >= SLI_CPC_HDLC_HEADER_RAW_SIZE) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] No extra bytes but enough data in spill buffer - searching for header", __LINE__);
          if (recovery_context.offset >= already_recvd_cnt) {
            SLI_CPC_ASSERT(0);
            goto exit;
          }
          if (find_valid_header(recovery_context.spilled_buffer + recovery_context.offset, already_recvd_cnt - recovery_context.offset, &header_flag_position)) {
            SL_CPC_RECOVERY_TRACE("[RECOVERY] Found valid header in spill buffer", __LINE__);
            // Copy the contents of the header to an entry and obtain payload length
            memcpy(recovery_context.buffer_handle->hdlc_header,
                   recovery_context.spilled_buffer + recovery_context.offset + header_flag_position,
                   SLI_CPC_HDLC_HEADER_RAW_SIZE);

            recovery_context.buffer_handle->data_length = sli_cpc_hdlc_get_length(recovery_context.buffer_handle->hdlc_header);
            recovery_context.offset += SLI_CPC_HDLC_HEADER_RAW_SIZE + header_flag_position;
            header_found = true;
          } else {
            SL_CPC_RECOVERY_TRACE("[RECOVERY] No valid header found in spill buffer", __LINE__);
          }
        } else {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Not enough data in spill buffer", __LINE__);
        }
      }

      if (header_found) {
        SL_CPC_RECOVERY_TRACE("[RECOVERY] Header found - processing based on payload length", __LINE__);
        if (recovery_context.buffer_handle == NULL) {
          SLI_CPC_ASSERT(0);
          recovery_state = RECOVERY_FAILED;
          goto exit;
        }
        if (recovery_context.buffer_handle->data_length == 0) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Header with no payload - sending to core", __LINE__);
          recovery_state = RECOVERY_FOUND_HEADER_NO_PAYLOAD;
          sli_cpc_push_back_driver_buffer_handle(&rx_pending_list_head, recovery_context.buffer_handle);
          sli_cpc_notify_rx_data_from_drv(driver_instance);
          recovery_context.buffer_handle = NULL;
        } else if (recovery_context.buffer_handle->data_length > SLI_CPC_DRV_UART_RX_DATA_MAX_LENGTH) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Payload too large - notifying core error", __LINE__);
          notify_core_error(recovery_context.buffer_handle, SL_CPC_REJECT_ERROR);
        } else {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Header with valid payload size - expecting payload next", __LINE__);
          recovery_state = RECOVERY_FOUND_HEADER;
          header_expected_next = false;
        }
        header_found = false;
      } else {
        SL_CPC_RECOVERY_TRACE("[RECOVERY] No header found in this cycle", __LINE__);
      }
    } else {
      SL_CPC_RECOVERY_TRACE("[RECOVERY] Expecting payload next - processing payload recovery", __LINE__);
      //===========================================================================
      //
      //  Driver is in sync. Attempt to recover a payload that has spilled
      //
      //===========================================================================
      if (recovery_context.buffer_handle == NULL) {
        SL_CPC_RECOVERY_TRACE("[RECOVERY] Buffer handle is NULL - recovery failed", __LINE__);
        SLI_CPC_ASSERT(0);
        recovery_state = RECOVERY_FAILED;
        goto exit;
      }

      if (recovery_context.out_of_sync_extra_bytes_len >= recovery_context.buffer_handle->data_length) {
        SL_CPC_RECOVERY_TRACE("[RECOVERY] Payload can be recovered from out_of_sync_extra_bytes", __LINE__);
        //===========================================================================
        //
        //  There was recently a resync and the sliding resync window had enough
        //  extra bytes to populate a payload
        //
        //===========================================================================
        status = sli_cpc_get_raw_rx_buffer(driver_instance, &rx_buffer_ptr);
        if (status == SL_STATUS_OK) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Got raw buffer - copying payload from extra bytes", __LINE__);
          memcpy(rx_buffer_ptr, recovery_context.out_of_sync_extra_bytes,
                 recovery_context.buffer_handle->data_length);
          memmove(recovery_context.out_of_sync_extra_bytes,
                  recovery_context.out_of_sync_extra_bytes + recovery_context.buffer_handle->data_length,
                  recovery_context.out_of_sync_extra_bytes_len - recovery_context.buffer_handle->data_length);

          recovery_context.out_of_sync_extra_bytes_len -= recovery_context.buffer_handle->data_length;

          recovery_context.buffer_handle->data = rx_buffer_ptr;
          recovery_state = RECOVERY_FOUND_PAYLOAD;
          recovery_context.offset += recovery_context.buffer_handle->data_length;
          sli_cpc_push_back_driver_buffer_handle(&rx_pending_list_head, recovery_context.buffer_handle);
          sli_cpc_notify_rx_data_from_drv(driver_instance);
          recovery_context.buffer_handle = NULL;
          header_expected_next = true;
        } else {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Failed to get raw buffer - recovery failed", __LINE__);
          recovery_state = RECOVERY_FAILED;
          goto exit;
        }
      } else if (misaligned_payload) {
        SL_CPC_RECOVERY_TRACE("[RECOVERY] Processing misaligned payload", __LINE__);
        //===========================================================================
        //
        //  A misaligned_payload is when a partial payload was previously recovered
        //  but a DMA IRQ was required to receive the second part
        //
        //                         Resize               ISR
        //                          ^                   ^
        //                          |                   |
        //       +---------+--------+-------------------+
        //       | Header  |    P1  |         P2        |
        //       +---------+--------+-------------------+
        //                       Payload (part 1 & 2)
        //
        //===========================================================================
        if (recovery_context.buffer_handle->data_length <= recovery_context.out_of_sync_extra_bytes_len) {
          SLI_CPC_ASSERT(0);
          goto exit;
        }

        memmove(recovery_context.spilled_buffer + recovery_context.out_of_sync_extra_bytes_len,
                recovery_context.spilled_buffer + recovery_context.offset,
                recovery_context.buffer_handle->data_length - recovery_context.out_of_sync_extra_bytes_len);

        memcpy(recovery_context.spilled_buffer,
               recovery_context.out_of_sync_extra_bytes,
               recovery_context.out_of_sync_extra_bytes_len);
        recovery_context.out_of_sync_extra_bytes_len = 0;

        recovery_context.buffer_handle->data = recovery_context.spilled_buffer;
        recovery_context.spilled_buffer = recovery_context.active_buffer;
        recovery_state = RECOVERY_FOUND_PAYLOAD;
        sli_cpc_push_back_driver_buffer_handle(&rx_pending_list_head, recovery_context.buffer_handle);
        sli_cpc_notify_rx_data_from_drv(driver_instance);
        recovery_context.buffer_handle = NULL;
        header_expected_next = true;
        recovery_context.offset = 0; // Offset is zero because we just came back from an ISR
      } else {
        SL_CPC_RECOVERY_TRACE("[RECOVERY] Checking if payload is completely in spill buffer", __LINE__);
        //===========================================================================
        //
        //  Check if the payload is completely in the spill buffer
        //
        //===========================================================================
        if (get_already_received_cnt(&already_recvd_cnt) != SL_STATUS_OK) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] get_already_received_cnt failed - recovery failed", __LINE__);
          recovery_state = RECOVERY_FAILED;
          goto exit;
        }

        if (already_recvd_cnt + recovery_context.out_of_sync_extra_bytes_len >= recovery_context.buffer_handle->data_length + recovery_context.offset) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Payload completely available - getting raw buffer", __LINE__);
          status = sli_cpc_get_raw_rx_buffer(driver_instance, &rx_buffer_ptr);
          if (status == SL_STATUS_OK) {
            SL_CPC_RECOVERY_TRACE("[RECOVERY] Got raw buffer - copying complete payload", __LINE__);
            // When we did a resync we probably had a few bytes extra in the sliding window.
            // These bytes are part of the payload.
            if (recovery_context.out_of_sync_extra_bytes_len > 0) {
              SL_CPC_RECOVERY_TRACE("[RECOVERY] Copying payload from extra bytes + spill buffer", __LINE__);
              // Bounds check to prevent integer underflow
              if (recovery_context.buffer_handle->data_length < recovery_context.out_of_sync_extra_bytes_len) {
                SLI_CPC_ASSERT(0); // Data length underflow - should not happen
                goto exit;
              }
              memcpy(rx_buffer_ptr,
                     recovery_context.out_of_sync_extra_bytes,
                     recovery_context.out_of_sync_extra_bytes_len);

              memcpy((uint8_t*)rx_buffer_ptr + recovery_context.out_of_sync_extra_bytes_len,
                     recovery_context.spilled_buffer + recovery_context.offset,
                     recovery_context.buffer_handle->data_length - recovery_context.out_of_sync_extra_bytes_len);
              recovery_context.out_of_sync_extra_bytes_len = 0;
            } else {
              SL_CPC_RECOVERY_TRACE("[RECOVERY] Copying payload from spill buffer only", __LINE__);
              memcpy(rx_buffer_ptr, recovery_context.spilled_buffer + recovery_context.offset, recovery_context.buffer_handle->data_length);
            }
            recovery_context.buffer_handle->data = rx_buffer_ptr;
            recovery_state = RECOVERY_FOUND_PAYLOAD;
            recovery_context.offset += recovery_context.buffer_handle->data_length;
            sli_cpc_push_back_driver_buffer_handle(&rx_pending_list_head, recovery_context.buffer_handle);
            sli_cpc_notify_rx_data_from_drv(driver_instance);
            recovery_context.buffer_handle = NULL;
            header_expected_next = true;
          } else {
            SL_CPC_RECOVERY_TRACE("[RECOVERY] Failed to get raw buffer for payload - recovery failed", __LINE__);
            recovery_state = RECOVERY_FAILED;
            goto exit;
          }
        } else {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Payload not completely available yet", __LINE__);
        }
      }
    }

    if (recovery_state == RECOVERY_FOUND_HEADER) {
      SL_CPC_RECOVERY_TRACE("[RECOVERY] Processing RECOVERY_FOUND_HEADER state", __LINE__);
      if (header_expected_next != false) {
        SLI_CPC_ASSERT(0);
        goto exit;
      }
      if (recovery_context.buffer_handle == NULL) {
        SLI_CPC_ASSERT(0);
        goto exit;
      }

      if (recovery_context.buffer_handle->data_length + recovery_context.offset > SL_CPC_DRV_UART_RX_MAX_BUFFER_SIZE) {
        SL_CPC_RECOVERY_TRACE("[RECOVERY] Buffer size exceeded - recovery failed", __LINE__);
        recovery_state = RECOVERY_FAILED;
        goto exit;
      } else {
        SL_CPC_RECOVERY_TRACE("[RECOVERY] Buffer size OK - checking payload recovery options", __LINE__);
        if (recovery_context.buffer_handle->data_length <= recovery_context.out_of_sync_extra_bytes_len) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Payload can be recovered from out_of_sync_extra_bytes", __LINE__);
          // We will recover the payload from the out of sync extra bytes
          spilled_data_fully_recovered = false;
        } else {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Need to resize descriptor for payload", __LINE__);
          uint32_t temp_required_size = recovery_context.offset + recovery_context.buffer_handle->data_length;
          if (temp_required_size > UINT16_MAX) {
            SLI_CPC_ASSERT(0);
            recovery_state = RECOVERY_FAILED;
            goto exit;
          }
          uint16_t required_size = (uint16_t)temp_required_size;
          if (required_size < recovery_context.out_of_sync_extra_bytes_len) {
            // Underflow would occur
            SLI_CPC_ASSERT(0);
            recovery_state = RECOVERY_FAILED;
            goto exit;
          }
          status = resize_current_dma_descriptor(required_size - recovery_context.out_of_sync_extra_bytes_len);
          if (status == SL_STATUS_OK) {
            SL_CPC_RECOVERY_TRACE("[RECOVERY] Resize successful - setting misaligned_payload flag", __LINE__);
            recovery_context.misaligned_payload = true; // We will need to re-align the payload outside of the ISR context
          } else if (status == SL_STATUS_ALREADY_EXISTS) {
            SL_CPC_RECOVERY_TRACE("[RECOVERY] Resize returned ALREADY_EXISTS - data not fully recovered", __LINE__);
            spilled_data_fully_recovered = false;
          } else {
            SL_CPC_RECOVERY_TRACE("[RECOVERY] Resize failed with other status", __LINE__);
          }
        }
      }
    } else {
      SL_CPC_RECOVERY_TRACE("[RECOVERY] Not in RECOVERY_FOUND_HEADER state", __LINE__);
    }

    if (recovery_state == RECOVERY_FOUND_PAYLOAD || recovery_state == RECOVERY_FOUND_HEADER_NO_PAYLOAD) {
      SL_CPC_RECOVERY_TRACE("[RECOVERY] Processing RECOVERY_FOUND_PAYLOAD or RECOVERY_FOUND_HEADER_NO_PAYLOAD state", __LINE__);
      if ((uint16_t)(SLI_CPC_HDLC_HEADER_RAW_SIZE + recovery_context.offset) > SL_CPC_DRV_UART_RX_MAX_BUFFER_SIZE) {
        SL_CPC_RECOVERY_TRACE("[RECOVERY] Header + offset exceeds buffer size - recovery failed", __LINE__);
        recovery_state = RECOVERY_FAILED;
        goto exit;
      } else {
        SL_CPC_RECOVERY_TRACE("[RECOVERY] Size OK - resizing descriptor for next header", __LINE__);
        if (recovery_context.offset + SLI_CPC_HDLC_HEADER_RAW_SIZE > UINT16_MAX) {
          SLI_CPC_ASSERT(0);
          recovery_state = RECOVERY_FAILED;
          goto exit;
        }
        status = resize_current_dma_descriptor((uint16_t)(recovery_context.offset + SLI_CPC_HDLC_HEADER_RAW_SIZE));
        if (status == SL_STATUS_OK) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Resize successful - recovery completed", __LINE__);
          recovery_context.spilled_buffer = NULL; // Will be reused by the ISR
          recovery_context.recovery_completed = true;
          recovery_context.out_of_sync_extra_bytes_len = 0;
        } else if (status == SL_STATUS_ALREADY_EXISTS) {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Resize returned ALREADY_EXISTS - data not fully recovered", __LINE__);
          spilled_data_fully_recovered = false;
        } else {
          SL_CPC_RECOVERY_TRACE("[RECOVERY] Resize failed with other status", __LINE__);
        }
      }
    } else {
      SL_CPC_RECOVERY_TRACE("[RECOVERY] Not in payload/no-payload state", __LINE__);
    }
  } while (spilled_data_fully_recovered == false);

  SL_CPC_RECOVERY_TRACE("[RECOVERY] Exited main recovery loop", __LINE__);

  exit:
  SL_CPC_RECOVERY_TRACE("[RECOVERY] At exit label", __LINE__);

  if (recovery_state == RECOVERY_FAILED) {
    SL_CPC_RECOVERY_TRACE("[RECOVERY] Recovery failed - restarting DMA", __LINE__);
    if (recovery_context.buffer_handle != NULL) {
      SL_CPC_RECOVERY_TRACE("[RECOVERY] Recovery failed - pushing back buffer handle at exit", __LINE__);
      push_free_rx_buffer_handle(recovery_context.buffer_handle);
    } else {
      SL_CPC_RECOVERY_TRACE("[RECOVERY] Recovery failed - no buffer handle to push back at exit", __LINE__);
    }
    restart_dma(); // Will clear the recovery context
  } else {
    SL_CPC_RECOVERY_TRACE("[RECOVERY] Recovery completed successfully", __LINE__);
  }
  SL_CPC_RECOVERY_TRACE("[RECOVERY] Recovery function exit", __LINE__);
  MCU_EXIT_ATOMIC();
}
#endif

#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITH_HWFC)
static bool rx_dma_complete_hwfc(unsigned int channel,
                                 unsigned int sequenceNo,
                                 void *userParam)
{
  MCU_DECLARE_IRQ_STATE;

  static sl_cpc_buffer_handle_t *active_dma_rx_buffer_handle = NULL;
  cpc_ldma_descriptor_t *completed_desc = NULL;
  uint8_t *rx_buffer = NULL;
  uint8_t *active_rx_buffer = NULL;
  bool notify_core = false;
  bool found_header = false;
  uint16_t header_flag_position = 0;
  static bool out_of_sync = false;
  static bool was_out_of_sync = false;
  static uint8_t out_of_sync_extra_bytes = 0;
  static uint8_t hdlc_header_sliding_window[SLI_CPC_HDLC_HEADER_RAW_SIZE * 2];

  (void)channel;
  (void)sequenceNo;
  (void)userParam;

  MCU_ENTER_ATOMIC();
  completed_desc = rx_descriptor_head;
  SLI_CPC_ASSERT(completed_desc != NULL);

  if (was_out_of_sync) {
    active_rx_buffer = (uint8_t *)(completed_desc->xfer.CPC_LDMA_DESCRIPTOR_DST_ADDR) - out_of_sync_extra_bytes;
    out_of_sync_extra_bytes = 0;
    was_out_of_sync = false;
  } else {
    active_rx_buffer = (uint8_t *)(completed_desc->xfer.CPC_LDMA_DESCRIPTOR_DST_ADDR);
  }

  //===========================================================================
  //
  //  Resync
  //
  //===========================================================================
  if (out_of_sync) {
    // Update the sliding window
    memcpy(hdlc_header_sliding_window + SLI_CPC_HDLC_HEADER_RAW_SIZE, active_rx_buffer, SLI_CPC_HDLC_HEADER_RAW_SIZE);

    // Look for a header, no need to check the first byte or else we would have resynced on the last attempt
    found_header = find_valid_header(hdlc_header_sliding_window + 1, (SLI_CPC_HDLC_HEADER_RAW_SIZE * 2) - 1, &header_flag_position);
    if (!found_header) {
      // No header found. Slide the window, prime the DMA for another header.
      memcpy(hdlc_header_sliding_window, active_rx_buffer, SLI_CPC_HDLC_HEADER_RAW_SIZE);

      sli_cpc_free_raw_rx_buffer(driver_instance, active_rx_buffer);

      // Get more bytes to populate the sliding window used to resync
      resize_current_dma_descriptor(NULL, 0, SLI_CPC_HDLC_HEADER_RAW_SIZE);

      MCU_EXIT_ATOMIC();
      return false;
    }

    header_flag_position += 1; // We skipped the first byte

    // Reconstruct the valid header
    memcpy(active_rx_buffer, hdlc_header_sliding_window + header_flag_position, SLI_CPC_HDLC_HEADER_RAW_SIZE);

    // Resync completed
    out_of_sync = false;
    was_out_of_sync = true;
  }

  //===========================================================================
  //
  //  Process HDLC header
  //
  //===========================================================================
  if (header_expected_next) {
    SLI_CPC_ASSERT(out_of_sync == false);
    active_dma_rx_buffer_handle = pop_free_rx_buffer_handle();

    if (active_dma_rx_buffer_handle == NULL) {
      // We couldn't attach the current RX buffer to a buffer handle. Free it now,
      // or lose it forever!
      sli_cpc_free_raw_rx_buffer(driver_instance, active_rx_buffer);
      MCU_EXIT_ATOMIC();
      return false;
    }

    // Validate HCS
    if (!was_out_of_sync && !find_valid_header(active_rx_buffer, SLI_CPC_HDLC_HEADER_RAW_SIZE, NULL)) {
      SLI_CPC_ASSERT(!was_out_of_sync); // If we just resynced, the header should be valid

      // Remember this invalid header when we will attempt a resync
      memcpy(hdlc_header_sliding_window, active_rx_buffer, SLI_CPC_HDLC_HEADER_RAW_SIZE);
      sli_cpc_free_raw_rx_buffer(driver_instance, active_rx_buffer);
      push_free_rx_buffer_handle(active_dma_rx_buffer_handle);
      out_of_sync = true;

      // Get more bytes to populate the sliding window used to resync
      resize_current_dma_descriptor(NULL, 0, SLI_CPC_HDLC_HEADER_RAW_SIZE);

      MCU_EXIT_ATOMIC();
      return false;
    }

    active_dma_rx_buffer_handle->data_length = sli_cpc_hdlc_get_length(active_rx_buffer);

    // Copy useful fields of header. Unfortunately with this method the header must always be copied
    memcpy(active_dma_rx_buffer_handle->hdlc_header, active_rx_buffer, SLI_CPC_HDLC_HEADER_RAW_SIZE);

    // Free the unused buffer
    sli_cpc_free_raw_rx_buffer(driver_instance, active_rx_buffer);

    if (active_dma_rx_buffer_handle->data_length == 0) {
      notify_core = true;
      header_expected_next = true;
      next_rx_size = SLI_CPC_HDLC_HEADER_RAW_SIZE;
    } else if (next_rx_size > SLI_CPC_DRV_UART_RX_DATA_MAX_LENGTH) {
      SLI_CPC_DEBUG_TRACE_CORE_DRIVER_PACKET_DROPPED(driver_instance);
      notify_core_error(active_dma_rx_buffer_handle, SL_CPC_REJECT_ERROR);
      header_expected_next = true;
      active_dma_rx_buffer_handle = NULL;
      next_rx_size = SLI_CPC_HDLC_HEADER_RAW_SIZE;
    } else {
      header_expected_next = false;
      next_rx_size = active_dma_rx_buffer_handle->data_length;
    }

    if (was_out_of_sync) {
      // The resync sliding window might contain data that belongs to another header or another payload
      // Prepare a buffer with the extra bytes already pre-loaded
      sl_status_t status = sli_cpc_get_raw_rx_buffer(driver_instance, &rx_buffer);

      if (status == SL_STATUS_OK) {
        // Calculate how many bytes of the next payload are already in the header resync window
        out_of_sync_extra_bytes = (2 * SLI_CPC_HDLC_HEADER_RAW_SIZE) - (header_flag_position + SLI_CPC_HDLC_HEADER_RAW_SIZE);

        if (out_of_sync_extra_bytes >= next_rx_size) {
          SLI_CPC_ASSERT(active_dma_rx_buffer_handle->data_length > 0);

          // We already have all the bytes that we need to process a payload, no need to resize the DMA
          out_of_sync_extra_bytes -= next_rx_size;

          // Populate the payload buffers with the contents of the resync sliding window
          memcpy(rx_buffer, hdlc_header_sliding_window + SLI_CPC_HDLC_HEADER_RAW_SIZE + header_flag_position, next_rx_size);

          // Add to current rx handle
          SLI_CPC_ASSERT(active_dma_rx_buffer_handle != NULL);
          notify_core = true;
          active_dma_rx_buffer_handle->data = rx_buffer;

          if (out_of_sync_extra_bytes > 0) {
            // There are still bytes available in the resync window
            // Prepare a buffer with the remaining bytes to prefix the next header
            sl_status_t status = sli_cpc_get_raw_rx_buffer(driver_instance, &rx_buffer);

            if (status == SL_STATUS_OK) {
              memcpy(rx_buffer, hdlc_header_sliding_window + SLI_CPC_HDLC_HEADER_RAW_SIZE + header_flag_position + next_rx_size, out_of_sync_extra_bytes);
              next_rx_size = SLI_CPC_HDLC_HEADER_RAW_SIZE;
              resize_current_dma_descriptor(rx_buffer, out_of_sync_extra_bytes, SLI_CPC_HDLC_HEADER_RAW_SIZE);
            } else {
              // Drop the extra bytes and wait for a new buffer
              SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] stopping DMA", __LINE__);
              DMADRV_StopTransfer(read_channel);
              out_of_sync_extra_bytes = 0;
              next_rx_size = SLI_CPC_HDLC_HEADER_RAW_SIZE;
              need_rx_buffer = true;
              SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] need_rx_buffer = true", __LINE__);
              MCU_EXIT_ATOMIC();
              return false;
            }
          } else {
            next_rx_size = SLI_CPC_HDLC_HEADER_RAW_SIZE;
            resize_current_dma_descriptor(NULL, 0, SLI_CPC_HDLC_HEADER_RAW_SIZE);
          }

          // A header is always expected after a payload
          header_expected_next = true;
        } else {
          // The contents of the next payload are not fully in the resync
          // window. We need to resize the DMA to generate an interrupt on
          // the rest of the bytes.
          memcpy(rx_buffer, hdlc_header_sliding_window + SLI_CPC_HDLC_HEADER_RAW_SIZE + header_flag_position, out_of_sync_extra_bytes);
          resize_current_dma_descriptor(rx_buffer, out_of_sync_extra_bytes, next_rx_size);
        }
      } else {
        // Could not allocate a buffer for the next payload
        SLI_CPC_DEBUG_TRACE_CORE_DRIVER_ERROR(driver_instance);
        SLI_CPC_ASSERT(0);
        SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] stopping DMA", __LINE__);
        DMADRV_StopTransfer(read_channel);
        out_of_sync_extra_bytes = 0;
        next_rx_size = SLI_CPC_HDLC_HEADER_RAW_SIZE;
        need_rx_buffer = true;
        SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] need_rx_buffer = true", __LINE__);
        MCU_EXIT_ATOMIC();
        return false;
      }
    } else {
      resize_current_dma_descriptor(NULL, 0, next_rx_size);
    }

    //===========================================================================
    //
    //  Process payload
    //
    //===========================================================================
  } else {
    // Add to current rx handle
    SLI_CPC_ASSERT(active_dma_rx_buffer_handle != NULL);
    for (uint16_t i = 0; i < active_dma_rx_buffer_handle->data_length; ++i) {
    }
    active_dma_rx_buffer_handle->data = active_rx_buffer;

    next_rx_size = SLI_CPC_HDLC_HEADER_RAW_SIZE;
    resize_current_dma_descriptor(NULL, 0, SLI_CPC_HDLC_HEADER_RAW_SIZE);

    notify_core = true;

    // A header is always expected after a payload
    header_expected_next = true;
  }

  if (notify_core) {
    // Push rx_entry to pending list
    sli_cpc_push_back_driver_buffer_handle(&rx_pending_list_head, active_dma_rx_buffer_handle);

    // Notify core
    sli_cpc_notify_rx_data_from_drv(driver_instance);
  }

  MCU_EXIT_ATOMIC();
  return false;
}
#endif

#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITHOUT_HWFC)
static bool rx_dma_complete_no_hwfc(unsigned int channel,
                                    unsigned int sequenceNo,
                                    void *userParam)
{
  MCU_DECLARE_IRQ_STATE;

  static sl_cpc_buffer_handle_t *active_dma_rx_buffer_handle = NULL;
  sl_status_t update_status = SL_STATUS_FAIL;
  cpc_ldma_descriptor_t *completed_desc = NULL;
  uint8_t *active_rx_buffer = NULL;
  uint8_t *rx_data = NULL;
  uint8_t *spill_buffer;
  bool notify_core = false;
  bool failed_to_allocate = false;

  (void)channel;
  (void)sequenceNo;
  (void)userParam;

  MCU_ENTER_ATOMIC();
  SLI_CPC_ASSERT(recovery_dispatcher_handle.submitted == false);

  completed_desc = rx_descriptor_head;
  SLI_CPC_ASSERT(completed_desc != NULL);

  // Restore descriptor transfer count to its maximum value
  completed_desc->xfer.CPC_LDMA_DESCRIPTOR_XFER_CNT = SL_MIN(SLI_CPC_DRV_UART_RX_FRAME_MAX_LENGTH,
                                                             CPC_LDMA_DESCRIPTOR_MAX_XFER_SIZE) - 1;

  active_rx_buffer = (uint8_t *)(completed_desc->xfer.CPC_LDMA_DESCRIPTOR_DST_ADDR);
  rx_data = active_rx_buffer;

  if (recovery_context.recovery_completed || recovery_context.out_of_sync) {
    SLI_CPC_ASSERT(header_expected_next);
    rx_data += recovery_context.offset;
    next_rx_size = SLI_CPC_HDLC_HEADER_RAW_SIZE;
  }

  // Every time a RX complete ISR occurs, the LDMA automatically loads the linkAddr
  // register. On our side we need to sync with it by swapping the descriptor
  // so that our rx_descriptor_head matches the descriptor currently being used
  // The previously active descriptor now becomes the spill descriptor and vice versa
  swap_descriptors();
  spill_buffer = (uint8_t *)(rx_descriptor_head->xfer.CPC_LDMA_DESCRIPTOR_DST_ADDR);

  uint8_t *new_buffer;
  sl_status_t status = sli_cpc_get_raw_rx_buffer(driver_instance, &new_buffer);
  if (status == SL_STATUS_OK) {
    completed_desc->xfer.CPC_LDMA_DESCRIPTOR_DST_ADDR = (uint32_t)new_buffer;

    //===========================================================================
    //
    //  Process misaligned payload
    //
    //===========================================================================
    if (recovery_context.misaligned_payload) {
      // We need to re-align the entry outside of the ISR context
      recovery_context.spilled_buffer = active_rx_buffer;
      recovery_context.active_buffer = spill_buffer;
      dispatch_recovery();
      MCU_EXIT_ATOMIC();
      return false;
    }
  } else {
    // Allocation failed, try to recycle the buffer if we are not expecting a header next
    failed_to_allocate = true;
    if (!header_expected_next) {
      // Just got a payload, handle the RX buffer starvation by recycling the buffer.
      // Extract the HDLC header for and remove any associated payload.

      if (recovery_context.misaligned_payload) {
        SLI_CPC_ASSERT(recovery_context.buffer_handle != NULL);
        active_dma_rx_buffer_handle = recovery_context.buffer_handle;
        memset(&recovery_context, 0x00, sizeof(recovery_context));
      }
      SLI_CPC_ASSERT(active_dma_rx_buffer_handle != NULL);

      SL_CPC_JOURNAL_RECORD_INFO("[DRV] RX Buffer starvation - Dropping payload", 0);
      active_dma_rx_buffer_handle->data_length = 0;
      active_dma_rx_buffer_handle->data = NULL;

      sli_cpc_push_back_driver_buffer_handle(&rx_pending_list_head, active_dma_rx_buffer_handle);
      sli_cpc_notify_rx_data_from_drv(driver_instance);
      active_dma_rx_buffer_handle = NULL;

      // Recycle the RX buffer right away
      completed_desc->xfer.CPC_LDMA_DESCRIPTOR_DST_ADDR = (uint32_t)active_rx_buffer;

      header_expected_next = true;
      update_status = resize_current_dma_descriptor(SLI_CPC_HDLC_HEADER_RAW_SIZE);
      if (update_status == SL_STATUS_ALREADY_EXISTS) {
        // Next header is already in the spill buffer, try to recover it
        recovery_context.spilled_buffer = spill_buffer;
        recovery_context.buffer_handle = NULL; // Allocate an entry during recovery
        recovery_context.offset = 0;
        dispatch_recovery();
      }

      MCU_EXIT_ATOMIC();
      return false;
    }
  }

  //===========================================================================
  //
  //  Process HDLC header
  //
  //===========================================================================
  if (header_expected_next) {
    SLI_CPC_ASSERT(active_dma_rx_buffer_handle == NULL);
    active_dma_rx_buffer_handle = pop_free_rx_buffer_handle();
    if (active_dma_rx_buffer_handle == NULL) {
      sli_cpc_free_raw_rx_buffer(driver_instance, active_rx_buffer);
      recovery_context.recovery_completed = false;
      MCU_EXIT_ATOMIC();
      return false;
    }

    // Copy useful fields of header
    memcpy(active_dma_rx_buffer_handle->hdlc_header, rx_data, SLI_CPC_HDLC_HEADER_RAW_SIZE);

    // We freed the buffer used to capture the HDLC header, reclaim it if we previously failed to allocate one
    if (failed_to_allocate) {
      completed_desc->xfer.CPC_LDMA_DESCRIPTOR_DST_ADDR = (uint32_t)active_rx_buffer;
      failed_to_allocate = false;
    } else {
      // Can free the buffer right away
      sli_cpc_free_raw_rx_buffer(driver_instance, active_rx_buffer);
    }

    if (!find_valid_header(active_dma_rx_buffer_handle->hdlc_header, SLI_CPC_HDLC_HEADER_RAW_SIZE, NULL)) {
      memcpy(recovery_context.invalid_hdlc_header, active_dma_rx_buffer_handle->hdlc_header, SLI_CPC_HDLC_HEADER_RAW_SIZE);

      push_free_rx_buffer_handle(active_dma_rx_buffer_handle);
      active_dma_rx_buffer_handle = NULL;
      recovery_context.buffer_handle = NULL;
      recovery_context.recovery_completed = false;
      recovery_context.out_of_sync = true;
      recovery_context.spilled_buffer = spill_buffer;
      recovery_context.offset = 0;
      dispatch_recovery();

      MCU_EXIT_ATOMIC();
      return false;
    }

    recovery_context.out_of_sync = false;

    active_dma_rx_buffer_handle->data_length = sli_cpc_hdlc_get_length(active_dma_rx_buffer_handle->hdlc_header);

    // Prepare the size of the next transfer, according to the payload field of the HDLC header
    if (active_dma_rx_buffer_handle->data_length == 0) {
      header_expected_next = true;
      notify_core = true;
      next_rx_size = SLI_CPC_HDLC_HEADER_RAW_SIZE;
    } else if (active_dma_rx_buffer_handle->data_length > SLI_CPC_DRV_UART_RX_DATA_MAX_LENGTH) {
      SLI_CPC_DEBUG_TRACE_CORE_DRIVER_PACKET_DROPPED(driver_instance);
      notify_core_error(active_dma_rx_buffer_handle, SL_CPC_REJECT_ERROR);
      active_dma_rx_buffer_handle = NULL;
      header_expected_next = true;
      next_rx_size = SLI_CPC_HDLC_HEADER_RAW_SIZE;
    } else {
      header_expected_next = false;
      next_rx_size = active_dma_rx_buffer_handle->data_length;
    }

    // Attempt a resize of the currently active descriptor to generate an interrution on completion
    update_status = resize_current_dma_descriptor(next_rx_size);
    if (update_status == SL_STATUS_ALREADY_EXISTS) {
      recovery_context.spilled_buffer = spill_buffer;
      recovery_context.offset = 0;

      if (notify_core) {
        // Push rx_entry to pending list
        sli_cpc_push_back_driver_buffer_handle(&rx_pending_list_head, active_dma_rx_buffer_handle);

        // Notify core
        sli_cpc_notify_rx_data_from_drv(driver_instance);
        recovery_context.buffer_handle = NULL;
      } else {
        if (header_expected_next) {
          push_free_rx_buffer_handle(active_dma_rx_buffer_handle);
          recovery_context.buffer_handle = NULL;
        } else {
          recovery_context.buffer_handle = active_dma_rx_buffer_handle;
        }
      }
      active_dma_rx_buffer_handle = NULL;
      dispatch_recovery();

      recovery_context.recovery_completed = false;
      MCU_EXIT_ATOMIC();
      return false;
    }
    //===========================================================================
    //
    //  Process payload
    //
    //===========================================================================
  } else {
    // Add to current rx handle
    SLI_CPC_ASSERT(active_dma_rx_buffer_handle != NULL);

    active_dma_rx_buffer_handle->data = rx_data;
    notify_core = true;

    // A header is always expected after a payload
    header_expected_next = true;
    next_rx_size = SLI_CPC_HDLC_HEADER_RAW_SIZE;
    update_status = resize_current_dma_descriptor(SLI_CPC_HDLC_HEADER_RAW_SIZE);
    if (update_status == SL_STATUS_ALREADY_EXISTS) {
      header_expected_next = true;

      recovery_context.spilled_buffer = spill_buffer;
      recovery_context.buffer_handle = NULL; // Allocate an entry during recovery, because this one will be used for the payload
      recovery_context.offset = 0;

      dispatch_recovery();
    }
  }

  if (notify_core) {
    // Push rx_entry to pending list
    sli_cpc_push_back_driver_buffer_handle(&rx_pending_list_head, active_dma_rx_buffer_handle);

    // Notify core
    sli_cpc_notify_rx_data_from_drv(driver_instance);
    active_dma_rx_buffer_handle = NULL;
  }

  recovery_context.recovery_completed = false;
  MCU_EXIT_ATOMIC();
  return false;
}

/***************************************************************************/ /**
 * Update dma desccriptor link absolute address.
 *
 * @param current_dma_desc DMA descriptor to update.
 *
 * @param next_dma_desc DMA descriptor to link to.
 ******************************************************************************/
static void update_dma_desc_link_abs(cpc_ldma_descriptor_t *current_dma_desc,
                                     cpc_ldma_descriptor_t *next_dma_desc,
                                     bool enable_link)
{
  current_dma_desc->sync.CPC_LDMA_DESCRIPTOR_LINK_ADDR = CPC_LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(next_dma_desc);
  current_dma_desc->sync.CPC_LDMA_DESCRIPTOR_LINK_MODE = CPC_LDMA_DESCRIPTOR_LINK_MODE_ABS;
  current_dma_desc->sync.link = enable_link ? 1 : 0;
}

/***************************************************************************/ /**
 * Update XferCnt of the current DMA xfer with the length passed as argument.
 *
 * @param new_length Total length of next transfer.
 *
 * @return SL_STATUS_OK if new length successfully applied.
 *         SL_STATUS_ALREADY_EXISTS if the DMA already transferred the requested bytes or more.
 ******************************************************************************/
static sl_status_t resize_current_dma_descriptor(uint16_t new_length)
{
  MCU_DECLARE_IRQ_STATE;

  MCU_ENTER_ATOMIC();
  uint32_t chMask = 1UL << (uint8_t)read_channel;
  uint32_t ctrl;

  if (new_length > CPC_LDMA_DESCRIPTOR_MAX_XFER_SIZE) {
    SLI_CPC_ASSERT(0);
  }

  uint16_t already_recvd_cnt;
  uint16_t remaining;

  // Adjust current dma xfer with new_length
  // For safety we momentarily pause the DMA. If we are afraid of suspending
  // it for too long and loosing bytes, we could use a critical section to make
  // sure there are not higher priority interrupt until we resume it.
  DMADRV_PauseTransfer(read_channel);

  ctrl = LDMA_PERIPH->CH[read_channel].CTRL;
  if (get_already_received_cnt(&already_recvd_cnt) != SL_STATUS_OK) {
    SLI_CPC_ASSERT(0);
    DMADRV_ResumeTransfer(read_channel);
    MCU_EXIT_ATOMIC();
    return SL_STATUS_FAIL;
  }

  if (already_recvd_cnt >= new_length) {
    DMADRV_ResumeTransfer(read_channel);
    MCU_EXIT_ATOMIC();
    return SL_STATUS_ALREADY_EXISTS;
  }

  remaining = new_length - already_recvd_cnt;

  // Update xfercnt
  ctrl &= ~_LDMA_CH_CTRL_XFERCNT_MASK;
  ctrl |= ((remaining - 1) << _LDMA_CH_CTRL_XFERCNT_SHIFT) & _LDMA_CH_CTRL_XFERCNT_MASK;
  LDMA_PERIPH->CH[read_channel].CTRL = ctrl;

  LDMA_PERIPH->CH[read_channel].LINK |= LDMA_CH_LINK_LINK;

  // Enable interrupts
#ifdef _LDMA_CH_CTRL_DONEIEN_MASK
  LDMA_PERIPH->CH[read_channel].CTRL |= LDMA_CH_CTRL_DONEIEN;
#else
  LDMA_PERIPH->CH[read_channel].CTRL |= LDMA_CH_CTRL_DONEIFSEN;
#endif

  cpc_ldma_int_enable(chMask);

  // Disable linking from the next descriptor
  cpc_ldma_descriptor_t *next = (cpc_ldma_descriptor_t *)(LDMA_PERIPH->CH[read_channel].LINK & _LDMA_CH_LINK_LINKADDR_MASK);
  next->xfer.link = 0;
  next_rx_size = new_length;

  DMADRV_ResumeTransfer(read_channel);

  SLI_CPC_ASSERT(already_recvd_cnt + remaining <= SL_CPC_DRV_UART_RX_MAX_BUFFER_SIZE);

  MCU_EXIT_ATOMIC();
  return SL_STATUS_OK;
}
#else
static void resize_current_dma_descriptor(void *pre_loaded_rx_buffer, uint16_t offset, uint16_t new_length)
{
  Ecode_t ecode;
  uint8_t *buffer_ptr;
  sl_status_t status;

  SLI_CPC_ASSERT(new_length > offset);
  next_rx_size = (new_length - offset);

  if (pre_loaded_rx_buffer == NULL) {
    status = sli_cpc_get_raw_rx_buffer(driver_instance, &buffer_ptr);
    if (status != SL_STATUS_OK) {
      SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] stopping DMA", __LINE__);
      DMADRV_StopTransfer(read_channel);
      need_rx_buffer = true;
      SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] need_rx_buffer = true", __LINE__);
      return;
    }
  } else {
    buffer_ptr = pre_loaded_rx_buffer;
  }

  if (new_length <= CPC_LDMA_DESCRIPTOR_MAX_XFER_SIZE) {
    rx_descriptor[0u] = (cpc_ldma_descriptor_t)CPC_LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&(SL_CPC_DRV_UART_PERIPHERAL->RXDATA), (uint8_t *)buffer_ptr + offset, new_length - offset);
  } else {
    rx_descriptor[0u] = (cpc_ldma_descriptor_t)CPC_LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(&(SL_CPC_DRV_UART_PERIPHERAL->RXDATA), (uint8_t *)buffer_ptr + offset, CPC_LDMA_DESCRIPTOR_MAX_XFER_SIZE - offset, 1u);
    rx_descriptor[1u] = (cpc_ldma_descriptor_t)CPC_LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&(SL_CPC_DRV_UART_PERIPHERAL->RXDATA), &((uint8_t *)buffer_ptr)[CPC_LDMA_DESCRIPTOR_MAX_XFER_SIZE], (new_length - CPC_LDMA_DESCRIPTOR_MAX_XFER_SIZE));
    rx_descriptor[0u].xfer.CPC_LDMA_DESCRIPTOR_DONE_IFS = 0u;
    rx_descriptor[1u].xfer.CPC_LDMA_DESCRIPTOR_DONE_IFS = 1u;
  }

  // Start read channel
  ecode = DMADRV_LdmaStartTransfer(read_channel,
                                   &rx_config,
                                   rx_descriptor_head,
                                   rx_dma_complete_hwfc,
                                   0);
  SLI_CPC_ASSERT(ecode == ECODE_OK);

  return;
}
#endif

/***************************************************************************/ /**
 * Send a reject notification to CPC core.
 *
 * @param reason Reject reason.
 ******************************************************************************/
static void notify_core_error(sl_cpc_buffer_handle_t *buffer_handle, sl_cpc_reject_reason_t reason)
{
  buffer_handle->reason = reason;

  // Push rx_entry to pending list
  sli_cpc_push_back_driver_buffer_handle(&rx_pending_list_head, buffer_handle);

  // Notify core
  sli_cpc_notify_rx_data_from_drv(driver_instance);
}

/***************************************************************************/ /**
 * Stop DMA mode and start receiving bytes in interrupt mode.
 ******************************************************************************/
static void restart_dma(void)
{
  SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] Restarting DMA", __LINE__);
#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITHOUT_HWFC)
  static bool initial_start = true;
#endif

  MCU_DECLARE_IRQ_STATE;

  MCU_ENTER_ATOMIC();

  DMADRV_StopTransfer(read_channel);

#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITH_HWFC)
  if (need_rx_buffer) {
    MCU_EXIT_ATOMIC();
    return;
  }
#else
  if (!initial_start) {
    SLI_CPC_DEBUG_TRACE_CORE_DRIVER_PACKET_DROPPED(driver_instance);
  }
  initial_start = false;

  SLI_CPC_ASSERT(recovery_dispatcher_handle.submitted == false);
  SLI_CPC_ASSERT(recovery_context.misaligned_payload == false);

  memset(&recovery_context, 0x00, sizeof(recovery_context));
#endif

#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITH_HWFC)
  uint8_t *buffer_ptr;
  sl_status_t status = sli_cpc_get_raw_rx_buffer(driver_instance, &buffer_ptr);
  if (status != SL_STATUS_OK) {
    need_rx_buffer = true;
    SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] need_rx_buffer = true", __LINE__);
    MCU_EXIT_ATOMIC();
    return;
  }

  rx_descriptor[0u] = (cpc_ldma_descriptor_t)CPC_LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&(SL_CPC_DRV_UART_PERIPHERAL->RXDATA), buffer_ptr, next_rx_size);
#endif
  rx_descriptor_head = &rx_descriptor[0];

#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITHOUT_HWFC)
  // Without HWFC a header is always expected next when doing a restart
  // This is because a restart is usually done when we freed a rx_buffer
  // and we don't know where we are in the bytestream sent by the host.
  // i.g. we could be in the middle of a payload
  header_expected_next = true;
  next_rx_size = SLI_CPC_HDLC_HEADER_RAW_SIZE;
  rx_descriptor_head->xfer.CPC_LDMA_DESCRIPTOR_XFER_CNT = SLI_CPC_HDLC_HEADER_RAW_SIZE - 1;
#endif

  Ecode_t ecode = DMADRV_LdmaStartTransfer(read_channel,
                                           &rx_config,
                                           rx_descriptor_head,
#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITH_HWFC)
                                           rx_dma_complete_hwfc,
#else
                                           rx_dma_complete_no_hwfc,
#endif
                                           0);
  SLI_CPC_ASSERT(ecode == ECODE_OK);

#if (SL_CPC_DRV_UART_FLOW_CONTROL_TYPE == WITHOUT_HWFC)
  LDMA_PERIPH->CH[read_channel].LINK |= LDMA_CH_LINK_LINK;

  // Enable interrupts
#ifdef _LDMA_CH_CTRL_DONEIEN_MASK
  LDMA_PERIPH->CH[read_channel].CTRL |= LDMA_CH_CTRL_DONEIEN;
#else
  LDMA_PERIPH->CH[read_channel].CTRL |= LDMA_CH_CTRL_DONEIFSEN;
#endif

  uint32_t chMask = 1UL << (uint8_t)read_channel;
  cpc_ldma_int_enable(chMask);
#endif

  SL_CPC_JOURNAL_RECORD_DEBUG("[DRV] Restarted DMA", __LINE__);
  MCU_EXIT_ATOMIC();
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
