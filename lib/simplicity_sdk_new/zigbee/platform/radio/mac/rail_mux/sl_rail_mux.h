/***************************************************************************//**
 * @file sl_rail_mux.h
 * @brief RAIL Multiplexer APIs, types, macros and globals.
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

#ifndef RAIL_MUX_H
#define RAIL_MUX_H

#include PLATFORM_HEADER
#include "sl_status.h"

// TODO: possibly turn this into a CMSIS option
#define SUPPORTED_PROTOCOL_COUNT 2

// Global flags
#define RAIL_MUX_FLAGS_RAIL_INIT_STARTED                                  0x0001
#define RAIL_MUX_FLAGS_RAIL_INIT_COMPLETED                                0x0002
#define RAIL_MUX_FLAGS_IEEE802154_INIT_COMPLETED                          0x0004
#define RAIL_MUX_FLAGS_RAIL_SET_PTI_DONE                                  0x0008
#define RAIL_MUX_FLAGS_RAIL_CONFIG_TX_POWER_DONE                          0x0010

// Protocol-specific flags
#define RAIL_MUX_PROTOCOL_FLAGS_INIT_CB_PENDING                           0x0001
#define RAIL_MUX_PROTOCOL_FLAGS_SETUP_TX_FIFO                             0x0002
#define RAIL_MUX_PROTOCOL_FLAGS_START_TX_PENDING                          0x0004
#define RAIL_MUX_PROTOCOL_FLAGS_TX_IN_PROGRESS                            0x0008
#define RAIL_MUX_PROTOCOL_FLAGS_WAIT_FOR_ACK                              0x0010
#define RAIL_MUX_PROTOCOL_FLAGS_SCHEDULED_TX_PENDING                      0x0020
#define RAIL_MUX_PROTOCOL_FLAGS_TX_SCHEDULED                              0x0040
#define RAIL_MUX_PROTOCOL_FLAGS_CONFIG_REPEATED_TX                        0x0080
#define RAIL_MUX_PROTOCOL_FLAGS_CSMA_CONFIG_VALID                         0x0100
#define RAIL_MUX_PROTOCOL_FLAGS_SCHEDULER_INFO_VALID                      0x0200
#define RAIL_MUX_PROTOCOL_FLAGS_SCHEDULE_TX_CONFIG_VALID                  0x0400
#define RAIL_MUX_PROTOCOL_FLAGS_RX_SCHEDULED                              0x0800
#define RAIL_MUX_PROTOCOL_FLAGS_STACK_EVENT_RX_ACTIVE                     0x1000
#define RAIL_MUX_PROTOCOL_FLAGS_STACK_EVENT_TX_ACTIVE                     0x2000
#define RAIL_MUX_PROTOCOL_FLAGS_STACK_EVENT_RADIO_ACTIVE                  0x4000
#define RAIL_MUX_PROTOCOL_FLAGS_LOCK_ACTIVE                               0x8000

#define RAIL_MUX_PROTOCOL_ACTIVE_TX_FLAGS \
  (RAIL_MUX_PROTOCOL_FLAGS_TX_IN_PROGRESS | RAIL_MUX_PROTOCOL_FLAGS_WAIT_FOR_ACK)

// Other macros
#ifdef INVALID_CHANNEL
#undef INVALID_CHANNEL
#endif
#define INVALID_CHANNEL 0xFFFF

#define INVALID_CONTEXT_INDEX 0xFF

// normally 15.4 protocols have use max packet size of 128, but e.g.Zigbee CSL needs 512 bytes
#define RAIL_MUX_MAX_BUFFER_SIZE 512 // 4 * 128 in ZB CSL

// For 802.15.4 the filtering bits in the RAIL_AddrFilterMask_t are:
//  7     6     5     4       3    2    1    0
//  Addr3 Addr2 Addr1 AddrB | Pan3 Pan2 Pan1 PanB
#define RAIL_MUX_FILTERING_MASK_BROADCAST_ENABLED  0x11

#include "sl_rail_types.h"
#include "sl_rail_ieee802154.h"
#include "sl_rail_util_ieee802154/sl_rail_util_ieee802154_stack_event.h"
#include "coexistence/protocol/ieee802154_uc/coexistence-802154.h"

typedef enum {
  SCHEDULED_TX_REQUEST,
  SINGLE_TX_REQUEST,
  SINGLE_TX_CCA_CSMA_REQUEST,
  SCHEDULED_TX_CCA_CSMA_REQUEST,
} sli_tx_type_t;

typedef struct {
  sli_tx_type_t tx_type;
  uint16_t channel;
  sl_rail_tx_options_t options;
  sl_rail_csma_config_t csmaConfig;
  sl_rail_scheduled_tx_config_t scheduler_config;
  sl_rail_scheduler_info_t schedulerInfo;
} sli_csma_tx_info_t;

typedef struct {
  uint16_t tx_init_length;
  uint16_t tx_size;
  uint8_t *data_ptr;
} sli_fifo_tx_info_t;

typedef void (*COEX_CounterHandler_t)(sl_rail_util_coex_event_t event);

typedef struct {
  sl_rail_config_t *rail_config;
  sl_rail_init_complete_callback_t init_callback;
  sl_rail_radio_config_changed_callback_t config_channels_callback;
  sl_rail_events_t events;
  volatile uint16_t flags;
  uint16_t channel;
  sl_rail_tx_power_t tx_power;
  sl_rail_multi_timer_t timer;
  sl_rail_timer_callback_t timer_callback;
  COEX_CounterHandler_t coex_counter_handler;

  // 802.15.4 specific fields
  sl_rail_ieee802154_addr_config_t addr_802154;
  bool is_pan_coordinator_802154;
  uint8_t addr_filter_mask_802154;

  // TODO: all protocol-specific information should be added here
  sli_fifo_tx_info_t fifo_tx_info;
  sli_csma_tx_info_t csma_tx_info;
  sl_rail_tx_repeat_config_t tx_repeat_config;
} sl_rail_mux_context_t;

// System-wide initialization callback
void sli_rail_mux_local_init(void);

sl_rail_status_t sl_rail_mux_YieldRadio(sl_rail_handle_t railHandle);

sl_rail_status_t sl_rail_mux_ConfigRxOptions(sl_rail_handle_t railHandle,
                                             sl_rail_rx_options_t mask,
                                             sl_rail_rx_options_t options);

sl_rail_status_t sl_rail_mux_SetStateTiming (sl_rail_handle_t railHandle,
                                             sl_rail_state_timing_t *timings);

sl_rail_status_t sl_rail_mux_IEEE802154_SetRxToEnhAckTx(sl_rail_handle_t railHandle,
                                                        sl_rail_transition_time_t *pRxToEnhAckTx);

int8_t sl_rail_mux_GetRssiOffset(sl_rail_handle_t railHandle);

sl_rail_status_t sl_rail_mux_CancelTimer(sl_rail_handle_t railHandle);

sl_rail_status_t sl_rail_mux_SetTimer(sl_rail_handle_t railHandle,
                                      sl_rail_time_t time,
                                      sl_rail_time_mode_t mode,
                                      sl_rail_timer_callback_t cb);

bool sl_rail_mux_IsRxAutoAckPaused(sl_rail_handle_t railHandle);

sl_rail_status_t sl_rail_mux_GetRxIncomingPacketInfo(sl_rail_handle_t railHandle,
                                                     sl_rail_rx_packet_info_t *pPacketInfo);

sl_rail_status_t sl_rail_mux_SetFreqOffset(sl_rail_handle_t railHandle,
                                           sl_rail_frequency_offset_t freqOffset);

sl_rail_status_t sl_rail_mux_SetTaskPriority(sl_rail_handle_t railHandle,
                                             uint8_t priority,
                                             sl_rail_task_type_t taskType);

sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadio(sl_rail_handle_t railHandle);

sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadioAntDiv(sl_rail_handle_t railHandle);

sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadio1MbpsFec(sl_rail_handle_t railHandle);

sl_rail_status_t sl_rail_mux_ConfigEvents(sl_rail_handle_t railHandle,
                                          sl_rail_events_t mask,
                                          sl_rail_events_t events);

sl_rail_status_t sl_rail_mux_Init(sl_rail_handle_t *p_rail_handle,
                                  sl_rail_config_t *p_rail_config,
                                  sl_rail_init_complete_callback_t init_complete_callback);

sl_rail_status_t sl_rail_mux_SetPtiProtocol(sl_rail_handle_t railHandle,
                                            sl_rail_pti_protocol_t protocol);

sl_rail_status_t sl_rail_mux_PauseRxAutoAck(sl_rail_handle_t railHandle,
                                            bool pause);

sl_rail_status_t sl_rail_mux_Idle(sl_rail_handle_t railHandle,
                                  sl_rail_idle_mode_t mode,
                                  bool wait);

sl_rail_status_t sl_rail_mux_CopyRxPacket(sl_rail_handle_t railHandle,
                                          uint8_t *pDest,
                                          const sl_rail_rx_packet_info_t *pPacketInfo);

sl_rail_status_t sl_rail_mux_ConfigChannels(sl_rail_handle_t railHandle,
                                            const sl_rail_channel_config_t *config,
                                            sl_rail_radio_config_changed_callback_t cb);

sl_rail_status_t sl_rail_mux_IEEE802154_Init(sl_rail_handle_t railHandle,
                                             const sl_rail_ieee802154_config_t *config);

sl_rail_status_t sl_rail_mux_ConvertLqi(sl_rail_handle_t railHandle,
                                        sl_rail_convert_lqi_callback_t cb);

sl_rail_status_t sl_rail_mux_Calibrate(sl_rail_handle_t railHandle,
                                       sl_rail_cal_values_t *calValues,
                                       sl_rail_cal_mask_t calForce);

sl_rail_status_t sl_rail_mux_IEEE802154_CalibrateIr2p4Ghz(sl_rail_handle_t railHandle,
                                                          uint32_t *imageRejection);

uint16_t sl_rail_mux_GetRadioEntropy(sl_rail_handle_t railHandle,
                                     uint8_t *buffer,
                                     uint16_t bytes);

sl_rail_status_t sl_rail_mux_IEEE802154_SetShortAddress(sl_rail_handle_t railHandle,
                                                        uint16_t shortAddr,
                                                        uint8_t index);

sl_rail_status_t sl_rail_mux_IEEE802154_SetLongAddress(sl_rail_handle_t railHandle,
                                                       const uint8_t *longAddr,
                                                       uint8_t index);

sl_rail_status_t sl_rail_mux_IEEE802154_SetPanId(sl_rail_handle_t railHandle,
                                                 uint16_t panId,
                                                 uint8_t index);

sl_rail_status_t sl_rail_mux_IEEE802154_SetPanCoordinator(sl_rail_handle_t railHandle,
                                                          bool isPanCoordinator);

sl_rail_status_t sl_rail_mux_IsValidChannel(sl_rail_handle_t railHandle, uint16_t channel);

int16_t sl_rail_mux_GetRssi(sl_rail_handle_t railHandle, sl_rail_time_t  waitTimeout);

uint32_t sl_rail_mux_GetSymbolRate(sl_rail_handle_t railHandle);

uint32_t sl_rail_mux_GetBitRate(sl_rail_handle_t railHandle);

sl_rail_cal_mask_t sl_rail_mux_GetPendingCal(sl_rail_handle_t railHandle);

sl_rail_status_t sl_rail_mux_CalibrateTemp(sl_rail_handle_t railHandle);

sl_rail_rx_packet_handle_t sl_rail_mux_GetRxPacketInfo(sl_rail_handle_t railHandle,
                                                       sl_rail_rx_packet_handle_t packetHandle,
                                                       sl_rail_rx_packet_info_t *pPacketInfo);

sl_rail_status_t sl_rail_mux_GetRxPacketDetailsAlt(sl_rail_handle_t railHandle,
                                                   sl_rail_rx_packet_handle_t packetHandle,
                                                   sl_rail_rx_packet_details_t *pPacketDetails);

sl_rail_status_t sl_rail_mux_GetRxTimeSyncWordEnd(sl_rail_handle_t railHandle,
                                                  uint16_t totalPacketBytes,
                                                  sl_rail_time_t *pPacketTime);

uint16_t sl_rail_mux_PeekRxPacket(sl_rail_handle_t railHandle,
                                  sl_rail_rx_packet_handle_t packetHandle,
                                  uint8_t *pDst,
                                  uint16_t len,
                                  uint16_t offset);

sl_rail_status_t sl_rail_mux_IEEE802154_GetAddress(sl_rail_handle_t railHandle,
                                                   sl_rail_ieee802154_address_t *pAddress);

sl_rail_status_t sl_rail_mux_IEEE802154_SetFramePending(sl_rail_handle_t railHandle);

sl_rail_status_t sl_rail_mux_GetSchedulerStatus(sl_rail_handle_t rail_handle,
                                                sl_rail_scheduler_status_t *p_scheduler_status,
                                                sl_rail_status_t *p_rail_status);

sl_rail_status_t sli_rail_mux_GetTxPowerConfig(sl_rail_handle_t railHandle,
                                               sl_rail_tx_power_config_t *config);

sl_rail_tx_power_t sl_rail_mux_GetTxPowerDbm(sl_rail_handle_t railHandle);
sl_rail_tx_power_t sli_rail_mux_GetTxPower(sl_rail_handle_t railHandle);
sl_rail_status_t sl_rail_mux_util_pa_post_init(sl_rail_handle_t railHandle,
                                               sl_rail_tx_pa_mode_t pa_mode);
sl_rail_tx_pa_mode_t sl_rail_mux_get_pa_mode(sl_rail_handle_t railHandle);
sl_rail_status_t sl_rail_mux_ConfigTxPower(sl_rail_handle_t railHandle,
                                           const sl_rail_tx_power_config_t *config);

sl_rail_status_t sl_rail_mux_SetTxPowerDbm(sl_rail_handle_t railHandle,
                                           sl_rail_tx_power_t power);

sl_rail_radio_state_t sl_rail_mux_GetRadioState(sl_rail_handle_t railHandle);

sl_rail_status_t sl_rail_mux_StartRx(sl_rail_handle_t railHandle,
                                     uint16_t channel,
                                     const sl_rail_scheduler_info_t *schedulerInfo);

sl_rail_status_t sl_rail_mux_SetTxFifo(sl_rail_handle_t rail_handle,
                                       sl_rail_fifo_buffer_align_t *p_addr,
                                       uint16_t size_bytes,
                                       uint16_t init_bytes,
                                       uint16_t start_offset_bytes);

sl_rail_status_t sl_rail_mux_StartCcaCsmaTx(sl_rail_handle_t railHandle,
                                            uint16_t channel,
                                            sl_rail_tx_options_t options,
                                            const sl_rail_csma_config_t *csmaConfig,
                                            const sl_rail_scheduler_info_t *schedulerInfo);

sl_rail_status_t sl_rail_mux_StartTx(sl_rail_handle_t railHandle,
                                     uint16_t channel,
                                     sl_rail_tx_options_t options,
                                     const sl_rail_scheduler_info_t *schedulerInfo);

sl_rail_status_t sl_rail_mux_StartScheduledCcaCsmaTx(sl_rail_handle_t railHandle,
                                                     uint16_t channel,
                                                     sl_rail_tx_options_t options,
                                                     const sl_rail_scheduled_tx_config_t *scheduleTxConfig,
                                                     const sl_rail_csma_config_t *csmaConfig,
                                                     const sl_rail_scheduler_info_t *schedulerInfo);

sl_rail_status_t sl_rail_mux_StopTxStream(sl_rail_handle_t railHandle);

uint8_t sl_rail_mux_IEEE802154_ConvertRssiToEd(int8_t rssiDbm);

sl_rail_status_t sl_rail_mux_IEEE802154_SetPromiscuousMode(sl_rail_handle_t railHandle,
                                                           bool enable);

sl_rail_status_t sl_rail_mux_SetCcaThreshold(sl_rail_handle_t railHandle,
                                             int8_t ccaThresholdDbm);

sl_rail_status_t sl_rail_mux_SetRxTransitions(sl_rail_handle_t railHandle,
                                              const sl_rail_state_transitions_t *transitions);
sl_rail_status_t sl_rail_mux_ConfigCal(sl_rail_handle_t railHandle,
                                       sl_rail_cal_mask_t calEnable);
sl_rail_status_t sl_rail_mux_EnablePaAutoMode(sl_rail_handle_t railHandle, bool enable);

uint16_t sl_rail_mux_ReadRxFifo(sl_rail_handle_t railHandle,
                                uint8_t *dataPtr,
                                uint16_t readLength);

uint16_t sl_rail_mux_GetRxFifoBytesAvailable(sl_rail_handle_t railHandle);

sl_rail_status_t sl_rail_mux_StartScheduledTx(sl_rail_handle_t railHandle,
                                              uint16_t channel,
                                              sl_rail_tx_options_t options,
                                              const sl_rail_scheduled_tx_config_t *config,
                                              const sl_rail_scheduler_info_t *schedulerInfo);

sl_rail_status_t sl_rail_mux_ConfigSleep(sl_rail_handle_t rail_handle,
                                         const sl_rail_timer_sync_config_t *p_timer_sync_config);

sl_rail_status_t sl_rail_mux_StartAverageRssi(sl_rail_handle_t railHandle,
                                              uint16_t channel,
                                              sl_rail_time_t averagingTimeUs,
                                              const sl_rail_scheduler_info_t *schedulerInfo);

uint16_t sl_rail_mux_WriteTxFifo(sl_rail_handle_t railHandle,
                                 const uint8_t *dataPtr,
                                 uint16_t writeLength,
                                 bool reset);

sl_rail_status_t sl_rail_mux_IEEE802154_WriteEnhAck(sl_rail_handle_t railHandle,
                                                    const uint8_t *ackData,
                                                    uint16_t ackDataLen);

sl_rail_status_t sl_rail_mux_ReleaseRxPacket(sl_rail_handle_t railHandle,
                                             sl_rail_rx_packet_handle_t packetHandle);

sl_rail_rx_packet_handle_t sl_rail_mux_HoldRxPacket(sl_rail_handle_t railHandle);

int16_t sl_rail_mux_GetAverageRssi(sl_rail_handle_t railHandle);

sl_rail_status_t sl_rail_mux_GetRxTimeSyncWordEndAlt(sl_rail_handle_t railHandle,
                                                     sl_rail_rx_packet_details_t *pPacketDetails);

sl_rail_ieee802154_phy_t sl_rail_mux_IEEE802154_GetPtiRadioConfig(sl_rail_handle_t railHandle);

sl_rail_status_t sl_rail_mux_IEEE802154_EnableEarlyFramePending(sl_rail_handle_t railHandle,
                                                                bool enable);

uint16_t sl_rail_mux_SetTxFifoThreshold(sl_rail_handle_t railHandle, uint16_t txThreshold);

sl_rail_status_t sl_rail_mux_RAIL_ScheduleRx(sl_rail_handle_t railHandle,
                                             uint16_t channel,
                                             const sl_rail_scheduled_rx_config_t *cfg,
                                             const sl_rail_scheduler_info_t *schedulerInfo);

sl_rail_status_t sl_rail_mux_SetNextTxRepeat(sl_rail_handle_t railHandle,
                                             const sl_rail_tx_repeat_config_t *repeatConfig);

void sl_rail_mux_update_active_radio_config(void);

void sl_rail_mux_set_coex_counter_handler(sl_rail_handle_t railHandle,
                                          COEX_CounterHandler_t counter_handler);

sl_rail_util_ieee802154_stack_status_t sl_rail_mux_ieee802154_on_event(sl_rail_handle_t railHandle,
                                                                       sl_rail_util_ieee802154_stack_event_t stack_event,
                                                                       uint32_t supplement);
sl_rail_status_t sl_rail_mux_IEEE802154_EnableDataFramePending(sl_rail_handle_t railHandle,
                                                               bool enable);

sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadio2Mbps(sl_rail_handle_t railHandle);

#ifdef HIGH_DATARATE_PHY
void sl_rail_mux_set_high_datarate_phy_index(sl_rail_handle_t railHandle);
#endif //HIGH_DATARATE_PHY

sl_rail_status_t sl_rail_mux_ConfigMultiTimer(sl_rail_handle_t rail_handle,
                                              bool enable);

sl_rail_status_t sl_rail_mux_SetMultiTimer(sl_rail_handle_t rail_handle,
                                           sl_rail_multi_timer_t *p_tmr,
                                           sl_rail_time_t expiration_time,
                                           sl_rail_time_mode_t expiration_mode,
                                           sl_rail_multi_timer_callback_t expiration_callback,
                                           void *cb_arg);

sl_rail_status_t sl_rail_mux_CancelMultiTimer(sl_rail_handle_t rail_handle,
                                              sl_rail_multi_timer_t *p_tmr);

bool sl_rail_mux_IsMultiTimerRunning(sl_rail_handle_t rail_handle,
                                     sl_rail_multi_timer_t *p_tmr);

sl_rail_time_t sl_rail_mux_GetTime(sl_rail_handle_t rail_handle);

sl_rail_status_t sl_rail_mux_PrepareChannel(sl_rail_handle_t rail_handle,
                                            uint16_t channel);

/**
 * In a MUX configuration, where a single radio is used concurrently
 * by two or more protocol stacks, this function allows the caller to
 * lock the radio for exclusive use. This will prevent the other multiplexed
 * protocol stacks from performing certain RAIL operations including but
 * not limited to future transmits, scheduled tx, etc. Note that this does not
 * prevent transmits and receives that were scheduled prior to the lock being acquired
 *
 * Due to its disruptive nature, the caller
 * must attempt to return things to normal by unlocking the radio in a timely
 * fashion. Lock may be used to perform composite radio operations without
 * interruptions from other protocol stacks.
 *
 * NOTE: Part of this function runs with interrupts disabled
 *
 * @param[in] railHandle  Rail handle of protocol stack requesting the lock
 *
 * @retval SL_STATUS_FAIL  Radio lock unsuccessful.
 * @retval SL_STATUS_OK   Radio lock successful.
 *
 */
sl_status_t sli_rail_mux_lock_radio (sl_rail_handle_t railHandle);

/**
 * In a MUX configuration, where a single radio is used concurrently
 * by two or more protocol stacks, this function allows the caller to
 * resume multiplexed radio operation from a locked state. The caller of
 * the lock function must be the one to unlock the radio from a locked state.
 * Unlock operations that are performed when there is no existing lock on the
 * radio shall always be allowed.
 *
 * NOTE: Part of this function runs with interrupts disabled
 *
 * @param[in] railHandle  Rail handle of protocol stack requesting the lock
 *
 * @retval SL_STATUS_FAIL  Radio unlock unsuccessful.
 * @retval SL_STATUS_OK   Radio unlock successful.
 *
 */
sl_status_t sli_rail_mux_unlock_radio (sl_rail_handle_t railHandle);

sl_rail_status_t sl_rail_mux_GetChannel(sl_rail_handle_t railHandle, uint16_t *channel);
uint16_t sl_rail_mux_GetTxPacketsRemaining(sl_rail_handle_t railHandle);
sl_rail_status_t sl_rail_mux_ResetFifo(sl_rail_handle_t railHandle, bool txFifo, bool rxFifo);
sl_rail_status_t sl_rail_mux_ConfigRxDutyCycle(sl_rail_handle_t railHandle,
                                               const sl_rail_rx_duty_cycle_config_t *config);
sl_rail_status_t sl_rail_mux_EnableRxDutyCycle(sl_rail_handle_t railHandle,
                                               bool enable);
uint16_t sl_rail_mux_GetTxFifoSpaceAvailable(sl_rail_handle_t railHandle);
sl_rail_status_t sl_rail_mux_IEEE802154_AcceptFrames(sl_rail_handle_t railHandle,
                                                     uint8_t framesMask);
sl_rail_status_t sl_rail_mux_util_ieee802154_config_radio(sl_rail_handle_t railHandle);
sl_rail_status_t sl_rail_mux_ieee802154_config_2_mbps_rx_channel(sl_rail_handle_t railHandle,
                                                                 uint16_t channel);
sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadioAntDivCoex(sl_rail_handle_t railHandle);
sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadioAntDivCoexFem(sl_rail_handle_t railHandle);
sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadioAntDivFem(sl_rail_handle_t railHandle);
sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadioCoex(sl_rail_handle_t railHandle);
sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadioCoexFem(sl_rail_handle_t railHandle);
sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadioFem(sl_rail_handle_t railHandle);
sl_rail_status_t sl_rail_mux_IEEE802154_ConfigCcaMode(sl_rail_handle_t railHandle,
                                                      sl_rail_ieee802154_cca_mode_t ccaMode);
sl_rail_status_t sl_rail_mux_IEEE802154_ConfigEOptions(sl_rail_handle_t railHandle,
                                                       sl_rail_ieee802154_e_options_t mask,
                                                       sl_rail_ieee802154_e_options_t options);
sl_rail_status_t sl_rail_mux_IEEE802154_ConfigGOptions(sl_rail_handle_t railHandle,
                                                       sl_rail_ieee802154_g_options_t mask,
                                                       sl_rail_ieee802154_g_options_t options);
sl_rail_status_t sl_rail_mux_IEEE802154_SetPtiRadioConfig(sl_rail_handle_t railHandle,
                                                          sl_rail_ieee802154_phy_t ptiRadioConfigId);
sl_rail_status_t sl_rail_mux_SetRssiOffset(sl_rail_handle_t railHandle, int8_t rssiOffset);
sl_rail_status_t sl_rail_mux_StartTxStreamAlt(sl_rail_handle_t railHandle,
                                              uint16_t channel,
                                              sl_rail_stream_mode_t mode,
                                              sl_rail_tx_options_t options);
sl_rail_status_t sl_rail_mux_GetRxTimeFrameEnd(sl_rail_handle_t railHandle,
                                               sl_rail_rx_packet_details_t *pPacketdetails);
bool sl_rail_mux_IsNextCcaNow(sl_rail_handle_t railHandle);
//------------------------------------------------------------------------------
// Internals

#if !defined(HIDDEN)
#ifdef SL_ZIGBEE_SCRIPTED_TEST
#define HIDDEN
#else
#define HIDDEN static
#endif
#endif

#ifdef SL_ZIGBEE_TEST
#define RAIL_MUX_DECLARE_IRQ_STATE
#define RAIL_MUX_ENTER_CRITICAL()
#define RAIL_MUX_EXIT_CRITICAL()
#else // !SL_ZIGBEE_TEST
#include "sl_core.h"
#define RAIL_MUX_DECLARE_IRQ_STATE CORE_DECLARE_IRQ_STATE
#define RAIL_MUX_ENTER_CRITICAL() CORE_ENTER_CRITICAL()
#define RAIL_MUX_EXIT_CRITICAL() CORE_EXIT_CRITICAL()
#endif // SL_ZIGBEE_TEST

#endif //RAIL_MUX_H
