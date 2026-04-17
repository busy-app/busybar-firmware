/***************************************************************************/ /**
 * @file apds9960.c
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
/**=============================================================================
 * @brief : This file contains the APDS9960 sensor's driver file for Sensor Hub
 * application
 * This example uses simple communication APIs for transferring the APDS9960
 * sensor data through I2C interface.
 * This sensor will detect the RGB light, Gesture and Proximity and sends the
 * detected RGB data values, the detection value for Gesture and Proximity data
 * value
 ============================================================================**/
#include "apds9960.h"
#include "sensor_hub.h"
#include "rsi_debug.h"
#include "Driver_I2C.h"

#pragma GCC diagnostic ignored "-Wpedantic"

/*******************************************************************************
 ************************ APDS Defines / Macros  ******************************
 ******************************************************************************/
#define SL_APDS9960_POWER_ON   (0x80) //ADPS power on address
#define SL_APDS9960_POWER_DOWN (0x80) //ADPS power on address
#define SL_SIZE_BUFFERS        8      //Buffer size
#define SL_BLOCK_SIZE          128    //Block size
#define SL_MAX_WRITE           8      // Max bytes to write in one step

/*******************************************************************************
 ************************ APDS Variables   *************************************
 ******************************************************************************/
extern ARM_DRIVER_I2C *I2Cdrv;         //i2c driver
sl_gesture_data_type_t gesture_data_;  //gesture structure
static bool is_gpr_init       = false; //APDS sensor status
sl_apds9960_handle_t apds9960 = NULL;  //APDS handle
uint8_t wr_buf[SL_MAX_WRITE + 2];      //Sensor write buff
uint8_t rd_buf[SL_SIZE_BUFFERS];       //sensor read buff
uint8_t rd_block_buf[SL_BLOCK_SIZE];   //Sensor read block buff

/*******************************************************************************
 **************************** APDS Data Variables  *****************************
 ******************************************************************************/
int gesture_ud_delta_;
int gesture_lr_delta_;
int gesture_ud_count_;
int gesture_lr_count_;
int gesture_near_count_;
int gesture_far_count_;
int gesture_state_;
int gesture_motion_;

#define SL_MAX_TIME_TO_WAIT 5000
//TickType_t xTicksToWait = SL_MAX_TIME_TO_WAIT;
//TimeOut_t xTimeOut;

/*******************************************************************************
*  @fn          sl_apds9960_handle_t sl_si91x_apds9960_create
*                                    (sl_i2c_bus_handle_t bus, uint8_t dev_addr)
*  @brief       Creates APDS9960 sensor in the Sensor Hub.
*  @param[in]   bus:  I2C bus handle
*  @param[in]   dev_addr: APDS9960 I2C secondary address
*  @return      Returns the apds9960 sensor handle
*******************************************************************************/
sl_apds9960_handle_t sl_si91x_apds9960_create(sl_sensor_bus_t bus, uint8_t dev_addr)
{
  sl_apds9960_dev_t *sensor = (sl_apds9960_dev_t *)pvPortMalloc(sizeof(sl_apds9960_dev_t));
  uint8_t a[5];
  (void)bus;

  a[0]            = (0x01 & 0xFF);
  sensor->i2c_dev = I2Cdrv->MasterTransmit(SL_APDS9960_I2C_ADDRESS_DEFAULT, a, 1, false);

  DEBUGOUT("\r\n I2C - APDS9960 Sensor Create \r\n");
  while (I2Cdrv->GetStatus().busy)
    ;
  /*TODO: check the time out */
  //  vTaskSetTimeOutState( &xTimeOut );
  //  while (I2Cdrv->GetStatus().busy) {
  //      if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE ) {
  //          DEBUGOUT("\r\n I2C Status TimeOut \r\n");
  //          return RSI_FAIL;
  //      }
  //  }

  sensor->dev_addr = dev_addr;
  return (sl_apds9960_handle_t)sensor;
}

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_delete(sl_apds9960_handle_t *sensor)
*  @brief       Deletes APDS9960 sensor handle in the Sensor Hub.
*  @param[in]   sensor:  APDS9960 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
sl_apds_err_t sl_si91x_apds9960_delete(sl_apds9960_handle_t *sensor)
{
  if (*sensor == NULL) {
    return RSI_OK;
  }
  free(sensor);
  sensor = NULL;
  return RSI_OK;
}

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_power_down(
*                                                   sl_apds9960_handle_t sensor)
*  @brief       Power downs the APDS9960 sensor in the Sensor Hub.
*  @param[in]   sensor:  APDS9960 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
int32_t sl_si91x_apds9960_power_down(sl_apds9960_handle_t sensor)
{
  int32_t ret = 0;
  //sl_apds9960_dev_t *sens = (sl_apds9960_dev_t *)sensor;
  uint8_t a[5] = "\0";

  (void)sensor;
  a[0] = (uint8_t)(SL_APDS9960_POWER_DOWN & 0xFF);
  ret  = I2Cdrv->MasterTransmit(SL_APDS9960_I2C_ADDRESS_DEFAULT, a, 1, false);

  DEBUGOUT("\r\n I2C sensor create \r\n");
  while (I2Cdrv->GetStatus().busy)
    ;

  return ret;
}

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_power_on(
*                                                   sl_apds9960_handle_t sensor)
*  @brief       Power ON the APDS9960 sensor in the Sensor Hub.
*  @param[in]   sensor:  APDS9960 Sensor handle
*  @return      Returns RSI_OK if success, else RSI_FAIL for Failure
*******************************************************************************/
int32_t sl_si91x_apds9960_power_on(sl_apds9960_handle_t sensor)
{
  int32_t ret             = 0;
  uint8_t a[5]            = "\0";
  sl_apds9960_dev_t *sens = (sl_apds9960_dev_t *)sensor;
  (void)sensor;
  (void)sens;

  a[0] = (uint8_t)(SL_APDS9960_POWER_ON & 0xFF);
  ret  = I2Cdrv->MasterTransmit(SL_APDS9960_I2C_ADDRESS_DEFAULT, a, 1, false); //sens->i2c_dev
  while (I2Cdrv->GetStatus().busy)
    ;

  return ret;
}

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_init(
*                                               sl_i2c_bus_handle_t i2c_bus)
*  @brief       Initializing the APDS9960 sensor in the Sensor Hub.
*  @param[in]   i2c_bus:  APDS9960 Sensor I2C Bus handle
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_apds_err_t sl_si91x_apds9960_init(sl_sensor_bus_t i2c_bus)
{
  bool status;
  apds9960 = sl_si91x_apds9960_create(i2c_bus, SL_APDS9960_I2C_ADDRESS_DEFAULT);

  status = sl_si91x_apds9960_sensor_init();

  if (status) {
    DEBUGOUT("\r\n APDS Sensor Init Failed = %d \r \n", status);
    return SL_STATUS_FAIL;
  }

  if (NULL == apds9960) {
    DEBUGOUT("\r\n Sensor APDS Creation Failed \r \n");
    return SL_STATUS_FAIL;
  }
  is_gpr_init = true;
  return RSI_OK;
}

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_deinit(void)
*  @brief       DeInitializing the APDS9960 sensor in the Sensor Hub.
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_apds_err_t sl_si91x_apds9960_deinit(void)
{
  if (!is_gpr_init) {
    return SL_STATUS_FAIL;
  }

  sl_apds_err_t ret = sl_si91x_apds9960_delete(&apds9960);

  if (ret == RSI_OK) {
    is_gpr_init = false;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_test(void)
*  @brief       Testing the APDS9960 sensor in the Sensor Hub.
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_apds_err_t sl_si91x_apds9960_test(void)
{
  if (!is_gpr_init) {
    return SL_STATUS_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          void sl_si91x_apds9960_reset_gestureparameters()
*  @brief       Resets Gesture Parameters in APDS9960 sensor.
*  @param[in]   NULL
*  @return      -
*******************************************************************************/
void sl_si91x_apds9960_reset_gestureparameters()
{
  gesture_data_.index          = 0;
  gesture_data_.total_gestures = 0;

  gesture_ud_delta_ = 0;
  gesture_lr_delta_ = 0;

  gesture_ud_count_ = 0;
  gesture_lr_count_ = 0;

  gesture_near_count_ = 0;
  gesture_far_count_  = 0;

  gesture_state_  = 0;
  gesture_motion_ = SL_DIR_NONE;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_ledboost(uint8_t boost)
*  @brief       It sets the LED boost values in APDS9960 sensor.
*  @param[in]   boost: LED boost values
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_ledboost(uint8_t boost)
{
  uint8_t val;

  /* Read value from CONFIG2 register */
  if (sl_si91x_read_buffer(SL_CONFIG2, &val, 1)) {
    return SL_FAIL;
  }

  /* Set bits in register to given value */
  //boost &= 0b00000011;//b2h
  boost &= 0x03;
  boost = boost << 4;
  //val &= 0b11001111;//b2h
  val &= 0xCF;
  val |= boost;

  /* Write register value back into CONFIG2 register */
  if (sl_si91x_write_buffer(SL_CONFIG2, val, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_gesture_mode(uint8_t mode)
*  @brief       It either starts or ends the Gesture state machine in APDS9960
*               sensor.
*  @param[in]   mode: Different modes in APDS9960 sensor
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_gesture_mode(uint8_t mode)
{
  uint8_t val;

  /* Read value from GCONF4 register */
  if (sl_si91x_read_buffer(SL_GCONF4, &val, 1)) {
    return SL_FAIL;
  }

  /* Set bits in register to given value */
  //mode &= 0b00000001;//b2h
  //val &= 0b11111110;//b2h
  mode &= 0x01; //b2h
  val &= 0xFE;  //b2h
  val |= mode;

  /* Write register value back into GCONF4 register */
  if (sl_si91x_write_buffer(SL_GCONF4, val, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_enable_gesturesensor(bool interrupts)
*  @brief       Enables the Gesture Sensing in APDS9960 sensor.
*  @param[in]   interrupts: Enable interrupt for Gesture
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_enable_gesturesensor(bool interrupts)
{

  sl_si91x_apds9960_reset_gestureparameters();
  if (sl_si91x_write_buffer(SL_WTIME, 0xFF, 1)) {
    return SL_FAIL;
  }
  if (sl_si91x_write_buffer(SL_PPULSE, SL_DEFAULT_GESTURE_PPULSE, 1)) {
    return SL_FAIL;
  }
  if (sl_si91x_apds9960_set_ledboost(SL_LED_BOOST_300)) {
    return SL_FAIL;
  }
  if (interrupts) {
    if (sl_si91x_apds9960_set_gesture_intteruptenable(1)) {
      return SL_FAIL;
    }
  } else {
    if (sl_si91x_apds9960_set_gesture_intteruptenable(0)) {
      return SL_FAIL;
    }
  }
  if (sl_si91x_apds9960_set_gesture_mode(1)) {
    return SL_FAIL;
  }
  if (sl_si91x_apds9960_enable_power()) {
    return SL_FAIL;
  }
  if (sl_si91x_apds9960_set_mode(SL_WAIT, 1)) {
    return SL_FAIL;
  }
  if (sl_si91x_apds9960_set_mode(SL_PROXIMITY, 1)) {
    return SL_FAIL;
  }
  if (sl_si91x_apds9960_set_mode(SL_GESTURE, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_is_gesture_available(void)
*  @brief       Checks if Gesture Sensing is available in APDS9960 sensor.
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_is_gesture_available(void)
{
  uint8_t val;

  /* Read value from GSTATUS register */
  if (sl_si91x_read_buffer(SL_GSTATUS, &val, 1)) {
    return SL_FAIL;
  }

  /* Shift and mask out GVALID bit */
  //val &= SL_GVALID;
  /* Return true/false based on GVALID bit */
  if (val == 1) {
    return RSI_OK;
  } else {
    return SL_FAIL;
  }
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_process_gesture_data(void)
*  @brief       Processes the gesture data.
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_process_gesture_data(void)
{
  uint8_t u_first = 0;
  uint8_t d_first = 0;
  uint8_t l_first = 0;
  uint8_t r_first = 0;
  uint8_t u_last  = 0;
  uint8_t d_last  = 0;
  uint8_t l_last  = 0;
  uint8_t r_last  = 0;
  int ud_ratio_first;
  int lr_ratio_first;
  int ud_ratio_last;
  int lr_ratio_last;
  int ud_delta;
  int lr_delta;
  int i;

  /* If we have less than 4 total gestures, that's not enough */
  if (gesture_data_.total_gestures <= 4) {
    return SL_FAIL;
  }

  /* Check to make sure our data isn't out of bounds */
  if ((gesture_data_.total_gestures <= 32) && (gesture_data_.total_gestures > 0)) {

    /* Find the first value in U/D/L/R above the threshold */
    for (i = 0; i < gesture_data_.total_gestures; i++) {
      if ((gesture_data_.u_data[i] > SL_GESTURE_THRESHOLD_OUT) && (gesture_data_.d_data[i] > SL_GESTURE_THRESHOLD_OUT)
          && (gesture_data_.l_data[i] > SL_GESTURE_THRESHOLD_OUT)
          && (gesture_data_.r_data[i] > SL_GESTURE_THRESHOLD_OUT)) {

        u_first = gesture_data_.u_data[i];
        d_first = gesture_data_.d_data[i];
        l_first = gesture_data_.l_data[i];
        r_first = gesture_data_.r_data[i];
        break;
      }
    }

    /* If one of the _first values is 0, then there is no good data */
    if ((u_first == 0) || (d_first == 0) || (l_first == 0) || (r_first == 0)) {

      return SL_FAIL;
    }
    /* Find the last value in U/D/L/R above the threshold */
    for (i = gesture_data_.total_gestures - 1; i >= 0; i--) {

      if ((gesture_data_.u_data[i] > SL_GESTURE_THRESHOLD_OUT) && (gesture_data_.d_data[i] > SL_GESTURE_THRESHOLD_OUT)
          && (gesture_data_.l_data[i] > SL_GESTURE_THRESHOLD_OUT)
          && (gesture_data_.r_data[i] > SL_GESTURE_THRESHOLD_OUT)) {

        u_last = gesture_data_.u_data[i];
        d_last = gesture_data_.d_data[i];
        l_last = gesture_data_.l_data[i];
        r_last = gesture_data_.r_data[i];
        break;
      }
    }
  }

  /* Calculate the first vs. last ratio of up/down and left/right */
  ud_ratio_first = ((u_first - d_first) * 100) / (u_first + d_first);
  lr_ratio_first = ((l_first - r_first) * 100) / (l_first + r_first);
  ud_ratio_last  = ((u_last - d_last) * 100) / (u_last + d_last);
  lr_ratio_last  = ((l_last - r_last) * 100) / (l_last + r_last);

  /* Determine the difference between the first and last ratios */
  ud_delta = ud_ratio_last - ud_ratio_first;
  lr_delta = lr_ratio_last - lr_ratio_first;

  /* Accumulate the UD and LR delta values */
  gesture_ud_delta_ += ud_delta;
  gesture_lr_delta_ += lr_delta;

  /* Determine U/D gesture */
  if (gesture_ud_delta_ >= SL_GESTURE_SENSITIVITY_1) {
    gesture_ud_count_ = 1;
  } else if (gesture_ud_delta_ <= -SL_GESTURE_SENSITIVITY_1) {
    gesture_ud_count_ = -1;
  } else {
    gesture_ud_count_ = 0;
  }

  /* Determine L/R gesture */
  if (gesture_lr_delta_ >= SL_GESTURE_SENSITIVITY_1) {
    gesture_lr_count_ = 1;
  } else if (gesture_lr_delta_ <= -SL_GESTURE_SENSITIVITY_1) {
    gesture_lr_count_ = -1;
  } else {
    gesture_lr_count_ = 0;
  }

  /* Determine Near/Far gesture */
  if ((gesture_ud_count_ == 0) && (gesture_lr_count_ == 0)) {
    if ((abs(ud_delta) < SL_GESTURE_SENSITIVITY_2) && (abs(lr_delta) < SL_GESTURE_SENSITIVITY_2)) {

      if ((ud_delta == 0) && (lr_delta == 0)) {
        gesture_near_count_++;
      } else if ((ud_delta != 0) || (lr_delta != 0)) {
        gesture_far_count_++;
      }

      if ((gesture_near_count_ >= 10) && (gesture_far_count_ >= 2)) {
        if ((ud_delta == 0) && (lr_delta == 0)) {
          gesture_state_ = SL_NEAR_STATE;
        } else if ((ud_delta != 0) && (lr_delta != 0)) {
          gesture_state_ = SL_FAR_STATE;
        }
        return RSI_OK;
      }
    }
  } else {
    if ((abs(ud_delta) < SL_GESTURE_SENSITIVITY_2) && (abs(lr_delta) < SL_GESTURE_SENSITIVITY_2)) {

      if ((ud_delta == 0) && (lr_delta == 0)) {
        gesture_near_count_++;
      }

      if (gesture_near_count_ >= 10) {
        gesture_ud_count_ = 0;
        gesture_lr_count_ = 0;
        gesture_ud_delta_ = 0;
        gesture_lr_delta_ = 0;
      }
    }
  }

  return SL_FAIL;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_decode_gesture(void)
*  @brief       Decodes the gesture data in its state and motion
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_decode_gesture(void)
{
  /* Return if near or far event is detected */
  if (gesture_state_ == SL_NEAR_STATE) {
    gesture_motion_ = SL_DIR_NEAR;
    return RSI_OK;
  } else if (gesture_state_ == SL_FAR_STATE) {
    gesture_motion_ = SL_DIR_FAR;
    return RSI_OK;
  }

  /* Determine swipe direction */
  if ((gesture_ud_count_ == -1) && (gesture_lr_count_ == 0)) {
    gesture_motion_ = SL_DIR_UP;
  } else if ((gesture_ud_count_ == 1) && (gesture_lr_count_ == 0)) {
    gesture_motion_ = SL_DIR_DOWN;
  } else if ((gesture_ud_count_ == 0) && (gesture_lr_count_ == 1)) {
    gesture_motion_ = SL_DIR_RIGHT;
  } else if ((gesture_ud_count_ == 0) && (gesture_lr_count_ == -1)) {
    gesture_motion_ = SL_DIR_LEFT;
  } else if ((gesture_ud_count_ == -1) && (gesture_lr_count_ == 1)) {
    if (abs(gesture_ud_delta_) > abs(gesture_lr_delta_)) {
      gesture_motion_ = SL_DIR_UP;
    } else {
      gesture_motion_ = SL_DIR_RIGHT;
    }
  } else if ((gesture_ud_count_ == 1) && (gesture_lr_count_ == -1)) {
    if (abs(gesture_ud_delta_) > abs(gesture_lr_delta_)) {
      gesture_motion_ = SL_DIR_DOWN;
    } else {
      gesture_motion_ = SL_DIR_LEFT;
    }
  } else if ((gesture_ud_count_ == -1) && (gesture_lr_count_ == -1)) {
    if (abs(gesture_ud_delta_) > abs(gesture_lr_delta_)) {
      gesture_motion_ = SL_DIR_UP;
    } else {
      gesture_motion_ = SL_DIR_LEFT;
    }
  } else if ((gesture_ud_count_ == 1) && (gesture_lr_count_ == 1)) {
    if (abs(gesture_ud_delta_) > abs(gesture_lr_delta_)) {
      gesture_motion_ = SL_DIR_DOWN;
    } else {
      gesture_motion_ = SL_DIR_RIGHT;
    }
  } else {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          int sl_si91x_apds9960_read_gesture(void)
*  @brief       It reads the gesture after processing and decoding the gesture
*               data
*  @param[in]   NULL
*  @return      Returns Motion read if Success, else 0xFF if Fails
*******************************************************************************/
int sl_si91x_apds9960_read_gesture(void)
{
  uint8_t fifo_level = 0;
  int32_t bytes_read = 0;
  uint8_t fifo_dump[128];
  uint8_t gstatus;
  int motion;
  uint32_t d = 500;
  int i;

  /* Make sure that power and gesture is on and data is valid */
  //  if (sl_si91x_apds9960_is_gesture_available() || !(sl_si91x_apds9960_get_mode() & 0b01000001))//b2h
  //  {
  //    return SL_DIR_NONE;
  //  }
  if (sl_si91x_apds9960_is_gesture_available() || !(sl_si91x_apds9960_get_mode() & 0x41)) {
    return SL_DIR_NONE;
  }

  /* Keep looping as long as gesture data is valid */
  while (1) {

    /* Wait some time to collect next batch of FIFO data */
    sli_si91x_delay(d);

    /* Get the contents of the STATUS register. Is data still valid? */
    if (sl_si91x_read_buffer(SL_GSTATUS, &gstatus, 1)) {
      return SL_ERROR;
    }

    /* If we have valid data, read in FIFO */
    if ((gstatus & SL_GVALID) == SL_GVALID) {

      /* Read the current FIFO level */
      if (sl_si91x_read_buffer(SL_GFLVL, &fifo_level, 1)) {
        return SL_ERROR;
      }

      /* If there's stuff in the FIFO, read it into our data block */
      if (fifo_level > 0) {
        bytes_read = sl_si91x_read_blockbuffer(SL_GFIFO_U, fifo_dump, (fifo_level * 4));
        if (bytes_read == -1) {
          return SL_ERROR;
        }

        /* If at least 1 set of data, sort the data into U/D/L/R */
        if (bytes_read >= 4) {
          for (i = 0; i < bytes_read; i += 4) {
            gesture_data_.u_data[gesture_data_.index] = rd_block_buf[i + 0];
            gesture_data_.d_data[gesture_data_.index] = rd_block_buf[i + 1];
            gesture_data_.l_data[gesture_data_.index] = rd_block_buf[i + 2];
            gesture_data_.r_data[gesture_data_.index] = rd_block_buf[i + 3];
            gesture_data_.index++;
            gesture_data_.total_gestures++;
          }

          /* Filter and process gesture data. Decode near/far state */
          if (sl_si91x_apds9960_process_gesture_data()) {
            if (!sl_si91x_apds9960_decode_gesture()) {
            }
          }

          /* Reset data */
          gesture_data_.index          = 0;
          gesture_data_.total_gestures = 0;
        }
      }
    } else {

      /* Determine best guessed gesture and clean up */
      sli_si91x_delay(d);
      sl_si91x_apds9960_decode_gesture();
      motion = gesture_motion_;

      sl_si91x_apds9960_reset_gestureparameters();
      return motion;
    }
  }
}

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
int32_t sl_si91x_read_blockbuffer(uint16_t addr, uint8_t *buff, uint32_t len)
{
  uint8_t a[2];

  a[0] = (uint8_t)(addr & 0xFF);

  I2Cdrv->MasterTransmit(SL_APDS9960_I2C_ADDRESS_DEFAULT, a, 1, false);
  while (I2Cdrv->GetStatus().busy)
    ;

  I2Cdrv->MasterReceive(SL_APDS9960_I2C_ADDRESS_DEFAULT, rd_block_buf, len, false);
  while (I2Cdrv->GetStatus().busy)
    ;

  uint32_t cnt_num = (uint32_t)I2Cdrv->GetDataCount();

  if (cnt_num != (len)) {
    return RSI_FAIL;
    // Acknowledge polling
  }
  /*TODO: Need to verify this after adding the warning macros*/
  //*buff = rd_block_buf;
  memcpy(buff, rd_block_buf, SL_BLOCK_SIZE);
  return (int32_t)cnt_num;
}

/*******************************************************************************
*  @fn          int32_t sl_si91x_write_buffer(uint8_t sub_addr, uint8_t *buf,
*                                                                  uint32_t len)
*  @brief       I2C Write operations
*  @param[in]   sub_addr: Register address from which Primary needs to read the
*               data from secondary
*  @param[in]   buf: Buffer which stores the register address of secondary
*               in which the primary has to write to or read from
*  @param[in]   len: the length of the buffer that needs to be filled
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
int32_t sl_si91x_write_buffer(uint8_t sub_addr, uint8_t apds_data_buf, uint32_t len)
{
  wr_buf[0] = sub_addr;
  wr_buf[1] = apds_data_buf;
  //memcpy((void *)&wr_buf[1], &apds_data_buf, 8);

  I2Cdrv->MasterTransmit(SL_APDS9960_I2C_ADDRESS_DEFAULT, wr_buf, len + 1, false);
  while (I2Cdrv->GetStatus().busy)
    ;

  uint32_t cnt_num = (uint32_t)I2Cdrv->GetDataCount();

  if (cnt_num != (len + 1)) {
    return RSI_FAIL;
    // Acknowledge polling
  }

  return RSI_OK;
}

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
int32_t sl_si91x_read_buffer(uint16_t addr, uint8_t *buffer, uint32_t len)
{
  uint8_t a[2];

  a[0] = (uint8_t)(addr & 0xFF);

  I2Cdrv->MasterTransmit(SL_APDS9960_I2C_ADDRESS_DEFAULT, a, 1, false);
  while (I2Cdrv->GetStatus().busy)
    ;

  I2Cdrv->MasterReceive(SL_APDS9960_I2C_ADDRESS_DEFAULT, rd_buf, len, false);
  while (I2Cdrv->GetStatus().busy)
    ;

  int32_t cnt_num = I2Cdrv->GetDataCount();
  if (cnt_num != (int32_t)(len)) {
    return RSI_FAIL;
    // Acknowledge polling
  }

  *buffer = rd_buf[0];
  return RSI_OK;
}

/*******************************************************************************
*  @fn          uint8_t sl_si91x_apds9960_get_mode(void)
*  @brief       Obtain the SL_ENABLE_APDS9960 register value
*  @param[in]   NULL
*  @return      Returns SL_ENABLE_APDS9960 register value if Success, else
*               0xFF for Failure
*******************************************************************************/
uint8_t sl_si91x_apds9960_get_mode(void)
{
  uint8_t enable_value;

  /* Read current ENABLE register */
  if (sl_si91x_read_buffer(0x80, &enable_value, 1)) {
    DEBUGOUT("Err= %d\n", SL_ERROR);
    return SL_ERROR;
  }
  return enable_value;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_mode(uint8_t mode, uint8_t enable)
*  @brief       Enables or disables the feature in APDS9960
*  @param[in]   mode: Select the mode for the feature to enable
*  @param[in]   enable: If Enable set it to 1, else for Disable set it to 0
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_mode(int8_t mode, uint8_t enable)
{
  uint8_t reg_val;

  /* Read current ENABLE register */
  reg_val = sl_si91x_apds9960_get_mode();

  /* Change bit(s) in ENABLE register */
  enable = enable & 0x01;
  if (mode >= 0 && mode <= 6) {
    if (enable) {
      reg_val |= (uint8_t)(1 << mode);
    } else {
      reg_val &= (uint8_t)(~(1 << mode));
    }
  } else if (mode == SL_ALL) {
    if (enable) {
      reg_val = 0x7F;
    } else {
      reg_val = 0x00;
    }
  }

  /* Write value back to ENABLE register */
  if (sl_si91x_write_buffer(SL_ENABLE_APDS9960, reg_val, 1)) {
    return SL_FAIL;
  }
  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_led_drive(uint8_t drive)
*  @brief       Sets LED strength for proximity and ambient light sensor
*  @param[in]   drive: Select the current with which the LED should be driven
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_led_drive(uint8_t drive)
{
  uint8_t val;

  /* Read value from CONTROL register */
  if (sl_si91x_read_buffer(SL_CONTROL, &val, 1)) {
    return SL_FAIL;
  }

  /* Set bits in register to given value */
  //drive &= 0b00000011;//b2h
  drive &= 0x03;
  drive = drive << 6;
  //val &= 0b00111111;//b2h
  val &= 0x3F;
  val |= drive;

  /* Write register value back into CONTROL register */
  if (sl_si91x_write_buffer(SL_CONTROL, val, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_proximitygain(uint8_t drive)
*  @brief       Sets the receiver gain for Proximity sensing
*  @param[in]   drive: Select the gain value
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_proximitygain(uint8_t drive)
{
  uint8_t val;

  /* Read value from CONTROL register */
  if (sl_si91x_read_buffer(SL_CONTROL, &val, 1)) {
    return SL_FAIL;
  }

  /* Set bits in register to given value */
  // drive &= 0b00000011;b2h
  drive &= 0x03;
  drive = drive << 2;
  //val &= 0b11110011;b2h
  val &= 0xF3;
  val |= drive;

  /* Write register value back into CONTROL register */
  if (sl_si91x_write_buffer(SL_CONTROL, val, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_ambientlight_gain(uint8_t drive)
*  @brief       Sets the receiver gain for Ambient light sensor
*  @param[in]   drive: Select the gain value
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_ambientlight_gain(uint8_t drive)
{
  uint8_t val;

  /* Read value from CONTROL register */
  if (sl_si91x_read_buffer(SL_CONTROL, &val, 1)) {
    return SL_FAIL;
  }

  /* Set bits in register to given value */
  //  drive &= 0b00000011;//b2h
  //  val &= 0b11111100;//b2h
  drive &= 0x03;
  val &= 0xFC;
  val |= drive;

  /* Write register value back into CONTROL register */
  if (sl_si91x_write_buffer(SL_CONTROL, val, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_proximity_interrupt_low_threshold
*                                                            (uint8_t threshold)
*  @brief       Sets the low threshold for proximity interrupts
*  @param[in]   threshold: Select the low threshold value for the interrupt to
*               activate
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_proximity_intterupt_low_threshold(uint8_t threshold)
{
  if (sl_si91x_write_buffer(SL_PILT, threshold, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_proximity_interrupt_high_threshold
*                                                            (uint8_t threshold)
*  @brief       Sets the high threshold for proximity interrupts
*  @param[in]   threshold: Select the high threshold value for the interrupt to
*               activate
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_proximity_intterupt_high_threshold(uint8_t threshold)
{
  if (sl_si91x_write_buffer(SL_PIHT, threshold, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_light_interrupt_low_threshold
*                                                            (uint8_t threshold)
*  @brief       Sets the low threshold for light interrupts
*  @param[in]   threshold: Select the low threshold value for the interrupt to
*               activate
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_light_intterupt_low_threshold(uint16_t threshold)
{
  uint8_t val_low;
  uint8_t val_high;

  /* Break 16-bit threshold into 2 8-bit values */
  val_low  = (uint8_t)(threshold & 0x00FF);
  val_high = (uint8_t)((threshold & 0xFF00) >> 8);

  /* Write low byte */
  if (sl_si91x_write_buffer(SL_AILTL, val_low, 1)) {
    return SL_FAIL;
  }

  /* Write high byte */
  if (sl_si91x_write_buffer(SL_AILTH, val_high, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_light_interrupt_high_threshold
*                                                            (uint8_t threshold)
*  @brief       Sets the high threshold for light interrupts
*  @param[in]   threshold: Select the high threshold value for the interrupt to
*               activate
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_light_intterupt_high_threshold(uint16_t threshold)
{
  uint8_t val_low;
  uint8_t val_high;

  /* Break 16-bit threshold into 2 8-bit values */
  val_low  = (uint8_t)(threshold & 0x00FF);
  val_high = (uint8_t)((threshold & 0xFF00) >> 8);

  /* Write low byte */
  if (sl_si91x_write_buffer(SL_AIHTL, val_low, 1)) {
    return SL_FAIL;
  }

  /* Write high byte */
  if (sl_si91x_write_buffer(SL_AIHTH, val_high, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          uint8_t sl_si91x_apds9960_set_gesture_enter_threshold
*                                                            (uint8_t threshold)
*  @brief       Sets the entry proximity threshold for gesture sensing
*  @param[in]   threshold: Proximity Value needed to start Gesture mode
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
uint8_t sl_si91x_apds9960_set_gesture_enter_threshold(uint8_t threshold)
{
  if (sl_si91x_write_buffer(SL_GPENTH, threshold, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          uint8_t sl_si91x_apds9960_set_gesture_exit_threshold
*                                                            (uint8_t threshold)
*  @brief       Sets the Exit proximity threshold for gesture sensing
*  @param[in]   threshold: Proximity Value needed to stop Gesture mode
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
uint8_t sl_si91x_apds9960_set_gesture_exit_threshold(uint8_t threshold)
{
  if (sl_si91x_write_buffer(SL_GEXTH, threshold, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          uint8_t sl_si91x_apds9960_set_gesture_gain(uint8_t gain)
*  @brief       Sets the gain of the photo-sensor in Gesture mode
*  @param[in]   gain: The value for photo-sensor gain
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
uint8_t sl_si91x_apds9960_set_gesture_gain(uint8_t gain)
{
  uint8_t val;

  /* Read value from GCONF2 register */
  if (sl_si91x_read_buffer(SL_GCONF2, &val, 1)) {
    return SL_FAIL;
  }

  /* Set bits in register to given value */
  //gain &= 0b00000011;//b2h
  gain &= 0x03;
  gain = gain << 5;
  //val &= 0b10011111;//b2h
  val &= 0x9F;
  val |= gain;

  /* Write register value back into GCONF2 register */
  if (sl_si91x_write_buffer(SL_GCONF2, val, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          uint8_t sl_si91x_apds9960_set_gesture_led_drive(uint8_t drive)
*  @brief       Sets the LED current in Gesture mode
*  @param[in]   drive: The value for LED current
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
uint8_t sl_si91x_apds9960_set_gesture_led_drive(uint8_t drive)
{
  uint8_t val;

  /* Read value from GCONF2 register */
  if (sl_si91x_read_buffer(SL_GCONF2, &val, 1)) {
    return SL_FAIL;
  }

  /* Set bits in register to given value */
  //drive &= 0b00000011;//b2h
  drive &= 0x03;
  drive = drive << 3;
  //val &= 0b11100111;//b2h
  val &= 0xE7;
  val |= drive;

  /* Write register value back into GCONF2 register */
  if (sl_si91x_write_buffer(SL_GCONF2, val, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          uint8_t sl_si91x_apds9960_set_gesture_wait_time(uint8_t time)
*  @brief       Sets the wait time between gesture recognition
*  @param[in]   time: The value for wait time
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
uint8_t sl_si91x_apds9960_set_gesture_wait_time(uint8_t time)
{
  uint8_t val;

  /* Read value from GCONF2 register */
  if (sl_si91x_read_buffer(SL_GCONF2, &val, 1)) {
    return SL_FAIL;
  }

  /* Set bits in register to given value */
  //  time &= 0b00000111;
  //  val &= 0b11111000;
  time &= 0x07;
  val &= 0xF8;
  val |= time;

  /* Write register value back into GCONF2 register */
  if (sl_si91x_write_buffer(SL_GCONF2, val, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          uint8_t sl_si91x_apds9960_set_gesture_interruptenable
*                                                               (uint8_t enable)
*  @brief       Enables or Disables Gesture Interrupts
*  @param[in]   enable: The value 1 to enable the interrupt and 0 to disable the
*                                                                      interrupt
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
uint8_t sl_si91x_apds9960_set_gesture_intteruptenable(uint8_t enable)
{

  uint8_t val;
  /* Read value from GCONF4 register */
  if (sl_si91x_read_buffer(SL_GCONF4, &val, 1)) {
    return SL_FAIL;
  }
  /* Set bits in register to given value */
  //enable &= 0b00000001;//b2h
  enable &= 0x01;
  enable = enable << 1;
  //val &= 0b11111101;//b2h
  val &= 0xFD;
  val |= enable;
  /* Write register value back into GCONF4 register */
  if (sl_si91x_write_buffer(SL_GCONF4, val, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_enable_power(void)
*  @brief       Enable the power to APDS9960 sensor
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_enable_power(void)
{
  if (sl_si91x_apds9960_set_mode(SL_POWER, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_ambientlight_interruptenable
*                                                               (uint8_t enable)
*  @brief       Enables or Disables Ambient Light Interrupts
*  @param[in]   enable: The value 1 to enable the interrupt and 0 to disable the
*                                                                      interrupt
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_ambientlight_intteruptenable(uint8_t enable)
{
  uint8_t val;
  /* Read value from ENABLE register */
  if (sl_si91x_read_buffer(SL_ENABLE_APDS9960, &val, 1)) {
    return SL_FAIL;
  }

  /* Set bits in register to given value */
  //enable &= 0b00000001;//b2h
  enable &= 0x01;
  enable = enable << 4;
  //val &= 0b11101111;//b2h
  val &= 0xEF;
  val |= enable;

  /* Write register value back into ENABLE register */
  if (sl_si91x_write_buffer(SL_ENABLE_APDS9960, val, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_sensor_init(void)
*  @brief       This API is used to configure the I2C communications with
*               APDS9960 and initializes all the registers to its default values
*  @param[in]   NULL
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_sensor_init(void)
{
  uint8_t id;

  /* Initialize I2C */

  /* Read ID register and check against known values for APDS-9960 */
  if (sl_si91x_read_buffer(SL_ID, &id, 1)) {

    return RSI_FAIL;
  }
  if (!(id == SL_ID_1_9960 || id == SL_ID_2_9960 || id == SL_ID_3_9960)) {
    DEBUGOUT("APDS9960 ID mismatch: 0x%02X\n", id);
    return RSI_FAIL;
  }

  /* Set ENABLE register to 0 (disable all features) */
  if (sl_si91x_apds9960_set_mode(SL_ALL, SL_OFF)) {

    return RSI_FAIL;
  }
  if (sl_si91x_write_buffer(SL_ATIME, SL_DEFAULT_ATIME, 1)) {
    return RSI_FAIL;
  }
  if (sl_si91x_write_buffer(SL_WTIME, SL_DEFAULT_WTIME, 1)) {
    return RSI_FAIL;
  }
  if (sl_si91x_write_buffer(SL_PPULSE, SL_DEFAULT_PROX_PPULSE, 1)) {
    return RSI_FAIL;
  }
  if (sl_si91x_write_buffer(SL_POFFSET_UR, SL_DEFAULT_POFFSET_UR, 1)) {
    return RSI_FAIL;
  }
  if (sl_si91x_write_buffer(SL_POFFSET_DL, SL_DEFAULT_POFFSET_DL, 1)) {
    return RSI_FAIL;
  }
  if (sl_si91x_write_buffer(SL_CONFIG1, SL_DEFAULT_CONFIG1, 1)) {
    return RSI_FAIL;
  }
  if (sl_si91x_apds9960_set_led_drive(SL_DEFAULT_LDRIVE)) {
    return RSI_FAIL;
  }
  if (sl_si91x_apds9960_set_proximitygain(SL_DEFAULT_PGAIN)) {
    return RSI_FAIL;
  }
  if (sl_si91x_apds9960_set_ambientlight_gain(SL_DEFAULT_AGAIN)) {
    return RSI_FAIL;
  }
  if (sl_si91x_apds9960_set_proximity_intterupt_low_threshold(SL_DEFAULT_PILT)) {
    return RSI_FAIL;
  }
  if (sl_si91x_apds9960_set_proximity_intterupt_high_threshold(SL_DEFAULT_PIHT)) {
    return RSI_FAIL;
  }
  if (sl_si91x_apds9960_set_light_intterupt_low_threshold(SL_DEFAULT_AILT)) {
    return RSI_FAIL;
  }
  if (sl_si91x_apds9960_set_light_intterupt_high_threshold(SL_DEFAULT_AIHT)) {
    return RSI_FAIL;
  }
  if (sl_si91x_write_buffer(SL_PERS, SL_DEFAULT_PERS, 1)) {
    return RSI_FAIL;
  }
  if (sl_si91x_write_buffer(SL_CONFIG2, SL_DEFAULT_CONFIG2, 1)) {
    return RSI_FAIL;
  }
  if (sl_si91x_write_buffer(SL_CONFIG3, SL_DEFAULT_CONFIG3, 1)) {
    return RSI_FAIL;
  }
  if (sl_si91x_apds9960_set_gesture_enter_threshold(SL_DEFAULT_GPENTH)) {
    return RSI_FAIL;
  }
  if (sl_si91x_apds9960_set_gesture_exit_threshold(SL_DEFAULT_GEXTH)) {
    return RSI_FAIL;
  }
  if (sl_si91x_write_buffer(SL_GCONF1, SL_DEFAULT_GCONF1, 1)) {
    return RSI_FAIL;
  }
  if (sl_si91x_apds9960_set_gesture_gain(SL_DEFAULT_GGAIN)) {
    return RSI_FAIL;
  }
  if (sl_si91x_apds9960_set_gesture_led_drive(SL_DEFAULT_GLDRIVE)) {
    return RSI_FAIL;
  }
  if (sl_si91x_apds9960_set_gesture_wait_time(SL_DEFAULT_GWTIME)) {
    return RSI_FAIL;
  }
  if (sl_si91x_write_buffer(SL_GOFFSET_U, SL_DEFAULT_GOFFSET, 1)) {
    return RSI_FAIL;
  }
  if (sl_si91x_write_buffer(SL_GOFFSET_D, SL_DEFAULT_GOFFSET, 1)) {
    return RSI_FAIL;
  }
  if (sl_si91x_write_buffer(SL_GOFFSET_L, SL_DEFAULT_GOFFSET, 1)) {
    return RSI_FAIL;
  }
  if (sl_si91x_write_buffer(SL_GOFFSET_R, SL_DEFAULT_GOFFSET, 1)) {
    return RSI_FAIL;
  }
  if (sl_si91x_write_buffer(SL_GPULSE, SL_DEFAULT_GPULSE, 1)) {
    return RSI_FAIL;
  }
  if (sl_si91x_write_buffer(SL_GCONF3, SL_DEFAULT_GCONF3, 1)) {
    return RSI_FAIL;
  }
  if (sl_si91x_apds9960_set_gesture_intteruptenable(SL_DEFAULT_GIEN)) {
    return RSI_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_set_proximity_interruptenable
*                                                               (uint8_t enable)
*  @brief       Enables or Disables Proximity Interrupts
*  @param[in]   enable: The value 1 to enable the interrupt and 0 to disable the
*                                                                      interrupt
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_set_proximity_intteruptenable(uint8_t enable)
{
  uint8_t val;

  /* Read value from ENABLE register */
  if (sl_si91x_read_buffer(SL_ENABLE_APDS9960, &val, 1)) {
    return SL_FAIL;
  }

  /* Set bits in register to given value */
  //enable &= 0b00000001;//b2h
  enable &= 0x01;
  enable = enable << 5;
  //val &= 0b11011111;//b2h
  val &= 0xDF;
  val |= enable;

  /* Write register value back into ENABLE register */
  if (sl_si91x_write_buffer(SL_ENABLE_APDS9960, val, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_enable_proximitysensor(bool interrupts)
*  @brief       Enables the Proximity Sensing in APDS9960 sensor.
*  @param[in]   interrupts: Enable interrupt for Gesture
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_enable_proximitysensor(bool interrupts)
{
  /* Set default gain, LED, interrupts, enable power, and enable sensor */
  if (sl_si91x_apds9960_set_proximitygain(SL_DEFAULT_PGAIN)) {
    return SL_FAIL;
  }
  if (sl_si91x_apds9960_set_led_drive(SL_DEFAULT_LDRIVE)) {
    return SL_FAIL;
  }
  if (interrupts) {
    if (sl_si91x_apds9960_set_proximity_intteruptenable(1)) {
      return SL_FAIL;
    }
  } else {
    if (sl_si91x_apds9960_set_proximity_intteruptenable(0)) {
      return SL_FAIL;
    }
  }
  if (sl_si91x_apds9960_enable_power()) {
    return SL_FAIL;
  }
  if (sl_si91x_apds9960_set_mode(SL_PROXIMITY, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_read_proximity(uint8_t *value)
*  @brief       Reads the Proximity level
*  @param[in]   value: Value of proximity
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_read_proximity(uint8_t *val)
{

  uint8_t val_byte;
  /* Read value from proximity data register */
  if (sl_si91x_read_buffer(SL_PDATA, &val_byte, 1)) {
    return SL_FAIL;
  }
  *val = val_byte;
  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_enable_lightsensor(bool interrupts)
*  @brief       Activates Light Sensor (R/G/B/Ambient) sensor on the APDS-9960
*  @param[in]   interrupts: Set true to enable hardware interrupt on high or low
*               light
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_enable_lightsensor(bool interrupts)
{

  /* Set default gain, interrupts, enable power, and enable sensor */
  if (sl_si91x_apds9960_set_ambientlight_gain(SL_DEFAULT_AGAIN)) {
    return SL_FAIL;
  }
  if (interrupts) {
    if (sl_si91x_apds9960_set_ambientlight_intteruptenable(1)) {
      return SL_FAIL;
    }
  } else {
    if (sl_si91x_apds9960_set_ambientlight_intteruptenable(0)) {
      return SL_FAIL;
    }
  }
  if (sl_si91x_apds9960_enable_power()) {
    return SL_FAIL;
  }
  if (sl_si91x_apds9960_set_mode(SL_AMBIENT_LIGHT, 1)) {
    return SL_FAIL;
  }

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_read_ambientlight(uint16_t *value)
*  @brief       To read the Ambient light level
*  @param[in]   value: Value of Light sensor
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_read_ambientlight(uint16_t *val)
{
  uint8_t val_byte;

  /* Read value from clear channel, low byte register */
  if (sl_si91x_read_buffer(SL_CDATAL, &val_byte, 1)) {
    return SL_FAIL;
  }
  *val = val_byte;

  /* Read value from clear channel, high byte register */
  if (sl_si91x_read_buffer(SL_CDATAH, &val_byte, 1)) {
    return SL_FAIL;
  }

  *val = (uint8_t)(*val + ((uint16_t)val_byte << 8));

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_read_greenlight(uint16_t *value)
*  @brief       To read the Green light level
*  @param[in]   value: Value of Green Light sensor
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_read_greenlight(uint16_t *val)
{
  uint8_t val_byte;
  *val = 0;

  /* Read value from clear channel, low byte register */
  if (sl_si91x_read_buffer(SL_GDATAL, &val_byte, 1)) {
    return SL_FAIL;
  }
  *val = val_byte;

  /* Read value from clear channel, high byte register */
  if (sl_si91x_read_buffer(SL_GDATAH, &val_byte, 1)) {
    return SL_FAIL;
  }
  *val = (uint8_t)(*val + ((uint16_t)val_byte << 8));

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_read_redlight(uint16_t *value)
*  @brief       To read the Red light level
*  @param[in]   value: Value of Red Light sensor
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_read_redlight(uint16_t *val)
{
  uint8_t val_byte;
  *val = 0;

  /* Read value from clear channel, low byte register */
  if (sl_si91x_read_buffer(SL_RDATAL, &val_byte, 1)) {
    return SL_FAIL;
  }
  *val = val_byte;

  /* Read value from clear channel, high byte register */
  if (sl_si91x_read_buffer(SL_RDATAH, &val_byte, 1)) {
    return SL_FAIL;
  }
  *val = (uint8_t)(*val + ((uint16_t)val_byte << 8));

  return RSI_OK;
}

/*******************************************************************************
*  @fn          bool sl_si91x_apds9960_read_bluelight(uint16_t *value)
*  @brief       To read the Blue light level
*  @param[in]   value: Value of Blue Light sensor
*  @return      Returns RSI_OK if Success, else SL_FAIL for Failure
*******************************************************************************/
bool sl_si91x_apds9960_read_bluelight(uint16_t *val)
{
  uint8_t val_byte;
  *val = 0;

  /* Read value from clear channel, low byte register */
  if (sl_si91x_read_buffer(SL_BDATAL, &val_byte, 1)) {
    return SL_FAIL;
  }
  *val = val_byte;

  /* Read value from clear channel, high byte register */
  if (sl_si91x_read_buffer(SL_BDATAH, &val_byte, 1)) {
    return SL_FAIL;
  }
  *val = (uint8_t)(*val + ((uint16_t)val_byte << 8));

  return RSI_OK;
}

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
sl_apds_err_t sl_si91x_apds9960_sample_colordata(float *r, float *g, float *b, float *w)
{
  if (!is_gpr_init) {
    return RSI_FAIL;
  }
  uint16_t ambient_light = 0;
  uint16_t red_light     = 0;
  uint16_t green_light   = 0;
  uint16_t blue_light    = 0;

  if (!sl_si91x_apds9960_enable_lightsensor(false)) {

  } else {
    DEBUGOUT("\r\n light sensor sample_colordata failed !\r\n");
    return RSI_FAIL;
  }

  if (sl_si91x_apds9960_read_ambientlight(&ambient_light) || sl_si91x_apds9960_read_redlight(&red_light)
      || sl_si91x_apds9960_read_greenlight(&green_light) || sl_si91x_apds9960_read_bluelight(&blue_light)) {
    DEBUGOUT("\r\nError reading light values\r\n");
    return RSI_FAIL;
  } else {
    *r = (float)red_light;
    *g = (float)green_light;
    *b = (float)blue_light;
    *w = (float)ambient_light;
    // sli_si91x_delay(1000);

    return RSI_OK;
  }
}

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_sample_proximitydata(float *data)
*  @brief       Processes and Reads the value of Proximity sensor and sends the
*               value to Sensor Hub
*  @param[in]   data: Value of Proximity data
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_apds_err_t sl_si91x_apds9960_sample_proximitydata(float *data)
{
  if (!is_gpr_init) {
    return RSI_FAIL;
  }
  *data                  = 0;
  uint8_t proximity_data = 0;
  if (sl_si91x_apds9960_set_proximitygain(SL_PGAIN_2X)) {
    DEBUGOUT("\r\n Set PGAIN wrong\r\n");
  }

  // Start running the APDS-9960 proximity sensor (no interrupts)
  if (!sl_si91x_apds9960_enable_proximitysensor(false)) {
  } else {
    DEBUGOUT("\r\n enable proximity sensor fail apds9960!\r\n");
  }

  if (sl_si91x_apds9960_read_proximity(&proximity_data)) {
    DEBUGOUT("\r\n Error reading proximity value\r\n");
    return RSI_FAIL;
  } else {
    *data = (float)proximity_data;
    return RSI_OK;
  }
}

/*******************************************************************************
*  @fn          void sli_si91x_delay(uint32_t idelay) //1.002ms delay
*  @brief       This function produce the delay by using the for loop.
*  @param[in]   idelay: Delay value
*  @return      NONE
*******************************************************************************/
void sli_si91x_delay(uint32_t idelay) //1.002ms delay
{
  for (uint32_t x = 0; x < 4600 * idelay; x++) {
    __NOP();
  }
}

/*******************************************************************************
*  @fn          sl_err_t sl_si91x_apds9960_sample_gesturedata(char *data)
*  @brief       Processes and Reads the value of Gesture sensor and sends the
*               value to Sensor Hub
*  @param[in]   data: Value of Gesture data
*  @return      Returns RSI_OK if Success, else RSI_FAIL for Failure
*******************************************************************************/
sl_apds_err_t sl_si91x_apds9960_sample_gesturedata(char *data)
{
  int direction = 0;
  if (!sl_si91x_apds9960_enable_gesturesensor(true)) {
    //DEBUGOUT("\r\nGesture sensor is now running\r\n");
  } else {
    DEBUGOUT("\r\nSomething went wrong during gesture sensor sl_si91x_apds9960_init!\r\n");
    return RSI_FAIL;
  }
  if (!sl_si91x_apds9960_is_gesture_available()) {
    direction = sl_si91x_apds9960_read_gesture();
    DEBUGOUT("\r\n *********** direction ********* %d \r\n", direction);
    switch (direction) {

      case SL_DIR_UP:
        DEBUGOUT("\r\nUP\r\n");
        *data = 'U';
        break;
      case SL_DIR_DOWN:
        DEBUGOUT("\r\nDOWN\r\n");
        *data = 'D';
        break;
      case SL_DIR_LEFT:
        DEBUGOUT("\r\nLEFT\r\n");
        *data = 'L';
        break;
      case SL_DIR_RIGHT:
        DEBUGOUT("\r\nRIGHT\r\n");
        *data = 'R';
        break;
      case SL_DIR_NEAR:
        DEBUGOUT("\r\nNEAR\r\n");
        *data = 'N';
        break;
      case SL_DIR_FAR:
        DEBUGOUT("\r\nFAR\r\n");
        *data = 'F';
        break;
      default:
        DEBUGOUT("\r\nNONE\r\n");
    }
  }
  return RSI_OK;
}
