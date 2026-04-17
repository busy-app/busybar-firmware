/******************************************************************************
* @file  rsi_rs485.c
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

#include "rsi_rs485.h"
#include "rsi_rom_egpio.h"
#include "rsi_error.h"

/**
 * @fn           void rsi_usart_rs485_enable(USART0_Type *usart)
 * @brief        This API used to enable the RS485 module
 * @param[in]    usart  Pointer to USART/UART register block
 * @return       None
 */
void rsi_usart_rs485_enable(USART0_Type *usart)
{
  // RS485 Enable
  usart->TCR_b.RS485_EN = 1;
}

/**
 * @fn           void rsi_usart_rs485_configure(USART0_Type *usart, usart_rs485_t usart_rs485)
 * @brief        This API used to enable the RS485 module
 * @param[in]    usart        Pointer to USART/UART register block
 * @param[in]    usart_rs485  RS485 config parameters
 * @return       None
 */
void rsi_usart_rs485_configure(USART0_Type *usart, usart_rs485_config_t usart_rs485)
{
  // Set the rs485 transfer mode
  usart->TCR_b.XFER_MODE = usart_rs485.transfer_mode;
  // Set Driver enable Assert time
  usart->DET_b.DE_ASSERT_TIME = usart_rs485.de_assert_time;
  // Set the Driver enable De-Assert time
  usart->DET_b.DE_DE_ASSERT_TIME = usart_rs485.de_deassert_time;
  // Set Driver enable to reciver enable Turnaround time
  usart->TAT_b.DE_RE = usart_rs485.tat_set.de_re_tat_set;
  // Set Receiver enable to Driver enable Turnaround time
  usart->TAT_b.RE_DE = usart_rs485.tat_set.re_de_tat_set;
  // Set Driver enable Polarity
  usart->TCR_b.DE_POL = usart_rs485.de_pol;
  // Set Receiver Enable Polarity
  usart->TCR_b.RE_POL = usart_rs485.re_pol;
  // Check multidrop feature enabled
  if (usart_rs485.multidrop_enable) {
    // Set the 9 bit data mode
    usart->LCR_EXT_b.DLS_E = 1;
    // Set the Addr match mode while receiving
    usart->LCR_EXT_b.ADDR_MATCH = usart_rs485.multidrop.addr_match;
    // Check whether transfer mode is hardware/ software controlled mode
    if (usart->TCR_b.XFER_MODE == RS485_HW_CTRL_HALF_DUPLEX_MODE) {
      // Set the multidrop transmit mode
      usart->LCR_EXT_b.TRANSMIT_MODE = RS485_TX_MODE_0;
    } else if ((usart->TCR_b.XFER_MODE == RS485_SW_CTRL_HALF_DUPLEX_MODE)
               || (usart->TCR_b.XFER_MODE == RS485_SW_CTRL_FULL_DUPLEX_MODE)) {
      // Set the multidrop transmit mode
      usart->LCR_EXT_b.TRANSMIT_MODE = RS485_TX_MODE_1;
    }
  }
}

/**
 * @fn           void rsi_usart_rs485_de_enable(USART0_Type *usart, bool enable)
 * @brief        This API used to enable/disable Driver Enable signal
 * @param[in]    usart      Pointer to USART/UART register block
 * @param[in]    enable     Driver enable. 1 - Enable 0 - Disable
 * @return       None
 */
void rsi_usart_rs485_de_enable(USART0_Type *usart, bool enable)
{
  // Enalbe the Driver enable
  usart->DE_EN_b.DE_EN = enable;
}

/**
 * @fn           void rsi_usart_rs485_re_enable(USART0_Type *usart, bool enable)
 * @brief        This API used to enable/disable Receiver Enable signal
 * @param[in]    usart      Pointer to USART/UART register block
 * @param[in]    enable     Receiver enable. 1 - Enable 0 - Disable
 * @return       None
 */
void rsi_usart_rs485_re_enable(USART0_Type *usart, bool enable)
{
  // Enable the Receiver enable
  usart->RE_EN_b.RE_EN = enable;
}

/**
 * @fn           void rsi_usart_rs485_rx_address_set(USART0_Type *usart, uint8_t rx_addr)
 * @brief        This API used to set the address to the module in receive mode .
 * @param[in]    usart      Pointer to USART/UART register block
 * @param[in]    rx_addr    Receive address to be set to the module
 * @return       None
 */
void rsi_usart_rs485_rx_address_set(USART0_Type *usart, uint8_t rx_addr)
{
  // Set the RX address in slave
  usart->RAR_b.RAR = rx_addr;
}

/**
 * @fn           void rsi_usart_rs485_transfer_address(USART0_Type *usart, usart_rs485_multidrop_t usart_rs485_multidrop, uint8_t tx_addr)
 * @brief        This API used to transfer the slave address from master. This is an address matching api during transmit mode 0. 
 *                When 9th bit in character is set to 1 and remaining 8-bit address will be sent using this API.
 * @param[in]    usart      Pointer to USART/UART register block
 * @param[in]    usart_rs485_multidrop      RS485 multidrop config
 * @param[in]    tx_addr    Address of the slave to which data to be transmitted
 * @return       None
 */
void rsi_usart_rs485_transfer_address(USART0_Type *usart,
                                      usart_rs485_multidrop_t usart_rs485_multidrop,
                                      uint8_t tx_addr)
{
  // Set the transmit address
  usart->TAR_b.TAR = tx_addr;

  // Set Send Addr bit while transfering address
  usart->LCR_EXT_b.SEND_ADDR = usart_rs485_multidrop.send_addr;
}

/**
 * @fn           void rsi_usart_rs485_resume_tx(USART0_Type *usart)
 * @brief        This API used to resume / suspend the data transfer
 * @param[in]    usart      Pointer to USART/UART register block
 * @param[in]    enable     Suspend/Resume the data transfer 1- Suspend 0 - Resume
 * @return       None
 */

void rsi_usart_rs485_resume_tx(USART0_Type *usart, bool enable)
{
  usart->HTX_b.HALT_TX = enable;
}

/**
 * @fn           void rsi_usart_rs485_control_send_address(USART0_Type *usart, bool send_addr_ctrl)
 * @brief        This API used to enable/disable send address bit while transferring address
 * @param[in]    usart      Pointer to USART/UART register block
 * @param[in]    send_addr_ctrl   send address bit control
 * @return       None
 */
void rsi_usart_rs485_control_send_address(USART0_Type *usart, bool send_addr_ctrl)
{
  // Send Addr bit while transferring address
  usart->LCR_EXT_b.SEND_ADDR = send_addr_ctrl;
}

/**
* @fn           void rsi_usart_rs485_control_address_match(USART0_Type *usart, bool addr_match_ctrl)
* @brief        This API used to enable/disable address match bit while receiving.
* @param[in]    usart      Pointer to USART/UART register block
* @param[in]    send_addr_ctrl   send address bit control
* @return       None
*/
void rsi_usart_rs485_control_address_match(USART0_Type *usart, bool addr_match_ctrl)
{
  // Set the Addr match mode while receiving
  usart->LCR_EXT_b.ADDR_MATCH = addr_match_ctrl;
}

/**
 * @brief Sets the Driver Enable assertion time for RS-485.
 * @param[in] usart Pointer to USART/UART register block.
 * @param[in] assert_time The Driver Enable assertion time value.
 * @return None.
 */
void rsi_usart_rs485_set_de_assert_time(USART0_Type *usart, uint8_t assert_time)
{
  usart->DET_b.DE_ASSERT_TIME = assert_time;
}

/**
 * @brief Sets the Driver Enable de-assertion time for RS-485.
 * @param[in] usart Pointer to USART/UART register block.
 * @param[in] deassert_time The Driver Enable de-assertion time value.
 * @return None.
 */
void rsi_usart_rs485_set_de_deassert_time(USART0_Type *usart, uint8_t deassert_time)
{
  usart->DET_b.DE_DE_ASSERT_TIME = deassert_time;
}

/**
 * @brief Sets the Driver Enable to Receiver Enable turnaround time for RS-485.
 * @param[in] usart Pointer to USART/UART register block.
 * @param[in] de_re_tat The DE to RE turnaround time value.
 * @return None.
 */
void rsi_usart_rs485_set_de_re_turnaround_time(USART0_Type *usart, uint16_t de_re_tat)
{
  usart->TAT_b.DE_RE = de_re_tat;
}

/**
 * @brief Sets the Receiver Enable to Driver Enable turnaround time for RS-485.
 * @param[in] usart Pointer to USART/UART register block.
 * @param[in] re_de_tat The RE to DE turnaround time value.
 * @return None.
 */
void rsi_usart_rs485_set_re_de_turnaround_time(USART0_Type *usart, uint16_t re_de_tat)
{
  usart->TAT_b.RE_DE = re_de_tat;
}

/**
 * @brief Sets the Driver Enable polarity for RS-485.
 * @param[in] usart Pointer to USART/UART register block.
 * @param[in] de_polarity The Driver Enable polarity (0: active low, 1: active high).
 * @return None.
 */
void rsi_usart_rs485_set_de_polarity(USART0_Type *usart, uint8_t de_polarity)
{
  usart->TCR_b.DE_POL = de_polarity;
}

/**
 * @brief Sets the Receiver Enable polarity for RS-485.
 * @param[in] usart Pointer to USART/UART register block.
 * @param[in] re_polarity The Receiver Enable polarity (0: active low, 1: active high).
 * @return None.
 */
void rsi_usart_rs485_set_re_polarity(USART0_Type *usart, uint8_t re_polarity)
{
  usart->TCR_b.RE_POL = re_polarity;
}

/**
 * @fn           void rsi_usart_rs485_deinit(USART0_Type *usart)
 * @brief        This API used to disable the RS485 module
 * @param[in]    usart      Pointer to USART/UART register block
 * @return       None
 */
void rsi_usart_rs485_disable(USART0_Type *usart)
{
  // RS485 Disable
  usart->TCR_b.RS485_EN = 0;
}
