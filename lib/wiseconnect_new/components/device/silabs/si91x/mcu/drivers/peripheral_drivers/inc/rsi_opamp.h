/******************************************************************************
* @file  rsi_opamp.h
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
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
******************************************************************************/

// Includes Files

#ifndef RSI_OPAMP_H
#define RSI_OPAMP_H

#include "si91x_device.h"

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif // UNUSED_PARAMETER

#ifdef __cplusplus
extern "C" {
#endif

#define BG_SCDC_PROG_REG_1 0x127

#define AGPIO_PIN0  0
#define AGPIO_PIN1  1
#define AGPIO_PIN2  2
#define AGPIO_PIN3  3
#define AGPIO_PIN4  4
#define AGPIO_PIN5  5
#define AGPIO_PIN6  6
#define AGPIO_PIN7  7
#define AGPIO_PIN8  8
#define AGPIO_PIN9  9
#define AGPIO_PIN10 10
#define AGPIO_PIN11 11
#define AGPIO_PIN12 12
#define AGPIO_PIN13 13
#define AGPIO_PIN14 14
#define AGPIO_PIN15 15

#define TGPIO_PIN0 25
#define TGPIO_PIN1 26
#define TGPIO_PIN2 27
#define TGPIO_PIN3 28
#define TGPIO_PIN4 29
#define TGPIO_PIN5 30
#define TGPIO_PIN6 23
#define TGPIO_PIN7 24

#define AGPIO_MODE 7
#define TGPIO_MODE 14

#define OPAMP1_P_SEL_PIN AGPIO_PIN0
#define OPAMP1_N_SEL_PIN AGPIO_PIN7
#define OPAMP1_OUT       AGPIO_PIN4

#define OPAMP2_P_SEL_PIN AGPIO_PIN5
#define OPAMP2_N_SEL_PIN AGPIO_PIN11
#define OPAMP2_OUT       AGPIO_PIN9

#define OPAMP3_P_SEL_PIN AGPIO_PIN10
#define OPAMP3_N_SEL_PIN
#define OPAMP3_OUT TGPIO_PIN2

typedef struct {
  uint8_t opamp1_dyn_en;
  uint8_t opamp1_sel_p_mux;
  uint8_t opamp1_sel_n_mux;
  uint8_t opamp1_lp_mode;
  uint8_t opamp1_r1_sel;
  uint8_t opamp1_r2_sel;
  uint8_t opamp1_en_res_bank;
  uint8_t opamp1_res_mux_sel;
  uint8_t opamp1_res_to_out_vdd;
  uint8_t opamp1_out_mux_en;
  uint8_t opamp1_out_mux_sel;
  uint8_t opamp1_enable;
} OPAMP1_CONFIG_T;

typedef struct {
  uint8_t opamp2_dyn_en;
  uint8_t opamp2_sel_p_mux;
  uint8_t opamp2_sel_n_mux;
  uint8_t opamp2_lp_mode;
  uint8_t opamp2_r1_sel;
  uint8_t opamp2_r2_sel;
  uint8_t opamp2_en_res_bank;
  uint8_t opamp2_res_mux_sel;
  uint8_t opamp2_res_to_out_vdd;
  uint8_t opamp2_out_mux_en;
  uint8_t opamp2_enable;

} OPAMP2_CONFIG_T;

typedef struct {
  uint8_t opamp3_dyn_en;
  uint8_t opamp3_sel_p_mux;
  uint8_t opamp3_sel_n_mux;
  uint8_t opamp3_lp_mode;
  uint8_t opamp3_r1_sel;
  uint8_t opamp3_r2_sel;
  uint8_t opamp3_en_res_bank;
  uint8_t opamp3_res_mux_sel;
  uint8_t opamp3_res_to_out_vdd;
  uint8_t opamp3_out_mux_en;
  uint8_t opamp3_enable;
} OPAMP3_CONFIG_T;

typedef struct {
  OPAMP1_CONFIG_T opamp1;
  OPAMP2_CONFIG_T opamp2;
  OPAMP3_CONFIG_T opamp3;
} OPAMP_CONFIG_T;

void RSI_OPAMP1_UGB(uint8_t vin_p_sel,
                    uint8_t enable,
                    uint8_t lp_mode,
                    uint8_t out_mux_en,
                    uint8_t out_mux_sel,
                    uint8_t dyn_en,
                    uint8_t channel);
void RSI_OPAMP2_UGB(uint8_t vin_p_sel,
                    uint8_t enable,
                    uint8_t lp_mode,
                    uint8_t out_mux_en,
                    uint8_t dyn_en,
                    uint8_t channel);

void RSI_OPAMP3_UGB(uint8_t vin_p_sel,
                    uint8_t enable,
                    uint8_t lp_mode,
                    uint8_t out_mux_en,
                    uint8_t dyn_en,
                    uint8_t channel);

void RSI_OPAMP1_TIA(uint8_t vref_sel,
                    uint8_t iin_sel,
                    uint8_t enable,
                    uint8_t lp_mode,
                    uint8_t r2_sel,
                    uint8_t out_mux_en,
                    uint8_t out_mux_sel,
                    uint8_t dyn_en,
                    uint8_t channel);

void RSI_OPAMP2_TIA(uint8_t vref_sel,
                    uint8_t iin_sel,
                    uint8_t enable,
                    uint8_t lp_mode,
                    uint8_t r2_sel,
                    uint8_t out_mux_en,
                    uint8_t dyn_en,
                    uint8_t channel);

void RSI_OPAMP3_TIA(uint8_t vref_sel,
                    uint8_t iin_sel,
                    uint8_t enable,
                    uint8_t lp_mode,
                    uint8_t r2_sel,
                    uint8_t out_mux_en,
                    uint8_t dyn_en,
                    uint8_t channel);

void RSI_OPAMP1_InvPGA(uint8_t vref_sel,
                       uint8_t vin_sel,
                       uint8_t enable,
                       uint8_t lp_mode,
                       uint8_t r1_sel,
                       uint8_t r2_sel,
                       uint8_t out_mux_en,
                       uint8_t out_mux_sel,
                       uint8_t dyn_en,
                       uint8_t channel);

void RSI_OPAMP2_InvPGA(uint8_t vref_sel,
                       uint8_t vin_sel,
                       uint8_t enable,
                       uint8_t lp_mode,
                       uint8_t r1_sel,
                       uint8_t r2_sel,
                       uint8_t out_mux_en,
                       uint8_t dyn_en,
                       uint8_t channel);

void RSI_OPAMP3_InvPGA(uint8_t vref_sel,
                       uint8_t vin_sel,
                       uint8_t enable,
                       uint8_t lp_mode,
                       uint8_t r1_sel,
                       uint8_t r2_sel,
                       uint8_t out_mux_en,
                       uint8_t dyn_en,
                       uint8_t channel);

void RSI_OPAMP1_NonInvPGA(uint8_t vin_sel,
                          uint8_t vref_sel,
                          uint8_t enable,
                          uint8_t lp_mode,
                          uint8_t r1_sel,
                          uint8_t r2_sel,
                          uint8_t out_mux_en,
                          uint8_t out_mux_sel,
                          uint8_t dyn_en,
                          uint8_t channel);

void RSI_OPAMP2_NonInvPGA(uint8_t vin_sel,
                          uint8_t vref_sel,
                          uint8_t enable,
                          uint8_t lp_mode,
                          uint8_t r1_sel,
                          uint8_t r2_sel,
                          uint8_t out_mux_en,
                          uint8_t dyn_en,
                          uint8_t channel);

void RSI_OPAMP3_NonInvPGA(uint8_t vin_sel,
                          uint8_t vref_sel,
                          uint8_t enable,
                          uint8_t lp_mode,
                          uint8_t r1_sel,
                          uint8_t r2_sel,
                          uint8_t out_mux_en,
                          uint8_t dyn_en,
                          uint8_t channel);

void RSI_OPAMP1_InvCMP(uint8_t vin_sel,
                       uint8_t vref_sel,
                       uint8_t enable,
                       uint8_t lp_mode,
                       uint8_t r1_sel,
                       uint8_t r2_sel,
                       uint8_t out_mux_en,
                       uint8_t out_mux_sel,
                       uint8_t dyn_en,
                       uint8_t channel);

void RSI_OPAMP2_InvCMP(uint8_t vin_sel,
                       uint8_t vref_sel,
                       uint8_t enable,
                       uint8_t lp_mode,
                       uint8_t r1_sel,
                       uint8_t r2_sel,
                       uint8_t out_mux_en,
                       uint8_t dyn_en,
                       uint8_t channel);

void RSI_OPAMP3_InvCMP(uint8_t vin_sel,
                       uint8_t vref_sel,
                       uint8_t enable,
                       uint8_t lp_mode,
                       uint8_t r1_sel,
                       uint8_t r2_sel,
                       uint8_t out_mux_en,
                       uint8_t dyn_en,
                       uint8_t channel);

void RSI_OPAMP1_NonInvCMP(uint8_t vin_sel,
                          uint8_t vref_sel,
                          uint8_t enable,
                          uint8_t lp_mode,
                          uint8_t r1_sel,
                          uint8_t r2_sel,
                          uint8_t out_mux_en,
                          uint8_t out_mux_sel,
                          uint8_t dyn_en,
                          uint8_t channel);

void RSI_OPAMP2_NonInvCMP(uint8_t vin_sel,
                          uint8_t vref_sel,
                          uint8_t enable,
                          uint8_t lp_mode,
                          uint8_t r1_sel,
                          uint8_t r2_sel,
                          uint8_t out_mux_en,
                          uint8_t dyn_en,
                          uint8_t channel);

void RSI_OPAMP3_NonInvCMP(uint8_t vin_sel,
                          uint8_t vref_sel,
                          uint8_t enable,
                          uint8_t lp_mode,
                          uint8_t r1_sel,
                          uint8_t r2_sel,
                          uint8_t out_mux_en,
                          uint8_t dyn_en,
                          uint8_t channel);

void RSI_OPAMP_InstrAMP(uint8_t vin_p_sel,
                        uint8_t vin_n_sel,
                        uint8_t enable,
                        uint8_t lp_mode,
                        uint8_t r1_sel,
                        uint8_t r2_sel,
                        uint8_t out_mux_en,
                        uint8_t dyn_en,
                        uint8_t channel);
void RSI_OPAMP_Instrumentation_Amplifier(uint32_t channel, const OPAMP_CONFIG_T *Config);
void RSI_OPAMP1_Config(OPAMP_Type *opamp, uint32_t channel, const OPAMP_CONFIG_T *config);

void RSI_OPAMP2_Config(OPAMP_Type *pstcOpamp, uint32_t channel, const OPAMP_CONFIG_T *config);

void RSI_OPAMP3_Config(OPAMP_Type *pstcOpamp, uint32_t channel, const OPAMP_CONFIG_T *config);
void OPAMP_Pinmuxing();

typedef struct {
  uint8_t vin_p_sel;
  uint8_t vin_n_sel;
  uint8_t vref_sel;
  uint8_t enable;
  uint8_t lp_mode;
  uint8_t r1_sel;
  uint8_t r2_sel;
  uint8_t out_mux_en;
  uint8_t out_mux_sel;
} Configure_OPAMP_t;

typedef struct {
  uint8_t vin1_p_sel;
  uint8_t vin1_n_sel;
  uint8_t vref1_sel;
  uint8_t vin2_p_sel;
  uint8_t vin2_n_sel;
  uint8_t vref2_sel;
  uint8_t enable;
  uint8_t lp_mode;
  uint8_t opamp1_r1_sel;
  uint8_t opamp1_r2_sel;
  uint8_t opamp2_r1_sel;
  uint8_t opamp2_r2_sel;
  uint8_t out_mux_en;
  uint8_t out_mux_sel;
} Configure_OPAMP1_OPAMP2_t;

typedef struct {
  uint8_t vin2_p_sel;
  uint8_t vin2_n_sel;
  uint8_t vref2_sel;
  uint8_t vin3_p_sel;
  uint8_t vin3_n_sel;
  uint8_t vref3_sel;
  uint8_t enable;
  uint8_t lp_mode;
  uint8_t opamp2_r1_sel;
  uint8_t opamp2_r2_sel;
  uint8_t opamp3_r1_sel;
  uint8_t opamp3_r2_sel;
  uint8_t out_mux_en;
  uint8_t out_mux_sel;
} Configure_OPAMP2_OPAMP3_t;

typedef struct {
  Configure_OPAMP_t opamp;
  Configure_OPAMP1_OPAMP2_t opamp1_opamp2;
  Configure_OPAMP2_OPAMP3_t opamp2_opamp3;
} Configure_OPAMP_All_t;

//<-- OPAMP instance -->
#define OPAMP1_INSTANCE 1 //<-- OPAMP1 instance -->
#define OPAMP2_INSTANCE 2 //<-- OPAMP2 instance -->
#define OPAMP3_INSTANCE 3 //<-- OPAMP3 instance -->

void RSI_OPAMP_UGB_V2(uint8_t opamp_instance, const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel);

void RSI_OPAMP_TIA_V2(uint8_t opamp_instance, const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel);

void RSI_OPAMP_InvPGA_V2(uint8_t opamp_instance, const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel);

void RSI_OPAMP_NonInvPGA_V2(uint8_t opamp_instance, const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel);

void RSI_OPAMP_InvCMP_V2(uint8_t opamp_instance, const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel);

void RSI_OPAMP_NonInvCMP_V2(uint8_t opamp_instance, const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel);

void RSI_OPAMP_InstrAMP_v2(const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel);

void RSI_OPAMP1_OPAMP2_CascInvtPGAmp(const Configure_OPAMP1_OPAMP2_t *params, uint8_t dyn_en, uint8_t channel);

void RSI_OPAMP2_OPAMP3_CascInvtPGAmp(const Configure_OPAMP2_OPAMP3_t *params, uint8_t dyn_en, uint8_t channel);

void RSI_OPAMP1_OPAMP2_CascNonInvtPGAmp(const Configure_OPAMP1_OPAMP2_t *params, uint8_t dyn_en, uint8_t channel);

void RSI_OPAMP2_OPAMP3_CascNonInvtPGAmp(const Configure_OPAMP2_OPAMP3_t *params, uint8_t dyn_en, uint8_t channel);

void RSI_OPAMP1_OPAMP2_TwoOpampsDiffAmp(const Configure_OPAMP1_OPAMP2_t *params, uint8_t dyn_en, uint8_t channel);

void RSI_OPAMP2_OPAMP3_TwoOpampsDiffAmp(const Configure_OPAMP2_OPAMP3_t *params, uint8_t dyn_en, uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif // RSI_OPAMP_H
