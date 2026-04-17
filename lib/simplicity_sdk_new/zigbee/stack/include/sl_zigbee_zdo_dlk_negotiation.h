/***************************************************************************//**
 * @file sl_zigbee_zdo_dlk_negotiation.h
 * @brief declarations of ZDO interface for dynamic link key negotiation
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_ZIGBEE_ZDO_DLK_NEGOTIATION_H
#define SL_ZIGBEE_ZDO_DLK_NEGOTIATION_H

#include "stack/include/sl_zigbee_dlk_negotiation.h"
#include "stack/include/sl_zigbee_tlv_core.h"

/**
 * @brief Return true if DLK is enabled by the stack
 */
bool sl_zigbee_zdo_dlk_enabled(void);

/**
 * @brief Return the bitmask indicating the supported key negotiation methods, shared secrets
 *
 * @param method_mask A bitmask of the locally supported key negotiation methods
 * @param secret_mask A bitmask of the locally supported key negotiation shared secrets
 */
void sl_zigbee_zdo_dlk_get_supported_negotiation_parameters(sl_zigbee_dlk_supported_negotiation_method *method_mask,
                                                            sl_zigbee_dlk_negotiation_supported_shared_secret_source *secret_mask);

/**
 * @brief A callback that is fired when performing dynamic link key (DLK) operations
 * with another device to negotiate a link key for APS encryption. A default implementation
 * of this function is provided to all applications whereby a common negotiation method
 * and selected secret are chosen. Users may refer to the weak definition of this function
 * for that logic. The default definition may be overridden with a strong definition of
 * this routine, which will allow the application to decide its own selected DLK negotiation
 * method and shared secret.
 *
 * @param[in] partner The ID pair (short, long) of the partner device
 * @param[in] their_supported_methods A bitmask of the supported DLK methods that the partner supports
 * @param[in] their_supported_secrets A bitmask of the supported shared secret that the partner supports
 * @param[out] selected_method The DLK method to use
 * @param[out] selected_secret The DLK shared secret to use
 *
 * @return SL_STATUS_OK to proceed with DLK with the partner, else any error code to fall
 * back to static key request, which is the Request Key mechanism as defined by Zigbe PRO
 * Core R21.
 *
 * @note This routine runs in the context of the stack task, and thus care must be taken to
 * not cause deadlocks or other hangs in execution.
 */
sl_status_t sl_zigbee_zdo_dlk_select_negotiation_parameters_callback(sl_zigbee_address_info *partner,
                                                                     sl_zigbee_dlk_supported_negotiation_method their_supported_methods,
                                                                     sl_zigbee_dlk_negotiation_supported_shared_secret_source their_supported_secrets,
                                                                     sl_zigbee_dlk_negotiation_method *selected_method,
                                                                     sl_zigbee_dlk_negotiation_shared_secret_source *selected_secret);

/**
 * @brief Initiate a ZDO Start Key Update request, notifying the target the methods and
 * parameters to use in key negotiation
 *
 * @param target The short id of the device (not neccessarily on network) to initiate key update
 * @param selected_method The enumeration indicating the selected negotiation methods
 * @param selected_secret The enumeration indicating the selected shared secret
 * @return Status code indicating if the request is submitted successfully
 */
sl_status_t sl_zigbee_zdo_dlk_start_key_update(sl_zigbee_address_info *target,
                                               sl_zigbee_dlk_negotiation_method selected_method,
                                               sl_zigbee_dlk_negotiation_shared_secret_source selected_secret);
/**
 * @brief Initiate a ZDO Start Key Negotiation request
 *
 * @param partner the ID pair (short, long) of the negotiation partner
 * @param selected_method The enumeration indicating the selected negotiation methods
 * @param selected_secret The enumeration indicating the selected shared secret
 * @return status code indicating if the request is submitted successfully
 */
sl_status_t sl_zigbee_zdo_dlk_start_key_negotiation(sl_zigbee_address_info *partner,
                                                    sl_zigbee_dlk_negotiation_method selected_method,
                                                    sl_zigbee_dlk_negotiation_shared_secret_source selected_secret);

/**
 * @brief Testing only: callback to override values of the supported key negotiation bitmask
 */
extern void slx_zigbee_gu_zdo_dlk_override_supported_params(sl_zigbee_dlk_supported_negotiation_method *method_mask,
                                                            sl_zigbee_dlk_negotiation_supported_shared_secret_source *secret_mask);
/** @brief Testing only: fetch 16 byte overridden PSK to key_buffer
 * @internal SL_ZIGBEE_IPC_ARGS
 * {# key_buffer | length: SL_ZIGBEE_ENCRYPTION_KEY_SIZE | max: SL_ZIGBEE_ENCRYPTION_KEY_SIZE #}
 * @return status code indicating if DLK PSK override is enabled
 */
extern bool slx_zigbee_gu_zdo_dlk_override_psk_fetch(uint8_t *key_buffer);
/**
 * @brief Testing only: callback to override DLK packet
 */
extern bool slx_zigbee_gu_zdo_dlk_mangle_packet(sli_buffer_manager_buffer_t *buffer);

#endif // SL_ZIGBEE_ZDO_DLK_NEGOTIATION_H
