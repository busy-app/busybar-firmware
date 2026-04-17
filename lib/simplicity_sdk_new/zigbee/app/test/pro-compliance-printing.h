/***************************************************************************//**
 * @file
 * @brief This file contains printing utilities for the zigbee_pro_compliance app
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

void sl_zigbee_af_core_print(const char * formatString, ...);
void sl_zigbee_af_core_println(const char * formatString, ...);
void sl_zigbee_af_print_big_endian_eui64(const sl_802154_long_addr_t eui64);

//#define sl_zigbee_af_core_print(...) proComplianceCorePrint(__VA_ARGS__)
//#define sl_zigbee_af_core_println(...) proComplianceCorePrintln(__VA_ARGS__)
//#define sl_zigbee_af_print_big_endian_eui64(eui64) proCompliancePrintBigEndianEui64(eui64)
/*
 #define sl_zigbee_af_core_print(functionality, formatString, ...)                \
   do {                                                                     \
   if (corePrintingEnabled) {                                             \
     sli_legacy_serial_printf(serialPort, formatString, __VA_ARGS__);            \
   }                                                                      \
   } while (0)

   void sl_zigbee_af_core_println(functionality, formatString, ...)                 \
   do {                                                                     \
   if (corePrintingEnabled) {                                             \
     sli_legacy_serial_printf_line(serialPort, formatString, __VA_ARGS__);        \
   }                                                                      \
   } while (0)

   void sl_zigbee_af_print_big_endian_eui64(eui64)                                    \
   do {                                                                     \
   if (corePrintingEnabled) {                                             \
     printBigEndianEui64(serialPort, eui64);                              \
   }                                                                      \
   } while (0)*/
