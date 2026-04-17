/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef _SILABS_GREEN_POWER_AF_WRAPPER_H_
#define _SILABS_GREEN_POWER_AF_WRAPPER_H_

#include "zap-config.h"
#include "zap-command.h"
#include "zap-id.h"
#include "zap-type.h"
#include "zap-enabled-incoming-commands.h"
#if !defined SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#define SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "zap-command-structs.h"
#undef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#endif
#include "sl_service_function.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "zcl-framework-core-config.h"
#endif // SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

#include "green-power-adapter-config.h"

#endif // SL_COMPONENT_CATALOG_PRESENT

#ifdef ZCL_FIXED_ENDPOINT_COUNT
#define MAX_ENDPOINT_COUNT ZCL_FIXED_ENDPOINT_COUNT
#endif

extern uint8_t  appResponseData[SL_ZIGBEE_AF_MAXIMUM_SEND_PAYLOAD_LENGTH];
extern uint16_t appResponseLength;

#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_ADAPTER_USE_CUSTOM_ATTRIBUTE_SYSTEM == 1)
sl_zigbee_af_status_t sl_zigbee_af_read_attribute_wrapper(uint8_t endpoint,
                                                          sl_zigbee_af_cluster_id_t cluster,
                                                          sl_zigbee_af_attribute_id_t attributeID,
                                                          uint8_t mask,
                                                          uint8_t *dataPtr,
                                                          uint8_t readLength,
                                                          sl_zigbee_af_attribute_type_t *dataType);
sl_zigbee_af_status_t sl_zigbee_af_write_attribute_wrapper(uint8_t endpoint,
                                                           sl_zigbee_af_cluster_id_t cluster,
                                                           sl_zigbee_af_attribute_id_t attributeID,
                                                           uint8_t mask,
                                                           uint8_t* dataPtr,
                                                           sl_zigbee_af_attribute_type_t dataType);
sl_zigbee_af_status_t sl_zigbee_af_write_client_attribute_wrapper(uint8_t endpoint,
                                                                  sl_zigbee_af_cluster_id_t cluster,
                                                                  sl_zigbee_af_attribute_id_t attributeID,
                                                                  uint8_t* dataPtr,
                                                                  sl_zigbee_af_attribute_type_t dataType);
bool sl_zigbee_af_contains_server_wrapper(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId);
bool sl_zigbee_af_contains_client_wrapper(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId);

#define sl_zigbee_af_read_attribute sl_zigbee_af_read_attribute_wrapper
#define sl_zigbee_af_write_attribute sl_zigbee_af_write_attribute_wrapper
#define sl_zigbee_af_write_client_attribute sl_zigbee_af_write_client_attribute_wrapper
#define sl_zigbee_af_contains_server sl_zigbee_af_contains_server_wrapper
#define sl_zigbee_af_contains_client sl_zigbee_af_contains_client_wrapper

#elif defined(SL_ZIGBEE_AF_NCP) && defined(SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT)
bool sl_zigbee_af_contains_server_wrapper(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId);
bool sl_zigbee_af_contains_client_wrapper(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId);
#define sl_zigbee_af_contains_server sl_zigbee_af_contains_server_wrapper
#define sl_zigbee_af_contains_client sl_zigbee_af_contains_client_wrapper
#endif // (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_ADAPTER_USE_CUSTOM_ATTRIBUTE_SYSTEM == 1)

#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_ADAPTER_USE_CUSTOM_AF_INTERFACE == 1)
sl_zigbee_aps_frame_t *sl_zigbee_af_get_command_aps_frame_wrapper(void);
sl_status_t sl_zigbee_af_send_command_unicast_wrapper(sl_zigbee_outgoing_message_type_t type,
                                                      uint16_t indexOrDestination);

sl_status_t sl_zigbee_af_send_command_broadcast_wrapper(sl_802154_short_addr_t destination,
                                                        sl_802154_short_addr_t alias,
                                                        uint8_t sequence);

sl_status_t sl_zigbee_af_send_command_multicast_wrapper(sl_zigbee_multicast_id_t multicastId, sl_802154_short_addr_t alias, uint8_t sequence);

uint16_t sl_zigbee_af_fill_external_buffer_wrapper(uint8_t frameControl,
                                                   sl_zigbee_af_cluster_id_t clusterId,
                                                   uint8_t commandId,
                                                   const char *format,
                                                   ...);

#define sl_zigbee_af_get_command_aps_frame sl_zigbee_af_get_command_aps_frame_wrapper
#define sl_zigbee_af_send_command_unicast sl_zigbee_af_send_command_unicast_wrapper
#define sl_zigbee_af_send_command_broadcast sl_zigbee_af_send_command_broadcast_wrapper
#define sl_zigbee_af_send_command_multicast sl_zigbee_af_send_command_multicast_wrapper
#define sl_zigbee_af_fill_external_buffer sl_zigbee_af_fill_external_buffer_wrapper
#endif // (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_ADAPTER_USE_CUSTOM_AF_INTERFACE == 1)

bool sl_zigbee_af_attribute_write_access_cb(int8u endpoint,
                                            sl_zigbee_af_cluster_id_t clusterId,
                                            int16u manufacturerCode,
                                            int16u attributeId);
bool sl_zigbee_af_memory_byte_compare(const uint8_t* pointer, uint8_t count, uint8_t byteValue);
uint8_t sl_zigbee_af_get_radio_channel_wrapper(void);

#define sl_zigbee_af_get_radio_channel sl_zigbee_af_get_radio_channel_wrapper

#endif //_SILABS_GREEN_POWER_AF_WRAPPER_H_
