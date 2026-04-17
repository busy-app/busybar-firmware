/***************************************************************************//**
 * @file sl_token_manufacturing.c
 * @brief Manufacturing toke API implementation.
 * @version 1.0.0
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"
#include "sl_component_catalog.h"
#include "sl_token_manager.h"
#include "sl_token_api.h"
#include "sl_token_manufacturing_api.h"
#if defined(_SILICON_LABS_32B_SERIES_2)
#include "em_msc.h"
#endif // (_SILICON_LABS_32B_SERIES_2)

#if (_SILICON_LABS_32B_SERIES_2_CONFIG == 1)
#include "sl_se_manager_util.h"
#endif // (_SILICON_LABS_32B_SERIES_2_CONFIG == 1)

#if defined(_SILICON_LABS_32B_SERIES_3)
#include "sl_se_manager.h"
#include "sl_se_manager_extmem.h"
#endif //_SILICON_LABS_32B_SERIES_3

#define DEFINETOKENS
#define TOKEN_MFG(name, creator, iscnt, isidx, type, arraysize, ...) \
  const uint16_t TOKEN_##name = TOKEN_##name##_ADDRESS;
// Multiple inclusion of unguarded token-related header files is by design; suppress violation.
//cstat !MISRAC2012-Dir-4.10
  #include "sl_token_manufacturing.h"
#undef TOKEN_DEF
#undef TOKEN_MFG
#undef DEFINETOKENS

#if defined(_SILICON_LABS_32B_SERIES_2) || defined(_SILICON_LABS_32B_SERIES_3)
#define SILABS_DEVINFO_EUI64_LOW   (DEVINFO->EUI64L)
#define SILABS_DEVINFO_EUI64_HIGH  (DEVINFO->EUI64H)
#else
  #error The flash map of manufacturing tokens is not available for this device
#endif

static const uint8_t nullEui[] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU };

#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS) || defined(USE_NVM3)
// NOTE: Added for zigbee purpose. these values are used only by ZB for now.
static const char* mfg_string_token = "MFG Tkn NVM3";
static uint16_t mfg_security_config_token = 0xFFFF; // Reading + Hashing allowed. See security-config-mfg.c
#endif //defined(SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS) || defined(USE_NVM3)

static void getMfgTokenData(void *data,
                            uint16_t token,
                            uint8_t index,
                            uint32_t len)
{
  uint8_t *ram = (uint8_t*)data;

  //0x7F is a non-indexed token.  Remap to 0 for the address calculation
  index = (index == 0x7FU) ? 0U : index;

  if (token == MFG_EUI_64_LOCATION) {
    //There are two EUI64's stored in the flash, Ember and Custom.
    //MFG_EUI_64_LOCATION is the address used by the generic EUI64 token.
    //It is the low level routine's responbility to pick and return EUI64 from
    //either Ember or Custom.  Return the Custom EUI64 if it is not all FF's,
    //otherwise return the Ember EUI64.
    tokTypeMfgEui64 eui64;
    (void)sl_token_get_manufacturing_data(TOKEN_MFG_CUSTOM_EUI_64, 0x7F, &eui64, TOKEN_MFG_CUSTOM_EUI_64_SIZE);
    if (memcmp(eui64, nullEui, 8 /*EUI64_SIZE*/) == 0) {
      (void)sl_token_get_manufacturing_data(TOKEN_MFG_EMBER_EUI_64, 0x7F, &eui64, TOKEN_MFG_EMBER_EUI_64_SIZE);
    }
    memcpy(ram, eui64, 8 /*EUI64_SIZE*/);
  } else if (token == MFG_EMBER_EUI_64_LOCATION) {
    uint32_t low = SILABS_DEVINFO_EUI64_LOW;
    uint32_t high = SILABS_DEVINFO_EUI64_HIGH;
    uint8_t i = 0U;
    while ((i < 4U) && (len > 0U)) {
      ram[i] = low & 0xFFU;
      low >>= 8;
      len--;
      i++;
    }
    while ((i < 8U) && (len > 0U)) {
      ram[i] = high & 0xFFU;
      high >>= 8;
      len--;
      i++;
    }
  } else if ((token & 0xF000) == (USERDATA_TOKENS & 0xF000)) {
#if (_SILICON_LABS_32B_SERIES == 2)
    uint32_t realAddress = ((USERDATA_BASE + (token & 0x0FFF)) + (len * index));
    uint8_t *flash = (uint8_t *)realAddress;

    memcpy(ram, flash, len);
#endif
  } else if (((token & 0xF000) == (LOCKBITS_TOKENS & 0xF000))
             || ((token & 0xF000) == (LOCKBITSDATA_TOKENS & 0xF000))) {
    uint32_t realAddress = ((LOCKBITS_BASE + (token & 0x0FFF)) + (len * index));
    uint8_t *flash = (uint8_t *)realAddress;

    memcpy(ram, flash, len);
  } else {
    // Sate MISRA
  }
}

/***************************************************************************//**
 * Read the data associated with the specified manufacturing token.
 ******************************************************************************/
sl_status_t sl_token_get_manufacturing_data(uint32_t token,
                                            uint32_t index,
                                            void *data,
                                            uint32_t length)
{
  if (length == 0U) {
    return SL_STATUS_INVALID_PARAMETER;   // Nothing to do...
  }

#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS) || defined(USE_NVM3)
  // In case of static token support alone, we don't need to handle override tokens.
  sl_status_t status = SL_STATUS_OK;

  if (nvm3OverrideActive == true) {
    // NOTE: Added for zigbee purpose
    if (token == TOKEN_MFG_EUI_64) {
      tokTypeMfgEui64 eui64;

      index = (index == 0x7FU) ? 0U : index;
      memset(eui64, 0xFFU, sizeof(tokTypeMfgEui64));
      // Check if the Custom EUI64 is set, if not, return the Ember EUI64
      status = sl_token_get_data(SL_TOKEN_GET_DYNAMIC_OVERRIDE_TOKEN(TOKEN_MFG_CUSTOM_EUI_64),
                                 index,
                                 &eui64,
                                 length);
      if ((memcmp(eui64, nullEui, sizeof(eui64)) == 0)) {
        getMfgTokenData(&eui64, TOKEN_MFG_EMBER_EUI_64, index, length);
      }
      memcpy((char*)data, eui64, sizeof(eui64));
    } else if (token == TOKEN_MFG_EMBER_EUI_64) {
      getMfgTokenData(data, token, index, length);
    } else if (token == TOKEN_MFG_STRING) {
      memcpy(data, mfg_string_token, strlen(mfg_string_token));
    } else if (token == TOKEN_MFG_SECURITY_CONFIG) {
      memcpy(data, &mfg_security_config_token, sizeof(mfg_security_config_token));
    } else {
      //0x7F is a non-indexed token.  Remap to 0 for the address calculation
      index = (index == 0x7FU) ? 0U : index;
      //Read the token from NVM3
      status = sl_token_get_data(SL_TOKEN_GET_DYNAMIC_OVERRIDE_TOKEN(token),
                                 index,
                                 data,
                                 length);
    }
    //Any outcome of the read, return from here
    return status;
  }
#endif // SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS || USE_NVM3

  if (token == MFG_EUI_64_LOCATION) {
    //There are two EUI64's stored in the flash, Ember and Custom.
    //MFG_EUI_64_LOCATION is the address used by the generic EUI64 token.
    //It is the low level routine's responsibility to pick and return EUI64 from
    //either Ember or Custom.  Return the Custom EUI64 if it is not all FF's,
    //otherwise return the Ember EUI64.
    if (length > sizeof(nullEui)) {
      length = sizeof(nullEui);
    }
    getMfgTokenData(data, MFG_CUSTOM_EUI_64_LOCATION, 0x7FU, length);
    if (memcmp(data, nullEui, length) != 0) {
      return SL_STATUS_OK;
    }
    token = MFG_EMBER_EUI_64_LOCATION;
  }
  getMfgTokenData(data, token, index, length);

  return SL_STATUS_OK;
}

/**
 * @brief Assign numerical value to the type of erasure requested.
 */
#define MFB_MASS_ERASE 0x01
#if defined(_SILICON_LABS_32B_SERIES_2)
// The parameter 'eraseType' chooses which erasure will be performed while
// the 'address' parameter chooses the page to be erased during MFB page erase.
sl_status_t halInternalFlashErase(uint8_t eraseType, uint32_t address)
{
  msc_Return_TypeDef ret = mscReturnOk;

  MSC_Init();
  if (eraseType == MFB_MASS_ERASE) {
#if defined (_SILICON_LABS_32B_SERIES_2)
    // masserase is currently not support on Series2 devices.
    return SL_STATUS_FAIL;
#else
    ret = MSC_MassErase();
#endif // !defined (_SILICON_LABS_32B_SERIES_2)
  } else {
    ret = MSC_ErasePage((uint32_t *) address);
  }

  /* Check for errors. If there are errors, set the global error variable and
   * de-initialize the MSC */
  if (ret != mscReturnOk) {
    MSC_Deinit();
    return SL_STATUS_FAIL;
  }

  /* De-initialize the MSC. This disables writing and locks the MSC */
  MSC_Deinit();

  return SL_STATUS_OK;
}

// The parameter 'address' defines the starting address of where the
// programming will occur - this parameter MUST be half-word aligned since all
// programming operations are HW.  The parameter 'data' is a pointer to a buffer
// containing the 16bit half-words to be written.  Length is the number of 16bit
// half-words contained in 'data' to be written to flash.
// NOTE: This function can NOT write the option bytes and will throw an error
// if that is attempted.
sl_status_t halInternalFlashWrite(uint32_t address, uint16_t *data, uint32_t length)
{
  // halInternalFlashWriteSeries2 should be called for more efficient writing, the following
  // is for backwards compatibility
  uint32_t byteCount, *fp32, i, wordToWrite;
  uint16_t *dp16;

  // check if even number of 16 bit half words
  if ((length & 1U) != 0U) {
    return SL_STATUS_FAIL;
  }

  // adjust length to number of 32 bit words
  length >>= 1;

  // hardcode byteCount to 4
  byteCount = 4U;

  // check if data address and address are 4 byte aligned
  if ((address & 3U) != 0U) {
    return SL_STATUS_FAIL; //UNALIGNED;
  }

  // First, do a pass over flash and data and verify that the current flash
  // value is 0xFFFFFFFF.  If this is untrue for any 32-bit word that is being
  // written, return a failure without writing anything
  fp32      = (uint32_t *)address;
  i = 0;
  while (i < length) {
    if (*fp32 != 0xFFFFFFFFU) {
      return SL_STATUS_FLASH_PROGRAM_FAILED;
    }
    ++fp32;
    ++i;
  }

  i = 0;
  fp32 = (uint32_t *)address;
  dp16 = data;

  MSC_Init();

  // write data in 32 bit (1 word) increments
  while (i < length) {
    // create 32 bit wordToWrite and increment data address
    wordToWrite = (uint32_t)*dp16;
    ++dp16;
    wordToWrite = (wordToWrite) | ((uint32_t)*dp16 << 16);
    ++dp16;

    // attempt to write word
    if (MSC_WriteWord(fp32, (void const *)&wordToWrite, byteCount) != mscReturnOk) {
      MSC_Deinit();
      return SL_STATUS_FLASH_PROGRAM_FAILED;
    }

    // increment flash address
    ++fp32;

    // increment counter
    ++i;
  }

  MSC_Deinit();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * This function is used to write data to the User or Lockbits page.
 * Odd len is not supported by this function
 ******************************************************************************/
static void hal_flash_write(uint32_t realAddress, void *data, uint32_t len)
{
  sl_status_t flashStatus = SL_STATUS_FAIL;
  flashStatus = halInternalFlashWrite(realAddress, (uint16_t*)data, (len / 2));
  assert(flashStatus == SL_STATUS_OK);
}

#endif

#if defined(_SILICON_LABS_32B_SERIES_3)
#define CHECK_DATA  1           ///< Macro defining if data should be checked
/***************************************************************************//**
 * Open the MFG hal for usage. SE will take care of clocking the externalflash.
 * This function initializes the SE lock mutex and checks if LOCKBITS
 * start and end addresses are within the data region or not.
 ******************************************************************************/
static sl_status_t hal_external_flash_open(void)
{
  sl_status_t slStatus;
  void *startAdr;
  size_t regSize;
  sl_se_command_context_t cmd_ctx;

  slStatus = sl_se_init();

  if (slStatus != SL_STATUS_OK) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sl_se_init_command_context(&cmd_ctx);
  slStatus = sl_se_data_region_get_location(&cmd_ctx, &startAdr, &regSize);

  if (slStatus == SL_STATUS_OK) {
    // Check if LOCKBITS page's start and end addresses are within the data region or not
    if (((size_t)LOCKBITS_BASE < (size_t)startAdr)
        || (((size_t)LOCKBITS_END) > ((size_t)startAdr + regSize))) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  } else {
    return SL_STATUS_INVALID_STATE;
  }

  return slStatus;
}

/***************************************************************************//**
 * This function is used to write data to the LOCKBITS page, using SE API.
 ******************************************************************************/
static void hal_external_flash_write(uint32_t dstAdr, void const *src, size_t length)
{
  sl_status_t slStatus;
  sl_se_command_context_t cmd_ctx;
  size_t byte_count = length;
  size_t word_count = byte_count / sizeof(uint32_t);

  //make it word aligned
  if ((byte_count % sizeof(uint32_t)) != 0) {
    word_count = word_count + 1;
  }

  sl_se_init_command_context(&cmd_ctx);
  slStatus = sl_se_data_region_write(&cmd_ctx, (void *)dstAdr, src, (word_count * sizeof(uint32_t)));

#if CHECK_DATA
  if (slStatus == SL_STATUS_OK) {
    if (memcmp((void *)dstAdr, src, byte_count) != 0) {
      slStatus = SL_STATUS_FAIL;
    }
  }
#endif

  assert(slStatus == SL_STATUS_OK);
}
#endif

/***************************************************************************//**
 * This function is used to initialize SE
 ******************************************************************************/
void sl_token_mfg_init(void)
{
#if defined(_SILICON_LABS_32B_SERIES_3)
  sl_status_t slStatus;
  //Init SE manager
  slStatus = hal_external_flash_open();
  assert(slStatus == SL_STATUS_OK);
#endif
}

#if (_SILICON_LABS_32B_SERIES_2_CONFIG == 1)
static void halFlashWriteSE(uint32_t realAddress, void *data, uint32_t len)
{
  if ((realAddress & USERDATA_BASE) == USERDATA_BASE) {
    sl_status_t status = SL_STATUS_OK;
    sl_se_command_context_t cmd_ctx;
    status = sl_se_write_user_data(&cmd_ctx, (realAddress & 0x0FFF), data, len);
    assert(status == SL_STATUS_OK);
  } else {
    hal_flash_write(realAddress, data, len);
  }
}
#endif // (_SILICON_LABS_32B_SERIES_2_CONFIG == 1)

#if (_SILICON_LABS_32B_SERIES == 2)
  #if (_SILICON_LABS_32B_SERIES_2_CONFIG == 1)
    #define FLASHWRITE(realAddress, data, len) \
  (halFlashWriteSE((realAddress), (data), (len)))
  #elif (_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 3)    \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 4)    \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 5)    \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6)    \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 7)    \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8)    \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 9)    \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 13)
    #define FLASHWRITE(realAddress, data, len) \
  (hal_flash_write((realAddress), (data), (len)))
  #else
    #error Unknown device configuration
  #endif
#elif (_SILICON_LABS_32B_SERIES == 3)
#define FLASHWRITE(realAddress, data, len) \
  (hal_external_flash_write((realAddress), (data), (len)))
#else
  #error Unknown device series
#endif

#if (_SILICON_LABS_32B_SERIES == 2)
static void writeStartWord(uint32_t realAddress, void *data)
{
  uint32_t startWordAddress, startWordToWrite;
  startWordAddress = realAddress - 2;
  // create word to write buffer bytes and first 2 bytes of token
  startWordToWrite = 0x0000FFFF | (*(uint16_t *)data << 16);
  FLASHWRITE(startWordAddress, &startWordToWrite, 4);
}

static void writeEndWord(uint32_t realAddress, void *data, uint32_t len)
{
  uint32_t endWordAddress, endWordToWrite;
  // address for end word at aligned address where the last two bytes of data would
  // be written
  endWordAddress = realAddress + len;
  // create word to write last 2 bytes of token and buffer bytes
  endWordToWrite = 0xFFFF0000 | *((uint16_t *)data + len / 2);
  FLASHWRITE(endWordAddress, &endWordToWrite, 4);
}
#endif

/***************************************************************************//**
 * Writes data to a manufacturing token.
 *
 * @note Only manufacturing token values that have not been written since
 * the last erase can be written.  For areas of flash that cannot be erased
 * by user code, those manufacturing tokens are effectively write-once.
 *
 * @param token The NVM3KEY define for the token.
 * @param data A pointer to the data being written.
 * @param length The size of the token data in number of bytes.
 *
 * @return 0 if successful. Error code otherwise.
 ******************************************************************************/
sl_status_t sl_token_set_manufacturing_data(uint32_t token,
                                            void *data,
                                            uint32_t length)
{
  uint32_t realAddress = 0;
  //Initializing to a high memory address adds protection by causing a
  //hardfault if accidentally used.
  uint8_t *flash = (uint8_t *)0xFFFFFFF0U;
  uint32_t i;

  //The flash library requires the address and length to both
  //be multiples of 16bits.  Since this API is only valid for writing to
  //the UserPage or LockBits page, verify that the token+len falls within
  //acceptable addresses..
  assert((token & 1) != 1);
  assert((length & 1) != 1);

#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS) || defined(USE_NVM3)
  // In case of static token support alone, we don't need to handle override tokens.
  sl_status_t status = SL_STATUS_OK;
  if (nvm3OverrideActive == true) {
    // NOTE: Added for zigbee purpose
    if (token == TOKEN_MFG_SECURITY_CONFIG) {
      memcpy(&mfg_security_config_token, data, length);
    } else {
      //Write the token into NVM3
      status = sl_token_set_data(SL_TOKEN_GET_DYNAMIC_OVERRIDE_TOKEN(token),
                                 0x7F,
                                 data,
                                 length);
    }
    //Any outcome of the write, return from here
    return status;
  }
#endif

  if ((token & 0xF000) == (USERDATA_TOKENS & 0xF000)) {
#if (_SILICON_LABS_32B_SERIES == 2)
    realAddress = ((USERDATA_BASE + (token & 0x0FFF)));
    flash = (uint8_t *)realAddress;
    assert((realAddress >= USERDATA_BASE) && ((realAddress + length - 1) <= USERDATA_END));
#endif
  } else if (((token & 0xF000) == (LOCKBITS_TOKENS & 0xF000))
             || ((token & 0xF000) == (LOCKBITSDATA_TOKENS & 0xF000))) {
    realAddress = ((LOCKBITS_BASE + (token & 0x0FFF)));
    flash = (uint8_t *)realAddress;
  } else {
    //in coming token parameter is an illegal address.
    assert(0);
  }

#if (_SILICON_LABS_32B_SERIES == 2)
  //UserData and LockBits manufacturing tokens can only be written by on-chip
  //code if the token is currently unprogrammed.  Verify the entire token is
  //unwritten.  The flash library performs a similar check, but verifying here
  //ensures that the entire token is unprogrammed and will prevent partial
  //writes.
  //NOTE:  On chip code CANNOT erase LockBits Page.
  for (i = 0; i < length; i++) {
    assert(flash[i] == 0xFF);
  }

  // sate any compiler warnings about unused variable.
  (void) flash;

  // if address is 2 byte aligned instead of 4, write two buffer bytes of 0xFFFF with
  // the first 2 bytes of data separately from the rest of data 2 bytes before
  // realAddress. There is buffer space built into the token map to account for these
  // extra two bytes at the beginning
  if ((realAddress & 3U) != 0U) {
    writeStartWord(realAddress, data);
    // adjust realAddress, data pointer and byte count
    realAddress += 2;
    data = (uint16_t *)data + 1;
    length -= 2;
  }

  // If data size is 2 byte aligned instead of 4, write the last 2 bytes of data and
  // two buffer bytes of 0xFFFF at the end of the token separately from the rest of
  // data. Buffer space is built into the mfg token map to account for the extra bytes
  if ((length & 3U) != 0) {
    // adjust to new length
    length -= 2;
    writeEndWord(realAddress, data, length);
  }
#elif (_SILICON_LABS_32B_SERIES == 3)
  (void) flash;
  (void) i;
#endif

  if (length > 0) {
    FLASHWRITE(realAddress, data, length);
  }
  return SL_STATUS_OK;
}
