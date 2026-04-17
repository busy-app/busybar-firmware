/***************************************************************************/ /**
 * @file i2c_app.h
 * @brief I2C APP
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
#ifndef I2C_APP_H_
#define I2C_APP_H_

#define EVENT_FLAGS_I2C_TIMER_MASKBIT 0x00000001U // Mask bit to address specific Event Flag

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
 * It is an initialization function, it will create a task which will initiate
 * I2C --> i2c_leader_example_process_action.
 * init_para for I2C communication.
 *
 * @param none
 * @return none
 ******************************************************************************/
void i2c_app(void);

/***************************************************************************/ /**
 * It is an initialization function, it initializes the clock, pin configuration
 * init_para for I2C communication.
 * 
 * @param none
 * @return none
 ******************************************************************************/
void i2c_init(void);

/***************************************************************************/ /**
 * The state machine code for send and receive is implemented here.
 * This function is called in while loop.
 * 
 * @param none
 * @return none
 ******************************************************************************/
void i2c_leader_example_process_action(void);

#endif /* I2C_APP_H_ */
