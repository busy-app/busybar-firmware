/***************************************************************************/ /**
 * @file i2c_leader_example.h
 * @brief I2C Leader examples functions
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
#ifndef I2C_LEADER_EXAMPLE_H_
#define I2C_LEADER_EXAMPLE_H_

// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************/ /**
 * Initialize I2C with PS4 power requirement
 *
 * Adds PS4, re-initializes GPIO pin mux, resets I2C hardware FSM, and
 * performs full I2C driver initialization. Call before each I2C transaction
 * cycle to recover from M4 deep sleep state corruption.
 *
 * @param none
 * @return none
 ******************************************************************************/
void i2c_init(void);

/***************************************************************************/ /**
 * Deinitialize I2C and remove PS4 power requirement
 *
 * Performs clean I2C driver shutdown while PS4 is active, then removes PS4
 * to allow M4 deep sleep between transactions.
 *
 * @param none
 * @return none
 ******************************************************************************/
void i2c_deinit(void);

/***************************************************************************/ /**
 * Execute one I2C temperature read cycle
 *
 * Reads temperature from LM75 sensor using a state machine (write pointer
 * register, then read data). PS4 must be active before calling.
 *
 * @param none
 * @return none
 ******************************************************************************/
void i2c_leader_example_process_action(void);

/***************************************************************************/ /**
 * Getter function for current temperature sensor reading
 *
 * @param none
 * @return Temperature in Celsius (float)
 ******************************************************************************/
float get_sensor_temperature(void);

#endif /* I2C_LEADER_EXAMPLE_H_ */
