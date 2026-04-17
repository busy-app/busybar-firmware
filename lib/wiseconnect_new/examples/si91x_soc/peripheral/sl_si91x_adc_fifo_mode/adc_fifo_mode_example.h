/***************************************************************************/ /**
 * @file adc_fifo_mode_example.h
 * @brief ADC example for FIFO mode
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

#ifndef ADC_FIFO_MODE_EXAMPLE_H_
#define ADC_FIFO_MODE_EXAMPLE_H_

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
 * ADC example initialization function for fifo mode of operation.
 * ADC clock is configured followed by ADC initialize, ADC reference voltage
 * set configure, ADC channel configure and ADC start.
 *
 * @param none
 * @return none
 ******************************************************************************/
void adc_fifo_mode_example_init(void);

/***************************************************************************/ /**
 * Function will run continuously
 * It will execute on ADC fifo mode function and given
 * equivalent output of 12 bit ADC data
 *
 * @param none
 * @return none
 ******************************************************************************/
void adc_fifo_mode_example_process_action(void);

#endif /* ADC_FIFO_MODE_EXAMPLE_H_ */
