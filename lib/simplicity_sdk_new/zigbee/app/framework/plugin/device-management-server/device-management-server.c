/***************************************************************************//**
 * @file
 * @brief Server side implementation of the SE 1.2 cluster, Device Management.
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
#include "app/framework/plugin/device-management-server/device-management-server.h"

#include "zap-cluster-command-parser.h"

static sl_zigbee_af_device_management_info_t pmInfo;

void sl_zigbee_af_device_management_server_init_cb(uint8_t init_level)
{
  (void)init_level;

  // invalidate passwords since 0 means forever valid
  pmInfo.servicePassword.durationInMinutes = 1;
  pmInfo.consumerPassword.durationInMinutes = 1;

  // since implementationDateTime of 0 is special. we'll be using 1 (12/31/1970)
  // to initialize as a non-valid pending time.
  pmInfo.cin.implementationDateTime = 1;
  pmInfo.pendingUpdates = 0;
}

bool sl_zigbee_af_device_management_set_provider_id(uint32_t providerId)
{
  pmInfo.providerId = providerId;
  return true;
}

bool sl_zigbee_af_device_management_set_issuer_event_id(uint32_t issuerEventId)
{
  pmInfo.issuerEventId = issuerEventId;
  return true;
}

bool sl_zigbee_af_device_management_set_tariff_type(sl_zigbee_af_tariff_type_t tariffType)
{
  pmInfo.tariffType = (sl_zigbee_af_tariff_type_t )tariffType;
  return true;
}

bool sl_zigbee_af_device_management_set_info_global_data(uint32_t providerId,
                                                         uint32_t issuerEventId,
                                                         uint8_t tariffType)
{
  pmInfo.providerId = providerId;
  pmInfo.issuerEventId = issuerEventId;
  pmInfo.tariffType = (sl_zigbee_af_tariff_type_t ) tariffType;

  return true;
}

bool sl_zigbee_af_device_management_set_tenancy(sl_zigbee_af_device_management_tenancy_t *tenancy,
                                                bool validateOptionalFields)
{
  // Don't allow tenancy to be set with an older event ID than we currently have.
  if (validateOptionalFields) {
    if ((tenancy->providerId != pmInfo.providerId)
        || (tenancy->issuerEventId < pmInfo.issuerEventId)
        || (tenancy->tariffType != pmInfo.tariffType)) {
      return false;
    }
  }

  if (tenancy == NULL) {
    memset(&(pmInfo.tenancy), 0, sizeof(sl_zigbee_af_device_management_tenancy_t));
  } else {
    memmove(&(pmInfo.tenancy), tenancy, sizeof(sl_zigbee_af_device_management_tenancy_t));
  }

  return true;
}

bool sl_zigbee_af_device_management_get_tenancy(sl_zigbee_af_device_management_tenancy_t *tenancy)
{
  if (tenancy == NULL) {
    return false;
  }

  memmove(tenancy, &(pmInfo.tenancy), sizeof(sl_zigbee_af_device_management_tenancy_t));

  return true;
}

bool sl_zigbee_af_device_management_set_supplier(uint8_t endpoint, sl_zigbee_af_device_management_supplier_t *supplier)
{
  if (supplier == NULL) {
    memset(&(pmInfo.supplier), 0, sizeof(sl_zigbee_af_device_management_supplier_t));
  } else {
    memmove(&(pmInfo.supplier), supplier, sizeof(sl_zigbee_af_device_management_supplier_t));
  }

  if (supplier->implementationDateTime <= sl_zigbee_af_get_current_time()) {
    sl_status_t status  = sl_zigbee_af_write_attribute(endpoint,
                                                       ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                                       ZCL_PROVIDER_NAME_ATTRIBUTE_ID,
                                                       CLUSTER_MASK_SERVER,
                                                       (uint8_t *)&(supplier->proposedProviderName),
                                                       ZCL_OCTET_STRING_ATTRIBUTE_TYPE);
    if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      status  = sl_zigbee_af_write_attribute(endpoint,
                                             ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                             ZCL_PROVIDER_CONTACT_DETAILS_ATTRIBUTE_ID,
                                             CLUSTER_MASK_SERVER,
                                             (uint8_t *)&(supplier->proposedProviderContactDetails),
                                             ZCL_OCTET_STRING_ATTRIBUTE_TYPE);
    }
  } else {
    // TODO: need to setup timer to 'schedule' attribute update when implementation is reached.
  }

  return true;
}

bool sl_zigbee_af_device_management_get_supplier(sl_zigbee_af_device_management_supplier_t *supplier)
{
  if (supplier == NULL) {
    return false;
  }

  memmove(supplier, &(pmInfo.supplier), sizeof(sl_zigbee_af_device_management_supplier_t));

  return true;
}

bool sl_zigbee_af_device_management_set_site_id(sl_zigbee_af_device_management_site_id_t *siteId)
{
  if (siteId == NULL) {
    memset(&(pmInfo.siteId), 0, sizeof(sl_zigbee_af_device_management_site_id_t));
  } else {
    memmove(&(pmInfo.siteId), siteId, sizeof(sl_zigbee_af_device_management_site_id_t));
  }

  return true;
}

bool sl_zigbee_af_device_management_get_site_id(sl_zigbee_af_device_management_site_id_t *siteId)
{
  if (siteId == NULL) {
    return false;
  }

  memmove(siteId, &(pmInfo.siteId), sizeof(sl_zigbee_af_device_management_site_id_t));

  return true;
}

bool sl_zigbee_af_device_management_set_cin(sl_zigbee_af_device_management_c_i_n_t *cin)
{
  memcpy(&(pmInfo.cin), cin, sizeof(sl_zigbee_af_device_management_c_i_n_t));
  return true;
}

bool sl_zigbee_af_device_management_get_cin(sl_zigbee_af_device_management_c_i_n_t *cin)
{
  if (cin == NULL) {
    return false;
  }

  memcpy(cin, &(pmInfo.cin), sizeof(sl_zigbee_af_device_management_c_i_n_t));
  return true;
}

bool sl_zigbee_af_device_management_set_password(sl_zigbee_af_device_management_password_t *password)
{
  if (password == NULL) {
    memset(&(pmInfo.servicePassword), 0, sizeof(sl_zigbee_af_device_management_password_t));
    memset(&(pmInfo.consumerPassword), 0, sizeof(sl_zigbee_af_device_management_password_t));
  } else {
    if (password->passwordType == SERVICE_PASSWORD) {
      memmove(&(pmInfo.servicePassword), password, sizeof(sl_zigbee_af_device_management_password_t));
    } else if (password->passwordType == CONSUMER_PASSWORD) {
      memmove(&(pmInfo.consumerPassword), password, sizeof(sl_zigbee_af_device_management_password_t));
    } else {
      return false;
    }
  }

  return true;
}

bool sl_zigbee_af_device_management_get_password(sl_zigbee_af_device_management_password_t *password,
                                                 uint8_t passwordType)
{
  if (password == NULL) {
    return false;
  }

  switch (passwordType) {
    case SERVICE_PASSWORD:
      memmove(password, &(pmInfo.servicePassword), sizeof(sl_zigbee_af_device_management_password_t));
      break;
    case CONSUMER_PASSWORD:
      memmove(password, &(pmInfo.consumerPassword), sizeof(sl_zigbee_af_device_management_password_t));
      break;
    default:
      return false;
  }

  return true;
}

void sl_zigbee_af_device_management_server_print(void)
{
  sl_zigbee_af_device_management_cluster_println("== Device Management Information ==\n");

  sl_zigbee_af_device_management_cluster_println("  == Tenancy ==");
  sl_zigbee_af_device_management_cluster_println("  Implementation Date / Time: %08X", pmInfo.tenancy.implementationDateTime);
  sl_zigbee_af_device_management_cluster_println("  Tenancy: %08X\n", pmInfo.tenancy.tenancy);

  sl_zigbee_af_device_management_cluster_println("  == Supplier ==");
  sl_zigbee_af_device_management_cluster_print("  Provider name: ");
  sl_zigbee_af_device_management_cluster_print_string(pmInfo.supplier.proposedProviderName);
  sl_zigbee_af_device_management_cluster_println("\n  Proposed Provider Id: %08X", pmInfo.supplier.proposedProviderId);
  sl_zigbee_af_device_management_cluster_println("  Implementation Date / Time: %08X", pmInfo.supplier.implementationDateTime);
  sl_zigbee_af_device_management_cluster_println("  Provider Change Control: %08X\n", pmInfo.supplier.providerChangeControl);

  sl_zigbee_af_device_management_cluster_println("  == Supply ==");
  sl_zigbee_af_device_management_cluster_println("  Request Date / Time: %08X", pmInfo.supply.requestDateTime);
  sl_zigbee_af_device_management_cluster_println("  Implementation Date / Time: %08X", pmInfo.supply.implementationDateTime);
  sl_zigbee_af_device_management_cluster_println("  Supply Status: %02X", pmInfo.supply.supplyStatus);
  sl_zigbee_af_device_management_cluster_println("  Originator Id / Supply Control Bits: %02X\n", pmInfo.supply.originatorIdSupplyControlBits);

  sl_zigbee_af_device_management_cluster_println("  == Site ID ==");
  sl_zigbee_af_device_management_cluster_print("  Site ID: ");
  sl_zigbee_af_device_management_cluster_print_string(pmInfo.siteId.siteId);
  sl_zigbee_af_device_management_cluster_println("\n  Site Id Implementation Date / Time: %08X\n", pmInfo.siteId.implementationDateTime);

  sl_zigbee_af_device_management_cluster_println("  == Customer ID Number ==");
  sl_zigbee_af_device_management_cluster_print("  Customer ID Number: ");
  sl_zigbee_af_device_management_cluster_print_string(pmInfo.cin.cin);
  sl_zigbee_af_device_management_cluster_println("\n  Customer ID Number Implementation Date / Time: %08X\n", pmInfo.cin.implementationDateTime);

  sl_zigbee_af_device_management_cluster_println("  == Supply Status ==");
  sl_zigbee_af_device_management_cluster_println("  Supply Status: %02X\n", pmInfo.supplyStatus.supplyStatus);
  sl_zigbee_af_device_management_cluster_println("  Implementation Date / Time: %08X", pmInfo.supplyStatus.implementationDateTime);

  sl_zigbee_af_device_management_cluster_println("  == Passwords ==");

  sl_zigbee_af_device_management_cluster_println("   = Service Password =");
  sl_zigbee_af_device_management_cluster_print("   Password: ");
  sl_zigbee_af_device_management_cluster_print_string(pmInfo.servicePassword.password);
  sl_zigbee_af_device_management_cluster_println("\n   Implementation Date / Time: %08X", pmInfo.servicePassword.implementationDateTime);
  sl_zigbee_af_device_management_cluster_println("   Duration In Minutes: %04X", pmInfo.servicePassword.durationInMinutes);
  sl_zigbee_af_device_management_cluster_println("   Password Type: %02X\n", pmInfo.servicePassword.passwordType);

  sl_zigbee_af_device_management_cluster_println("   = Consumer Password =");
  sl_zigbee_af_device_management_cluster_print("   Password: ");
  sl_zigbee_af_device_management_cluster_print_string(pmInfo.consumerPassword.password);
  sl_zigbee_af_device_management_cluster_println("\n   Implementation Date / Time: %08X", pmInfo.consumerPassword.implementationDateTime);
  sl_zigbee_af_device_management_cluster_println("   Duration In Minutes: %04X", pmInfo.consumerPassword.durationInMinutes);
  sl_zigbee_af_device_management_cluster_println("   Password Type: %02X\n", pmInfo.consumerPassword.passwordType);

  sl_zigbee_af_device_management_cluster_println("  == Uncontrolled Flow Threshold ==");
  sl_zigbee_af_device_management_cluster_println("  Uncontrolled Flow Threshold: %04X", pmInfo.threshold.uncontrolledFlowThreshold);
  sl_zigbee_af_device_management_cluster_println("  Multiplier: %04X", pmInfo.threshold.multiplier);
  sl_zigbee_af_device_management_cluster_println("  Divisor: %04X", pmInfo.threshold.divisor);
  sl_zigbee_af_device_management_cluster_println("  Measurement Period: %04X", pmInfo.threshold.measurementPeriod);
  sl_zigbee_af_device_management_cluster_println("  Unit of Measure: %02X", pmInfo.threshold.unitOfMeasure);
  sl_zigbee_af_device_management_cluster_println("  Stabilisation Period: %02X\n", pmInfo.threshold.stabilisationPeriod);

  sl_zigbee_af_device_management_cluster_println("  == Issuer Event ID ==");
  sl_zigbee_af_device_management_cluster_println("  Issuer Event Id: %08X", pmInfo.issuerEventId);

  sl_zigbee_af_device_management_cluster_println("== End of Device Management Information ==");
}

bool sl_zigbee_af_device_management_cluster_publish_change_of_tenancy(sl_802154_short_addr_t dstAddr,
                                                                      uint8_t srcEndpoint,
                                                                      uint8_t dstEndpoint)
{
  sl_status_t status;
  sl_zigbee_af_device_management_tenancy_t *tenancy = &(pmInfo.tenancy);
  sl_zigbee_af_fill_command_device_management_cluster_publish_change_of_tenancy(pmInfo.providerId,
                                                                                pmInfo.issuerEventId,
                                                                                pmInfo.tariffType,
                                                                                tenancy->implementationDateTime,
                                                                                tenancy->tenancy);
  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, dstAddr);

  if (status != SL_STATUS_OK) {
    sl_zigbee_af_device_management_cluster_println("Unable to unicast PublishChangeOfTenancy command: 0x%02X", status);
  }
  return true;
}

bool sl_zigbee_af_device_management_cluster_publish_change_of_supplier(sl_802154_short_addr_t dstAddr,
                                                                       uint8_t srcEndpoint,
                                                                       uint8_t dstEndpoint)
{
  sl_status_t status;
  sl_zigbee_af_device_management_supplier_t *supplier = &(pmInfo.supplier);
  sl_zigbee_af_fill_command_device_management_cluster_publish_change_of_supplier(pmInfo.providerId,
                                                                                 pmInfo.issuerEventId,
                                                                                 pmInfo.tariffType,
                                                                                 supplier->proposedProviderId,
                                                                                 supplier->implementationDateTime,
                                                                                 supplier->providerChangeControl,
                                                                                 supplier->proposedProviderName,
                                                                                 supplier->proposedProviderContactDetails);
  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, dstAddr);

  if (status != SL_STATUS_OK) {
    sl_zigbee_af_device_management_cluster_println("Unable to unicast PublishChangeOfSupplier command: 0x%02X", status);
  }
  return true;
}

bool sl_zigbee_af_device_management_cluster_send_request_new_password_response(uint8_t passwordType,
                                                                               sl_802154_short_addr_t dstAddr,
                                                                               uint8_t srcEndpoint,
                                                                               uint8_t dstEndpoint)
{
  sl_status_t status;
  sl_zigbee_af_device_management_password_t *password;

  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);

  switch (passwordType) {
    case SERVICE_PASSWORD:
      password = &(pmInfo.servicePassword);
      break;
    case CONSUMER_PASSWORD:
      password = &(pmInfo.consumerPassword);
      break;
    default:
      return false;
  }

  // Is the password still valid?
  if ((password->durationInMinutes != 0)
      && (sl_zigbee_af_get_current_time() > password->implementationDateTime + (password->durationInMinutes * 60))) {
    return false;
  } else {
    sl_zigbee_af_fill_command_device_management_cluster_request_new_password_response(pmInfo.issuerEventId,
                                                                                      password->implementationDateTime,
                                                                                      password->durationInMinutes,
                                                                                      password->passwordType,
                                                                                      password->password);

    status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, dstAddr);
    if (status != SL_STATUS_OK) {
      return false;
    } else {
      return true;
    }
  }
}

bool sl_zigbee_af_device_management_cluster_update_site_id(sl_802154_short_addr_t dstAddr,
                                                           uint8_t srcEndpoint,
                                                           uint8_t dstEndpoint)
{
  sl_status_t status;
  sl_zigbee_af_device_management_site_id_t *siteId = &(pmInfo.siteId);
  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);

  sl_zigbee_af_fill_command_device_management_cluster_update_site_id(pmInfo.issuerEventId,
                                                                     siteId->implementationDateTime,
                                                                     pmInfo.providerId,
                                                                     siteId->siteId);
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, dstAddr);

  if (status != SL_STATUS_OK) {
    sl_zigbee_af_device_management_cluster_println("Unable to unicast UpdateSiteId command: 0x%02X", status);
  }
  return true;
}

bool sl_zigbee_af_device_management_cluster_update_cin(sl_802154_short_addr_t dstAddr,
                                                       uint8_t srcEndpoint,
                                                       uint8_t dstEndpoint)
{
  sl_status_t status;
  sl_zigbee_af_device_management_c_i_n_t *cin = &(pmInfo.cin);
  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);

  pmInfo.pendingUpdates |= SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_CIN_PENDING_MASK;

  sl_zigbee_af_fill_command_device_management_cluster_update_cin(pmInfo.issuerEventId,
                                                                 cin->implementationDateTime,
                                                                 pmInfo.providerId,
                                                                 cin->cin);
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, dstAddr);

  if (status != SL_STATUS_OK) {
    sl_zigbee_af_device_management_cluster_println("Unable to unicast UpdateCIN command: 0x%02X", status);
  }
  return true;
}

void sl_zigbee_af_device_management_cluster_set_pending_updates(sl_zigbee_af_device_management_change_pending_flags_t pendingUpdatesMask)
{
  pmInfo.pendingUpdates = pendingUpdatesMask;
}

void sl_zigbee_af_device_management_cluster_get_pending_updates(sl_zigbee_af_device_management_change_pending_flags_t *pendingUpdatesMask)
{
  *(pendingUpdatesMask) = pmInfo.pendingUpdates;
}

// Command callbacks

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_device_management_cluster_get_change_of_tenancy_cb(void)
{
  sl_zigbee_af_device_management_tenancy_t *tenancy = &(pmInfo.tenancy);
  if ((pmInfo.pendingUpdates & SL_ZIGBEE_AF_DEVICE_MANAGEMENT_CHANGE_OF_TENANCY_PENDING_MASK) == 0) {
    return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  }
  sl_zigbee_af_fill_command_device_management_cluster_publish_change_of_tenancy(pmInfo.providerId,
                                                                                pmInfo.issuerEventId,
                                                                                pmInfo.tariffType,
                                                                                tenancy->implementationDateTime,
                                                                                tenancy->tenancy);
  sl_zigbee_af_send_response();

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_device_management_cluster_get_change_of_supplier_cb(void)
{
  sl_zigbee_af_device_management_supplier_t *supplier = &(pmInfo.supplier);

  if ((pmInfo.pendingUpdates & SL_ZIGBEE_AF_DEVICE_MANAGEMENT_CHANGE_OF_SUPPLIER_PENDING_MASK) == 0 ) {
    return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  }
  sl_zigbee_af_fill_command_device_management_cluster_publish_change_of_supplier(pmInfo.providerId,
                                                                                 pmInfo.issuerEventId,
                                                                                 pmInfo.tariffType,
                                                                                 supplier->proposedProviderId,
                                                                                 supplier->implementationDateTime,
                                                                                 supplier->providerChangeControl,
                                                                                 supplier->proposedProviderName,
                                                                                 supplier->proposedProviderContactDetails);
  sl_zigbee_af_send_response();

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_device_management_cluster_get_site_id_cb(void)
{
  sl_zigbee_af_device_management_site_id_t *siteId = &(pmInfo.siteId);

  if ((pmInfo.pendingUpdates & SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_SITE_ID_PENDING_MASK) == 0) {
    return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  }
  sl_zigbee_af_fill_command_device_management_cluster_update_site_id(pmInfo.issuerEventId,
                                                                     siteId->implementationDateTime,
                                                                     pmInfo.providerId,
                                                                     siteId->siteId);
  sl_zigbee_af_send_response();

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_device_management_cluster_get_cin_cb(void)
{
  sl_zigbee_af_device_management_c_i_n_t *cin = &(pmInfo.cin);

  if ((pmInfo.pendingUpdates & SL_ZIGBEE_AF_DEVICE_MANAGEMENT_UPDATE_CIN_PENDING_MASK) == 0) {
    return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  }
  sl_zigbee_af_fill_command_device_management_cluster_update_cin(pmInfo.issuerEventId,
                                                                 cin->implementationDateTime,
                                                                 pmInfo.providerId,
                                                                 cin->cin);
  sl_zigbee_af_send_response();
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_device_management_cluster_request_new_password_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_device_management_cluster_request_new_password_command_t cmd_data;
  sl_zigbee_af_device_management_password_t *password;

  if (zcl_decode_device_management_cluster_request_new_password_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  switch (cmd_data.passwordType) {
    case SERVICE_PASSWORD:
      password = &(pmInfo.servicePassword);
      break;
    case CONSUMER_PASSWORD:
      password = &(pmInfo.consumerPassword);
      break;
    default:
      return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  }

  // Is the password still valid?
  if ((password->durationInMinutes != 0)
      && (sl_zigbee_af_get_current_time() > password->implementationDateTime + (password->durationInMinutes * 60))) {
    return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  }
  sl_zigbee_af_fill_command_device_management_cluster_request_new_password_response(pmInfo.issuerEventId,
                                                                                    password->implementationDateTime,
                                                                                    password->durationInMinutes,
                                                                                    password->passwordType,
                                                                                    password->password);
  sl_zigbee_af_send_response();

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_device_management_cluster_report_event_configuration_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

uint32_t sl_zigbee_af_device_management_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                                     sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_GET_CHANGE_OF_TENANCY_COMMAND_ID:
      {
        status = sl_zigbee_af_device_management_cluster_get_change_of_tenancy_cb();
        break;
      }
      case ZCL_GET_CHANGE_OF_SUPPLIER_COMMAND_ID:
      {
        status = sl_zigbee_af_device_management_cluster_get_change_of_supplier_cb();
        break;
      }
      case ZCL_REQUEST_NEW_PASSWORD_COMMAND_ID:
      {
        status = sl_zigbee_af_device_management_cluster_request_new_password_cb(cmd);
        break;
      }
      case ZCL_GET_SITE_ID_COMMAND_ID:
      {
        status = sl_zigbee_af_device_management_cluster_get_site_id_cb();
        break;
      }
      case ZCL_REPORT_EVENT_CONFIGURATION_COMMAND_ID:
      {
        status = sl_zigbee_af_device_management_cluster_report_event_configuration_cb(cmd);
        break;
      }
      case ZCL_GET_CIN_COMMAND_ID:
      {
        status = sl_zigbee_af_device_management_cluster_get_cin_cb();
        break;
      }
    }
  }

  return status;
}
