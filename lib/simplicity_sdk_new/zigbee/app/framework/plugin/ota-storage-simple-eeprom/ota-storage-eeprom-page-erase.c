/***************************************************************************//**
 * @file
 * @brief This code is intended for EEPROM devices that do not support
 * read-modify-write and must perform a page erase prior to writing data.
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

#include "app/framework/plugin/ota-common/ota.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"

#include "eeprom.h"

//#define DEBUG_PRINT
#define OTA_STORAGE_EEPROM_INTERNAL_HEADER
#include "ota-storage-eeprom.h"
#undef OTA_STORAGE_EEPROM_INTERNAL_HEADER

#ifndef READ_MODIFY_WRITE_SUPPORT

#if defined(SL_ZIGBEE_TEST)
 #include "hal/micro/unix/simulation/fake-eeprom.h"
#endif

//------------------------------------------------------------------------------
// Globals

static int32_t lastRecordedByteMaskIndex = -1;
static bool lastRecordedByteMaskIndexKnown = false;

static uint32_t currentEraseOffset;
static uint32_t endEraseOffset;
static bool newEraseOperation;

static sl_zigbee_af_event_sleep_control_t storedSleepControl;

// this arbitrary size is to limit the amount we store on the call stack
#define BYTE_MASK_READ_SIZE 20

//------------------------------------------------------------------------------
// Forward declarations

#define startEraseOperation(begin, end)       \
  eraseOperation(true, /* start new erase? */ \
                 (begin),                     \
                 (end))

#define continueEraseOperation()                       \
  eraseOperation(false,   /* start new erase?       */ \
                 0,       /* begin offset (ignored) */ \
                 0)       /* end offset (ignored)   */ \

//------------------------------------------------------------------------------

// We want to get the log(2, PAGE_SIZE) so that we can use bitwise shifts
// instead of multiple and divide for various page size related operations.
// For the xap, 32-bit divide/modulus requires a software library and eats up
// a lot of flash.
static uint8_t determinePageSizeLog(void)
{
  uint8_t pageSizeLog;
  for (pageSizeLog = 0;
       (uint32_t) (1 << pageSizeLog) < (sl_util_af_eeprom_info()->pageSize);
       pageSizeLog++) {
  }
  //  debugPrint("PageSizeLog: %d", pageSizeLog);
  return pageSizeLog;
}

static bool checkDelay(bool mustSetTimer)
{
  if (sl_util_af_eeprom_busy() || mustSetTimer) {
    uint32_t delay = sl_util_af_eeprom_info()->pageEraseMs >> 2;
    if (delay == 0) {
      delay = 1;
    }
    debugPrint("Waiting %d ms for erase to complete.", delay);
    sl_zigbee_af_event_set_delay_ms(eraseEvent, delay);
    return true;
  }

  return false;
}

// Returns true for success (erase operation continuing or completed)
// Returns false for error (erase not started).
static bool eraseOperation(bool startNewErase,
                           uint32_t beginOffset,
                           uint32_t endOffset)
{
  bool success = true;

  SL_ZIGBEE_TEST_ASSERT(!startNewErase
                        || (startNewErase
                            && !sl_zigbee_af_event_is_scheduled(eraseEvent)));

  // In case the first time we are called the EEPROM is busy,
  // we will delay.  However we haven't erased the first page
  // yet so we must take care not to increment the offset yet.

  if (startNewErase) {
    newEraseOperation = true;
    currentEraseOffset = beginOffset;
    endEraseOffset = endOffset;
    otaPrintln("Starting erase from offset 0x%08X to 0x%08X",
               beginOffset,
               endEraseOffset);
    storedSleepControl = sl_zigbee_af_get_default_sleep_control_cb();
    sl_zigbee_af_set_default_sleep_control_cb(SL_ZIGBEE_AF_STAY_AWAKE);
  }

  if (checkDelay(false)) {                        // must set timer?
    return true;
  }

  if (!newEraseOperation) {
    currentEraseOffset += sl_util_af_eeprom_info()->pageSize;
  }

  if (currentEraseOffset < endEraseOffset) {
    uint8_t status;
    debugPrint("Erasing page %d of %d",
               (currentEraseOffset >> determinePageSizeLog()) + 1,
               (endEraseOffset >> determinePageSizeLog()));
    status = sl_util_af_eeprom_erase(currentEraseOffset, sl_util_af_eeprom_info()->pageSize);
    success = (status == EEPROM_SUCCESS);
    newEraseOperation = false;
    if (success) {
      checkDelay(true); // must set timer?
      return true;
    }
    otaPrintln("Could not start ERASE! (0x%02X)", status);
  }

  sl_zigbee_af_set_default_sleep_control(storedSleepControl);

  otaPrintln("EEPROM Erase complete");

  if (!sli_zigbee_af_ota_storage_check_download_meta_data()) {
    // This was a full erase that wiped the meta-data.
    sli_zigbee_af_ota_storage_write_download_meta_data();
  }

  sl_zigbee_af_ota_storage_simple_eeprom_erase_complete_cb(success);
  return true;
}

static bool isMultipleOfPageSize(uint32_t address)
{
  uint32_t pageSizeBits = ((1 << determinePageSizeLog()) - 1);
  return ((pageSizeBits & address) == 0);
}

void sli_zigbee_af_ota_storage_eeprom_init(void)
{
  uint16_t expectedCapabilities = (EEPROM_CAPABILITIES_PAGE_ERASE_REQD
                                   | EEPROM_CAPABILITIES_ERASE_SUPPORTED);
  uint32_t spaceReservedForOta = (otaStorageEepromGetStorageEndAddress()
                                  - otaStorageEepromGetStorageStartAddress());
  const HalEepromInformationType *info = sl_util_af_eeprom_info();

  // NOTE: if the info pointer is NULL it's a good indicator that your data
  // flash isn't properly connected and jumpered in or that your bootloader
  // is too old to support EEPROM info.
  assert(info != NULL);
  assert(expectedCapabilities
         == (info->capabilitiesMask & expectedCapabilities));
  assert(isMultipleOfPageSize(otaStorageEepromGetStorageStartAddress()));
  assert(isMultipleOfPageSize(spaceReservedForOta));

  // Need to make sure that the bytemask used to store each
  // fully downloaded page is big enough to hold all the pages we have been
  // allocated.
  assert((MAX_BYTEMASK_LENGTH / sl_util_af_eeprom_get_word_size())
         >= (spaceReservedForOta >> determinePageSizeLog()));
}

void sl_zigbee_af_ota_storage_simple_eeprom_page_erase_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);

  sl_zigbee_af_event_set_inactive(eraseEvent);
  continueEraseOperation();
}

static int32_t getByteMaskIndexFromEeprom(void)
{
  uint8_t byteMask[BYTE_MASK_READ_SIZE];
  uint32_t readOffset = otaStorageEepromGetImageInfoStartAddress()
                        + SAVED_DOWNLOAD_OFFSET_INDEX;
  uint16_t byteMaskIndex;
  uint8_t wordSize = sl_util_af_eeprom_get_word_size();

  for (byteMaskIndex = 0;
       byteMaskIndex < MAX_BYTEMASK_LENGTH;
       byteMaskIndex += BYTE_MASK_READ_SIZE,
       readOffset  += BYTE_MASK_READ_SIZE) {
    uint8_t i;
    uint8_t status = sl_zigbee_af_eeprom_read(readOffset,
                                              byteMask,
                                              BYTE_MASK_READ_SIZE);
    debugPrint("Bytemask read status: 0x%02X", status);
    SL_ZIGBEE_TEST_ASSERT(status == 0);
#if !defined(DEBUG_PRINT) && !defined(SL_ZIGBEE_TEST)
    UNUSED_VAR(status);
#endif

    if (byteMaskIndex == 0 && byteMask[0] == 0xFF) {
      debugFlush();
      debugPrint("All bytes in bytemask erased, assuming index of -1");
      return -1;
    }

    for (i = 0; i < BYTE_MASK_READ_SIZE; i += wordSize) {
      if (byteMask[i] == 0xFF) {
        uint16_t index = (byteMaskIndex + i - 1) / wordSize;
        debugPrint("Last Download offset Bytemask index: %d",
                   index);
        return (index);
      }
    }
  }

  debugPrint("Error in determining bytemask index, assuming -1");
  return -1;
}

// The bytemask notes the real EEPROM offset of the pages that have been fully
// downloaded.  Each downloaded page is recorded as a 0 byte.  The OTA offset
// is determined based on the SOC_BOOTLOADING_SUPPORT.  In that case the
// first page is considered to be the combination of the portion of the OTA
// image at the bottom of the EEPROM space (the OTA header) and the first
// full flash page at the top of the EEPROM space (the EBL and its data).
// Without SOC bootloading support the OTA offset is equivalent to number
// of EEPROM pages written minus the overhead of the meta-data (namely this
// bytemask and some other data).
static uint32_t getOffsetFromByteMaskIndex(int32_t byteMaskIndex)
{
  // To convert to the number of fully written pages from the bytemask index
  // we must add 1.
  int32_t writtenPages = byteMaskIndex + 1;
  uint32_t otaOffset = (((uint32_t)(writtenPages)) << determinePageSizeLog());

  debugPrint("Unadjusted offset:    0x%08X", otaOffset);

  if (otaOffset != 0) {
#if defined(SOC_BOOTLOADING_SUPPORT)
    otaOffset += sli_zigbee_af_get_ebl_start_offset();
#else
    otaOffset -= OTA_HEADER_INDEX;
#endif
  }

  debugFlush();
  debugPrint("Last OTA Download offset: 0x%08X", otaOffset);
  debugFlush();

  return otaOffset;
}

static int32_t getByteMaskIndexFromOtaOffset(uint32_t otaOffset)
{
  int32_t adjustment;

#if defined(SOC_BOOTLOADING_SUPPORT)
  adjustment = sli_zigbee_af_get_ebl_start_offset();
#else
  adjustment = 0 - OTA_HEADER_INDEX;
#endif

  // debugPrint("Offset: 0x%08X, Adjustment: 0x%08X, EBL Start Offset: 0x%08X, Page Log: %d, Page Size: %d",
  //            otaOffset,
  //            adjustment,
  //            sli_zigbee_af_get_ebl_start_offset(),
  //            determinePageSizeLog(),
  //            sl_util_af_eeprom_info()->pageSize);

  if (otaOffset < (sl_util_af_eeprom_info()->pageSize + adjustment)) {
    return -1;
  }

  return (((otaOffset + adjustment) >> determinePageSizeLog()) - 1);
}

void sli_zigbee_af_storage_eeprom_update_download_offset(uint32_t otaOffsetNew, bool finalOffset)
{
  int32_t byteMaskIndexNew = getByteMaskIndexFromOtaOffset(otaOffsetNew);

  // debugPrint("Checking whether to update bytemask, New Offset: 0x%08X, new bytemask index: %d, old bytemask index: %d, final update: %c",
  //            otaOffsetNew,
  //            byteMaskIndexNew,
  //            lastRecordedByteMaskIndex,
  //            (finalOffset ? 'y' : 'n'));

  if (finalOffset
      && byteMaskIndexNew == lastRecordedByteMaskIndex) {
    byteMaskIndexNew++;
  }

  if (byteMaskIndexNew > lastRecordedByteMaskIndex) {
    uint8_t status;
    uint8_t byteArray[2] = { 0, 0 };

    debugFlush();
    debugPrint("Writing Last Download offset bytemask, new (old): %d (%d)",
               byteMaskIndexNew,
               lastRecordedByteMaskIndex);
    debugFlush();
    debugPrint("OTA Offsets, new (old): 0x%08X (0x%08X)",
               otaOffsetNew,
               getOffsetFromByteMaskIndex(lastRecordedByteMaskIndex));
    debugFlush();

    status = sl_zigbee_af_eeprom_write(
      (otaStorageEepromGetImageInfoStartAddress()
       + SAVED_DOWNLOAD_OFFSET_INDEX
       + (byteMaskIndexNew
          * sl_util_af_eeprom_get_word_size())),
      byteArray,
      sl_util_af_eeprom_get_word_size());
#if !defined(DEBUG_PRINT) && !defined(SL_ZIGBEE_TEST)
    UNUSED_VAR(status);
#endif
    debugPrint("EEPROM Write status: 0x%02X", status);
    SL_ZIGBEE_TEST_ASSERT(status == 0);

    lastRecordedByteMaskIndex = getByteMaskIndexFromEeprom();

    SL_ZIGBEE_TEST_ASSERT(lastRecordedByteMaskIndex == byteMaskIndexNew);
  }
}

void sli_zigbee_af_ota_wipe_storage_device(void)
{
  sl_zigbee_af_ota_storage_driver_invalidate_image_cb();
}

sl_zigbee_af_ota_storage_status_t sl_zigbee_af_ota_storage_driver_invalidate_image_cb(void)
{
  lastRecordedByteMaskIndex = -1;
  lastRecordedByteMaskIndexKnown = false;

  return (startEraseOperation(otaStorageEepromGetStorageStartAddress(),
                              otaStorageEepromGetStorageEndAddress())
          ? SL_ZIGBEE_AF_OTA_STORAGE_OPERATION_IN_PROGRESS
          : SL_ZIGBEE_AF_OTA_STORAGE_ERROR);
}

uint32_t sl_zigbee_af_ota_storage_driver_retrieve_last_stored_offset_cb(void)
{
  if (!sli_zigbee_af_ota_storage_check_download_meta_data()) {
    return 0;
  }

  // Since retrieving the last download offset from the bytemask
  // may involve multiple halEepromRead() calls and this may be slow,
  // we cache the offset.

  if (!lastRecordedByteMaskIndexKnown) {
    lastRecordedByteMaskIndex = getByteMaskIndexFromEeprom();
    lastRecordedByteMaskIndexKnown = true;
  }
  return getOffsetFromByteMaskIndex(lastRecordedByteMaskIndex);
}

sl_zigbee_af_ota_storage_status_t sl_zigbee_af_ota_storage_driver_prepare_to_resume_download_cb(void)
{
  uint32_t otaEepromStart;
  uint32_t pageOffsetStart;
  uint32_t pageSize;

  if (lastRecordedByteMaskIndex < 0) {
    return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
  }

  otaEepromStart = otaStorageEepromGetStorageStartAddress();
  pageSize = sl_util_af_eeprom_info()->pageSize;

  pageOffsetStart = (lastRecordedByteMaskIndex + 1) << determinePageSizeLog();

  return (startEraseOperation(otaEepromStart + pageOffsetStart,
                              otaEepromStart + pageOffsetStart + pageSize)
          ? SL_ZIGBEE_AF_OTA_STORAGE_OPERATION_IN_PROGRESS
          : SL_ZIGBEE_AF_OTA_STORAGE_ERROR);
}

#endif // !READ_MODIFY_WRITE_SUPPORT

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_CLI_PRESENT

#include "sl_cli.h"
void sli_zigbee_af_ota_wipe_storage_device_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  sl_zigbee_af_ota_storage_driver_invalidate_image_cb();
}
#endif // SL_CATALOG_CLI_PRESENT
