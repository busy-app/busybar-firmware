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

#ifndef SILABS_MAC_INFO_ELEMENT_PARSING_H
#define SILABS_MAC_INFO_ELEMENT_PARSING_H

#include "stack/include/sl_zigbee_types_internal.h"

// This is an arbitrary limit to prevent us from looping forever and
// parsing info element fields in the MAC.
// Zigbee Pro also only currently needs 3 IEs:
//    Header Termiation IE 1
//    Payload IE (Nested MLME with EB Filter or Zigbee Nested IE)
//    Payload Termination IE
#define EM_MAC_FRAME_MAX_INFO_ELEMENT_COUNT 5

sli_802154mac_frame_info_element_parse_result sli_802154mac_parse_info_elements_in_packet(sli_zigbee_packet_header_t header,
                                                                                          sli_802154mac_info_element_field* infoElementsArray,
                                                                                          uint8_t  maxInfoElementCount,
                                                                                          uint8_t* macInfoElementsLength);

bool sli_802154mac_header_get_info_elements_length(sli_zigbee_packet_header_t header,
                                                   uint8_t* returnMacInfoElementLength);

#endif // SILABS_MAC_INFO_ELEMENT_PARSING_H
