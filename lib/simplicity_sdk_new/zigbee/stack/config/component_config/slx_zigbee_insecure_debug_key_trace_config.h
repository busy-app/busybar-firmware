/***************************************************************************//**
 * @file slx_zigbee_insecure_debug_key_trace_config.h
 * @brief configure debug of security information via several interfaces
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

// <h>ZigBee Insecure Debug configuration

#ifndef SLX_ZIGBEE_INSECURE_DEBUG_KEY_TRACE_CONFIG_H
#define SLX_ZIGBEE_INSECURE_DEBUG_KEY_TRACE_CONFIG_H

// <q SLX_ZIGBEE_INSECURE_DEBUG_DEFAULT_STATE> Insecure Debug Key Trace Default
// <i> Default: FALSE
// <i> Enables insecure debug key trace operation and the use of the slx_zigbee_insecure_debug_generate_trace() function
#define SLX_ZIGBEE_INSECURE_DEBUG_DEFAULT_STATE   0

// <q SLX_ZIGBEE_INSECURE_DEBUG_NWK_REPORT_KEY_PACKET_ENABLED> Insecure Debug Network report key packet enable
// <i> Default: TRUE
// <i> Enables insecure debug network report key tracing.  This format is understood by Wireshark decoders
#define SLX_ZIGBEE_INSECURE_DEBUG_NWK_REPORT_KEY_PACKET_ENABLED   1

// <q SLX_ZIGBEE_INSECURE_DEBUG_TRANSPORT_KEY_PACKET> Insecure Debug Transport key trace enable
// <i> Default: TRUE
// <i> Enables insecure debug aps transport key tracing.  This format is understood by Network Analyzer decoders
#define SLX_ZIGBEE_INSECURE_DEBUG_TRANSPORT_KEY_PACKET_ENABLED   1

#endif // SLX_ZIGBEE_INSECURE_DEBUG_KEY_TRACE_CONFIG_H
// </h>
// <<< end of configuration section >>>
