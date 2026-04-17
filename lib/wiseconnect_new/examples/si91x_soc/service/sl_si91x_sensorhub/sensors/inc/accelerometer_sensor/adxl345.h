/***************************************************************************/ /**
 * @file adxl345.h
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

#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_
#include <stdbool.h>
#include "rsi_data_types.h"
#include "sensors_config.h"
#include "sensor_type.h"

/*******************************************************************************
 ************************* ADXL Typedefs/ Defines  *****************************
 ******************************************************************************/
typedef void *sl_adxl345_handle_t;
typedef void *sl_spi_bus_device_handle_t;
typedef int32_t sl_adxl_err_t;

typedef enum {
  SL_ADXL345_CONTINUE_1LX_RES    = 0x10, /*!< Command to set measure mode as Continuously H-Resolution mode*/
  SL_ADXL345_CONTINUE_HALFLX_RES = 0x11, /*!< Command to set measure mode as Continuously H-Resolution mode2*/
  SL_ADXL345_CONTINUE_4LX_RES    = 0x13, /*!< Command to set measure mode as Continuously L-Resolution mode*/
  SL_ADXL345_ONETIME_1LX_RES     = 0x20, /*!< Command to set measure mode as One Time H-Resolution mode*/
  SL_ADXL345_ONETIME_HALFLX_RES  = 0x21, /*!< Command to set measure mode as One Time H-Resolution mode2*/
  SL_ADXL345_ONETIME_4LX_RES     = 0x23, /*!< Command to set measure mode as One Time L-Resolution mode*/
} sl_adxl345_cmd_measure_t;

/*************************** REGISTER MAP ***************************/
#define SL_ADXL345_DEVID          0x00 // Device ID
#define SL_ADXL345_RESERVED1      0x01 // Reserved. Do Not Access.
#define SL_ADXL345_THRESH_TAP     0x1D // Tap Threshold.
#define SL_ADXL345_OFSX           0x1E // X-Axis Offset.
#define SL_ADXL345_OFSY           0x1F // Y-Axis Offset.
#define SL_ADXL345_OFSZ           0x20 // Z- Axis Offset.
#define SL_ADXL345_DUR            0x21 // Tap Duration.
#define SL_ADXL345_LATENT         0x22 // Tap Latency.
#define SL_ADXL345_WINDOW         0x23 // Tap Window.
#define SL_ADXL345_THRESH_ACT     0x24 // Activity Threshold
#define SL_ADXL345_THRESH_INACT   0x25 // Inactivity Threshold
#define SL_ADXL345_TIME_INACT     0x26 // Inactivity Time
#define SL_ADXL345_ACT_INACT_CTL  0x27 // Axis Enable Control for Activity and Inactivity Detection
#define SL_ADXL345_THRESH_FF      0x28 // Free-Fall Threshold.
#define SL_ADXL345_TIME_FF        0x29 // Free-Fall Time.
#define SL_ADXL345_TAP_AXES       0x2A // Axis Control for Tap/Double Tap.
#define SL_ADXL345_ACT_TAP_STATUS 0x2B // Source of Tap/Double Tap
#define SL_ADXL345_BW_RATE        0x2C // Data Rate and Power mode Control
#define SL_ADXL345_POWER_CTL      0x2D // Power-Saving Features Control
#define SL_ADXL345_INT_ENABLE     0x2E // Interrupt Enable Control
#define SL_ADXL345_INT_MAP        0x2F // Interrupt Mapping Control
#define SL_ADXL345_INT_SOURCE     0x30 // Source of Interrupts
#define SL_ADXL345_DATA_FORMAT    0x31 // Data Format Control
#define SL_ADXL345_DATAX0         0x32 // X-Axis Data 0
#define SL_ADXL345_DATAX1         0x33 // X-Axis Data 1
#define SL_ADXL345_DATAY0         0x34 // Y-Axis Data 0
#define SL_ADXL345_DATAY1         0x35 // Y-Axis Data 1
#define SL_ADXL345_DATAZ0         0x36 // Z-Axis Data 0
#define SL_ADXL345_DATAZ1         0x37 // Z-Axis Data 1
#define SL_ADXL345_FIFO_CTL       0x38 // FIFO Control
#define SL_ADXL345_FIFO_STATUS    0x39 // FIFO Status

#define SL_ADXL345_BW_1600 0xF // 1111   IDD = 40uA
#define SL_ADXL345_BW_800  0xE // 1110   IDD = 90uA
#define SL_ADXL345_BW_400  0xD // 1101   IDD = 140uA
#define SL_ADXL345_BW_200  0xC // 1100   IDD = 140uA
#define SL_ADXL345_BW_100  0xB // 1011   IDD = 140uA
#define SL_ADXL345_BW_50   0xA // 1010   IDD = 140uA
#define SL_ADXL345_BW_25   0x9 // 1001   IDD = 90uA
#define SL_ADXL345_BW_12_5 0x8 // 1000   IDD = 60uA
#define SL_ADXL345_BW_6_25 0x7 // 0111   IDD = 50uA
#define SL_ADXL345_BW_3_13 0x6 // 0110   IDD = 45uA
#define SL_ADXL345_BW_1_56 0x5 // 0101   IDD = 40uA
#define SL_ADXL345_BW_0_78 0x4 // 0100   IDD = 34uA
#define SL_ADXL345_BW_0_39 0x3 // 0011   IDD = 23uA
#define SL_ADXL345_BW_0_20 0x2 // 0010   IDD = 23uA
#define SL_ADXL345_BW_0_10 0x1 // 0001   IDD = 23uA
#define SL_ADXL345_BW_0_05 0x0 // 0000   IDD = 23uA

/************************** INTERRUPT PINS **************************/
#define SL_ADXL345_INT1_PIN 0x00 //INT1: 0
#define SL_ADXL345_INT2_PIN 0x01 //INT2: 1

/********************** INTERRUPT BIT POSITION **********************/
#define SL_ADXL345_INT_DATA_READY_BIT 0x07
#define SL_ADXL345_INT_SINGLE_TAP_BIT 0x06
#define SL_ADXL345_INT_DOUBLE_TAP_BIT 0x05
#define SL_ADXL345_INT_ACTIVITY_BIT   0x04
#define SL_ADXL345_INT_INACTIVITY_BIT 0x03
#define SL_ADXL345_INT_FREE_FALL_BIT  0x02
#define SL_ADXL345_INT_WATERMARK_BIT  0x01
#define SL_ADXL345_INT_OVERRUNY_BIT   0x00

#define SL_ADXL345_DATA_READY 0x07
#define SL_ADXL345_SINGLE_TAP 0x06
#define SL_ADXL345_DOUBLE_TAP 0x05
#define SL_ADXL345_ACTIVITY   0x04
#define SL_ADXL345_INACTIVITY 0x03
#define SL_ADXL345_FREE_FALL  0x02
#define SL_ADXL345_WATERMARK  0x01
#define SL_ADXL345_OVERRUNY   0x00

/****************************** ERRORS ******************************/
#define SL_ADXL345_OK    1 // No Error
#define SL_ADXL345_ERROR 0 // Error Exists

#define SL_ADXL345_NO_ERROR   0 // Initial State
#define SL_ADXL345_READ_ERROR 1 // Accelerometer Reading Error
#define SL_ADXL345_BAD_ARG    2 // Bad Argument

#define compare(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

/*******************************************************************************
 ******************** Global Structure for ADXL Sensor  ************************
 ******************************************************************************/
typedef struct {
  int32_t spi_dev;
  uint8_t adx_dev_addr;
} sl_adxl345_dev_t;

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*  @fn          sl_adxl345_handle_t sl_si91x_adxl345_create
*                                                      (sl_sensor_bus_t bus)
*  @brief       Creates ADXL345 sensor in the Sensor Hub.
*  @param[in]   bus:  I2C bus handle
*  @return      Returns the ADXL345 sensor handle
*******************************************************************************/
sl_adxl345_handle_t sl_si91x_adxl345_create(sl_sensor_bus_t bus);

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_adxl345_delete(sl_adxl345_handle_t *sensor)
*  @brief       Deletes ADXL345 sensor handle in the Sensor Hub.
*  @param[in]   sensor: ADXL345 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_adxl_err_t sl_si91x_adxl345_delete(sl_adxl345_handle_t *sensor);

/*******************************************************************************
*  @fn          sl_adxl_err_t sl_si91x_adxl345_set_measure_mode(
*               sl_adxl345_handle_t sensor,sl_adxl345_cmd_measure_t cmd_measure)
*  @brief       Sets the Measure Mode
*  @param[in]   sensor: ADXL345 Sensor handle
*  @param[in]   cmd_measure Command Measure
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_adxl_err_t sl_si91x_adxl345_set_measure_mode(sl_adxl345_handle_t sensor, sl_adxl345_cmd_measure_t cmd_measure);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_isr(void)
*  @brief       Looks for Interrupts and Triggered Action
*  @param[in]   NULL
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_isr(void);

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_adxl345_get_data(sl_adxl345_handle_t sensor,
*                                                          float adxl345_data[])
*  @brief       Data from the sensor is received and calculated and stored
*  @param[in]   sensor: ADXL345 Sensor handle
*  @param[in]   data: Processed ADXL345 data
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_adxl_err_t sl_si91x_adxl345_get_data(sl_adxl345_handle_t sensor, float adxl345_data[]);

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_adxl345_init(sl_sensor_bus_t spi_bus)
*  @brief       Initializing the ADXL345 sensor in the Sensor Hub.
*  @param[in]   spi_bus: ADXL345 Sensor I2C Bus handle
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_adxl_err_t sl_si91x_adxl345_init(sl_sensor_bus_t handle);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_power_on(void)
*  @brief       Power ON the ADXL345 sensor and sets interrupts in the ADXL345.
*  @param[in]   sensor: ADXL345 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
void sl_si91x_adxl345_power_on(void);

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_adxl345_deinit(void)
*  @brief       DeInitializing the ADXL345 sensor in the Sensor Hub.
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_adxl_err_t sl_si91x_adxl345_deinit(void);

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_adxl345_test(void)
*  @brief       Testing the ADXL345 sensor in the Sensor Hub.
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_adxl_err_t sl_si91x_adxl345_test(void);

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
sl_adxl_err_t sl_si91x_adxl345_sample_accelerometer(float *x, float *y, float *z);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_gain(void)
*  @brief       Set Original Gains
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
void sl_si91x_adxl345_gain(void);
void sl_si91x_adxl345_power_on(void);
/*******************************************************************************
*  @fn          void sl_si91x_adxl345_read_acceleration_value(int *xyz)
*  @brief       Reads Acceleration into Three Variables: x, y and z
*  @param[in]   xyz: variable in which acceleration vale is stored
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
void sl_si91x_adxl345_read_acceleration_value(int *xyz);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_read_acceleration(int *a, int *b, int *c)
*  @brief       Reads and stores processes Acceleration value into three
*               variables: a, b and c
*  @param[in]   a, b, c: Axis at all g ranges with 10 Bit Resolution (2 Bytes)
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
void sl_si91x_adxl345_read_acceleration(int16_t *read_a, int16_t *read_b, int16_t *read_c);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_get_Gxyz(double *xyz)
*  @brief       Reads and stores Acceleration value into variable xyz_int and
*               processes with gain value
*  @param[in]   xyz: Axis value at g ranges
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
void sl_si91x_adxl345_get_Gxyz(double *xyz);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_tapthreshold(int tapThreshold)
*  @brief       Setting Tap Threshold value between 0 and 255. Scale Factor is
*               62.5 mg/LSB. A Value of 0 May Result in Undesirable Behavior
*  @param[in]   tapThreshold: value to be set in Tap Threshold value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_tapthreshold(int tapthreshold);

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_tapthreshold()
*  @brief       Gets the value of Tap Threshold value.
*               Scale Factor is 62.5 mg/LSB
*  @param[in]   NULL
*  @return      Returns Value Between 0 and 255
*******************************************************************************/
int sl_si91x_adxl345_get_tapthreshold();

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_axisgains(double *_gains)
*  @brief       Sets the Axis gain value for each Axis in Gs/Count
*  @param[in]   _gains: gain value for each Axis in Gs/Count
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_axisgains(double *_gains);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_get_axisgains(double *_gains)
*  @brief       Gets the AXis gain value for each Axis in Gs/Count.
*  @param[in]   _gains: gain value for each Axis in Gs/Count
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_get_axisgains(double *_gains);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_axisoffset(int x, int y, int z)
*  @brief       Sets the User Offset Adjustments in Twos Complement Format.
*               Scale Factor of 15.6mg/LSB
*  @param[in]   _gains: gain value for each Axis in Gs/Count
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_axisoffset(uint8_t x, uint8_t y, uint8_t z);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_get_axisoffset(int *x, int *y, int *z)
*  @brief       Gets the User Offset Adjustments in Twos Complement Format.
*               Scale Factor of 15.6mg/LSB
*  @param[in]   x, y, z: Axis offset value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_get_axisoffset(int *x, int *y, int *z);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_tapduration(int tapDuration)
*  @brief       Sets an Unsigned Time Value Representing the Max Time
*               that an Event must be Above the THRESH_TAP Threshold to qualify
*               as a Tap Event. The scale factor is 625�s/LSB. Value of 0
*               Disables the Tap/Double Tap Functions. Max value is 255.
*  @param[in]   tapDuration: Maximum Time for a tap event
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_tapduration(int tapDuration);

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_tapduration()
*  @brief       Gets an Tap/Double Tap duration
*  @param[in]   NULL
*  @return      Returns tap duration value
*******************************************************************************/
int sl_si91x_adxl345_get_tapduration();

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_doubletaplatency(int doubleTapLatency)
*  @brief       Sets the Latent register. Wait Time from the Detection of a Tap
*               Event to the Start of the Time Window (defined by the Window
*               Register) during which a possible Second Tap Even can be
*               Detected. Scale Factor is 1.25ms/LSB.
*               A Value of 0 Disables the Double Tap Function.
*               It Accepts a Maximum Value of 255.
*  @param[in]   doubleTapLatency: Latency time
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_doubletaplatency(int doubleTapLatency);

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_doubletaplatency()
*  @brief       Gets latent register value
*  @param[in]   doubleTapLatency: Latency time
*  @return      Returns the Latent register value
*******************************************************************************/
int sl_si91x_adxl345_get_doubletaplatency();

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_doubletapwindow(int doubleTapWindow)
*  @brief       Sets the the Amount of Time After the Expiration of the Latency
*               Time (determined by Latent register) during which a second valid
*               tap can Begin.
*               Scale Factor is 1.25ms/LSB.
*               A Value of 0 Disables the Double Tap Function.
*               It Accepts a Maximum Value of 255.
*  @param[in]   doubleTapWindow: Double tap Window Register
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_doubletapwindow(int doubleTapWindow);

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_doubletapwindow())
*  @brief       Gets the value of Window Register
*  @param[in]   NULL
*  @return      Returns the value of Window Register
*******************************************************************************/
int sl_si91x_adxl345_get_doubletapwindow();

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
void sl_si91x_adxl345_set_activitythreshold(int activityThreshold);

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_activitythreshold()
*  @brief       Gets the THRESH_ACT byte
*  @param[in]   NULL
*  @return      Returns the value THRESH_ACT
*******************************************************************************/
int sl_si91x_adxl345_get_activitythreshold();

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
void sl_si91x_adxl345_set_inactivitythreshold(int inactivityThreshold);

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_inactivitythreshold()
*  @brief       Gets the Threshold Inactive register value
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_THRESH_INACT
*******************************************************************************/
int sl_si91x_adxl345_get_inactivitythreshold();

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
void sl_si91x_adxl345_set_timeinactivity(int timeInactivity);

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_timeinactivity()
*  @brief       Gets the Time Inactivity register value
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_TIME_INACT
*******************************************************************************/
int sl_si91x_adxl345_get_timeinactivity();

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
void sl_si91x_adxl345_set_freefallthreshold(int freeFallthreshold);

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_freefallthreshold()
*  @brief       Gets the Free Fall threshold register value
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_THRESH_FF register
*******************************************************************************/
int sl_si91x_adxl345_get_freefallthreshold();

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
void sl_si91x_adxl345_set_freefallduration(int freeFallDuration);

/*******************************************************************************
*  @fn          int sl_si91x_adxl345_get_freefallduration()
*  @brief       Gets the Free Fall Duration register value
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_TIME_FF register
*******************************************************************************/
int sl_si91x_adxl345_get_freefallduration();

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isactivity_x_enabled()
*  @brief       Enable X Axis Enable Control for Activity & Inactivity Detection
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_ACT_INACT_CTL register bit
*******************************************************************************/
bool sl_si91x_adxl345_isactivity_x_enabled();

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isactivity_y_enabled()
*  @brief       Enable Y Axis: Enable Control for Activity & Inactivity Detection
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_ACT_INACT_CTL register bit
*******************************************************************************/
bool sl_si91x_adxl345_isactivity_y_enabled();

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isactivity_z_enabled()
*  @brief       Enable Z Axis: Enable Control for Activity & Inactivity Detection
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_ACT_INACT_CTL register bit
*******************************************************************************/
bool sl_si91x_adxl345_isactivity_z_enabled();

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isinactivity_x_enabled()
*  @brief       Disable X Axis Enable Control for Activity & Inactivity Detection
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_ACT_INACT_CTL register bit
*******************************************************************************/
bool sl_si91x_adxl345_isinactivity_x_enabled();

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isinactivity_y_enabled()
*  @brief       Disable Y Axis: Enable Control for Activity & Inactivity Detection
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_ACT_INACT_CTL register bit
*******************************************************************************/
bool sl_si91x_adxl345_isinactivity_y_enabled();

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isinactivity_z_enabled()
*  @brief       Disable Z Axis: Enable Control for Activity & Inactivity Detection
*  @param[in]   NULL
*  @return      Returns the value SL_ADXL345_ACT_INACT_CTL register bit
*******************************************************************************/
bool sl_si91x_adxl345_isinactivity_z_enabled();

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isactivity_ac()
*  @brief       Gets the status of the register isActivity
*  @param[in]   NULL
*  @return      Returns the status of the register
*******************************************************************************/
bool sl_si91x_adxl345_isactivity_ac();

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isinactivity_ac()
*  @brief       Gets the status of the register isActivity
*  @param[in]   NULL
*  @return      Returns the status of the register
*******************************************************************************/
bool sl_si91x_adxl345_isinactivity_ac();

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_activity_ac(bool state)
*  @brief       Sets the status of the Activity register with 0 or 1
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_activity_ac(bool state);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_inactivity_ac(bool state)
*  @brief       Sets the status of the InActivity register with 0 or 1
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_inactivity_ac(bool state);

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_suppress_bit()
*  @brief       Gets the status of the Suppress bit register, which is used for
*               Axis Control for Tap/Double Tap
*  @param[in]   NULL
*  @return      Returns the register value
*******************************************************************************/
bool sl_si91x_adxl345_get_suppress_bit();

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_suppress_bit(bool state)
*  @brief       Sets the status of the Suppress bit register, which is used for
*               Axis Control for Tap/Double Tap
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_suppress_bit(bool state);

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_istapdetection_on_x()
*  @brief       Gets the status of the Tap Detection on X
*  @param[in]   NULL
*  @return      Returns the register value for Tap Detection
*******************************************************************************/
bool sl_si91x_adxl345_istapdetection_on_x();

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_tapdetection_on_x(bool state)
*  @brief       Sets the status of the Tap/Double Tap Detection on X
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_tapdetection_on_x(bool state);

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_istapdetection_on_y()
*  @brief       Gets the status of the Tap Detection on Y
*  @param[in]   NULL
*  @return      Returns the register value for Tap Detection on Y
*******************************************************************************/
bool sl_si91x_adxl345_istapdetection_on_y();

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_tapdetection_on_y(bool state)
*  @brief       Sets the status of the Tap/Double Tap Detection on Y
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_tapdetection_on_y(bool state);

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_istapdetection_on_z()
*  @brief       Gets the status of the Tap Detection on Z
*  @param[in]   NULL
*  @return      Returns the register value for Tap Detection on Z
*******************************************************************************/
bool sl_si91x_adxl345_istapdetection_on_z();

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_tapdetection_on_z(bool state)
*  @brief       Sets the status of the Tap/Double Tap Detection on Z
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_tapdetection_on_z(bool state);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_tapdetection_on_xyz(bool stateX,
*                                                      bool stateY, bool stateZ)
*  @brief       Sets Tap Detection ON for XYZ Axis
*  @param[in]   stateX: State to change the register value for X axis
*  @param[in]   stateY: State to change the register value for Y axis
*  @param[in]   stateZ: State to change the register value for Z axis
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_tapdetection_on_xyz(bool stateX, bool stateY, bool stateZ);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_setactivity_x(bool state)
*  @brief       Sets Activity for X Axis
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_setactivity_x(bool state);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_setactivity_y(bool state)
*  @brief       Sets Activity for Y Axis
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_setactivity_y(bool state);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_setactivity_z(bool state)
*  @brief       Sets Activity for Z Axis
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_setactivity_z(bool state);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_setactivity_xyz(bool stateX, bool stateY,
*                                                                   bool stateZ)
*  @brief       Sets Activity for XYZ Axis
*  @param[in]   stateX: State to change the register value for X axis
*  @param[in]   stateY: State to change the register value for Y axis
*  @param[in]   stateZ: State to change the register value for Z axis
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_setactivity_xyz(bool stateX, bool stateY, bool stateZ);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_setinactivity_x(bool state)
*  @brief       Sets InActivity for X Axis
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_setinactivity_x(bool state);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_setinactivity_y(bool state)
*  @brief       Sets InActivity for Y Axis
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_setinactivity_y(bool state);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_setinactivity_z(bool state)
*  @brief       Sets InActivity for Z Axis
*  @param[in]   state: State to change the register value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_setinactivity_z(bool state);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_setinactivity_xyz(bool stateX, bool stateY,
*                                                                   bool stateZ)
*  @brief       Sets InActivity for XYZ Axis
*  @param[in]   stateX: State to change the register value for X axis
*  @param[in]   stateY: State to change the register value for Y axis
*  @param[in]   stateZ: State to change the register value for Z axis
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_setinactivity_xyz(bool stateX, bool stateY, bool stateZ);

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isactivitysource_on_x()
*  @brief       Gets the status of Actual Tap Status on X Axis
*  @param[in]   NULL
*  @return      Returns the register value of Actual Tap on X Axis
*******************************************************************************/
bool sl_si91x_adxl345_isactivitysource_on_x();

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isactivitysource_on_y()
*  @brief       Gets the status of Actual Tap Status on Y Axis
*  @param[in]   NULL
*  @return      Returns the register value of Actual Tap on Y Axis
*******************************************************************************/
bool sl_si91x_adxl345_isactivitysource_on_y();

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isactivitysource_on_z()
*  @brief       Gets the status of Actual Tap Status on Z Axis
*  @param[in]   NULL
*  @return      Returns the register value of Actual Tap on Z Axis
*******************************************************************************/
bool sl_si91x_adxl345_isactivitysource_on_z();

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_istapsource_on_x()
*  @brief       Gets the status of Tap Source on X Axis
*  @param[in]   NULL
*  @return      Returns the register value of Tap Source on X Axis
*******************************************************************************/
bool sl_si91x_adxl345_istapsource_on_x();

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_istapsource_on_y()
*  @brief       Gets the status of Tap Source on Y Axis
*  @param[in]   NULL
*  @return      Returns the register value of Tap Source on Y Axis
*******************************************************************************/
bool sl_si91x_adxl345_istapsource_on_y();

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_istapsource_on_z()
*  @brief       Gets the status of Tap Source on Z Axis
*  @param[in]   NULL
*  @return      Returns the register value of Tap Source on Z Axis
*******************************************************************************/
bool sl_si91x_adxl345_istapsource_on_z();

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_isasleep()
*  @brief       Gets the status of Tap if it is Asleep
*  @param[in]   NULL
*  @return      Returns the register value of Tap Source if it is Asleep
*******************************************************************************/
bool sl_si91x_adxl345_isasleep();

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_islowpower()
*  @brief       Gets the status of Data Rate and Power mode Control
*  @param[in]   NULL
*  @return      Returns the register value of sensor with its Power Mode
*******************************************************************************/
bool sl_si91x_adxl345_islowpower();

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_lowpower(bool state)
*  @brief       Sets the status of Power mode Control to low
*  @param[in]   state: to change the power mode to low
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_lowpower(bool state);

/*******************************************************************************
*  @fn          double sl_si91x_adxl345_get_rate()
*  @brief       Gets the status of Data Rate
*  @param[in]   NULL
*  @return      Returns the register value of sensor with its Data rate
*******************************************************************************/
double sl_si91x_adxl345_get_rate();

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_rate(double rate)
*  @brief       Sets the Data Rate
*  @param[in]   rate: value to set the data rate
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_rate(double rate);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_bw(uint8_t bw_code)
*  @brief       Sets the Bandwidth value
*  @param[in]   bw_code: value to set the Bandwidth code
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_bw(int8_t bw_code);

/*******************************************************************************
*  @fn          uint8_t sl_si91x_adxl345_get_bw_code()
*  @brief       Gets the Bandwidth value
*  @param[in]   NULL
*  @return      Returns the value of Bandwidth code
*******************************************************************************/
uint8_t sl_si91x_adxl345_get_bw_code();

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_triggered(uint8_t interrupts, int mask)
*  @brief       Gets the status if Action was Triggered in Interrupts
*  @param[in]   interrupts: interrupt value
*  @param[in]   mask: The action type of interrupt triggered
*  @return      Returns the value if the action triggered
*******************************************************************************/
bool sl_si91x_adxl345_triggered(uint8_t interrupts_triggered, int mask);

/*******************************************************************************
*  @fn          buint8_t sl_si91x_adxl345_get_interrupt_source_val()
*  @brief       Gets the value of interrupt value
*  @param[in]   NULL
*  @return      Returns the value of the interrupt source
*******************************************************************************/
uint8_t sl_si91x_adxl345_get_interrupt_source_val();

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_interrupt_source(uint8_t interruptBit)
*  @brief       Sets the interruptbit to the Interrupt source
*  @param[in]   interruptBit:
*  @return      Returns the value of the register value
*******************************************************************************/
bool sl_si91x_adxl345_get_interrupt_source(uint8_t interruptBit);

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_interrupt_mapping(uint8_t interruptBit)
*  @brief       Sets the interruptbit to the Interrupt Mapping Control
*  @param[in]   interruptBit: bit value
*  @return      Returns the value of the register value
*******************************************************************************/
bool sl_si91x_adxl345_get_interrupt_mapping(uint8_t interruptBit);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_interrupt_mapping(uint8_t interruptBit,
*                                                             bool interruptPin)
*  @brief       Sets the interrupt bit of the Interrupt Mapping Control to pin1
*               and pin2
*  @param[in]   interruptBit: bit value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_interrupt_mapping(uint8_t interruptBit, bool sl_si91x_adxl345_interruptPin);

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_is_interrupt_enabled(uint8_t interruptBit)
*  @brief       Gets the status of Interrupt Enable Control register
*  @param[in]   interrupts: interrupt value
*  @return      Returns the value of the Enable Control Register
*******************************************************************************/
bool sl_si91x_adxl345_is_interrupt_enabled(uint8_t interruptBit);

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_is_interrupt_enabled(uint8_t interruptBit)
*  @brief       Sets the Interrupt Enable Control register
*  @param[in]   interrupts: interrupt value
*  @param[in]   state: the state of interrupt
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_interrupt(uint8_t interruptBit, bool state);

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
                                                      int inactivity);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_inactivity_interrupt(bool status)
*  @brief       Sets the InActivity Interrupt bit ON or OFF
*  @param[in]   status: 1 to set and 0 to clear
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_inactivity_interrupt(bool status);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_activity_interrupt(bool status)
*  @brief       Sets the Activity Interrupt bit ON or OFF
*  @param[in]   status: 1 to set and 0 to clear
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_activity_interrupt(bool status);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_freefall_interrupt(bool status)
*  @brief       Sets the Free Fall Interrupt bit ON or OFF
*  @param[in]   status: 1 to set and 0 to clear
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_freefall_interrupt(bool status);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_doubletap_interrupt(bool status)
*  @brief       Sets the Double tap Interrupt bit ON or OFF
*  @param[in]   status: 1 to set and 0 to clear
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_doubletap_interrupt(bool status);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_singletap_interrupt(bool status)
*  @brief       Sets the Single tap Interrupt bit ON or OFF
*  @param[in]   status: 1 to set and 0 to clear
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_singletap_interrupt(bool status);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_get_range_setting(uint8_t *rangeSetting)
*  @brief       Gets the range setting value
*  @param[in]   rangeSetting: Get the range value (2g, 4g, 8g, 16g)
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_get_range_setting(uint8_t *rangeSetting);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_range_setting(int val)
*  @brief       Sets the range setting value
*  @param[in]   val: Sets the range value as 2g/ 4g/ 8g/ 16g
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_range_setting(int val);

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_selftest_bit()
*  @brief       Gets the Self test bit value: 1 for Self-Test Applied.
*               0 for Self-Test Disabled.
*  @param[in]   NULL
*  @return      Returns self Test bit value
*******************************************************************************/
bool sl_si91x_adxl345_get_selftest_bit();

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_selftest_bit(bool selfTestBit)
*  @brief       Sets the Self test bit value: Set 1 for Self-Test Applied.
*               Electrostatic Force exerted on the sensor causing a shift in the
*               output data. Set 0 for Self-Test Disabled.
*  @param[in]   selfTestBit: value to be set in selfTestBit
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_selftest_bit(bool selfTestBit);

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_spi_bit()
*  @brief       Gets the SPI bit value. 1 for 3 wire mode and 0 for 4 wire SPI
*               mode
*  @param[in]   NULL
*  @return      Returns spiBit value
*******************************************************************************/
bool sl_si91x_adxl345_get_spi_bit();

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_spi_bit(bool spiBit)
*  @brief       Sets the SPI bit value. Set to 1 to put the device in 3-wire
*               Mode and set to 0 to put the device in 4-wire SPI Mode
*  @param[in]   spiBit: value to be set in spiBit
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_spi_bit(bool spiBit);

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_interruptlevel_bit()
*  @brief       Gets the Interrupt level bit
*  @param[in]   NULL
*  @return      Returns Interrupt level bit
*******************************************************************************/
bool sl_si91x_adxl345_get_interruptlevel_bit();

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_interrupt_level_bit
*                                                       (bool interruptLevelBit)
*  @brief       Set to 1 to make the Interrupts Active
*               HIGH and set to 0 to make the Interrupts Active LOW
*  @param[in]   interruptLevelBit: value to be set in interruptLevelBit
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_interrupt_level_bit(bool sl_si91x_adxl345_interruptLevelBit);

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_full_resolution_bit()
*  @brief       Gets the full resolution bit value: 1 is when Full Resolution
*               Mode and and 0 is when Device is in 10-bit Mode
*  @param[in]   NULL
*  @return      Returns full resolution bit value
*******************************************************************************/
bool sl_si91x_adxl345_get_full_resolution_bit();

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
void sl_si91x_adxl345_set_full_resolution_bit(bool fullResBit);

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_justify_bit()
*  @brief       Gets the Justify bit value. 1 is Left Justified mode and 0 is
*               Right Justified Mode with Sign Extension
*  @param[in]   NULL
*  @return      Returns Justify bit value
*******************************************************************************/
bool sl_si91x_adxl345_get_justify_bit();

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_justify_bit(bool justifyBit)
*  @brief       Setting to 1 Selects the Left Justified Mode and
*               Setting to 1 Selects Right Justified Mode with Sign Extension
*  @param[in]   justifyBit: value to be set in Justify bit value
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_justify_bit(bool justifyBit);

/*******************************************************************************
*  @fn          void write(uint8_t address, uint8_t value)
*  @brief       SPI Write operations
*  @param[in]   address: Register address to which Primary needs to send the
*               data to secondary
*  @param[in]   value: The value which needs to stored the above address
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
void write(uint8_t address, uint8_t val);

/*******************************************************************************
*  @fn          void read(uint8_t address, int len, uint8_t buffer[]
*  @brief       SPI Read operations
*  @param[in]   address: Register address from which Primary needs to read the
*               data from secondary
*  @param[in]   len: Number of Bytes Read from buffer
*  @param[in]   buffer: The buffer in which the values to be stored after
*               reading the Number of Bytes
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
void read(uint8_t address, uint32_t num, uint8_t _buff[]);

/*******************************************************************************
*  @fn          void sl_si91x_adxl345_set_registerbit(uint8_t regAdress,
*                                                       int bitPos, bool state)
*  @brief       Sets the InActivity Interrupt bit ON or OFF
*  @param[in]   regAdress: Register Address bit
*  @param[in]   bitPos: bit position
*  @param[in]   state: state of the bit position
*  @return      NULL
*******************************************************************************/
void sl_si91x_adxl345_set_registerbit(uint8_t regAdress, uint8_t bitPos, bool state);

/*******************************************************************************
*  @fn          bool sl_si91x_adxl345_get_registerbit(uint8_t regAdress,
*                                                                    int bitPos)
*  @brief       Gets the register bit
*  @param[in]   regAdress: Register Address bit
*  @param[in]   bitPos: bit position
*  @return      Returns the state of the register bit
*******************************************************************************/
bool sl_si91x_adxl345_get_registerbit(uint8_t regAdress, uint8_t bitPos);

#ifdef __cplusplus
}
#endif

#endif /* ADXL345_H_ */
