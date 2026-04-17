/***************************************************************************//**
 * @file
 * @brief APIs and defines for the Meter Mirror plugin, which handles mirroring
 *        attributes of a sleepy end device.
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

#include "meter-mirror-config.h"

/**
 * @defgroup meter-mirror  Meter Mirror Server
 * @ingroup component cluster
 * @brief API and Callbacks for the Meter Mirror Cluster Server Component
 *
 * A component for mirroring attributes of a sleepy meter in a Smart Energy network.
 * The device is expected to be a configured as mirror server on as many
 * endpoints as are allocated as meters.  Those endpoints must be contiguous.
 *
 */

/**
 * @addtogroup meter-mirror
 * @{
 */

// A bit confusing, the SL_ZIGBEE_ZCL_MANUFACTURER_CODE is actually the manufacturer
// code defined in AppBuilder.  This is usually the specific vendor of
// the local application.  It does not have to be "Ember's" (Silabs) manufacturer
// code, but that is the default.
#define EM_AF_APPLICATION_MANUFACTURER_CODE SL_ZIGBEE_ZCL_MANUFACTURER_CODE

extern sl_802154_long_addr_t nullEui64;

#define EM_AF_MIRROR_ENDPOINT_END                  \
  (SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_ENDPOINT_START \
   + SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS)

/**
 * @name API
 * @{
 */

/** @brief Get EUI64 by endpoint.
 *
 * @param endpoint Ver.: always
 * @param returnEui64 Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_meter_mirror_get_eui64_by_endpoint(uint8_t endpoint,
                                                     sl_802154_long_addr_t returnEui64);

/** @brief Mirror used status.
 *
 * @param endpoint Ver.: always
 *
 * @return bool Mirror used state
 *
 */
bool sl_zigbee_af_meter_mirror_is_mirror_used(uint8_t endpoint);

/** @brief Get endpoint by EUI64.
 *
 * @param eui64 Ver.: always
 * @param returnEndpoint Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_meter_mirror_get_endpoint_by_eui64(sl_802154_long_addr_t eui64,
                                                     uint8_t *returnEndpoint);

/** @brief Request mirror.
 *
 * @param requestingDeviceIeeeAddress Ver.: always
 *
 * @return uint16_t Endpoint if valid
 *
 */
uint16_t sl_zigbee_af_meter_mirror_request_mirror(sl_802154_long_addr_t requestingDeviceIeeeAddress);

/** @brief Remove mirror.
 *
 * @param requestingDeviceIeeeAddress
 *
 * @return uint16_t endpoint if success
 *
 */
uint16_t sl_zigbee_af_meter_mirror_remove_mirror(sl_802154_long_addr_t requestingDeviceIeeeAddress);
/** @} */ // end of name API

/**
 * @name Callbacks
 * @{
 */

/**
 * @defgroup meter_mirror_cb Meter Mirror
 * @ingroup af_callback
 * @brief Callbacks for Meter Mirror Component
 *
 */

/**
 * @addtogroup meter_mirror_cb
 * @{
 */

/** @brief Mirror is added.
 *
 * This function is called by the Meter Mirror plugin whenever a RequestMirror
 * command is successfully processed.
 *
 * @param requestingDeviceIeeeAddress   Ver.: always
 * @param endpoint   Ver.: always
 */
void sl_zigbee_af_meter_mirror_mirror_added_cb(const sl_802154_long_addr_t requestingDeviceIeeeAddress,
                                               uint8_t endpoint);

/** @brief Mirror is removed.
 *
 * This function is called by the Meter Mirror plugin whenever a RemoveMirror
 * command is successfully processed.
 *
 * @param requestingDeviceIeeeAddress   Ver.: always
 * @param endpoint   Ver.: always
 */
void sl_zigbee_af_meter_mirror_mirror_removed_cb(const sl_802154_long_addr_t requestingDeviceIeeeAddress,
                                                 uint8_t endpoint);

/** @brief Reporting is complete.
 *
 * This function is called by the Meter Mirror plugin after processing an
 * AttributeReportingStatus attribute set to ReportingComplete. If the
 * application needs to do any post attribute reporting processing, it can do it
 * from within this callback.
 *
 * @param endpoint   Ver.: always
 */
void sl_zigbee_af_meter_mirror_reporting_complete_cb(uint8_t endpoint);

/** @} */ // end of meter_mirror_cb
/** @} */ // end of name Callbacks
/** @} */ // end of meter-mirror

uint8_t sli_zigbee_af_meter_mirror_get_mirrors_allocated(void);
