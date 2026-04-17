/***************************************************************************/
/**
 * @file ulp_i2c_leader_example.h
 * @brief ULP_I2C Leader Blocking examples functions
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
#ifndef ULP_I2C_LEADER_EXAMPLE_H_
#define ULP_I2C_LEADER_EXAMPLE_H_

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/
/**
 * It is an initialization function, it initializes the clock, pin configuration
 * init_para for I2C communication.
 *
 * @param none
 * @return none
 ******************************************************************************/
void ulp_i2c_leader_example_init(void);

/***************************************************************************/
/**
 * The state machine code for send and receive is implemented here.
 * This function is called in while loop.
 *
 * @param none
 * @return none
 ******************************************************************************/
void ulp_i2c_leader_example_process_action(void);

#endif /* I2C_LEADER_EXAMPLE_H_ */
