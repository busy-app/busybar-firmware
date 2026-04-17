/***************************************************************************//**
 * @file
 * @brief Implementation of the ZigBee Pro security policies.
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

#ifndef SILABS_APS_SECURITY_POLICY_H
#define SILABS_APS_SECURITY_POLICY_H

#define sli_zigbee_is_security_state_initialized() \
  ((sli_zigbee_security_state_bitmask & EM_SECURITY_INITIALIZED) == EM_SECURITY_INITIALIZED)

#define sli_zigbee_get_security_state(item) ((sli_zigbee_security_state_bitmask & (item)) == (item))

#define sli_zigbee_set_security_state(item)   (sli_zigbee_modify_security_state(0, item))
#define sli_zigbee_clear_security_state(item) (sli_zigbee_modify_security_state(item, 0))

#endif // SILABS_APS_SECURITY_POLICY_H
