/***************************************************************************//**
 * @file
 * @brief Universal asynchronous receiver/transmitter (EUSART)
 *        driver for the Silicon Labs Bootloader.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "config/btl_config.h"

#include "btl_serial_driver.h"
#include "btl_driver_delay.h"
#include "api/btl_interface.h"
#include "btl_driver_util.h"

#include "em_device.h"
#include "btl_driver_cmu.h"
#include "sl_hal_bus.h"
#include "sl_hal_gpio.h"
#include "sl_hal_ldma.h"
#include <stddef.h>

#include "debug/btl_debug.h"
#ifdef BTL_CONFIG_FILE
#include BTL_CONFIG_FILE
#else
#include "btl_euart_driver_cfg.h"
#endif

#define SL_DRIVER_EUSART_LDMA_RX_CHANNEL     0
#define SL_DRIVER_EUSART_LDMA_TX_CHANNEL     1

// ‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
// Configuration validation
#if SL_SERIAL_EUART_BAUD_RATE < 100
#warning "EUART baud rate is very low, consider increasing speed"
#endif
#if SL_DRIVER_EUART_RX_BUFFER_SIZE < 8
#error "EUART RX buffer too small"
#endif
#if SL_DRIVER_EUART_TX_BUFFER_SIZE < 8
#error "EUART TX buffer too small"
#endif
#if (SL_DRIVER_EUART_RX_BUFFER_SIZE % 2) != 0
#error "EUART RX buffer size is not even"
#endif

// A btl_euart_drv driver instance initialization structure contains peripheral name
// of the euart, euart instance number and cmu_clock_type.
typedef struct {
  EUSART_TypeDef            *port;             // The peripheral used for EUART
  uint8_t                    uartNum;          // EUART instance number.
  CMU_Clock_TypeDef          euartClock;       // Clock source for the euart peripheral
} btl_euart_drv;

btl_euart_drv sl_uart_init_inst = {
  .port       = SL_SERIAL_EUART_PERIPHERAL,
  .uartNum    = SL_SERIAL_EUART_PERIPHERAL_NO,
  .euartClock = cmuClock_EUSART0,
};

// ‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
// Static variables

/// Flag to indicate hardware is up and running
static bool    initialized = false;
/// Receive buffer (forced to even size).
/// LDMA will pingpong between two halves of this buffer.
static uint8_t rxBuffer[SL_DRIVER_EUART_RX_BUFFER_SIZE];
/// Transmit buffer for LDMA use.
static uint8_t txBuffer[SL_DRIVER_EUART_TX_BUFFER_SIZE];
/// Amount of bytes in the current transmit operation
static size_t  txLength;

/// Index into the receive buffer indicating which byte is due to be read next.
static size_t  rxHead;

/// LDMA channel configuration triggering on free space in UART transmit FIFO
static sl_hal_ldma_transfer_config_t ldmaTxTransfer = SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(0);

/// LDMA channel configuration triggering on available byte in UART receive FIFO
static sl_hal_ldma_transfer_config_t ldmaRxTransfer = SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(0);

/// LDMA transfer for copying transmit buffer to UART TX FIFO
static sl_hal_ldma_descriptor_t ldmaTxDesc = SL_HAL_LDMA_DESCRIPTOR_SINGLE_M2P(
  SL_HAL_LDMA_CTRL_SIZE_BYTE,
  txBuffer,
  NULL,
  0
  );

static sl_hal_ldma_descriptor_t ldmaRxDesc[4] = {
  /// First half of receive pingpong configuration
  SL_HAL_LDMA_DESCRIPTOR_LINKREL_P2M(
    SL_HAL_LDMA_CTRL_SIZE_BYTE,
    NULL,
    &(rxBuffer[0]),
    SL_DRIVER_EUART_RX_BUFFER_SIZE / 2,
    1
    ),
  // Sync structure waiting for SYNC[1], clearing SYNC[0]
  SL_HAL_LDMA_DESCRIPTOR_LINKREL_SYNC(
    0,
    (1 << 0),
    (1 << 1),
    (1 << 1),
    1
    ),
  /// Second half of receive pingpong configuration
  SL_HAL_LDMA_DESCRIPTOR_LINKREL_P2M(
    SL_HAL_LDMA_CTRL_SIZE_BYTE,
    NULL,
    &(rxBuffer[SL_DRIVER_EUART_RX_BUFFER_SIZE / 2]),
    SL_DRIVER_EUART_RX_BUFFER_SIZE / 2,
    1
    ),
  /// Sync structure waiting for SYNC[0], clearing SYNC[1]
  SL_HAL_LDMA_DESCRIPTOR_LINKREL_SYNC(
    0,
    (1 << 1),
    (1 << 0),
    (1 << 0),
    -3
    )
};
//  ‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
// Functions

/**
 * Wait for ongoing sync of register(s) to the low-frequency domain to complete.
 *
 * @param mask A bitmask corresponding to SYNCBUSY register defined bits,
 *             indicating registers that must complete any ongoing
 *             synchronization.
 */
void eusart_sync(uint32_t mask)
{
  // Wait for any pending previous write operation to have been completed
  // in the low-frequency domain.
  while ((sl_uart_init_inst.port->SYNCBUSY & mask) != 0U) {
  }
}

/**
 * Get EUSART STATUS register.
 *
 * @return STATUS register value.
 */
uint32_t eusart_status_get(void)
{
  return sl_uart_init_inst.port->STATUS;
}

/**
 * Configure the EUSART to its reset state.
 */
void eusart_reset(void)
{
  // 1. Properly disable the module
  eusart_disable();

  // Manual toggling tx_sclk_mst to synchronize handshake
  // when switching from SPI master to other modes
  // so module is disabling correctly.
  uint32_t forcedClkCycle = 4u;

  while (forcedClkCycle--) {
    sl_uart_init_inst.port->CFG2_SET = _EUSART_CFG2_CLKPHA_MASK;
    sl_uart_init_inst.port->CFG2_CLR = _EUSART_CFG2_CLKPHA_MASK;
  }

  // All registers that end with CFG should be programmed before EUSART gets enabled (EUSARTn_EN is set).
  // Set all configurable register to its reset value.
  // Note: Program desired settings to all registers that have names ending with CFG in the following sequence:
  //  a. CFG2
  sl_uart_init_inst.port->CFG2 = _EUSART_CFG2_RESETVALUE;
  //  b. CFG1
  sl_uart_init_inst.port->CFG1 = _EUSART_CFG1_RESETVALUE;
  //  c. CFG0
  sl_uart_init_inst.port->CFG0 = _EUSART_CFG0_RESETVALUE;
  //  d. FRAMECFG, DTXDATCFG, TIMINGCFG (Any sequence)
  sl_uart_init_inst.port->FRAMECFG = _EUSART_FRAMECFG_RESETVALUE;
  sl_uart_init_inst.port->DTXDATCFG = _EUSART_DTXDATCFG_RESETVALUE;
#if defined(EUSART_DALICFG_DALIEN)
  sl_uart_init_inst.port->DALICFG = _EUSART_DALICFG_RESETVALUE;
#endif /* EUSART_DALICFG_DALIEN */
  sl_uart_init_inst.port->TIMINGCFG = _EUSART_TIMINGCFG_RESETVALUE;
  sl_uart_init_inst.port->IRHFCFG = _EUSART_IRHFCFG_RESETVALUE;
  sl_uart_init_inst.port->IRLFCFG = _EUSART_IRLFCFG_RESETVALUE;
  sl_uart_init_inst.port->STARTFRAMECFG = _EUSART_STARTFRAMECFG_RESETVALUE;
  sl_uart_init_inst.port->SIGFRAMECFG = _EUSART_SIGFRAMECFG_RESETVALUE;
  sl_uart_init_inst.port->TRIGCTRL = _EUSART_TRIGCTRL_RESETVALUE;
  sl_uart_init_inst.port->IEN = _EUSART_IEN_RESETVALUE;
  sl_uart_init_inst.port->IF_CLR = _EUSART_IF_MASK;

  // no need to sync while EN=0, multiple writes can be queued up,
  // and the last one will synchronize once EN=1
  sl_uart_init_inst.port->CLKDIV = _EUSART_CLKDIV_RESETVALUE;
}

/**
 * Enables the EUSART receiver and transmitter.
 */
void eusart_enable(void)
{
  uint32_t tmp = 0;

  // Enable peripheral to configure Rx and Tx.
  sl_uart_init_inst.port->EN_SET = EUSART_EN_EN;

  // Enable Rx and Tx
  tmp = (EUSART_CMD_RXEN | EUSART_CMD_TXEN)
        & (_EUSART_CMD_RXEN_MASK | _EUSART_CMD_TXEN_MASK
           | _EUSART_CMD_RXDIS_MASK | _EUSART_CMD_TXDIS_MASK);

  eusart_sync(_EUSART_SYNCBUSY_MASK);
  sl_uart_init_inst.port->CMD = tmp;
  eusart_sync(EUSART_SYNCBUSY_RXEN | EUSART_SYNCBUSY_TXEN
              | EUSART_SYNCBUSY_RXDIS | EUSART_SYNCBUSY_TXDIS);

  // Wait for the status register to be updated.
  tmp = 0;
  if (_EUSART_CMD_RXEN_MASK & (EUSART_CMD_RXEN | EUSART_CMD_TXEN)) {
    tmp |= EUSART_STATUS_RXENS;
  }
  if (_EUSART_CMD_TXEN_MASK & (EUSART_CMD_RXEN | EUSART_CMD_TXEN)) {
    tmp |= EUSART_STATUS_TXENS;
  }
  while ((sl_uart_init_inst.port->STATUS & (_EUSART_STATUS_TXENS_MASK | _EUSART_STATUS_RXENS_MASK)) != tmp) {
  }
}

/**
 * Configures the baudrate (or as close as possible to a specified baudrate)
 * depending on the current mode of the EU(S)ART peripheral.
 *
 * @note (1) When the oversampling is disabled, the peripheral clock frequency
 *           must be at least three times higher than the chosen baud rate.
 */
void eusart_baudrate_set(void)
{
  uint32_t          clkdiv;
  uint8_t           oversample = 16;
  uint32_t baudrate = SL_SERIAL_EUART_BAUD_RATE;
  uint32_t refFreq = util_getClockFreq() / (1U + ((CMU->SYSCLKCTRL & _CMU_SYSCLKCTRL_PCLKPRESC_MASK)
                                                  >> _CMU_SYSCLKCTRL_PCLKPRESC_SHIFT));

#if defined(EUSART_DALICFG_DALIEN)
  if (sl_uart_init_inst.port->DALICFG & EUSART_DALICFG_DALIEN) {
    // adjust for manchester double-clocking scheme
    baudrate *= 2;
  }
#endif

  /*
   * Use integer division to avoid forcing in float division
   * utils, and yet keep rounding effect errors to a minimum.
   *
   * CLKDIV is given by:
   *
   * CLKDIV = 256 * (fUARTn/(oversample * br) - 1)
   * or
   * CLKDIV = (256 * fUARTn)/(oversample * br) - 256
   *
   * Since fUARTn may be derived from HFCORECLK, consider the overflow when
   * using integer arithmetic.
   *
   * The basic problem with integer division in the above formula is that
   * the dividend (256 * fUARTn) may become higher than the maximum 32 bit
   * integer. Yet, the dividend should be evaluated first before dividing
   * to get as small rounding effects as possible.
   * Also, harsh restrictions on the maximum fUARTn value should not be made.
   *
   * Since the last 3 bits of CLKDIV are don't care, base the
   * integer arithmetic on the below formula:
   *
   * CLKDIV/8 = ((32*fUARTn)/(br * Oversample)) - 32
   *
   * and calculate 1/8 of CLKDIV first. This allows for fUARTn
   * up to 128 MHz without overflowing a 32 bit value.
   */

  // Calculate and set the CLKDIV with fractional bits.
  clkdiv  = (32 * refFreq) / (baudrate * oversample);
  clkdiv -= 32;
  clkdiv *= 8;

  // If the EFM_ASSERT is not enabled, make sure not to write to reserved bits.
  clkdiv &= _EUSART_CLKDIV_MASK;

  eusart_sync(_EUSART_SYNCBUSY_DIV_MASK);
  sl_uart_init_inst.port->CLKDIV = clkdiv;
  eusart_sync(_EUSART_SYNCBUSY_DIV_MASK);
}

/**
 * Initializes the EUSART with asynchronous common settings to high
 * and low frequency clock.
 */
void eusart_async_init_common(void)
{
  // LF register about to be modified requires sync busy check.
  if (sl_uart_init_inst.port->EN) {
    eusart_sync(_EUSART_SYNCBUSY_MASK);
  }
  // Initialize EUSART registers to hardware reset state.
  eusart_reset();

#if (SL_SERIAL_EUART_FLOW_CONTROL == 1)
  sl_uart_init_inst.port->CFG1 |= EUSART_CFG1_CTSEN;
  // Enable RTS route pin if necessary. CTS is an input so it is enabled by default.
  GPIO->EUSARTROUTE_SET[EUSART_NUM(sl_uart_init_inst.port)].ROUTEEN = GPIO_EUSART_ROUTEEN_RTSPEN;
#else
  GPIO->EUSARTROUTE_CLR[EUSART_NUM(sl_uart_init_inst.port)].ROUTEEN = GPIO_EUSART_ROUTEEN_RTSPEN;
#endif

  // Enable EUSART IP.
  eusart_enable();

  // Configure the baudrate if auto baud detection is not used.
  eusart_baudrate_set();

  // Finally disable the Rx and Tx channel (as specified).
  eusart_disable();
  while (~eusart_status_get() & (_EUSART_STATUS_RXIDLE_MASK | _EUSART_STATUS_TXIDLE_MASK)) {
  }
}

/**
 * Perform EUSART Module disablement - resetting all internal flops/FSM.
 */
void eusart_disable(void)
{
  if (sl_uart_init_inst.port->EN & _EUSART_EN_EN_MASK) {
    // This step should be skipped especially in Synchronous Slave mode when
    // external SCLK is not running and CS is active
    if (!(sl_uart_init_inst.port->CFG0 & _EUSART_CFG0_SYNC_MASK) || (sl_uart_init_inst.port->CFG2 & _EUSART_CFG2_MASTER_MASK)) {
      // General Programming Guideline to properly disable the module:
      // 1a. Disable TX and RX using TXDIS and RXDIS cmd
      sl_uart_init_inst.port->CMD = EUSART_CMD_TXDIS | EUSART_CMD_RXDIS;
      // 1b. Poll for EUSARTn_SYNCBUSY.TXDIS and EUSARTn_SYNCBUSY.RXDIS to go low;
      eusart_sync(EUSART_SYNCBUSY_TXDIS | EUSART_SYNCBUSY_RXDIS);
      // 1c. Wait for EUSARTn_STATUS.TXENS and EUSARTn_STATUS.RXENS to go low
      while (sl_uart_init_inst.port->STATUS & (_EUSART_STATUS_TXENS_MASK | _EUSART_STATUS_RXENS_MASK)) {
      }
    }
    sl_uart_init_inst.port->CLKDIV = sl_uart_init_inst.port->CLKDIV;
    eusart_sync(_EUSART_SYNCBUSY_DIV_MASK);

    // Read data until FIFO is emptied
    // but taking care not to underflow the receiver
    while (sl_uart_init_inst.port->STATUS & EUSART_STATUS_RXFL) {
      sl_uart_init_inst.port->RXDATA;
    }

    sl_uart_init_inst.port->EN_CLR = EUSART_EN_EN;

#if defined(_EUSART_EN_DISABLING_MASK)
    // 2. Polling for EUSARTn_EN.DISABLING = 0.
    while (sl_uart_init_inst.port->EN & _EUSART_EN_DISABLING_MASK) {
    }
#endif
  }
}

/**
 * Initialize the configured EUART peripheral for UART operation. Also sets up
 *  GPIO settings for TX, RX, and, if configured, flow control.
 */
void uart_init(void)
{
#if defined(EUSART0)  && (SL_SERIAL_EUART_PERIPHERAL_NO == 0)
  sl_uart_init_inst.euartClock = cmuClock_EUSART0;
  ldmaTxTransfer.request_sel = (LDMAXBAR0_CH_REQSEL_SIGSEL_EUSART0TXFL | LDMAXBAR0_CH_REQSEL_SOURCESEL_EUSART0);
  ldmaRxTransfer.request_sel = (LDMAXBAR0_CH_REQSEL_SIGSEL_EUSART0RXFL | LDMAXBAR0_CH_REQSEL_SOURCESEL_EUSART0);
  CMU->CLKEN1_SET = CMU_CLKEN1_EUSART0;
#elif defined(EUSART1) && (SL_SERIAL_EUART_PERIPHERAL_NO == 1)
  sl_uart_init_inst.euartClock = cmuClock_EUSART1;
  ldmaTxTransfer.request_sel = (LDMAXBAR0_CH_REQSEL_SIGSEL_EUSART1TXFL | LDMAXBAR0_CH_REQSEL_SOURCESEL_EUSART1);
  ldmaRxTransfer.request_sel = (LDMAXBAR0_CH_REQSEL_SIGSEL_EUSART1RXFL | LDMAXBAR0_CH_REQSEL_SOURCESEL_EUSART1);
  CMU->CLKEN1_SET = CMU_CLKEN1_EUSART1;
#elif defined(EUSART2) && (SL_SERIAL_EUART_PERIPHERAL_NO == 2)
  sl_uart_init_inst.euartClock = cmuClock_EUSART2;
  ldmaTxTransfer.request_sel = (LDMAXBAR0_CH_REQSEL_SIGSEL_EUSART2TXFL | LDMAXBAR0_CH_REQSEL_SOURCESEL_EUSART2);
  ldmaRxTransfer.request_sel = (LDMAXBAR0_CH_REQSEL_SIGSEL_EUSART2RXFL | LDMAXBAR0_CH_REQSEL_SOURCESEL_EUSART2);
  CMU->CLKEN1_SET = CMU_CLKEN1_EUSART2;
#endif

  ldmaTxDesc.xfer.dst_addr = (uint32_t)&(sl_uart_init_inst.port->TXDATA);

  ldmaRxDesc[0].xfer.src_addr = (uint32_t)&(sl_uart_init_inst.port->RXDATA);

  ldmaRxDesc[2].xfer.src_addr = (uint32_t)&(sl_uart_init_inst.port->RXDATA);

  CMU->CLKEN0_SET = CMU_CLKEN0_GPIO;

  // Configure CTS/RTS in case of flow control
#if (SL_SERIAL_EUART_FLOW_CONTROL == 1)
  const sl_gpio_t gpio_hf_ctrl[] = {  { SL_SERIAL_EUART_RTS_PORT,
                                        SL_SERIAL_EUART_RTS_PIN },
                                      { SL_SERIAL_EUART_CTS_PORT,
                                        SL_SERIAL_EUART_CTS_PIN } };
  sl_hal_gpio_set_pin_mode(&gpio_hf_ctrl[0U],
                           SL_GPIO_MODE_PUSH_PULL,
                           1);
  sl_hal_gpio_set_pin_mode(&gpio_hf_ctrl[1U],
                           SL_GPIO_MODE_INPUT,
                           1);

  GPIO->EUSARTROUTE_SET[sl_uart_init_inst.uartNum].ROUTEEN = GPIO_EUSART_ROUTEEN_RTSPEN;
  GPIO->EUSARTROUTE_SET[sl_uart_init_inst.uartNum].RTSROUTE =
    (gpio_hf_ctrl[0U].port << _GPIO_EUSART_RTSROUTE_PORT_SHIFT)
    | (gpio_hf_ctrl[0U].pin << _GPIO_EUSART_RTSROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE_SET[sl_uart_init_inst.uartNum].CTSROUTE =
    (gpio_hf_ctrl[1U].port << _GPIO_EUSART_CTSROUTE_PORT_SHIFT)
    | (gpio_hf_ctrl[1U].pin << _GPIO_EUSART_CTSROUTE_PIN_SHIFT);
#endif

  eusart_async_init_common();
  const sl_gpio_t gpio_uart[] = { {
                                    SL_SERIAL_EUART_TX_PORT,
                                    SL_SERIAL_EUART_TX_PIN
                                  },
                                  {
                                    SL_SERIAL_EUART_RX_PORT,
                                    SL_SERIAL_EUART_RX_PIN
                                  } };

  sl_hal_gpio_set_pin_mode(&gpio_uart[0U],
                           SL_GPIO_MODE_PUSH_PULL,
                           1);
  sl_hal_gpio_set_pin_mode(&gpio_uart[1U],
                           SL_GPIO_MODE_INPUT,
                           1);

  GPIO->EUSARTROUTE[sl_uart_init_inst.uartNum].ROUTEEN |= GPIO_EUSART_ROUTEEN_TXPEN;
  GPIO->EUSARTROUTE[sl_uart_init_inst.uartNum].TXROUTE = (SL_SERIAL_EUART_TX_PORT
                                                          << _GPIO_EUSART_TXROUTE_PORT_SHIFT)
                                                         | (SL_SERIAL_EUART_TX_PIN << _GPIO_EUSART_TXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[sl_uart_init_inst.uartNum].RXROUTE = (SL_SERIAL_EUART_RX_PORT
                                                          << _GPIO_EUSART_RXROUTE_PORT_SHIFT)
                                                         | (SL_SERIAL_EUART_RX_PIN << _GPIO_EUSART_RXROUTE_PIN_SHIFT);

#if (SL_VCOM_ENABLE == 1) && defined(SL_VCOM_ENABLE_PORT)
  const sl_gpio_t gpio_vcom = { .port = SL_VCOM_ENABLE_PORT,
                                .pin = SL_VCOM_ENABLE_PIN };
  sl_hal_gpio_set_pin_mode(&gpio_vcom,
                           SL_GPIO_MODE_PUSH_PULL,
                           1);
#endif
  eusart_enable();

  // Configure LDMA
  CMU->CLKEN0_SET |= (CMU_CLKEN0_LDMA0 | CMU_CLKEN0_LDMAXBAR0);
  LDMA0->EN = LDMA_EN_EN;
  // Reset LDMA
  LDMA0->CTRL = _LDMA_CTRL_RESETVALUE;
  LDMA0->CHEN = _LDMA_CHEN_RESETVALUE;
  LDMA0->DBGHALT = _LDMA_DBGHALT_RESETVALUE;
  LDMA0->REQDIS = _LDMA_REQDIS_RESETVALUE;
  LDMA0->IEN = _LDMA_IEN_RESETVALUE;

  // Set up channel 0 as RX transfer
  LDMAXBAR0->CH[SL_DRIVER_EUSART_LDMA_RX_CHANNEL].REQSEL = ldmaRxTransfer.request_sel;
  LDMA0->CH[SL_DRIVER_EUSART_LDMA_RX_CHANNEL].LOOP
    = (ldmaRxTransfer.loop_count << _LDMA_CH_LOOP_LOOPCNT_SHIFT);
  LDMA0->CH[SL_DRIVER_EUSART_LDMA_RX_CHANNEL].CFG
    = (ldmaRxTransfer.arb_slots << _LDMA_CH_CFG_ARBSLOTS_SHIFT)
      | (ldmaRxTransfer.src_inc_sign << _LDMA_CH_CFG_SRCINCSIGN_SHIFT)
      | (ldmaRxTransfer.dst_inc_sign << _LDMA_CH_CFG_DSTINCSIGN_SHIFT);

  LDMA0->CH[SL_DRIVER_EUSART_LDMA_RX_CHANNEL].LINK
    = (uint32_t)(&ldmaRxDesc[0]) & _LDMA_CH_LINK_LINKADDR_MASK;

  // Set up channel 1 as TX transfer
  LDMAXBAR0->CH[SL_DRIVER_EUSART_LDMA_TX_CHANNEL].REQSEL = ldmaTxTransfer.request_sel;
  LDMA0->CH[SL_DRIVER_EUSART_LDMA_TX_CHANNEL].LOOP
    = (ldmaTxTransfer.loop_count << _LDMA_CH_LOOP_LOOPCNT_SHIFT);
  LDMA0->CH[SL_DRIVER_EUSART_LDMA_TX_CHANNEL].CFG
    = (ldmaTxTransfer.arb_slots << _LDMA_CH_CFG_ARBSLOTS_SHIFT)
      | (ldmaTxTransfer.src_inc_sign << _LDMA_CH_CFG_SRCINCSIGN_SHIFT)
      | (ldmaTxTransfer.dst_inc_sign << _LDMA_CH_CFG_DSTINCSIGN_SHIFT);

  // Clear DONE flag on both RX and TX channels
  sl_hal_bus_reg_clear_mask(&LDMA0->CHDONE,
                            ((1 << SL_DRIVER_EUSART_LDMA_RX_CHANNEL)
                             | (1 << SL_DRIVER_EUSART_LDMA_TX_CHANNEL)));

  // Kick off background RX
  LDMA0->LINKLOAD = (1 << SL_DRIVER_EUSART_LDMA_RX_CHANNEL);

  // Mark second half of RX buffer as ready
  LDMA0->SYNCSWSET_SET = (1UL << 1);

  initialized = true;
}

/**
 * Disable the configured EUART peripheral for UART operation.
 */
void uart_deinit(void)
{
  CMU->CLKEN0_CLR = (CMU_CLKEN0_LDMA0 | CMU_CLKEN0_LDMAXBAR0);
  CMU->CLKEN1_CLR = CMU_CLKEN1_EUSART0;

  initialized = false;
}

/**
 * Write a data buffer to the UART.
 *
 * @param[in] buffer The data buffer to send
 * @param[in] length Amount of bytes in the buffer to send
 * @param[in] blocking Indicates whether this transfer can be offloaded to LDMA
 *  and return, or whether it should wait on completion before returning.
 *
 * @return BOOTLOADER_OK if successful, error code otherwise
 */
int32_t uart_sendBuffer(const uint8_t* buffer, size_t length, bool blocking)
{
  BTL_ASSERT(initialized == true);
  BTL_ASSERT(length > 0);

  if (length == 0 || length >= SL_DRIVER_EUART_TX_BUFFER_SIZE) {
    return BOOTLOADER_ERROR_SPI_PERIPHERAL_ARGUMENT;
  }

  if (!uart_isTxIdle()) {
    return BOOTLOADER_ERROR_UART_BUSY;
  }

  // Copy buffer
  txLength = length;
  length = 0;
  for (; length < txLength; length++) {
    txBuffer[length] = buffer[length];
  }

  // Populate descriptor
  ldmaTxDesc.xfer.xfer_count = txLength - 1;

  // Kick off transfer
  LDMA0->CH[SL_DRIVER_EUSART_LDMA_TX_CHANNEL].LINK
    = (uint32_t)(&ldmaTxDesc) & _LDMA_CH_LINK_LINKADDR_MASK;
  LDMA0->LINKLOAD = (1UL << SL_DRIVER_EUSART_LDMA_TX_CHANNEL);

  // Optional wait for completion
  if (blocking) {
    while (uart_isTxIdle() == false) {
      // Do nothing
    }
    while (!(sl_uart_init_inst.port->STATUS & EUSART_STATUS_TXC)) {
      // Do nothing
    }
  }

  return BOOTLOADER_OK;
}

/**
 * Write one byte to the UART in a blocking fashion.
 *
 * @param[in] byte The byte to send
 *
 * @return BOOTLOADER_OK if successful, error code otherwise
 */
int32_t uart_sendByte(uint8_t byte)
{
  BTL_ASSERT(initialized == true);

  // Wait until previous LDMA transfer is done
  while (!uart_isTxIdle()) {
    // Do nothing
  }

  // Wait until there is room for one more byte
  while (!(sl_uart_init_inst.port->STATUS & EUSART_STATUS_TXFL)) {
    // Do nothing
  }

  // Send byte
  sl_uart_init_inst.port->TXDATA = byte;

  // Wait until byte has been fully sent out
  while (!(sl_uart_init_inst.port->STATUS & EUSART_STATUS_TXC)) {
    // Do nothing
  }
  return BOOTLOADER_OK;
}

/**
 * Find out whether the UART can accept more data to send.
 *
 * @return true if the UART is not currently transmitting
 */
bool uart_isTxIdle(void)
{
  BTL_ASSERT(initialized == true);
  if (LDMA0->CHDONE & (1 << SL_DRIVER_EUSART_LDMA_TX_CHANNEL)) {
    LDMA0->CHDIS = (1 << SL_DRIVER_EUSART_LDMA_TX_CHANNEL);
    sl_hal_bus_reg_clear_mask(&LDMA0->CHDONE, 1 << SL_DRIVER_EUSART_LDMA_TX_CHANNEL);
    txLength = 0;
    return true;
  } else if ((LDMA0->CHSTATUS & (1 << SL_DRIVER_EUSART_LDMA_TX_CHANNEL)) == 0) {
    sl_hal_bus_reg_clear_mask(&LDMA0->CHDONE, 1 << SL_DRIVER_EUSART_LDMA_TX_CHANNEL);
    txLength = 0;
    return true;
  }

  return false;
}

/**
 * Gets the amount of bytes ready for reading.
 *
 * @return Amount of bytes in the receive buffer available for reading with
 *   @ref protocol_uart_recv
 */
size_t  uart_getRxAvailableBytes(void)
{
  size_t ldmaHead;
  size_t dst;

  BTL_ASSERT(initialized == true);

  // Get destination address for next transfer
  dst = LDMA0->CH[SL_DRIVER_EUSART_LDMA_RX_CHANNEL].DST;

  if (dst == 0x0101) {
    // SYNC descriptor with bit 0 of MATCHEN and MATCHVAL set
    ldmaHead = 0;
  } else if (dst == 0x0202) {
    // SYNC descriptor with bit 1 of MATCHEN and MATCHVAL set
    ldmaHead = SL_DRIVER_EUART_RX_BUFFER_SIZE / 2;
  } else {
    // XFER descriptor with absolute address in buffer
    ldmaHead = dst - (uint32_t)(rxBuffer);
  }

  // Return difference between received head and LDMA head
  if (rxHead == ldmaHead) {
    return 0;
  } else if (rxHead < ldmaHead) {
    return ldmaHead - rxHead;
  } else {
    return SL_DRIVER_EUART_RX_BUFFER_SIZE - (rxHead - ldmaHead);
  }
}

/**
 * Read from the UART into a data buffer.
 *
 * @param[out] buffer The data buffer to receive into
 * @param[in] requestedLength Amount of bytes we'd like to read
 * @param[out] receivedLength Amount of bytes read
 * @param[in] blocking Indicates whether we to wait for requestedLength
 *   bytes to be available and read before returning, whether to read out
 *   data currently in the buffer and return.
 * @param[in] timeout Number of milliseconds to wait for data in blocking mode
 *
 * @return BOOTLOADER_OK if successful, error code otherwise
 */
int32_t uart_receiveBuffer(uint8_t  * buffer,
                           size_t   requestedLength,
                           size_t   * receivedLength,
                           bool     blocking,
                           uint32_t timeout)
{
  size_t copyBytes = 0;
  size_t copiedBytes = 0;

  BTL_ASSERT(initialized == true);
  BTL_ASSERT(requestedLength < SL_DRIVER_EUART_RX_BUFFER_SIZE);

  // Check whether we have enough data
  // Optional spin for timeout cycles
  if (blocking) {
    if (timeout != 0) {
      delay_init();
      delay_milliseconds(timeout, false);
    }

    while (uart_getRxAvailableBytes() < requestedLength) {
      if ((timeout != 0) && delay_expired()) {
        break;
      }
    }
  }

  copyBytes = uart_getRxAvailableBytes();
  if (requestedLength < copyBytes) {
    copyBytes = requestedLength;
  }

  // Copy up to requested bytes to given buffer
  while (copiedBytes < copyBytes) {
    buffer[copiedBytes] = rxBuffer[rxHead];
    copiedBytes++;
    rxHead++;

    if (rxHead == SL_DRIVER_EUART_RX_BUFFER_SIZE) {
      rxHead = 0;
      // Completed processing of second half of the buffer, mark it as available
      // for LDMA again by setting SYNC[1]
      LDMA0->SYNCSWSET_SET = 1 << 1;
    } else if (rxHead == SL_DRIVER_EUART_RX_BUFFER_SIZE / 2) {
      // Completed processing of first half of the buffer, mark it as available
      // for LDMA again by setting SYNC[0]
      LDMA0->SYNCSWSET_SET = 1 << 0;
    }
  }

  if ((uint32_t)receivedLength != 0UL) {
    *receivedLength = copiedBytes;
  }

  if (copiedBytes < requestedLength) {
    return BOOTLOADER_ERROR_UART_TIMEOUT;
  } else {
    return BOOTLOADER_OK;
  }
}

/**
 * Get one byte from the UART in a blocking fashion.
 *
 * @param[out] byte The byte to send
 *
 * @return BOOTLOADER_OK if successful, error code otherwise
 */
int32_t uart_receiveByte(uint8_t* byte)
{
  return uart_receiveBuffer(byte, 1, (size_t*)0UL, true, 0);
}

/**
 * Get one byte from the UART in a blocking fashion.
 *
 * @param[out] byte The byte to send
 * @param[in]  timeout Maximum timeout before aborting transfer
 *
 * @return BOOTLOADER_OK if successful, error code otherwise
 */
int32_t uart_receiveByteTimeout(uint8_t* byte, uint32_t timeout)
{
  return uart_receiveBuffer(byte, 1, (size_t *)0UL, true, timeout);
}

/**
 * Flushes one or both UART buffers.
 *
 * @param[in] flushTx Flushes the transmit buffer when true
 * @param[in] flushRx Flushes the receive buffer when true
 *
 * @return BOOTLOADER_OK
 */
int32_t uart_flush(bool flushTx, bool flushRx)
{
  BTL_ASSERT(initialized == true);

  if (flushTx) {
    LDMA0->CHDIS = (1 << SL_DRIVER_EUSART_LDMA_TX_CHANNEL);
    txLength = 0;
  }

  if (flushRx) {
    LDMA0->CHDIS = (1 << SL_DRIVER_EUSART_LDMA_RX_CHANNEL);
    sl_hal_bus_reg_clear_mask(&LDMA0->CHDONE, 1 << SL_DRIVER_EUSART_LDMA_RX_CHANNEL);
    LDMA0->CH[SL_DRIVER_EUSART_LDMA_RX_CHANNEL].LINK
      = (uint32_t)(&ldmaRxDesc[0]) & _LDMA_CH_LINK_LINKADDR_MASK;
    rxHead = 0;
    LDMA0->LINKLOAD = (1 << SL_DRIVER_EUSART_LDMA_RX_CHANNEL);

    // Mark second half of RX buffer as ready
    sl_hal_bus_reg_set_mask(&LDMA0->SYNCSWSET, 1 << 1);
  }

  return BOOTLOADER_OK;
}
