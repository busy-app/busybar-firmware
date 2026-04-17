/***************************************************************************//**
 * @file
 * @brief APIs for interfacing multiple PHYs.
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

#ifndef SILABS_EMBER_MULTI_PHY_H
#define SILABS_EMBER_MULTI_PHY_H

typedef struct {
  uint32_t pgChannelMasks[4];
  sl_zigbee_multi_phy_nwk_config_t optionsMask;
} sl_zigbee_multi_phy_network_info_t;

extern sl_zigbee_multi_phy_network_info_t* sli_zigbee_current_phy2_network;

//Phy2 network assumed to be a SubGhz
#define sli_802154phy2_routers_allowed (sli_zigbee_current_phy2_network->optionsMask & SL_ZIGBEE_MULTI_PHY_ROUTERS_ALLOWED)
#define sli_802154phy2_broadcasts_enabled (sli_zigbee_current_phy2_network->optionsMask & SL_ZIGBEE_MULTI_PHY_BROADCASTS_ENABLED)
#define sli_802154phy2_interface_disabled (sli_zigbee_current_phy2_network->optionsMask & SL_ZIGBEE_MULTI_PHY_DISABLED)

#define MULTI_PHY_NWK_OPTION_MASK   0x003

bool sli_802154phy_multi_phy_is_up(void);
void sli_zigbee_set_multi_phy_state(sl_status_t multiPhyState);
void sli_zigbee_write_multi_phy_nwk_info_token(void);
void sli_zigbee_read_multi_phy_nwk_info_token(sl_zigbee_multi_phy_network_info_t *nwkInfo);

sl_status_t sli_802154phy_multi_phy_init(uint8_t phyIndex);
#endif  //__EMBER_MULTI_PHY_H__
