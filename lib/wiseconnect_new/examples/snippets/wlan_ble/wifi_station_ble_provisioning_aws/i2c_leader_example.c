/***************************************************************************/ /**
 * @file i2c_leader_example.c
 * @brief I2C Leader example using i2c_instance component for LM75 temperature sensor
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "i2c_leader_example.h"
#include "rsi_debug.h"
#include "sl_i2c_instances.h"
#include "sl_si91x_i2c.h"
#include "sl_si91x_power_manager.h"
#include "sl_status.h"
#include "sl_component_catalog.h"
#include "cmsis_os2.h"

/*******************************************************************************
  ***************************  Defines / Macros  ********************************
  ******************************************************************************/
#define LM75_POINTER_REG_SIZE          1    // Size of LM75 pointer register (1 byte)
#define LM75_DATA_SIZE                 2    // Size of LM75 temperature data (2 bytes)
#define FOLLOWER_I2C_ADDR              0x48 // LM75 Temperature sensor I2C address
#define I2C_TX_FIFO_THRESHOLD          0    // FIFO threshold for TX
#define I2C_RX_FIFO_THRESHOLD          0    // FIFO threshold for RX
#define MS_DELAY_COUNTER               4600 // Delay count for 1ms
#define I2C_INTER_TRANSACTION_DELAY_MS 10   // Delay between I2C send and receive operations

/*******************************************************************************
   ******************************  Data Types  ***********************************
   ******************************************************************************/
// Enum for different transmission scenarios
typedef enum {
  SL_I2C_SEND_DATA,    // Send mode
  SL_I2C_RECEIVE_DATA, // Receive mode
} i2c_action_enum_t;

/*******************************************************************************
   *************************** LOCAL VARIABLES   *******************************
   ******************************************************************************/
static sl_i2c_instance_t i2c_instance;
static uint8_t i2c_read_buffer[LM75_DATA_SIZE];
static uint8_t i2c_write_buffer[LM75_POINTER_REG_SIZE];
static i2c_action_enum_t current_mode  = SL_I2C_SEND_DATA;
static boolean_t i2c_send_data_flag    = false;
static boolean_t i2c_receive_data_flag = false;
static sl_i2c_config_t sl_i2c_config;
static boolean_t i2c_initialized = false;

// Encapsulated sensor data
static float sensor_data = 0.0f;

/*******************************************************************************
  **********************  Local Function prototypes   ***************************
  ******************************************************************************/
static sl_i2c_status_t i2c_leader_example_init(void);
static float lm75_convert_temperature(uint8_t *raw_data);
static void delay(uint32_t idelay);
static void handle_i2c_send_error(sl_i2c_status_t status);
static void handle_i2c_receive_error(sl_i2c_status_t status);
static void reset_i2c_state_machine(void);

/*******************************************************************************
  **************************   GLOBAL FUNCTIONS   *******************************
  ******************************************************************************/

/*******************************************************************************
  * Get current sensor temperature reading
  *
  * @param None
  * @return Current temperature in Celsius (0.0 if no valid reading)
  ******************************************************************************/
float get_sensor_temperature(void)
{
  return sensor_data;
}

/*******************************************************************************
  * Set sensor temperature value
  *
  * @param[in] temperature Temperature value in Celsius
  * @return None
  ******************************************************************************/
static void set_sensor_temperature(float temperature)
{
  sensor_data = temperature;
}

/*******************************************************************************
  * Initialize I2C with PS4 power requirement
  *
  * Adds PS4 requirement, waits for power stabilization, then performs
  * full I2C driver initialization. Call before each I2C transaction cycle.
  *
  * @param None
  * @return None
  ******************************************************************************/
void i2c_init(void)
{
  // Skip if already initialized (prevents double init and PS4 reference leak)
  if (i2c_initialized) {
    return;
  }

  // Add PS4 requirement to prevent M4 deep sleep during I2C operations
  sl_status_t status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\n Failed to add PS4 requirement, Error Code: 0x%lX\r\n", (unsigned long)status);
    return;
  }
  DEBUGOUT("\r\n Successfully added PS4 requirement\r\n");

  // Full I2C driver initialization from clean state
  sl_i2c_status_t i2c_status = i2c_leader_example_init();
  if (i2c_status != SL_I2C_SUCCESS) {
    DEBUGOUT("\r\n I2C init failed, removing PS4 requirement\r\n");
    status = sl_si91x_power_manager_remove_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n Failed to remove PS4 requirement, Error Code: 0x%lX\r\n", (unsigned long)status);
    }
    return;
  }
  i2c_initialized = true;

  // Initialize state machine for new transaction
  current_mode          = SL_I2C_SEND_DATA;
  i2c_send_data_flag    = true;
  i2c_receive_data_flag = false;
}

/*******************************************************************************
  * Deinitialize I2C and remove PS4 power requirement
  *
  * Properly shuts down I2C driver while PS4 is still active (clean shutdown),
  * then removes PS4 to allow M4 deep sleep between transactions.
  *
  * @param None
  * @return None
  ******************************************************************************/
void i2c_deinit(void)
{
  // Skip if already deinitialized (prevents double deinit and PS4 underflow)
  if (!i2c_initialized) {
    return;
  }

  // Deinit I2C driver while PS4 is still active (clean shutdown)
  sl_i2c_status_t i2c_status = sl_i2c_driver_deinit(i2c_instance);
  if (i2c_status != SL_I2C_SUCCESS) {
    DEBUGOUT("\r\n I2C driver deinit failed, Error Code: %u\r\n", i2c_status);
  }
  i2c_initialized = false;

  // Remove PS4 requirement after clean I2C shutdown
  sl_status_t status = sl_si91x_power_manager_remove_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\n Failed to remove PS4 requirement, Error Code: 0x%lX\r\n", (unsigned long)status);
    return;
  }
  DEBUGOUT("\r\n Successfully removed PS4 requirement\r\n");
}

/*******************************************************************************
  * Initialize I2C driver for LM75 sensor
  *
  * Configures I2C instance from UC, sets FIFO thresholds, enables repeated
  * start, and initializes buffers. Re-initializes GPIO pins and performs
  * hardware soft reset to recover from M4 deep sleep state corruption.
  *
  * @param None
  * @return SL_I2C_SUCCESS on success, error code on failure
  *
  * @note Called internally by i2c_init()
  ******************************************************************************/
static sl_i2c_status_t i2c_leader_example_init(void)
{
  sl_i2c_status_t i2c_status;

  // Select I2C instance based on UC configuration
#if defined(SL_CATALOG_I2C_I2C0_PRESENT)
  sl_i2c_config = sl_i2c_i2c0_config;
  i2c_instance  = SL_I2C0;
#elif defined(SL_CATALOG_I2C_I2C1_PRESENT)
  sl_i2c_config = sl_i2c_i2c1_config;
  i2c_instance  = SL_I2C1;
#elif defined(SL_CATALOG_I2C_I2C2_PRESENT)
  sl_i2c_config = sl_i2c_i2c2_config;
  i2c_instance  = SL_I2C2;
#else
  DEBUGOUT("\r\n No I2C instance configured in UC\r\n");
  return SL_I2C_INVALID_PARAMETER;
#endif

  // Re-initialize GPIO pin muxing for I2C SDA/SCL lines.
  // GPIO configurations are lost after M4 deep sleep, so pins must be
  // reconfigured on every init cycle (per Silicon Labs sleep-wakeup documentation).
  sl_i2c_init_instances();

  // Assert module-level hardware soft reset to clear any corrupted I2C FSM
  // state caused by ULP domain power transitions during M4 deep sleep.
  ULPCLK->ULP_TA_PERI_RESET_REG_b.I2C_SOFT_RESET_CNTRL_b = 0;
  delay(1);
  ULPCLK->ULP_TA_PERI_RESET_REG_b.I2C_SOFT_RESET_CNTRL_b = 1;

  // Initialize I2C using unified driver with UC-generated config
  i2c_status = sl_i2c_driver_init(i2c_instance, &sl_i2c_config);
  if (i2c_status != SL_I2C_SUCCESS) {
    DEBUGOUT("\r\n sl_i2c_driver_init Failed, Error Code: %u\r\n", i2c_status);
    return i2c_status;
  }

  // Configure RX and TX FIFO thresholds
  i2c_status = sl_i2c_driver_configure_fifo_threshold(i2c_instance, I2C_TX_FIFO_THRESHOLD, I2C_RX_FIFO_THRESHOLD);
  if (i2c_status != SL_I2C_SUCCESS) {
    DEBUGOUT("\r\n FIFO threshold config FAILED, Error Code: %u\r\n", i2c_status);
    return i2c_status;
  }

  // Enable repeated start for LM75 sensor communication (write pointer then read data)
  i2c_status = sl_i2c_driver_enable_repeated_start(i2c_instance, true);
  if (i2c_status != SL_I2C_SUCCESS) {
    DEBUGOUT("\r\n Repeated start enable FAILED, Error Code: %u\r\n", i2c_status);
    return i2c_status;
  }

  // Initialize buffers
  i2c_write_buffer[0] = 0x00; // LM75 temperature register pointer
  i2c_read_buffer[0]  = 0;
  i2c_read_buffer[1]  = 0;

  i2c_send_data_flag = true;
  current_mode       = SL_I2C_SEND_DATA;

  return SL_I2C_SUCCESS;
}

/*******************************************************************************
   * Blocking delay function
   *
   * @param[in] idelay Delay duration in milliseconds
   * @return None
   ******************************************************************************/
static void delay(uint32_t idelay)
{
  for (uint32_t x = 0; x < MS_DELAY_COUNTER * idelay; x++) {
    __NOP();
  }
}

/*******************************************************************************
  * Convert raw LM75 sensor data to temperature in Celsius
  *
  * @param[in] raw_data Pointer to 2-byte LM75 data [MSB, LSB]
  * @return Temperature in Celsius (0.0 if raw_data is NULL)
  * 
  *******************************************************************************/
static float lm75_convert_temperature(uint8_t *raw_data)
{
  float temperature = 0.0f;
  int16_t temp_raw;

  // Validate input
  if (raw_data == NULL) {
    return 0.0f; // Return safe default on NULL pointer
  }

  // LM75 temperature format:
  // Byte 0: MSB - D10 to D3 (integer part + sign)
  // Byte 1: LSB - D2 to D-4 (fractional part, only top 3 bits D2, D1, D0 are used)

  // Combine the two bytes using unsigned arithmetic to avoid undefined behavior
  temp_raw = (int16_t)(((uint16_t)raw_data[0] << 8) | raw_data[1]);

  // Shift to get 11-bit signed value (D10-D0)
  temp_raw >>= 5;

  // Check if negative (bit 10 is sign bit)
  if (temp_raw & 0x0400) {
    // Extend sign for 11-bit to 16-bit
    temp_raw |= 0xF800;
  }

  // Convert to temperature (each LSB = 0.125°C)
  temperature = temp_raw * 0.125f;

  return temperature;
}

/*******************************************************************************
  * Reset I2C state machine after error
  *
  * @param None
  * @return None
  ******************************************************************************/
static void reset_i2c_state_machine(void)
{
  i2c_send_data_flag    = true;
  i2c_receive_data_flag = false;
  current_mode          = SL_I2C_SEND_DATA;
  set_sensor_temperature(0.0f);
}

/*******************************************************************************
  * Handle I2C send errors
  * 
  * @param[in] status I2C error status code
  * @return None
  ******************************************************************************/
static void handle_i2c_send_error(sl_i2c_status_t status)
{
  DEBUGOUT("\r\n Send FAILED, Error Code: %u\r\n", status);

  if (status == SL_I2C_TIMEOUT) {
    DEBUGOUT("\r\n TIMEOUT: No response from LM75 sensor\r\n");
  } else if (status == SL_I2C_NACK) {
    DEBUGOUT("\r\n NACK: LM75 not acknowledging at address 0x%02X\r\n", FOLLOWER_I2C_ADDR);
  }
  reset_i2c_state_machine();
}

/*******************************************************************************
  * Handle I2C receive errors
  * 
  * @param[in] status I2C error status code
  * @return None
  ******************************************************************************/
static void handle_i2c_receive_error(sl_i2c_status_t status)
{
  DEBUGOUT("\r\n Receive FAILED, Error Code: %u\r\n", status);

  if (status == SL_I2C_TIMEOUT) {
    DEBUGOUT("\r\n TIMEOUT: No data received from LM75\r\n");
  } else if (status == SL_I2C_NACK) {
    DEBUGOUT("\r\n NACK: Device not responding to read request\r\n");
  }

  reset_i2c_state_machine();
}

/*******************************************************************************
  * Execute one I2C temperature read cycle
  *
  * Reads temperature from LM75 sensor using state machine (write pointer,
  * read data). Call periodically to refresh temperature readings.
  *
  * PS4 power requirement must be active before calling this function.
  *
  * @param None
  * @return None
  *
  * @note Blocking operation (~10-20ms)
  * @note Temperature available via get_sensor_temperature() after successful read
  ******************************************************************************/
void i2c_leader_example_process_action(void)
{
  sl_i2c_status_t i2c_status;

  // State machine for I2C data transfer
  switch (current_mode) {
    case SL_I2C_SEND_DATA:
      if (i2c_send_data_flag) {
        // LM75 protocol: Send 1 byte (pointer register 0x00 for temperature)
        i2c_write_buffer[0] = 0x00; // Point to temperature register

        // Use unified driver blocking send API
        i2c_status =
          sl_i2c_driver_send_data_blocking(i2c_instance, FOLLOWER_I2C_ADDR, i2c_write_buffer, LM75_POINTER_REG_SIZE);
        if (i2c_status != SL_I2C_SUCCESS) {
          handle_i2c_send_error(i2c_status);
          return;
        }

        DEBUGOUT("\r\n Data is transferred to Follower successfully\r\n");
        i2c_send_data_flag = false;
      }

      // Move to receive state
      i2c_receive_data_flag = true;
      current_mode          = SL_I2C_RECEIVE_DATA;

      // CRITICAL: Delay after send before receive
      // Allows LM75 sensor time to prepare temperature data
      delay(I2C_INTER_TRANSACTION_DELAY_MS);

      __attribute__((fallthrough));

    case SL_I2C_RECEIVE_DATA:
      if (i2c_receive_data_flag) {
        // Use unified driver blocking receive API (with repeated start enabled)
        i2c_status =
          sl_i2c_driver_receive_data_blocking(i2c_instance, FOLLOWER_I2C_ADDR, i2c_read_buffer, LM75_DATA_SIZE);

        if (i2c_status != SL_I2C_SUCCESS) {
          handle_i2c_receive_error(i2c_status);
          return;
        }

        // Convert and display temperature
        set_sensor_temperature(lm75_convert_temperature(i2c_read_buffer));

        DEBUGOUT("\r\n Temperature in Celsius %f \t \r\n", get_sensor_temperature());

        DEBUGOUT("\r\n Data is received from Follower successfully\r\n");
        i2c_receive_data_flag = false;
      }

      // Clear read buffer
      i2c_read_buffer[0] = 0;
      i2c_read_buffer[1] = 0;

      // Transaction complete - ready for next cycle
      i2c_send_data_flag = true;
      current_mode       = SL_I2C_SEND_DATA;
      break;

    default:
      DEBUGOUT("\r\n Unknown mode: %d\r\n", current_mode);
      break;
  }
}
