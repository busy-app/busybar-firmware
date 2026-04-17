/***************************************************************************//**
 * # License
 * <b> Copyright 2025 Silicon Laboratories Inc. www.silabs.com </b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of the Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * https://www.silabs.com/about-us/legal/master-software-license-agreement
 * By installing, copying or otherwise using this software, you agree to the
 * terms of the MSLA.
 *
 ******************************************************************************/

#ifndef _ZW_HARDWARE_CONFIG_H_
#define _ZW_HARDWARE_CONFIG_H_

#include "em_emu.h"
#include "sl_dcdc.h"
// <<< Use Configuration Wizard in Context Menu >>>

// <h> Z-Wave Energy Management Unit Configuration

// <o ZW_DCDC_CONFIG> DC/DC Converter Mode
// <i> Select the DC/DC converter operating mode. AUTO mode enables voltage monitoring and dynamically switches between bypass and regulation based on supply voltage. See sl_dcdc.h for more details.
// <EDCDCMODE_AUTO=> Auto - Dynamic switching based on voltage monitoring
// <EDCDCMODE_BYPASS=> Bypass - Always use bypass mode
// <EDCDCMODE_DCDC_LOW_NOISE=> DCDC Low Noise - Always use regulation mode
// <i> Default: EDCDCMODE_AUTO
#define ZW_DCDC_CONFIG  EDCDCMODE_AUTO

// <o ZW_EM4_PIN_RETENTION_MODE> EM4 Pin Retention Mode
// <i> Configure how GPIO pins behave during EM4 deep sleep. Latch mode retains pin states through EM4 and requires unlatching after wakeup. See em_emu.h for more details.
// <emuPinRetentionDisable=> No Retention - Pads enter reset state when entering EM4
// <emuPinRetentionEm4Exit=> Retention through EM4 - Pads enter reset state when exiting EM4
// <emuPinRetentionLatch=> Retention through EM4 and wakeup - Requires EMU_UnlatchPinRetention() after wakeup
// <i> Default: emuPinRetentionLatch
#define ZW_EM4_PIN_RETENTION_MODE  emuPinRetentionLatch

// <o ZW_EM4_INITIAL_STATE> EM4 Initial State
// <i> Configure the EM4 energy mode state. Hibernate retains RAM and registers, Shutoff provides lowest power but loses all state.
// <emuEM4Shutoff=> EM4 Shutoff - Lowest power, no state retention
// <emuEM4Hibernate=> EM4 Hibernate - Retains RAM and registers
// <i> Default: emuEM4Hibernate
#define ZW_EM4_INITIAL_STATE  emuEM4Hibernate

// <o ZW_EM23_VOLTAGE_SCALE> EM2/3 Voltage Scaling Level
// <i> Configure the voltage scaling level for EM2 and EM3 energy modes. Low Power mode adds approximately 30 us to wakeup time but reduces power consumption.
// <emuVScaleEM23_FastWakeup=> Fast Wakeup - Faster wakeup time, higher power consumption
// <emuVScaleEM23_LowPower=> Low Power - Lower power consumption, adds ~30 us to wakeup time
// <i> Default: emuVScaleEM23_LowPower
#define ZW_EM23_VOLTAGE_SCALE  emuVScaleEM23_LowPower

// <q ZW_EM4_RETAIN_LFXO> Retain LFXO in EM4
// <i> Enable to retain the Low Frequency Crystal Oscillator (LFXO) during EM4 deep sleep.
// <i> Default: 0
#define ZW_EM4_RETAIN_LFXO  0

// <q ZW_EM4_RETAIN_LFRCO> Retain LFRCO in EM4
// <i> Enable to retain the Low Frequency RC Oscillator (LFRCO) during EM4 deep sleep. Required for BURTC timer functionality.
// <i> Default: 1
#define ZW_EM4_RETAIN_LFRCO  1

// <q ZW_EM4_RETAIN_ULFRCO> Retain ULFRCO in EM4
// <i> Enable to retain the Ultra Low Frequency RC Oscillator (ULFRCO) during EM4 deep sleep.
// <i> Default: 0
#define ZW_EM4_RETAIN_ULFRCO  0

// </h>

// <<< end of configuration section >>>

#endif /* _ZW_HARDWARE_CONFIG_H_ */
