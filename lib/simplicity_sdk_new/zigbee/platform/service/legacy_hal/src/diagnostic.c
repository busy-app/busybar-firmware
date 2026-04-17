/***************************************************************************//**
 * @file
 * @brief stubs for HAL diagnostic functions.
 *******************************************************************************
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
#include PLATFORM_HEADER
#include "hal.h"
#include "cortexm3/diagnostic.h"
#include "micro.h"
#include "em_device.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#ifdef SL_CATALOG_EMLIB_RMU_PRESENT
#include "em_rmu.h"
#define sl_hal_emu_get_reset_cause   RMU_ResetCauseGet
#define sl_hal_emu_clear_reset_cause RMU_ResetCauseClear
#endif // SL_CATALOG_EMLIB_RMU_PRESENT

#ifdef SL_CATALOG_HAL_EMU_PRESENT
#include "sl_hal_emu.h"
#endif // SL_CATALOG_HAL_EMU_PRESENT

#if defined(SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT)
#include "sl_iostream.h"
#endif

// Extended reset info live in noinit RAM segment that is not modified during startup.
NO_INIT(HalResetCauseType halExtendedResetCause);

// Crash info live in noinit RAM segment that is not modified during startup.
NO_INIT(HalCrashInfoType halCrashInfo);

//------------------------------------------------------------------------------
// Preprocessor definitions

// Reserved instruction executed after a failed assert to cause a usage fault
#define ASSERT_USAGE_OPCODE                 0xDE42

#if !defined(WDOG0)
  #define WDOG0      WDOG
  #define WDOG0_IRQn WDOG_IRQn
#endif

#if defined(SL_CATALOG_CMSIS_OS_COMMON_PRESENT)
  #define freeRTOS 1
#else
  #define freeRTOS 0
#endif

// Forward Declarations
extern void sli_802154phy_radio_sleep(void);

//------------------------------------------------------------------------------
// Local Variables

#if defined(SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT)

static const char * const cfsrBits[] =
{
  // Memory management (MPU) faults
  "IACCVIOL: attempted instruction fetch from a no-execute address",  // B0
  "DACCVIOL: attempted load or store at an illegal address",          // B1
  "",                                                                 // B2
  "MUNSTKERR: unstack from exception return caused access violation", // B3
  "MSTKERR: stacking from exception caused access violation",         // B4
  "",                                                                 // B5
  "",                                                                 // B6
  "MMARVALID: MMAR contains valid fault address",                     // B7

  // Bus faults
  "IBUSERR: instruction prefetch caused bus fault",                   // B8
  "PRECISERR: precise data bus fault",                                // B9
  "IMPRECISERR: imprecise data bus fault",                            // B10
  "UNSTKERR: unstacking on exception return caused data bus fault",   // B11
  "STKERR: stacking on exception entry caused data bus fault",        // B12
  "",                                                                 // B13
  "",                                                                 // B14
  "BFARVALID: BFAR contains valid fault address",                     // B15

  // Usage faults
  "UNDEFINSTR: tried to execute an undefined instruction",            // B16
  "INVSTATE: invalid EPSR - e.g., tried to switch to ARM mode",       // B17
  "INVPC: exception return integrity checks failed",                  // B18
  "NOCP: attempted to execute a coprocessor instruction",             // B19
  "",                                                                 // B20
  "",                                                                 // B21
  "",                                                                 // B22
  "",                                                                 // B23
  "UNALIGNED: attempted an unaligned memory access",                  // B24
  "DIVBYZERO: attempted to execute SDIV or UDIV with divisor of 0"    // B25
};

// Names of raw crash data items - each name is null terminated, and the
// end of the array is flagged by two null bytes in a row.
// NOTE: the order of these names must match HalCrashInfoType members.
static const char nameStrings[] = "R0\0R1\0R2\0R3\0"
                                  "R4\0R5\0R6\0R7\0"
                                  "R8\0R9\0R10\0R11\0"
                                  "R12\0R13(LR)\0MSP\0PSP\0"
                                  "PC\0xPSR\0MSP used\0PSP used\0"
                                  "CSTACK bottom\0ICSR\0SHCSR\0INT_ACTIVE0\0"
                                  "INT_ACTIVE1\0"
                                  "CFSR\0HFSR\0DFSR\0MMAR/BFAR\0AFSR\0"
                                  "Ret0\0Ret1\0Ret2\0Ret3\0"
                                  "Ret4\0Ret5\0Dat0\0Dat1\0";

#endif // SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT

static uint16_t savedResetCause;
static HalAssertInfoType savedAssertInfo;

//------------------------------------------------------------------------------
// Functions

#if defined(SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT)

void halPrintCrashData(uint8_t port)
{
  (void)port;
  uint32_t *data = (uint32_t*)&halCrashInfo.R0;
  char const *name = nameStrings;
  char const *separator;
  uint8_t i = 0;

  while (*name != '\0') {
    sl_iostream_printf(SL_IOSTREAM_STDOUT, "%s = %4lx", name, (unsigned long)*data++);
    // increment pointer to end of name
    while (*name != '\0') {
      name++;
    }
    // increment past null pointer for next name
    name++;

    /*lint -save -e448 */
    separator = ((*name != '\0') && ((i & 3) != 3)) ? ", " : "\r\n";

    /*lint -restore */
    sl_iostream_printf(SL_IOSTREAM_STDOUT, separator);
    i++;
  }
}

void halPrintCrashDetails(uint8_t port)
{
  (void)port;

  HalCrashInfoType *c = &halCrashInfo;
  uint16_t reason = savedResetCause;
  uint8_t bit;
  const uint8_t numFaults = sizeof(cfsrBits) / sizeof(cfsrBits[0]);

  switch (reason) {
    case RESET_WATCHDOG_EXPIRED:
      sl_iostream_printf(SL_IOSTREAM_STDOUT,
                         "Reset cause: Watchdog expired, no reliable extra information\n");
      break;
    case RESET_WATCHDOG_CAUGHT:
      sl_iostream_printf(SL_IOSTREAM_STDOUT, "Reset cause: Watchdog caught with enhanced info\n");
      sl_iostream_printf(SL_IOSTREAM_STDOUT, "Instruction address: %4lx\n", (unsigned long)c->PC);
      break;
    case RESET_CRASH_ASSERT:
      sl_iostream_printf(SL_IOSTREAM_STDOUT, "Reset cause: Assert %s:%ld\n",
                         c->data.assertInfo.file, (long)c->data.assertInfo.line);
      break;
    case RESET_FAULT_HARD:
      sl_iostream_printf(SL_IOSTREAM_STDOUT, "Reset cause: Hard Fault\n");
      if (c->hfsr.bits.VECTTBL) {
        sl_iostream_printf(SL_IOSTREAM_STDOUT,
                           "HFSR.VECTTBL: error reading vector table for an exception\n");
      }
      if (c->hfsr.bits.FORCED) {
        sl_iostream_printf(SL_IOSTREAM_STDOUT,
                           "HFSR.FORCED: configurable fault could not activate\n");
      }
      if (c->hfsr.bits.DEBUGEVT) {
        sl_iostream_printf(SL_IOSTREAM_STDOUT,
                           "HFSR.DEBUGEVT: fault related to debug - e.g., executed BKPT\n");
      }
      break;
    case RESET_FAULT_MEM:
      sl_iostream_printf(SL_IOSTREAM_STDOUT, "Reset cause: Memory Management Fault\n");
      if (c->cfsr.bits.DACCVIOL || c->cfsr.bits.IACCVIOL) {
        sl_iostream_printf(SL_IOSTREAM_STDOUT, "Instruction address: %4lx\n", (unsigned long)c->PC);
      }
      if (c->cfsr.bits.MMARVALID) {
        sl_iostream_printf(SL_IOSTREAM_STDOUT, "Illegal access address: %4lx\n", (unsigned long)c->faultAddress);
      }
      for (bit = SCB_CFSR_MEMFAULTSR_Pos; bit < (SCB_CFSR_MEMFAULTSR_Pos + 8); bit++) {
        if ((c->cfsr.word & (1 << bit)) && (*cfsrBits[bit] != '\0')) {
          sl_iostream_printf(SL_IOSTREAM_STDOUT, "CFSR.%s\n", cfsrBits[bit]);
        }
      }
      break;
    case RESET_FAULT_BUS:
      sl_iostream_printf(SL_IOSTREAM_STDOUT, "Reset cause: Bus Fault\n");
      sl_iostream_printf(SL_IOSTREAM_STDOUT, "Instruction address: %4lx\n", (unsigned long)c->PC);
      if (c->cfsr.bits.IMPRECISERR) {
        sl_iostream_printf(SL_IOSTREAM_STDOUT,
                           "Address is of an instruction after bus fault occurred, not the cause.\n");
      }
      if (c->cfsr.bits.BFARVALID) {
        sl_iostream_printf(SL_IOSTREAM_STDOUT, "Illegal access address: %4lx\n",
                           (unsigned long)c->faultAddress);
      }
      for (bit = SCB_CFSR_BUSFAULTSR_Pos; bit < SCB_CFSR_USGFAULTSR_Pos; bit++) {
        if (((c->cfsr.word >> bit) & 1U) && (*cfsrBits[bit] != '\0')) {
          sl_iostream_printf(SL_IOSTREAM_STDOUT, "CFSR.%s\n", cfsrBits[bit]);
        }
      }
      if ((c->cfsr.word & 0xFF) == 0) {
        sl_iostream_printf(SL_IOSTREAM_STDOUT, "CFSR.(none) load or store at an illegal address\n");
      }
      break;
    case RESET_FAULT_USAGE:
      sl_iostream_printf(SL_IOSTREAM_STDOUT, "Reset cause: Usage Fault\n");
      sl_iostream_printf(SL_IOSTREAM_STDOUT, "Instruction address: %4lx\n", (unsigned long)c->PC);
      for (bit = SCB_CFSR_USGFAULTSR_Pos;
           (bit < numFaults) && (bit < (sizeof(c->cfsr.word) * 8));
           bit++) {
        if (((c->cfsr.word >> bit) & 1U) && (*cfsrBits[bit] != '\0')) {
          sl_iostream_printf(SL_IOSTREAM_STDOUT, "CFSR.%s\n", cfsrBits[bit]);
        }
      }
      break;
    case RESET_FAULT_DBGMON:
      sl_iostream_printf(SL_IOSTREAM_STDOUT, "Reset cause: Debug Monitor Fault\n");
      sl_iostream_printf(SL_IOSTREAM_STDOUT, "Instruction address: %4lx\n", (unsigned long)c->PC);
      break;
    default:
      break;
  }
}

void halPrintCrashSummary(uint8_t port)
{
  (void)port;

  HalCrashInfoType *c = &halCrashInfo;
  uint32_t sp, stackBegin, stackEnd, size, used;
  uint16_t pct;
  uint8_t *mode;
  const char *stack;
  uint8_t bit;

  if (c->LR & 4) {
    stack = "process";
    sp = c->processSP;
    used = c->processSPUsed;
    stackBegin = 0;
    stackEnd = 0;
  } else {
    stack = "main";
    sp = c->mainSP;
    used = c->mainSPUsed;
    stackBegin = (uint32_t)c->mainStackBottom;
    stackEnd = (uint32_t)(uint8_t *)_CSTACK_SEGMENT_END;
  }

  mode = (uint8_t *)((c->LR & 8) ? "Thread" : "Handler");
  size = stackEnd - stackBegin;
  pct = size ? (uint16_t)(((100 * used) + (size / 2)) / size) : 0;
  sl_iostream_printf(SL_IOSTREAM_STDOUT, "%s mode using %s stack (%4lx to %4lx), SP = %4lx\n",
                     mode, stack, (unsigned long)stackBegin, (unsigned long)stackEnd, (unsigned long)sp);
  sl_iostream_printf(SL_IOSTREAM_STDOUT, "%u bytes used (%u%%) in %s stack (out of %u bytes total)\n",
                     (uint16_t)used, pct, stack, (uint16_t)size);

  // Valid SP range is [stackBegin, stackEnd] inclusive, but contents
  // of stack only go into [stackBegin, stackend).
  if ((sp > stackEnd) || (sp < stackBegin)) {
    sl_iostream_printf(SL_IOSTREAM_STDOUT, "SP is outside %s stack range!\n", stack);
  }

  bool interrupts_active = false;
  uint8_t num_bitmasks = (sizeof(c->intActive.word) / sizeof(c->intActive.word[0]));
  for (uint8_t mask_num = 0; mask_num < num_bitmasks; mask_num++) {
    if (c->intActive.word[mask_num]) {
      interrupts_active = true;
    }
  }

  if (interrupts_active) {
    sl_iostream_printf(SL_IOSTREAM_STDOUT, "Interrupts active (or pre-empted and stacked):\n");
    for (uint8_t mask_num = 0; mask_num < num_bitmasks; mask_num++) {
      for (bit = 0; bit < 32; bit++) {
        if (c->intActive.word[mask_num] & (1 << bit)) {
          sl_iostream_printf(SL_IOSTREAM_STDOUT, " %u", bit + (mask_num * 32));
        }
      }
    }
    sl_iostream_printf(SL_IOSTREAM_STDOUT, "\n");
  } else {
    sl_iostream_printf(SL_IOSTREAM_STDOUT, "No interrupts active\n");
  }
  sl_iostream_printf(SL_IOSTREAM_STDOUT, "\n");
}

#endif // SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT

//------------------------------------------------------------------------------

void halInternalClassifyReset(void)
{
  // Table used to convert from RESET_EVENT register bits to reset types
  static const uint16_t resetEventTable[] = {
  #if defined (_SILICON_LABS_32B_SERIES_2)
    RESET_POWERON_HV,                  // bit  0 : POR
    RESET_EXTERNAL_PIN,                // bit  1 : PIN
    RESET_SOFTWARE_EM4,                // bit  2 : EM4
    RESET_WATCHDOG_EXPIRED,            // bit  3 : WDOG0
    RESET_WATCHDOG_EXPIRED,            // bit  4 : WDOG1
    RESET_FATAL_LOCKUP,                // bit  5 : LOCKUP
    RESET_SOFTWARE,                    // bit  6 : SYSREQ
    RESET_BROWNOUT_DVDD,               // bit  7 : DVDDBOD
    RESET_UNKNOWN_UNKNOWN,             // bit  8 : DVDDLEBOD // TODO: make new reset cause?
    RESET_BROWNOUT_DEC,                // bit  9 : DECBOD
    RESET_BROWNOUT_AVDD,               // bit 10 : AVDDBOD
    RESET_UNKNOWN_UNKNOWN,             // bit 11 : IOVDD0BOD // TODO: make new reset cause?
    RESET_UNKNOWN_UNKNOWN,             // bit 12 : RESERVED
    RESET_UNKNOWN_UNKNOWN,             // bit 13 : TAMPER // TODO: make new reset cause?
    RESET_UNKNOWN_UNKNOWN,             // bit 14 : M0SYSREQ // TODO: make new reset cause?
    RESET_UNKNOWN_UNKNOWN,             // bit 15 : M0LOCKUP // TODO: make new reset cause?
  #elif defined (_SILICON_LABS_32B_SERIES_3)
    RESET_POWERON_HV,                  // bit  0 : POR
    RESET_EXTERNAL_PIN,                // bit  1 : PIN
    RESET_SOFTWARE_EM4,                // bit  2 : EM4
    RESET_WATCHDOG_EXPIRED,            // bit  3 : WDOG0
    RESET_WATCHDOG_EXPIRED,            // bit  4 : WDOG1
    RESET_FATAL_LOCKUP,                // bit  5 : LOCKUP
    RESET_SOFTWARE,                    // bit  6 : SYSREQ
    RESET_BROWNOUT_DVDD,               // bit  7 : DVDDBOD
    RESET_UNKNOWN_UNKNOWN,             // bit  8 : DVDDLEBOD // TODO: make new reset cause?
    RESET_BROWNOUT_DEC,                // bit  9 : DECBOD
    RESET_BROWNOUT_AVDD,               // bit 10 : AVDDBOD
    RESET_UNKNOWN_UNKNOWN,             // bit 11 : IOVDD0BOD // TODO: make new reset cause?
    RESET_UNKNOWN_UNKNOWN,             // bit 12 : IOVDD1BOD // TODO: make new reset cause?
    RESET_UNKNOWN_UNKNOWN,             // bit 13 : FLBOD // TODO: make new reset cause?
    RESET_UNKNOWN_UNKNOWN,             // bit 14 : SETAMPER // TODO: make new reset cause?
    RESET_UNKNOWN_UNKNOWN,             // bit 15 : M0SYSREQ // TODO: make new reset cause?
    RESET_UNKNOWN_UNKNOWN,             // bit 16 : M0LOCKUP // TODO: make new reset cause?
    RESET_UNKNOWN_UNKNOWN,             // bit 17 : SEM0SYSREQ // TODO: make new reset cause?
    RESET_UNKNOWN_UNKNOWN,             // bit 18 : SEM0LOCKUP // TODO: make new reset cause?
  #elif defined (_EFR_DEVICE)
    RESET_POWERON_HV,                  // bit  0: PORST
    RESET_UNKNOWN_UNKNOWN,             // bit  1: RESERVED
    RESET_BROWNOUT_AVDD,               // bit  2: AVDDBOD
    RESET_BROWNOUT_DVDD,               // bit  3: DVDDBOD
    RESET_BROWNOUT_DEC,                // bit  4: DECBOD
    RESET_UNKNOWN_UNKNOWN,             // bit  5: RESERVED
    RESET_UNKNOWN_UNKNOWN,             // bit  6: RESERVED
    RESET_UNKNOWN_UNKNOWN,             // bit  7: RESERVED
    RESET_EXTERNAL_PIN,                // bit  8: EXTRST
    RESET_FATAL_LOCKUP,                // bit  9: LOCKUPRST
    RESET_SOFTWARE,                    // bit 10: SYSREQRST
    RESET_WATCHDOG_EXPIRED,            // bit 11: WDOGRST
    RESET_UNKNOWN_UNKNOWN,             // bit 12: RESERVED
    RESET_UNKNOWN_UNKNOWN,             // bit 13: RESERVED
    RESET_UNKNOWN_UNKNOWN,             // bit 14: RESERVED
    RESET_UNKNOWN_UNKNOWN,             // bit 15: RESERVED
    RESET_SOFTWARE_EM4,                // bit 16: EM4RST
  #else
    #error "Reset causes are undefined for this part. You'll need to find your _emu.h file for the part and define the reset bits accordingly."
  #endif
  };

  uint32_t resetEvent = sl_hal_emu_get_reset_cause();
  sl_hal_emu_clear_reset_cause();
  uint16_t cause = RESET_UNKNOWN;
  uint16_t i;

  for (i = 0; i < sizeof(resetEventTable) / sizeof(resetEventTable[0]); i++) {
    if (resetEvent & (1 << i)) {
      cause = resetEventTable[i];
      break;
    }
  }

  HalResetCauseType resetCause = halExtendedResetCause;

  if (cause == RESET_SOFTWARE) {
    if ((resetCause.signature == RESET_VALID_SIGNATURE)
        && (RESET_BASE_TYPE(resetCause.reason) < NUM_RESET_BASE_TYPES)) {
      // The extended reset cause is recovered from RAM
      // This can be trusted because the hardware reset event was software
      //  and additionally because the signature is valid
      savedResetCause = resetCause.reason;
    } else {
      savedResetCause = RESET_SOFTWARE_UNKNOWN;
    }
    // mark the signature as invalid
    halExtendedResetCause.signature = RESET_INVALID_SIGNATURE;
  } else if ((cause == RESET_BOOTLOADER_DEEPSLEEP)
             && (resetCause.signature == RESET_VALID_SIGNATURE)
             && (resetCause.reason == RESET_BOOTLOADER_DEEPSLEEP)) {
    // Save the crash info for bootloader deep sleep (even though it's not used
    // yet) and invalidate the reset signature.
    halExtendedResetCause.signature = RESET_INVALID_SIGNATURE;
    savedResetCause = resetCause.reason;
  } else {
    savedResetCause = cause;
  }

  // If the last reset was due to an assert, save the assert info.
  if (savedResetCause == RESET_CRASH_ASSERT) {
    savedAssertInfo = halCrashInfo.data.assertInfo;
  }
}

uint16_t halGetExtendedResetInfo(void)
{
  return savedResetCause;
}

const HalAssertInfoType *halGetAssertInfo(void)
{
  return &savedAssertInfo;
}

uint8_t halGetResetInfo(void)
{
  return RESET_BASE_TYPE(savedResetCause);
}

// Translate EM3xx reset codes to the codes previously used by the EM2xx.
// If there is no corresponding code, return the EM3xx base code with bit 7 set.
uint8_t halGetEm2xxResetInfo(void)
{
  uint8_t reset = halGetResetInfo();

  // Any reset with an extended value field of zero is considered an unknown
  // reset, except for FIB resets.
  if ((RESET_EXTENDED_FIELD(halGetExtendedResetInfo()) == 0)
      && (reset != RESET_FIB)) {
    return EM2XX_RESET_UNKNOWN;
  }

  switch (reset) {
    case RESET_UNKNOWN:
      return EM2XX_RESET_UNKNOWN;
    case RESET_BOOTLOADER:
      return EM2XX_RESET_BOOTLOADER;
    case RESET_EXTERNAL:    // map pin resets to poweron for EM2xx compatibility
//    return EM2XX_RESET_EXTERNAL;
    case RESET_POWERON:
      return EM2XX_RESET_POWERON;
    case RESET_WATCHDOG:
      return EM2XX_RESET_WATCHDOG;
    case RESET_SOFTWARE:
      return EM2XX_RESET_SOFTWARE;
    case RESET_CRASH:
      return EM2XX_RESET_ASSERT;
    default:
      return (reset | 0x80);    // set B7 for all other reset codes
  }
}

const char * halGetResetString(void)
{
  // Table used to convert from reset types to reset strings.
  #define RESET_BASE_DEF(basename, value, string)  string,
  #define RESET_EXT_DEF(basename, extname, extvalue, string)     /*nothing*/
  static const char resetStringTable[][4] = {
    #include "reset-def.h"
  };
  #undef RESET_BASE_DEF
  #undef RESET_EXT_DEF
  uint8_t resetInfo = halGetResetInfo();
  if (resetInfo >= (sizeof(resetStringTable) / sizeof(resetStringTable[0]))) {
    return resetStringTable[0x00];   // return unknown
  } else {
    return resetStringTable[resetInfo];
  }
}

const char * halGetExtendedResetString(void)
{
  // Create a table of reset strings for each extended reset type
  typedef const char ResetStringTableType[][4];
  // Uncrustify doesn't like the following defines
  /* *INDENT-OFF* */
  #define RESET_BASE_DEF(basename, value, string) \
  }; static ResetStringTableType basename##ResetStringTable = {
  #define RESET_EXT_DEF(basename, extname, extvalue, string)  string,
  {
    #include "reset-def.h"
  };
  /* *INDENT-ON* */
  #undef RESET_BASE_DEF
  #undef RESET_EXT_DEF

  // Create a table of pointers to each of the above tables
  #define RESET_BASE_DEF(basename, value, string)  (ResetStringTableType *)basename##ResetStringTable,
  #define RESET_EXT_DEF(basename, extname, extvalue, string)     /*nothing*/
  static ResetStringTableType * const extendedResetStringTablePtrs[] = {
    #include "reset-def.h"
  };
  #undef RESET_BASE_DEF
  #undef RESET_EXT_DEF

  uint16_t extResetInfo = halGetExtendedResetInfo();
  // access the particular table of extended strings we are interested in
  ResetStringTableType *extendedResetStringTable =
    extendedResetStringTablePtrs[RESET_BASE_TYPE(extResetInfo)];

  // return the string from within the proper table
  return (*extendedResetStringTable)[((extResetInfo) & 0xFF)];
}

void halInternalSysReset(uint16_t extendedCause)
{
  INTERRUPTS_OFF();

  halExtendedResetCause.reason = extendedCause;
  halExtendedResetCause.signature = RESET_VALID_SIGNATURE;

  // force write to complete before reset
  asm ("DMB");
  NVIC_SystemReset();
}

// Cause a usage fault by executing a special UNDEFINED instruction.
// The high byte (0xDE) is reserved to be undefined - the low byte (0x42)
// is arbitrary and distiguishes a failed assert from other usage faults.
// the fault handler with then decode this, grab the filename and linenumber
// parameters from R0 and R1 and save the information for display after a reset
#if defined (__ICCARM__)
#pragma diag_suppress=Og014
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_LEGACY_HAL, SL_CODE_CLASS_TIME_CRITICAL)
static void halInternalAssertFault(const char * filename, int linenumber)
{
  asm ("DC16 0DE42h");
}
#pragma diag_default=Og014
#elif defined (__GNUC__)
__attribute__((noinline))
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_LEGACY_HAL, SL_CODE_CLASS_TIME_CRITICAL)
static void halInternalAssertFault(const char * filename, int linenumber)
{
  asm (".short 0xDE42\n" : : "r" (filename), "r" (linenumber));
}
#endif

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_LEGACY_HAL, SL_CODE_CLASS_TIME_CRITICAL)
void halInternalAssertFailed(const char * filename, int linenumber)
{
#if !defined (_SILICON_LABS_32B_SERIES_2)
  sli_802154phy_radio_sleep();
#endif

#if defined(SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT)
  sl_iostream_printf(SL_IOSTREAM_STDOUT, "\r\n[ASSERT:%s:%d]\r\n", filename, linenumber);
#endif // SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT

  halResetWatchdog();              // In case we're close to running out.
  INTERRUPTS_OFF();

#if defined (__ICCARM__) || defined (__GNUC__)
  // We can use the special fault mechanism to preserve more assert
  // information for display after a crash
  halInternalAssertFault(filename, linenumber);
#else
  // Other toolchains don't handle the inline assembly correctly, so
  // we just call the internal reset
  halCrashInfo.data.assertInfo.file = filename;
  halCrashInfo.data.assertInfo.line = linenumber;
  halInternalSysReset(RESET_CRASH_ASSERT);
#endif
}

// Returns the bytes used in the main stack area.
static uint32_t halInternalGetMainStackBytesUsed(uint32_t *p)
{
  for (; p < (uint32_t *)_CSTACK_SEGMENT_END; p++) {
    if (*p != STACK_FILL_VALUE) {
      break;
    }
  }
  return (uint32_t)((uint8_t *)_CSTACK_SEGMENT_END - (uint8_t *)p);
}

// After the low-level fault handler (in faults.s79) has saved the processor
// registers (R0-R12, LR and both MSP an PSP), it calls halInternalCrashHandler
// to finish saving additional crash data. This function returns the reason for
// the crash to the low-level fault handler that then calls
// halInternalSystsemReset() to reset the processor.
uint16_t halInternalCrashHandler(void)
{
  uint32_t activeException;
  uint16_t reason = (uint16_t)RESET_FAULT_UNKNOWN;
  HalCrashInfoType *c = &halCrashInfo;
  uint8_t i, j;
  uint32_t *sp, *s, *sEnd, *stackBottom, *stackTop;
  uint32_t data;

  c->icsr.word = SCB->ICSR;
  c->shcsr.word = SCB->SHCSR;

  uint8_t num_bitmasks = (sizeof(c->intActive.word) / sizeof(c->intActive.word[0]));
  for (uint8_t mask_num = 0; mask_num < num_bitmasks; mask_num++) {
    c->intActive.word[mask_num] = NVIC->IABR[mask_num];
  }

  c->cfsr.word = SCB->CFSR;
  c->hfsr.word = SCB->HFSR;
  c->dfsr.word = SCB->DFSR;
  c->faultAddress = SCB->MMFAR;
  c->afsr.word = SCB->AFSR;

  // Examine B2 of the saved LR to know the stack in use when the fault occurred
  sp = (uint32_t *)(((c->LR & 4U) != 0U) ? c->processSP : c->mainSP);
  sEnd = sp; // Keep a copy around for walking the stack later

  // Get the bottom of the stack since we allow stack resizing
  c->mainStackBottom = (uint32_t)_CSTACK_SEGMENT_BEGIN;

  // If we're running FreeRTOS and this is a process stack then add
  // extra diagnostic information
  if ((freeRTOS != 0) && ((c->LR & 4U) != 0U)) {
    stackBottom = sp;
    stackTop = sp + 8;
    c->processSPUsed = stackTop - sp;
  } else {
    stackBottom = (uint32_t*)c->mainStackBottom;
    stackTop = (uint32_t*)_CSTACK_SEGMENT_END;
    c->processSPUsed = 0;   // process stack not in use
  }

  // If the stack pointer is valid, read and save the stacked PC and xPSR
  if ((sp >= stackBottom)
      && ((sp + 8) <= stackTop)) {
    sp += 6; // Skip over R0,R1,R2,R3,R12,LR
    c->PC = *sp++;
    c->xPSR.word = *sp++;

    // See if fault was due to a failed assert. This is indicated by
    // a usage fault caused by executing a reserved instruction.
    if ( c->icsr.bits.VECTACTIVE == USAGE_FAULT_VECTOR_INDEX
         && ((uint16_t *)c->PC >= (uint16_t *)_TEXT_SEGMENT_BEGIN)
         && ((uint16_t *)c->PC < (uint16_t *)_TEXT_SEGMENT_END)
         && *(uint16_t *)(c->PC) == ASSERT_USAGE_OPCODE ) {
      // Copy halInternalAssertFailed() arguments into data member specific
      // to asserts.
      c->data.assertInfo.file = (const char *)c->R0;
      c->data.assertInfo.line = c->R1;
#ifdef PUSH_REGS_BEFORE_ASSERT
      // Just before calling halInternalAssertFailed(), R0, R1, R2 and LR were
      // pushed onto the stack - copy these values into the crash data struct.
      c->R0 = *sp++;
      c->R1 = *sp++;
      c->R2 = *sp++;
      c->LR = *sp++;
#endif
      reason = (uint16_t)RESET_CRASH_ASSERT;
    }
    // If a bad stack pointer, PC and xPSR to 0 to indicate they are not known.
  } else {
    c->PC = 0;
    c->xPSR.word = 0;
    sEnd = stackBottom;
  }

  c->mainSPUsed = halInternalGetMainStackBytesUsed((uint32_t*)c->mainStackBottom);

  for (i = 0; i < NUM_RETURNS; i++) {
    c->returns[i] = 0;
  }

  // Search the stack downward for probable return addresses. A probable
  // return address is a value in the CODE segment that also has bit 0 set
  // (since we're in Thumb mode).
  i = 0U;
  s = stackTop;
  while (s > sEnd) {
    data = *(--s);
    if (((uint16_t *)data >= (uint16_t *)_TEXT_SEGMENT_BEGIN)
        && ((uint16_t *)data < (uint16_t *)_TEXT_SEGMENT_END)
        && ((data & 1U) != 0U)) {
      // Only record the first occurrence of a return - other copies could
      // have been in registers that then were pushed.
      for (j = 0; j < NUM_RETURNS; j++) {
        if (c->returns[j] == data) {
          break;
        }
      }
      if (j != NUM_RETURNS) {
        continue;
      }
      // Save the return in the returns array managed as a circular buffer.
      // This keeps only the last NUM_RETURNS in the event that there are more.
      i = (i != 0U) ? i - 1U : NUM_RETURNS - 1U;
      c->returns[i] = data;
    }
  }
  // Shuffle the returns array so returns[0] has last probable return found.
  // If there were fewer than NUM_RETURNS, unused entries will contain zero.
  while ((i--) != 0U) {
    data = c->returns[0];
    for (j = 0; j < NUM_RETURNS - 1U; j++ ) {
      c->returns[j] = c->returns[j + 1U];
    }
    c->returns[NUM_RETURNS - 1U] = data;
  }

  // Read the highest priority active exception to get reason for fault
  activeException = c->icsr.bits.VECTACTIVE;
  switch (activeException) {
    #if defined(WDOG_IF_WARN) && !defined(BOOTLOADER)
    case IRQ_TO_VECTOR_NUMBER(WDOG0_IRQn):
      if (WDOG0->IF & WDOG_IF_WARN) {
        reason = RESET_WATCHDOG_CAUGHT;
      }
      break;
    #endif
    case HARD_FAULT_VECTOR_INDEX:
      reason = (uint16_t)RESET_FAULT_HARD;
      break;
    case MEMORY_FAULT_VECTOR_INDEX:
      reason = (uint16_t)RESET_FAULT_MEM;
      break;
    case BUS_FAULT_VECTOR_INDEX:
      reason = (uint16_t)RESET_FAULT_BUS;
      break;
    case USAGE_FAULT_VECTOR_INDEX:
      // make sure we didn't already identify the usage fault as an assert
      if (reason == (uint16_t)RESET_FAULT_UNKNOWN) {
        reason = (uint16_t)RESET_FAULT_USAGE;
      }
      break;
    case DEBUG_MONITOR_VECTOR_INDEX:
      reason = (uint16_t)RESET_FAULT_DBGMON;
      break;
    default:
      if ((activeException != 0U) && (activeException < VECTOR_TABLE_LENGTH)) {
        reason = (uint16_t)RESET_FAULT_BADVECTOR;
      }
      break;
  }
  return reason;
}
