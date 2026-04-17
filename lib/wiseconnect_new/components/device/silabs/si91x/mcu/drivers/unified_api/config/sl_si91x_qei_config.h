/***************************************************************************/ /**
 * @file sl_si91x_qei_config.h
 * @brief QEI configuration file.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#ifndef SL_SI91X_QEI_CONFIG_H
#define SL_SI91X_QEI_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/******************************* QEI Configuration **************************/
//  <e>Quadrature Encoder Mode
//  <i> Enable: Enables the Quadrature Encoder Mode
//  <i> Disable: Disables the Quadrature Encoder Mode(Enables Timer Mode)
#define ENCODER_MODE 1

// <o SL_QEI_ENCODING_MODE> QEI Encoding Mode
//   <SL_QEI_ENCODE_MODE_1X=> 1x Counting
//   <SL_QEI_ENCODE_MODE_2X=> 2x Counting
//   <SL_QEI_ENCODE_MODE_4X=> 4x Counting
// <i> Default: SL_QEI_ENCODE_MODE_1X
#define SL_QEI_ENCODING_MODE SL_QEI_ENCODE_MODE_1X

// <o SL_QEI_POS_CNT_MODE> QEI position counter mode
//   <SL_QEI_POS_CNT_32=> 32-bit position counter
//   <SL_QEI_POS_CNT_16=> 16-bit position counter
// <i> Default: SL_QEI_POS_CNT_32
#define SL_QEI_POS_CNT_MODE SL_QEI_POS_CNT_32

// <o SL_QEI_DIGITAL_FILTER_BYPASS> digital filter bypass
//   <SL_QEI_DIGITAL_FILTER=> Digital Filter is in-path
//   <SL_QEI_BYPASS_FILTER=> Digital Filter Bypassed
// <i> Default: SL_QEI_DIGITAL_FILTER
#define SL_QEI_DIGITAL_FILTER_BYPASS SL_QEI_DIGITAL_FILTER
// </e>

//  <e>QEI Velocity start
//  <i> Enable: Enables the Velocity
//  <i> Disable: Disables the Velocity
#define VELOCITY 0

// <o SL_QEI_DELTA_TIME> Delta Time(us) <0-1048576>
// <i> Default: 1000000
#define SL_QEI_DELTA_TIME 1000000
// </e>

/******************************************************************************/
// <<< end of configuration section >>>
// <<< sl:start pin_tool >>>
// <qei signal=(DIR),(IDX),PHA,PHB> SL_QEI
// $[QEI_SL_QEI]
#ifndef SL_QEI_PERIPHERAL
#define SL_QEI_PERIPHERAL QEI
#endif
#ifndef SL_QEI_PERIPHERAL_NO
#define SL_QEI_PERIPHERAL_NO 0
#endif

// QEI DIR on GPIO_28
#ifndef SL_QEI_DIR_PORT
#define SL_QEI_DIR_PORT HP
#endif
#ifndef SL_QEI_DIR_PIN
#define SL_QEI_DIR_PIN 28
#endif
#ifndef SL_QEI_DIR_LOC
#define SL_QEI_DIR_LOC 1
#endif

// QEI IDX on GPIO_25
#ifndef SL_QEI_IDX_PORT
#define SL_QEI_IDX_PORT HP
#endif
#ifndef SL_QEI_IDX_PIN
#define SL_QEI_IDX_PIN 25
#endif
#ifndef SL_QEI_IDX_LOC
#define SL_QEI_IDX_LOC 8
#endif

// QEI PHA on GPIO_26
#ifndef SL_QEI_PHA_PORT
#define SL_QEI_PHA_PORT HP
#endif
#ifndef SL_QEI_PHA_PIN
#define SL_QEI_PHA_PIN 26
#endif
#ifndef SL_QEI_PHA_LOC
#define SL_QEI_PHA_LOC 16
#endif

// QEI PHB on GPIO_27
#ifndef SL_QEI_PHB_PORT
#define SL_QEI_PHB_PORT HP
#endif
#ifndef SL_QEI_PHB_PIN
#define SL_QEI_PHB_PIN 27
#endif
#ifndef SL_QEI_PHB_LOC
#define SL_QEI_PHB_LOC 24
#endif
// [QEI_SL_QEI]$
// <<< sl:end pin_tool >>>

sl_qei_config_t sl_qei_config = {
  .config_param = (SL_QEI_DIGITAL_FILTER_BYPASS << QEI_DIGITAL_FILTER_BYPASS_MASK)
                  | (SL_QEI_POS_CNT_MODE << QEI_POS_CNT_MODE_MASK),
  .ctrl_param = (SL_QEI_ENCODING_MODE & QEI_ENCODING_MODE_MASK),
};

sl_qei_init_t sl_qei_init = {
  .dir = {
    .port = SL_SL91X_QEI_DIR_PORT,
    .pin  = SL_SL91X_QEI_DIR_PIN,
    .mux  = SL_SL91X_QEI_DIR_MUX,
    .pad  = SL_SL91X_QEI_DIR_PAD,
  },
  .idx = {
    .port = SL_SL91X_QEI_IDX_PORT,
    .pin  = SL_SL91X_QEI_IDX_PIN,
    .mux  = SL_SL91X_QEI_IDX_MUX,
    .pad  = SL_SL91X_QEI_IDX_PAD,
  },
  .phasea = {
    .port = SL_SL91X_QEI_PHA_PORT,
    .pin  = SL_SL91X_QEI_PHA_PIN,
    .mux  = SL_SL91X_QEI_PHA_MUX,
    .pad  = SL_SL91X_QEI_PHA_PAD,
  },
  .phaseb = {
    .port = SL_SL91X_QEI_PHB_PORT,
    .pin  = SL_SL91X_QEI_PHB_PIN,
    .mux  = SL_SL91X_QEI_PHB_MUX,
    .pad  = SL_SL91X_QEI_PHB_PAD,
  },
};

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_QEI_CONFIG_H */
