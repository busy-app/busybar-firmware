/***************************************************************************//**
 * @file
 * @brief I2C SPM examples functions
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stdio.h>
#include "sl_i2cspm.h"
#include "sl_i2cspm_instances.h"
#include "sl_sleeptimer.h"
#include "sl_simple_led.h"
#include "sl_simple_led_instances.h"
#include "FreeRTOS.h"
#include "task.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#ifndef I2CSPM_TASK_STACK_SIZE
#define I2CSPM_TASK_STACK_SIZE          256
#endif

#ifndef I2CSPM_TASK_PRIO
#define I2CSPM_TASK_PRIO                20
#endif

#ifndef EXAMPLE_USE_STATIC_ALLOCATION
#define EXAMPLE_USE_STATIC_ALLOCATION   1
#endif

// The limits are set relative to the initial temperature
#define TEMPERATURE_BAND_C               4

// SHT4X Configuration Settings
#define SHT4X_I2C_DEVICE                (sl_i2cspm_sensor) /**< I2C device used to control the SHT4X  */
#define SHT4X_I2C_BUS_ADDRESS           0x44               /**< I2C bus address                        */

// SHT4X command macro definitions
#define SHT4X_CMD_MEASURE_HPM    0xFD  /**< Measure Temp and Relative Humidity in High Precision Mode */
#define SHT4X_CMD_MEASURE_LPM    0xE0  /**< Measure Temp and Relative Humidity in Low Precision Mode */
#define SHT4X_CMD_READ_SERIAL    { 0x89 }  /**< Read the serial data */

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

static uint32_t relative_humidity;
static int32_t temperature;
static int32_t start_temperature;
static int32_t high_limit, low_limit;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

static void i2cspm_task(void *arg);

/***************************************************************************//**
 * Function to perform I2C transactions on the SHT4X
 *
 * This function is used to perform I2C transactions on the SHT4X
 * including read, write and continued write read operations.
 ******************************************************************************/
static I2C_TransferReturn_TypeDef SHT4X_transaction(uint16_t flag,
                                                    uint8_t *writeCmd,
                                                    size_t writeLen,
                                                    uint8_t *readCmd,
                                                    size_t readLen)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  seq.addr = SHT4X_I2C_BUS_ADDRESS << 1;
  seq.flags = flag;

  switch (flag) {
    // Send the write command from writeCmd
    case I2C_FLAG_WRITE:
      seq.buf[0].data = writeCmd;
      seq.buf[0].len  = writeLen;

      break;

    // Receive data into readCmd of readLen
    case I2C_FLAG_READ:
      seq.buf[0].data = readCmd;
      seq.buf[0].len  = readLen;

      break;

    // Send the write command from writeCmd
    // and receive data into readCmd of readLen
    case I2C_FLAG_WRITE_READ:
      seq.buf[0].data = writeCmd;
      seq.buf[0].len  = writeLen;

      seq.buf[1].data = readCmd;
      seq.buf[1].len  = readLen;

      break;

    default:
      return i2cTransferUsageFault;
  }

  // Perform the transfer and return status from the transfer
  ret = I2CSPM_Transfer(SHT4X_I2C_DEVICE, &seq);

  return ret;
}

/***************************************************************************//**
 * Function to decode Relative Humidity from the read value
 *
 * This function decodes RH data using the formula provided in the SHT4X
 * datasheet. Returns the value in Percent.
 ******************************************************************************/
uint32_t decode_rh(uint32_t rh_data)
{
  return ((((rh_data) * 15625L) >> 13) - 6000) / 1000;
}

/***************************************************************************//**
 * Function to decode Temperature from the read Temperature value
 *
 * This function decodes Temperature using the formula provided in the SHT4X
 * datasheet. Returns the value in Celsius.
 ******************************************************************************/
uint32_t decode_temp(int32_t temp_data)
{
  return ((((temp_data) * 21875L) >> 13) - 45000) / 1000;
}

/***************************************************************************//**
 * Function to measure humidity and temperature from the SHT4X
 *
 * This function measures current humidity and temperature using the SHT4X,
 * returning values using the pointer inputs.
 ******************************************************************************/
static void SHT4X_measure(uint32_t *rhData, int32_t *tData)
{
  I2C_TransferReturn_TypeDef ret;
  uint8_t cmd;
  uint8_t readData[6];
  uint32_t timeout;

  // Start high precision measurement by writing command to SHT4X
  cmd = SHT4X_CMD_MEASURE_HPM;
  ret = SHT4X_transaction(I2C_FLAG_WRITE, &cmd, 1, NULL, 0);
  EFM_ASSERT(ret == i2cTransferDone);

  // Wait for data to be ready
  timeout = 10;
  while (timeout--) {
    ret = SHT4X_transaction(I2C_FLAG_READ, NULL, 0, readData, 6);
    if (ret == i2cTransferDone) {
      break;
    } else if (ret == i2cTransferNack) {
      // SHT4X returns a Nack if data not ready
      const TickType_t xDelay = pdMS_TO_TICKS(10);
      vTaskDelay(xDelay);
    }
  }

  EFM_ASSERT(timeout > 0);

  //Extract the raw temperature data from the read data
  *tData = ((uint16_t)readData[0] << 8) | readData[1];

  //Extract the raw relative humidity data from the read data
  *rhData = ((uint16_t)readData[3] << 8) | readData[4];

  // Data is ready, decode the RH value
  *rhData = decode_rh(*rhData);

  // Data is ready, decode the Temperature value
  *tData = decode_temp(*tData);
}

/***************************************************************************//**
 * Helper function to Initialize the upper and lower limits
 *
 * This function sets the limits based on the temperature when the application
 * is started.
 ******************************************************************************/
static void initialise_temp_limits()
{
  // Get current temperature
  SHT4X_measure(&relative_humidity, &start_temperature);

  // Set temperature limits based on initial temperature.
  low_limit = start_temperature - (TEMPERATURE_BAND_C / 2);
  high_limit = start_temperature + (TEMPERATURE_BAND_C / 2);
}

#if SL_SIMPLE_LED_COUNT > 1
/***************************************************************************//**
 * Helper function to turn on led 0 or 1 based on current temperature
 *
 * This function turns on led 0 or 1 based on the current temperature and
 * lower and upper limits.
 ******************************************************************************/
static void set_leds(int32_t temp)
{
  if (temp > high_limit) {
    // For higher temperature, turn led0 on and turn led1 off
    sl_led_turn_on(&sl_led_led0);
    sl_led_turn_off(&sl_led_led1);
  } else if (temp < low_limit) {
    // For lower temperature, turn led1 on and turn led0 off
    sl_led_turn_off(&sl_led_led0);
    sl_led_turn_on(&sl_led_led1);
  }
}
#endif

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Initialize i2cspm example.
 ******************************************************************************/
void i2cspm_init(void)
{
  TaskHandle_t xHandle = NULL;

#if (EXAMPLE_USE_STATIC_ALLOCATION == 1)

  static StaticTask_t xTaskBuffer;
  static StackType_t  xStack[I2CSPM_TASK_STACK_SIZE];

  // Create I2CSPM Task without using any dynamic memory allocation
  xHandle = xTaskCreateStatic(i2cspm_task,
                              "i2cspm task",
                              I2CSPM_TASK_STACK_SIZE,
                              ( void * ) NULL,
                              tskIDLE_PRIORITY + 1,
                              xStack,
                              &xTaskBuffer);

  // Since puxStackBuffer and pxTaskBuffer parameters are not NULL,
  // it is impossible for xHandle to be null. This check is for
  // rigorous example demonstration.
  EFM_ASSERT(xHandle != NULL);

#else

  BaseType_t xReturned = pdFAIL;

  // Create Blink Task using dynamic memory allocation
  xReturned = xTaskCreate(i2cspm_task,
                          "i2cspm task",
                          I2CSPM_TASK_STACK_SIZE,
                          ( void * ) NULL,
                          tskIDLE_PRIORITY + 1,
                          &xHandle);

  // Unlike task creation using static allocation, dynamic task creation can very likely
  // fail due to lack of memory. Checking the return value is relevant.
  EFM_ASSERT(xReturned == pdPASS);

#endif
}

/*******************************************************************************
 * i2cspm task.
 ******************************************************************************/
static void i2cspm_task(void *arg)
{
  (void)&arg;

  I2C_TransferReturn_TypeDef ret = i2cTransferInProgress;
  uint8_t cmdReadId[1] = SHT4X_CMD_READ_SERIAL;
  uint8_t deviceId[6];
  TickType_t xDelay;

  // Wait for sensor to become ready
  xDelay = pdMS_TO_TICKS(80);
  vTaskDelay(xDelay);

  // Check for device presence
  // Each SHT4x sensor has a unique 48-bit device Id
  for (uint8_t attempt = 0; attempt < 5 && ret != i2cTransferDone; attempt++) {
    ret = SHT4X_transaction(I2C_FLAG_WRITE,
                            cmdReadId,
                            sizeof(cmdReadId),
                            NULL,
                            0);
    if (ret != i2cTransferDone) {
      vTaskDelay(xDelay);
      continue;
    }

    vTaskDelay(pdMS_TO_TICKS(1));

    ret = SHT4X_transaction(I2C_FLAG_READ,
                            NULL,
                            0,
                            deviceId,
                            sizeof(deviceId));
    if (ret == i2cTransferDone) {
      break;
    }

    vTaskDelay(xDelay);
  }

  EFM_ASSERT(ret == i2cTransferDone);

  // printf is configured to redirect to vcom in project file
  printf("\r\nWelcome to the I2C SPM sample application using FreeRTOS\r\n\r\n");

  // Get current temperature
  initialise_temp_limits();

  //Use the provided calculation macro to convert milliseconds to OS ticks
  xDelay = pdMS_TO_TICKS(1000);

  while (1) {
    //Wait for specified delay
    vTaskDelay(xDelay);

    // Measure the current humidity and temperature
    SHT4X_measure(&relative_humidity, &temperature);

    // Print the current humidity and temperature to vcom
    printf("\r\n");
    printf("Relative Humidity = %ld%%\r\n", relative_humidity);
    printf("Temperature = %ld C\r\n", temperature);

#if SL_SIMPLE_LED_COUNT > 1
    // Set appropriate LEDs (led0 or 1) based on temperature
    set_leds(temperature);
#endif

    if (temperature > high_limit) {
      printf("Temperature is high.\r\n");
    } else if (temperature < low_limit) {
      printf("Temperature is low.\r\n");
    }
  }
}
