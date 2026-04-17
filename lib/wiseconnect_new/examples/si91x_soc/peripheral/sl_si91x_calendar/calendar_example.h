/***************************************************************************/ /**
 * @file calendar_example.h
 * @brief Calendar example
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

#ifndef CALENDAR_EXAMPLE_H_
#define CALENDAR_EXAMPLE_H_

// -----------------------------------------------------------------------------
// Macros
#define ALARM_EXAMPLE   DISABLE ///< To enable alarm trigger
#define SEC_INTR        DISABLE ///< To enable one second trigger
#define MILLI_SEC_INTR  DISABLE ///< To enable one millisecond trigger
#define TIME_CONVERSION DISABLE ///< To enable time conversion

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
 * Calendar example initialization function
 * Calendar clock is configured.
 * A deafult structure of datetime is builded and set as the current value of calendar
 * The value of calendar is fetched and displayed on serial console.
 * As per the macros are enabled, the example will run alarm, millisecond trigger
 * one second trigger, time conversion and clock calibration.
 * 
 * @param none
 * @return none
 ******************************************************************************/
void calendar_example_init(void);

/***************************************************************************/ /**
 * Function will run continuously and will wait for trigger
 * 
 * @param none
 * @return none
 ******************************************************************************/
void calendar_example_process_action(void);

#endif /* CALENDAR_EXAMPLE_H_ */
