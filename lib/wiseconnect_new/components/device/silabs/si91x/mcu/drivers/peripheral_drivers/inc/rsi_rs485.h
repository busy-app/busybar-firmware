/******************************************************************************
* @file  rsi_rs485.h
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
#include "rsi_ccp_common.h"

#define RS485_SW_CTRL_FULL_DUPLEX_MODE 0 // Software controlled Full duplex mode
#define RS485_SW_CTRL_HALF_DUPLEX_MODE 1 // Software controlled Half duplex mode
#define RS485_HW_CTRL_HALF_DUPLEX_MODE 2 // Hardware controlled Half duplex mode

#define RS485_TX_MODE_0 \
  0 // THR is 8-bit wide.The user needs to program the address into TAR and data into the THR register.SEND_ADDR bit is used as a control knob to send the address.
#define RS485_TX_MODE_1 \
  1 // THR is 9-bit wide.User must ensure THR register is written correctly for address/data. Address: 9th bit is set to 1, Data: 9th bit is set to 0.

/**
 * @brief Structure to hold the rs485 multidrop configuration for USART RS485.
 *
 * This structure defines the configuration parameters for the USART RS485 multidrop including the data bit 9, address match,
 * send address, and transmit mode.
 */
typedef struct {
  uint8_t addr_match;    // Enable Address match with RAR register during receive
  uint8_t send_addr;     // Used to determine when to send the address during transmit mode
  uint8_t transmit_mode; // This bit is used to set transmit mode during 9-bit data transfers
} usart_rs485_multidrop_t;

/**
 * @brief Structure to set TurnAround Timing.
 *
 * This structure hold the DE and RE enable Turn around time 
 */
typedef struct {
  uint16_t de_re_tat_set; // Driver Enable to Receiver Enable TurnAround time
  uint16_t re_de_tat_set; // Receiver Enable to Driver Enable TurnAround time.
} usart_rs485_tat_t;

/**
 * @brief Structure to hold the usart rs485 configs.
 */
typedef struct {
  uint8_t
    transfer_mode; // Set the transfer mode 0 =  Full Duplex Mode, 1 = Software-Enabled Half Duplex Mode, 2 = Hardware enabled Half Duplex mode.
  uint8_t de_assert_time;            // Driver enable assertion time
  uint8_t de_deassert_time;          // Driver enable de-assertion time
  bool de_pol;                       // Driver Enable Polarity
  bool re_pol;                       // Receiver Enable Polarity
  bool multidrop_enable;             // Multidrop enable to confiure the multidrop parameterts
  usart_rs485_tat_t tat_set;         // Structure to set TurnAround Timing
  usart_rs485_multidrop_t multidrop; // Structure to multidrop parameters
} usart_rs485_config_t;

/*****************************************************************************************************************/ /**
 * This function initializes the RS485 required gpio's and enables RS485 communicationn.
 *
 * @param[in]usart         Pointer to the USART/UART peripheral register block.
 *
 * @return                 None.
********************************************************************************************************************/
void rsi_usart_rs485_enable(USART0_Type *usart);

/*****************************************************************************************************************/ /**
 * This function configures rs485 communication.
 *
 * @param[in]usart         Pointer to the USART/UART peripheral register block.
 * @param[in]usart_rs485   RS485 config parameters 
 *
 * @return                 None
********************************************************************************************************************/
void rsi_usart_rs485_configure(USART0_Type *usart, usart_rs485_config_t usart_rs485);

/*****************************************************************************************************************/ /**
 * This function Enable/Disable the Driver Enable.
 *
 * @param[in]usart         Pointer to the USART/UART peripheral register block.
 * @param[in]enable        Driver Enable signal enable or Disable 
 *
 * @return                 None.
********************************************************************************************************************/
void rsi_usart_rs485_de_enable(USART0_Type *usart, bool enable);

/*****************************************************************************************************************/ /**
 * This function Enable/Disable the Receiver output.
 *
 * @param[in]usart         Pointer to the USART/UART peripheral register block.
 * @param[in]enable        Receiver output enable or Disable
 *
 * @return                 RSI_OK if success
********************************************************************************************************************/
void rsi_usart_rs485_re_enable(USART0_Type *usart, bool enable);

/*****************************************************************************************************************/ /**
 * This function set address to the slave on which master can send data.
 *
 * @param[in]usart         Pointer to the USART/UART peripheral register block.
 * @param[in]rx_addr       address to be set
 *
 * @return                 None
********************************************************************************************************************/
void rsi_usart_rs485_rx_address_set(USART0_Type *usart, uint8_t rx_addr);

/*****************************************************************************************************************/ /**
 * This function check the address received or not..
 *
 * @param[in]usart         Pointer to the USART/UART peripheral register block.
 * @param[in]usart_rs485    usart_rs485      RS485 multidrop config
 * @param[in]tx_addr    Address of the slave to which data to be transmitted
 * 
 * @return                 None
********************************************************************************************************************/
void rsi_usart_rs485_transfer_address(USART0_Type *usart, usart_rs485_multidrop_t usart_rs485, uint8_t tx_addr);

/*****************************************************************************************************************/ /**
 * This function used to halt or resume the transfer.
 *
 * @details This api is used when application is required to fill the data bytes in the TxFIFO before sending the address on 
 * the uart line.Then this api need to call with enable bit set to 1 so that uart does not start sending out the data in the 
 * TxFIFO as data byte, Once the TxFIFO is filled, then call the rsi_rs485_transfer_address() api  and call this api with 
 * enable bit set as 0.
 * @param[in]usart         Pointer to the USART/UART peripheral register block.
 * @param[in]enable       resume or halt the data transmission
 *
 * @return                 None
********************************************************************************************************************/
void rsi_usart_rs485_resume_tx(USART0_Type *usart, bool enable);

/*****************************************************************************************************************/ /**
 * This function enable/disable send address bit while transferring address.
 * @param[in] usart        Pointer to the USART/UART peripheral register block.
 * @param[in]  send_addr_ctrl   send address bit control
 * @return                 None
********************************************************************************************************************/
void rsi_usart_rs485_control_send_address(USART0_Type *usart, bool send_addr_ctrl);

/*****************************************************************************************************************/ /**
* This function enable/disable address match bit while receiving.
* @param[in] usart        Pointer to the USART/UART peripheral register block.
* @param[in] addr_match_ctrl  address match bit control
* @return                 None
********************************************************************************************************************/
void rsi_usart_rs485_control_address_match(USART0_Type *usart, bool addr_match_ctrl);

/*****************************************************************************************************************/ /**
 * @brief Sets the Driver Enable assertion time for RS-485.
 *
 * This function configures the duration for which the Driver Enable (DE) signal
 * is asserted before the start of data transmission.
 *
 * @param[in] usart Pointer to the USART/UART peripheral register block.
 * @param[in] assert_time The Driver Enable assertion time value (in serial clock periods).
 * @return None.
********************************************************************************************************************/
void rsi_usart_rs485_set_de_assert_time(USART0_Type *usart, uint8_t assert_time);

/*****************************************************************************************************************/ /**
 * @brief Sets the Driver Enable de-assertion time for RS-485.
 *
 * This function configures the duration for which the Driver Enable (DE) signal
 * remains asserted after the completion of data transmission.
 *
 * @param[in] usart Pointer to the USART/UART peripheral register block.
 * @param[in] deassert_time The Driver Enable de-assertion time value (in serial clock periods).
 * @return None.
 ********************************************************************************************************************/
void rsi_usart_rs485_set_de_deassert_time(USART0_Type *usart, uint8_t deassert_time);

/*****************************************************************************************************************/ /**
 * @brief Sets the Driver Enable to Receiver Enable turnaround time for RS-485.
 *
 * This function configures the delay between de-asserting the Driver Enable (DE)
 * signal and asserting the Receiver Enable (RE) signal when switching from
 * transmit to receive mode. 
 *
 * @param[in] usart Pointer to the USART/UART peripheral register block.
 * @param[in] de_re_tat The DE to RE turnaround time value (in serial clock periods).
 * @return None.
********************************************************************************************************************/
void rsi_usart_rs485_set_de_re_turnaround_time(USART0_Type *usart, uint16_t de_re_tat);

/*****************************************************************************************************************/ /**
 * @brief Sets the Receiver Enable to Driver Enable turnaround time for RS-485.
 *
 * This function configures the delay between de-asserting the Receiver Enable (RE)
 * signal and asserting the Driver Enable (DE) signal when switching from
 * receive to transmit mode. 
 *
 * @param[in] usart Pointer to the USART/UART peripheral register block.
 * @param[in] re_de_tat The RE to DE turnaround time value (in serial clock periods).
 * @return None.
********************************************************************************************************************/
void rsi_usart_rs485_set_re_de_turnaround_time(USART0_Type *usart, uint16_t re_de_tat);

/*****************************************************************************************************************/ /**
 * @brief Sets the Driver Enable polarity for RS-485.
 *
 * This function configures the active level of the Driver Enable (DE) signal.
 *
 * @param[in] usart Pointer to the USART/UART peripheral register block.
 * @param[in] de_polarity The Driver Enable polarity:
 * - `0`: DE signal is active low.
 * - `1`: DE signal is active high.
 * @return None.
********************************************************************************************************************/
void rsi_usart_rs485_set_de_polarity(USART0_Type *usart, uint8_t de_polarity);

/*****************************************************************************************************************/ /**
 * @brief Sets the Receiver Enable polarity for RS-485.
 *
 * This function configures the active level of the Receiver Enable (RE) signal.
 *
 * @param[in] usart Pointer to the USART/UART peripheral register block.
 * @param[in] re_polarity The Receiver Enable polarity:
 * - `0`: RE signal is active low.
 * - `1`: RE signal is active high.
 * @return None.
********************************************************************************************************************/
void rsi_usart_rs485_set_re_polarity(USART0_Type *usart, uint8_t re_polarity);

/*****************************************************************************************************************/ /**
 * This function deinitialize the rs485.
 *
 * @param[in]usart        Pointer to the USART/UART peripheral register block.
 *
 * @return                 None
********************************************************************************************************************/
void rsi_usart_rs485_disable(USART0_Type *usart);
