/***************************************************************************//**
 * @file app_process.h
 * @brief app_process.h
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
#ifndef APP_PROCESS_H
#define APP_PROCESS_H

void app_process_action(void);
void sl_zigbee_gpd_switch_toggle_event(void);
void sl_zigbee_gpd_switch_set_sleep(void);
void sl_zigbee_gpd_app_single_event_commission(void);

#endif  // APP_PROCESS_H
