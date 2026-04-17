/***************************************************************************/ /**
 * @file apds9960.h
 * @brief APDS9960 API implementation
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

#ifndef APDS9960_H_
#define APDS9960_H_
#include <stdbool.h>
#include "rsi_data_types.h"
#include "sensors_config.h"
#include "sensor_type.h"

#pragma GCC diagnostic ignored "-Wpedantic"
/*******************************************************************************
 ************************* APDS Typedefs/ Defines  *****************************
 ******************************************************************************/
typedef int32_t sl_apds_err_t;      //APDS error status
typedef void *sl_apds9960_handle_t; //APDS sensor handle
typedef void *sl_i2c_bus_handle_t;  //i2c bus handle

#define SL_APDS9960_I2C_ADDRESS_DEFAULT (0x39) //sensor address
/* Gesture parameters */
#define SL_GESTURE_THRESHOLD_OUT 10
#define SL_GESTURE_SENSITIVITY_1 50
#define SL_GESTURE_SENSITIVITY_2 20

/* Error code for returned values */
#define SL_ERROR 0xFF

/* Acceptable device IDs */
#define SL_ID_1_9960 0xAB
#define SL_ID_2_9960 0x9C
#define SL_ID_3_9960 0XA8

/* Misc parameters */
#define SL_FIFO_PAUSE_TIME 30 // Wait period (ms) between FIFO reads

/* APDS-9960 register addresses */
#define SL_ENABLE_APDS9960 0x80
#define SL_ATIME           0x81
#define SL_WTIME           0x83
#define SL_AILTL           0x84
#define SL_AILTH           0x85
#define SL_AIHTL           0x86
#define SL_AIHTH           0x87
#define SL_PILT            0x89
#define SL_PIHT            0x8B
#define SL_PERS            0x8C
#define SL_CONFIG1         0x8D
#define SL_PPULSE          0x8E
#define SL_CONTROL         0x8F
#define SL_CONFIG2         0x90
#define SL_ID              0x92
#define SL_STATUS          0x93
#define SL_CDATAL          0x94
#define SL_CDATAH          0x95
#define SL_RDATAL          0x96
#define SL_RDATAH          0x97
#define SL_GDATAL          0x98
#define SL_GDATAH          0x99
#define SL_BDATAL          0x9A
#define SL_BDATAH          0x9B
#define SL_PDATA           0x9C
#define SL_POFFSET_UR      0x9D
#define SL_POFFSET_DL      0x9E
#define SL_CONFIG3         0x9F
#define SL_GPENTH          0xA0
#define SL_GEXTH           0xA1
#define SL_GCONF1          0xA2
#define SL_GCONF2          0xA3
#define SL_GOFFSET_U       0xA4
#define SL_GOFFSET_D       0xA5
#define SL_GOFFSET_L       0xA7
#define SL_GOFFSET_R       0xA9
#define SL_GPULSE          0xA6
#define SL_GCONF3          0xAA
#define SL_GCONF4          0xAB
#define SL_GFLVL           0xAE
#define SL_GSTATUS         0xAF
#define SL_IFORCE          0xE4
#define SL_PICLEAR         0xE5
#define SL_CICLEAR         0xE6
#define SL_AICLEAR         0xE7
#define SL_GFIFO_U         0xFC
#define SL_GFIFO_D         0xFD
#define SL_GFIFO_L         0xFE
#define SL_GFIFO_R         0xFF

// Bit fields
#define SL_PON          0b00000001
#define SL_AEN          0b00000010
#define SL_APDS9960_PEN 0b00000100
#define SL_WEN          0b00001000
#define SL_AIEN         0b00010000
#define SL_PIEN         0b00100000
#define SL_GEN          0b01000000
//#define SL_GVALID       0b00000001//b2h
#define SL_GVALID 0x01

/* Status bit fields */
#define SL_AVALID 0b0000001
#define SL_PVALID 0b0000010
#define SL_GINT   0b0000100
#define SL_AINT   0b0010000
#define SL_PGSAT  0b0100000
#define SL_CPSAT  0b1000000

/* On/Off definitions */
#define SL_OFF 0
#define SL_ON  1

/* Acceptable parameters for setMode */
#define SL_POWER             0
#define SL_AMBIENT_LIGHT     1
#define SL_PROXIMITY         2
#define SL_WAIT              3
#define SL_AMBIENT_LIGHT_INT 4
#define SL_PROXIMITY_INT     5
#define SL_GESTURE           6
#define SL_ALL               7

/* LED Drive values */
#define SL_LED_DRIVE_100MA  0
#define SL_LED_DRIVE_50MA   1
#define SL_LED_DRIVE_25MA   2
#define SL_LED_DRIVE_12_5MA 3

/* Proximity Gain (PGAIN) values */
#define SL_PGAIN_1X 0
#define SL_PGAIN_2X 1
#define SL_PGAIN_4X 2
#define SL_PGAIN_8X 3

/* ALS Gain (AGAIN) values */
#define SL_AGAIN_1X  0
#define SL_AGAIN_4X  1
#define SL_AGAIN_16X 2
#define SL_AGAIN_64X 3

/* Gesture Gain (GGAIN) values */
#define SL_GGAIN_1X 0
#define SL_GGAIN_2X 1
#define SL_GGAIN_4X 2
#define SL_GGAIN_8X 3

/* LED Boost values */
#define SL_LED_BOOST_100 0
#define SL_LED_BOOST_150 1
#define SL_LED_BOOST_200 2
#define SL_LED_BOOST_300 3

/* Gesture wait time values */
#define SL_GWTIME_0MS    0
#define SL_GWTIME_2_8MS  1
#define SL_GWTIME_5_6MS  2
#define SL_GWTIME_8_4MS  3
#define SL_GWTIME_14_0MS 4
#define SL_GWTIME_22_4MS 5
#define SL_GWTIME_30_8MS 6
#define SL_GWTIME_39_2MS 7

/* Default values */
#define SL_DEFAULT_ATIME          219  // 103ms
#define SL_DEFAULT_WTIME          246  // 27ms
#define SL_DEFAULT_PROX_PPULSE    0x87 // 16us, 8 pulses
#define SL_DEFAULT_GESTURE_PPULSE 0x89 // 16us, 10 pulses
#define SL_DEFAULT_POFFSET_UR     0    // 0 offset
#define SL_DEFAULT_POFFSET_DL     0    // 0 offset
#define SL_DEFAULT_CONFIG1        0x60 // No 12x wait (WTIME) factor
#define SL_DEFAULT_LDRIVE         SL_LED_DRIVE_100MA
#define SL_DEFAULT_PGAIN          SL_PGAIN_4X
#define SL_DEFAULT_AGAIN          SL_AGAIN_4X
#define SL_DEFAULT_PILT           0      // Low proximity threshold
#define SL_DEFAULT_PIHT           50     // High proximity threshold
#define SL_DEFAULT_AILT           0xFFFF // Force interrupt for calibration
#define SL_DEFAULT_AIHT           0
#define SL_DEFAULT_PERS           0x11 // 2 consecutive prox or ALS for int.
#define SL_DEFAULT_CONFIG2        0x01 // No saturation interrupts or LED boost
#define SL_DEFAULT_CONFIG3        0    // Enable all photodiodes, no SAI
#define SL_DEFAULT_GPENTH         40   // Threshold for entering gesture mode
#define SL_DEFAULT_GEXTH          30   // Threshold for exiting gesture mode
#define SL_DEFAULT_GCONF1         0x40 // 4 gesture events for int., 1 for exit
#define SL_DEFAULT_GGAIN          SL_GGAIN_4X
#define SL_DEFAULT_GLDRIVE        SL_LED_DRIVE_100MA
#define SL_DEFAULT_GWTIME         SL_GWTIME_2_8MS
#define SL_DEFAULT_GOFFSET        0    // No offset scaling for gesture mode
#define SL_DEFAULT_GPULSE         0xC9 // 32us, 10 pulses
#define SL_DEFAULT_GCONF3         0    // All photodiodes active during gesture
#define SL_DEFAULT_GIEN           0    // Disable gesture interrupts
#define SL_FAIL                   1

/*******************************************************************************
 **************** Global Enums/Structures for APDS Sensor  *********************
 ******************************************************************************/
//apds sensor status
enum apds_sensor_en { SL_APDS_FAIL = -1, SL_APDS_OK = 0 };

/* Direction definitions */
enum { SL_DIR_NONE, SL_DIR_LEFT, SL_DIR_RIGHT, SL_DIR_UP, SL_DIR_DOWN, SL_DIR_NEAR, SL_DIR_FAR, SL_DIR_ALL };

/* State definitions */
enum { SL_NA_STATE, SL_NEAR_STATE, SL_FAR_STATE, SL_ALL_STATE };

//sensor device structures
typedef struct {
  int32_t i2c_dev;
  uint8_t dev_addr;
} sl_apds9960_dev_t;

/* Container for gesture data */
typedef struct gesture_data_type {
  uint8_t u_data[32];
  uint8_t d_data[32];
  uint8_t l_data[32];
  uint8_t r_data[32];
  uint8_t index;
  uint8_t total_gestures;
  uint8_t in_threshold;
  uint8_t out_threshold;
} sl_gesture_data_type_t;

/*******************************************************************************
*  @fn          sl_apds9960_handle_t sl_si91x_apds9960_create
*                                    (sl_i2c_bus_handle_t bus, uint8_t dev_addr)
*  @brief       Creates APDS9960 sensor in the Sensor Hub.
*  @param[in]   bus:  I2C bus handle
*  @param[in]   dev_addr: APDS9960 I2C secondary address
*  @return      Returns the apds9960 sensor handle
*******************************************************************************/
sl_apds9960_handle_t sl_si91x_apds9960_create(sl_sensor_bus_t bus, uint8_t dev_addr);

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_delete(sl_apds9960_handle_t *sensor)
*  @brief       Deletes APDS9960 sensor handle in the Sensor Hub.
*  @param[in]   sensor:  APDS9960 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_apds_err_t sl_si91x_apds9960_delete(sl_apds9960_handle_t *sensor);

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_power_down(
*                                                   sl_apds9960_handle_t sensor)
*  @brief       Power down the APDS9960 sensor in the Sensor Hub.
*  @param[in]   sensor:  APDS9960 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_init(
*                                               sl_i2c_bus_handle_t i2c_bus)
*  @brief       Initializing the APDS9960 sensor in the Sensor Hub.
*  @param[in]   i2c_bus:  APDS9960 Sensor I2C Bus handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_apds_err_t sl_si91x_apds9960_init(sl_sensor_bus_t i2c_bus);

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_deinit(void)
*  @brief       DeInitializing the APDS9960 sensor in the Sensor Hub.
*  @param[in]   NULL
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_apds_err_t sl_si91x_apds9960_deinit(void);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_ledboost(uint8_t boost)
*  @brief       It sets the LED boost values in the APDS9960 sensor.
*               LED Boost values can be as follows:
*               0 for 100%, 1 for 150%, 2 for 200% and 3 for 300%
*  @param[in]   boost: LED boost values from 0 to 3
*  @return      Returns true if Success, else false if Fails
*******************************************************************************/
bool sl_si91x_apds9960_set_ledboost(uint8_t boost);

/*******************************************************************************
*  @fn          uint8_t sl_si91x_apds9960_get_mode(void)
*  @brief       Obtain the SL_ENABLE_APDS9960 register value
*  @param[in]   NULL
*  @return      Returns SL_ENABLE_APDS9960 register value if Success, else
*               0xFF for Failure
*******************************************************************************/
uint8_t sl_si91x_apds9960_get_mode(void);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_gesture_mode(uint8_t mode)
*  @brief       Makes the state machine either start or end the Gesture
*               state machine in APDS9960 sensor.
*  @param[in]   mode: Mode 1 is to enter the gesture state machine and Mode 0
*               is to exit the state machine in the APDS9960 sensor
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_gesture_mode(uint8_t mode);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_enable_gesturesensor(bool interrupts)
*  @brief       Enables the Gesture Sensing in APDS9960 sensor.
*  @param[in]   interrupts: Enable interrupt for Gesture
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_enable_gesture_sensor(bool interrupts);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_is_gesture_available(void)
*  @brief       Checks if Gesture Sensing is available in the APDS9960 sensor.
*  @param[in]   NULL
*  @return      Returns true if Success, else false if Fails
*******************************************************************************/
bool sl_si91x_apds9960_is_gesture_available(void);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_process_gesture_data(void)
*  @brief       Processes the gesture data.
*  @param[in]   NULL
*  @return      Returns true if Success, else false if Fails
*******************************************************************************/
bool sl_si91x_apds9960_process_gesture_data(void);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_decode_gesture(void)
*  @brief       Decodes the gesture data in its state and motion
*  @param[in]   NULL
*  @return      Returns true if Success, else false if Fails
*******************************************************************************/
bool sl_si91x_apds9960_decode_gesture(void);

/*******************************************************************************
*  @fn          int sl_si91x_apds9960_read_gesture(void)
*  @brief       It reads the gesture after processing and decoding the gesture
*  @brief       data
*  @param[in]   NULL
*  @return      Returns Motion read if Success, else false if Fails
*******************************************************************************/
int sl_si91x_apds9960_read_gesture(void);

void sl_si91x_apds9960_handle_gesture(void);

/*******************************************************************************
*  @fn          int32_t sl_si91x_write_buffer(uint8_t sub_addr, uint8_t *buf,
*                                                                  uint32_t len)
*  @brief       I2C Write operations
*  @param[in]   sub_addr: Register address from which Primary needs to read the
*               data from secondary
*  @param[in]   buf: Buffer which stores the registered address of secondary
*               in which the primary has to write to or read from
*  @param[in]   len: the length of the buffer that needs to be filled
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
int32_t sl_si91x_write_buffer(uint8_t sub_addr, uint8_t apds_data_buf, uint32_t len); //*buff to buff

/*******************************************************************************
*  @fn          int32_t sl_si91x_read_buffer(uint16_t addr, uint8_t *buffer,
*                                                                  uint32_t len)
*  @brief       I2C Read operations from secondary's
*  @param[in]   sub_addr: Register address from which Primary needs to read the
*               data from secondary
*  @param[in]   buf: Buffer in which data needs to be stored
*  @param[in]   len: the length of the buffer that needs to be filled
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
int32_t sl_si91x_read_buffer(uint16_t addr, uint8_t *buffer, uint32_t len);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_mode(uint8_t mode, uint8_t enable)
*  @brief       Obtain the SL_ENABLE_APDS9960 register value
*  @param[in]   mode: Select the mode for the feature to enable
*  @param[in]   enable: If Enable set it to 1, else for Disable set it to 0
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_mode(int8_t mode, uint8_t enable);
/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_led_drive(uint8_t drive)
*  @brief       Sets LED strength for proximity and ambient light sensor
*  @param[in]   drive: Select the current with which the LED should be driven
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_leddrive(uint8_t drive);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_proximitygain(uint8_t drive)
*  @brief       Sets the receiver gain for Proximity sensing
*  @param[in]   drive: Select the gain value
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_proximity_gain(uint8_t drive);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_proximitygain(uint8_t drive)
*  @brief       Sets the receiver gain for the Ambient light sensor
*  @param[in]   drive: Select the gain value
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_ambientlight_gain(uint8_t drive);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_proximity_interrupt_low_threshold
*                                                            (uint8_t threshold)
*  @brief       Sets the low threshold for proximity interrupts
*  @param[in]   threshold: Select the low threshold value for the interrupt to
*               activate
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_proximity_intterupt_low_threshold(uint8_t threshold); //comment

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_proximity_interrupt_high_threshold
*                                                            (uint8_t threshold)
*  @brief       Sets the high threshold for proximity interrupts
*  @param[in]   threshold: Select the high threshold value for the interrupt to
*               activate
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_proximity_intterupt_high_threshold(uint8_t threshold);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_light_interrupt_low_threshold
*                                                            (uint8_t threshold)
*  @brief       Sets the low threshold for light interrupts
*  @param[in]   threshold: Select the low threshold value for the interrupt to
*               activate
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_light_intterupt_low_threshold(uint16_t threshold);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_light_interrupt_high_threshold
*                                                            (uint8_t threshold)
*  @brief       Sets the high threshold for light interrupts
*  @param[in]   threshold: Select the high threshold value for the interrupt to
*               activate
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_light_intterupt_high_threshold(uint16_t threshold);

/*******************************************************************************
*  @fn          uint8_t sl_si91x_apds9960_set_gesture_enter_threshold
*                                                            (uint8_t threshold)
*  @brief       Sets the entry proximity threshold for gesture sensing
*  @param[in]   threshold: Proximity Value needed to start Gesture mode
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
uint8_t sl_si91x_apds9960_set_gesture_enter_threshold(uint8_t threshold);

/*******************************************************************************
*  @fn          uint8_t sl_si91x_apds9960_set_gesture_exit_threshold
*                                                            (uint8_t threshold)
*  @brief       Sets the Exit proximity threshold for gesture sensing
*  @param[in]   threshold: Proximity Value needed to stop Gesture mode
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
uint8_t sl_Si91x_apds9960_set_gesture_exit_threshold(uint8_t threshold);

/*******************************************************************************
*  @fn          uint8_t sl_si91x_apds9960_set_gesture_gain(uint8_t gain)
*  @brief       Sets the gain of the  photo-sensor in Gesture mode
*  @param[in]   gain: The value for  photo-sensor gain
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
uint8_t sl_si91x_apds9960_set_gesture_gain(uint8_t gain);

/*******************************************************************************
*  @fn          uint8_t sl_si91x_apds9960_set_gesture_led_drive(uint8_t drive)
*  @brief       Sets the LED current in Gesture mode
*  @param[in]   drive: The value for LED current
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
uint8_t sl_si91x_apds9960_set_gesture_leddrive(uint8_t drive);

/*******************************************************************************
*  @fn          uint8_t sl_si91x_apds9960_set_gesture_wait_time(uint8_t time)
*  @brief       Sets the wait time between gesture recognition
*  @param[in]   time: The value for wait time
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
uint8_t sl_Si91x_apds9960_set_gesture_waittime(uint8_t time);

/*******************************************************************************
*  @fn          uint8_t sl_si91x_apds9960_set_gesture_interruptenable
*                                                               (uint8_t enable)
*  @brief       Enables or Disables Gesture Interrupts
*  @param[in]   enable: The value 1 to enable the interrupt and 0 to disable the
*                                                                      interrupt
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
uint8_t sl_si91x_apds9960_set_gesture_intteruptenable(uint8_t enable);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_enable_power(void)
*  @brief       Enable the power to the APDS9960 sensor
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_enable_power(void);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_ambientlight_interruptenable
*                                                               (uint8_t enable)
*  @brief       Enables or Disables Ambient Light Interrupts
*  @param[in]   enable: The value 1 to enable the interrupt and 0 to disable the
*                                                                      interrupt
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_ambientlight_intteruptenable(uint8_t enable);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_sensor_init(void)
*  @brief       This API is used to configure the I2C communications with
*               APDS9960 and initializes all the registers to its default values
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_sensor_init(void);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_proximity_interruptenable
*                                                               (uint8_t enable)
*  @brief       Enables or Disables Proximity Interrupts
*  @param[in]   enable: The value 1 to enable the interrupt and 0 to disable the
*                                                                      interrupt
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_proximity_intteruptenable(uint8_t enable);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_enable_proximitysensor(bool interrupts)
*  @brief       Enables the Proximity Sensing in the APDS9960 sensor.
*  @param[in]   interrupts: Enable interrupt for Gesture
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_enable_proximity_sensor(bool interrupts);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_read_proximity(uint8_t *value)
*  @brief       Reads the Proximity level as 8-bit value
*  @param[in]   value: Value of Proximity
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_read_proximity(uint8_t *val);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_enable_lightsensor(bool interrupts)
*  @brief       Activates Light Sensor (R/G/B/Ambient) sensor on the APDS-9960
*  @param[in]   interrupts: Set true to enable hardware interrupt on high or low
*               light
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_enable_lightsensor(bool interrupts);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_read_ambientlight(uint16_t *value)
*  @brief       To read the Ambient light level as a 16-bit value
*  @param[in]   value: Value of Light sensor
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_read_ambientlight(uint16_t *val);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_read_greenlight(uint16_t *value)
*  @brief       To read the Greenlight level as a 16-bit value
*  @param[in]   value: Value of Green Light sensor
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_read_greenlight(uint16_t *val);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_read_redlight(uint16_t *value)
*  @brief       To read the Red light level as a 16-bit value
*  @param[in]   value: Value of Red Light sensor
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_read_redlight(uint16_t *val);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_read_bluelight(uint16_t *value)
*  @brief       To read the Blue light level as a 16-bit value
*  @param[in]   value: Value of Blue Light sensor
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_read_bluelight(uint16_t *val);

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_sample_colordata
*                                       (float *r, float *g, float *b, float *w)
*  @brief       Processes and Reads the value of RED, GREEN and BLUE color light
*               and sends the value to Sensor Hub
*  @param[in]   r: Value of Red Light sensor
*  @param[in]   g: Value of Green Light sensor
*  @param[in]   b: Value of Blue Light sensor
*  @param[in]   w: Value of Ambient Light sensor
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_apds_err_t sl_si91x_apds9960_sample_colordata(float *r, float *g, float *b, float *w);

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_sample_proximitydata(float *data)
*  @brief       Processes and Reads the value of Proximity sensor and sends the
*               value to Sensor Hub
*  @param[in]   data: Value of Proximity data
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_apds_err_t sl_si91x_apds9960_sample_proximitydata(float *data);

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_sample_gesturedata(char *data)
*  @brief       Processes and Reads the value of Gesture sensor and sends the
*               value to Sensor Hub
*  @param[in]   data: Value of Gesture data
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_apds_err_t sl_si91x_apds9960_sample_gesturedata(char *data);

/*******************************************************************************
*  @fn          void sli_si91x_delay(uint32_t idelay) //1.002ms delay
*  @brief       This function produce the delay by using the for loop.
*  @param[in]   idelay: Delay value
*  @return      NONE
*******************************************************************************/
void sli_si91x_delay(uint32_t idelay);

/*******************************************************************************
*  @fn          int32_t sl_si91x_read_blockbuffer(uint16_t addr, uint8_t *buff,
*                                                                  uint32_t len)
*  @brief       I2C read operations takes place for 128 bytes
*               data
*  @param[in]   addr: Register address from which Primary needs to read the data
*               from secondary
*  @param[in]   buff: Buffer in which data needs to be stored
*  @param[in]   len: the length of the buffer that needs to be filled
*  @return      Returns count number if Success, else 1 if Fails
*******************************************************************************/
int32_t sl_si91x_read_blockbuffer(uint16_t addr, uint8_t *buff, uint32_t len);

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_test(void)
*  @brief       Testing the APDS9960 sensor in the Sensor Hub.
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_apds_err_t sl_si91x_apds9960_test(void);

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_power_down(
*                                                   sl_apds9960_handle_t sensor)
*  @brief       Power downs the APDS9960 sensor in the Sensor Hub.
*  @param[in]   sensor:  APDS9960 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
int32_t sl_si91x_apds9960_power_down(sl_apds9960_handle_t sensor);

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_power_on(
*                                                   sl_apds9960_handle_t sensor)
*  @brief       Power ON the APDS9960 sensor in the Sensor Hub.
*  @param[in]   sensor:  APDS9960 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
int32_t sl_si91x_apds9960_power_on(sl_apds9960_handle_t sensor);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_enable_gesturesensor(bool interrupts)
*  @brief       Enables the Gesture Sensing in APDS9960 sensor.
*  @param[in]   interrupts: Enable interrupt for Gesture
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_enable_gesturesensor(bool interrupts);

/*******************************************************************************
*  @fn          void sl_si91x_apds9960_reset_gestureparameters()
*  @brief       Resets Gesture Parameters in APDS9960 sensor.
*  @param[in]   NULL
*  @return      -
*******************************************************************************/
void sl_si91x_apds9960_reset_gestureparameters();

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_led_drive(uint8_t drive)
*  @brief       Sets LED strength for proximity and ambient light sensor
*  @param[in]   drive: Select the current with which the LED should be driven
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_led_drive(uint8_t drive);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_proximitygain(uint8_t drive)
*  @brief       Sets the receiver gain for Proximity sensing
*  @param[in]   drive: Select the gain value
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_proximitygain(uint8_t drive);

/*******************************************************************************
*  @fn          uint8_t sl_si91x_apds9960_set_gesture_exit_threshold
*                                                            (uint8_t threshold)
*  @brief       Sets the Exit proximity threshold for gesture sensing
*  @param[in]   threshold: Proximity Value needed to stop Gesture mode
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
uint8_t sl_si91x_apds9960_set_gesture_exit_threshold(uint8_t threshold);

/*******************************************************************************
*  @fn          uint8_t sl_si91x_apds9960_set_gesture_led_drive(uint8_t drive)
*  @brief       Sets the LED current in Gesture mode
*  @param[in]   drive: The value for LED current
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
uint8_t sl_si91x_apds9960_set_gesture_led_drive(uint8_t drive);

/*******************************************************************************
*  @fn          uint8_t sl_si91x_apds9960_set_gesture_wait_time(uint8_t time)
*  @brief       Sets the wait time between gesture recognition
*  @param[in]   time: The value for wait time
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
uint8_t sl_si91x_apds9960_set_gesture_wait_time(uint8_t time);

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_enable_proximitysensor(bool interrupts)
*  @brief       Enables the Proximity Sensing in APDS9960 sensor.
*  @param[in]   interrupts: Enable interrupt for Gesture
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_enable_proximitysensor(bool interrupts);

#ifdef __cplusplus
}
#endif

#endif /*APDS9960*/
