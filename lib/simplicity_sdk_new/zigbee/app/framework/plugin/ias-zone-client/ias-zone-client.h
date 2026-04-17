/***************************************************************************//**
 * @file
 * @brief APIs and defines for the IAS Zone Client plugin, which keeps track of
 *        IAS Zone servers.
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

/**
 * @defgroup ias-zone-client IAS Zone Client
 * @ingroup component cluster
 * @brief API and Callbacks for the IAS Zone Cluster Client Component
 *
 * An implementation of a client for IAS Zone that configures and keeps
 * track of IAS Zone Servers. It requires extending to handle server
 * notifications that might trigger some behavior on the
 * client (e.g., alarm).
 *
 */

/**
 * @addtogroup ias-zone-client
 * @{
 */

typedef struct {
  sl_802154_long_addr_t ieeeAddress;
  sl_802154_short_addr_t nodeId;
  uint16_t zoneType;
  uint16_t zoneStatus;
  uint8_t zoneState;
  uint8_t endpoint;
  uint8_t zoneId;
} sli_zigbee_ias_zone_device_t;

extern sli_zigbee_ias_zone_device_t sl_zigbee_af_ias_zone_client_known_servers[];

#define NO_INDEX 0xFF
#define UNKNOWN_ENDPOINT 0

#define UNKNOWN_ZONE_ID 0xFF

/** @brief API to control the setting of CIE address
 *
 * @param allow
 *
 */
void sl_zigbee_ias_client_allow_set_cie_address(bool allow);

/**
 * @name Callbacks
 * @{
 */

/**
 * @defgroup ias_zon_client_cb IAS Zone Client
 * @ingroup af_callback
 * @brief Callbacks for IAS Zone Client Component
 *
 */

/**
 * @addtogroup ias_zon_client_cb
 * @{
 */

/** @brief Callback for IAS zone client ZDO.
 *
 * @param sl_zigbee_node_id node ID Ver.: always
 * @param apsFrame Ver.: always
 * @param message Ver.: always
 * @param length Ver.: always
 *
 */
void sl_zigbee_af_ias_zone_client_zdo_cb(sl_802154_short_addr_t sl_zigbee_node_id,
                                         sl_zigbee_aps_frame_t* apsFrame,
                                         uint8_t* message,
                                         uint16_t length);

/** @brief Callback for IAS zone client write attributes.
 *
 * @param clusterId Ver.: always
 * @param bufffer Ver.: always
 * @param bufLen
 *
 */
void sl_zigbee_af_ias_zone_client_write_attributes_response_cb(sl_zigbee_af_cluster_id_t clusterId,
                                                               uint8_t * buffer,
                                                               uint16_t bufLen);

/** @brief Callback for IAS zone read attributes.
 *
 * @param clusterId Ver.: always
 * @param buffer Ver.: always
 * @param bufLen Ver.: always
 *
 */
void sl_zigbee_af_ias_zone_client_read_attributes_response_cb(sl_zigbee_af_cluster_id_t clusterId,
                                                              uint8_t * buffer,
                                                              uint16_t bufLen);

/** @} */ // end of ias_zon_client_cb
/** @} */ // end of name Callbacks
/** @} */ // end of ias-zone-client

void sli_zigbee_af_clear_servers(void);
