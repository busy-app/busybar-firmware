#ifndef SL_ICM40627_DEFS_H
#define SL_ICM40627_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/***************************************************************************//**
 * @addtogroup icm40627 ICM240627 - Motion Sensor
 * @brief Driver for the Invensense ICM40627 6-axis motion sensor.
 * @{
 ******************************************************************************/
/***************************************************************************//**
 * @addtogroup icm40627_details Register definitions
 * @brief Register definitions.
 * @{
 ******************************************************************************/
/**************************************************************************//**
* @name ICM40627 register banks
* @{
******************************************************************************/
#define ICM40627_BANK_0                (uint16_t)(0 << 7)     /**< Register bank 0 */
#define ICM40627_BANK_1                (uint16_t)(1 << 7)     /**< Register bank 1 */
#define ICM40627_BANK_2                (uint16_t)(2 << 7)     /**< Register bank 2 */
#define ICM40627_BANK_3                (uint16_t)(3 << 7)     /**< Register bank 3 */
#define ICM40627_BANK_4                (uint16_t)(4 << 7)     /**< Register bank 4 */

/**@}*/

/**************************************************************************//**
* @name Register and associated bit definitions
* @{
******************************************************************************/
/***********************/
/* Bank 0 register map */
/***********************/
#define ICM40627_REG_DEVICE_CONFIG (ICM40627_BANK_0 | 0x11) /**< DEVICE_CONFIG >**/
#define ICM40627_BIT_SPI_MODE 4 /**< SPI mode selection >**/
#define ICM40627_BIT_SOFT_RESET_CONFIG 0 /**< Software reset configuration >**/
#define ICM40627_BIT_SOFT_RESET_CONFIG_ENABLE (1 << ICM40627_BIT_SOFT_RESET_CONFIG) /**< Enable the Software reset configuration >**/

#define ICM40627_REG_DRIVE_CONFIG (ICM40627_BANK_0 | 0x13) /**< DRIVE_CONFIG >**/
#define ICM40627_SHIFT_I2C_SLEW_RATE 3 /**< Controls slew rate for output pin 14 in I2C mode only >**/
#define ICM40627_I2C_SLEW_RATE_VALUE_0 (0 << ICM40627_SHIFT_I2C_SLEW_RATE) /**< 20 ns-60 ns >**/
#define ICM40627_I2C_SLEW_RATE_VALUE_1 (1 << ICM40627_SHIFT_I2C_SLEW_RATE) /**< 12 ns-36 ns >**/
#define ICM40627_I2C_SLEW_RATE_VALUE_10 (2 << ICM40627_SHIFT_I2C_SLEW_RATE) /**< 6 ns-18 ns >**/
#define ICM40627_I2C_SLEW_RATE_VALUE_11 (3 << ICM40627_SHIFT_I2C_SLEW_RATE) /**< 4 ns-12 ns >**/
#define ICM40627_I2C_SLEW_RATE_VALUE_100 (4 << ICM40627_SHIFT_I2C_SLEW_RATE) /**< 2 ns-6 ns >**/
#define ICM40627_I2C_SLEW_RATE_VALUE_101 (5 << ICM40627_SHIFT_I2C_SLEW_RATE) /**< < 2 ns >**/
#define ICM40627_I2C_SLEW_RATE_VALUE_110 (6 << ICM40627_SHIFT_I2C_SLEW_RATE) /**< Reserved >**/
#define ICM40627_I2C_SLEW_RATE_VALUE_111 (7 << ICM40627_SHIFT_I2C_SLEW_RATE) /**< Reserved >**/
#define ICM40627_SHIFT_SPI_SLEW_RATE 0 /**< Controls slew rate for output pin 14 in SPI mode, and for all other outputpins >**/
#define ICM40627_SPI_SLEW_RATE_VALUE_0 (0 << ICM40627_SHIFT_SPI_SLEW_RATE) /**< 20 ns-60 ns >**/
#define ICM40627_SPI_SLEW_RATE_VALUE_1 (1 << ICM40627_SHIFT_SPI_SLEW_RATE) /**< 12 ns-36 ns >**/
#define ICM40627_SPI_SLEW_RATE_VALUE_10 (2 << ICM40627_SHIFT_SPI_SLEW_RATE) /**< 6 ns-18 ns >**/
#define ICM40627_SPI_SLEW_RATE_VALUE_11 (3 << ICM40627_SHIFT_SPI_SLEW_RATE) /**< 4 ns-12 ns >**/
#define ICM40627_SPI_SLEW_RATE_VALUE_100 (4 << ICM40627_SHIFT_SPI_SLEW_RATE) /**< 2 ns-6 ns >**/
#define ICM40627_SPI_SLEW_RATE_VALUE_101 (5 << ICM40627_SHIFT_SPI_SLEW_RATE) /**< < 2 ns >**/
#define ICM40627_SPI_SLEW_RATE_VALUE_110 (6 << ICM40627_SHIFT_SPI_SLEW_RATE) /**< Reserved >**/
#define ICM40627_SPI_SLEW_RATE_VALUE_111 (7 << ICM40627_SHIFT_SPI_SLEW_RATE) /**< Reserved >**/

#define ICM40627_REG_INT_CONFIG (ICM40627_BANK_0 | 0x14) /**< INT_CONFIG >**/
#define ICM40627_BIT_INT2_MODE 5 /**< INT2 interrupt mode >**/
#define ICM40627_BIT_INT2_DRIVE_CIRCUIT 4 /**< INT2 drive circuit >**/
#define ICM40627_BIT_INT2_POLARITY 3 /**< INT2 interrupt polarity >**/
#define ICM40627_BIT_INT1_MODE 2 /**< INT1 interrupt mode >**/
#define ICM40627_BIT_INT1_DRIVE_CIRCUIT 1 /**< INT1 drive circuit >**/
#define ICM40627_BIT_INT1_DRIVE_CIRCUIT_OPEN_DRAIN (0 << ICM40627_BIT_INT1_DRIVE_CIRCUIT) /**< INT1 drive circuit-Open Drain>**/
#define ICM40627_BIT_INT1_DRIVE_CIRCUIT_PUSH_PULL (1 << ICM40627_BIT_INT1_DRIVE_CIRCUIT) /**< INT1 drive circuit- Push Pull >**/
#define ICM40627_BIT_INT1_POLARITY 0 /**< INT1 interrupt polarity >**/
#define ICM40627_BIT_INT1_POLARITY_ACTL (0 << ICM40627_BIT_INT1_POLARITY) /**< INT1 interrupt polarity- Active Low>**/
#define ICM40627_BIT_INT1_POLARITY_ACTH (1 << ICM40627_BIT_INT1_POLARITY) /**< INT1 interrupt polarity- Active High>**/

#define ICM40627_REG_FIFO_CONFIG (ICM40627_BANK_0 | 0x16) /**< FIFO_CONFIG >**/
#define ICM40627_SHIFT_FIFO_MODE 6
#define ICM40627_FIFO_MODE_BYPASS_MODE (0 << ICM40627_SHIFT_FIFO_MODE) /**< Bypass Mode (default) >**/
#define ICM40627_FIFO_MODE_STREAM_TO_FIFO (1 << ICM40627_SHIFT_FIFO_MODE) /**< Stream-to-FIFO Mode >**/
#define ICM40627_FIFO_MODE_STOP_ON_FULL (2 << ICM40627_SHIFT_FIFO_MODE) /**< STOP-on-FULL Mode >**/
#define ICM40627_FIFO_MODE_VALUE_11 (3 << ICM40627_SHIFT_FIFO_MODE) /**< STOP-on-FULL Mode >**/

#define ICM40627_REG_FIFO_HEADER_MSG 7 /**<1: FIFO is empty 0: Packet contains sensor data>**/
#define ICM40627_REG_FIFO_HEADER_ACCEL 6 /*1: Packet is sized so that accel data have location in the packet, FIFO_ACCEL_EN must be 1 0: Packet does not contain accel sample*/
#define ICM40627_REG_FIFO_HEADER_GYRO 5 /*1: Packet is sized so that gyro data have location in the packet, FIFO_GYRO_EN must be 1 0: Packet does not contain gyro sample*/
#define ICM40627_REG_FIFO_HEADER_TIMESTAMP_FSYNC 2 /*00: Packet does not contain timestamp or FSYNC time data 01: Reserved 10: Packet contains ODR Timestamp 11: Packet contains FSYNC time, and this packet is flagged as first ODR after FSYNC (only if FIFO_TMST_FSYNC_EN is 1)*/
#define ICM40627_REG_FIFO_HEADER_ODR_ACCEL 1 /*1: The ODR for accel is different for this accel data packet compared to the previous accel packet 0: The ODR for accel is the same as the previous packet with accel*/
#define ICM40627_REG_FIFO_HEADER_ODR_GYRO 0 /*1: The ODR for gyro is different for this gyro data packet compared to the previous gyro packet 0: The ODR for gyro is the same as the previous packet with gyro*/
#define ICM40627_REG_FIFO_HEADER_MSG_EMPTY (1 << ICM40627_REG_FIFO_HEADER_MSG)/**<1: FIFO is empty>**/
#define ICM40627_REG_FIFO_HEADER_ACCEL_MASK (1 << ICM40627_REG_FIFO_HEADER_ACCEL) /*FIFO consists of Accel data */
#define ICM40627_REG_FIFO_HEADER_GYRO_MASK (1 << ICM40627_REG_FIFO_HEADER_GYRO) /*FIFO consists of Gyro data */

#define ICM40627_REG_TEMP_DATA1 (ICM40627_BANK_0 | 0x1D) /**< TEMP_DATA1 >**/
#define ICM40627_SHIFT_TEMP_DATA_HIGH 0 /**< Upper byte of temperature data >**/

#define ICM40627_REG_TEMP_DATA0 (ICM40627_BANK_0 | 0x1E) /**< TEMP_DATA0 >**/
#define ICM40627_SHIFT_TEMP_DATA_LOW 0 /**< Lower byte of temperature data >**/

#define ICM40627_REG_ACCEL_DATA_X1 (ICM40627_BANK_0 | 0x1F) /**< ACCEL_DATA_X1 >**/
#define ICM40627_SHIFT_ACCEL_DATA_X_HIGH 0 /**< Upper byte of Accel X-axis data >**/

#define ICM40627_REG_ACCEL_DATA_X0 (ICM40627_BANK_0 | 0x20) /**< ACCEL_DATA_X0 >**/
#define ICM40627_SHIFT_ACCEL_DATA_X_LOW 0 /**< Lower byte of Accel X-axis data >**/

#define ICM40627_REG_ACCEL_DATA_Y1 (ICM40627_BANK_0 | 0x21) /**< ACCEL_DATA_Y1 >**/
#define ICM40627_SHIFT_ACCEL_DATA_Y_HIGH 0 /**< Upper byte of Accel Y-axis data >**/

#define ICM40627_REG_ACCEL_DATA_Y0 (ICM40627_BANK_0 | 0x22) /**< ACCEL_DATA_Y0 >**/
#define ICM40627_SHIFT_ACCEL_DATA_Y_LOW 0 /**< Lower byte of Accel Y-axis data >**/

#define ICM40627_REG_ACCEL_DATA_Z1 (ICM40627_BANK_0 | 0x23) /**< ACCEL_DATA_Z1 >**/
#define ICM40627_SHIFT_ACCEL_DATA_Z_HIGH 0 /**< Upper byte of Accel Z-axis data >**/

#define ICM40627_REG_ACCEL_DATA_Z0 (ICM40627_BANK_0 | 0x24) /**< ACCEL_DATA_Z0 >**/
#define ICM40627_SHIFT_ACCEL_DATA_Z_LOW 0 /**< Lower byte of Accel Z-axis data >**/

#define ICM40627_REG_GYRO_DATA_X1 (ICM40627_BANK_0 | 0x25) /**< GYRO_DATA_X1 >**/
#define ICM40627_SHIFT_GYRO_DATA_X_HIGH 0 /**< Upper byte of Gyro X-axis data >**/

#define ICM40627_REG_GYRO_DATA_X0 (ICM40627_BANK_0 | 0x26) /**< GYRO_DATA_X0 >**/
#define ICM40627_SHIFT_GYRO_DATA_X_LOW 0 /**< Lower byte of Gyro X-axis data >**/

#define ICM40627_REG_GYRO_DATA_Y1 (ICM40627_BANK_0 | 0x27) /**< GYRO_DATA_Y1 >**/
#define ICM40627_SHIFT_GYRO_DATA_Y_HIGH 0 /**< Upper byte of Gyro Y-axis data >**/

#define ICM40627_REG_GYRO_DATA_Y0 (ICM40627_BANK_0 | 0x28) /**< GYRO_DATA_Y0 >**/
#define ICM40627_SHIFT_GYRO_DATA_Y_LOW 0 /**< Lower byte of Gyro Y-axis data >**/

#define ICM40627_REG_GYRO_DATA_Z1 (ICM40627_BANK_0 | 0x29) /**< GYRO_DATA_Z1 >**/
#define ICM40627_SHIFT_GYRO_DATA_Z_HIGH 0 /**< Upper byte of Gyro Z-axis data >**/

#define ICM40627_REG_GYRO_DATA_Z0 (ICM40627_BANK_0 | 0x2A) /**< GYRO_DATA_Z0 >**/
#define ICM40627_SHIFT_GYRO_DATA_Z_LOW 0 /**< Lower byte of Gyro Z-axis data >**/

#define ICM40627_REG_TMST_FSYNCH (ICM40627_BANK_0 | 0x2B) /**< TMST_FSYNCH >**/
#define ICM40627_SHIFT_TMST_FSYNC_DATA_HIGH 0 /**< Stores the upper byte of the time delta from the rising edge of FSYNC tothe latest ODR until the UI Interface reads the FSYNC tag in the statusregister >**/

#define ICM40627_REG_TMST_FSYNCL (ICM40627_BANK_0 | 0x2C) /**< TMST_FSYNCL >**/
#define ICM40627_SHIFT_TMST_FSYNC_DATA_LOW 0 /**< Stores the lower byte of the time delta from the rising edge of FSYNC tothe latest ODR until the UI Interface reads the FSYNC tag in the statusregister >**/

#define ICM40627_REG_INT_STATUS (ICM40627_BANK_0 | 0x2D) /**< INT_STATUS >**/
#define ICM40627_BIT_UI_FSYNC_INT 6 /**< This bit automatically sets to 1 when a UI FSYNC interrupt is generated. Thebit clears to 0 after the register has been read. >**/
#define ICM40627_BIT_PLL_RDY_INT 5 /**< This bit automatically sets to 1 when a PLL Ready interrupt is generated. Thebit clears to 0 after the register has been read. >**/
#define ICM40627_BIT_RESET_DONE_INT 4 /**< This bit automatically sets to 1 when software reset is complete. The bitclears to 0 after the register has been read. >**/
#define ICM40627_BIT_DATA_RDY_INT 3 /**< This bit automatically sets to 1 when a Data Ready interrupt is generated.The bit clears to 0 after the register has been read. >**/
#define ICM40627_BIT_FIFO_THS_INT 2 /**< This bit automatically sets to 1 when the FIFO buffer reaches the thresholdvalue. The bit clears to 0 after the register has been read. >**/
#define ICM40627_BIT_FIFO_FULL_INT 1 /**< This bit automatically sets to 1 when the FIFO buffer is full. The bit clears to 0after the register has been read. >**/
#define ICM40627_BIT_AGC_RDY_INT 0 /**< This bit automatically sets to 1 when an AGC Ready interrupt is generated.The bit clears to 0 after the register has been read. >**/
#define ICM40627_MASK_DATA_RDY_INT (1 << ICM40627_BIT_DATA_RDY_INT)
#define ICM40627_MASK_RESET_DONE_INT (1 << ICM40627_BIT_RESET_DONE_INT)

#define ICM40627_REG_FIFO_COUNTH (ICM40627_BANK_0 | 0x2E) /**< FIFO_COUNTH >**/
#define ICM40627_SHIFT_FIFO_COUNT_HIGH 0 /**< High Bits, count indicates the number of records or bytes available in FIFOaccording to FIFO_COUNT_REC setting.Note: Must read FIFO_COUNTL to latch new data for both FIFO_COUNTHand FIFO_COUNTL. >**/

#define ICM40627_REG_FIFO_COUNTL (ICM40627_BANK_0 | 0x2F) /**< FIFO_COUNTL >**/
#define ICM40627_SHIFT_FIFO_COUNT_LOW 0 /**< Low Bits, count indicates the number of records or bytes available in FIFOaccording to FIFO_COUNT_REC setting.Reading this byte latches the data for both FIFO_COUNTH, andFIFO_COUNTL. >**/

#define ICM40627_REG_FIFO_DATA (ICM40627_BANK_0 | 0x30) /**< FIFO_DATA >**/
#define ICM40627_SHIFT_FIFO_DATA 0 /**< FIFO data port >**/

#define ICM40627_REG_APEX_DATA0 (ICM40627_BANK_0 | 0x31) /**< APEX_DATA0 >**/
#define ICM40627_SHIFT_STEP_CNT_LOW 0 /**< Pedometer Output: Lower byte of Step Count measured by pedometer >**/

#define ICM40627_REG_APEX_DATA1 (ICM40627_BANK_0 | 0x32) /**< APEX_DATA1 >**/
#define ICM40627_SHIFT_STEP_CNT_HIGH 0 /**< Pedometer Output: Upper byte of Step Count measured by pedometer >**/

#define ICM40627_REG_APEX_DATA2 (ICM40627_BANK_0 | 0x33) /**< APEX_DATA2 >**/
#define ICM40627_SHIFT_STEP_CADENCE 0 /**< Pedometer Output: Walk/run cadency in number of samples. Format is u6.2.e.g. At 50 Hz ODR and 2 Hz walk frequency, the cadency is 25 samples. Theregister will output 100. >**/

#define ICM40627_REG_APEX_DATA3 (ICM40627_BANK_0 | 0x34) /**< APEX_DATA3 >**/
#define ICM40627_BIT_DMP_IDLE 2
#define ICM40627_SHIFT_ACTIVITY_CLASS 0 /**< Pedometer Output: Detected activity >**/
#define ICM40627_ACTIVITY_CLASS_VALUE_0 (0 << ICM40627_SHIFT_ACTIVITY_CLASS) /**< Unknown >**/
#define ICM40627_ACTIVITY_CLASS_VALUE_1 (1 << ICM40627_SHIFT_ACTIVITY_CLASS) /**< Walk >**/
#define ICM40627_ACTIVITY_CLASS_VALUE_10 (2 << ICM40627_SHIFT_ACTIVITY_CLASS) /**< Run >**/
#define ICM40627_ACTIVITY_CLASS_VALUE_11 (3 << ICM40627_SHIFT_ACTIVITY_CLASS) /**< Reserved >**/

#define ICM40627_REG_APEX_DATA4 (ICM40627_BANK_0 | 0x35) /**< APEX_DATA4 >**/
#define ICM40627_SHIFT_TAP_NUM 3 /**< Tap Detection Output: Number of taps in the current Tap event >**/
#define ICM40627_TAP_NUM_VALUE_0 (0 << ICM40627_SHIFT_TAP_NUM) /**< No tap >**/
#define ICM40627_TAP_NUM_VALUE_1 (1 << ICM40627_SHIFT_TAP_NUM) /**< Single tap >**/
#define ICM40627_TAP_NUM_VALUE_10 (2 << ICM40627_SHIFT_TAP_NUM) /**< Double tap >**/
#define ICM40627_TAP_NUM_VALUE_11 (3 << ICM40627_SHIFT_TAP_NUM) /**< Reserved >**/
#define ICM40627_SHIFT_TAP_AXIS 1 /**< Tap Detection Output: Represents the accelerometer axis on which tapenergy is concentrated >**/
#define ICM40627_TAP_AXIS_VALUE_0 (0 << ICM40627_SHIFT_TAP_AXIS) /**< X-axis >**/
#define ICM40627_TAP_AXIS_VALUE_1 (1 << ICM40627_SHIFT_TAP_AXIS) /**< Y-axis >**/
#define ICM40627_TAP_AXIS_VALUE_10 (2 << ICM40627_SHIFT_TAP_AXIS) /**< Z-axis >**/
#define ICM40627_TAP_AXIS_VALUE_11 (3 << ICM40627_SHIFT_TAP_AXIS) /**< Reserved >**/
#define ICM40627_BIT_TAP_DIR 0 /**< Tap Detection Output: Polarity of tap pulsevalue >**/

#define ICM40627_REG_APEX_DATA5 (ICM40627_BANK_0 | 0x36) /**< APEX_DATA5 >**/
#define ICM40627_SHIFT_DOUBLE_TAP_TIMING 0 /**< DOUBLE_TAP_TIMING measures the time interval between the two tapswhen double tap is detected.  It counts every 16 accelerometer samples asone unit between the 2 tap pulses. Therefore, the value is related to theaccelerometer ODR.Time in seconds = DOUBLE_TAP_TIMING * 16 / ODRFor example, if the accelerometer ODR is 500 Hz, and theDOUBLE_TAP_TIMING register reading is 6, the time interval value is6*16/500 = 0.192s. >**/

#define ICM40627_REG_INT_STATUS2 (ICM40627_BANK_0 | 0x37) /**< INT_STATUS2 >**/
#define ICM40627_BIT_SMD_INT 3 /**< Significant Motion Detection Interrupt, clears on read >**/
#define ICM40627_BIT_WOM_Z_INT 2 /**< Wake on Motion Interrupt on Z-axis, clears on read >**/
#define ICM40627_BIT_WOM_Y_INT 1 /**< Wake on Motion Interrupt on Y-axis, clears on read >**/
#define ICM40627_BIT_WOM_X_INT 0 /**< Wake on Motion Interrupt on X-axis, clears on read >**/

#define ICM40627_REG_INT_STATUS3 (ICM40627_BANK_0 | 0x38) /**< INT_STATUS3 >**/
#define ICM40627_BIT_STEP_DET_INT 5 /**< Step Detection Interrupt, clears on read >**/
#define ICM40627_BIT_STEP_CNT_OVF_INT 4 /**< Step Count Overflow Interrupt, clears on read >**/
#define ICM40627_BIT_TILT_DET_INT 3 /**< Tilt Detection Interrupt, clears on read >**/
#define ICM40627_BIT_WAKE_INT 2 /**< Wake Event Interrupt, clears on read >**/
#define ICM40627_BIT_SLEEP_INT 1 /**< Sleep Event Interrupt, clears on read >**/
#define ICM40627_BIT_TAP_DET_INT 0 /**< Tap Detection Interrupt, clears on read >**/
#define ICM40627_BIT_MASK_WAKE_INT 0x04 /**< Wake Event Interrupt, clears on read >**/
#define ICM40627_BIT_MASK_SLEEP_INT 0x02 /**< Sleep Event Interrupt, clears on read >**/

#define ICM40627_REG_SIGNAL_PATH_RESET (ICM40627_BANK_0 | 0x4B) /**< SIGNAL_PATH_RESET >**/
#define ICM40627_BIT_DMP_INIT_EN 6 /**< When this bit is set to 1, the DMP is enabled >**/
#define ICM40627_BIT_DMP_MEM_RESET_EN 5 /**< When this bit is set to 1, the DMP memory is reset >**/
#define ICM40627_BIT_ABORT_AND_RESET 3 /**< When this bit is set to 1, the signal path is reset by restarting the ODRcounter and signal path controls >**/
#define ICM40627_BIT_TMST_STROBE 2 /**< When this bit is set to 1, the time stamp counter is latched into the timestamp register. This is a write on clear bit. >**/
#define ICM40627_BIT_FIFO_FLUSH 1 /**< When set to 1, FIFO will get flushed. >**/
#define ICM40627_MASK_BIT_ABORT_AND_RESET (1 << ICM40627_BIT_ABORT_AND_RESET)
#define ICM40627_MASK_BIT_FIFO_FLUSH (1 << ICM40627_BIT_FIFO_FLUSH)

#define ICM40627_REG_INTF_CONFIG0 (ICM40627_BANK_0 | 0x4C) /**< INTF_CONFIG0 >**/
#define ICM40627_BIT_FIFO_HOLD_LAST_DATA_E 7 /**< Setting 0 corresponds to the following:Sense Registers from Power on Reset till first sample:Sense Registers after first sample received:FIFO:Setting 1 corresponds to the following:Sense Registers from Power on Reset till first sample:Sense Registers after first sample received:FIFO: >**/
#define ICM40627_BIT_FIFO_COUNT_REC 6 /**< accel + temp sensor data + time stamp, or 8 bytes for header + gyro/accel +temp sensor data) >**/
#define ICM40627_BIT_FIFO_COUNT_ENDIAN 5
#define ICM40627_BIT_SENSOR_DATA_ENDIAN 4
#define ICM40627_SHIFT_UI_SIFS_CFG 0 /**< 0x: Reserved >**/
#define ICM40627_UI_SIFS_CFG_VALUE_0 (2 << ICM40627_SHIFT_UI_SIFS_CFG) /**< Disable SPI >**/
#define ICM40627_UI_SIFS_CFG_VALUE_1 (3 << ICM40627_SHIFT_UI_SIFS_CFG) /**< Disable I2C >**/
#define ICM40627_MASK_BIT_FIFO_COUNT_REC (1 << ICM40627_BIT_FIFO_COUNT_REC)

#define ICM40627_REG_INTF_CONFIG1 (ICM40627_BANK_0 | 0x4D) /**< INTF_CONFIG1 >**/
#define ICM40627_BIT_ACCEL_LP_CLK_SEL 3
#define ICM40627_BIT_ACCEL_LP_CLK_SEL_MASK (1 << ICM40627_BIT_ACCEL_LP_CLK_SEL)
#define ICM40627_SHIFT_CLKSEL 0
#define ICM40627_CLKSEL_VALUE_0 (0 << ICM40627_SHIFT_CLKSEL) /**< Always select internal RC oscillator >**/
#define ICM40627_CLKSEL_VALUE_1 (1 << ICM40627_SHIFT_CLKSEL) /**< Select PLL when available, else select RC oscillator (default) >**/
#define ICM40627_CLKSEL_VALUE_10 (2 << ICM40627_SHIFT_CLKSEL) /**< Reserved >**/
#define ICM40627_CLKSEL_VALUE_11 (3 << ICM40627_SHIFT_CLKSEL) /**< Disable all clocks >**/

#define ICM40627_REG_PWR_MGMT0 (ICM40627_BANK_0 | 0x4E) /**< PWR_MGMT0 >**/
#define ICM40627_BIT_TEMP_DIS 5 /**< Temperature sensor >**/
#define ICM40627_BIT_IDLE 4 /**< If this bit is set to 1, the RC oscillator is powered on even if Accel and Gyroare powered off.Nominally this bit is set to 0, so when Accel and Gyro are powered off,the chip will go to OFF state, since the RC oscillator will also be powered off00: Turns gyroscope off (default)01: Places gyroscope in Standby Mode10: Reserved >**/
#define ICM40627_SHIFT_GYRO_MODE 2 /**< Gyroscope needs to be kept ON for a minimum of 45 ms. When transitioningfrom OFF to any of the other modes, do not issue any register writes for200 µs. >**/
#define ICM40627_GYRO_MODE_VALUE_0 (0 << ICM40627_SHIFT_GYRO_MODE) /**<  Turns gyroscope off (default) >**/
#define ICM40627_GYRO_MODE_VALUE_1 (1 << ICM40627_SHIFT_GYRO_MODE) /**<  Places gyroscope in Standby Mode >**/
#define ICM40627_GYRO_MODE_VALUE_10 (2 << ICM40627_SHIFT_GYRO_MODE) /**<  Reserved >**/
#define ICM40627_GYRO_MODE_VALUE_11 (3 << ICM40627_SHIFT_GYRO_MODE) /**<  Places gyroscope in Low Noise (LN) Mode >**/
#define ICM40627_SHIFT_ACCEL_MODE 0 /**< When transitioning from OFF to any of the other modes, do not issue anyregister writes for 200 µs. >**/
#define ICM40627_ACCEL_MODE_VALUE_0 (0 << ICM40627_SHIFT_ACCEL_MODE) /**< Turns accelerometer off (default) >**/
#define ICM40627_ACCEL_MODE_VALUE_1 (1 << ICM40627_SHIFT_ACCEL_MODE) /**<  Turns accelerometer off >**/
#define ICM40627_ACCEL_MODE_VALUE_10 (2 << ICM40627_SHIFT_ACCEL_MODE) /**<  Places accelerometer in Low Power (LP) Mode >**/
#define ICM40627_ACCEL_MODE_VALUE_11 (3 << ICM40627_SHIFT_ACCEL_MODE) /**<  Places accelerometer in Low Noise (LN) Mode >**/
#define ICM40627_PWR_MGMT0_ACCEL_MODE_MASK 0x03
#define ICM40627_PWR_MGMT0_ACCEL_MODE_LN 0x03 /**< Enable accelerometer standby mode */
#define ICM40627_PWR_MGMT0_ACCEL_MODE_LP 0x02 /**< Enable accelerometer Low power */
#define ICM40627_PWR_MGMT0_ACCEL_MODE_DIS 0x3C /**< Disable accelerometer */
#define ICM40627_PWR_MGMT0_GYRO_MODE_LN  0x0C /**< Enable gyroscope */
#define ICM40627_PWR_MGMT0_GYRO_MODE_DIS 0x33  /**< Disable gyroscope */
#define ICM40627_PWR_MGMT0_TEMP_MODE_EN 0x1F /**< Enable temperature sensor */
#define ICM40627_PWR_MGMT0_TEMP_MODE_DIS 0x20  /**< Disable temperature sensor */

#define ICM40627_REG_GYRO_CONFIG0 (ICM40627_BANK_0 | 0x4F) /**< GYRO_CONFIG0 >**/
#define ICM40627_SHIFT_GYRO_FS_SEL 5 /**< Full scale select for gyroscope UI interface output >**/
#define ICM40627_MASK_GYRO_FS_SEL 0xE0 /**< Bitmask for Full scale select for gyroscope  >**/
#define ICM40627_GYRO_FS_SEL_VALUE_2000 (0 << ICM40627_SHIFT_GYRO_FS_SEL) /**< ±2000 dps (default) >**/
#define ICM40627_GYRO_FS_SEL_VALUE_1000 (1 << ICM40627_SHIFT_GYRO_FS_SEL) /**< ±1000 dps >**/
#define ICM40627_GYRO_FS_SEL_VALUE_500 (2 << ICM40627_SHIFT_GYRO_FS_SEL) /**< ±500 dps >**/
#define ICM40627_GYRO_FS_SEL_VALUE_250 (3 << ICM40627_SHIFT_GYRO_FS_SEL) /**< ±250 dps >**/
#define ICM40627_GYRO_FS_SEL_VALUE_125 (4 << ICM40627_SHIFT_GYRO_FS_SEL) /**< ±125 dps >**/
#define ICM40627_GYRO_FS_SEL_VALUE_62_5 (5 << ICM40627_SHIFT_GYRO_FS_SEL) /**< ±62.5 dps >**/
#define ICM40627_GYRO_FS_SEL_VALUE_31_25 (6 << ICM40627_SHIFT_GYRO_FS_SEL) /**< ±31.25 dps >**/
#define ICM40627_GYRO_FS_SEL_VALUE_15_625 (7 << ICM40627_SHIFT_GYRO_FS_SEL) /**< ±15.625 dps >**/
#define ICM40627_SHIFT_GYRO_ODR 0 /**< Gyroscope ODR selection for UI interface output >**/
#define ICM40627_MASK_GYRO_ODR_VALUE 0x0F
#define ICM40627_GYRO_ODR_VALUE_0 (0 << ICM40627_SHIFT_GYRO_ODR) /**< Reserved >**/
#define ICM40627_GYRO_ODR_VALUE_1 (1 << ICM40627_SHIFT_GYRO_ODR) /**< Reserved >**/
#define ICM40627_GYRO_ODR_VALUE_10 (2 << ICM40627_SHIFT_GYRO_ODR) /**< Reserved >**/
#define ICM40627_GYRO_ODR_VALUE_8000 (3 << ICM40627_SHIFT_GYRO_ODR) /**< 8 kHz >**/
#define ICM40627_GYRO_ODR_VALUE_4000 (4 << ICM40627_SHIFT_GYRO_ODR) /**< 4 kHz >**/
#define ICM40627_GYRO_ODR_VALUE_2000 (5 << ICM40627_SHIFT_GYRO_ODR) /**< 2 kHz >**/
#define ICM40627_GYRO_ODR_VALUE_1000 (6 << ICM40627_SHIFT_GYRO_ODR) /**< 1 kHz >**/
#define ICM40627_GYRO_ODR_VALUE_200 (7 << ICM40627_SHIFT_GYRO_ODR) /**< 200 Hz >**/
#define ICM40627_GYRO_ODR_VALUE_100 (8 << ICM40627_SHIFT_GYRO_ODR) /**< 100 Hz >**/
#define ICM40627_GYRO_ODR_VALUE_50 (9 << ICM40627_SHIFT_GYRO_ODR) /**< 50 Hz >**/
#define ICM40627_GYRO_ODR_VALUE_25 (0x0A << ICM40627_SHIFT_GYRO_ODR) /**< 25Hz >**/
#define ICM40627_GYRO_ODR_VALUE_12_5 (0x0B << ICM40627_SHIFT_GYRO_ODR) /**< 12.5 Hz >**/
#define ICM40627_GYRO_ODR_VALUE_1100 (0x0C << ICM40627_SHIFT_GYRO_ODR) /**< Reserved >**/
#define ICM40627_GYRO_ODR_VALUE_1101 (0x0D << ICM40627_SHIFT_GYRO_ODR) /**< Reserved >**/
#define ICM40627_GYRO_ODR_VALUE_1110 (0x0E << ICM40627_SHIFT_GYRO_ODR) /**< Reserved >**/
#define ICM40627_GYRO_ODR_VALUE_500 (0x0F << ICM40627_SHIFT_GYRO_ODR) /**< 500Hz >**/

#define ICM40627_REG_ACCEL_CONFIG0 (ICM40627_BANK_0 | 0x50) /**< ACCEL_CONFIG0 >**/
#define ICM40627_SHIFT_ACCEL_FS_SEL 5 /**< Full scale select for accelerometer UI interface output >**/
#define ICM40627_ACCEL_FS_SEL_VALUE_16G (0 << ICM40627_SHIFT_ACCEL_FS_SEL) /**< ±16g (default) >**/
#define ICM40627_ACCEL_FS_SEL_VALUE_8G (1 << ICM40627_SHIFT_ACCEL_FS_SEL) /**< ±8g >**/
#define ICM40627_ACCEL_FS_SEL_VALUE_4G (2 << ICM40627_SHIFT_ACCEL_FS_SEL) /**< ±4g >**/
#define ICM40627_ACCEL_FS_SEL_VALUE_2G (3 << ICM40627_SHIFT_ACCEL_FS_SEL) /**< ±2g >**/
#define ICM40627_ACCEL_FS_SEL_VALUE_100 (4 << ICM40627_SHIFT_ACCEL_FS_SEL) /**< Reserved >**/
#define ICM40627_ACCEL_FS_SEL_VALUE_101 (5 << ICM40627_SHIFT_ACCEL_FS_SEL) /**< Reserved >**/
#define ICM40627_ACCEL_FS_SEL_VALUE_110 (6 << ICM40627_SHIFT_ACCEL_FS_SEL) /**< Reserved >**/
#define ICM40627_ACCEL_FS_SEL_VALUE_111 (7 << ICM40627_SHIFT_ACCEL_FS_SEL) /**< Reserved >**/
#define ICM40627_MASK_ACCEL_FS_SEL 0xE0 /*Accel Full Scale Select bitmask */
#define ICM40627_SHIFT_ACCEL_ODR 0 /**< Accelerometer ODR selection for UI interface output >**/
#define ICM40627_ACCEL_ODR_VALUE_0 (0 << ICM40627_SHIFT_ACCEL_ODR) /**< Reserved >**/
#define ICM40627_ACCEL_ODR_VALUE_1 (1 << ICM40627_SHIFT_ACCEL_ODR) /**< Reserved >**/
#define ICM40627_ACCEL_ODR_VALUE_10 (2 << ICM40627_SHIFT_ACCEL_ODR) /**< Reserved >**/
#define ICM40627_ACCEL_ODR_VALUE_8000 (3 << ICM40627_SHIFT_ACCEL_ODR) /**< 8 kHz (LN mode) >**/
#define ICM40627_ACCEL_ODR_VALUE_4000 (4 << ICM40627_SHIFT_ACCEL_ODR) /**< 4 kHz (LN mode) >**/
#define ICM40627_ACCEL_ODR_VALUE_2000 (5 << ICM40627_SHIFT_ACCEL_ODR) /**< 2 kHz (LN mode) >**/
#define ICM40627_ACCEL_ODR_VALUE_1000 (6 << ICM40627_SHIFT_ACCEL_ODR) /**< 1 kHz (LN mode) >**/
#define ICM40627_ACCEL_ODR_VALUE_200 (7 << ICM40627_SHIFT_ACCEL_ODR) /**< 200 Hz (LP or LN mode) >**/
#define ICM40627_ACCEL_ODR_VALUE_100 (8 << ICM40627_SHIFT_ACCEL_ODR) /**< 100 Hz (LP or LN mode) >**/
#define ICM40627_ACCEL_ODR_VALUE_50 (9 << ICM40627_SHIFT_ACCEL_ODR) /**< 50 Hz (LP or LN mode) >**/
#define ICM40627_ACCEL_ODR_VALUE_25 (0x0A << ICM40627_SHIFT_ACCEL_ODR) /**< 25Hz (LP or LN mode) >**/
#define ICM40627_ACCEL_ODR_VALUE_12_5 (0x0B << ICM40627_SHIFT_ACCEL_ODR) /**< 12.5 Hz (LP or LN mode) >**/
#define ICM40627_ACCEL_ODR_VALUE_6_25 (0x0C << ICM40627_SHIFT_ACCEL_ODR) /**< 6.25Hz (LP mode) >**/
#define ICM40627_ACCEL_ODR_VALUE_3_125 (0x0D << ICM40627_SHIFT_ACCEL_ODR) /**< 3.125 Hz (LP mode) >**/
#define ICM40627_ACCEL_ODR_VALUE_1_5625 (0x0E << ICM40627_SHIFT_ACCEL_ODR) /**< 1.5625Hz (LP mode) >**/
#define ICM40627_ACCEL_ODR_VALUE_500 (0x0F << ICM40627_SHIFT_ACCEL_ODR) /**< 500 Hz (LP or LN mode) >**/
#define ICM40627_MASK_ACCEL_ODR_VALUE 0x0F

#define ICM40627_REG_GYRO_CONFIG1 (ICM40627_BANK_0 | 0x51) /**< GYRO_CONFIG1 >**/
#define ICM40627_SHIFT_TEMP_FILT_BW 5 /**< Sets the bandwidth of the temperature signal DLPF >**/
#define ICM40627_TEMP_FILT_BW_VALUE_0 (0 << ICM40627_SHIFT_TEMP_FILT_BW) /**< DLPF BW = 4000 Hz; DLPF Latency = 0.125ms (default) >**/
#define ICM40627_TEMP_FILT_BW_VALUE_1 (1 << ICM40627_SHIFT_TEMP_FILT_BW) /**< DLPF BW = 170 Hz; DLPF Latency = 1ms >**/
#define ICM40627_TEMP_FILT_BW_VALUE_10 (2 << ICM40627_SHIFT_TEMP_FILT_BW) /**< DLPF BW = 82Hz; DLPF Latency = 2ms >**/
#define ICM40627_TEMP_FILT_BW_VALUE_11 (3 << ICM40627_SHIFT_TEMP_FILT_BW) /**< DLPF BW = 40 Hz; DLPF Latency = 4 ms >**/
#define ICM40627_TEMP_FILT_BW_VALUE_100 (4 << ICM40627_SHIFT_TEMP_FILT_BW) /**< DLPF BW = 20Hz; DLPF Latency = 8 ms >**/
#define ICM40627_TEMP_FILT_BW_VALUE_101 (5 << ICM40627_SHIFT_TEMP_FILT_BW) /**< DLPF BW = 10 Hz; DLPF Latency = 16 ms >**/
#define ICM40627_TEMP_FILT_BW_VALUE_110 (6 << ICM40627_SHIFT_TEMP_FILT_BW) /**< DLPF BW = 5Hz; DLPF Latency = 32 ms >**/
#define ICM40627_TEMP_FILT_BW_VALUE_111 (7 << ICM40627_SHIFT_TEMP_FILT_BW) /**< DLPF BW = 5 Hz; DLPF Latency = 32 ms >**/
#define ICM40627_SHIFT_GYRO_UI_FILT_ORD 2 /**< Selects order of GYRO UI filter >**/
#define ICM40627_GYRO_UI_FILT_ORD_VALUE_0 (0 << ICM40627_SHIFT_GYRO_UI_FILT_ORD) /**< 1st Order >**/
#define ICM40627_GYRO_UI_FILT_ORD_VALUE_1 (1 << ICM40627_SHIFT_GYRO_UI_FILT_ORD) /**< 2nd Order >**/
#define ICM40627_GYRO_UI_FILT_ORD_VALUE_10 (2 << ICM40627_SHIFT_GYRO_UI_FILT_ORD) /**< 3rd Order >**/
#define ICM40627_GYRO_UI_FILT_ORD_VALUE_11 (3 << ICM40627_SHIFT_GYRO_UI_FILT_ORD) /**< Reserved >**/
#define ICM40627_SHIFT_GYRO_DEC2_M2_ORD 0 /**< Selects order of GYRO DEC2_M2 Filter >**/
#define ICM40627_GYRO_DEC2_M2_ORD_VALUE_0 (0 << ICM40627_SHIFT_GYRO_DEC2_M2_ORD) /**< Reserved >**/
#define ICM40627_GYRO_DEC2_M2_ORD_VALUE_1 (1 << ICM40627_SHIFT_GYRO_DEC2_M2_ORD) /**< Reserved >**/
#define ICM40627_GYRO_DEC2_M2_ORD_VALUE_10 (2 << ICM40627_SHIFT_GYRO_DEC2_M2_ORD) /**< 3rd Order >**/
#define ICM40627_GYRO_DEC2_M2_ORD_VALUE_11 (3 << ICM40627_SHIFT_GYRO_DEC2_M2_ORD) /**< Reserved >**/

#define ICM40627_REG_GYRO_ACCEL_CONFIG0 (ICM40627_BANK_0 | 0x52) /**< GYRO_ACCEL_CONFIG0 >**/
#define ICM40627_SHIFT_ACCEL_UI_FILT_BW 4 /**< LN Mode:Bandwidth for Accel LPF0 BW=ODR/21 BW=max(400 Hz, ODR)/4 (default)2 BW=max(400 Hz, ODR)/53 BW=max(400 Hz, ODR)/84 BW=max(400 Hz, ODR)/105 BW=max(400 Hz, ODR)/166 BW=max(400 Hz, ODR)/207 BW=max(400 Hz, ODR)/408 to 13: Reserved14 Low Latency option: Trivial decimation @ ODR of Dec2 filter output. Dec2runs at max(400 Hz, ODR)15 Low Latency option: Trivial decimation @ ODR of Dec2 filter output. Dec2runs at max(200 Hz, 8*ODR)LP Mode:0 Reserved1 1x AVG filter (default)2 to 5 Reserved6 16x AVG filter7 to 15 ReservedLN Mode:Bandwidth for Gyro LPF0 BW=ODR/21 BW=max(400 Hz, ODR)/4 (default)2 BW=max(400 Hz, ODR)/53 BW=max(400 Hz, ODR)/84 BW=max(400 Hz, ODR)/10 >**/
#define ICM40627_SHIFT_GYRO_UI_FILT_BW 0 /**< 5 BW=max(400 Hz, ODR)/166 BW=max(400 Hz, ODR)/207 BW=max(400 Hz, ODR)/408 to 13: Reserved14 Low Latency option: Trivial decimation @ ODR of Dec2 filter output. Dec2runs at max(400 Hz, ODR)15 Low Latency option: Trivial decimation @ ODR of Dec2 filter output. Dec2runs at max(200 Hz, 8*ODR) >**/
#define ICM40627_MASK_GYRO_UI_FILT_BW 0x0F /*Gyro band-width select bit-mask */
#define ICM40627_MASK_ACCEL_UI_FILT_BW 0xF0 /*Accelerometer band-width select bit-mask */
/* Bandwidth for Gyro */
#define ICM40627_GYRO_UI_FILT_BW_VALUE_0 (0 << ICM40627_SHIFT_GYRO_UI_FILT_BW) /**< BW = ODR / 2 */
#define ICM40627_GYRO_UI_FILT_BW_VALUE_1 (1 << ICM40627_SHIFT_GYRO_UI_FILT_BW) /**< BW = max(400 Hz, ODR) / 4 (default) */
#define ICM40627_GYRO_UI_FILT_BW_VALUE_2 (2 << ICM40627_SHIFT_GYRO_UI_FILT_BW) /**< BW = max(400 Hz, ODR) / 5 */
#define ICM40627_GYRO_UI_FILT_BW_VALUE_3 (3 << ICM40627_SHIFT_GYRO_UI_FILT_BW) /**< BW = max(400 Hz, ODR) / 8 */
#define ICM40627_GYRO_UI_FILT_BW_VALUE_4 (4 << ICM40627_SHIFT_GYRO_UI_FILT_BW) /**< BW = max(400 Hz, ODR) / 10 */
#define ICM40627_GYRO_UI_FILT_BW_VALUE_5 (5 << ICM40627_SHIFT_GYRO_UI_FILT_BW) /**< BW = max(400 Hz, ODR) / 16 */
#define ICM40627_GYRO_UI_FILT_BW_VALUE_6 (6 << ICM40627_SHIFT_GYRO_UI_FILT_BW) /**< BW = max(400 Hz, ODR) / 20 */
#define ICM40627_GYRO_UI_FILT_BW_VALUE_7 (7 << ICM40627_SHIFT_GYRO_UI_FILT_BW) /**< BW = max(400 Hz, ODR) / 40 */
#define ICM40627_GYRO_UI_FILT_BW_VALUE_8 (14 << ICM40627_SHIFT_GYRO_UI_FILT_BW) /**< BW =  max(400 Hz, ODR) */
#define ICM40627_GYRO_UI_FILT_BW_VALUE_9 (15 << ICM40627_SHIFT_GYRO_UI_FILT_BW) /**< BW = max(200 Hz, 8*ODR) */
/* LN Mode: Bandwidth for Accel LPF */
#define ICM40627_ACCEL_UI_FILT_BW_VALUE_0 (0 << ICM40627_SHIFT_ACCEL_UI_FILT_BW) /**< BW = ODR / 2 */
#define ICM40627_ACCEL_UI_FILT_BW_VALUE_1 (1 << ICM40627_SHIFT_ACCEL_UI_FILT_BW) /**< BW = max(400 Hz, ODR) / 4 (default) */
#define ICM40627_ACCEL_UI_FILT_BW_VALUE_2 (2 << ICM40627_SHIFT_ACCEL_UI_FILT_BW) /**< BW = max(400 Hz, ODR) / 5 */
#define ICM40627_ACCEL_UI_FILT_BW_VALUE_3 (3 << ICM40627_SHIFT_ACCEL_UI_FILT_BW) /**< BW = max(400 Hz, ODR) / 8 */
#define ICM40627_ACCEL_UI_FILT_BW_VALUE_4 (4 << ICM40627_SHIFT_ACCEL_UI_FILT_BW) /**< BW = max(400 Hz, ODR) / 10 */
#define ICM40627_ACCEL_UI_FILT_BW_VALUE_5 (5 << ICM40627_SHIFT_ACCEL_UI_FILT_BW) /**< BW = max(400 Hz, ODR) / 16 */
#define ICM40627_ACCEL_UI_FILT_BW_VALUE_6 (6 << ICM40627_SHIFT_ACCEL_UI_FILT_BW) /**< BW = max(400 Hz, ODR) / 20 */
#define ICM40627_ACCEL_UI_FILT_BW_VALUE_7 (7 << ICM40627_SHIFT_ACCEL_UI_FILT_BW) /**< BW = max(400 Hz, ODR) / 40 */
#define ICM40627_ACCEL_UI_FILT_BW_VALUE_8 (14 << ICM40627_SHIFT_ACCEL_UI_FILT_BW) /**< BW =  max(400 Hz, ODR) */
#define ICM40627_ACCEL_UI_FILT_BW_VALUE_9 (15 << ICM40627_SHIFT_ACCEL_UI_FILT_BW) /**< BW = max(200 Hz, 8*ODR) */
/* LP Mode: Bandwidth for Accel LPF */
#define ICM40627_ACCEL_UI_FILT_BW_LP_VALUE_1 (1 << ICM40627_SHIFT_ACCEL_UI_FILT_BW) /**< 1x AVG filter (default) */
#define ICM40627_ACCEL_UI_FILT_BW_LP_VALUE_6 (6 << ICM40627_SHIFT_ACCEL_UI_FILT_BW) /**< 16x AVG filter */

#define ICM40627_REG_ACCEL_CONFIG1 (ICM40627_BANK_0 | 0x53) /**< ACCEL_CONFIG1 >**/
#define ICM40627_SHIFT_ACCEL_UI_FILT_ORD 3 /**< Selects order of ACCEL UI filter >**/
#define ICM40627_ACCEL_UI_FILT_ORD_VALUE_0 (0 << ICM40627_SHIFT_ACCEL_UI_FILT_ORD) /**< 1st Order >**/
#define ICM40627_ACCEL_UI_FILT_ORD_VALUE_1 (1 << ICM40627_SHIFT_ACCEL_UI_FILT_ORD) /**< 2nd Order >**/
#define ICM40627_ACCEL_UI_FILT_ORD_VALUE_10 (2 << ICM40627_SHIFT_ACCEL_UI_FILT_ORD) /**< 3rd Order >**/
#define ICM40627_ACCEL_UI_FILT_ORD_VALUE_11 (3 << ICM40627_SHIFT_ACCEL_UI_FILT_ORD) /**< Reserved >**/
#define ICM40627_SHIFT_ACCEL_DEC2_M2_ORD 1 /**< Order of Accelerometer DEC2_M2 filter >**/
#define ICM40627_ACCEL_DEC2_M2_ORD_VALUE_0 (0 << ICM40627_SHIFT_ACCEL_DEC2_M2_ORD) /**< Reserved >**/
#define ICM40627_ACCEL_DEC2_M2_ORD_VALUE_1 (1 << ICM40627_SHIFT_ACCEL_DEC2_M2_ORD) /**< Reserved >**/
#define ICM40627_ACCEL_DEC2_M2_ORD_VALUE_10 (2 << ICM40627_SHIFT_ACCEL_DEC2_M2_ORD) /**< 3rd order >**/
#define ICM40627_ACCEL_DEC2_M2_ORD_VALUE_11 (3 << ICM40627_SHIFT_ACCEL_DEC2_M2_ORD) /**< Reserved >**/

#define ICM40627_REG_TMST_CONFIG (ICM40627_BANK_0 | 0x54) /**< TMST_CONFIG >**/
#define ICM40627_BIT_TMST_TO_REGS_EN 4
#define ICM40627_BIT_TMST_RES 3 /**< Time Stamp resolution: When set to 0 (default), time stamp resolution is1 µs. When set to 1, resolution is 16 µs >**/
#define ICM40627_BIT_TMST_DELTA_EN 2 /**< Time Stamp delta enable: When set to 1, the time stamp field contains themeasurement of time since the last occurrence of ODR.Time Stamp register FSYNC enable (default). When set to 1, the contents of >**/
#define ICM40627_BIT_TMST_FSYNC_EN 1 /**< the Timestamp feature of FSYNC is enabled. The user also needs to selectFIFO_TMST_FSYNC_EN in order to propagate the timestamp value to theFIFO. >**/
#define ICM40627_BIT_TMST_EN 0

#define ICM40627_REG_APEX_CONFIG0 (ICM40627_BANK_0 | 0x56) /**< APEX_CONFIG0 >**/
#define ICM40627_BIT_DMP_POWER_SAVE 7
#define ICM40627_BIT_TAP_ENABLE 6
#define ICM40627_BIT_PED_ENABLE 5 /**< values supported by Tap Detection (200 Hz, 500 Hz, 1 kHz) >**/
#define ICM40627_BIT_TILT_ENABLE 4
#define ICM40627_BIT_R2W_EN 3
#define ICM40627_SHIFT_DMP_ODR 0
#define ICM40627_DMP_ODR_VALUE_0 (0 << ICM40627_SHIFT_DMP_ODR) /**< 25 Hz >**/
#define ICM40627_DMP_ODR_VALUE_1 (1 << ICM40627_SHIFT_DMP_ODR) /**< Reserved >**/
#define ICM40627_DMP_ODR_VALUE_10 (2 << ICM40627_SHIFT_DMP_ODR) /**< 50 Hz >**/
#define ICM40627_DMP_ODR_VALUE_11 (3 << ICM40627_SHIFT_DMP_ODR) /**< Reserved >**/

#define ICM40627_REG_SMD_CONFIG (ICM40627_BANK_0 | 0x57) /**< SMD_CONFIG >**/
#define ICM40627_BIT_WOM_INT_MODE 3 /**< WOM_INT_MODE >**/
#define ICM40627_BIT_WOM_MODE 2 /**< WOM_MODE >**/
#define ICM40627_BIT_WOM_MODE_MASK (1 << ICM40627_BIT_WOM_MODE)
#define ICM40627_SHIFT_SMD_MODE 0 /**< detected 1 sec apart >**/
#define ICM40627_SMD_MODE_VALUE_0 (0 << ICM40627_SHIFT_SMD_MODE) /**< SMD disabled >**/
#define ICM40627_SMD_MODE_VALUE_1 (1 << ICM40627_SHIFT_SMD_MODE) /**< WOM mode >**/
#define ICM40627_SMD_MODE_VALUE_10 (2 << ICM40627_SHIFT_SMD_MODE) /**< SMD short (1 sec wait) An SMD event is detected when two WOM are >**/
#define ICM40627_SMD_MODE_VALUE_11 (3 << ICM40627_SHIFT_SMD_MODE) /**< SMD long (3 sec wait) An SMD event is detected when two WOM are >**/

#define ICM40627_REG_FIFO_CONFIG1 (ICM40627_BANK_0 | 0x5F) /**< FIFO_CONFIG1 >**/
#define ICM40627_BIT_FIFO_RESUME_PARTIAL_RD 6
#define ICM40627_BIT_FIFO_WM_GT_TH 5 /**< Trigger FIFO watermark interrupt on every ODR (DMA write) if >**/
#define ICM40627_BIT_FIFO_TMST_FSYNC_EN 3 /**< Must be set to 1 for all use cases >**/
#define ICM40627_BIT_FIFO_TEMP_EN 2 /**< Enable temperature sensor packets to go to FIFO >**/
#define ICM40627_BIT_FIFO_GYRO_EN 1 /**< Enable gyroscope packets to go to FIFO >**/
#define ICM40627_BIT_FIFO_ACCEL_EN 0 /**< Enable accelerometer packets to go to FIFO >**/
#define ICM40627_MASK_FIFO_TMST_FSYNC_EN (1 << ICM40627_BIT_FIFO_TMST_FSYNC_EN) /**< Must be set to 1 for all use cases >**/
#define ICM40627_MASK_FIFO_TEMP_EN (1 << ICM40627_BIT_FIFO_TEMP_EN) /**< Enable temperature sensor packets to go to FIFO >**/
#define ICM40627_MASK_FIFO_GYRO_EN (1 << ICM40627_BIT_FIFO_GYRO_EN) /**< Enable gyroscope packets to go to FIFO >**/
#define ICM40627_MASK_FIFO_ACCEL_EN (1 << ICM40627_BIT_FIFO_ACCEL_EN) /**< Enable accelerometer packets to go to FIFO >**/
#define ICM40627_MASK_FIFO_WM_GT_TH (1 << ICM40627_BIT_FIFO_WM_GT_TH)

#define ICM40627_REG_FIFO_CONFIG2 (ICM40627_BANK_0 | 0x60) /**< FIFO_CONFIG2 >**/
#define ICM40627_SHIFT_FIFO_WM_LOW 0 /**< Lower bits of FIFO watermark. Generate interrupt when the FIFO reachesor exceeds FIFO_WM size in bytes or records according toFIFO_COUNT_REC setting. FIFO_WM_EN must be zero before writing thisregister. Interrupt only fires once. This register should be set to non-zerovalue, before choosing this interrupt source. >**/

#define ICM40627_REG_FIFO_CONFIG3 (ICM40627_BANK_0 | 0x61) /**< FIFO_CONFIG3 >**/
#define ICM40627_SHIFT_FIFO_WM_HIGH 0 /**< Upper bits of FIFO watermark. Generate interrupt when the FIFO reachesor exceeds FIFO_WM size in bytes or records according toFIFO_COUNT_REC setting. FIFO_WM_EN must be zero before writing thisregister. Interrupt only fires once. This register should be set to non-zerovalue, before choosing this interrupt source. >**/

#define ICM40627_REG_FSYNC_CONFIG (ICM40627_BANK_0 | 0x62) /**< FSYNC_CONFIG >**/
#define ICM40627_SHIFT_FSYNC_UI_SEL 4
#define ICM40627_FSYNC_UI_SEL_VALUE_0 (0 << ICM40627_SHIFT_FSYNC_UI_SEL) /**< Do not tag FSYNC flag >**/
#define ICM40627_FSYNC_UI_SEL_VALUE_1 (1 << ICM40627_SHIFT_FSYNC_UI_SEL) /**< Tag FSYNC flag to TEMP_OUT LSB >**/
#define ICM40627_FSYNC_UI_SEL_VALUE_10 (2 << ICM40627_SHIFT_FSYNC_UI_SEL) /**< Tag FSYNC flag to GYRO_XOUT LSB >**/
#define ICM40627_FSYNC_UI_SEL_VALUE_11 (3 << ICM40627_SHIFT_FSYNC_UI_SEL) /**< Tag FSYNC flag to GYRO_YOUT LSB >**/
#define ICM40627_FSYNC_UI_SEL_VALUE_100 (4 << ICM40627_SHIFT_FSYNC_UI_SEL) /**< Tag FSYNC flag to GYRO_ZOUT LSB >**/
#define ICM40627_FSYNC_UI_SEL_VALUE_101 (5 << ICM40627_SHIFT_FSYNC_UI_SEL) /**< Tag FSYNC flag to ACCEL_XOUT LSB >**/
#define ICM40627_FSYNC_UI_SEL_VALUE_110 (6 << ICM40627_SHIFT_FSYNC_UI_SEL) /**< Tag FSYNC flag to ACCEL_YOUT LSB >**/
#define ICM40627_FSYNC_UI_SEL_VALUE_111 (7 << ICM40627_SHIFT_FSYNC_UI_SEL) /**< Tag FSYNC flag to ACCEL_ZOUT LSB >**/
#define ICM40627_BIT_ 1
#define ICM40627_BIT_FSYNC_POLARITY 0 /**< FSYNC tagged axis >**/

#define ICM40627_REG_INT_CONFIG0 (ICM40627_BANK_0 | 0x63) /**< INT_CONFIG0 >**/
#define ICM40627_SHIFT_UI_DRDY_INT_CLEAR 4 /**< Data Ready Interrupt Clear Option (latched mode) >**/
#define ICM40627_UI_DRDY_INT_CLEAR_VALUE_0 (0 << ICM40627_SHIFT_UI_DRDY_INT_CLEAR) /**< Clear on Status Bit Read (default) >**/
#define ICM40627_UI_DRDY_INT_CLEAR_VALUE_1 (1 << ICM40627_SHIFT_UI_DRDY_INT_CLEAR) /**< Clear on Status Bit Read >**/
#define ICM40627_UI_DRDY_INT_CLEAR_VALUE_10 (2 << ICM40627_SHIFT_UI_DRDY_INT_CLEAR) /**< Clear on FIFO data 1Byte Read >**/
#define ICM40627_UI_DRDY_INT_CLEAR_VALUE_11 (3 << ICM40627_SHIFT_UI_DRDY_INT_CLEAR) /**< Clear on Status Bit Read AND on Sensor Register read >**/
#define ICM40627_SHIFT_FIFO_THS_INT_CLEAR 2 /**< FIFO Threshold Interrupt Clear Option (latched mode) >**/
#define ICM40627_FIFO_THS_INT_CLEAR_VALUE_0 (0 << ICM40627_SHIFT_FIFO_THS_INT_CLEAR) /**< Clear on Status Bit Read (default) >**/
#define ICM40627_FIFO_THS_INT_CLEAR_VALUE_1 (1 << ICM40627_SHIFT_FIFO_THS_INT_CLEAR) /**< Clear on Status Bit Read >**/
#define ICM40627_FIFO_THS_INT_CLEAR_VALUE_10 (2 << ICM40627_SHIFT_FIFO_THS_INT_CLEAR) /**< Clear on FIFO data 1Byte Read >**/
#define ICM40627_FIFO_THS_INT_CLEAR_VALUE_11 (3 << ICM40627_SHIFT_FIFO_THS_INT_CLEAR) /**< Clear on Status Bit Read AND on FIFO data 1 byte read >**/
#define ICM40627_SHIFT_FIFO_FULL_INT_CLEAR 0 /**< FIFO Full Interrupt Clear Option (latched mode) >**/
#define ICM40627_FIFO_FULL_INT_CLEAR_VALUE_0 (0 << ICM40627_SHIFT_FIFO_FULL_INT_CLEAR) /**< Clear on Status Bit Read (default) >**/
#define ICM40627_FIFO_FULL_INT_CLEAR_VALUE_1 (1 << ICM40627_SHIFT_FIFO_FULL_INT_CLEAR) /**< Clear on Status Bit Read >**/
#define ICM40627_FIFO_FULL_INT_CLEAR_VALUE_10 (2 << ICM40627_SHIFT_FIFO_FULL_INT_CLEAR) /**< Clear on FIFO data 1Byte Read >**/
#define ICM40627_FIFO_FULL_INT_CLEAR_VALUE_11 (3 << ICM40627_SHIFT_FIFO_FULL_INT_CLEAR) /**< Clear on Status Bit Read AND on FIFO data 1 byte read >**/

#define ICM40627_REG_INT_CONFIG1 (ICM40627_BANK_0 | 0x64) /**< INT_CONFIG1 >**/
#define ICM40627_BIT_INT_TPULSE_DURATION 6 /**< Interrupt pulse duration< 4 kHz.Interrupt de-assertion duration >**/
#define ICM40627_BIT_INT_TDEASSERT_DISABLE 5 /**< only if ODR < 4 kHz. (Default)4 kHz. >**/
#define ICM40627_BIT_INT_ASYNC_RESET 4 /**< User should change setting to 0 from default setting of 1, for proper INT1and INT2 pin operation >**/
#define ICM40627_BIT_INT_ASYNC_RESET_0 (0 << ICM40627_BIT_INT_ASYNC_RESET) /**< setting to 0 from default setting of 1, for proper INT1and INT2 pin operation >**/

#define ICM40627_REG_INT_SOURCE0 (ICM40627_BANK_0 | 0x65) /**< INT_SOURCE0 >**/
#define ICM40627_BIT_UI_FSYNC_INT1_EN 6 /**< UI FSYNC interrupt >**/
#define ICM40627_BIT_PLL_RDY_INT1_EN 5 /**< PLL ready interrupt >**/
#define ICM40627_BIT_RESET_DONE_INT1_EN 4 /**< Reset done interrupt >**/
#define ICM40627_BIT_UI_DRDY_INT1_EN 3 /**<  UI data ready interrupt >**/
#define ICM40627_MASK_UI_DRDY_INT1_EN (1 << ICM40627_BIT_UI_DRDY_INT1_EN) /**<  UI data ready interrupt Bit Mask >**/
#define ICM40627_BIT_FIFO_THS_INT1_EN 2 /**<  FIFO threshold interrupt >**/
#define ICM40627_BIT_FIFO_FULL_INT1_EN 1 /**< FIFO full interrupt >**/
#define ICM40627_BIT_UI_AGC_RDY_INT1_EN 0 /**< UI AGC ready interrupt >**/

#define ICM40627_REG_INT_SOURCE1 (ICM40627_BANK_0 | 0x66) /**< INT_SOURCE1 >**/
#define ICM40627_BIT_SMD_INT1_EN 3 /**< SMD interrupt >**/
#define ICM40627_BIT_WOM_Z_INT1_EN 2 /**< Z-axis WOM interrupt >**/
#define ICM40627_BIT_WOM_Y_INT1_EN 1 /**< Y-axis WOM interrupt >**/
#define ICM40627_BIT_WOM_X_INT1_EN 0 /**< X-axis WOM interrupt >**/
#define ICM40627_BIT_MASK_SMD_INT1_EN  (1 << ICM40627_BIT_SMD_INT1_EN)/**< SMD Bitmask interrupt routed to INT1 >**/
#define ICM40627_BIT_MASK_WOM_Z_INT1_EN (1 << ICM40627_BIT_WOM_Z_INT1_EN) /**< Z-axis WOM interrupt routed to INT1 >**/
#define ICM40627_BIT_MASK_WOM_Y_INT1_EN (1 << ICM40627_BIT_WOM_Y_INT1_EN) /**< Y-axis WOM interrupt routed to INT1 >**/
#define ICM40627_BIT_MASK_WOM_X_INT1_EN (1 << ICM40627_BIT_WOM_X_INT1_EN) /**< X-axis WOM interrupt routed to INT1 >**/

#define ICM40627_REG_INT_SOURCE3 (ICM40627_BANK_0 | 0x68) /**< INT_SOURCE3 >**/
#define ICM40627_BIT_UI_FSYNC_INT2_EN 6
#define ICM40627_BIT_PLL_RDY_INT2_EN 5
#define ICM40627_BIT_RESET_DONE_INT2_EN 4
#define ICM40627_BIT_UI_DRDY_INT2_EN 3
#define ICM40627_BIT_FIFO_THS_INT2_EN 2
#define ICM40627_BIT_FIFO_FULL_INT2_EN 1
#define ICM40627_BIT_UI_AGC_RDY_INT2_EN 0

#define ICM40627_REG_INT_SOURCE4 (ICM40627_BANK_0 | 0x69) /**< INT_SOURCE4 >**/
#define ICM40627_BIT_SMD_INT2_EN 3
#define ICM40627_BIT_WOM_Z_INT2_EN 2
#define ICM40627_BIT_WOM_Y_INT2_EN 1
#define ICM40627_BIT_WOM_X_INT2_EN 0

#define ICM40627_REG_FIFO_LOST_PKT0 (ICM40627_BANK_0 | 0x6C) /**< FIFO_LOST_PKT0 >**/
#define ICM40627_SHIFT_FIFO_LOST_PKT_CNT_LOW 0 /**< Low byte, number of packets lost in the FIFO >**/

#define ICM40627_REG_FIFO_LOST_PKT1 (ICM40627_BANK_0 | 0x6D) /**< FIFO_LOST_PKT1 >**/
#define ICM40627_SHIFT_FIFO_LOST_PKT_CNT_HIGH 0 /**< High byte, number of packets lost in the FIFO >**/

#define ICM40627_REG_SELF_TEST_CONFIG (ICM40627_BANK_0 | 0x70) /**< SELF_TEST_CONFIG >**/
#define ICM40627_BIT_ACCEL_ST_POWER 6 /**< Set to 1 for accel self-testOtherwise set to 0; Set to 0 after self-test is completed >**/
#define ICM40627_BIT_EN_AZ_ST 5 /**< Enable Z-accel self-test >**/
#define ICM40627_BIT_EN_AY_ST 4 /**< Enable Y-accel self-test >**/
#define ICM40627_BIT_EN_AX_ST 3 /**< Enable X-accel self-test >**/
#define ICM40627_BIT_EN_GZ_ST 2 /**< Enable Z-gyro self-test >**/
#define ICM40627_BIT_EN_GY_ST 1 /**< Enable Y-gyro self-test >**/
#define ICM40627_BIT_EN_GX_ST 0 /**< Enable X-gyro self-test >**/

#define ICM40627_REG_WHO_AM_I (ICM40627_BANK_0 | 0x75) /**< WHO_AM_I >**/
#define ICM40627_SHIFT_WHOAMI 0 /**< Register to indicate to user which device is being accessed >**/

#define ICM40627_REG_REG_BANK_SEL (0x76) /**< REG_BANK_SEL >**/

#define ICM40627_SHIFT_BANK_SEL 0 /**< Register bank selection >**/
#define ICM40627_BANK_SEL_VALUE_0 (0 << ICM40627_SHIFT_BANK_SEL) /**< Bank 0 (default) >**/
#define ICM40627_BANK_SEL_VALUE_1 (1 << ICM40627_SHIFT_BANK_SEL) /**< Bank 1 >**/
#define ICM40627_BANK_SEL_VALUE_10 (2 << ICM40627_SHIFT_BANK_SEL) /**< Bank 2 >**/
#define ICM40627_BANK_SEL_VALUE_11 (3 << ICM40627_SHIFT_BANK_SEL) /**< Bank 3 >**/
#define ICM40627_BANK_SEL_VALUE_100 (4 << ICM40627_SHIFT_BANK_SEL) /**< Bank 4 >**/
#define ICM40627_BANK_SEL_VALUE_101 (5 << ICM40627_SHIFT_BANK_SEL) /**< Reserved >**/
#define ICM40627_BANK_SEL_VALUE_110 (6 << ICM40627_SHIFT_BANK_SEL) /**< Reserved >**/
#define ICM40627_BANK_SEL_VALUE_111 (7 << ICM40627_SHIFT_BANK_SEL) /**< Reserved >**/

/***********************/
/* Bank 1 register map */
/***********************/
#define ICM40627_REG_SENSOR_CONFIG0 (ICM40627_BANK_1 | 0x03) /**< SENSOR_CONFIG0 >**/
#define ICM40627_BIT_ZG_DISABLE 5
#define ICM40627_BIT_YG_DISABLE 4
#define ICM40627_BIT_XG_DISABLE 3
#define ICM40627_BIT_ZA_DISABLE 2
#define ICM40627_BIT_YA_DISABLE 1
#define ICM40627_BIT_XA_DISABLE 0

#define ICM40627_REG_GYRO_CONFIG_STATIC2 (ICM40627_BANK_1 | 0x0B) /**< GYRO_CONFIG_STATIC2 >**/
#define ICM40627_BIT_GYRO_AAF_DIS 1
#define ICM40627_BIT_GYRO_NF_DIS 0

#define ICM40627_REG_GYRO_CONFIG_STATIC3 (ICM40627_BANK_1 | 0x0C) /**< GYRO_CONFIG_STATIC3 >**/
#define ICM40627_SHIFT_GYRO_AAF_DELT 0 /**< Controls bandwidth of the gyroscope anti-alias filterSee section 5.2for details >**/

#define ICM40627_REG_GYRO_CONFIG_STATIC4 (ICM40627_BANK_1 | 0x0D) /**< GYRO_CONFIG_STATIC4 >**/
#define ICM40627_SHIFT_GYRO_AAF_DELTSQR_LOW 0 /**< Controls bandwidth of the gyroscope anti-alias filterSee section 5.2 for details >**/

#define ICM40627_REG_GYRO_CONFIG_STATIC5 (ICM40627_BANK_1 | 0x0E) /**< GYRO_CONFIG_STATIC5 >**/
#define ICM40627_SHIFT_GYRO_AAF_BITSHIFT 4 /**< Controls bandwidth of the gyroscope anti-alias filterSee section 5.2 for details >**/
#define ICM40627_SHIFT_GYRO_AAF_DELTSQR_HIGH 0 /**< Controls bandwidth of the gyroscope anti-alias filterSee section 5.2 for details >**/

#define ICM40627_REG_GYRO_CONFIG_STATIC6 (ICM40627_BANK_1 | 0x0F) /**< GYRO_CONFIG_STATIC6 >**/
#define ICM40627_SHIFT_GYRO_X_NF_COSWZ_LOW 0 /**< Used for gyroscope X-axis notch filter frequency selectionSee section 5.1 for details >**/

#define ICM40627_REG_GYRO_CONFIG_STATIC7 (ICM40627_BANK_1 | 0x10) /**< GYRO_CONFIG_STATIC7 >**/
#define ICM40627_SHIFT_GYRO_Y_NF_COSWZ_LOW 0 /**< Used for gyroscope Y-axis notch filter frequency selectionSee section 5.1 for details >**/

#define ICM40627_REG_GYRO_CONFIG_STATIC8 (ICM40627_BANK_1 | 0x11) /**< GYRO_CONFIG_STATIC8 >**/
#define ICM40627_SHIFT_GYRO_Z_NF_COSWZ_LOW 0 /**< Used for gyroscope Z-axis notch filter frequency selectionSee section 5.1 for details >**/

#define ICM40627_REG_GYRO_CONFIG_STATIC9 (ICM40627_BANK_1 | 0x12) /**< GYRO_CONFIG_STATIC9 >**/
#define ICM40627_BIT_GYRO_Z_NF_COSWZ_SEL_LOW 5 /**< Used for gyroscope Z-axis notch filter frequency selectionSee section 5.1 for details >**/
#define ICM40627_BIT_GYRO_Y_NF_COSWZ_SEL_LOW 4 /**< Used for gyroscope Y-axis notch filter frequency selectionSee section 5.1 for details >**/
#define ICM40627_BIT_GYRO_X_NF_COSWZ_SEL_LOW 3 /**< Used for gyroscope X-axis notch filter frequency selectionSee section 5.1 for details >**/
#define ICM40627_BIT_GYRO_Z_NF_COSWZ_HIGH 2 /**< Used for gyroscope Z-axis notch filter frequency selectionSee section 5.1 for details >**/
#define ICM40627_BIT_GYRO_Y_NF_COSWZ_HIGH 1 /**< Used for gyroscope Y-axis notch filter frequency selectionSee section 5.1 for details >**/
#define ICM40627_BIT_GYRO_X_NF_COSWZ_HIGH 0 /**< Used for gyroscope X-axis notch filter frequency selectionSee section 5.1 for details >**/

#define ICM40627_REG_GYRO_CONFIG_STATIC10 (ICM40627_BANK_1 | 0x13) /**< GYRO_CONFIG_STATIC10 >**/
#define ICM40627_SHIFT_GYRO_NF_BW_SEL 4 /**< Selects bandwidth for gyroscope notch filterSee section 5.1 for details >**/
#define ICM40627_SHIFT_GYRO_HPF_BW_IND 1 /**< Selects HPF 3 dB cutoff frequency bandwidthSee section 5.6 for detailsSelects HPF filter order (see section 5.6 for details) >**/
#define ICM40627_BIT_GYRO_HPF_ORD_IND 0

#define ICM40627_REG_XG_ST_DATA (ICM40627_BANK_1 | 0x5F) /**< XG_ST_DATA >**/
#define ICM40627_SHIFT_XG_ST_DATA 0 /**< X-gyro self-test data >**/

#define ICM40627_REG_YG_ST_DATA (ICM40627_BANK_1 | 0x60) /**< YG_ST_DATA >**/
#define ICM40627_SHIFT_YG_ST_DATA 0 /**< Y-gyro self-test data >**/

#define ICM40627_REG_ZG_ST_DATA (ICM40627_BANK_1 | 0x61) /**< ZG_ST_DATA >**/
#define ICM40627_SHIFT_ZG_ST_DATA 0 /**< Z-gyro self-test data >**/

#define ICM40627_REG_TMSTVAL0 (ICM40627_BANK_1 | 0x62) /**< TMSTVAL0 >**/
#define ICM40627_SHIFT_TMST_VALUE_LOW 0 /**< When TMST_STROBE is programmed, the current value of the internalcounter is latched to this register. Allows the full 20-bit precision of the timestamp to be read back. >**/

#define ICM40627_REG_TMSTVAL1 (ICM40627_BANK_1 | 0x63) /**< TMSTVAL1 >**/
#define ICM40627_SHIFT_TMST_VALUE_HIGH 0 /**< When TMST_STROBE is programmed, the current value of the internalcounter is latched to this register. Allows the full 20-bit precision of the timestamp to be read back. >**/

#define ICM40627_REG_TMSTVAL2 (ICM40627_BANK_1 | 0x64) /**< TMSTVAL2 >**/
#define ICM40627_SHIFT_TMST_VALUE 0 /**< When TMST_STROBE is programmed, the current value of the internalcounter is latched to this register. Allows the full 20-bit precision of the timestamp to be read back. >**/

#define ICM40627_REG_INTF_CONFIG4 (ICM40627_BANK_1 | 0x7A) /**< INTF_CONFIG4 >**/
#define ICM40627_BIT_SPI_AP_4WIRE 1

#define ICM40627_REG_INTF_CONFIG5 (ICM40627_BANK_1 | 0x7B) /**< INTF_CONFIG5 >**/
#define ICM40627_SHIFT_PIN9_FUNCTION 1 /**< Selects among the following functionalities for pin 9 >**/
#define ICM40627_PIN9_FUNCTION_VALUE_0 (0 << ICM40627_SHIFT_PIN9_FUNCTION) /**< INT2 >**/
#define ICM40627_PIN9_FUNCTION_VALUE_1 (1 << ICM40627_SHIFT_PIN9_FUNCTION) /**< FSYNC >**/
#define ICM40627_PIN9_FUNCTION_VALUE_10 (2 << ICM40627_SHIFT_PIN9_FUNCTION) /**< Reserved >**/
#define ICM40627_PIN9_FUNCTION_VALUE_11 (3 << ICM40627_SHIFT_PIN9_FUNCTION) /**< Reserved >**/

/***********************/
/* Bank 2 register map */
/***********************/
#define ICM40627_REG_ACCEL_CONFIG_STATIC2 (ICM40627_BANK_2 | 0x03) /**< ACCEL_CONFIG_STATIC2 >**/
#define ICM40627_SHIFT_ACCEL_AAF_DELT 1 /**< Controls bandwidth of the accelerometer anti-alias filterSee section 5.2 for details >**/
#define ICM40627_BIT_ACCEL_AAF_DIS 0

#define ICM40627_REG_ACCEL_CONFIG_STATIC3 (ICM40627_BANK_2 | 0x04) /**< ACCEL_CONFIG_STATIC3 >**/
#define ICM40627_SHIFT_ACCEL_AAF_DELTSQR_LOW 0 /**< Controls bandwidth of the accelerometer anti-alias filterSee section 5.2for details >**/

#define ICM40627_REG_ACCEL_CONFIG_STATIC4 (ICM40627_BANK_2 | 0x05) /**< ACCEL_CONFIG_STATIC4 >**/
#define ICM40627_SHIFT_ACCEL_AAF_BITSHIFT 4 /**< Controls bandwidth of the accelerometer anti-alias filterSee section 5.2 for details >**/
#define ICM40627_SHIFT_ACCEL_AAF_DELTSQR_HIGH 0 /**< Controls bandwidth of the accelerometer anti-alias filterSee section 5.2 for details >**/

#define ICM40627_REG_XA_ST_DATA (ICM40627_BANK_2 | 0x3B) /**< XA_ST_DATA >**/
#define ICM40627_SHIFT_XA_ST_DATA 0 /**< X-accel self-test data >**/

#define ICM40627_REG_YA_ST_DATA (ICM40627_BANK_2 | 0x3C) /**< YA_ST_DATA >**/
#define ICM40627_SHIFT_YA_ST_DATA 0 /**< Y-accel self-test data >**/

#define ICM40627_REG_ZA_ST_DATA (ICM40627_BANK_2 | 0x3D) /**< ZA_ST_DATA >**/
#define ICM40627_SHIFT_ZA_ST_DATA 0 /**< Z-accel self-test data >**/

/***********************/
/* Bank 4 register map */
/***********************/
#define ICM40627_REG_GYRO_ON_OFF_CONFIG (ICM40627_BANK_4 | 0x0E) /**< GYRO_ON_OFF_CONFIG >**/
#define ICM40627_BIT_GYRO_ON_OFF_CONFIG 6 /**< Set value to 0 when turning off gyroscope.Set value to 1 when turning on gyroscope.See Section 12 for details. >**/

#define ICM40627_REG_APEX_CONFIG1 (ICM40627_BANK_4 | 0x40)  /**< APEX_CONFIG1 >**/
#define ICM40627_SHIFT_LOW_ENERGY_AMP_TH_SEL 4 /**< Pedometer Low Energy mode amplitude threshold selectionUse default value 1010bWhen the DMP is in power save mode, it is awakened by the WOM and willwait for a certain duration before going back to sleep. This bitfieldconfigures this duration. >**/
#define ICM40627_LOW_ENERGY_AMP_TH_SEL_VALUE_0 (0 << ICM40627_SHIFT_LOW_ENERGY_AMP_TH_SEL) /**< 0s >**/
#define ICM40627_LOW_ENERGY_AMP_TH_SEL_VALUE_1 (1 << ICM40627_SHIFT_LOW_ENERGY_AMP_TH_SEL) /**< 4s >**/
#define ICM40627_LOW_ENERGY_AMP_TH_SEL_VALUE_10 (2 << ICM40627_SHIFT_LOW_ENERGY_AMP_TH_SEL) /**< 8s >**/
#define ICM40627_LOW_ENERGY_AMP_TH_SEL_VALUE_11 (3 << ICM40627_SHIFT_LOW_ENERGY_AMP_TH_SEL) /**< 12s >**/
#define ICM40627_LOW_ENERGY_AMP_TH_SEL_VALUE_100 (4 << ICM40627_SHIFT_LOW_ENERGY_AMP_TH_SEL) /**< 16s >**/
#define ICM40627_LOW_ENERGY_AMP_TH_SEL_VALUE_101 (5 << ICM40627_SHIFT_LOW_ENERGY_AMP_TH_SEL) /**< 20s >**/
#define ICM40627_SHIFT_ 0
#define ICM40627__VALUE_0 (0 << ICM40627_SHIFT_) /**< 24s >**/
#define ICM40627__VALUE_1 (1 << ICM40627_SHIFT_) /**< 28s >**/
#define ICM40627__VALUE_10 (2 << ICM40627_SHIFT_) /**< 32s >**/
#define ICM40627__VALUE_11 (3 << ICM40627_SHIFT_) /**< 36s >**/
#define ICM40627__VALUE_100 (4 << ICM40627_SHIFT_) /**< 40s >**/
#define ICM40627__VALUE_101 (5 << ICM40627_SHIFT_) /**< 44s >**/
#define ICM40627__VALUE_110 (6 << ICM40627_SHIFT_) /**< 48s >**/
#define ICM40627__VALUE_111 (7 << ICM40627_SHIFT_) /**< 52s >**/
#define ICM40627__VALUE_1000 (8 << ICM40627_SHIFT_) /**< 56s >**/
#define ICM40627__VALUE_1001 (9 << ICM40627_SHIFT_) /**< 60s >**/

#define ICM40627_REG_APEX_CONFIG2 (ICM40627_BANK_4 | 0x41) /**< APEX_CONFIG2 >**/
#define ICM40627_SHIFT_PED_AMP_TH_SEL 4 /**< Pedometer amplitude threshold selectionUse default value 1000bPedometer step count detection windowUse default value 0101b >**/
#define ICM40627_PED_AMP_TH_SEL_VALUE_0 (0 << ICM40627_SHIFT_PED_AMP_TH_SEL) /**< 0 steps >**/
#define ICM40627_PED_AMP_TH_SEL_VALUE_1 (1 << ICM40627_SHIFT_PED_AMP_TH_SEL) /**< 1 step >**/
#define ICM40627_PED_AMP_TH_SEL_VALUE_10 (2 << ICM40627_SHIFT_PED_AMP_TH_SEL) /**< 2 steps >**/
#define ICM40627_PED_AMP_TH_SEL_VALUE_11 (3 << ICM40627_SHIFT_PED_AMP_TH_SEL) /**< 3 steps >**/
#define ICM40627_PED_AMP_TH_SEL_VALUE_100 (4 << ICM40627_SHIFT_PED_AMP_TH_SEL) /**< 4 steps >**/
#define ICM40627_PED_AMP_TH_SEL_VALUE_101 (5 << ICM40627_SHIFT_PED_AMP_TH_SEL) /**< 5 steps (default) >**/
#define ICM40627_SHIFT_PED_STEP_CNT_TH_SEL 0
#define ICM40627_PED_STEP_CNT_TH_SEL_VALUE_0 (0 << ICM40627_SHIFT_PED_STEP_CNT_TH_SEL) /**< 6 steps >**/
#define ICM40627_PED_STEP_CNT_TH_SEL_VALUE_1 (1 << ICM40627_SHIFT_PED_STEP_CNT_TH_SEL) /**< 7 steps >**/
#define ICM40627_PED_STEP_CNT_TH_SEL_VALUE_10 (2 << ICM40627_SHIFT_PED_STEP_CNT_TH_SEL) /**< 8 steps >**/
#define ICM40627_PED_STEP_CNT_TH_SEL_VALUE_11 (3 << ICM40627_SHIFT_PED_STEP_CNT_TH_SEL) /**< 9 steps >**/
#define ICM40627_PED_STEP_CNT_TH_SEL_VALUE_100 (4 << ICM40627_SHIFT_PED_STEP_CNT_TH_SEL) /**< 10 steps >**/
#define ICM40627_PED_STEP_CNT_TH_SEL_VALUE_101 (5 << ICM40627_SHIFT_PED_STEP_CNT_TH_SEL) /**< 11 steps >**/
#define ICM40627_PED_STEP_CNT_TH_SEL_VALUE_110 (6 << ICM40627_SHIFT_PED_STEP_CNT_TH_SEL) /**< 12 steps >**/
#define ICM40627_PED_STEP_CNT_TH_SEL_VALUE_111 (7 << ICM40627_SHIFT_PED_STEP_CNT_TH_SEL) /**< 13 steps >**/
#define ICM40627_PED_STEP_CNT_TH_SEL_VALUE_1000 (8 << ICM40627_SHIFT_PED_STEP_CNT_TH_SEL) /**< 14 steps >**/
#define ICM40627_PED_STEP_CNT_TH_SEL_VALUE_1001 (9 << ICM40627_SHIFT_PED_STEP_CNT_TH_SEL) /**< 15 steps >**/

#define ICM40627_REG_APEX_CONFIG3 (ICM40627_BANK_4 | 0x42)  /**< APEX_CONFIG3 >**/
#define ICM40627_SHIFT_PED_STEP_DET_TH_SEL 5 /**< Pedometer step detection threshold selectionUse default value 010b >**/
#define ICM40627_PED_STEP_DET_TH_SEL_VALUE_0 (0 << ICM40627_SHIFT_PED_STEP_DET_TH_SEL) /**< 0 steps >**/
#define ICM40627_PED_STEP_DET_TH_SEL_VALUE_1 (1 << ICM40627_SHIFT_PED_STEP_DET_TH_SEL) /**< 1 step >**/
#define ICM40627_PED_STEP_DET_TH_SEL_VALUE_10 (2 << ICM40627_SHIFT_PED_STEP_DET_TH_SEL) /**< 2 steps (default) >**/
#define ICM40627_PED_STEP_DET_TH_SEL_VALUE_11 (3 << ICM40627_SHIFT_PED_STEP_DET_TH_SEL) /**< 3 steps >**/
#define ICM40627_PED_STEP_DET_TH_SEL_VALUE_100 (4 << ICM40627_SHIFT_PED_STEP_DET_TH_SEL) /**< 4 steps >**/
#define ICM40627_PED_STEP_DET_TH_SEL_VALUE_101 (5 << ICM40627_SHIFT_PED_STEP_DET_TH_SEL) /**< 5 steps >**/
#define ICM40627_PED_STEP_DET_TH_SEL_VALUE_110 (6 << ICM40627_SHIFT_PED_STEP_DET_TH_SEL) /**< 6 steps >**/
#define ICM40627_PED_STEP_DET_TH_SEL_VALUE_111 (7 << ICM40627_SHIFT_PED_STEP_DET_TH_SEL) /**< 7 steps >**/
#define ICM40627_SHIFT_PED_SB_TIMER_TH_SEL 2 /**< Pedometer step buffer timer threshold selectionUse default value 100b >**/
#define ICM40627_PED_SB_TIMER_TH_SEL_VALUE_0 (0 << ICM40627_SHIFT_PED_SB_TIMER_TH_SEL) /**< 0 samples >**/
#define ICM40627_PED_SB_TIMER_TH_SEL_VALUE_1 (1 << ICM40627_SHIFT_PED_SB_TIMER_TH_SEL) /**< 1 sample >**/
#define ICM40627_PED_SB_TIMER_TH_SEL_VALUE_10 (2 << ICM40627_SHIFT_PED_SB_TIMER_TH_SEL) /**< 2 samples >**/
#define ICM40627_PED_SB_TIMER_TH_SEL_VALUE_11 (3 << ICM40627_SHIFT_PED_SB_TIMER_TH_SEL) /**< 3 samples >**/
#define ICM40627_PED_SB_TIMER_TH_SEL_VALUE_100 (4 << ICM40627_SHIFT_PED_SB_TIMER_TH_SEL) /**< 4 samples (default) >**/
#define ICM40627_PED_SB_TIMER_TH_SEL_VALUE_101 (5 << ICM40627_SHIFT_PED_SB_TIMER_TH_SEL) /**< 5 samples >**/
#define ICM40627_PED_SB_TIMER_TH_SEL_VALUE_110 (6 << ICM40627_SHIFT_PED_SB_TIMER_TH_SEL) /**< 6 samples >**/
#define ICM40627_PED_SB_TIMER_TH_SEL_VALUE_111 (7 << ICM40627_SHIFT_PED_SB_TIMER_TH_SEL) /**< 7 samples >**/
#define ICM40627_SHIFT_PED_HI_EN_TH_SEL 0 /**< Pedometer high energy threshold selectionUse default value 01b >**/

#define ICM40627_REG_APEX_CONFIG4 (ICM40627_BANK_4 | 0x43)  /**< APEX_CONFIG4 >**/
#define ICM40627_SHIFT_TILT_WAIT_TIME_SEL 6 /**< Configures duration of delay after tilt is detected before interrupt istriggered >**/
#define ICM40627_TILT_WAIT_TIME_SEL_VALUE_0 (0 << ICM40627_SHIFT_TILT_WAIT_TIME_SEL) /**< 0s >**/
#define ICM40627_TILT_WAIT_TIME_SEL_VALUE_1 (1 << ICM40627_SHIFT_TILT_WAIT_TIME_SEL) /**< 2s >**/
#define ICM40627_TILT_WAIT_TIME_SEL_VALUE_10 (2 << ICM40627_SHIFT_TILT_WAIT_TIME_SEL) /**< 4s (default) >**/
#define ICM40627_TILT_WAIT_TIME_SEL_VALUE_11 (3 << ICM40627_SHIFT_TILT_WAIT_TIME_SEL) /**< 6s >**/
#define ICM40627_SHIFT_SLEEP_TIME_OUT 3 /**< Configures the time out for sleep detection, for Raise to Wake/Sleepfeature >**/
#define ICM40627_SLEEP_TIME_OUT_VALUE_0 (0 << ICM40627_SHIFT_SLEEP_TIME_OUT) /**< 1.28s >**/
#define ICM40627_SLEEP_TIME_OUT_VALUE_1 (1 << ICM40627_SHIFT_SLEEP_TIME_OUT) /**< 2.56s >**/
#define ICM40627_SLEEP_TIME_OUT_VALUE_10 (2 << ICM40627_SHIFT_SLEEP_TIME_OUT) /**< 3.84s >**/
#define ICM40627_SLEEP_TIME_OUT_VALUE_11 (3 << ICM40627_SHIFT_SLEEP_TIME_OUT) /**< 5.12s >**/
#define ICM40627_SLEEP_TIME_OUT_VALUE_100 (4 << ICM40627_SHIFT_SLEEP_TIME_OUT) /**< 6.40s >**/
#define ICM40627_SLEEP_TIME_OUT_VALUE_101 (5 << ICM40627_SHIFT_SLEEP_TIME_OUT) /**< 7.68s >**/
#define ICM40627_SLEEP_TIME_OUT_VALUE_110 (6 << ICM40627_SHIFT_SLEEP_TIME_OUT) /**< 8.96s >**/
#define ICM40627_SLEEP_TIME_OUT_VALUE_111 (7 << ICM40627_SHIFT_SLEEP_TIME_OUT) /**< 10.24s >**/

#define ICM40627_REG_APEX_CONFIG5 (ICM40627_BANK_4 | 0x44) /**< APEX_CONFIG5 >**/
#define ICM40627_SHIFT_MOUNTING_MATRIX 0 /**< Defines mounting matrix, chip to device frame >**/
#define ICM40627_MOUNTING_MATRIX_VALUE_0 (0 << ICM40627_SHIFT_MOUNTING_MATRIX) /**< [ 1  0  0;  0  1  0;  0  0  1] >**/
#define ICM40627_MOUNTING_MATRIX_VALUE_1 (1 << ICM40627_SHIFT_MOUNTING_MATRIX) /**< [ 1  0  0;  0 -1  0;  0  0 -1] >**/
#define ICM40627_MOUNTING_MATRIX_VALUE_10 (2 << ICM40627_SHIFT_MOUNTING_MATRIX) /**< [-1  0  0;  0  1  0;  0  0 -1] >**/
#define ICM40627_MOUNTING_MATRIX_VALUE_11 (3 << ICM40627_SHIFT_MOUNTING_MATRIX) /**< [-1  0  0;  0 -1  0;  0  0  1] >**/
#define ICM40627_MOUNTING_MATRIX_VALUE_100 (4 << ICM40627_SHIFT_MOUNTING_MATRIX) /**< [ 0  1  0;  1  0  0;  0  0 -1] >**/
#define ICM40627_MOUNTING_MATRIX_VALUE_101 (5 << ICM40627_SHIFT_MOUNTING_MATRIX) /**< [ 0  1  0;  -1  0  0;  0  0  1] >**/
#define ICM40627_MOUNTING_MATRIX_VALUE_110 (6 << ICM40627_SHIFT_MOUNTING_MATRIX) /**< [ 0 -1  0;  1  0  0;  0  0  1] >**/
#define ICM40627_MOUNTING_MATRIX_VALUE_111 (7 << ICM40627_SHIFT_MOUNTING_MATRIX) /**< [ 0 -1  0; -1  0  0;  0  0 -1] >**/

#define ICM40627_REG_APEX_CONFIG6 (ICM40627_BANK_4 | 0x45)  /**< APEX_CONFIG6 >**/
#define ICM40627_SHIFT_SLEEP_GESTURE_DELAY 0 /**< Configures detection window for sleep gesture detection >**/
#define ICM40627_SLEEP_GESTURE_DELAY_VALUE_0 (0 << ICM40627_SHIFT_SLEEP_GESTURE_DELAY) /**< 0.32s >**/
#define ICM40627_SLEEP_GESTURE_DELAY_VALUE_1 (1 << ICM40627_SHIFT_SLEEP_GESTURE_DELAY) /**< 0.64s >**/
#define ICM40627_SLEEP_GESTURE_DELAY_VALUE_10 (2 << ICM40627_SHIFT_SLEEP_GESTURE_DELAY) /**< 0.96s >**/
#define ICM40627_SLEEP_GESTURE_DELAY_VALUE_11 (3 << ICM40627_SHIFT_SLEEP_GESTURE_DELAY) /**< 1.28s >**/
#define ICM40627_SLEEP_GESTURE_DELAY_VALUE_100 (4 << ICM40627_SHIFT_SLEEP_GESTURE_DELAY) /**< 1.60s >**/
#define ICM40627_SLEEP_GESTURE_DELAY_VALUE_101 (5 << ICM40627_SHIFT_SLEEP_GESTURE_DELAY) /**< 1.92s >**/
#define ICM40627_SLEEP_GESTURE_DELAY_VALUE_110 (6 << ICM40627_SHIFT_SLEEP_GESTURE_DELAY) /**< 2.24s >**/
#define ICM40627_SLEEP_GESTURE_DELAY_VALUE_111 (7 << ICM40627_SHIFT_SLEEP_GESTURE_DELAY) /**< 2.56s >**/

#define ICM40627_REG_APEX_CONFIG7 (ICM40627_BANK_4 | 0x46)  /**< APEX_CONFIG7 >**/
#define ICM40627_SHIFT_TAP_MIN_JERK_THR 2 /**< Tap Detection minimum jerk thresholdUse default value 010001b >**/
#define ICM40627_SHIFT_TAP_MAX_PEAK_TOL 0 /**< Tap Detection maximum peak toleranceUse default value 01b >**/

#define ICM40627_REG_APEX_CONFIG8 (ICM40627_BANK_4 | 0x47) /**< APEX_CONFIG8 >**/
#define ICM40627_SHIFT_TAP_TMAX 5 /**< Tap measurement window (number of samples)Use default value 01b >**/
#define ICM40627_SHIFT_TAP_TAVG 3 /**< Tap energy measurement window (number of samples)Use default value 01b >**/
#define ICM40627_SHIFT_TAP_TMIN 0 /**< Single tap window (number of samples)Use default value 011b >**/

#define ICM40627_REG_APEX_CONFIG9 (ICM40627_BANK_4 | 0x48) /**< APEX_CONFIG9 >**/
#define ICM40627_BIT_SENSITIVITY_MODE 0 /**< at accelerometer ODR ≥ 50 Hz >**/

#define ICM40627_REG_ACCEL_WOM_X_THR (ICM40627_BANK_4 | 0x4A) /**< ACCEL_WOM_X_THR >**/
#define ICM40627_SHIFT_WOM_X_TH 0 /**< Threshold value for the Wake on Motion Interrupt for X-axis accelerometerWoM thresholds are expressed in fixed “mg” independent of the selectedRange [0g : 1g]; Resolution 1g/256=~3.9 mg >**/

#define ICM40627_REG_ACCEL_WOM_Y_THR (ICM40627_BANK_4 | 0x4B) /**< ACCEL_WOM_Y_THR >**/
#define ICM40627_SHIFT_WOM_Y_TH 0 /**< Threshold value for the Wake on Motion Interrupt for Y-axis accelerometerWoM thresholds are expressed in fixed “mg” independent of the selectedRange [0g : 1g]; Resolution 1g/256=~3.9 mg >**/

#define ICM40627_REG_ACCEL_WOM_Z_THR (ICM40627_BANK_4 | 0x4C) /**< ACCEL_WOM_Z_THR >**/
#define ICM40627_SHIFT_WOM_Z_TH 0 /**< Threshold value for the Wake on Motion Interrupt for Z-axis accelerometerWoM thresholds are expressed in fixed “mg” independent of the selectedRange [0g : 1g]; Resolution 1g/256=~3.9 mg >**/

#define ICM40627_REG_INT_SOURCE6 (ICM40627_BANK_4 | 0x4D) /**< INT_SOURCE6 >**/
#define ICM40627_BIT_STEP_DET_INT1_EN 5
#define ICM40627_BIT_STEP_CNT_OFL_INT1_EN 4
#define ICM40627_BIT_TILT_DET_INT1_EN 3
#define ICM40627_BIT_TAP_DET_INT1_EN 0

#define ICM40627_REG_INT_SOURCE7 (ICM40627_BANK_4 | 0x4E) /**< INT_SOURCE7 >**/
#define ICM40627_BIT_STEP_DET_INT2_EN 5
#define ICM40627_BIT_STEP_CNT_OFL_INT2_EN 4
#define ICM40627_BIT_TILT_DET_INT2_EN 3
#define ICM40627_BIT_TAP_DET_INT2_EN 0

#define ICM40627_REG_OFFSET_USER0 (ICM40627_BANK_4 | 0x77) /**< OFFSET_USER0 >**/
#define ICM40627_SHIFT_GYRO_X_OFFUSER_LOW 0 /**< Lower bits of X-gyro offset programmed by user. Max value is ±64 dps,resolution is 1/32 dps. >**/

#define ICM40627_REG_OFFSET_USER1 (ICM40627_BANK_4 | 0x78) /**< OFFSET_USER1 >**/
#define ICM40627_SHIFT_GYRO_Y_OFFUSER_HIGH 4 /**< Upper bits of Y-gyro offset programmed by user. Max value is ±64 dps,resolution is 1/32 dps. >**/
#define ICM40627_SHIFT_GYRO_X_OFFUSER_HIGH 0 /**< Upper bits of X-gyro offset programmed by user. Max value is ±64 dps,resolution is 1/32 dps. >**/

#define ICM40627_REG_OFFSET_USER2 (ICM40627_BANK_4 | 0x79) /**< OFFSET_USER2 >**/
#define ICM40627_SHIFT_GYRO_Y_OFFUSER_LOW 0 /**< Lower bits of Y-gyro offset programmed by user. Max value is ±64 dps,resolution is 1/32 dps. >**/

#define ICM40627_REG_OFFSET_USER3 (ICM40627_BANK_4 | 0x7A) /**< OFFSET_USER3 >**/
#define ICM40627_SHIFT_GYRO_Z_OFFUSER_LOW 0 /**< Lower bits of Z-gyro offset programmed by user. Max value is ±64 dps,resolution is 1/32 dps. >**/

#define ICM40627_REG_OFFSET_USER4 (ICM40627_BANK_4 | 0x7B) /**< OFFSET_USER4 >**/
#define ICM40627_SHIFT_ACCEL_X_OFFUSER_HIGH 4 /**< Upper bits of X-accel offset programmed by user. Max value is ±1g,resolution is 0.5 mg. >**/
#define ICM40627_SHIFT_GYRO_Z_OFFUSER_HIGH 0 /**< Upper bits of Z-gyro offset programmed by user. Max value is ±64 dps,resolution is 1/32 dps. >**/

#define ICM40627_REG_OFFSET_USER5 (ICM40627_BANK_4 | 0x7C)  /**< OFFSET_USER5 >**/
#define ICM40627_SHIFT_ACCEL_X_OFFUSER_LOW 0 /**< Lower bits of X-accel offset programmed by user. Max value is ±1g,resolution is 0.5 mg. >**/

#define ICM40627_REG_OFFSET_USER6 (ICM40627_BANK_4 | 0x7D) /**< OFFSET_USER6 >**/
#define ICM40627_SHIFT_ACCEL_Y_OFFUSER_LOW 0 /**< Lower bits of Y-accel offset programmed by user. Max value is ±1g,resolution is 0.5 mg. >**/

#define ICM40627_REG_OFFSET_USER7 (ICM40627_BANK_4 | 0x7E) /**< OFFSET_USER7 >**/
#define ICM40627_SHIFT_ACCEL_Z_OFFUSER_HIGH 4 /**< Upper bits of Z-accel offset programmed by user. Max value is ±1g,resolution is 0.5 mg. >**/
#define ICM40627_SHIFT_ACCEL_Y_OFFUSER_HIGH 0 /**< Upper bits of Y-accel offset programmed by user. Max value is ±1g,resolution is 0.5 mg. >**/

#define ICM40627_REG_OFFSET_USER8 (ICM40627_BANK_4 | 0x7F) /**< OFFSET_USER8 >**/
#define ICM40627_SHIFT_ACCEL_Z_OFFUSER_LOW 0 /**< Lower bits of Z-accel offset programmed by user. Max value is ±1g,resolution is 0.5 mg. >**/

#ifdef __cplusplus
}
#endif

/** @endcond */
#endif // SL_ICM40627_DEFS_H
