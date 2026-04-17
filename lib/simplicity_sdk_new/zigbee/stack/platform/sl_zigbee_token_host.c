/***************************************************************************//**
 * @brief ZigBee token management code for host.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

// For ftruncate(2) in glibc and to resolve O_CLOEXEC
#define _XOPEN_SOURCE 700

#include <sys/mman.h>
#include <sys/stat.h>
#include <err.h>
#include <fcntl.h>
#include <sysexits.h>
#include <unistd.h>
#include <stdlib.h>

#include "sl_zigbee_types.h"
#include "sl_zigbee_token.h"
#include "stack/config/sl_zigbee_token_defines.h"
#include "sl_token_manager_api.h"

#define VERSION 2

static void initializeTokenSystem(void);
static void resetTokenData(void);
static size_t getNvmOffset(uint32_t token, uint32_t len);
/* TODO - EMZIGBEE-14548: Fix version 1 token file upgrades
   static void upgradeTokenFileToVersion2(int fd);
 */

#ifndef SL_ZIGBEE_AF_TOKEN_FILENAME
    #define SL_ZIGBEE_AF_TOKEN_FILENAME "host_token.nvm"
#endif
#include <stdio.h>
// #define SL_ZIGBEE_AF_HOST_TOKEN_DEBUG
#ifdef SL_ZIGBEE_AF_HOST_TOKEN_DEBUG
  #define hostTokenDebugPrintf(...) printf(__VA_ARGS__)
#else
  #define hostTokenDebugPrintf(...)
#endif

int fd;

// mmap(2) returns MAP_FAILED on failure, which is not necessarily defined the
// same as NULL.
static uint8_t *nvm = MAP_FAILED;
static bool nvm_initialized = false;
#define isInitialized() (nvm_initialized)

extern uint32_t *tokenNvm3Keys;
extern bool *tokenIsCnt;
extern bool *tokenIsIdx;
extern uint32_t *tokenSize;
extern uint8_t *tokenArraySize;
extern uint8_t *tokenDefaults;
extern uint32_t sli_zigbee_token_count;
extern uint32_t sli_zigbee_token_total_size;
extern uint32_t sli_zigbee_base_token_total_size;

extern uint32_t legacy_token_count;
extern uint16_t legacy_token_addresses[];
extern uint16_t legacy_token_sizes[];

extern sl_status_t sli_zigbee_stack_initialize_token(uint32_t token_base,
                                                     void *default_token_value,
                                                     uint32_t token_size,
                                                     uint8_t token_index_size,
                                                     bool token_is_counter);

// TODO how do we get this define on the host?
#define NVM3_KEY_SIZE               20U                           ///< Unique object key identifier size in number of bits
#define NVM3_KEY_MASK               ((1U << NVM3_KEY_SIZE) - 1U)  ///< Unique object key identifier mask

// tokenSize is the uint8_t, kept at 1 byte for backwards compatibility
#define PER_TOKEN_OVERHEAD  \
  (sizeof(tokenNvm3Keys[0]) \
   + sizeof(tokenIsCnt[0])  \
   + sizeof(uint8_t)        \
   + sizeof(tokenArraySize[0]))
#define TOTAL_SIZE                               \
  (1 /* version overhead */                      \
   + sli_zigbee_token_count * PER_TOKEN_OVERHEAD \
   + sli_zigbee_token_total_size)

typedef struct {
  size_t offset; // offset from start of nvm file
  bool present;  // true if entry is present in nvm file
} nvmCreatorOffsetType;

// keeps track of token offsets in nvm file (helps with rearranged tokens)
// when populating, each index is maintained to be the same as the creator's
// index in tokenNvm3Keys[], tokenIsCnt[] etc.

static nvmCreatorOffsetType *nvmCreatorOffset;

// Special case for security config for host apps
#if defined(ZIGBEE_STACK_ON_HOST)
typedef uint16_t tokTypeMfgSecurityConfig;
static tokTypeMfgSecurityConfig tokTypeMfgSecurityConfigData = 0xFFFF;
#define SL_TOKEN_STATIC_SECURE_DATA_TOKENS        (0x3000)
#define TOKEN_MFG_SECURITY_CONFIG                 (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x260)
#define TOKEN_MFG_SECURITY_CONFIG_SIZE            (2u)
#endif

static sl_status_t read_from_nvm(uint32_t token, void *data, uint32_t length)
{
  {
    DECLARE_INTERRUPT_STATE;
    DISABLE_INTERRUPTS();
    size_t offset = getNvmOffset(token, length);
    memcpy(data, nvm + offset, length);
    RESTORE_INTERRUPTS();
  }
  return SL_STATUS_OK;
}

uint32_t getNewTokenId(uint32_t old_token)
{
  if (legacy_token_count < 1) {
    return old_token;
  }

  uint8_t new_token_index[legacy_token_count];

  // Invalidate to start
  for (uint8_t i = 0; i < legacy_token_count; i++) {
    new_token_index[i] = 0xFF;
  }

  // Then check sizes and set if nonzero token size
  uint8_t new_index = 0;
  for (uint8_t i = 0; i < legacy_token_count - 1; i++) {
    if (legacy_token_addresses[i] < legacy_token_addresses[i + 1]) {
      new_token_index[i] = new_index++;
    }
  }

  return new_token_index[old_token];
}

sl_status_t sl_token_get_data(uint32_t token, uint32_t index, void *data, uint32_t length)
{
  // Old token IDs are 0-based enumerations, whereas new token IDs refer to the
  // KLV ID in the new Common Token Manager. These new IDs contain the NVM3 key ID
  // embedded in the value
  // To get the NVM offset within the local file on disk, we need the NVM3 key ID
  // derived from the old token ID. We cannot just dereference tokenNvm3Keys with
  // token because the old token indices do not match up to the new IDs (this is
  // because the new system rejects any 0-length tokens, whereas the old system
  // had to admit all token sizes)
  // To account for this, we convert the old token ID to the new ID
  uint32_t new_token = getNewTokenId(token);

  if (new_token >= sli_zigbee_token_count) {
    // We don't know about this token. DEFINE_*_TOKEN must have never been called
    return SL_STATUS_NOT_INITIALIZED;
  }

  uint32_t nvm3_key = tokenNvm3Keys[new_token];

  // A difference with respect to the new API is that in this API, we receive the
  // base token ID with an index; in the new API, the token ID is the NVM3 key ID
  // already offset by the index. So, here we bump the NVM3 key by the index
  nvm3_key += index;

  return read_from_nvm(nvm3_key, data, length);
}

sl_status_t sl_token_manager_get_data(uint32_t token, void *data, uint32_t length)
{
  // Special case for security config for host apps
  #if defined(ZIGBEE_STACK_ON_HOST)
  if (token == SL_TOKEN_GET_STATIC_SECURE_TOKEN(TOKEN_MFG_SECURITY_CONFIG)) {
    uint16_t* ptr = (uint16_t*)data;
    *ptr = tokTypeMfgSecurityConfigData;
    return SL_STATUS_OK;
  }
  #endif
  // Host code only supports reading and writing dynamic tokens, which are NVM3 objects
  // These NVM3 objects are actually written on disk to a file
  // There are no MFG tokens (or static tokens) on the host
  if ((token & SL_TOKEN_DYNAMIC_MASK) != SL_TOKEN_TYPE_NVM3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  uint32_t nvm3_key = token & NVM3_KEY_MASK;

  return read_from_nvm(nvm3_key, data, length);
}

static sl_status_t write_to_nvm(uint32_t token, void *data, uint32_t length)
{
  {
    DECLARE_INTERRUPT_STATE;
    DISABLE_INTERRUPTS();
    size_t offset = getNvmOffset(token, length);
    memcpy(nvm + offset, data, length);
    if (msync(nvm, sli_zigbee_token_total_size, MS_SYNC) == -1) {
      err(EX_IOERR, "Could not write " SL_ZIGBEE_AF_TOKEN_FILENAME " to disk");
    }
    RESTORE_INTERRUPTS();
  }
  return SL_STATUS_OK;
}

sl_status_t sl_token_set_data(uint32_t token, uint32_t index, void *data, uint32_t length)
{
  uint32_t nvm3_key = tokenNvm3Keys[token];

  nvm3_key += index; // getNvmOffset knows how to handle an indexed token

  return write_to_nvm(nvm3_key, data, length);
}

sl_status_t sl_token_manager_set_data(uint32_t token, void *data, uint32_t length)
{
  // Special case for security config for host apps
  #if defined(ZIGBEE_STACK_ON_HOST)
  if (token == SL_TOKEN_GET_STATIC_SECURE_TOKEN(TOKEN_MFG_SECURITY_CONFIG)) {
    uint16_t* ptr = (uint16_t*)data;
    assert(tokTypeMfgSecurityConfigData == 0xFFFF);
    tokTypeMfgSecurityConfigData = *ptr;
    return SL_STATUS_OK;
  }
  #endif
  // Host code only supports reading and writing dynamic tokens, which are NVM3 objects
  // These NVM3 objects are actually written on disk to a file
  // There are no MFG tokens (or static tokens) on the host
  if ((token & SL_TOKEN_DYNAMIC_MASK) != SL_TOKEN_TYPE_NVM3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  uint32_t nvm3_key = token & NVM3_KEY_MASK;

  return write_to_nvm(nvm3_key, data, length);
}

// This function (aptly named) finds the given token (either "base" token or offset by index for index tokens)
// and returns the index of the given token for accessing the arrays (tokenNvm3Keys, tokenSize, tokenArraySize).
sl_status_t get_token_index(uint32_t token, uint32_t *token_index)
{
  uint32_t indexed_token_start_id = 0;
  uint32_t indexed_token_end_id = 0;
  uint32_t nvm3_key = token & NVM3_KEY_MASK;

  for (uint32_t i = 0; i < sli_zigbee_token_count; i++) {
    indexed_token_start_id = tokenNvm3Keys[i];
    // An indexed token (which has tokenArraySize[i] > 1) consumes tokenArraySize[i] NVM3 key IDs
    indexed_token_end_id = indexed_token_start_id + (tokenArraySize[i] - 1);
    if ((nvm3_key >= indexed_token_start_id) && (nvm3_key <= indexed_token_end_id)) {
      *token_index = i;
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

sl_status_t sl_token_manager_get_partial_data(uint32_t token,
                                              void *data,
                                              uint32_t offset,
                                              uint32_t length)
{
  sl_status_t status = SL_STATUS_OK;

  // Host code only supports reading and writing dynamic tokens, which are NVM3 objects
  // These NVM3 objects are actually written on disk to a file
  // There are no MFG tokens (or static tokens) on the host
  if ((token & SL_TOKEN_DYNAMIC_MASK) != SL_TOKEN_TYPE_NVM3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // get the token's entire size
  uint32_t token_length = 0;
  status = sl_token_manager_get_size(token, &token_length);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // validate length of data to be read does not go beyond total token length
  if (token_length - offset < length) {
    return SL_STATUS_WOULD_OVERFLOW;
  }

  // get the token's entire data entry
  uint8_t *full_token_data = (uint8_t *)malloc(token_length);
  if (!full_token_data) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }
  status = sl_token_manager_get_data(token, full_token_data, token_length);
  if (status != SL_STATUS_OK) {
    free(full_token_data);
    return status;
  }

  // copy over the requested portion of data
  memcpy(data, full_token_data + offset, length);
  free(full_token_data);
  return status;
}

sl_status_t sl_token_manager_delete_dynamic_token(uint32_t token)
{
  // TODO: EMZIGBEE-14595 add support for dynamic token deletion
  UNUSED_VAR(token);
  hostTokenDebugPrintf("Warning: dynamic token deletion is not supported on Host");
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_token_manager_get_size(uint32_t token,
                                      uint32_t *size_out)
{
  assert(isInitialized());

  if (size_out == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  // Host code only supports reading and writing dynamic tokens, which are NVM3 objects
  // These NVM3 objects are actually written on disk to a file
  // There are no MFG tokens (or static tokens) on the host
  if ((token & SL_TOKEN_DYNAMIC_MASK) != SL_TOKEN_TYPE_NVM3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // get token's index into the arrays
  uint32_t token_index;
  if (get_token_index(token, &token_index) != SL_STATUS_OK) {
    return SL_STATUS_NOT_FOUND;
  }
  // extra validation for accessing the tokenSize array
  if (token_index >= sli_zigbee_token_count) {
    return SL_STATUS_INVALID_KEY;
  }

  *size_out = tokenSize[token_index];
  return SL_STATUS_OK;
}

sl_status_t sl_token_manager_increment_counter(uint32_t token)
{
  uint32_t token_counter_value = 0;
  sl_status_t status = sl_token_manager_get_data(token, &token_counter_value, sizeof(uint32_t));
  if (status != SL_STATUS_OK) {
    return status;
  }
  token_counter_value++;
  status = sl_token_manager_set_data(token, &token_counter_value, sizeof(uint32_t));
  return status;
}

void halInternalGetMfgTokenData(void *data, uint16_t token, uint8_t index, uint8_t len)
{
  // No manufacturing tokens on host.
  UNUSED_VAR(data);
  UNUSED_VAR(token);
  UNUSED_VAR(index);
  UNUSED_VAR(len);

  assert(false);
}

void halInternalSetMfgTokenData(uint16_t token, void *data, uint8_t len)
{
  // No manufacturing tokens on host.
  UNUSED_VAR(data);
  UNUSED_VAR(token);
  UNUSED_VAR(len);

  assert(false);
}

// check if token in nvm file is present in stack/app
// if present, return new index in tokenNvm3Keys, else return false
static bool isTokenInitialized(uint32_t tokCreator, size_t* index)
{
  for (size_t i = 0; i < sli_zigbee_token_count; i++) {
    if (tokenNvm3Keys[i] == tokCreator) {
      *index = i;
      return true;
    }
  }
  return false;
}

// should we copy nvm values?
// if index is present (was populated) and token structure is same, return true
// else return false, so that we reset values
static bool copyNvm(uint8_t* nvmData,
                    size_t index,
                    size_t* tokOffset,
                    bool oldFile)
{
  uint8_t* nvmTokFinger;
  bool nvmTokIsCnt;
  size_t nvmTokSize, nvmTokArraySize;

  if (nvmCreatorOffset[index].present) {
    nvmTokFinger = nvmData + nvmCreatorOffset[index].offset;
    uint32_t tokCreator;
    uint8_t byte_diff;
    if (oldFile) {
      tokCreator = (SL_TOKEN_NVM3_REGION_ZIGBEE | ((nvmTokFinger[0] << 8) + nvmTokFinger[1]));
      byte_diff = 2;
    } else {
      tokCreator = (nvmTokFinger[0] << 24) + (nvmTokFinger[1] << 16) + (nvmTokFinger[2] << 8) + nvmTokFinger[3];
      byte_diff = 0;
    }
    assert(tokenNvm3Keys[index] == tokCreator);
    nvmTokIsCnt = nvmTokFinger[4 - byte_diff];
    nvmTokSize = nvmTokFinger[5 - byte_diff];
    nvmTokArraySize = nvmTokFinger[6 - byte_diff];

    if (tokenIsCnt[index] == nvmTokIsCnt
        && tokenSize[index] == nvmTokSize) {
      if (tokenArraySize[index] < nvmTokArraySize) {
        hostTokenDebugPrintf("Changed Token - array size reduced: %4x\n", tokenNvm3Keys[index]);
      } else if (tokenArraySize[index] > nvmTokArraySize) {
        hostTokenDebugPrintf("Changed Token - array size increased: %4x\n", tokenNvm3Keys[index]);
      } else {
        hostTokenDebugPrintf("Unchanged Token: %4x\n", tokenNvm3Keys[index]);
      }
      *tokOffset = nvmCreatorOffset[index].offset;
      return true;
    } else { // reset resized token
      hostTokenDebugPrintf("Changed Token - reset: %4x\n", tokenNvm3Keys[index]);
      return false;
    }
  }

  hostTokenDebugPrintf("New Token: %4x\n", tokenNvm3Keys[index]);
  return false;
}

static void initializeTokenSystem(void)
{
  assert(!isInitialized());

  hostTokenDebugPrintf("initializeTokenSystem: there are %d tokens of total length %d but with base token length %d\n", sli_zigbee_token_count,
                       sli_zigbee_token_total_size,
                       sli_zigbee_base_token_total_size);
  for (uint8_t i = 0; i < sli_zigbee_token_count; i++) {
    hostTokenDebugPrintf("Token %d -> tokenNvm3Keys %08X tokenIsCnt %d tokenIsIdx %d tokenArraySize %d tokenSize %d\n",
                         i,
                         tokenNvm3Keys[i],
                         tokenIsCnt[i],
                         tokenIsIdx[i],
                         tokenArraySize[i],
                         tokenSize[i]);
  }
  hostTokenDebugPrintf("There are %d legacy tokens initialized\n", legacy_token_count);

  fd = open(SL_ZIGBEE_AF_TOKEN_FILENAME,
            (O_RDWR | O_CREAT | O_CLOEXEC), // open read/write, create if missing, close on exit
            (S_IRUSR | S_IWUSR));    // read/write permissions for owner
  if (fd == -1) {
    err(EX_NOINPUT, "Could not open or create " SL_ZIGBEE_AF_TOKEN_FILENAME);
  }

  struct stat buf;
  if (fstat(fd, &buf) == -1) {
    err(EX_IOERR, "Could not determine size of " SL_ZIGBEE_AF_TOKEN_FILENAME);
  }

  bool reset = (buf.st_size == 0); // new or empty file
  if (reset) {
    if (ftruncate(fd, TOTAL_SIZE) == -1) {
      err(EX_IOERR, "Could not set size of " SL_ZIGBEE_AF_TOKEN_FILENAME);
    }
  }

  nvmCreatorOffset = (nvmCreatorOffsetType *)realloc(nvmCreatorOffset, sizeof(nvmCreatorOffsetType) * sli_zigbee_token_count);
  for (uint8_t t = 0; t < sli_zigbee_token_count; t++) {
    nvmCreatorOffset[t].present = false;
  }

  nvm = mmap(NULL,                     // let system choose address
             (buf.st_size == 0 ? TOTAL_SIZE : (size_t) buf.st_size),
             (PROT_READ | PROT_WRITE), // data can be read/written
             MAP_SHARED,               // writes change the file
             fd,
             0);                       // no offset
  if (nvm == MAP_FAILED) {
    err(EX_UNAVAILABLE, "Could not map " SL_ZIGBEE_AF_TOKEN_FILENAME " to memory");
  }

  if (!reset) {
    if (*nvm == 1 && VERSION == 2) {
      err(EX_CONFIG, "Cannot read v1 token file, please upgrade file to v2 using app built off of `sisdk-2024.6' branch first");
      /* TODO - EMZIGBEE-14548: Fix version 1 token file upgrades
         // upgrade token file from version 1 to version 2
         upgradeTokenFileToVersion2(fd);
         // we upgraded the token file then need to refresh its metadata info
         if (fstat(fd, &buf) == -1) {
         err(EX_IOERR, "Could not determine size of " SL_ZIGBEE_AF_TOKEN_FILENAME);
         }
       */
    }
    reset = (*nvm != VERSION);
  }

  if (!reset) {
    // save original content as we keep modifying "nvm"
    uint8_t *origNvm = (uint8_t*)malloc(buf.st_size * sizeof(uint8_t));
    if (!origNvm) {
      assert(0);
    }
    memcpy(origNvm, nvm, buf.st_size);

    bool oldFile = false;
    uint8_t byte_diff = 0;
    if (*nvm == 1 && VERSION == 2) {
      oldFile = true;
      byte_diff = 2;
    }

    hostTokenDebugPrintf("Reading from token file on disk. Version %d detected\n", *nvm);

    uint8_t *finger = origNvm + 1; // skip version; already verified

    // read token file and save old token offsets (helps with rearranged tokens)
    while ((finger - origNvm) < buf.st_size) { // iterate through origNvm
      uint32_t tokCreator;
      if (oldFile) {
        tokCreator = (SL_TOKEN_NVM3_REGION_ZIGBEE | ((finger[0] << 8) + finger[1]));
      } else {
        tokCreator = (finger[0] << 24) + (finger[1] << 16) + (finger[2] << 8) + finger[3];
      }
      bool tokIsCnt = finger[4 - byte_diff];
      uint8_t tokSize = finger[5 - byte_diff];
      uint8_t tokArraySize = finger[6 - byte_diff];
      size_t newIndex;
      if (isTokenInitialized(tokCreator, &newIndex) == false) {
        if (tokSize && tokArraySize) {
          // This just updates our arrays and token count. It won't write to down NVM because isInitialized() will return false
          sli_zigbee_stack_initialize_token(tokCreator, finger + PER_TOKEN_OVERHEAD - byte_diff, tokSize, tokArraySize, tokIsCnt);
          nvmCreatorOffset = (nvmCreatorOffsetType *)realloc(nvmCreatorOffset, sizeof(nvmCreatorOffsetType) * sli_zigbee_token_count);
          newIndex = sli_zigbee_token_count - 1;
          nvmCreatorOffset[newIndex].offset = (finger - origNvm);
          nvmCreatorOffset[newIndex].present = true;
          hostTokenDebugPrintf("Restoring previously written token 0x%4x size %d B and array size %d\n", tokCreator, tokSize, tokArraySize);
        } else {
          hostTokenDebugPrintf("Ingoring persisted token %4x because of size %d B or array size %d\n", tokCreator, tokSize, tokArraySize);
        }
      } else {
        hostTokenDebugPrintf("Restoring previously written token 0x%4x value\n", tokCreator);
        nvmCreatorOffset[newIndex].offset = (finger - origNvm);
        nvmCreatorOffset[newIndex].present = true;
      }
      finger += PER_TOKEN_OVERHEAD + (tokSize * tokArraySize) - byte_diff;
    }

    // If this asserts, either we are reading the file incorrectly (did we change
    // the format?) or the file was written incorrectly on a previous boot
    assert((finger - origNvm) == buf.st_size); // corrupt nvm file check

    // Now that we have all the tokens we are about, resize the file to the total number of
    // tokens that we care about
    if (ftruncate(fd, TOTAL_SIZE) == -1) {
      err(EX_IOERR, "Could not set size of " SL_ZIGBEE_AF_TOKEN_FILENAME);
    }

    if (fstat(fd, &buf) == -1) {
      err(EX_IOERR, "Could not determine size of " SL_ZIGBEE_AF_TOKEN_FILENAME);
    }

    // rewrite tokens file
    finger = nvm; // skip version; already verified
    *finger++ = VERSION;
    size_t i, j, nvmArraySizeToCopy, nvmTokOffset, nvmTokArraySize;
    uint8_t* nvmTokFinger = NULL;
    for (i = 0; i < sli_zigbee_token_count; i++) { // iterate through stack/app tokens
      *finger++ = (uint8_t)((tokenNvm3Keys[i] & 0xFF000000) >> 24);
      *finger++ = (uint8_t)((tokenNvm3Keys[i] & 0x00FF0000) >> 16);
      *finger++ = (uint8_t)((tokenNvm3Keys[i] & 0x0000FF00) >> 8);
      *finger++ = (uint8_t)(tokenNvm3Keys[i] & 0x000000FF);
      *finger++ = tokenIsCnt[i];
      *finger++ = tokenSize[i];
      *finger++ = tokenArraySize[i];
      nvmArraySizeToCopy = 0;

      if (copyNvm(origNvm, i, &nvmTokOffset, oldFile)) { // if true, get token offset from nvm
        nvmTokFinger = origNvm + nvmTokOffset;
        nvmTokArraySize = oldFile ? nvmTokFinger[2 + 1 + 1] : nvmTokFinger[4 + 1 + 1];
        nvmArraySizeToCopy = nvmTokArraySize < tokenArraySize[i] ? nvmTokArraySize : tokenArraySize[i];
        nvmTokFinger += PER_TOKEN_OVERHEAD - byte_diff;
      }

      // copy as many array elements as possible
      memcpy(finger,
             nvmTokFinger,
             nvmArraySizeToCopy * tokenSize[i]);

      finger += nvmArraySizeToCopy * tokenSize[i];

      // set the remaining array elements (if any) to default
      for (j = nvmArraySizeToCopy; j < tokenArraySize[i]; j++) {
        memcpy(finger,
               (uint8_t *)&tokenDefaults[i],
               tokenSize[i]);
        finger += tokenSize[i];
      }
    }
    free(origNvm);
    origNvm = NULL;
  }

  nvm_initialized = true;

  if (reset) {
    resetTokenData();
  }
}

static void resetTokenData(void)
{
  assert(isInitialized());

  uint8_t *finger = nvm;
  *finger++ = VERSION;

  // First write the token overhead to the file
  for (size_t i = 0; i < sli_zigbee_token_count; i++) {
    *finger++ = (uint8_t)((tokenNvm3Keys[i] & 0xFF000000) >> 24);
    *finger++ = (uint8_t)((tokenNvm3Keys[i] & 0x00FF0000) >> 16);
    *finger++ = (uint8_t)((tokenNvm3Keys[i] & 0x0000FF00) >> 8);
    *finger++ = (uint8_t)(tokenNvm3Keys[i] & 0x000000FF);
    *finger++ = tokenIsCnt[i];
    *finger++ = (uint8_t)tokenSize[i];
    *finger++ = tokenArraySize[i];
    finger += tokenArraySize[i] * tokenSize[i];
  }

  // Lastly, write the actual token values to file
  sl_status_t status = SL_STATUS_OK;
  for (uint32_t i = 0; i < sli_zigbee_token_count; i++) {
    uint32_t token = tokenNvm3Keys[i];
    uint32_t token_size = tokenSize[i];
    uint8_t token_index_size = tokenArraySize[i];

    uint8_t *default_token_value = (uint8_t *)malloc(token_size);
    if (!default_token_value) {
      assert(0);
    }
    status = sl_zigbee_get_token_default(token, default_token_value);
    assert(status == SL_STATUS_OK);

    for (uint8_t index_offset = 0; index_offset < token_index_size; index_offset++) {
      status = sl_token_manager_set_data(token + index_offset, (void *)default_token_value, token_size);
      assert(status == SL_STATUS_OK);
    }
    free(default_token_value);
  }
}

static size_t getNvmOffset(uint32_t token, uint32_t len)
{
  UNUSED_VAR(len);

  if (!isInitialized()) {
    initializeTokenSystem();
  }

  // First byte is version, then come the tokens
  size_t offset = 1;

  uint8_t i;
  uint32_t indexed_token_start_id = 0;
  uint32_t indexed_token_end_id = 0;

  // Given the token ID, we can find the NVM file offset of this token
  // by comparing the token ID against our saved keys in tokenNvm3Keys
  for (i = 0; i < sli_zigbee_token_count; i++) {
    offset += PER_TOKEN_OVERHEAD;
    indexed_token_start_id = tokenNvm3Keys[i];
    // An indexed token (which has tokenArraySize[i] > 1) consumes tokenArraySize[i] NVM3 key IDs
    indexed_token_end_id = indexed_token_start_id + (tokenArraySize[i] - 1);
    if ((token >= indexed_token_start_id) && (token <= indexed_token_end_id)) {
      break;
    } else {
      offset += (tokenSize[i] * tokenArraySize[i]);
    }
  }

  if (token > indexed_token_end_id) {
    printf("Error: trying to read token 0x%08X, which hasn't been initialized\n", token);
    assert(0);
  }

  // token is now greater than or equal to tokenNvm3Keys[i].
  // token might be one of the indices of an indexed token, so we
  // increase offset by (token - base_token) * token_size
  // (e.g. say it's the 3rd TOKEN_STACK_BINDING_TABLE entry, we need offset value 2,
  // which is token - COMMON_TOKEN_STACK_BINDING_TABLE)
  offset += ((token - tokenNvm3Keys[i]) * tokenSize[i]);

  return offset;
}

/* TODO - EMZIGBEE-14548: Fix version 1 token file upgrades
   static void upgradeTokenFileToVersion2(int fd)
   {
   struct stat buf;
   if (fstat(fd, &buf) == -1) {
    err(EX_IOERR, "Could not determine size of " SL_ZIGBEE_AF_TOKEN_FILENAME);
   }

   // backup version 1 token file so we can use that to copy data to version 2 token file
   uint8_t *oldTokenFileBuf = (uint8_t*)malloc(buf.st_size * sizeof(uint8_t));
   memcpy(oldTokenFileBuf, nvm, buf.st_size);

   if (ftruncate(fd, TOTAL_SIZE) == -1) {
    err(EX_IOERR, "Could not set size of " SL_ZIGBEE_AF_TOKEN_FILENAME);
   }

   // initialize new version 2 token file
   // We mark nvm as initialized temporarily so that we can write contents to the NVM file
   nvm_initialized = true;
   resetTokenData();
   nvm_initialized = false;

   uint8_t *newTokenFileFinger = nvm + 1; // skip version
   const uint8_t *oldTokenFileFinger = oldTokenFileBuf + 1; // skip version

   for (size_t token = 0; token < sli_zigbee_token_count; token++) {
    size_t nByteToCopy = (tokenSize[token] * tokenArraySize[token]);
    newTokenFileFinger += PER_TOKEN_OVERHEAD;
    oldTokenFileFinger += (PER_TOKEN_OVERHEAD - 2); // version 1 overhead is 2 byte less than version 2 overhead
    assert(oldTokenFileFinger < (oldTokenFileBuf + buf.st_size));
    memcpy(newTokenFileFinger, oldTokenFileFinger, nByteToCopy);
    newTokenFileFinger += nByteToCopy;
    oldTokenFileFinger += nByteToCopy;
   }

   if (msync(nvm, TOTAL_SIZE, MS_SYNC) == -1) {
    err(EX_IOERR, "Could not write " SL_ZIGBEE_AF_TOKEN_FILENAME " to disk");
   }
   free(oldTokenFileBuf);
   oldTokenFileBuf = NULL;
   }
 */
sl_status_t sli_zigbee_host_token_init(uint32_t token_base,
                                       void *default_token_value,
                                       uint32_t token_size,
                                       uint8_t token_index_size,
                                       bool token_is_counter)
{
  // At this point the token has been added to our arrays, like tokenNvm3Keys[]
  // We need to check if the token is in our NVM file on disk, and if not, add the token to that file

  // If we haven't initialized yet, the init routine will take care of everything for us
  if (!isInitialized()) {
    return SL_STATUS_OK;
  }

  uint8_t *finger = nvm;

  finger++; // skip version

  uint32_t nvm3_key = token_base & NVM3_KEY_MASK;

  // Find the token if it exists
  for (size_t i = 0; i < sli_zigbee_token_count; i++) {
    uint32_t existing_token_nvm3_key = (finger[0] << 24) + (finger[1] << 16) + (finger[2] << 8) + finger[3];
    if (existing_token_nvm3_key == nvm3_key) {
      hostTokenDebugPrintf("Token %04X already exists in NVM file\n", nvm3_key);
      return SL_STATUS_OK;
    }
    finger += sizeof(tokenNvm3Keys[0]);
    finger += sizeof(tokenIsCnt[0]);
    uint8_t existing_token_size = finger[0];
    finger += sizeof(uint8_t); // tokenSize limited to 1 byte in nvm file to remain backwards compatible
    uint8_t existing_token_array_size = finger[0];
    finger += sizeof(tokenArraySize[0]);
    finger += (existing_token_size * existing_token_array_size); // token data
  }

  // If we're here, it doesn't exist, so append
  struct stat buf;
  if (fstat(fd, &buf) == -1) {
    err(EX_IOERR, "Could not determine size of " SL_ZIGBEE_AF_TOKEN_FILENAME);
  }
  uint32_t nvm_offset_to_append = buf.st_size;

  // sli_zigbee_token_count has already been bumped, so TOTAL_SIZE reflects the new total size
  if (ftruncate(fd, TOTAL_SIZE) == -1) {
    err(EX_IOERR, "Could not set size of " SL_ZIGBEE_AF_TOKEN_FILENAME);
  }

  DECLARE_INTERRUPT_STATE;
  DISABLE_INTERRUPTS();
  finger = nvm + nvm_offset_to_append;
  *finger++ = (uint8_t)((token_base & 0xFF000000) >> 24);
  *finger++ = (uint8_t)((token_base & 0x00FF0000) >> 16);
  *finger++ = (uint8_t)((token_base & 0x0000FF00) >> 8);
  *finger++ = (uint8_t)(token_base & 0x000000FF);
  *finger++ = (uint8_t)token_is_counter;
  *finger++ = (uint8_t)token_size;
  *finger++ = token_index_size;
  for (uint8_t i = 0; i < token_index_size; i++) {
    memcpy(finger, default_token_value, token_size);
    finger += token_size;
  }
  RESTORE_INTERRUPTS();

  // Size check to ensure we wrote the correct number of bytes
  if (fstat(fd, &buf) == -1) {
    err(EX_IOERR, "Could not determine size of " SL_ZIGBEE_AF_TOKEN_FILENAME);
  }

  if (((finger - nvm) != buf.st_size)) {
    return SL_STATUS_FAIL;
  }

  hostTokenDebugPrintf("Token %04X added to NVM file at offset %d. New file size is %lld B\n",
                       nvm3_key,
                       nvm_offset_to_append,
                       buf.st_size);

  return SL_STATUS_OK;
}
