/***************************************************************************//**
 * @file
 * @brief Routines for the Device Management Client plugin, which is based on
 *        the SE 1.2 cluster.
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
#include "app/framework/util/common.h"
#include "app/framework/plugin/device-management-server/device-management-common.h"
#include "device-management-client.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "zap-cluster-command-parser.h"

static sl_zigbee_af_device_management_info_t pmInfo;
static sl_zigbee_af_device_management_password_t servicePassword;
static sl_zigbee_af_device_management_password_t consumerPassword;

static uint8_t numberOfAttributeSets = 9;
#define isValidAttributeTableIndex(index) ((index) < (numberOfAttributeSets))
static sl_zigbee_af_device_management_attribute_table_t attributeTable[9] = {
  {
    0x00,
    {
      { 0x00, 0x01 }, //min inclusive, max not included
      { 0x10, 0x11 },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
    },
  },
  {
    0x01,
    {
      { 0x00, 0x0E },
      { 0xB0, 0xE8 },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
    },
  },
  {
    0x02,
    {
      { 0x00, 0x2A },
      { 0x30, 0x38 },
      { 0x50, 0x52 },
      { 0x60, 0x66 },
      { 0x70, 0xA0 },
      { 0xB0, 0xB9 },
      { 0xC0, 0xFA },
    }
  },
  {
    0x03,
    {
      { 0x00, 0x01 },
      { 0xC0, 0xC8 },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
    }
  },
  {
    0x04,
    {
      { 0x00, 0x06 },
      { 0x20, 0x34 },
      { 0x41, 0x44 },
      { 0xC0, 0xF0 },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
    }
  },
  {
    0x05,
    {
      { 0x00, 0x03 },
      { 0xC0, 0xD4 },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
    }
  },
  {
    0x06,
    {
      { 0x00, 0x05 },
      { 0x10, 0x16 },
      { 0xC0, 0xD8 },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
    }
  },
  {
    0x07,
    {
      { 0x00, 0x03 },
      { 0xC0, 0xD5 },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
    }
  },
  {
    0x08,
    {
      { 0x00, 0x06 },
      { 0xC0, 0xCF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
      { 0xFF, 0xFF },
    }
  }
};

static void sl_zigbee_af_device_management_cluster_print_pending_status(sl_zigbee_af_device_management_change_pending_flags_t mask)
{
  if (pmInfo.pendingUpdates & mask) {
    sl_zigbee_af_device_management_cluster_println("  Status: pending");
  } else {
    sl_zigbee_af_device_management_cluster_println("  Status: not pending");
  }
}

static sl_zigbee_af_zcl_request_status_t sendDeviceManagementClusterReportWildCardAttribute(uint8_t attributeSet,
                                                                                            uint8_t endpoint)
{
  uint8_t attrSet, j, k;
  uint8_t eventConfiguration;
  sl_zigbee_af_status_t status;
  bool atLeastOneEvent = false;
  uint8_t numberOfAttrSetsToReport = 1; // allow one iteratoion

  if (attributeSet == 0xFF) {
    numberOfAttrSetsToReport = numberOfAttributeSets;
  }

  (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                            | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),
                                           ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                           ZCL_REPORT_EVENT_CONFIGURATION_COMMAND_ID,
                                           "uu",
                                           0,
                                           1);
// The maximum number of intervals possible in attributeRange.
  for (attrSet = 0; attrSet < numberOfAttrSetsToReport; attrSet++) {
    for (j = 0; j < 7; j++) {
      //min and max for each interval.
      uint8_t actualSet = (!isValidAttributeTableIndex(attributeSet)) ? attrSet : attributeSet;
      for (k = attributeTable[actualSet].attributeRange[j].startIndex; k < attributeTable[actualSet].attributeRange[j].endIndex; k++) {
        uint8_t topByte = (attributeSet == 0xFF) ? attributeTable[attrSet].attributeSetId : attributeSet;
        uint16_t attributeId = ((topByte << 8) | k);
        if (sl_zigbee_af_locate_attribute_metadata(endpoint,
                                                   ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                                   attributeId,
                                                   CLUSTER_MASK_CLIENT,
                                                   SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE)) {
          status = sl_zigbee_af_read_client_attribute(endpoint,
                                                      ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                                      attributeId,
                                                      (uint8_t *)&eventConfiguration,
                                                      sizeof(eventConfiguration));

          if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
            //sl_zigbee_af_device_management_cluster_println("Attributed Id 0x%04X",attributeId);
            atLeastOneEvent = true;
            (void) sl_zigbee_af_put_int16u_in_resp(attributeId);
            (void) sl_zigbee_af_put_int8u_in_resp(eventConfiguration);
          }
        }
      }
    }
  }

  if (!atLeastOneEvent) {
    sl_zigbee_af_device_management_cluster_println("sending default response");
    return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  }
  sl_zigbee_af_send_response();
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

static void writeDeviceManagementClusterWildCardAttribute(uint8_t attributeSet,
                                                          uint8_t endpoint,
                                                          uint8_t attributeConfiguration)
{
  uint8_t attrRange, attrId;

  // CSTAT needs assurance that attributeSet won't exceed array bounds.
  if (!isValidAttributeTableIndex(attributeSet)) {
    return;
  }

  for (attrRange = 0; attrRange < 7; attrRange++) {
    for (attrId = attributeTable[attributeSet].attributeRange[attrRange].startIndex; attrId < attributeTable[attributeSet].attributeRange[attrRange].endIndex; attrId++) {
      uint16_t attributeId = ((attributeSet << 8) | attrId);
      if (sl_zigbee_af_locate_attribute_metadata(endpoint,
                                                 ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                                 attributeId,
                                                 CLUSTER_MASK_CLIENT,
                                                 SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE)) {
        (void)sl_zigbee_af_write_attribute(endpoint,
                                           ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                           attributeId,
                                           CLUSTER_MASK_CLIENT,
                                           (uint8_t*)&attributeConfiguration,
                                           ZCL_BITMAP8_ATTRIBUTE_TYPE);
      }
    }
  }
}

static void writeDeviceManagementClusterByLogTypeAttribute(uint8_t logType,
                                                           uint8_t endpoint,
                                                           uint8_t attributeConfiguration)
{
  uint8_t i, j, k;
  uint8_t eventConfiguration;
  sl_zigbee_af_status_t status;

  for (i = 0; i < numberOfAttributeSets; i++) {
    for (j = 0; j < 7; j++) {
      for (k = attributeTable[i].attributeRange[j].startIndex; k < attributeTable[i].attributeRange[j].endIndex; k++) {
        uint8_t topByte = attributeTable[i].attributeSetId;
        uint16_t attributeId = ((topByte << 8) | k);
        if (sl_zigbee_af_locate_attribute_metadata(endpoint,
                                                   ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                                   attributeId,
                                                   CLUSTER_MASK_CLIENT,
                                                   SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE)) {
          status = sl_zigbee_af_read_client_attribute(endpoint,
                                                      ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                                      attributeId,
                                                      (uint8_t *)&eventConfiguration,
                                                      sizeof(eventConfiguration));
          if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
            if ((eventConfiguration & 0x03) == logType) {
              sl_zigbee_af_status_t status  = sl_zigbee_af_write_attribute(endpoint,
                                                                           ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                                                           attributeId,
                                                                           CLUSTER_MASK_CLIENT,
                                                                           (uint8_t*)&attributeConfiguration,
                                                                           ZCL_BITMAP8_ATTRIBUTE_TYPE);
              if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
                sl_zigbee_af_device_management_cluster_println("Writing %02X to 0x%04X", attributeConfiguration, attributeId);
              }
            }
          }
        }
      }
    }
  }
}

static void writeDeviceManagementClusterByMatchingAttribute(uint8_t currentConfiguration,
                                                            uint8_t endpoint,
                                                            uint8_t attributeConfiguration)
{
  uint8_t i, j, k;
  uint8_t eventConfiguration;
  sl_zigbee_af_status_t status;

  for (i = 0; i < numberOfAttributeSets; i++) {
    for (j = 0; j < 7; j++) {
      for (k = attributeTable[i].attributeRange[j].startIndex; k < attributeTable[i].attributeRange[j].endIndex; k++) {
        uint8_t topByte = attributeTable[i].attributeSetId;
        uint16_t attributeId = ((topByte << 8) | k);
        if (sl_zigbee_af_locate_attribute_metadata(endpoint,
                                                   ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                                   attributeId,
                                                   CLUSTER_MASK_CLIENT,
                                                   SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE)) {
          status = sl_zigbee_af_read_client_attribute(endpoint,
                                                      ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                                      attributeId,
                                                      (uint8_t *)&eventConfiguration,
                                                      sizeof(eventConfiguration));
          if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
            if (eventConfiguration == currentConfiguration) {
              sl_zigbee_af_status_t status  = sl_zigbee_af_write_attribute(endpoint,
                                                                           ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                                                           attributeId,
                                                                           CLUSTER_MASK_CLIENT,
                                                                           (uint8_t*)&attributeConfiguration,
                                                                           ZCL_BITMAP8_ATTRIBUTE_TYPE);
              if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
                sl_zigbee_af_device_management_cluster_println("Writing %02X to 0x%04X", attributeConfiguration, attributeId);
              }
            }
          }
        }
      }
    }
  }
}

static void writeDeviceManagementAttribute(uint16_t attributeId,
                                           uint8_t attributeConfiguration)
{
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  sl_zigbee_af_status_t status  = sl_zigbee_af_write_attribute(endpoint,
                                                               ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                                               attributeId,
                                                               CLUSTER_MASK_CLIENT,
                                                               (uint8_t*)&attributeConfiguration,
                                                               ZCL_BITMAP8_ATTRIBUTE_TYPE);
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_device_management_cluster_println("Wrote %u to %u", attributeConfiguration, attributeId);
  } else {
    sl_zigbee_af_device_management_cluster_println("couldn't write %u to %u", attributeConfiguration, attributeId);
  }
}

void sl_zigbee_af_device_management_cluster_client_init_cb(uint8_t endpoint)
{
  UNUSED_VAR(endpoint);

  pmInfo.providerId = 0;
  pmInfo.issuerEventId = 0;
  pmInfo.tariffType = (sl_zigbee_af_tariff_type_t) 0xFF;
  pmInfo.pendingUpdates = 0;
  attributeTable[0].attributeSetId = 0x01;
  // memset(&servicePassword, 0x00, sizeof(servicePassword));
  // memset(&consumerPassword, 0x00, sizeof(consumerPassword));
}

void sl_zigbee_af_device_management_cluster_client_tick_cb(uint8_t endpoint)
{
  uint32_t currentTime = sl_zigbee_af_get_current_time();
  uint32_t nextTick = currentTime;

  // Action pending changes
  if (pmInfo.pendingUpdates & SL_ZIGBEE_AF_DEVICE_MANAGEMENT_CHANGE_OF_TENANCY_PENDING_MASK) {
    if (currentTime >= pmInfo.tenancy.implementationDateTime) {
      sl_zigbee_af_device_management_cluster_println("DEVICE MANAGEMENT CLIENT: Enacting change of tenancy at time 0x%08X\n",
                                                     currentTime);
      sl_zigbee_af_device_management_client_enact_change_of_tenancy_cb(endpoint,
                                                                       &(pmInfo.tenancy));
      pmInfo.pendingUpdates &= ~(SL_ZIGBEE_AF_DEVICE_MANAGEMENT_CHANGE_OF_TENANCY_PENDING_MASK);
    } else if (pmInfo.tenancy.implementationDateTime - currentTime < nextTick) {
      nextTick = pmInfo.tenancy.implementationDateTime - currentTime;
    }
  }

  if (pmInfo.pendingUpdates & SL_ZIGBEE_AF_DEVICE_MANAGEMENT_CHANGE_OF_SUPPLIER_PENDING_MASK) {
    if (currentTime >= pmInfo.supplier.implementationDateTime) {
      sl_zigbee_af_device_management_cluster_println("DEVICE MANAGEMENT CLIENT: Enacting change of supplier at time 0x%08X\n",
                                                     currentTime);
      sl_zigbee_af_device_management_client_enact_change_of_supplier_cb(endpoint,
                                                                        &(pmInfo.supplier));
      pmInfo.pendingUpdates &= ~(SL_ZIGBEE_AF_DEVICE_MANAGEMENT_CHANGE_OF_SUPPLIER_PENDING_MASK);
    } else if (pmInfo.supplier.implementationDateTime - currentTime < nextTick) {
      nextTick = pmInfo.supplier.implementationDateTime - currentTime;
    }
  }

  if (pmInfo.pendingUpdates & SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_SITE_ID_PENDING_MASK) {
    if (currentTime >= pmInfo.siteId.implementationDateTime) {
      sl_zigbee_af_status_t status = sl_zigbee_af_write_attribute(endpoint,
                                                                  ZCL_SIMPLE_METERING_CLUSTER_ID,
                                                                  ZCL_SITE_ID_ATTRIBUTE_ID,
                                                                  CLUSTER_MASK_SERVER,
                                                                  (uint8_t*)&(pmInfo.siteId.siteId),
                                                                  ZCL_OCTET_STRING_ATTRIBUTE_TYPE);
      if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
        pmInfo.pendingUpdates &= ~(SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_SITE_ID_PENDING_MASK);
      }
      sl_zigbee_af_device_management_cluster_println("DEVICE MANAGEMENT CLIENT: Enacting site id update at time 0x%08X: %d\n",
                                                     currentTime,
                                                     status);
    } else if (pmInfo.siteId.implementationDateTime - currentTime < nextTick) {
      nextTick = pmInfo.siteId.implementationDateTime - currentTime;
    }
  }

  if (pmInfo.pendingUpdates & SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_CIN_PENDING_MASK) {
    if (currentTime >= pmInfo.cin.implementationDateTime) {
      sl_zigbee_af_status_t status = sl_zigbee_af_write_attribute(endpoint,
                                                                  ZCL_SIMPLE_METERING_CLUSTER_ID,
                                                                  ZCL_CUSTOMER_ID_NUMBER_ATTRIBUTE_ID,
                                                                  CLUSTER_MASK_SERVER,
                                                                  (uint8_t*)&(pmInfo.cin.cin),
                                                                  ZCL_OCTET_STRING_ATTRIBUTE_TYPE);
      if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
        pmInfo.pendingUpdates &= ~(SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_CIN_PENDING_MASK);
      }
      sl_zigbee_af_device_management_cluster_println("DEVICE MANAGEMENT CLIENT: Enacting customer id number update at time 0x%08X: %d\n",
                                                     currentTime,
                                                     status);
    } else if (pmInfo.cin.implementationDateTime - currentTime < nextTick) {
      nextTick = pmInfo.cin.implementationDateTime - currentTime;
    }
  }

  if (pmInfo.pendingUpdates & SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_SERVICE_PASSWORD_PENDING_MASK) {
    sl_zigbee_af_device_management_cluster_println("DEVICE MANAGEMENT CLIENT: Service password current time 0x%08X: implementationTime 0x%08X\n",
                                                   currentTime,
                                                   servicePassword.implementationDateTime);
    if (currentTime >= servicePassword.implementationDateTime) {
      pmInfo.servicePassword.implementationDateTime = servicePassword.implementationDateTime;
      pmInfo.servicePassword.durationInMinutes = servicePassword.durationInMinutes;
      pmInfo.servicePassword.passwordType = servicePassword.passwordType;
      memcpy(&pmInfo.servicePassword.password, &servicePassword.password, sl_zigbee_af_string_length(servicePassword.password) + 1);
      pmInfo.pendingUpdates &= (~SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_SERVICE_PASSWORD_PENDING_MASK);
    } else if (servicePassword.implementationDateTime - currentTime < nextTick) {
      nextTick = servicePassword.implementationDateTime - currentTime;
    }
  }

  if (pmInfo.pendingUpdates & SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_CONSUMER_PASSWORD_PENDING_MASK) {
    if (currentTime >= consumerPassword.implementationDateTime) {
      pmInfo.consumerPassword.implementationDateTime = consumerPassword.implementationDateTime;
      pmInfo.consumerPassword.durationInMinutes = consumerPassword.durationInMinutes;
      pmInfo.consumerPassword.passwordType = consumerPassword.passwordType;
      memcpy(&pmInfo.consumerPassword.password, &consumerPassword.password, sl_zigbee_af_string_length(consumerPassword.password) + 1);
      pmInfo.pendingUpdates &= (~SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_CONSUMER_PASSWORD_PENDING_MASK);
    } else if (consumerPassword.implementationDateTime - currentTime < nextTick) {
      nextTick = consumerPassword.implementationDateTime - currentTime;
    }
  }

  // Reschedule the next tick, if necessary
  if (pmInfo.pendingUpdates != 0) {
    sl_zigbee_zcl_schedule_client_tick(endpoint,
                                       ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                       nextTick * MILLISECOND_TICKS_PER_SECOND);
  }
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_device_management_cluster_publish_change_of_tenancy_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_device_management_cluster_publish_change_of_tenancy_command_t cmd_data;
  uint32_t currentTime = sl_zigbee_af_get_current_time();
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  sl_zigbee_af_status_t status;

  if (zcl_decode_device_management_cluster_publish_change_of_tenancy_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_device_management_cluster_println("RX: PublishChangeOfTenancy: 0x%08X, 0x%08X, 0x%02X, 0x%08X, 0x%08X",
                                                 cmd_data.providerId,
                                                 cmd_data.issuerEventId,
                                                 cmd_data.tariffType,
                                                 cmd_data.implementationDateTime,
                                                 cmd_data.proposedTenancyChangeControl);

  if (pmInfo.providerId == 0) {
    pmInfo.providerId = cmd_data.providerId;
  }

  if (cmd_data.issuerEventId > pmInfo.issuerEventId) {
    pmInfo.issuerEventId = cmd_data.issuerEventId;
  }

  pmInfo.tariffType = (sl_zigbee_af_tariff_type_t)cmd_data.tariffType;

  pmInfo.tenancy.implementationDateTime = cmd_data.implementationDateTime;
  pmInfo.tenancy.tenancy = cmd_data.proposedTenancyChangeControl;

  // Even if we aren't to immediately action the change of tenancy,
  // we still set these attributes accordingly
  status = sl_zigbee_af_write_attribute(endpoint,
                                        ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                        ZCL_CHANGE_OF_TENANCY_UPDATE_DATE_TIME_ATTRIBUTE_ID,
                                        CLUSTER_MASK_CLIENT,
                                        (uint8_t*)&cmd_data.implementationDateTime,
                                        ZCL_UTC_TIME_ATTRIBUTE_TYPE);

  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    goto kickout;
  }

  status = sl_zigbee_af_write_attribute(endpoint,
                                        ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                        ZCL_PROPOSED_TENANCY_CHANGE_CONTROL_ATTRIBUTE_ID,
                                        CLUSTER_MASK_CLIENT,
                                        (uint8_t*)&cmd_data.proposedTenancyChangeControl,
                                        ZCL_INT32U_ATTRIBUTE_TYPE);

  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    goto kickout;
  }

  // If the time has passed since the change of tenancy was to be implemented, take action
  if (currentTime >= cmd_data.implementationDateTime) {
    sl_zigbee_af_device_management_client_enact_change_of_tenancy_cb(endpoint,
                                                                     &(pmInfo.tenancy));
    pmInfo.pendingUpdates &= ~SL_ZIGBEE_AF_DEVICE_MANAGEMENT_CHANGE_OF_TENANCY_PENDING_MASK;
  } else {
    // Otherwise, wait until the time of implementation
    sl_zigbee_zcl_schedule_client_tick(endpoint,
                                       ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                       (cmd_data.implementationDateTime - currentTime)
                                       * MILLISECOND_TICKS_PER_SECOND);
    pmInfo.pendingUpdates |= SL_ZIGBEE_AF_DEVICE_MANAGEMENT_CHANGE_OF_TENANCY_PENDING_MASK;
  }

  kickout:
  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_device_management_cluster_publish_change_of_supplier_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_device_management_cluster_publish_change_of_supplier_command_t cmd_data;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  uint32_t currentTime = sl_zigbee_af_get_current_time();
  uint8_t endpoint = sl_zigbee_af_current_endpoint();

  if (zcl_decode_device_management_cluster_publish_change_of_supplier_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_device_management_cluster_println("RX: PublishChangeOfSupplier: 0x%08X, 0x%08X, 0x%02X, 0x%08X, 0x%08X, 0x%08X, ",
                                                 cmd_data.currentProviderId,
                                                 cmd_data.issuerEventId,
                                                 cmd_data.tariffType,
                                                 cmd_data.proposedProviderId,
                                                 cmd_data.providerChangeImplementationTime,
                                                 cmd_data.providerChangeControl);
  sl_zigbee_af_device_management_cluster_print_string(cmd_data.proposedProviderName);
  sl_zigbee_af_device_management_cluster_println(", ");
  sl_zigbee_af_device_management_cluster_print_string(cmd_data.proposedProviderContactDetails);
  sl_zigbee_af_device_management_cluster_println("");

  if (cmd_data.proposedProviderName == NULL) {
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
    goto kickout;
  }

  if (pmInfo.providerId == 0) {
    pmInfo.providerId = cmd_data.currentProviderId;
  }

  if (pmInfo.issuerEventId == 0) {
    pmInfo.issuerEventId = cmd_data.issuerEventId;
  }

  pmInfo.tariffType = (sl_zigbee_af_tariff_type_t)cmd_data.tariffType;

  pmInfo.supplier.proposedProviderId = cmd_data.proposedProviderId;
  pmInfo.supplier.implementationDateTime = cmd_data.providerChangeImplementationTime;
  pmInfo.supplier.providerChangeControl = cmd_data.providerChangeControl;
  sl_zigbee_af_copy_string(pmInfo.supplier.proposedProviderName,
                           cmd_data.proposedProviderName,
                           SL_ZIGBEE_AF_DEVICE_MANAGEMENT_MAXIMUM_PROPOSED_PROVIDER_NAME_LENGTH);
  sl_zigbee_af_copy_string(pmInfo.supplier.proposedProviderContactDetails,
                           cmd_data.proposedProviderContactDetails,
                           SL_ZIGBEE_AF_DEVICE_MANAGEMENT_MAXIMUM_PROPOSED_PROVIDER_CONTACT_DETAILS_LENGTH);

  // If the time has passed since the change of supplier was to be implemented, take action
  if (currentTime >= cmd_data.providerChangeImplementationTime) {
    sl_zigbee_af_device_management_client_enact_change_of_supplier_cb(endpoint,
                                                                      &(pmInfo.supplier));
    pmInfo.pendingUpdates &= ~SL_ZIGBEE_AF_DEVICE_MANAGEMENT_CHANGE_OF_SUPPLIER_PENDING_MASK;
  } else {
    // Otherwise, wait until the time of implementation
    sl_zigbee_zcl_schedule_client_tick(endpoint,
                                       ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                       (cmd_data.providerChangeImplementationTime - currentTime)
                                       * MILLISECOND_TICKS_PER_SECOND);
    pmInfo.pendingUpdates |= SL_ZIGBEE_AF_DEVICE_MANAGEMENT_CHANGE_OF_SUPPLIER_PENDING_MASK;
  }

  kickout:
  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_device_management_cluster_request_new_password_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_device_management_cluster_request_new_password_response_command_t cmd_data;
  uint32_t currentTime = sl_zigbee_af_get_current_time();
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  sl_zigbee_af_device_management_password_t *pass;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;

  if (zcl_decode_device_management_cluster_request_new_password_response_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (cmd_data.password == NULL) {
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
    goto kickout;
  }

  if (cmd_data.issuerEventId > pmInfo.issuerEventId) {
    pmInfo.issuerEventId = cmd_data.issuerEventId;
  }

  if (cmd_data.implementationDateTime < currentTime && cmd_data.implementationDateTime != 0x00000000) {
    goto kickout;
  }

  if (cmd_data.passwordType & SERVICE_PASSWORD) {
    if (currentTime == cmd_data.implementationDateTime || cmd_data.implementationDateTime == 0x00000000) {
      pass = &(pmInfo.servicePassword);
    } else {
      pass = &(servicePassword);
      pmInfo.pendingUpdates |= SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_SERVICE_PASSWORD_PENDING_MASK;
      sl_zigbee_zcl_schedule_client_tick(endpoint,
                                         ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                         (cmd_data.implementationDateTime - currentTime)
                                         * MILLISECOND_TICKS_PER_SECOND);
    }
  } else if (cmd_data.passwordType & CONSUMER_PASSWORD) {
    if (currentTime == cmd_data.implementationDateTime || cmd_data.implementationDateTime == (0x00000000)) {
      pass = &(pmInfo.consumerPassword);
    } else {
      pass = &(consumerPassword);
      pmInfo.pendingUpdates |= SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_CONSUMER_PASSWORD_PENDING_MASK;
      sl_zigbee_zcl_schedule_client_tick(endpoint,
                                         ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                         (cmd_data.implementationDateTime - currentTime)
                                         * MILLISECOND_TICKS_PER_SECOND);
    }
  } else {
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
    goto kickout;
  }

  pass->implementationDateTime = cmd_data.implementationDateTime;
  pass->durationInMinutes = cmd_data.durationInMinutes;
  pass->passwordType = cmd_data.passwordType;
  sl_zigbee_af_copy_string(pass->password,
                           cmd_data.password,
                           SL_ZIGBEE_AF_DEVICE_MANAGEMENT_MAXIMUM_PASSWORD_LENGTH);

  kickout:
  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_device_management_cluster_update_site_id_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_device_management_cluster_update_site_id_command_t cmd_data;
  uint32_t currentTime = sl_zigbee_af_get_current_time();
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_FAILURE;

  if (zcl_decode_device_management_cluster_update_site_id_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (pmInfo.providerId != cmd_data.providerId) {
    status = SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED;
    goto kickout;
  }

  sl_zigbee_af_copy_string(pmInfo.siteId.siteId,
                           cmd_data.siteId,
                           SL_ZIGBEE_AF_DEVICE_MANAGEMENT_MAXIMUM_SITE_ID_LENGTH);

  // If siteIdTime is 0xFFFFFFFF, cancel any pending change and replace it with
  // the incoming site ID
  // cancelling a scheduled update
  if (cmd_data.siteIdTime == 0xFFFFFFFF) {
    if ((pmInfo.pendingUpdates & SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_SITE_ID_PENDING_MASK)
        && (pmInfo.providerId == cmd_data.providerId)
        && (pmInfo.siteId.issuerEventId == cmd_data.issuerEventId)) {
      pmInfo.pendingUpdates &= ~SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_SITE_ID_PENDING_MASK;
      memset(&(pmInfo.siteId), 0, sizeof(sl_zigbee_af_device_management_site_id_t));
      status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    } else {
      sl_zigbee_af_device_management_cluster_println("Unable to cancel scheduled siteId update.");
      sl_zigbee_af_device_management_cluster_println("Provider ID: 0x%08X", pmInfo.providerId);
      sl_zigbee_af_device_management_cluster_println("Issuer Event ID: 0x%08X", pmInfo.issuerEventId);
      sl_zigbee_af_device_management_cluster_print_pending_status(SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_SITE_ID_PENDING_MASK);
      status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
      goto kickout;
    }
  } else { // schedule siteid update or immediate update
    if (cmd_data.issuerEventId > pmInfo.issuerEventId) {
      pmInfo.issuerEventId = cmd_data.issuerEventId;
    } else {
      status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
      goto kickout;
    }

    // implementation time:
    //   0x00000000 - execute cmd immediately
    //   0xFFFFFFFF - cancel update (case covered by previous code blok)
    //   otherwise  - schedule for new pending action
    if (cmd_data.siteIdTime == 0x00000000) {
      // Are we meter or ihd?
      // Meter - update attr and reply with error if not sucessful
      // Ihd - no update attr needed. reply success.
      status = sl_zigbee_af_write_attribute(endpoint,
                                            ZCL_SIMPLE_METERING_CLUSTER_ID,
                                            ZCL_SITE_ID_ATTRIBUTE_ID,
                                            CLUSTER_MASK_SERVER,
                                            (uint8_t*)cmd_data.siteId,
                                            ZCL_OCTET_STRING_ATTRIBUTE_TYPE);
      if (sl_zigbee_af_contains_server(sl_zigbee_af_current_endpoint(), ZCL_SIMPLE_METERING_CLUSTER_ID)
          && (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS)) {
        sl_zigbee_af_device_management_cluster_println("Unable to write siteId attr in Metering cluster: 0x%d", status);
        status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
      } else {
        status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
      }
      pmInfo.pendingUpdates &= ~SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_SITE_ID_PENDING_MASK;
      memset(&(pmInfo.siteId), 0, sizeof(sl_zigbee_af_device_management_site_id_t));
    } else {
      sl_zigbee_zcl_schedule_client_tick(endpoint,
                                         ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                         (cmd_data.siteIdTime - currentTime)
                                         * MILLISECOND_TICKS_PER_SECOND);
      pmInfo.pendingUpdates |= SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_SITE_ID_PENDING_MASK;
      sl_zigbee_af_copy_string(pmInfo.siteId.siteId,
                               cmd_data.siteId,
                               SL_ZIGBEE_AF_DEVICE_MANAGEMENT_MAXIMUM_SITE_ID_LENGTH);
      pmInfo.siteId.implementationDateTime = cmd_data.siteIdTime;
      pmInfo.siteId.issuerEventId = cmd_data.issuerEventId;
      status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    }
  }

  kickout:
  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_device_management_cluster_update_cin_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_device_management_cluster_update_cin_command_t cmd_data;
  uint32_t currentTime = sl_zigbee_af_get_current_time();
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_FAILURE;

  if (zcl_decode_device_management_cluster_update_cin_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (pmInfo.providerId != cmd_data.providerId) {
    status = SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED;
    goto kickout;
  }

  // cancelling a scheduled update
  if (cmd_data.implementationTime == 0xFFFFFFFF) {
    if ((pmInfo.pendingUpdates & SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_CIN_PENDING_MASK)
        && (pmInfo.providerId == cmd_data.providerId)
        && (pmInfo.cin.issuerEventId == cmd_data.issuerEventId)) {
      pmInfo.pendingUpdates &= ~SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_CIN_PENDING_MASK;
      memset(&(pmInfo.cin), 0, sizeof(sl_zigbee_af_device_management_c_i_n_t));
      status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    } else {
      sl_zigbee_af_device_management_cluster_println("Unable to cancel scheduled CIN update.");
      sl_zigbee_af_device_management_cluster_println("Provider ID: 0x%08X", pmInfo.providerId);
      sl_zigbee_af_device_management_cluster_println("Issuer Event ID: 0x%08X", pmInfo.issuerEventId);
      sl_zigbee_af_device_management_cluster_print_pending_status(SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_CIN_PENDING_MASK);
      status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
      goto kickout;
    }
  } else { // schedule CIN update or immediate update
    if (cmd_data.issuerEventId > pmInfo.issuerEventId) {
      pmInfo.issuerEventId = cmd_data.issuerEventId;
    } else {
      status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
      goto kickout;
    }

    // implementation time:
    //   0x00000000 - execute cmd immediately
    //   0xFFFFFFFF - cancel update (case covered by previous code blok)
    //   otherwise  - schedule for new pending action
    if (cmd_data.implementationTime == 0x00000000) {
#ifdef SL_CATALOG_ZIGBEE_SIMPLE_METERING_SERVER_PRESENT
      // only try to update attr if we actually implement cluster / ESI
      status = sl_zigbee_af_write_attribute(endpoint,
                                            ZCL_SIMPLE_METERING_CLUSTER_ID,
                                            ZCL_CUSTOMER_ID_NUMBER_ATTRIBUTE_ID,
                                            CLUSTER_MASK_SERVER,
                                            (uint8_t*)cmd_data.customerIdNumber,
                                            ZCL_OCTET_STRING_ATTRIBUTE_TYPE);
      if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
        sl_zigbee_af_device_management_cluster_println("Unable to write CIN attr in Metering cluster: 0x%d", status);
        status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
      } else {
#endif
      status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
#ifdef SL_CATALOG_ZIGBEE_SIMPLE_METERING_SERVER_PRESENT
    }
#endif
      pmInfo.pendingUpdates &= ~SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_CIN_PENDING_MASK;
    } else {
      sl_zigbee_zcl_schedule_client_tick(endpoint,
                                         ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                         (cmd_data.implementationTime - currentTime)
                                         * MILLISECOND_TICKS_PER_SECOND);
      pmInfo.pendingUpdates |= SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_CIN_PENDING_MASK;
      status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    }

    sl_zigbee_af_copy_string(pmInfo.cin.cin, cmd_data.customerIdNumber, SL_ZIGBEE_AF_DEVICE_MANAGEMENT_MAXIMUM_CIN_LENGTH);
    pmInfo.cin.implementationDateTime = cmd_data.implementationTime;
    pmInfo.cin.issuerEventId = cmd_data.issuerEventId;
  }

  kickout:
  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_device_management_cluster_get_event_configuration_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_device_management_cluster_get_event_configuration_command_t cmd_data;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t eventConfiguration;
  sl_zigbee_af_status_t status;

  if (zcl_decode_device_management_cluster_get_event_configuration_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if ((cmd_data.eventId & 0xFF) == 0xFF) {
    //sl_zigbee_af_device_management_cluster_println("Wildcard profile Id requested %u", cmd_data.eventId & 0xFF00);
    uint8_t attributeSet = (cmd_data.eventId & 0xFF00) >> 8;
    //sl_zigbee_af_device_management_cluster_println("attribute set %u",attributeSet);
    if ((attributeSet < 1 || attributeSet > 8) && attributeSet != 0xFF) {
      return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
    } else {
      return sendDeviceManagementClusterReportWildCardAttribute(attributeSet,
                                                                endpoint);
    }
  } else {
    sl_zigbee_af_device_management_cluster_println("Get Event callback %u", cmd_data.eventId);
    status = sl_zigbee_af_read_client_attribute(endpoint,
                                                ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                                cmd_data.eventId,
                                                (uint8_t *)&eventConfiguration,
                                                sizeof(eventConfiguration));
    sl_zigbee_af_device_management_cluster_println("Get Event status %u eventConfiguration %u", status, eventConfiguration);

    if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                                | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),
                                               ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                               ZCL_REPORT_EVENT_CONFIGURATION_COMMAND_ID,
                                               "uu",
                                               0,
                                               1);
      (void) sl_zigbee_af_put_int16u_in_resp(cmd_data.eventId);
      (void) sl_zigbee_af_put_int8u_in_resp(eventConfiguration);
      sl_zigbee_af_send_response();
      return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
    } else {
      return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
    }
  }
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_device_management_cluster_set_event_configuration_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_device_management_cluster_set_event_configuration_command_t cmd_data;
  uint8_t payloadIndex = 0;

  if (zcl_decode_device_management_cluster_set_event_configuration_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  switch (cmd_data.configurationControl) {
    case SL_ZIGBEE_ZCL_EVENT_CONFIGURATION_CONTROL_APPLY_BY_LIST: {
      uint8_t numberOfEvents = cmd_data.eventConfigurationPayload[0];
      sl_zigbee_af_device_management_cluster_println("Number of Events %u", numberOfEvents);
      payloadIndex = 1;
      while (payloadIndex < numberOfEvents * 2) {
        uint16_t attributeId = sl_zigbee_af_get_int16u(cmd_data.eventConfigurationPayload, payloadIndex, numberOfEvents * 2 + 1);
        payloadIndex += 2;
        sl_zigbee_af_device_management_cluster_println("AttributeId 0x%04X", attributeId);
        writeDeviceManagementAttribute(attributeId, cmd_data.eventConfiguration);
      }
      break;
    }
    case SL_ZIGBEE_ZCL_EVENT_CONFIGURATION_CONTROL_APPLY_BY_EVENT_GROUP: {
      uint16_t eventGroupId = sl_zigbee_af_get_int16u(cmd_data.eventConfigurationPayload, 0, 2);
      uint8_t attributeSet = (eventGroupId & 0xFF00) >> 8;
      writeDeviceManagementClusterWildCardAttribute(attributeSet,
                                                    sl_zigbee_af_current_endpoint(),
                                                    cmd_data.eventConfiguration);
      break;
    }
    case SL_ZIGBEE_ZCL_EVENT_CONFIGURATION_CONTROL_APPLY_BY_LOG_TYPE: {
      uint8_t logType = cmd_data.eventConfigurationPayload[0];
      writeDeviceManagementClusterByLogTypeAttribute(logType,
                                                     sl_zigbee_af_current_endpoint(),
                                                     cmd_data.eventConfiguration);
      break;
    }
    case SL_ZIGBEE_ZCL_EVENT_CONFIGURATION_CONTROL_APPLY_BY_CONFIGURATION_MATCH: {
      uint8_t currentConfiguration = cmd_data.eventConfigurationPayload[0];
      writeDeviceManagementClusterByMatchingAttribute(currentConfiguration,
                                                      sl_zigbee_af_current_endpoint(),
                                                      cmd_data.eventConfiguration);
      break;
    }
    default:
      return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

void sl_zigbee_af_device_management_client_print(void)

{
  sl_zigbee_af_device_management_cluster_println("== Device Management Information ==\n");

  sl_zigbee_af_device_management_cluster_println("  == Tenancy ==");
  sl_zigbee_af_device_management_cluster_println("  Implementation Date / Time: 0x%08X", pmInfo.tenancy.implementationDateTime);
  sl_zigbee_af_device_management_cluster_println("  Tenancy: 0x%08X", pmInfo.tenancy.tenancy);
  sl_zigbee_af_device_management_cluster_print_pending_status(SL_ZIGBEE_AF_DEVICE_MANAGEMENT_CHANGE_OF_TENANCY_PENDING_MASK);
  sl_zigbee_af_device_management_cluster_println("");

  sl_zigbee_af_device_management_cluster_println("  == Supplier ==");
  sl_zigbee_af_device_management_cluster_print("  Provider name: ");
  sl_zigbee_af_device_management_cluster_print_string(pmInfo.supplier.proposedProviderName);
  sl_zigbee_af_device_management_cluster_println("\n  Proposed Provider Id: 0x%08X", pmInfo.supplier.proposedProviderId);
  sl_zigbee_af_device_management_cluster_println("  Implementation Date / Time: 0x%08X", pmInfo.supplier.implementationDateTime);
  sl_zigbee_af_device_management_cluster_println("  Provider Change Control: 0x%08X", pmInfo.supplier.providerChangeControl);
  sl_zigbee_af_device_management_cluster_print_pending_status(SL_ZIGBEE_AF_DEVICE_MANAGEMENT_CHANGE_OF_TENANCY_PENDING_MASK);
  sl_zigbee_af_device_management_cluster_println("");

  sl_zigbee_af_device_management_cluster_println("  == Site ID ==");
  sl_zigbee_af_device_management_cluster_print("  Site ID: ");
  sl_zigbee_af_device_management_cluster_print_string(pmInfo.siteId.siteId);
  sl_zigbee_af_device_management_cluster_println("\n  Site Id Implementation Date / Time: 0x%08X", pmInfo.siteId.implementationDateTime);
  sl_zigbee_af_device_management_cluster_print_pending_status(SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_SITE_ID_PENDING_MASK);
  sl_zigbee_af_device_management_cluster_println("");

  sl_zigbee_af_device_management_cluster_println("  == Customer ID Number ==");
  sl_zigbee_af_device_management_cluster_print("  Customer ID Number: ");
  sl_zigbee_af_device_management_cluster_print_string(pmInfo.cin.cin);
  sl_zigbee_af_device_management_cluster_println("\n  Customer ID Number Implementation Date / Time: 0x%08X\n", pmInfo.cin.implementationDateTime);
  sl_zigbee_af_device_management_cluster_print_pending_status(SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_CIN_PENDING_MASK);

  sl_zigbee_af_device_management_cluster_println("  == Passwords ==");
  sl_zigbee_af_device_management_cluster_println("   = Service Password =");
  sl_zigbee_af_device_management_cluster_print("   Password: ");
  sl_zigbee_af_device_management_cluster_print_string(pmInfo.servicePassword.password);
  sl_zigbee_af_device_management_cluster_println("\n   Implementation Date / Time: 0x%08X", pmInfo.servicePassword.implementationDateTime);
  sl_zigbee_af_device_management_cluster_println("   Duration In Minutes: 0x%04X", pmInfo.servicePassword.durationInMinutes);
  sl_zigbee_af_device_management_cluster_println("   Password Type: 0x%02X\n", pmInfo.servicePassword.passwordType);

  sl_zigbee_af_device_management_cluster_println("   = Consumer Password =");
  sl_zigbee_af_device_management_cluster_print("   Password: ");
  sl_zigbee_af_device_management_cluster_print_string(pmInfo.consumerPassword.password);
  sl_zigbee_af_device_management_cluster_println("\n   Implementation Date / Time: 0x%08X", pmInfo.consumerPassword.implementationDateTime);
  sl_zigbee_af_device_management_cluster_println("   Duration In Minutes: 0x%04X", pmInfo.consumerPassword.durationInMinutes);
  sl_zigbee_af_device_management_cluster_println("   Password Type: 0x%02X\n", pmInfo.consumerPassword.passwordType);

  sl_zigbee_af_device_management_cluster_println("  == Issuer Event ID ==");
  sl_zigbee_af_device_management_cluster_println("  Issuer Event ID: 0x%08X\n", pmInfo.issuerEventId);

  sl_zigbee_af_device_management_cluster_println("  == Provider ID ==");
  sl_zigbee_af_device_management_cluster_println("  Provider Id: 0x%08X", pmInfo.providerId);

  sl_zigbee_af_device_management_cluster_println("== End of Device Management Information ==");
}

uint32_t sl_zigbee_af_device_management_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                                     sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_PUBLISH_CHANGE_OF_TENANCY_COMMAND_ID:
      {
        status = sl_zigbee_af_device_management_cluster_publish_change_of_tenancy_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_CHANGE_OF_SUPPLIER_COMMAND_ID:
      {
        status = sl_zigbee_af_device_management_cluster_publish_change_of_supplier_cb(cmd);
        break;
      }
      case ZCL_REQUEST_NEW_PASSWORD_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_device_management_cluster_request_new_password_response_cb(cmd);
        break;
      }
      case ZCL_UPDATE_SITE_ID_COMMAND_ID:
      {
        status = sl_zigbee_af_device_management_cluster_update_site_id_cb(cmd);
        break;
      }
      case ZCL_SET_EVENT_CONFIGURATION_COMMAND_ID:
      {
        status = sl_zigbee_af_device_management_cluster_set_event_configuration_cb(cmd);
        break;
      }
      case ZCL_GET_EVENT_CONFIGURATION_COMMAND_ID:
      {
        status = sl_zigbee_af_device_management_cluster_get_event_configuration_cb(cmd);
        break;
      }
      case ZCL_UPDATE_CIN_COMMAND_ID:
      {
        status = sl_zigbee_af_device_management_cluster_update_cin_cb(cmd);
        break;
      }
    }
  }

  return status;
}
