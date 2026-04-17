/***************************************************************************//**
 * @file
 * @brief This defines the custom storage interface used by ota-storage-simple.c
 * to interact with the actual hardware where the images are stored.
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

bool sl_zigbee_af_custom_storage_init_cb(void);
bool sl_zigbee_af_custom_storage_read_cb(uint32_t offset,
                                         uint32_t length,
                                         uint8_t* returnData);
bool sl_zigbee_af_custom_storage_write_cb(const uint8_t* dataToWrite,
                                          uint32_t offset,
                                          uint32_t length);
