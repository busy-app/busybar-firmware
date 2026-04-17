/***************************************************************************//**
 * @file
 * @brief interface for adapting the form-and-join library to run on
 * both an EZSP host processor and on a Zigbee SoC such as the EM250.
 * Application writers do not need to look at this file.  See
 * form-and-join.h for information on using the library.
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

//The sl_zigbee_zigbee_network_t struct type does not include lqi and rssi, but those
// values are typically desired as well.  Rather than changing the stack API,
// bundle them together in a new struct.
typedef struct {
  sl_zigbee_zigbee_network_t network;
  uint8_t lqi;
  int8_t rssi;
} sli_zigbee_network_info_t;

// Functions the adapters must implement.
uint8_t        formAndJoinStackProfile(void);
sli_zigbee_network_info_t *formAndJoinGetNetworkPointer(uint8_t index);
void         formAndJoinSetCleanupTimeout(void);
bool       formAndJoinAllocateBuffer(void);
uint8_t*   getFormAndJoinBufferPointer(void);
void         formAndJoinReleaseBuffer(void);
sl_status_t  formAndJoinSetBufferLength(uint8_t entryCount);

// The node implementation relies on the sli_zigbee_network_info_t struct being 16 bytes, in
// order to place two of them into a 32 byte sli_buffer_manager_buffer_t.  This compile
// time check is courtesy of http://www.jaggersoft.com/pubs/CVu11_5.html.
#ifndef EZSP_HOST
extern char require_16_bytes[(sizeof(sli_zigbee_network_info_t) == 16) ? 1 : -1];
#endif
