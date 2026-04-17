/***************************************************************************//**
 * @brief Zigbee Demand Response Load Control Cluster Client component configuration header.
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

// <h>Zigbee Demand Response Load Control Cluster Client configuration

// <o SL_ZIGBEE_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE> Load control event table size <3-255>
// <i> Default: 3
// <i> Maximum number of load control events in a table
#define SL_ZIGBEE_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE   3

// <o SL_ZIGBEE_AF_PLUGIN_DRLC_DEVICE_CLASS> Device class
// <i> Device class
// <SL_ZIGBEE_AF_AMI_DEVICE_CLASS_HVAC_COMPRESSOR_OR_FURNACE=> HVAC Compressor Or Furnace
// <SL_ZIGBEE_AF_AMI_DEVICE_CLASS_STRIP_HEAT_BASEBOARD_HEAT=> Strip Heat Baseboard Heat
// <SL_ZIGBEE_AF_AMI_DEVICE_CLASS_WATER_HEATER=> Water Heater
// <SL_ZIGBEE_AF_AMI_DEVICE_CLASS_POOL_PUMP_SPA_JACUZZI=> Pool Pump Spa Jacuzzi
// <SL_ZIGBEE_AF_AMI_DEVICE_CLASS_SMART_APPLIANCES=> Smart Appliances
// <SL_ZIGBEE_AF_AMI_DEVICE_CLASS_IRRIGATION_PUMP=> Irrigation Pump
// <SL_ZIGBEE_AF_AMI_DEVICE_CLASS_MANAGED_C_AND_I_LOADS=> Managed C And I Loads
// <SL_ZIGBEE_AF_AMI_DEVICE_CLASS_SIMPLE_MISC_LOADS=> Simple Misc Loads
// <SL_ZIGBEE_AF_AMI_DEVICE_CLASS_EXTERIOR_LIGHTING=> Exterior Lighting
// <SL_ZIGBEE_AF_AMI_DEVICE_CLASS_INTERIOR_LIGHTING=> Interior Lighting
// <SL_ZIGBEE_AF_AMI_DEVICE_CLASS_ELECTRIC_VEHICLE=> Electric Vehicle
// <SL_ZIGBEE_AF_AMI_DEVICE_CLASS_GENERATION_SYSTEMS=> Generation Systems
// <i> Default: SL_ZIGBEE_AF_AMI_DEVICE_CLASS_HVAC_COMPRESSOR_OR_FURNACE
// <i> Determines the class of this device
#define SL_ZIGBEE_AF_PLUGIN_DRLC_DEVICE_CLASS   SL_ZIGBEE_AF_AMI_DEVICE_CLASS_HVAC_COMPRESSOR_OR_FURNACE

// </h>

// <<< end of configuration section >>>
