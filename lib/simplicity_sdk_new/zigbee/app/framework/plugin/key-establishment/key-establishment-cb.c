/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "app/framework/include/af.h"

/** @brief Key Establishment
 *
 * A callback by the key-establishment code to indicate an event has occurred.
 * For error codes this is purely a notification.  For non-error status codes
 * (besides LINK_KEY_ESTABLISHED), it is the application's chance to allow or
 * disallow the operation.  If the application returns true then the key
 * establishment is allowed to proceed.  If it returns false, then key
 * establishment is aborted.  LINK_KEY_ESTABLISHED is a notification of success.
 *
 * @param status   Ver.: always
 * @param amInitiator   Ver.: always
 * @param partnerShortId   Ver.: always
 * @param delayInSeconds   Ver.: always
 */
WEAK(bool sl_zigbee_af_key_establishment_event_cb(sl_zigbee_af_key_establishment_notify_message_t status,
                                                  bool amInitiator,
                                                  sl_802154_short_addr_t partnerShortId,
                                                  uint8_t delayInSeconds))
{
  UNUSED_VAR(status);
  UNUSED_VAR(amInitiator);
  UNUSED_VAR(partnerShortId);
  UNUSED_VAR(delayInSeconds);
  return true;
}

/** @brief Key Establishment Inter Pan
 *
 * A callback by the key-establishment code to indicate an event has occurred.
 * For error codes this is purely a notification.  For non-error status codes
 * (besides LINK_KEY_ESTABLISHED), it is the application's chance to allow or
 * disallow the operation.  If the application returns true then the key
 * establishment is allowed to proceed.  If it returns false, then key
 * establishment is aborted.  LINK_KEY_ESTABLISHED is a notification of success.
 *
 * @param status   Ver.: always
 * @param amInitiator   Ver.: always
 * @param panId   Ver.: always
 * @param eui64   Ver.: always
 * @param delayInSeconds   Ver.: always
 */
WEAK(bool sl_zigbee_af_key_establishment_inter_pan_cb(sl_zigbee_af_key_establishment_notify_message_t status,
                                                      bool amInitiator,
                                                      sl_802154_pan_id_t panId,
                                                      const sl_802154_long_addr_t eui64,
                                                      uint8_t delayInSeconds))
{
  UNUSED_VAR(status);
  UNUSED_VAR(amInitiator);
  UNUSED_VAR(panId);
  UNUSED_VAR(eui64);
  UNUSED_VAR(delayInSeconds);
  return true;
}
