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

#ifndef SILABS_EZSP_ENUM_DECODE_H
#define SILABS_EZSP_ENUM_DECODE_H

const char *decode_sl_zigbee_ezsp_config_id_t(uint8_t value);
const char *decode_sl_zigbee_ezsp_policy_id_t(uint8_t value);
const char *decode_sl_zigbee_ezsp_decision_id_t(uint8_t value);
const char *decode_sl_zigbee_ezsp_mfg_token_id_t(uint8_t value);
const char *decode_sl_zigbee_ezsp_status_t(uint8_t value);
const char *decodeFrameId(uint16_t value);

#endif // __EZSP_H__
