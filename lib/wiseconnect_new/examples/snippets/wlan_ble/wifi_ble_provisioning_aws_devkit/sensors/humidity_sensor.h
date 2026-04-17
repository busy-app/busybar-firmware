/***************************************************************************/ /**
 * @file humidity_sensor.h
 * @brief Humidity sensor
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
#ifndef HUMIDITY_SENSOR_H_
#define HUMIDITY_SENSOR_H_

void humidity_sensor_init(void);

sl_status_t humidity_read(uint32_t *humidity);

sl_status_t temperature_read(int32_t *temperature);

#endif /* HUMIDITY_SENSOR_H_ */
