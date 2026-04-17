/***************************************************************************//**
 * @file
 * @brief Duty-Cycle API
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef DUTY_CYCLE_H_INCLUDED
#define DUTY_CYCLE_H_INCLUDED

//-- Public Configuration

#define EM_PHY_DC_MAX_CHANNELS    2  // Track up to 2 channels' duty cycle
#define EM_PHY_DC_PERIOD_SECS  3600  // aDUTYCYCLEMeasurementPeriod (1 hour)
#define EM_PHY_DC_BUCKET_SECS   300  // Seconds per bucket
#if     ((EM_PHY_DC_PERIOD_SECS % EM_PHY_DC_BUCKET_SECS) != 0)
  #error EM_PHY_DC_BUCKET_SECS must divide evenly into EM_PHY_DC_PERIOD_SECS
#endif//((EM_PHY_DC_PERIOD_SECS % EM_PHY_DC_BUCKET_SECS) != 0)

#define EM_PHY_DC_DEFAULT_TX_TIMEOUT_MS MILLISECOND_TICKS_PER_SECOND // 1 second

//-- Public Types

/** @brief Duty Cycle, Limits, and Thresholds are reported in units of
 *  Percent * 100 (i.e. 10000 = 100.00%, 1 = 0.01%).
 */
typedef uint16_t EmPhyDcHectoPct; // Percent * 100

/** @brief Duty Cycle Limit setting indicating use the PHY config default */
#define EM_PHY_DC_LIMIT_USE_DEFAULT 0     // Use PHY config default

/** @brief Duty Cycle Limit setting indicating the limit is unlimited */
#define EM_PHY_DC_LIMIT_UNLIMITED   (100 * 100) // 100.00%

/** @strct EmPhyDcLimits
 * @brief Duty Cycle Limits
 */
typedef struct EmPhyDcLimits {
  EmPhyDcHectoPct talkLimit;  /**< The Talk Limit (non-LBT) in % * 100 */
  EmPhyDcHectoPct limiThresh; /**< The Limited Threshold in % * 100 */
  EmPhyDcHectoPct critThresh; /**< The Critical Threshold in % * 100 */
  EmPhyDcHectoPct suspLimit;  /**< The Suspended Limit (LBT) in % * 100 */
} EmPhyDcLimits;

/** @brief Duty Cycle Modes - applications have some control over the mode.
 */
typedef enum {
  EM_PHY_DC_MODE_OFF = 0,       // 0 No DC enforcement
  EM_PHY_DC_MODE_TALK,          // 1 Non RDC, Tx w/o LBT/CSMA,  no TX_OFF
  EM_PHY_DC_MODE_CSMA,          // 2 Non RDC, Tx w/  CSMA,      no TX_OFF
  EM_PHY_DC_MODE_DIALOG,        // 3 LBT RDC, Tx w/o LBT,       no TX_OFF
  EM_PHY_DC_MODE_LBTX,          // 4 LBT RDC, Tx w/  LBT,       no TX_OFF
  EM_PHY_DC_MODE_LBT,           // 5 LBT RDC, Tx w/  LBT,      min TX_OFF
  EM_PHY_DC_MODE_UNCHANGED = 6, // 6 Don't change the mode
  EM_PHY_DC_MODE_ITEMS = EM_PHY_DC_MODE_UNCHANGED // Must be last
} EmPhyDcMode;

/** @brief Duty Cycle States - applications have no control over the state
 *         but the callback exposes state changes to the application.
 */
typedef enum {
  EM_PHY_DC_STATE_OFF = 0,      //  0 Off (No DC monitoring)
  EM_PHY_DC_STATE_NORMAL,       //  1 LBT=ok  Crit=ok  NODC=ok  Talk=ok
  EM_PHY_DC_STATE_REQUIRE_LBT,  //  2 LBT=ok  Crit=ok  NODC=ok  Talk=hit
  EM_PHY_DC_STATE_LIMITED_TALK, //  3 LBT=ok  Crit=ok  NODC=hit Talk=ok  (rare)
  EM_PHY_DC_STATE_LIMITED_LBT,  //  4 LBT=ok  Crit=ok  NODC=hit Talk=hit
  EM_PHY_DC_STATE_ILLEGAL5,     //  5 LBT=ok  Crit=hit NODC=ok  Talk=ok  (impossible)
  EM_PHY_DC_STATE_CRITICAL_NODC,//  6 LBT=ok  Crit=hit NODC=ok  Talk=hit (rare)
  EM_PHY_DC_STATE_ILLEGAL7,     //  7 LBT=ok  Crit=hit NODC=hit Talk=ok  (impossible)
  EM_PHY_DC_STATE_CRITICAL_LBT, //  8 LBT=ok  Crit=hit NODC=hit Talk=hit
  EM_PHY_DC_STATE_ILLEGAL9,     //  9 LBT=hit Crit=ok  NODC=ok  Talk=ok  (impossible)
  EM_PHY_DC_STATE_ILLEGAL10,    // 10 LBT=hit Crit=ok  NODC=ok  Talk=hit (impossible)
  EM_PHY_DC_STATE_ILLEGAL11,    // 11 LBT=hit Crit=ok  NODC=hit Talk=ok  (impossible)
  EM_PHY_DC_STATE_ILLEGAL12,    // 12 lBT=hit Crit=ok  NODC=hit Talk=hit (impossible)
  EM_PHY_DC_STATE_ILLEGAL13,    // 13 LBT=hit Crit=hit NODC=ok  Talk=ok  (impossible)
  EM_PHY_DC_STATE_ILLEGAL14,    // 14 LBT=hit Crit=hit NODC=ok  Talk=hit (impossible)
  EM_PHY_DC_STATE_ILLEGAL15,    // 15 LBT=hit Crit=hit NODC=hit Talk=ok  (impossible)
  EM_PHY_DC_STATE_SUSPENDED,    // 16 LBT=hit Crit=hit NODC=hit Talk=hit
  EM_PHY_DC_STATE_ITEMS         // Must be last
} EmPhyDcState;

/** @brief Duty Cycle / LBT Extended Transmit Status
 * Low-order nibble is an enumerated status; high-order nibble is how many
 * LBT/CSMA tries occurred.
 */
#define EM_PHY_DC_EXT_LBT_TRIES_SHIFT 4
#define EM_PHY_DC_EXT_LBT_TRIES_MASK  (0x0Fu << EM_PHY_DC_EXT_LBT_TRIES_SHIFT)
#define EM_PHY_DC_EXT_STATUS_MASK     0x0Fu

enum {
  EM_PHY_DC_EXT_STATUS_SUCCESS,             // 0 (SL_ZIGBEE_SUCCESS)
  EM_PHY_DC_EXT_STATUS_PHY_BUSY,            // 1 (TX_BUSY)
  EM_PHY_DC_EXT_STATUS_DC_EXCEEDED,         // 2 (TX_BUSY)
  EM_PHY_DC_EXT_STATUS_TX_TIME_LIMIT,       // 3 (TX_BUSY)
  EM_PHY_DC_EXT_STATUS_LBT_TRIES_EXCEEDED,  // 4 (TX_CCA_FAIL)
  EM_PHY_DC_EXT_STATUS_LBT_TIMEOUT,         // 5 (TX_CCA_FAIL)
  EM_PHY_DC_EXT_STATUS_LBT_TIMEDOUT,        // 6 (TX_CCA_FAIL)
  EM_PHY_DC_EXT_STATUS_VALUES               // Must be last
};
typedef uint8_t EmPhyDcExtendedStatus;

//-- Public APIs

/** @brief Set a new Duty Cycle Mode.
 *  @param dcMode The EmPhyDcMode to set.
 * @return SL_ZIGBEE_SUCCESS on success, SL_ZIGBEE_BAD_ARGUMENT on failure.
 */
extern sl_status_t emPhyDcSetMode(EmPhyDcMode dcMode);

/** @brief Get the current Duty Cycle Mode.
 * @return The current EmPhyDcMode in effect.
 */
extern EmPhyDcMode emPhyDcGetMode(void);

/** @brief Get the current packet's Duty Cycle Mode.
 * @return The current EmPhyDcMode in effect for the packet being transmitted.
 */
extern EmPhyDcMode emPhyDcGetPktMode(void);

/** @brief Get the current Duty Cycle State.  This state reflects the
 * ability to send a full-sized packet.
 * @return The current EmPhyDcState in effect.
 */
extern EmPhyDcState emPhyDcGetState(void);

/** @brief Get the actual Duty Cycle State.  This state reflects the
 * actual consumed duty cycle.
 * @return The actual EmPhyDcState in effect.
 */
extern EmPhyDcState emPhyDcGetActualState(void);

/** @brief Obtain the current duty cycle, and optionally the various limits
 *         currently in force.
 *  @param limitsPtr Pointer to the set of limit to fill in if not NULL.
 *         These are the actual limits in use based on the current limits
 *         configuration.
 * @return The current duty cycle consumed.
 */
extern EmPhyDcHectoPct emPhyDcGetDcAndLimits(EmPhyDcLimits* limitsPtr);

/** @brief Retrieve the current duty cycle limits configuration.
 * @return The current duty cycle limits configuration.
 */
extern const EmPhyDcLimits* emPhyDcGetLimitsConfig(void);

/** @brief Set the current duty cycle limits configuration.
 * @param The duty cycle limits configuration to utilize henceforth.
 * @return SL_STATUS_OK if config looks kosher; SL_ZIGBEE_BAD_ARGUMENT otherwise
 * (e.g. illegal value or violates constraints Talk <= Limi <= Crit <= Susp).
 */
extern sl_status_t emPhyDcSetLimitsConfig(const EmPhyDcLimits* limitsConfig);

/** @brief Initiate sending of a packet as soon as practical.
 *  @param packet The packet to send, starting with its length byte.
 *  @param limitMs A time limit for sending the packet.
 *  @param callbackOnErrorStatus If true, call sli_802154phy_radio_transmit_complete_callback()
 *         before returning an error status, otherwise don't (caller will
 *         handle the error).
 * @return SL_STATUS_OK if packet is accepted to be sent (its disposition
 *         will be reported later via sli_802154phy_radio_transmit_complete_callback()),
 *         or an error like SL_ZIGBEE_PHY_TX_BUSY if packet cannot be accepted.
 */
extern sl_status_t sli_802154phy_dc_send_pkt_asap(uint8_t* packet,
                                                  uint32_t limitMs,
                                                  bool callbackOnErrorStatus);

/** @brief Credit duty cycle due to packet failed to be transmitted.
 * @note This routine may be called from interrupt context.
 */
extern void sli_802154phy_dc_send_pkt_failed(void);

/** @brief Debit duty cycle for a transmitted ACK.
 * @note This routine may be called from interrupt context.
 */
extern void emPhyDcAckSent(void);

/** @brief Tick routine ensures DC is kept up-to-date
 */
extern void emPhyDcTick(void);

/** @brief Return the extended Duty-Cycle/LBT status after transmit completion.
 * Only valid on SL_ZIGBEE_PHY_TX_BUSY and SL_ZIGBEE_PHY_TX_CCA_FAIL errors.
 */
extern EmPhyDcExtendedStatus emPhyDcGetExtendedStatus(void);

/** @brief Helper function to set the status appropriately.
 */
extern void emPhyDcSetExtendedStatus(EmPhyDcExtendedStatus dcStatus);

//-- Public Callbacks

/** @brief Callback on Duty Cycle State changes.
 *  @param macPgChan The channel whose duty cycle state has changed.
 *  @param curMode The current Duty Cycle Mode in effect.
 *  @param oldState The prior Duty Cycle State that was in effect.
 *  @param newState The new Duty Cycle State now taking effect.
 * @return A new Duty Cycle mode to put into effect, or
 *         EM_PHY_DC_MODE_UNCHANGED to leave it unchanged.
 */
extern EmPhyDcMode sli_802154phy_dc_state_callback(uint8_t macPgChan,
                                                   EmPhyDcMode curMode,
                                                   EmPhyDcState oldState,
                                                   EmPhyDcState newState);

#endif//DUTY_CYCLE_H_INCLUDED
