/***************************************************************************//**
 * @file af-support-host.h
 * @brief Application Framework Support Host code
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef _SILABS_AF_SUPPORT_HOST_H_
#define _SILABS_AF_SUPPORT_HOST_H_
#include "app/framework/include/af.h"

sl_zigbee_af_status_t sli_zigbee_af_support_read_attribute(sl_zigbee_af_attribute_search_record_t *attRecord,
                                                           sl_zigbee_af_attribute_type_t *dataType,
                                                           uint8_t *dataPtr,
                                                           uint16_t readLength);
sl_zigbee_af_status_t sli_zigbee_af_support_write_attribute(sl_zigbee_af_attribute_search_record_t *attRecord,
                                                            sl_zigbee_af_attribute_type_t dataType,
                                                            uint8_t *dataPtr,
                                                            bool overrideReadOnlyAndDataType,
                                                            bool justTest);

#endif //_SILABS_AF_SUPPORT_HOST_H_
