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

/** @brief Mirror Added
 *
 * This function is called by the Meter Mirror plugin whenever a RequestMirror
 * command is successfully processed.
 *
 * @param requestingDeviceIeeeAddress   Ver.: always
 * @param endpoint   Ver.: always
 */
WEAK(void sl_zigbee_af_meter_mirror_mirror_added_cb(const sl_802154_long_addr_t requestingDeviceIeeeAddress,
                                                    uint8_t endpoint))
{
  UNUSED_VAR(requestingDeviceIeeeAddress);
  UNUSED_VAR(endpoint);
}

/** @brief Mirror Removed
 *
 * This function is called by the Meter Mirror plugin whenever a RemoveMirror
 * command is successfully processed.
 *
 * @param requestingDeviceIeeeAddress   Ver.: always
 * @param endpoint   Ver.: always
 */
WEAK(void sl_zigbee_af_meter_mirror_mirror_removed_cb(const sl_802154_long_addr_t requestingDeviceIeeeAddress,
                                                      uint8_t endpoint))
{
  UNUSED_VAR(requestingDeviceIeeeAddress);
  UNUSED_VAR(endpoint);
}

/** @brief Reporting Complete
 *
 * This function is called by the Meter Mirror plugin after processing an
 * AttributeReportingStatus attribute set to ReportingComplete. If the
 * application needs to do any post attribute reporting processing it can do it
 * from within this callback.
 *
 * @param endpoint   Ver.: always
 */
WEAK(void sl_zigbee_af_meter_mirror_reporting_complete_cb(uint8_t endpoint))
{
  UNUSED_VAR(endpoint);
}
