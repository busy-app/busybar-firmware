/***************************************************************************//**
 * @file
 * @brief The IEEE 802.15.4 specific header file for the RAIL library.
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

#ifndef SL_RAIL_IEEE802154_H
#define SL_RAIL_IEEE802154_H

#include "sl_rail_types.h"

#ifdef __cplusplus
extern "C" {
#endif

///
/// @addtogroup IEEE802_15_4 IEEE 802.15.4
/// @ingroup Protocol_Specific
/// @brief IEEE 802.15.4 configuration routines
///
/// The functions in this group configure RAIL IEEE 802.15.4 hardware
/// acceleration which includes IEEE 802.15.4 format filtering, address
/// filtering, Acking, and filtering based on the frame type.
///
/// To configure IEEE 802.15.4 functionality, the application must first set up
/// a RAIL instance with \ref sl_rail_init() and other setup functions.
/// Instead of \ref sl_rail_config_channels(), however, an
/// application may use \ref sl_rail_ieee802154_config_2p4_ghz_radio() to set up the
/// official IEEE 2.4 GHz 802.15.4 PHY. This configuration is shown below.
///
/// 802.15.4 defines its macAckWaitDuration from the end of the transmitted
/// packet to complete reception of the Ack. RAIL's ack_timeout_us only covers
/// sync word detection of the Ack. Therefore, subtract the Ack's
/// PHY header and payload time to get RAIL's ack_timeout_us setting.
/// For 2.4 GHz OQPSK, macAckWaitDuration is specified as 54 symbols;
/// subtracting 2-symbol PHY header and 10-symbol payload yields a RAIL
/// ack_timeout_us of 42 symbols or 672 microseconds at 16 microseconds/symbol.
/// @code{.c}
/// static sl_rail_handle_t rail_handle = NULL; // Initialized somewhere else.
///
/// static const sl_rail_ieee802154_config_t rail_154_config = {
///   .p_addresses = NULL,
///   .ack_config = {
///     .enable = true,        // Turn on auto Ack for IEEE 802.15.4.
///     .ack_timeout_us = 672, // See note above: 54-12 sym * 16 us/sym = 672 us.
///     .rx_transitions = {
///       .success = SL_RAIL_RF_STATE_RX,  // Return to RX after Ack processing
///       .error = SL_RAIL_RF_STATE_RX,    // Ignored
///     },
///     .tx_transitions = {
///       .success = SL_RAIL_RF_STATE_RX,  // Return to RX after Ack processing
///       .error = SL_RAIL_RF_STATE_RX,    // Ignored
///     },
///   },
///   .timings = {
///     .idle_to_rx = 100,
///     .idle_to_tx = 100,
///     .rx_to_tx = 192,    // 12 symbols * 16 us/symbol = 192 us
///     .tx_to_rx = 192,    // 12 symbols * 16 us/symbol = 192 us
///     .rxsearch_timeout = 0, // Not used
///     .tx_to_rxsearch_timeout = 0, // Not used
///   },
///   .frames_mask = SL_RAIL_IEEE802154_ACCEPT_STANDARD_FRAMES,
///   .promiscuous_mode = false,  // Enable format and address filtering.
///   .is_pan_coordinator = false,
///   .default_frame_pending_in_outgoing_acks = false,
/// };
///
/// void config154(void)
/// {
///   // Configure the radio and channels for 2.4 GHz IEEE 802.15.4.
///   sl_rail_ieee802154_config_2p4_ghz_radio(rail_handle);
///   // Initialize the IEEE 802.15.4 configuration using the static configuration above.
///   sl_rail_ieee802154_init(rail_handle, &rail_154_config);
/// }
/// @endcode
///
/// To configure address filtering, call \ref
/// sl_rail_ieee802154_set_addresses() with a structure containing all addresses or
/// call the individual \ref sl_rail_ieee802154_set_pan_id(), \ref
/// sl_rail_ieee802154_set_short_address(), and \ref sl_rail_ieee802154_set_long_address()
/// APIs. RAIL supports \ref SL_RAIL_IEEE802154_MAX_ADDRESSES number of address
/// pairs to receive packets from multiple IEEE
/// 802.15.4 networks at the same time. Broadcast addresses are supported by
/// default without any additional configuration so they do not consume one of
/// these slots. If the application does not require all address pairs, be sure
/// to set unused ones to the proper disabled value for each type. These can
/// be found in the \ref sl_rail_ieee802154_addr_config_t documentation. Below is
/// an example of setting filtering for one set of addresses.
/// @code{.c}
/// // PAN Id over-the-air value of 0x34 0x12.
/// // Short Address over-the-air byte order of 0x78 0x56.
/// // Long address with over-the-air byte order of 0x11 0x22 0x33 0x44 0x55 0x66 0x77 0x88.
///
/// // Set up all addresses simultaneously.
/// sl_rail_status_t setup_1(void)
/// {
///   sl_rail_ieee802154_addr_config_t node_addresses = {
///     .pan_id     = { 0x1234, 0xFFFF, 0xFFFF },
///     .short_addr = { 0x5678, 0xFFFF, 0xFFFF },
///     .long_addr  = { { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 },
///                     { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
///                     { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }
///   };
///   return sl_rail_ieee802154_set_addresses(rail_handle, &node_addresses);
/// }
///
/// // Alternatively, the addresses can be set up individually as follows:
/// void setup_2(void)
/// {
///   sl_rail_status_t status;
///   const uint8_t long_address[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
///
///   status = sl_rail_ieee802154_set_pan_id(rail_handle, 0x1234, 0);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///   status = sl_rail_ieee802154_set_short_address(rail_handle, 0x5678, 0);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///   status = sl_rail_ieee802154_set_long_address(rail_handle, long_address, 0);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
/// }
/// @endcode
///
/// Address filtering will be enabled except when in promiscuous mode, which can
/// be set with \ref sl_rail_ieee802154_set_promiscuous_mode(). The addresses may be
/// changed at runtime. However, if you are receiving a packet while
/// reconfiguring the address filters, you may get undesired behavior so it's
/// safest to do this while not in receive.
///
/// Auto Ack is controlled by the \ref sl_rail_ieee802154_config_t::ack_config
/// and \ref sl_rail_ieee802154_config_t::timings fields passed to \ref
/// sl_rail_ieee802154_init(). After initialization, they may be controlled
/// using the normal \ref Auto_Ack and \ref State_Transitions APIs. When in IEEE
/// 802.15.4 mode, the Ack will generally have a 5 byte length, its Frame Type
/// will be Ack, its Frame Version 0 (2003), and its Frame Pending bit will be
/// false unless the \ref SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND event is
/// triggered in which case it will default to the
/// \ref sl_rail_ieee802154_config_t::default_frame_pending_in_outgoing_acks setting.
/// If the default Frame Pending setting is incorrect,
/// the app must call \ref sl_rail_ieee802154_toggle_frame_pending() while
/// handling the \ref SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND event.
///
/// This event must be turned on by the user and will fire whenever a data
/// request is being received so that the stack can determine if there
/// is pending data. Note that if the default Frame Pending bit needs to
/// be changed, it must be done quickly. Otherwise, the Ack may already
/// have been transmitted with the default setting. Check the return code of \ref
/// sl_rail_ieee802154_toggle_frame_pending() to be sure that the bit was changed
/// in time.
///
/// Transmit and receive operations are done using the standard RAIL APIs in
/// IEEE 802.15.4 mode. To send packets using the correct CSMA configuration,
/// use \ref SL_RAIL_CSMA_CONFIG_802_15_4_2003_2P4_GHZ_OQPSK_CSMA define
/// that can initialize the csma_config structure passed to \ref
/// sl_rail_start_cca_csma_tx().
/// @{
///

/**
 * @enum sl_rail_ieee802154_address_length_t
 * @brief Different lengths that an 802.15.4 address can have
 */
SLI_RAIL_ENUM(sl_rail_ieee802154_address_length_t) {
  /**
   * 2 byte short address.
   */
  SL_RAIL_IEEE802154_SHORT_ADDRESS = 2,
  /**
   * 8 byte extended address.
   */
  SL_RAIL_IEEE802154_LONG_ADDRESS = 3,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_IEEE802154_SHORT_ADDRESS ((sl_rail_ieee802154_address_length_t) SL_RAIL_IEEE802154_SHORT_ADDRESS)
#define SL_RAIL_IEEE802154_LONG_ADDRESS  ((sl_rail_ieee802154_address_length_t) SL_RAIL_IEEE802154_LONG_ADDRESS)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @struct sl_rail_ieee802154_address_t
 * @brief Representation of 802.15.4 address
 *
 * This structure is only used for received source address information
 * needed to perform Frame Pending lookup.
 */
typedef struct sl_rail_ieee802154_address {
  /**
   * Convenient storage for different address types.
   */
  union {
    /**
     * Present for 2 byte addresses.
     */
    uint16_t short_address;
    /**
     * Present for 8 byte addresses.
     */
    uint8_t long_address[8];
  };
  /**
   * Enumeration of the received address length.
   */
  sl_rail_ieee802154_address_length_t address_length;
  /**
   * A bitmask representing which address filter(s) this packet has passed.
   * It is undefined on platforms lacking \ref SL_RAIL_SUPPORTS_ADDR_FILTER_MASK.
   */
  sl_rail_addr_filter_mask_t filter_mask;
} sl_rail_ieee802154_address_t;

/**
 * The maximum number of allowed addresses of each type.
 */
#define SL_RAIL_IEEE802154_MAX_ADDRESSES (3U)

/**
 * @struct sl_rail_ieee802154_addr_config_t
 * @brief A configuration structure for IEEE 802.15.4 Address Filtering.
 *
 * This structure allows configuration of multi-PAN functionality by specifying
 * multiple PAN Ids and short addresses. A packet will be received if it matches
 * an address and its corresponding PAN Id. Long address 0 and short address 0
 * match against PAN Id 0, etc. The broadcast PAN Id and address will work with
 * any address or PAN Id, respectively.
 *
 * @note The broadcast addresses are handled separately and do not need to be
 *   specified here. Any address to be ignored should be set with all bits high.
 */
typedef struct sl_rail_ieee802154_addr_config {
  /**
   * PAN Ids for destination filtering. All must be specified.
   * To disable a PAN Id, set it to the broadcast value, 0xFFFF.
   */
  uint16_t pan_id[SL_RAIL_IEEE802154_MAX_ADDRESSES];
  /**
   * A short network addresses for destination filtering. All must be specified.
   * To disable a short address, set it to the broadcast value, 0xFFFF.
   */
  uint16_t short_addr[SL_RAIL_IEEE802154_MAX_ADDRESSES];
  /**
   * A 64-bit address for destination filtering. All must be specified.
   * This field is parsed in over-the-air byte order. To disable a long
   * address, set it to the reserved value of 0x00 00 00 00 00 00 00 00.
   */
  uint8_t long_addr[SL_RAIL_IEEE802154_MAX_ADDRESSES][8];
} sl_rail_ieee802154_addr_config_t;

/**
 * @struct sl_rail_ieee802154_config_t
 * @brief A configuration structure for IEEE 802.15.4 in RAIL.
 */
typedef struct sl_rail_ieee802154_config {
  /**
   * Configure the RAIL Address Filter to allow the given destination
   * addresses. If this pointer is NULL, defer destination address configuration.
   * If a member of addresses is NULL, defer configuration of just that member.
   * This can be overridden via \ref sl_rail_ieee802154_set_addresses(), or the
   * individual members can be changed via \ref sl_rail_ieee802154_set_pan_id(), \ref
   * sl_rail_ieee802154_set_short_address(), and \ref sl_rail_ieee802154_set_long_address().
   */
  const sl_rail_ieee802154_addr_config_t *p_addresses;
  /**
   * Define the Acking configuration for the IEEE 802.15.4 implementation.
   */
  sl_rail_auto_ack_config_t ack_config;
  /**
   * Define state timings for the IEEE 802.15.4 implementation.
   */
  sl_rail_state_timing_t timings;
  /**
   * Set which 802.15.4 frame types will be received, of Beacon, Data, Ack, and
   * Command. This setting can be overridden via \ref sl_rail_ieee802154_accept_frames().
   */
  uint8_t frames_mask;
  /**
   * Enable promiscuous mode during configuration. This can be overridden via
   * \ref sl_rail_ieee802154_set_promiscuous_mode() afterwards.
   */
  bool promiscuous_mode;
  /**
   * Set whether the device is a PAN Coordinator during configuration. This can
   * be overridden via \ref sl_rail_ieee802154_set_pan_coordinator() afterwards.
   */
  bool is_pan_coordinator;
  /**
   * The default value for the Frame Pending bit in outgoing Acks for packets
   * that triggered the \ref SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND event.
   * Such an Ack's Frame Pending bit can be inverted if necessary during the
   * handling of that event by calling \ref sl_rail_ieee802154_toggle_frame_pending().
   */
  bool default_frame_pending_in_outgoing_acks;
} sl_rail_ieee802154_config_t;

/**
 * RX channel switching buffer size, in bytes.
 */
#ifdef  SLI_LIBRARY_BUILD
#define SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_BUF_BYTES (704U) // largest of below
#else//!SLI_LIBRARY_BUILD
#if     (_SILICON_LABS_32B_SERIES_2_CONFIG == 1)
#define SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_BUF_BYTES (704U)
#elif   ((_SILICON_LABS_32B_SERIES_2_CONFIG == 4) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6))
#define SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_BUF_BYTES (640U)
#else// RX channel switching not supported and series 3 do not use this parameter
#define SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_BUF_BYTES (0U)
#endif
#endif//SLI_LIBRARY_BUILD

/**
 * Fixed-width type indicating the needed alignment for RX channel switching buffer.
 */
#define SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_BUF_ALIGNMENT_TYPE     uint32_t

/**
 * Alignment that is needed for RX channel switching buffer.
 */
#define SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_BUF_ALIGNMENT          (sizeof(SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_BUF_ALIGNMENT_TYPE))

/**
 * Maximum numbers of channels supported for RX channel switching
 */
#define SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_NUM_CHANNELS           (2U)

/**
 * @struct sl_rail_ieee802154_rx_channel_switching_cfg_t
 * @brief A configuration structure for RX channel switching.
 */
typedef struct sl_rail_ieee802154_rx_channel_switching_cfg {
  /**
   * Pointer to contiguous global read-write memory that will be used
   * by RAIL to store channel specific settings for concurrent listening,
   * for series 2 parts.
   * It need not be initialized and applications should never write
   * data anywhere in this buffer.
   * This parameter is not used for series 3 parts.
   *
   * @note the size of this buffer must be at least as large as the
   *   \ref SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_BUF_BYTES and needs to be word
   *   aligned.
   */
  SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_BUF_ALIGNMENT_TYPE *p_buffer;
  /**
   * For series 2 parts, this parameter must be set to the length of the buffer
   * array, in bytes. This way, during configuration, the software can confirm
   * it's writing within the range of the buffer.
   * The configuration API will return an error if buffer_bytes is insufficient.
   * This parameter is not used for series 3 parts.
   */
  uint16_t buffer_bytes;
  /**
   * Array to hold the channel numbers for RX channel switching.
   * @note Radio will switch between the exact channels specified, and not
   *   across an inclusive range of channels between the specified channels.
   */
  uint16_t channels[SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_NUM_CHANNELS];
} sl_rail_ieee802154_rx_channel_switching_cfg_t;

///
/// @addtogroup IEEE802154_PHY IEEE 802.15.4 Radio Configurations
/// Radio configurations for the RAIL 802.15.4 Accelerator
///
/// These radio configurations are used to configure 802.15.4 when a function
/// such as \ref sl_rail_ieee802154_config_2p4_ghz_radio() is called. Each radio
/// configuration listed below is compiled into the RAIL library as a weak
/// symbol that will take into account per-die defaults. If the board
/// configuration in use has different settings than the default, such as a
/// different radio subsystem clock frequency, these radio configurations can
/// be overridden to account for those settings.
/// @{
///

/**
 * Default PHY to use for 2.4 GHz 802.15.4. Will be NULL if
 * \ref SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154 or \ref SL_RAIL_SUPPORTS_2P4_GHZ_BAND
 * is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ieee802154_phy_2p4_ghz;

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * Default PHY to use for 1 Mbps 2.4 GHz 802.15.4 with forward error correction.
 * Will be NULL if \ref SL_RAIL_IEEE802154_SUPPORTS_2_MBPS_PHY is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ieee802154_phy_2p4_ghz_1_mbps_fec;

/**
 * Default PHY to use for 2 Mbps 2.4 GHz 802.15.4. Will be NULL if
 * \ref SL_RAIL_IEEE802154_SUPPORTS_2_MBPS_PHY is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ieee802154_phy_2p4_ghz_2_mbps;

/**
 * Default PHY to use for 1 Mbps 2.4 GHz 802.15.4 with forward error correction and RX channel switching.
 * Will be NULL if \ref SL_RAIL_IEEE802154_SUPPORTS_2_MBPS_PHY is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ieee802154_phy_2p4_ghz_fcs_1_mbps_fec;

/**
 * Default PHY to use for 2 Mbps 2.4 GHz 802.15.4 with RX channel switching. Will be NULL if
 * \ref SL_RAIL_IEEE802154_SUPPORTS_2_MBPS_PHY is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ieee802154_phy_2p4_ghz_fcs_2_mbps;
#endif //DOXYGEN_UNDOCUMENTED

/**
 * Default PHY to use for 2.4 GHz 802.15.4 with antenna diversity. Will be NULL
 * if \ref SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154, \ref SL_RAIL_SUPPORTS_2P4_GHZ_BAND, or
 * \ref SL_RAIL_SUPPORTS_ANTENNA_DIVERSITY is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ieee802154_phy_2p4_ghz_ant_div;

/**
 * Default PHY to use for 2.4 GHz 802.15.4 optimized for coexistence. Will be
 * NULL if \ref SL_RAIL_IEEE802154_SUPPORTS_COEX_PHY is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ieee802154_phy_2p4_ghz_coex;

/**
 * Default PHY to use for 2.4 GHz 802.15.4 optimized for coexistence, while
 * supporting antenna diversity. Will be NULL if
 * \ref SL_RAIL_SUPPORTS_ANTENNA_DIVERSITY or
 * \ref SL_RAIL_IEEE802154_SUPPORTS_COEX_PHY is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ieee802154_phy_2p4_ghz_ant_div_coex;

/**
 * Default PHY to use for 2.4 GHz 802.15.4 with a configuration that supports a
 * front-end module. Will be NULL if
 * \ref SL_RAIL_IEEE802154_SUPPORTS_FEM_PHY is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ieee802154_phy_2p4_ghz_fem;

/**
 * Default PHY to use for 2.4 GHz 802.15.4 with a configuration that supports a
 * front-end module and antenna diversity. Will be NULL if
 * \ref SL_RAIL_IEEE802154_SUPPORTS_FEM_PHY or \ref SL_RAIL_SUPPORTS_ANTENNA_DIVERSITY
 * is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ieee802154_phy_2p4_ghz_ant_div_fem;

/**
 * Default PHY to use for 2.4 GHz 802.15.4 with a configuration that supports a
 * front-end module and is optimized for radio coexistence. Will be NULL if
 * \ref SL_RAIL_IEEE802154_SUPPORTS_FEM_PHY or
 * \ref SL_RAIL_IEEE802154_SUPPORTS_COEX_PHY is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ieee802154_phy_2p4_ghz_coex_fem;

/**
 * Default PHY to use for 2.4 GHz 802.15.4 with a configuration that supports a
 * front-end module and antenna diversity, and is optimized for radio
 * coexistence. Will be NULL if \ref SL_RAIL_IEEE802154_SUPPORTS_FEM_PHY,
 * \ref SL_RAIL_IEEE802154_SUPPORTS_COEX_PHY, or
 * \ref SL_RAIL_SUPPORTS_ANTENNA_DIVERSITY is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ieee802154_phy_2p4_ghz_ant_div_coex_fem;

/**
 * Default PHY to use for 863 MHz GB868 802.15.4. Will be NULL if
 * \ref SL_RAIL_IEEE802154_SUPPORTS_G_SUBSET_GB868 is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ieee802154_phy_gb863_mhz;

/**
 * Default PHY to use for 915 MHz GB868 802.15.4. Will be NULL if
 * \ref SL_RAIL_IEEE802154_SUPPORTS_G_SUBSET_GB868 is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ieee802154_phy_gb915_mhz;

/**
 * Default PHY to use for 2.4 GHz 802.15.4 with RX channel switching. Will be
 * NULL if \ref SL_RAIL_IEEE802154_SUPPORTS_RX_CHANNEL_SWITCHING is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ieee802154_phy_2p4_ghz_rx_ch_switching;

/**
 * Default PHY to use for 2.4 GHz 802.15.4 with duty cycling. Will be NULL if
 * \ref SL_RAIL_SUPPORTS_RX_DUTY_CYCLING is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ieee802154_phy_2p4_ghz_rx_duty_cycling;

/// @} // End of group IEEE802154_PHY

/**
 * Initialize RAIL for IEEE802.15.4 features.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_config A non-NULL pointer to an IEEE802154 configuration structure.
 * @return Status code indicating success of the function call.
 *
 * This function calls the following RAIL functions to configure the radio for
 * IEEE802.15.4 features.
 *
 * Initializes the following:
 *   - Enables IEEE802154 hardware acceleration
 *   - Configures RAIL Auto Ack functionality
 *   - Configures RAIL Address Filter for 802.15.4 address filtering
 *
 * It saves having to call the following functions individually:
 * - \ref sl_rail_config_auto_ack()
 * - \ref sl_rail_set_rx_transitions()
 * - \ref sl_rail_set_tx_transitions()
 * - \ref sl_rail_write_auto_ack_fifo()
 * - \ref sl_rail_set_state_timing()
 * - \ref sl_rail_config_address_filter()
 * - \ref sl_rail_enable_address_filter()
 *
 * It must be called before most of the sl_rail_ieee802154_* functions.
 */
sl_rail_status_t sl_rail_ieee802154_init(sl_rail_handle_t rail_handle,
                                         const sl_rail_ieee802154_config_t *p_config);

/**
 * Configure the radio for 2.4 GHz 802.15.4 operation.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This initializes the radio for 2.4 GHz operation. It takes the place of
 * calling \ref sl_rail_config_channels(). After this call,
 * channels 11-26 will be available, giving the frequencies of those channels
 * on channel page 0, as defined by IEEE 802.15.4-2011 section 8.1.2.2.
 *
 * @note This call implicitly disables all \ref sl_rail_ieee802154_g_options_t.
 */
sl_rail_status_t sl_rail_ieee802154_config_2p4_ghz_radio(sl_rail_handle_t rail_handle);

/**
 * Configure the radio for 2.4 GHz 802.15.4 operation with antenna diversity.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This initializes the radio for 2.4 GHz operation, but with a configuration
 * that supports antenna diversity. It takes the place of
 * calling \ref sl_rail_config_channels(). After this call,
 * channels 11-26 will be available, giving the frequencies of those channels
 * on channel page 0, as defined by IEEE 802.15.4-2011 section 8.1.2.2.
 *
 * @note This call implicitly disables all \ref sl_rail_ieee802154_g_options_t.
 */
sl_rail_status_t sl_rail_ieee802154_config_2p4_ghz_radio_ant_div(sl_rail_handle_t rail_handle);

/**
 * Configure the radio for 2.4 GHz 802.15.4 operation with antenna diversity
 * optimized for radio coexistence.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This initializes the radio for 2.4 GHz operation, but with a configuration
 * that supports antenna diversity optimized for radio coexistence. It takes
 * the place of calling \ref sl_rail_config_channels(). After this call,
 * channels 11-26 will be available, giving the frequencies of those channels
 * on channel page 0, as defined by IEEE 802.15.4-2011 section 8.1.2.2.
 *
 * @note This call implicitly disables all \ref sl_rail_ieee802154_g_options_t.
 */
sl_rail_status_t sl_rail_ieee802154_config_2p4_ghz_radio_ant_div_coex(sl_rail_handle_t rail_handle);

/**
 * Configure the radio for 2.4 GHz 802.15.4 operation optimized for radio coexistence.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This initializes the radio for 2.4 GHz operation, but with a configuration
 * that supports radio coexistence. It takes the place of
 * calling \ref sl_rail_config_channels(). After this call,
 * channels 11-26 will be available, giving the frequencies of those channels
 * on channel page 0, as defined by IEEE 802.15.4-2011 section 8.1.2.2.
 *
 * @note This call implicitly disables all \ref sl_rail_ieee802154_g_options_t.
 */
sl_rail_status_t sl_rail_ieee802154_config_2p4_ghz_radio_coex(sl_rail_handle_t rail_handle);

/**
 * Configure the radio for 2.4 GHz 802.15.4 operation with a front end module.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This initializes the radio for 2.4 GHz operation, but with a configuration
 * that supports a front end module. It takes the place of
 * calling \ref sl_rail_config_channels(). After this call,
 * channels 11-26 will be available, giving the frequencies of those channels
 * on channel page 0, as defined by IEEE 802.15.4-2011 section 8.1.2.2.
 *
 * @note This call implicitly disables all \ref sl_rail_ieee802154_g_options_t.
 */
sl_rail_status_t sl_rail_ieee802154_config_2p4_ghz_radio_fem(sl_rail_handle_t rail_handle);

/**
 * Configure the radio for 2.4 GHz 802.15.4 operation with antenna diversity
 * optimized for a front end module.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This initializes the radio for 2.4 GHz operation, but with a configuration
 * that supports antenna diversity and a front end module. It takes the place of
 * calling \ref sl_rail_config_channels(). After this call,
 * channels 11-26 will be available, giving the frequencies of those channels
 * on channel page 0, as defined by IEEE 802.15.4-2011 section 8.1.2.2.
 *
 * @note This call implicitly disables all \ref sl_rail_ieee802154_g_options_t.
 */
sl_rail_status_t sl_rail_ieee802154_config_2p4_ghz_radio_ant_div_fem(sl_rail_handle_t rail_handle);

/**
 * Configure the radio for 2.4 GHz 802.15.4 operation optimized for radio coexistence
 * and a front end module.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This initializes the radio for 2.4 GHz operation, but with a configuration
 * that supports radio coexistence and a front end module. It takes the place of
 * calling \ref sl_rail_config_channels(). After this call,
 * channels 11-26 will be available, giving the frequencies of those channels
 * on channel page 0, as defined by IEEE 802.15.4-2011 section 8.1.2.2.
 *
 * @note This call implicitly disables all \ref sl_rail_ieee802154_g_options_t.
 */
sl_rail_status_t sl_rail_ieee802154_config_2p4_ghz_radio_coex_fem(sl_rail_handle_t rail_handle);

/**
 * Configure the radio for 2.4 GHz 802.15.4 operation with antenna diversity
 * optimized for radio coexistence and a front end module.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This initializes the radio for 2.4 GHz operation, but with a configuration
 * that supports antenna diversity, radio coexistence and a front end module.
 * It takes the place of calling \ref sl_rail_config_channels().
 * After this call, channels 11-26 will be available, giving the frequencies of
 * those channels on channel page 0, as defined by IEEE 802.15.4-2011 section 8.1.2.2.
 *
 * @note This call implicitly disables all \ref sl_rail_ieee802154_g_options_t.
 */
sl_rail_status_t sl_rail_ieee802154_config_2p4_ghz_radio_ant_div_coex_fem(sl_rail_handle_t rail_handle);

#ifndef DOXYGEN_UNDOCUMENTED

/**
 * Time in microseconds to listen for a packet on the 2 Mbps channel.
 * This timeout can be configured at runtime using \ref sl_rail_ieee802154_config_2_mbps_rx_timeout().
 */
#define SL_RAIL_IEEE802154_2_MBPS_RECEIVE_TIMEOUT_US 1500UL

/**
 * @brief Enables high data rate mode switching for 2.4 GHz IEEE 802.15.4.
 *
 * Enables switching between the standard 2.4 GHz 802.15.4 PHY and a proprietary
 * 2.4 GHz high data rate PHY.
 *
 * **Note:** This function does not modify or load the channel configuration.
 * It is required that a channel configuration supporting high data rate mode
 * switching has already been loaded before calling this API.
 *
 * @param[in] rail_handle Handle to a valid RAIL instance.
 *
 * @return
 *   - \ref SL_RAIL_STATUS_NO_ERROR The operation completed successfully.
 *   - \ref SL_RAIL_STATUS_INVALID_PARAMETER The rail_handle is invalid or a suitable
 *     channel configuration has not been loaded.
 *   - Other status codes as defined by sl_rail_status_t.
 *
 * See \ref sl_rail_ieee802154_config_rx_channel_switching() for configuring
 * switching between the normal 802.15.4 PHY and the high data rate PHY.
 */
sl_rail_status_t sl_rail_ieee802154_enable_2p4_ghz_high_data_rate(sl_rail_handle_t rail_handle);

/**
 * Configure the radio for 2.4 GHz 802.15.4 for 250 kbps or 2 Mbps operation.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This initializes the radio for 2.4 GHz high speed operation.
 * It takes the place of calling \ref sl_rail_config_channels().
 * After this call, channels 11-26 will be available, giving the frequencies of
 * those channels on channel page 0, as defined by IEEE 802.15.4-2011 section 8.1.2.2
 * at 250 kbps.
 * Channels 11-26 will support transmitting and receiving using dual sync words.
 * Channels 27-42 will transmit and receive on the frequency of [channel - 16] at 2 Mbps.
 * Auto-ack and address filtering are disabled when channels 27-42 are selected.
 *
 * @note This call implicitly disables all \ref sl_rail_ieee802154_g_options_t.
 *   This API can be safely used with 802.15.4/BLE DMP but not 802.15.4/802.15.4 DMP.
 */
sl_rail_status_t sl_rail_ieee802154_config_2p4_ghz_radio_2_mbps(sl_rail_handle_t rail_handle);

/**
 * Configure the radio for 2.4 GHz 802.15.4 for 250 kbps or 1 Mbps forward error correction
 * operation.
 *
 * @param[in] rail_handle A real handle of RAIL instance.
 * @return Status code indicating success of the function call.
 *
 * This initializes the radio for 2.4 GHz high speed operation.
 * It takes the place of calling \ref sl_rail_config_channels().
 * After this call, channels 11-26 will be available, giving the frequencies of
 * those channels on channel page 0, as defined by IEEE 802.15.4-2011 section 8.1.2.2
 * at 250 kbps.
 * Channels 11-26 will support transmitting and receiving using dual sync words.
 * Channels 27-42 will transmit and receive on the frequency of [channel - 16] at 1 Mbps.
 * Auto-ack and address filtering are disabled when channels 27-42 are selected.
 *
 * @note This call implicitly disables all \ref sl_rail_ieee802154_g_options_t.
 *   This API can be safely used with 802.15.4/BLE DMP but not 802.15.4/802.15.4 DMP.
 */
sl_rail_status_t sl_rail_ieee802154_config_2p4_ghz_radio_1_mbps_fec(sl_rail_handle_t rail_handle);

/**
 * Configure the radio for 2.4 GHz 802.15.4 for 250 kbps or 2 Mbps operation.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This initializes the radio for 2.4 GHz high speed operation.
 * It takes the place of calling \ref sl_rail_config_channels().
 * After this call, channels 11-26 will be available, giving the frequencies of
 * those channels on channel page 0, as defined by IEEE 802.15.4-2011 section 8.1.2.2
 * at 250 kbps.
 * Channels 11-26 will support transmitting and receiving using dual sync words.
 * Channels 27-42 will transmit and receive on the frequency of [channel - 16] at 2 Mbps.
 * Auto-ack and address filtering are disabled when channels 27-42 are selected.
 *
 * @note This call implicitly disables all \ref sl_rail_ieee802154_g_options_t.
 *   This API can be safely used with 802.15.4/BLE DMP but not 802.15.4/802.15.4 DMP.
 */
sl_rail_status_t sl_rail_ieee802154_config_2p4_ghz_radio_fcs_2_mbps(sl_rail_handle_t rail_handle);

/**
 * Configure the radio for 2.4 GHz 802.15.4 for 250 kbps or 1 Mbps forward error correction
 * operation.
 *
 * @param[in] rail_handle A real handle of RAIL instance.
 * @return Status code indicating success of the function call.
 *
 * This initializes the radio for 2.4 GHz high speed operation.
 * It takes the place of calling \ref sl_rail_config_channels().
 * After this call, channels 11-26 will be available, giving the frequencies of
 * those channels on channel page 0, as defined by IEEE 802.15.4-2011 section 8.1.2.2
 * at 250 kbps.
 * Channels 11-26 will support transmitting and receiving using dual sync words.
 * Channels 27-42 will transmit and receive on the frequency of [channel - 16] at 1 Mbps.
 * Auto-ack and address filtering are disabled when channels 27-42 are selected.
 *
 * @note This call implicitly disables all \ref sl_rail_ieee802154_g_options_t.
 *   This API can be safely used with 802.15.4/BLE DMP but not 802.15.4/802.15.4 DMP.
 */
sl_rail_status_t sl_rail_ieee802154_config_2p4_ghz_radio_fcs_1_mbps_fec(sl_rail_handle_t rail_handle);

/**
 * Configure the 802.15.4 2 Mbps mode switch receive channel.
 *
 * @param[in] rail_handle A RAIL instance handle.
 * @param[in] channel 802.15.4 channel(11-26) to listen for a mode switch packet.
 * @return Status code indicating success of the function call.
 *
 * Mode switch to the 2 Mbps radio config will only occur on the provided channel.
 * Mode switch packets received on other channels will be ignored.
 */
sl_rail_status_t sl_rail_ieee802154_config_2_mbps_rx_channel(sl_rail_handle_t rail_handle,
                                                             uint16_t channel);

/**
 * Configure the 802.15.4 2 Mbps receive timeout.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] timeout Time to listen for a packet on the 2 Mbps channel
 *   before switching back to the 250 kbps channel.
 * @return Status code indicating success of the function call.
 *
 * @note By default the 2 Mbps receive timeout is \ref SL_RAIL_IEEE802154_2_MBPS_RECEIVE_TIMEOUT_US.
 */
sl_rail_status_t sl_rail_ieee802154_config_2_mbps_rx_timeout(sl_rail_handle_t rail_handle,
                                                             sl_rail_time_t timeout);

#endif //DOXYGEN_UNDOCUMENTED

/**
 * Configure the radio for Sub-GHz GB868 863 MHz 802.15.4 operation.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This initializes the radio for Sub-GHz GB868 863 MHz operation. It takes the
 * place of calling \ref sl_rail_config_channels().
 * After this call, GB868 channels in the 863 MHz band (channel pages 28, 29,
 * and 30 -- logical channels 0x80..0x9A, 0xA0..0xA8, 0xC0..0xDA, respectively)
 * will be available, as defined by Rev 22 of the Zigbee Specification, 2017
 * document 05-3474-22, section D.10.2.1.3.2.
 *
 * @note This call implicitly enables \ref SL_RAIL_IEEE802154_G_OPTION_GB868.
 */
sl_rail_status_t sl_rail_ieee802154_config_gb863_mhz_radio(sl_rail_handle_t rail_handle);

/**
 * Configure the radio for Sub-GHz GB868 915 MHz 802.15.4 operation.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This initializes the radio for Sub-GHz GB868 915 MHz operation. It takes the
 * place of calling \ref sl_rail_config_channels().
 * After this call, GB868 channels in the 915 MHz band (channel page 31 --
 * logical channels 0xE0..0xFA) will be available, as defined by Rev 22 of
 * the Zigbee Specification, 2017 document 05-3474-22, section D.10.2.1.3.2.
 *
 * @note This call implicitly enables \ref SL_RAIL_IEEE802154_G_OPTION_GB868.
 */
sl_rail_status_t sl_rail_ieee802154_config_gb915_mhz_radio(sl_rail_handle_t rail_handle);

/**
 * De-initialize IEEE802.15.4 hardware acceleration.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * Disables and resets all IEE802.15.4 hardware acceleration features. This
 * function should only be called when the radio is IDLE. This calls the
 * following:
 * - \ref sl_rail_set_state_timing(), to reset all timings to 100 us
 * - \ref sl_rail_enable_address_filter() passing false for its enable parameter
 * - \ref sl_rail_reset_address_filter()
 */
sl_rail_status_t sl_rail_ieee802154_deinit(sl_rail_handle_t rail_handle);

/**
 * Return whether IEEE802.15.4 hardware acceleration is currently enabled.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return true if IEEE802.15.4 hardware acceleration was enabled to start with
 *   and false otherwise.
 */
bool sl_rail_ieee802154_is_enabled(sl_rail_handle_t rail_handle);

/**
 * @enum sl_rail_ieee802154_phy_t
 * @brief The 802.15.4 PHY identifier (aka PTI radio configuration id).
 */
SLI_RAIL_ENUM(sl_rail_ieee802154_phy_t) {
  /**
   * Built-in 2.4 GHz 802.15.4 radio configuration.
   */
  SL_RAIL_IEEE802154_PHY_2P4_GHZ = 0x00u,
  /**
   * Built-in 2.4 GHz 802.15.4 radio configuration
   * with RX antenna diversity support.
   */
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_ANT_DIV = 0x01u,
  /**
   * Built-in 2.4 GHz 802.15.4 radio configuration
   * optimized for radio coexistence.
   */
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_COEX = 0x02u,
  /**
   * Built-in 2.4 GHz 802.15.4 radio configuration with
   * RX antenna diversity support optimized for radio coexistence.
   */
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_ANT_DIV_COEX = 0x03u,
  /**
   * Built-in 2.4 GHz 802.15.4 radio configuration
   * optimized for front end modules.
   */
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM = 0x08u,
  /**
   * Built-in 2.4 GHz 802.15.4 radio configuration
   * with RX antenna diversity support optimized for
   * front end modules.
   */
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_ANT_DIV = 0x09u,
  /**
   * Built-in 2.4 GHz 802.15.4 radio configuration
   * optimized for radio coexistence and front end modules.
   */
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_COEX = 0x0Au,
  /**
   * Built-in 2.4 GHz 802.15.4 radio configuration with
   * RX antenna diversity support optimized for radio coexistence
   * and front end modules.
   */
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_ANT_DIV_COEX = 0x0Bu,
  /**
   * 2Mbps 2.4 GHz operation (1-byte PHR).
   * Built-in 2.4 GHz 802.15.4 radio configuration with
   * mode switch to 2Mbps operation (1-byte PHR).
   */
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_2_MBPS = 0x0Du,
  /**
   * Built-in 2.4 GHz 802.15.4 radio configuration with
   * RX channel switching support optimized.
   */
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_RX_CH_SWITCHING = 0x0Eu,
  /**
   * Built-in 2.4 GHz 802.15.4 radio configuration with
   * mode switch to 1Mbps 2.4 GHz operation with forward error correction (1-byte PHR).
   */
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_1_MBPS_FEC = 0x0Fu,
  /**
   * Built-in 2.4 GHz 802.15.4 radio configuration with RX channel switching
   * and 2Mbps 2.4 GHz operation (1-byte PHR).
   */
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_FCS_2_MBPS = 0x10U,
  /**
   * Built-in 2.4 GHz 802.15.4 radio configuration with RX channel switching
   * and 1Mbps 2.4 GHz operation with forward error correction (1-byte PHR).
   */
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_FCS_1_MBPS_FEC = 0x11U,
  /**
   * Built-in 2.4 GHz 802.15.4 radio configuration with duty cycling.
   */
  SL_RAIL_IEEE802154_PHY_2P4_GHZ_RX_DUTY_CYCLING = 0x12U,
  /**
   * Built-in 863 MHz GB868 802.15.4 radio configuration.
   */
  SL_RAIL_IEEE802154_PHY_863_MHZ_GB868 = 0x85u,
  /**
   * Built-in 915 MHz GB868 802.15.4 radio configuration.
   */
  SL_RAIL_IEEE802154_PHY_915_MHZ_GB868 = 0x86u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_IEEE802154_PHY_2P4_GHZ                  ((sl_rail_ieee802154_phy_t) SL_RAIL_IEEE802154_PHY_2P4_GHZ)
#define SL_RAIL_IEEE802154_PHY_2P4_GHZ_ANT_DIV          ((sl_rail_ieee802154_phy_t) SL_RAIL_IEEE802154_PHY_2P4_GHZ_ANT_DIV)
#define SL_RAIL_IEEE802154_PHY_2P4_GHZ_COEX             ((sl_rail_ieee802154_phy_t) SL_RAIL_IEEE802154_PHY_2P4_GHZ_COEX)
#define SL_RAIL_IEEE802154_PHY_2P4_GHZ_ANT_DIV_COEX     ((sl_rail_ieee802154_phy_t) SL_RAIL_IEEE802154_PHY_2P4_GHZ_ANT_DIV_COEX)
#define SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM              ((sl_rail_ieee802154_phy_t) SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM)
#define SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_ANT_DIV      ((sl_rail_ieee802154_phy_t) SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_ANT_DIV)
#define SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_COEX         ((sl_rail_ieee802154_phy_t) SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_COEX)
#define SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_ANT_DIV_COEX ((sl_rail_ieee802154_phy_t) SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_ANT_DIV_COEX)
#define SL_RAIL_IEEE802154_PHY_2P4_GHZ_2_MBPS           ((sl_rail_ieee802154_phy_t) SL_RAIL_IEEE802154_PHY_2P4_GHZ_2_MBPS)
#define SL_RAIL_IEEE802154_PHY_2P4_GHZ_RX_CH_SWITCHING  ((sl_rail_ieee802154_phy_t) SL_RAIL_IEEE802154_PHY_2P4_GHZ_RX_CH_SWITCHING)
#define SL_RAIL_IEEE802154_PHY_2P4_GHZ_1_MBPS_FEC       ((sl_rail_ieee802154_phy_t) SL_RAIL_IEEE802154_PHY_2P4_GHZ_1_MBPS_FEC)
#define SL_RAIL_IEEE802154_PHY_2P4_GHZ_FCS_2_MBPS       ((sl_rail_ieee802154_phy_t) SL_RAIL_IEEE802154_PHY_2P4_GHZ_FCS_2_MBPS)
#define SL_RAIL_IEEE802154_PHY_2P4_GHZ_FCS_1_MBPS_FEC   ((sl_rail_ieee802154_phy_t) SL_RAIL_IEEE802154_PHY_2P4_GHZ_FCS_1_MBPS_FEC)
#define SL_RAIL_IEEE802154_PHY_2P4_GHZ_RX_DUTY_CYCLING  ((sl_rail_ieee802154_phy_t) SL_RAIL_IEEE802154_PHY_2P4_GHZ_RX_DUTY_CYCLING)
#define SL_RAIL_IEEE802154_PHY_863_MHZ_GB868            ((sl_rail_ieee802154_phy_t) SL_RAIL_IEEE802154_PHY_863_MHZ_GB868)
#define SL_RAIL_IEEE802154_PHY_915_MHZ_GB868            ((sl_rail_ieee802154_phy_t) SL_RAIL_IEEE802154_PHY_915_MHZ_GB868)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * PHY id shift of extension for stackinfo.phyId
 */
#define SL_RAIL_IEEE802154_PHY_EXTENSION_SHIFT     (8)
/**
 * PHY id bitmask of extension for stackinfo.phyId
 */
#define SL_RAIL_IEEE802154_PHY_EXTENSION_MASK      (0xFF00U)
/**
 * PHY id bitmask of 2-byte PHR length flag
 * for use with indicated radio configurations.
 */
#define SL_RAIL_IEEE802154_PHY_2BYTE_PHR_MASK      (0x80U)
/**
 * PHY id bitmask for external (non-built-in) radio configuration.
 */
#define SL_RAIL_IEEE802154_PHY_EXTERNAL_MASK       (0x10U)

/**
 * Load an 802.15.4 channel config.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param p_channel_config A pointer to a channel configuration.
 * @param phy_id PTI config ID.
 * @return Status code indicating success of the function call.
 *
 * This initializes the radio for 802.15.4 operation. It takes the place of
 * calling \ref sl_rail_config_channels(). After this call,
 * channels 11-26 will be available, giving the frequencies of those channels
 * on channel page 0, as defined by IEEE 802.15.4-2011 section 8.1.2.2.
 * The channelConfig can potentially define additional channels outside of the
 * range 11-26.
 *
 * @note This call implicitly disables all \ref sl_rail_ieee802154_g_options_t.
 */
sl_rail_status_t sl_rail_ieee802154_config_channels(sl_rail_handle_t rail_handle,
                                                    const sl_rail_channel_config_t *p_channel_config,
                                                    sl_rail_ieee802154_phy_t phy_id);

/**
 * Return IEEE802.15.4 PHY identifier.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return PHY identifier (aka PTI radio config id).
 */
sl_rail_ieee802154_phy_t sl_rail_ieee802154_get_phy_id(sl_rail_handle_t rail_handle);

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * Set IEEE802.15.4 PHY identifier (for Silicon Labs internal use only).
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] phy_id PHY identifier (aka PTI radio config id).
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_ieee802154_set_phy_id(sl_rail_handle_t rail_handle,
                                               sl_rail_ieee802154_phy_t phy_id);
#endif//DOXYGEN_UNDOCUMENTED

/**
 * @typedef sl_rail_ieee802154_phy_features_t
 * @brief The 802.15.4 PHY feature bitmask.
 */
typedef uint32_t sl_rail_ieee802154_phy_features_t;
/**
 * Shift position of \ref SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_ANT_DIV bit.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_ANT_DIV_SHIFT (0x01u)
/**
 * Shift position of \ref SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_COEX bit.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_COEX_SHIFT (0x02u)
/**
 * Shift position of \ref SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FEM bit.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FEM_SHIFT (0x03u)
/**
 * Shift position of \ref SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_2_MBPS bit.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_2_MBPS_SHIFT (0x04u)
/**
 * Shift position of \ref SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_1_MBPS_FEC bit.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_1_MBPS_FEC_SHIFT (0x05u)
/**
 * Shift position of \ref SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_CH_SWITCHING bit.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_CH_SWITCHING_SHIFT (0x06u)
/**
 * Shift position of \ref SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_DUTY_CYCLING bit.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_DUTY_CYCLING_SHIFT (0x07u)
/**
 * Shift position of \ref SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FCS_2_MBPS bit.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FCS_2_MBPS_SHIFT (0x08u)
/**
 * Shift position of \ref SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FCS_1_MBPS_FEC bit.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FCS_1_MBPS_FEC_SHIFT (0x09u)

/**
 * No IEEE802.15.4 features selected.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ (0UL)

/**
 * RX antenna diversity feature.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_ANT_DIV (1UL << SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_ANT_DIV_SHIFT)
/**
 * WiFi coex feature.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_COEX (1UL << SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_COEX_SHIFT)
/**
 * Front end module feature.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FEM (1UL << SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FEM_SHIFT)
/**
 * Mode switch feature for 2Mbps 2.4 GHz operation.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_2_MBPS (1UL << SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_2_MBPS_SHIFT)
/**
 * Mode switch feature for 1Mbps 2.4 GHz operation with forward error correction (1-byte PHR).
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_1_MBPS_FEC (1UL << SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_1_MBPS_FEC_SHIFT)
/**
 * RX channel switching feature.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_CH_SWITCHING (1UL << SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_CH_SWITCHING_SHIFT)
/**
 * RX Duty cycling feature.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_DUTY_CYCLING (1UL << SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_DUTY_CYCLING_SHIFT)
/**
 * Mode switch feature for 2Mbps 2.4 GHz operation with fast channel switching.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FCS_2_MBPS (SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_2_MBPS | SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_CH_SWITCHING)
/**
 * Mode switch feature for 1Mbps 2.4 GHz operation with forward error correction (1-byte PHR)
 * and fast channel switching.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FCS_1_MBPS_FEC (SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_1_MBPS_FEC | SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_CH_SWITCHING)
/**
 * Mode switch feature for 2Mbps 2.4 GHz operation with duty cycling.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_DC_2_MBPS (SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_2_MBPS | SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_DUTY_CYCLING)
/**
 * Mode switch feature for 1Mbps 2.4 GHz operation with forward error correction (1-byte PHR)
 * and duty cycling.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_DC_1_MBPS_FEC (SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_1_MBPS_FEC | SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_DUTY_CYCLING)
/**
 * RX antenna diversity and WiFi coex features.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_ANT_DIV_COEX (SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_ANT_DIV | SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_COEX)

/**
 * Front end module and RX antenna diversity features.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FEM_ANT_DIV (SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FEM | SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_ANT_DIV)

/**
 * Front end module and WiFi coex feature.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FEM_COEX (SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FEM | SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_COEX)

/**
 * Front end module and RX antenna diversity and WiFi coex feature.
 */
#define SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FEM_ANT_DIV_COEX (SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_FEM | SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_ANT_DIV | SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_COEX)

/**
 * Configure the RAIL Address Filter for 802.15.4 filtering.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_addresses A pointer to the address information that should be used.
 * @return Status code indicating success of the function call. If this returns
 *   an error, the 802.15.4 address filter is in an undefined state.
 *
 * Set up the 802.15.4 address filter to accept messages to the given
 * addresses. This will return false if any of the addresses failed to be set.
 * If NULL is passed in for p_addresses, all addresses will be set to their
 * reset value.
 */
sl_rail_status_t sl_rail_ieee802154_set_addresses(sl_rail_handle_t rail_handle,
                                                  const sl_rail_ieee802154_addr_config_t *p_addresses);

/**
 * Set a PAN Id for 802.15.4 address filtering.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] pan_id The 16-bit PAN Id information.
 *   This will be matched against the destination PAN Id of incoming messages.
 *   The PAN Id is sent little endian over the air, meaning pan_id<7:0> is first in
 *   the payload followed by pan_id<15:8>. Set to 0xFFFF to disable for this index.
 * @param[in] index Indicates which PAN Id to set. Must be below
 *   \ref SL_RAIL_IEEE802154_MAX_ADDRESSES.
 * @return Status code indicating success of the function call.
 *
 * Set up the 802.15.4 address filter to accept messages to the given PAN Id.
 */
sl_rail_status_t sl_rail_ieee802154_set_pan_id(sl_rail_handle_t rail_handle,
                                               uint16_t pan_id,
                                               uint8_t index);

/**
 * Set a short address for 802.15.4 address filtering.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] short_addr A 16 bit short address value. This will be matched against the
 *   destination short address of incoming messages. The short address is sent
 *   little endian over the air meaning short_addr<7:0> is first in the payload
 *   followed by short_addr<15:8>. Set to 0xFFFF to disable for this index.
 * @param[in] index Which short address to set. Must be below
 *   \ref SL_RAIL_IEEE802154_MAX_ADDRESSES.
 * @return Status code indicating success of the function call.
 *
 * Set up the 802.15.4 address filter to accept messages to the given short
 * address.
 */
sl_rail_status_t sl_rail_ieee802154_set_short_address(sl_rail_handle_t rail_handle,
                                                      uint16_t short_addr,
                                                      uint8_t index);

/**
 * Set a long address for 802.15.4 address filtering.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_long_addr A pointer to an 8-byte array containing the long address
 *   information. The long address must be in over-the-air byte order. This will
 *   be matched against the destination long address of incoming messages. Set to
 *   0x00 00 00 00 00 00 00 00 to disable for this index.
 * @param[in] index Indicates which long address to set. Must be below
 *   \ref SL_RAIL_IEEE802154_MAX_ADDRESSES.
 * @return Status code indicating success of the function call.
 *
 * Set up the 802.15.4 address filter to accept messages to the given long
 * address.
 */
sl_rail_status_t sl_rail_ieee802154_set_long_address(sl_rail_handle_t rail_handle,
                                                     const uint8_t *p_long_addr,
                                                     uint8_t index);

/**
 * Set whether the current node is a PAN coordinator.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] is_pan_coordinator true if this device is a PAN coordinator.
 * @return Status code indicating success of the function call.
 *
 * If the device is a PAN Coordinator, it will accept data and command
 * frames with no destination address. This function will fail if 802.15.4
 * hardware acceleration is not currently enabled by calling
 * \ref sl_rail_ieee802154_init(). This setting may be changed
 * at any time when 802.15.4 hardware acceleration is enabled.
 */
sl_rail_status_t sl_rail_ieee802154_set_pan_coordinator(sl_rail_handle_t rail_handle,
                                                        bool is_pan_coordinator);

/**
 * Set whether to enable 802.15.4 promiscuous mode.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] enable true if all frames and addresses should be accepted.
 * @return Status code indicating success of the function call.
 *
 * If promiscuous mode is enabled, no frame or address filtering steps
 * will be performed other than checking the CRC. This function will fail if
 * 802.15.4 hardware acceleration is not currently enabled by calling
 * \ref sl_rail_ieee802154_init(). This setting may be changed at any time when
 * 802.15.4 hardware acceleration is enabled.
 */
sl_rail_status_t sl_rail_ieee802154_set_promiscuous_mode(sl_rail_handle_t rail_handle,
                                                         bool enable);

/**
 * @enum sl_rail_ieee802154_e_options_t
 * @brief 802.15.4E-2012 options, in reality a bitmask.
 */
SLI_RAIL_ENUM_GENERIC(sl_rail_ieee802154_e_options_t, uint32_t) {
  /**
   * Shift position of \ref SL_RAIL_IEEE802154_E_OPTION_GB868 bit.
   */
  SL_RAIL_IEEE802154_E_OPTION_GB868_SHIFT = 0,
  /**
   * Shift position of \ref SL_RAIL_IEEE802154_E_OPTION_ENH_ACK bit.
   */
  SL_RAIL_IEEE802154_E_OPTION_ENH_ACK_SHIFT = 1,
  /**
   * Shift position of \ref SL_RAIL_IEEE802154_E_OPTION_IMPLICIT_BROADCAST bit.
   */
  SL_RAIL_IEEE802154_E_OPTION_IMPLICIT_BROADCAST_SHIFT = 2,
};

/**
 * A value representing no options enabled.
 */
#define SL_RAIL_IEEE802154_E_OPTIONS_NONE 0UL
/**
 * All options disabled by default .
 */
#define SL_RAIL_IEEE802154_E_OPTIONS_DEFAULT SL_RAIL_IEEE802154_E_OPTIONS_NONE

/**
 * An option to enable/disable 802.15.4E-2012 features needed for GB868.
 * When not promiscuous, RAIL normally accepts only 802.15.4 MAC frames
 * whose MAC header Frame Version is 0 (802.15.4-2003) or 1 (802.15.4-2006),
 * filtering out higher Frame Version packets (as \ref
 * SL_RAIL_RX_PACKET_ABORT_FORMAT).
 * Enabling this feature additionally allows Frame Version 2 (802.15.4E-2012 /
 * 802.15.4-2015) packets to be accepted and passed to the application.
 *
 * @note Enabling this feature also automatically enables \ref
 *   SL_RAIL_IEEE802154_E_OPTION_ENH_ACK on platforms that support
 *   that feature.
 *
 * @note This feature does not automatically enable receiving Multipurpose
 *   frames; that can be enabled via \ref sl_rail_ieee802154_accept_frames()'s
 *   \ref SL_RAIL_IEEE802154_ACCEPT_MULTIPURPOSE_FRAMES.
 */
#define SL_RAIL_IEEE802154_E_OPTION_GB868 (1UL << SL_RAIL_IEEE802154_E_OPTION_GB868_SHIFT)

/**
 * An option to enable/disable 802.15.4E-2012 features needed for Enhanced Acks.
 * This option requires that \ref SL_RAIL_IEEE802154_E_OPTION_GB868 also be
 * enabled, and is enabled automatically on platforms that support this
 * feature. It exists as a separate flag to allow runtime detection of whether
 * the platform supports this feature or not.
 *
 * When enabled, only an Enhanced Ack is expected in response to a transmitted
 * Ack-requesting 802.15.4E Version 2 frame. RAIL only knows how to construct
 * 802.15.4 Immediate Acks but not Enhanced Acks.
 *
 * This option causes \ref SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND to be
 * issued for Ack-requesting Version 2 MAC Command frames, Data frames
 * (if \ref sl_rail_ieee802154_enable_data_frame_pending() is enabled), and
 * Multipurpose Frames (if \ref SL_RAIL_IEEE802154_ACCEPT_MULTIPURPOSE_FRAMES
 * is enabled).
 *
 * The application is expected to handle this event by calling \ref
 * sl_rail_get_rx_incoming_packet_info() and parsing the partly-received incoming
 * frame to determine the type of Ack needed:
 * - If an Immediate Ack, determine Frame Pending needs based on the packet
 *   type and addressing information and call \ref
 *   sl_rail_ieee802154_toggle_frame_pending() if necessary;
 * - If an Enhanced Ack, generate the complete payload of the Enhanced Ack
 *   including any Frame Pending information and call \ref
 *   sl_rail_ieee802154_write_enh_ack() in time for that Enhanced Ack to
 *   be sent. If not called in time, \ref SL_RAIL_EVENT_TXACK_UNDERFLOW will
 *   likely result.
 *   Note that if 802.15.4 MAC-level encryption is used with Version 2
 *   frames, the application should decrypt the MAC Command byte in a
 *   MAC Command frame to determine whether it is a Data Request or other
 *   MAC Command.
 *
 * An application can also enable \ref
 * sl_rail_ieee802154_enable_early_frame_pending() if the protocol doesn't
 * need to examine the MAC Command byte of MAC Command frames but can
 * infer it to be a Data Request.
 *
 * On 802.15.4E GB868 platforms that lack this support, legacy Immediate Acks
 * are sent/expected for received/transmitted Ack-requesting 802.15.4E Frame
 * Version 2 frames; calls to \ref sl_rail_ieee802154_write_enh_ack() have no
 * effect. Attempting to use this feature via \ref
 * sl_rail_ieee802154_config_e_options() returns an error.
 */
#define SL_RAIL_IEEE802154_E_OPTION_ENH_ACK (1UL << SL_RAIL_IEEE802154_E_OPTION_ENH_ACK_SHIFT)

/**
 * An option to enable/disable 802.15.4E-2012 macImplicitBroadcast feature.
 *
 * When enabled, received Frame Version 2 frames without a destination
 * PAN Id or destination address are treated as though they are addressed
 * to the broadcast PAN Id and broadcast short address. When disabled, such
 * frames are filtered unless the device is the PAN coordinator and
 * appropriate source addressing information exists in the packet
 */
#define SL_RAIL_IEEE802154_E_OPTION_IMPLICIT_BROADCAST (1UL << SL_RAIL_IEEE802154_E_OPTION_IMPLICIT_BROADCAST_SHIFT)

/**
 * A value representing all possible options.
 */
#define SL_RAIL_IEEE802154_E_OPTIONS_ALL 0xFFFFFFFFUL

/**
 * Configure certain 802.15.4E-2012 / 802.15.4-2015 Frame Version 2 features.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] mask A bitmask containing which options should be modified.
 * @param[in] options A bitmask containing desired options settings.
 *   Bit positions for each option are found in the \ref
 *   sl_rail_ieee802154_e_options_t.
 * @return Status code indicating success of the function call.
 *
 * This function will fail if 802.15.4 hardware acceleration is not
 * currently enabled by calling \ref sl_rail_ieee802154_init() or the platform
 * does not support the feature(s).
 * These settings may be changed at any time when 802.15.4 hardware
 * acceleration is enabled.
 */
sl_rail_status_t sl_rail_ieee802154_config_e_options(sl_rail_handle_t rail_handle,
                                                     sl_rail_ieee802154_e_options_t mask,
                                                     sl_rail_ieee802154_e_options_t options);

/**
 * @enum sl_rail_ieee802154_g_options_t
 * @brief 802.15.4G-2012 options, in reality a bitmask.
 */
SLI_RAIL_ENUM_GENERIC(sl_rail_ieee802154_g_options_t, uint32_t) {
  /**
   * Shift position of \ref SL_RAIL_IEEE802154_G_OPTION_GB868 bit.
   */
  SL_RAIL_IEEE802154_G_OPTION_GB868_SHIFT = 0,
  /**
   * Shift position of \ref SL_RAIL_IEEE802154_G_OPTION_DYN_FEC bit.
   */
  SL_RAIL_IEEE802154_G_OPTION_DYN_FEC_SHIFT = 1,
  /**
   * Shift position of \ref SL_RAIL_IEEE802154_G_OPTION_WI_SUN_MODE_SWITCH bit.
   */
  SL_RAIL_IEEE802154_G_OPTION_WI_SUN_MODE_SWITCH_SHIFT = 2,
};

/**
 * A value representing no options enabled.
 */
#define SL_RAIL_IEEE802154_G_OPTIONS_NONE 0UL
/**
 * All options disabled by default .
 */
#define SL_RAIL_IEEE802154_G_OPTIONS_DEFAULT SL_RAIL_IEEE802154_G_OPTIONS_NONE

/**
 * An option to enable/disable 802.15.4G-2012 features needed for GB868.
 * Normally RAIL supports 802.15.4-2003 and -2006 radio configurations
 * that have the single-byte PHY header allowing frames up to 128 bytes
 * in size. This feature must be enabled for 802.15.4G-2012 or
 * 802.15.4-2015 SUN PHY radio configurations with the two-byte
 * bit-reversed-length PHY header format.
 *
 * While GB868 only supports whitened non-FEC non-mode-switch frames
 * up to 129 bytes including 2-byte CRC, this option also enables:
 * - On platforms where \ref SL_RAIL_IEEE802154_SUPPORTS_G_4_BYTE_CRC
 *   is true: automatic per-packet 2/4-byte Frame Check Sequence (FCS)
 *   reception and transmission based on the FCS Type bit in the
 *   received/transmitted PHY header. This includes Ack reception
 *   and automatically-generated Acks reflect the CRC size of the
 *   incoming frame being acknowledged (i.e., their MAC payload will be
 *   increased to 7 bytes when sending 4-byte FCS).
 *   On other platforms, only the 2-byte FCS is supported.
 * - On platforms where \ref SL_RAIL_IEEE802154_SUPPORTS_G_UNWHITENED_RX
 *   and/or \ref SL_RAIL_IEEE802154_SUPPORTS_G_UNWHITENED_TX are true:
 *   automatic per-packet whitened/unwhitened reception and transmission,
 *   respectively, based on the Data Whitening bit in the received/transmitted
 *   PHY header. This includes Ack reception and automatically-generated Acks
 *   which reflect the whitening of the incoming frame being acknowledged.
 *   On other platforms, only whitened frames are supported.
 * - Support for frames up to 2049 bytes per the radio configuration's
 *   maximum packet length setting.
 *
 * @note Sending/receiving whitened frames assumes the radio configuration
 *   has established an appropriate 802.15.4-compliant whitening algorithm.
 *   RAIL does not itself override the radio configuration's whitening
 *   settings other than to enable/disable it per-packet based on the
 *   packet's PHY header Data Whitening flag.
 */
#define SL_RAIL_IEEE802154_G_OPTION_GB868 (1UL << SL_RAIL_IEEE802154_G_OPTION_GB868_SHIFT)

/**
 * An option to enable/disable 802.15.4G dynamic FEC feature (SUN FSK only).
 * The sync word, called start-of-frame delimiter (SFD) in the 15.4 spec, indicates whether
 * the rest of the packet is FEC encoded or not. This feature requires per-packet
 * dual sync word detection and specific receiver pausing.
 * Note that this feature is only available on platforms where
 * \ref SL_RAIL_IEEE802154_SUPPORTS_G_DYN_FEC is true.
 *
 * This option is only valid for SUN PHYs that have the FEC configured and enabled.
 *
 * The sync word used during transmit is selected with \ref SL_RAIL_TX_OPTION_SYNC_WORD_ID.
 *
 * The sync word corresponding to the FEC encoded mode must be SYNC1, with SYNC2 indicating non-FEC.
 * Sync words are set appropriately in all Sun FEC-enabled PHYs so there should
 * never be a need to call \ref sl_rail_config_sync_words() when this option is enabled.
 *
 * Also, dual sync word detection is set in all SUN FEC enabled PHYs, then there is no need
 * to change \ref SL_RAIL_RX_OPTION_ENABLE_DUAL_SYNC.
 */
#define SL_RAIL_IEEE802154_G_OPTION_DYN_FEC (1UL << SL_RAIL_IEEE802154_G_OPTION_DYN_FEC_SHIFT)

/**
 * An option to enable/disable Wi-SUN Mode Switch feature.
 * This feature consists in switching to a new PHY mode with a higher rate typically
 * by sending/receiving a specific Mode Switch packet that indicates the incoming new PHY mode.
 * The Mode Switch packet is an FSK-modulated 2-byte PHY header with no payload.
 * Because this feature relies on specific receiver pausing, note that it is only available
 * on platforms where \ref SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH is true.
 */
#define SL_RAIL_IEEE802154_G_OPTION_WI_SUN_MODE_SWITCH (1UL << SL_RAIL_IEEE802154_G_OPTION_WI_SUN_MODE_SWITCH_SHIFT)

/**
 * A value representing all possible options.
 */
#define SL_RAIL_IEEE802154_G_OPTIONS_ALL 0xFFFFFFFFUL

/**
 * Configure certain 802.15.4G-2012 / 802.15.4-2015 SUN PHY features
 * (only for radio configurations designed accordingly).
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] mask A bitmask containing which options should be modified.
 * @param[in] options A bitmask containing desired options settings.
 *   Bit positions for each option are found in the \ref
 *   sl_rail_ieee802154_g_options_t.
 * @return Status code indicating success of the function call.
 *
 * This function will fail if 802.15.4 hardware acceleration is not
 * currently enabled by calling \ref sl_rail_ieee802154_init(), the platform does
 * not support the feature(s), the radio configuration is not appropriate,
 * or the radio is not idle.
 */
sl_rail_status_t sl_rail_ieee802154_config_g_options(sl_rail_handle_t rail_handle,
                                                     sl_rail_ieee802154_g_options_t mask,
                                                     sl_rail_ieee802154_g_options_t options);

/**
 * @struct sl_rail_ieee802154_mode_switch_phr_t
 * @brief A structure containing the PHY Mode Id value and the corresponding mode
 *   switch PHR as defined in Wi-SUN spec.
 *   These structures are usually generated by the radio configurator.
 */
typedef struct sl_rail_ieee802154_mode_switch_phr {
  /**
   * PHY mode Id.
   */
  uint8_t phy_mode_id;
  /**
   * Corresponding Mode Switch PHY header.
   */
  uint16_t phr;
} sl_rail_ieee802154_mode_switch_phr_t;

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * When filtering PHY Mode Id, this is the minimum OFDM value
 */
#define SL_RAIL_IEEE802154_MIN_OFDM_PHY_MODE_ID (0x20U)
/**
 * When filtering PHY Mode Id, this is the maximum OFDM value
 */
#define SL_RAIL_IEEE802154_MAX_OFDM_PHY_MODE_ID (0x5FU)
#endif //DOXYGEN_UNDOCUMENTED

/**
 * Compute channel to switch to given a targeted PHY Mode Id
 * in the context of Wi-SUN mode switching.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] new_phy_mode_id A targeted PHY Mode Id.
 * @param[out] p_channel A pointer to the channel to switch to.
 * @return Status code indicating success of the function call.
 *
 * This function will fail if:
 * - the targeted phy_mode_id is the same as the current PHY Mode Id
 * - called on a platform that lacks \ref SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH
 * - called on a platform that doesn't have 802154G options enabled
 *     by \ref sl_rail_ieee802154_config_g_options().
 * For new_phy_mode_id associated with a FSK FEC_off PHY, if dynamic FEC is
 * activated (see \ref SL_RAIL_IEEE802154_G_OPTION_DYN_FEC), the returned
 * channel can correspond to the associated FSK FEC_on PHY corresponding
 * then to PHY Mode Id = new_phy_mode_id + 16
 */
sl_rail_status_t sl_rail_ieee802154_compute_channel_from_phy_mode_id(sl_rail_handle_t rail_handle,
                                                                     uint8_t new_phy_mode_id,
                                                                     uint16_t *p_channel);

/**
 * Manage forbidden channels during mode switch.
 *
 * @param[in] rail_handle The real RAIL instance handle issuing the callback.
 * @param[in] current_base_freq_hz The current frequency of the base channel.
 * @param[in] new_phy_mode_id A targeted PHY Mode Id.
 * @param[in] p_config_entry_new_phy_mode_id A pointer to \ref sl_rail_channel_config_entry_t
 *   structure corresponding to the new PHY config_entry.
 * @param[in,out] p_channel A pointer to the channel to switch to. If channel
 *   is valid, the function must just return. If channel is forbidden, the
 *   function must update it with the closest valid channel. The highest
 *   channel must be selected in case of two valid channels being equidistant
 *   to a forbidden channel.
 * @return Status code indicating success of the function call. It must
 *   return \ref SL_RAIL_STATUS_INVALID_PARAMETER for failure or \ref
 *   SL_RAIL_STATUS_NO_ERROR for success.
 *
 * This function must fail if no valid channel has been found. If so, RAIL will
 * abort the mode switch.
 *
 * @note This callback will only be called on platforms where
 *   \ref SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH is true, \ref
 *   SL_RAIL_IEEE802154_G_OPTION_WI_SUN_MODE_SWITCH was successfully enabled,
 *   and a valid mode switch PHY header is received.
 */
sl_rail_status_t sl_railcb_ieee802154_is_mode_switch_new_channel_valid(sl_rail_handle_t rail_handle,
                                                                       uint32_t current_base_freq_hz,
                                                                       uint8_t new_phy_mode_id,
                                                                       const sl_rail_channel_config_entry_t *p_config_entry_new_phy_mode_id,
                                                                       uint16_t *p_channel);

/**
 * When receiving packets, accept 802.15.4 BEACON frame types.
 */
#define SL_RAIL_IEEE802154_ACCEPT_BEACON_FRAMES       (0x01)
/**
 * When receiving packets, accept 802.15.4 DATA frame types.
 */
#define SL_RAIL_IEEE802154_ACCEPT_DATA_FRAMES         (0x02)
/**
 * When receiving packets, accept 802.15.4 Ack frame types.
 * @note Expected Ack frame types will still be accepted regardless
 *   of this setting when waiting for an Ack after a transmit that
 *   used \ref SL_RAIL_TX_OPTION_WAIT_FOR_ACK and auto-Ack is enabled.
 */
#define SL_RAIL_IEEE802154_ACCEPT_ACK_FRAMES          (0x04)
/**
 * When receiving packets, accept 802.15.4 COMMAND frame types.
 */
#define SL_RAIL_IEEE802154_ACCEPT_COMMAND_FRAMES      (0x08)
// Reserved for possible future use:                  (0x10)
/**
 * When receiving packets, accept 802.15.4-2015 Multipurpose frame types.
 */
#define SL_RAIL_IEEE802154_ACCEPT_MULTIPURPOSE_FRAMES (0x20)

/**
 * In standard operation, accept BEACON, DATA and COMMAND frames.
 * Don't receive Ack frames unless waiting for Ack (i.e., only
 * receive expected Acks).
 */
#define SL_RAIL_IEEE802154_ACCEPT_STANDARD_FRAMES (SL_RAIL_IEEE802154_ACCEPT_BEACON_FRAMES \
                                                   | SL_RAIL_IEEE802154_ACCEPT_DATA_FRAMES \
                                                   | SL_RAIL_IEEE802154_ACCEPT_COMMAND_FRAMES)

/**
 * Set which 802.15.4 frame types to accept.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] frames_mask A mask containing which 802.15.4 frame types to receive.
 * @return Status code indicating success of the function call.
 *
 * This function will fail if 802.15.4 hardware acceleration is not currently
 * enabled by calling \ref sl_rail_ieee802154_init() or frames_mask requests an
 * unsupported frame type.
 * This setting may be changed at any time when 802.15.4 hardware
 * acceleration is enabled. Only Beacon, Data, Ack, Command, and Multipurpose
 * frames may be received.
 * The SL_RAIL_IEEE802154_ACCEPT_*_FRAMES defines may be combined to create a
 * bitmask to pass into this function.
 *
 * \ref SL_RAIL_IEEE802154_ACCEPT_ACK_FRAMES behaves slightly different than the
 * other defines. If \ref SL_RAIL_IEEE802154_ACCEPT_ACK_FRAMES is set, the radio
 * will accept an Ack frame during normal packet reception, but only a
 * truly expected Ack will have its \ref sl_rail_rx_packet_details_t::is_ack true.
 * If \ref SL_RAIL_IEEE802154_ACCEPT_ACK_FRAMES is not set, Ack frames will be
 * filtered unless they're expected when the radio is waiting for an Ack.
 */
sl_rail_status_t sl_rail_ieee802154_accept_frames(sl_rail_handle_t rail_handle,
                                                  uint8_t frames_mask);

/**
 * Enable early Frame Pending lookup event notification
 * (\ref SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND).
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] enable true to enable, false to disable.
 * @return Status code indicating success of the function call.
 *
 * Normally, \ref SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND is triggered after
 * receiving the entire MAC header and MAC command byte for an Ack-requesting
 * MAC command frame. Version 0/1 frames also require that command to be a
 * Data Request for this event to occur.
 * Enabling this feature causes this event to be triggered earlier to allow for
 * more time to determine the type of Ack needed (Immediate or Enhanced) and/or
 * perform frame pending lookup to influence the outgoing Ack by using \ref
 * sl_rail_ieee802154_write_enh_ack() or \ref sl_rail_ieee802154_toggle_frame_pending().
 *
 * For Frame Version 0/1 packets and for Frame Version 2 packets when \ref
 * SL_RAIL_IEEE802154_E_OPTION_ENH_ACK is not in use, "early" means right
 * after receiving the source address information in the MAC header.
 *
 * For Frame Version 2 packets when \ref SL_RAIL_IEEE802154_E_OPTION_ENH_ACK
 * is in use, "early" means right after receiving any Auxiliary Security
 * header which follows the source address information in the MAC header.
 *
 * This feature is useful when the protocol knows an Ack-requesting MAC
 * Command must be a data poll without needing to receive the MAC Command
 * byte, giving it a bit more time to adjust Frame Pending or generate an
 * Enhanced Ack.
 *
 * This function will fail if 802.15.4 hardware acceleration is not
 * currently enabled by calling \ref sl_rail_ieee802154_init(),
 * or on platforms that do not support this feature.
 * This setting may be changed at any time when 802.15.4 hardware
 * acceleration is enabled.
 */
sl_rail_status_t sl_rail_ieee802154_enable_early_frame_pending(sl_rail_handle_t rail_handle,
                                                               bool enable);

/**
 * Enable Frame Pending lookup event notification
 * (\ref SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND) for MAC Data frames.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] enable true to enable, false to disable.
 * @return Status code indicating success of the function call.
 *
 * Normally \ref SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND is triggered only
 * for Ack-requesting MAC command frames.
 * Enabling this feature causes this event to also be triggered for MAC data
 * frames, at the same point in the packet as \ref
 * sl_rail_ieee802154_enable_early_frame_pending() would trigger.
 * This feature is necessary to support the Thread Basil-Hayden Enhanced
 * Frame Pending feature in Version 0/1 frames, and to support Version 2
 * Data frames which require an Enhanced Ack.
 *
 * This function will fail if 802.15.4 hardware acceleration is not
 * currently enabled by calling \ref sl_rail_ieee802154_init().
 * This setting may be changed at any time when 802.15.4 hardware acceleration
 * is enabled.
 */
sl_rail_status_t sl_rail_ieee802154_enable_data_frame_pending(sl_rail_handle_t rail_handle,
                                                              bool enable);

/**
 * Change the Frame Pending bit on the outgoing legacy Immediate Ack from
 * the default specified by
 * \ref sl_rail_ieee802154_config_t::default_frame_pending_in_outgoing_acks.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This function must only be called while processing the \ref
 * SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND if the Ack
 * for this packet should go out with its Frame Pending bit set differently
 * than what was specified by
 * \ref sl_rail_ieee802154_config_t::default_frame_pending_in_outgoing_acks.
 *
 * It's intended only for use with 802.15.4 legacy immediate Acks and
 * not 802.15.4E enhanced Acks.
 * This will return \ref SL_RAIL_STATUS_INVALID_STATE if it is too late to
 * modify the outgoing Immediate Ack.
 */
sl_rail_status_t sl_rail_ieee802154_toggle_frame_pending(sl_rail_handle_t rail_handle);

/**
 * Get the source address of the incoming data request.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_address A pointer to \ref sl_rail_ieee802154_address_t structure
 *   to populate with source address information.
 * @return Status code indicating success of the function call.
 *
 * This function must only be called when handling the \ref
 * SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND event. This will return
 * \ref SL_RAIL_STATUS_INVALID_STATE if the address information is stale
 * (i.e., it is too late to affect the outgoing Ack).
 */
sl_rail_status_t sl_rail_ieee802154_get_address(sl_rail_handle_t rail_handle,
                                                sl_rail_ieee802154_address_t *p_address);

/**
 * Write the Auto-Ack FIFO for the next outgoing 802.15.4E Enhanced Ack.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_ack_data A pointer to Ack data to transmit
 *   This may be NULL, in which case it's assumed the data has already
 *   been emplaced into the Ack buffer and RAIL just needs to be told
 *   how many bytes are there. Use \ref sl_rail_get_auto_ack_fifo() to get
 *   the address of RAIL's Auto-Ack buffer in RAM and its size.
 * @param[in] ack_data_bytes Length of Ack data, in bytes.
 *   If this exceeds the size of the Auto-Ack FIFO buffer the function
 *   will return \ref SL_RAIL_STATUS_INVALID_PARAMETER.
 * @return Status code indicating success of the function call.
 *
 * This function sets the Auto-Ack data to use in acknowledging the frame
 * being received. It must only be called while processing the \ref
 * SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND, and is intended for use
 * when packet information from \ref sl_rail_get_rx_incoming_packet_info()
 * indicates an 802.15.4E Enhanced Ack must be sent instead of a legacy
 * Immediate Ack. \ref sl_rail_ieee802154_toggle_frame_pending() should not be
 * called for an Enhanced Ack; instead the Enhanced Ack's Frame Control
 * Field should have the Frame Pending bit set appropriately in its p_ack_data.
 * This will return \ref SL_RAIL_STATUS_INVALID_STATE if it is too late to
 * write the outgoing Ack -- a situation that will likely trigger
 * a \ref SL_RAIL_EVENT_TXACK_UNDERFLOW event. When successful, the Enhanced
 * p_ack_data will only be sent once. Subsequent packets needing an Enhanced
 * Ack will each need to call this function to write their Ack information.
 */
sl_rail_status_t sl_rail_ieee802154_write_enh_ack(sl_rail_handle_t rail_handle,
                                                  const uint8_t *p_ack_data,
                                                  uint16_t ack_data_bytes);

/**
 * Set a separate RX packet to TX state transition turnaround time for
 * sending an Enhanced Ack.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in,out] p_rx_to_enh_ack_tx A pointer to the turnaround transition requested
 *   for Enhanced Acks. It will be updated with the actual time set.
 *   Requesting a time of 0 will sync the Enhanced Ack turnaround time with
 *   that used for immediate Acks (and output 0). Requesting a time of \ref
 *   SL_RAIL_TRANSITION_TIME_KEEP will output the current Enhanced Ack timing
 *   parameter (0 if it is the same as that used for Immediate Acks).
 * @return Status code indicating a success of the function call.
 *   An error will not update the p_rx_to_enh_ack_tx output parameter.
 *
 * Normally Immediate and Enhanced Acks are both sent using the
 * \ref sl_rail_ieee802154_config_t::timings rx_to_tx turnaround time.
 * If the stack needs more time to prepare an Enhanced Ack, it can
 * call this function after \ref sl_rail_ieee802154_init() to set a
 * longer turnaround time used just for Enhanced Ack transmits.
 *
 * This function will fail on platforms that lack
 * \ref SL_RAIL_IEEE802154_SUPPORTS_E_ENHANCED_ACK.
 */
sl_rail_status_t sl_rail_ieee802154_set_rx_to_enh_ack_tx(sl_rail_handle_t rail_handle,
                                                         sl_rail_transition_time_t *p_rx_to_enh_ack_tx);

/**
 * Convert RSSI into 802.15.4 Link Quality Indication (LQI) metric
 * compatible with the Silicon Labs Zigbee stack.
 *
 * @param[in] orig_lqi The original LQI, for example from
 *   \ref sl_rail_rx_packet_details_t::lqi.
 *   This parameter is not currently used but may be used in the future.
 * @param[in] rssi_dbm The RSSI in dBm, for example from
 *   \ref sl_rail_rx_packet_details_t::rssi_dbm.
 * @return An LQI value (range 0..255 but not all intermediate values are
 *   possible) based on the rssi_dbm and the chip's RSSI sensitivity range.
 *
 * This function is compatible with \ref sl_rail_convert_lqi_callback_t() and
 * is suitable to pass to \ref sl_rail_convert_lqi().
 */
uint8_t sl_rail_ieee802154_convert_rssi_to_lqi(uint8_t orig_lqi, int8_t rssi_dbm);

/**
 * Convert RSSI into 802.15.4 Energy Detection (ED) metric
 * compatible with the Silicon Labs Zigbee stack.
 *
 * @param[in] rssi_dbm The RSSI in dBm, for example from
 *   \ref sl_rail_rx_packet_details_t::rssi_dbm.
 * @return An Energy Detect value (range 0..255 but not all intermediate
 *   values are possible) based on the rssiDbm and the chip's RSSI
 *   sensitivity range.
 */
uint8_t sl_rail_ieee802154_convert_rssi_to_ed(int8_t rssi_dbm);

/**
 * @enum sl_rail_ieee802154_cca_mode_t
 * @brief Available CCA modes.
 */
SLI_RAIL_ENUM(sl_rail_ieee802154_cca_mode_t) {
  /**
   * RSSI-based CCA. CCA reports a busy medium upon detecting any energy
   * above \ref sl_rail_csma_config_t::cca_threshold_dbm.
   */
  SL_RAIL_IEEE802154_CCA_MODE_RSSI = 0u,
  /**
   * Signal Identifier-based CCA. CCA reports a busy medium only upon the
   * detection of a signal compliant with this standard with the same modulation
   * and spreading characteristics of the PHY that is currently in use.
   */
  SL_RAIL_IEEE802154_CCA_MODE_SIGNAL = 1u,
  /**
   * RSSI or signal identifier-based CCA. CCA reports a busy medium on
   * either detecting any energy above \ref sl_rail_csma_config_t.cca_threshold_dbm
   * or detection of a signal compliant with this standard with the same
   * modulation and spreading characteristics of the PHY that is currently in use.
   */
  SL_RAIL_IEEE802154_CCA_MODE_SIGNAL_OR_RSSI = 2u,
  /**
   * RSSI and signal identifier-based CCA. CCA reports a busy medium only
   * on detecting any energy above \ref sl_rail_csma_config_t.cca_threshold_dbm of a
   * signal compliant with this standard with the same modulation and spreading
   * characteristics of the PHY that is currently in use.
   */
  SL_RAIL_IEEE802154_CCA_MODE_SIGNAL_AND_RSSI = 3u,
  /**
   * ALOHA. Always transmit CCA=1. CCA always reports an idle medium.
   */
  SL_RAIL_IEEE802154_CCA_MODE_ALWAYS_TRANSMIT = 4u,
  /**
   * Number of CCA modes. Must be last.
   */
  SL_RAIL_IEEE802154_CCA_MODE_COUNT
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_IEEE802154_CCA_MODE_RSSI            ((sl_rail_ieee802154_cca_mode_t) SL_RAIL_IEEE802154_CCA_MODE_RSSI)
#define SL_RAIL_IEEE802154_CCA_MODE_SIGNAL          ((sl_rail_ieee802154_cca_mode_t) SL_RAIL_IEEE802154_CCA_MODE_SIGNAL)
#define SL_RAIL_IEEE802154_CCA_MODE_SIGNAL_OR_RSSI  ((sl_rail_ieee802154_cca_mode_t) SL_RAIL_IEEE802154_CCA_MODE_SIGNAL_OR_RSSI)
#define SL_RAIL_IEEE802154_CCA_MODE_SIGNAL_AND_RSSI ((sl_rail_ieee802154_cca_mode_t) SL_RAIL_IEEE802154_CCA_MODE_SIGNAL_AND_RSSI)
#define SL_RAIL_IEEE802154_CCA_MODE_ALWAYS_TRANSMIT ((sl_rail_ieee802154_cca_mode_t) SL_RAIL_IEEE802154_CCA_MODE_ALWAYS_TRANSMIT)
#define SL_RAIL_IEEE802154_CCA_MODE_COUNT           ((sl_rail_ieee802154_cca_mode_t) SL_RAIL_IEEE802154_CCA_MODE_COUNT)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @enum sl_rail_ieee802154_signal_identifier_mode_t
 * @brief Available Signal identifier modes.
 */
SLI_RAIL_ENUM(sl_rail_ieee802154_signal_identifier_mode_t) {
  /**
   * Disable signal detection mode.
   */
  SL_RAIL_IEEE802154_SIGNAL_IDENTIFIER_MODE_DISABLE = 0u,
  /**
   * 2.4 GHz 802.15.4 signal detection mode with SI block reset on GPIO falling edge (via PRS).
   */
  SL_RAIL_IEEE802154_SIGNAL_IDENTIFIER_MODE_154 = 1u,
  /**
   * 2.4 GHz 802.15.4 signal detection mode with SI block reset on AGC negative step.
   */
  SL_RAIL_IEEE802154_SIGNAL_IDENTIFIER_MODE_154_WITH_AGC_RESET = 2u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_IEEE802154_SIGNAL_IDENTIFIER_MODE_DISABLE ((sl_rail_ieee802154_signal_identifier_mode_t) SL_RAIL_IEEE802154_SIGNAL_IDENTIFIER_MODE_DISABLE)
#define SL_RAIL_IEEE802154_SIGNAL_IDENTIFIER_MODE_154     ((sl_rail_ieee802154_signal_identifier_mode_t) SL_RAIL_IEEE802154_SIGNAL_IDENTIFIER_MODE_154)
#define SL_RAIL_IEEE802154_SIGNAL_IDENTIFIER_MODE_154_WITH_AGC_RESET     ((sl_rail_ieee802154_signal_identifier_mode_t) SL_RAIL_IEEE802154_SIGNAL_IDENTIFIER_MODE_154_WITH_AGC_RESET)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * Configure signal identifier for 802.15.4 signal detection.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] signal_identifier_mode Mode of signal identifier operation.
 *
 * This features allows detection of 2.4 GHz 802.15.4 signal on air. This
 * function must be called once before \ref sl_rail_ieee802154_enable_signal_detection()
 * to configure and enable signal identifier.
 *
 * To enable event for signal detection \ref sl_rail_config_events() must be called
 * for enabling \ref SL_RAIL_EVENT_SIGNAL_DETECTED.
 *
 * This function is only supported by chips where
 * \ref SL_RAIL_IEEE802154_SUPPORTS_SIGNAL_IDENTIFIER and
 * \ref sl_rail_ieee802154_supports_signal_identifier() are true.
 *
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_ieee802154_config_signal_identifier(sl_rail_handle_t rail_handle,
                                                             sl_rail_ieee802154_signal_identifier_mode_t signal_identifier_mode);

/**
 * Enable or disable signal identifier for 802.15.4 signal detection.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] enable Signal detection is enabled if true, disabled if false.
 *
 * \ref sl_rail_ieee802154_config_signal_identifier() must be called once before calling
 * this function to configure and enable signal identifier.
 * Once a signal is detected signal detection will be turned off and this
 * function should be called to re-enable the signal detection without needing
 * to call \ref sl_rail_ieee802154_config_signal_identifier() if the signal identifier
 * is already configured and enabled.
 *
 * This function is only supported by chips where
 * \ref SL_RAIL_IEEE802154_SUPPORTS_SIGNAL_IDENTIFIER and
 * \ref sl_rail_ieee802154_supports_signal_identifier() are true.
 *
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_ieee802154_enable_signal_detection(sl_rail_handle_t rail_handle,
                                                            bool enable);

/**
 * Set 802.15.4 CCA mode.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] cca_mode Mode of CCA operation.
 *
 * This function sets the CCA mode \ref sl_rail_ieee802154_cca_mode_t.
 * If not called, SL_RAIL_IEEE802154_CCA_MODE_RSSI (RSSI-based CCA) is used for CCA.
 *
 * In \ref SL_RAIL_IEEE802154_CCA_MODE_SIGNAL, \ref SL_RAIL_IEEE802154_CCA_MODE_SIGNAL_OR_RSSI and
 * \ref SL_RAIL_IEEE802154_CCA_MODE_SIGNAL_AND_RSSI the signal identifier is enabled
 * for the duration of LBT. If previously enabled by
 * \ref sl_rail_ieee802154_config_signal_identifier(), the signal identifier will remain
 * active until triggered.
 *
 * This function is only supported by chips where
 * \ref SL_RAIL_IEEE802154_SUPPORTS_SIGNAL_IDENTIFIER and
 * \ref sl_rail_ieee802154_supports_signal_identifier() are true.
 *
 * @return Status code indicating success of the function call.
 *   An error should be returned if cca_mode is unsuppported on a given device.
 */
sl_rail_status_t sl_rail_ieee802154_config_cca_mode(sl_rail_handle_t rail_handle,
                                                    sl_rail_ieee802154_cca_mode_t cca_mode);

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * Allow certain malformed MAC Header frames to be received.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] allow true to allow, false to disallow.
 * @return Status code indicating success of the function call.
 *
 * When allowed, certain MAC header formats that 802.15.4 deems
 * illegal will be received rather than filtered. This is to
 * support interoperablity with an old spec-violating 802.15.4
 * peer and is only supported on certain EFR32 Series 2 platforms.
 *
 * @note \ref sl_rail_ieee802154_set_pan_coordinator() must also be
 *   enabled to receive malformed frames lacking destination
 *   address information.
 */
sl_rail_status_t sl_rail_ieee802154_allow_malformed(sl_rail_handle_t rail_handle,
                                                    bool allow);
#endif //DOXYGEN_UNDOCUMENTED

/**
 * Configure RX channel switching for 802.15.4.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_config A pointer to \ref sl_rail_ieee802154_rx_channel_switching_cfg_t
 *  structure. NULL will disable any switching previously set up. The feature can
 *  also be disabled by disabling \ref SL_RAIL_RX_OPTION_CHANNEL_SWITCHING.
 * @return Status code indicating success of the function call.
 *   For series 3 parts:
 *     return \ref SL_RAIL_STATUS_INVALID_STATE if IEEE 802.15.4 is not enabled via
 *       \ref sl_rail_ieee802154_init(),
 *     return \ref SL_RAIL_STATUS_NO_ERROR otherwise.
 *
 * This function configures RX channel switching, allowing reception of 2.4 GHz
 * 802.15.4 signals on two different radio channels within the same PHY.
 * This function should be called once before \ref sl_rail_start_rx() and/or
 * enabling \ref SL_RAIL_RX_OPTION_CHANNEL_SWITCHING.
 *
 * When \ref SL_RAIL_RX_OPTION_CHANNEL_SWITCHING is enabled, channel switching
 * will occur during normal listening but is suspended (and the radio is
 * idled) when starting any kind of transmit, including scheduled or CSMA
 * transmits. It remains suspended after a \ref SL_RAIL_TX_OPTION_WAIT_FOR_ACK
 * transmit until the Ack is received or times out.
 *
 * When \ref SL_RAIL_RX_OPTION_CHANNEL_SWITCHING is disabled after switching
 * has been active, the radio could be left listening on either channel,
 * so the application should call \ref sl_rail_start_rx() to put it on the
 * desired non-switching channel.
 *
 * @note Switching is cancelled on any PHY change and this function would
 *   need to be re-called to reestablish switching after such a change.
 *   When RX channel switching is active, receive sensitivity and
 *   performance are slightly impacted.
 *
 * @note Switching is stopped on any channel change that is not from
 *   \ref sl_rail_ieee802154_rx_channel_switching_cfg_t but is resumed
 *   on configuring back the appropriate channel.
 *
 * @parblock
 * @note For series 2 parts:
 *   If the two channels are same, the function behaves the same as if
 *     p_config was NULL.
 * @note IEEE 802.15.4 must be enabled via \ref sl_rail_ieee802154_init(), and the
 *     radio must be in the idle state when configuring RX channel switching.
 *     A DMA channel must be allocated with \ref sl_rail_use_dma() or by
 *     incorporating the \ref rail_util_dma plugin; otherwise this API will
 *     return \ref SL_RAIL_STATUS_INVALID_CALL.
 *     This feature also requires a PRS channel, internally allocated by the
 *     RAIL library, to use and hold onto for future use. If no PRS channel is
 *     available, the function returns \ref SL_RAIL_STATUS_INVALID_PARAMETER.
 * @note This function internally uses \ref sl_rail_enable_cache_synth_cal() to
 *     enable/disable the sequencer cache to store the synth calibration value.
 * @warning For series 2 parts, this function relies on LDMA access and RAIL is
 *   meant to run in TrustZone non-secure world, it is not supported if LDMA
 *   is configured as secure peripheral and it will return
 *   \ref SL_RAIL_STATUS_INVALID_CALL.
 * @endparblock
 *
 * @parblock
 * @note For series 3 parts:
 * @note IEEE 802.15.4 must be enabled via \ref sl_rail_ieee802154_init().
 * @note If the two channels are same, the function will do channel switching
 *   on a single frequency but will still duty cycle if PHY allows it.
 *
 * @note \ref SL_RAIL_RX_OPTION_CHANNEL_SWITCHING is only considered when
 *   \ref sl_rail_start_rx() or \ref sl_rail_prepare_channel() is called,
 *   not instantly.
 * @endparblock
 */
sl_rail_status_t sl_rail_ieee802154_config_rx_channel_switching(sl_rail_handle_t rail_handle,
                                                                const sl_rail_ieee802154_rx_channel_switching_cfg_t *p_config);

/// @} // end of IEEE802.15.4

///
/// @addtogroup Calibration
/// @brief IEEE802154 protocol-specific APIs for calibrating the radio.
/// @{
///

/**
 * Calibrate image rejection for IEEE 802.15.4 2.4 GHz.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_image_rejection A pointer to the result of the image rejection calibration.
 * @return Status code indicating success of the function call.
 *
 * Some chips have protocol-specific image rejection calibrations programmed
 * into their flash. This function will either get the value from flash and
 * apply it, or run the image rejection algorithm to find the value.
 */
sl_rail_status_t sl_rail_ieee802154_calibrate_ir_2p4_ghz(sl_rail_handle_t rail_handle,
                                                         uint32_t *p_image_rejection);

/// @} // End of group Calibration

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// Temporary RAIL 3.x API mappings to RAIL 2.x APIs

#include "rail_ieee802154.h"
#define sl_rail_ieee802154_phy_2p4_ghz \
  ((const RAIL_ChannelConfig_t *const)RAIL_IEEE802154_Phy2p4GHz)
#define sl_rail_ieee802154_phy_2p4_ghz_1_mbps_fec \
  ((const RAIL_ChannelConfig_t *const)RAIL_IEEE802154_Phy2p4GHz1MbpsFec)
#define  sl_rail_ieee802154_phy_2p4_ghz_2_mbps \
  ((const RAIL_ChannelConfig_t *const)RAIL_IEEE802154_Phy2p4GHz2Mbps)
#define sl_rail_ieee802154_phy_2p4_ghz_fcs_1_mbps_fec \
  ((const RAIL_ChannelConfig_t *const)RAIL_IEEE802154_Phy2p4GHzFcs1MbpsFec)
#define  sl_rail_ieee802154_phy_2p4_ghz_fcs_2_mbps \
  ((const RAIL_ChannelConfig_t *const)RAIL_IEEE802154_Phy2p4GHzFcs2Mbps)
#define sl_rail_ieee802154_phy_2p4_ghz_ant_div \
  ((const RAIL_ChannelConfig_t *const)RAIL_IEEE802154_Phy2p4GHzAntDiv)
#define  sl_rail_ieee802154_phy_2p4_ghz_coex \
  ((const RAIL_ChannelConfig_t *const)RAIL_IEEE802154_Phy2p4GHzCoex)
#define  sl_rail_ieee802154_phy_2p4_ghz_ant_div_coex \
  ((const RAIL_ChannelConfig_t *const)RAIL_IEEE802154_Phy2p4GHzAntDivCoex)
#define  sl_rail_ieee802154_phy_2p4_ghz_fem \
  ((const RAIL_ChannelConfig_t *const)RAIL_IEEE802154_Phy2p4GHzFem)
#define  sl_rail_ieee802154_phy_2p4_ghz_ant_div_fem \
  ((const RAIL_ChannelConfig_t *const)RAIL_IEEE802154_Phy2p4GHzAntDivFem)
#define  sl_rail_ieee802154_phy_2p4_ghz_coex_fem \
  ((const RAIL_ChannelConfig_t *const)RAIL_IEEE802154_Phy2p4GHzCoexFem)
#define  sl_rail_ieee802154_phy_2p4_ghz_ant_div_coex_fem \
  ((const RAIL_ChannelConfig_t *const)RAIL_IEEE802154_Phy2p4GHzAntDivCoexFem)
#define  sl_rail_ieee802154_phy_gb863_mhz \
  ((const RAIL_ChannelConfig_t *const)RAIL_IEEE802154_PhyGB863MHz)
#define  sl_rail_ieee802154_phy_gb915_mhz \
  ((const RAIL_ChannelConfig_t *const)RAIL_IEEE802154_PhyGB915MHz)
#define  sl_rail_ieee802154_phy_2p4_ghz_rx_ch_switching \
  ((const RAIL_ChannelConfig_t *const)RAIL_IEEE802154_Phy2p4GHzRxChSwitching)

#endif//DOXYGEN_SHOULD_SKIP_THIS

#ifdef __cplusplus
}
#endif

#endif // SL_RAIL_IEEE802154_H
