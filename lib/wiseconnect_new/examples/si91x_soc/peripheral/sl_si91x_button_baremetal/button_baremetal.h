/***************************************************************************/ /**
 * @file button_baremetal.h
 * @brief button baremetal examples functions
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

#ifndef BUTTON_BAREMETAL_H
#define BUTTON_BAREMETAL_H

/***************************************************************************/ /**
 * Initialize example
 ******************************************************************************/
void button_init(void);

/***************************************************************************/ /**
 * Call back on button change.
 ******************************************************************************/
void button_process_action(void);

#endif // BUTTON_BAREMETAL_H
