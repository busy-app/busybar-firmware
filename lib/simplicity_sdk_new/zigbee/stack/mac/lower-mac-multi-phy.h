/***************************************************************************//**
 * @file
 * @brief
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

#ifndef SILABS_LOWER_MAC_MULTI_PHY_H
#define SILABS_LOWER_MAC_MULTI_PHY_H

#define PHY_INDEX_UNSPECIFIED   0xFF
#define VALIDATE_PHY_INDEX_RANGE(phyIndex)  assert(phyIndex <= PHY_INDEX_PRO2PLUS)

#include "stack/mac/multi-mac.h"

#ifdef MAC_DUAL_PRESENT
#define MAC_PHY_INTERFACES_PRESENT            2
sl_status_t sli_802154phy_multi_phy_set_radio_power(uint8_t phyIndex, int8_t power);
int8_t sli_802154phy_multi_phy_get_radio_power(uint8_t phyIndex);
sl_status_t sli_802154phy_multi_phy_set_radio_channel(uint8_t phyIndex, uint8_t macPgChan);
uint8_t sli_802154phy_multi_phy_get_radio_channel(uint8_t phyIndex);
uint8_t sli_802154phy_multi_phy_get_channel(uint8_t phyIndex, uint8_t macPgChan);
void sli_802154phy_multi_phy_set_radio_node_id(uint8_t phyIndex, sl_802154_short_addr_t nodeId);
sl_802154_short_addr_t sli_802154phy_multi_phy_get_radio_node_id(uint8_t phyIndex);
void sli_802154phy_multi_phy_radio_init(uint8_t phyIndex, RadioPowerMode initialRadioPowerMode);
void sli_802154phy_multi_phy_set_radio_pan_id(uint8_t phyIndex, sl_802154_pan_id_t panId);
sl_802154_pan_id_t sli_802154phy_multi_phy_get_radio_pan_id(uint8_t phyIndex);
void sli_802154phy_multi_phy_set_coordinator(uint8_t phyIndex, bool coordinator);
void sli_802154phy_multi_phy_radio_ok_to_idle(uint8_t phyIndex);
sl_status_t sli_802154phy_multi_phy_set_radio_idle_mode(uint8_t phyIndex, RadioPowerMode mode);
RadioPowerMode sli_802154phy_multi_phy_get_radio_idle_mode(uint8_t phyIndex);
sl_status_t sli_802154phy_multi_phy_dc_send_pkt_asap(uint8_t phyIndex,
                                                     uint8_t* packet,
                                                     uint32_t limitMs,
                                                     bool callbackOnErrorStatus);
int8_t sli_802154phy_multi_phy_radio_energy_detection(uint8_t phyIndex);
bool sli_802154phy_multi_phy_radio_auto_ack_enabled(uint8_t phyIndex);
bool sli_802154phy_multi_phy_radio_check_radio(uint8_t phyIndex);
uint8_t sli_802154phy_multi_phy_radio_convert_rssi_to_ed(uint8_t phyIndex, int8_t rssiDbm);

//PHY1 declarations
sl_status_t sli_phy0_sli_802154phy_set_phy_radio_power(int8_t power);
int8_t sli_phy0_sli_802154phy_get_phy_radio_power(void);
sl_status_t sli_phy0_sli_802154phy_set_phy_radio_channel(uint8_t macPgChan);
uint8_t sli_phy0_sli_802154phy_get_phy_radio_channel(void);
uint8_t sli_phy0_sli_802154phy_get_phy_channel(uint8_t macPgChan);
void sli_phy0_sli_802154phy_radio_set_node_id(sl_802154_short_addr_t nodeId);
sl_802154_short_addr_t sli_phy0_sli_802154mac_radio_get_node_id(void);
void sli_phy0_sli_802154phy_radio_init(RadioPowerMode initialRadioPowerMode);
void sli_phy0_sli_802154phy_radio_set_pan_id(sl_802154_pan_id_t panId);
sl_802154_pan_id_t sli_phy0_sli_802154phy_radio_get_pan_id(void);
void sli_phy0_sli_802154phy_set_coordinator(bool coordinator);
void sli_phy0_sli_802154phy_radio_ok_to_idle(void);
sl_status_t sli_phy0_sli_802154phy_radio_set_idle_mode(RadioPowerMode mode);
RadioPowerMode sli_phy0_sli_802154phy_radio_get_idle_mode(void);
uint32_t sli_phy0_emPhySymbolsToUs(uint32_t symbols);
sl_status_t sli_phy0_sli_802154phy_dc_send_pkt_asap(uint8_t* packet,
                                                    uint32_t limitMs,
                                                    bool callbackOnErrorStatus);
void sli_phy0_sli_802154phy_dc_send_pkt_failed(void);
int8_t sli_phy0_sli_802154phy_radio_energy_detection(void);
bool sli_phy0_sli_802154phy_radio_auto_ack_enabled(void);
bool sli_phy0_sli_802154phy_radio_check_radio(void);
uint8_t sli_phy0_sli_802154phy_radio_calculate_link_quality(uint16_t unpackedChipErrors,
                                                            uint16_t packetLength);
uint16_t sli_phy0_sli_802154phy_radio_unpack_chip_errors(uint8_t internalChipErrors);
uint8_t sli_phy0_sli_802154phy_radio_convert_rssi_to_ed(int8_t rssiDbm);

RadioPowerMode sli_phy0_sli_802154phy_radio_get_power_status(void);
RadioPowerMode sli_phy1_sli_802154phy_radio_get_power_status(void);

//PHY2 declarations
sl_status_t sli_phy1_sli_802154phy_set_phy_radio_power(int8_t power);
int8_t sli_phy1_sli_802154phy_get_phy_radio_power(void);
sl_status_t sli_phy1_sli_802154phy_set_phy_radio_channel(uint8_t macPgChan);
uint8_t sli_phy1_sli_802154phy_get_phy_radio_channel(void);
uint8_t sli_phy1_sli_802154phy_get_phy_channel(uint8_t macPgChan);
void sli_phy1_sli_802154phy_radio_set_node_id(sl_802154_short_addr_t nodeId);
sl_802154_short_addr_t sli_phy1_sli_802154mac_radio_get_node_id(void);
void sli_phy1_sli_802154phy_radio_init(RadioPowerMode initialRadioPowerMode);
void sli_phy1_sli_802154phy_radio_set_pan_id(sl_802154_pan_id_t panId);
sl_802154_pan_id_t sli_phy1_sli_802154phy_radio_get_pan_id(void);
void sli_phy1_sli_802154phy_set_coordinator(bool coordinator);
void sli_phy1_sli_802154phy_radio_ok_to_idle(void);
sl_status_t sli_phy1_sli_802154phy_radio_set_idle_mode(RadioPowerMode mode);
RadioPowerMode sli_phy1_sli_802154phy_radio_get_idle_mode(void);
uint32_t sli_phy1_emPhySymbolsToUs(uint32_t symbols);
sl_status_t sli_phy1_sli_802154phy_dc_send_pkt_asap(uint8_t* packet,
                                                    uint32_t limitMs,
                                                    bool callbackOnErrorStatus);
void sli_phy1_sli_802154phy_dc_send_pkt_failed(void);
int8_t sli_phy1_sli_802154phy_radio_energy_detection(void);
bool sli_phy1_sli_802154phy_radio_auto_ack_enabled(void);
bool sli_phy1_sli_802154phy_radio_check_radio(void);
uint8_t sli_phy1_sli_802154phy_radio_calculate_link_quality(uint16_t unpackedChipErrors,
                                                            uint16_t packetLength);
uint16_t sli_phy1_sli_802154phy_radio_unpack_chip_errors(uint8_t internalChipErrors);
uint8_t sli_phy1_sli_802154phy_radio_convert_rssi_to_ed(int8_t rssiDbm);
#else
#include "stack/core/sl_zigbee_multi_phy.h"
#define MAC_PHY_INTERFACES_PRESENT            1
#define sli_802154phy_multi_phy_set_radio_power(phyIndex, power) sli_802154phy_set_phy_radio_power(power)
#define sli_802154phy_multi_phy_get_radio_power(phyIndex) sli_802154phy_get_phy_radio_power()
#define sli_802154phy_multi_phy_set_radio_channel(phyIndex, macPgChan) sli_802154phy_set_phy_radio_channel(macPgChan)
#define sli_802154phy_multi_phy_get_radio_channel(phyIndex) sli_mac_lower_mac_get_radio_channel(0)
#define sli_802154phy_multi_phy_get_channel(phyIndex, macPgChan) sli_802154phy_get_phy_channel(macPgChan)
#define sli_802154phy_multi_phy_set_radio_node_id(phyIndex, nodeId) sli_802154phy_radio_set_node_id(nodeId)
#define sli_802154phy_multi_phy_get_radio_node_id(phyIndex) sli_zigbee_get_radio_node_id()
#define sli_802154phy_multi_phy_radio_init(phyIndex, initialRadioPowerMode) sli_802154phy_radio_init(initialRadioPowerMode)
#define sli_802154phy_multi_phy_set_radio_pan_id(phyIndex, panId) sli_802154phy_radio_set_pan_id(panId)
#define sli_802154phy_multi_phy_get_radio_pan_id(phyIndex) sli_802154phy_radio_get_pan_id()
#define sli_802154phy_multi_phy_set_coordinator(phyIndex, coordinator) sli_802154phy_set_coordinator(coordinator)
#define sli_802154phy_multi_phy_radio_ok_to_idle(phyIndex) sli_802154phy_radio_ok_to_idle()
#define sli_802154phy_multi_phy_dc_send_pkt_asap(phyIndex, packet, limitMs, callbackOnErrorStatus) sli_802154phy_dc_send_pkt_asap(packet, limitMs, callbackOnErrorStatus)
#define sli_802154phy_multi_phy_radio_energy_detection(phyIndex) sli_mac_radio_energy_detection()
#define sli_802154phy_multi_phy_radio_auto_ack_enabled(phyIndex) sli_802154phy_radio_auto_ack_enabled()
#define sli_802154phy_multi_phy_radio_check_radio(phyIndex) sli_802154phy_radio_check_radio()
#define sli_802154phy_multi_phy_radio_convert_rssi_to_ed(phyIndex, rssiDbm) sli_802154phy_radio_convert_rssi_to_ed(rssiDbm)
#endif

#endif  //__LOWER_MAC_MULTI_PHY_H__
