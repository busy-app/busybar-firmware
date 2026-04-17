/******************************************************************************
* @file sl_si91x_peripheral_i2c.h
* @brief I2C peripheral API implementation
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SI91X_PERIPHERAL_I2C_H
#define SL_SI91X_PERIPHERAL_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"
#include "rsi_ccp_common.h"

#ifndef UNUSED_VARIABLE
#define UNUSED_VARIABLE(x) (void)(x)
#endif // UNUSED_VARIABLE

/***************************************************************************/ /**
 * @addtogroup I2C-PERIPHERAL-DRIVER I2C Peripheral Driver
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 * 
 ******************************************************************************/
// -----------------------------------------------------------------------------
// Macros
#define SL_I2C_ASSERT(expr) ((void)(expr)) ///< Macro function to assert the I2C instance
#define I2C_NUM(ref) \
  (((ref) == I2C0) ? 0 : ((ref) == I2C1) ? 1 : ((ref) == ULP_I2C) ? 2 : -1) ///< Macro to validate I2C instance number
#define I2C_REF_VALID(ref) (I2C_NUM(ref) != -1) ///< Macro function to validate the user input and I2C instance

#define SL_I2C_ENABLE_MASK       0x01   ///< I2C enable mask
#define SL_I2C_WRITE_MASK        0x00   ///< I2C write mask
#define SL_I2C_READ_MASK         0x01   ///< I2C read mask
#define SL_I2C_CLEAR_MASK        0x00   ///< I2C clear mask
#define SL_I2C_SCL_STUCK_TIMEOUT 20     ///< SCL stuck timeout, used to clear the I2C bus
#define SL_I2C_SDA_STUCK_TIMEOUT 20     ///< SDA stick timeout, used to clear the I2C bus
#define SL_16_BIT_MASK           0xFFFF ///< 16 bit mask value
#define SL_8_BIT_MASK            0xFF   ///< 8 bit mask value
#define SL_4_BIT_MASK            0xF    ///< 4 bit mask value
#define SL_I2C_DMA_ENABLE        0x1    ///< DMA enable
#define SL_I2C_DMA_DISABLE       0x0    ///< DMA disable
#define SL_MAX_7_BIT_ADDRESS     127    ///< Max 7-bit address
#define SL_BIT_SET               1      // Set bit
#define SL_STOP_BIT              9      // Bit to send stop command
#define SL_MASK_READ_BIT         8      // Bit to mask read and write

// I2C Interrupt Events
#define SL_I2C_EVENT_RECEIVE_UNDER (1UL << 0) ///< If processor attempts to read receive buffer when buffer is empty
#define SL_I2C_EVENT_RECEIVE_OVER \
  (1UL << 1) ///< If receive buffer is full and another byte is received (extra bytes are lost)
#define SL_I2C_EVENT_RECEIVE_FULL   (1UL << 2) ///< When receive buffer is full up to the RX_TL threshold level
#define SL_I2C_EVENT_TRANSMIT_OVER  (1UL << 3) ///< If transmit buffer is full and user attempts to transmit another byte
#define SL_I2C_EVENT_TRANSMIT_EMPTY (1UL << 4) ///< When transmit buffer is filled over the TX_TL threshold level
#define SL_I2C_EVENT_READ_REQ       (1UL << 5) ///< SCL is on hold in follower mode when another leader attempts to read data
#define SL_I2C_EVENT_TRANSMIT_ABORT \
  (1UL << 6) ///< Transmit is aborted, to validate the reason, read IC_TX_ABRT_SOURCE register
#define SL_I2C_EVENT_RECEIVE_DONE (1UL << 7) ///< In follower mode if leader does not acknowledge, happens on last byte
#define SL_I2C_EVENT_ACTIVITY_ON_BUS \
  (1UL << 8) ///< Ongoing or previous activity was there on bus, to clear read IC_CLR_INTR / IC_CLR_ACTIVITY register
#define SL_I2C_EVENT_STOP_DETECT (1UL << 9) ///< Stop condition is detected on the bus (leader and follower both mode)
#define SL_I2C_EVENT_START_DETECT \
  (1UL << 10) ///< Start or Restart condition is detected on the bus (leader and follower both mode)
#define SL_I2C_EVENT_GENERAL_CALL \
  (1UL << 11) ///< General call address is received, to clear read IC_CLR_GEN_CALL register
#define SL_I2C_EVENT_RESTART_DET      (1UL << 12) ///< Restart condition is detected on bus (follower mode)
#define SL_I2C_EVENT_MST_ON_HOLD      (1UL << 13) ///< Leader is holding the bus and FIFO is empty
#define SL_I2C_EVENT_SCL_STUCK_AT_LOW (1UL << 14) ///< SCL line is stuck at low

// I2C Abort Interrupts flags
#define SL_I2C_ABORT_7B_ADDRESS_NOACK   (1UL << 0) ///< Address sent was not acknowledged by follower in 7-bit mode
#define SL_I2C_ABORT_10B_ADDRESS1_NOACK (1UL << 1) ///< First 10-bit address byte sent was not acknowledged by follower
#define SL_I2C_ABORT_10B_ADDRESS2_NOACK (1UL << 2) ///< Second 10-bit address byte sent was not acknowledged by follower
#define SL_I2C_ABORT_TX_DATA_NOACK \
  (1UL << 3) ///< Received acknowledgement for address bu did not receive acknowledgement for data
#define SL_I2C_ABORT_GENERAL_CALL_NOACK    (1UL << 4) ///< No follower acknowledged General call
#define SL_I2C_ABORT_GENERAL_CALL_READ     (1UL << 5) ///< Leader send general call but program was to read it from bus
#define SL_I2C_ABORT_HIGH_SPEED_ACK        (1UL << 6) ///< High speed leader call was acknowledged by follower
#define SL_I2C_ABORT_START_BYTE_ACK        (1UL << 7) ///< Start byte was acknowledged by follower
#define SL_I2C_ABORT_HIGH_SPEED_NO_RESTART (1UL << 8) ///< Restart is disabled and data transfer is in high speed mode
#define SL_I2C_ABORT_START_BYTE_NO_RESTART (1UL << 9) ///< Restart is disabled and user tries to send start byte
#define SL_I2C_ABORT_10B_READ_NO_RESTART \
  (1UL << 10) ///< Restart is disabled and 10-bit address read command is received
#define SL_I2C_ABORT_MASTER_DISABLED            (1UL << 11) ///< Leader mode is disabled and leader mode operation is preformed
#define SL_I2C_ABORT_MASTER_ARBITRATION_LOST    (1UL << 12) ///< Leader has lost arbitration
#define SL_I2C_ABORT_SLAVE_FLUSH_TX_FIFO        (1UL << 13) ///< Follower flushes old TX FIFO to respond new read command
#define SL_I2C_ABORT_SLAVE_ARBITRATION_LOST     (1UL << 14) ///< Follower lost the bus
#define SL_I2C_ABORT_SLAVE_READ_INTX            (1UL << 15) ///< Follower requests for data and user writes 1 in CMD register
#define SL_I2C_ABORT_USER_ABORT                 (1UL << 16) ///< Leader has detected transfer abort
#define SL_I2C_ABORT_SDA_STUCK_AT_LOW           (1UL << 17) ///< SDA is stuck at low
#define SL_I2C_ABORT_DEVICE_NOACK               (1UL << 18) ///< No acknowledgement for Device ID transfer by leader
#define SL_I2C_ABORT_DEVICE_SLAVE_ADDRESS_NOACK (1UL << 19) ///< No acknowledgement for follower address to leader
#define SL_I2C_ABORT_WRITE                      (1UL << 20) ///< Leader sends device ID transfer and TX FIFO has write command
#define SL_I2C_TX_TX_FLUSH_CNT                  (1UL << 23) ///< TX data flush count due to TX Abort interrupt
// I2C status registers flags
#define SL_I2C_ACTIVITY_STATUS               (1U << 0) ///< 'OR' of Leader and Follower activity
#define SL_I2C_TRANSFER_FIFO_NOT_FULL_STATUS (1U << 1) ///< Transmit FIFO is not full
#define SL_I2C_TRANSFER_FIFO_EMPTY_STATUS    (1U << 2) ///< Transmit FIFO is completely empty
#define SL_I2C_RECEIVE_FIFO_NOT_EMPTY_STATUS (1U << 3) ///< Receive FIFO is not empty
#define SL_I2C_RECEIVE_FIFO_FULL_STATUS      (1U << 4) ///< Receive FIFO is completely full
#define SL_I2C_MASTER_ACTIVITY_STATUS        (1U << 5) ///< Leader is not in idle state
#define SL_I2C_SLAVE_ACTIVITY_STATUS         (1U << 6) ///< Follower is not in idle state
#define SL_I2C_MASTER_HOLD_TX_FIFO_EMPTY_STATUS \
  (1U << 7) ///< Leader holds SCL as previous command did not have a STOP bit
#define SL_I2C_MASTER_HOLD_RX_FIFO_FULL_STATUS \
  (1U << 8) ///< Leader holds SCL as RX FIFO is full and additional byte is received
#define SL_I2C_SLAVE_HOLD_TX_FIFO_EMPTY_STATUS \
  (1U << 9) ///< Follower holds SCL to request read data when TX FIFO is empty
#define SL_I2C_SLAVE_HOLD_RX_FIFO_FULL_STATUS \
  (1U << 10) ///< Follower holds SCL as RX FIFO is full and additional byte is received
#define SL_I2C_SDA_STUCK_NOT_RECOVERED_STATUS (1U << 11) ///< SDA is stuck at low and has not recovered

// -----------------------------------------------------------------------------
// Data Types
typedef I2C0_Type I2C_TypeDef; ///< Renaming the I2C register set

/// @brief Enumeration for I2C bus speed
typedef enum {
  SL_I2C_STANDARD_BUS_SPEED = 1, ///< Standard bus speed, 100 Kbps
  SL_I2C_FAST_BUS_SPEED,         ///< Fast bus speed, 400 Kbps
  SL_I2C_HIGH_BUS_SPEED,         ///< High bus speed, 3.4 Mbps
  SL_I2C_FAST_PLUS_BUS_SPEED,    ///< Fast plus bus speed, 1 Mbps
} sl_i2c_clock_hlr_t;

/// @brief Enumeration for I2C direction i.e., read/write
typedef enum {
  SL_I2C_WRITE = 0, ///< I2C write operation mask
  SL_I2C_READ  = 1, ///< I2C read operation mask
} sl_i2c_direction_t;

/// @brief Enumeration for I2C mode i.e., leader/follower
typedef enum {
  SL_I2C_LEADER_MODE,   ///< I2C Leader mode
  SL_I2C_FOLLOWER_MODE, ///< I2C Follower mode
} sl_i2c_mode_t;

/// @brief Structure to hold the I2C initialization parameters
typedef struct {
  sl_i2c_mode_t mode;      ///< I2C mode
  uint32_t freq;           ///< I2C SCL frequency
  sl_i2c_clock_hlr_t clhr; ///< I2C bus speed
} sl_i2c_init_params_t;

/// @brief Structure to hold the versions number of peripheral API
typedef struct {
  uint8_t release; ///< Release version number
  uint8_t major;   ///< Major version number
  uint8_t minor;   ///< Minor version number
} sl_i2c_version_t;

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
 * Initialize the I2C i.e., set the parameters to the respective registers.
 * It needs to be called at the beginning of the code.
 * It sets the frequency as per the input given in freq_scl and chlr.
 * It accepts the following members for initialization parameters:
 *  - mode (Leader/Follower) \ref sl_i2c_mode_t.
 *  - freq_scl (I2C SCL frequency).
 *  - chlr (Bus Speed) \ref sl_i2c_clock_hlr_t.
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * @param[in] p_config (sl_i2c_init_params_t) Pointer to the 
 *                      initialization parameters structure.
 * @return none
 ******************************************************************************/
void sl_si91x_i2c_init(I2C_TypeDef *i2c, const sl_i2c_init_params_t *p_config);

/***************************************************************************/ /**
 * Reset the I2C peripheral.
 * It resets the initialization parameters to zero
 * and clears all the interrupts.
 * It resets the target and slave addresses.
 *  
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return none
 ******************************************************************************/
void sl_si91x_i2c_reset(I2C_TypeDef *i2c);

/***************************************************************************/ /**
 * Transmit one byte on SDA line.
 * Threshold needs to be set before calling this API \ref sl_si91x_i2c_set_tx_threshold.
 * Direction needs to be selected before calling this API \ref sl_si91x_i2c_control_direction.
 * @pre \ref sl_si91x_i2c_init(); \n
 *      \ref sl_si91x_i2c_set_tx_threshold(); \n
 *      \ref sl_si91x_i2c_control_direction(); \n
 *      \ref sl_si91x_i2c_enable(); 
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * @param[in] data (uint8_t) Data that needs to be transmitted via I2C bus.
 * 
 * @return none
 ******************************************************************************/
void sl_si91x_i2c_tx(I2C_TypeDef *i2c, uint8_t data);

/***************************************************************************/ /**
 * Receive one byte on SDA line.
 * Threshold needs to be set before calling this API \ref sl_si91x_i2c_set_rx_threshold.
 * Direction needs to be selected before calling this API \ref sl_si91x_i2c_control_direction.
 * @pre \ref sl_si91x_i2c_init(); \n
 *      \ref sl_si91x_i2c_set_rx_threshold(); \n
 *      \ref sl_si91x_i2c_control_direction(); 
 *      \ref sl_si91x_i2c_enable(); 
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return data (uint8_t) Data that is received on the I2C bus.
 ******************************************************************************/
uint8_t sl_si91x_i2c_rx(I2C_TypeDef *i2c);

/***************************************************************************/ /**
 * Configure the frequency of the I2C transmission.
 * This API is called inside sl_si91x_i2c_init, but can be called separately to
 * change the frequency.
 * According the input of freq_scl frequency is calculated and filled in the
 * register appropriately as the speed selected in chlr.
 * The accepted parameters for chlr are: \ref sl_i2c_clock_hlr_t.
 *   - SL_I2C_STANDARD_BUS_SPEED = 1,    ///< Standard bus speed, 100 Kbps
 *   - SL_I2C_FAST_BUS_SPEED,            ///< Fast bus speed, 400 Kbps
 *   - SL_I2C_FAST_PLUS_BUS_SPEED,       ///< Fast plus bus speed, 1 Mbps
 *   - SL_I2C_HIGH_BUS_SPEED,            ///< High bus speed, 3.4 Mbps
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * @param[in] freq_scl (uint32_t) SCL frequency.
 * @param[in] chlr (sl_i2c_clock_hlr_t) Bus speed for I2C.
 * 
 * @return none
 ******************************************************************************/
void sl_si91x_i2c_set_frequency(I2C_TypeDef *i2c, uint32_t ref_freq, uint32_t freq_scl, sl_i2c_clock_hlr_t clhr);

/***************************************************************************/ /**
 * Fetch the frequency of I2C peripheral clock.
 * It returns uint32_t value of the SystemCoreClock frequency.
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return frequency (uint32_t) The frequency of the System.
 ******************************************************************************/
uint32_t sl_si91x_i2c_get_frequency(I2C_TypeDef *i2c);

/***************************************************************************/ /**
 * Get the release, sqa and dev version of I2C.
 * 
 * @param[in] none
 * @return (sl_i2c_version_t) type structure
 *         
 ******************************************************************************/
sl_i2c_version_t sl_si91x_i2c_get_version(void);

/***************************************************************************/ /**
 * Enable the I2C interface.
 * It sets the register bit which enabled the I2C interface.
 * @pre \ref sl_si91x_i2c_init();
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_enable(I2C_TypeDef *i2c)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // I2C enable
  i2c->IC_ENABLE_b.EN = ENABLE;
}

/***************************************************************************/ /**
 * Disable the I2C interface.
 * It clears the register bit which enabled the I2C interface.
 * @pre \ref sl_si91x_i2c_enable();
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_disable(I2C_TypeDef *i2c)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // I2C Disable
  i2c->IC_ENABLE_b.EN = DISABLE;
}

/***************************************************************************/ /**
 * Hold the SCL line low and stalls the leader until new command is available
 * in TX FIFO.
 * STOP condition is not issued after completion of transfer irrespective of
 * TX FIFO status.
 * @pre \ref sl_si91x_i2c_enable();
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_start_cmd(I2C_TypeDef *i2c)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // It do not issues the STOP byte
  i2c->IC_DATA_CMD_b.STOP = DISABLE;
}

/***************************************************************************/ /**
 * Issue the STOP after completion of transfer irrespective of TX FIFO status.
 * If TX FIFO has data, leader sends start byte and starts the transmission.
 * @pre \ref sl_si91x_i2c_enable(); \n
 *      \ref sl_si91x_i2c_start_cmd();
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_stop_cmd(I2C_TypeDef *i2c)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // It issues the STOP byte after the completion of transfer to release SCL line.
  i2c->IC_DATA_CMD_b.STOP = ENABLE;
}

/***************************************************************************/ /**
 * Get the I2C IC status.
 * The bits of status are set according to the events.
 * The macros are written in this file that can be directly used to mask the
 * value of this register.
 * @pre \ref sl_si91x_i2c_enable();
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE uint32_t sl_si91x_i2c_get_status(I2C_TypeDef *i2c)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // It returns the I2C status register
  return i2c->IC_STATUS;
}

/***************************************************************************/ /**
 * Disable the I2C interrupts.
 * It is mandatory to disable interrupts before configuring the I2C parameters.
 * @pre \ref sl_si91x_i2c_enable_interrupts
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_disable_interrupts(I2C_TypeDef *i2c, uint32_t flags)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // Verifies the I2C base address to disable the particular IRQ.
  if ((uint32_t)i2c == I2C0_BASE) {
    // Disables IRQ for instance 0
    NVIC_DisableIRQ(I2C0_IRQn);
  } else if ((uint32_t)i2c == I2C1_BASE) {
    // Disables IRQ for instance 1
    NVIC_DisableIRQ(I2C1_IRQn);
  } else if ((uint32_t)i2c == I2C2_BASE) {
    // Disables IRQ for instance 2
    NVIC_DisableIRQ(I2C2_IRQn);
  }
  UNUSED_VARIABLE(flags);
}

/***************************************************************************/ /**
 * Enable the I2C interrupts.
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_enable_interrupts(I2C_TypeDef *i2c, uint32_t flags)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // Verifies the I2C base address to enable the particular IRQ.
  if ((uint32_t)i2c == I2C0_BASE) {
    // Enables IRQ for instance 0
    NVIC_EnableIRQ(I2C0_IRQn);
  } else if ((uint32_t)i2c == I2C1_BASE) {
    // Enables IRQ for instance 1
    NVIC_EnableIRQ(I2C1_IRQn);
  } else if ((uint32_t)i2c == I2C2_BASE) {
    // Enables IRQ for instance 2
    NVIC_EnableIRQ(I2C2_IRQn);
  }
  UNUSED_VARIABLE(flags);
}

/***************************************************************************/ /**
 * Set the interrupts i.e., unmask according to the flags passed in the parameter.
 * @pre \ref sl_si91x_i2c_disable_interrupts();
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * @param[in] flags (uint32_t) Interrupt flags which needs to be unmasked.
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_set_interrupts(I2C_TypeDef *i2c, uint32_t flags)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // It performs "OR" operation of the register with flags provided in the parameter.
  i2c->IC_INTR_MASK |= flags;
}

/***************************************************************************/ /**
 * Clear the interrupts i.e., mask according to the flags passed in the parameter.
 * @pre \ref sl_si91x_i2c_disable_interrupts(); \n
 *      \ref sl_si91x_i2c_set_interrupts();
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * @param[in] flags (uint32_t) Interrupt flags which needs to be masked.
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_clear_interrupts(I2C_TypeDef *i2c, uint32_t flags)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // It performs "AND" operation of the register with flags provided in the parameter.
  i2c->IC_INTR_MASK &= (~flags);
}

/***************************************************************************/ /**
 * Get the pending interrupts.
 * It returns the uint32_t type interrupt status register.
 * The macros are written in this file that can be directly used to mask the
 * value of this register.
 * @pre \ref sl_si91x_i2c_enable_interrupts();
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return status (uint32_t) Interrupts which are pending.
 ******************************************************************************/
__STATIC_INLINE uint32_t sl_si91x_i2c_get_pending_interrupts(I2C_TypeDef *i2c)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // It returns the register with pending interrupts which needs to be served.
  return i2c->IC_INTR_STAT;
}

/***************************************************************************/ /**
 * Get the enabled interrupts.
 * It returns the uint32_t type interrupt mask register.
 * The macros are written in this file that can be directly used to mask the
 * value of this register.
 * @pre \ref sl_si91x_i2c_set_interrupts();
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return status (uint32_t) Interrupts which are unmasked.
 ******************************************************************************/
__STATIC_INLINE uint32_t sl_si91x_i2c_get_enabled_interrupts(I2C_TypeDef *i2c)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // It returns the registers with enabled interrupts which need to be triggered.
  return i2c->IC_INTR_MASK;
}

/***************************************************************************/ /**
 * Get the enabled and pending interrupts.
 * It returns the uint32_t type value which is bitwise end of enabled interrupts
 * and pending interrupts.
 * The macros are written in this file that can be directly used to mask the
 * value of this register.
 * @pre \ref sl_si91x_i2c_set_interrupts(); \n
 *      \ref sl_si91x_i2c_enable_interrupts();
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return status (uint32_t) Interrupts which are unmasked and pending.
 ******************************************************************************/
__STATIC_INLINE uint32_t sl_si91x_i2c_get_enabled_pending_interrupts(I2C_TypeDef *i2c)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // Stores enabled interrupts
  uint32_t enabled_interrupts = i2c->IC_INTR_MASK;
  // Stores pending interrupts
  uint32_t pending_interrupts = i2c->IC_INTR_STAT;
  // "AND" operation is performed between enabled and pending interrupts.
  return (enabled_interrupts & pending_interrupts);
}

/***************************************************************************/ /**
 * Wait till the process is completed.
 * Mask takes the interrupt flags and waits until it is triggered.
 * Interrupt is cleared once it is triggered.
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * @param[in] mask (uint32_t) Interrupt mask for that it waits. 
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_wait_ready(I2C_TypeDef *i2c, uint32_t mask)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // Clears the pending interrupts.
  sl_si91x_i2c_clear_interrupts(i2c, mask);
}

/***************************************************************************/ /**
 * Wait till the synchronization is completed.
 * It checks whether the I2C interface is enabled and the bus is free i.e.,
 * no activity is pending on the bus.
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_wait_sync(I2C_TypeDef *i2c)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // Wait for all synchronizations to finish.
  while (!(i2c->IC_ENABLE_b.EN & SL_I2C_ENABLE_MASK) && (i2c->IC_STATUS_b.ACTIVITY))
    ;
}

/***************************************************************************/ /**
 * Set the follower address when I2C is in leader mode.
 * It validates the is_10bit_addr parameter and sets the 10 bit address register.
 * It updates the register of follower address with the value passed in the parameter.
 * This API is called in sl_si91x_i2c_init function \ref sl_si91x_i2c_init
 * It can also be called separately from to update the follower address.
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * @param[in] address (uint16_t) Follower address.
 * @param[in] is_10bit_addr (bool) boolean value for the 10 bit address.
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_set_follower_address(I2C_TypeDef *i2c, uint16_t addr, bool is_10bit_addr)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // Validates if the address is 10 bit or not
  if (is_10bit_addr) {
    // If address is of 10 bit, register is configured accordingly.
    i2c->IC_TAR_b.IC_10BITADDR_MASTER = ENABLE;
  } else {
    // If address is not of 10 bit, register is configured accordingly.
    i2c->IC_TAR_b.IC_10BITADDR_MASTER = DISABLE;
  }
  // Follower address is set in the target address.
  i2c->IC_TAR_b.IC_TAR = (unsigned int)(addr & 0x3FF);
}

/***************************************************************************/ /**
 * Fetch the current follower address.
 * It returns uint8_t value as the current follower address.
 * @pre \ref sl_si91x_i2c_set_follower_address();
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return address(uint8_t) address of the follower.
 ******************************************************************************/
__STATIC_INLINE uint16_t sl_si91x_i2c_get_follower_address(I2C_TypeDef *i2c)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // It returns the follower address stored in the register.
  return i2c->IC_TAR_b.IC_TAR;
}

/***************************************************************************/ /**
 * Update the address of leader when it is in follower mode.
 * It updates the register of follower address with the value passed in the parameter.
 * This API is called in sl_si91x_i2c_init function \ref sl_si91x_i2c_init
 * It can also be called separately from to update the address.
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_set_follower_mask_address(I2C_TypeDef *i2c, uint16_t mask)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // Enabling 10-bit address mode for follower, if address greater than max 7-bit address
  if (mask > SL_MAX_7_BIT_ADDRESS) {
    i2c->IC_CON_b.IC_10BITADDR_SLAVE = ENABLE;
  } else {
    i2c->IC_CON_b.IC_10BITADDR_SLAVE = DISABLE;
  }
  // It stores the address in follower address register, used when leader acts as
  // follower.
  i2c->IC_SAR_b.IC_SAR = (unsigned int)((mask & 0x3FF) & 0x03FF);
}

/***************************************************************************/ /**
 * Fetch the masked follower address i.e. leader address when it is in follower mode.
 * It returns uint8_t value as the current masked follower address.
 * @pre \ref sl_si91x_i2c_set_follower_mask_address();
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return address(uint8_t) leader's address when it is in follower mode
 ******************************************************************************/
__STATIC_INLINE uint16_t sl_si91x_i2c_get_follower_mask_address(I2C_TypeDef *i2c)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // It returns the leader address when it is in follower mode.
  return i2c->IC_SAR_b.IC_SAR;
}

/***************************************************************************/ /**
 * Generate NACK only when it is in follower-receiver mode.
 * It generates NACK after a byte is received and the transfer will be aborted.
 * @pre \ref sl_si91x_i2c_enable();
 *      \ref sl_si91x_i2c_inti();
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_send_nack(I2C_TypeDef *i2c)
{
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // It generates NACK after a byte is received.
  // If this bit is not set then normal transfer is performed.
  i2c->IC_SLV_DATA_NACK_ONLY = ENABLE;
}

/***************************************************************************/ /**
 * Abort the transfer.
 * It disables the interrupts, I2C interface and waits till the abort is
 * completed.
 * Abort issues the stop condition and flushes the TX FIFO and sets the
 * SL_I2C_EVENT_TRANSMIT_ABORT interrupt after the operation is completed.
 * For abort it is necessary to keep to I2C instance in enable mode.
 * @pre \ref sl_si91x_i2c_enable();
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_abort_transfer(I2C_TypeDef *i2c)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // It disables all the interrupts.
  sl_si91x_i2c_disable_interrupts(i2c, 0);
  // Abort is initiated.
  i2c->IC_ENABLE_b.ABORT = ENABLE;
  // Interrupts are enabled.
  sl_si91x_i2c_enable_interrupts(i2c, 0);
  // It waits till the abort is completed.
  sl_si91x_i2c_wait_ready(i2c, SL_I2C_EVENT_TRANSMIT_ABORT);
}

/***************************************************************************/ /**
 * Clear the I2C bus.
 * It disables the I2C interface, loads the SCL and SDA timeout values
 * and again enabled the I2C interface.
 * @pre \ref sl_si91x_i2c_enable();
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_clear_bus(I2C_TypeDef *i2c)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // I2C is disabled.
  sl_si91x_i2c_disable(i2c);
  // Enable the bus clear feature
  i2c->IC_CON_b.BUS_CLEAR_FEATURE_CTRL = 1;
  // SCL timeout value is updated to the register.
  i2c->IC_SCL_STUCK_AT_LOW_TIMEOUT = SL_I2C_SCL_STUCK_TIMEOUT;
  // SCL timeout value is updated to the register.
  i2c->IC_SDA_STUCK_AT_LOW_TIMEOUT = SL_I2C_SDA_STUCK_TIMEOUT;
  // I2C is enabled.
  sl_si91x_i2c_enable(i2c);
}

/***************************************************************************/ /**
 * Update the RX threshold values.
 * @pre \ref sl_si91x_i2c_disable();
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * @param[in] threshold (uint8_t) Threshold value which need to be filled in FIFO.
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_set_rx_threshold(I2C_TypeDef *i2c, uint8_t threshold)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // Threshold is updated in IO FIFO.
  i2c->IC_RX_TL_b.RX_TL = threshold;
}

/***************************************************************************/ /**
 * Update the TX threshold values.
 * @pre \ref sl_si91x_i2c_disable();
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * @param[in] threshold (uint8_t) Threshold value which need to be filled in FIFO.
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_set_tx_threshold(I2C_TypeDef *i2c, uint8_t threshold)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // Threshold is updated in IO FIFO.
  i2c->IC_TX_TL_b.TX_TL = threshold;
}

/***************************************************************************/ /**
 * Control the direction of the I2C interface.
 * It updates the register with the read mask / write mask value
 * \ref sl_i2c_direction_t
 * @pre \ref sl_si91x_i2c_enable();
 * @note I2C instance should be enabled before calling this API.
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * @param[in] direction (sl_i2c_direction_t) I2C direction for operation. 
 *            - Read/write
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_control_direction(I2C_TypeDef *i2c, sl_i2c_direction_t direction)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // It sets the register with the read mask / write mask value.
  i2c->IC_DATA_CMD_b.CMD = direction;
}

/***************************************************************************/ /**
 * Configure the SDA hold time.
 * According to the direction @ref sl_i2c_direction_t, it updates the registers
 * with the time value which is passed as the function parameter.
 * It holds the SDA in the units of clock period.
 * 
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 * @param[in] direction (sl_i2c_direction_t) C direction for operation. 
 *            - Read/write.
 * @param[in] time Time which needs to be filled in the SDA hold register.
 * 
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_set_sda_hold_time(I2C_TypeDef *i2c, sl_i2c_direction_t direction, uint32_t time)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  if (direction == SL_I2C_WRITE) {
    // The register value is of 16 bit, so the time is masked with MASK_16_BIT.
    i2c->IC_SDA_HOLD_b.IC_SDA_TX_HOLD = (time & SL_16_BIT_MASK);
  }
  if (direction == SL_I2C_READ) {
    // The register value is of 8 bit, so the time is masked with MASK_8_BIT.
    i2c->IC_SDA_HOLD_b.IC_SDA_RX_HOLD = (time & SL_8_BIT_MASK);
  }
}

/***************************************************************************/ /**
 * Sets read direction and adds stop bit for I2C interface
 * It updates the register with the read mask value & adds stop bit.
 *
 * @pre \ref sl_si91x_i2c_enable();
 * @note I2C instance should be enabled before calling this API.
 *
 * @param[in] i2c (I2C_TypeDef) Pointer to the I2C instance base address.
 *
 * @return none
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_i2c_set_read_direction_and_stop_bit(I2C_TypeDef *i2c)
{
  // Validates the I2C instance with the corresponding base address of instance.
  SL_I2C_ASSERT(I2C_REF_VALID(i2c));
  // It sets the register with the read mask / write mask value.
  i2c->IC_DATA_CMD = (SL_BIT_SET << SL_STOP_BIT) | (SL_BIT_SET << SL_MASK_READ_BIT);
}
//----------------------------------------------------------------------------------------------
// void sl_i2c_si91x_set_tx_fifo_watermark(I2C_TypeDef *i2c,sl_i2c_tx_fifo_watermark_t tx_fifo_watermark);
// sl_i2c_tx_fifo_watermark_t sl_i2c_si91x_get_tx_fifo_watermark(I2C_TypeDef *i2c);
// void sl_i2c_si91x_set_rx_fifo_watermark(I2C_TypeDef *i2c,sl_i2c_rx_fifo_watermark_t rx_fifo_watermark);
// sl_i2c_rx_fifo_watermark_t sl_i2c_si91x_get_rx_fifo_watermark(I2C_TypeDef *i2c);
// void sl_i2c_si91x_flush_rx_fifo(I2C_TypeDef *i2c);
//-----------------------------------------------------------------------------------------------

/** @} (end addtogroup I2C) */

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_PERIPHERAL_I2C_H */
