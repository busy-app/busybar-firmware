/***************************************************************************//**
 * @file
 * @brief The BLE specific header file for the RAIL library.
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

#ifndef SL_RAIL_BLE_H
#define SL_RAIL_BLE_H

// Get the standard include types
#include <stdint.h>
#include <stdbool.h>

// Get the RAIL specific structures and types
#include "sl_rail_types.h"

#ifdef __cplusplus
extern "C" {
#endif

///
/// @addtogroup BLE
/// @ingroup Protocol_Specific
/// Accelerator routines for Bluetooth Low Energy (BLE).
///
/// The APIs in this module configure the radio for BLE
/// operation and provide additional helper routines necessary for
/// normal BLE send/receive that aren't available directly in RAIL.
/// RAIL APIs should be used to set up the application. However,
/// \ref sl_rail_config_channels() should not be called to set up
/// the PHY. Instead, sl_rail_ble_coding* APIs should be used to set up the
/// 1 Mbps, 2 Mbps, or Coded PHY configurations needed by the application. These
/// APIs will configure the hardware and also configure the set of valid BLE
/// channels.
///
/// To implement a standard BLE link layer, you will also need to handle tight
/// turnaround times and send packets at specific instants. This can all be
/// managed through general RAIL functions, such as \ref sl_rail_start_scheduled_tx(),
/// \ref sl_rail_start_scheduled_rx(), and \ref sl_rail_set_state_timing(). See RAIL APIs for more
/// useful functions.
///
/// A simple example to set up the application to be in BLE mode is shown
/// below. Note that this will put the radio on the first advertising channel
/// with the advertising Access Address. In any full-featured BLE application you
/// will need to use the \ref sl_rail_ble_config_channel_radio_params() function to change
/// the sync word and other parameters as needed based on your connection.
/// @code{.c}
/// // RAIL handle set at initialization time.
/// static sl_rail_handle_t g_rail_handle = SL_RAIL_EFR32_HANDLE;
///
/// #define BLE_RX_PACKET_QUEUE_ENTRIES 16
/// #define BLE_FIFO_BYTES 512 // Handle max-sized BLE packets, AoX
///
/// static sl_rail_packet_queue_t g_rx_packet_queue[BLE_RX_PACKET_QUEUE_ENTRIES];
/// static SL_RAIL_DECLARE_FIFO_BUFFER(g_rx_fifo_buffer, BLE_FIFO_BYTES);
/// static SL_RAIL_DECLARE_FIFO_BUFFER(g_tx_fifo_buffer, BLE_FIFO_BYTES);
///
/// static void radio_event_handler(sl_rail_handle_t rail_handle,
///                                 sl_rail_events_t events)
/// {
///   // ... handle RAIL events, e.g., receive and transmit completion
/// }
///
/// // Set the radio to receive on the first BLE advertising channel.
/// void ble_advertise_enable(void)
/// {
///   sl_rail_config_t rail_config = {
///     .events_callback         = &radio_event_handler,
///     .rx_packet_queue_entries = BLE_RX_PACKET_QUEUE_ENTRIES,
///     .rx_fifo_bytes           = sizeof(g_rx_fifo_buffer),
///     .tx_fifo_bytes           = sizeof(g_tx_fifo_buffer),
///     .tx_fifo_init_bytes      = 0,
///     .rx_packet_queue         = g_rx_packet_queue,
///     .rx_fifo_buffer          = g_rx_fifo_buffer,
///     .tx_fifo_buffer          = g_tx_fifo_buffer,
///   };
///   sl_rail_status_t status;
///
///   // Initializes the RAIL library and any internal state it requires.
///   status = sl_rail_init(&g_rail_handle, &rail_config, NULL);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///
///   // Calls the BLE initialization function to load the right radio configuration.
///   status = sl_rail_ble_init(g_rail_handle);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///
///   status = sl_rail_ble_config_phy_1_mbps(g_rail_handle);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///
///   // Configures us for the first advertising channel (Physical: 0, Logical: 37).
///   // The CRC init value and Access Address come from the BLE specification.
///   sl_rail_ble_state_t ble_params = {
///     .crc_init = 0x555555,
///     .access_address = 0x0x8E89BED6,
///     .logical_channel = 37,
///     .disable_whitening = false,
///   };
///   status = sl_rail_ble_config_channel_radio_params(g_rail_handle, &ble_params);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///
///   // Starts receiving on physical channel 0 (logical channel 37).
///   status = sl_rail_start_rx(g_rail_handle, 0, NULL);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///  }
/// @endcode
///
/// @{
///

/**
 * @enum sl_rail_ble_coding_t
 * @brief The variant of the BLE Coded PHY.
 */
SLI_RAIL_ENUM(sl_rail_ble_coding_t) {
  /**
   * Enables the 125 kbps variant of the BLE Coded PHY.
   */
  SL_RAIL_BLE_CODING_125_KBPS = 0u,
  /**
   * Enables the 500 kbps variant of the BLE Coded PHY.
   */
  SL_RAIL_BLE_CODING_500_KBPS = 2u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_BLE_CODING_125_KBPS     ((sl_rail_ble_coding_t) SL_RAIL_BLE_CODING_125_KBPS)
#define SL_RAIL_BLE_CODING_500_KBPS     ((sl_rail_ble_coding_t) SL_RAIL_BLE_CODING_500_KBPS)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/**
 * @enum sl_rail_ble_phy_t
 * @brief The BLE PHY identifier (aka PTI radio configuration id).
 */
SLI_RAIL_ENUM(sl_rail_ble_phy_t) {
  /**
   * BLE 1 Mbps PHY.
   */
  SL_RAIL_BLE_PHY_1_MBPS = 0u,
  /**
   * BLE 2 Mbps PHY.
   */
  SL_RAIL_BLE_PHY_2_MBPS = 1u,
  /**
   * BLE 125 kbps coded PHY.
   */
  SL_RAIL_BLE_PHY_CODED_125_KBPS = 2u,
  /**
   * BLE 500 kbps coded PHY.
   */
  SL_RAIL_BLE_PHY_CODED_500_KBPS = 3u,
  /**
   * BLE Simulscan PHY.
   */
  SL_RAIL_BLE_PHY_SIMULSCAN = 4u,
  /**
   * BLE 2 Mbps PHY with AoX functionality.
   */
  SL_RAIL_BLE_PHY_AOX_2_MBPS = 5u,
  /**
   * BLE 1 Mbps Quuppa PHY.
   */
  SL_RAIL_BLE_PHY_QUUPPA_1_MBPS = 6u,
  /**
   * BLE 1 Mbps PHY with CS.
   */
  SL_RAIL_BLE_PHY_CS_1_MBPS = 7u,
  /**
   * BLE 2 Mbps PHY with CS.
   */
  SL_RAIL_BLE_PHY_CS_2_MBPS = 8u,
  /**
   * BLE PHY undefined.
   */
  SL_RAIL_BLE_PHY_UNDEFINED = 0xFFu,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_BLE_PHY_1_MBPS         ((sl_rail_ble_phy_t) SL_RAIL_BLE_PHY_1_MBPS)
#define SL_RAIL_BLE_PHY_2_MBPS         ((sl_rail_ble_phy_t) SL_RAIL_BLE_PHY_2_MBPS)
#define SL_RAIL_BLE_PHY_CODED_125_KBPS ((sl_rail_ble_phy_t) SL_RAIL_BLE_PHY_CODED_125_KBPS)
#define SL_RAIL_BLE_PHY_CODED_500_KBPS ((sl_rail_ble_phy_t) SL_RAIL_BLE_PHY_CODED_500_KBPS)
#define SL_RAIL_BLE_PHY_SIMULSCAN      ((sl_rail_ble_phy_t) SL_RAIL_BLE_PHY_SIMULSCAN)
#define SL_RAIL_BLE_PHY_AOX_2_MBPS     ((sl_rail_ble_phy_t) SL_RAIL_BLE_PHY_AOX_2_MBPS)
#define SL_RAIL_BLE_PHY_QUUPPA_1_MBPS  ((sl_rail_ble_phy_t) SL_RAIL_BLE_PHY_QUUPPA_1_MBPS)
#define SL_RAIL_BLE_PHY_CS_1_MBPS      ((sl_rail_ble_phy_t) SL_RAIL_BLE_PHY_CS_1_MBPS)
#define SL_RAIL_BLE_PHY_CS_2_MBPS      ((sl_rail_ble_phy_t) SL_RAIL_BLE_PHY_CS_2_MBPS)
#endif //DOXYGEN_SHOULD_SKIP_THIS

///
/// @addtogroup BLE_PHY BLE Radio Configurations
/// Radio configurations for the RAIL BLE Accelerator
///
/// These radio configurations are used to configure BLE when a function such
/// as \ref sl_rail_ble_config_phy_1_mbps() is called. Each radio
/// configuration listed below is compiled into the RAIL library as a weak
/// symbol that will take into account per-die defaults. If the board
/// configuration in use has different settings than the default, such as a
/// different radio subsystem clock frequency, these radio configurations can
/// be overriden to account for those settings.
/// @{
///

/**
 * Default PHY to use for BLE 1 Mbps. Will be NULL if
 * \ref SL_RAIL_BLE_SUPPORTS_1_MBPS is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ble_phy_1_mbps;

/**
 * Default PHY to use for BLE 2 Mbps. Will be NULL if
 * \ref SL_RAIL_BLE_SUPPORTS_2_MBPS is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ble_phy_2_mbps;

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * Default PHY to use for BLE 1 Mbps CS. Will be NULL if
 * \ref SL_RAIL_BLE_SUPPORTS_CS is 0. On EFR32xG24, this will also
 * be NULL for non 40MHz HFXO frequencies.
 */
extern const sl_rail_channel_config_t *const sl_rail_ble_phy_1_mbps_cs;

/**
 * Default PHY to use for BLE 2 Mbps CS. Will be NULL if
 * \ref SL_RAIL_BLE_SUPPORTS_CS is 0. On EFR32xG24, this will also
 * be NULL for non 40MHz HFXO frequencies.
 */
extern const sl_rail_channel_config_t *const sl_rail_ble_phy_2_mbps_cs;
#endif//DOXYGEN_UNDOCUMENTED

/**
 * PHY to use for BLE 2 Mbps with AoX functionality. Will be NULL if either
 * \ref SL_RAIL_BLE_SUPPORTS_2_MBPS or \ref SL_RAIL_BLE_SUPPORTS_AOX is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ble_phy_2_mbps_aox;

/**
 * Default PHY to use for BLE Coded 125 kbps. Will be NULL if
 * \ref SL_RAIL_BLE_SUPPORTS_CODED_PHY is 0. This PHY can receive on both
 * 125 kbps and 500 kbps BLE Coded, but will only transmit at 125 kbps.
 */
extern const sl_rail_channel_config_t *const sl_rail_ble_phy_125_kbps;

/**
 * Default PHY to use for BLE Coded 500 kbps. Will be NULL if
 * \ref SL_RAIL_BLE_SUPPORTS_CODED_PHY is 0. This PHY can receive on both
 * 125 kbps and 500 kbps BLE Coded, but will only transmit at 125 kbps.
 */
extern const sl_rail_channel_config_t *const sl_rail_ble_phy_500_kbps;

/**
 * Default PHY to use for BLE Simulscan. Will be NULL if
 * \ref SL_RAIL_BLE_SUPPORTS_SIMULSCAN_PHY is 0. This PHY can receive on 1 Mbps
 * as well as 125 kbps and 500 kbps BLE Coded, but will only transmit at 1 Mbps.
 */
extern const sl_rail_channel_config_t *const sl_rail_ble_phy_simulscan;

/**
 * Default 1 Mbps Quuppa PHY. Will be NULL if
 * \ref SL_RAIL_BLE_SUPPORTS_QUUPPA is 0.
 */
extern const sl_rail_channel_config_t *const sl_rail_ble_phy_quuppa;

/// @} // End of group BLE_PHY

/**
 * \ref sl_rail_rx_packet_details_t::sub_phy_id indicating a 500 kbps packet.
 */
#define SL_RAIL_BLE_RX_SUBPHY_ID_500_KBPS     (0U)
/**
 * \ref sl_rail_rx_packet_details_t::sub_phy_id indicating a 125 kbps packet.
 */
#define SL_RAIL_BLE_RX_SUBPHY_ID_125_KBPS     (1U)
/**
 * \ref sl_rail_rx_packet_details_t::sub_phy_id value indicating a 1 Mbps packet.
 */
#define SL_RAIL_BLE_RX_SUBPHY_ID_1_MBPS       (2U)
/**
 * \ref sl_rail_rx_packet_details_t::sub_phy_id invalid value.
 */
#define SL_RAIL_BLE_RX_SUBPHY_ID_INVALID  (3U)
/**
 * The total count of BLE sub_phy_id's. Must be last.
 */
#define SL_RAIL_BLE_RX_SUBPHY_COUNT       (4U)

/**
 * @enum sl_rail_ble_signal_identifier_mode_t
 * @brief Available Signal Identifier modes.
 */
SLI_RAIL_ENUM(sl_rail_ble_signal_identifier_mode_t) {
  /**
   * Disable signal detection mode.
   */
  SL_RAIL_BLE_SIGNAL_IDENTIFIER_MODE_DISABLE = 0u,
  /**
   * BLE 1 Mbps (GFSK) detection mode with SI block reset on GPIO falling edge (via PRS).
   */
  SL_RAIL_BLE_SIGNAL_IDENTIFIER_MODE_1_MBPS = 1u,
  /**
   * BLE 2 Mbps (GFSK) detection mode with SI block reset on GPIO falling edge (via PRS).
   */
  SL_RAIL_BLE_SIGNAL_IDENTIFIER_MODE_2_MBPS = 2u,
  /**
   * BLE 1 Mbps (GFSK) detection mode with SI block reset on AGC negative step.
   */
  SL_RAIL_BLE_SIGNAL_IDENTIFIER_MODE_1_MBPS_WITH_AGC_RESET = 3u,
  /**
   * BLE 2 Mbps (GFSK) detection mod with SI block reset on AGC negative step.
   */
  SL_RAIL_BLE_SIGNAL_IDENTIFIER_MODE_2_MBPS_WITH_AGC_RESET = 4u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_BLE_SIGNAL_IDENTIFIER_MODE_DISABLE ((sl_rail_ble_signal_identifier_mode_t) SL_RAIL_BLE_SIGNAL_IDENTIFIER_MODE_DISABLE)
#define SL_RAIL_BLE_SIGNAL_IDENTIFIER_MODE_1_MBPS  ((sl_rail_ble_signal_identifier_mode_t) SL_RAIL_BLE_SIGNAL_IDENTIFIER_MODE_1_MBPS)
#define SL_RAIL_BLE_SIGNAL_IDENTIFIER_MODE_2_MBPS  ((sl_rail_ble_signal_identifier_mode_t) SL_RAIL_BLE_SIGNAL_IDENTIFIER_MODE_2_MBPS)
#define SL_RAIL_BLE_SIGNAL_IDENTIFIER_MODE_1_MBPS_WITH_AGC_RESET  ((sl_rail_ble_signal_identifier_mode_t) SL_RAIL_BLE_SIGNAL_IDENTIFIER_MODE_1_MBPS_WITH_AGC_RESET)
#define SL_RAIL_BLE_SIGNAL_IDENTIFIER_MODE_2_MBPS_WITH_AGC_RESET  ((sl_rail_ble_signal_identifier_mode_t) SL_RAIL_BLE_SIGNAL_IDENTIFIER_MODE_2_MBPS_WITH_AGC_RESET)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @struct sl_rail_ble_state_t
 * @brief A structure for BLE radio state parameters.
 */
typedef struct sl_rail_ble_params {
  /**
   * The value used to initialize the CRC algorithm.
   */
  uint32_t crc_init;
  /**
   * The access address used for the connection.
   * It is transmitted or received least-significant bit first.
   */
  uint32_t access_address;
  /**
   * The logical channel used.
   */
  uint16_t logical_channel;
  /**
   * Indicates whether the whitening engine should be off (generally used for testing only).
   */
  bool disable_whitening;
  /**
   * Reserved for future use; specify 0.
   */
  uint16_t white_init;
} sl_rail_ble_state_t;

/**
 * Configure RAIL to run in BLE mode.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This function changes your radio, channel configuration, and other
 * parameters to match what is needed for BLE, initially establishing
 * the BLE 1 Mbps PHY. To switch back to a
 * default RAIL mode, call \ref sl_rail_ble_deinit() first. This function
 * will configure the protocol output on PTI to \ref SL_RAIL_PTI_PROTOCOL_BLE.
 *
 * @note BLE may not be enabled while Auto-ACKing is enabled.
 */
sl_rail_status_t sl_rail_ble_init(sl_rail_handle_t rail_handle);

/**
 * Take RAIL out of BLE mode.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This function will undo some of the configuration that happens when you call
 * \ref sl_rail_ble_init(). After this you can safely run your normal radio
 * initialization code to use a non-BLE configuration. This function does \b
 * not change back your radio or channel configurations so you must do this by
 * manually reinitializing. This also resets the protocol output on PTI to \ref
 * SL_RAIL_PTI_PROTOCOL_CUSTOM.
 */
sl_rail_status_t sl_rail_ble_deinit(sl_rail_handle_t rail_handle);

/**
 * Determine whether BLE mode is enabled or not.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return true if BLE mode is enabled and false otherwise.
 *
 * This function returns the current status of RAIL's BLE mode. It is enabled by
 * a call to \ref sl_rail_ble_init() and disabled by a call to \ref sl_rail_ble_deinit().
 */
bool sl_rail_ble_is_enabled(sl_rail_handle_t rail_handle);

/**
 * Load BLE channel config.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_channel_config A pointer to the channel configuration structure.
 * @param[in] phy_id The PHY ID to configure.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_ble_config_channels(sl_rail_handle_t rail_handle,
                                             const sl_rail_channel_config_t *p_channel_config,
                                             sl_rail_ble_phy_t phy_id);

/**
 * Switch to the 1 Mbps Quuppa PHY.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * You can use this function to switch to the Quuppa PHY.
 *
 * @note Not all chips support the 1 Mbps Quuppa PHY. This API should return \ref SL_RAIL_STATUS_INVALID_CALL if
 *   unsupported by the hardware we're building for.
 */
sl_rail_status_t sl_rail_ble_config_phy_quuppa(sl_rail_handle_t rail_handle);

/**
 * Switch to the 1 Mbps BLE PHY.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * Use this function to switch back to the default BLE 1 Mbps PHY if you
 * have switched to the 2 Mbps or another configuration. You may only call this
 * function after initializing BLE and while the radio is idle.
 */
sl_rail_status_t sl_rail_ble_config_phy_1_mbps(sl_rail_handle_t rail_handle);

/**
 * Switch to the 2 Mbps BLE PHY.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * Use this function to switch back to the BLE 2 Mbps PHY from the
 * default 1 Mbps option. You may only call this function after initializing BLE
 * and while the radio is idle.
 */
sl_rail_status_t sl_rail_ble_config_phy_2_mbps(sl_rail_handle_t rail_handle);

/**
 * Switch to the BLE Coded PHY.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] ble_coding The \ref sl_rail_ble_coding_t to use.
 * @return Status code indicating success of the function call.
 *
 * Use this function to switch back to BLE Coded PHY from the default
 * 1 Mbps option. You may only call this function after initializing BLE and
 * while the radio is idle. When using a BLE Coded PHY, the \ref
 * sl_rail_rx_packet_details_t::sub_phy_id marks the coding of the received packet.
 * A sub_phy_id of 0 marks a 500 kbps packet, and a sub_phy_id of 1 marks a 125
 * kbps packet.
 */
sl_rail_status_t sl_rail_ble_config_phy_coded(sl_rail_handle_t rail_handle,
                                              sl_rail_ble_coding_t ble_coding);

/**
 * Switch to the Simulscan PHY.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * Use this function to switch to the BLE Simulscan PHY. You may only
 * call this function after initializing BLE and while the radio is idle.
 * When using Simulscan PHY, the \ref sl_rail_rx_packet_details_t::sub_phy_id
 * marks the coding of the received packet. A sub_phy_id of 0 marks a
 * 500 kbps packet, a sub_phy_id of 1 marks a 125 kbps packet, and a
 * sub_phy_id of 2 marks a 1 Mbps packet.
 *
 * @note: The Simulscan PHY is supported only on some parts.
 *   The preprocessor symbol \ref SL_RAIL_BLE_SUPPORTS_SIMULSCAN_PHY and the
 *   runtime function \ref sl_rail_ble_supports_simulscan_phy() may be used to
 *   test for support of the Simulscan PHY.
 */
sl_rail_status_t sl_rail_ble_config_phy_simulscan(sl_rail_handle_t rail_handle);

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * Switch to the 1 Mbps BLE PHY for CS.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * Use this function to switch back to the BLE 1 Mbps CS PHY from
 * another configuration. You may only call this
 * function after initializing BLE and while the radio is idle.
 *
 * @note This PHY is only supported when \ref SL_RAIL_BLE_SUPPORTS_CS is not 0.
 */
sl_rail_status_t sl_rail_ble_config_phy_1_mbps_cs(sl_rail_handle_t rail_handle);

/**
 * Switch to the 2 Mbps BLE PHY for CS.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * Use this function to switch back to the BLE 2 Mbps CS PHY from
 * another configuration. You may only call this
 * function after initializing BLE and while the radio is idle.
 *
 * @note This PHY is only supported when \ref SL_RAIL_BLE_SUPPORTS_CS is not 0.
 */
sl_rail_status_t sl_rail_ble_config_phy_2_mbps_cs(sl_rail_handle_t rail_handle);
#endif //DOXYGEN_UNDOCUMENTED

/**
 * Change BLE radio parameters.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_params A non-NULL pointer to a sl_rail_ble_state_t structure specifying
 *   the BLE radio state parameters to use.
 * @return Status code indicating success of the function call.
 *
 * This function can be used to switch radio parameters on every connection
 * and/or channel change. It is BLE-aware and will set the access address,
 * preamble, CRC initialization value, and whitening configuration without
 * requiring you to load a new radio configuration. This function should be
 * called after switching to a particular BLE phy (1 Mbps, 2 Mbps, etc.) and
 * not while the radio is active.
 */
sl_rail_status_t sl_rail_ble_config_channel_radio_params(sl_rail_handle_t rail_handle,
                                                         const sl_rail_ble_state_t *p_params);

/**
 * Change the current BLE PHY and go into receive.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] phy Indicates which PHY to receive on.
 * @param[in] rail_channel Which channel of the given PHY to receive on.
 * @param[in] start_rx_time Absolute near-future RAIL time to enter RX.
 * @param[in] p_params A non-NULL pointer to the parameters for this operation.
 *   \ref sl_rail_ble_state_t::logical_channel should be the logical
 *   channel that you're changing to, which initializes the whitener if used.
 * @return Status code indicating success of the function call.
 *
 * This function is used to implement auxiliary packet reception, as defined in
 * the BLE specification. The radio will be put into IDLE, the PHY and channel
 * will be changed, and then receive will be entered at the start time given.
 * The new receive will have a timeout of 30 us, which means that this function
 * should only be called if the offset unit is 30 us.
 *
 * This function is extremely time-sensitive, and may only be called within the
 * interrupt context of a \ref SL_RAIL_EVENT_RX_PACKET_RECEIVED event.
 */
sl_rail_status_t sl_rail_ble_phy_switch_to_rx(sl_rail_handle_t rail_handle,
                                              sl_rail_ble_phy_t phy,
                                              uint16_t rail_channel,
                                              sl_rail_time_t start_rx_time,
                                              const sl_rail_ble_state_t *p_params);

/**
 * Configure and enable signal identifier for BLE signal detection.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] signal_identifier_mode Mode of signal identifier operation.
 * @return Status code indicating success of the function call.
 *
 * This features allows detection of BLE signal on air based on the mode.
 * This function must be called once before \ref sl_rail_ble_enable_signal_detection
 * to configure and enable signal identifier.
 *
 * To enable event for signal detection \ref sl_rail_config_events() must be called
 * for enabling \ref SL_RAIL_EVENT_SIGNAL_DETECTED.
 *
 * This function is only supported by chips where
 * \ref SL_RAIL_BLE_SUPPORTS_SIGNAL_IDENTIFIER and
 * \ref sl_rail_ble_supports_signal_identifier() are true.
 */
sl_rail_status_t sl_rail_ble_config_signal_identifier(sl_rail_handle_t rail_handle,
                                                      sl_rail_ble_signal_identifier_mode_t signal_identifier_mode);

/**
 * Enable or disable signal identifier interrupt for BLE signal detection.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] enable Signal detection is enabled if true, disabled if false.
 * @return Status code indicating success of the function call.
 *
 * \ref sl_rail_ble_config_signal_identifier() must be called once before calling this
 * function to configure and enable signal identifier.
 * Once a signal is detected signal detection will be turned off and this
 * function should be called to re-enable the signal detection without needing
 * to call \ref sl_rail_ble_config_signal_identifier() if the signal identifier
 * is already configured and enabled.
 *
 * This function is only supported by chips where
 * \ref SL_RAIL_BLE_SUPPORTS_SIGNAL_IDENTIFIER and
 * \ref sl_rail_ble_supports_signal_identifier() are true.
 */
sl_rail_status_t sl_rail_ble_enable_signal_detection(sl_rail_handle_t rail_handle,
                                                     bool enable);

/******************************************************************************
 * Angle of Arrival/Departure (AoX)
 *****************************************************************************/
/**
 * @addtogroup AoX Angle of Arrival/Departure
 * @{
 * @brief These APIs are to a stack implementing BLE's angle of arrival and
 *   angle of departure functionality.
 *
 * They are designed for use by the Silicon Labs BLE stack only at this time and
 * may cause problems if accessed directly.
 */

/**
 * The maximum number of GPIO pins used for AoX Antenna switching.
 *
 * If the user configures more pins using
 * \ref sl_rail_ble_config_aox_antenna() than allowed
 * \ref SL_RAIL_BLE_AOX_ANTENNA_PIN_COUNT, then
 * \ref SL_RAIL_STATUS_INVALID_PARAMETER status will be returned.
 *
 * \ref SL_RAIL_STATUS_INVALID_CALL is returned if
 * \ref SL_RAIL_BLE_AOX_ANTENNA_PIN_COUNT is set to 0 or
 * the user configures no pins.
 *
 * The maximum value \ref SL_RAIL_BLE_AOX_ANTENNA_PIN_COUNT can take depends on
 * number of Antenna route pins , a chip provides.
 * For EFR32xG22, the maximum value of \ref SL_RAIL_BLE_AOX_ANTENNA_PIN_COUNT is 6.
 * If the user configures fewer pins than \ref SL_RAIL_BLE_AOX_ANTENNA_PIN_COUNT,
 * then only number of pins asked by user will be configured with
 * \ref SL_RAIL_STATUS_NO_ERROR.
 */
#define SL_RAIL_BLE_AOX_ANTENNA_PIN_COUNT (6U)

/**
 * @enum sl_rail_ble_aox_options_t
 * @brief Angle of Arrival/Departure options bit fields
 */
SLI_RAIL_ENUM_GENERIC(sl_rail_ble_aox_options_t, uint16_t) {
  /**
   * Shift position of \ref SL_RAIL_BLE_AOX_OPTIONS_SAMPLE_MODE bit.
   */
  SL_RAIL_BLE_AOX_OPTIONS_SAMPLE_MODE_SHIFT = 0,
  /**
   * Shift position of \ref SL_RAIL_BLE_AOX_OPTIONS_CONNLESS bit.
   */
  SL_RAIL_BLE_AOX_OPTIONS_CONNLESS_SHIFT = 1,
  /**
   * Shift position of \ref SL_RAIL_BLE_AOX_OPTIONS_CONN bit.
   */
  SL_RAIL_BLE_AOX_OPTIONS_CONN_SHIFT = 2,
  /**
   * Shift position of \ref SL_RAIL_BLE_AOX_OPTIONS_DISABLE_BUFFER_LOCK bit.
   */
  SL_RAIL_BLE_AOX_OPTIONS_DISABLE_BUFFER_LOCK_SHIFT = 3,
};

/**
 * Disable the AoX feature.
 */
#define SL_RAIL_BLE_AOX_OPTIONS_DISABLED            (0U)
/**
 * Sets one of the two AoX sampling/switching modes: 1 us or 2 us window.
 */
#define SL_RAIL_BLE_AOX_OPTIONS_SAMPLE_MODE         (1U << SL_RAIL_BLE_AOX_OPTIONS_SAMPLE_MODE_SHIFT)
/**
 * Enables connectionless AoX Rx packets.
 */
#define SL_RAIL_BLE_AOX_OPTIONS_CONNLESS            (1U << SL_RAIL_BLE_AOX_OPTIONS_CONNLESS_SHIFT)
/**
 * Enables connection based AoX Rx packets.
 */
#define SL_RAIL_BLE_AOX_OPTIONS_CONN                (1U << SL_RAIL_BLE_AOX_OPTIONS_CONN_SHIFT)
/**
 * Disables CTE buffer lock.
 */
#define SL_RAIL_BLE_AOX_OPTIONS_DISABLE_BUFFER_LOCK (1U << SL_RAIL_BLE_AOX_OPTIONS_DISABLE_BUFFER_LOCK_SHIFT)
/**
 * Enables connection based or connectionless AoX Rx packets.
 */
#define SL_RAIL_BLE_AOX_OPTIONS_ENABLED             (SL_RAIL_BLE_AOX_OPTIONS_CONN | SL_RAIL_BLE_AOX_OPTIONS_CONNLESS)

/**
 * @struct sl_rail_ble_aox_config_t
 * @brief Contains arguments for \ref sl_rail_ble_config_aox() function.
 */
typedef struct sl_rail_ble_aox_config {
  /**
   * AoX options.
   */
  sl_rail_ble_aox_options_t aox_options;
  /**
   * Size of the raw AoX CTE (continuous tone extension) data capture buffer in
   * bytes. Note this value should be a multiple of 4 as each IQ sample
   * requires 4 bytes.
   */
  uint16_t cte_buffer_bytes;
  /**
   * Address to where the received CTE is written. Buffer must be 32-bit
   * aligned.
   */
  uint32_t *p_cte_buffer;
  /**
   * Address to first element of antenna pattern array. Array must be in RAM.
   * Each element of the array contains an antenna number. The switching pattern
   * is defined by the order of antennas in this array.
   */
  uint8_t *p_antenna_array;
  /**
   * Number of entries in the antenna pattern array.
   */
  uint8_t antenna_array_entries;
} sl_rail_ble_aox_config_t;

/**
 * @struct sl_rail_ble_aox_antenna_port_pins_t
 * @brief Contains elements of \ref sl_rail_ble_aox_antenna_config_t struct.
 */
typedef struct sl_rail_ble_aox_antenna_port_pins {
  /**
   * The port which is used for AoX antenna switching.
   */
  uint8_t antenna_port;
  /**
   * The pin which is used for AoX antenna switching.
   */
  uint8_t antenna_pin;
} sl_rail_ble_aox_antenna_port_pins_t;

/**
 * @struct sl_rail_ble_aox_antenna_config_t
 * @brief Contains arguments for \ref sl_rail_ble_config_aox_antenna() function.
 */
typedef struct sl_rail_ble_aox_antenna_config {
  /**
   * A pointer to an array containing struct of port and pin used for
   * AoX antenna switching.
   */
  sl_rail_ble_aox_antenna_port_pins_t *p_antenna_port_pin;
  /**
   * Number of antenna pins to be configured.
   */
  uint8_t antenna_count;
} sl_rail_ble_aox_antenna_config_t;

/**
 * Lock/unlock the CTE buffer from the application's perspective. The radio
 * will write to the buffer only if the bit is NOT set at the beginning of the
 * sampling period. The radio will set the bit once the sampling period starts
 * to indicate that some CTE data has been collected, which will not be
 * overwritten during the next sampling period, unless the buffer is unlocked by
 * the application.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] lock Lock the CTE buffer if true and unlock it if false.
 * @return true if the CTE buffer is locked after the call, otherwise false.
 */
bool sl_rail_ble_lock_cte_buffer(sl_rail_handle_t rail_handle, bool lock);

/**
 * Determine whether the CTE buffer is currently locked or not.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return true if CTE buffer is locked and false otherwise.
 */
bool sl_rail_ble_cte_buffer_is_locked(sl_rail_handle_t rail_handle);

/**
 * Get the offset into CTE sample of CTE data.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The offset of CTE data in a CTE sample in bytes.
 *   On unsupported platforms this returns 0.
 */
uint8_t sl_rail_ble_get_cte_sample_offset(sl_rail_handle_t rail_handle);

/**
 * Get the effective sample rate used by the ADC to capture the CTE samples.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The actual sample rate used to capture the CTE in samples per second.
 * On unsupported platforms this returns 0.
 */
uint32_t sl_rail_ble_get_cte_sample_rate(sl_rail_handle_t rail_handle);

/**
 * Configure Angle of Arrival/Departure (AoX) functionality.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_aox_config A pointer to AoX configuration settings.
 * @return Status code indicating success of the function call.
 *
 * AoX is a method
 * of radio localization which infers angle of arrival/departure of the signal
 * based on different phases of the raw I/Q signal from different antennas by
 * controlling external RF switch during the continuous tone extension (CTE).
 * Connection based AoX packets are different than normal BLE packets in that
 * they have 3 header bytes instead of 2 and they have CTE appended after the
 * payload's CRC. 3rd byte or CTE info contains CTE length. Connectionless AoX
 * packets have 2 header bytes and CTE info is part of the payload.
 *
 * @note Calling \ref sl_rail_get_radio_entropy() during AoX reception may break
 *   packet reception.
 */
sl_rail_status_t sl_rail_ble_config_aox(sl_rail_handle_t rail_handle,
                                        const sl_rail_ble_aox_config_t *p_aox_config);

/**
 * Perform one time initialization of AoX registers.
 * This function must be called before \ref sl_rail_ble_config_aox()
 * and before configuring the BLE PHY.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_ble_init_cte(sl_rail_handle_t rail_handle);

/**
 * Perform initialization of AoX antenna GPIO pins.
 * This function must be called before calls to \ref sl_rail_ble_init_cte()
 * and \ref sl_rail_ble_config_aox(), and before configuring the BLE PHY,
 * else a \ref SL_RAIL_STATUS_INVALID_CALL is returned.
 *
 * If user configures more pins in \ref sl_rail_ble_aox_antenna_config_t::antenna_count
 * than allowed by \ref SL_RAIL_BLE_AOX_ANTENNA_PIN_COUNT, then the API returns
 * \ref SL_RAIL_STATUS_INVALID_PARAMETER.
 *
 * If user configures lesser than or equal to number of pins allowed by
 * \ref SL_RAIL_BLE_AOX_ANTENNA_PIN_COUNT, then the requested number of pins
 * are configured and \ref SL_RAIL_STATUS_NO_ERROR is returned.
 *
 * If AoX antenna switching is inactive, non-AoX transmits and receives
 * will occur on the first antenna specified by the antenna pattern or
 * on the default antenna if no antenna pattern is provided.
 *
 * @note This function must only be called while channel sounding is disabled.
 *   If channel sounding has been enabled \ref SL_RAIL_STATUS_INVALID_STATE
 *   will be returned.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_antenna_config A pointer to the antenna configuration
 *   structure to hold the set of GPIO ports and pins for AoX antenna
 *   switching.
 * @return Status code indicating success of the function call.
 *
 * @note Use the compile time symbol \ref
 *   SL_RAIL_BLE_SUPPORTS_ANTENNA_SWITCHING or the runtime call \ref
 *   sl_rail_ble_supports_antenna_switching() to check whether the platform
 *   supports this feature.
 *
 * @warning As this function relies on GPIO access and RAIL is meant to run in
 *   TrustZone non-secure world, it is not supported if GPIO is configured as
 *   secure peripheral and it will return \ref SL_RAIL_STATUS_INVALID_CALL.
 */
sl_rail_status_t sl_rail_ble_config_aox_antenna(sl_rail_handle_t rail_handle,
                                                const sl_rail_ble_aox_antenna_config_t *p_antenna_config);

/** @} */  // end of group AoX

#ifndef DOXYGEN_UNDOCUMENTED
/******************************************************************************
 * Channel Sounding (CS)
 *****************************************************************************/
/**
 * @addtogroup CS Channel Sounding
 * @{
 * @brief These APIs are to a stack implementing BLE's channel sounding
 *   functionality.
 *
 * They are designed for use by the Silicon Labs BLE stack only at this time and
 * may cause problems if accessed directly.
 */

/**
 * Total number of CS Channels.
 */
#define SL_RAIL_BLE_CS_NUM_CHANNELS 79

/**
 * Total number of allowed CS Channels.
 */
#define SL_RAIL_BLE_CS_NUM_ALLOWED_CHANNELS 72

/**
 * @enum sl_rail_ble_cs_role_t
 * @brief The device role during CS events.
 */
SLI_RAIL_ENUM(sl_rail_ble_cs_role_t) {
  /**
   * Device cannot perform CS events.
   */
  SL_RAIL_BLE_CS_ROLE_UNASSIGNED = 0u,
  /**
   * Device is an initiator during CS events.
   */
  SL_RAIL_BLE_CS_ROLE_INITIATOR = 1u,
  /**
   * Device is a reflector during CS events.
   */
  SL_RAIL_BLE_CS_ROLE_REFLECTOR = 2u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_BLE_CS_ROLE_UNASSIGNED ((sl_rail_ble_cs_role_t) SL_RAIL_BLE_CS_ROLE_UNASSIGNED)
#define SL_RAIL_BLE_CS_ROLE_INITIATOR  ((sl_rail_ble_cs_role_t) SL_RAIL_BLE_CS_ROLE_INITIATOR)
#define SL_RAIL_BLE_CS_ROLE_REFLECTOR  ((sl_rail_ble_cs_role_t) SL_RAIL_BLE_CS_ROLE_REFLECTOR)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @struct sl_rail_ble_cs_results_t
 * @brief Contains measurement results from CS step.
 */
typedef struct {
  /**
   * CS measurement data for a particular step.
   */
  uint32_t result[7];
} sl_rail_ble_cs_results_t;

/**
 * @enum sl_rail_ble_cs_rtt_type_t
 * @brief CS RTT Types.
 */
SLI_RAIL_ENUM(sl_rail_ble_cs_rtt_type_t) {
  /**
   * Coarse cost function engine method RTT.
   */
  SL_RAIL_BLE_CS_RTT_AA_ONLY = 0U,
  /**
   * 32 bit sounding sequence method RTT.
   */
  SL_RAIL_BLE_CS_RTT_32_BIT_SS = 1U,
  /**
   * 96 bit sounding sequence method RTT.
   */
  SL_RAIL_BLE_CS_RTT_96_BIT_SS = 2U,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_BLE_CS_RTT_AA_ONLY   ((sl_rail_ble_cs_rtt_type_t) SL_RAIL_BLE_CS_RTT_AA_ONLY)
#define SL_RAIL_BLE_CS_RTT_32_BIT_SS ((sl_rail_ble_cs_rtt_type_t) SL_RAIL_BLE_CS_RTT_32_BIT_SS)
#define SL_RAIL_BLE_CS_RTT_96_BIT_SS ((sl_rail_ble_cs_rtt_type_t) SL_RAIL_BLE_CS_RTT_96_BIT_SS)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 *  The minimum size in 32 bit words for the IQ buffer. This value guarantees
 *  all IQ samples for a single 1 Mbps CS step can be stored.
 */
#define SL_RAIL_BLE_CS_1_MBPS_MINIMUM_IQ_BUFFER_WORDS  1500U

/**
 * @struct sl_rail_ble_cs_config_t
 * @brief Contains arguments for \ref sl_rail_ble_config_cs() function.
 */
typedef struct sl_rail_ble_cs_config {
  /**
   * The device role during CS event.
   */
  sl_rail_ble_cs_role_t role;
  /**
   * Number of mode 2 phase measurement slots, including the
   * tone extension slot. This value should be between
   * \ref SL_RAIL_BLE_CS_MIN_ANTENNA_SLOTS and
   * \ref SL_RAIL_BLE_CS_MAX_ANTENNA_SLOTS, inclusive.
   * A provided value below or above this range will be pegged
   * to the appropriate minimum or maximum value.
   */
  uint8_t slot_count;
  /**
   * Number of steps in CS event.
   */
  uint16_t cs_sqte_steps;
  /**
   * Pointer to CS measurements. Set to NULL if unused.
   */
  sl_rail_ble_cs_results_t *p_cs_data_output;
  /**
   * Frequency change spacing (in us).
   */
  uint16_t t_fcs_us;
  /**
   * Interlude period for mode 0 & 1 steps (in us).
   */
  uint16_t t_ip1_us;
  /**
   * Interlude period for mode 2 steps (in us).
   */
  uint16_t t_ip2_us;
  /**
   * Phase measurement time (in us).
   */
  uint16_t t_pm_us;
  /**
   * Antenna switching time (in us).
   */
  uint16_t t_sw_us;
  /**
   * Pointer to buffer where IQ data will be written. Buffer must be 32-bit
   * aligned.
   */
  uint32_t *p_iq_buffer;
  /**
   * Size of IQ buffer in 32 bit words. Must be at least \ref
   * SL_RAIL_BLE_CS_1_MBPS_MINIMUM_IQ_BUFFER_WORDS or else an error will be
   * returned by \ref sl_rail_ble_config_cs().
   */
  uint16_t iq_buffer_words;
  /**
   * Step Index to perform the event calibration. This index must correspond
   * to a mode 0 step or else the event calibration won't occur.
   */
  uint8_t event_cal_step_index;
  /**
   * RTT type returned during mode 1 step.
   */
  sl_rail_ble_cs_rtt_type_t rtt_type;
  /**
   * A pointer to the selected CS event gain index. This field will be
   * populated after \ref event_cal_step_index has been reached.
   */
  uint8_t *p_event_gain_index;
  /**
   * A pointer to the selected CS event Fractional Frequency Offset
   * (FFO) * pp100m (parts-per-100-million). This field will be populated
   * after \ref event_cal_step_index has been reached.
   */
  int16_t *p_event_ffo_pp100m;
  /**
   * Debug flag to disable RTT GD compensation.
   */
  bool disable_rtt_gd_comp;
  /**
   * Debug flag to disable PBR DC compensation.
   */
  bool disable_pbr_dc_comp;
  /**
   * Debug flag to disable PBR GD compensation.
   */
  bool disable_pbr_gd_comp;
  /**
   * Debug flag to force event gain for calibration.
   */
  bool force_agc_gain;
  /**
   * Pointer to an FAE table of size \ref SL_RAIL_BLE_CS_NUM_ALLOWED_CHANNELS
   * that holds the FAE value for each allowed CS channel in units of
   * parts-per-32-million (pp32m). In units of parts-per-million (ppm),
   * the FAE ranges from [-4, +3.96875] ppm with a resolution of 0.03125 ppm.
   * Set to NULL if unused.
   */
  int8_t(*p_fae_table)[SL_RAIL_BLE_CS_NUM_ALLOWED_CHANNELS];
  /**
   * Equivalent AGC STATUS0 register to force.
   */
  uint32_t forced_agc_status0;
} sl_rail_ble_cs_config_t;

/**
 * The maximum number of CS steps allowed during a CS event.
 */
#define SL_RAIL_BLE_CS_MAX_SQTE_STEPS 512U

/**
 * @enum sl_rail_ble_cs_step_state_t
 * @brief The current CS step state.
 */
SLI_RAIL_ENUM(sl_rail_ble_cs_step_state_t) {
  /**
   * CS step state idle.
   */
  SL_RAIL_BLE_CS_STATE_IDLE = 0u,
  /**
   * CS step state initiator transmit mode 0.
   */
  SL_RAIL_BLE_CS_STATE_I_TX_MODE_0 = 1u,
  /**
   * CS step state reflector transmit mode 0.
   */
  SL_RAIL_BLE_CS_STATE_R_TX_MODE_0 = 2u,
  /**
   * CS step state initiator transmit mode 1.
   */
  SL_RAIL_BLE_CS_STATE_I_TX_MODE_1 = 3u,
  /**
   * CS step state reflector transmit mode 1.
   */
  SL_RAIL_BLE_CS_STATE_R_TX_MODE_1 = 4u,
  /**
   * CS step state reflector transmit mode 2.
   */
  SL_RAIL_BLE_CS_STATE_R_TX_MODE_2 = 6u,
  /**
   * CS step state initiator transmit mode 2.
   */
  SL_RAIL_BLE_CS_STATE_I_TX_MODE_2 = 7u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_BLE_CS_STATE_IDLE        ((sl_rail_ble_cs_step_state_t) SL_RAIL_BLE_CS_STATE_IDLE)
#define SL_RAIL_BLE_CS_STATE_I_TX_MODE_0 ((sl_rail_ble_cs_step_state_t) SL_RAIL_BLE_CS_STATE_I_TX_MODE_0)
#define SL_RAIL_BLE_CS_STATE_R_TX_MODE_0 ((sl_rail_ble_cs_step_state_t) SL_RAIL_BLE_CS_STATE_R_TX_MODE_0)
#define SL_RAIL_BLE_CS_STATE_I_TX_MODE_1 ((sl_rail_ble_cs_step_state_t) SL_RAIL_BLE_CS_STATE_I_TX_MODE_1)
#define SL_RAIL_BLE_CS_STATE_R_TX_MODE_1 ((sl_rail_ble_cs_step_state_t) SL_RAIL_BLE_CS_STATE_R_TX_MODE_1)
#define SL_RAIL_BLE_CS_STATE_R_TX_MODE_2 ((sl_rail_ble_cs_step_state_t) SL_RAIL_BLE_CS_STATE_R_TX_MODE_2)
#define SL_RAIL_BLE_CS_STATE_I_TX_MODE_2 ((sl_rail_ble_cs_step_state_t) SL_RAIL_BLE_CS_STATE_I_TX_MODE_2)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * First step state for CS mode 0.
 */
#define SL_RAIL_BLE_CS_STEP_MODE_0             SL_RAIL_BLE_CS_STATE_I_TX_MODE_0

/**
 * First step state for CS mode 1.
 */
#define SL_RAIL_BLE_CS_STEP_MODE_1             SL_RAIL_BLE_CS_STATE_I_TX_MODE_1

/**
 * First step state for CS mode 2.
 */
#define SL_RAIL_BLE_CS_STEP_MODE_2             SL_RAIL_BLE_CS_STATE_I_TX_MODE_2

/**
 * @enum sl_rail_ble_cs_step_mode_t
 * @brief The CS step mode.
 */
SLI_RAIL_ENUM(sl_rail_ble_cs_step_mode_t) {
  /**
   * CS step mode 0.
   */
  SL_RAIL_BLE_CS_MODE_0 = 0u,
  /**
   * CS step mode 1.
   */
  SL_RAIL_BLE_CS_MODE_1 = 1u,
  /**
   * CS step mode 2.
   */
  SL_RAIL_BLE_CS_MODE_2 = 2u,
  /**
   * CS step mode 3.
   */
  SL_RAIL_BLE_CS_MODE_3 = 3u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_BLE_CS_MODE_0 ((sl_rail_ble_cs_step_mode_t) SL_RAIL_BLE_CS_MODE_0)
#define SL_RAIL_BLE_CS_MODE_1 ((sl_rail_ble_cs_step_mode_t) SL_RAIL_BLE_CS_MODE_1)
#define SL_RAIL_BLE_CS_MODE_2 ((sl_rail_ble_cs_step_mode_t) SL_RAIL_BLE_CS_MODE_2)
#define SL_RAIL_BLE_CS_MODE_3 ((sl_rail_ble_cs_step_mode_t) SL_RAIL_BLE_CS_MODE_3)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * The maximum number of antennas supported.
 */
#define SL_RAIL_BLE_CS_MAX_ANTENNAS 4U

/**
 * @enum sl_rail_ble_cs_antenna_id_t
 * @brief The CS antenna ID. Valid values according to the CS spec are within
 *   the range 1 and 4 inclusive.
 */
SLI_RAIL_ENUM(sl_rail_ble_cs_antenna_id_t) {
  /**
   * Antenna ID of the first supported antenna.
   */
  SL_RAIL_BLE_CS_ANTENNA_ID_1 = 1u,
  /**
   * Antenna ID of the second supported antenna.
   */
  SL_RAIL_BLE_CS_ANTENNA_ID_2 = 2u,
  /**
   * Antenna ID of the third supported antenna.
   */
  SL_RAIL_BLE_CS_ANTENNA_ID_3 = 3u,
  /**
   * Antenna ID of the fourth supported antenna.
   */
  SL_RAIL_BLE_CS_ANTENNA_ID_4 = 4u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using RAIL_ENUM
#define SL_RAIL_BLE_CS_ANTENNA_ID_1 ((sl_rail_ble_cs_antenna_id_t) SL_RAIL_BLE_CS_ANTENNA_ID_1)
#define SL_RAIL_BLE_CS_ANTENNA_ID_2 ((sl_rail_ble_cs_antenna_id_t) SL_RAIL_BLE_CS_ANTENNA_ID_2)
#define SL_RAIL_BLE_CS_ANTENNA_ID_3 ((sl_rail_ble_cs_antenna_id_t) SL_RAIL_BLE_CS_ANTENNA_ID_3)
#define SL_RAIL_BLE_CS_ANTENNA_ID_4 ((sl_rail_ble_cs_antenna_id_t) SL_RAIL_BLE_CS_ANTENNA_ID_4)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * The value returned by RAIL for an invalid CS antenna count.
 */
#define SL_RAIL_BLE_CS_INVALID_ANTENNA_COUNT 0u

/**
 * @enum sl_rail_ble_cs_rtt_packet_quality_t
 * @brief The CS antenna ID. Valid values according to the CS spec are within
 *   the range 1 and 4 inclusive.
 */
SLI_RAIL_ENUM(sl_rail_ble_cs_rtt_packet_quality_t) {
  /**
   * Access address check succeeded.
   */
  SL_RAIL_BLE_CS_RTT_AA_SUCCESS = 0U,
  /**
   * Access address had one or more bit errors.
   */
  SL_RAIL_BLE_CS_RTT_AA_BIT_ERRORS = 1U,
  /**
   * Access address not found.
   */
  SL_RAIL_BLE_CS_RTT_AA_NOT_FOUND = 2U,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_BLE_CS_RTT_AA_SUCCESS    ((sl_rail_ble_cs_rtt_packet_quality_t) SL_RAIL_BLE_CS_RTT_AA_SUCCESS)
#define SL_RAIL_BLE_CS_RTT_AA_BIT_ERRORS ((sl_rail_ble_cs_rtt_packet_quality_t) SL_RAIL_BLE_CS_RTT_AA_BIT_ERRORS)
#define SL_RAIL_BLE_CS_RTT_AA_NOT_FOUND  ((sl_rail_ble_cs_rtt_packet_quality_t) SL_RAIL_BLE_CS_RTT_AA_NOT_FOUND)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @struct sl_rail_ble_cs_mode_0_results_t
 * @brief Contains CS mode 0 step measurement results.
 */
typedef struct sl_rail_ble_cs_mode_0_results {
  /**
   * Mode of CS step.
   */
  uint8_t mode;
  /**
   * Antenna ID.
   */
  sl_rail_ble_cs_antenna_id_t antenna;
  /**
   * RSSI during step in integer dBm.
   */
  int8_t rssi_dbm;
  /**
   * Packet quality.
   */
  uint8_t packet_quality;
  /**
   * Reserved.
   */
  uint16_t reserved;
  /**
   * Fractional Frequency Offset (FFO) in units of parts per 100 million.
   */
  int16_t cs_ffo_pp100m;
  /**
   * The gain setting.
   */
  uint32_t step_gain_setting;
  /**
   * Reserved.
   */
  uint32_t reserved_1[4];
} sl_rail_ble_cs_mode_0_results_t;

/**
 *  A sentinel value to indicate an invalid rtt time value in
 *  \ref sl_rail_ble_cs_mode_1_results_t::rtt_half_ns
 */
#define SL_RAIL_BLE_CS_INVALID_RTT_VALUE ((int16_t)0x8000)

/**
 * @struct sl_rail_ble_cs_mode_1_results_t
 * @brief Contains CS mode 1 step measurement results.
 */
typedef struct sl_rail_ble_cs_mode_1_results {
  /**
   * Mode of CS step.
   */
  uint8_t mode;
  /**
   * Antenna ID.
   */
  sl_rail_ble_cs_antenna_id_t antenna;
  /**
   * RSSI during step in integer dBm.
   */
  int8_t rssi_dbm;
  /**
   * Packet quality.
   */
  uint8_t packet_quality;
  /**
   * For the initiator, this is the time (in 0.5 ns units) between time of
   * departure and time of arrival excluding known offsets such as interlude
   * period and packet length.
   * For the reflector, this is the time (in 0.5 ns units) between time of
   * arrival and time of departure excluding known offsets such as interlude
   * period and packet length.
   */
  int16_t rtt_half_ns;
  /**
   * Flag used to indicate whether we have missed frequency calibration.
   */
  uint8_t missed_f_cal;
  /**
   * Reserved.
   */
  uint8_t reserved;
  /**
   * Reserved.
   */
  uint32_t reserved_1[5];
} sl_rail_ble_cs_mode_1_results_t;

/**
 * @enum sl_rail_ble_cs_tone_quality_t
 * @brief CS tone quality.
 */
SLI_RAIL_ENUM(sl_rail_ble_cs_tone_quality_t) {
  /**
   * Good quality CS mode 2 tone.
   */
  SL_RAIL_BLE_CS_TONE_QUALITY_GOOD = 0U,
  /**
   * Medium quality CS mode 2 tone.
   */
  SL_RAIL_BLE_CS_TONE_QUALITY_MEDIUM = 1U,
  /**
   * Low quality CS mode 2 tone.
   */
  SL_RAIL_BLE_CS_TONE_QUALITY_LOW = 2U,
  /**
   * CS mode 2 tone quality indication unavailable.
   */
  SL_RAIL_BLE_CS_TONE_QUALITY_UNAVAILABLE = 3U,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_BLE_CS_TONE_QUALITY_GOOD        ((sl_rail_ble_cs_tone_quality_t) SL_RAIL_BLE_CS_TONE_QUALITY_GOOD)
#define SL_RAIL_BLE_CS_TONE_QUALITY_MEDIUM      ((sl_rail_ble_cs_tone_quality_t) SL_RAIL_BLE_CS_TONE_QUALITY_MEDIUM)
#define SL_RAIL_BLE_CS_TONE_QUALITY_LOW         ((sl_rail_ble_cs_tone_quality_t) SL_RAIL_BLE_CS_TONE_QUALITY_LOW)
#define SL_RAIL_BLE_CS_TONE_QUALITY_UNAVAILABLE ((sl_rail_ble_cs_tone_quality_t) SL_RAIL_BLE_CS_TONE_QUALITY_UNAVAILABLE)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * The minimum number of antenna slots supported during a CS event.
 */
#define SL_RAIL_BLE_CS_MIN_ANTENNA_SLOTS 2U

/**
 * The maximum number of antenna slots supported during a CS event.
 */
#define SL_RAIL_BLE_CS_MAX_ANTENNA_SLOTS 5U

/**
 * @struct sl_rail_ble_cs_mode_2_results_t
 * @brief Contains CS mode 2 step measurement results.
 */
typedef struct sl_rail_ble_cs_mode_2_results {
  /**
   * Mode of CS step.
   */
  uint8_t mode;
  /**
   * Flag used to indicate whether we have missed frequency calibration.
   */
  uint8_t missed_f_cal;
  /**
   * PCT i value.
   */
  int16_t pct_i[SL_RAIL_BLE_CS_MAX_ANTENNA_SLOTS];
  /**
   * PCT q value.
   */
  int16_t pct_q[SL_RAIL_BLE_CS_MAX_ANTENNA_SLOTS];
  /**
   * Tone quality indicator.
   */
  sl_rail_ble_cs_tone_quality_t tqi[SL_RAIL_BLE_CS_MAX_ANTENNA_SLOTS];
  /**
   * Reserved.
   */
  uint8_t reserved[3];
} sl_rail_ble_cs_mode_2_results_t;

/**
 * @struct sl_rail_ble_cs_step_results_t
 * @brief Generic CS step mode result structure. Based on the value of the
 *   mode field, this structure can be type cast to the appropriate mode
 *   specific structure \ref sl_rail_ble_cs_mode_0_results_t,
 *   \ref sl_rail_ble_cs_mode_1_results_t, or \ref sl_rail_ble_cs_mode_2_results_t.
 */
typedef struct sl_rail_ble_cs_step_results {
  /**
   * Mode of CS step.
   */
  uint8_t mode;
  /**
   * Reserved.
   */
  uint8_t reserved;
  /**
   * Reserved.
   */
  uint16_t reserved_1;
  /**
   * Reserved.
   */
  uint32_t reserved_2[6];
} sl_rail_ble_cs_step_results_t;

/**
 * @struct sl_rail_ble_cs_mode_0_debug_results_t
 * @brief Contains CS mode 0 step measurement debug results.
 */
typedef struct sl_rail_ble_cs_mode_0_debug_results {
  /**
   * AGC gain value of the Mode 0 step with the highest recorded
   * RSSI up to and including the current Mode 0 step.
   */
  uint32_t agc_status0;
  /**
   * For devices configured as an initiator, the measured frequency offset
   * in Hz between the two devices during a CS mode 0 step. For devices
   * configured as a reflector, this value will always be 0.
   */
  int32_t freq_offset_hz;
  /**
   * Estimated coarse frequency offset in internal units.
   */
  int32_t hw_freq_offset_est;
  /**
   * Starting index IQ sample index of unmodulated carrier.
   */
  uint16_t uc_start_index;
  /**
   * End index IQ sample index of unmodulated carrier.
   */
  uint16_t uc_end_index;
  /**
   * FFO of the Mode 0 step with the highest recorded RSSI
   * up to and including the current Mode 0 step.
   */
  int16_t cs_ffo_pp100m;
  /**
   * Highest recorded RSSI up to and including the current mode 0 step, in dBm.
   */
  int8_t highest_rssi_dbm;
  /**
   * Tx time stamp.
   */
  uint8_t tx_time_stamp_pre;
  /**
   * Tx time stamp.
   */
  uint32_t tx_time_stamp;
  /**
   * Reserved.
   */
  uint32_t reserved[4];
} sl_rail_ble_cs_mode_0_debug_results_t;

/**
 * @struct sl_rail_ble_cs_mode_1_debug_results_t
 * @brief Contains CS mode 1 step measurement debug results.
 */
typedef struct sl_rail_ble_cs_mode_1_debug_results {
  /**
   * Coarse time of flight in units of HFXO clock cycles.
   */
  uint16_t tox_clks;
  /**
   * Fractional component of time of flight in units of half nanoseconds.
   */
  int16_t frac_rtt_half_ns;
  /**
   * Coarse component of time of flight in units of half nanoseconds.
   */
  uint32_t coarse_rtt_half_ns;
  /**
   * Group delay compensation in units of half nanoseconds.
   */
  int32_t gd_comp_rtt_half_ns;
  /**
   * Time of flight without T_SY_CENTER_DELTA compensation in units of half nanoseconds.
   */
  int32_t tox_with_offsets_rtt_half_ns;
  /**
   * Internal CS status register.
   */
  uint32_t cs_status_3;
  /**
   * Internal CS status register.
   */
  uint32_t cs_status_4;
  /**
   * Internal CS status register.
   */
  uint32_t cs_status_5;
  /**
   * Tx time stamp.
   */
  uint32_t tx_time_stamp;
  /**
   * Tx time stamp.
   */
  uint8_t tx_time_stamp_pre;
  /**
   * Reserved.
   */
  uint8_t reserved;
  /**
   * Reserved.
   */
  uint16_t reserved_1;
  /**
   * Reserved.
   */
  uint32_t reserved_2;
} sl_rail_ble_cs_mode_1_debug_results_t;

/**
 * @struct sl_rail_ble_cs_mode_2_debug_results_t
 * @brief Contains CS mode 2 step measurement debug results.
 */
typedef struct sl_rail_ble_cs_mode_2_debug_results {
  /**
   * Hardware PCT I value.
   */
  int16_t hardware_pct_i;
  /**
   * Hardware PCT Q value.
   */
  int16_t hardware_pct_q;
  /**
   * DCCOMP i value.
   */
  int16_t dc_comp_i;
  /**
   * DCCOMP q value.
   */
  int16_t dc_comp_q;
  /**
   * GDCOMP i value.
   */
  int16_t gd_comp_i[SL_RAIL_BLE_CS_MAX_ANTENNAS];
  /**
   * GDCOMP q value.
   */
  int16_t gd_comp_q[SL_RAIL_BLE_CS_MAX_ANTENNAS];
  /**
   * Raw tone quality value.
   */
  uint16_t tqi_raw;
  /**
   * Raw tone quality tone extension value.
   */
  uint16_t tqi_tone_ext_raw;
  /**
   * Pointer to the starting index of each antenna slot for
   * reading IQ samples.
   */
  uint16_t *p_uc_start_index;
  /**
   * Pointer to the end index of each antenna slot for
   * reading IQ samples.
   */
  uint16_t *p_uc_end_index;
  /**
   * Frequency calibration value in internal units.
   */
  uint16_t f_cal;
  /**
   * Reserved.
   */
  uint16_t reserved;
} sl_rail_ble_cs_mode_2_debug_results_t;

/**
 * @struct sl_rail_ble_cs_step_debug_results_t
 * @brief Generic CS step mode debug result structure. Based on the value of
 *   the mode field, this structure can be type cast to the appropriate mode
 *   specific structure \ref sl_rail_ble_cs_mode_0_debug_results_t,
 *   \ref sl_rail_ble_cs_mode_1_debug_results_t, or \ref sl_rail_ble_cs_mode_2_debug_results_t.
 */
typedef struct sl_rail_ble_cs_step_debug_results {
  /**
   * Reserved.
   */
  uint32_t reserved[10];
} sl_rail_ble_cs_step_debug_results_t;

/**
 * @struct sl_rail_ble_cs_step_config_t
 * @brief Contains arguments for \ref sl_rail_ble_set_next_cs_step().
 */
typedef struct sl_rail_ble_cs_step_config {
  /**
   * Sets the CS step state.
   */
  sl_rail_ble_cs_step_state_t step_state;
  /**
   * Indicates whether this is final step in CS event.
   */
  bool last_step;
  /**
   * Transmit tone during tone extension slot in mode 2 packet.
   * This field is ignored during RX and for all non mode 2 packets.
   */
  bool transmit_tone_extension;
  /**
   * Length of packet payload in bytes. Should not include trailer, guard,
   * or UC bits. Only used for mode 1 steps, ignored otherwise.
   */
  uint8_t packet_bytes;
  /**
   * Sets the CS step logical channel.
   */
  uint16_t channel;
  /**
   * RTT marker bit positions. Ignored for mode 0 and 2 steps.
   */
  uint8_t rtt_marker_bit_position[2];
  /**
   * The initiator (first) access address during step.
   */
  uint32_t init_access_address;
  /**
   * The reflector (second) access address during step.
   */
  uint32_t refl_access_address;
  /**
   * A pointer to TX data to be transmitted. Ignored for mode 0 and 2 steps.
   */
  uint8_t *p_tx_data;
  /**
   * A pointer to an array of CS step results. These results will be
   * populated after the completion of the CS step. This array can be cast to
   * \ref sl_rail_ble_cs_mode_0_results_t, \ref sl_rail_ble_cs_mode_1_results_t, or
   * \ref sl_rail_ble_cs_mode_2_results_t as appropriate to read mode specific
   * results.
   */
  sl_rail_ble_cs_step_results_t *p_results;
  /**
   * A pointer to an array of CS step debug results. These results will be
   * populated after the completion of the CS step. This array can be cast to
   * \ref sl_rail_ble_cs_mode_0_debug_results_t, \ref
   * sl_rail_ble_cs_mode_1_debug_results_t, or \ref sl_rail_ble_cs_mode_2_debug_results_t
   * as appropriate to read mode specific debug results.
   *
   * Setting this pointer to NULL means no debug data will be collected.
   */
  sl_rail_ble_cs_step_debug_results_t *p_debug_results;
  /**
   * A pointer to the start of captured IQ data for this step. This pointer
   * will be populated after the completion of the CS step.
   */
  uint32_t **pp_iq_buffer;
  /**
   * A pointer to captured IQ data size in 32 bit words. This pointer will be
   * populated after the completion of the CS step.
   */
  uint16_t *p_iq_buffer_words;
  /**
   * A pointer to a boolean to indicate whether to preserve IQ data for this
   * step. If this is the final step of the event, IQ data will automatically
   * be preserved regardless of how this boolean is set. For other steps, if
   * this boolean is set true, and there are at least \ref
   * SL_RAIL_BLE_CS_1_MBPS_MINIMUM_IQ_BUFFER_WORDS unused 32 bit words still
   * available in the event IQ buffer, this step's IQ data will be preserved
   * and not be overwritten by IQ data from a subsequent step. Otherwise, this
   * step's IQ data will not be preserved and may be overwritten. This boolean
   * will be updated after completion of the CS step to indicate whether the
   * IQ data from that step was actually preserved.
   */
  bool *p_save_iq_data;
  /**
   * Array containing antenna settings for this step. This field has two uses
   * the antenna settings for this step.
   *
   * On mode 0 and mode 1 steps, only the first element will be used to
   * indicate the antenna to be utilized during a mode 0 and
   * mode 1 step.
   *
   * On mode 2 steps, as many elements as
   * \ref SL_RAIL_BLE_CS_MAX_ANTENNA_SLOTS - 1 that were configured for the
   * CS event will be applied.
   *
   * @note \ref sl_rail_ble_config_cs_antenna must be called prior to setting
   *   this field in order to set the antenna count as well as configure
   *   each antenna. Each element must be a valid antenna between 1 and
   *   the set antenna count.
   */
  sl_rail_ble_cs_antenna_id_t antenna_select_buffer[SL_RAIL_BLE_CS_MAX_ANTENNAS];
} sl_rail_ble_cs_step_config_t;

/**
 * @struct sl_rail_ble_cs_antenna_config_t
 * @brief Contains arguments for \ref sl_rail_ble_config_cs_antenna() function.
 */
typedef struct sl_rail_ble_cs_antenna_config {
  /**
   * Total number of antenna elements.
   */
  uint8_t antenna_count;
  /**
   * A pointer to antenna offsets in cm units.
   */
  const int16_t *p_antenna_offset_cm;
} sl_rail_ble_cs_antenna_config_t;

/**
 * @struct sl_rail_ble_cs_gd_comp_tables_t
 * @brief Contains pointers to CS group delay compensation tables.
 */
typedef struct sl_rail_ble_cs_gd_comp_tables {
  /**
   * A pointer to PBR phase LSB group delay compensation table.
   */
  const int16_t *p_pbr_phase_lsb;
  /**
   * A pointer to RTT slope group delay compensation table.
   */
  const int16_t *p_rtt_slope;
  /**
   * A pointer to RTT offset group delay compensation table.
   */
  const int16_t *p_rtt_offset;
  /**
   * Common length for each table in units of int16_t.
   */
  uint8_t table_length_halfwords;
} sl_rail_ble_cs_gd_comp_tables_t;

/**
 * CS group delay compensation tables for 0 dbm PA.
 */
extern const sl_rail_ble_cs_gd_comp_tables_t sl_rail_ble_cs_gd_comp_0_dbm_tables;
/**
 * CS group delay compensation tables for 10 dbm PA.
 */
extern const sl_rail_ble_cs_gd_comp_tables_t sl_rail_ble_cs_gd_comp_10_dbm_tables;
/**
 * CS group delay compensation tables for 20 dbm PA.
 */
extern const sl_rail_ble_cs_gd_comp_tables_t sl_rail_ble_cs_gd_comp_20_dbm_tables;

/**
 * Configure Channel Sounding (CS) functionality.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_cs_config A pointer to configuration options for CS.
 * @return Status code indicating success of the function call.
 *
 * @warning This API is not safe to use in a multiprotocol app.
 *
 * @warning As this function relies on GPIO access when more than one antenna
 *   is configured by \ref sl_rail_ble_config_cs_antenna and RAIL is meant to
 *   run in TrustZone non-secure world, it is not supported if GPIO is
 *   configured as secure peripheral and it will return \ref
 *   SL_RAIL_STATUS_INVALID_CALL.
 */
sl_rail_status_t sl_rail_ble_config_cs(sl_rail_handle_t rail_handle,
                                       const sl_rail_ble_cs_config_t *p_cs_config);

/**
 * Enable Channel Sounding (CS) functionality.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] enable Enable or disable CS functionality.
 * @return Status code indicating success of the function call.
 *
 * @warning This API is not safe to use in a multiprotocol app.
 *
 * @note While CS is enabled, the PA ramp time is cached and overridden to 5us.
 *   Upon disabling CS, the original PA ramp time is restored. While CS is
 *   enabled, calls to \ref sl_rail_config_tx_power() should be avoided as they
 *   may corrupt the ramp time and result in incorrect CS event timing.
 */
sl_rail_status_t sl_rail_ble_enable_cs(sl_rail_handle_t rail_handle,
                                       bool enable);

/**
 * Set up the next CS step.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in,out] p_cs_step_config A pointer to configuration options for next CS step.
 * @param[in] pend If true, apply configuration at next appropriate radio
 *   transition (i.e., at Rx-to-Tx for an initiator, or Tx-to-Rx for a reflector).
 *   Otherwise, apply configuration immediately.
 * @return Status code indicating success of the function call.
 *
 * @note When the next CS step is to be pended, the specified step in
 *   p_cs_step_config must be the initial step state for a particular mode (e.g.
 *   \ref SL_RAIL_BLE_CS_STEP_MODE_0, \ref SL_RAIL_BLE_CS_STEP_MODE_1, or \ref
 *   SL_RAIL_BLE_CS_STEP_MODE_2). Otherwise this API will return \ref
 *   SL_RAIL_STATUS_INVALID_PARAMETER.
 *
 * @warning This API is not safe to use in a multiprotocol app.
 */
sl_rail_status_t sl_rail_ble_set_next_cs_step(sl_rail_handle_t rail_handle,
                                              const sl_rail_ble_cs_step_config_t *p_cs_step_config,
                                              bool pend);

/**
 * Configure antennas for CS event.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_antenna_config A pointer to the antenna config.
 *   The \ref sl_rail_ble_cs_antenna_config_t::antenna_count of this
 *   parameter must be a value greater than 0
 *   and less than or equal to \ref SL_RAIL_BLE_CS_MAX_ANTENNAS.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_ble_config_cs_antenna(sl_rail_handle_t rail_handle,
                                               const sl_rail_ble_cs_antenna_config_t *p_antenna_config);

/**
 * Returns the number of antennas configured for a CS event.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The number of antennas configured for a CS event.
 *
 * @note If \ref sl_rail_ble_config_cs_antenna has not been called, this
 *   API will return \ref SL_RAIL_BLE_CS_INVALID_ANTENNA_COUNT as
 *   no antennas have been configured for the CS event. The CS event
 *   will still run with an antenna count of 1 and 0 cm antenna offset.
 */
uint8_t sl_rail_ble_get_cs_antenna_count(sl_rail_handle_t rail_handle);

/**
 * @enum sl_rail_ble_cs_pa_index_t
 * @brief CS PA index.
 */
SLI_RAIL_ENUM(sl_rail_ble_cs_pa_index_t) {
  /** Higher-power PA. */
  SL_RAIL_BLE_CS_PA_INDEX_HP = 0U,
  /** Lower-power PA. */
  SL_RAIL_BLE_CS_PA_INDEX_LP = 1U,
  /** Count of valid PA indices. Must be last. */
  SL_RAIL_BLE_CS_PA_INDEX_COUNT = 2U
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_BLE_CS_PA_INDEX_HP    ((sl_rail_ble_cs_pa_index_t) SL_RAIL_BLE_CS_PA_INDEX_HP)
#define SL_RAIL_BLE_CS_PA_INDEX_LP    ((sl_rail_ble_cs_pa_index_t) SL_RAIL_BLE_CS_PA_INDEX_LP)
#define SL_RAIL_BLE_CS_PA_INDEX_COUNT ((sl_rail_ble_cs_pa_index_t) SL_RAIL_BLE_CS_PA_INDEX_COUNT)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * Loads the CS RTT and PBR group delay compensation tables for a
 * particular PA mode.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_tables A pointer to group delay compensation lookup tables.
 * @param[in] pa_index The PA mode for which to load compensation tables.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_ble_load_cs_comp_tables(sl_rail_handle_t rail_handle,
                                                 const sl_rail_ble_cs_gd_comp_tables_t *p_tables,
                                                 sl_rail_ble_cs_pa_index_t pa_index);

/**
 * Callback used to load CS group delay compensation tables for all PA modes
 * supported by device during \ref sl_rail_ble_enable_cs() when enable is true.
 * This function is optional to implement.
 *
 * @return Status code indicating success of the function call.
 *
 * @note If this callback function is not implemented, unneeded tables may not
 *   be dead stripped, resulting in larger overall code size. The API \ref
 *   sl_rail_ble_load_cs_comp_tables() should be used within this callback to load the
 *   appropriate tables for each supported PA mode.
 */
sl_rail_status_t sl_railcb_ble_cs_gd_comp_table_load(void);

/** @} */  // end of group CS
#endif//DOXYGEN_UNDOCUMENTED

///
/// @addtogroup BLETX2TX BLE TX Channel Hopping
/// @{
/// @code{.c}
/// // Configuration to send one additional packet
/// static sl_rail_ble_tx_channel_hopping_config_entry_t entry[1];
/// static uint32_t buffer[BUFFER_WORDS];
/// static sl_rail_ble_tx_repeat_config_t tx_repeat_config = {
///   .iterations = 1,
///   .repeat_options = SL_RAIL_TX_REPEAT_OPTION_HOP,
///   .delay_or_hop.channel_hopping = {
///     .p_buffer = buffer,
///     .buffer_words = BUFFER_WORDS,
///     .number_of_channels = 1,
///     .entries = &entry[0],
///   },
/// };
///
/// // Send a normal packet on the current channel, then a packet on a new channel
/// void ble_send_then_advertise(uint8_t *p_first_packet,
///                              uint8_t first_packet_bytes,
///                              uint8_t *p_second_packet,
///                              uint8_t second_packet_bytes)
/// {
///   // Load both packets into the FIFO
///   sl_rail_write_tx_fifo(rail_handle, p_first_packet, first_packet_bytes, true);
///   sl_rail_write_tx_fifo(rail_handle, p_second_packet, second_packet_bytes, false);
///
///   // Configure a 300 us turnaround between transmits
///   entry[0].delay_mode = SL_RAIL_CHANNEL_HOPPING_DELAY_MODE_STATIC;
///   entry[0].delay = 300; // microseconds
///
///   // Use default advertising parameters
///   entry[0].disable_whitening = false;
///   entry[0].crc_init = 0x00555555;
///   entry[0].access_address = 0x8E89BED6;
///
///   // Transmit the repeated packet on the first advertising channel
///   entry[0].phy = sl_rail_ble_1_mbps;
///   entry[0].rail_channel = 0;
///   entry[0].logical_channel = 37;
///
///  // Configure repeated transmit in RAIL, then transmit, sending both packets
///  sl_rail_ble_set_next_tx_repeat(rail_handle, &tx_repeat_config);
///  sl_rail_start_tx(rail_handle, current_channel, SL_RAIL_TX_OPTIONS_DEFAULT, NULL);
/// }
/// @endcode
///

/**
 * @struct sl_rail_ble_tx_channel_hopping_config_entry_t
 * @brief Structure that represents one of the channels that is part of a
 *   \ref sl_rail_ble_tx_channel_hopping_config_t sequence of channels used in
 *   channel hopping.
 */
typedef struct sl_rail_ble_tx_channel_hopping_config_entry {
  /**
   * Idle time in microseconds to wait before hopping into the
   * channel indicated by this entry.
   */
  uint32_t delay_us;
  /**
   * The BLE PHY to use for this hop's transmit.
   */
  sl_rail_ble_phy_t phy;
  /**
   * The logical channel to use for this hop's transmit. The whitener will
   * be reinitialized if used.
   */
  uint8_t logical_channel;
  /**
   * The channel number to be used for this hop's transmit. If this is an
   * invalid channel for the chosen PHY, the call to \ref sl_rail_set_next_tx_repeat()
   * will fail.
   */
  uint8_t rail_channel;
  /**
   * This can turn off the whitening engine and is useful for sending BLE test
   * mode packets that don't have this turned on.
   */
  bool disable_whitening;
  /**
   * The value to use for CRC initialization.
   */
  uint32_t crc_init;
  /**
   * The access address to use for the connection.
   */
  uint32_t access_address;
} sl_rail_ble_tx_channel_hopping_config_entry_t;

/**
 * @struct sl_rail_ble_tx_channel_hopping_config_t
 * @brief Wrapper struct that will contain the sequence of
 *   \ref sl_rail_ble_tx_channel_hopping_config_entry_t that represents the channel
 *   sequence to use during TX Channel Hopping.
 */
typedef struct sl_rail_ble_tx_channel_hopping_config {
  /**
   * Pointer to contiguous global read-write memory that will be used
   * by RAIL to store channel hopping information throughout its operation.
   * It need not be initialized and applications should never write
   * data anywhere in this buffer.
   */
  uint32_t *p_buffer;
  /**
   * This parameter must be set to the length of the buffer array. This way,
   * during configuration, the software can confirm it's writing within the
   * range of the buffer. The configuration API will return an error
   * if buffer_words is insufficient.
   */
  uint16_t buffer_words;
  /**
   * The number of channels that is in the channel hopping sequence.
   */
  uint8_t number_of_channels;
  /**
   * Pad bytes reserved for future use and currently ignored.
   */
  uint8_t reserved;
  /**
   * A pointer to the first element of an array of \ref
   * sl_rail_ble_tx_channel_hopping_config_entry_t that represents the channels
   * used during channel hopping. The number of entries in this array must be
   * number_of_channels.
   */
  sl_rail_ble_tx_channel_hopping_config_entry_t *p_entries;
} sl_rail_ble_tx_channel_hopping_config_t;

/**
 * @struct sl_rail_ble_tx_repeat_config_t
 * @brief A configuration structure for repeated transmits
 */
typedef struct sl_rail_ble_tx_repeat_config {
  /**
   * The number of repeated transmits to run. A total of (iterations + 1)
   * transmits will go on-air in the absence of errors.
   */
  uint16_t iterations;
  /**
   * Repeat option(s) to apply.
   */
  sl_rail_tx_repeat_options_t repeat_options;
  /**
   * Per-repeat delay or hopping configuration, depending on repeat_options.
   */
  union {
    /**
     * When \ref SL_RAIL_TX_REPEAT_OPTION_HOP is not set, this specifies
     * the delay time between each repeated transmit. Specify \ref
     * SL_RAIL_TRANSITION_TIME_KEEP to use the current \ref
     * sl_rail_state_timing_t::tx_to_tx transition time setting.
     */
    sl_rail_transition_time_t delay_us;
    /**
     * When \ref SL_RAIL_TX_REPEAT_OPTION_HOP is set, this specifies
     * the channel hopping configuration to use when hopping between
     * repeated transmits. Per-hop delays are configured within each
     * \ref sl_rail_ble_tx_channel_hopping_config_entry_t::delay_us rather than
     * this union's delay_us field.
     */
    sl_rail_ble_tx_channel_hopping_config_t channel_hopping;
  } delay_or_hop;
} sl_rail_ble_tx_repeat_config_t;

/**
 * Set up automatic repeated transmits after the next transmit.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_repeat_config A non-NULL pointer to the configuration structure for repeated transmits.
 * @return Status code indicating a success of the function call.
 *
 * Repeated transmits will occur after an application-initiated transmit caused
 * by calling one of the \ref Packet_TX APIs. The repetition will only occur
 * after the first application-initiated transmit after this function is
 * called. Future repeated transmits must be requested by calling this function
 * again.
 *
 * Each repeated transmit that occurs will have full \ref PTI information and
 * will receive events such as \ref SL_RAIL_EVENT_TX_PACKET_SENT as normal.
 *
 * If a TX error occurs during the repetition, the process will abort and the
 * TX error transition from \ref sl_rail_set_tx_transitions() will be used. If the
 * repetition completes successfully, the TX success transition from
 * \ref sl_rail_set_tx_transitions() will be used.
 *
 * Any call to \ref sl_rail_idle(), \ref sl_rail_stop_tx(), or \ref
 * sl_rail_set_tx_transitions() will clear the pending
 * repeated transmits. The state will also be cleared by another call to this
 * function. To clear the repeated transmits before they've started without
 * stopping other radio actions, call this function with a \ref
 * sl_rail_ble_tx_repeat_config_t::iterations count of 0. A DMP switch will clear this
 * state only if the initial transmit triggering the repeated transmits has
 * started.
 *
 * The application is responsible for populating the transmit data to be used
 * by the repeated transmits via \ref sl_rail_set_tx_fifo() or \ref sl_rail_write_tx_fifo().
 * Data will be transmitted from the TX FIFO. If the TX FIFO does not have
 * sufficient data to transmit, a TX error and a \ref
 * SL_RAIL_EVENT_TX_UNDERFLOW will occur. To avoid an underflow, the
 * application should queue data to be transmitted as early as possible.
 *
 * This function will fail to configure the repetition if a transmit of any
 * kind is ongoing, including during the time between an initial transmit and
 * the end of a previously-configured repetition.
 *
 * @note Use the compile time symbol \ref SL_RAIL_SUPPORTS_TX_TO_TX or the runtime
 *   call \ref sl_rail_supports_tx_to_tx() to check whether the platform supports
 *   this feature.
 */
sl_rail_status_t sl_rail_ble_set_next_tx_repeat(sl_rail_handle_t rail_handle,
                                                const sl_rail_ble_tx_repeat_config_t *p_repeat_config);

/// @} // end of group BLETX2TX

/// @} // end of BLE

///
/// @addtogroup Calibration
/// @brief Bluetooth protocol-specific APIs for calibrating the radio.
/// @{
///

/**
 * Calibrate image rejection for Bluetooth Low Energy.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_image_rejection A pointer to where the result of the image
 *   rejection calibration will be stored.
 * @return Status code indicating success of the function call.
 *
 * Some chips have protocol-specific image rejection calibrations programmed
 * into their flash. This function will either get the value from flash and
 * apply it, or run the image rejection algorithm to find the value.
 */
sl_rail_status_t sl_rail_ble_calibrate_ir(sl_rail_handle_t rail_handle,
                                          uint32_t *p_image_rejection);

/// @} // End of group Calibration

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// Temporary RAIL 3.x API mappings to RAIL 2.x APIs

#include "rail_ble.h"
#define sl_rail_ble_phy_1_mbps \
  ((const sl_rail_channel_config_t *)RAIL_BLE_Phy1MbpsViterbi)
#define sl_rail_ble_phy_2_mbps \
  ((const sl_rail_channel_config_t *)RAIL_BLE_Phy2MbpsViterbi)
#define sl_rail_ble_phy_1_mbps_cs \
  ((const sl_rail_channel_config_t *)RAIL_BLE_Phy1MbpsViterbiCs)
#define sl_rail_ble_phy_2_mbps_cs \
  ((const sl_rail_channel_config_t *)RAIL_BLE_Phy2MbpsViterbiCs)
#define sl_rail_ble_phy_2_mbps_aox \
  ((const sl_rail_channel_config_t *)RAIL_BLE_Phy2MbpsAox)
#define sl_rail_ble_phy_125_kbps \
  ((const sl_rail_channel_config_t *)RAIL_BLE_Phy125kbps)
#define sl_rail_ble_phy_500_kbps \
  ((const sl_rail_channel_config_t *)RAIL_BLE_Phy500kbps)
#define sl_rail_ble_phy_simulscan \
  ((const sl_rail_channel_config_t *)RAIL_BLE_PhySimulscan)
#define sl_rail_ble_phy_quuppa \
  ((const sl_rail_channel_config_t *)RAIL_BLE_PhyQuuppa)

#endif//DOXYGEN_SHOULD_SKIP_THIS

#ifdef __cplusplus
}
#endif

#endif // SL_RAIL_BLE_H
