/***************************************************************************//**
 * @file
 * @brief A sample of custom EZSP protocol.
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

/**
 * @defgroup custom-ezsp Custom EZSP
 * @ingroup component host
 * @brief API and Callbacks for the Custom EZSP Component
 *
 * This plugin provides an implementation of a custom EZSP protocol.
 * It requires NCP support for the custom EZSP commands, which is
 * typically achieved by building the NCP image including the XNCP library.
 * This plugin is NOT compatible with an system-on-a-chip (SOC) platform.
 * It is sample code that defines a protocol
 * between a custom-built XNCP sample and a host.
 *
 */

/**
 * @addtogroup custom-ezsp
 * @{
 */

#define SL_ZIGBEE_MAX_CUSTOM_EZSP_MESSAGE_PAYLOAD 119

/** @} */ // end of custom-ezsp
