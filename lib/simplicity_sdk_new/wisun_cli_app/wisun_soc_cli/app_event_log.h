/***************************************************************************//**
 * @file app_event_log.h
 * @brief Application Event Log
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef APP_EVENT_LOG_H
#define APP_EVENT_LOG_H

#include "sl_status.h"

sl_status_t app_event_log_init(void);

void app_event_log_print(const sl_wisun_mac_address_t *address);

void app_handle_event_logger_ind(sl_wisun_evt_t *evt);

#endif  // APP_EVENT_LOG_H