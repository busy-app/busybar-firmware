/***************************************************************************//**
 * @file
 * @brief Test code for the Simple Metering Server plugin.
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

#include "../include/af.h"
#include "../../util/common.h"
#include "../../util/af-main.h"
#include "../../util/client-api.h"
#include "simple-metering-server.h"
#include "simple-metering-test.h"

#ifdef TEST_METER_ENABLE

// Test mode:
//   bit 0: 0 - electric, 1 - gas
//   bit 1: reserved for future use
//   bit 2: 0 - no profiles, 1 - profiles
//   bit 3: 0 - no tick, 1 = tick
uint8_t testMode = 0;
uint8_t errorChance = 0;
uint16_t meterConsumptionRate = 0;
uint16_t meterConsumptionVariance = 0;
uint16_t batteryRate = 1;
uint32_t nextGetProfileInterval;
static uint32_t hourCounterTable[SL_ZIGBEE_ZCL_SIMPLE_METERING_CLUSTER_SERVER_ENDPOINT_COUNT];

#define sli_zigbee_af_contains_simple_meter_server_attribute(endpoint, attribute) \
  sl_zigbee_af_contains_attribute((endpoint), ZCL_SIMPLE_METERING_CLUSTER_ID, (attribute), CLUSTER_MASK_SERVER, SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE)

static void hourCounterTableInit(void);

static void hourCounterTableInit(void)
{
  uint8_t i;

  for (i = 0; i < SL_ZIGBEE_ZCL_SIMPLE_METERING_CLUSTER_SERVER_ENDPOINT_COUNT; i++) {
    hourCounterTable[i] = 0;
  }
}

static void addToByteArray(uint8_t *data,
                           uint8_t len,
                           uint32_t toAdd,
                           bool lowHigh)
{
  uint16_t sum = 0;
  int8_t loc, end, incr;
  if (lowHigh) {
    loc  = 0;
    end  = len;
    incr = 1;
  } else {
    loc  = len - 1;
    end  = -1;
    incr = -1;
  }

  while ( loc != end ) {
    uint8_t t, s;
    t = data[loc];
    s = toAdd & 0xff;
    sum += t + s;
    data[loc] = sum & 0xff;
    sum >>= 8;
    toAdd >>= 8;
    loc += incr;
  }
}

static void addToByteArrayLowHigh(uint8_t *data,
                                  uint8_t len,
                                  uint32_t toAdd)
{
  addToByteArray(data, len, toAdd, true);
}

#if BIGENDIAN_CPU
static void addToByteArrayHighLow(uint8_t *data,
                                  uint8_t len,
                                  uint32_t toAdd)
{
  addToByteArray(data, len, toAdd, false);
}
#endif // BIGENDIAN_CPU

void afTestMeterPrint(void)
{
  sl_zigbee_af_simple_metering_cluster_println("TM:\r\n"
                                               "mode:%02X\r\n"
                                               "meterConsumptionRate:%04X\r\n"
                                               "meterConsumptionVariance:%04X\r\n",
                                               testMode,
                                               meterConsumptionRate,
                                               meterConsumptionVariance);
}

void afTestMeterSetConsumptionRate(uint16_t rate, uint8_t endpoint)
{
  UNUSED_VAR(endpoint);
  sl_zigbee_af_simple_metering_cluster_println("TM: set consumption rate %04X", rate);
  meterConsumptionRate = rate;
}

void afTestMeterSetConsumptionVariance(uint16_t variance)
{
  sl_zigbee_af_simple_metering_cluster_println("TM: set consumption variance %04X", variance);
  meterConsumptionVariance = variance;
}

void afTestMeterAdjust(uint8_t endpoint)
{
  uint8_t summation[] = { 0, 0, 0, 0, 0, 0 };

  // Seconds in day
  uint32_t ct = sl_zigbee_af_get_current_time() % 86400;
  uint32_t diff;

  sl_zigbee_af_simple_metering_cluster_println("TM: adjust");

  diff = (uint32_t)meterConsumptionRate * ct;

  if ( meterConsumptionVariance > 0 ) {
    diff += ct * (uint32_t)(sl_zigbee_get_pseudo_random_number() % meterConsumptionVariance);
  }

 #if BIGENDIAN_CPU
  addToByteArrayHighLow(summation, 6, diff);
#else   // !BIGENDIAN_CPU
  addToByteArrayLowHigh(summation, 6, diff);
#endif  // BIGENDIAN_CPU

  sl_zigbee_af_simple_metering_cluster_println("Summation:%02X %02X %02X %02X %02X %02X",
                                               summation[0],
                                               summation[1],
                                               summation[2],
                                               summation[3],
                                               summation[4],
                                               summation[5]);

  (void) sl_zigbee_af_write_attribute(endpoint,
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER,
                                      summation,
                                      ZCL_INT48U_ATTRIBUTE_TYPE);
}

// 0 off, 1 if electric, 2 if gas
void afTestMeterMode(uint8_t endpoint, uint8_t mode)
{
  uint8_t status = 0;
  uint8_t unitOfMeasure;
  uint8_t deviceType;
  uint8_t summationFormatting = 0x2C; // 00101100

  if ( mode == 0 ) {
    testMode &= (~0x08);
  } else if ( mode == 1 ) {
    testMode &= (~0x01);
    testMode |= 0x08;
    unitOfMeasure = SL_ZIGBEE_ZCL_AMI_UNIT_OF_MEASURE_KILO_WATT_HOURS;
    deviceType = SL_ZIGBEE_ZCL_METERING_DEVICE_TYPE_ELECTRIC_METERING;
  } else if ( mode == 2 ) {
    testMode |= (0x01 | 0x08);
    unitOfMeasure = SL_ZIGBEE_ZCL_AMI_UNIT_OF_MEASURE_BT_US_OR_BTU_PER_HOUR;
    deviceType = SL_ZIGBEE_ZCL_METERING_DEVICE_TYPE_GAS_METERING;
  }
  sl_zigbee_af_simple_metering_cluster_println("TM: mode %02X", testMode);

  // Set attributes...

  // Status = OK
  (void) sl_zigbee_af_write_attribute(endpoint,
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_STATUS_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER,
                                      &status,
                                      ZCL_BITMAP8_ATTRIBUTE_TYPE);

  // Device type is either gas or electric
  (void) sl_zigbee_af_write_attribute(endpoint,
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_METERING_DEVICE_TYPE_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER,
                                      &deviceType,
                                      ZCL_BITMAP8_ATTRIBUTE_TYPE);

  // Unit of measure is either KWH or BTU
  (void) sl_zigbee_af_write_attribute(endpoint,
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_UNIT_OF_MEASURE_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER,
                                      &unitOfMeasure,
                                      ZCL_ENUM8_ATTRIBUTE_TYPE);

  // Summation formatting is 0x2C
  (void) sl_zigbee_af_write_attribute(endpoint,
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_SUMMATION_FORMATTING_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER,
                                      &summationFormatting,
                                      ZCL_BITMAP8_ATTRIBUTE_TYPE);
}

#ifdef TEST_METER_ERRORS
void afTestMeterSetError(uint8_t endpoint,
                         uint8_t error)
{
  sl_zigbee_af_simple_metering_cluster_println("TM: set error %02X", error);
  (void) sl_zigbee_af_write_attribute(endpoint,
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_STATUS_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER,
                                      &error,
                                      ZCL_BITMAP8_ATTRIBUTE_TYPE);
}

void afTestMeterRandomError(uint8_t chanceIn256)
{
  sl_zigbee_af_simple_metering_cluster_println("TM: random error %02X", chanceIn256);
  errorChance = chanceIn256;
}
#endif // TEST_METER_ERRORS

#if (SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0)

uint8_t testMeterProfiles[SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES][3];
uint32_t testMeterIntervalEndTimes[SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES];
void afTestMeterEnableProfiles(uint8_t enable)
{
  uint8_t i;
  sl_zigbee_af_simple_metering_cluster_println("TM: profiles %02X", enable);
  switch (enable) {
    case 0:
      testMode &= (~0x04);
      break;
    case 1:
      testMode |= 0x04;
      break;
    case 2:
      for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES; i++ ) {
        testMeterProfiles[i][0]
          = testMeterProfiles[i][1]
              = testMeterProfiles[i][2]
                  = 0x00;
      }
    // fallthrough
    case 3:
      for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES; i++ ) {
        sl_zigbee_af_simple_metering_cluster_println("P %02X: %02X%02X%02X",
                                                     i,
                                                     testMeterProfiles[i][0],
                                                     testMeterProfiles[i][1],
                                                     testMeterProfiles[i][2]);
      }
      break;
  }
}

#endif // SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0

void sli_zigbee_af_test_meter_init(uint8_t endpoint)
{
  memset(testMeterProfiles,
         0,
         (SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES * 3));
  memset(testMeterIntervalEndTimes,
         0,
         (SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES) * 4);
  // battery life remaining (0x0201), begin at 255, and decrement every minute. INT8U.
  if ( sli_zigbee_af_contains_simple_meter_server_attribute(endpoint,
                                                            ZCL_REMAINING_BATTERY_LIFE_ATTRIBUTE_ID) ) {
    uint8_t batteryLife = 100; // 100% to begin. 0xff is reserved.
    (void) sl_zigbee_af_write_attribute(endpoint,
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        ZCL_REMAINING_BATTERY_LIFE_ATTRIBUTE_ID,
                                        CLUSTER_MASK_SERVER,
                                        &batteryLife,
                                        ZCL_INT8U_ATTRIBUTE_TYPE);
  }

  hourCounterTableInit();

#ifdef SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_BATTERY_RATE
#if (SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_BATTERY_RATE != 0)
  batteryRate = SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_BATTERY_RATE;
#else
  batteryRate = 1;
#endif
#endif
  if (batteryRate > 100) {
    batteryRate = 100;
  }

  #if (SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0)
  nextGetProfileInterval = PROFILE_INTERVAL_PERIOD_IN_SECONDS;
  #endif
}

// Note: In the final implementation, according to the SE spec following must
//  happen:
//   - ReadingSnapShotTime attribute must be set to the actual UTC time of
//        when the CurrentSummationDelivered, CurrentSummationReceived,
//        CurrentMaxDemandDelievered and CurrentMaxDemandReceived were measured.
//   - CurrentMaxDemandDeliveredTime must be set to UTC time of when the
//        CurrentMaxDelivered was read
//   - CurrentMaxDemandReceivedTime must be set to UTC time of when the
//        CurrentMaxDemandReceived was read
void sli_zigbee_af_test_meter_tick(uint8_t endpoint)
{
  uint32_t *hourCounter;
  uint32_t diff, currentTime;
  uint8_t status, dataType;
  uint8_t summation[] = { 0, 0, 0, 0, 0, 0 };

#if (SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0)
  uint8_t intervalSummation[] = { 0, 0, 0 };
#endif

  // random counters for keeping track of minutes/seconds locally
  // should probably use local time but I'm worried what happens when
  // that changes through the CLI, so I'm doing this for now.
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint,
                                                               ZCL_SIMPLE_METERING_CLUSTER_ID);
  if (ep == 0xFF) {
    sl_zigbee_af_simple_metering_cluster_println("Invalid endpoint %02X", endpoint);
    return;
  }

  hourCounter = &hourCounterTable[ep];
  //static uint16_t batteryLifeLastUpdateTime = 0; // would be used for local time
  //static uint16_t hoursInOperationLastUpdateTime = 0;

#ifdef TEST_METER_ERRORS
  uint8_t meterStatus;
#endif // TEST_METER_ERRORS

  // Now let's adjust the summation
  status = sl_zigbee_af_read_attribute(endpoint,
                                       ZCL_SIMPLE_METERING_CLUSTER_ID,
                                       ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID,
                                       CLUSTER_MASK_SERVER,
                                       summation,
                                       6,
                                       &dataType);

  if ( status != SL_ZIGBEE_ZCL_STATUS_SUCCESS ) {
    sl_zigbee_af_simple_metering_cluster_println("ERR: can't read summation status 0x%02X", status);
    return;
  }

  diff = (uint32_t)meterConsumptionRate;
  if ( meterConsumptionVariance > 0 ) {
    diff += (uint32_t)(sl_zigbee_get_pseudo_random_number() % meterConsumptionVariance);
  }

#if BIGENDIAN_CPU
  addToByteArrayHighLow(summation, 6, diff);
#else // !BIGENDIAN_CPU
  addToByteArrayLowHigh(summation, 6, diff);
#endif  // BIGENDIAN_CPU

  if ((*hourCounter % 60) == 0) {
    // sl_zigbee_af_simple_metering_cluster_println("TM summation:%02X %02X %02X %02X %02X %02X",
    //                                     summation[0],
    //                                     summation[1],
    //                                     summation[2],
    //                                     summation[3],
    //                                     summation[4],
    //                                     summation[5]);
  }

  (void) sl_zigbee_af_write_attribute(endpoint,
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER,
                                      summation,
                                      ZCL_INT48U_ATTRIBUTE_TYPE);

#ifdef TEST_METER_ERRORS
  if ( errorChance > 0 ) {
    if ( (sl_zigbee_get_pseudo_random_number() % 256) < errorChance ) {
      (void)sl_zigbee_af_read_attribute(endpoint,
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        ZCL_STATUS_ATTRIBUTE_ID,
                                        CLUSTER_MASK_SERVER,
                                        &meterStatus,
                                        1,
                                        &dataType);
      if ( meterStatus == 0 ) {
        sl_zigbee_af_simple_metering_cluster_println("TM: random error set");
        meterStatus = 1;
        (void) sl_zigbee_af_write_attribute(endpoint,
                                            ZCL_SIMPLE_METERING_CLUSTER_ID,
                                            ZCL_STATUS_ATTRIBUTE_ID,
                                            CLUSTER_MASK_SERVER,
                                            &meterStatus,
                                            ZCL_BITMAP8_ATTRIBUTE_TYPE);
      }
    }
  }
#endif // TEST_METER_ERRORS
#if (SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0)
  if ( testMode & 0x04 ) { // Profiles are enabled
    currentTime = sl_zigbee_af_get_current_time();
    if (currentTime >= nextGetProfileInterval) {
      sl_zigbee_af_simple_metering_cluster_println("Current Time %u", currentTime);

      //sl_zigbee_af_simple_metering_cluster_println("currentTime %u nextGetProfileInterval %u",currentTime,nextGetProfileInterval);
      status = sl_zigbee_af_read_attribute(endpoint,
                                           ZCL_SIMPLE_METERING_CLUSTER_ID,
                                           ZCL_CURRENT_PARTIAL_PROFILE_INTERVAL_VALUE_DELIVERED_ATTRIBUTE_ID,
                                           CLUSTER_MASK_SERVER,
                                           intervalSummation,
                                           3,
                                           &dataType);
      if ( status != SL_ZIGBEE_ZCL_STATUS_SUCCESS ) {
        sl_zigbee_af_simple_metering_cluster_println("ERR: can't read interval summation status 0x%02X", status);
        return;
      }

#if BIGENDIAN_CPU
      addToByteArrayHighLow(intervalSummation, 3, diff);
#else // !BIGENDIAN_CPU
      addToByteArrayLowHigh(intervalSummation, 3, diff);
#endif  // BIGENDIAN_CPU

      sl_zigbee_af_simple_metering_cluster_println("diff %u", diff);

      (void) sl_zigbee_af_write_attribute(endpoint,
                                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                                          ZCL_CURRENT_PARTIAL_PROFILE_INTERVAL_VALUE_DELIVERED_ATTRIBUTE_ID,
                                          CLUSTER_MASK_SERVER,
                                          intervalSummation,
                                          ZCL_INT24U_ATTRIBUTE_TYPE);

      // Profile swap...
      //sl_zigbee_af_simple_metering_cluster_println("TM: swapping profile");

      for ( status = SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES - 1;
            status > 0;
            status-- ) {
        //sl_zigbee_af_simple_metering_cluster_println("Copying from %u:%02X%02X%02X to %u:%02X%02X%02X", status, testMeterProfiles[status][0],testMeterProfiles[status][1],testMeterProfiles[status][2],status-1,testMeterProfiles[status-1][0],testMeterProfiles[status-1][1],testMeterProfiles[status-1][2]);
        memcpy(testMeterProfiles[status],
               testMeterProfiles[status - 1],
               3);
        memcpy(&testMeterIntervalEndTimes[status],
               &testMeterIntervalEndTimes[status - 1],
               4);
      }
      memcpy(testMeterProfiles[0],
             intervalSummation,
             3);
      testMeterIntervalEndTimes[0] = nextGetProfileInterval;

      for ( status = 0;
            status < SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES;
            status++ ) {
        sl_zigbee_af_simple_metering_cluster_println("i %u profile %02X%02X%02X time %u", status, testMeterProfiles[status][0], testMeterProfiles[status][1], testMeterProfiles[status][2], testMeterIntervalEndTimes[status]);
      }
      //sl_zigbee_af_simple_metering_cluster_println("Copying Interval Summation %02X%02X%02X",testMeterProfiles[0][0],testMeterProfiles[0][1],testMeterProfiles[0][2]);
      intervalSummation[0]
        = intervalSummation[1]
            = intervalSummation[2] = 0;
      nextGetProfileInterval += PROFILE_INTERVAL_PERIOD_IN_SECONDS;
      // Reset summation
      (void) sl_zigbee_af_write_attribute(endpoint,
                                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                                          ZCL_CURRENT_PARTIAL_PROFILE_INTERVAL_VALUE_DELIVERED_ATTRIBUTE_ID,
                                          CLUSTER_MASK_SERVER,
                                          intervalSummation,
                                          ZCL_INT24U_ATTRIBUTE_TYPE);

      // Set interval time
      currentTime = sl_zigbee_af_get_current_time();
      //sl_zigbee_af_simple_metering_cluster_println("WRITING CURRENT TIME %u",currentTime);
      (void) sl_zigbee_af_write_attribute(endpoint,
                                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                                          ZCL_CURRENT_PARTIAL_PROFILE_INTERVAL_START_TIME_DELIVERED_ATTRIBUTE_ID,
                                          CLUSTER_MASK_SERVER,
                                          (uint8_t*)&nextGetProfileInterval,
                                          ZCL_UTC_TIME_ATTRIBUTE_TYPE);

      for ( status = 0;
            status < SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES;
            status++ ) {
        // sl_zigbee_af_simple_metering_cluster_println("TM: Pr %02X: %02X%02X%02X",
        //  status,
        //  testMeterProfiles[status][0],
        //  testMeterProfiles[status][1],
        //  testMeterProfiles[status][2]);
      }
    } // 2nd if
  } // testMode & 0x04
#endif // (SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0)

  // adjust the optional attributes, if they were selected in the cluster configuration window
  // current tier 1 summation delivered (0x0100), increment with same values from current summation
  // delivered. Type is INT48U so same as current samation delivered.
  if ( sli_zigbee_af_contains_simple_meter_server_attribute(endpoint,
                                                            ZCL_CURRENT_TIER1_SUMMATION_DELIVERED_ATTRIBUTE_ID) ) {
    (void) sl_zigbee_af_write_attribute(endpoint,
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        ZCL_CURRENT_TIER1_SUMMATION_DELIVERED_ATTRIBUTE_ID,
                                        CLUSTER_MASK_SERVER,
                                        summation,
                                        ZCL_INT48U_ATTRIBUTE_TYPE);

    if ((*hourCounter % 60) == 0) {
      //sl_zigbee_af_simple_metering_cluster_println("TM tier1 updated too");
    }
  }

  // battery life remaining (0x0201), begin at 255, and decrement every minute. INT8U.
  if ( sli_zigbee_af_contains_simple_meter_server_attribute(endpoint,
                                                            ZCL_REMAINING_BATTERY_LIFE_ATTRIBUTE_ID) ) {
    if ((*hourCounter) && ((*hourCounter % (60 * batteryRate)) == 0) ) { // every minute
      uint8_t batteryLife;
      status = sl_zigbee_af_read_attribute(endpoint,
                                           ZCL_SIMPLE_METERING_CLUSTER_ID,
                                           ZCL_REMAINING_BATTERY_LIFE_ATTRIBUTE_ID,
                                           CLUSTER_MASK_SERVER,
                                           &batteryLife,
                                           1,
                                           &dataType);
      if ( status != SL_ZIGBEE_ZCL_STATUS_SUCCESS ) {
        sl_zigbee_af_simple_metering_cluster_println("ERR: can't read battery life");
        return;
      }
      if (batteryLife) {
        batteryLife--; // decrement every minute. stop at zero.
      }
      (void) sl_zigbee_af_write_attribute(endpoint,
                                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                                          ZCL_REMAINING_BATTERY_LIFE_ATTRIBUTE_ID,
                                          CLUSTER_MASK_SERVER,
                                          &batteryLife,
                                          ZCL_INT8U_ATTRIBUTE_TYPE);
      sl_zigbee_af_simple_metering_cluster_println("TM battery life: %02X",
                                                   batteryLife);
    } // end if hourCounter is at minute else do nothing
  } // end if contains attribute battery life

  // hours in operation (0x0202), increment every 60 minutes. INT24U.
  if ( sli_zigbee_af_contains_simple_meter_server_attribute(endpoint,
                                                            ZCL_HOURS_IN_OPERATION_ATTRIBUTE_ID) ) {
    if ((*hourCounter) && ((*hourCounter % 3600) == 0)) {  // every hour, but skip 0
      uint8_t hoursInOperation[] = { 0, 0, 0 };
      status = sl_zigbee_af_read_attribute(endpoint,
                                           ZCL_SIMPLE_METERING_CLUSTER_ID,
                                           ZCL_HOURS_IN_OPERATION_ATTRIBUTE_ID,
                                           CLUSTER_MASK_SERVER,
                                           hoursInOperation,
                                           3,
                                           &dataType);
      if ( status != SL_ZIGBEE_ZCL_STATUS_SUCCESS ) {
        sl_zigbee_af_simple_metering_cluster_println("ERR: can't read hours in operation");
        return;
      }
      // increment every hour. no clue what happens on overflow
#if BIGENDIAN_CPU
      addToByteArrayHighLow(hoursInOperation, 3, 1);
#else // !BIGENDIAN_CPU
      addToByteArrayLowHigh(hoursInOperation, 3, 1);
#endif  // BIGENDIAN_CPU
      (void) sl_zigbee_af_write_attribute(endpoint,
                                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                                          ZCL_HOURS_IN_OPERATION_ATTRIBUTE_ID,
                                          CLUSTER_MASK_SERVER,
                                          hoursInOperation,
                                          ZCL_INT24U_ATTRIBUTE_TYPE);
      sl_zigbee_af_simple_metering_cluster_println("TM hours in operation:%02X %02X %02X",
                                                   hoursInOperation[0],
                                                   hoursInOperation[1],
                                                   hoursInOperation[2]);
    } // end if hourCounter is at hour else do nothing
  } // end if contains attribute hours in operation

  // instantaneous demand (0x0400), increment with same values from current summation
  // delivered, namely the difference from the last second (the rate +/- the variance
  // applied this current second. this is a INT24S.
  if ( sli_zigbee_af_contains_simple_meter_server_attribute(endpoint,
                                                            ZCL_INSTANTANEOUS_DEMAND_ATTRIBUTE_ID) ) {
    // how do you do signed intergers? I think ZCL uses two's complement, but
    // I can't find this in the document anywhere. In this implementation,
    // the demand will always be positive, so note that this code will not
    // support a negative demand as it is.
    uint8_t instantaneousDemand[] = { 0, 0, 0 };
 #if BIGENDIAN_CPU
    addToByteArrayHighLow(instantaneousDemand, 3, diff);
 #else  // !BIGENDIAN_CPU
    addToByteArrayLowHigh(instantaneousDemand, 3, diff);
#endif  // BIGENDIAN_CPU
    // uncomment this to test. it's too noisy to do each time and
    // not useful to do once in a while, since it's random.
    //sl_zigbee_af_simple_metering_cluster_println("TM instantaneous demand:%02X %02X %02X",
    //                                    instantaneousDemand[0],
    //                                    instantaneousDemand[1],
    //                                    instantaneousDemand[2]);
    (void) sl_zigbee_af_write_attribute(endpoint,
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        ZCL_INSTANTANEOUS_DEMAND_ATTRIBUTE_ID,
                                        CLUSTER_MASK_SERVER,
                                        instantaneousDemand,
                                        ZCL_INT24S_ATTRIBUTE_TYPE);
  }
  // need to do this whether or not hours in operation is selected, so
  // battery life counter still works properly
  (*hourCounter)++; // this function called every second
}

sl_zigbee_af_zcl_request_status_t sli_zigbee_af_test_meter_get_profiles(uint8_t intervalChannel,
                                                                        uint32_t endTime,
                                                                        uint8_t numberOfPeriods)
{
#if (SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0)
  // Get the current time
  uint32_t intervalEndTime;
  uint32_t ct;
  uint8_t start = 0, stop, dt, profilesReturned;
  uint8_t profileIntervalPeriod = PROFILE_INTERVAL_PERIOD_TIMEFRAME;
  uint8_t ep = sl_zigbee_af_current_command()->apsFrame->destinationEndpoint;
  sl_zigbee_af_status_t status = sl_zigbee_af_read_attribute(ep,
                                                             ZCL_SIMPLE_METERING_CLUSTER_ID,
                                                             ZCL_CURRENT_PARTIAL_PROFILE_INTERVAL_START_TIME_DELIVERED_ATTRIBUTE_ID,
                                                             CLUSTER_MASK_SERVER,
                                                             (uint8_t*)&ct,
                                                             4,
                                                             &dt);
  sl_zigbee_af_ami_get_profile_status_t returnStatus;
  // If we can read the last profile time, AND either the endTime requested is 0 OR it is greater than
  // the oldest read time, we can handle this - otherwise we return failure 0x05.
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    uint32_t lastValidInterval = (ct / PROFILE_INTERVAL_PERIOD_IN_SECONDS) * PROFILE_INTERVAL_PERIOD_IN_SECONDS;
    uint32_t lastEndTime = (lastValidInterval >= (PROFILE_INTERVAL_PERIOD_IN_SECONDS * SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES)
                            ? lastValidInterval - (PROFILE_INTERVAL_PERIOD_IN_SECONDS * SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES)
                            : PROFILE_INTERVAL_PERIOD_IN_SECONDS);
    sl_zigbee_af_simple_metering_cluster_println("lastValidInterval %u lastEndTime %u", lastValidInterval, lastEndTime);
    if (endTime >= lastEndTime || endTime == 0) {
      uint8_t i;
      for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES; i++) {
        if (endTime >= testMeterIntervalEndTimes[i]) {
          start = i;
          break;
        }
      }
      // start = ((endTime > 0 && endTime >lastEndTime ) ? SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES - ((endTime - lastEndTime) / PROFILE_INTERVAL_PERIOD_IN_SECONDS) -1 : 0);

      if (endTime == 0) {
        start = 0;
      }
      //If the end time is far ahead of the lastEndTime interval, we want to start at the latest interval.
      if (start > SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES) {
        start = 0;
      }

      sl_zigbee_af_simple_metering_cluster_println("ct-endtime%u, start%u ", (ct - endTime), start);

      //stop = (((MAX_PROFILE_INDEX - start) > numberOfPeriods) ? (start + numberOfPeriods - 1) : MAX_PROFILE_INDEX);
      if (lastValidInterval < PROFILE_INTERVAL_PERIOD_IN_SECONDS && start == 0 && endTime > 0) {
        stop = start;
      } else {
        stop = (lastValidInterval / PROFILE_INTERVAL_PERIOD_IN_SECONDS) - 1;
      }

      if (stop > SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES) {
        stop = SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES;
      }

      sl_zigbee_af_simple_metering_cluster_println("end Time 0x%08X, lastValidInterval 0x%08X, start %u", endTime, lastValidInterval, start);
      profilesReturned = (stop - start);

      if (profilesReturned > numberOfPeriods) {
        profilesReturned = numberOfPeriods;
      }

      //DEBUG
      intervalEndTime = testMeterIntervalEndTimes[start];
      sl_zigbee_af_simple_metering_cluster_println("start: %02X, stop: %02X, preq: %02X, pret, %02X", start, stop, numberOfPeriods, profilesReturned);
      sl_zigbee_af_simple_metering_cluster_flush();

      //Set the return Status
      returnStatus = SL_ZIGBEE_ZCL_AMI_GET_PROFILE_STATUS_SUCCESS;

      if (intervalChannel > SL_ZIGBEE_ZCL_AMI_INTERVAL_CHANNEL_CONSUMPTION_RECEIVED) {
        returnStatus = SL_ZIGBEE_ZCL_AMI_GET_PROFILE_STATUS_UNDEFINED_INTERVAL_CHANNEL_REQUESTED;
      }

      if (profilesReturned == 0) {
        returnStatus = SL_ZIGBEE_ZCL_AMI_GET_PROFILE_STATUS_NO_INTERVALS_AVAILABLE_FOR_THE_REQUESTED_TIME;
        profileIntervalPeriod = 0;
        intervalEndTime = 0;
      }

      sl_zigbee_af_fill_command_simple_metering_cluster_get_profile_response(intervalEndTime,
                                                                             returnStatus,
                                                                             profileIntervalPeriod,
                                                                             profilesReturned,
                                                                             appResponseData,
                                                                             profilesReturned * 3);
      appResponseData[1] = sl_zigbee_af_incoming_zcl_sequence_number;

      for (i = 0; i < profilesReturned; i++) {
        uint32_t data = (0x00 << 24) | (testMeterProfiles[start + i][0] << 16) | (testMeterProfiles[start + i][1] << 8) | (testMeterProfiles[start + i][2]);
        sl_zigbee_af_copy_int24u(appResponseData, 10 + (i * 3), data);
        sl_zigbee_af_simple_metering_cluster_println("i %u profile %02X%02X%02X time %u", i, testMeterProfiles[start + i][0], testMeterProfiles[start + i][1], testMeterProfiles[start + i][2], testMeterIntervalEndTimes[start + i]);
      }
      appResponseLength = 10 + (3 * profilesReturned);

      sl_zigbee_af_send_response();

      sl_zigbee_af_simple_metering_cluster_println("get profile: 0x%02X, 0x%08X, 0x%02X",
                                                   intervalChannel,
                                                   endTime,
                                                   numberOfPeriods);

      return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
    }
  }
#endif //(SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0)
  //Otherwise we send back a failure

  sl_zigbee_af_fill_command_simple_metering_cluster_get_profile_response(0x00000000,
                                                                         0x05,
                                                                         0,
                                                                         0,
                                                                         appResponseData,
                                                                         0);
  appResponseData[1] = sl_zigbee_af_incoming_zcl_sequence_number;
  appResponseLength = 10;
  sl_zigbee_af_send_response();
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

#else

//TODO: Clean this else block while removing "#ifndef UC_BUILD"
void sli_zigbee_af_test_meter_init(uint8_t endpoint)

{
}
void sli_zigbee_af_test_meter_tick(uint8_t endpoint)
{
}
sl_zigbee_af_zcl_request_status_t sli_zigbee_af_test_meter_get_profiles(uint8_t intervalChannel,
                                                                        uint32_t endTime,
                                                                        uint8_t numberOfPeriods)
{
  return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
}
#endif // TEST_METER_ENABLE
