/***************************************************************************/ /**
 * @file adxl345.c
 * @brief ADXL345 API implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
/**=============================================================================
 * @brief : This file contains the ADXL sensor's driver file for Sensor Hub
 * application
 * This example uses simple communication APIs for transferring the ADXL
 * sensor data through I2C interface.
 * This sensor will detect the RGB light, Gesture and Proximity and sends the
 * detected RGB data values, the detection value for Gesture and Proximity data
 * value
 ============================================================================**/
#include "adxl345.h"
#include "sensor_hub.h"
#include "rsi_debug.h"
#include "math.h"
#include "Driver_SPI.h"

/*******************************************************************************
 ************************ APDS Defines / Macros  ******************************
 ******************************************************************************/
extern ARM_DRIVER_SPI *SPIdrv;
extern sl_bus_intf_config_t bus_intf_info;
#define SL_ADXL345_DEVICE  (0x53) // Device Address for SL_ADXL345
#define SL_ADXL345_TO_READ (6)    // Number of Bytes Read - Two Bytes Per Axis
#define SL_BUFFER_SIZE     10     // Number of data to be sent through SPI

/*******************************************************************************
 ************************ ADXL data Variables   ********************************
 ******************************************************************************/
sl_adxl345_handle_t adxl345 = NULL;        //adxl handle
uint8_t testdata_out[SL_BUFFER_SIZE];      //out data buffer
uint8_t testdata_in[SL_BUFFER_SIZE];       //in data buffer
uint8_t buff[7];                           //data buffer
static bool is_accelerometer_init = false; //ADXL sensor status
uint8_t error_code;                        //Initial State
double gains[3];                           //Counts to Gs
int32_t status1 = 0;                       //data status
int16_t a = 0, b = 0, c = 0;               //data variables
float xg = 0, yg = 0, zg = 0;              //data variables
uint8_t interrupts;                        //sensor interrupt status

/*******************************************************************************
*  @fn          sl_adxl345_handle_t sl_si91x_adxl345_create
*                                                      (sl_sensor_bus_t bus)
*  @brief       Creates ADXL345 sensor in the Sensor Hub.
*  @param[in]   bus:  I2C bus handle
*  @return      Returns the ADXL345 sensor handle
*******************************************************************************/
sl_adxl345_handle_t sl_si91x_adxl345_create(sl_sensor_bus_t bus)
{
  sl_adxl345_dev_t *sensor = (sl_adxl345_dev_t *)pvPortMalloc(sizeof(sl_adxl345_dev_t));
  (void)bus;
  DEBUGOUT("\r\n SPI - ADXL345 Sensor created \r\n");
  return (sl_adxl345_handle_t)sensor;
}

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_adxl345_delete(sl_adxl345_handle_t *sensor)
*  @brief       Deletes ADXL345 sensor handle in the Sensor Hub.
*  @param[in]   sensor: ADXL345 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_adxl_err_t sl_si91x_adxl345_delete(sl_adxl345_handle_t *sensor)
{
  if (*sensor == NULL) {
    return RSI_OK;
  }
  free(sensor);
  sensor = NULL;
  return RSI_OK;
}

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_adxl345_set_measure_mode(
*               sl_adxl345_handle_t sensor,sl_adxl345_cmd_measure_t cmd_measure)
*  @brief       Sets the Measure Mode
*  @param[in]   sensor: ADXL345 Sensor handle
*  @param[in]   cmd_measure Command Measure
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_adxl_err_t sl_si91x_adxl345_set_measure_mode(sl_adxl345_handle_t sensor, sl_adxl345_cmd_measure_t cmd_measure)
{
  uint8_t ret            = 0, adxl_data_buf[1];
  sl_adxl345_dev_t *sens = (sl_adxl345_dev_t *)sensor;
  (void)cmd_measure;
  (void)adxl_data_buf;
  (void)sens;
  /*  a[0] = (uint8_t)(cmd_measure & 0xFF);
  I2Cdrv->MasterTransmit(SL_ADXL345_I2C_ADDRESS_DEFAULT, a, 1, false);
   TODO: Add timeout and PASS error
  while (I2Cdrv->GetStatus().busy)
    ;*/
  return ret;
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_isr(void)
*  @brief       Looks for Interrupts and Triggered Action
*  @param[in]   NULL
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_isr(void)
{

  // getInterruptSource clears all triggered actions after returning value
  // Do not call again until you need to recheck for triggered actions
  interrupts = sl_si91x_adxl345_get_interrupt_source_val();

  // Free Fall Detection
  if (sl_si91x_adxl345_triggered(interrupts, SL_ADXL345_FREE_FALL)) {
    DEBUGOUT("\r\n*** FREE FALL ***\r\n");
    //add code here to do when free fall is sensed
  }

  // Inactivity
  if (sl_si91x_adxl345_triggered(interrupts, SL_ADXL345_INACTIVITY)) {
    DEBUGOUT("\r\n*** INACTIVITY ***\r\n");
    //add code here to do when inactivity is sensed
  }

  // Activity
  if (sl_si91x_adxl345_triggered(interrupts, SL_ADXL345_ACTIVITY)) {
    DEBUGOUT("\r\n*** ACTIVITY ***\r\n");
    //add code here to do when activity is sensed
  }

  // Double Tap Detection
  if (sl_si91x_adxl345_triggered(interrupts, SL_ADXL345_DOUBLE_TAP)) {
    DEBUGOUT("\r\n*** DOUBLE TAP ***\r\n");
    //add code here to do when a 2X tap is sensed
  }

  // Tap Detection
  if (sl_si91x_adxl345_triggered(interrupts, SL_ADXL345_SINGLE_TAP)) {
    DEBUGOUT("\r\n*** TAP ***\r\n");
    //add code here to do when a tap is sensed
  }
}

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_adxl345_init(sl_sensor_bus_t spi_bus)
*  @brief       Initializing the ADXL345 sensor in the Sensor Hub.
*  @param[in]   spi_bus: ADXL345 Sensor I2C Bus handle
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_adxl_err_t sl_si91x_adxl345_init(sl_sensor_bus_t spi_bus)
{
  adxl345 = sl_si91x_adxl345_create(spi_bus);

  sl_si91x_adxl345_power_on();

  if (adxl345 == NULL) {
    return RSI_FAIL;
  }
  is_accelerometer_init = true;
  return RSI_OK;
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_power_on(void)
*  @brief       Power ON the ADXL345 sensor and sets interrupts in the ADXL345.
*  @param[in]   sensor: ADXL345 Sensor handle
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_power_on(void)
{
  sl_si91x_adxl345_gain();

  write(SL_ADXL345_POWER_CTL, 0x00);   // Wakeup
  write(SL_ADXL345_POWER_CTL, 0x08);   // Measure
  write(SL_ADXL345_DATA_FORMAT, 0x03); // Setting SPI bit

  sl_si91x_adxl345_set_range_setting(16);
  sl_si91x_adxl345_setactivity_xyz(
    1,
    0,
    0); // Set to activate movement detection in the axes "sl_si91x_adxl345_setactivity_xyz(X, Y, Z);" (1 == ON, 0 == OFF)
  sl_si91x_adxl345_set_activitythreshold(
    75); // 62.5mg per increment   // Set activity   // Inactivity thresholds (0-255)

  sl_si91x_adxl345_setinactivity_xyz(
    1,
    0,
    0); // Set to detect inactivity in all the axes "sl_si91x_adxl345_setinactivity_xyz(X, Y, Z);" (1 == ON, 0 == OFF)
  sl_si91x_adxl345_set_inactivitythreshold(
    75);                                   // 62.5mg per increment   // Set inactivity // Inactivity thresholds (0-255)
  sl_si91x_adxl345_set_timeinactivity(10); // How many seconds of no activity is inactive?

  sl_si91x_adxl345_set_tapdetection_on_xyz(
    0,
    0,
    1); // Detect taps in the directions turned ON "setTapDetectionOnX(X, Y, Z);" (1 == ON, 0 == OFF)
  // Set values for what is considered a TAP and what is a DOUBLE TAP (0-255)
  sl_si91x_adxl345_set_tapthreshold(50);     // 62.5 mg per increment
  sl_si91x_adxl345_set_tapduration(15);      // 625 μs per increment
  sl_si91x_adxl345_set_doubletaplatency(80); // 1.25 ms per increment
  sl_si91x_adxl345_set_doubletapwindow(200); // 1.25 ms per increment

  // Set values for what is considered FREE FALL (0-255)
  sl_si91x_adxl345_set_freefallthreshold(7); // (5 - 9) recommended - 62.5mg per increment
  sl_si91x_adxl345_set_freefallduration(30); // (20 - 70) recommended - 5ms per increment

  // Turn on Interrupts for each mode (1 == ON, 0 == OFF)
  sl_si91x_adxl345_inactivity_interrupt(1);
  sl_si91x_adxl345_activity_interrupt(1);
  sl_si91x_adxl345_freefall_interrupt(1);
  sl_si91x_adxl345_doubletap_interrupt(1);
  sl_si91x_adxl345_singletap_interrupt(1);
}

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_adxl345_deinit(void)
*  @brief       DeInitializing the ADXL345 sensor in the Sensor Hub.
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_adxl_err_t sl_si91x_adxl345_deinit(void)
{
  if (!is_accelerometer_init) {
    return RSI_FAIL;
  }
  sl_adxl_err_t ret = sl_si91x_adxl345_delete(&adxl345);
  if (ret == RSI_OK) {
    is_accelerometer_init = false;
  }
  return RSI_OK;
}

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_adxl345_test(void)
*  @brief       Testing the ADXL345 sensor in the Sensor Hub.
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_adxl_err_t sl_si91x_adxl345_test(void)
{
  if (!is_accelerometer_init) {
    return RSI_FAIL;
  }
  return RSI_OK;
}

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_adxl345_sample_accelerometer(float *x,
*                                                           float *y, float *z)
*  @brief       Processes and Reads the value of RED, GREEN and BLUE color light
*               and sends the value to Sensor Hub
*  @param[in]   x: Value of X Axis
*  @param[in]   y: Value of Y Axis
*  @param[in]   z: Value of Z Axis
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_adxl_err_t sl_si91x_adxl345_sample_accelerometer(float *x, float *y, float *z)
{
  if (!is_accelerometer_init) {
    DEBUGOUT("\r\n ADXL sensor data reading fail \r\n");
    return RSI_FAIL;
  }
  sl_adxl_err_t ret = 0;
  write(SL_ADXL345_POWER_CTL, 0x00);   // Wakeup
  write(SL_ADXL345_POWER_CTL, 0x08);   // Measure
  write(SL_ADXL345_DATA_FORMAT, 0x03); // Setting SPI bit

  sl_si91x_adxl345_read_acceleration(
    &a,
    &b,
    &c); // Read the accelerometer values and store them in variables declared above a, b, c

  //For 16g
  xg = (float)(a * .0312);
  yg = (float)(b * .0312);
  zg = (float)(c * .0312);

  for (int i = 0; i < 10000; i++)
    ;

  sl_si91x_adxl345_isr();
  // You may also choose to avoid using interrupts and simply run the functions within ADXL_ISR();
  //  and place it within the loop instead.
  // This may come in handy when it doesn't matter when the action occurs.

  if (ret == RSI_OK) {
    *x = xg;
    *y = yg;
    *z = zg;
    return RSI_OK;
  }

  *x = 0;
  *y = 0;
  *z = 0;
  return RSI_FAIL;
}

/*******************************************************************************
*  @fn          void write(uint8_t address, uint8_t value)
*  @brief       SPI Write operations
*  @param[in]   address: Register address to which Primary needs to send the
*               data to secondary
*  @param[in]   value: The value which needs to stored the above address
*  @return      NULL
*******************************************************************************/
void write(uint8_t address, uint8_t value)
{
  int32_t status = 0;
  uint8_t data[2];
  address = address | 0x40;
  data[0] = address;
  data[1] = value;
  // Assert the slave select line for the connected slave
  SPIdrv->Control(bus_intf_info.spi_config.spi_cs_misc_mode, bus_intf_info.spi_config.spi_sec_sel_sig);

  status = SPIdrv->Send(data, 2);
  /*TODO: add the failure case */
  if (status != RSI_OK) {
    DEBUGOUT("\r\n SPI Send FAIL \r\n");
  }
}

/*******************************************************************************
*  @fn          void read(uint8_t address, int len, uint8_t buffer[]
*  @brief       SPI Read operations
*  @param[in]   address: Register address from which Primary needs to read the
*               data from secondary
*  @param[in]   len: Number of Bytes Read from buffer
*  @param[in]   buffer: The buffer in which the values to be stored after
*               reading the Number of Bytes
*  @return      NULL
*******************************************************************************/
void read(uint8_t address, uint32_t len, uint8_t *buffer)
{
  int32_t status = 0;
  uint8_t data[len + 1];
  address = address | 0xC0;
  data[0] = address;
  for (uint32_t i = 1; i < len + 1; ++i)
    data[i] = 0;
  // Assert the slave select line for the connected slave
  SPIdrv->Control(bus_intf_info.spi_config.spi_cs_misc_mode, bus_intf_info.spi_config.spi_sec_sel_sig);
  status = SPIdrv->Transfer(data, buffer, len + 1);

  if (status != RSI_OK) {
    DEBUGOUT("\r\n SPI Transfer FAIL \r\n");
  }
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_gain(void)
*  @brief       Set Original Gains
*  @param[in]   NULL
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_gain(void)
{
  status1    = SL_ADXL345_OK;
  error_code = SL_ADXL345_NO_ERROR;
  gains[0]   = 0.00376390;
  gains[1]   = 0.00376009;
  gains[2]   = 0.00349265;
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_read_acceleration_value(int *xyz)
*  @brief       Reads Acceleration into Three Variables: x, y and z
*  @param[in]   xyz: variable in which acceleration vale is stored
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_read_acceleration_value(int *xyz)
{
  sl_si91x_adxl345_read_acceleration((int16_t *)xyz, (int16_t *)(xyz + 1), (int16_t *)(xyz + 2));
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_read_acceleration(int *a, int *b, int *c)
*  @brief       Reads and stores processes Acceleration value into three
*               variables: a, b and c
*  @param[in]   a, b, c: Axis at all g ranges with 10 Bit Resolution (2 Bytes)
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_read_acceleration(int16_t *read_a, int16_t *read_b, int16_t *read_c)
{
  read(SL_ADXL345_DATAX0, SL_ADXL345_TO_READ, buff); // Read Accel Data from ADXL345

  // Each Axis @ All g Ranges: 10 Bit Resolution (2 Bytes)
  *read_a = ((buff[2] << 8) | buff[1]);
  *read_b = ((buff[4] << 8) | buff[3]);
  *read_c = ((buff[6] << 8) | buff[5]);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_get_Gxyz(double *xyz)
*  @brief       Reads and stores Acceleration value into variable xyz_int and
*               processes with gain value
*  @param[in]   xyz: Axis value at g ranges
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_get_Gxyz(double *xyz)
{
  int i;
  int xyz_int[3];
  sl_si91x_adxl345_read_acceleration_value(xyz_int);
  for (i = 0; i < 3; i++) {
    xyz[i] = xyz_int[i] * gains[i];
  }
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_get_range_setting(uint8_t *rangeSetting)
*  @brief       Gets the range setting value
*  @param[in]   rangeSetting: Get the range value (2g, 4g, 8g, 16g)
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_get_range_setting(uint8_t *rangeSetting)
{
  uint8_t _b[2];
  uint32_t temp = 0;
  read(SL_ADXL345_DATA_FORMAT, 1, _b);
  temp          = _b[0] | (_b[1] << 8);
  *rangeSetting = (uint8_t)temp & 0x03;
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_range_setting(int val)
*  @brief       Sets the range setting value
*  @param[in]   val: Sets the range value as 2g/ 4g/ 8g/ 16g
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_range_setting(int val)
{
  uint8_t _s;
  uint8_t _b[2];
  uint32_t temp;

  switch (val) {
    case 2:
      _s = 0x00;
      break;
    case 4:
      _s = 0x01;
      break;
    case 8:
      _s = 0x02;
      break;
    case 16:
      _s = 0x03;
      break;
    default:
      _s = 0x00;
  }

  read(SL_ADXL345_DATA_FORMAT, 1, _b);
  temp = _b[0] | (_b[1] << 8);
  _s |= (uint8_t)(temp & 0xEC);
  write(SL_ADXL345_DATA_FORMAT, _s);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_selftest_bit()
*  @brief       Gets the Self test bit value
*  @param[in]   NULL
*  @return      Returns self Test bit value
*******************************************************************************/
bool sl_si91x_adxl345_get_selftest_bit()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_DATA_FORMAT, 7);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_selftest_bit(bool selfTestBit)
*  @brief       Sets the Self test bit value
*  @param[in]   selfTestBit: value to be set in selfTestBit
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_selftest_bit(bool selfTestBit)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_DATA_FORMAT, 7, selfTestBit);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_spi_bit()
*  @brief       Gets the SPI bit value
*  @param[in]   NULL
*  @return      Returns spiBit value
*******************************************************************************/
bool sl_si91x_adxl345_get_spi_bit()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_DATA_FORMAT, 6);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_spi_bit(bool spiBit)
*  @brief       Sets the SPI bit value
*  @param[in]   spiBit: value to be set in spiBit
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_spi_bit(bool spiBit)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_DATA_FORMAT, 6, spiBit);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_interruptlevel_bit()
*  @brief       Gets the Interrupt level bit
*  @param[in]   NULL
*  @return      Returns Interrupt level bit
*******************************************************************************/
bool sl_si91x_adxl345_get_interruptlevel_bit(void)
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_DATA_FORMAT, 5);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_interrupt_level_bit
*                                                       (bool interruptLevelBit)
*  @brief       Set to 1 to make the Interrupts Active
*               HIGH and set to 0 to make the Interrupts Active LOW
*  @param[in]   interruptLevelBit: value to be set in interruptLevelBit
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_interrupt_level_bit(bool interruptLevelBit)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_DATA_FORMAT, 5, interruptLevelBit);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_full_resolution_bit()
*  @brief       Gets the full resolution bit value: 1 is when Full Resolution
*               Mode and and 0 is when Device is in 10-bit Mode
*  @param[in]   NULL
*  @return      Returns full resolution bit value
*******************************************************************************/
bool sl_si91x_adxl345_get_full_resolution_bit()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_DATA_FORMAT, 3);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_full_resolution_bit(bool fullResBit)
*  @brief       Set to 1 to set the device in Full Resolution Mode. Output
*               Resolution Increase with G Range Set by the Range Bits to
*               Maintain a 4mg/LSB Scale Factor.
*               Set to 1 to set the device in 10-bit Mode: Range Bits Determine
*               Maximum G Range and Scale Factor
*  @param[in]   fullResBit: value to be set in full Resolution Bit
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_full_resolution_bit(bool fullResBit)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_DATA_FORMAT, 3, fullResBit);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_justify_bit()
*  @brief       Gets the Justify bit value. 1 is Left Justified mode and 0 is
*               Right Justified Mode with Sign Extension
*  @param[in]   NULL
*  @return      Returns Justify bit value
*******************************************************************************/
bool sl_si91x_adxl345_get_justify_bit()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_DATA_FORMAT, 2);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_justify_bit(bool justifyBit)
*  @brief       Setting to 1 Selects the Left Justified Mode and
*               Setting to 1 Selects Right Justified Mode with Sign Extension
*  @param[in]   justifyBit: value to be set in Justify bit value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_justify_bit(bool justifyBit)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_DATA_FORMAT, 2, justifyBit);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_tapthreshold(int tapThreshold)
*  @brief       Setting Tap Threshold value between 0 and 255. Scale Factor is
*               62.5 mg/LSB. A Value of 0 May Result in Undesirable Behavior
*  @param[in]   tapThreshold: value to be set in Tap Threshold value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_tapthreshold(int tapThreshold)
{
  tapThreshold = compare(tapThreshold, 0, 255);
  uint8_t _b   = (uint8_t)(tapThreshold);
  write(SL_ADXL345_THRESH_TAP, _b);
}

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_tapthreshold()
*  @brief       Gets the value of Tap Threshold value.
*               Scale Factor is 62.5 mg/LSB
*  @param[in]   NULL
*  @return      Returns Value Between 0 and 255
*******************************************************************************/
int sl_si91x_adxl345_get_tapthreshold()
{
  uint8_t _b[2];
  int temp = 0;
  read(SL_ADXL345_THRESH_TAP, 1, _b);
  temp = _b[0] | (_b[1] << 8);
  return temp;
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_axisgains(double *_gains)
*  @brief       Sets the Axis gain value for each Axis in Gs/Count
*  @param[in]   _gains: gain value for each Axis in Gs/Count
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_axisgains(double *_gains)
{
  int i;
  for (i = 0; i < 3; i++) {
    gains[i] = _gains[i];
  }
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_get_axisgains(double *_gains)
*  @brief       Gets the AXis gain value for each Axis in Gs/Count.
*  @param[in]   _gains: gain value for each Axis in Gs/Count
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_get_axisgains(double *_gains)
{
  int i;
  for (i = 0; i < 3; i++) {
    _gains[i] = gains[i];
  }
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_axisoffset(int x, int y, int z)
*  @brief       Sets the User Offset Adjustments in Twos Complement Format.
*               Scale Factor of 15.6mg/LSB
*  @param[in]   _gains: gain value for each Axis in Gs/Count
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_axisoffset(uint8_t x, uint8_t y, uint8_t z)
{
  write(SL_ADXL345_OFSX, (x));
  write(SL_ADXL345_OFSY, (y));
  write(SL_ADXL345_OFSZ, (z));
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_get_axisoffset(int *x, int *y, int *z)
*  @brief       Gets the User Offset Adjustments in Twos Complement Format.
*               Scale Factor of 15.6mg/LSB
*  @param[in]   x, y, z: Axis offset value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_get_axisoffset(int *x, int *y, int *z)
{
  uint8_t _b[2];
  int temp;
  read(SL_ADXL345_OFSX, 1, _b);
  temp = _b[0] | (_b[1] << 8);
  *x   = (temp);
  read(SL_ADXL345_OFSY, 1, _b);
  temp = _b[0] | (_b[1] << 8);
  *y   = (temp);
  read(SL_ADXL345_OFSZ, 1, _b);
  temp = _b[0] | (_b[1] << 8);
  *z   = (temp);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_tapduration(int tapDuration)
*  @brief       Sets an Unsigned Time Value Representing the Max Time
*               that an Event must be Above the THRESH_TAP Threshold to qualify
*               as a Tap Event. The scale factor is 625µs/LSB. Value of 0
*               Disables the Tap/Double Tap Functions. Max value is 255.
*  @param[in]   tapDuration: Maximum Time for a tap event
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_tapduration(int tapDuration)
{
  tapDuration = compare(tapDuration, 0, 255);
  uint8_t _b  = (uint8_t)(tapDuration);
  write(SL_ADXL345_DUR, _b);
}

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_tapduration()
*  @brief       Gets an Tap/Double Tap duration
*  @param[in]   NULL
*  @return      Returns tap duration value
*******************************************************************************/
int sl_si91x_adxl345_get_tapduration()
{
  uint8_t _b[2];
  int temp = 0;
  read(SL_ADXL345_DUR, 1, _b);
  temp = _b[0] | (_b[1] << 8);
  return temp;
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_doubletaplatency(int doubleTapLatency)
*  @brief       Sets the Latent register. Wait Time from the Detection of a Tap
*               Event to the Start of the Time Window (defined by the Window
*               Register) during which a possible Second Tap Even can be
*               Detected. Scale Factor is 1.25ms/LSB.
*               A Value of 0 Disables the Double Tap Function.
*               Max value: 255.
*  @param[in]   doubleTapLatency: Latency time
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_doubletaplatency(int doubleTapLatency)
{
  uint8_t _b = (uint8_t)(doubleTapLatency);
  write(SL_ADXL345_LATENT, _b);
}

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_doubletaplatency()
*  @brief       Gets latent register value
*  @param[in]   doubleTapLatency: Latency time
*  @return      Returns the Latent register value
*******************************************************************************/
int sl_si91x_adxl345_get_doubletaplatency()
{
  uint8_t _b[2];
  int temp = 0;
  read(SL_ADXL345_LATENT, 1, _b);
  temp = _b[0] | (_b[1] << 8);
  return temp;
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_doubletapwindow(int doubleTapWindow)
*  @brief       Sets the the Amount of Time After the Expiration of the Latency
*               Time (determined by Latent register) during which a second valid
*               tap can Begin.
*               Scale Factor is 1.25ms/LSB.
*               A Value of 0 Disables the Double Tap Function.
*               Max value: 255.
*  @param[in]   doubleTapWindow: Double tap Window Register
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_doubletapwindow(int doubleTapWindow)
{
  doubleTapWindow = compare(doubleTapWindow, 0, 255);
  uint8_t _b      = (uint8_t)(doubleTapWindow);
  write(SL_ADXL345_WINDOW, _b);
}

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_doubletapwindow()
*  @brief       Gets the value of Window Register
*  @param[in]   NULL
*  @return      Returns the value of Window Register
*******************************************************************************/
int sl_si91x_adxl345_get_doubletapwindow()
{
  uint8_t _b[2];
  int temp = 0;
  read(SL_ADXL345_WINDOW, 1, _b);
  temp = _b[0] | (_b[1] << 8);
  return temp;
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_activitythreshold(int activityThreshold)
*  @brief       Sets Threshold Value for Detecting Activity. Magnitude of the
*               Activity Event is Compared with the Value is Compared with the
*               Value in the THRESH_ACT Register.
*               The Scale Factor is 62.5mg/LSB.
*               Value of 0 may Result in Undesirable Behavior if the Activity
*               Interrupt Enabled.
*               Max value: 255.
*  @param[in]   activityThreshold: Value of Threshold Activity Register
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_activitythreshold(int activityThreshold)
{
  activityThreshold = compare(activityThreshold, 0, 255);
  uint8_t _b        = (uint8_t)(activityThreshold);
  write(SL_ADXL345_THRESH_ACT, _b);
}

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_activitythreshold()
*  @brief       Gets the THRESH_ACT byte
*  @param[in]   NULL
*  @return      Returns the value THRESH_ACT
*******************************************************************************/
int sl_si91x_adxl345_get_activitythreshold()
{
  uint8_t _b[2];
  int temp = 0;
  read(SL_ADXL345_THRESH_ACT, 1, _b);
  temp = _b[0] | (_b[1] << 8);
  return temp;
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_inactivitythreshold
*                                                      (int inactivityThreshold)
*  @brief       Sets Threshold Inactive register Value for Detecting InActivity.
*               Magnitude of the InActivity Event is Compared with the Value is
*               Compared with the Value in the SL_ADXL345_THRESH_INACT Register.
*               The Scale Factor is 62.5mg/LSB.
*               Value of 0 may Result in Undesirable Behavior if the Activity
*               Interrupt Enabled. Max value: 255.
*  @param[in]   inactivityThreshold: Value of Threshold InActivity Register
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_inactivitythreshold(int inactivityThreshold)
{
  inactivityThreshold = compare(inactivityThreshold, 0, 255);
  uint8_t _b          = (uint8_t)(inactivityThreshold);
  write(SL_ADXL345_THRESH_INACT, _b);
}

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_inactivitythreshold()
*  @brief       Gets the Threshold Inactive register value
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_THRESH_INACT
*******************************************************************************/
int sl_si91x_adxl345_get_inactivitythreshold()
{
  uint8_t _b[2];
  int temp = 0;
  read(SL_ADXL345_THRESH_INACT, 1, _b);
  temp = _b[0] | (_b[1] << 8);
  return temp;
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_timeinactivity(int timeInactivity)
*  @brief       Sets Time Inactivity register Value. Acceleration must be Less
*               Than the Value in the SL_ADXL345_THRESH_INACT Register for
*                Inactivity to be Declared.
*               Scale Factor is 1sec/LSB.
*               Min value: 0, Max value: 255.
*  @param[in]   timeInactivity: Value of Time InActivity Register
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_timeinactivity(int timeInactivity)
{
  timeInactivity = compare(timeInactivity, 0, 255);
  uint8_t _b     = (uint8_t)(timeInactivity);
  write(SL_ADXL345_TIME_INACT, _b);
}

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_timeinactivity()
*  @brief       Gets the Time Inactivity register value
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_TIME_INACT
*******************************************************************************/
int sl_si91x_adxl345_get_timeinactivity()
{
  uint8_t _b[2];
  int temp = 0;
  read(SL_ADXL345_TIME_INACT, 1, _b);
  temp = _b[0] | (_b[1] << 8);
  return temp;
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_freefallthreshold(int freeFallThreshold)
*  @brief       Sets the free fall register Value. The Acceleration on all Axes
*               is Compared with the Value in THRES_FF to determine if a
*               Free-Fall Event Occurred.
*               Scale Factor is 62.5mg/LSB.
*               Value of 0 May Result in Undesirable Behavior if the Free-Fall
*               interrupt Enabled, Max value: 255.
*  @param[in]   freeFallThreshold: Value of Free fall Threshold register
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_freefallthreshold(int freeFallThreshold)
{
  freeFallThreshold = compare(freeFallThreshold, 0, 255);
  uint8_t _b        = (uint8_t)freeFallThreshold;
  write(SL_ADXL345_THRESH_FF, _b);
}

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_freefallthreshold()
*  @brief       Gets the Free Fall threshold register value
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_THRESH_FF register
*******************************************************************************/
int sl_si91x_adxl345_get_freefallthreshold()
{
  uint8_t _b[2];
  int temp = 0;
  read(SL_ADXL345_THRESH_FF, 1, _b);
  temp = _b[0] | (_b[1] << 8);
  return temp;
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_freefallduration(int freeFallDuration)
*  @brief       Sets the free fall duration register Value. the Minimum Time
*               that the Value of all Axes must be Less Than THRES_FF to
*               Generate a Free-Fall Interrupt.
*               Scale Factor is 5ms/LSB.
*               Value of 0 May Result in Undesirable Behavior if the Free-Fall
*               interrupt is Enabled, Max value: 255.
*  @param[in]   freeFallDuration: Value of Free fall Duration register
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_freefallduration(int freeFallDuration)
{
  freeFallDuration = compare(freeFallDuration, 0, 255);
  uint8_t _b       = (uint8_t)freeFallDuration;
  write(SL_ADXL345_TIME_FF, _b);
}

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_freefallduration()
*  @brief       Gets the Free Fall Duration register value
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_TIME_FF register
*******************************************************************************/
int sl_si91x_adxl345_get_freefallduration()
{
  uint8_t _b[2];
  int temp = 0;
  read(SL_ADXL345_TIME_FF, 1, _b);
  temp = _b[0] | (_b[1] << 8);
  return temp;
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isactivity_x_enabled()
*  @brief       Enable X Axis Enable Control for Activity & Inactivity Detection
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_ACT_INACT_CTL register bit
*******************************************************************************/
bool sl_si91x_adxl345_isactivity_x_enabled()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_ACT_INACT_CTL, 6);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isactivity_y_enabled()
*  @brief       Enable Y Axis: Enable Control for Activity & Inactivity Detection
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_ACT_INACT_CTL register bit
*******************************************************************************/
bool sl_si91x_adxl345_isactivity_y_enabled()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_ACT_INACT_CTL, 5);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isactivity_z_enabled()
*  @brief       Enable Z Axis: Enable Control for Activity & Inactivity Detection
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_ACT_INACT_CTL register bit
*******************************************************************************/
bool sl_si91x_adxl345_isactivity_z_enabled()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_ACT_INACT_CTL, 4);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isinactivity_x_enabled()
*  @brief       Disable X Axis Enable Control for Activity & Inactivity Detection
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_ACT_INACT_CTL register bit
*******************************************************************************/
bool sl_si91x_adxl345_isinactivity_x_enabled()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_ACT_INACT_CTL, 2);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isinactivity_y_enabled()
*  @brief       Disable Y Axis: Enable Control for Activity & Inactivity Detection
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_ACT_INACT_CTL register bit
*******************************************************************************/
bool sl_si91x_adxl345_isinactivity_y_enabled()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_ACT_INACT_CTL, 1);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isinactivity_z_enabled()
*  @brief       Disable Z Axis: Enable Control for Activity & Inactivity Detection
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_ACT_INACT_CTL register bit
*******************************************************************************/
bool sl_si91x_adxl345_isinactivity_z_enabled()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_ACT_INACT_CTL, 0);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_setactivity_x(bool state)
*  @brief       Sets Activity for X Axis
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_setactivity_x(bool state)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_ACT_INACT_CTL, 6, state);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_setactivity_y(bool state)
*  @brief       Sets Activity for Y Axis
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_setactivity_y(bool state)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_ACT_INACT_CTL, 5, state);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_setactivity_z(bool state)
*  @brief       Sets Activity for Z Axis
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_setactivity_z(bool state)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_ACT_INACT_CTL, 4, state);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_setactivity_xyz(bool stateX, bool stateY,
*                                                                   bool stateZ)
*  @brief       Sets Activity for XYZ Axis
*  @param[in]   stateX: State to change the register value for X axis
*  @param[in]   stateY: State to change the register value for Y axis
*  @param[in]   stateZ: State to change the register value for Z axis
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_setactivity_xyz(bool stateX, bool stateY, bool stateZ)
{
  sl_si91x_adxl345_setactivity_x(stateX);
  sl_si91x_adxl345_setactivity_y(stateY);
  sl_si91x_adxl345_setactivity_z(stateZ);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_setinactivity_x(bool state)
*  @brief       Sets InActivity for X Axis
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_setinactivity_x(bool state)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_ACT_INACT_CTL, 2, state);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_setinactivity_y(bool state)
*  @brief       Sets InActivity for Y Axis
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_setinactivity_y(bool state)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_ACT_INACT_CTL, 1, state);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_setinactivity_z(bool state)
*  @brief       Sets InActivity for Z Axis
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_setinactivity_z(bool state)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_ACT_INACT_CTL, 0, state);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_setinactivity_xyz(bool stateX, bool stateY,
*                                                                   bool stateZ)
*  @brief       Sets InActivity for XYZ Axis
*  @param[in]   stateX: State to change the register value for X axis
*  @param[in]   stateY: State to change the register value for Y axis
*  @param[in]   stateZ: State to change the register value for Z axis
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_setinactivity_xyz(bool stateX, bool stateY, bool stateZ)
{
  sl_si91x_adxl345_setinactivity_x(stateX);
  sl_si91x_adxl345_setinactivity_y(stateY);
  sl_si91x_adxl345_setinactivity_z(stateZ);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isactivity_ac()
*  @brief       Gets the status of the register isActivity
*  @param[in]   NULL
*  @return      Returns the status of the register
*******************************************************************************/
bool sl_si91x_adxl345_isactivity_ac()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_ACT_INACT_CTL, 7);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isinactivity_ac()
*  @brief       Gets the status of the register isActivity
*  @param[in]   NULL
*  @return      Returns the status of the register
*******************************************************************************/
bool sl_si91x_adxl345_isinactivity_ac()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_ACT_INACT_CTL, 3);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_activity_ac(bool state)
*  @brief       Sets the status of the Activity register with 0 or 1
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_activity_ac(bool state)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_ACT_INACT_CTL, 7, state);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_inactivity_ac(bool state)
*  @brief       Sets the status of the InActivity register with 0 or 1
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_inactivity_ac(bool state)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_ACT_INACT_CTL, 3, state);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_suppress_bit()
*  @brief       Gets the status of the Suppress bit register, which is used for
*               Axis Control for Tap/Double Tap
*  @param[in]   NULL
*  @return      Returns the register value
*******************************************************************************/
bool sl_si91x_adxl345_get_suppress_bit()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_TAP_AXES, 3);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_suppress_bit(bool state)
*  @brief       Sets the status of the Suppress bit register, which is used for
*               Axis Control for Tap/Double Tap
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_suppress_bit(bool state)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_TAP_AXES, 3, state);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_istapdetection_on_x()
*  @brief       Gets the status of the Tap Detection on X
*  @param[in]   NULL
*  @return      Returns the register value for Tap Detection
*******************************************************************************/
bool sl_si91x_adxl345_istapdetection_on_x()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_TAP_AXES, 2);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_tapdetection_on_x(bool state)
*  @brief       Sets the status of the Tap/Double Tap Detection on X
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_tapdetection_on_x(bool state)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_TAP_AXES, 2, state);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_istapdetection_on_y()
*  @brief       Gets the status of the Tap Detection on Y
*  @param[in]   NULL
*  @return      Returns the register value for Tap Detection on Y
*******************************************************************************/
bool sl_si91x_adxl345_istapdetection_on_y()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_TAP_AXES, 1);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_tapdetection_on_y(bool state)
*  @brief       Sets the status of the Tap/Double Tap Detection on Y
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_tapdetection_on_y(bool state)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_TAP_AXES, 1, state);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_istapdetection_on_z()
*  @brief       Gets the status of the Tap Detection on Z
*  @param[in]   NULL
*  @return      Returns the register value for Tap Detection on Z
*******************************************************************************/
bool sl_si91x_adxl345_istapdetection_on_z()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_TAP_AXES, 0);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_tapdetection_on_z(bool state)
*  @brief       Sets the status of the Tap/Double Tap Detection on Z
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_tapdetection_on_z(bool state)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_TAP_AXES, 0, state);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_tapdetection_on_xyz(bool stateX,
*                                                      bool stateY, bool stateZ)
*  @brief       Sets Tap Detection ON for XYZ Axis
*  @param[in]   stateX: State to change the register value for X axis
*  @param[in]   stateY: State to change the register value for Y axis
*  @param[in]   stateZ: State to change the register value for Z axis
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_tapdetection_on_xyz(bool stateX, bool stateY, bool stateZ)
{
  sl_si91x_adxl345_set_tapdetection_on_x(stateX);
  sl_si91x_adxl345_set_tapdetection_on_y(stateY);
  sl_si91x_adxl345_set_tapdetection_on_z(stateZ);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isactivitysource_on_x()
*  @brief       Gets the status of Actual Tap Status on X Axis
*  @param[in]   NULL
*  @return      Returns the register value of Actual Tap on X Axis
*******************************************************************************/
bool sl_si91x_adxl345_isactivitysource_on_x()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_ACT_TAP_STATUS, 6);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isactivitysource_on_y()
*  @brief       Gets the status of Actual Tap Status on Y Axis
*  @param[in]   NULL
*  @return      Returns the register value of Actual Tap on Y Axis
*******************************************************************************/
bool sl_si91x_adxl345_isactivitysource_on_y()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_ACT_TAP_STATUS, 5);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isactivitysource_on_z()
*  @brief       Gets the status of Actual Tap Status on Z Axis
*  @param[in]   NULL
*  @return      Returns the register value of Actual Tap on Z Axis
*******************************************************************************/
bool sl_si91x_adxl345_isactivitysource_on_z()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_ACT_TAP_STATUS, 4);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_istapsource_on_x()
*  @brief       Gets the status of Tap Source on X Axis
*  @param[in]   NULL
*  @return      Returns the register value of Tap Source on X Axis
*******************************************************************************/
bool sl_si91x_adxl345_istapsource_on_x()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_ACT_TAP_STATUS, 2);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_istapsource_on_y()
*  @brief       Gets the status of Tap Source on Y Axis
*  @param[in]   NULL
*  @return      Returns the register value of Tap Source on Y Axis
*******************************************************************************/
bool sl_si91x_adxl345_istapsource_on_y()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_ACT_TAP_STATUS, 1);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_istapsource_on_z()
*  @brief       Gets the status of Tap Source on Z Axis
*  @param[in]   NULL
*  @return      Returns the register value of Tap Source on Z Axis
*******************************************************************************/
bool sl_si91x_adxl345_istapsource_on_z()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_ACT_TAP_STATUS, 0);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isasleep()
*  @brief       Gets the status of Tap if it is Asleep
*  @param[in]   NULL
*  @return      Returns the register value of Tap Source if it is Asleep
*******************************************************************************/
bool sl_si91x_adxl345_isasleep()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_ACT_TAP_STATUS, 3);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_islowpower()
*  @brief       Gets the status of Data Rate and Power mode Control
*  @param[in]   NULL
*  @return      Returns the register value of sensor with its Power Mode
*******************************************************************************/
bool sl_si91x_adxl345_islowpower()
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_BW_RATE, 4);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_lowpower(bool state)
*  @brief       Sets the status of Power mode Control to low
*  @param[in]   state: to change the power mode to low
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_lowpower(bool state)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_BW_RATE, 4, state);
}

/*******************************************************************************
*  @fn          double sl_si91x_adxl345_get_rate()
*  @brief       Gets the status of Data Rate
*  @param[in]   NULL
*  @return      Returns the register value of sensor with its Data rate
*******************************************************************************/
double sl_si91x_adxl345_get_rate()
{
  uint8_t _b[2];
  uint32_t temp = 0;
  read(SL_ADXL345_BW_RATE, 1, _b);
  temp = _b[0] | (_b[1] << 8);
  temp &= 0x0F;
  return (pow(2, (temp)-6)) * 6.25;
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_rate(double rate)
*  @brief       Sets the Data Rate
*  @param[in]   rate: value to set the data rate
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_rate(double rate)
{
  uint8_t _b[2], _s;
  uint32_t temp = 0;
  int v         = (int)(rate / 6.25);
  int r         = 0;
  while (v >>= 1) {
    r++;
  }
  if (r <= 9) {
    read(SL_ADXL345_BW_RATE, 1, _b);
    temp = _b[0] | (_b[1] << 8);
    _s   = (uint8_t)(r + 6) | (temp & 0xF0);
    write(SL_ADXL345_BW_RATE, _s);
  }
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_bw(uint8_t bw_code)
*  @brief       Sets the Bandwidth value
*  @param[in]   bw_code: value to set the Bandwidth code
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_bw(int8_t bw_code)
{
  if ((bw_code < (uint8_t)SL_ADXL345_BW_0_05) || (bw_code > (uint8_t)SL_ADXL345_BW_1600)) {
    status1    = false;
    error_code = SL_ADXL345_BAD_ARG;
  } else {
    write(SL_ADXL345_BW_RATE, (uint8_t)bw_code);
  }
}

/*******************************************************************************
*  @fn          uint8_t sl_si91x_adxl345_get_bw_code()
*  @brief       Gets the Bandwidth value
*  @param[in]   NULL
*  @return      Returns the value of Bandwidth code
*******************************************************************************/
uint8_t sl_si91x_adxl345_get_bw_code()
{
  uint8_t bw_code;
  read(SL_ADXL345_BW_RATE, 1, &bw_code);
  return bw_code;
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_triggered(uint8_t interrupts, int mask)
*  @brief       Gets the status if Action was Triggered in Interrupts
*  @param[in]   interrupts: interrupt value
*  @param[in]   mask: The action type of interrupt triggered
*  @return      Returns the value if the action triggered
*******************************************************************************/
bool sl_si91x_adxl345_triggered(uint8_t interrupts_triggered, int mask)
{
  return ((interrupts_triggered >> mask) & 1);
}

/*******************************************************************************
*  @fn          uint8_t sl_si91x_adxl345_get_interrupt_source_val()
*  @brief       Gets the value of interrupt value
*  @param[in]   NULL
*  @return      Returns the value of the interrupt source
*******************************************************************************/
uint8_t sl_si91x_adxl345_get_interrupt_source_val()
{
  uint8_t _b;
  read(SL_ADXL345_INT_SOURCE, 0, &_b);
  return _b;
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_interrupt_source(uint8_t interruptBit)
*  @brief       Sets the interruptbit to the Interrupt source
*  @param[in]   interruptBit: bit value
*  @return      Returns the value of the register value
*******************************************************************************/
bool sl_si91x_adxl345_get_interrupt_source(uint8_t interruptBit)
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_INT_SOURCE, interruptBit);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_interrupt_mapping(uint8_t interruptBit)
*  @brief       Enables the interruptbit of the Interrupt Mapping Control
*  @param[in]   interruptBit: bit value
*  @return      Returns the value of the register value
*******************************************************************************/
bool sl_si91x_adxl345_get_interrupt_mapping(uint8_t interruptBit)
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_INT_MAP, interruptBit);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_interrupt_mapping(uint8_t interruptBit,
*                                                             bool interruptPin)
*  @brief       Sets the interrupt bit of the Interrupt Mapping Control to pin1
*               and pin2
*  @param[in]   interruptBit: bit value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_interrupt_mapping(uint8_t interruptBit, bool interruptPin)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_INT_MAP, interruptBit, interruptPin);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_important_interrupt_mapping
*               (int single_tap, int double_tap, int free_fall, int activity,
                                                                int inactivity)
*  @brief       Sets the interrupt bit of the Interrupt Mapping Control to pin1
*               and pin2, say particular interrupt to particular pin
*  @param[in]   single_tap: Set the interrupt to pin1 or pin2
*  @param[in]   double_tap: Set the interrupt to pin1 or pin2
*  @param[in]   free_fall: Set the interrupt to pin1 or pin2
*  @param[in]   activity: Set the interrupt to pin1 or pin2
*  @param[in]   inactivity: Set the interrupt to pin1 or pin2
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_important_interrupt_mapping(int single_tap,
                                                      int double_tap,
                                                      int free_fall,
                                                      int activity,
                                                      int inactivity)
{
  if (single_tap == 1) {
    sl_si91x_adxl345_set_interrupt_mapping(SL_ADXL345_INT_SINGLE_TAP_BIT, SL_ADXL345_INT1_PIN);
  } else if (single_tap == 2) {
    sl_si91x_adxl345_set_interrupt_mapping(SL_ADXL345_INT_SINGLE_TAP_BIT, SL_ADXL345_INT2_PIN);
  }

  if (double_tap == 1) {
    sl_si91x_adxl345_set_interrupt_mapping(SL_ADXL345_INT_DOUBLE_TAP_BIT, SL_ADXL345_INT1_PIN);
  } else if (double_tap == 2) {
    sl_si91x_adxl345_set_interrupt_mapping(SL_ADXL345_INT_DOUBLE_TAP_BIT, SL_ADXL345_INT2_PIN);
  }

  if (free_fall == 1) {
    sl_si91x_adxl345_set_interrupt_mapping(SL_ADXL345_INT_FREE_FALL_BIT, SL_ADXL345_INT1_PIN);
  } else if (free_fall == 2) {
    sl_si91x_adxl345_set_interrupt_mapping(SL_ADXL345_INT_FREE_FALL_BIT, SL_ADXL345_INT2_PIN);
  }

  if (activity == 1) {
    sl_si91x_adxl345_set_interrupt_mapping(SL_ADXL345_INT_ACTIVITY_BIT, SL_ADXL345_INT1_PIN);
  } else if (activity == 2) {
    sl_si91x_adxl345_set_interrupt_mapping(SL_ADXL345_INT_ACTIVITY_BIT, SL_ADXL345_INT2_PIN);
  }

  if (inactivity == 1) {
    sl_si91x_adxl345_set_interrupt_mapping(SL_ADXL345_INT_INACTIVITY_BIT, SL_ADXL345_INT1_PIN);
  } else if (inactivity == 2) {
    sl_si91x_adxl345_set_interrupt_mapping(SL_ADXL345_INT_INACTIVITY_BIT, SL_ADXL345_INT2_PIN);
  }
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_is_interrupt_enabled(uint8_t interruptBit)
*  @brief       Gets the status of Interrupt Enable Control register
*  @param[in]   interrupts: interrupt value
*  @return      Returns the value of the Enable Control Register
*******************************************************************************/
bool sl_si91x_adxl345_is_interrupt_enabled(uint8_t interruptBit)
{
  return sl_si91x_adxl345_get_registerbit(SL_ADXL345_INT_ENABLE, interruptBit);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_is_interrupt_enabled(uint8_t interruptBit)
*  @brief       Sets the Interrupt Enable Control register
*  @param[in]   interrupts: interrupt value
*  @param[in]   state: the state of interrupt
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_interrupt(uint8_t interruptBit, bool state)
{
  sl_si91x_adxl345_set_registerbit(SL_ADXL345_INT_ENABLE, interruptBit, state);
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_singletap_interrupt(bool status)
*  @brief       Sets the Single tap Interrupt bit ON or OFF
*  @param[in]   status: 1 to set and 0 to clear
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_singletap_interrupt(bool status)
{
  if (status) {
    sl_si91x_adxl345_set_interrupt(SL_ADXL345_INT_SINGLE_TAP_BIT, 1);
  } else {
    sl_si91x_adxl345_set_interrupt(SL_ADXL345_INT_SINGLE_TAP_BIT, 0);
  }
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_doubletap_interrupt(bool status)
*  @brief       Sets the Double tap Interrupt bit ON or OFF
*  @param[in]   status: 1 to set and 0 to clear
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_doubletap_interrupt(bool status)
{
  if (status) {
    sl_si91x_adxl345_set_interrupt(SL_ADXL345_INT_DOUBLE_TAP_BIT, 1);
  } else {
    sl_si91x_adxl345_set_interrupt(SL_ADXL345_INT_DOUBLE_TAP_BIT, 0);
  }
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_freefall_interrupt(bool status)
*  @brief       Sets the Free Fall Interrupt bit ON or OFF
*  @param[in]   status: 1 to set and 0 to clear
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_freefall_interrupt(bool status)
{
  if (status) {
    sl_si91x_adxl345_set_interrupt(SL_ADXL345_INT_FREE_FALL_BIT, 1);
  } else {
    sl_si91x_adxl345_set_interrupt(SL_ADXL345_INT_FREE_FALL_BIT, 0);
  }
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_activity_interrupt(bool status)
*  @brief       Sets the Activity Interrupt bit ON or OFF
*  @param[in]   status: 1 to set and 0 to clear
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_activity_interrupt(bool status)
{
  if (status) {
    sl_si91x_adxl345_set_interrupt(SL_ADXL345_INT_ACTIVITY_BIT, 1);
  } else {
    sl_si91x_adxl345_set_interrupt(SL_ADXL345_INT_ACTIVITY_BIT, 0);
  }
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_inactivity_interrupt(bool status)
*  @brief       Sets the InActivity Interrupt bit ON or OFF
*  @param[in]   status: 1 to set and 0 to clear
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_inactivity_interrupt(bool status)
{
  if (status) {
    sl_si91x_adxl345_set_interrupt(SL_ADXL345_INT_INACTIVITY_BIT, 1);
  } else {
    sl_si91x_adxl345_set_interrupt(SL_ADXL345_INT_INACTIVITY_BIT, 0);
  }
}

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_registerbit(uint8_t regAdress,
*                                                       int bitPos, bool state)
*  @brief       Sets the InActivity Interrupt bit ON or OFF
*  @param[in]   regAdress: Register Address bit
*  @param[in]   bitPos: bit position
*  @param[in]   state: state of the bit position
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_registerbit(uint8_t regAdress, uint8_t bitPos, bool state)
{
  uint8_t _b;
  read(regAdress, 0, &_b);
  if (state) {
    _b |= (1 << bitPos); // Forces nth Bit of _b to 1. Other Bits Unchanged.
  } else {
    _b &= (~(1 << bitPos)); // Forces nth Bit of _b to 0. Other Bits Unchanged.
  }
  write(regAdress, _b);
}

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_registerbit(uint8_t regAdress,
*                                                                    int bitPos)
*  @brief       Gets the register bit
*  @param[in]   regAdress: Register Address bit
*  @param[in]   bitPos: bit position
*  @return      Returns the state of the register bit
*******************************************************************************/
bool sl_si91x_adxl345_get_registerbit(uint8_t regAdress, uint8_t bitPos)
{
  uint8_t _b;
  read(regAdress, 0, &_b);
  return ((_b >> bitPos) & 1);
}
