/***************************************************************************/ /**
 * @file si70xx_example.h
 * @brief si70xx example
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
#ifndef SI70XX_EXAMPLE_H_
#define SI70XX_EXAMPLE_H_

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
 * RHT example initialization function.
 * @param none
 * @return none
 ******************************************************************************/
void si70xx_example_init(void);

/***************************************************************************/ /**
 * Function will run continuously in while loop and reads relative humidity and
 * temperature from sensor
 * @param none
 * @return none
 ******************************************************************************/
void si70xx_example_process_action(void);

#endif /* SI70XX_EXAMPLE_H_ */
