/*******************************************************************************
* @file  sl_qei_board.h
* @brief sl QEI board specific configuration
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef __SL_QEI_BOARD_H__
#define __SL_QEI_BOARD_H__

/**
 * @defgroup  Board_Specific_Driver   Board specific drivers and support functions
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup  Board_Drivers BOARD: Common board components used with board drivers
 * @{
 */
#include "em_device.h"

// QEI Direction Pin
#define SL_SL91X_QEI_DIR_PORT RTE_QEI_DIR_PORT
#define SL_SL91X_QEI_DIR_PIN  RTE_QEI_DIR_PIN
#define SL_SL91X_QEI_DIR_MUX  RTE_QEI_DIR_MUX
#define SL_SL91X_QEI_DIR_PAD  RTE_QEI_DIR_PAD

// QEI Index Pin
#define SL_SL91X_QEI_IDX_PORT RTE_QEI_IDX_PORT
#define SL_SL91X_QEI_IDX_PIN  RTE_QEI_IDX_PIN
#define SL_SL91X_QEI_IDX_MUX  RTE_QEI_IDX_MUX
#define SL_SL91X_QEI_IDX_PAD  RTE_QEI_IDX_PAD

// QEI Phase A Pin
#define SL_SL91X_QEI_PHA_PORT RTE_QEI_PHA_PORT
#define SL_SL91X_QEI_PHA_PIN  RTE_QEI_PHA_PIN
#define SL_SL91X_QEI_PHA_MUX  RTE_QEI_PHA_MUX
#define SL_SL91X_QEI_PHA_PAD  RTE_QEI_PHA_PAD

// QEI Phase B Pin
#define SL_SL91X_QEI_PHB_PORT RTE_QEI_PHB_PORT
#define SL_SL91X_QEI_PHB_PIN  RTE_QEI_PHB_PIN
#define SL_SL91X_QEI_PHB_MUX  RTE_QEI_PHB_MUX
#define SL_SL91X_QEI_PHB_PAD  RTE_QEI_PHB_PAD

#ifdef __cplusplus
}
#endif

#endif /*__SL_QEI_BOARD_H__*/

/* @} end of Board_Drivers */
/* @} end of Board_Specific_Driver */
