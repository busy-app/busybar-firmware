/***************************************************************************//**
 * @file
 * @brief Functionality specific to adding a DSA signature to the end of a
 * message.
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

#include PLATFORM_HEADER
#include "stack/include/sl_zigbee_types.h"
#include "stack/include/library.h"

const sl_zigbee_library_status_t sli_zigbee_cbke_dsa_library_status = SL_ZIGBEE_LIBRARY_IS_STUB;

sl_status_t sli_zigbee_stack_dsa_sign(sli_buffer_manager_buffer_t buffer)
{
  (void)buffer;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_status_t sli_zigbee_dsa_sign_and_send_message(uint8_t mode,
                                                 uint16_t indexOrDestination,
                                                 sl_zigbee_aps_frame_t *apsStruct,
                                                 uint8_t radius,
                                                 sli_buffer_manager_buffer_t message)
{
  (void)mode;
  (void)indexOrDestination;
  (void)apsStruct;
  (void)radius;
  (void)message;
  return SL_STATUS_NOT_AVAILABLE;
}

void sli_zigbee_dsa_sign(void)
{
}
