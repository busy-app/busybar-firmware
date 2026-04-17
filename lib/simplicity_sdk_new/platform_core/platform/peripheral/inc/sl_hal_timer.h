/***************************************************************************//**
 * @file
 * @brief Timer/counter (TIMER) peripheral API
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_HAL_TIMER_H
#define SL_HAL_TIMER_H

#include "em_device.h"
#if defined(TIMER_COUNT) && (TIMER_COUNT > 0)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sl_assert.h"
#include "sl_enum.h"

/***************************************************************************//**
 * @addtogroup timer TIMER - Timer/Counter
 * @{
 ******************************************************************************/

/*******************************************************************************
 ********************************   DEFINES   **********************************
 ******************************************************************************/
/// Check if TIMER instance is valid.
#define SL_HAL_TIMER_REF_VALID(timer_ref)    (TIMER_NUM(timer_ref) != -1)

/// Return TIMER instance max count.
#define SL_HAL_TIMER_MAX_COUNT(timer_ref)    ((uint32_t)((1ULL << TIMER_CNTWIDTH(TIMER_NUM(timer_ref))) - 1))

/// Return TIMER instance channel count.
#define SL_HAL_TIMER_CHANNEL_NUM(timer_ref)  (TIMER_CC_NUM(TIMER_NUM(timer_ref)))

/// Check if TIMER instance supports DTI.
#define SL_HAL_TIMER_SUPPORTS_DTI(timer_ref) (TIMER_DTI(TIMER_NUM(timer_ref)) != 0)

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/
/// Timer compare/capture mode.
SL_ENUM(sl_hal_timer_channel_mode_t) {
  SL_HAL_TIMER_CHANNEL_MODE_OFF     = _TIMER_CC_CFG_MODE_OFF,            ///< Channel turned off.
  SL_HAL_TIMER_CHANNEL_MODE_CAPTURE = _TIMER_CC_CFG_MODE_INPUTCAPTURE,   ///< Input capture.
  SL_HAL_TIMER_CHANNEL_MODE_COMPARE = _TIMER_CC_CFG_MODE_OUTPUTCOMPARE,  ///< Output compare.
  SL_HAL_TIMER_CHANNEL_MODE_PWM     = _TIMER_CC_CFG_MODE_PWM             ///< Pulse-Width modulation.
};

/// Clock select.
SL_ENUM(sl_hal_timer_clock_select_t) {
  SL_HAL_TIMER_CLKSEL_HFPERCLK = _TIMER_CFG_CLKSEL_PRESCEM01GRPACLK, ///< Prescaled EM01GRPA clock.
  SL_HAL_TIMER_CLKSEL_CC1      = _TIMER_CFG_CLKSEL_CC1,              ///< Compare/Capture Channel 1 Input.
  SL_HAL_TIMER_CLKSEL_CASCADE  = _TIMER_CFG_CLKSEL_TIMEROUF          ///< Cascaded clocked by underflow or overflow by lower numbered timer.
};

/// Input capture edge select.
SL_ENUM(sl_hal_timer_channel_input_edge_t) {
  SL_HAL_TIMER_CHANNEL_EDGE_RISING  = _TIMER_CC_CTRL_ICEDGE_RISING,  ///< Rising edges detected.
  SL_HAL_TIMER_CHANNEL_EDGE_FALLING = _TIMER_CC_CTRL_ICEDGE_FALLING, ///< Falling edges detected.
  SL_HAL_TIMER_CHANNEL_EDGE_BOTH    = _TIMER_CC_CTRL_ICEDGE_BOTH,    ///< Both edges detected.
  SL_HAL_TIMER_CHANNEL_EDGE_NONE    = _TIMER_CC_CTRL_ICEDGE_NONE     ///< No edge detection, leave signal as is.
};

/// Input capture event control.
SL_ENUM(sl_hal_timer_channel_input_event_t) {
  /// PRS output pulse, interrupt flag, and DMA request set on every capture.
  SL_HAL_TIMER_CHANNEL_EVENT_EVERY_EDGE         = _TIMER_CC_CTRL_ICEVCTRL_EVERYEDGE,
  /// PRS output pulse, interrupt flag, and DMA request set on every second capture.
  SL_HAL_TIMER_CHANNEL_EVENT_EVERY_SECONDE_EDGE = _TIMER_CC_CTRL_ICEVCTRL_EVERYSECONDEDGE,
  /// PRS output pulse, interrupt flag, and DMA request set on rising edge (if
  /// input capture edge = BOTH).
  SL_HAL_TIMER_CHANNEL_EVENT_RISING             = _TIMER_CC_CTRL_ICEVCTRL_RISING,
  /// PRS output pulse, interrupt flag, and DMA request set on falling edge (if
  /// input capture edge = BOTH).
  SL_HAL_TIMER_CHANNEL_EVENT_FALLING            = _TIMER_CC_CTRL_ICEVCTRL_FALLING
};

/// Input edge action.
SL_ENUM(sl_hal_timer_input_edge_action_t) {
  SL_HAL_TIMER_INPUT_ACTION_NONE         = _TIMER_CTRL_FALLA_NONE,       ///< No action taken.
  SL_HAL_TIMER_INPUT_ACTION_START        = _TIMER_CTRL_FALLA_START,      ///< Start counter without reload.
  SL_HAL_TIMER_INPUT_ACTION_STOP         = _TIMER_CTRL_FALLA_STOP,       ///< Stop counter without reload.
  SL_HAL_TIMER_INPUT_ACTION_RELOAD_START = _TIMER_CTRL_FALLA_RELOADSTART ///< Reload and start counter.
};

/// Timer mode.
SL_ENUM(sl_hal_timer_mode_t) {
  SL_HAL_TIMER_MODE_UP     = _TIMER_CFG_MODE_UP,         ///< Up-counting.
  SL_HAL_TIMER_MODE_DOWN   = _TIMER_CFG_MODE_DOWN,       ///< Down-counting.
  SL_HAL_TIMER_MODE_UPDOWN = _TIMER_CFG_MODE_UPDOWN,     ///< Up/down-counting.
  SL_HAL_TIMER_MODE_QDEC   = _TIMER_CFG_MODE_QDEC        ///< Quadrature decoder.
};

/// Compare/capture output action.
SL_ENUM(sl_hal_timer_channel_output_action_t) {
  SL_HAL_TIMER_CHANNEL_OUTPUT_ACTION_NONE   = _TIMER_CC_CTRL_CUFOA_NONE,   ///< No action.
  SL_HAL_TIMER_CHANNEL_OUTPUT_ACTION_TOGGLE = _TIMER_CC_CTRL_CUFOA_TOGGLE, ///< Toggle on event.
  SL_HAL_TIMER_CHANNEL_OUTPUT_ACTION_CLEAR  = _TIMER_CC_CTRL_CUFOA_CLEAR,  ///< Clear on event.
  SL_HAL_TIMER_CHANNEL_OUTPUT_ACTION_SET    = _TIMER_CC_CTRL_CUFOA_SET     ///< Set on event.
};

/// Prescaler.
SL_ENUM_GENERIC(sl_hal_timer_prescaler_t, uint32_t) {
  SL_HAL_TIMER_PRESCALER_DIV1    = _TIMER_CFG_PRESC_DIV1,        ///< Divide by 1.
  SL_HAL_TIMER_PRESCALER_DIV2    = _TIMER_CFG_PRESC_DIV2,        ///< Divide by 2.
  SL_HAL_TIMER_PRESCALER_DIV4    = _TIMER_CFG_PRESC_DIV4,        ///< Divide by 4.
  SL_HAL_TIMER_PRESCALER_DIV8    = _TIMER_CFG_PRESC_DIV8,        ///< Divide by 8.
  SL_HAL_TIMER_PRESCALER_DIV16   = _TIMER_CFG_PRESC_DIV16,       ///< Divide by 16.
  SL_HAL_TIMER_PRESCALER_DIV32   = _TIMER_CFG_PRESC_DIV32,       ///< Divide by 32.
  SL_HAL_TIMER_PRESCALER_DIV64   = _TIMER_CFG_PRESC_DIV64,       ///< Divide by 64.
  SL_HAL_TIMER_PRESCALER_DIV128  = _TIMER_CFG_PRESC_DIV128,      ///< Divide by 128.
  SL_HAL_TIMER_PRESCALER_DIV256  = _TIMER_CFG_PRESC_DIV256,      ///< Divide by 256.
  SL_HAL_TIMER_PRESCALER_DIV512  = _TIMER_CFG_PRESC_DIV512,      ///< Divide by 512.
  SL_HAL_TIMER_PRESCALER_DIV1024 = _TIMER_CFG_PRESC_DIV1024      ///< Divide by 1024.
};

/// Input type.
SL_ENUM(sl_hal_timer_channel_input_type_t) {
  SL_HAL_TIMER_CHANNEL_INPUT_PIN             =  _TIMER_CC_CFG_INSEL_PIN,           ///< No PRS input.
  SL_HAL_TIMER_CHANNEL_INPUT_PRS_SYNC        =  _TIMER_CC_CFG_INSEL_PRSSYNC,       ///< Synchronous PRS selected.
  SL_HAL_TIMER_CHANNEL_INPUT_PRS_ASYNC_LEVEL =  _TIMER_CC_CFG_INSEL_PRSASYNCLEVEL, ///< Asynchronous level PRS selected.
  SL_HAL_TIMER_CHANNEL_INPUT_PRS_ASYNC_PULSE =  _TIMER_CC_CFG_INSEL_PRSASYNCPULSE, ///< Asynchronous pulse PRS selected.
};

/// DT (Dead Time) Fault Actions.
SL_ENUM(sl_hal_timer_dti_fault_action_t) {
  SL_HAL_TIMER_DTI_FAULT_ACTION_NONE     = _TIMER_DTFCFG_DTFA_NONE,        ///< No action on fault.
  SL_HAL_TIMER_DTI_FAULT_ACTION_INACTIVE = _TIMER_DTFCFG_DTFA_INACTIVE,    ///< Set outputs inactive.
  SL_HAL_TIMER_DTI_FAULT_ACTION_CLEAR    = _TIMER_DTFCFG_DTFA_CLEAR,       ///< Clear outputs.
  SL_HAL_TIMER_DTI_FAULT_ACTION_TRISTATE = _TIMER_DTFCFG_DTFA_TRISTATE     ///< Tristate outputs.
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/
/// TIMER initialization structure.
typedef struct {
  /// Counting mode.
  sl_hal_timer_mode_t           count_mode;

  /// Clock selection.
  sl_hal_timer_clock_select_t   clock_select;

  /// Prescaling factor, if HFPER / HFPERB clock used.
  sl_hal_timer_prescaler_t      prescaler;

  /// Action on rising input edge.
  sl_hal_timer_input_edge_action_t   input_rise_action;

  /// Action on falling input edge.
  sl_hal_timer_input_edge_action_t   input_fall_action;

  /// Timer can be start/stop/reload by other timers.
  bool                      input_enable_sync;

  /// Determines if only counting up or down once.
  bool                      count_one_shot;

  /// Select X2 or X4 quadrature decode mode (if used)
  bool                      quad_mode_x4;

  /// Counter shall keep running during debug halt.
  bool                      debug_run;

  /// DMA request clear on active.
  bool                      dma_clear_on_active;

  /// Enable retiming of PWM output.
  bool                      enable_retime;

  /// Disable ability of timer to start/stop/reload other timers that have their
  ///  SYNC bit set.
  bool                      output_disable_sync;

  /// ATI (Always Track Inputs) makes CCPOL always track
  /// the polarity of the inputs.
  bool                      always_track_input;

  /// When enabled, compare output is set to COIST value on a Reload-Start event.
  bool                      reload_start_default;

  /// 2x Count mode, counter increments/decrements by 2, meant for PWM mode.
  bool                      count_2x;

#if defined(_TIMER_CFG_UPDATEMODE_MASK)
  /// When enabled, writes from CCB to CC registers must be performed using software commands.
  bool enable_update_mode;
#endif
} sl_hal_timer_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_timer_init_t sl_hal_timer_config_t;
/** @endcond */

/// Default configuration for TIMER initialization structure.
#if defined(_TIMER_CFG_UPDATEMODE_MASK)
#define SL_HAL_TIMER_INIT_DEFAULT                                                                                  \
  {                                                                                                                \
    (sl_hal_timer_mode_t)_TIMER_CFG_MODE_DEFAULT,            /* Up-counting. */                                    \
    (sl_hal_timer_clock_select_t)_TIMER_CFG_CLKSEL_DEFAULT,  /* Select HFPER / HFPERB clock. */                    \
    (sl_hal_timer_prescaler_t)_TIMER_CFG_PRESC_DEFAULT,      /* No prescaling. */                                  \
    (sl_hal_timer_input_edge_action_t)_TIMER_CTRL_RISEA_DEFAULT,  /* No action on rising input edge. */            \
    (sl_hal_timer_input_edge_action_t)_TIMER_CTRL_FALLA_DEFAULT,  /* No action on falling input edge. */           \
    (bool)_TIMER_CFG_SYNC_DEFAULT,                       /* Not started/stopped/reloaded by other timers. */       \
    (bool)_TIMER_CFG_OSMEN_DEFAULT,                      /* Disable one shot. */                                   \
    (bool)_TIMER_CFG_QDM_DEFAULT,                        /* Select default X2 quadrature decode mode (if used). */ \
    (bool)_TIMER_CFG_DEBUGRUN_DEFAULT,                   /* Stop counter during debug halt. */                     \
    (bool)_TIMER_CFG_DMACLRACT_DEFAULT,                  /* Stop counter during debug halt. */                     \
    (bool)_TIMER_CFG_RETIMEEN_DEFAULT,                   /* Disable retime. */                                     \
    (bool)_TIMER_CFG_DISSYNCOUT_DEFAULT,                 /* Disable ability to start/stop/reload other timers. */  \
    (bool)_TIMER_CFG_ATI_DEFAULT,                        /* No ATI. */                                             \
    (bool)_TIMER_CFG_RSSCOIST_DEFAULT,                   /* RSSCOIST is disabled. */                               \
    (bool)_TIMER_CTRL_X2CNT_DEFAULT,                     /* Not 2x count mode. */                                  \
    (bool)_TIMER_CFG_UPDATEMODE_DEFAULT                  /* Automatic write from CCB to CC registers. */           \
  }
#else
#define SL_HAL_TIMER_INIT_DEFAULT                                                                                  \
  {                                                                                                                \
    (sl_hal_timer_mode_t)_TIMER_CFG_MODE_DEFAULT,            /* Up-counting. */                                    \
    (sl_hal_timer_clock_select_t)_TIMER_CFG_CLKSEL_DEFAULT,  /* Select HFPER / HFPERB clock. */                    \
    (sl_hal_timer_prescaler_t)_TIMER_CFG_PRESC_DEFAULT,      /* No prescaling. */                                  \
    (sl_hal_timer_input_edge_action_t)_TIMER_CTRL_RISEA_DEFAULT,  /* No action on rising input edge. */            \
    (sl_hal_timer_input_edge_action_t)_TIMER_CTRL_FALLA_DEFAULT,  /* No action on falling input edge. */           \
    (bool)_TIMER_CFG_SYNC_DEFAULT,                       /* Not started/stopped/reloaded by other timers. */       \
    (bool)_TIMER_CFG_OSMEN_DEFAULT,                      /* Disable one shot. */                                   \
    (bool)_TIMER_CFG_QDM_DEFAULT,                        /* Select default X2 quadrature decode mode (if used). */ \
    (bool)_TIMER_CFG_DEBUGRUN_DEFAULT,                   /* Stop counter during debug halt. */                     \
    (bool)_TIMER_CFG_DMACLRACT_DEFAULT,                  /* Stop counter during debug halt. */                     \
    (bool)_TIMER_CFG_RETIMEEN_DEFAULT,                   /* Disable retime. */                                     \
    (bool)_TIMER_CFG_DISSYNCOUT_DEFAULT,                 /* Disable ability to start/stop/reload other timers. */  \
    (bool)_TIMER_CFG_ATI_DEFAULT,                        /* No ATI. */                                             \
    (bool)_TIMER_CFG_RSSCOIST_DEFAULT,                   /* RSSCOIST is disabled. */                               \
    (bool)_TIMER_CTRL_X2CNT_DEFAULT                      /* Not 2x count mode. */                                  \
  }
#endif // defined(_TIMER_CFG_UPDATEMODE_MASK)

/// TIMER compare/capture initialization structure.
typedef struct {
#if defined(_TIMER_CC_PHASE_MASK)
  /// Phase value determines the value at which CC output is set to 1 in PWM
  /// upcount mode.
  uint32_t                            phase;
#endif
#if defined(_TIMER_CC_DITHER_MASK)
  /// Dither value determines the number of PWM pulses to add the dithering to,
  /// out of 2^DITHERWIDTH pulses.
  uint32_t                            dither;
#endif
  /// Compare/capture channel mode.
  sl_hal_timer_channel_mode_t          channel_mode;

  /// Select Compare/Capture channel input.
  sl_hal_timer_channel_input_type_t    input_type;

  /// Counter match output action.
  sl_hal_timer_channel_output_action_t compare_match_output_action;

  /// Counter underflow output action.
  sl_hal_timer_channel_output_action_t compare_underflow_output_action;

  /// Counter overflow output action.
  sl_hal_timer_channel_output_action_t compare_overflow_output_action;

  /// Input capture event control.
  sl_hal_timer_channel_input_event_t   input_capture_event;

  /// Input capture edge select.
  sl_hal_timer_channel_input_edge_t    input_capture_edge;

  /// Sets the watermark level of generation of fifo interrupt and DMA requests.
  uint8_t                          input_watermark_level;

  /**
   * Compare output initial state. Only used in Output Compare and PWM mode.
   * When true, the compare/PWM output is set high when the counter is
   * disabled. When counting resumes, this value will represent the initial
   * value for the compare/PWM output. If the bit is cleared, the output
   * will be cleared when the counter is disabled.
   */
  bool                             output_initial_state;

  /**
   * PRS output configuration. PRS output from a timer can either be a
   * pulse output or a level output that follows the CC out value.
   * if set, prs outputs a level that follows cc out value.
   */
  bool                             prs_output;

  /// Enable digital filter.
  bool                             filter;

  /// Invert output from compare/capture channel.
  bool                             output_invert;
} sl_hal_timer_channel_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_timer_channel_init_t sl_hal_timer_channel_config_t;
/** @endcond */

#if defined(_TIMER_CC_PHASE_MASK) && defined(_TIMER_CC_DITHER_MASK)
/// Default configuration for TIMER compare/capture initialization structure.
#define SL_HAL_TIMER_CHANNEL_INIT_DEFAULT                                                                            \
  {                                                                                                                  \
    (uint32_t)_TIMER_CC_PHASE_PHASE_DEFAULT,                         /* No phase value. */                           \
    (uint32_t)_TIMER_CC_DITHER_DITHER_DEFAULT,                       /* No dither value. */                          \
    (sl_hal_timer_channel_mode_t)_TIMER_CC_CFG_MODE_DEFAULT,             /* Disable compare/capture channel. */      \
    (sl_hal_timer_channel_input_type_t)_TIMER_CC_CFG_INSEL_DEFAULT,      /* Select PIN. */                           \
    (sl_hal_timer_channel_output_action_t)_TIMER_CC_CTRL_CMOA_DEFAULT,   /* No action on match. */                   \
    (sl_hal_timer_channel_output_action_t)_TIMER_CC_CTRL_CUFOA_DEFAULT,  /* No action on underflow. */               \
    (sl_hal_timer_channel_output_action_t)_TIMER_CC_CTRL_COFOA_DEFAULT,  /* No action on overflow. */                \
    (sl_hal_timer_channel_input_event_t)_TIMER_CC_CTRL_ICEVCTRL_DEFAULT, /* Event on every capture. */               \
    (sl_hal_timer_channel_input_edge_t)_TIMER_CC_CTRL_ICEDGE_DEFAULT,    /* Use default PRS output configuration. */ \
    (uint8_t)_TIMER_CC_CFG_ICFWL_DEFAULT,                            /* Disable watermark level check. */            \
    (bool)_TIMER_CC_CFG_COIST_DEFAULT,                               /* Clear output when counter disabled. */       \
    (bool)_TIMER_CC_CFG_PRSCONF_DEFAULT,                             /* Use default PRS output configuration. */     \
    (bool)_TIMER_CC_CFG_FILT_DEFAULT,                                /* Disable filter. */                           \
    (bool)_TIMER_CC_CTRL_OUTINV_DEFAULT,                             /* Do not invert output. */                     \
  }

/// PWM mode configuration for TIMER compare/capture initialization structure.
#define SL_HAL_TIMER_CHANNEL_PWM_INIT_DEFAULT                                                                         \
  {                                                                                                                   \
    (uint32_t)_TIMER_CC_PHASE_PHASE_DEFAULT,                         /* No phase value. */                            \
    (uint32_t)_TIMER_CC_DITHER_DITHER_DEFAULT,                       /* No dither value. */                           \
    SL_HAL_TIMER_CHANNEL_MODE_PWM,                                       /* Set PWM mode. */                          \
    (sl_hal_timer_channel_input_type_t)_TIMER_CC_CFG_INSEL_DEFAULT,      /* Select PIN. */                            \
    SL_HAL_TIMER_CHANNEL_OUTPUT_ACTION_TOGGLE,                           /* Toggle on match. */                       \
    (sl_hal_timer_channel_output_action_t)_TIMER_CC_CTRL_CUFOA_DEFAULT,  /* No action on underflow. */                \
    (sl_hal_timer_channel_output_action_t)_TIMER_CC_CTRL_COFOA_DEFAULT,  /* No action on overflow. */                 \
    (sl_hal_timer_channel_input_event_t)_TIMER_CC_CTRL_ICEVCTRL_DEFAULT, /* Event on every capture. */                \
    (sl_hal_timer_channel_input_edge_t)_TIMER_CC_CTRL_ICEDGE_DEFAULT,     /* Use default PRS output configuration. */ \
    (uint8_t)_TIMER_CC_CFG_ICFWL_DEFAULT,                            /* Disable watermark level check. */             \
    (bool)_TIMER_CC_CFG_COIST_DEFAULT,                               /* Clear output when counter disabled. */        \
    (bool)_TIMER_CC_CFG_PRSCONF_DEFAULT,                             /* Use default PRS output configuration. */      \
    (bool)_TIMER_CC_CFG_FILT_DEFAULT,                                /* Disable filter. */                            \
    false,                                                           /* Do not invert output. */                      \
  }
#else
/// Default configuration for TIMER compare/capture initialization structure.
#define SL_HAL_TIMER_CHANNEL_INIT_DEFAULT                                                                            \
  {                                                                                                                  \
    (sl_hal_timer_channel_mode_t)_TIMER_CC_CFG_MODE_DEFAULT,             /* Disable compare/capture channel. */      \
    (sl_hal_timer_channel_input_type_t)_TIMER_CC_CFG_INSEL_DEFAULT,      /* Select PIN. */                           \
    (sl_hal_timer_channel_output_action_t)_TIMER_CC_CTRL_CMOA_DEFAULT,   /* No action on match. */                   \
    (sl_hal_timer_channel_output_action_t)_TIMER_CC_CTRL_CUFOA_DEFAULT,  /* No action on underflow. */               \
    (sl_hal_timer_channel_output_action_t)_TIMER_CC_CTRL_COFOA_DEFAULT,  /* No action on overflow. */                \
    (sl_hal_timer_channel_input_event_t)_TIMER_CC_CTRL_ICEVCTRL_DEFAULT, /* Event on every capture. */               \
    (sl_hal_timer_channel_input_edge_t)_TIMER_CC_CTRL_ICEDGE_DEFAULT,    /* Use default PRS output configuration. */ \
    (uint8_t)_TIMER_CC_CFG_ICFWL_DEFAULT,                            /* Disable watermark level check. */            \
    (bool)_TIMER_CC_CFG_COIST_DEFAULT,                               /* Clear output when counter disabled. */       \
    (bool)_TIMER_CC_CFG_PRSCONF_DEFAULT,                             /* Use default PRS output configuration. */     \
    (bool)_TIMER_CC_CFG_FILT_DEFAULT,                                /* Disable filter. */                           \
    (bool)_TIMER_CC_CTRL_OUTINV_DEFAULT,                             /* Do not invert output. */                     \
  }

/// PWM mode configuration for TIMER compare/capture initialization structure.
#define SL_HAL_TIMER_CHANNEL_PWM_INIT_DEFAULT                                                                         \
  {                                                                                                                   \
    SL_HAL_TIMER_CHANNEL_MODE_PWM,                                       /* Set PWM mode. */                          \
    (sl_hal_timer_channel_input_type_t)_TIMER_CC_CFG_INSEL_DEFAULT,      /* Select PIN. */                            \
    SL_HAL_TIMER_CHANNEL_OUTPUT_ACTION_TOGGLE,                           /* Toggle on match. */                       \
    (sl_hal_timer_channel_output_action_t)_TIMER_CC_CTRL_CUFOA_DEFAULT,  /* No action on underflow. */                \
    (sl_hal_timer_channel_output_action_t)_TIMER_CC_CTRL_COFOA_DEFAULT,  /* No action on overflow. */                 \
    (sl_hal_timer_channel_input_event_t)_TIMER_CC_CTRL_ICEVCTRL_DEFAULT, /* Event on every capture. */                \
    (sl_hal_timer_channel_input_edge_t)_TIMER_CC_CTRL_ICEDGE_DEFAULT,     /* Use default PRS output configuration. */ \
    (uint8_t)_TIMER_CC_CFG_ICFWL_DEFAULT,                            /* Disable watermark level check. */             \
    (bool)_TIMER_CC_CFG_COIST_DEFAULT,                               /* Clear output when counter disabled. */        \
    (bool)_TIMER_CC_CFG_PRSCONF_DEFAULT,                             /* Use default PRS output configuration. */      \
    (bool)_TIMER_CC_CFG_FILT_DEFAULT,                                /* Disable filter. */                            \
    false,                                                           /* Do not invert output. */                      \
  }
#endif // defined(_TIMER_CC_PHASE_MASK) && defined(_TIMER_CC_DITHER_MASK)

/// TIMER Dead Time Insertion (DTI) initialization structure.
typedef struct {
  /// Enable Automatic Start-up functionality (when debugger exits)
  bool                          auto_restart;

  /// Allows DTI channel 0 to keep running even when timer is stopped.
  bool                          ch0_always_running;

  /// When timers stops, DTI block outputs go to safe state.
  bool                          safe_state;

  /// Enable/disable PRS as DTI input.
  bool                          enable_prs_input;

  /// DTI Complementary Output Invert.
  bool                          invert_complementary_output;

  /// DTI Output Polarity.
  bool                          output_polarity;

#if defined(_TIMER_DTFCFG_DTEM23FEN_MASK)
  /// Enable EM2/3 entry as a fault source.
  bool                          enable_fault_source_em23;
#endif

  /// Enable core lockup as a fault source.
  bool                          enable_fault_source_core_loockup;

  /// Enable debugger as a fault source.
  bool                          enable_fault_source_debugger;

  /// Enable PRS fault source 0 (@p faultSourcePrsSel0)
  bool                          enable_fault_source_prs_0;

  /// Enable PRS fault source 1 (@p faultSourcePrsSel1)
  bool                          enable_fault_source_prs_1;

  /// DTI prescaling factor, if HFPER / HFPERB clock used.
  sl_hal_timer_prescaler_t          prescaler;

  /// DTI Rise Time
  uint8_t                       rise_time;

  /// DTI Fall Time
  uint8_t                       fall_time;

  /**  DTI outputs enable bit mask, consisting of one bit per DTI
      output signal, i.e., CC0, CC1, CC2, CDTI0, CDTI1, and CDTI2.
      This value should consist of one or more TIMER_DTOGEN_DTOGnnnEN flags
      (defined in \<part_name\>_timer.h) OR'ed together. */
  uint32_t                      output_gen_mask;

  /// Fault Action */
  sl_hal_timer_dti_fault_action_t   fault_action;
} sl_hal_timer_dti_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_timer_dti_init_t sl_hal_timer_dti_config_t;
/** @endcond */

#if defined(_TIMER_DTFCFG_DTEM23FEN_MASK)
/// Default configuration for TIMER DTI initialization structure. 
#define SL_HAL_TIMER_DTI_INIT_DEFAULT                                                                             \
  {                                                                                                               \
    (bool)_TIMER_DTCFG_DTDAS_DEFAULT,                        /* No auto restart when debugger exits. */           \
    (bool)_TIMER_DTCFG_DTAR_DEFAULT,                         /* Stop DTI output on channel 0 when timer stops. */ \
    (bool)_TIMER_DTCFG_DTFATS_DEFAULT,                       /* Disable safe state when timer stops. */           \
    (bool)_TIMER_DTCFG_DTPRSEN_DEFAULT,                      /* No PRS source selected. */                        \
    (bool)_TIMER_DTCTRL_DTCINV_DEFAULT,                      /* CDTI[0|1|2] outputs are not inverted. */          \
    (bool)_TIMER_DTCTRL_DTIPOL_DEFAULT,                      /* CC[0|1|2] outputs are active high. */             \
    (bool)_TIMER_DTFCFG_DTEM23FEN_DEFAULT,                   /* Disable EM23 entry as fault source. */            \
    (bool)_TIMER_DTFCFG_DTLOCKUPFEN_DEFAULT,                 /* Disable core lockup as fault source. */           \
    (bool)_TIMER_DTFCFG_DTDBGFEN_DEFAULT,                    /* Disable debugger as fault source. */              \
    (bool)_TIMER_DTFCFG_DTPRS0FEN_DEFAULT,                   /* Disable PRS fault source 0. */                    \
    (bool)_TIMER_DTFCFG_DTPRS1FEN_DEFAULT,                   /* Disable PRS fault source 1. */                    \
    (sl_hal_timer_prescaler_t)_TIMER_DTTIMECFG_DTPRESC_DEFAULT,  /* No prescaling.  */                            \
    (uint8_t)_TIMER_DTTIMECFG_DTRISET_DEFAULT,               /* No rise time. */                                  \
    (uint8_t)_TIMER_DTTIMECFG_DTFALLT_DEFAULT,               /* No rise time. */                                  \
    (uint32_t)0,                                             /* Disable output generation for all channels. */    \
    (sl_hal_timer_dti_fault_action_t)_TIMER_DTFCFG_DTFA_DEFAULT, /* No fault action. */                           \
  }
#else
/// Default configuration for TIMER DTI initialization structure.
#define SL_HAL_TIMER_DTI_INIT_DEFAULT                                                                             \
  {                                                                                                               \
    (bool)_TIMER_DTCFG_DTDAS_DEFAULT,                        /* No auto restart when debugger exits. */           \
    (bool)_TIMER_DTCFG_DTAR_DEFAULT,                         /* Stop DTI output on channel 0 when timer stops. */ \
    (bool)_TIMER_DTCFG_DTFATS_DEFAULT,                       /* Disable safe state when timer stops. */           \
    (bool)_TIMER_DTCFG_DTPRSEN_DEFAULT,                      /* No PRS source selected. */                        \
    (bool)_TIMER_DTCTRL_DTCINV_DEFAULT,                      /* CDTI[0|1|2] outputs are not inverted. */          \
    (bool)_TIMER_DTCTRL_DTIPOL_DEFAULT,                      /* CC[0|1|2] outputs are active high. */             \
    (bool)_TIMER_DTFCFG_DTLOCKUPFEN_DEFAULT,                 /* Disable core lockup as fault source. */           \
    (bool)_TIMER_DTFCFG_DTDBGFEN_DEFAULT,                    /* Disable debugger as fault source. */              \
    (bool)_TIMER_DTFCFG_DTPRS0FEN_DEFAULT,                   /* Disable PRS fault source 0. */                    \
    (bool)_TIMER_DTFCFG_DTPRS1FEN_DEFAULT,                   /* Disable PRS fault source 1. */                    \
    (sl_hal_timer_prescaler_t)_TIMER_DTTIMECFG_DTPRESC_DEFAULT,  /* No prescaling.  */                            \
    (uint8_t)_TIMER_DTTIMECFG_DTRISET_DEFAULT,               /* No rise time. */                                  \
    (uint8_t)_TIMER_DTTIMECFG_DTFALLT_DEFAULT,               /* No rise time. */                                  \
    (uint32_t)0,                                             /* Disable output generation for all channels. */    \
    (sl_hal_timer_dti_fault_action_t)_TIMER_DTFCFG_DTFA_DEFAULT, /* No fault action. */                           \
  }
#endif

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Alias for deprecated macro names used for backward compatibility purposes.
#define SL_HAL_TIMER_CONFIG_DEFAULT           SL_HAL_TIMER_INIT_DEFAULT
#define SL_HAL_TIMER_CHANNEL_CONFIG_DEFAULT   SL_HAL_TIMER_CHANNEL_INIT_DEFAULT
#define SL_HAL_TIMER_CHANNEL_CONFIG_PWM       SL_HAL_TIMER_CHANNEL_PWM_INIT_DEFAULT
#define SL_HAL_TIMER_DTI_CONFIG_DEFAULT       SL_HAL_TIMER_DTI_INIT_DEFAULT
/** @endcond */

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initialize TIMER.
 *
 * @details
 *   Notice that the counter top must be configured separately with, for instance
 *   sl_hal_timer_set_top(). In addition, compare/capture and dead-time insertion
 *   initialization must be initialized separately if used, which should probably
 *   be done prior to using this function if configuring the TIMER to
 *   start when initialization is completed.
 *
 * @param[in] timer
 *   A pointer to the TIMER peripheral register block.
 *
 * @param[in] init
 *   A pointer to the TIMER initialization structure used to configure TIMER.
 ******************************************************************************/
void sl_hal_timer_init(TIMER_TypeDef *timer,
                       const sl_hal_timer_init_t *init);

/***************************************************************************//**
 * @brief
 *   Initialize the TIMER compare/capture channel.
 *
 * @details
 *   Notice that if operating the channel in compare mode, the OC and OCB register
 *   must be set separately, as required.
 *
 * @note
 *   This function temporarily disables the TIMER module during execution.
 *   All initialized channels will be non-functional while the module is disabled
 *   but will resume normal operation when the TIMER is re-enabled at the end of
 *   this function.
 *
 * @param[in] timer
 *   A pointer to the TIMER peripheral register block.
 *
 * @param[in] channel
 *   A compare/capture channel to initialize for.
 *
 * @param[in] init
 *   A pointer to the TIMER initialization structure.
 ******************************************************************************/
void sl_hal_timer_channel_init(TIMER_TypeDef *timer,
                               uint8_t channel,
                               const sl_hal_timer_channel_init_t *init);

/***************************************************************************//**
 * @brief
 *   Initialize the TIMER DTI unit.
 *
 * @note
 *   This function must be called before enabling the timer.
 *
 * @note
 *   This function temporarily disables the TIMER module during execution.
 *   All initialized channels will be non-functional while the module is disabled
 *   but will resume normal operation when the TIMER is re-enabled.
 *
 * @param[in] timer
 *   A pointer to the TIMER peripheral register block.
 *
 * @param[in] init
 *   A pointer to the TIMER DTI initialization structure.
 ******************************************************************************/
void sl_hal_timer_dti_init(TIMER_TypeDef *timer,
                           const sl_hal_timer_dti_init_t *init);

/***************************************************************************//**
 * @brief
 *   Reset the TIMER to the same state that it was in after a hardware reset.
 *
 * @note
 *   The ROUTE register is NOT reset by this function to allow for
 *   a centralized setup of this feature.
 *
 * @param[in] timer
 *   A pointer to the TIMER peripheral register block.
 ******************************************************************************/
void sl_hal_timer_reset(TIMER_TypeDef *timer);

/***************************************************************************//**
 * @brief
 *   Wait for ongoing sync of register(s) to the low-frequency domain to complete.
 *
 * @param[in]
 *   timer Pointer to the timer peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_timer_wait_sync(TIMER_TypeDef *timer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

  // Wait for any pending previous write operation to have been completed
  // in the low-frequency domain.
  while ((timer->EN & _TIMER_EN_EN_MASK)
         && (timer->STATUS & _TIMER_STATUS_SYNCBUSY_MASK)) {
    // Wait for the update to finish
  }
}

/***************************************************************************//**
 * @brief
 *   Wait for disabling to finish.
 *
 * @param[in]
 *   timer Pointer to the TIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_timer_wait_ready(TIMER_TypeDef *timer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

#if defined(_TIMER_EN_DISABLING_MASK)
  while (timer->EN & _TIMER_EN_DISABLING_MASK) {
    // Wait for disabling to finish
  }
#endif
}

/***************************************************************************//**
 * @brief
 *   Enable TIMER.
 *
 * @param[in]
 *   timer Pointer to the TIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_timer_enable(TIMER_TypeDef *timer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

  if (timer->EN == TIMER_EN_EN) {
    return;
  }
  // Enable peripheral.
  timer->EN_SET = TIMER_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Disable TIMER.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_timer_disable(TIMER_TypeDef *timer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

  if (timer->EN != TIMER_EN_EN) {
    return;
  }
  // Disable TIMER
  timer->EN_CLR = TIMER_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Start TIMER.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_timer_start(TIMER_TypeDef *timer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure module is enabled
  EFM_ASSERT(timer->EN & _TIMER_EN_EN_MASK);

  sl_hal_timer_wait_sync(timer);
  timer->CMD = TIMER_CMD_START;
}

/***************************************************************************//**
 * @brief
 *   Stop TIMER.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_timer_stop(TIMER_TypeDef *timer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure module is enabled
  EFM_ASSERT(timer->EN & _TIMER_EN_EN_MASK);

  sl_hal_timer_wait_sync(timer);
  timer->CMD = TIMER_CMD_STOP;
}

#if defined(_TIMER_CFG_UPDATEMODE_MASK)
/***************************************************************************//**
 * @brief
 *   Sends software command to update channel registers in the following
 *   update event using software.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 *  @note
 *   If timer enable_update_mode is false, channel registers are updated
 *   automatically, and this API will do nothing. This function is only
 *   useful when enable_update_mode is true.
 ******************************************************************************/
__INLINE void sl_hal_timer_channel_update_registers(TIMER_TypeDef *timer)
{
  // If UPDATEMODE is enabled send software command to write from CCB
  // to CC registers.
  if (timer->CFG & _TIMER_CFG_UPDATEMODE_MASK) {
    timer->CMD = TIMER_CMD_UPDATECMD;
  }
}
#endif

/***************************************************************************//**
 * @brief
 *   Get capture value for the capture channel.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] channel
 *   Capture channel to access.
 *
 * @return
 *   Current capture value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_timer_channel_get_capture(TIMER_TypeDef *timer,
                                                   uint8_t channel)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure channel number is valid.
  EFM_ASSERT(channel < SL_HAL_TIMER_CHANNEL_NUM(timer));

  sl_hal_timer_wait_sync(timer);
  return timer->CC[channel].ICF;
}

/***************************************************************************//**
 * @brief
 *   Get the buffered capture value for capture channel.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] channel
 *   Capture channel to access.
 *
 * @return
 *   Current buffered capture value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_timer_channel_get_capture_buffer(TIMER_TypeDef *timer,
                                                          uint8_t channel)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure channel number is valid.
  EFM_ASSERT(channel < SL_HAL_TIMER_CHANNEL_NUM(timer));

  sl_hal_timer_wait_sync(timer);
  return timer->CC[channel].ICOF;
}

/***************************************************************************//**
 * @brief
 *   Get compare value for the compare channel.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] channel
 *   Compare channel to access.
 *
 * @return
 *   Current compare value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_timer_channel_get_compare(TIMER_TypeDef *timer,
                                                   uint8_t channel)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure channel number is valid.
  EFM_ASSERT(channel < SL_HAL_TIMER_CHANNEL_NUM(timer));

  sl_hal_timer_wait_sync(timer);
  return timer->CC[channel].OC;
}

/***************************************************************************//**
 * @brief
 *   Get the buffered compare value for compare channel.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] channel
 *   Compare channel to access.
 *
 * @return
 *   Current buffered compare value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_timer_channel_get_compare_buffer(TIMER_TypeDef *timer,
                                                          uint8_t channel)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure channel number is valid.
  EFM_ASSERT(channel < SL_HAL_TIMER_CHANNEL_NUM(timer));

  sl_hal_timer_wait_sync(timer);
  return timer->CC[channel].OCB;
}

/***************************************************************************//**
 * @brief
 *   Set the compare value buffer for the compare/capture channel when operating in
 *   compare or PWM mode.
 *
 * @details
 *   The compare value buffer holds the value which will be written to
 *   TIMERn_CCx_CCV on an update event if the buffer has been updated since
 *   the last event.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] channel
 *   Compare/capture channel to access.
 *
 * @param[in] value
 *   Value to set in compare value buffer register.
 ******************************************************************************/
__INLINE void sl_hal_timer_channel_set_compare_buffer(TIMER_TypeDef *timer,
                                                      uint8_t channel,
                                                      uint32_t value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure channel number is valid.
  EFM_ASSERT(channel < SL_HAL_TIMER_CHANNEL_NUM(timer));
  // Make sure counter value is valid.
  EFM_ASSERT(value <= SL_HAL_TIMER_MAX_COUNT(timer));
  // Make sure module is enabled
  EFM_ASSERT(timer->EN & _TIMER_EN_EN_MASK);

  sl_hal_timer_wait_sync(timer);
  timer->CC[channel].OCB = value;
}

/***************************************************************************//**
 * @brief
 *   Set the compare value for compare/capture channel when operating in compare
 *   or PWM mode.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] channel
 *   Compare/capture channel to access.
 *
 * @param[in] value
 *   Value to set in compare value register.
 ******************************************************************************/
__INLINE void sl_hal_timer_channel_set_compare(TIMER_TypeDef *timer,
                                               uint8_t channel,
                                               uint32_t value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure channel number is valid.
  EFM_ASSERT(channel < SL_HAL_TIMER_CHANNEL_NUM(timer));
  // Make sure counter value is valid.
  EFM_ASSERT(value <= SL_HAL_TIMER_MAX_COUNT(timer));
  // Make sure module is enabled
  EFM_ASSERT(timer->EN & _TIMER_EN_EN_MASK);

  sl_hal_timer_wait_sync(timer);
  timer->CC[channel].OC = value;
}

#if defined(_TIMER_CC_PHASE_MASK)
/***************************************************************************//**
 * @brief
 *   Update timer phase value.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] channel
 *   A compare/capture channel to set phase for.
 *
 * @param[in] phase
 *   Phase value that determines the value at which CC output is set to
 *   1 in PWM upcout mode.
 ******************************************************************************/
__INLINE void sl_hal_timer_channel_set_phase(TIMER_TypeDef *timer,
                                             uint8_t channel,
                                             uint32_t phase)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure channel number is valid.
  EFM_ASSERT(channel < SL_HAL_TIMER_CHANNEL_NUM(timer));
  // Make sure phase value is valid.
  EFM_ASSERT(phase <= SL_HAL_TIMER_MAX_COUNT(timer));
  // Make sure module is enabled
  EFM_ASSERT(timer->EN & _TIMER_EN_EN_MASK);

  // Wait for ongoing sync of register
  sl_hal_timer_wait_sync(timer);
  timer->CC[channel].PHASE = phase;
}

/***************************************************************************//**
 * @brief
 *   Get timer phase value.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] channel
 *   A compare/capture channel to get phase value from.
 *
 *  @return
 *   TIMER CC phase value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_timer_channel_get_phase(TIMER_TypeDef *timer,
                                                 uint8_t channel)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure channel number is valid.
  EFM_ASSERT(channel < SL_HAL_TIMER_CHANNEL_NUM(timer));

  sl_hal_timer_wait_sync(timer);
  return timer->CC[channel].PHASE;
}

/***************************************************************************//**
 * @brief
 *   Update timer phase buffer value.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] channel
 *   A compare/capture channel to set phase for.
 *
 * @param[in] phase
 *   Phase value to write to the phase buffer. The phase buffer value is
 *   written to the phase register automatically or via software command using
 *   @ref sl_hal_timer_channel_update_registers.
 ******************************************************************************/
__INLINE void sl_hal_timer_channel_set_phase_buffer(TIMER_TypeDef *timer,
                                                    uint8_t channel,
                                                    uint32_t phase)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure channel number is valid.
  EFM_ASSERT(channel < SL_HAL_TIMER_CHANNEL_NUM(timer));
  // Make sure phase value is valid.
  EFM_ASSERT(phase <= SL_HAL_TIMER_MAX_COUNT(timer));
  // Make sure module is enabled
  EFM_ASSERT(timer->EN & _TIMER_EN_EN_MASK);

  // Wait for ongoing sync of register
  sl_hal_timer_wait_sync(timer);
  timer->CC[channel].PHASEB = phase;
}

/***************************************************************************//**
 * @brief
 *   Get timer phase buffer value.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] channel
 *   A compare/capture channel to get phase value from.
 *
 *  @return
 *   TIMER CC phase buffer value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_timer_channel_get_phase_buffer(TIMER_TypeDef *timer,
                                                        uint8_t channel)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure channel number is valid.
  EFM_ASSERT(channel < SL_HAL_TIMER_CHANNEL_NUM(timer));

  sl_hal_timer_wait_sync(timer);
  return timer->CC[channel].PHASEB;
}

#endif // defined(_TIMER_CC_PHASE_MASK)

#if defined(_TIMER_CC_DITHER_MASK)
/***************************************************************************//**
 * @brief
 *   Update timer dither value.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] channel
 *   A compare/capture channel to set dither for.
 *
 * @param[in] dither
 *   Dither value that determines the number of PWM pulses to add the
 *   dithering to, out of 2^DITHERWIDTH pulses.
 ******************************************************************************/
__INLINE void sl_hal_timer_channel_set_dither(TIMER_TypeDef *timer,
                                              uint8_t channel,
                                              uint8_t dither)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure channel number is valid.
  EFM_ASSERT(channel < SL_HAL_TIMER_CHANNEL_NUM(timer));
  // Make sure phase value is valid.
  EFM_ASSERT(dither <= _TIMER_CC_DITHER_DITHER_MASK);
  // Make sure module is enabled
  EFM_ASSERT(timer->EN & _TIMER_EN_EN_MASK);

  // Wait for ongoing sync of register
  sl_hal_timer_wait_sync(timer);
  timer->CC[channel].DITHER = dither;
}

/***************************************************************************//**
 * @brief
 *   Get timer dither value.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] channel
 *   A compare/capture channel to get dither value from.
 *
 *  @return
 *   TIMER CC dither value.
 ******************************************************************************/
__INLINE uint8_t sl_hal_timer_channel_get_dither(TIMER_TypeDef *timer,
                                                 uint8_t channel)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure channel number is valid.
  EFM_ASSERT(channel < SL_HAL_TIMER_CHANNEL_NUM(timer));

  sl_hal_timer_wait_sync(timer);
  return (uint8_t)(timer->CC[channel].DITHER);
}

/***************************************************************************//**
 * @brief
 *   Update timer dither buffer value.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] channel
 *   A compare/capture channel to set dither buffer value for.
 *
 * @param[in] dither
 *   The dither buffer value is written to the dither register automatically
 *   or via software command using @ref sl_hal_timer_channel_update_registers.
 ******************************************************************************/
__INLINE void sl_hal_timer_channel_set_dither_buffer(TIMER_TypeDef *timer,
                                                     uint8_t channel,
                                                     uint8_t dither)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure channel number is valid.
  EFM_ASSERT(channel < SL_HAL_TIMER_CHANNEL_NUM(timer));
  // Make sure phase value is valid.
  EFM_ASSERT(dither <= _TIMER_CC_DITHER_DITHER_MASK);
  // Make sure module is enabled
  EFM_ASSERT(timer->EN & _TIMER_EN_EN_MASK);

  // Wait for ongoing sync of register
  sl_hal_timer_wait_sync(timer);
  timer->CC[channel].DITHERB = dither;
  sl_hal_timer_wait_sync(timer);
}

/***************************************************************************//**
 * @brief
 *   Get timer dither buffer value.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] channel
 *   A compare/capture channel to get dither buffer value from.
 *
 *  @return
 *   TIMER CC buffer dither value.
 ******************************************************************************/
__INLINE uint8_t sl_hal_timer_channel_get_dither_buffer(TIMER_TypeDef *timer,
                                                        uint8_t channel)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure channel number is valid.
  EFM_ASSERT(channel < SL_HAL_TIMER_CHANNEL_NUM(timer));

  sl_hal_timer_wait_sync(timer);
  return (uint8_t)(timer->CC[channel].DITHERB);
}

#endif // defined(_TIMER_CC_DITHER_MASK)

/***************************************************************************//**
 * @brief
 *   Get the TIMER counter value.
 *
 * @param[in] timer
 *   Pointer to TIMER peripheral register block.
 *
 * @return
 *   Current TIMER counter value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_timer_get_counter(TIMER_TypeDef *timer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

  sl_hal_timer_wait_sync(timer);
  return timer->CNT;
}

/***************************************************************************//**
 * @brief
 *   Set the TIMER counter value.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] value
 *   Value to set counter to.
 ******************************************************************************/
__INLINE void sl_hal_timer_set_counter(TIMER_TypeDef *timer,
                                       uint32_t value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure counter value is valid.
  EFM_ASSERT(value <= SL_HAL_TIMER_MAX_COUNT(timer));
  // Make sure module is enabled
  EFM_ASSERT(timer->EN & _TIMER_EN_EN_MASK);

  sl_hal_timer_wait_sync(timer);
  timer->CNT = value;
}

/***************************************************************************//**
 * @brief
 *   Set the top value buffer for the timer.
 *
 * @details
 *   When top value buffer register is updated, value is loaded into
 *   top value register at the next wrap around. This feature is useful
 *   in order to update top value safely when timer is running.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] value
 *   Value to set in top value buffer register.
 ******************************************************************************/
__INLINE void sl_hal_timer_set_top_buffer(TIMER_TypeDef *timer,
                                          uint32_t value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure counter value is valid.
  EFM_ASSERT(value <= SL_HAL_TIMER_MAX_COUNT(timer));
  // Make sure module is enabled
  EFM_ASSERT(timer->EN & _TIMER_EN_EN_MASK);

  sl_hal_timer_wait_sync(timer);
  timer->TOPB = value;
}

/***************************************************************************//**
 * @brief
 *   Set the top value for timer.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] value
 *   Value to set in top value register.
 ******************************************************************************/
__INLINE void sl_hal_timer_set_top(TIMER_TypeDef *timer,
                                   uint32_t value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure counter value is valid.
  EFM_ASSERT(value <= SL_HAL_TIMER_MAX_COUNT(timer));
  // Make sure module is enabled
  EFM_ASSERT(timer->EN & _TIMER_EN_EN_MASK);

  sl_hal_timer_wait_sync(timer);

  timer->TOP = value;
}

/***************************************************************************//**
 * @brief
 *   Get the top value setting for the timer.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @return
 *   Current top value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_timer_get_top(TIMER_TypeDef *timer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

  sl_hal_timer_wait_sync(timer);
  return timer->TOP;
}

/***************************************************************************//**
 * @brief
 *   Get status values for the timer.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @return
 *   Current status values.
 ******************************************************************************/
__INLINE uint32_t sl_hal_timer_get_status(TIMER_TypeDef *timer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

  return timer->STATUS;
}

#if defined(_TIMER_STATUS2_MASK)
/***************************************************************************//**
 * @brief
 *   Get status2 values for the timer.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @return
 *   Current status2 values.
 ******************************************************************************/
__INLINE uint32_t sl_hal_timer_get_status2(TIMER_TypeDef *timer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

  return timer->STATUS2;
}
#endif // defined(_TIMER_STATUS2_MASK)

#if defined(_TIMER_LOCK_MASK)
/***************************************************************************//**
 * @brief
 *   Lock some TIMER registers to protect them from being
 *   modified.
 *
 * @details
 *   Refer to the reference manual for TIMER registers that will be locked.
 *
 * @note
 *   If locking the TIMER registers, they must be unlocked prior to using any
 *   TIMER API function that modifies TIMER registers protected by the lock.
 *
 * @param[in] timer
 *   Pointer to TIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_timer_lock(TIMER_TypeDef *timer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

  timer->LOCK = ~_TIMER_LOCK_LOCKKEY_UNLOCK;
}

/***************************************************************************//**
 * @brief
 *   Unlock some TIMER registers to be able to modify registers.
 *
 * @details
 *   Refer to the reference manual for TIMER registers that will be unlocked.
 *
 * @note
 *   Unlock prior to using any TIMER API function that modifies TIMER registers
 *   protected by the lock.
 *
 * @param[in] timer
 *   Pointer to TIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_timer_unlock(TIMER_TypeDef *timer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

  timer->LOCK = _TIMER_LOCK_LOCKKEY_UNLOCK;
}
#endif

/***************************************************************************//**
 * @brief
 *   Enable DTI unit. Make sure to enable the DTI before enabling the timer.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_timer_dti_enable(TIMER_TypeDef *timer)
{
  // Check if timer supports DTI.
  EFM_ASSERT(SL_HAL_TIMER_SUPPORTS_DTI(timer));

  sl_hal_timer_disable(timer);
  sl_hal_timer_wait_ready(timer);

  timer->DTCFG_SET = TIMER_DTCFG_DTEN;
}

/***************************************************************************//**
 * @brief
 *   Disable DTI unit. Make sure to disable the DTI before enabling the timer.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_timer_dti_disable(TIMER_TypeDef *timer)
{
  // Check if timer supports DTI.
  EFM_ASSERT(SL_HAL_TIMER_SUPPORTS_DTI(timer));

  sl_hal_timer_disable(timer);
  sl_hal_timer_wait_ready(timer);

  timer->DTCFG_CLR = TIMER_DTCFG_DTEN;
}

/***************************************************************************//**
 * @brief
 *   Get DTI fault source flags status.
 *
 * @note
 *   Event bits are not cleared by this function.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @return
 *   Status of the DTI fault source flags. Returns one or more valid
 *   DTI fault source flags (TIMER_DTFAULT_nnn) OR'ed together.
 ******************************************************************************/
__INLINE uint32_t sl_hal_timer_dti_get_fault(TIMER_TypeDef *timer)
{
  // Check if timer supports DTI.
  EFM_ASSERT(SL_HAL_TIMER_SUPPORTS_DTI(timer));

  return timer->DTFAULT;
}

/***************************************************************************//**
 * @brief
 *   Clear DTI fault source flags.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] flags
 *   DTI fault source(s) to clear. Use one or more valid DTI fault
 *   source flags (TIMER_DTFAULT_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_timer_dti_clear_fault(TIMER_TypeDef *timer,
                                           uint32_t flags)
{
  // Check if timer supports DTI.
  EFM_ASSERT(SL_HAL_TIMER_SUPPORTS_DTI(timer));
  // Make sure module is enabled
  EFM_ASSERT(timer->EN & TIMER_EN_EN);

  timer->DTFAULTC = flags;
}

#if defined(_TIMER_DTLOCK_MASK)
/***************************************************************************//**
 * @brief
 *   Unlock timer DTI to enable writing to locked registers again.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_timer_dti_unlock(TIMER_TypeDef *timer)
{
  // Check if timer supports DTI.
  EFM_ASSERT(SL_HAL_TIMER_SUPPORTS_DTI(timer));

  timer->DTLOCK = TIMER_DTLOCK_DTILOCKKEY_UNLOCK;
}

/***************************************************************************//**
 * @brief
 *   Lock timer DTI to disable writing to registers.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_timer_dti_lock(TIMER_TypeDef *timer)
{
  // Check if timer supports DTI.
  EFM_ASSERT(SL_HAL_TIMER_SUPPORTS_DTI(timer));

  timer->DTLOCK = ~TIMER_DTLOCK_DTILOCKKEY_UNLOCK;
}
#endif

/***************************************************************************//**
 * @brief
 *   Clear one or more pending TIMER interrupts.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] flags
 *   Pending TIMER interrupt source(s) to clear. Use one or more valid
 *   interrupt flags for the TIMER module (TIMER_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_timer_clear_interrupts(TIMER_TypeDef *timer,
                                            uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

  timer->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Disable one or more TIMER interrupts.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] flags
 *   TIMER interrupt source(s) to disable. Use one or more valid
 *   interrupt flags for the TIMER module (TIMER_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_timer_disable_interrupts(TIMER_TypeDef *timer,
                                              uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

  timer->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Enable one or more TIMER interrupts.
 *
 * @note
 *   Depending on the use, a pending interrupt may already be set prior to
 *   enabling the interrupt.  To ignore a pending interrupt, consider using
 *   TIMER_IntClear() prior to enabling the interrupt.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] flags
 *   TIMER interrupt source(s) to enable. Use one or more valid
 *   interrupt flags for the TIMER module (TIMER_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_timer_enable_interrupts(TIMER_TypeDef *timer,
                                             uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

  timer->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get pending TIMER interrupt flags.
 *
 * @note
 *   Event bits are not cleared by this function.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @return
 *   TIMER interrupt source(s) pending. Returns one or more valid
 *   interrupt flags for the TIMER module (TIMER_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE uint32_t sl_hal_timer_get_pending_interrupts(TIMER_TypeDef *timer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

  return timer->IF;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending TIMER interrupt flags.
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @note
 *   Interrupt flags are not cleared by this function.
 *
 * @return
 *   Pending and enabled TIMER interrupt sources.
 *   The return value is the bitwise AND combination of
 *   - the OR combination of enabled interrupt sources in TIMERx_IEN_nnn
 *     register (TIMERx_IEN_nnn) and
 *   - the OR combination of valid interrupt flags of the TIMER module
 *     (TIMERx_IF_nnn).
 ******************************************************************************/
__INLINE uint32_t sl_hal_timer_get_enabled_pending_interrupts(TIMER_TypeDef *timer)
{
  uint32_t ien;

  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

  /* Store TIMER->IEN in temporary variable in order to define explicit order
   * of volatile accesses. */
  ien = timer->IEN;

  /* Bitwise AND of pending and enabled interrupts */
  return timer->IF & ien;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending TIMER interrupts from SW.
 *
 * @param[in] timer
 *   Pointer to the TIMER peripheral register block.
 *
 * @param[in] flags
 *   TIMER interrupt source(s) to set to pending. Use one or more valid
 *   interrupt flags for the TIMER module (TIMER_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_timer_set_interrupts(TIMER_TypeDef *timer,
                                          uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

  timer->IF_SET = flags;
}
/** @} (end addtogroup timer) */

#ifdef __cplusplus
}
#endif

/* *INDENT-OFF* */
/****************************************************************************//**
 * @addtogroup timer TIMER - Timer/Counter
 * @{
 *@n @section timer_intro Introduction
 *  This module contains functions to control the TIMER peripheral of Silicon
 *  Labs' 32-bit MCUs and SoCs. The general purpose timer has 3 or 4
 *  compare/capture channels for input capture and compare/Pulse-Width
 *  Modulation (PWM) output. The TIMER module may be 16 or 32 bits wide. Some
 *  timers also include a Dead-Time Insertion module suitable for motor control
 *  applications.
 *
 *  Refer to the device data sheet to determine the capabilities (capture/compare
 *  channel count, width, and DTI) of each timer instance.
 *
 *@n @section timer_example Examples
 *
 *  TIMER enable counter example:
 *  @code{.c}
 *  {
 *    // Enable the clock for the timer.
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);
 *
 *    // Initialize a timer and enable it.
 *    sl_hal_timer_init_t init = SL_HAL_TIMER_INIT_DEFAULT;
 *    sl_hal_timer_init(TIMER0, &init);
 *    sl_hal_timer_enable(TIMER0);
 *
 *    // Get counter value.
 *    uint32_t counter_value = sl_hal_timer_get_counter(TIMER0);
 *
 *    // Start counter.
 *    sl_hal_timer_start(TIMER0);
 *
 *    // [User code]
 *
 *    // Get a new counter value.
 *    uint32_t new_counter_value = sl_hal_timer_get_counter(TIMER0);
 *
 *    // Stop counter.
 *    sl_hal_timer_stop(TIMER0);
 *
 *    sl_hal_timer_disable(TIMER0);
 *  }
 *
 *  @endcode
 *
 *  TIMER interrupt example:
 *  @code{.c}
 *  // Global irq flag.
 *  static volatile uint32_t irq_flag = 0;
 *
 *  // Example IRQHandler.
 *  void TIMER0_IRQHandler(void)
 *  {
 *    // Get all pending and enabled interrupts.
 *    uint32_t pending = sl_hal_timer_get_enabled_pending_interrupts(TIMER0);
 *
 *    // Handle different IRQ flags here, execute callbacks, etc.
 *    if (pending) {
 *      sl_hal_timer_clear_interrupts(TIMER0, pending);
 *      irq_flag = pending;
 *    }
 *  }
 *
 *  void timer_example(void)
 *  {
 *    // Enable the clock for the timer.
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);
 *
 *    // Initialize a timer and enable it.
 *    sl_hal_timer_init_t init = SL_HAL_TIMER_INIT_DEFAULT;
 *    sl_hal_timer_init(TIMER0, &init);
 *    sl_hal_timer_enable(TIMER0);
 *
 *    sl_hal_timer_clear_interrupts(TIMER0, _TIMER_IF_MASK);
 *    sl_hal_timer_enable_interrupts(TIMER0, _TIMER_IEN_MASK);
 *    sl_interrupt_manager_enable_irq(TIMER0_IRQn);
 *
 *    sl_hal_timer_start(TIMER0);
 *
 *    // Wait until we get the flag.
 *    while (!irq_flag) ;
 *
 *    // Cleanup.
 *    sl_hal_timer_stop(TIMER0);
 *    sl_interrupt_manager_disable_irq(TIMER0_IRQn);
 *    sl_hal_timer_reset(TIMER0);
 *    sl_clock_manager_disable_bus_clock(SL_BUS_CLOCK_TIMER0);
 *  }
 *
 *  @endcode
 *
 *  TIMER buffer update mode example:
 *  @code{.c}
 *  void timer_buffer_update_example(void)
 *  {
 *    // Enable the clock for the timer
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);
 *
 *    // Initialize timer with update mode enabled
 *    sl_hal_timer_init_t init = SL_HAL_TIMER_INIT_DEFAULT;
 *    init.enable_update_mode = true;  // Enable manual update mode
 *
 *    // Initialize a channel for PWM operation
 *    sl_hal_timer_channel_init_t channel_config = SL_HAL_TIMER_CHANNEL_PWM_INIT_DEFAULT;
 *
 *    // Configure timer and channel
 *    sl_hal_timer_init(TIMER0, &init);
 *    sl_hal_timer_channel_init(TIMER0, 0, &channel_config);
 *
 *    // Set initial parameters
 *    sl_hal_timer_set_top(TIMER0, 10000);                // Set PWM period
 *    sl_hal_timer_channel_set_compare(TIMER0, 0, 5000);  // 50% duty cycle
 *
 *    // Enable timer and start counting
 *    sl_hal_timer_enable(TIMER0);
 *    sl_hal_timer_wait_sync(TIMER0);
 *    sl_hal_timer_start(TIMER0);
 *
 *    // Change PWM parameters using buffer registers
 *    uint32_t new_compare_value = 7500;       // New PWM compare value (75% duty cycle)
 *    uint32_t new_phase_value = 1000;         // New phase value
 *    uint8_t new_dither_value = 8;            // New dither value
 *
 *    // Write to buffer registers (not immediately applied)
 *    sl_hal_timer_channel_set_compare_buffer(TIMER0, 0, new_compare_value);
 *    sl_hal_timer_channel_set_phase_buffer(TIMER0, 0, new_phase_value);
 *    sl_hal_timer_channel_set_dither_buffer(TIMER0, 0, new_dither_value);
 *
 *    // With update_mode enabled, buffer values are only marked for
 *    // transfer after explicitly calling sl_hal_timer_channel_update_registers()
 *    sl_hal_timer_channel_update_registers(TIMER0);
 *
 *    // Check if buffer values are pending transfer
 *    uint32_t status = sl_hal_timer_get_status2(TIMER0);
 *    if ((status & _TIMER_STATUS2_PHASEBV0_MASK) &&
 *        (status & _TIMER_STATUS2_DITHERBV0_MASK)) {
 *      // Buffer values are pending transfer at next overflow/underflow
 *    }
 *
 *    // Optionally force an update event soon by setting counter close to overflow
 *    sl_hal_timer_set_counter(TIMER0, 0xffff0000);
 *
 *    // Wait for synchronization and transfer to complete
 *    sl_hal_timer_wait_sync(TIMER0);
 *
 *    // Verify buffer values have been transferred
 *    status = sl_hal_timer_get_status2(TIMER0);
 *    if (!(status & _TIMER_STATUS2_PHASEBV0_MASK) &&
 *        !(status & _TIMER_STATUS2_DITHERBV0_MASK)) {
 *      // Buffer values have been transferred to active registers
 *
 *      // Read active register values to confirm
 *      uint32_t current_compare = sl_hal_timer_channel_get_compare(TIMER0, 0);
 *      uint32_t current_phase = sl_hal_timer_channel_get_phase(TIMER0, 0);
 *      uint32_t current_dither = sl_hal_timer_channel_get_dither(TIMER0, 0);
 *    }
 *
 *    // Cleanup
 *    sl_hal_timer_stop(TIMER0);
 *    sl_hal_timer_disable(TIMER0);
 *    sl_clock_manager_disable_bus_clock(SL_BUS_CLOCK_TIMER0);
 *  }
 *  @endcode
 *
 *  @note When update_mode is disabled (default), buffer values are automatically
 *  transferred to active registers at the next update event (overflow/underflow).
 *  When update_mode is enabled, sl_hal_timer_channel_update_registers() must be
 *  called to mark buffers for transfer at the next update event.
 *
 *  @cond DOXYDOC_SIMG301
 *  TIMER PWM with PRS example:
 *  @code{.c}
 *  void timer_pwm_prs_example(void)
 *  {
 *    // Enable clocks
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);
 *
 *    // Init timer
 *    sl_hal_timer_config_t timer_config = SL_HAL_TIMER_CONFIG_DEFAULT;
 *    sl_hal_timer_init(TIMER0, &timer_config);
 *
 *    // Init CC channel
 *    sl_hal_timer_channel_config_t channel_config = SL_HAL_TIMER_CHANNEL_CONFIG_PWM;
 *    // Route TIMER output to PRS
 *    channel_config.prs_output = true;
 *    sl_hal_timer_channel_init(TIMER0, 0, &channel_config);
 *
 *    // Setup PRS channel
 *    // Get PRS channel
 *    uint8_t channel;
 *    sl_hal_prs_get_free_channel(&channel, SL_HAL_PRS_TYPE_ASYNC);
 *    sl_hal_prs_async_channel_config_t init_prs = SL_HAL_PRS_ASYNC_INIT_DEFAULT(channel, SL_HAL_PRS_ASYNC_TIMER0_CC0, SL_HAL_PRS_CONSUMER_NONE);
 *    // Invert PRS signal (PWM signal)
 *    init_prs.aux_prs = 0;
 *    init_prs.logic = SL_HAL_PRS_LOGIC_NOT_A;
 *    // Init PRS
 *    sl_hal_prs_async_init_channel(&init_prs);
 *
 *    // Route PRS signal to GPIO pin
 *    sl_hal_gpio_set_pin_mode(&pc1, SL_GPIO_MODE_PUSH_PULL, 0);
 *    // Update GPIO pin according to your application
 *    sl_hal_prs_pin_output(channel, SL_HAL_PRS_TYPE_ASYNC, SL_GPIO_PORT_C, 1);
 *
 *    sl_hal_timer_enable(TIMER0);
 *    // Set the desired TOP value
 *    sl_hal_timer_set_top(TIMER0, 0xffffff);
 *    sl_hal_timer_wait_sync(TIMER0);
 *
 *    // Set the desired compare match value
 *    sl_hal_timer_channel_set_compare_buffer(TIMER0, 0, 0x8fffff);
 *    sl_hal_timer_channel_set_phase_buffer(TIMER0, 0, 0);
 *    // Set the desired Dither value
 *    sl_hal_timer_channel_set_dither_buffer(TIMER0, 0, 4);
 *    uint32_t pending = sl_hal_timer_get_pending_interrupts(TIMER0);
 *    sl_hal_timer_clear_interrupts(TIMER0, pending);
 *    sl_hal_timer_channel_update_registers(TIMER0);
 *
 *    sl_hal_timer_start(TIMER0);
 *    sl_hal_timer_wait_sync(TIMER0);
 *
 *    // Cleanup
 *    sl_hal_timer_stop(TIMER0);
 *    sl_hal_timer_disable(TIMER0);
 *    sl_clock_manager_disable_bus_clock(SL_BUS_CLOCK_TIMER0);
 *    sl_clock_manager_disable_bus_clock(SL_BUS_CLOCK_PRS);
 *  }
 *  @endcode
 *
 *  @note This example demonstrates a workaround for Rainier devices where a hardware
 *  issue prevents using both dither and output inversion features simultaneously within
 *  the TIMER module. The workaround routes the PWM signal through a PRS channel that
 *  applies the inversion logic, while the TIMER module handles the dither functionality.
 *  This allows users to achieve both dithered PWM output and signal inversion by
 *  combining TIMER dither capabilities with PRS logic operations.
 *  @endcond
 *
 *  TIMER DTI safely initialize example:
 *  @code{.c}
 *  {
 *    const uint8_t channel_num = SL_HAL_TIMER_CHANNEL_NUM(TIMER0);
 *    uint8_t i;
 *    // OCB values of all channels, value of 7 represents an assumption about the maximum channels per timer.
 *    uint32_t ocb_value[7];
 *    // OCBV values of all channels, value of 7 represents an assumption about the maximum channels per timer.
 *    uint32_t ocbv[7];
 *    uint32_t topb_value;
 *    uint32_t topbv;
 *
 *    // Enable the clock for the timer.
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER0);
 *
 *    // Initialize a timer and enable it.
 *    sl_hal_timer_init_t init = SL_HAL_TIMER_INIT_DEFAULT;
 *    sl_hal_timer_init(TIMER0, &init);
 *
 *    // Store value of buffer registers and buffer valid flags of all channels
 *    // before calling of sl_hal_timer_dti_init().
 *    for (i = 0; i < channel_num; i++) {
 *      ocb_value[i] = timer->CC[i].OCB;
 *      ocbv[i] = timer->STATUS & (_TIMER_STATUS_OCBV0_MASK << i);
 *    }
 *    topb_value = timer->TOPB;
 *    topbv = timer->STATUS & _TIMER_STATUS_TOPBV_MASK;
 *
 *    // Enable DTI unit.
 *    sl_hal_timer_dti_config_t initDTI = SL_HAL_TIMER_DTI_CONFIG_DEFAULT;
 *    sl_hal_timer_dti_init(TIMER0, &initDTI);
 *    sl_hal_timer_dti_enable(TIMER0);
 *    sl_hal_timer_enable(TIMER0);
 *    sl_hal_timer_wait_ready(TIMER0);
 *
 *    // Re-arm buffer values and buffer valid flags of all channels.
 *    for (i = 0; i < channel_num; i++) {
 *      if (ocbv[i] != 0) {
 *        // If the OCBV value is valid, write it back to the OCB register.
 *        timer->CC[i].OCB = ocb_value[i];
 *      }
 *    }
 *    // Write back the TOPB value.
 *    if (topbv != 0) {
 *      timer->TOPB = topb_value;
 *    }
 *  }
 *  @endcode
 *
 *  @note This example shows how to safely initialize the DTI unit when other
 *  channels of a TIMER are working. It stores the buffer values and buffer
 *  valid flags before initializing the DTI unit, and then restores them after
 *  TIMER is enabled again.
 *
 * @} (end addtogroup timer)
 ******************************************************************************/
/* *INDENT-ON* */

#endif /* TIMER_PRESENT */
#endif /* SL_HAL_TIMER_H */
