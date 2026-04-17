/***************************************************************************//**
 * @file
 * @brief A sample policy file that implements the callbacks for the
 * Zigbee Over-the-air bootload cluster server.
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
#include "app/framework/plugin/ota-server/ota-server.h"
#include "app/framework/plugin/ota-server/ota-server-dynamic-block-period.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"
#include "app/framework/util/util.h"
#include "app/framework/util/common.h"
#include "ota-server-policy.h"

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

typedef enum {
  UPGRADE_IF_SERVER_HAS_NEWER = 0,
  DOWNGRADE_IF_SERVER_HAS_OLDER = 1,
  REINSTALL_IF_SERVER_HAS_SAME = 2,
  NO_NEXT_VERSION = 3,
} sli_zigbee_next_version_policy_t;
#define QUERY_POLICY_MAX NO_NEXT_VERSION

#include "ota-server-config.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_CLI_PRESENT
#include "sl_cli_config.h"
#if (SL_CLI_HELP_DESCRIPTION_ENABLED == 1)
#define DESCRIPTION_FIELD
#endif // (SL_CLI_HELP_DESCRIPTION_ENABLED == 1)
#endif // SL_CATALOG_CLI_PRESENT
#if (SL_ZIGBEE_AF_PLUGIN_OTA_SERVER_PAGE_REQUEST_SUPPORT == 1)
#define OTA_SERVER_PAGE_REQUEST_SUPPORT
#endif

#ifdef DESCRIPTION_FIELD
static const char * nextVersionPolicyStrings[] = {
  "Upgrade if server has newer",
  "Downgrade if server has older",
  "Reinstall if server has same",
  "No next version",
};
#endif // DESCRIPTION_FIELD

static sli_zigbee_next_version_policy_t nextVersionPolicy = UPGRADE_IF_SERVER_HAS_NEWER;

// Image Block Response Message Format
// Status Code: 1-byte
// Manuf Code:  2-bytes
// Image Type:  2-bytes
// File Ver:    4-bytes
// File Offset: 4-bytes
// Data Size:   1-byte
// Data:        variable
#define IMAGE_BLOCK_RESPONSE_OVERHEAD (SL_ZIGBEE_AF_ZCL_OVERHEAD + 14)

#if defined EM_AF_TEST_HARNESS_CODE
typedef enum {
  SEND_BLOCK = 0,
  DELAY_DOWNLOAD_ONCE = 1,
  ABORT_DOWNLOAD = 2,
} sli_zigbee_image_block_request_policy_t;
#define BLOCK_REQUEST_POLICY_MAX ABORT_DOWNLOAD

static sli_zigbee_image_block_request_policy_t imageBlockRequestPolicy = SEND_BLOCK;

const char * imageBlockRequestPolicyStrings[] = {
  "Send block",
  "Delay download once",
  "Abort download",
};

#define IMAGE_BLOCK_REQUEST_DELAY_TIME_SECONDS (2 * 60)
#endif

typedef enum {
  UPGRADE_NOW = 0,
  UPGRADE_SOON = 1,
  UPGRADE_ASK_ME_LATER = 2,
  UPGRADE_ABORT = 3,
} sli_zigbee_upgrade_policy_t;
#define UPGRADE_POLICY_MAX  UPGRADE_ABORT

sli_zigbee_upgrade_policy_t upgradePolicy = UPGRADE_NOW;
#define UPGRADE_SOON_TIME_SECONDS (2 * 60)

const char * upgradePolicyStrings[] = {
  "Upgrade Now",
  "Upgrade In a few minutes",
  "Ask me later to upgrade",
  "Abort upgrade",
};

// This corresponds to the enumerated sli_zigbee_upgrade_policy_t list.
const uint32_t upgradeTimes[] = {
  0,                            // Now
  UPGRADE_SOON_TIME_SECONDS,    // in a little while
  0xFFFFFFFFL,                  // go ask your father (wait forever)
  0,                            // unused
};

#if defined EM_AF_TEST_HARNESS_CODE
static uint16_t missedBlockModulus = 0;
#endif

#if defined(OTA_SERVER_PAGE_REQUEST_SUPPORT)
  #define PAGE_REQUEST_STATUS_CODE SL_ZIGBEE_ZCL_STATUS_SUCCESS
  #define PAGE_REQUEST_COMPILE_TIME_SUPPORT "yes"
#else
  #define PAGE_REQUEST_STATUS_CODE SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND
  #define PAGE_REQUEST_COMPILE_TIME_SUPPORT "no"
#endif

static uint8_t pageRequestStatus = PAGE_REQUEST_STATUS_CODE;

static uint16_t otaMinimumBlockPeriodMs = 0;

#define SERVER_AND_CLIENT_SUPPORT_MIN_BLOCK_REQUEST                       \
  (SL_ZIGBEE_AF_IMAGE_BLOCK_REQUEST_MIN_BLOCK_REQUEST_SUPPORTED_BY_CLIENT \
   | SL_ZIGBEE_AF_IMAGE_BLOCK_REQUEST_MIN_BLOCK_REQUEST_SUPPORTED_BY_SERVER)

#ifdef SL_ZIGBEE_TEST
uint8_t testClientDelayUnit = OTA_SERVER_DO_NOT_OVERRIDE_CLIENT_DELAY_UNITS;
#endif // SL_ZIGBEE_TEST

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

void sli_zigbee_af_ota_server_policy_print(void)
{
#ifdef DESCRIPTION_FIELD
  otaPrintln("OTA Server Policies");
  otaPrintln("Query Policy: %s (%d)",
             nextVersionPolicyStrings[nextVersionPolicy],
             nextVersionPolicy);
#endif // DESCRIPTION_FIELD

#if defined EM_AF_TEST_HARNESS_CODE
  otaPrintln("Image Block Request Policy: %s (%d)",
             imageBlockRequestPolicyStrings[imageBlockRequestPolicy],
             imageBlockRequestPolicy);
  if (imageBlockRequestPolicy == DELAY_DOWNLOAD_ONCE) {
    otaPrintln("  Delay time: %d seconds",
               IMAGE_BLOCK_REQUEST_DELAY_TIME_SECONDS);
  }
  otaPrintln("Page Request Block Missed Modulus: %d", missedBlockModulus);
#else
  otaPrintln("Image Block Request Policy: Send block");
#endif
  otaPrintln("Image Block Request Min Period: %d milliseconds",
             otaMinimumBlockPeriodMs);
  sl_zigbee_af_core_flush();

  otaPrintln("Page Request Code Compiled in: %s",
             PAGE_REQUEST_COMPILE_TIME_SUPPORT);
  otaPrintln("Page Request Runtime Status Code: 0x%02X",
             pageRequestStatus);
  sl_zigbee_af_core_flush();

  otaPrintln("Upgrade Request Policy: %s (%d)",
             upgradePolicyStrings[upgradePolicy],
             upgradePolicy);
  if (upgradePolicy == UPGRADE_SOON) {
    otaPrintln("  (%d seconds)", UPGRADE_SOON_TIME_SECONDS);
  }
  sl_zigbee_af_core_flush();
}

static bool determineNextSoftwareVersion(uint32_t versionServerHas,
                                         uint32_t versionClientHas)
{
  // Our system here controls whether we tell the client to
  // (A) upgrade, because we have a newer version
  // (B) downgrade, because we have an older version we want to install
  // (C) reinstall, because we have the same version you have currently
  // (D) do nothing (no 'next' image is avaiable)

  switch (nextVersionPolicy) {
    case UPGRADE_IF_SERVER_HAS_NEWER:
      if (versionServerHas > versionClientHas) {
        return true;
      }
      break;
    case DOWNGRADE_IF_SERVER_HAS_OLDER:
      if (versionServerHas < versionClientHas) {
        return true;
      }
      break;
    case REINSTALL_IF_SERVER_HAS_SAME:
      if (versionServerHas == versionClientHas) {
        return true;
      }
      break;
    case NO_NEXT_VERSION:
    default:
      break;
  }
  return false;
}

uint8_t sl_zigbee_af_ota_server_query_cb(const sl_zigbee_af_ota_image_id_t* currentImageId,
                                         uint16_t* hardwareVersion,
                                         sl_zigbee_af_ota_image_id_t* nextUpgradeImageId)
{
  // This function is called by the OTA cluster server to determine what
  // the 'next' version of software is for a particular device requesting
  // a new download image.  The server returns a status code indicating
  // SL_ZIGBEE_ZCL_STATUS_NO_IMAGE_AVAILABLE, or SL_ZIGBEE_ZCL_STATUS_SUCCESS
  // (new image is available).  It then also fills in the 'nextUpgradeImageId'
  // structure with the appropriate version.

  // The server can use whatever criteria it wants to dictate what
  // the 'next' version is and if it is currently available.
  // This sample does this based on a global policy value.

  uint8_t status = SL_ZIGBEE_ZCL_STATUS_NO_IMAGE_AVAILABLE;
  bool hardwareVersionMismatch = false;

  otaPrintln("QueryNextImageRequest mfgId:0x%04X imageTypeId:0x%04X, fw:0x%08X",
             currentImageId->manufacturerId,
             currentImageId->imageTypeId,
             currentImageId->firmwareVersion);
  *nextUpgradeImageId
    = sl_zigbee_af_ota_storage_search_cb(currentImageId->manufacturerId,
                                         currentImageId->imageTypeId,
                                         hardwareVersion);

  if (sl_zigbee_af_is_ota_image_id_valid(nextUpgradeImageId)) {
    // We only perform a check if both the query and the
    // file have hardware version(s).  If one or the other doesn't
    // have them, we assume a match is still possible.
    if (hardwareVersion) {
      sl_zigbee_af_ota_header_t header;
      sl_zigbee_af_ota_storage_get_full_header_cb(nextUpgradeImageId,
                                                  &header);
      if (header.fieldControl & HARDWARE_VERSIONS_PRESENT_MASK) {
        if (*hardwareVersion < header.minimumHardwareVersion
            || header.maximumHardwareVersion < *hardwareVersion) {
          otaPrintln("Hardware version 0x%02X does not fall within the min (0x%02X) and max (0x%02X) hardware versions in the file.",
                     *hardwareVersion,
                     header.minimumHardwareVersion,
                     header.maximumHardwareVersion);
          hardwareVersionMismatch = true;
        }
      }
    }
    // "!hardwareVersionMismatch" does not mean the hardware
    // versions match.  It just means we don't *disqualify* the image
    // as a potential upgrade candidate because the hardware is out
    // of range.
    if (!hardwareVersionMismatch) {
      status = (determineNextSoftwareVersion(nextUpgradeImageId->firmwareVersion,
                                             currentImageId->firmwareVersion)
                ? SL_ZIGBEE_ZCL_STATUS_SUCCESS
                : SL_ZIGBEE_ZCL_STATUS_NO_IMAGE_AVAILABLE);
      if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
        otaPrintln("Next fw version is: 0x%08X",
                   nextUpgradeImageId->firmwareVersion);
      }
    }
  }
  return status;
}

uint8_t sl_zigbee_af_ota_server_block_size_cb(sl_802154_short_addr_t clientNodeId)
{
  // This function provides a way for the server to potentially
  // adjust the block size based on the client who is requesting.
  // In other words if we are using source routing we will limit
  // data returned by enough to put a source route into the message.

  sl_zigbee_aps_frame_t apsFrame;
  uint8_t maxSize;
  apsFrame.options = SL_ZIGBEE_APS_OPTION_NONE;

  if (sl_zigbee_af_is_current_security_profile_smart_energy()) {
    apsFrame.options |= SL_ZIGBEE_APS_OPTION_ENCRYPTION;
  }

  maxSize = sl_zigbee_af_maximum_aps_payload_length(SL_ZIGBEE_OUTGOING_DIRECT,
                                                    clientNodeId,
                                                    &apsFrame);
  maxSize -= IMAGE_BLOCK_RESPONSE_OVERHEAD;
  return maxSize;
}

uint8_t sl_zigbee_af_ota_server_image_block_request_cb(sl_zigbee_af_image_block_request_callback_struct_t* data)
{
  uint8_t status;
  uint16_t serverBlockPeriodValue = otaMinimumBlockPeriodMs;
  uint8_t clientDelayUnit;
  bool useSecondsDelay = false;

  if (SERVER_AND_CLIENT_SUPPORT_MIN_BLOCK_REQUEST
      == (data->bitmask & SERVER_AND_CLIENT_SUPPORT_MIN_BLOCK_REQUEST)) {
    clientDelayUnit = sl_zigbee_af_ota_server_policy_get_client_delay_units(
      data->source,
      data->sourceEui);

#ifdef SL_ZIGBEE_TEST
    if (OTA_SERVER_DO_NOT_OVERRIDE_CLIENT_DELAY_UNITS != testClientDelayUnit) {
      clientDelayUnit = testClientDelayUnit;
    }
#endif // SL_ZIGBEE_TEST

    switch (clientDelayUnit) {
      case OTA_SERVER_NO_RATE_LIMITING_FOR_CLIENT:
        return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
        break;

      case OTA_SERVER_CLIENT_USES_MILLISECONDS:
        break;

      case OTA_SERVER_CLIENT_USES_SECONDS:
        useSecondsDelay = true;
        break;

      case OTA_SERVER_DISCOVER_CLIENT_DELAY_UNITS:
      {
        // If we support dynamic block request, check to see if we know how the
        // client treats the field. If we haven't tested the client yet or don't
        // have a spot for it as an active OTA session, we tell it to
        // WAIT_FOR_DATA
        status = sli_zigbee_af_ota_server_check_dynamic_block_period_download(data);
        if (SL_ZIGBEE_ZCL_STATUS_WAIT_FOR_DATA == status) {
          return SL_ZIGBEE_ZCL_STATUS_WAIT_FOR_DATA;
        }

        useSecondsDelay = sli_zigbee_af_ota_server_dynamic_block_period_client_uses_seconds(data->source);
      }
      break;

      default:
        break;
    }

    if (useSecondsDelay) {
      serverBlockPeriodValue /= 1000;
    }

    if (data->minBlockRequestPeriod != serverBlockPeriodValue) {
      data->minBlockRequestPeriod = serverBlockPeriodValue;
      return SL_ZIGBEE_ZCL_STATUS_WAIT_FOR_DATA;
    }
  }

#if defined EM_AF_TEST_HARNESS_CODE
  // TEST Harness code
  // This will artificially delay once or abort the download as a
  // demonstration.  The test cases use this.

  if (imageBlockRequestPolicy == DELAY_DOWNLOAD_ONCE) {
    data->waitTimeSecondsResponse = IMAGE_BLOCK_REQUEST_DELAY_TIME_SECONDS;
    imageBlockRequestPolicy = SEND_BLOCK;
    return SL_ZIGBEE_ZCL_STATUS_WAIT_FOR_DATA;
  } else if (data->offset > 0 && imageBlockRequestPolicy == ABORT_DOWNLOAD) {
    // Only abort after the first block to insure the client handles
    // this correctly.
    return SL_ZIGBEE_ZCL_STATUS_ABORT;
  }
#endif

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

bool sl_zigbee_af_ota_server_upgrade_end_request_cb(sl_802154_short_addr_t source,
                                                    uint8_t status,
                                                    uint32_t* returnValue,
                                                    const sl_zigbee_af_ota_image_id_t* imageId)
{
  UNUSED_VAR(imageId);
  otaPrintln("Client 0x%04X indicated upgrade status: 0x%02X",
             source,
             status);

  sli_zigbee_af_ota_server_complete_dynamic_block_period_download(source);

  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    // If status != SL_ZIGBEE_ZCL_STATUS_SUCCESS then this callback is
    // only informative.  Return code will be ignored.
    return false;
  }

  otaPrintln("Upgrade End Response: %s", upgradePolicyStrings[upgradePolicy]);

  if (upgradePolicy == UPGRADE_ABORT) {
    return false;
  }

  *returnValue = upgradeTimes[upgradePolicy];
  return true;
}

void sli_zigbee_af_ota_server_set_query_policy(uint8_t value)
{
  if (value <= QUERY_POLICY_MAX) {
    nextVersionPolicy = (sli_zigbee_next_version_policy_t)value;
  }
}

void sli_zigbee_af_ota_server_set_block_request_policy(uint8_t value)
{
#if defined EM_AF_TEST_HARNESS_CODE
  if (value <= BLOCK_REQUEST_POLICY_MAX) {
    imageBlockRequestPolicy = (sli_zigbee_image_block_request_policy_t)value;
  }
#else
  UNUSED_VAR(value);
  otaPrintln("Unsupported.");
#endif
}

uint8_t sl_zigbee_af_ota_page_request_server_policy_cb(void)
{
  return pageRequestStatus;
}

void sli_zigbee_af_ota_server_set_page_request_policy(uint8_t value)
{
  // This allows test code to be compiled with support for page request but
  // tell requesting devices it doesn't support it.
  pageRequestStatus = (value
                       ? SL_ZIGBEE_ZCL_STATUS_SUCCESS
                       : SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND);
}

void sli_zigbee_af_ota_server_set_upgrade_policy(uint8_t value)
{
  if (value <= UPGRADE_POLICY_MAX) {
    upgradePolicy = (sli_zigbee_upgrade_policy_t)value;
  }
}

#if defined EM_AF_TEST_HARNESS_CODE

// There is no reason in production why certain block responses
// for a page request would need a callback, so that's why we wrap
// this whole function in a #define.
bool sli_zigbee_af_server_page_request_tick_callback(uint16_t relativeOffset,
                                                     uint8_t dataSize)
{
  uint16_t block = (relativeOffset + dataSize) / dataSize;
  sl_zigbee_af_core_flush();
  if (missedBlockModulus
      && (block % missedBlockModulus == 0)) {
    sl_zigbee_af_core_flush();
    otaPrintln("Inducing artificial failure for block %d", block);
    return false;
  }
  return true;
}

#endif

void sli_zigbee_af_set_page_request_missed_block_modulus(uint16_t modulus)
{
#if defined(EM_AF_TEST_HARNESS_CODE)
  missedBlockModulus = modulus;
#else
  UNUSED_VAR(modulus);
  otaPrintln("Unsupported.");
#endif
}

void sli_zigbee_af_ota_server_policy_set_min_block_request_period(uint16_t minBlockPeriodMs)
{
  otaMinimumBlockPeriodMs = minBlockPeriodMs;
}
