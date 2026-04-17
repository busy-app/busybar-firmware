/***************************************************************************//**
 * @file
 * @brief This file contains the code to manipulate the Smart Energy attribute
 * table.  This handles external calls to read/write the table, as
 * well as internal ones.
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

// this file contains all the common includes for clusters in the zcl-util
#include "app/framework/util/common.h"
#include "app/framework/util/attribute-storage.h"
#if !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
#define DEFINETYPES
#endif
#include "stack/config/sl_zigbee_token_defines.h"

// for pulling in defines dealing with EITHER server or client
#include "af-main.h"

#include "zap-type.h"
#if defined(EZSP_HOST) && defined(SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT)
#include "af-support-host.h"
#endif //defined(EZSP_HOST) && defined(SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT)

#if defined(SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_DEFER_ATTRIBUTE_WRITES_TO_NVM_MS) && (SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_DEFER_ATTRIBUTE_WRITES_TO_NVM_MS > 0) && defined(NUM_PERSISTED_ZCL_ATTRIBUTES)
 #define DEFER_ATTRIBUTE_UPDATES_IN_NVM
#endif // defined(SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_DEFER_ATTRIBUTE_WRITES_TO_NVM_MS) && (SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_DEFER_ATTRIBUTE_WRITES_TO_NVM_MS > 0) && defined(NUM_PERSISTED_ZCL_ATTRIBUTES)

//------------------------------------------------------------------------------
// Static Declarations
static sl_zigbee_af_status_t sli_zigbee_af_process_write_attribute_data(sl_zigbee_af_attribute_metadata_t *metadata,
                                                                        uint8_t* data,
                                                                        sl_zigbee_af_attribute_search_record_t *record,
                                                                        sl_zigbee_af_attribute_type_t dataType,
                                                                        bool updateNvm,
                                                                        bool syncMultiProtocol);

//------------------------------------------------------------------------------
// External Declarations

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------
// Globals

sl_zigbee_af_status_t sl_zigbee_af_write_attribute_external(uint8_t endpoint,
                                                            sl_zigbee_af_cluster_id_t cluster,
                                                            sl_zigbee_af_attribute_id_t attributeID,
                                                            uint8_t mask,
                                                            uint16_t manufacturerCode,
                                                            uint8_t* dataPtr,
                                                            sl_zigbee_af_attribute_type_t dataType)
{
  sl_zigbee_af_attribute_write_permission_t extWritePermission
    = sl_zigbee_af_allow_network_write_attribute_cb(endpoint,
                                                    cluster,
                                                    attributeID,
                                                    mask,
                                                    manufacturerCode,
                                                    dataPtr,
                                                    dataType);
  switch (extWritePermission) {
    case SL_ZIGBEE_ZCL_ATTRIBUTE_WRITE_PERMISSION_DENY_WRITE:
      return SL_ZIGBEE_ZCL_STATUS_FAILURE;
    case SL_ZIGBEE_ZCL_ATTRIBUTE_WRITE_PERMISSION_ALLOW_WRITE_NORMAL:
    case SL_ZIGBEE_ZCL_ATTRIBUTE_WRITE_PERMISSION_ALLOW_WRITE_OF_READ_ONLY:
      return sli_zigbee_af_write_attribute(endpoint,
                                           cluster,
                                           attributeID,
                                           mask,
                                           manufacturerCode,
                                           dataPtr,
                                           dataType,
                                           (extWritePermission
                                            == SL_ZIGBEE_ZCL_ATTRIBUTE_WRITE_PERMISSION_ALLOW_WRITE_OF_READ_ONLY),
                                           false,
                                           true); // update NVM
    default:
      return (sl_zigbee_af_status_t)extWritePermission;
  }
}

//@deprecated use sl_zigbee_af_write_server_attribute or sl_zigbee_af_write_client_attribute
sl_zigbee_af_status_t sl_zigbee_af_write_attribute(uint8_t endpoint,
                                                   sl_zigbee_af_cluster_id_t cluster,
                                                   sl_zigbee_af_attribute_id_t attributeID,
                                                   uint8_t mask,
                                                   uint8_t* dataPtr,
                                                   sl_zigbee_af_attribute_type_t dataType)
{
  return sli_zigbee_af_write_attribute(endpoint,
                                       cluster,
                                       attributeID,
                                       mask,
                                       SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE,
                                       dataPtr,
                                       dataType,
                                       true, // override read-only?
                                       false, // just test?
                                       true); // update NVM
}

sl_zigbee_af_status_t sl_zigbee_af_write_client_attribute(uint8_t endpoint,
                                                          sl_zigbee_af_cluster_id_t cluster,
                                                          sl_zigbee_af_attribute_id_t attributeID,
                                                          uint8_t* dataPtr,
                                                          sl_zigbee_af_attribute_type_t dataType)
{
  return sli_zigbee_af_write_attribute(endpoint,
                                       cluster,
                                       attributeID,
                                       CLUSTER_MASK_CLIENT,
                                       SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE,
                                       dataPtr,
                                       dataType,
                                       true, // override read-only?
                                       false, // just test?
                                       true); // update NVM
}

sl_zigbee_af_status_t sl_zigbee_af_write_server_attribute(uint8_t endpoint,
                                                          sl_zigbee_af_cluster_id_t cluster,
                                                          sl_zigbee_af_attribute_id_t attributeID,
                                                          uint8_t* dataPtr,
                                                          sl_zigbee_af_attribute_type_t dataType)
{
  return sli_zigbee_af_write_attribute(endpoint,
                                       cluster,
                                       attributeID,
                                       CLUSTER_MASK_SERVER,
                                       SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE,
                                       dataPtr,
                                       dataType,
                                       true, // override read-only?
                                       false, // just test?
                                       true); // update NVM
}

sl_zigbee_af_status_t sl_zigbee_af_write_server_attribute_without_sync(uint8_t endpoint,
                                                                       sl_zigbee_af_cluster_id_t cluster,
                                                                       sl_zigbee_af_attribute_id_t attributeID,
                                                                       uint8_t* dataPtr,
                                                                       sl_zigbee_af_attribute_type_t dataType)
{
  return sli_zigbee_af_write_attribute_with_sync(endpoint,
                                                 cluster,
                                                 attributeID,
                                                 CLUSTER_MASK_SERVER,
                                                 SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE,
                                                 dataPtr,
                                                 dataType,
                                                 true, // override read-only?
                                                 false, // just test?
                                                 true, // update NVM
                                                 false); // Don't sync with multiprotocol datamodel
}

sl_zigbee_af_status_t sl_zigbee_af_write_manufacturer_specific_client_attribute(uint8_t endpoint,
                                                                                sl_zigbee_af_cluster_id_t cluster,
                                                                                sl_zigbee_af_attribute_id_t attributeID,
                                                                                uint16_t manufacturerCode,
                                                                                uint8_t* dataPtr,
                                                                                sl_zigbee_af_attribute_type_t dataType)
{
  return sli_zigbee_af_write_attribute(endpoint,
                                       cluster,
                                       attributeID,
                                       CLUSTER_MASK_CLIENT,
                                       manufacturerCode,
                                       dataPtr,
                                       dataType,
                                       true, // override read-only?
                                       false, // just test?
                                       true); // update NVM
}

sl_zigbee_af_status_t sl_zigbee_af_write_manufacturer_specific_server_attribute(uint8_t endpoint,
                                                                                sl_zigbee_af_cluster_id_t cluster,
                                                                                sl_zigbee_af_attribute_id_t attributeID,
                                                                                uint16_t manufacturerCode,
                                                                                uint8_t* dataPtr,
                                                                                sl_zigbee_af_attribute_type_t dataType)
{
  return sli_zigbee_af_write_attribute(endpoint,
                                       cluster,
                                       attributeID,
                                       CLUSTER_MASK_SERVER,
                                       manufacturerCode,
                                       dataPtr,
                                       dataType,
                                       true, // override read-only?
                                       false, // just test?
                                       true); // update NVM
}

sl_zigbee_af_status_t sl_zigbee_af_verify_attribute_write(uint8_t endpoint,
                                                          sl_zigbee_af_cluster_id_t cluster,
                                                          sl_zigbee_af_attribute_id_t attributeID,
                                                          uint8_t mask,
                                                          uint16_t manufacturerCode,
                                                          uint8_t* dataPtr,
                                                          sl_zigbee_af_attribute_type_t dataType)
{
  return sli_zigbee_af_write_attribute(endpoint,
                                       cluster,
                                       attributeID,
                                       mask,
                                       manufacturerCode,
                                       dataPtr,
                                       dataType,
                                       false, // override read-only?
                                       true, // just test?
                                       true); // update NVM, doesn't matter
}

sl_zigbee_af_status_t sl_zigbee_af_read_attribute(uint8_t endpoint,
                                                  sl_zigbee_af_cluster_id_t cluster,
                                                  sl_zigbee_af_attribute_id_t attributeID,
                                                  uint8_t mask,
                                                  uint8_t *dataPtr,
                                                  uint8_t readLength,
                                                  sl_zigbee_af_attribute_type_t *dataType)
{
  return sli_zigbee_af_read_attribute(endpoint,
                                      cluster,
                                      attributeID,
                                      mask,
                                      SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE,
                                      dataPtr,
                                      readLength,
                                      dataType);
}

sl_zigbee_af_status_t sl_zigbee_af_read_server_attribute(uint8_t endpoint,
                                                         sl_zigbee_af_cluster_id_t cluster,
                                                         sl_zigbee_af_attribute_id_t attributeID,
                                                         uint8_t* dataPtr,
                                                         uint8_t readLength)
{
  return sli_zigbee_af_read_attribute(endpoint,
                                      cluster,
                                      attributeID,
                                      CLUSTER_MASK_SERVER,
                                      SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE,
                                      dataPtr,
                                      readLength,
                                      NULL);
}

sl_zigbee_af_status_t sl_zigbee_af_read_client_attribute(uint8_t endpoint,
                                                         sl_zigbee_af_cluster_id_t cluster,
                                                         sl_zigbee_af_attribute_id_t attributeID,
                                                         uint8_t* dataPtr,
                                                         uint8_t readLength)
{
  return sli_zigbee_af_read_attribute(endpoint,
                                      cluster,
                                      attributeID,
                                      CLUSTER_MASK_CLIENT,
                                      SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE,
                                      dataPtr,
                                      readLength,
                                      NULL);
}

sl_zigbee_af_status_t sl_zigbee_af_read_manufacturer_specific_server_attribute(uint8_t endpoint,
                                                                               sl_zigbee_af_cluster_id_t cluster,
                                                                               sl_zigbee_af_attribute_id_t attributeID,
                                                                               uint16_t manufacturerCode,
                                                                               uint8_t* dataPtr,
                                                                               uint8_t readLength)
{
  return sli_zigbee_af_read_attribute(endpoint,
                                      cluster,
                                      attributeID,
                                      CLUSTER_MASK_SERVER,
                                      manufacturerCode,
                                      dataPtr,
                                      readLength,
                                      NULL);
}

sl_zigbee_af_status_t sl_zigbee_af_read_manufacturer_specific_client_attribute(uint8_t endpoint,
                                                                               sl_zigbee_af_cluster_id_t cluster,
                                                                               sl_zigbee_af_attribute_id_t attributeID,
                                                                               uint16_t manufacturerCode,
                                                                               uint8_t* dataPtr,
                                                                               uint8_t readLength)
{
  return sli_zigbee_af_read_attribute(endpoint,
                                      cluster,
                                      attributeID,
                                      CLUSTER_MASK_CLIENT,
                                      manufacturerCode,
                                      dataPtr,
                                      readLength,
                                      NULL);
}

// Resolve the manufacturing code for an attribute when the Discover Attribute
// request specifies the wildcard 0xFFFF for the mfg code. Iterate through
// attributes and select the mfg-code of the mfg-specific attr having the
// lowest attrId equal to or greater than startAttributeId. If two or more
// such mfg-specific attrs have the same attrId, select the lower numbered
// mfg-code. If no valid mfg code is resolved, return 0xFFFF.
uint16_t sli_zigbee_af_resolve_mfg_code_for_discover_attribute(uint8_t endpoint,
                                                               sl_zigbee_af_cluster_id_t clusterId,
                                                               sl_zigbee_af_attribute_id_t startAttributeId,
                                                               uint8_t clientServerMask)
{
  uint16_t attrMfgCode = 0xFFFFu;
  uint16_t candidateMfgCode;
  sl_zigbee_af_cluster_t *cluster;
  sl_zigbee_af_attribute_metadata_t *metadata;
  bool foundFirst = false;
  uint16_t foundAttrId = 0u;
  uint16_t i;
  bool isMfgSpecCluster = (clusterId >= 0xFC00u);

  extern const sl_zigbee_af_manufacturer_code_entry_t clusterManufacturerCodes[];
  extern const uint16_t clusterManufacturerCodeCount;

  // If mfg-specific cluster, iterate clusterManufacturerCodeCount times.
  // If std cluster, iterate once.
  uint16_t loopbound = (isMfgSpecCluster ? clusterManufacturerCodeCount : 1u);
  for (i = 0; i < loopbound; i++) {
    if (isMfgSpecCluster) {
      // mfg-specific cluster, all attributes are mfg-specific.
      const sl_zigbee_af_manufacturer_code_entry_t *mce = &clusterManufacturerCodes[i];
      cluster = sl_zigbee_af_find_cluster_with_mfg_code(endpoint,
                                                        clusterId,
                                                        clientServerMask,
                                                        mce->manufacturerCode);
    } else {
      // standard cluster, some attributes might be mfg-specific.
      cluster = sl_zigbee_af_find_cluster(endpoint,
                                          clusterId,
                                          clientServerMask);
    }

    if (cluster != NULL) {
      for (i = 0; i < cluster->attributeCount; i++) {
        metadata = &cluster->attributes[i];

        // Only start from the passed attribute id
        if (metadata->attributeId < startAttributeId) {
          continue;
        }

        // After having previously found a first mfg-spec candidate,
        // ignore an attr that has a higher attrId.
        if (foundFirst && metadata->attributeId > foundAttrId) {
          continue;
        }

        // Get attribute's mfg-code. Update the search state if:
        // this is the first qualifying attr found;
        // else, this qualifying attr has a lower attrId than prior attr found;
        // else, this attrId equals prior, prefer this attr's lower mfg-code.
        candidateMfgCode = sli_zigbee_af_get_manufacturer_code_for_attribute(cluster, metadata);
        if (candidateMfgCode != SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE
            && (!foundFirst
                || metadata->attributeId < foundAttrId
                || candidateMfgCode < attrMfgCode)) {
          foundFirst = true;
          foundAttrId = metadata->attributeId;
          attrMfgCode = candidateMfgCode;
        }
      }
    }
  }

  return attrMfgCode;
}

bool sl_zigbee_af_read_sequential_attributes_add_to_response(uint8_t endpoint,
                                                             sl_zigbee_af_cluster_id_t clusterId,
                                                             sl_zigbee_af_attribute_id_t startAttributeId,
                                                             uint8_t mask,
                                                             uint16_t manufacturerCode,
                                                             uint8_t maxAttributeIds,
                                                             bool includeAccessControl)
{
  uint16_t i;
  uint16_t discovered = 0;
  uint16_t skipped = 0;
  uint16_t total = 0;

  sl_zigbee_af_cluster_t *cluster = sl_zigbee_af_find_cluster_with_mfg_code(endpoint,
                                                                            clusterId,
                                                                            mask,
                                                                            manufacturerCode);

  sl_zigbee_af_attribute_search_record_t record;
  record.endpoint = endpoint;
  record.clusterId = clusterId;
  record.clusterMask = mask;
  record.attributeId = startAttributeId;
  record.manufacturerCode = manufacturerCode;

  // If we don't have the cluster or it doesn't match the search, we're done.
  if (cluster == NULL || !sli_zigbee_af_match_cluster(cluster, &record)) {
    return true;
  }

  for (i = 0; i < cluster->attributeCount; i++) {
    sl_zigbee_af_attribute_metadata_t *metadata = &cluster->attributes[i];

    // If the cluster is not manufacturer-specific, an attribute is considered
    // only if its manufacturer code matches that of the command (which may be
    // unset).
    if (!sl_zigbee_af_cluster_is_manufacturer_specific(cluster)) {
      record.attributeId = metadata->attributeId;
      if (!sli_zigbee_af_match_attribute(cluster, metadata, &record)) {
        continue;
      }
    }

    if (metadata->attributeId < startAttributeId) {
      skipped++;
    } else if (discovered < maxAttributeIds) {
      (void) sl_zigbee_af_put_int16u_in_resp(metadata->attributeId);
      (void) sl_zigbee_af_put_int8u_in_resp(metadata->attributeType);
      if (includeAccessControl) {
        // bit 0 : Readable <-- All our attributes are readable
        // bit 1 : Writable <-- The only thing we track in the attribute metadata mask
        // bit 2 : Reportable <-- All our attributes are reportable
        (void) sl_zigbee_af_put_int8u_in_resp((metadata->mask & ATTRIBUTE_MASK_WRITABLE)
                                              ? 0x07
                                              : 0x05);
      }
      discovered++;
    } else {
      // MISRA requires ..else if.. to have terminating else.
    }
    total++;
  }

  // We are finished if there are no more attributes to find, which means the
  // number of attributes discovered plus the number skipped equals the total
  // attributes in the cluster.  For manufacturer-specific clusters, the total
  // includes all attributes in the cluster.  For standard ZCL clusters, if the
  // the manufacturer code is set, the total is the number of attributes that
  // match the manufacturer code.  Otherwise, the total is the number of
  // standard ZCL attributes in the cluster.
  return (discovered + skipped == total);
}

static void sl_zigbee_af_attribute_decode_and_print_cluster(sl_zigbee_af_cluster_id_t cluster, uint16_t mfgCode)
{
  uint16_t index = sl_zigbee_af_find_cluster_name_index_with_mfg_code(cluster, mfgCode);
  if (index != 0xFFFFu) {
    sl_zigbee_af_attributes_println("(%s)", sl_zigbee_zcl_cluster_names[index].name);
  }
  sl_zigbee_af_attributes_println("");
}

void sl_zigbee_af_print_attribute_table(void)
{
  uint8_t data[ZCL_ATTRIBUTE_MAX_DATA_SIZE];
  uint8_t endpointIndex, clusterIndex;
  uint16_t attributeIndex;
  sl_zigbee_af_status_t status;
  uint16_t mfgCode;
  for (endpointIndex = 0;
       endpointIndex < sl_zigbee_af_endpoint_count();
       endpointIndex++) {
    sl_zigbee_af_defined_endpoint_t *ep = &(sli_zigbee_af_endpoints[endpointIndex]);
    sl_zigbee_af_attributes_println("ENDPOINT %02X", ep->endpoint);
    sl_zigbee_af_attributes_println("clus / side / attr / mfg  /type(len)/ rw / storage / data (raw)");
    sl_zigbee_af_attributes_flush();
    for (clusterIndex = 0;
         clusterIndex < ep->endpointType->clusterCount;
         clusterIndex++) {
      sl_zigbee_af_cluster_t *cluster = &(ep->endpointType->cluster[clusterIndex]);

      for (attributeIndex = 0;
           attributeIndex < cluster->attributeCount;
           attributeIndex++) {
        sl_zigbee_af_attribute_metadata_t *metaData = &(cluster->attributes[attributeIndex]);

        // Depending on user config, this loop can take a very long time to
        // run and watchdog reset will  kick in. As a workaround, we'll
        // manually reset the watchdog.
        halResetWatchdog();

        sl_zigbee_af_attributes_print("%04X / %s / %04X / ",
                                      cluster->clusterId,
                                      (sl_zigbee_af_attribute_is_client(metaData) ? "clnt" : "srvr"),
                                      metaData->attributeId);
        mfgCode = sli_zigbee_af_get_manufacturer_code_for_attribute(cluster, metaData);
        if (mfgCode == SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE) {
          sl_zigbee_af_attributes_print("----");
        } else {
          sl_zigbee_af_attributes_print("%04X", mfgCode);
        }
        sl_zigbee_af_attributes_print(" / %02X (%02X) / %s / %s / ",
                                      metaData->attributeType,
                                      sl_zigbee_af_attribute_size(metaData),
                                      (sl_zigbee_af_attribute_is_read_only(metaData) ? "RO" : "RW"),
                                      (sl_zigbee_af_attribute_is_tokenized(metaData)
                                       ? " token "
                                       : (sl_zigbee_af_attribute_is_external(metaData)
                                          ? "extern "
                                          : "  RAM  ")));
        sl_zigbee_af_attributes_flush();
        status = sli_zigbee_af_read_attribute(ep->endpoint,
                                              cluster->clusterId,
                                              metaData->attributeId,
                                              (sl_zigbee_af_attribute_is_client(metaData)
                                               ? CLUSTER_MASK_CLIENT
                                               : CLUSTER_MASK_SERVER),
                                              mfgCode,
                                              data,
                                              ZCL_ATTRIBUTE_MAX_DATA_SIZE,
                                              NULL);
        if (status == SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE) {
          sl_zigbee_af_attributes_println("Unsupported");
        } else {
          uint16_t length;
          if (sl_zigbee_af_is_string_attribute_type(metaData->attributeType)) {
            length = (uint16_t) sl_zigbee_af_string_length(data) + 1u;
          } else if (sl_zigbee_af_is_long_string_attribute_type(metaData->attributeType)) {
            length = sl_zigbee_af_long_string_length(data) + 2u;
          } else {
            length = sl_zigbee_af_attribute_size(metaData);
          }
          UNUSED_VAR(length);
          sl_zigbee_af_attributes_print_buffer(data, length, true);
          sl_zigbee_af_attributes_flush();
          sl_zigbee_af_attribute_decode_and_print_cluster(cluster->clusterId, mfgCode);
        }
      }
    }
    sl_zigbee_af_attributes_flush();
  }
}

// given a clusterId and an attribute to read, this crafts the response
// and places it in the response buffer. Response is one of two items:
// 1) unsupported: [attrId:2] [status:1]
// 2) supported:   [attrId:2] [status:1] [type:1] [data:n]
//
sl_zigbee_af_status_t sl_zigbee_af_retrieve_attribute_and_craft_response(uint8_t endpoint,
                                                                         sl_zigbee_af_cluster_id_t clusterId,
                                                                         sl_zigbee_af_attribute_id_t attrId,
                                                                         uint8_t mask,
                                                                         uint16_t manufacturerCode,
                                                                         uint16_t readLength)
{
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  uint8_t data[ZCL_ATTRIBUTE_MAX_DATA_SIZE];
  uint8_t dataType;
  uint16_t dataLen = 0;

  // account for at least one byte of data
  if (readLength < 5u) {
    return SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
  }

  sl_zigbee_af_attributes_println("OTA READ: ep:%02X cid:%04X attid:%04X msk:%02X mfcode:%04X",
                                  endpoint,
                                  clusterId, attrId, mask, manufacturerCode);

  // lookup the attribute in our table
  status = sli_zigbee_af_read_attribute(endpoint,
                                        clusterId,
                                        attrId,
                                        mask,
                                        manufacturerCode,
                                        data,
                                        ZCL_ATTRIBUTE_MAX_DATA_SIZE,
                                        &dataType);
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    dataLen = sl_zigbee_af_attribute_value_size(dataType, data, (uint16_t) sizeof(data));
    if (dataLen == 0u || (readLength - 4u) < dataLen) {
      // Size retrieval failed or not enough space for attribute.
      status = SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
    }
  } else {
    sl_zigbee_af_attributes_println("READ: clus %04X, attr %04X failed %02X",
                                    clusterId,
                                    attrId,
                                    status);
    sl_zigbee_af_attributes_flush();
  }

  // put attribute in least sig byte first
  (void) sl_zigbee_af_put_int16u_in_resp(attrId);

  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    (void) sl_zigbee_af_put_int8u_in_resp(status);
    return status;
  }

  if ((dataLen + 2 /* status + dataType size */) < (SL_ZIGBEE_AF_RESPONSE_BUFFER_LEN - appResponseLength)) {
    // attribute is found, so copy in the status and the data type
    (void) sl_zigbee_af_put_int8u_in_resp(status);
    (void) sl_zigbee_af_put_int8u_in_resp(dataType);
#if (BIGENDIAN_CPU)
    // strings go over the air as length byte and then in human
    // readable format. These should not be flipped. Other attributes
    // need to be flipped so they go little endian OTA
    if (isThisDataTypeSentLittleEndianOTA(dataType)) {
      uint8_t i;
      for (i = 0; i < dataLen; i++) {
        appResponseData[appResponseLength + i] = data[dataLen - i - 1];
      }
    } else {
      memmove(&(appResponseData[appResponseLength]), data, dataLen);
    }
#else //(BIGENDIAN_CPU)
    memmove(&(appResponseData[appResponseLength]), data, dataLen);
#endif //(BIGENDIAN_CPU)
    appResponseLength += dataLen;
  } else {
    (void) sl_zigbee_af_put_int8u_in_resp(SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE);
    return SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
  }

  sl_zigbee_af_attributes_println("READ: clus %04X, attr %04X, dataLen: %02X, OK",
                                  clusterId,
                                  attrId,
                                  dataLen);
  sl_zigbee_af_attributes_flush();
  return status;
}

// This function appends the attribute report fields for the given endpoint,
// cluster, and attribute to the buffer starting at the index.  If there is
// insufficient space in the buffer or an error occurs, buffer and bufIndex will
// remain unchanged.  Otherwise, bufIndex will be incremented appropriately and
// the fields will be written to the buffer.
sl_zigbee_af_status_t sl_zigbee_af_append_attribute_report_fields(uint8_t endpoint,
                                                                  sl_zigbee_af_cluster_id_t clusterId,
                                                                  sl_zigbee_af_attribute_id_t attributeId,
                                                                  uint8_t mask,
                                                                  uint8_t *buffer,
                                                                  uint8_t bufLen,
                                                                  uint8_t *bufIndex)
{
  sl_zigbee_af_status_t status;
  sl_zigbee_af_attribute_type_t type;
  uint16_t size;
  uint16_t bufLen16 = (uint16_t) bufLen;
  uint8_t data[ZCL_ATTRIBUTE_MAX_DATA_SIZE];

  status = sl_zigbee_af_read_attribute(endpoint,
                                       clusterId,
                                       attributeId,
                                       mask,
                                       data,
                                       (uint8_t) sizeof(data),
                                       &type);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    goto kickout;
  }

  size = sl_zigbee_af_attribute_value_size(type, data, (uint16_t) sizeof(data));
  if ((uint16_t)(bufLen16 - *bufIndex) < 3u || size == 0u || size > bufLen16 - (uint16_t)(*bufIndex + 3)) {
    status = SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
    goto kickout;
  }

  buffer[(*bufIndex)++] = LOW_BYTE(attributeId);
  buffer[(*bufIndex)++] = HIGH_BYTE(attributeId);
  buffer[(*bufIndex)++] = type;
#if (BIGENDIAN_CPU)
  if (isThisDataTypeSentLittleEndianOTA(type)) {
    sl_util_reverse_mem_copy(buffer + *bufIndex, data, size);
  } else {
    memmove(buffer + *bufIndex, data, size);
  }
#else
  memmove(buffer + *bufIndex, data, size);
#endif
  *bufIndex += size;

  kickout:
  sl_zigbee_af_attributes_println("REPORT: clus 0x%04X, attr 0x%04X: 0x%02X",
                                  clusterId,
                                  attributeId,
                                  status);
  sl_zigbee_af_attributes_flush();

  return status;
}

//------------------------------------------------------------------------------
// Internal Functions

sl_zigbee_af_status_t sli_zigbee_af_write_attribute(uint8_t endpoint,
                                                    sl_zigbee_af_cluster_id_t cluster,
                                                    sl_zigbee_af_attribute_id_t attributeID,
                                                    uint8_t mask,
                                                    uint16_t manufacturerCode,
                                                    uint8_t *data,
                                                    sl_zigbee_af_attribute_type_t dataType,
                                                    bool overrideReadOnlyAndDataType,
                                                    bool justTest,
                                                    bool updateNvm)
{
  return sli_zigbee_af_write_attribute_with_sync(endpoint,
                                                 cluster,
                                                 attributeID,
                                                 mask,
                                                 manufacturerCode,
                                                 data,
                                                 dataType,
                                                 overrideReadOnlyAndDataType,
                                                 justTest,
                                                 updateNvm,
                                                 true); // syncMultiProtocol
}

// writes an attribute (identified by clusterID and attrID to the given value.
// this returns:
// - SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE: if attribute isnt supported by the device (the
//           device is not found in the attribute table)
// - SL_ZIGBEE_ZCL_STATUS_INVALID_DATA_TYPE: if the data type passed in doesnt match the type
//           stored in the attribute table
// - SL_ZIGBEE_ZCL_STATUS_READ_ONLY: if the attribute isnt writable
// - SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE: if the value is set out of the allowable range for
//           the attribute
// - SL_ZIGBEE_ZCL_STATUS_SUCCESS: if the attribute was found and successfully written
//
// if true is passed in for overrideReadOnlyAndDataType then the data type is
// not checked and the read-only flag is ignored. This mode is meant for
// testing or setting the initial value of the attribute on the device.
//
// if true is passed for justTest, then the type is not written but all
// checks are done to see if the type could be written
//
// if true is passed for updateNvm, then NVM is updated if the attribute is
// stored in NVM (in addition to RAM). Startup code is typically the case
// for not writing to NVM, since values in NVM are used to update the RAM
// contents.
//
// if true is passed for syncMultiProtocol, then the written attribute will be evaluated by the
// multiprotocol attribute map. If it maps to the counterpart protocol datamodel,
// the stack will call the multiprocol attribute synchronization api.
//
// reads the attribute specified, returns false if the attribute is not in
// the table or the data is too large, returns true and writes to dataPtr
// if the attribute is supported and the readLength specified is less than
// the length of the data.
sl_zigbee_af_status_t sli_zigbee_af_write_attribute_with_sync(uint8_t endpoint,
                                                              sl_zigbee_af_cluster_id_t cluster,
                                                              sl_zigbee_af_attribute_id_t attributeID,
                                                              uint8_t mask,
                                                              uint16_t manufacturerCode,
                                                              uint8_t *data,
                                                              sl_zigbee_af_attribute_type_t dataType,
                                                              bool overrideReadOnlyAndDataType,
                                                              bool justTest,
                                                              bool updateNvm,
                                                              bool syncMultiProtocol)
{
  sl_zigbee_af_attribute_metadata_t *metadata = NULL;
  sl_zigbee_af_attribute_search_record_t record;
  record.endpoint = endpoint;
  record.clusterId = cluster;
  record.clusterMask = mask;
  record.attributeId = attributeID;
  record.manufacturerCode = manufacturerCode;

  sl_zigbee_af_status_t status = sli_zigbee_af_read_attribute_from_storage(&record,
                                                                           &metadata,
                                                                           NULL, // buffer
                                                                           0); // buffer size
  (void)status;

  // if we dont support that attribute
  if (metadata == NULL) {
#if defined(EZSP_HOST) && defined(SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT)
    // If no attribute was found then try searching on NCP.
    if (status == SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE) {
      return sli_zigbee_af_support_write_attribute(&record, dataType, data, overrideReadOnlyAndDataType, justTest);
    } else
#endif
    {
      sl_zigbee_af_attributes_println("%sep %02X clus %04X attr %04X not supported",
                                      "WRITE ERR: ",
                                      endpoint,
                                      cluster,
                                      attributeID);
      sl_zigbee_af_attributes_flush();
      return SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
    }
  }

  // if the data type specified by the caller is incorrect
  if (!(overrideReadOnlyAndDataType)) {
    if (dataType != metadata->attributeType) {
      sl_zigbee_af_attributes_println("%sinvalid data type", "WRITE ERR: ");
      sl_zigbee_af_attributes_flush();
      return SL_ZIGBEE_ZCL_STATUS_INVALID_DATA_TYPE;
    }

    if (sl_zigbee_af_attribute_is_read_only(metadata)) {
      sl_zigbee_af_attributes_println("%sattr not writable", "WRITE ERR: ");
      sl_zigbee_af_attributes_flush();
      return SL_ZIGBEE_ZCL_STATUS_READ_ONLY;
    }
  }

  // if the value the attribute is being set to is out of range
  // return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE
  if ((metadata->mask & ATTRIBUTE_MASK_MIN_MAX) != 0U) {
    sl_zigbee_af_default_attribute_value_t minv = metadata->defaultValue.ptrToMinMaxValue->minValue;
    sl_zigbee_af_default_attribute_value_t maxv = metadata->defaultValue.ptrToMinMaxValue->maxValue;
    bool isAttributeSigned = sl_zigbee_af_is_type_signed(metadata->attributeType);
    uint8_t dataLen = sl_zigbee_af_attribute_size(metadata);
    if (dataLen <= 2u) {
      int8_t minR, maxR;
      uint8_t* minI = (uint8_t*)&(minv.defaultValue);
      uint8_t* maxI = (uint8_t*)&(maxv.defaultValue);
      //On big endian cpu with length 1 only the second byte counts
      #if (BIGENDIAN_CPU)
      if (dataLen == 1) {
        minI++;
        maxI++;
      }
      #endif //BIGENDIAN_CPU
      minR = sl_zigbee_af_compare_values(minI, data, dataLen, isAttributeSigned);
      maxR = sl_zigbee_af_compare_values(maxI, data, dataLen, isAttributeSigned);
      if ((minR == 1) || (maxR == -1)) {
        return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
      }
    } else {
      if ((sl_zigbee_af_compare_values(minv.ptrToDefaultValue, data, dataLen, isAttributeSigned) == 1)
          || (sl_zigbee_af_compare_values(maxv.ptrToDefaultValue, data, dataLen, isAttributeSigned) == -1)) {
        return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
      }
    }
  }

  // write the data unless this is only a test
  if (!justTest) {
    return sli_zigbee_af_process_write_attribute_data(metadata, data, &record, dataType, updateNvm, syncMultiProtocol);
  } else {
    // bug: 11618, we are not handling properly external attributes
    // in this case... We need to do something. We don't really
    // know if it will succeed.
    sl_zigbee_af_attributes_println("WRITE: no write, just a test");
    sl_zigbee_af_attributes_flush();
  }

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

static sl_zigbee_af_status_t sli_zigbee_af_process_write_attribute_data(sl_zigbee_af_attribute_metadata_t *metadata,
                                                                        uint8_t* data,
                                                                        sl_zigbee_af_attribute_search_record_t *record,
                                                                        sl_zigbee_af_attribute_type_t dataType,
                                                                        bool updateNvm,
                                                                        bool syncMultiProtocol)
{
  // Do not know the size of data buffer so use the max 0xFFFF as a bound.
  uint16_t dataSize = sl_zigbee_af_attribute_value_size(metadata->attributeType,
                                                        data,
                                                        0xFFFF);
  if (dataSize == 0u) {
    return SL_ZIGBEE_ZCL_STATUS_FAILURE;
  }
  // Pre write attribute callback for all attribute changes,
  // regardless of cluster.
  sl_zigbee_af_status_t status
    = sl_zigbee_af_pre_attribute_change_cb(record->endpoint,
                                           record->clusterId,
                                           record->attributeId,
                                           record->clusterMask,
                                           record->manufacturerCode,
                                           dataType,
                                           dataSize,
                                           data);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }

  // Pre-write attribute callback specific
  // to the cluster that the attribute lives in.
  status = sli_zigbee_af_cluster_pre_attribute_changed_callback(record->endpoint,
                                                                record->clusterId,
                                                                record->attributeId,
                                                                record->clusterMask,
                                                                record->manufacturerCode,
                                                                dataType,
                                                                dataSize,
                                                                data);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }

  // write the attribute
  status = sli_zigbee_af_write_attribute_to_storage(record,
                                                    data,
                                                    syncMultiProtocol);

  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }

  // Startup code that restores attributes from NVM will call this function with updateNvm=false
  if (updateNvm) {
    // Save the attribute to token if needed
    // Function itself will weed out tokens that are not tokenized.
#ifdef DEFER_ATTRIBUTE_UPDATES_IN_NVM
    defer_attribute_write_to_token(data, record->endpoint, record->clusterId, metadata, record->clusterMask, record->manufacturerCode);
#else // DEFER_ATTRIBUTE_UPDATES_IN_NVM
    sli_zigbee_af_save_attribute_to_token(data, record->endpoint, record->clusterId, metadata);
#endif // DEFER_ATTRIBUTE_UPDATES_IN_NVM
  }

  sl_zigbee_af_reporting_attribute_change_cb(record->endpoint,
                                             record->clusterId,
                                             record->attributeId,
                                             record->clusterMask,
                                             record->manufacturerCode,
                                             dataType,
                                             data);

  // Post write attribute callback for all attributes changes, regardless
  // of cluster.
  sl_zigbee_af_post_attribute_change_cb(record->endpoint,
                                        record->clusterId,
                                        record->attributeId,
                                        record->clusterMask,
                                        record->manufacturerCode,
                                        dataType,
                                        dataSize,
                                        data);

  // Post-write attribute callback specific
  // to the cluster that the attribute lives in.
  sli_zigbee_af_cluster_attribute_changed_callback(record->endpoint,
                                                   record->clusterId,
                                                   record->attributeId,
                                                   record->clusterMask,
                                                   record->manufacturerCode);
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

// If dataPtr is NULL, no data is copied to the caller.
// readLength should be 0 in that case.

sl_zigbee_af_status_t sli_zigbee_af_read_attribute(uint8_t endpoint,
                                                   sl_zigbee_af_cluster_id_t cluster,
                                                   sl_zigbee_af_attribute_id_t attributeID,
                                                   uint8_t mask,
                                                   uint16_t manufacturerCode,
                                                   uint8_t *dataPtr,
                                                   uint16_t readLength,
                                                   sl_zigbee_af_attribute_type_t *dataType)
{
  sl_zigbee_af_attribute_metadata_t *metadata = NULL;
  sl_zigbee_af_attribute_search_record_t record;
  sl_zigbee_af_status_t status;
  record.endpoint = endpoint;
  record.clusterId = cluster;
  record.clusterMask = mask;
  record.attributeId = attributeID;
  record.manufacturerCode = manufacturerCode;
  status = sli_zigbee_af_read_attribute_from_storage(&record,
                                                     &metadata,
                                                     dataPtr,
                                                     readLength);

  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    // It worked!  If the user asked for the type, set it before returning.
    if (dataType != NULL) {
      (*dataType) = metadata->attributeType;
    }
  } else { // failed, print debug info
    if (status == SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE) {
      sl_zigbee_af_attributes_println("READ: attribute size too large for caller");
      sl_zigbee_af_attributes_flush();
    }
#if defined(EZSP_HOST) && defined(SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT)
    else if (status == SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE) {
      // If no attribute was found then try searching on NCP.
      // It's not possible to return metadata of attribute which reside on NCP, but we can retreive dataType.
      sl_zigbee_af_attribute_type_t attDataType;
      status = sli_zigbee_af_support_read_attribute(&record, &attDataType, dataPtr, readLength);
      if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS && dataType != NULL) {
        *dataType = attDataType;
      }
    }
#endif
    else {
      // to Prevent Misra warning
    }
  }

  return status;
}

#ifdef DEFER_ATTRIBUTE_UPDATES_IN_NVM

typedef struct {
  sl_zigbee_af_cluster_id_t   cluster_id;
  sl_zigbee_af_attribute_id_t attribute_id;
  sl_zigbee_af_cluster_mask_t cluster_mask;
  uint8_t                     endpoint;
  uint16_t                    manufacturer_code;
} deferred_zcl_write_attribute_record_t;

static deferred_zcl_write_attribute_record_t deferred_write_values[NUM_PERSISTED_ZCL_ATTRIBUTES];
static sl_zigbee_af_event_t deferred_attribute_write_events[NUM_PERSISTED_ZCL_ATTRIBUTES];

uint8_t find_event_index(sl_zigbee_af_event_t* event, sl_zigbee_af_event_t events[])
{
  for (uint8_t i = 0; i < NUM_PERSISTED_ZCL_ATTRIBUTES; i++) {
    if (event == &events[i]) {
      return i;
    }
  }
  return 0xFF;
}

// When deferring an attribute write, we need to arm an event. To find that event,
// we search for currently armed events to see if we already armed an event for the
// write that we want to do, else, we find a free event
sl_zigbee_af_event_t* find_armed_or_free_event(deferred_zcl_write_attribute_record_t write_record, sl_zigbee_af_event_t events[], uint8_t* event_index)
{
  for (uint8_t i = 0; i < NUM_PERSISTED_ZCL_ATTRIBUTES; i++) {
    bool is_scheduled = sli_zigbee_event_is_scheduled(&events[i]);
    if (is_scheduled) {
      deferred_zcl_write_attribute_record_t armed_write_record = deferred_write_values[i];
      if (write_record.endpoint == armed_write_record.endpoint
          && write_record.cluster_id == armed_write_record.cluster_id
          && write_record.attribute_id == armed_write_record.attribute_id
          && write_record.cluster_mask == armed_write_record.cluster_mask
          && write_record.manufacturer_code == armed_write_record.manufacturer_code) {
        *event_index = i;
        return &events[i];
      }
    }
  }

  // There isn't an event armed already for this write. Find a free event
  for (uint8_t i = 0; i < NUM_PERSISTED_ZCL_ATTRIBUTES; i++) {
    bool is_scheduled = sli_zigbee_event_is_scheduled(&events[i]);
    if (is_scheduled == false) {
      *event_index = i;
      return &events[i];
    }
  }
  return NULL;
}

void defer_attribute_write_to_token(uint8_t *data,
                                    uint8_t endpoint,
                                    sl_zigbee_af_cluster_id_t clusterId,
                                    sl_zigbee_af_attribute_metadata_t *metadata,
                                    sl_zigbee_af_cluster_mask_t clusterMask,
                                    uint16_t manufacturerCode)
{
  // If the attribute isn't saved in NVM, exit
  if (!sl_zigbee_af_attribute_is_tokenized(metadata)) {
    return;
  }

  deferred_zcl_write_attribute_record_t write_record = {
    .endpoint = endpoint,
    .cluster_id = clusterId,
    .attribute_id = metadata->attributeId,
    .cluster_mask = clusterMask,
    .manufacturer_code = manufacturerCode
  };

  // Arm an event to update the attribute after a delay
  uint8_t event_index;
  sl_zigbee_af_event_t* event = find_armed_or_free_event(write_record, deferred_attribute_write_events, &event_index);
  if (!event || (event_index >= NUM_PERSISTED_ZCL_ATTRIBUTES)) {
    // This shouldn't happen. We allocated one event for every persisted attribute, but just
    // in case, we'll write directly to NVM as a fallback
    sli_zigbee_af_save_attribute_to_token(data, endpoint, clusterId, metadata);
    return;
  }

  // Save info and arm event to update NVM later
  deferred_write_values[event_index] = write_record;
  sl_zigbee_af_event_set_delay_ms(event, SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_DEFER_ATTRIBUTE_WRITES_TO_NVM_MS);
}

static void deferred_attribute_write_event_handler(sl_zigbee_af_event_t *event)
{
  sl_zigbee_af_event_set_inactive(event);

  uint8_t index = find_event_index(event, deferred_attribute_write_events);
  if (index >= NUM_PERSISTED_ZCL_ATTRIBUTES) {
    return;
  }

  deferred_zcl_write_attribute_record_t write_value = deferred_write_values[index];

  uint8_t data[ZCL_ATTRIBUTE_LARGEST];

  sl_zigbee_af_attribute_metadata_t *metadata = NULL;
  sl_zigbee_af_attribute_search_record_t record;
  record.endpoint = write_value.endpoint;
  record.clusterId = write_value.cluster_id;
  record.clusterMask = write_value.cluster_mask;
  record.attributeId = write_value.attribute_id;
  record.manufacturerCode = write_value.manufacturer_code;
  sl_zigbee_af_status_t status = sli_zigbee_af_read_attribute_from_storage(&record,
                                                                           &metadata,
                                                                           data,
                                                                           sizeof(data));

  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sli_zigbee_af_save_attribute_to_token(data,
                                          write_value.endpoint,
                                          write_value.cluster_id,
                                          metadata);
  }
}

#endif // DEFER_ATTRIBUTE_UPDATES_IN_NVM

void sli_zigbee_af_init_attribute_storage(void)
{
#ifdef DEFER_ATTRIBUTE_UPDATES_IN_NVM
  // Initialize events
  for (uint8_t i = 0; i < NUM_PERSISTED_ZCL_ATTRIBUTES; i++) {
    sl_zigbee_af_event_init(&deferred_attribute_write_events[i], deferred_attribute_write_event_handler);
  }
#endif // DEFER_ATTRIBUTE_UPDATES_IN_NVM
}
