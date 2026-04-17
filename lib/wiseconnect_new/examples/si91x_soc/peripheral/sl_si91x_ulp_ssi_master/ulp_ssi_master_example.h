/***************************************************************************/ /**
* @file ulp_ssi_master_example.h
* @brief Ulp SSI Master examples functions
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

#ifndef ULP_SSI_MASTER_EXAMPLE_H_
#define ULP_SSI_MASTER_EXAMPLE_H_

#define ULP_SSI_MASTER_TRANSFER ENABLE
#define ULP_SSI_MASTER_SEND     DISABLE
#define ULP_SSI_MASTER_RECEIVE  DISABLE

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
* SSI master code entry point
*
* This function initializes the clock config, inits the driver, powers up the
* SSI peripheral. Then it configures the SSI peripheral as master, and starts
* a full duplex bi-directional data transfer. The board is configured as
* a loopback mode, and the Tx and Rx buffers are compared and if they are equal,
* the basic loopback test passes. This is displayed on the DEBUG prints output.
*
* @param none
* @return none
******************************************************************************/
void ssi_master_example_init(void);

/***************************************************************************/ /**
* Function will run continuously and will wait for interrupt
*
* @param none
* @return none
******************************************************************************/
void ssi_master_example_process_action(void);

#endif /* SL_SI91X_SSI_MASTER_EXAMPLE_H_ */
