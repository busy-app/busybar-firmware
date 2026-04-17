/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef _SILABS_GREEN_POWER_PRINT_WRAPPER_H_
#define _SILABS_GREEN_POWER_PRINT_WRAPPER_H_

#include "zap-command.h"
#include "zap-id.h"
#include "zap-type.h"
#include "zap-enabled-incoming-commands.h"
#if !defined SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#define SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "zap-command-structs.h"
#undef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#endif

#include "sl_service_function.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "zcl-framework-core-config.h"
#endif // SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

#include "green-power-adapter-config.h"

#endif // SL_COMPONENT_CATALOG_PRESENT

/***************************************************************************
* Print stub functions
* Customer should define these in their own application code
***************************************************************************/

#ifndef SL_ZIGBEE_TEST
#define SL_ZIGBEE_AF_PRINT_CORE 0x0001

void sl_zigbee_af_print_wrapper(uint16_t area, const char * formatString, ...);
void sl_zigbee_af_print_big_endian_eui64_wrapper(uint8_t * eui, ...);
void sl_zigbee_af_print_buffer_wrapper(uint16_t area, const uint8_t *buffer, uint16_t bufferLen, bool withSpace);
void sl_zigbee_af_core_print_wrapper(const char * formatString, ...);
void sl_zigbee_af_core_println_wrapper(const char * formatString, ...);
void sl_zigbee_af_app_println_wrapper(const char * formatString, ...);
void sl_zigbee_af_app_print_wrapper(const char * formatString, ...);
void sl_zigbee_af_debug_print_wrapper(const char * formatString, ...);
void sl_zigbee_af_debug_println_wrapper(const char * formatString, ...);
void sl_zigbee_af_green_power_cluster_print_wrapper(const char * formatString, ...);
void sl_zigbee_af_green_power_cluster_println_wrapper(const char * formatString, ...);
void sl_zigbee_af_green_power_cluster_print_buffer_wrapper(const uint8_t *buffer, uint16_t bufferLen, bool withSpace);
void sl_zigbee_af_green_power_cluster_print_string_wrapper(const uint8_t *buffer);
void sl_zigbee_af_debug_flush_wrapper(void);
void sl_zigbee_af_println_wrapper(uint16_t area, const char * formatString, ...);

#ifdef sl_zigbee_af_core_print
#undef sl_zigbee_af_core_print
#endif //sl_zigbee_af_core_print
#define sl_zigbee_af_core_print(...) sl_zigbee_af_core_print_wrapper(__VA_ARGS__)

#ifdef sl_zigbee_af_print_big_endian_eui64
#undef sl_zigbee_af_print_big_endian_eui64
#endif //sl_zigbee_af_print_big_endian_eui64
#define sl_zigbee_af_print_big_endian_eui64(...) sl_zigbee_af_print_big_endian_eui64_wrapper(__VA_ARGS__)

#ifdef sl_zigbee_af_core_println
#undef sl_zigbee_af_core_println
#endif //sl_zigbee_af_core_println
#define sl_zigbee_af_core_println(...) sl_zigbee_af_core_println_wrapper(__VA_ARGS__)

#ifdef sl_zigbee_af_green_power_cluster_print
#undef sl_zigbee_af_green_power_cluster_print
#endif //sl_zigbee_af_green_power_cluster_print
#define sl_zigbee_af_green_power_cluster_print(...) sl_zigbee_af_green_power_cluster_print_wrapper(__VA_ARGS__)

#ifdef sl_zigbee_af_green_power_cluster_println
#undef sl_zigbee_af_green_power_cluster_println
#endif //sl_zigbee_af_green_power_cluster_println
#define sl_zigbee_af_green_power_cluster_println(...) sl_zigbee_af_green_power_cluster_println_wrapper(__VA_ARGS__)

#ifdef sl_zigbee_af_green_power_cluster_print_buffer
#undef sl_zigbee_af_green_power_cluster_print_buffer
#endif //sl_zigbee_af_green_power_cluster_print_buffer
#define sl_zigbee_af_green_power_cluster_print_buffer(buffer, len, withSpace) sl_zigbee_af_green_power_cluster_print_buffer_wrapper(buffer, len, withSpace)

#ifdef sl_zigbee_af_green_power_cluster_print_string
#undef sl_zigbee_af_green_power_cluster_print_string
#endif //sl_zigbee_af_green_power_cluster_print_string
#define sl_zigbee_af_green_power_cluster_print_string(buffer) sl_zigbee_af_green_power_cluster_print_string_wrapper(buffer)

#ifdef sl_zigbee_af_debug_print
#undef sl_zigbee_af_debug_print
#endif //sl_zigbee_af_debug_print
#define sl_zigbee_af_debug_print(...) sl_zigbee_af_debug_print_wrapper(__VA_ARGS__)

#ifdef sl_zigbee_af_debug_println
#undef sl_zigbee_af_debug_println
#endif //sl_zigbee_af_debug_println
#define sl_zigbee_af_debug_println(...) sl_zigbee_af_debug_println_wrapper(__VA_ARGS__)

#ifdef sl_zigbee_af_zdo_print
#undef sl_zigbee_af_zdo_print
#endif // sl_zigbee_af_zdo_print
#define sl_zigbee_af_zdo_print(...) sl_zigbee_af_print(0x00, __VA_ARGS__)

#ifdef sl_zigbee_af_attributes_print
#undef sl_zigbee_af_attributes_print
#endif // sl_zigbee_af_attributes_print
#define sl_zigbee_af_attributes_print(...) sl_zigbee_af_print(0x00, __VA_ARGS__)

#ifdef sl_zigbee_af_service_discovery_print
#undef sl_zigbee_af_service_discovery_print
#endif // sl_zigbee_af_service_discovery_print
#define sl_zigbee_af_service_discovery_print(...) sl_zigbee_af_print(0x00, __VA_ARGS__)

#ifdef sl_zigbee_af_print
#undef sl_zigbee_af_print
#endif //sl_zigbee_af_print
#define sl_zigbee_af_print(...) sl_zigbee_af_print_wrapper(__VA_ARGS__)

#ifdef sl_zigbee_af_app_println
#undef sl_zigbee_af_app_println
#endif //sl_zigbee_af_app_println
#define sl_zigbee_af_app_println(...) sl_zigbee_af_app_println_wrapper(__VA_ARGS__)

#ifdef sl_zigbee_af_app_print
#undef sl_zigbee_af_app_print
#endif //sl_zigbee_af_app_print
#define sl_zigbee_af_app_print(...) sl_zigbee_af_app_print_wrapper(__VA_ARGS__)

#ifdef sl_zigbee_af_attributes_print_buffer
#undef sl_zigbee_af_attributes_print_buffer
#endif //sl_zigbee_af_attributes_print_buffer
#define sl_zigbee_af_attributes_print_buffer(buffer, len, withSpace) sl_zigbee_af_print_buffer(0x00, (buffer), (len), (withSpace))

#ifdef sl_zigbee_af_app_print_buffer
#undef sl_zigbee_af_app_print_buffer
#endif //sl_zigbee_af_app_print_buffer
#define sl_zigbee_af_app_print_buffer(buffer, len, withSpace)   sl_zigbee_af_print_buffer(0x00, (buffer), (len), (withSpace))

#ifdef sl_zigbee_af_debug_print_buffer
#undef sl_zigbee_af_debug_print_buffer
#endif //sl_zigbee_af_debug_print_buffer
#define sl_zigbee_af_debug_print_buffer(buffer, len, withSpace) sl_zigbee_af_print_buffer(0x00, (buffer), (len), (withSpace))

#ifdef sl_zigbee_af_print_buffer
#undef sl_zigbee_af_print_buffer
#endif //sl_zigbee_af_print_buffer
#define sl_zigbee_af_print_buffer(...) sl_zigbee_af_print_buffer_wrapper(__VA_ARGS__)

#ifdef sl_zigbee_af_debug_flush
#undef sl_zigbee_af_debug_flush
#endif // sl_zigbee_af_debug_flush
#define sl_zigbee_af_debug_flush()

#ifdef sl_zigbee_af_attributes_println
#undef sl_zigbee_af_attributes_println
#endif // sl_zigbee_af_attributes_println
#define sl_zigbee_af_attributes_println(...) sl_zigbee_af_println(0x00, __VA_ARGS__)

#ifdef sl_zigbee_af_app_flush
#undef sl_zigbee_af_app_flush
#endif // sl_zigbee_af_app_flush
#define sl_zigbee_af_app_flush()

#ifdef sl_zigbee_af_ota_bootload_cluster_println
#undef sl_zigbee_af_ota_bootload_cluster_println
#endif // sl_zigbee_af_ota_bootload_cluster_println
#define sl_zigbee_af_ota_bootload_cluster_println(...) sl_zigbee_af_println(0x00, __VA_ARGS__)

#ifdef sl_zigbee_af_zdo_println
#undef sl_zigbee_af_zdo_println
#endif // sl_zigbee_af_zdo_println
#define sl_zigbee_af_zdo_println(...) sl_zigbee_af_println(0x00, __VA_ARGS__)

#ifdef sl_zigbee_af_service_discovery_println
#undef sl_zigbee_af_service_discovery_println
#endif // sl_zigbee_af_service_discovery_println
#define sl_zigbee_af_service_discovery_println(...) sl_zigbee_af_println(0x00, __VA_ARGS__)

#ifdef sl_zigbee_af_println
#undef sl_zigbee_af_println
#endif // sl_zigbee_af_println
#define sl_zigbee_af_println(...) sl_zigbee_af_println_wrapper(__VA_ARGS__)

#ifdef sl_zigbee_af_core_flush
#undef sl_zigbee_af_core_flush
#endif // sl_zigbee_af_core_flush
#define sl_zigbee_af_core_flush()

#ifdef sl_zigbee_af_attributes_flush
#undef sl_zigbee_af_attributes_flush
#endif // sl_zigbee_af_attributes_flush
#define sl_zigbee_af_attributes_flush()

#ifdef sl_zigbee_af_app_debug_exec
#undef sl_zigbee_af_app_debug_exec
#endif // sl_zigbee_af_app_debug_exec
#define sl_zigbee_af_app_debug_exec(x) if ( true ) { x; }

extern uint16_t sl_zigbee_af_print_active_area;

#endif // !SL_ZIGBEE_TEST

#endif //_SILABS_GREEN_POWER_PRINT_WRAPPER_H_
