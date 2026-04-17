/***************************************************************************//**
 * @brief NCP init functions
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

#ifndef __NCP_INIT_H__
#define __NCP_INIT_H__

/**
 * Event Handler for loading a security key from the NVM at startup
 */
void sli_connect_ncp_key_loader_handler(void);

/**
 * @brief Init event to load the security key if it exists
 */
void sli_connect_init_ncp_security(void);

#endif
