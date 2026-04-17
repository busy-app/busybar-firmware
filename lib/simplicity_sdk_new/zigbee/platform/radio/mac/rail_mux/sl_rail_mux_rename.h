/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef RAIL_MUX_RENAME_H
#define RAIL_MUX_RENAME_H

#include PLATFORM_HEADER
#include "sl_rail_mux.h"
#include "coexistence/protocol/ieee802154_uc/coexistence-802154.h"

#define sl_rail_util_ieee802154_config_radio sl_rail_mux_util_ieee802154_config_radio
#define sl_rail_set_state_timing  sl_rail_mux_SetStateTiming
#define sl_rail_ieee802154_set_rx_to_enh_ack_tx   sl_rail_mux_IEEE802154_SetRxToEnhAckTx
#define sl_rail_config_rx_options   sl_rail_mux_ConfigRxOptions
#define sl_rail_ieee802154_config_2_mbps_rx_channel sl_rail_mux_ieee802154_config_2_mbps_rx_channel
#define sl_rail_set_task_priority   sl_rail_mux_SetTaskPriority
#define sl_rail_ieee802154_config_2p4_ghz_radio         sl_rail_mux_IEEE802154_Config2p4GHzRadio
#define sl_rail_ieee802154_config_2p4_ghz_radio_ant_div   sl_rail_mux_IEEE802154_Config2p4GHzRadioAntDiv
#define sl_rail_ieee802154_config_2p4_ghz_radio_2_mbps    sl_rail_mux_IEEE802154_Config2p4GHzRadio2Mbps
#define sl_rail_ieee802154_config_2p4_ghz_radio_fcs_2_mbps   sl_rail_mux_IEEE802154_Config2p4GHzRadioFcs2Mbps
#define sl_rail_ieee802154_config_2p4_ghz_radio_fast_channel_switching sl_rail_mux_ieee802154_config_2p4_ghz_radio_fast_channel_switching
#define sl_rail_ieee802154_config_2p4_ghz_radio_rx_duty_cycling sl_rail_mux_ieee802154_config_2p4_ghz_radio_rx_duty_cycling
#define sl_rail_config_events   sl_rail_mux_ConfigEvents
#define sl_rail_init   sl_rail_mux_Init
#define sl_rail_set_pti_protocol   sl_rail_mux_SetPtiProtocol
#define sl_rail_config_channels   sl_rail_mux_ConfigChannels
#define sl_rail_ieee802154_init   sl_rail_mux_IEEE802154_Init
#define sl_rail_convert_lqi   sl_rail_mux_ConvertLqi
#define sl_rail_calibrate   sl_rail_mux_Calibrate
#define sl_rail_ieee802154_calibrate_ir_2p4_ghz   sl_rail_mux_IEEE802154_CalibrateIr2p4Ghz
#define sl_rail_get_radio_entropy   sl_rail_mux_GetRadioEntropy
#define sl_rail_ieee802154_set_short_address   sl_rail_mux_IEEE802154_SetShortAddress
#define sl_rail_ieee802154_set_long_address   sl_rail_mux_IEEE802154_SetLongAddress
#define sl_rail_ieee802154_set_pan_id   sl_rail_mux_IEEE802154_SetPanId
#define sl_rail_ieee802154_set_pan_coordinator   sl_rail_mux_IEEE802154_SetPanCoordinator
#define sl_rail_ieee802154_set_addresses sl_rail_mux_IEEE802154_SetAddresses
#define sl_rail_is_valid_channel   sl_rail_mux_IsValidChannel
#define sl_rail_get_rssi   sl_rail_mux_GetRssi
#define sl_rail_get_symbol_rate   sl_rail_mux_GetSymbolRate
#define sl_rail_get_bit_rate   sl_rail_mux_GetBitRate
#define sl_rail_cancel_timer   sl_rail_mux_CancelTimer
#define sl_rail_get_pending_cal   sl_rail_mux_GetPendingCal
#define sl_rail_calibrate_temp   sl_rail_mux_CalibrateTemp
#define sl_rail_get_rx_packet_info   sl_rail_mux_GetRxPacketInfo
#define sl_rail_get_rx_packet_details   sl_rail_mux_GetRxPacketDetailsAlt
#define sl_rail_peek_rx_packet   sl_rail_mux_PeekRxPacket
#define sl_rail_ieee802154_get_address   sl_rail_mux_IEEE802154_GetAddress
#define sl_rail_get_rx_incoming_packet_info   sl_rail_mux_GetRxIncomingPacketInfo
#define sl_rail_ieee802154_toggle_frame_pending   sl_rail_mux_IEEE802154_SetFramePending
#define sl_rail_get_scheduler_status   sl_rail_mux_GetSchedulerStatus
#define sl_rail_is_rx_auto_ack_paused sl_rail_mux_IsRxAutoAckPaused
#define sli_rail_get_tx_power_config   sli_rail_mux_GetTxPowerConfig
#define sl_rail_util_pa_post_init   sl_rail_mux_util_pa_post_init
#define sl_rail_get_pa_mode sl_rail_mux_get_pa_mode
#define sl_rail_config_tx_power   sl_rail_mux_ConfigTxPower
#define sl_rail_set_tx_power_dbm   sl_rail_mux_SetTxPowerDbm
#define sl_rail_yield_radio   sl_rail_mux_YieldRadio
#define sl_rail_get_radio_state   sl_rail_mux_GetRadioState
#define sl_rail_idle   sl_rail_mux_Idle
#define sl_rail_start_rx   sl_rail_mux_StartRx
#define sl_rail_set_tx_fifo   sl_rail_mux_SetTxFifo
#define sl_rail_start_cca_csma_tx   sl_rail_mux_StartCcaCsmaTx
#define sl_rail_start_tx  sl_rail_mux_StartTx
#define sl_rail_stop_tx_stream   sl_rail_mux_StopTxStream
#define sl_rail_ieee802154_set_promiscuous_mode   sl_rail_mux_IEEE802154_SetPromiscuousMode
#define sl_rail_pause_rx_auto_ack   sl_rail_mux_PauseRxAutoAck
#define sl_rail_set_cca_threshold   sl_rail_mux_SetCcaThreshold

#define sl_rail_set_freq_offset sl_rail_mux_SetFreqOffset
#define sl_rail_set_timer sl_rail_mux_SetTimer

#define sl_rail_ieee802154_is_enabled   sl_rail_mux_IEEE802154_IsEnabled
#define sl_rail_set_rx_transitions   sl_rail_mux_SetRxTransitions
#define sl_rail_config_cal   sl_rail_mux_ConfigCal
#define sl_rail_write_tx_fifo   sl_rail_mux_WriteTxFifo
#define sl_rail_read_rx_fifo   sl_rail_mux_ReadRxFifo
#define sl_rail_get_rx_fifo_bytes_available   sl_rail_mux_GetRxFifoBytesAvailable
#define sl_rail_start_scheduled_tx   sl_rail_mux_StartScheduledTx
#define sl_rail_start_scheduled_cca_csma_tx sl_rail_mux_StartScheduledCcaCsmaTx
#define sl_rail_start_scheduled_rx sl_rail_mux_RAIL_ScheduleRx
#define sl_rail_config_sleep   sl_rail_mux_ConfigSleep
#define sl_rail_get_rssi_offset sl_rail_mux_GetRssiOffset

#define sl_rail_start_average_rssi   sl_rail_mux_StartAverageRssi

#define sl_rail_get_tx_power_dbm   sl_rail_mux_GetTxPowerDbm
#define sli_rail_get_tx_power   sli_rail_mux_GetTxPower

#define sl_rail_ieee802154_write_enh_ack   sl_rail_mux_IEEE802154_WriteEnhAck

#define sl_rail_release_rx_packet   sl_rail_mux_ReleaseRxPacket

#define sl_rail_hold_rx_packet   sl_rail_mux_HoldRxPacket

#define sl_rail_get_average_rssi   sl_rail_mux_GetAverageRssi

#define sl_rail_get_rx_time_sync_word_end   sl_rail_mux_GetRxTimeSyncWordEndAlt

#define sl_rail_ieee802154_get_phy_id   sl_rail_mux_IEEE802154_GetPtiRadioConfig

#define sl_rail_ieee802154_enable_early_frame_pending   sl_rail_mux_IEEE802154_EnableEarlyFramePending

#define sl_rail_set_tx_fifo_threshold     sl_rail_mux_SetTxFifoThreshold

#define sl_rail_set_next_tx_repeat sl_rail_mux_SetNextTxRepeat

#define sl_rail_get_channel sl_rail_mux_GetChannel

#define sl_rail_ieee802154_enable_data_frame_pending sl_rail_mux_IEEE802154_EnableDataFramePending

#define sl_rail_get_tx_packets_remaining sl_rail_mux_GetTxPacketsRemaining

#define sl_rail_reset_fifo sl_rail_mux_ResetFifo
#define sl_rail_config_rx_duty_cycle sl_rail_mux_ConfigRxDutyCycle
#define sl_rail_enable_rx_duty_cycle sl_rail_mux_EnableRxDutyCycle
#define sl_rail_get_tx_fifo_space_available sl_rail_mux_GetTxFifoSpaceAvailable
#define sl_rail_ieee802154_accept_frames sl_rail_mux_IEEE802154_AcceptFrames
#define sl_rail_ieee802154_config_2p4_ghz_radio_ant_div_coex sl_rail_mux_IEEE802154_Config2p4GHzRadioAntDivCoex
#define sl_rail_ieee802154_config_2p4_ghz_radio_ant_div_coex_fem sl_rail_mux_IEEE802154_Config2p4GHzRadioAntDivCoexFem
#define sl_rail_ieee802154_config_2p4_ghz_radio_ant_div_fem sl_rail_mux_IEEE802154_Config2p4GHzRadioAntDivFem
#define sl_rail_ieee802154_config_2p4_ghz_radio_coex sl_rail_mux_IEEE802154_Config2p4GHzRadioCoex
#define sl_rail_ieee802154_config_2p4_ghz_radio_coex_fem sl_rail_mux_IEEE802154_Config2p4GHzRadioCoexFem
#define sl_rail_ieee802154_config_2p4_ghz_radio_fem sl_rail_mux_IEEE802154_Config2p4GHzRadioFem
#define sl_rail_ieee802154_config_cca_mode sl_rail_mux_IEEE802154_ConfigCcaMode
#define sl_rail_ieee802154_config_e_options sl_rail_mux_IEEE802154_ConfigEOptions
#define sl_rail_ieee802154_config_g_options sl_rail_mux_IEEE802154_ConfigGOptions
#define sl_rail_ieee802154_set_phy_id sl_rail_mux_IEEE802154_SetPtiRadioConfig
#define sl_rail_set_rssi_offset sl_rail_mux_SetRssiOffset
#define sl_rail_start_tx_stream sl_rail_mux_StartTxStreamAlt
#define sl_rail_ieee802154_config_2p4_ghz_radio_1_mbps_fec sl_rail_mux_IEEE802154_Config2p4GHzRadio1MbpsFec
#define sl_rail_ieee802154_config_2p4_ghz_radio_fcs_1_mbps_fec sl_rail_mux_IEEE802154_Config2p4GHzRadioFcs1MbpsFec
#define sl_rail_get_rx_time_frame_end sl_rail_mux_GetRxTimeFrameEnd
#define sl_rail_copy_rx_packet sl_rail_mux_CopyRxPacket
#define sl_rail_config_multi_timer sl_rail_mux_ConfigMultiTimer
#define sl_rail_set_multi_timer sl_rail_mux_SetMultiTimer
#define sl_rail_cancel_multi_timer sl_rail_mux_CancelMultiTimer
#define sl_rail_is_multi_timer_running sl_rail_mux_IsMultiTimerRunning
#define sl_rail_get_time sl_rail_mux_GetTime
#define sl_rail_prepare_channel sl_rail_mux_PrepareChannel
#define sl_rail_is_next_cca_now sl_rail_mux_IsNextCcaNow

#define ATOMIC_SECTION(RETURN_TYPE, yourcode, ...) \
  ({                                               \
    CORE_DECLARE_IRQ_STATE;                        \
    RETURN_TYPE result;                            \
    CORE_ENTER_ATOMIC();                           \
    {                                              \
      result = yourcode(__VA_ARGS__);              \
    }                                              \
    CORE_EXIT_ATOMIC();                            \
    result;                                        \
  })

#define sl_rail_util_coex_set_bool(...)         ATOMIC_SECTION(sl_status_t, sl_rail_util_coex_set_bool, __VA_ARGS__)
#define sl_rail_util_coex_get_options(...)      ATOMIC_SECTION(sl_rail_util_coex_options_t, sl_rail_util_coex_get_options, __VA_ARGS__)
#define sl_rail_util_coex_set_options(...)      ATOMIC_SECTION(sl_status_t, sl_rail_util_coex_set_options, __VA_ARGS__)
#define sl_rail_util_coex_set_enable(...)       ATOMIC_SECTION(sl_status_t, sl_rail_util_coex_set_enable, __VA_ARGS__)
#define sl_rail_util_coex_set_tx_request(...)   ATOMIC_SECTION(sl_status_t, sl_rail_util_coex_set_tx_request, __VA_ARGS__)
#define sl_rail_util_coex_set_rx_request(...)   ATOMIC_SECTION(sl_status_t, sl_rail_util_coex_set_rx_request, __VA_ARGS__)
#define sl_rail_util_coex_set_request_pwm(...)  ATOMIC_SECTION(sl_status_t, sl_rail_util_coex_set_request_pwm, __VA_ARGS__)

#endif //RAIL_MUX_RENAME_H
