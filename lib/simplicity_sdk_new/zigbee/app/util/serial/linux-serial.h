/***************************************************************************//**
 * @file
 * @brief  Ember serial functionality specific to a PC with Unix library
 *    support.
 *
 * See @ref serial_comm for documentation.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/** @addtogroup serial_comm
 *@{
 */

// The normal CLI is accessible via port 0 while port 1 is usable for
// raw input.  This is often used by applications to receive a 260
// image for bootloading.
#define SERIAL_PORT_RAW 0
#define SERIAL_PORT_CLI 1

void sli_legacy_serial_set_prompt(const char* thePrompt);
void sli_legacy_serial_cleanup(void);
int sli_legacy_serial_get_input_fd(uint8_t port);
void sli_legacy_serial_send_ready_to_read(uint8_t port);

/** @} END addtogroup */
