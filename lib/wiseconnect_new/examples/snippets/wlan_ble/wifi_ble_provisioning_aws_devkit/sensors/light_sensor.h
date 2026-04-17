/***************************************************************************/ /**
 * @file light_sensor.h
 * @brief Light Sensor
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
#ifndef LIGHT_SENSOR_H_
#define LIGHT_SENSOR_H_

void light_sensor_init(void);

sl_status_t white_light_read(float *white_light_lux);

sl_status_t ambient_light_read(float *ambient_light_lux);

void light_sensor_deinit(void);

#endif /* LIGHT_SENSOR_H_ */
