/***************************************************************************/ /**
 * @file adc_example.h
 * @brief ADC example
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

#ifndef ADC_EXAMPLE_H_
#define ADC_EXAMPLE_H_

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
 * ADC example initialization function
 * ADC clock is configured followed by ADC initialize, ADC reference voltage
 * set configure, ADC channel configure and ADC start.
 *
 * @param none
 * @return none
 ******************************************************************************/
void adc_example_init(void);

/***************************************************************************/ /**
 * Function will run continuously
 * Based on ADC operation mode it will execute two different function and given
 * equivalent output of 12 bit ADC data
 *
 * @param none
 * @return none
 ******************************************************************************/
void adc_example_process_action(void);

#endif /* ADC_EXAMPLE_H_ */
