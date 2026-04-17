/***************************************************************************//**
 * @file
 * @brief The Time server plugin is responsible for keeping track of the current
 * time.  All endpoints that implement the Time cluster server should use a
 * singleton time attribute.  Sleepy devices should not use this plugin as it
 * will prevent the device from sleeping for longer than one second.
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

#include "../../include/af.h"
#include "../../util/common.h"
#include "time-server.h"

#include "time-server-config.h"
#if (SL_ZIGBEE_AF_PLUGIN_TIME_SERVER_SUPERSEDING == 1)
#define SUPERSEDING
#endif
#if (SL_ZIGBEE_AF_PLUGIN_TIME_SERVER_MASTER == 1)
#define MASTER
#endif
#if (SL_ZIGBEE_AF_PLUGIN_TIME_SERVER_SYNCHRONIZED == 1)
#define SYNCHRONIZED
#endif
#if (SL_ZIGBEE_AF_PLUGIN_TIME_SERVER_MASTER_ZONE_DST == 1)
#define MASTER_ZONE_DST
#endif

#define INVALID_ENDPOINT 0xFF

static sl_zigbee_af_status_t readTime(uint8_t endpoint, uint32_t *time);
static sl_zigbee_af_status_t writeTime(uint8_t endpoint, uint32_t time);

static uint8_t singleton = INVALID_ENDPOINT;

#define sli_zigbee_af_contains_time_server_attribute(endpoint, attribute) \
  sl_zigbee_af_contains_attribute((endpoint), ZCL_TIME_CLUSTER_ID, (attribute), CLUSTER_MASK_SERVER, SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE)

void sl_zigbee_af_time_cluster_server_init_cb(uint8_t endpoint)
{
  sl_zigbee_af_attribute_metadata_t *metadata;
  sl_zigbee_af_status_t status;
  uint32_t currentTime;
  uint8_t timeStatus = 0;

  metadata = sl_zigbee_af_locate_attribute_metadata(endpoint,
                                                    ZCL_TIME_CLUSTER_ID,
                                                    ZCL_TIME_ATTRIBUTE_ID,
                                                    CLUSTER_MASK_SERVER,
                                                    SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
  if (metadata == NULL) {
    return;
  }
  if (sl_zigbee_af_attribute_is_singleton(metadata)) {
    if (singleton != INVALID_ENDPOINT) {
      return;
    }
    singleton = endpoint;
  }

  // Initialize the attribute with the real time, if it's available.
  currentTime = sl_zigbee_af_get_current_time_cb();
  if (currentTime != 0) {
    writeTime(endpoint, currentTime);
  }

#ifdef MASTER
  // The first bit of TimeStatus indicates whether the real time clock
  // corresponding to the Time attribute is internally set to the time
  // standard.
  timeStatus |= BIT(0);
#elif defined(SYNCHRONIZED)
  // The Synchronized bit specifies whether Time has been set over the ZigBee
  // network to synchronize it (as close as may be practical) to the time standard
  // bit must be explicitly written to indicate this - i.e. it is not set
  // automatically on writing to the Time attribute. If the Master bit is 1, the value of
  // this bit is 0.
  timeStatus |= BIT(1);
#endif

#ifdef MASTER_ZONE_DST
  // The third bit of TimeStatus indicates whether the TimeZone, DstStart,
  // DstEnd, and DstShift attributes are set internally to correct values for
  // the location of the clock.
  if (sli_zigbee_af_contains_time_server_attribute(endpoint, ZCL_TIME_ZONE_ATTRIBUTE_ID)
      && sli_zigbee_af_contains_time_server_attribute(endpoint, ZCL_DST_START_ATTRIBUTE_ID)
      && sli_zigbee_af_contains_time_server_attribute(endpoint, ZCL_DST_END_ATTRIBUTE_ID)
      && sli_zigbee_af_contains_time_server_attribute(endpoint, ZCL_DST_SHIFT_ATTRIBUTE_ID)) {
    timeStatus |= BIT(2);
  }
#endif // MASTER_ZONE_DST

#ifdef SUPERSEDING
  // Indicates that the time server should be considered as a more authoritative
  // time server.
  timeStatus |= BIT(3);
#endif // SUPERSEDING

  status = sl_zigbee_af_write_attribute(endpoint,
                                        ZCL_TIME_CLUSTER_ID,
                                        ZCL_TIME_STATUS_ATTRIBUTE_ID,
                                        CLUSTER_MASK_SERVER,
                                        (uint8_t *)&timeStatus,
                                        ZCL_BITMAP8_ATTRIBUTE_TYPE);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_time_cluster_println("ERR: writing time status %02X", status);
  }

  // Ticks are scheduled for all endpoints that do not have a singleton time
  // attribute and for one of the endpoints with a singleton attribute.
  sl_zigbee_zcl_schedule_server_tick(endpoint,
                                     ZCL_TIME_CLUSTER_ID,
                                     MILLISECOND_TICKS_PER_SECOND);
}

void sl_zigbee_af_time_cluster_server_tick_cb(uint8_t endpoint)
{
  // Update the attribute with the real time if we have it; otherwise, just
  // increment the current the value.
  uint32_t currentTime = sl_zigbee_af_get_current_time_cb();
  if (currentTime == 0) {
    readTime(endpoint, &currentTime);
    currentTime++;
  }
  writeTime(endpoint, currentTime);

  // Reschedule the tick callback.
  sl_zigbee_zcl_schedule_server_tick(endpoint,
                                     ZCL_TIME_CLUSTER_ID,
                                     MILLISECOND_TICKS_PER_SECOND);
}

sl_zigbee_af_status_t sl_zigbee_af_time_cluster_server_pre_attribute_changed_cb(
  uint8_t endpoint,
  sl_zigbee_af_attribute_id_t attributeId,
  sl_zigbee_af_attribute_type_t attributeType,
  uint8_t size,
  uint8_t *value)
{
  (void)attributeType;
  (void)size;
  (void)value;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;

  UNUSED_VAR(attributeType);
  UNUSED_VAR(size);
  UNUSED_VAR(value);

  // Only apply guard to network commands, not internal updates.
  if (sl_zigbee_af_current_command() == NULL) {
    return status;
  }

  // Only allow Time to be written if not master, per ZCL spec.
  uint8_t timeStatus = 0;
  status = sl_zigbee_af_read_attribute(endpoint,
                                       ZCL_TIME_CLUSTER_ID,
                                       ZCL_TIME_STATUS_ATTRIBUTE_ID,
                                       CLUSTER_MASK_SERVER,
                                       (uint8_t *)&timeStatus,
                                       sizeof(timeStatus),
                                       NULL);  // data type

  switch (attributeId) {
    case ZCL_TIME_ATTRIBUTE_ID:
    {
      if (SL_ZIGBEE_ZCL_STATUS_SUCCESS == status) {
        if (timeStatus & BIT(0)) {
          // Master bit is set in TimeStatus, disallow write.
          status = SL_ZIGBEE_ZCL_STATUS_READ_ONLY;
        }
      }
    }
    break;

    // Only allow Time zone, DST start, DST end, DST shift to be written if not master, per ZCL spec.
    case ZCL_TIME_ZONE_ATTRIBUTE_ID:
    case ZCL_DST_START_ATTRIBUTE_ID:
    case ZCL_DST_END_ATTRIBUTE_ID:
    case ZCL_DST_SHIFT_ATTRIBUTE_ID:
    {
      if (SL_ZIGBEE_ZCL_STATUS_SUCCESS == status) {
        if (timeStatus & BIT(2)) {
          // MasterZoneDst bit is set in TimeStatus, disallow write.
          status = SL_ZIGBEE_ZCL_STATUS_READ_ONLY;
        }
      }
    }
    break;

    default:
      break;
  }

  return status;
}

uint32_t sli_zigbee_af_time_cluster_server_get_current_time(void)
{
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  uint32_t currentTime = sl_zigbee_af_get_current_time_cb();

  // If we don't have the current time, we have to try to get it from an
  // endpoint by rolling through all of them until one returns a time.
  if (currentTime == 0) {
    uint8_t i;
    for (i = 0; i < sl_zigbee_af_endpoint_count(); i++) {
      uint8_t endpoint = sl_zigbee_af_endpoint_from_index(i);
      if (sli_zigbee_af_contains_time_server_attribute(endpoint, ZCL_TIME_ATTRIBUTE_ID)) {
        status = readTime(endpoint, &currentTime);
        if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
          break;
        }
      }
    }
  }

  return (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS ? currentTime : 0);
}

void sli_zigbee_af_time_cluster_server_set_current_time(uint32_t utcTime)
{
  // Set the time on all endpoints that do not have a singleton time attribute
  // as well as on one of the endpoints with a singleton attribute.
  uint8_t i;
  for (i = 0; i < sl_zigbee_af_endpoint_count(); i++) {
    uint8_t endpoint = sl_zigbee_af_endpoint_from_index(i);
    if (sli_zigbee_af_contains_time_server_attribute(endpoint, ZCL_TIME_ATTRIBUTE_ID)) {
      sl_zigbee_af_attribute_metadata_t *metadata = sl_zigbee_af_locate_attribute_metadata(endpoint,
                                                                                           ZCL_TIME_CLUSTER_ID,
                                                                                           ZCL_TIME_ATTRIBUTE_ID,
                                                                                           CLUSTER_MASK_SERVER,
                                                                                           SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
      if (metadata == NULL) {
        return;
      }
      if (!sl_zigbee_af_attribute_is_singleton(metadata) || singleton == endpoint) {
        writeTime(endpoint, utcTime);
      }
    }
  }
}

static sl_zigbee_af_status_t readTime(uint8_t endpoint, uint32_t *time)
{
  sl_zigbee_af_status_t status = sl_zigbee_af_read_attribute(endpoint,
                                                             ZCL_TIME_CLUSTER_ID,
                                                             ZCL_TIME_ATTRIBUTE_ID,
                                                             CLUSTER_MASK_SERVER,
                                                             (uint8_t *)time,
                                                             sizeof(*time),
                                                             NULL); // data type
#if ((defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_TIME_CLUSTER)) || defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT))
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_time_cluster_println("ERR: reading time %02X", status);
  }
#endif // ((defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_TIME_CLUSTER)) || defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT))
  return status;
}

static sl_zigbee_af_status_t writeTime(uint8_t endpoint, uint32_t time)
{
  sl_zigbee_af_status_t status = sl_zigbee_af_write_attribute(endpoint,
                                                              ZCL_TIME_CLUSTER_ID,
                                                              ZCL_TIME_ATTRIBUTE_ID,
                                                              CLUSTER_MASK_SERVER,
                                                              (uint8_t *)&time,
                                                              ZCL_UTC_TIME_ATTRIBUTE_TYPE);
#if ((defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_TIME_CLUSTER)) || defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT))
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_time_cluster_println("ERR: writing time %02X", status);
  }
#endif // ((defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_TIME_CLUSTER)) || defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT))
  return status;
}
