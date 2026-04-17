/*******************************************************************************
 * @file  sl_si91x_psram_aps6404l_sqrh_config.h
 * @brief
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
#ifndef __SL_SI91X_PSRAM_APS6404L_SQRH_CONFIG_H_
#define __SL_SI91X_PSRAM_APS6404L_SQRH_CONFIG_H_

#include "rsi_qspi_proto.h"
#include "sl_si91x_psram.h"

// <<< Use Configuration Wizard in Context Menu >>>

// <h> PSRAM Device Configurations

// <o PSRAM_RW_TYPE>PSRAM Read-Write Type
//   <NORMAL_RW=> Normal Read-Write
//   <FAST_RW=> Fast Read-Write
//   <QUAD_RW=> Quad IO Read-Write
// <i> Default: Quad IO Read-Write
// <d> QUAD_RW
#define PSRAM_RW_TYPE QUAD_RW

// <o PSRAM_INTERFACE_MODE>PSRAM Interface Mode
//   <SINGLE_MODE=> SPI Mode
//   <QUAD_MODE=> QPI Mode
// <i> Default: QPI Mode
// <d> QUAD_MODE
#define PSRAM_INTERFACE_MODE QUAD_MODE

// </h> end PSRAM Device Configurations

// <<< end of configuration section >>>

// PSRAM Device Specific Information
#define PSRAM_DEVICE_NAME APS6404L_SQRH

#define PSRAM_DEVICE_DENSITY_MBIT 64

#define PSRAM_DEVICE_MFID 0x0D

#define PSRAM_DEVICE_KGD 0x5D

#define PSRAM_BURST_LEN_TOGGLE_SUPPORTED 0

#define PSRAM_HALF_SLEEP_SUPPORTED 1

#define PSRAM_ROW_BOUNDARY_CROSSING_SUPPORTED 1

#define PSRAM_NORMAL_READ_MAX_FREQ_MHZ 33

#define PSRAM_FAST_READ_MAX_FREQ_MHZ 84

#define PSRAM_DEFAULT_BURST_WRAP_SIZE 0

#define PSRAM_TOGGLE_BURST_WRAP_SIZE 0

/* Configurations for Interface Modes */
#if (PSRAM_INTERFACE_MODE == SINGLE_MODE)

#if (PSRAM_RW_TYPE == NORMAL_RW)

#define PSRAM_INST_MODE       SINGLE_MODE
#define PSRAM_ADDR_MODE       SINGLE_MODE
#define PSRAM_DIO_MODE        SINGLE_MODE
#define PSRAM_WAIT_CYCLES     0
#define PSRAM_DUMMY_MODE      PSRAM_ADDR_MODE
#define PSRAM_EXTRA_BYTE_MODE PSRAM_ADDR_MODE
#define PSRAM_RD_DUMMY_BITS   (PSRAM_WAIT_CYCLES)

#endif // (PSRAM_RW_TYPE == NORMAL_RW)

#if (PSRAM_RW_TYPE == FAST_RW)

#define PSRAM_INST_MODE       SINGLE_MODE
#define PSRAM_ADDR_MODE       SINGLE_MODE
#define PSRAM_DIO_MODE        SINGLE_MODE
#define PSRAM_WAIT_CYCLES     8
#define PSRAM_DUMMY_MODE      PSRAM_ADDR_MODE
#define PSRAM_EXTRA_BYTE_MODE PSRAM_ADDR_MODE
#define PSRAM_RD_DUMMY_BITS   (PSRAM_WAIT_CYCLES)

#endif //(PSRAM_RW_TYPE == FAST_RW)

#if (PSRAM_RW_TYPE == QUAD_RW)

#define PSRAM_INST_MODE       SINGLE_MODE
#define PSRAM_ADDR_MODE       QUAD_MODE
#define PSRAM_DIO_MODE        QUAD_MODE
#define PSRAM_WAIT_CYCLES     6
#define PSRAM_DUMMY_MODE      PSRAM_ADDR_MODE
#define PSRAM_EXTRA_BYTE_MODE PSRAM_ADDR_MODE
#define PSRAM_RD_DUMMY_BITS   (PSRAM_WAIT_CYCLES * 4) //due to dummy mode being QPI

#endif //(PSRAM_RW_TYPE == QUAD_RW)

#elif (PSRAM_INTERFACE_MODE == QUAD_MODE)

#if (PSRAM_RW_TYPE == NORMAL_RW)

#define PSRAM_INST_MODE       QUAD_MODE
#define PSRAM_ADDR_MODE       QUAD_MODE
#define PSRAM_DIO_MODE        QUAD_MODE
#define PSRAM_WAIT_CYCLES     0
#define PSRAM_DUMMY_MODE      PSRAM_ADDR_MODE
#define PSRAM_EXTRA_BYTE_MODE PSRAM_ADDR_MODE

#endif // (PSRAM_RW_TYPE == NORMAL_RW)

#if (PSRAM_RW_TYPE == FAST_RW)

#define PSRAM_INST_MODE       QUAD_MODE
#define PSRAM_ADDR_MODE       QUAD_MODE
#define PSRAM_DIO_MODE        QUAD_MODE
#define PSRAM_WAIT_CYCLES     4
#define PSRAM_DUMMY_MODE      PSRAM_ADDR_MODE
#define PSRAM_EXTRA_BYTE_MODE PSRAM_ADDR_MODE

#endif //(PSRAM_RW_TYPE == FAST_RW)

#if (PSRAM_RW_TYPE == QUAD_RW)

#define PSRAM_INST_MODE       QUAD_MODE
#define PSRAM_ADDR_MODE       QUAD_MODE
#define PSRAM_DIO_MODE        QUAD_MODE
#define PSRAM_WAIT_CYCLES     6
#define PSRAM_DUMMY_MODE      PSRAM_ADDR_MODE
#define PSRAM_EXTRA_BYTE_MODE PSRAM_ADDR_MODE

#endif //(PSRAM_RW_TYPE == QUAD_RW)

#define PSRAM_RD_DUMMY_BITS (PSRAM_WAIT_CYCLES * 4)

#endif

#if (PSRAM_RW_TYPE == QUAD_RW)

#define PSRAM_READ_CMD_CODE  0xEB
#define PSRAM_WRITE_CMD_CODE 0x38

#elif (PSRAM_RW_TYPE == FAST_RW)

#define PSRAM_READ_CMD_CODE  0x0B
#define PSRAM_WRITE_CMD_CODE 0x02

#elif (PSRAM_RW_TYPE == NORMAL_RW)

#define PSRAM_READ_CMD_CODE  0x03
#define PSRAM_WRITE_CMD_CODE 0x02

#endif

#define PSRAM_MODEL_WRAP 0

// protect this macros under a macro for non secure chips. else not required
/*AES Security Configuration*/
#define PSRAM_AES_KEY_SIZE   128
#define PSRAM_AES_USE_KEY_KH 1

#endif //__SL_SI91X_PSRAM_APS6404L_SQRH_CONFIG_H_