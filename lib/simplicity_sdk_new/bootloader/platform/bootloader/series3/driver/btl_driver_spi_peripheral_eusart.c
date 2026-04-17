/***************************************************************************//**
 * @file
 * @brief Universal SPI peripheral eusart driver for the Silicon Labs Bootloader.
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
#include "btl_driver_delay.h"
#include "btl_driver_spi_peripheral.h"
#include "btl_driver_cmu.h"
#include "sl_hal_gpio.h"
#include "sl_hal_ldma.h"
#include "debug/btl_debug.h"
#include "btl_spi_peripheral_eusart_driver_cfg.h"

#if SL_EUSART_SPINCP_PERIPHERAL_NO == 0
#define BTL_SPI_PERIPHERAL_EUSART                     EUSART0
#define BTL_SPI_PERIPHERAL_EUSART_NUM                 0
#define BTL_SPI_PERIPHERAL_EUSART_CLOCK               cmuClock_EUSART0
#define BTL_SPI_PERIPHERAL_EUSART_LDMA_TXFL_SIGNAL    (SL_HAL_LDMA_PERIPHERAL_SIGNAL_EUART0_TXFL)
#define BTL_SPI_PERIPHERAL_EUSART_LDMA_RXFL_SIGNAL    (SL_HAL_LDMA_PERIPHERAL_SIGNAL_EUSART0_RXFL)
#elif SL_EUSART_SPINCP_PERIPHERAL_NO == 1
#define BTL_SPI_PERIPHERAL_EUSART                     EUSART1
#define BTL_SPI_PERIPHERAL_EUSART_NUM                 1
#define BTL_SPI_PERIPHERAL_EUSART_CLOCK               cmuClock_EUSART1
#define BTL_SPI_PERIPHERAL_EUSART_LDMA_TXFL_SIGNAL    (SL_HAL_LDMA_PERIPHERAL_SIGNAL_EUSART1_TXFL)
#define BTL_SPI_PERIPHERAL_EUSART_LDMA_RXFL_SIGNAL    (SL_HAL_LDMA_PERIPHERAL_SIGNAL_EUSART1_RXFL)
#elif SL_EUSART_SPINCP_PERIPHERAL_NO == 2
#define BTL_SPI_PERIPHERAL_EUSART                     EUSART2
#define BTL_SPI_PERIPHERAL_EUSART_NUM                 2
#define BTL_SPI_PERIPHERAL_EUSART_CLOCK               cmuClock_EUSART2
#define BTL_SPI_PERIPHERAL_EUSART_LDMA_TXFL_SIGNAL    (SL_HAL_LDMA_PERIPHERAL_SIGNAL_EUSART2_TXFL)
#define BTL_SPI_PERIPHERAL_EUSART_LDMA_RXFL_SIGNAL    (SL_HAL_LDMA_PERIPHERAL_SIGNAL_EUSART2_RXFL)
#else
#error "Invalid BTL_SPI_PERIPHERAL_EUSART"
#endif

// ‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
// Configuration validation
#if (SL_SPI_PERIPHERAL_EUSART_RX_BUFFER_SIZE % 2) != 0
#error "SPI Peripheral RX buffer size is not even"
#endif

// ‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
// Static variables

/// Flag to indicate hardware is up and running
static bool    initialized = false;
/// Receive buffer (forced to even size).
/// LDMA will pingpong between two halves of this buffer.
static uint8_t rxBuffer[SL_SPI_PERIPHERAL_EUSART_RX_BUFFER_SIZE];
/// Transmit buffer for LDMA use.
static uint8_t txBuffer[SL_SPI_PERIPHERAL_EUSART_TX_BUFFER_SIZE];

/// Index into the receive buffer indicating which byte is due to be read next.
static size_t  rxHead;

/// LDMA channel configuration triggering on free space in hardware FIFO
static const sl_hal_ldma_transfer_config_t ldmaTxTransfer = SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(
  BTL_SPI_PERIPHERAL_EUSART_LDMA_TXFL_SIGNAL
  );
/// LDMA channel configuration triggering on available byte in hardware FIFO
static const sl_hal_ldma_transfer_config_t ldmaRxTransfer = SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(
  BTL_SPI_PERIPHERAL_EUSART_LDMA_RXFL_SIGNAL
  );

/// LDMA transfer for copying transmit buffer to hardware FIFO
static sl_hal_ldma_descriptor_t ldmaTxDesc = SL_HAL_LDMA_DESCRIPTOR_SINGLE_M2P(
  SL_HAL_LDMA_CTRL_SIZE_BYTE,
  txBuffer,
  &(BTL_SPI_PERIPHERAL_EUSART->TXDATA),
  0
  );

static const sl_hal_ldma_descriptor_t ldmaRxDesc[4] = {
  /// First half of receive pingpong configuration
  SL_HAL_LDMA_DESCRIPTOR_LINKREL_P2M(
    SL_HAL_LDMA_CTRL_SIZE_BYTE,
    &(BTL_SPI_PERIPHERAL_EUSART->RXDATA),
    &(rxBuffer[0]),
    SL_SPI_PERIPHERAL_EUSART_RX_BUFFER_SIZE / 2,
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
    &(BTL_SPI_PERIPHERAL_EUSART->RXDATA),
    &(rxBuffer[SL_SPI_PERIPHERAL_EUSART_RX_BUFFER_SIZE / 2]),
    SL_SPI_PERIPHERAL_EUSART_RX_BUFFER_SIZE / 2,
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

static void sync_buffer(uint32_t index)
{
#if defined(_LDMA_SYNCSWSET_MASK)
  LDMA0->SYNCSWSET_SET = 1 << index;
#else
  LDMA0->SYNC |= 1 << index;
#endif
}

static void eusart_sync(uint32_t mask)
{
  // Wait for any pending previous write operation to have been completed
  // in the low-frequency domain.
  while ((BTL_SPI_PERIPHERAL_EUSART->SYNCBUSY & mask) != 0U) {
  }
}

/**
 * Enables the EUSART receiver and transmitter.
 */
void eusart_enable()
{
  uint32_t tmp = 0;

  // Enable peripheral to configure Rx and Tx.
  BTL_SPI_PERIPHERAL_EUSART->EN_SET = EUSART_EN_EN;

  // Enable Rx and Tx
  tmp = (EUSART_CMD_RXEN | EUSART_CMD_TXEN)
        & (_EUSART_CMD_RXEN_MASK | _EUSART_CMD_TXEN_MASK
           | _EUSART_CMD_RXDIS_MASK | _EUSART_CMD_TXDIS_MASK);

  eusart_sync(_EUSART_SYNCBUSY_MASK);
  BTL_SPI_PERIPHERAL_EUSART->CMD = tmp;
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
  while ((BTL_SPI_PERIPHERAL_EUSART->STATUS & (_EUSART_STATUS_TXENS_MASK | _EUSART_STATUS_RXENS_MASK)) != tmp) {
  }
}

/**
 * Perform EUSART Module disablement - resetting all internal flops/FSM.
 *
 */
void eusart_disable()
{
  if (BTL_SPI_PERIPHERAL_EUSART->EN & _EUSART_EN_EN_MASK) {
    // This step should be skipped especially in Synchronous Slave mode when
    // external SCLK is not running and CS is active
    if (!(BTL_SPI_PERIPHERAL_EUSART->CFG0 & _EUSART_CFG0_SYNC_MASK) || (BTL_SPI_PERIPHERAL_EUSART->CFG2 & _EUSART_CFG2_MASTER_MASK)) {
      // General Programming Guideline to properly disable the module:
      // 1a. Disable TX and RX using TXDIS and RXDIS cmd
      BTL_SPI_PERIPHERAL_EUSART->CMD = EUSART_CMD_TXDIS | EUSART_CMD_RXDIS;
      // 1b. Poll for EUSARTn_SYNCBUSY.TXDIS and EUSARTn_SYNCBUSY.RXDIS to go low;
      eusart_sync(EUSART_SYNCBUSY_TXDIS | EUSART_SYNCBUSY_RXDIS);
      // 1c. Wait for EUSARTn_STATUS.TXENS and EUSARTn_STATUS.RXENS to go low
      while (BTL_SPI_PERIPHERAL_EUSART->STATUS & (_EUSART_STATUS_TXENS_MASK | _EUSART_STATUS_RXENS_MASK)) {
      }
    }
    BTL_SPI_PERIPHERAL_EUSART->CLKDIV = BTL_SPI_PERIPHERAL_EUSART->CLKDIV;
    eusart_sync(_EUSART_SYNCBUSY_DIV_MASK);

    // Read data until FIFO is emptied
    // but taking care not to underflow the receiver
    while (BTL_SPI_PERIPHERAL_EUSART->STATUS & EUSART_STATUS_RXFL) {
      BTL_SPI_PERIPHERAL_EUSART->RXDATA;
    }

    BTL_SPI_PERIPHERAL_EUSART->EN_CLR = EUSART_EN_EN;

#if defined(_EUSART_EN_DISABLING_MASK)
    // 2. Polling for EUSARTn_EN.DISABLING = 0.
    while (BTL_SPI_PERIPHERAL_EUSART->EN & _EUSART_EN_DISABLING_MASK) {
    }
#endif
  }
}

/**
 * Initialize the configured EUSART peripheral for SPI peripheral operation.
 *  Also sets up GPIO settings for MOSI, MISO, SCLK and SS.
 *
 * @return BOOTLOADER_OK if succesful, error code otherwise
 */
void spi_peripheral_init(void)
{
#if defined(_CMU_CLKEN0_MASK)
  CMU->CLKEN0_SET = CMU_CLKEN0_GPIO;
#if SL_EUSART_SPINCP_PERIPHERAL_NO == 0
  CMU->CLKEN1_SET = CMU_CLKEN1_EUSART0;
#elif SL_EUSART_SPINCP_PERIPHERAL_NO == 1
  CMU->CLKEN1_SET = CMU_CLKEN1_EUSART1;
#elif SL_EUSART_SPINCP_PERIPHERAL_NO == 2
  CMU->CLKEN1_SET = CMU_CLKEN1_EUSART2;
#else
#error "Invalid SL_SPINCP_EUSART_PERIPHERAL"
#endif
#endif

  const sl_gpio_t gpio_spi[] = {
    { SL_EUSART_SPINCP_RX_PORT, SL_EUSART_SPINCP_RX_PIN },
    { SL_EUSART_SPINCP_TX_PORT, SL_EUSART_SPINCP_TX_PIN },
    { SL_EUSART_SPINCP_SCLK_PORT, SL_EUSART_SPINCP_SCLK_PIN },
    { SL_EUSART_SPINCP_CS_PORT, SL_EUSART_SPINCP_CS_PIN }
  };
  sl_hal_gpio_set_pin_mode(&gpio_spi[0U],
                           SL_GPIO_MODE_PUSH_PULL,
                           1);
  sl_hal_gpio_set_pin_mode(&gpio_spi[1U],
                           SL_GPIO_MODE_INPUT,
                           1);
  sl_hal_gpio_set_pin_mode(&gpio_spi[2U],
                           SL_GPIO_MODE_INPUT,
                           1);
  sl_hal_gpio_set_pin_mode(&gpio_spi[3U],
                           SL_GPIO_MODE_INPUT,
                           1);

  // Configure route
  GPIO->EUSARTROUTE[BTL_SPI_PERIPHERAL_EUSART_NUM].TXROUTE = 0
                                                             | (SL_EUSART_SPINCP_TX_PORT << _GPIO_EUSART_TXROUTE_PORT_SHIFT)
                                                             | (SL_EUSART_SPINCP_TX_PIN  << _GPIO_EUSART_TXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[BTL_SPI_PERIPHERAL_EUSART_NUM].RXROUTE = 0
                                                             | (SL_EUSART_SPINCP_RX_PORT << _GPIO_EUSART_RXROUTE_PORT_SHIFT)
                                                             | (SL_EUSART_SPINCP_RX_PIN  << _GPIO_EUSART_RXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[BTL_SPI_PERIPHERAL_EUSART_NUM].SCLKROUTE = 0
                                                               | (SL_EUSART_SPINCP_SCLK_PORT << _GPIO_EUSART_SCLKROUTE_PORT_SHIFT)
                                                               | (SL_EUSART_SPINCP_SCLK_PIN  << _GPIO_EUSART_SCLKROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[BTL_SPI_PERIPHERAL_EUSART_NUM].CSROUTE = 0
                                                             | (SL_EUSART_SPINCP_CS_PORT << _GPIO_EUSART_CSROUTE_PORT_SHIFT)
                                                             | (SL_EUSART_SPINCP_CS_PIN  << _GPIO_EUSART_CSROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[BTL_SPI_PERIPHERAL_EUSART_NUM].ROUTEEN = 0
                                                             | GPIO_EUSART_ROUTEEN_TXPEN
                                                             | GPIO_EUSART_ROUTEEN_CSPEN
                                                             | GPIO_EUSART_ROUTEEN_SCLKPEN;

  BTL_SPI_PERIPHERAL_EUSART->CFG2 |= _EUSART_CFG2_RESETVALUE
                                     | EUSART_CFG2_CLKPOL_IDLELOW
                                     | EUSART_CFG2_CLKPHA_SAMPLELEADING
                                     | EUSART_CFG2_FORCELOAD;

  // Bump EUSART into SPI mode
  BTL_SPI_PERIPHERAL_EUSART->CFG0 |= _EUSART_CFG0_RESETVALUE
                                     | EUSART_CFG0_SYNC
                                     | EUSART_CFG0_MSBF;

  // Configure databits to one byte.
  BTL_SPI_PERIPHERAL_EUSART->FRAMECFG = _EUSART_FRAMECFG_RESETVALUE
                                        | EUSART_FRAMECFG_DATABITS_EIGHT;

#if defined(_CMU_CLKEN0_MASK)
  CMU->CLKEN0_SET = (CMU_CLKEN0_LDMA0 | CMU_CLKEN0_LDMAXBAR0);
#endif

  eusart_enable();

#if defined(LDMA_EN_EN)
  LDMA0->EN_SET = LDMA_EN_EN;
#endif

  // Reset LDMA
  LDMA0->CTRL = _LDMA_CTRL_RESETVALUE;
  LDMA0->CHEN = _LDMA_CHEN_RESETVALUE;
  LDMA0->DBGHALT = _LDMA_DBGHALT_RESETVALUE;
  LDMA0->REQDIS = _LDMA_REQDIS_RESETVALUE;
  LDMA0->IEN = _LDMA_IEN_RESETVALUE;

  // Set up channel 0 as RX transfer
#if defined(LDMAXBAR0)
  LDMAXBAR0->CH[SL_SPI_PERIPHERAL_EUSART_LDMA_RX_CHANNEL].REQSEL = ldmaRxTransfer.request_sel;
#else
  LDMA0->CH[SL_SPI_PERIPHERAL_EUSART_LDMA_RX_CHANNEL].REQSEL = ldmaRxTransfer.request_sel;
#endif
  LDMA0->CH[SL_SPI_PERIPHERAL_EUSART_LDMA_RX_CHANNEL].LOOP
    = (ldmaRxTransfer.loop_count << _LDMA_CH_LOOP_LOOPCNT_SHIFT);
  LDMA0->CH[SL_SPI_PERIPHERAL_EUSART_LDMA_RX_CHANNEL].CFG
    = (ldmaRxTransfer.arb_slots << _LDMA_CH_CFG_ARBSLOTS_SHIFT)
      | (ldmaRxTransfer.src_inc_sign << _LDMA_CH_CFG_SRCINCSIGN_SHIFT)
      | (ldmaRxTransfer.dst_inc_sign << _LDMA_CH_CFG_DSTINCSIGN_SHIFT);

  // Set up channel 1 as TX transfer
#if defined(LDMAXBAR0)
  LDMAXBAR0->CH[SL_SPI_PERIPHERAL_EUSART_LDMA_TX_CHANNEL].REQSEL = ldmaTxTransfer.request_sel;
#else
  LDMA0->CH[SL_SPI_PERIPHERAL_EUSART_LDMA_TX_CHANNEL].REQSEL = ldmaTxTransfer.request_sel;
#endif
  LDMA0->CH[SL_SPI_PERIPHERAL_EUSART_LDMA_TX_CHANNEL].LOOP
    = (ldmaTxTransfer.loop_count << _LDMA_CH_LOOP_LOOPCNT_SHIFT);
  LDMA0->CH[SL_SPI_PERIPHERAL_EUSART_LDMA_TX_CHANNEL].CFG
    = (ldmaTxTransfer.arb_slots << _LDMA_CH_CFG_ARBSLOTS_SHIFT)
      | (ldmaTxTransfer.src_inc_sign << _LDMA_CH_CFG_SRCINCSIGN_SHIFT)
      | (ldmaTxTransfer.dst_inc_sign << _LDMA_CH_CFG_DSTINCSIGN_SHIFT);

  // Clear DONE flag on both RX and TX channels
  LDMA0->CHDONE &= ~((1 << SL_SPI_PERIPHERAL_EUSART_LDMA_RX_CHANNEL)
                     | (1 << SL_SPI_PERIPHERAL_EUSART_LDMA_TX_CHANNEL));

  // Kick off background RX
  LDMA0->CH[SL_SPI_PERIPHERAL_EUSART_LDMA_RX_CHANNEL].LINK
    = (uint32_t)(&ldmaRxDesc[0]) & _LDMA_CH_LINK_LINKADDR_MASK;

  LDMA0->LINKLOAD = (1 << SL_SPI_PERIPHERAL_EUSART_LDMA_RX_CHANNEL);

  // Mark second half of RX buffer as ready
  sync_buffer(1U);

  // Enable TX/RX
  BTL_SPI_PERIPHERAL_EUSART->CMD = EUSART_CMD_RXEN
                                   | EUSART_CMD_TXEN
                                   | EUSART_CMD_RXBLOCKEN;

  eusart_sync(_EUSART_SYNCBUSY_RXEN_MASK | _EUSART_SYNCBUSY_TXEN_MASK | _EUSART_SYNCBUSY_RXBLOCKEN_MASK);
  delay_init();
  initialized = true;
}

/**
 * Disable the configured EUSART peripheral for SPI operation.
 */
void spi_peripheral_deinit(void)
{
  eusart_disable();
#if defined(_CMU_CLKEN0_MASK)
  CMU->CLKEN0_CLR = (CMU_CLKEN0_LDMA0 | CMU_CLKEN0_LDMAXBAR0 | CMU_CLKEN0_GPIO);
#endif
  initialized = false;
}

/**
 * Write a data buffer to the controller next time the controller starts clocking SCLK.
 * This transfer will be non-blocking, and its progress can be tracked through
 * @ref spi_peripheral_getTxBytesLeft
 *
 * @param[in] buffer   The data buffer to send
 * @param[in] length   Amount of bytes in the buffer to send
 *
 * @return BOOTLOADER_OK if successful, error code otherwise
 */
int32_t spi_peripheral_send_buffer(uint8_t* buffer,
                                   size_t length)
{
  size_t iterator;

  BTL_ASSERT(initialized == true);
  BTL_ASSERT(length > 0);

  if (length == 0 || length >= SL_SPI_PERIPHERAL_EUSART_TX_BUFFER_SIZE) {
    return BOOTLOADER_ERROR_SPI_PERIPHERAL_ARGUMENT;
  }

  if (spi_peripheral_get_tx_bytes_left() != 0) {
    return BOOTLOADER_ERROR_SPI_PERIPHERAL_BUSY;
  }

  // Copy buffer
  for (iterator = 0; iterator < length; iterator++) {
    txBuffer[iterator] = buffer[iterator];
  }

  // DMA count field for transferring N bytes is defined as N-1.
  ldmaTxDesc.xfer.xfer_count = length - 1;

  // Kick off transfer. Done flag was already cleared by buffer flush.
  LDMA0->CH[SL_SPI_PERIPHERAL_EUSART_LDMA_TX_CHANNEL].LINK = (uint32_t)(&ldmaTxDesc)
                                                             & _LDMA_CH_LINK_LINKADDR_MASK;
  LDMA0->LINKLOAD = (1 << SL_SPI_PERIPHERAL_EUSART_LDMA_TX_CHANNEL);
  // Wait for the DMA transfer to kick into action (otherwise getTxBytesLeft
  // gets confused)
  while (spi_peripheral_get_tx_bytes_left() == 0) {
    // Do nothing
  }

  spi_peripheral_enable_transmitter(true);
  return BOOTLOADER_OK;
}

/**
 * Write one byte to the controller in a blocking fashion.
 *  Warning: if the controller goes down, this will block forever!
 *
 * @param[in] byte The byte to send
 *
 * @return BOOTLOADER_OK if succesful, error code otherwise
 */
int32_t spi_peripheral_send_byte(uint8_t byte)
{
  BTL_ASSERT(initialized == true);

  // Wait until previous LDMA transfer is done
  while (spi_peripheral_get_tx_bytes_left() != 0) {
    // Do nothing
  }

  // Wait until there is room for one more byte
  while (!(BTL_SPI_PERIPHERAL_EUSART->STATUS & EUSART_STATUS_TXFL)) {
    // Do nothing
  }

  // Send byte
  BTL_SPI_PERIPHERAL_EUSART->TXDATA = byte;

  // Wait until byte has been fully sent out
  while (!(BTL_SPI_PERIPHERAL_EUSART->STATUS & EUSART_STATUS_TXIDLE)) {
    // Do nothing
  }

  return BOOTLOADER_OK;
}

/**
 * Gets the amount of bytes left in the TX data buffer.
 * WARNING: There is currently no way of correctly accounting for the last
 * three bytes going out, so this function might indicate 0 while it is
 * still clocking out the last 2/3 bytes.
 *
 * @return Amount of bytes in the transmit buffer still needing to go out
 */
size_t  spi_peripheral_get_tx_bytes_left(void)
{
  size_t txLeft, txLeftPrev;

  BTL_ASSERT(initialized == true);
#if defined(_LDMA_CHSTATUS_MASK)
  if ((LDMA0->CHSTATUS & (1 << SL_SPI_PERIPHERAL_EUSART_LDMA_TX_CHANNEL)) != 0) {
#else
  if ((LDMA0->CHEN & (1 << SL_SPI_PERIPHERAL_EUSART_LDMA_TX_CHANNEL)) != 0) {
#endif
    // 1 byte in shift register, 2 in FIFO.
    txLeft = SL_SPI_PERIPHERAL_EUSART_TX_BUFFER_SIZE + 1;
    do {
      // TX bytes left = bytes in SPI plus bytes in DMA
      txLeftPrev = txLeft;
      // First, bytes remaining in LDMA
      txLeft = ((LDMA0->CH[SL_SPI_PERIPHERAL_EUSART_LDMA_TX_CHANNEL].CTRL
                 & _LDMA_CH_CTRL_XFERCNT_MASK)
                >> _LDMA_CH_CTRL_XFERCNT_SHIFT)
               + 1;
      // Then, add bytes remaining in EUSART
      txLeft += ((BTL_SPI_PERIPHERAL_EUSART->STATUS & _EUSART_STATUS_TXFCNT_MASK)
                 >> _EUSART_STATUS_TXFCNT_SHIFT);
    } while (txLeftPrev != txLeft);
    return txLeft;
  } else {
    // DMA transfer is already done, but there might be leftover bytes in EUSART
    return ((BTL_SPI_PERIPHERAL_EUSART->STATUS & _EUSART_STATUS_TXFCNT_MASK)
            >> _EUSART_STATUS_TXFCNT_SHIFT);
  }
}

/**
 * Enable/disable MISO output
 *
 * @param[in] enable True to enable the transmitter, false to disable
 * @return BOOTLOADER_OK if succesful, error code otherwise
 */
void spi_peripheral_enable_transmitter(bool enable)
{
  BTL_ASSERT(initialized == true);

  if (enable) {
    GPIO->EUSARTROUTE_SET[BTL_SPI_PERIPHERAL_EUSART_NUM].ROUTEEN = GPIO_EUSART_ROUTEEN_RXPEN;
  } else {
    GPIO->EUSARTROUTE_CLR[BTL_SPI_PERIPHERAL_EUSART_NUM].ROUTEEN = GPIO_EUSART_ROUTEEN_RXPEN;
    spi_peripheral_flush(true, false);
  }
}

/**
 * Enable/disable receiving bytes from the controller into our internal buffer.
 *
 * @param[in] enable True to enable the receiver, false to disable
 * @return BOOTLOADER_OK if succesful, error code otherwise
 */
void spi_peripheral_enable_receiver(bool enable)
{
  BTL_ASSERT(initialized == true);

  if (enable) {
    BTL_SPI_PERIPHERAL_EUSART->CMD = EUSART_CMD_RXBLOCKDIS;
  } else {
    BTL_SPI_PERIPHERAL_EUSART->CMD = EUSART_CMD_RXBLOCKEN;
  }
}

/**
 * Gets the amount of bytes ready for reading.
 *
 * @return Amount of bytes in the receive buffer available for reading
 */
size_t  spi_peripheral_get_rx_available_bytes(void)
{
  size_t ldmaHead;
  uint32_t dst;

  BTL_ASSERT(initialized == true);

  // Get destination address for next transfer
  dst = LDMA0->CH[SL_SPI_PERIPHERAL_EUSART_LDMA_RX_CHANNEL].DST;

  if (dst == 0x0101) {
    // SYNC descriptor with bit 0 of MATCHEN and MATCHVAL set
    ldmaHead = 0;
  } else if (dst == 0x0202) {
    // SYNC descriptor with bit 1 of MATCHEN and MATCHVAL set
    ldmaHead = SL_SPI_PERIPHERAL_EUSART_RX_BUFFER_SIZE / 2;
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
    return SL_SPI_PERIPHERAL_EUSART_RX_BUFFER_SIZE - (rxHead - ldmaHead);
  }
}

/**
 * Read from the RX buffer into a local buffer.
 *
 * @param[out] buffer The data buffer to receive into
 * @param[in] requestedLength Amount of bytes we'd like to read
 * @param[out] receivedLength Amount of bytes read
 * @param[in] blocking Indicate whether we should wait for requestedLength
 *   bytes to be available and read before returning, or we can read out
 *   whatever is currently in the buffer and return.
 * @param[in] timeout Number of milliseconds to wait for data in blocking mode
 *
 * @return BOOTLOADER_OK if succesful, error code otherwise
 */
int32_t spi_peripheral_receive_buffer(uint8_t  * buffer,
                                      size_t   requestedLength,
                                      size_t   * receivedLength,
                                      bool     blocking,
                                      uint32_t timeout)
{
  size_t copiedBytes;

  BTL_ASSERT(initialized == true);
  BTL_ASSERT(requestedLength < SL_SPI_PERIPHERAL_EUSART_RX_BUFFER_SIZE);

  // Check whether we have enough data
  // Optional spin for timeout cycles
  if (blocking) {
    if (timeout != 0) {
      delay_milliseconds(timeout, false);
    }

    while (spi_peripheral_get_rx_available_bytes() < requestedLength) {
      if ((timeout != 0) && delay_expired()) {
        break;
      }
    }
  }

  if (requestedLength > spi_peripheral_get_rx_available_bytes()) {
    requestedLength = spi_peripheral_get_rx_available_bytes();
  }

  // Copy up to requested bytes to given buffer
  copiedBytes = 0;
  while (copiedBytes < requestedLength) {
    buffer[copiedBytes] = rxBuffer[rxHead];
    copiedBytes++;
    rxHead++;

    if (rxHead == SL_SPI_PERIPHERAL_EUSART_RX_BUFFER_SIZE) {
      rxHead = 0;
      // Completed processing of second half of the buffer, mark it as available
      // for LDMA again by setting SYNC[1]
      sync_buffer(1U);
    } else if (rxHead == SL_SPI_PERIPHERAL_EUSART_RX_BUFFER_SIZE / 2) {
      // Completed processing of first half of the buffer, mark it as available
      // for LDMA again by setting SYNC[0]
      sync_buffer(0U);
    }
  }

  if ((uint32_t)receivedLength != 0UL) {
    *receivedLength = copiedBytes;
  }

  return BOOTLOADER_OK;
}

/**
 * Get one byte from the SPI peripheral in a blocking fashion.
 *  Warning: if the controller never clocks in a byte,
 *  this function will block forever!
 *
 * @param[out] byte The byte to send
 *
 * @return BOOTLOADER_OK if succesful, error code otherwise
 */
int32_t spi_peripheral_receive_byte(uint8_t* byte)
{
  return spi_peripheral_receive_buffer(byte, 1, (size_t*)0UL, true, 0);
}

/**
 * Flushes one or both buffers.
 *
 * @param[in] flushTx Flushes the transmit buffer when true
 * @param[in] flushRx Flushes the receive buffer when true
 */
void spi_peripheral_flush(bool flushTx, bool flushRx)
{
  BTL_ASSERT(initialized == true);

  if (flushTx) {
#if defined(_LDMA_CHDIS_MASK)
    LDMA0->CHDIS = (1 << SL_SPI_PERIPHERAL_EUSART_LDMA_TX_CHANNEL);
#else
    LDMA0->CHEN &= ~(1 << SL_SPI_PERIPHERAL_EUSART_LDMA_TX_CHANNEL);
#endif
    LDMA0->CHDONE &= ~(1 << SL_SPI_PERIPHERAL_EUSART_LDMA_TX_CHANNEL);

    BTL_SPI_PERIPHERAL_EUSART->CMD |= EUSART_CMD_CLEARTX;
    while ((BTL_SPI_PERIPHERAL_EUSART->STATUS & EUSART_STATUS_CLEARTXBUSY) != 0U) ;

    // Read data until FIFO is emptied
    // but taking care not to underflow the receiver
    while (BTL_SPI_PERIPHERAL_EUSART->STATUS & EUSART_STATUS_RXFL) {
      BTL_SPI_PERIPHERAL_EUSART->RXDATA;
    }
  }

  if (flushRx) {
#if defined(_LDMA_CHDIS_MASK)
    LDMA0->CHDIS = (1 << SL_SPI_PERIPHERAL_EUSART_LDMA_RX_CHANNEL);
#else
    LDMA0->CHEN &= ~(1 << SL_SPI_PERIPHERAL_EUSART_LDMA_RX_CHANNEL);
#endif
    LDMA0->CHDONE &= ~(1 << SL_SPI_PERIPHERAL_EUSART_LDMA_RX_CHANNEL);
    LDMA0->CH[SL_SPI_PERIPHERAL_EUSART_LDMA_RX_CHANNEL].LINK = (uint32_t)(&ldmaRxDesc[0])
                                                               & _LDMA_CH_LINK_LINKADDR_MASK;
    rxHead = 0;
    LDMA0->LINKLOAD = (1 << SL_SPI_PERIPHERAL_EUSART_LDMA_RX_CHANNEL);

    // Mark second half of RX buffer as ready
    sync_buffer(1U);
  }
}
