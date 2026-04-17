/***************************************************************************//**
 * @brief Zigbee Device Database component configuration header.
 *\n*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

// <<< Use Configuration Wizard in Context Menu >>>

// <h>Zigbee Device Database configuration

// <o SL_ZIGBEE_AF_PLUGIN_DEVICE_DATABASE_MAX_DEVICES> Max Devices <10-50>
// <i> Default: 20
// <i> The maximum number of devices that can be recorded in the database.
#define SL_ZIGBEE_AF_PLUGIN_DEVICE_DATABASE_MAX_DEVICES   20

// <o SL_ZIGBEE_AF_MAX_ENDPOINTS_PER_DEVICE> Max Endpoints <1-10>
// <i> Default: 5
// <i> The maximum number of endpoints per device that can be recorded in the database. Increasing this number increases total RAM usage.
#define SL_ZIGBEE_AF_MAX_ENDPOINTS_PER_DEVICE   5

// <o SL_ZIGBEE_AF_MAX_CLUSTERS_PER_ENDPOINT> Max Clusters <1-50>
// <i> Default: 15
// <i> The maximum number of clusters per endpoint that can be recorded in the database. Increasing this number increases total RAM usage.
#define SL_ZIGBEE_AF_MAX_CLUSTERS_PER_ENDPOINT   15

// </h>

// <<< end of configuration section >>>
