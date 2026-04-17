/***************************************************************************//**
 * @file
 * @brief Utilities and command line interface for printing, and enabling/disabling
 * printing to different areas.
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

#include "app/framework/include/af.h"
#include "app/framework/plugin/debug-print/sl_zigbee_debug_print.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
//------------------------------------------------------------------------------
// Globals

// Enable this if you want area names printed. It proved annoying
// so we're disabling it here.
//#define SL_ZIGBEE_AF_PRINT_AREA_NAME

#ifdef SL_ZIGBEE_AF_PRINT_AREA_NAME
static void reallyPrintAreaName(uint16_t area);
  #define printAreaName(area) reallyPrintAreaName(area)
#else
  #define printAreaName(area)
#endif //SL_ZIGBEE_AF_PRINT_AREA_NAME

#ifdef SL_ZIGBEE_AF_PRINT_NAMES
static const char * areaNames[] = SL_ZIGBEE_AF_PRINT_NAMES;
#endif

#ifdef SL_ZIGBEE_AF_PRINT_BITS
static uint8_t enablementBytes[] = SL_ZIGBEE_AF_PRINT_BITS;
#endif

uint16_t sl_zigbee_af_print_active_area = 0;

//------------------------------------------------------------------------------

// In the first stage of Zigbee/UC there is no run-time enabling/disabling of
// debug prints
bool sl_zigbee_af_print_enabled(uint16_t area)
{
  (void) area;
  return true;
}

static void printEnable(uint16_t area, bool on)
{
#ifdef SL_ZIGBEE_AF_PRINT_BITS
  uint8_t index = (uint8_t)(area >> 8);
  if ( index < sizeof(enablementBytes) ) {
    if ( on ) {
      enablementBytes[index] |= (uint8_t)(area & 0xFF);
    } else {
      enablementBytes[index] &= ~(uint8_t)(area & 0xFF);
    }
  }
#else
  (void) area;
  (void) on;
#endif // SL_ZIGBEE_AF_PRINT_BITS
}

static uint16_t convertUserNumberAreaToInternalArea(uint16_t userNumberedArea)
{
  uint16_t index = userNumberedArea / 8;
  index = (uint16_t)(index << 8)
          + (uint16_t)( ((uint16_t)0x0001) << (userNumberedArea % 8) );
  return index;
}

#if defined SL_ZIGBEE_AF_PRINT_AREA_NAME
static void reallyPrintAreaName(uint16_t area)
{
#ifdef SL_ZIGBEE_AF_PRINT_NAMES
  uint8_t hi, lo, count;
  uint16_t index;

  hi = (uint8_t)(area >> 8);
  lo = (uint8_t)(area & 0xFF);
  count = 0;

  if ( lo != 0 ) {
    while ( !(lo & 0x01) ) {
      lo >>= 1;
      count++;
    }
  }
  index = ((8 * hi) + count);

  if (area != 0xFFFF
      && index < EMBER_AF_PRINT_NAME_NUMBER) {
    sl_zigbee_core_debug_print("%s:", areaNames[index]);
  }
#endif // SL_ZIGBEE_AF_PRINT_NAMES
}
#endif //SL_ZIGBEE_AF_PRINT_AREA_NAME

// Prints the trace if trace is enabled
static void sli_zigbee_af_print_internal_var_arg(uint16_t area,
                                                 bool newLine,
                                                 const char * formatString,
                                                 va_list ap)
{
  if ( !sl_zigbee_af_print_enabled(area) ) {
    return;
  }
  printAreaName(area);
#ifdef SL_CATALOG_ZIGBEE_SIMULATION_PRESENT
  (void) local_vprintf(formatString, ap);
#else
  (void) vprintf(formatString, ap);
#endif // SL_CATALOG_ZIGBEE_SIMULATION_PRESENT
  if (newLine) {
    sl_zigbee_core_debug_println("");
  }
  sl_zigbee_af_print_active_area = area;
}

void sl_zigbee_af_println(uint16_t area, const char * formatString, ...)
{
  va_list ap = { 0 };
  va_start(ap, formatString);
  sli_zigbee_af_print_internal_var_arg(area, true, formatString, ap);
  va_end(ap);
}

void sl_zigbee_af_print(uint16_t area, const char * formatString, ...)
{
  va_list ap = { 0 };
  va_start(ap, formatString);
  sli_zigbee_af_print_internal_var_arg(area, false, formatString, ap);
  va_end(ap);
}

void sl_zigbee_af_print_status(void)
{
#ifdef SL_ZIGBEE_AF_PRINT_NAMES
  uint8_t i;
  for (i = 0; i < EMBER_AF_PRINT_NAME_NUMBER; i++) {
    sl_zigbee_core_debug_println("[%d] %s : %s",
                                 i,
                                 areaNames[i],
                                 (sl_zigbee_af_print_enabled(
                                    convertUserNumberAreaToInternalArea(i))
                                  ? "YES"
                                  : "no"));
    (void) sli_legacy_serial_wait_send(SL_ZIGBEE_AF_PRINT_OUTPUT);
  }
#endif // SL_ZIGBEE_AF_PRINT_NAMES
}

void sl_zigbee_af_print_all_on(void)
{
#ifdef SL_ZIGBEE_AF_PRINT_BITS
  memset(enablementBytes, 0xFF, sizeof(enablementBytes));
#endif
}

void sl_zigbee_af_print_all_off(void)
{
#ifdef SL_ZIGBEE_AF_PRINT_BITS
  memset(enablementBytes, 0x00, sizeof(enablementBytes));
#endif
}

// These are CLI functions where a user will supply a sequential numbered
// area; as opposed to the bitmask area number that we keep track
// of internally.

void sl_zigbee_af_print_on(uint16_t userArea)
{
  printEnable(convertUserNumberAreaToInternalArea(userArea),
              true);   // enable?
}

void sl_zigbee_af_print_off(uint16_t userArea)
{
  printEnable(convertUserNumberAreaToInternalArea(userArea),
              false);  // enable?
}
