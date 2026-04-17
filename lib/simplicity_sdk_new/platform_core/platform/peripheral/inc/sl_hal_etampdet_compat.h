/***************************************************************************//**
 * @file
 * @brief ETAMPDET Compatibility Layer.
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

#ifndef SL_HAL_ETAMPDET_COMPAT_H
#define SL_HAL_ETAMPDET_COMPAT_H

#include "em_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

// Enum.
#define sl_etampdet_channel_t                         sl_hal_etampdet_channel_t
#define sl_etampdet_channel_cnt_mismatch_threshold_t  sl_hal_etampdet_channel_cnt_mismatch_threshold_t
#define sl_etampdet_channel_filt_win_size_t           sl_hal_etampdet_channel_filt_win_size_t
#define sl_etampdet_upper_clk_presc_t                 sl_hal_etampdet_upper_clk_presc_t
#define sl_etampdet_lower_clk_presc_t                 sl_hal_etampdet_lower_clk_presc_t

// Structure.
#define sl_etampdet_config_channel_t                  sl_hal_etampdet_config_channel_t
#define sl_etampdet_config_t                          sl_hal_etampdet_config_t

// Function.
#define sl_etampdet_init                              sl_hal_etampdet_init
#define sl_etampdet_init_channel                      sl_hal_etampdet_init_channel
#define sl_etampdet_wait_sync                         sl_hal_etampdet_wait_sync
#define sl_etampdet_wait_ready                        sl_hal_etampdet_wait_ready
#define sl_etampdet_enable                            sl_hal_etampdet_enable
#define sl_etampdet_disable                           sl_hal_etampdet_disable
#define sl_etampdet_load                              sl_hal_etampdet_load
#define sl_etampdet_start                             sl_hal_etampdet_start
#define sl_etampdet_stop                              sl_hal_etampdet_stop
#define sl_etampdet_enable_interrupts                 sl_hal_etampdet_enable_interrupts
#define sl_etampdet_disable_interrupts                sl_hal_etampdet_disable_interrupts
#define sl_etampdet_set_interrupts                    sl_hal_etampdet_set_interrupts
#define sl_etampdet_clear_interrupts                  sl_hal_etampdet_clear_interrupts
#define sl_etampdet_get_interrupts                    sl_hal_etampdet_get_interrupts
#define sl_etampdet_get_enabled_interrupts            sl_hal_etampdet_get_enabled_interrupts
#define sl_etampdet_lock                              sl_hal_etampdet_lock
#define sl_etampdet_unlock                            sl_hal_etampdet_unlock
#define sl_etampdet_get_status                        sl_hal_etampdet_get_status
#define sl_etampdet_set_seed_value                    sl_hal_etampdet_set_seed_value
#define sl_etampdet_get_seed_value                    sl_hal_etampdet_get_seed_value

// CMSIS defines.
#if !defined(_ETAMPDET_CLKPRESCVAL_UPPERPRESC_Bypass)
#define _ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX0_DetectFilterThreshold1     _ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX0_DETECTFILTERTHRESHOLD1
#define _ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX0_DetectFilterThreshold2     _ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX0_DETECTFILTERTHRESHOLD2
#define _ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX0_DetectFilterThreshold3     _ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX0_DETECTFILTERTHRESHOLD3
#define _ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX0_DetectFilterThreshold4     _ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX0_DETECTFILTERTHRESHOLD4
#define _ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX0_DetectFilterThreshold5     _ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX0_DETECTFILTERTHRESHOLD5
#define _ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX0_DetectFilterThreshold6     _ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX0_DETECTFILTERTHRESHOLD6
#define _ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX0_DetectFilterThreshold7     _ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX0_DETECTFILTERTHRESHOLD7
#define _ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX0_DetectFilterThreshold8     _ETAMPDET_CNTMISMATCHMAX_CHNLCNTMISMATCHMAX0_DETECTFILTERTHRESHOLD8

#define _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DetectFilterMovingWinSize2   _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DETECTFILTERMOVINGWINSIZE2
#define _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DetectFilterMovingWinSize3   _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DETECTFILTERMOVINGWINSIZE3
#define _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DetectFilterMovingWinSize4   _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DETECTFILTERMOVINGWINSIZE4
#define _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DetectFilterMovingWinSize5   _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DETECTFILTERMOVINGWINSIZE5
#define _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DetectFilterMovingWinSize6   _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DETECTFILTERMOVINGWINSIZE6
#define _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DetectFilterMovingWinSize7   _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DETECTFILTERMOVINGWINSIZE7
#define _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DetectFilterMovingWinSize8   _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DETECTFILTERMOVINGWINSIZE8
#define _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DetectFilterMovingWinSize9   _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DETECTFILTERMOVINGWINSIZE9
#define _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DetectFilterMovingWinSize10  _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DETECTFILTERMOVINGWINSIZE10
#define _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DetectFilterMovingWinSize11  _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DETECTFILTERMOVINGWINSIZE11
#define _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DetectFilterMovingWinSize12  _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DETECTFILTERMOVINGWINSIZE12
#define _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DetectFilterMovingWinSize13  _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DETECTFILTERMOVINGWINSIZE13
#define _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DetectFilterMovingWinSize14  _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DETECTFILTERMOVINGWINSIZE14
#define _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DetectFilterMovingWinSize15  _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DETECTFILTERMOVINGWINSIZE15
#define _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DetectFilterMovingWinSize16  _ETAMPDET_CHNLFILTWINSIZE_CHNLFILTWINSIZE0_DETECTFILTERMOVINGWINSIZE16

#define _ETAMPDET_CLKPRESCVAL_UPPERPRESC_Bypass                                 _ETAMPDET_CLKPRESCVAL_UPPERPRESC_BYPASS
#define _ETAMPDET_CLKPRESCVAL_UPPERPRESC_DivideBy2                              _ETAMPDET_CLKPRESCVAL_UPPERPRESC_DIVIDEBY2
#define _ETAMPDET_CLKPRESCVAL_UPPERPRESC_DivideBy4                              _ETAMPDET_CLKPRESCVAL_UPPERPRESC_DIVIDEBY4
#define _ETAMPDET_CLKPRESCVAL_UPPERPRESC_DivideBy8                              _ETAMPDET_CLKPRESCVAL_UPPERPRESC_DIVIDEBY8
#define _ETAMPDET_CLKPRESCVAL_UPPERPRESC_DivideBy16                             _ETAMPDET_CLKPRESCVAL_UPPERPRESC_DIVIDEBY16
#define _ETAMPDET_CLKPRESCVAL_UPPERPRESC_DivideBy32                             _ETAMPDET_CLKPRESCVAL_UPPERPRESC_DIVIDEBY32
#define _ETAMPDET_CLKPRESCVAL_UPPERPRESC_DivideBy64                             _ETAMPDET_CLKPRESCVAL_UPPERPRESC_DIVIDEBY64

#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_Bypass                                 _ETAMPDET_CLKPRESCVAL_LOWERPRESC_BYPASS
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy2                              _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY2
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy3                              _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY3
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy4                              _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY4
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy5                              _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY5
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy6                              _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY6
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy7                              _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY7
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy8                              _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY8
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy9                              _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY9
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy10                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY10
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy11                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY11
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy12                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY12
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy13                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY13
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy14                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY14
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy15                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY15
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy16                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY16
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy17                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY17
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy18                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY18
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy19                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY19
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy20                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY20
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy21                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY21
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy22                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY22
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy23                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY23
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy24                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY24
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy25                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY25
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy26                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY26
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy27                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY27
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy28                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY28
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy29                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY29
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy30                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY30
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy31                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY31
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy32                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY32
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy33                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY33
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy34                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY34
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy35                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY35
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy36                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY36
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy37                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY37
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy38                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY38
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy39                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY39
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy40                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY40
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy41                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY41
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy42                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY42
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy43                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY43
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy44                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY44
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy45                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY45
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy46                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY46
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy47                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY47
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy48                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY48
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy49                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY49
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy50                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY50
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy51                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY51
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy52                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY52
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy53                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY53
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy54                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY54
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy55                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY55
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy56                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY56
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy57                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY57
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy58                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY58
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy59                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY59
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy60                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY60
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy61                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY61
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy62                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY62
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy63                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY63
#define _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DivideBy64                             _ETAMPDET_CLKPRESCVAL_LOWERPRESC_DIVIDEBY64
#endif

#ifdef __cplusplus
}
#endif

#endif // SL_HAL_ETAMPDET_COMPAT_H
