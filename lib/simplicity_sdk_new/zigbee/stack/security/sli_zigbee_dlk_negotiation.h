/**
 * @brief Dynamic Link Key Negotiation enables support
 * for secure link key derivation using Elliptic Curve Diffie-Hellman
 * and Secure Passphrase Ephemeral Key Exchange for public-key cryptography
 * (INTERNAL)
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

#ifndef SLI_ZIGBEE_DLK_NEGOTIATION_H
#define SLI_ZIGBEE_DLK_NEGOTIATION_H

#include "stack/include/sl_zigbee_dlk_negotiation.h"

/**
 * @brief 'binds' a dlk context struct with the associated data and callbacks
 */
sl_status_t sli_zigbee_dlk_context_bind(sl_zigbee_dlk_negotiation_context_t *dlk_negotiation_ctx,
                                        sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                        sli_zigbee_event_t *dlk_event_control);

#endif // SLI_ZIGBEE_DLK_NEGOTIATION_H
