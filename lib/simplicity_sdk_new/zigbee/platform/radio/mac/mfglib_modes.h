/***************************************************************************//**
 * @file mfglib_modes.h
 *
 * @brief Mfglib Modes
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

#ifndef _MFGLIB_MODES_H_
#define _MFGLIB_MODES_H_

// Defined values of sli_legacy_mfglib_mode
#define MFGLIB_OFF                0
#define MFGLIB_NORMAL             1
#define MFGLIB_TONE               2
#define MFGLIB_STREAM             3
#define MFGLIB_INTERMEDIATE       4 //MAC index 1 has been shutdown, Waiting on index 0 to shutdown

#endif /* _MFGLIB_MODES_H_ */
