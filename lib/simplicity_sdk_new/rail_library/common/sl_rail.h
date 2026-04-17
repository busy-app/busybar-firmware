/***************************************************************************//**
 * @file
 * @brief The main header file for the RAIL library. It describes the external
 *   APIs available to a RAIL user
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

#ifndef SL_RAIL_H
#define SL_RAIL_H

#include <string.h> // For memcpy()

// Get the RAIL-specific structures and types
#include "sl_rail_types.h"
#include "sl_rail_assert_error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup RAIL_API RAIL API
 * @brief This is the primary API layer for the Radio Abstraction Interface
 *   Layer (RAIL).
 * @{
 */

/**
 * @defgroup Chip_Specific Chip-Specific
 * @brief Chip-Specific RAIL APIs, types, and information
 */

/**
 * @defgroup Protocol_Specific Protocol-specific
 * @brief Protocol-Specific RAIL APIs
 */

/******************************************************************************
 * General Radio Operation
 *****************************************************************************/
/**
 * @addtogroup General
 * @brief Basic APIs to set up and interact with the RAIL library
 * @{
 */

/**
 * Get the version information for the compiled RAIL library.
 *
 * @param[out] p_version A non-NULL pointer to \ref sl_rail_version_t structure to
 *   populate with version information.
 * @return Status code indicating success of the function call.
 *
 * The p_version information contains a major version number, a minor version
 * number, a rev (revision) number, and some supplemental build details.
 */
sl_rail_status_t sl_rail_get_version(sl_rail_version_t *p_version);

#ifndef DOXYGEN_UNDOCUMENTED

/**
 * A global pointer to the head of a linked list of state buffers
 * \ref sl_rail_init() can use.
 *
 * RAIL internally provides one statically-allocated RAM state buffer
 * for a single protocol and two for dynamic multiprotocol. If your
 * application needs more, they can be provided via \ref
 * sl_rail_add_state_buffer_3() or \ref sl_rail_add_state_buffer_4(), which use
 * internal buffers, or the more general \ref sl_rail_add_state_buffer().
 *
 * This symbol is WEAK in the RAIL library in case an application wants
 * to allocate and provide its own buffers. However, this use is highly
 * discouraged.
 */
extern sl_rail_state_buffer_entry_t *sl_rail_state_buffer_head;

/**
 * Get the run-time size of the radio's state buffer.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @return Size, in bytes, of the radio's internal state buffer.
 *   If the handle is invalid, 0 is returned.
 *
 * See \ref SL_RAIL_STATE_BUFFER_BYTES for a compile-time estimated size
 * definition, which may be larger than what this function returns.
 */
uint32_t sl_rail_get_state_buffer_bytes(sl_rail_handle_t radio_handle);

/**
 * Add an app-provided state buffer to the \ref sl_rail_state_buffer_head list.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @param[in] p_new_entry A pointer to a \ref sl_rail_state_buffer_entry_t to
 *   add to the liked list of state buffers headed by
 *   \ref sl_rail_state_buffer_head. Both the \ref sl_rail_state_buffer_entry_t
 *   to which this parameter points and the \ref
 *   sl_rail_state_buffer_entry_t::p_buffer to which that points must be
 *   allocated in RAM and persist indefinitely beyond this call.
 * @return Status code indicating success of the function call.
 *   An error should be returned if the entry's
 *   \ref sl_rail_state_buffer_entry_t::buffer_bytes is too small or
 *   the \ref sl_rail_state_buffer_entry_t::p_buffer pointer seems invalid.
 *
 * RAIL's internal \ref sl_rail_state_buffer_head should prove
 * sufficient for most applications, providing one (single protocol)
 * or two (dynamic multiprotocol) buffers preallocated in RAM for
 * use by \ref sl_rail_init(). This function exists for dynamic
 * multiprotocol applications that needs more than two protocols, or
 * that prefer to dynamically allocate RAIL state buffers just prior
 * to calling \ref sl_rail_init() rather than having them statically
 * allocated in RAM.
 */
sl_rail_status_t sl_rail_add_state_buffer(sl_rail_handle_t radio_handle,
                                          sl_rail_state_buffer_entry_t *p_new_entry);

#endif//DOXYGEN_UNDOCUMENTED

/**
 * Add a 3rd multiprotocol internal state buffer for use by \ref sl_rail_init().
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @return Status code indicating success of the function call.
 *   An error is returned if the 3rd state buffer was previously added
 *   or this isn't the RAIL multiprotocol library.
 */
sl_rail_status_t sl_rail_add_state_buffer_3(sl_rail_handle_t radio_handle);

/**
 * Add a 4th multiprotocol internal state buffer for use by \ref sl_rail_init().
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @return Status code indicating success of the function call.
 *   An error is returned if the 4th state buffer was previously added.
 *   or this isn't the RAIL multiprotocol library.
 */
sl_rail_status_t sl_rail_add_state_buffer_4(sl_rail_handle_t radio_handle);

/**
 * Allocate a DMA channel for RAIL to work with.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in] dma_channel The DMA channel to use when copying memory. If a value of
 *   \ref SL_RAIL_DMA_INVALID is passed, RAIL will stop using any DMA channel.
 * @return Status code indicating success of the function call.
 *
 * To use this API, the application must initialize the DMA engine
 * on the chip and allocate a DMA channel. This channel will be used
 * periodically to copy memory more efficiently. Call this function
 * before \ref sl_rail_init() to have the most benefit. If the application needs
 * to take back control of the DMA channel that RAIL is using, this API may be
 * called with a channel of \ref SL_RAIL_DMA_INVALID to tell RAIL to stop using DMA.
 *
 * @warning To allocate and use a DMA channel for RAIL to work with when
 *   TrustZone is enabled and LDMA is configured as secure peripheral, the
 *   secure application must initialize the DMA engine and call this API. The
 *   non-secure application must provide a non-NULL
 *   \ref sl_rail_tz_config_t::radio_perform_m2m_ldma_callback to
 *   \ref sl_rail_tz_init_non_secure().
 *   To take back control of the DMA channel when TrustZone is enabled and LDMA
 *   is configured as secure peripheral, the secure application must call this
 *   API with a channel of \ref SL_RAIL_DMA_INVALID. The non-secure application
 *   must provide a NULL
 *   \ref sl_rail_tz_config_t::radio_perform_m2m_ldma_callback to
 *   \ref sl_rail_tz_init_non_secure().
 */
sl_rail_status_t sl_rail_use_dma(sl_rail_handle_t rail_handle, uint8_t dma_channel);

#ifndef DOXYGEN_UNDOCUMENTED

/**
 * Load the first image \ref SL_RAIL_SEQ_IMAGE_0 into the radio sequencer during
 * RAIL initialization.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @return Status code indicating success of the function call.
 *
 * This function must only be called from within the RAIL callback context of
 * \ref sl_railcb_radio_sequencer_image_load(). Otherwise, the function returns \ref
 * SL_RAIL_STATUS_INVALID_STATE.
 */
sl_rail_status_t sl_rail_load_sequencer_image_0(sl_rail_handle_t radio_handle);

/**
 * Load the second image \ref SL_RAIL_SEQ_IMAGE_1 into the radio sequencer during
 * RAIL initialization.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @return Status code indicating success of the function call.
 *
 * This function must only be called from within the RAIL callback context of
 * \ref sl_railcb_radio_sequencer_image_load(). Otherwise, the function returns \ref
 * SL_RAIL_STATUS_INVALID_STATE. On platforms where \ref SL_RAIL_SEQ_IMAGE_COUNT < 2,
 * the function returns with \ref SL_RAIL_STATUS_INVALID_CALL.
 */
sl_rail_status_t sl_rail_load_sequencer_image_1(sl_rail_handle_t radio_handle);

/**
 * Callback used to load the radio sequencer image during RAIL initialization.
 * This function is optional to implement.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @return Status code indicating success of the function call.
 *
 * This callback is used by RAIL to load a radio sequencer image during \ref
 * sl_rail_init() via an API such as \ref sl_rail_load_sequencer_image_0(). If this
 * function is not implemented, a default image will be loaded. On some
 * platforms, (in particular EFR32xG24), not implementing this function may
 * result in a larger overall code size due to unused sequencer images not
 * being dead stripped.
 *
 * @note If this function is implemented without a call to an image loading API
 *   such as \ref sl_rail_load_sequencer_image_0(), an assert will occur during
 *   RAIL initialization. Similarly, if an image is loaded that is
 *   unsupported by the platform, an assert will occur.
 */
sl_rail_status_t sl_railcb_radio_sequencer_image_load(sl_rail_handle_t radio_handle);

/**
 * Load the OFDM and OQPSK image into the software modem (SFM) sequencer during
 * RAIL initialization.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @return Status code indicating success of the function call.
 *
 * This function must only be called from within the RAIL callback context of
 * \ref sl_railcb_load_sfm_sequencer(). Otherwise, the function returns \ref
 * SL_RAIL_STATUS_INVALID_STATE.
 */
sl_rail_status_t sl_rail_load_sfm_sun_ofdm_oqpsk(sl_rail_handle_t radio_handle);

/**
 * Load the OFDM image into the software modem (SFM) sequencer during
 * RAIL initialization.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @return Status code indicating success of the function call.
 *
 * This function must only be called from within the RAIL callback context of
 * \ref sl_railcb_load_sfm_sequencer(). Otherwise, the function returns \ref
 * SL_RAIL_STATUS_INVALID_STATE.
 */
sl_rail_status_t sl_rail_load_sfm_sun_ofdm(sl_rail_handle_t radio_handle);

/**
 * Load the empty image into the software modem (SFM) sequencer during
 * RAIL initialization.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @return Status code indicating success of the function call.
 *
 * This function must only be called from within the RAIL callback context of
 * \ref sl_railcb_load_sfm_sequencer(). Otherwise, the function returns \ref
 * SL_RAIL_STATUS_INVALID_STATE.
 */
sl_rail_status_t sl_rail_load_sfm_empty(sl_rail_handle_t radio_handle);

/**
 * Callback used to load the software modem (SFM) sequencer image during RAIL
 * initialization. This function is optional to implement.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @return Status code indicating success of the function call.
 *
 * This callback is used by RAIL to load a software modem sequencer image
 * during \ref sl_rail_init() via an API such as \ref
 * sl_rail_load_sfm_sun_ofdm_oqpsk(). If this function is not implemented, a
 * default image including OFDM and OQPSK modulations will be loaded.
 *
 * @note If this function is implemented without a call to an image loading API
 *   such as \ref sl_rail_load_sfm_sun_ofdm_oqpsk(), an assert will occur
 *   during RAIL initialization. Similarly, if an image is loaded that is
 *   unsupported by the platform, an assert will occur.
 */
sl_rail_status_t sl_railcb_load_sfm_sequencer(sl_rail_handle_t radio_handle);

#endif//DOXYGEN_UNDOCUMENTED

/**
 * Reads out device specific data that may be needed by RAIL
 * and populates appropriate data structures in the library.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @return Status code indicating success of the function call.
 *
 * @note This function must be called before calling \ref sl_rail_init()
 *   on any platforms that require this data
 *   and should not be called inside a critical section.
 *   This function does nothing on EFR32 Series 2 devices.
 */
sl_rail_status_t sl_rail_copy_device_info(sl_rail_handle_t radio_handle);

/**
 * Initialize RAIL.
 *
 * @param[in,out] p_rail_handle A non-NULL pointer to a non-zero radio-generic
 *   RAIL handle (e.g., \ref SL_RAIL_EFR32_HANDLE) that RAIL will overwrite
 *   with the real RAIL instance handle when returning a success status code.
 * @param[in] p_rail_config The configuration for setting up the protocol.
 * @param[in] init_complete_callback A callback that notifies the application when the radio is
 *   finished initializing and is ready for further configuration. This
 *   callback is useful for potential transceiver products that require a
 *   power up sequence before further configuration is available. After the
 *   callback fires, the radio is ready for additional configuration before
 *   transmit and receive operations.
 * @return Status code indicating success of the function call.
 *
 * @note Call this function only once per protocol. If called
 *   again, it will do nothing and return \ref SL_RAIL_STATUS_INVALID_STATE.
 *   \ref sl_rail_copy_device_info() should be called once before calling this
 *   function for Silicon Labs Series 3 devices.
 *
 * @note The first call to \ref sl_rail_init() implicitly enables PTI, but
 *   it won't take effect unless or until \ref sl_rail_config_pti() has been
 *   called with a mode other than the default \ref SL_RAIL_PTI_MODE_DISABLED.
 */
sl_rail_status_t sl_rail_init(sl_rail_handle_t *p_rail_handle,
                              const sl_rail_config_t *p_rail_config,
                              sl_rail_init_complete_callback_t init_complete_callback);

/**
 * Get RAIL initialization status.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if the radio has finished initializing and
 *   false otherwise.
 *
 * RAIL APIs, e.g., \ref sl_rail_get_time(), which work only if \ref sl_rail_init() has been called,
 * can use \ref sl_rail_is_initialized() to determine whether RAIL has been initialized or not.
 */
bool sl_rail_is_initialized(sl_rail_handle_t rail_handle);

/**
 * Get the \ref sl_rail_config_t pointer associated with a \ref sl_rail_handle_t.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The pointer to the \ref sl_rail_config_t associated with the RAIL instance
 *   handle, or NULL if the handle is not a valid instance provided by a \ref
 *   sl_rail_init() call.
 *
 * Applications can read but must not change any \ref sl_rail_config_t fields using
 * the returned pointer.
 */
const sl_rail_config_t *sl_rail_get_config(sl_rail_handle_t rail_handle);

/**
 * Collect entropy from the radio if available.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_buffer A non-NULL pointer to the buffer to write the collected entropy.
 * @param[in] bytes The number of bytes to fill in the input buffer.
 * @return The number of bytes of entropy collected. For
 *   radios that don't support entropy collection, the function returns 0.
 *   Values less than the requested amount may also be returned on platforms
 *   that use entropy pools to collect random data periodically.
 *
 * Attempts to fill the provided buffer with the requested number of bytes of
 * entropy. If the requested number of bytes can't be provided, as many
 * bytes as possible will be filled and returned. For radios
 * that do not support this function, 0 bytes are always returned. For
 * information about the specific mechanism for gathering entropy, see
 * documentation for the chip family.
 *
 * This function returns 0 if a radio configuration is not loaded. If a channel
 * is not set then this function sets the first channel in the loaded
 * configuration implicitly. If the implicit channel setting fails, the function
 * returns 0.
 *
 * Returns 0 if no receive FIFO or Packet Queue have been configured.
 */
uint16_t sl_rail_get_radio_entropy(sl_rail_handle_t rail_handle,
                                   uint8_t *p_buffer,
                                   uint16_t bytes);

/** @} */ // end of group General

/******************************************************************************
 * PTI
 *****************************************************************************/
/**
 * @addtogroup PTI PTI Packet Trace
 * @brief Basic APIs to set up and interact with PTI settings
 * @{
 */

/**
 * Configure PTI pin locations, serial protocols, and baud rates.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in] p_pti_config A non-NULL pointer to the PTI configuration structure
 *   to use.
 * @return Status code indicating success of the function call.
 *
 * When this function is called prior to the first \ref sl_rail_init() call,
 * the PTI configuration is recorded but activation is deferred to \ref
 * sl_rail_init().
 * When called subsequent to \ref sl_rail_init(), the radio should be off
 * (idle) and the PTI configuration is recorded and put into effect
 * immediately unless \ref sl_rail_enable_pti() had been called just prior
 * to disable PTI, in which case activation is deferred to when \ref
 * sl_rail_enable_pti() is subsequently called to enable PTI.
 *
 * Only one PTI configuration can be active on a
 * radio, regardless of the number of protocols (unless the application
 * updates the configuration upon a protocol switch -- RAIL does not
 * save the configuration in a protocol RAIL instance).
 *
 * @note On EFR32 platforms GPIO configuration must be unlocked
 *   (see GPIO->LOCK register) to configure or use PTI.
 *
 * @warning As this function relies on GPIO access and RAIL is meant to run in
 *   TrustZone non-secure world, it is not supported if GPIO is configured as
 *   secure peripheral and it will return \ref SL_RAIL_STATUS_INVALID_CALL.
 */
sl_rail_status_t sl_rail_config_pti(sl_rail_handle_t rail_handle,
                                    const sl_rail_pti_config_t *p_pti_config);

/**
 * Get the currently-active PTI configuration.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[out] p_pti_config A non-NULL pointer to the configuration structure
 *   to be filled in with the active PTI configuration on the radio.
 * @return Status code indicating success of the function call.
 *
 * Although most combinations of configurations can be set, it is safest
 * to call this method after configuration to confirm which values were
 * actually set. This function always returns the active PTI configuration
 * on the radio regardless of the active protocol.
 */
sl_rail_status_t sl_rail_get_pti_config(sl_rail_handle_t rail_handle,
                                        sl_rail_pti_config_t *p_pti_config);

/**
 * Enable Packet Trace Interface (PTI) output of packet data.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in] enable PTI is enabled if true; disabled if false.
 * @return Status code indicating success of the function call.
 *
 * Similarly to having only one PTI configuration per radio,
 * PTI can only be enabled or disabled for all protocols. It cannot
 * be individually set to enabled and disabled per protocol
 * (unless the application switches it when the protocol switches --
 * RAIL does not save this state in a protocol RAIL instance).
 *
 * PTI should be enabled or disabled only when the radio is off (idle).
 *
 * @note The first call to \ref sl_rail_init() implicitly enables PTI, but
 *   it won't take effect unless or until \ref sl_rail_config_pti() has been
 *   called with a mode other than the default \ref SL_RAIL_PTI_MODE_DISABLED.
 *
 * @warning On EFR32 platforms GPIO configuration must be unlocked
 *   (see GPIO->LOCK register) to configure or use PTI, otherwise a fault
 *   or assert might occur.
 *   If GPIO configuration locking is desired, PTI must be disabled
 *   beforehand either with this function or with \ref sl_rail_config_pti()
 *   using \ref SL_RAIL_PTI_MODE_DISABLED.
 *
 * @warning As this function relies on GPIO access and RAIL is meant to run in
 *   TrustZone non-secure world, it is not supported if GPIO is configured as
 *   secure peripheral and it will return \ref SL_RAIL_STATUS_INVALID_CALL.
 */
sl_rail_status_t sl_rail_enable_pti(sl_rail_handle_t rail_handle,
                                    bool enable);

/**
 * Set a protocol that RAIL outputs on PTI.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] protocol The enumeration representing which protocol the app is using.
 * @return Status code indicating success of the function call.
 *
 * The protocol is output via PTI for each packet.
 * Before any protocol is set, the default value is \ref
 * SL_RAIL_PTI_PROTOCOL_CUSTOM. Use one of the enumeration values so that
 * the Network Analyzer can decode the packet.
 *
 * @note This function cannot be called unless the radio is currently in the
 *   \ref SL_RAIL_RF_STATE_IDLE or \ref SL_RAIL_RF_STATE_INACTIVE states. For this
 *   reason, call this function early on before starting radio
 *   operations and not changed later.
 */
sl_rail_status_t sl_rail_set_pti_protocol(sl_rail_handle_t rail_handle,
                                          sl_rail_pti_protocol_t protocol);

/**
 * Get the protocol that RAIL outputs on PTI.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return PTI protocol in use.
 */
sl_rail_pti_protocol_t sl_rail_get_pti_protocol(sl_rail_handle_t rail_handle);

/** @} */ // end of group PTI

/******************************************************************************
 * Antenna Control
 *****************************************************************************/
/**
 * @addtogroup Antenna_Control Antenna Control
 * @brief Basic APIs to control the antenna functionality
 * @{
 */
/**
 * Configure antenna path and pin locations.
 *
 * @warning This API must be called before any TX or RX occurs. Otherwise,
 *   the antenna configurations for those functions will not take effect.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in] p_config A pointer to a configuration structure applied to the relevant Antenna
 *   Configuration registers. A NULL configuration will produce undefined behavior.
 * @return Status code indicating success of the function call.
 *
 * This function informs RAIL how to select each antenna, but not when.
 * Antenna selection for receive is controlled by the
 * \ref SL_RAIL_RX_OPTION_ANTENNA_0 and
 * \ref SL_RAIL_RX_OPTION_ANTENNA_1 options
 * (and the \ref SL_RAIL_RX_OPTION_ANTENNA_AUTO combination).
 * Antenna selection for transmit is controlled by the
 * \ref SL_RAIL_TX_OPTION_ANTENNA_0 and
 * \ref SL_RAIL_TX_OPTION_ANTENNA_1 options.
 *
 * Only one antenna configuration can be active on a
 * radio, regardless of the number of protocols (unless the application
 * updates the configuration upon a protocol switch --
 * RAIL does not save this configuration in a protocol RAIL instance).
 */
sl_rail_status_t sl_rail_config_antenna(sl_rail_handle_t rail_handle,
                                        const sl_rail_antenna_config_t *p_config);

/**
 * Get the default RF path.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_rf_path A pointer to RF path updated by the function.
 * @return Status code indicating success of the function call.
 *
 * If multiple protocols are used, this function returns
 * \ref SL_RAIL_STATUS_INVALID_STATE if it is called and the given rail_handle is
 * not active. In that case, the caller must attempt to re-call this function later,
 * for example when \ref SL_RAIL_EVENT_CONFIG_SCHEDULED trigger.
 */
sl_rail_status_t sl_rail_get_rf_path(sl_rail_handle_t rail_handle,
                                     sl_rail_antenna_sel_t *p_rf_path);

/** @} */ // end of group Antenna_Control

/******************************************************************************
 * Radio Configuration
 *****************************************************************************/
/// @addtogroup Radio_Configuration Radio Configuration
/// @brief Routines for setting up and querying radio configuration information.
///
/// These routines allow for runtime flexibility in the radio
/// configuration. Some of the parameters, however, are meant to be generated
/// from the radio calculator in Simplicity Studio. The basic code to configure
/// the radio from this calculator output looks like the example below.
/// @code{.c}
/// // Associate a specific channel configuration with a particular RAIL instance.
/// sl_rail_config_channels(rail_handle, channel_configs[0], NULL);
/// // If want RAIL 2.x behavior where the first channel is prepared by default
/// // then do:
/// sl_rail_prepare_channel(rail_handle,
///                         sl_rail_get_first_channel(rail_handle,
///                                                   channel_configs[0]));
/// // Otherwise channel preparation is deferred to the first API call that
/// // specifies a particular channel.
/// @endcode
///
/// For more information about the types of parameters that can be changed in
/// the other functions and how to use them, see their individual documentation.
///
/// @{

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * Load a static radio configuration.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] config A radio configuration array (pointer).
 * @return Status code indicating success of the function call.
 *
 * The configuration passed into this function should be auto-generated
 * and not manually created or edited. Do not call this function unless
 * instructed by Silicon Labs because it may bypass updating certain
 * RAIL state.
 */
sl_rail_status_t sl_rail_config_radio(sl_rail_handle_t rail_handle,
                                      sl_rail_radio_config_t config);
#endif//DOXYGEN_UNDOCUMENTED

/**
 * Returns the first channel in the given configuration.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_config A pointer to the channel configuration to use.
 *   If NULL, the protocol's currently-configured channel configuration is used.
 * @return Returns the first available channel in the configuration.
 */
uint16_t sl_rail_get_first_channel(sl_rail_handle_t rail_handle,
                                   const sl_rail_channel_config_t *p_config);

/**
 * Modify the currently configured fixed frame length in bytes.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] bytes The expected fixed frame length in bytes. A value of 0 is infinite.
 *   A value of \ref SL_RAIL_SET_FIXED_LENGTH_INVALID restores the frame's length back to
 *   that specified by the default frame type configuration.
 * @return The new frame length in bytes configured into the hardware
 *   for use: 0 if in infinite mode, or \ref SL_RAIL_SET_FIXED_LENGTH_INVALID if the frame
 *   length has not yet been overridden by a valid value.
 *
 * Sets the fixed-length configuration for transmit and receive.
 * Be careful when using this function in receive and transmit as this
 * function changes the default frame configuration and remains in force until
 * it is called again with an input value of \ref SL_RAIL_SET_FIXED_LENGTH_INVALID. This
 * function will override any fixed or variable length settings from a radio
 * configuration.
 */
uint16_t sl_rail_set_fixed_length(sl_rail_handle_t rail_handle, uint16_t bytes);

/**
 * Configure the channels supported by this device.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_config A pointer to the channel configuration for your device.
 *   This pointer will be cached in the library so it must
 *   exist for the runtime of the application. Typically, this should be
 *   what is stored in Flash by the configuration tool.
 * @param[in] config_changed_callback A pointer to a function called whenever a radio
 *   configuration change occurs. May be NULL if do not need a callback.
 * @return Status code indicating success of the function call.
 *
 * @note Unlike RAIL 2.x \ref RAIL_ConfigChannels(), this function only caches
 *   the configuration and does not prepare any channel in the configuration.
 *   That action is deferred to the next call to a RAIL API where channel is
 *   passed as a parameter, namely
 *   \ref sl_rail_prepare_channel(), \ref sl_rail_start_tx(),
 *   \ref sl_rail_start_scheduled_tx(), \ref sl_rail_start_cca_csma_tx(),
 *   \ref sl_rail_start_cca_lbt_tx(), \ref sl_rail_start_scheduled_cca_csma_tx(),
 *   \ref sl_rail_start_scheduled_cca_lbt_tx(), \ref sl_rail_start_rx(),
 *   \ref sl_rail_start_scheduled_rx(), \ref sl_rail_start_average_rssi(),
 *   \ref sl_rail_start_tx_stream().
 *
 * @note config can be NULL to simply register or unregister the cb callback
 *   function when using RAIL internal protocol-specific radio configuration
 *   APIs for BLE, IEEE 802.15.4, or Z-Wave, which lack callback specification.
 */
sl_rail_status_t sl_rail_config_channels(sl_rail_handle_t rail_handle,
                                         const sl_rail_channel_config_t *p_config,
                                         sl_rail_radio_config_changed_callback_t config_changed_callback);

/**
 * Get verbose listing of channel metadata for the current channel configuration.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_channel_metadata A pointer to an app-allocated array that
 *   will be populated with channel metadata.
 * @param[in,out] p_number_of_entries A pointer to the number of entries available
 *   in the p_channel_metadata array.
 *   This value will be updated to the number of channels written to the array,
 *   not to exceed the length number passed in.
 * @param[in] min_channel Minimum channel number about which to collect data.
 * @param[in] max_channel Maximum channel number about which to collect data.
 * @return Status code indicating success of the function call.
 *   \ref SL_RAIL_STATUS_INVALID_PARAMETER means that,
 *   based on the currently active radio configuration, there are more
 *   channels to write than there is space provided in the allocated
 *   p_channel_metadata. However, the channel metadata that was written is valid.
 *   \ref SL_RAIL_STATUS_INVALID_STATE indicates that the channel configuration
 *   has not been configured. \ref SL_RAIL_STATUS_NO_ERROR indicates complete
 *   success.
 */
sl_rail_status_t sl_rail_get_channel_metadata(sl_rail_handle_t rail_handle,
                                              sl_rail_channel_metadata_t *p_channel_metadata,
                                              uint16_t *p_number_of_entries,
                                              uint16_t min_channel,
                                              uint16_t max_channel);

/**
 * Check whether the channel exists in the current RAIL channel configuration.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] channel A channel number to check.
 * @return \ref SL_RAIL_STATUS_NO_ERROR if channel exists or \ref
 *   SL_RAIL_STATUS_INVALID_PARAMETER if the given channel does not exist.
 */
sl_rail_status_t sl_rail_is_valid_channel(sl_rail_handle_t rail_handle,
                                          uint16_t channel);

/**
 * Cause radio settings associated with a particular channel to be applied to
 * hardware.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] channel The channel to prepare for use.
 * @return \ref SL_RAIL_STATUS_NO_ERROR on success or
 *   \ref SL_RAIL_STATUS_INVALID_PARAMETER if the given channel does not have an
 *   associated channel configuration entry.
 *
 * This function walks the current \ref sl_rail_channel_config_t::p_entries list
 * and applies the configuration associated with the specified channel if
 * found.
 * This function manually changes channels without starting a TX or RX
 * operation.
 *
 * When successful, the radio is idled.
 * When unsuccessful, the radio state will not be altered.
 */
sl_rail_status_t sl_rail_prepare_channel(sl_rail_handle_t rail_handle,
                                         uint16_t channel);

/**
 * Return the most-recently requested RAIL channel.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_channel A pointer to the channel for which RAIL was
 *   most-recently configured.
 * @return \ref SL_RAIL_STATUS_NO_ERROR on success or
 *   \ref SL_RAIL_STATUS_INVALID_CALL if the radio is not configured for any channel
 *   or \ref SL_RAIL_STATUS_INVALID_PARAMETER if p_channel is NULL.
 *
 * This function returns the channel most recently specified in API calls that
 * pass in a channel to tune to, namely \ref sl_rail_prepare_channel(),
 * \ref sl_rail_start_tx(), \ref sl_rail_start_scheduled_tx(), \ref sl_rail_start_cca_csma_tx(),
 * \ref sl_rail_start_cca_lbt_tx(), \ref sl_rail_start_scheduled_cca_csma_tx(),
 * \ref sl_rail_start_scheduled_cca_lbt_tx(), \ref sl_rail_start_rx(), \ref sl_rail_start_scheduled_rx(),
 * \ref sl_rail_start_average_rssi(), \ref sl_rail_start_tx_stream().
 * It doesn't follow changes RAIL performs implicitly during channel hopping
 * and mode switch.
 */
sl_rail_status_t sl_rail_get_channel(sl_rail_handle_t rail_handle,
                                     uint16_t *p_channel);

/**
 * Return the RAIL channel to which the radio is currently tuned.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_channel A pointer to the currently-tuned channel.
 * @return \ref SL_RAIL_STATUS_NO_ERROR on success or
 *   \ref SL_RAIL_STATUS_INVALID_CALL if the radio is not configured for any channel
 *   or \ref SL_RAIL_STATUS_INVALID_PARAMETER if p_channel is NULL.
 *
 * This function returns the channel to which the radio is currently tuned if
 * the specified RAIL handle is active. It returns the channel to which it
 * will be tuned during the next protocol switch if the handle is inactive.
 * The channel returned may be different than what \ref sl_rail_get_channel()
 * returns when channel hopping or mode switch are involved.
 */
sl_rail_status_t sl_rail_get_tuned_channel(sl_rail_handle_t rail_handle,
                                           uint16_t *p_channel);

/**
 * Return the symbol rate for the current PHY.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The symbol rate in symbols per second or 0.
 *
 * The symbol rate is the rate of symbol changes over the air. For non-DSSS
 * PHYs, this is the same as the baudrate. For DSSS PHYs, it is the baudrate
 * divided by the length of a chipping sequence. For more information,
 * see the modem calculator documentation. If the rate cannot be
 * calculated, this function returns 0.
 */
uint32_t sl_rail_get_symbol_rate(sl_rail_handle_t rail_handle);

/**
 * Return the bit rate for the current PHY.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The bit rate in bits per second or 0.
 *
 * The bit rate is the effective over-the-air data rate. It does not account
 * for extra spreading for forward error correction, and so on, but
 * accounts for modulation schemes, DSSS, and other configurations. For more
 * information, see the modem calculator documentation. If the rate cannot be
 * calculated, this function returns 0.
 */
uint32_t sl_rail_get_bit_rate(sl_rail_handle_t rail_handle);

/**
 * Get the sync words and their length.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_sync_word_config An application-provided non-NULL pointer to
 *   store \ref sl_rail_sync_word_config_t sync word information.
 * @return Status code indicating success of the function call.
 **/
sl_rail_status_t sl_rail_get_sync_words(sl_rail_handle_t rail_handle,
                                        sl_rail_sync_word_config_t *p_sync_word_config);

/**
 * Set the selected sync words and their length.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_sync_word_config A non-NULL pointer to \ref sl_rail_sync_word_config_t
 *   specifying the sync words and their length.
 *   The desired length should be between 2 and 32 bits inclusive, however it is
 *   recommended to not change the length below what the PHY sync word length is
 *   configured to be. Changing the sync word length, especially to that which is
 *   lower than the default length, may result in a decrease in packet reception
 *   rate or may not work at all.
 *   Other values will result in \ref SL_RAIL_STATUS_INVALID_PARAMETER. The default
 *   sync word continues to be valid.
 * @return Status code indicating success of the function call.
 *
 * When the custom sync word(s) applied by this API are no longer needed, or to
 * revert to default sync word, calling \ref sl_rail_config_channels() will re-establish
 * the sync words specified in the radio configuration.
 *
 * This function will return \ref SL_RAIL_STATUS_INVALID_STATE if called when BLE
 * has been enabled for this rail_handle. When changing sync words in BLE mode,
 * use \ref sl_rail_ble_config_channel_radio_params() instead.
 *
 * @note If multiple protocols share the same radio configuration, the user
 *   should not set custom sync words in any of those protocols as these
 *   sync words could leak into the other protocol sharing the same radio
 *   configuration.
 **/
sl_rail_status_t sl_rail_config_sync_words(sl_rail_handle_t rail_handle,
                                           const sl_rail_sync_word_config_t *p_sync_word_config);

/**
 * Sets the whitening initialization value.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The whitening initialization value currently being used.
 */
uint16_t sl_rail_get_whitening_init_val(sl_rail_handle_t rail_handle);

/**
 * Returns the CRC initialization value.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The CRC initialization value currently being used.
 */
uint32_t sl_rail_get_crc_init_val(sl_rail_handle_t rail_handle);

/**
 * Sets the whitening initialization value.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] white_init A whitening initialization value.
 * @return Status code indicating success of the function call.
 *
 * Use this function to override the whitening initialization value
 * defined by the current PHY's radio configuration. The new value
 * will persist until this function is called again, \ref
 * sl_rail_reset_whitening_init_val() is called, or the PHY is
 * changed.
 *
 * @note Overriding a PHY's whitening initialization value
 *   will break communication with peers unless they effect
 *   a similar change.
 *
 * @warning This API must not be used when either 802.15.4
 *   or BLE modes are enabled.
 */
sl_rail_status_t sl_rail_set_whitening_init_val(sl_rail_handle_t rail_handle,
                                                uint16_t white_init);

/**
 * Sets the CRC initialization value.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] crc_init A CRC initialization value.
 * @return Status code indicating success of the function call.
 *
 * Use this function to override the CRC initialization value
 * defined by the current PHY's radio configuration. The new value
 * will persist until this function is called again, \ref
 * sl_rail_reset_crc_init_val() is called, or the PHY is changed.
 *
 * @note Overriding a PHY's CRC initialization value
 *   will break communication with peers unless they effect
 *   a similar change.
 *
 * @warning This API must not be used when either 802.15.4
 *   or BLE modes are enabled.
 */
sl_rail_status_t sl_rail_set_crc_init_val(sl_rail_handle_t rail_handle,
                                          uint32_t crc_init);

/**
 * Restores the whitening initialization value to its initial setting
 * from the Radio Configurator.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * Can use this function after using \ref sl_rail_set_whitening_init_val().
 */
sl_rail_status_t sl_rail_reset_whitening_init_val(sl_rail_handle_t rail_handle);

/**
 * Restores the CRC initialization value to its initial setting from
 * the Radio Configurator.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * Can use this function after using \ref sl_rail_set_crc_init_val().
 */
sl_rail_status_t sl_rail_reset_crc_init_val(sl_rail_handle_t rail_handle);

/** @} */ // end of group Radio_Configuration

/******************************************************************************
 * Timing Information
 *****************************************************************************/
/// @addtogroup System_Timing System Timing
/// @brief Functionality related to the RAIL timer and general system time.
///
/// These functions can be used to get information about the current system time
/// or to manipulate the RAIL timer.
///
/// The system time returned by \ref sl_rail_get_time() is in the same timebase that is
/// used throughout RAIL. Any callbacks or structures that provide a time stamp,
/// such as \ref sl_rail_rx_packet_details_t::time_received, will use the same timebase
/// as will any APIs that accept an absolute time for scheduling their action.
/// Throughout the documentation, the timebase is referred to as the RAIL
/// timebase. The timebase is currently a value in microseconds from \ref
/// sl_rail_init() time, which means that it will wrap every 1.19 hours.
/// (`(2^32 - 1) / (3600 sec/hr * 1000000 us/sec)`).
///
/// The provided timer is hardware-backed and interrupt-driven. It can be used
/// for timing any event in the system, but is especially helpful for
/// timing protocol-based state machines and other systems that interact with
/// the radio. To avoid processing the expiration in interrupt
/// context, leave the cb parameter passed to \ref sl_rail_set_timer() as NULL and poll
/// for expiration with the \ref sl_rail_is_timer_expired() function. See below for an
/// example of the interrupt driven method of interacting with the timer.
/// @code{.c}
/// void timer_callback(sl_rail_handle_t rail_handle)
/// {
///   // Timer callback action
/// }
///
/// void main(void)
/// {
///   // Initialize RAIL ...
///
///   // Set up a timer for 1 ms from now
///   sl_rail_set_timer(rail_handle, 1000, SL_RAIL_TIME_DELAY, &timer_callback);
///
///   // Run main loop
///   while(1);
/// }
/// @endcode
///
/// If multiple software timers are needed to be run off of the one available
/// hardware timer, enable a software multiplexing layer within RAIL
/// using the \ref sl_rail_config_multi_timer() function. This will allow you to
/// set up as many timers as you want using the sl_rail_*_multi_timer() functions.
/// See the example below for using the multitimer functionality.
/// @code{.c}
/// // Declare timer structures in global space or somewhere that will exist
/// // until the callback has fired
/// sl_rail_multi_timer_t tmr_0, tmr_1;
///
/// void timer_callback(sl_rail_multi_timer_t *p_tmr,
///                     sl_rail_time_t expected_time_of_event,
///                     void *cb_arg)
/// {
///   if (p_tmr == &tmr_0) {
///     // Timer 1 action
///   } else {
///     // Timer 2 action
///   }
/// }
///
/// void main(void)
/// {
///   // Initialize RAIL ...
///
///   sl_rail_config_multi_timer(SL_RAIL_EFR32_HANDLE, true);
///
///   // Set up one timer for 1 ms from now and one at time 2000000 in the RAIL
///   // timebase
///   sl_rail_set_multi_timer(&tmr_0, 1000, SL_RAIL_TIME_DELAY, &timer_callback, NULL);
///   sl_rail_set_multi_timer(&tmr_1, 2000000, SL_RAIL_TIME_ABSOLUTE, &timer_callback, NULL);
///
///   // Run main loop
///   while(1);
/// }
/// @endcode
///
/// @{

/**
 * Get the current RAIL radio time.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return The RAIL radio time in microseconds. Note that this wraps
 *   after about 1.19 hours since it's stored in a 32 bit value.
 *
 * Returns the current radio time in the RAIL timebase (microseconds). It can be
 * used to compare with packet time stamps or to schedule transmits on that radio.
 */
sl_rail_time_t sl_rail_get_time(sl_rail_handle_t rail_handle);

/**
 * Set the current RAIL radio time.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in] time Set the RAIL radio time to this value in microseconds.
 * @return Status code indicating the success of the function call.
 *
 * @warning Use this API only for testing purposes or in
 *   very limited circumstances during RAIL Timer Synchronization.
 *   Undefined behavior can result by calling it in multiprotocol or
 *   when the radio is not idle or timed events are active. Applications
 *   using \ref sl_rail_get_time() may not be designed for discontinuous
 *   changes to the RAIL time base.
 */
sl_rail_status_t sl_rail_set_time(sl_rail_handle_t rail_handle,
                                  sl_rail_time_t time);

/**
 * Blocking delay routine for a specified number of microseconds.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in] microseconds Delay duration in microseconds.
 * @return Status code indicating success of the function call.
 *
 * Use this RAIL API only for short blocking delays because it has less overhead
 * than calling sl_rail_get_time() in a loop.
 *
 * @note Passing large delay values may give unpredictable results or trigger
 *   the Watchdog reset.
 *   \n Also, this function will start the clocks required for the RAIL timebase if they
 *   are not running, except between \ref sl_rail_sleep() and \ref sl_rail_wake()
 *   where the timer must remain stopped.
 *   \n Interrupts are not disabled during the delay, so the delay may be longer if an
 *   interrupt extends beyond the delay duration.
 */
sl_rail_status_t sl_rail_delay_us(sl_rail_handle_t rail_handle,
                                  sl_rail_time_t microseconds);

/**
 * Schedule a timer to expire using the RAIL timebase.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] time The timer's expiration time in the RAIL timebase.
 * @param[in] mode Indicates whether the time argument is an absolute
 *   RAIL time or relative to the current RAIL time. Specifying mode
 *   \ref SL_RAIL_TIME_DISABLED is the same as calling \ref sl_rail_cancel_timer().
 * @param[in] expiration_callback A pointer to a callback function that RAIL will call
 *   when the timer expires. May be NULL if no callback is desired.
 * @return \ref SL_RAIL_STATUS_NO_ERROR on success and
 *   \ref SL_RAIL_STATUS_INVALID_PARAMETER if the timer can't be scheduled.
 *
 * Configures a timer to expire after a period in the RAIL timebase.
 * This timer can be used to implement low-level protocol features.
 *
 * @warning Attempting to schedule the timer when it is
 *   still running from a previous request is bad practice, unless the cb
 *   callback is identical to that used in the previous request, in which case
 *   the timer is rescheduled to the new time. Note that if the original timer
 *   expires as it is being rescheduled, the callback may or may not occur. It
 *   is generally good practice to cancel a running timer before rescheduling
 *   it to minimize ambiguity.
 */
sl_rail_status_t sl_rail_set_timer(sl_rail_handle_t rail_handle,
                                   sl_rail_time_t time,
                                   sl_rail_time_mode_t mode,
                                   sl_rail_timer_callback_t expiration_callback);

/**
 * Return the absolute time that the RAIL timer was configured to expire.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The absolute time that this timer was set to expire.
 *
 * Provides the absolute time regardless of the \ref sl_rail_time_mode_t that
 * was passed into \ref sl_rail_set_timer(). Note that the time might be in the
 * past if the timer has already expired. The return value is undefined if the
 * timer was never set.
 */
sl_rail_time_t sl_rail_get_timer(sl_rail_handle_t rail_handle);

/**
 * Stop the currently scheduled RAIL timer.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return \ref SL_RAIL_STATUS_NO_ERROR on success and
 *   \ref SL_RAIL_STATUS_INVALID_CALL if the timer is not running.
 *
 * Cancels the timer. If this function is called before the timer expires,
 * the expiration_callback specified in the earlier sl_rail_set_timer() call will never
 * be called.
 */
sl_rail_status_t sl_rail_cancel_timer(sl_rail_handle_t rail_handle);

/**
 * Check whether the RAIL timer has expired.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return true if the previously scheduled timer has expired and false
 *   otherwise.
 *
 * Polling with this function is an alternative to the expiration_callback.
 */
bool sl_rail_is_timer_expired(sl_rail_handle_t rail_handle);

/**
 * Check whether the RAIL timer is currently running.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return true if the timer is running and false if
 *   the timer has expired or was never set.
 */
bool sl_rail_is_timer_running(sl_rail_handle_t rail_handle);

/**
 * Configure the RAIL software timer feature on a radio.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in] enable Enables/disables the RAIL multitimer.
 * @return \ref SL_RAIL_STATUS_NO_ERROR if the multitimer was successfully
 *   enabled/disabled, an appropriate error otherwise.
 *
 * Turning this on will add a software timer layer above the physical RAIL timer
 * so that the user can have as many timers as desired. It is not necessary to
 * call this function if the multi_timer APIs are not used.
 *
 * @note This function must be called before calling \ref sl_rail_set_multi_timer().
 *   This function is a no-op on multiprotocol as this layer is already used
 *   under the hood.
 *   Do not call this function while the RAIL timer is running.
 *   Call \ref sl_rail_is_timer_running() before enabling/disabling the multitimer.
 *   If the multitimer is not needed, do not call this function to
 *   allow the multitimer code to be dead stripped. If the multitimer is
 *   enabled for use, the multitimer and timer APIs can both be used.
 *   However, no timer can be in use while this function is being called.
 */
sl_rail_status_t sl_rail_config_multi_timer(sl_rail_handle_t rail_handle,
                                            bool enable);

/**
 * Start a multitimer instance.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in,out] p_tmr A pointer to the timer instance to start.
 * @param[in] expiration_time A time when the timer is set to expire.
 * @param[in] expiration_mode Select mode of expiration_time. See \ref
 *   sl_rail_time_mode_t.
 * @param[in] expiration_callback A function to call on timer expiry. See \ref
 *   sl_rail_multi_timer_callback_t(). May be NULL if no callback is desired.
 * @param[in] cb_arg An extra callback function parameter for the user application.
 *   Since the \ref sl_rail_multi_timer_callback_t() callback function lacks a
 *   \ref sl_rail_handle_t parameter this can be used to pass the current
 *   RAIL handle if desired.
 * @return
 *   \ref SL_RAIL_STATUS_NO_ERROR on success.@n
 *   \ref SL_RAIL_STATUS_INVALID_PARAMETER if tmr has an illegal value or if
 *   timeout is in the past.
 *
 * @note It is legal to start an already running timer. If this is done,
 *   the timer will first be stopped before the new configuration is applied.
 *   If expiration_time is 0, the callback is called immediately.
 */
sl_rail_status_t sl_rail_set_multi_timer(sl_rail_handle_t rail_handle,
                                         sl_rail_multi_timer_t *p_tmr,
                                         sl_rail_time_t expiration_time,
                                         sl_rail_time_mode_t expiration_mode,
                                         sl_rail_multi_timer_callback_t expiration_callback,
                                         void *cb_arg);

/**
 * Stop the currently scheduled RAIL multitimer.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in,out] p_tmr A pointer to a RAIL timer instance.
 * @return \ref SL_RAIL_STATUS_NO_ERROR if the timer was successfully canceled,
 *   or an error status otherwise.
 *
 * Cancels the timer. If this function is called before the timer expires,
 * the expiration_callback specified in the earlier \ref sl_rail_set_timer() call will never
 * be called.
 */
sl_rail_status_t sl_rail_cancel_multi_timer(sl_rail_handle_t rail_handle,
                                            sl_rail_multi_timer_t *p_tmr);

/**
 * Check if a given timer is running.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in,out] p_tmr A pointer to the timer instance.
 * @return true if the timer is running; false if the timer is not running
 *    or p_tmr is not a timer instance associated with rail_handle.
 */
bool sl_rail_is_multi_timer_running(sl_rail_handle_t rail_handle,
                                    sl_rail_multi_timer_t *p_tmr);

/**
 * Check if a given timer has expired.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in,out] p_tmr A pointer to the timer instance.
 * @return true if the timer has expired or p_tmr is not a timer instance
 *   associated with rail_handle; false if the timer is running.
 */
bool sl_rail_is_multi_timer_expired(sl_rail_handle_t rail_handle,
                                    sl_rail_multi_timer_t *p_tmr);

/**
 * Get time left before a given timer instance expires.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in,out] p_tmr A pointer to the timer instance to query.
 * @param[in] time_mode Indicates how the function provides the time
 *   remaining. By choosing \ref
 *   sl_rail_time_mode_t::SL_RAIL_TIME_ABSOLUTE, the function returns the
 *   absolute expiration time, and by choosing \ref
 *   sl_rail_time_mode_t::SL_RAIL_TIME_DELAY, the function returns the
 *   amount of time remaining before the timer's expiration.
 * @return
 *   Time left expressed in the RAIL radio's time units.
 *   0 if the timer is not running or has already expired.
 */
sl_rail_time_t sl_rail_get_multi_timer(sl_rail_handle_t rail_handle,
                                       sl_rail_multi_timer_t *p_tmr,
                                       sl_rail_time_mode_t time_mode);

/** @} */ // end of group System_Timing

/******************************************************************************
 * Sleep APIs
 *****************************************************************************/
/// @addtogroup Sleep
/// @brief These APIs help when putting the system to an EM2/EM3/EM4 sleep
/// states where the high frequency clock is disabled.
/// @{
///
/// The RAIL library has its own timebase and the ability to schedule operations
/// into the future. When going to any power mode that disables the HF clock
/// used for the radio (EM2/EM3/EM4), it is important that this timebase is
/// synchronized to a running LFCLK and the chip is set to wake up before the
/// next scheduled event.
/// If RAIL has not been configured to use the power manager,
/// \ref sl_rail_sleep() and \ref sl_rail_wake() must be called for performing this
/// synchronization.
/// If RAIL has been configured to use the power manager,
/// \ref sl_rail_init_power_manager(), it will automatically perform timer
/// synchronization based on the selected \ref sl_rail_timer_sync_config_t. Calls to
/// \ref sl_rail_sleep() and \ref sl_rail_wake() are unsupported in such a scenario.
///
/// Following example code snippets demonstrate synchronizing the timebase
/// with and without timer synchronization:
///
/// <b>Sleep with timer synchronization:</b>
///
/// When sleeping with timer synchronization, you must first get the required
/// LFCLK up and running and leave it running across sleep so that the high
/// frequency clock that drives the RAIL time base can be synchronized to it.
/// The \ref sl_rail_sleep() API will also set up a wake event on the timer to wake
/// up wakeup_process_time_us before the next timer event so that it can run successfully.
/// See the \ref efr32_main sections on Low-Frequency Clocks and RAIL Timer
/// Synchronization for more setup details.
///
/// This is useful when maintaining packet time stamps
/// across sleep or use the scheduled RX/TX APIs while sleeping in between. It
/// does take more time and code to do the synchronization. If your
/// application does not need this, it should be avoided.
///
/// Example (without Power Manager):
/// @code{.c}
/// #include "sl_rail.h"
///
/// extern sl_rail_handle_t rail_handle;
/// // Wakeup time for your crystal/board/chip combination
/// extern uint32_t wakeup_process_time_us;
///
/// void main(void)
/// {
///   sl_rail_status_t status;
///   bool should_sleep = false;
///
///   // This function depends on your board/chip but it must enable the LFCLK
///   // you intend to use for RTCC sync before we configure sleep as that function
///   // will attempt to auto detect the clock.
///   Board_Setup_LFCLK();
///
///   // Configure sleep for timer synchronization
///   sl_rail_timer_sync_config_t timer_sync_config = SL_RAIL_TIMER_SYNC_DEFAULT;
///   status = sl_rail_config_sleep(rail_handle, &timer_sync_config);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///
///   // Application main loop
///   while(1) {
///     // ... do normal app stuff and set should_sleep to true when we want to
///     // sleep
///     if (should_sleep) {
///       bool sleep_allowed = false;
///
///       // Go critical to assess sleep decisions
///       CORE_ENTER_CRITICAL();
///       if (sl_rail_sleep(wakeup_process_time_us, &sleep_allowed) != SL_RAIL_STATUS_NO_ERROR) {
///         printf("Error trying to go to sleep!");
///         CORE_EXIT_CRITICAL();
///         continue;
///       }
///       if (sleep_allowed) {
///         // Go to sleep
///       }
///       // Wakeup and sync the RAIL timebase back up
///       sl_rail_wake(0);
///       CORE_EXIT_CRITICAL();
///     }
///   }
/// }
/// @endcode
///
/// Example (with Power Manager):
/// @code{.c}
/// #include "sl_rail.h"
/// #include "sl_power_manager.h"
///
/// extern sl_rail_handle_t rail_handle;
///
/// void main(void)
/// {
///   sl_rail_status_t status;
///   bool should_sleep = false;
///
///   // This function depends on your board/chip but it must enable the LFCLK
///   // you intend to use for RTCC sync before we configure sleep as that function
///   // will attempt to auto detect the clock.
///   Board_Setup_LFCLK();
///   // Configure sleep for timer synchronization
///   sl_rail_timer_sync_config_t timer_sync_config = SL_RAIL_TIMER_SYNC_DEFAULT;
///   status = sl_rail_config_sleep(rail_handle, &timer_sync_config);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///   // Initialize application-level power manager service
///   sl_power_manager_init();
///   // Initialize RAIL library's use of the power manager
///   sl_rail_init_power_manager();
///
///   // Application main loop
///   while (1) {
///     // ... do normal app stuff and set should_sleep to true when we want to
///     // sleep
///     if (should_sleep) {
///       // Let the CPU go to sleep if the system allows it.
///       sl_power_manager_sleep();
///     }
///   }
/// }
/// @endcode
///
/// RAIL APIs such as, \ref sl_rail_start_scheduled_tx(), \ref sl_rail_start_scheduled_rx(),
/// \ref sl_rail_set_timer(), \ref sl_rail_set_multi_timer() can be used to schedule periodic
/// wakeups to perform a scheduled operation. The call to
/// sl_power_manager_sleep() in the main loop ensures that the device sleeps
/// until the scheduled operation is due.
/// Upon completion, each instantaneous or scheduled RX/TX operation will
/// indicate radio busy to the power manager to allow the application to
/// service the RAIL event and perform subsequent operations before going to
/// sleep. Therefore, it is important that the application idle the radio by either
/// calling \ref sl_rail_idle() or \ref sl_rail_yield_radio().
/// If the radio transitions to RX after an RX or TX operation,
/// always call \ref sl_rail_idle() in order transition to a lower sleep state.
/// If the radio transitions to idle after an RX or TX operation,
/// \ref sl_rail_yield_radio() should suffice in indicating to the power manager
/// that the radio is no longer busy and the device can sleep.
///
/// The following example shows scheduling periodic TX on getting a TX completion
/// event:
/// @code{.c}
/// void rail_events_callback(sl_rail_handle_t rail_handle, sl_rail_events_t events)
/// {
///   // Omitting other event handlers
///   if (events & SL_RAIL_EVENTS_TX_COMPLETION) {
///     // Schedule the next TX.
///     sl_rail_scheduled_tx_config_t config = {
///       .when = (sl_rail_time_t)parameters->start_time,
///       .mode = (sl_rail_time_mode_t)parameters->start_time_mode
///     };
///     (void)sl_rail_start_scheduled_tx(rail_handle, channel, 0, &config, NULL);
///   }
/// }
/// @endcode
///
/// @note The above code assumes that RAIL automatic state transitions after TX
///   are idle. Use \ref sl_rail_set_tx_transitions() to ensure the right state
///   transitions are used. Radio must be idle for the device to enter EM2 or lower
///   energy mode.
///
/// @note When using the power manager, usage of \ref sl_rail_yield_radio() in
///   single protocol RAIL is similar to its usage in multiprotocol RAIL.
///   See \ref rail_radio_scheduler_yield for more details.
///
/// @note Back to back scheduled operations do not require an explicit call to
///   \ref sl_rail_yield_radio() if the radio transitions to idle.
///
/// <b>Sleep without timer synchronization:</b>
///
/// When sleeping without timer synchronization, you are free to enable only the
/// LFCLKs and wake sources required by the application. RAIL will not attempt
/// to configure any wake events and may miss anything that occurs over sleep.
///
/// This is useful when your application does not care about
/// packet time stamps or scheduling operations accurately over sleep.
///
/// Example (without Power Manager):
/// @code{.c}
/// #include "sl_rail.h"
///
/// extern sl_rail_handle_t rail_handle;
///
/// void main(void)
/// {
///   sl_rail_status_t status;
///   bool should_sleep = false;
///
///   // Configure sleep for no timer synchronization
///   sl_rail_timer_sync_config_t timer_sync_config = {
///     .sleep = SL_RAIL_SLEEP_CONFIG_TIMERSYNC_DISABLED,
///   };
///   status = sl_rail_config_sleep(rail_handle, &timer_sync_config);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///
///   // Application main loop
///   while(1) {
///     // ... do normal app stuff and set should_sleep to true when we want to
///     // sleep
///     if (should_sleep) {
///       bool sleep_allowed = false;
///       uint32_t sleep_time = 0;
///
///       // Go critical to assess sleep decisions
///       CORE_ENTER_CRITICAL();
///       if (sl_rail_sleep(0, &sleep_allowed) != SL_RAIL_STATUS_NO_ERROR) {
///         printf("Error trying to go to sleep!");
///         CORE_EXIT_CRITICAL();
///         continue;
///       }
///       if (sleep_allowed) {
///         // Go to sleep and optionally update sleep_time to the correct value
///         // in microseconds
///       }
///       // Wakeup and sync the RAIL timebase back up
///       sl_rail_wake(sleep_time);
///       CORE_EXIT_CRITICAL();
///     }
///   }
/// }
/// @endcode
///
/// Example (with Power Manager):
/// @code{.c}
/// #include "sl_rail.h"
/// #include "sl_power_manager.h"
///
/// extern sl_rail_handle_t rail_handle;
///
/// void main(void)
/// {
///   sl_rail_status_t status;
///   bool should_sleep = false;
///
///   // This function depends on your board/chip but it must enable the LFCLK
///   // you intend to use for RTCC sync before we configure sleep as that function
///   // will attempt to auto detect the clock.
///   Board_Setup_LFCLK();
///   // Configure sleep for no timer synchronization
///   sl_rail_timer_sync_config_t timer_sync_config = {
///     .sleep = SL_RAIL_SLEEP_CONFIG_TIMERSYNC_DISABLED,
///   };
///   status = sl_rail_config_sleep(rail_handle, &timer_sync_config);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///   // Initialize application-level power manager service
///   sl_power_manager_init();
///   // Initialize RAIL library's use of the power manager
///   sl_rail_init_power_manager();
///
///   // Application main loop
///   while(1) {
///     // ... do normal app stuff and set should_sleep to true when we want to
///     // sleep
///     if (should_sleep) {
///       // Let the CPU go to sleep if the system allows it.
///       sl_power_manager_sleep();
///     }
///   }
/// }
/// @endcode

/**
 * Initialize RAIL timer synchronization.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_timer_sync_config A non-NULL pointer to the timer synchronization configuration.
 * @return Status code indicating success of the function call.
 *
 * The default structure used to enable timer synchronization across sleep is
 * \ref SL_RAIL_TIMER_SYNC_DEFAULT.
 *
 * @warning As this function relies on PRS and SYSRTC access and RAIL is meant
 *   to run in TrustZone non-secure world, it is not supported if PRS or SYSRTC
 *   are configured as secure peripheral and p_timer_sync_config->sleep is set
 *   to \ref sl_rail_sleep_config_t::SL_RAIL_SLEEP_CONFIG_TIMERSYNC_ENABLED. It
 *   will return \ref SL_RAIL_STATUS_INVALID_CALL.
 */
sl_rail_status_t sl_rail_config_sleep(sl_rail_handle_t rail_handle,
                                      const sl_rail_timer_sync_config_t *p_timer_sync_config);

/**
 * Stop the RAIL timer(s) and prepare RAIL for sleep.
 *
 * @param[in] wakeup_process_time_us Time in microseconds that the application
 *   and hardware need to recover from sleep state.
 * @param[out] p_deep_sleep_allowed A pointer to boolean that will be set
 *   true if system can go to deep sleep or false if system must not go
 *   to deep sleep (EM2 or lower energy modes).
 * @return Status code indicating success of the function call.
 *
 * @warning The active RAIL configuration must be idle to enable sleep.
 *
 * @note This API must not be called if RAIL Power Manager is initialized.
 */
sl_rail_status_t sl_rail_sleep(uint16_t wakeup_process_time_us,
                               bool *p_deep_sleep_allowed);

/**
 * Wake RAIL from sleep and restart the RAIL timer(s).
 *
 * @param[in] elapsed_time_us Add this sleep duration in microseconds
 *   to the RAIL timer(s) before restarting it(them).
 * @return Status code indicating success of the function call.
 *
 * If the timer sync was enabled by \ref sl_rail_config_sleep(), synchronize the RAIL
 * timer(s) using an alternate timer. Otherwise, add elapsed_time_us to the RAIL
 * timer(s).
 *
 * @note This API must not be called if RAIL Power Manager is initialized.
 */
sl_rail_status_t sl_rail_wake(sl_rail_time_t elapsed_time_us);

/**
 * Initialize RAIL Power Manager.
 *
 * @return Status code indicating success of the function call.
 *
 * @note This function must be called only when the application is built
 *   and initialized with Power Manager plugin and when the radio is idle.
 *   RAIL will perform timer synchronization, upon transitioning from EM2 or
 *   lower to EM1 or higher energy mode or vice-versa, in the Power Manager EM
 *   transition callback.
 *
 * @warning Since EM transition callbacks are not called in a deterministic
 *   order, it is suggested to not call any RAIL time dependent APIs
 *   in an EM transition callback.
 *
 * @warning As this function relies on EMU access and RAIL is meant to run in
 *   TrustZone non-secure world, it is not supported if EMU is configured as
 *   secure peripheral and it will return \ref SL_RAIL_STATUS_INVALID_CALL.
 */
sl_rail_status_t sl_rail_init_power_manager(void);

/**
 * Stop the RAIL Power Manager.
 *
 * @return Status code indicating success of the function call.
 *
 * @note The active RAIL configuration must be idle to disable radio
 *   power manager and there should be no outstanding requirements by
 *   radio power manager.
 */
sl_rail_status_t sl_rail_deinit_power_manager(void);

/** @} */ // end of group Sleep

/******************************************************************************
 * Events
 *****************************************************************************/
/**
 * @addtogroup Events
 * @brief APIs related to events
 * @{
 */

/**
 * Configure radio events.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] mask A bitmask of events to configure.
 * @param[in] events A bitmask of events to trigger \ref sl_rail_config_t::events_callback().
 * @return Status code indicating success of the function call.
 *
 * Sets up which radio interrupts generate a RAIL event. The full list of
 * events is in \ref sl_rail_events_t.
 */
sl_rail_status_t sl_rail_config_events(sl_rail_handle_t rail_handle,
                                       sl_rail_events_t mask,
                                       sl_rail_events_t events);

/** @} */ // end of group Events

/******************************************************************************
 * Data Management
 *****************************************************************************/
/// @addtogroup Data_Management Data Management
/// @brief Data management functions
///
/// These functions allow the application to choose how data is presented to
/// the application. RAIL provides data in a packet-based method or in a
/// FIFO-based method. As originally conceived,
/// \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE was designed for handling packets
/// that fit within RAIL's FIFOs while \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE
/// was designed for handling packets larger than RAIL's FIFOs could hold.
/// Conceptually it is still useful to think of these modes this way, but
/// functionally their distinction has become blurred by improvements in
/// RAIL's flexibility -- applications now have much more control over both
/// receive and transmit FIFO sizes, and the FIFO-management and threshold
/// APIs and related events are no longer restricted to \ref
/// sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE operation but can be used in \ref
/// sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE too.
///
/// The application can configure RAIL data management through \ref
/// sl_rail_config_tx_data() and \ref sl_rail_config_rx_data(). These functions allows the
/// application to specify the type of radio data (\ref sl_rail_tx_data_source_t
/// and \ref sl_rail_rx_data_source_t) and the method of interacting with data
/// (\ref sl_rail_data_method_t). By default, RAIL configures TX and RX both
/// with packet data source and \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE.
///
/// For transmit, \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE and \ref
/// sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE are functionally the same:
///   - When not actively transmitting, load a packet's initial transmit
///     data using \ref sl_rail_write_tx_fifo() with reset set to true. Alternatively
///     this data copying can be avoided by changing the transmit FIFO to an
///     already-loaded section of memory with \ref sl_rail_set_tx_fifo().
///   - When actively transmitting, load remaining transmit data with
///     \ref sl_rail_write_tx_fifo() with reset set to false.
///   - If transmit packets exceed the FIFO size, set the transmit FIFO
///     threshold through \ref sl_rail_set_tx_fifo_threshold(). The \ref
///     sl_rail_config_t::events_callback() with \ref SL_RAIL_EVENT_TX_FIFO_ALMOST_EMPTY
///     will occur telling the application to load more TX packet data, if
///     needed, to prevent a \ref SL_RAIL_EVENT_TX_UNDERFLOW event from occurring.
///     One can get how much space is available in the transmit FIFO for more
///     transmit data through \ref sl_rail_get_tx_fifo_space_available().
///   - After transmit completes, the transmit FIFO can be manually reset
///     with \ref sl_rail_reset_fifo(), but this should rarely be necessary.
///
/// The transmit FIFO is specified by the application and its size is
/// the value returned from the most recent call to \ref sl_rail_set_tx_fifo().
/// The transmit FIFO is edge-based in that it only provides the \ref
/// SL_RAIL_EVENT_TX_FIFO_ALMOST_EMPTY event once when the threshold is crossed
/// in the emptying direction.
///
/// For receive, the distinction between \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE
/// and \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE basically boils down to how
/// unsuccessfully-received packets are handled. In \ref
/// sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE, data from such packets is automatically
/// rolled back as if the packet was never received, while in \ref
/// sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE, rollback does not occur putting more onus
/// on the application to deal with that data.
///
/// In receive \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE data management:
///   - Packet lengths are determined from the Radio Configurator configuration
///     and can be read out at the end using \ref sl_rail_get_rx_packet_info().
///   - Received packet data is made available on successful packet completion
///     via \ref sl_rail_config_t::events_callback() with \ref
///     SL_RAIL_EVENT_RX_PACKET_RECEIVED which can then use \ref sl_rail_get_rx_packet_info()
///     and \ref sl_rail_get_rx_packet_details() to access packet information and
///     \ref sl_rail_peek_rx_packet() to access packet data.
///   - FILTERED, ABORTED, or FRAMEERROR received packet data is automatically
///     rolled back (dropped) without the application needing to worry about
///     consuming it.
///     The application can choose to not even be bothered with the events
///     related to such packets: \ref SL_RAIL_EVENT_RX_ADDRESS_FILTERED,
///     \ref SL_RAIL_EVENT_RX_PACKET_ABORTED, or \ref SL_RAIL_EVENT_RX_FRAME_ERROR.
///
/// In receive \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE data management:
///   - Packet Lengths are determined from the Radio Configurator configuration
///     or by application knowledge of packet payload structure.
///   - Received data can be retrieved prior to packet completion through
///     \ref sl_rail_read_rx_fifo() and is never rolled back on FILTERED, ABORTED, or
///     FRAMEERROR packets. The application should enable and handle these
///     events so it can flush any packet data it's already retrieved.
///   - After packet completion, remaining packet data for FILTERED, ABORTED,
///     or FRAMEERROR packets remains in the FIFO and the appropriate event is
///     triggered to the user. This data may be consumed in the callback unlike
///     in packet mode where it is automatically rolled back. At the end of the
///     callback all remaining data in the FIFO will be cleaned up as usual.
///     Keep in mind that \ref sl_rail_get_rx_packet_details() provides full packet
///     detailed information only for successfully received packets.
///
/// Common receive data management features:
///   - Set the receive FIFO threshold through \ref sl_rail_set_rx_fifo_threshold(). The
///     \ref sl_rail_config_t::events_callback() with \ref SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL
///     will occur telling the application to consume some RX packet data to
///     prevent a \ref SL_RAIL_EVENT_RX_FIFO_OVERFLOW event from occurring.
///   - Get receive FIFO count information through
///     \ref sl_rail_get_rx_packet_info(\ref SL_RAIL_RX_PACKET_HANDLE_NEWEST)
///     (or \ref sl_rail_get_rx_fifo_bytes_available()).
///   - After receive completes and all its data has been consumed, the receive
///     FIFO and associated receive Packet Queue can be manually reset with
///     \ref sl_rail_reset_fifo(), though this should rarely be necessary and should
///     only be done with the radio idle.
///
/// When trying to determine an appropriate threshold, the application needs
/// to know the size of each FIFO. This can be obtained via \ref
/// sl_rail_get_config() if not already known. The receive FIFO event is
/// level-based in that the \ref SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL event will
/// constantly pend if the threshold is exceeded. This normally means that
/// inside this event's callback, the application should empty enough of the FIFO
/// to go under the threshold. To defer reading the FIFO to main context, the
/// application can disable or re-enable the receive FIFO threshold event using
/// \ref sl_rail_config_events() with the mask \ref SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL.
///
/// The receive FIFO can store multiple packets and processing of a packet can
/// be deferred from the RAIL event callback to main-loop processing
/// by using \ref sl_rail_hold_rx_packet() in the event callback and
/// \ref sl_rail_release_rx_packet() in the main-loop.
/// The receive FIFO is supplemented by the receive Packet Queue that limits
/// the number of packets RAIL and applications can hold onto for deferred
/// processing.
///
/// @note When using multiprotocol, if a protocol's receive FIFO or
///   receive Packet Queue is shared with another protocol, they will be reset
///   during a protocol switch so held packets will be lost if not processed
///   before then.
///
/// While \ref SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL occurs solely based on the
/// state of the receive FIFO used for packet data, both
/// \ref SL_RAIL_EVENT_RX_FIFO_FULL and \ref SL_RAIL_EVENT_RX_FIFO_OVERFLOW
/// can occur coincident with packet completion when either that or the
/// receive Packet Queue fills or overflows.
/// \ref SL_RAIL_EVENT_RX_FIFO_FULL informs the application it should
/// immediately process and free up the oldest packets/data to make room
/// for new packets/data, reducing the possibility of packet/data loss
/// and \ref SL_RAIL_EVENT_RX_FIFO_OVERFLOW.
///
/// Before a packet is fully received you can always use \ref
/// sl_rail_peek_rx_packet() to look at the contents. In FIFO mode, you may also
/// consume its data with \ref sl_rail_read_rx_fifo(). Remember that none of these
/// APIs will read across a packet boundary (even in FIFO mode) so you will
/// need to handle each received packet individually.
///
/// While RAIL defaults to \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE, the
/// application can explicitly initialize RAIL for \ref
/// sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE in the following manner:
/// @code{.c}
/// extern sl_rail_handle_t rail_handle;
///
/// void config_rail_for_packet_mode_operation(void)
/// {
///   sl_rail_tx_data_config_t rail_tx_data_config = {
///     .tx_source = SL_RAIL_TX_DATA_SOURCE_PACKET_DATA,
///     .tx_method = SL_RAIL_DATA_METHOD_PACKET_MODE,
///   };
///   sl_rail_rx_data_config_t rail_rx_data_config = {
///     .rx_source = SL_RAIL_RX_DATA_SOURCE_PACKET_DATA,
///     .rx_method = SL_RAIL_DATA_METHOD_PACKET_MODE,
///   };
///
///   (void) sl_rail_config_tx_data(rail_handle, &rail_tx_data_config);
///   (void) sl_rail_config_rx_data(rail_handle, &rail_rx_data_config);
///
///   // Events that can occur in Packet Mode:
///   //   SL_RAIL_EVENT_TX_PACKET_SENT
///   //   SL_RAIL_EVENT_RX_PACKET_RECEIVED
///   // and optionally (packet data automatically dropped):
///   //   SL_RAIL_EVENT_RX_ADDRESS_FILTERED
///   //   SL_RAIL_EVENT_RX_PACKET_ABORTED
///   //   SL_RAIL_EVENT_RX_FRAME_ERROR
///   //   SL_RAIL_EVENT_RX_FIFO_OVERFLOW
///   // and if enabled:
///   //   SL_RAIL_EVENT_TX_UNDERFLOW
///   //   SL_RAIL_EVENT_TXACK_UNDERFLOW
///   //   SL_RAIL_EVENT_TX_FIFO_ALMOST_EMPTY
///   //   SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL
/// }
/// @endcode
///
/// Initializing RAIL for \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE requires a few
/// more function calls:
/// @code{.c}
/// extern sl_rail_handle_t rail_handle;
///
/// void config_rail_for_fifo_mode_operation(void)
/// {
///   sl_rail_tx_data_config_t rail_tx_data_config = {
///     .tx_source = SL_RAIL_TX_DATA_SOURCE_PACKET_DATA,
///     .tx_method = SL_RAIL_DATA_METHOD_FIFO_MODE,
///   };
///   sl_rail_rx_data_config_t rail_rx_data_config = {
///     .rx_source = SL_RAIL_RX_DATA_SOURCE_PACKET_DATA,
///     .rx_method = SL_RAIL_DATA_METHOD_FIFO_MODE,
///   };
///
///   (void) sl_rail_config_tx_data(rail_handle, &rail_tx_data_config);
///   (void) sl_rail_config_rx_data(rail_handle, &rail_rx_data_config);
///
///   // Gets the size of the FIFOs.
///   // Assume that the transmit and receive FIFOs are the same size
///   uint16_t fifo_size = sl_rail_get_tx_fifo_space_available(rail_handle);
///
///   // Sets the transmit and receive FIFO thresholds.
///   // For this example, set the threshold in the middle of each FIFO.
///   (void) sl_rail_set_rx_fifo_threshold(rail_handle, fifo_size / 2);
///   (void) sl_rail_set_tx_fifo_threshold(rail_handle, fifo_size / 2);
///
///   // Events that can occur in FIFO mode:
///   //   SL_RAIL_EVENT_TX_FIFO_ALMOST_EMPTY
///   //   SL_RAIL_EVENT_TX_UNDERFLOW
///   //   SL_RAIL_EVENT_TXACK_UNDERFLOW
///   //   SL_RAIL_EVENT_TX_PACKET_SENT
///   //   SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL
///   //   SL_RAIL_EVENT_RX_FIFO_OVERFLOW
///   //   SL_RAIL_EVENT_RX_ADDRESS_FILTERED
///   //   SL_RAIL_EVENT_RX_PACKET_ABORTED
///   //   SL_RAIL_EVENT_RX_FRAME_ERROR
///   //   SL_RAIL_EVENT_RX_PACKET_RECEIVED
/// }
/// @endcode
///
/// On receive, an application can use a different \ref sl_rail_rx_data_source_t that
/// is only compatible with \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE. All that differs
/// from the FIFO mode example above is the sl_rail_rx_data_config_t::rx_source setting.
/// IQ data samples are taken at the hardware's oversample rate and the amount
/// of data can easily overwhelm the CPU processing time. The sample rate
/// depends on the chosen PHY, as determined by the data rate and the decimation
/// chain. It is <b>not</b> recommended to use the IQ data source with sample
/// rates above 300 k samples/second because the CPU might not be able to keep up
/// with the data stream. Depending on the application and the needed CPU
/// bandwidth, slower data rates may be required. On EFR32xG22 and later
/// platforms, it is recommended to reset the RX buffer before initiating a
/// receive for all modes except \ref sl_rail_rx_data_source_t::SL_RAIL_RX_DATA_SOURCE_PACKET_DATA since
/// these other modes require the RX buffer's current write offset to be 32-bit
/// aligned. If not reset, capture is performed to the remaining aligned space
/// available. An unaligned write offset will cause \ref sl_rail_config_rx_data() to
/// return error \ref SL_RAIL_STATUS_INVALID_STATE.
///
/// @code{.c}
/// // Reset RX buffer (EFR32xG22 and later platforms)
/// sl_rail_reset_fifo(rail_handle, false, true);
///
/// // IQ data is provided into the receive FIFO.
/// sl_rail_tx_data_config_t rail_tx_data_config = {
///   .tx_source = SL_RAIL_TX_DATA_SOURCE_PACKET_DATA,
///   .tx_method = SL_RAIL_DATA_METHOD_FIFO_MODE,
/// };
/// sl_rail_tx_data_config_t rail_tx_data_config = {
///   .rx_source = SL_RAIL_RX_DATA_SOURCE_IQDATA_FILTLSB,
///   .rx_method = SL_RAIL_DATA_METHOD_FIFO_MODE,
/// };
///
/// // IQ data comes in the following format when reading out of the FIFO:
/// //------------------------------------
/// // I[LSB] | I[MSB] | Q[LSB] | Q[MSB] |
/// //------------------------------------
/// @endcode
///
/// @{

/**
 * RAIL TX data management configuration.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_tx_data_config A non-NULL pointer to RAIL TX data configuration structure.
 * @return Status code indicating success of the function call.
 *
 * This function configures how RAIL manages TX data.
 *
 * Generally with \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE,
 * the application sets an appropriate FIFO threshold via
 * \ref sl_rail_set_tx_fifo_threshold() and then enables and handles the
 * \ref SL_RAIL_EVENT_TX_FIFO_ALMOST_EMPTY event callback (to feed more packet
 * data via \ref sl_rail_write_tx_fifo() before the FIFO underflows).
 *
 * When configuring TX for \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE, this
 * function resets the transmit FIFO. When configuring TX for
 * \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE, this function will reset
 * the corresponding FIFO thresholds such that they won't trigger
 * \ref SL_RAIL_EVENT_TX_FIFO_ALMOST_EMPTY events.
 */
sl_rail_status_t sl_rail_config_tx_data(sl_rail_handle_t rail_handle,
                                        const sl_rail_tx_data_config_t *p_tx_data_config);

/**
 * RAIL RX data management configuration.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_rx_data_config A non-NULL pointer to RAIL RX data configuration structure.
 * @return Status code indicating success of the function call.
 *   On EFR32xG22 and later platforms, \ref SL_RAIL_STATUS_INVALID_STATE
 *   indicates the RX buffer's current write offset is misaligned for the
 *   requested \ref sl_rail_rx_data_source_t.
 *
 * This function configures how RAIL manages RX data. The application can
 * configure RAIL to receive data in a packet-based or FIFO-based manner.
 * \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE is necessary to receive packets larger
 * than the radio's receive FIFO. It is also required for receive data
 * sources other than \ref sl_rail_rx_data_source_t::SL_RAIL_RX_DATA_SOURCE_PACKET_DATA.
 *
 * Generally with \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE, the
 * application sets an appropriate FIFO threshold via
 * \ref sl_rail_set_rx_fifo_threshold() and then enables and handles the \ref
 * SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL event callback (to consume packet data
 * via \ref sl_rail_read_rx_fifo() before the receive FIFO overflows).
 *
 * When configuring RX for \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE, this
 * function will reset the corresponding FIFO thresholds such that they
 * won't trigger \ref SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL events.
 *
 * When \ref sl_rail_rx_data_config_t::rx_method is set to \ref
 * sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE, the radio won't drop packet data of
 * aborted or CRC error packets, but will present it to the application
 * to deal with accordingly. On completion of erroneous packets, the
 * \ref sl_rail_config_t::events_callback() with \ref SL_RAIL_EVENT_RX_PACKET_ABORTED,
 * \ref SL_RAIL_EVENT_RX_FRAME_ERROR, or \ref SL_RAIL_EVENT_RX_ADDRESS_FILTERED will
 * tell the application it can drop any data it read via \ref sl_rail_read_rx_fifo() during reception.
 * For CRC error packets when the \ref SL_RAIL_RX_OPTION_IGNORE_CRC_ERRORS
 * RX option is in effect, the application should check for that from the
 * \ref sl_rail_rx_packet_status_t obtained by calling \ref sl_rail_get_rx_packet_info().
 * RAIL will automatically flush any remaining packet data after reporting
 * one of these packet completion events or the application can explicitly
 * flush it by calling \ref sl_rail_release_rx_packet().
 *
 * When \ref sl_rail_rx_data_config_t::rx_method is set to \ref
 * sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE, the radio will roll back (drop) all packet
 * data associated with aborted packets including those with CRC errors
 * (unless configured to ignore CRC errors via the
 * \ref SL_RAIL_RX_OPTION_IGNORE_CRC_ERRORS RX option). The application will
 * never have to deal with packet data from these packets.
 * In either mode, the application can set RX options as needed.
 *
 * When \ref sl_rail_rx_data_config_t::rx_source is set to a value other than
 * \ref SL_RAIL_RX_DATA_SOURCE_PACKET_DATA and \ref sl_rail_config_t::events_callback()
 * \ref SL_RAIL_EVENT_RX_FIFO_OVERFLOW is enabled RX will be terminated
 * if a RX FIFO overflow occurs. If \ref SL_RAIL_EVENT_RX_FIFO_OVERFLOW
 * is not enabled, data will be discarded until the overflow condition
 * is resolved. To continue capturing data RX must be restarted using
 * \ref sl_rail_start_rx().
 */
sl_rail_status_t sl_rail_config_rx_data(sl_rail_handle_t rail_handle,
                                        const sl_rail_rx_data_config_t *p_rx_data_config);

/**
 * Write data to the transmit FIFO previously established by sl_rail_set_tx_fifo().
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_data A pointer to transmit data.
 * @param[in] write_bytes A number of bytes to write to the transmit FIFO.
 * @param[in] reset If true, resets transmit FIFO before writing the data.
 * @return The number of bytes written to the transmit FIFO.
 *
 * This function copies write_bytes of data from the provided p_data into the
 * transmit FIFO previously established by \ref sl_rail_set_tx_fifo() or \ref sl_rail_init().
 * If the requested write_bytes exceeds the current number of bytes open
 * in the transmit FIFO, the function only writes until the transmit FIFO
 * is full. The function returns the number of bytes written to the transmit
 * FIFO or returns zero if rail_handle is NULL or if the transmit FIFO is full.
 *
 * @note The protocol's packet configuration, as set up by the radio
 *   configurator or via \ref sl_rail_set_fixed_length(), determines how many
 *   bytes of data are consumed from the transmit FIFO for a successful transmit
 *   operation, not the write_bytes value passed in. If not enough data has
 *   been put into the transmit FIFO, a \ref SL_RAIL_EVENT_TX_UNDERFLOW event will
 *   occur. If too much data is put into the transmit FIFO, the extra data will
 *   either become the first bytes
 *   sent in a subsequent packet, or will be thrown away if the FIFO gets
 *   reset prior to the next transmit. In general, the proper number of
 *   packet bytes to put into the transmit FIFO are all payload bytes except
 *   for any CRC bytes, which the packet configuration causes to be sent
 *   automatically.
 *
 * @note This function does not utilize a critical section but, depending on the
 *   application, calling it within a critical section could be appropriate.
 */
uint16_t sl_rail_write_tx_fifo(sl_rail_handle_t rail_handle,
                               const uint8_t *p_data,
                               uint16_t write_bytes,
                               bool reset);

/**
 * Set the address of the transmit FIFO, a circular buffer used for TX data,
 * possibly pre-populated with transmit data.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in,out] p_addr An appropriately-aligned (see \ref sl_rail_set_tx_fifo description)
 *   pointer to a read-write memory location in RAM used as the transmit FIFO. This memory
 *   must persist until the next call to this function.
 * @param[in] size_bytes The size of the transmit FIFO in bytes.
 *   See below for the range of sizes supported and non-power-of-2 limitations.
 * @param[in] init_bytes The number of valid bytes already populated in the transmit FIFO after start_offset.
 * @param[in] start_offset_bytes A number of bytes defining the start position of the TX data
 *   from the transmit FIFO p_addr, only valid if init_bytes is not 0.
 * @return Status code indicating success of the function call.
 *
 * This function sets the RAM memory area for the transmit FIFO.
 * It must be called at least once before any transmit operations occur,
 * unless a transmit FIFO was previously established by \ref sl_rail_init()
 * in its \ref sl_rail_config_t.
 *
 * The actual FIFO size is specified by the size_bytes argument.
 * It is recommended that size_bytes be a power-of-2 between \ref
 * SL_RAIL_MINIMUM_FIFO_BYTES and \ref SL_RAIL_MAXIMUM_FIFO_BYTES, inclusive,
 * so RAIL can treat the buffer as circular and support \ref
 * sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE operation.
 * If size_bytes is a non-power-of-2 it must be below \ref
 * SL_RAIL_MAXIMUM_FIFO_BYTES and is treated as a linear buffer
 * suitable for \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE
 * only.
 *
 * @note This is different than RAIL 2.x \ref RAIL_SetTxFifo() which
 *   disallowed transmit FIFOs smaller than \ref SL_RAIL_MINIMUM_FIFO_BYTES
 *   and would round down a non-power-of-2 size to the next-smaller power-of-2.
 *
 * Platforms have different FIFO alignment requirements. The \ref
 * sl_rail_fifo_buffer_align_t accurately reflects the platform's alignment
 * requirement, and the \ref SL_RAIL_DECLARE_FIFO_BUFFER() convenience macro
 * should be used to declare it. If the FIFO is improperly aligned, an
 * error is returned.
 *
 * Typically \ref sl_rail_write_tx_fifo() is used to write transmit data
 * to the FIFO after calling this function. However,
 * users may pre-populate transmit data directly in the FIFO's memory before
 * calling this function, in which case they must specify the start_offset
 * (allowing unaligned packet data in the aligned FIFO) and init_bytes
 * (relative to start_offset) of that data.
 * If the start_offset plus init_bytes is larger than the FIFOs actual size,
 * the data will be truncated to the FIFO's actual size.
 * Specifying a start_offset will not reduce the FIFO threshold or affect
 * \ref sl_rail_get_tx_fifo_space_available().
 *
 * @note The start_offset is essentially forgotten after the next transmit --
 *   it applies only to the next transmit operation, and is not re-established
 *   when the transmit FIFO is reset.
 *
 * In multiprotocol, each protocol should establish its own transmit FIFO;
 * it's not feasible to share a single transmit FIFO among multiple protocols.
 *
 * @note The protocol's packet configuration, as set up by the radio
 *   configurator or via \ref sl_rail_set_fixed_length(), determines how many
 *   bytes of data are consumed from the transmit FIFO for a successful transmit
 *   operation, not the init_bytes value passed in. If not enough data has
 *   been put into the transmit FIFO, a \ref SL_RAIL_EVENT_TX_UNDERFLOW event will
 *   occur. If too much data is put into the transmit FIFO, the extra data
 *   will either become the first bytes
 *   sent in a subsequent packet, or will be thrown away if the FIFO gets
 *   reset or re-established prior to the next transmit. In general, the
 *   proper number of packet bytes to put into the transmit FIFO are all
 *   payload bytes except for any CRC bytes which the packet configuration
 *   causes to be sent automatically.
 *
 * @note After this function is successfully called, any prior transmit FIFO
 *   is orphaned.
 */
sl_rail_status_t sl_rail_set_tx_fifo(sl_rail_handle_t rail_handle,
                                     sl_rail_fifo_buffer_align_t *p_addr,
                                     uint16_t size_bytes,
                                     uint16_t init_bytes,
                                     uint16_t start_offset_bytes);

/**
 * Set the address of the receive FIFO, a circular buffer used for receive data.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in,out] p_addr A pointer to a read-write memory location in RAM used as
 *   the receive FIFO. This memory must persist until the next call to this
 *   function.
 * @param[in,out] p_size_bytes A pointer to the desired size of the receive
 *   FIFO in bytes. This will be updated with the actual size during the
 *   function call.
 * @return Status code indicating success of the function call.
 *
 * This function sets the RAM memory area for the receive FIFO, a circular
 * buffer used to hold received data from one or more packets.
 * It must be called at least once before any receive operations occur,
 * unless a receive FIFO was previously established by \ref sl_rail_init()
 * in its \ref sl_rail_config_t.
 *
 * The actual FIFO size can be determined by the output p_size_bytes argument. The
 * chosen size is determined based on the available FIFO sizes supported by the
 * hardware and will be the closest allowed size less than or equal to the
 * requested size, unless the requested size is smaller than the minimum FIFO
 * size, in which case an error is returned.
 * For more on supported FIFO sizes and alignments, see chip-specific
 * documentation, such as \ref efr32_main.
 *
 * Platforms have different FIFO alignment requirements. The \ref
 * sl_rail_fifo_buffer_align_t accurately reflects the platform's alignment
 * requirement, and the \ref SL_RAIL_DECLARE_FIFO_BUFFER() convenience macro
 * should be used to declare it. If the FIFO is improperly aligned, an
 * error is returned.
 *
 * In multiprotocol, RAIL allows each protocol to have its own receive FIFO
 * (and receive packet queue) or to share one receive FIFO and receive Packet
 * Queue across multiple protocols.
 *
 * @note This should never be called while the radio is active nor from
 *   the \ref sl_rail_config_t::events_callback(). After it is called, any prior
 *   receive FIFO is orphaned.
 *
 * @note The receive FIFO works in conjunction with the receive Packet Queue
 *   that must be set up by \ref sl_rail_init() or \ref sl_rail_set_rx_packet_queue().
 */
sl_rail_status_t sl_rail_set_rx_fifo(sl_rail_handle_t rail_handle,
                                     sl_rail_fifo_buffer_align_t *p_addr,
                                     uint16_t *p_size_bytes);

/**
 * Compile-time size in bytes of the built-in receive FIFO buffer
 * to which \ref sl_rail_builtin_rx_fifo_ptr points.
 */
#define SL_RAIL_BUILTIN_RX_FIFO_BYTES 512

/**
 * Run-time size in bytes of the built-in receive FIFO buffer
 * to which \ref sl_rail_builtin_rx_fifo_ptr points.
 * Can be assigned to \ref sl_rail_config_t::rx_fifo_bytes.
 */
extern const uint16_t sl_rail_builtin_rx_fifo_bytes;

/**
 * Pointer to built-in receive FIFO buffer that can be shared among
 * built-in protocols on the same radio like RAIL 2.x provided internally.
 * Can be assigned to \ref sl_rail_config_t::p_rx_fifo_buffer.
 */
extern sl_rail_fifo_buffer_align_t * const sl_rail_builtin_rx_fifo_ptr;

/**
 * Compile-time number of entries in the built-in receive Packet Queue
 * to which \ref sl_rail_builtin_rx_packet_queue_ptr points.
 */
#define SL_RAIL_BUILTIN_RX_PACKET_QUEUE_ENTRIES 16

/**
 * Run-time number of entries in the built-in receive Packet Queue
 * to which \ref sl_rail_builtin_rx_packet_queue_ptr points.
 * Can be assigned to \ref sl_rail_config_t::rx_packet_queue_entries.
 */
extern const uint16_t sl_rail_builtin_rx_packet_queue_entries;

/**
 * Pointer to built-in receive Packet queue that can be shared among
 * built-in protocols on the same radio like RAIL 2.x provided internally.
 * Can be assigned to \ref sl_rail_config_t::p_rx_packet_queue.
 */
extern sl_rail_packet_queue_entry_t * const sl_rail_builtin_rx_packet_queue_ptr;

/**
 * Read packet data from RAIL's receive FIFO.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_data An application-provided pointer to store data.
 *   If NULL, the data is thrown away rather than copied out.
 * @param[in] read_bytes A number of packet bytes to read from the FIFO.
 * @return The number of packet bytes read from the receive FIFO.
 *
 * This function reads packet data from the head of receive FIFO and
 * writes it to the provided p_data. It does not permit reading more
 * data than is available in the FIFO, nor does it permit reading more
 * data than remains in the oldest unreleased packet.
 *
 * Because this function does not have a critical section, use it
 * only in one context or make sure function calls are protected to prevent
 * buffer corruption.
 *
 * @warning This function is intended for use only with \ref
 *   sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE and should never be called in \ref
 *   sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE where it could lead to receive FIFO
 *   corruption.
 *
 * @note When reading data from an arriving packet that is not yet complete,
 *   keep in mind its data is highly suspect because it has not yet passed
 *   any CRC integrity checking. Also note that the packet could be aborted,
 *   canceled, or fail momentarily, invalidating its data in Packet mode.
 *   Furthermore, there is a small chance towards the end of packet reception
 *   that the receive FIFO could include not only packet data received so far,
 *   but also some raw radio-appended info detail bytes that RAIL's
 *   packet-completion processing will subsequently deal with. It's up to the
 *   application to know its packet format well enough to avoid reading this
 *   info because it will corrupt the packet's details and possibly corrupt the
 *   receive FIFO.
 */
uint16_t sl_rail_read_rx_fifo(sl_rail_handle_t rail_handle,
                              uint8_t *p_data,
                              uint16_t read_bytes);

/**
 * Set the address of the receive Packet Queue, a circular buffer used by
 * RAIL for received packet metadata that supplements the receive FIFO.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_rx_packet_queue A pointer to a read-write memory location in RAM
 *   to use for the receive Packet Queue. This memory must persist until the
 *   next call to this function and must be able to hold at least the
 *   specified p_entries packets.
 * @param[in,out] p_entries The desired number of entries in the
 *   receive Packet Queue. This will be updated with the actual number of
 *   entries if the requested entries are not feasible but fewer are.
 * @return Status code indicating success of the function call.
 *
 * This function sets the RAM memory location for the receive Packet Queue.
 * The number of entries in this circular queue represents the number of
 * packets RAIL can queue to the application before \ref
 * SL_RAIL_EVENT_RX_FIFO_OVERFLOW might occur.
 *
 * This queue is used for per-packet metadata separate from actual packet
 * payload data placed in the receive FIFO buffer.
 * This function must be called at least once before any receive operations
 * occur if a receive Packet Queue was not provided at \ref sl_rail_init() time
 * via \ref sl_rail_config_t::p_rx_packet_queue.
 *
 * The actual number of entries can be determined by the output p_entries
 * argument. The chosen number is determined based on the available queue
 * entries supported by the radio and will be the closest allowed number
 * less than or equal to the requested number, unless the requested number
 * is smaller than the minimum number of queue entries, in which case an
 * error is returned.
 * For more on the supported number of receive Packet Queue entries see
 * chip-specific documentation, such as \ref efr32_main.
 *
 * @note This should never be called while the radio is active nor from
 *   the \ref sl_rail_config_t::events_callback(). After it is called, any prior
 *   receive Packet Queue is orphaned.
 *
 * @note If \ref sl_rail_reset_fifo() is called to reset the receive FIFO, the
 *   receive Packet Queue is also reset.
 */
sl_rail_status_t sl_rail_set_rx_packet_queue(sl_rail_handle_t rail_handle,
                                             sl_rail_packet_queue_entry_t *p_rx_packet_queue,
                                             uint16_t *p_entries);

/**
 * Configure the RAIL transmit FIFO almost empty threshold.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] tx_threshold_bytes The threshold below which the
 *   \ref SL_RAIL_EVENT_TX_FIFO_ALMOST_EMPTY event will fire.
 * @return Configured transmit FIFO threshold value.
 *
 * This function configures the threshold for the transmit FIFO. When the
 * number of bytes in the transmit FIFO falls below the configured threshold,
 * \ref sl_rail_config_t::events_callback() will fire with \ref
 * SL_RAIL_EVENT_TX_FIFO_ALMOST_EMPTY set.
 * The tx_threshold_bytes value should be smaller than or equal to the transmit
 * FIFO size; higher values will be pegged to the FIFO size.
 * A value of 0 or \ref SL_RAIL_FIFO_THRESHOLD_DISABLED will disable the
 * threshold, returning \ref SL_RAIL_FIFO_THRESHOLD_DISABLED.
 */
uint16_t sl_rail_set_tx_fifo_threshold(sl_rail_handle_t rail_handle,
                                       uint16_t tx_threshold_bytes);

/**
 * Configure the RAIL receive FIFO almost full threshold.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] rx_threshold_bytes The threshold above which the
 *   \ref SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL event will fire.
 * @return Configured receive FIFO threshold value.
 *
 * This function configures the threshold for the receive FIFO. When the
 * number of bytes of packet data in the receive FIFO exceeds the
 * configured threshold, \ref sl_rail_config_t::events_callback() will keep
 * firing with \ref SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL set as long as the
 * number of bytes in the receive FIFO exceeds the configured threshold
 * value. The rx_threshold_bytes value should be smaller than the receive FIFO
 * size; anything else, including a value of
 * \ref SL_RAIL_FIFO_THRESHOLD_DISABLED, will disable the threshold,
 * returning \ref SL_RAIL_FIFO_THRESHOLD_DISABLED.
 *
 * @note To avoid sticking in the event handler (even in idle state):
 *   1. Disable the event (via the config events API or the
 *      \ref SL_RAIL_FIFO_THRESHOLD_DISABLED parameter).
 *   2. Increase FIFO threshold.
 *   3. Read the FIFO (that's not an option in
 *      \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE) in the event handler.
 */
uint16_t sl_rail_set_rx_fifo_threshold(sl_rail_handle_t rail_handle,
                                       uint16_t rx_threshold_bytes);

/**
 * Get the RAIL transmit FIFO almost empty threshold value.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Configured TX Threshold value in bytes.
 *
 * Retrieves the configured TX threshold value.
 */
uint16_t sl_rail_get_tx_fifo_threshold(sl_rail_handle_t rail_handle);

/**
 * Get the RAIL receive FIFO almost full threshold value.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Configured RX Threshold value in bytes.
 *
 * Retrieves the configured RX threshold value.
 */
uint16_t sl_rail_get_rx_fifo_threshold(sl_rail_handle_t rail_handle);

/**
 * Reset the RAIL transmit and/or receive FIFOs.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] tx_fifo If true, reset the transmit FIFO.
 * @param[in] rx_fifo If true, reset the receive FIFO and its associated
 *   receive Packet Queue.
 * @return Status code indicating success of the function call.
 *
 * This function can reset each FIFO independently.
 * The application should not reset the receive FIFO while receiving a frame,
 * nor should it reset the transmit FIFO while transmitting a frame.
 */
sl_rail_status_t sl_rail_reset_fifo(sl_rail_handle_t rail_handle,
                                    bool tx_fifo, bool rx_fifo);

/**
 * Get the number of bytes used in the receive FIFO.
 * Only use this function in RX \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE.
 * Apps should use \ref sl_rail_get_rx_packet_info() instead.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Number of bytes used in the receive FIFO.
 *
 * This function indicates how much packet-related data exists in the receive
 * FIFO that could be read.
 *
 * @note The number of bytes returned may not just reflect the current
 *   packet's data but could also include raw appended info bytes added
 *   after successful packet reception and bytes from subsequently received
 *   packets. It is up to the app to never try to consume more than the
 *   packet's actual data when using the value returned here in a subsequent
 *   call to \ref sl_rail_read_rx_fifo(), otherwise the receive FIFO will be corrupted.
 */
uint16_t sl_rail_get_rx_fifo_bytes_available(sl_rail_handle_t rail_handle);

/**
 * Get the number of bytes unused in the transmit FIFO.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Number of bytes unused in the transmit FIFO.
 *
 * This function indicates how much space is available in the transmit FIFO
 * for writing additional packet data.
 */
uint16_t sl_rail_get_tx_fifo_space_available(sl_rail_handle_t rail_handle);

/** @} */ // end of group Data_Management

/******************************************************************************
 * State Transitions
 *****************************************************************************/
/**
 * @addtogroup State_Transitions State Transitions
 * @{
 */

/**
 * Configure RAIL automatic state transitions after RX.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_transitions A pointer to the state transitions to apply after reception.
 * @return Status code indicating success of the function call.
 *
 * This function fails if unsupported transitions are passed in or if the
 * radio is currently in the RX state. Success can transition to TX, RX, or
 * IDLE, while error can transition to RX or IDLE. The timings of state
 * transitions from the RX state are not guaranteed when packets are longer
 * than 16 seconds on-air.
 */
sl_rail_status_t sl_rail_set_rx_transitions(sl_rail_handle_t rail_handle,
                                            const sl_rail_state_transitions_t *p_transitions);

/**
 * Get the current RAIL automatic state transitions after RX.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_transitions A pointer to the state transitions that RAIL applies after receive.
 * @return Status code indicating a success of the function call.
 *
 * Retrieves the current state transitions after RX and stores them in the
 * transitions argument.
 */
sl_rail_status_t sl_rail_get_rx_transitions(sl_rail_handle_t rail_handle,
                                            sl_rail_state_transitions_t *p_transitions);

/**
 * Configure RAIL automatic state transitions after TX.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_transitions A pointer to the state transitions to apply after transmission.
 * @return Status code indicating a success of the function call.
 *
 * This function fails if unsupported transitions are passed in or if the
 * radio is currently in the TX state. Success and error can each transition
 * to RX or IDLE only, not TX. For the ability to run repeated transmits, see
 * \ref sl_rail_set_next_tx_repeat(). Calling this function will clear any repeated
 * transmissions set up by \ref sl_rail_set_next_tx_repeat().
 */
sl_rail_status_t sl_rail_set_tx_transitions(sl_rail_handle_t rail_handle,
                                            const sl_rail_state_transitions_t *p_transitions);

/**
 * Get the current RAIL automatic state transitions after TX.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_transitions A pointer to the state transitions that RAIL applies after transmission.
 * @return Status code indicating a success of the function call.
 *
 * Retrieves the current state transitions after TX and stores them in the
 * transitions argument.
 */
sl_rail_status_t sl_rail_get_tx_transitions(sl_rail_handle_t rail_handle,
                                            sl_rail_state_transitions_t *p_transitions);

/**
 * Set up automatic repeated transmits after the next transmit.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_repeat_config A pointer to the configuration structure for repeated transmits.
 * @return Status code indicating a success of the function call.
 *
 * Repeated transmits will occur after an application-initiated transmit caused
 * by calling one of the \ref Packet_TX APIs. The repetition will only occur
 * after the first application-initiated transmit after this function is
 * called. Future repeated transmits must be requested by calling this function
 * again.
 *
 * Each repeated transmit that occurs will have full \ref PTI information, and
 * will receive events such as \ref SL_RAIL_EVENT_TX_PACKET_SENT as normal.
 *
 * If a TX error occurs during the repetition, the process will abort and the
 * TX error transition from \ref sl_rail_set_tx_transitions() will be used. If the
 * repetition completes successfully, then the TX success transition from
 * \ref sl_rail_set_tx_transitions() will be used.
 *
 * Use \ref sl_rail_get_tx_packets_remaining() if need to know how many transmit
 * completion events are expected before the repeating sequence is done, or
 * how many were not performed due to a transmit error.
 *
 * Any call to \ref sl_rail_idle(), \ref sl_rail_stop_tx(), or \ref
 * sl_rail_set_tx_transitions() will clear the pending
 * repeated transmits. The state will also be cleared by another call to this
 * function. A DMP switch will clear this
 * state only if the initial transmit triggering the repeated transmits has
 * started.
 *
 * One can change the repeated transmit configuration by re-calling
 * this function with new parameters as long as that occurs prior to
 * calling a \ref Packet_TX API. Passing a \ref
 * sl_rail_tx_repeat_config_t::iterations count of 0 will prevent the next
 * transmit from repeating.
 *
 * The application is responsible for populating the transmit data to be used
 * by the repeated transmits via \ref sl_rail_set_tx_fifo() or \ref sl_rail_write_tx_fifo().
 * Data will be transmitted from the transmit FIFO. If the transmit FIFO does
 * not have sufficient data to transmit, a TX error will be caused and a \ref
 * SL_RAIL_EVENT_TX_UNDERFLOW will occur. In order to avoid an underflow, the
 * application should queue data to be transmitted as early as possible.
 * Consider using \ref SL_RAIL_TX_OPTION_RESEND if the same packet data is to
 * be repeated: then the transmit FIFO only needs to be set/written once.
 *
 * Do not call this function after starting a transmit operation via a \ref
 * Packet_TX API call or
 * before processing the final transmit completion event of a prior transmit.
 * This function will fail to (re)configure the repetition if a transmit of any
 * kind is ongoing, including during the time between an initial transmit and
 * the end of a previously-configured repetition.
 *
 * @note Use the compile time symbol \ref SL_RAIL_SUPPORTS_TX_TO_TX or the runtime
 *   call \ref sl_rail_supports_tx_to_tx() to check whether the platform supports
 *   this feature.
 */
sl_rail_status_t sl_rail_set_next_tx_repeat(sl_rail_handle_t rail_handle,
                                            const sl_rail_tx_repeat_config_t *p_repeat_config);

/**
 * Get the number of transmits remaining in a repeat operation.
 * Must only be called from within event callback context when handling
 * one of the \ref SL_RAIL_EVENTS_TX_COMPLETION events.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return transmits remaining as described below.
 *
 * If the TX completion event is \ref SL_RAIL_EVENT_TX_PACKET_SENT the
 * returned value indicates how many more such events are expected
 * before the repeat transmit operation is done. Due to interrupt
 * latency and timing, this may be an overcount if greater than 0
 * but is guaranteed to be accurate when 0.
 *
 * If the TX completion event is an error, the returned value indicates
 * the number of requested transmits that were not performed. For
 * \ref SL_RAIL_EVENT_TX_ABORTED and \ref SL_RAIL_EVENT_TX_UNDERFLOW the
 * count does not include the failing transmit itself. For the other
 * errors where a transmit never started or was blocked, the count
 * would include the failing transmit, which may be one higher than
 * the configured \ref sl_rail_tx_repeat_config_t::iterations if it was
 * the original transmit that was blocked.
 *
 * If an infinite repeat was configured, this will return \ref
 * SL_RAIL_TX_REPEAT_INFINITE_ITERATIONS.
 */
uint16_t sl_rail_get_tx_packets_remaining(sl_rail_handle_t rail_handle);

/**
 * Configure RAIL automatic state transition timing.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in,out] p_timings A pointer to the timings used to configure the RAIL state
 *   machine. This structure is updated with the actual times that were
 *   set, if an input timing is invalid.
 * @return Status code indicating a success of the function call.
 *
 * The timings given are close to the actual transition time. However,
 * a still uncharacterized software overhead occurs. Also, timings are not
 * always adhered to when using an automatic transition after an error, due to
 * the cleanup required to recover from the error.
 */
sl_rail_status_t sl_rail_set_state_timing(sl_rail_handle_t rail_handle,
                                          sl_rail_state_timing_t *p_timings);

/**
 * Place the radio into an idle state.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] mode The method for shutting down the radio.
 * @param[in] wait Whether this function should wait for the radio to reach
 *   idle before returning.
 * @return \ref SL_RAIL_STATUS_NO_ERROR if the radio has idled by the time this
 *   function returns, or \ref SL_RAIL_STATUS_SUSPENDED if it is still in the
 *   process of idling.
 *
 * This function is used to remove the radio from TX and RX states. How these
 * states are left is defined by the mode parameter.
 *
 * In multiprotocol, this API will also cause the radio to be yielded so that
 * other tasks can be run. See \ref rail_radio_scheduler_yield for more details.
 */
sl_rail_status_t sl_rail_idle(sl_rail_handle_t rail_handle,
                              sl_rail_idle_mode_t mode,
                              bool wait);

/**
 * Get the current radio state.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return An enumeration for the current radio state.
 *
 * Returns the state of the radio as a bitmask containing:
 * \ref SL_RAIL_RF_STATE_IDLE, \ref SL_RAIL_RF_STATE_RX, \ref SL_RAIL_RF_STATE_TX,
 * and \ref SL_RAIL_RF_STATE_ACTIVE. \ref SL_RAIL_RF_STATE_IDLE, \ref
 * SL_RAIL_RF_STATE_RX, and \ref SL_RAIL_RF_STATE_TX bits are mutually exclusive.
 * The radio can transition through intermediate states,
 * which are not reported but are instead considered part of the state
 * most closely associated. For example, when the radio is warming up
 * or shutting down the transmitter or receiver, this function returns
 * \ref SL_RAIL_RF_STATE_TX or \ref SL_RAIL_RF_STATE_RX, respectively.
 * When transitioning directly from RX to TX or vice-versa, this function
 * returns the earlier state.
 *
 * @note For a more detailed radio state, see \ref sl_rail_get_radio_state_detail().
 */
sl_rail_radio_state_t sl_rail_get_radio_state(sl_rail_handle_t rail_handle);

/**
 * Get the detailed current radio state.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return An enumeration for the current detailed radio state.
 *
 * Returns the state of the radio as a bitmask. The three core radio states
 * IDLE, RX, and TX are represented by mutually exclusive bits \ref
 * SL_RAIL_RF_STATE_DETAIL_IDLE_STATE, \ref SL_RAIL_RF_STATE_DETAIL_RX_STATE, and
 * \ref SL_RAIL_RF_STATE_DETAIL_TX_STATE respectively. If the radio is
 * transitioning between these three states, the returned bitmask will have
 * \ref SL_RAIL_RF_STATE_DETAIL_TRANSITION set along with a bit corresponding to
 * the destination core radio state. If, while in the receive state, the radio
 * is actively receiving a packet, \ref SL_RAIL_RF_STATE_DETAIL_ACTIVE will be set;
 * otherwise, this bit will be clear. If frame detection is disabled, \ref
 * SL_RAIL_RF_STATE_DETAIL_NO_FRAMES in the returned state bitmask will be set;
 * otherwise, this bit will be clear. If the radio is performing an LBT/CSMA
 * operation (e.g., a backoff period) \ref SL_RAIL_RF_STATE_DETAIL_LBT in the
 * returned state bitmask will be set; otherwise, this bit will be clear.
 *
 * For the most part, the more detailed radio states returned by this API
 * correspond to radio states returned by \ref sl_rail_get_radio_state() as follows:
 *
 *   \ref sl_rail_radio_state_detail_t           \ref sl_rail_radio_state_t
 *   SL_RAIL_RF_STATE_DETAIL_INACTIVE       SL_RAIL_RF_STATE_INACTIVE
 *   SL_RAIL_RF_STATE_DETAIL_IDLE_STATE
 *   | SL_RAIL_STATE_DETAIL_TRANSITION      If RX overflow or leaving RX unforced:
 *                                            SL_RAIL_RF_STATE_RX
 *                                          Else if leaving TX unforced:
 *                                            SL_RAIL_RF_STATE_TX
 *                                          Else:
 *                                            SL_RAIL_RF_STATE_IDLE
 *   SL_RAIL_RF_STATE_DETAIL_IDLE_STATE     SL_RAIL_RF_STATE_IDLE
 *   SL_RAIL_RF_STATE_DETAIL_IDLE_STATE
 *   | SL_RAIL_STATE_DETAIL_LBT             SL_RAIL_RF_STATE_TX
 *   SL_RAIL_RF_STATE_DETAIL_RX_STATE
 *   | SL_RAIL_STATE_DETAIL_TRANSITION      If leaving TX:
 *                                            SL_RAIL_RF_STATE_TX
 *                                          Else:
 *                                            SL_RAIL_RF_STATE_RX
 *   SL_RAIL_RF_STATE_DETAIL_RX_STATE
 *   | SL_RAIL_RF_STATE_DETAIL_TRANSITION
 *   | SL_RAIL_RF_STATE_DETAIL_NO_FRAMES    If leaving TX:
 *                                            SL_RAIL_RF_STATE_TX
 *                                          Else:
 *                                            SL_RAIL_RF_STATE_RX
 *   SL_RAIL_RF_STATE_DETAIL_RX_STATE       SL_RAIL_RF_STATE_RX
 *   SL_RAIL_RF_STATE_DETAIL_RX_STATE
 *   | SL_RAIL_RF_STATE_DETAIL_NO_FRAMES    SL_RAIL_RF_STATE_RX
 *   SL_RAIL_RF_STATE_DETAIL_RX_STATE
 *   | SL_RAIL_RF_STATE_DETAIL_LBT          SL_RAIL_RF_STATE_RX
 *   SL_RAIL_RF_STATE_DETAIL_RX_STATE
 *   | SL_RAIL_RF_STATE_DETAIL_NO_FRAMES
 *   | SL_RAIL_RF_STATE_DETAIL_LBT          SL_RAIL_RF_STATE_RX
 *   SL_RAIL_RF_STATE_DETAIL_RX_STATE
 *   | SL_RAIL_RF_STATE_DETAIL_ACTIVE       SL_RAIL_RF_STATE_RX_ACTIVE
 *   SL_RAIL_RF_STATE_DETAIL_TX_STATE
 *   | SL_RAIL_RF_STATE_TRANSITION          If leaving RX:
 *                                            SL_RAIL_RF_STATE_RX
 *                                          Else:
 *                                            SL_RAIL_RF_STATE_TX
 *   SL_RAIL_RF_STATE_DETAIL_TX_STATE
 *   | SL_RAIL_RF_STATE_ACTIVE              SL_RAIL_RF_STATE_TX_ACTIVE
 */
sl_rail_radio_state_detail_t sl_rail_get_radio_state_detail(sl_rail_handle_t rail_handle);

/**
 * Enable/disable caching of synth calibration value.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] enable A boolean to enable or disable caching of synth calibration.
 * @return Status code indicating success of the function call.
 *
 * Once enabled, the sequencer will start caching synth calibration values for
 * channels and apply them instead of performing calibration on every state
 * transition and channel change.
 * This will increase the transition time for the first time calibration is
 * performed. Subsequent state transitions will be faster. The cache size is 2.
 * \ref SL_RAIL_IEEE802154_SUPPORTS_RX_CHANNEL_SWITCHING internally uses this feature
 * and there is no need to enable/disable it. This function returns
 * \ref SL_RAIL_STATUS_INVALID_STATE if we try to disable it while
 * \ref SL_RAIL_IEEE802154_SUPPORTS_RX_CHANNEL_SWITCHING is enabled.
 *
 * @note This function will improve the minimum timings that can be achieved in
 *   \ref sl_rail_state_timing_t::idle_to_rx, \ref sl_rail_state_timing_t::idle_to_tx,
 *   \ref sl_rail_state_timing_t::rx_to_tx, \ref sl_rail_state_timing_t::tx_to_rx and
 *   \ref sl_rail_state_timing_t::tx_to_tx. A call to \ref sl_rail_set_state_timing()
 *   is needed to achieve lower transition times.
 *
 * @note On a protocol switch the cache is cleared, so it is not suitable for
 *   applications where a protocol switch happens frequently, like with
 *   Dynamic Multiprotocol.
 */
sl_rail_status_t sl_rail_enable_cache_synth_cal(sl_rail_handle_t rail_handle,
                                                bool enable);

/** @} */ // end of group State_Transitions

/******************************************************************************
 * Transmit
 *****************************************************************************/
/**
 * @addtogroup Transmit
 * @brief APIs related to transmitting data packets
 * @{
 */

/// @addtogroup PA Power Amplifier (PA)
/// @brief APIs for interacting with one of the on chip PAs.
///
/// These APIs let you configure the on-chip PA to get the desired deci-dBm
/// output power. Applications should only interact with RAIL using dBm power
/// units (or fraction thereof). The RAIL library requires an implementation
/// of \ref sl_railcb_convert_ddbm_to_power_setting_entry() for performing the
/// conversion of dBm to a chip-supported PA (\ref sl_rail_tx_power_mode_t)
/// and raw power level.
///
/// Refer to the \ref sl_rail_util_pa plugin for an implementation of that
/// callback and information on how PA conversion tables/curves are
/// established to support such conversion.
///
/// @{

#ifndef SLI_LIBRAIL_ALIAS

/**
 * Initialize TX power settings (provided primarily for compatibility with
 * the older RAIL 2.x \ref rail_util_pa power curve plugin).
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_config A pointer to a power config with the desired initial settings
 *   for the TX amplifier.
 * @return Status code indicating success of the function call.
 *
 * This function is generally unnecessary when using the \ref sl_rail_util_pa
 * plugin. It is primarily for backwards compatibility usage with the older
 * \ref rail_util_pa plugin, or possibly of use for performing PA RF path
 * characterization to generate or adjust the power conversion tables/curves
 * provided by Silicon Labs in \ref sl_rail_util_pa / \ref rail_util_pa
 * plugins.
 *
 * When using \ref sl_rail_util_pa, the PA ramp time and voltage can be set
 * via \ref sl_rail_set_tx_pa_ramp_time() and \ref sl_rail_set_tx_pa_voltage(),
 * respectively.
 *
 * @note A change in \ref sl_rail_tx_power_config_t::ramp_time_us may affect
 *   the minimum timings that can be achieved in \ref
 *   sl_rail_state_timing_t::idle_to_tx and \ref
 *   sl_rail_state_timing_t::rx_to_tx. Call \ref sl_rail_set_state_timing()
 *   again to check whether these times have changed.
 */
sl_rail_status_t sl_rail_config_tx_power(sl_rail_handle_t rail_handle,
                                         const sl_rail_tx_power_config_t *p_config);

#ifndef DOXYGEN_UNDOCUMENTED

/**
 * Get the TX power settings currently used in the amplifier.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_config A non-NULL pointer to a \ref
 *   sl_rail_tx_power_config_t structure filled in by the function.
 * @return Status code indicating success of the function call.
 *
 * Note that this API does not return the current TX power, which is separately
 * managed by the \ref sl_rail_get_tx_power_dbm() / \ref sl_rail_set_tx_power_dbm()
 * APIs. Use this API to determine which values were set as a result of
 * \ref sl_rail_config_tx_power().
 *
 * @warning This function is for PA RF path characterization use only.
 */
sl_rail_status_t sli_rail_get_tx_power_config(sl_rail_handle_t rail_handle,
                                              sl_rail_tx_power_config_t *p_config);

/**
 * Set the platform-and-PA-specific raw power level in the current PA.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] power_level Power in radio-specific \ref sli_rail_tx_power_level_t units.
 * @return Status code indicating success of the function call.
 *
 * @warning This function is for PA RF path characterization use only.
 */
sl_rail_status_t sli_rail_set_tx_power(sl_rail_handle_t rail_handle,
                                       sli_rail_tx_power_level_t power_level);

/**
 * Return the current platform-and-PA-specific raw power level of the PA.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The radio-specific \ref sli_rail_tx_power_level_t value of the current
 *   transmit power.
 *
 * @warning This function is for PA RF path characterization use only.
 */
sli_rail_tx_power_level_t sli_rail_get_tx_power(sl_rail_handle_t rail_handle);

/**
 * Get the TX PA power setting, which is used to configure power configurations
 * when the dBm to paPowerSetting mapping table mode is used.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The current PA power setting.
 */
sl_rail_pa_power_setting_t sli_rail_get_pa_power_setting(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports a particular power mode (PA) and
 * provides the maximum and minimum power level for that power mode
 * if supported by the chip.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in,out] p_power_mode A pointer to PA power mode to check if supported.
 *   If \ref SL_RAIL_TX_POWER_MODE_2P4_GHZ_HIGHEST or \ref
 *   SL_RAIL_TX_POWER_MODE_SUB_GHZ_HIGHEST is passed in, it will be updated
 *   to the highest corresponding PA available on the chip.
 * @param[out] p_max_power_level A pointer to a sli_rail_tx_power_level_t that
 *   if non-NULL will be filled in with the power mode's highest power level
 *   allowed if this function returns true.
 * @param[out] p_min_power_level A pointer to a sli_rail_tx_power_level_t that
 *   if non-NULL will be filled in with the power mode's lowest power level
 *   allowed if this function returns true.
 * @return true if *p_power_mode is supported; false otherwise.
 *
 * This function has no compile-time equivalent.
 */
bool sli_rail_supports_tx_power_mode(sl_rail_handle_t rail_handle,
                                     sl_rail_tx_power_mode_t *p_power_mode,
                                     sli_rail_tx_power_level_t *p_max_power_level,
                                     sli_rail_tx_power_level_t *p_min_power_level);

#endif//DOXYGEN_UNDOCUMENTED

///
/// Set the TX power in deci-dBm for all supported PA modes.
///
/// @param[in] rail_handle A real RAIL instance handle.
/// @param[in] power_ddbm A desired deci-dBm power to be set.
/// @return Status code indicating success of the function call.
///
/// This function sets the TX power for use with all supported PA modes.
/// If you want different power settings for any particular PA mode(s)
/// use \ref sl_rail_set_tx_power_dbm_with_pa_mode() instead.
///
/// @note RAIL will try its best to effect the requested TX power but it
///   may end up being truncated, rounded, or limited based on the PA mode's
///   PA power conversion tables/curves limits or granularity. One can use
///   \ref sl_rail_util_pa_convert_power_to_actual() to find the actual
///   power utilized, however that power is further subject to any channel
///   power limit specified in the channel configuration (\ref
///   sl_rail_channel_config_entry_t::max_power_ddbm).
///
///
sl_rail_status_t sl_rail_set_tx_power_dbm(sl_rail_handle_t rail_handle,
                                          sl_rail_tx_power_t power_ddbm);

///
/// Set the TX power in deci-dBm for just the specified PA mode.
///
/// @param[in] rail_handle A RAIL instance handle.
/// @param[in] power_ddbm The desired output power in deci-dBm.
/// @param[in] pa_mode The PA mode to use (platform-specific,
//  e.g., SL_RAIL_TX_PA_MODE_SUB_GHZ, SL_RAIL_TX_PA_MODE_SUB_GHZ_OFDM,
//  SL_RAIL_TX_PA_MODE_2P4_GHZ).
/// @return Status code indicating result of the function call.
///
/// This function sets the TX power for use with an explicit PA mode.
///
/// @note RAIL will try its best to effect the requested TX power but it
///   may end up being truncated, rounded, or limited based on the PA mode's
///   PA power conversion tables/curves limits or granularity. One can use
///   \ref sl_rail_util_pa_convert_power_to_actual() to find the actual
///   power utilized, however that power is further subject to any channel
///   power limit specified in the channel configuration (\ref
///   sl_rail_channel_config_entry_t::max_power_ddbm).
///
sl_rail_status_t sl_rail_set_tx_power_dbm_with_pa_mode(sl_rail_handle_t rail_handle,
                                                       sl_rail_tx_power_t power_ddbm,
                                                       sl_rail_tx_pa_mode_t pa_mode);

///
/// Get the TX power in deci-dBm.
///
/// @param[in] rail_handle A real RAIL instance handle.
/// @return The actual current output power in deci-dBm.
///
sl_rail_tx_power_t sl_rail_get_tx_power_dbm(sl_rail_handle_t rail_handle);

#endif//SLI_LIBRAIL_ALIAS

/**
 * @struct sl_rail_tx_power_table_config
 * Forward structure declaration of \ref sl_rail_tx_power_table_config_t.
 */
struct sl_rail_tx_power_table_config;

/**
 * Verify the TX Power Conversion configuration on modules.
 *
 * @param[in] p_config A pointer to TX Power Table to use on this module.
 * @return Status code indicating success of function call.
 *
 * This function only needs to be called when using a module and has no
 * effect otherwise. Transmit will not work before this function is called.
 */
sl_rail_status_t sl_rail_verify_tx_power_conversion(const struct sl_rail_tx_power_table_config *p_config);

/**
 * Gets the current PA mode from the state.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The PA mode.
 */
sl_rail_tx_pa_mode_t sl_rail_get_pa_mode(sl_rail_handle_t rail_handle);

/**
 * Gets the PA mode using the current channel config entry.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The PA mode.
 */
sl_rail_tx_pa_mode_t sl_rail_get_pa_mode_from_channel_entry(sl_rail_handle_t rail_handle);

/**
 * Sets the PA ramp time for transmission.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] ramp_time_us Desired ramp time in units of microseconds.
 * @return Status code indicating success of the function call.
 *
 * @note The ramp_time_us may affect the minimum timings that can be achieved
 *   in \ref sl_rail_state_timing_t::idle_to_tx and \ref
 *   sl_rail_state_timing_t::rx_to_tx. Call \ref sl_rail_set_state_timing()
 *   again to check whether these times have changed.
 */
sl_rail_status_t sl_rail_set_tx_pa_ramp_time(sl_rail_handle_t rail_handle,
                                             uint16_t ramp_time_us);

/**
 * Sets the PA voltage for transmission.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] voltage_mv Desired voltage in units of millivolts.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_set_tx_pa_voltage(sl_rail_handle_t rail_handle,
                                           uint16_t voltage_mv);

/**
 * Convert the desired decibel value (in units of deci-dBm)
 * to a power setting value that describes the configuration of
 * TX amplifier registers.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] power_ddbm Desired dBm value in units of deci-dBm.
 * @param[in] pa_mode PA mode used for this conversion.
 * @param[in] channel_restr_max_power_ddbm The maximum power(in deci-dBm) allowed on the channel
 * @param[out] p_power_setting_info A non-NULL pointer to memory allocated to hold
 *   the result of the call in \ref sl_rail_tx_power_setting_entry_t.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_railcb_convert_ddbm_to_power_setting_entry(sl_rail_handle_t rail_handle,
                                                               sl_rail_tx_power_t power_ddbm,
                                                               sl_rail_tx_pa_mode_t pa_mode,
                                                               sl_rail_tx_power_t channel_restr_max_power_ddbm,
                                                               sl_rail_tx_power_setting_entry_t *p_power_setting_info);

/** @} */ // end of group PA

/// @addtogroup Packet_TX Packet Transmit
/// @brief APIs which initiate a packet transmission in RAIL.
///
/// When using any of these functions, the data to be transmitted must have been
/// previously written to the transmit FIFO via \ref sl_rail_set_tx_fifo() and/or
/// \ref sl_rail_write_tx_fifo().
///
/// @{

/**
 * Start a transmit.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] channel Define the channel to transmit on.
 * @param[in] tx_options TX options to be applied to this transmit only.
 * @param[in] p_scheduler_info A pointer to information to allow the radio scheduler to place
 *   this transmit appropriately. This is only used in multiprotocol version of
 *   RAIL and may be set to NULL in all other versions.
 * @return Status code indicating success of the function call. If successfully
 *   initiated, transmit completion or failure will be reported by a later
 *   \ref sl_rail_config_t::events_callback() with the appropriate \ref sl_rail_events_t.
 *
 * The transmit process will begin immediately or as soon as a packet being
 * received has finished. The data to be transmitted must have been previously
 * established via \ref sl_rail_set_tx_fifo() and/or \ref sl_rail_write_tx_fifo().
 *
 * Returns an error if a previous transmit is still in progress.
 * If changing channels, any ongoing packet reception is aborted.
 *
 * Returns an error if no receive FIFO or Packet Queue have been configured
 * and either of the TX state transitions are to \ref SL_RAIL_RF_STATE_RX.
 *
 * In multiprotocol, ensure that the radio is properly yielded after this
 * operation completes. See \ref rail_radio_scheduler_yield for more details.
 */
sl_rail_status_t sl_rail_start_tx(sl_rail_handle_t rail_handle,
                                  uint16_t channel,
                                  sl_rail_tx_options_t tx_options,
                                  const sl_rail_scheduler_info_t *p_scheduler_info);

/**
 * Schedule sending a packet.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] channel The channel to transmit on.
 * @param[in] tx_options TX options to be applied to this transmit only.
 * @param[in] p_scheduled_tx_config A pointer to the \ref sl_rail_scheduled_tx_config_t
 *   structure indicating when and how the transmit should occur.
 * @param[in] p_scheduler_info A pointer to information to allow the radio scheduler to place
 *   this transmit appropriately. This is only used in multiprotocol version of
 *   RAIL and may be set to NULL in all other versions.
 * @return Status code indicating success of the function call. If successfully
 *   initiated, a transmit completion or failure will be reported by a later
 *   \ref sl_rail_config_t::events_callback() with the appropriate \ref sl_rail_events_t.
 *
 * The transmit process will begin at the scheduled time. The data to be
 * transmitted must have been previously established via \ref sl_rail_set_tx_fifo()
 * and/or \ref sl_rail_write_tx_fifo().
 * The time (in microseconds) and whether that time is absolute or
 * relative is specified using the \ref sl_rail_scheduled_tx_config_t structure.
 * What to do if a scheduled transmit fires in
 * the middle of receiving a packet is also specified in this structure.
 *
 * Returns an error if a previous transmit is still in progress.
 * If changing channels, the channel is changed immediately and
 * will abort any ongoing packet reception.
 *
 * Returns an error if no receive FIFO or Packet Queue have been configured
 * and either of the TX state transitions are to \ref SL_RAIL_RF_STATE_RX.
 *
 * In multiprotocol, ensure that the radio is properly yielded after this
 * operation completes. See \ref rail_radio_scheduler_yield for more details.
 */
sl_rail_status_t sl_rail_start_scheduled_tx(sl_rail_handle_t rail_handle,
                                            uint16_t channel,
                                            sl_rail_tx_options_t tx_options,
                                            const sl_rail_scheduled_tx_config_t *p_scheduled_tx_config,
                                            const sl_rail_scheduler_info_t *p_scheduler_info);

/**
 * Start a transmit using CSMA.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] channel The channel to transmit on.
 * @param[in] tx_options TX options to be applied to this transmit only.
 * @param[in] p_csma_config A pointer to the sl_rail_csma_config_t structure
 *   describing the CSMA parameters to use for this transmit.
 *   \n In multiprotocol this must point to global or heap storage that remains
 *   valid after the API returns until the transmit is actually started.
 * @param[in] p_scheduler_info A pointer to information to allow the radio scheduler to place
 *   this transmit appropriately. This is only used in multiprotocol version of
 *   RAIL and may be set to NULL in all other versions.
 * @return Status code indicating success of the function call. If successfully
 *   initiated, a transmit completion or failure will be reported by a later
 *   \ref sl_rail_config_t::events_callback() with the appropriate \ref sl_rail_events_t.
 *
 * Perform the Carrier Sense Multiple Access (CSMA) algorithm, and if
 * the channel is deemed clear (RSSI below the specified threshold), it will
 * commence transmission. The data to be transmitted must have been previously
 * established via \ref sl_rail_set_tx_fifo() and/or \ref sl_rail_write_tx_fifo().
 * Packets can be received during CSMA backoff periods if receive is active
 * throughout the CSMA process. This will happen either by starting the CSMA
 * process while receive is already active, or if the \ref
 * sl_rail_csma_config_t::cca_backoff_us is less than \ref sl_rail_state_timing_t::idle_to_rx
 * time. If the \ref sl_rail_csma_config_t::cca_backoff_us time is greater than the
 * \ref sl_rail_state_timing_t::idle_to_rx time, receive will only be active during
 * CSMA's clear channel assessments.
 *
 * If the CSMA algorithm deems the channel busy, the \ref sl_rail_config_t::events_callback()
 * occurs with \ref SL_RAIL_EVENT_TX_CHANNEL_BUSY, and the contents
 * of the transmit FIFO remain intact.
 *
 * Returns an error if a previous transmit is still in progress.
 * If changing channels, the channel is changed immediately and any ongoing
 * packet reception is aborted.
 *
 * Returns an error if a scheduled RX is still in progress, or if no
 * receive FIFO or Packet Queue have been configured.
 *
 * In multiprotocol, ensure that the radio is properly yielded after this
 * operation completes. See \ref rail_radio_scheduler_yield for more details.
 */
sl_rail_status_t sl_rail_start_cca_csma_tx(sl_rail_handle_t rail_handle,
                                           uint16_t channel,
                                           sl_rail_tx_options_t tx_options,
                                           const sl_rail_csma_config_t *p_csma_config,
                                           const sl_rail_scheduler_info_t *p_scheduler_info);

/**
 * Start a transmit using LBT.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] channel The channel to transmit on.
 * @param[in] tx_options TX options to be applied to this transmit only.
 * @param[in] p_lbt_config A pointer to the \ref sl_rail_lbt_config_t structure
 *   describing the LBT parameters to use for this transmit.
 *   \n In multiprotocol this must point to global or heap storage that remains
 *   valid after the API returns until the transmit is actually started.
 * @param[in] p_scheduler_info A pointer to information to allow the radio scheduler to place
 *   this transmit appropriately. This is only used in multiprotocol version of
 *   RAIL and may be set to NULL in all other versions.
 * @return Status code indicating success of the function call. If successfully
 *   initiated, a transmit completion or failure will be reported by a later
 *   \ref sl_rail_config_t::events_callback() with the appropriate \ref sl_rail_events_t.
 *
 * Performs the Listen Before Talk (LBT) algorithm, and if the channel
 * is deemed clear (RSSI below the specified threshold), it will commence
 * transmission. The data to be transmitted must have been previously established
 * via \ref sl_rail_set_tx_fifo() and/or \ref sl_rail_write_tx_fifo().
 * Packets can be received during LBT backoff periods if receive is active
 * throughout the LBT process. This will happen either by starting the LBT
 * process while receive is already active, or if the \ref
 * sl_rail_lbt_config_t::lbt_backoff_us is less than \ref sl_rail_state_timing_t::idle_to_rx
 * time. If the \ref sl_rail_lbt_config_t::lbt_backoff_us time is greater than the
 * \ref sl_rail_state_timing_t::idle_to_rx time, receive will only be active during
 * LBT's clear channel assessments.
 *
 * If the LBT algorithm deems the channel busy, the \ref sl_rail_config_t::events_callback() occurs with
 * \ref SL_RAIL_EVENT_TX_CHANNEL_BUSY, and the contents
 * of the transmit FIFO remain intact.
 *
 * Returns an error if a previous transmit is still in progress.
 * If changing channels, the channel is changed immediately and any ongoing
 * packet reception is aborted.
 *
 * Returns an error if a scheduled RX is still in progress, or if no
 * receive FIFO or Packet Queue have been configured.
 *
 * In multiprotocol, ensure that the radio is properly yielded after this
 * operation completes. See \ref rail_radio_scheduler_yield for more details.
 */
sl_rail_status_t sl_rail_start_cca_lbt_tx(sl_rail_handle_t rail_handle,
                                          uint16_t channel,
                                          sl_rail_tx_options_t tx_options,
                                          const sl_rail_lbt_config_t *p_lbt_config,
                                          const sl_rail_scheduler_info_t *p_scheduler_info);

/**
 * Schedule a transmit using CSMA.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] channel The channel to transmit on.
 * @param[in] tx_options TX options to be applied to this transmit only.
 * @param[in] p_scheduled_tx_config A pointer to the \ref sl_rail_scheduled_tx_config_t
 *   structure describing the CSMA parameters to use for this transmit.
 * @param[in] p_csma_config A pointer to the \ref sl_rail_csma_config_t structure
 *   describing the CSMA parameters to use for this transmit.
 *   \n In multiprotocol this must point to global or heap storage that remains
 *   valid after the API returns until the transmit is actually started.
 * @param[in] p_scheduler_info A pointer to information to allow the radio scheduler to place
 *   this transmit appropriately. This is only used in multiprotocol version of
 *   RAIL and may be set to NULL in all other versions.
 * @return Status code indicating success of the function call. If successfully
 *   initiated, a transmit completion or failure will be reported by a later
 *   \ref sl_rail_config_t::events_callback() with the appropriate \ref sl_rail_events_t.
 *
 * Internally, the RAIL library needs a PRS channel for this feature.
 * It will allocate an available PRS channel to use and hold onto that
 * channel for future use. If no PRS channel is available, the function
 * returns with \ref SL_RAIL_STATUS_INVALID_CALL.
 *
 * This is a scheduled variation of \ref sl_rail_start_cca_csma_tx(); refer to
 * that function for CSMA details.
 *
 * Returns an error if a previous transmit is still in progress.
 * If changing channels, the channel is changed immediately and any ongoing
 * packet reception is aborted.
 *
 * Returns an error if a scheduled RX is still in progress, or if no
 * receive FIFO or Packet Queue have been configured.
 *
 * In multiprotocol, ensure that the radio is properly yielded after this
 * operation completes. See \ref rail_radio_scheduler_yield for more details.
 */
sl_rail_status_t sl_rail_start_scheduled_cca_csma_tx(sl_rail_handle_t rail_handle,
                                                     uint16_t channel,
                                                     sl_rail_tx_options_t tx_options,
                                                     const sl_rail_scheduled_tx_config_t *p_scheduled_tx_config,
                                                     const sl_rail_csma_config_t *p_csma_config,
                                                     const sl_rail_scheduler_info_t *p_scheduler_info);

/**
 * Schedule a transmit using LBT.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] channel The channel to transmit on.
 * @param[in] tx_options TX options to be applied to this transmit only.
 * @param[in] p_scheduled_tx_config A pointer to the \ref sl_rail_scheduled_tx_config_t
 *   structure describing the CSMA parameters to use for this transmit.
 * @param[in] p_lbt_config A pointer to the \ref sl_rail_lbt_config_t structure
 *   describing the LBT parameters to use for this transmit.
 *   \n In multiprotocol this must point to global or heap storage that remains
 *   valid after the API returns until the transmit is actually started.
 * @param[in] p_scheduler_info A pointer to information to allow the radio scheduler to place
 *   this transmit appropriately. This is only used in multiprotocol version of
 *   RAIL and may be set to NULL in all other versions.
 * @return Status code indicating success of the function call. If successfully
 *   initiated, a transmit completion or failure will be reported by a later
 *   \ref sl_rail_config_t::events_callback() with the appropriate \ref sl_rail_events_t.
 *
 * Internally, the RAIL library needs a PRS channel for this feature.
 * It will allocate an available PRS channel to use and hold onto that
 * channel for future use. If no PRS channel is available, the function
 * returns with \ref SL_RAIL_STATUS_INVALID_CALL.
 *
 * This is a scheduled variation of \ref sl_rail_start_cca_lbt_tx(); refer to
 * that function for LBT details.
 *
 * Returns an error if a previous transmit is still in progress.
 * If changing channels, the channel is changed immediately and any ongoing
 * packet reception is aborted.
 *
 * Returns an error if a scheduled RX is still in progress, or if no
 * receive FIFO or Packet Queue have been configured.
 *
 * In multiprotocol, ensure that the radio is properly yielded after this
 * operation completes. See \ref rail_radio_scheduler_yield for more details.
 */
sl_rail_status_t sl_rail_start_scheduled_cca_lbt_tx(sl_rail_handle_t rail_handle,
                                                    uint16_t channel,
                                                    sl_rail_tx_options_t tx_options,
                                                    const sl_rail_scheduled_tx_config_t *p_scheduled_tx_config,
                                                    const sl_rail_lbt_config_t *p_lbt_config,
                                                    const sl_rail_scheduler_info_t *p_scheduler_info);

/**
 * Check whether next CCA is imminent. Valid only when handing \ref
 * SL_RAIL_EVENT_TX_CCA_RETRY. Typically used for coexistence signalling.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return true if the next CCA is imminent (zero backoff).
 */
bool sl_rail_is_next_cca_now(sl_rail_handle_t rail_handle);

/** @} */ // end of group Packet_TX

/**
 * Stop an active or pending transmit.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] mode Specifies the type(s) of transmit to stop.
 * @return \ref SL_RAIL_STATUS_NO_ERROR if the transmit was successfully
 *   stopped or \ref SL_RAIL_STATUS_INVALID_STATE if there is no transmit
 *   operation to stop.
 *
 * @note When mode includes \ref SL_RAIL_STOP_MODE_ACTIVE, this can also stop
 *   an active Auto-Ack transmit. When an active transmit is stopped, \ref
 *   SL_RAIL_EVENT_TX_ABORTED or \ref SL_RAIL_EVENT_TXACK_ABORTED should occur.
 *   When mode includes \ref SL_RAIL_STOP_MODE_PENDING this can also stop
 *   a \ref SL_RAIL_TX_OPTION_CCA_ONLY transmit operation. When a pending
 *   transmit is stopped, \ref SL_RAIL_EVENT_TX_BLOCKED should occur.
 */
sl_rail_status_t sl_rail_stop_tx(sl_rail_handle_t rail_handle,
                                 sl_rail_stop_mode_t mode);

/**
 * Set the CCA threshold in dBm.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] cca_threshold_dbm The CCA threshold in dBm.
 * @return Status code indicating success of the function call.
 *
 * Unlike \ref sl_rail_start_cca_csma_tx() or \ref sl_rail_start_cca_lbt_tx(), which can cause a
 * transmit, this function only modifies the CCA threshold. A possible
 * use case for this function involves setting the CCA threshold to invalid RSSI
 * of -128 which blocks transmission by preventing clear channel assessments
 * from succeeding.
 */
sl_rail_status_t sl_rail_set_cca_threshold(sl_rail_handle_t rail_handle,
                                           int8_t cca_threshold_dbm);

/**
 * Get detailed information about the last packet transmitted.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in,out] p_packet_details An application-provided pointer to store
 *   \ref sl_rail_tx_packet_details_t corresponding to the transmit event.
 *   The is_ack field must be initialized prior to each call:
 *   - is_ack true to obtain details about the most recent Ack transmit,
 *     false to obtain details about the most recent app-initiated transmit.
 *   The time_sent field packet_time will be populated with a time stamp
 *   corresponding to a default location in the packet. The time_sent field
 *   time_position will be populated with a \ref sl_rail_packet_time_position_t value
 *   specifying that default packet location.
 *   Call \ref sl_rail_get_tx_time_preamble_start(), \ref sl_rail_get_tx_time_sync_word_end(),
 *   or \ref sl_rail_get_tx_time_frame_end() to
 *   adjust the time stamp for different locations in the packet.
 * @return \ref SL_RAIL_STATUS_NO_ERROR if p_packet_details was filled in,
 *   or an appropriate error code otherwise.
 *
 * This function can only be called from callback context for either
 * \ref SL_RAIL_EVENT_TX_PACKET_SENT or \ref SL_RAIL_EVENT_TXACK_PACKET_SENT
 * events.
 */
sl_rail_status_t sl_rail_get_tx_packet_details(sl_rail_handle_t rail_handle,
                                               sl_rail_tx_packet_details_t *p_packet_details);

/**
 * Adjust a RAIL TX completion time stamp to refer to the start of the
 * preamble. Also used to retrieve the \ref SL_RAIL_EVENT_TX_STARTED
 * time stamp.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in,out] p_packet_details A non-NULL pointer to the details that were returned from
 *   a previous call to \ref sl_rail_get_tx_packet_details() for this same packet.
 *   The application must update the time_sent field total_packet_bytes to be
 *   the total number of bytes of the sent packet for RAIL to use when
 *   calculating the specified time stamp. This should account for all bytes
 *   transmitted over the air after the Preamble and Sync word(s), including CRC
 *   bytes. Pass \ref SL_RAIL_TX_STARTED_BYTES to retrieve the start-of-normal-TX
 *   time stamp (see below). After this function, the time_sent field packet_time
 *   will be updated with the time that the preamble for this packet started on air.
 * @return \ref SL_RAIL_STATUS_NO_ERROR if the packet time was successfully
 *   calculated, or an appropriate error code otherwise.
 *
 * When used for time stamp adjustment, call this function in the
 * same transmit-complete event-handling context as
 * \ref sl_rail_get_tx_packet_details() is called.
 *
 * This function may be called when handling the \ref SL_RAIL_EVENT_TX_STARTED
 * event to retrieve that event's start-of-normal-TX time stamp. (Ack
 * transmits currently have no equivalent event or associated time stamp.)
 * In this case, the time_sent field total_packet_bytes must be
 * \ref SL_RAIL_TX_STARTED_BYTES, and the time_sent field packet_time is an
 * output-only parameter filled in with that time (so no need to initialize
 * it beforehand by calling \ref sl_rail_get_tx_packet_details()).
 *
 */
sl_rail_status_t sl_rail_get_tx_time_preamble_start(sl_rail_handle_t rail_handle,
                                                    sl_rail_tx_packet_details_t *p_packet_details);

/**
 * Adjust a RAIL TX time stamp to refer to the end of the sync word.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in,out] p_packet_details A non-NULL pointer to the details that were returned from
 *   a previous call to \ref sl_rail_get_tx_packet_details() for this same packet.
 *   The application must update the time_sent field total_packet_bytes to be
 *   the total number of bytes of the sent packet for RAIL to use when
 *   calculating the specified time stamp. This should account for all bytes
 *   transmitted over the air after the Preamble and Sync word(s), including CRC
 *   bytes. After this function, the time_sent field packet_time
 *   will be updated with the time that the sync word for this packet finished on
 *   air.
 * @return \ref SL_RAIL_STATUS_NO_ERROR if the packet time was successfully
 *   calculated, or an appropriate error code otherwise.
 *
 * Call the time stamp adjustment function in the same
 * transmit-complete event-handling context as
 * \ref sl_rail_get_tx_packet_details() is called.
 */
sl_rail_status_t sl_rail_get_tx_time_sync_word_end(sl_rail_handle_t rail_handle,
                                                   sl_rail_tx_packet_details_t *p_packet_details);

/**
 * Adjust a RAIL TX time stamp to refer to the end of frame.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in,out] p_packet_details A non-NULL pointer to the details that were returned from
 *   a previous call to \ref sl_rail_get_tx_packet_details() for this same packet.
 *   The application must update the time_sent field total_packet_bytes to be
 *   the total number of bytes of the sent packet for RAIL to use when
 *   calculating the specified time stamp. This should account for all bytes
 *   transmitted over the air after the Preamble and Sync word(s), including CRC
 *   bytes. After this function, the time_sent field packet_time
 *   will be updated with the time that this packet finished on air.
 * @return \ref SL_RAIL_STATUS_NO_ERROR if the packet time was successfully
 *   calculated, or an appropriate error code otherwise.
 *
 * Call the time stamp adjustment function in the same
 * transmit-complete event-handling context as
 * \ref sl_rail_get_tx_packet_details() is called.
 */
sl_rail_status_t sl_rail_get_tx_time_frame_end(sl_rail_handle_t rail_handle,
                                               sl_rail_tx_packet_details_t *p_packet_details);

/**
 * Prevent the radio from starting a transmit.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] enable Enable/Disable TX hold off.
 * @return Status code indicating success of the function call.
 *
 * Enable TX hold off to prevent the radio from starting any transmits.
 * Disable TX hold off to allow the radio to transmit again.
 * Attempting to transmit with the TX hold off enabled will result in
 * \ref SL_RAIL_EVENT_TX_BLOCKED and/or \ref SL_RAIL_EVENT_TXACK_BLOCKED
 * events.
 *
 * @note This function does not affect a transmit that has already started.
 *   To stop an already-started transmission, use \ref sl_rail_idle() with
 *   \ref SL_RAIL_IDLE_ABORT.
 */
sl_rail_status_t sl_rail_enable_tx_hold_off(sl_rail_handle_t rail_handle,
                                            bool enable);

/**
 * Check whether or not TX hold off is enabled.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return true if TX hold off is enabled, false otherwise.
 *
 * TX hold off can be enabled/disabled using \ref sl_rail_enable_tx_hold_off().
 * Attempting to transmit with the TX hold off enabled will block the
 * transmission and result in \ref SL_RAIL_EVENT_TX_BLOCKED
 * and/or \ref SL_RAIL_EVENT_TXACK_BLOCKED events.
 */
bool sl_rail_is_tx_hold_off_enabled(sl_rail_handle_t rail_handle);

/**
 * Set an alternate transmitter preamble length.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] bits The desired preamble length, in bits.
 * @return Status code indicating success of the function call.
 *
 * To cause a transmission to use this alternate preamble length,
 * specify \ref SL_RAIL_TX_OPTION_ALT_PREAMBLE_LEN in the tx_options
 * parameter passed to the respective RAIL transmit API.
 *
 * @note Attempting to set a preamble length of 0xFFFF bits will result in
 *   \ref SL_RAIL_STATUS_INVALID_PARAMETER.
 **/
sl_rail_status_t sl_rail_set_tx_alt_preamble_length(sl_rail_handle_t rail_handle,
                                                    uint16_t bits);

/** @} */ // end of group Transmit

/******************************************************************************
 * Receive
 *****************************************************************************/
/**
 * @addtogroup Receive
 * @brief APIs related to packet receive
 * @{
 */

/**
 * Configure receive options.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] rx_options_mask A bitmask containing which options should be modified.
 * @param[in] rx_options A bitmask containing desired configuration settings.
 *   Bit positions for each option are found in the \ref sl_rail_rx_options_t.
 * @return Status code indicating success of the function call.
 *
 * Configure the radio receive flow based on the list of available options.
 * Only the options indicated by the mask parameter will be affected. Pass
 * \ref SL_RAIL_RX_OPTIONS_ALL to set all parameters.
 * The previous settings may affect the current frame if a packet is
 * received during this configuration.
 */
sl_rail_status_t sl_rail_config_rx_options(sl_rail_handle_t rail_handle,
                                           sl_rail_rx_options_t rx_options_mask,
                                           sl_rail_rx_options_t rx_options);

/**
 * Include the code necessary for frame type based length decoding.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This function must be called before \ref sl_rail_config_channels() to allow configurations
 * using a frame type based length setup. It may be called before \ref
 * sl_rail_init() by using a radio-generic handle like \ref SL_RAIL_EFR32_HANDLE.
 */
sl_rail_status_t sl_rail_include_frame_type_length(sl_rail_handle_t rail_handle);

/**
 * Start the receiver on a specific channel.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] channel The channel to listen on.
 * @param[in] p_scheduler_info A pointer to information to allow the radio scheduler to place
 *   this receive appropriately. This is only used in multiprotocol version of
 *   RAIL and may be set to NULL in all other versions.
 * @return Status code indicating success of the function call.
 *
 * This is a non-blocking function. Whenever a packet is received, \ref sl_rail_config_t::events_callback()
 * will fire with \ref SL_RAIL_EVENT_RX_PACKET_RECEIVED set. If you call
 * this while not idle but with a different channel, any ongoing
 * receive or transmit operation will be aborted.
 *
 * Returns an error if no receive FIFO or Packet Queue have been configured.
 */
sl_rail_status_t sl_rail_start_rx(sl_rail_handle_t rail_handle,
                                  uint16_t channel,
                                  const sl_rail_scheduler_info_t *p_scheduler_info);

/**
 * Schedule a receive window for some future time.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] channel A channel to listen on.
 * @param[in] p_scheduled_rx_config A pointer to a configuration structure to define the receive window.
 * @param[in] p_scheduler_info A pointer to information to allow the radio scheduler to place
 *   this receive appropriately. This is only used in multiprotocol version of
 *   RAIL and may be set to NULL in all other versions.
 * @return Status code indicating success of the function call.
 *
 * This API immediately changes the channel and schedules receive to start
 * at the specified time and end at the given end time. If you do not specify
 * an end time, you may call this API later with an end time as long as you set
 * the start time to disabled. You can also terminate the receive
 * operation immediately using the \ref sl_rail_idle() function. Note that relative
 * end times are always relative to the start unless no start time is
 * specified. If changing channels, the channel is changed immediately and
 * will abort any ongoing packet transmission or reception.
 *
 * Returns an error if a CSMA or LBT transmit is still in progress.
 *
 * Returns an error if no receive FIFO or Packet Queue have been configured.
 *
 * In multiprotocol, ensure that you properly yield the radio after this
 * call. See \ref rail_radio_scheduler_yield for more details.
 *
 * @note Unlike RAIL 2.x \ref RAIL_ScheduleRx(), in RAIL 3 the
 *   \ref SL_RAIL_EVENT_RX_SCHEDULED_RX_END occurs
 *   regardless of \ref sl_rail_scheduled_rx_config_t::rx_transition_end_schedule
 *   setting. See that event for details.
 */
sl_rail_status_t sl_rail_start_scheduled_rx(sl_rail_handle_t rail_handle,
                                            uint16_t channel,
                                            const sl_rail_scheduled_rx_config_t *p_scheduled_rx_config,
                                            const sl_rail_scheduler_info_t *p_scheduler_info);

/**
 * Enable automatic PRS LNA bypass for external FEM.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in] enable Enable/Disable automatic PRS LNA bypass.
 * @param[in] p_prs_lna_bypass_config A pointer to an automatic PRS LNA bypass
 *   configuration structure. It must be non-NULL to enable the feature.
 * @return Status code indicating success of the function call.
 *
 * If automatic PRS LNA bypass is enabled on chip that supports the feature
 * (\ref SL_RAIL_SUPPORTS_PRS_LNA_BYPASS), a level is generated on a PRS
 * channel when the received power exceed a threshold. It is turned off after
 * frame reception or after timeout if no frame has been detected and power is
 * below the threshold.
 *
 * @warning Calling this function before \ref sl_rail_init() won't enable the
 *   the feature now. It will return \ref SL_RAIL_STATUS_NO_ERROR but the
 *   feature will be enabled by \ref sl_rail_init().
 *
 * @warning As this function relies on PRS access and RAIL is meant to run in
 *   TrustZone non-secure world, it is not supported if PRS is configured as
 *   secure peripheral and it will return \ref SL_RAIL_STATUS_INVALID_CALL.
 */
sl_rail_status_t sl_rail_enable_prs_lna_bypass(sl_rail_handle_t rail_handle,
                                               bool enable,
                                               const sl_rail_prs_lna_bypass_config_t *p_prs_lna_bypass_config);

/******************************************************************************
 * Packet Information (RX)
 *****************************************************************************/
/// @addtogroup Packet_Information Packet Information
/// @brief APIs to get information about received packets.
///
/// After receiving a packet, RAIL will trigger a
/// \ref SL_RAIL_EVENT_RX_PACKET_RECEIVED event. At that point, there is a variety
/// of information available to the application about the received packet. The
/// following example code assumes that the
/// \ref SL_RAIL_RX_OPTION_REMOVE_APPENDED_INFO is not used, and the application
/// wants as much data about the packet as possible.
/// @code{.c}
/// // Get all information about a received packet.
/// sl_rail_status_t status;
/// sl_rail_rx_packet_info_t rx_info;
/// sl_rail_rx_packet_details_t rx_details;
/// sl_rail_rx_packet_handle_t rx_handle
///   = sl_rail_get_rx_packet_info(rail_handle, SL_RAIL_RX_PACKET_HANDLE_NEWEST, &rx_info);
/// assert(rx_handle != SL_RAIL_RX_PACKET_HANDLE_INVALID);
/// status = sl_rail_get_rx_packet_details(rail_handle, rx_handle, &rx_details);
/// assert(status == SL_RAIL_STATUS_NO_ERROR);
/// if (rx_details.time_received.time_position == SL_RAIL_PACKET_TIME_INVALID) {
///   return; // No time stamp available for this packet
/// }
/// // CRC_BYTES only needs to be added when not using SL_RAIL_RX_OPTION_STORE_CRC
/// rx_details.time_received.total_packet_bytes = rx_info.packet_bytes + CRC_BYTES;
/// // Choose the function which gives the desired time stamp
/// status = sl_rail_get_rx_time_frame_end(rail_handle, &rx_details);
/// assert(status == SL_RAIL_STATUS_NO_ERROR);
/// // Now all fields of rx_info and rx_details have been populated correctly
/// @endcode
///
/// @{

/**
 * Get basic information about a pending or received packet.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] packet_handle A packet handle for the unreleased packet as
 *   returned from a previous call, or sentinel values
 *   \ref SL_RAIL_RX_PACKET_HANDLE_OLDEST,
 *   \ref SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE or
 *   \ref SL_RAIL_RX_PACKET_HANDLE_NEWEST.
 * @param[out] p_packet_info A non-NULL pointer to a \ref sl_rail_rx_packet_info_t
 *   to store info for the requested packet.
 * @return The packet handle for the requested packet:
 *   if packet_handle was one of the sentinel values, returns the actual
 *   packet handle for that packet, otherwise returns packet_handle.
 *   It may return \ref SL_RAIL_RX_PACKET_HANDLE_INVALID to indicate an error.
 *
 * This function can be used in any RX mode. It does not free any receive FIFO
 * or receive Packet Queue resources. If used in receive
 * \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE, the
 * value in \ref sl_rail_rx_packet_info_t::packet_bytes will only return the data
 * remaining in the FIFO. Any data read via earlier calls to
 * \ref sl_rail_read_rx_fifo() is not included.
 *
 * @note When getting information about an arriving packet that is not yet complete,
 *   (i.e., p_packet_info->packet_status == \ref SL_RAIL_RX_PACKET_RECEIVING), keep
 *   in mind its data is highly suspect because it has not yet passed any CRC
 *   integrity checking. Also note that the packet could be aborted, canceled, or
 *   fail momentarily, invalidating its data in Packet mode. Furthermore, there
 *   is a small chance towards the end of packet reception that the filled-in
 *   \ref sl_rail_rx_packet_info_t could include not only packet data received so far,
 *   but also some raw radio-appended info detail bytes that RAIL's
 *   packet-completion processing will subsequently deal with. It's up to the
 *   application to know its packet format well enough to avoid confusing such
 *   info as packet data.
 */
sl_rail_rx_packet_handle_t sl_rail_get_rx_packet_info(sl_rail_handle_t rail_handle,
                                                      sl_rail_rx_packet_handle_t packet_handle,
                                                      sl_rail_rx_packet_info_t *p_packet_info);

/**
 * Get information about the live incoming packet (if any).
 * Differs from \ref sl_rail_get_rx_packet_info() by only returning information
 * about a packet actively being received, something which even the
 * \ref SL_RAIL_RX_PACKET_HANDLE_NEWEST may not represent if there are
 * completed but unprocessed packets in the receive FIFO.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_packet_info A non-NULL pointer to store \ref sl_rail_rx_packet_info_t
 *   for the incoming packet.
 * @return Status code indicating success of the function call.
 *
 * This function can only be called from callback context, e.g.,
 * when handling \ref SL_RAIL_EVENT_RX_FILTER_PASSED or
 * \ref SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND.
 * It must not be used with receive \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE
 * if any portion of an incoming packet has already been extracted from
 * the receive FIFO.
 *
 * @note The incomplete data of an arriving packet is highly suspect because
 *   it has not yet passed any CRC integrity checking. Also note that the
 *   packet could be aborted, canceled, or fail momentarily, invalidating
 *   its data in Packet mode. Furthermore, there is a small chance towards
 *   the end of packet reception that the filled-in \ref sl_rail_rx_packet_info_t
 *   could include not only packet data received so far, but also some raw
 *   radio-appended info detail bytes that RAIL's packet-completion
 *   processing will subsequently deal with. It's up to the application to
 *   know its packet format well enough to avoid confusing such info as
 *   packet data.
 */
sl_rail_status_t sl_rail_get_rx_incoming_packet_info(sl_rail_handle_t rail_handle,
                                                     sl_rail_rx_packet_info_t *p_packet_info);

/**
 * Abort an incoming packet actively being received.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *   \ref SL_RAIL_STATUS_INVALID_STATE would be returned when the radio
 *   is not actively receiving a packet or when the inactive protocol.
 *   It may be possible that \ref SL_RAIL_STATUS_NO_ERROR will be
 *   returned yet the abort attempt was too late to actually abort the
 *   packet.
 *
 * If an incoming packet does get aborted, RAIL will issue \ref
 * SL_RAIL_EVENT_RX_PACKET_ABORTED and the radio would respect the current
 * \ref SL_RAIL_RX_OPTION_TRACK_ABORTED_FRAMES setting and follow the
 * current RX \ref sl_rail_state_transitions_t::error state transition.
 */
sl_rail_status_t sl_rail_abort_rx_incoming_packet(sl_rail_handle_t rail_handle);

/**
 * Copy a full packet to a user-specified contiguous buffer.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_dest A non-NULL application-provided pointer to a buffer of at
 *   least p_packet_info->packet_bytes in size to store the packet data
 *   contiguously. This buffer must never overlay RAIL's receive FIFO buffer.
 *   Exactly p_packet_info->packet_bytes of packet data will be written into it.
 * @param[in] p_packet_info A non-NULL pointer to the
 *   \ref sl_rail_rx_packet_info_t for the requested packet.
 * @return Status code indicating success of the function call.
 *
 * @note This is a convenience helper function, which
 *   is intended to be expedient. As a result, it does not
 *   check the validity of its arguments,
 *   so don't pass either as NULL, and don't
 *   pass a p_dest pointer to a buffer that's too small for the packet's data.
 * @note If only a portion of the packet is needed, use \ref sl_rail_peek_rx_packet()
 *   instead.
 */
sl_rail_status_t sl_rail_copy_rx_packet(sl_rail_handle_t rail_handle,
                                        uint8_t *p_dest,
                                        const sl_rail_rx_packet_info_t *p_packet_info);

/**
 * Get detailed information about a received packet.
 * This function can be used in any RX mode; it does not free any receive FIFO
 * or receive Packet Queue resources.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] packet_handle A packet handle for the unreleased packet as
 *   returned from a previous call to \ref sl_rail_get_rx_packet_info() or
 *   \ref sl_rail_hold_rx_packet(), or sentinel values \ref SL_RAIL_RX_PACKET_HANDLE_OLDEST
 *   \ref SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE or
 *   \ref SL_RAIL_RX_PACKET_HANDLE_NEWEST.
 * @param[out] p_packet_details A non-NULL application-provided pointer to
 *   store \ref sl_rail_rx_packet_details_t for the requested packet.
 *   For \ref sl_rail_rx_packet_status_t SL_RAIL_RX_PACKET_READY_ packets,
 *   the time_received field packet_time will be populated with a time stamp
 *   corresponding to a default location in the packet. The time_received field
 *   time_position will be populated with a \ref sl_rail_packet_time_position_t value
 *   specifying that default packet location. Call
 *   \ref sl_rail_get_rx_time_preamble_start(), \ref sl_rail_get_rx_time_sync_word_end(), or
 *   \ref sl_rail_get_rx_time_frame_end() to adjust that time stamp for different
 *   locations in the packet.
 * @return \ref SL_RAIL_STATUS_NO_ERROR if p_packet_details was filled in,
 *   or an appropriate error code otherwise.
 *
 * @note Certain details are always available, while others are only available
 *   if the \ref sl_rail_rx_options_t \ref SL_RAIL_RX_OPTION_REMOVE_APPENDED_INFO
 *   option is not in effect and the received packet's
 *   \ref sl_rail_rx_packet_status_t is among the SL_RAIL_RX_PACKET_READY_ set.
 *   See \ref sl_rail_rx_packet_details_t for clarification.
 *
 * @note This function should be called soon (no more than a minute) after
 *   packet reception for the packet time stamp information to be valid.
 */
sl_rail_status_t sl_rail_get_rx_packet_details(sl_rail_handle_t rail_handle,
                                               sl_rail_rx_packet_handle_t packet_handle,
                                               sl_rail_rx_packet_details_t *p_packet_details);

/**
 * Adjust a RAIL RX time stamp to refer to the start of the preamble.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in,out] p_packet_details A non-NULL pointer to the details that were returned from
 *   a previous call to \ref sl_rail_get_rx_packet_details() for this same packet.
 *   The application must update the time_received field total_packet_bytes to be
 *   the total number of bytes of the received packet for RAIL to use when
 *   calculating the specified time stamp. This should account for all bytes
 *   received over the air after the Preamble and Sync word(s), including CRC
 *   bytes. After this function, the time_received field packet_time will be
 *   updated with the time that the preamble for this packet started on air.
 * @return \ref SL_RAIL_STATUS_NO_ERROR if the packet time was successfully
 *   calculated, or an appropriate error code otherwise.
 */
sl_rail_status_t sl_rail_get_rx_time_preamble_start(sl_rail_handle_t rail_handle,
                                                    sl_rail_rx_packet_details_t *p_packet_details);

/**
 * Adjust a RAIL RX time stamp to refer to the end of the sync word.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in,out] p_packet_details A non-NULL pointer to the details that were returned from
 *   a previous call to \ref sl_rail_get_rx_packet_details() for this same packet.
 *   The application must update the time_received field total_packet_bytes to be
 *   the total number of bytes of the received packet for RAIL to use when
 *   calculating the specified time stamp. This should account for all bytes
 *   received over the air after the Preamble and Sync word(s), including CRC
 *   bytes. After this function, the time_received field packet_time will be
 *   updated with the time that the sync word for this packet finished on air.
 * @return \ref SL_RAIL_STATUS_NO_ERROR if the packet time was successfully
 *   calculated, or an appropriate error code otherwise.
 */
sl_rail_status_t sl_rail_get_rx_time_sync_word_end(sl_rail_handle_t rail_handle,
                                                   sl_rail_rx_packet_details_t *p_packet_details);

/**
 * Adjust a RAIL RX time stamp to refer to the end of frame.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in,out] p_packet_details A non-NULL pointer to the details that were returned from
 *   a previous call to \ref sl_rail_get_rx_packet_details() for this same packet.
 *   The application must update the time_received field total_packet_bytes to be
 *   the total number of bytes of the received packet for RAIL to use when
 *   calculating the specified time stamp. This should account for all bytes
 *   received over the air after the Preamble and Sync word(s), including CRC
 *   bytes. After this function, the time_received field packet_time will be
 *   updated with the time that the packet finished on air.
 * @return \ref SL_RAIL_STATUS_NO_ERROR if the packet time was successfully
 *   calculated, or an appropriate error code otherwise.
 */
sl_rail_status_t sl_rail_get_rx_time_frame_end(sl_rail_handle_t rail_handle,
                                               sl_rail_rx_packet_details_t *p_packet_details);

/** @} */ // end of group Packet_Information

/**
 * Place a temporary hold on this packet's data and metadata in the receive
 * FIFO and receive Packet Queue.
 *
 * This function can only be called from within RAIL callback context.
 * It can be used in any RX mode.
 *
 * Normally, when RAIL issues its callback indicating a packet is ready
 * or aborted, it expects the application's callback to retrieve and
 * copy (or discard) the packet's information and data, and will free
 * the packet data and metadata after the callback returns. This function
 * tells RAIL to hold onto those resources in the receive FIFO and receive
 * Packet Queue after the callback returns in case the application wants to
 * defer processing the packet to a later time, e.g., outside of callback
 * context.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The packet handle for the packet associated with the callback,
 *   \ref SL_RAIL_RX_PACKET_HANDLE_INVALID if no such packet yet exists or
 *   rail_handle is not active.
 *
 * @note When using multiprotocol and either the receive
 *   FIFO or receive Packet Queue are shared with another protocol, both
 *   are reset during protocol switches losing any held packets.
 *   To avoid this, either give each protocol its own receive FIFO
 *   and receive Packet Queue, or make sure to invalidate any held
 *   packet handles or related \ref sl_rail_rx_packet_info_t information when
 *   the \ref SL_RAIL_EVENT_CONFIG_UNSCHEDULED occurs.
 */
sl_rail_rx_packet_handle_t sl_rail_hold_rx_packet(sl_rail_handle_t rail_handle);

/**
 * Copy 'len_bytes' of packet data starting from 'byte_offset' from the
 * receive FIFO. Those bytes remain valid for re-peeking.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] packet_handle A packet handle as returned from a previous
 *   \ref sl_rail_get_rx_packet_info() or \ref sl_rail_hold_rx_packet() call, or
 *   sentinel values \ref SL_RAIL_RX_PACKET_HANDLE_OLDEST,
 *   \ref SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE
 *   or \ref SL_RAIL_RX_PACKET_HANDLE_NEWEST.
 * @param[out] p_dest A pointer to the location where the received bytes will
 *   be copied. If NULL, no copying occurs.
 * @param[in] peek_bytes A number of packet data bytes to copy.
 * @param[in] offset_bytes A byte offset within remaining packet data from which
 *   to copy.
 * @return Number of packet bytes copied.
 *
 * @note Peek does not permit peeking beyond the requested packet's
 *   available packet data (though there is a small chance it might
 *   for a \ref SL_RAIL_RX_PACKET_HANDLE_NEWEST packet at the very end of
 *   still being received). Nor can one peek into already-consumed data read
 *   by \ref sl_rail_read_rx_fifo(). len and offset are relative to the remaining data
 *   available in the packet, if any was already consumed by \ref sl_rail_read_rx_fifo().
 */
uint16_t sl_rail_peek_rx_packet(sl_rail_handle_t rail_handle,
                                sl_rail_rx_packet_handle_t packet_handle,
                                uint8_t *p_dest,
                                uint16_t peek_bytes,
                                uint16_t offset_bytes);

/**
 * Release RAIL's resources for a packet previously held in the receive FIFO
 * and receive Packet Queue.
 *
 * This function must be called for any packet previously held via
 * \ref sl_rail_hold_rx_packet(). It may optionally be called within a
 * callback context to release RAIL resources sooner than at
 * callback completion time when not holding the packet.
 * This function can be used in any RX mode.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] packet_handle A packet handle as returned from a previous
 *   \ref sl_rail_hold_rx_packet() call, or sentinel values
 *   \ref SL_RAIL_RX_PACKET_HANDLE_OLDEST,
 *   \ref SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE
 *   or \ref SL_RAIL_RX_PACKET_HANDLE_NEWEST.
 *   The latter might be used within RAIL callback context to explicitly
 *   release the packet associated with the callback early, before it's
 *   released automatically by RAIL on callback return (unless explicitly
 *   held).
 * @return \ref SL_RAIL_STATUS_NO_ERROR if the held packet was released
 *   or an appropriate error code otherwise.
 */
sl_rail_status_t sl_rail_release_rx_packet(sl_rail_handle_t rail_handle,
                                           sl_rail_rx_packet_handle_t packet_handle);

/**
 * Return the current raw RSSI in quarter-dBm within a definitive time period.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] wait_timeout_us Sets the maximum time in microseconds to wait for a valid RSSI.
 *   If equal to \ref SL_RAIL_GET_RSSI_NO_WAIT returns instant RSSI with no checks.
 *   If equal to \ref SL_RAIL_GET_RSSI_WAIT_WITHOUT_TIMEOUT waits for a valid RSSI
 *   with no maximum timeout.
 * @return \ref SL_RAIL_RSSI_INVALID if the receiver is disabled and an RSSI
 *   value can't be obtained. Otherwise, return the RSSI in quarter dBm (dBm*4).
 *
 * Gets the current RSSI value. This value represents the current energy of the
 * channel. It can change rapidly, and will be low if no RF energy is
 * in the current channel. The function from the value reported to dBm is an
 * offset dependent on the PHY and the PCB layout. Characterize the
 * RSSI received on your hardware and apply an offset in the application to
 * account for board and PHY parameters. If a value of \ref SL_RAIL_GET_RSSI_NO_WAIT
 * is given for wait_timeout_us, the radio needs to be currently in RX and have been
 * in there for a sufficient amount of time for a fresh RSSI value to be read and
 * returned. Otherwise the RSSI is considered stale and \ref SL_RAIL_RSSI_INVALID is
 * returned instead. For non-zero values of wait_timeout_us, if the radio is
 * transitioning to or already in RX, this function will wait a maximum time equal
 * to wait_timeout_us (or indefinitely if wait_timeout_us is set to
 * \ref SL_RAIL_GET_RSSI_WAIT_WITHOUT_TIMEOUT) for a valid RSSI to be read and return
 * it. Otherwise, if the wait_timeout_us is reached, or the radio is in or transitions
 * to IDLE or TX, \ref SL_RAIL_RSSI_INVALID will be returned. On low datarate PHYs,
 * this function can take a significantly longer time when wait_timeout_us is non-zero.
 *
 * In multiprotocol, this function returns \ref SL_RAIL_RSSI_INVALID
 * immediately if rail_handle is not the currently active \ref sl_rail_handle_t.
 * Additionally, wait_timeout_us should never be set to a value other than
 * \ref SL_RAIL_GET_RSSI_NO_WAIT in multiprotocol as the integration between this
 * feature and the radio scheduler has not been implemented.
 *
 * @note If RX Antenna Diversity is enabled via \ref sl_rail_config_rx_options(),
 *   pass true for the wait parameter otherwise it's very likely
 *   \ref SL_RAIL_RSSI_INVALID will be returned.
 *
 * @note If RX Antenna Diversity is enabled via \ref sl_rail_config_rx_options(),
 *   the RSSI value returned could come from either antenna and vary between antennas.
 *
 * @note If RX channel hopping is turned on, do not use this API.
 *   Instead, see \ref sl_rail_get_channel_hopping_rssi().
 */
int16_t sl_rail_get_rssi(sl_rail_handle_t rail_handle,
                         sl_rail_time_t wait_timeout_us);

/**
 * Start the RSSI averaging over a specified time in us.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] channel The physical channel to set.
 * @param[in] averaging_time_us Averaging time in microseconds.
 * @param[in] p_scheduler_info A pointer to information to allow the radio scheduler to place
 *   this operation appropriately. This is only used in multiprotocol version of
 *   RAIL and may be set to NULL in all other versions.
 * @return Status code indicating success of the function call.
 *
 * Starts a non-blocking hardware-based RSSI averaging mechanism. Only a single
 * instance of RSSI averaging can be run at any time and the radio must be idle
 * to start.
 *
 * In multiprotocol, this is a scheduled event. It will start when rail_handle
 * becomes active. rail_handle needs to stay active until the averaging
 * completes. If the averaging is interrupted, calls to
 * \ref sl_rail_get_average_rssi() will return \ref SL_RAIL_RSSI_INVALID.
 *
 * Also in multiprotocol, the user is required to call \ref sl_rail_yield_radio()
 * after this event completes (i.e., when \ref SL_RAIL_EVENT_RSSI_AVERAGE_DONE
 * occurs).
 *
 * @note If the radio is idled while RSSI averaging is still in effect, a
 *   \ref SL_RAIL_EVENT_RSSI_AVERAGE_DONE event may not occur and
 *   \ref sl_rail_is_average_rssi_ready() may never return true.
 *
 * @note Completion of RSSI averaging, marked by RAIL event
 *   \ref SL_RAIL_EVENT_RSSI_AVERAGE_DONE, will return the radio to idle state.
 */
sl_rail_status_t sl_rail_start_average_rssi(sl_rail_handle_t rail_handle,
                                            uint16_t channel,
                                            sl_rail_time_t averaging_time_us,
                                            const sl_rail_scheduler_info_t *p_scheduler_info);

/**
 * Query whether the RSSI averaging is done.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return true if done and false otherwise.
 *
 * This function can be used to poll for completion of the RSSI averaging
 * to avoid relying on an interrupt-based callback.
 *
 * @note If the radio is idled while RSSI averaging is still in effect,
 *   this function may never return true.
 */
bool sl_rail_is_average_rssi_ready(sl_rail_handle_t rail_handle);

/**
 * Get the RSSI in quarter-dBm averaged over a specified time in microseconds.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The RSSI in quarter-dBm (dBm * 4), or \ref SL_RAIL_RSSI_INVALID
 *   if the receiver is disabled or an an RSSI value couldn't be obtained.
 *
 * Gets the hardware RSSI average after issuing \ref sl_rail_start_average_rssi().
 * Use after \ref sl_rail_start_average_rssi().
 */
int16_t sl_rail_get_average_rssi(sl_rail_handle_t rail_handle);

/**
 * Set an RSSI offset.
 *
 * @param[in] rail_handle a radio-generic or real RAIL instance handle.
 * @param[in] rssi_offset_db desired dB offset to be added to the RSSI measurements.
 * @return Status code indicating success of the function call.
 *   \ref SL_RAIL_STATUS_INVALID_CALL if called with radio-generic handle, such
 *   as \ref SL_RAIL_EFR32_HANDLE, after RAIL initialization.
 *   \ref SL_RAIL_STATUS_INVALID_PARAMETER if the RSSI offset is deemed large
 *   enough to cause the RSSI readings to underflow or overflow.
 *
 * Adds an offset to the RSSI in dBm. This offset affects all functionality that
 * depends on RSSI values, such as CCA functions. Do not modify the offset
 * dynamically during packet reception. This function
 * can only be called while the radio is off, or in the case of multiprotocol,
 * on an inactive protocol.
 *
 * @note: Prior to RAIL being initialized, a radio-wide protocol-independent
 *   offset can be established using a radio-generic handle like \ref
 *   SL_RAIL_EFR32_HANDLE. The radio RSSI offset can range from -50 to +20 dB.\n
 *   After RAIL has been initialized a real RAIL protocol handle must be
 *   provided to set a protocol-specific RSSI offset, which is added to any
 *   radio offset (plus a per-PHY offset set by the radio calculator). The
 *   absolute value of the overall sum of all these offsets cannot exceed \ref
 *   SL_RAIL_RSSI_OFFSET_MAX otherwise RSSIs could underflow or overflow.
 *
 * @note: During \ref Rx_Channel_Hopping this API will not update the
 *   RSSI offset immediately if channel hopping has already been configured.
 *   A subsequent call to \ref sl_rail_zwave_config_rx_channel_hopping() or
 *   \ref sl_rail_config_rx_channel_hopping() is required for the new RSSI offset to
 *   take effect.
 */
sl_rail_status_t sl_rail_set_rssi_offset(sl_rail_handle_t rail_handle,
                                         int8_t rssi_offset_db);

/**
 * Get the radio or protocol RSSI offset in dB.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return The RSSI offset in dB corresponding to the rail_handle.
 *
 * @note: A radio-generic handle, such as \ref SL_RAIL_EFR32_HANDLE, can be used to
 *   get the radio's RSSI offset otherwise this will return the RSSI offset
 *   value associated with the RAIL protocol instance handle, exclusive of any
 *   radio or PHY-specific RSSI offset correction, if any.
 */
int8_t sl_rail_get_rssi_offset(sl_rail_handle_t rail_handle);

/**
 * Set the RSSI detection threshold in dBm to trigger
 * \ref SL_RAIL_EVENT_DETECT_RSSI_THRESHOLD.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] rssi_threshold_dbm desired RSSI threshold in dBm over which the event
 *   \ref SL_RAIL_EVENT_DETECT_RSSI_THRESHOLD is triggered.
 * @return Status code indicating success of the function call.
 *   Returns \ref SL_RAIL_STATUS_INVALID_STATE in multiprotocol,
 *   if the requested rail_handle is not active.
 *   Returns \ref SL_RAIL_STATUS_INVALID_CALL if called on parts on which this function
 *   is not supported.
 *
 * When in receive, RSSI is sampled and if it exceeds the threshold,
 * \ref SL_RAIL_EVENT_DETECT_RSSI_THRESHOLD is triggered.
 *
 * @note: If the radio is idled or this function is called with rssi_threshold_dbm as
 *   \ref SL_RAIL_RSSI_INVALID_DBM while RSSI detect is still in effect, a
 *   \ref SL_RAIL_EVENT_DETECT_RSSI_THRESHOLD may not occur and the detection is disabled.
 *   If the RSSI is already above threshold when this function is called then
 *   \ref SL_RAIL_EVENT_DETECT_RSSI_THRESHOLD will occur.
 *   Once the RSSI goes over the configured threshold and
 *   \ref SL_RAIL_EVENT_DETECT_RSSI_THRESHOLD occurs, this function needs to be
 *   called again to reactivate the RSSI threshold detection.
 *
 * This function is only available on EFR32 Series 2 Sub-GHz parts EFR32xG23 and EFR32xG25.
 */
sl_rail_status_t sl_rail_set_rssi_detect_threshold(sl_rail_handle_t rail_handle,
                                                   int8_t rssi_threshold_dbm);

/**
 * Get the RSSI detection threshold in dBm.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The RSSI threshold in dBm corresponding to the rail_handle.
 *
 * @note: The function returns \ref SL_RAIL_RSSI_INVALID_DBM when
 *   \ref sl_rail_set_rssi_detect_threshold() is not supported or disabled.
 *   In multiprotocol, the function returns \ref SL_RAIL_RSSI_INVALID_DBM if rail_handle
 *   is not active.
 *
 * This function is only available on EFR32 Series 2 Sub-GHz parts EFR32xG23 and EFR32xG25.
 */
int8_t sl_rail_get_rssi_detect_threshold(sl_rail_handle_t rail_handle);

/**
 * Return the RSSI associated with the incoming packet.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The RSSI on the incoming packet in dBm,
 *   or \ref SL_RAIL_RSSI_INVALID_DBM if not available.
 *
 * This function can only be called from callback context, e.g.,
 * when handling \ref SL_RAIL_EVENT_RX_FILTER_PASSED or
 * \ref SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND.
 */
int8_t sl_rail_get_rx_incoming_packet_rssi(sl_rail_handle_t rail_handle);

/**
 * Set up a callback function capable of converting a RX packet's LQI value
 * before being consumed by application code.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] lqi_callback A callback of type \ref sl_rail_convert_lqi_callback_t() that is
 *   called before the RX packet LQI value is loaded into the \ref
 *   sl_rail_rx_packet_details_t structure for application consumption.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_convert_lqi(sl_rail_handle_t rail_handle,
                                     sl_rail_convert_lqi_callback_t lqi_callback);

/******************************************************************************
 * Address Filtering (RX)
 *****************************************************************************/
/**
 * @addtogroup Address_Filtering Address Filtering
 * @brief Configuration APIs for receive packet address filtering.
 *
 * The address filtering code examines the packet as follows.
 *
 * | `Bytes: 0 - 255` |  `0 - 8`  | `0 - 255` |  `0 - 8`  | `Variable` |
 * |:----------------:|----------:|----------:|----------:|:----------:|
 * | `Data_0`         | `Field_0` | `Data_1`  | `Field_1` | `Data_2`   |
 *
 * In the above structure, anything listed as Data_# is an optional section of
 * bytes that RAIL will not process for address filtering. The Field_# segments
 * reference specific sections in the packet that will each be interpreted
 * as an address during address filtering. The application may submit up to
 * four addresses to attempt to match each field segment and each address may
 * have a size of up to 8 bytes. To set up address filtering, first configure
 * the locations and length of the addresses in the packet. Next, configure
 * which combinations of matches in Field_0 and Field_1 should constitute an
 * address match. Last, enter addresses into tables for each field and
 * enable them. The first two of these are part of the \ref sl_rail_addr_config_t
 * structure while the second part is configured at runtime using the
 * \ref sl_rail_set_address_filter_address() API. A brief description of each
 * configuration is listed below.
 *
 * The offsets and sizes of the fields
 * are assumed fixed for the RAIL address filter. To set them, specify
 * arrays for these values in the sizes and offsets entries in the
 * \ref sl_rail_addr_config_t structure. A size of zero indicates that a field is
 * disabled. The start offset for a field is relative to the previous start
 * offset and, if you're using frame type decoding, the first start offset is
 * relative to the end of the byte containing the frame type.
 *
 * Configuring which combinations of Field_0 and Field_1 constitute a match is
 * the most complex portion of the address filter. The easiest way to think
 * about this is with a truth table. If you consider each of the four possible
 * address entries in a field, you can have a match on any one of those or a
 * match for none of them. This is shown in the 5x5 truth table below where
 * Field_0 matches are the rows and Field_1 matches are the columns.
 *
 * |                | No Match | Address 0 | Address 1 | Address 2 | Address 3 |
 * |----------------|----------|-----------|-----------|-----------|-----------|
 * | __No Match__   | bit 0    | bit 1     | bit 2     | bit 3     | bit 4     |
 * | __Address 0__  | bit 5    | bit 6     | bit 7     | bit 8     | bit 9     |
 * | __Address 1__  | bit 10   | bit 11    | bit 12    | bit 13    | bit 14    |
 * | __Address 2__  | bit 15   | bit 16    | bit 17    | bit 18    | bit 19    |
 * | __Address 3__  | bit 20   | bit 21    | bit 22    | bit 23    | bit 24    |
 *
 * Because this is only 25 bits, it can be represented in one 32-bit integer
 * where 1 indicates a filter pass and 0 indicates a filter fail. This is the
 * \ref sl_rail_addr_config_t::match_table field and is used during
 * filtering. For common simple configurations, two defines are provided with
 * the truth tables as shown below. The first is \ref
 * SL_RAIL_ADDR_CONFIG_MATCH_TABLE_SINGLE_FIELD, which can be used if only using
 * one address field (either field). If using two fields and want to
 * force in the same address entry in each field, use the second define: \ref
 * SL_RAIL_ADDR_CONFIG_MATCH_TABLE_DOUBLE_FIELD. For more complex systems,
 * create a valid custom table.
 *
 * @note Address filtering does not function reliably with PHYs that use a data
 *   rate greater than 500 kbps. If this is a requirement, filtering must
 *   currently be done by the application.
 *
 * @{
 */

/**
 * Configure address filtering.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_addr_config A pointer to the configuration structure, which defines how
 *   addresses are set up in your packets. May be NULL to reset address filtering.
 * @return Status code indicating success of the function call.
 *
 * You must call this function to set up address filtering. You may call it
 * multiple times but all previous information is wiped out each time you call
 * and any configured addresses must be reset.
 */
sl_rail_status_t sl_rail_config_address_filter(sl_rail_handle_t rail_handle,
                                               const sl_rail_addr_config_t *p_addr_config);

/**
 * Enable address filtering.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] enable An argument to indicate whether or not to enable address
 *   filtering.
 * @return true if address filtering was enabled to start with and false
 *   otherwise.
 *
 * Only allow packets through that pass the current address filtering
 * configuration. This does not reset or change the configuration so you can
 * set that up before turning on this feature.
 */
bool sl_rail_enable_address_filter(sl_rail_handle_t rail_handle, bool enable);

/**
 * Return whether address filtering is currently enabled.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return true if address filtering is enabled and false otherwise.
 */
bool sl_rail_is_address_filter_enabled(sl_rail_handle_t rail_handle);

/**
 * Reset the address filtering configuration.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * Resets all structures related to address filtering. This does not disable
 * address filtering. It leaves the radio in a state where no packets
 * pass filtering.
 */
sl_rail_status_t sl_rail_reset_address_filter(sl_rail_handle_t rail_handle);

/**
 * Set an address for filtering in hardware.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] field Indicates an address field for this address.
 * @param[in] index Indicates a match entry for this address for a
 *   given field.
 * @param[in] p_value A pointer to the address data. This must be at least as
 *   long as the size specified in \ref sl_rail_config_address_filter(). The first byte,
 *   p_value[0], will be compared to the first byte received over the air for this
 *   address field.
 * @param[in] enable A boolean to indicate whether this address should be
 *   enabled immediately.
 * @return Status code indicating success of the function call.
 *
 * This function loads the given address into hardware for filtering and
 * starts filtering if you set the enable parameter to true. Otherwise,
 * call \ref sl_rail_enable_address_filter_address() to turn it on later.
 */
sl_rail_status_t sl_rail_set_address_filter_address(sl_rail_handle_t rail_handle,
                                                    uint8_t field,
                                                    uint8_t index,
                                                    const uint8_t *p_value,
                                                    bool enable);

/**
 * Set an address bit mask for filtering in hardware.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] field Indicates an address field for this address bit mask.
 * @param[in] p_bitmask A pointer to the address bitmask. This must be at least
 *   as long as the size specified in \ref sl_rail_config_address_filter(). The first
 *   byte, p_bitmask[0], will be applied to the first byte received over the air
 *   for this address field. Bits set to 1 in the bit mask indicate which bit
 *   positions in the incoming packet to compare against the stored addresses
 *   during address filtering. Bits set to 0 indicate which bit positions to
 *   ignore in the incoming packet during address filtering. This bit mask is
 *   applied to all address entries.
 * @return Status code indicating success of the function call.
 *
 * This function loads the given address bit mask into hardware for use when
 * address filtering is enabled. All bits in the stored address bit mask are
 * set to 1 during hardware initialization and when either \ref
 * sl_rail_config_address_filter() or \ref sl_rail_reset_address_filter() are called.
 *
 * @note This feature/API is not supported on the EFR32xG21.
 *   Use the compile time symbol \ref
 *   SL_RAIL_SUPPORTS_ADDR_FILTER_ADDRESS_BIT_MASK or the runtime call \ref
 *   sl_rail_supports_addr_filter_address_bit_mask() to check whether the platform
 *   supports this feature.
 */
sl_rail_status_t sl_rail_set_address_filter_address_mask(sl_rail_handle_t rail_handle,
                                                         uint8_t field,
                                                         const uint8_t *p_bitmask);

/**
 * Enable address filtering for the specified address.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] enable An argument to indicate whether or not to enable address
 *   filtering.
 * @param[in] field Indicates an address for the address.
 * @param[in] index Indicates a match entry in the given field you want to enable.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_enable_address_filter_address(sl_rail_handle_t rail_handle,
                                                       bool enable,
                                                       uint8_t field,
                                                       uint8_t index);

/** @} */ // end of group Address_Filtering

/** @} */ // end of group Receive

/******************************************************************************
 * Auto-Acking
 *****************************************************************************/
/// @addtogroup Auto_Ack Auto-Ack
/// @brief APIs for configuring Auto-Ack functionality
///
/// These APIs configure the radio for automatic acknowledgment
/// features. Auto-Ack inherently changes how the underlying state machine
/// behaves so users should not modify \ref sl_rail_set_rx_transitions() and
/// \ref sl_rail_set_tx_transitions() while using Auto-Ack features.
/// @code{.c}
/// // Go to RX after Ack operation.
/// sl_rail_auto_ack_config_t auto_ack_config = {
///   .enable = true,
///   .ack_timeout_us = 1000,
///   // "error" param ignored
///   .rx_transitions = {
///      .success = SL_RAIL_RF_STATE_RX,
///      .error   = SL_RAIL_RF_STATE_RX
///   },
///   // "error" param ignored
///   .tx_transitions = {
///      .success = SL_RAIL_RF_STATE_RX,
///      .error   = SL_RAIL_RF_STATE_RX,
///   },
/// };
///
/// sl_rail_status_t status = sl_rail_config_auto_ack(rail_handle, &auto_ack_config);
///
/// uint8_t ack_data[] = {0x05, 0x02, 0x10, 0x00};
///
/// sl_rail_status_t status = sl_rail_write_auto_ack_fifo(rail_handle,
///                                                       ack_data,
///                                                       sizeof(ack_data));
/// @endcode
///
/// The acknowledgment transmits based on the frame format configured via
/// the Radio Configurator. For example, if the frame format is using a variable
/// length scheme, the Ack will be sent according to that scheme. If a 10-byte
/// packet is loaded into the Ack, but the variable length field of the Ack
/// payload specifies a length of 5, only 5 bytes will transmit for the Ack.
/// The converse is also true, if the frame length is configured to be a fixed
/// 10-byte packet but only 5 bytes are loaded into the Ack buffer, a TX
/// underflow occurs during the Ack transmit.
///
/// Unlike in non-Auto-Ack mode, Auto-Ack mode will always return to a single
/// state after all Ack sequences complete, regardless of whether
/// the Ack was successfully received/sent or not. See the documentation
/// of \ref sl_rail_config_auto_ack() for configuration information. To
/// suspend automatic acknowledgment of a series of packets after transmit
/// or receive call \ref sl_rail_pause_tx_auto_ack() or \ref sl_rail_pause_rx_auto_ack() respectively
/// with the pause parameter set to true. When Auto-Acking is paused, after
/// receiving or transmitting a packet (regardless of success), the radio
/// transitions to the same state it would use while Acking. To return to
/// normal state transition logic outside of Acking, call \ref
/// sl_rail_config_auto_ack() with the \ref sl_rail_auto_ack_config_t::enable field false
/// and specify the desired transitions in the \ref
/// sl_rail_auto_ack_config_t::rx_transitions and sl_rail_auto_ack_config_t::tx_transitions
/// fields. To get out of a paused state and resume Auto-Acking, call \ref
/// sl_rail_pause_tx_auto_ack() and/or \ref sl_rail_pause_rx_auto_ack() with the pause parameter
/// set to false.
///
/// Applications can cancel the transmission of an Ack with \ref
/// sl_rail_cancel_auto_ack(). Conversely, applications can control if a transmit
/// operation should wait for an Ack after transmitting by using
/// the \ref SL_RAIL_TX_OPTION_WAIT_FOR_ACK option.
///
/// When \ref Antenna_Control is used for multiple antennas, Acks are
/// transmitted on the antenna that was selected to receive the packet
/// being acknowledged. When receiving an Ack, the
/// \ref sl_rail_rx_options_t antenna options are used just like for any other
/// receive.
///
/// If the Ack payload is dynamic, the application must call \ref
/// sl_rail_write_auto_ack_fifo() with the appropriate Ack payload after the
/// application processes the receive. RAIL can Auto-Ack from the normal
/// transmit buffer if \ref sl_rail_use_tx_fifo_for_auto_ack() is called before the radio
/// transmits the Ack. Ensure the transmit buffer contains data loaded by \ref
/// sl_rail_write_tx_fifo().
///
/// Standard-based protocols that contain Auto-Ack functionality are normally
/// configured in the protocol-specific configuration function. For example,
/// \ref sl_rail_ieee802154_init() provides Auto-Ack configuration parameters in \ref
/// sl_rail_ieee802154_config_t and should only be configured through that
/// function. It is not advisable to call both \ref sl_rail_ieee802154_init() and \ref
/// sl_rail_config_auto_ack(). However, Ack modification functions are still valid to
/// use with protocol-specific Acks. To cancel an IEEE 802.15.4 Ack transmit,
/// use \ref sl_rail_cancel_auto_ack().
///
/// @{

///
/// Configure and enable automatic acknowledgment.
///
/// @param[in] rail_handle A real RAIL instance handle.
/// @param[in] p_auto_ack_config A pointer to an Auto-Ack configuration structure.
/// @return Status code indicating success of the function call.
///
/// Configures the RAIL state machine for hardware-accelerated automatic
/// acknowledgment. Ack timing parameters are defined in the configuration
/// structure.
///
/// While Auto-Acking is enabled, do not call the following RAIL functions:
///   - \ref sl_rail_set_rx_transitions()
///   - \ref sl_rail_set_tx_transitions()
///
/// When Auto-Acking is enabled, only one state transition can be defined
/// (without notion of success or error).
/// Thus if you are enabling Auto-Ack (i.e., p_config->enable field is true)
/// the "error" states of p_config->rx_transitions and p_config->tx_transitions are ignored.
/// After all Ack sequences, (success or fail) the state machine will return
/// the radio to the "success" state, which can be either
/// \ref SL_RAIL_RF_STATE_RX or \ref SL_RAIL_RF_STATE_IDLE (returning to
/// \ref SL_RAIL_RF_STATE_TX is not supported).
/// Alternatively when Auto-Acking is disabled (i.e., p_config->enable field is
/// false), transitions are reconfigured using all fields of p_config->rx_transitions
/// and p_config->tx_transitions. When disabling, the "ack_timeout" field isn't used.
///
/// If you need information about the
/// actual success of the Ack sequence, use RAIL events such as
/// \ref SL_RAIL_EVENT_TXACK_PACKET_SENT to make sure an Ack was sent, or
/// \ref SL_RAIL_EVENT_RX_ACK_TIMEOUT to make sure that an Ack was received
/// within the specified timeout.
///
/// To set a certain turnaround time (i.e., tx_to_rx and rx_to_tx
/// in \ref sl_rail_state_timing_t), make tx_to_rx lower than
/// desired to ensure you get to RX in time to receive the Ack.
/// Silicon Labs recommends setting 10 us lower than desired:
/// @code{.c}
/// void set_auto_ack_state_timings(void)
/// {
///   sl_rail_state_timing_t timings;
///
///   // User is already in Auto-Ack and wants a turnaround of 192 us.
///   timings.rx_to_tx = 192;
///   timings.tx_to_rx = 192 - 10;
///
///   // Set other fields of timings...
///   timings.idle_to_rx = 100;
///   timings.idle_to_tx = 100;
///   timings.rxsearch_timeout = 0;
///   timings.tx_to_rxsearch_timeout = 0;
///
///   sl_rail_set_state_timing(rail_handle, &timings);
/// }
/// @endcode
///
/// @note Auto-Acking may not be enabled while RX Channel Hopping is enabled,
///   or when BLE is enabled.
///
sl_rail_status_t sl_rail_config_auto_ack(sl_rail_handle_t rail_handle,
                                         const sl_rail_auto_ack_config_t *p_auto_ack_config);

/**
 * Return the enable status of the Auto-Ack feature.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return true if Auto-Ack is enabled, false if disabled.
 */
bool sl_rail_is_auto_ack_enabled(sl_rail_handle_t rail_handle);

/**
 * Load the Auto-Ack buffer with Ack data.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_ack_data A pointer to Ack data to transmit.
 *   This may be NULL, in which case it's assumed the data has already
 *   been emplaced into the Ack buffer and RAIL just needs to be told
 *   how many bytes are there. Use \ref sl_rail_get_auto_ack_fifo() to get
 *   the address of RAIL's Auto-Ack buffer in RAM and its size.
 * @param[in] ack_bytes The number of bytes in Ack data.
 * @return Status code indicating success of the function call.
 *
 * If the Ack buffer is available for updates, load the Ack buffer with data.
 * If it is not available, \ref SL_RAIL_STATUS_INVALID_STATE is returned.
 * If ack_bytes exceeds the size of the Auto-Ack FIFO buffer then
 * \ref SL_RAIL_STATUS_INVALID_PARAMETER will be returned and nothing is
 * written to the Ack buffer (unless p_ack_data is NULL in which case this
 * indicates the application has already likely corrupted RAM).
 */
sl_rail_status_t sl_rail_write_auto_ack_fifo(sl_rail_handle_t rail_handle,
                                             const uint8_t *p_ack_data,
                                             uint16_t ack_bytes);

/**
 * Get the address and size of the Auto-Ack transmit buffer for direct access.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in,out] pp_ack_buffer A pointer to a uint8_t pointer that will be
 *   updated to the RAM base address of the Auto-Ack FIFO buffer.
 * @param[in,out] p_ack_buffer_bytes A pointer to a uint16_t that will be
 *   updated to the size of the Auto-Ack FIFO buffer, in bytes.
 * @return Status code indicating success of the function call.
 *
 * Applications can use this to more flexibly write Auto-Ack data into
 * the buffer directly and in pieces, passing NULL p_ack_data parameter to
 * \ref sl_rail_write_auto_ack_fifo() or \ref sl_rail_ieee802154_write_enh_ack()
 * to inform RAIL of its final length.
 */
sl_rail_status_t sl_rail_get_auto_ack_fifo(sl_rail_handle_t rail_handle,
                                           uint8_t **pp_ack_buffer,
                                           uint16_t *p_ack_buffer_bytes);

/**
 * Set the address and size of the Auto-Ack transmit buffer,
 * possibly pre-populated with some Ack data.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in,out] p_ack_fifo An appropriately-aligned (see \ref sl_rail_set_tx_fifo description)
 *   pointer to a read-write memory location in RAM used as the transmit
 *   FIFO for acknowledgement packets. This memory must persist until the next
 *   call to this function. If NULL, will re-establish the built-in
 *   per-protocol FIFO of size \ref SL_RAIL_DEFAULT_AUTO_ACK_FIFO_BYTES (offset
 *   must be 0 in this case).
 * @param[in] size_bytes The desired size of the buffer in bytes.
 *   Must be greater than SL_RAIL_DEFAULT_AUTO_ACK_FIFO_BYTES and less
 *   than or equal to SL_RAIL_MAXIMUM_FIFO_BYTES.
 * @param[in] init_bytes The number of valid bytes already populated in the
 *   buffer. Must be less than or equal to size_bytes.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_set_auto_ack_fifo(sl_rail_handle_t rail_handle,
                                           sl_rail_fifo_buffer_align_t *p_ack_fifo,
                                           uint16_t size_bytes,
                                           uint16_t init_bytes);

/**
 * Pause/resume RX Auto-Ack functionality.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] pause Pause or resume RX Auto-Acking.
 * @return Status code indicating success of the function call.
 *
 * When RX Auto-Acking is paused, the radio transitions to
 * \ref sl_rail_auto_ack_config_t::rx_transitions's
 * \ref sl_rail_state_transitions_t::success state after receiving a packet and
 * does not transmit an Ack. When RX Auto-Ack is resumed, the radio resumes
 * automatically Acking every successfully received packet.
 */
sl_rail_status_t sl_rail_pause_rx_auto_ack(sl_rail_handle_t rail_handle,
                                           bool pause);

/**
 * Return whether the RX Auto-Ack is paused.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return true if RX Auto-Ack is paused, false if not paused.
 */
bool sl_rail_is_rx_auto_ack_paused(sl_rail_handle_t rail_handle);

/**
 * Pause/resume TX Auto-Ack functionality.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] pause Pause or resume TX Auto-Acking.
 * @return Status code indicating success of the function call.
 *
 * When TX Auto-Acking is paused, the radio transitions to
 * \ref sl_rail_auto_ack_config_t::tx_transitions's
 * \ref sl_rail_state_transitions_t::success state after transmitting a packet and
 * does not wait for an Ack. When TX Auto-Ack is resumed, the radio resumes
 * automatically waiting for an Ack after a successful transmit.
 */
sl_rail_status_t sl_rail_pause_tx_auto_ack(sl_rail_handle_t rail_handle,
                                           bool pause);

/**
 * Return whether the TX Auto-Ack is paused.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return true if TX Auto-Ack is paused, false if not paused.
 */
bool sl_rail_is_tx_auto_ack_paused(sl_rail_handle_t rail_handle);

/**
 * Modify the upcoming Ack to use the transmit FIFO.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call. The call will
 *   fail if it is too late to modify the outgoing Ack.
 *
 * This function allows the application to use the normal transmit FIFO as
 * the data source for the upcoming Ack. The Ack modification to use the
 * transmit FIFO only applies to one Ack transmission.
 *
 * This function only returns true if the following conditions are met:
 *   - Radio has not already decided to use the Ack buffer AND
 *   - Radio is either looking for sync, receiving the packet after sync, or in
 *     the \ref sl_rail_state_timing_t::rx_to_tx turnaround before the Ack is sent.
 *
 * @note The transmit FIFO must not be used for Auto-Ack when IEEE 802.15.4,
 *   Z-Wave, or BLE protocols are active.
 */
sl_rail_status_t sl_rail_use_tx_fifo_for_auto_ack(sl_rail_handle_t rail_handle);

/**
 * Cancel the upcoming Ack.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call. This call will
 *   fail if it is too late to modify the outgoing Ack.
 *
 * This function allows the application to cancel the upcoming automatic
 * acknowledgment.
 *
 * This function only returns true if the following conditions are met:
 *   - Radio has not already decided to transmit the Ack, and
 *   - Radio is either looking for sync, receiving the packet after sync or in
 *     the \ref sl_rail_state_timing_t::rx_to_tx turnaround before the Ack is sent.
 */
sl_rail_status_t sl_rail_cancel_auto_ack(sl_rail_handle_t rail_handle);

/**
 * Return whether the radio is currently waiting for an Ack.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return true if radio is waiting for Ack, false if radio is not waiting for
 *   an Ack.
 *
 * This function allows the application to query whether the radio is currently
 * waiting for an Ack after a transmit operation.
 */
bool sl_rail_is_auto_ack_waiting_for_ack(sl_rail_handle_t rail_handle);

/** @} */ // end of group Auto_Ack

/******************************************************************************
 * Calibration
 *****************************************************************************/
/// @addtogroup Calibration
/// @brief APIs for calibrating the radio
/// @{
///
/// These APIs calibrate the radio. The RAIL library
/// determines which calibrations are necessary. Calibrations can
/// be enabled/disabled with the \ref sl_rail_cal_mask_t parameter.
///
/// Some calibrations produce values that can be saved and reapplied to
/// avoid repeating the calibration process.
///
/// Calibrations can either be run with \ref sl_rail_calibrate(), or with the
/// individual chip-specific calibration routines. An example for running code
/// with \ref sl_rail_calibrate() looks like the following:
/// @code{.c}
/// static sl_rail_cal_values_t cal_values = SL_RAIL_CALVALUES_UNINIT;
/// static volatile bool calibrateRadio = false;
///
/// void rail_events_callback(sl_rail_handle_t rail_handle, sl_rail_events_t events)
/// {
///   // Omitting other event handlers
///   if (events & SL_RAIL_EVENT_CAL_NEEDED) {
///     calibrateRadio = true;
///   }
/// }
///
/// void main(void)
/// {
///   // Initialize RAIL ...
///
///   // Application main loop
///   while (1) {
///     ...
///     if (calibrateRadio) {
///       // Run all pending calibrations, and save the results
///       sl_rail_calibrate(rail_handle, &cal_values, SL_RAIL_CAL_ALL_PENDING);
///       calibrateRadio = false;
///     }
///     ...
///   }
/// }
/// @endcode
///
/// Alternatively, if the image rejection calibration for your radio can be
/// determined ahead of time, such as by running the calibration on a separate
/// firmware image on each chip, the following calibration process will
/// result in smaller code.
/// @code{.c}
/// static sl_rail_ir_cal_values_t ir_cal_values = {
///   .rx_ir_cal_values = {
///     RX_IRCAL_VALUE_RF_PATH0,
///     RX_IRCAL_VALUE_RF_PATH1,
///   },
///   .tx_ir_cal_values = {
///     .dc_offset_iq = TX_IRCAL_DC_OFFSET_IQ,
///     .phi_epsilon  = TX_IRCAL_PHI_EPSILON,
///   },
/// };
///
/// void rail_events_handler(sl_rail_handle_t rail_handle, sl_rail_events_t events)
/// {
///   // Omitting other event handlers
///   if (events & SL_RAIL_EVENT_CAL_NEEDED) {
///     sl_rail_cal_mask_t pending_cals = sl_rail_get_pending_cal(rail_handle);
///     // Disable the radio if we have to do an offline calibration
///     if (pending_cals & SL_RAIL_CAL_TEMP_VC0) {
///       (void) sl_rail_calibrate_temp(rail_handle);
///     }
///     if (pending_cals & SL_RAIL_CAL_ONETIME_IRCAL) {
///       (void) sl_rail_apply_ir_calibration(rail_handle, &ir_cal_values, SL_RAIL_ANTENNA_AUTO);
///     }
///   }
/// }
/// @endcode

/**
 * Initialize RAIL calibration.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] cal_enable_mask A bitmask that indicates which calibrations
 *   to enable for a callback notification.
 *   The exact meaning of these bits is radio-specific.
 * @return Status code indicating success of the function call.
 *
 * Calibration initialization provides the calibration settings that
 * correspond to the current radio configuration.
 */
sl_rail_status_t sl_rail_config_cal(sl_rail_handle_t rail_handle,
                                    sl_rail_cal_mask_t cal_enable_mask);

/**
 * Start the calibration process.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in,out] p_cal_values A pointer to a structure of calibration values to apply.
 *   If a valid calibration structure is provided and the structure
 *   contains valid calibration values, those values will be applied to the
 *   hardware and the RAIL library will cache those values for use again later.
 *   If a valid calibration structure is provided and the structure
 *   contains a calibration value of \ref SL_RAIL_CAL_INVALID_VALUE for the
 *   desired calibration, the desired calibration will run, the calibration
 *   values structure will be updated with a valid calibration value, and the
 *   RAIL library will cache that value for use again later.
 *   If a NULL pointer is provided, the desired calibration will run
 *   and the RAIL library will cache that value for use again later. However,
 *   the valid calibration value will not be returned to the application.
 * @param[in] cal_force_mask A mask to force specific calibration(s) to execute.
 *   To run all pending calibrations, use the value \ref SL_RAIL_CAL_ALL_PENDING.
 *   Only the calibrations specified will run, even if not enabled during
 *   initialization.
 * @return Status code indicating success of the function call.
 *
 * If calibrations were performed previously and the application saves the
 * calibration values (i.e., call this function with a calibration values
 * structure containing calibration values of \ref SL_RAIL_CAL_INVALID_VALUE
 * before a reset), the application can later bypass the time it would normally
 * take to recalibrate hardware by reusing previous calibration values (i.e.,
 * call this function with a calibration values structure containing valid
 * calibration values after a reset).
 *
 * Silicon Labs recommends calling this function from the application main loop.
 *
 * If multiple protocols are used, this function will make the given rail_handle
 * active, if not already, and perform calibration. If called during a protocol
 * switch, to perform an IR calibration for the first time, it will
 * return \ref SL_RAIL_STATUS_INVALID_STATE, in which case the application must
 * defer calibration until after the protocol switch is complete.
 *
 * @note Instead of this function, consider using the individual calibration-specific
 *   functions. Using the individual functions will allow for better
 *   dead-stripping if not all calibrations are run.
 * @note Some calibrations should only be executed when the radio is IDLE. See
 *   chip-specific documentation for more details.
 */
sl_rail_status_t sl_rail_calibrate(sl_rail_handle_t rail_handle,
                                   sl_rail_cal_values_t *p_cal_values,
                                   sl_rail_cal_mask_t cal_force_mask);

/**
 * Return the current set of pending calibrations.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return A mask of all pending calibrations that the user has been asked to
 *   perform.
 *
 * This function returns a full set of pending calibrations. The only way
 * to clear pending calibrations is to perform them using the \ref
 * sl_rail_calibrate() API with the appropriate list of calibrations.
 */
sl_rail_cal_mask_t sl_rail_get_pending_cal(sl_rail_handle_t rail_handle);

/**
 * Apply given image rejection calibration values.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_image_rejection A pointer to the image rejection values to apply.
 * @param[in] rf_path RF path(s) to calibrate.
 * @return Status code indicating success of the function call.
 *
 * Take image rejection calibration values and apply them. These values should be
 * determined from a previous run of \ref sl_rail_calibrate_ir() on the same
 * physical device with the same radio configuration. The p_image_rejection values
 * will also be stored to the \ref sl_rail_channel_config_entry_t::p_attr, if possible.
 *
 * @note: To make sure the p_image_rejection value is stored/configured correctly,
 *   \ref sl_rail_config_antenna() should be called before calling this API.
 *
 * If multiple protocols are used, this function will return
 * \ref SL_RAIL_STATUS_INVALID_STATE if it is called and the given rail_handle is
 * not active. In that case, the caller must attempt to re-call this function later.
 */
sl_rail_status_t sl_rail_apply_ir_calibration(sl_rail_handle_t rail_handle,
                                              sl_rail_ir_cal_values_t *p_image_rejection,
                                              sl_rail_antenna_sel_t rf_path);

/**
 * Run the image rejection calibration.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_image_rejection A pointer to the image rejection results.
 * @param[in] rf_path RF path(s) to calibrate.
 * @return Status code indicating success of the function call.
 *
 * Run the image rejection calibration and apply the resulting values. If the
 * p_image_rejection parameter is not NULL, store the values at that
 * location. The p_image_rejection values will also be stored to the
 * \ref sl_rail_channel_config_entry_t::p_attr, if possible. This is a long-running
 * calibration that adds significant code space when run and can be run with a
 * separate firmware image on each device to save code space in the
 * final image. Silicon Labs recommends calling this function from the
 * application main loop.
 *
 * @note: To make sure the p_image_rejection values are stored/configured correctly,
 *   \ref sl_rail_config_antenna() should be called before calling this API.
 *
 * If multiple protocols are used, this function will return
 * \ref SL_RAIL_STATUS_INVALID_STATE if it is called and the given rail_handle is
 * not active. In that case, the caller must attempt to re-call this function later.
 */
sl_rail_status_t sl_rail_calibrate_ir(sl_rail_handle_t rail_handle,
                                      sl_rail_ir_cal_values_t *p_image_rejection,
                                      sl_rail_antenna_sel_t rf_path);

/**
 * Run the temperature calibration.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * Run the temperature calibration, which needs to recalibrate the synth if
 * the temperature crosses 0C or the temperature delta since the last
 * calibration exceeds 70C while in receive. RAIL will run the VCO calibration
 * automatically upon entering receive or transmit states, so the application
 * can omit this calibration if the stack re-enters receive or transmit with
 * enough frequency to avoid reaching the temperature delta. If the application
 * does not calibrate for temperature, it's possible to miss receive packets due
 * to a drift in the carrier frequency.
 *
 * If multiple protocols are used, this function will return
 * \ref SL_RAIL_STATUS_INVALID_STATE if it is called and the given rail_handle is
 * not active. In that case, the calibration will be automatically performed
 * next time the radio enters receive.
 *
 * @note If RX channel hopping is enabled this function may trigger a
 *   channel hop, which can result in \ref
 *   SL_RAIL_EVENT_RX_CHANNEL_HOPPING_COMPLETE occuring before it returns.
 */
sl_rail_status_t sl_rail_calibrate_temp(sl_rail_handle_t rail_handle);

/**
 * Performs HFXO compensation.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_crystal_error_ppm A pointer for RAIL to store the current
 *   deviation that has been corrected, measured in PPM. May be NULL.
 * @return Status code indicating the result of the function call.
 *
 * Compute the PPM correction using the thermistor value available when
 * \ref SL_RAIL_EVENT_THERMISTOR_DONE occurs, after
 * \ref sl_rail_start_thermistor_measurement() call.
 * Then correct the RF frequency as well as TX and RX sampling.
 *
 * This function calls the following RAIL functions in sequence saving having
 * to call them individually:
 *   - \ref sl_railcb_convert_thermistor_impedance()
 *   - \ref sl_railcb_compute_hfxo_error_ppm()
 *   - \ref sl_rail_compensate_hfxo()
 *
 * @note This function makes the radio idle.
 */
sl_rail_status_t sl_rail_calibrate_hfxo(sl_rail_handle_t rail_handle,
                                        int8_t *p_crystal_error_ppm);

/**
 * Enable/disable the PA calibration.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] enable Enables/disables the PA calibration.
 *
 * Enabling will ensure that the PA power remains constant chip-to-chip.
 * This feature is enabled by default after reset.
 *
 * On EFR32xG21 and EFR32xG24 platforms, particularly the 20 dBm variants,
 * it is recommended to keep this feature enabled in order to utilize the
 * chip-specific calibrations designated for the chip.
 *
 * The chip-specific define SL_RAIL_UTIL_PA_CALIBRATION_ENABLE in the
 * \ref rail_util_pa plugin also enables/disables PA calibrations
 * on initialization, which can override the default state of the feature.
 *
 * @note Call this function before \ref sl_rail_config_tx_power() if this
 *   feature is not desired.
 */
void sl_rail_enable_pa_cal(sl_rail_handle_t rail_handle, bool enable);

/** @} */ // end of group Calibration

/******************************************************************************
 * RF Sense
 *****************************************************************************/
/**
 * @addtogroup Rf_Sense RF Sense
 * @{
 */

/**
 * Start/stop the RF Sense functionality in Energy Detection Mode for use
 * during low-energy sleep modes.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in] band The frequency band(s) on which to sense the RF energy.
 *   To stop RF Sense, specify \ref SL_RAIL_RF_SENSE_OFF.
 * @param[in] sense_time_us The time (in microseconds) the RF energy must be
 *   continually detected to be considered "sensed".
 * @param[in] cb \ref sl_rail_rf_sense_callback_t() is called when the RF is
 *   sensed. Set NULL if using \ref SL_RAIL_EVENT_RF_SENSED or polling via
 *   \ref sl_rail_is_rf_sensed().
 * @return The actual sense_time_us used, which may be different than
 *   requested due to limitations of the hardware. If 0, RF sense was
 *   disabled or could not be enabled (no callback or event will be issued).
 *
 * Some EFR32 radios have the ability to sense the presence of RF energy above -20 dBm
 * within either or both the 2.4 GHz and Sub-GHz bands and trigger an event
 * if that energy is continuously present for certain durations of time. An
 * application can check when RF energy is sensed either by enabling the event
 * \ref SL_RAIL_EVENT_RF_SENSED, by polling on the \ref sl_rail_is_rf_sensed() API, or
 * by using the cb callback.
 *
 * @note After RF energy has been sensed, the RF Sense is automatically
 *   disabled and \ref sl_rail_start_rf_sense() must be called again to reactivate it.
 *   If RF energy has not been sensed and to manually disable RF Sense,
 *   \ref sl_rail_start_rf_sense() must be called with band specified as
 *   \ref SL_RAIL_RF_SENSE_OFF or with sense_time_us set to 0 microseconds.
 *
 * @note Packet reception is not guaranteed to work correctly once RF Sense is
 *   enabled, both in single protocol and multiprotocol RAIL.
 *   To be safe, an application should turn this on only after idling
 *   the radio to stop receive and turn it off before attempting to restart
 *   receive. Since EM4 sleep causes the chip to come up through the reset
 *   vector any wake from EM4 must also shut off RF Sense to ensure proper
 *   receive functionality.
 *
 * @warning For some radios, RF Sense functionality is only guaranteed within
 *   a specified temperature range.
 *   See chip-specific documentation for more details.
 */
sl_rail_time_t sl_rail_start_rf_sense(sl_rail_handle_t rail_handle,
                                      sl_rail_rf_sense_band_t band,
                                      sl_rail_time_t sense_time_us,
                                      sl_rail_rf_sense_callback_t cb);

///
/// Start/stop the RF Sense functionality in Selective(OOK Based) Mode for use
/// during low-energy sleep modes.
///
/// @param[in] rail_handle A real RAIL instance handle.
/// @param[in] p_config A pointer to a \ref sl_rail_rf_sense_selective_ook_config_t
///   which holds the RF Sense configuration for Selective(OOK) mode.
/// @return Status code indicating success of the function call.
///
/// Some radios support Selective RF energy detection (OOK mode) where the
/// user can program the radio to look for a particular sync word pattern
/// (1-4 bytes) sent using OOK and wake only when that is detected.
/// See chip-specific documentation for more details.
///
/// The following code gives an example of how to use RF Sense functionality
/// in Selective(OOK Based) Mode.
/// @code{.c}
/// // Sync word length in bytes, 1-4 bytes.
/// #define NUM_SYNC_WORD_BYTES (2U)
/// // Sync word value.
/// #define SYNC_WORD           (0xB16FU)
/// // Desired RF band
/// sl_rail_rf_sense_band_t rf_band = SL_RAIL_RF_SENSE_2P4_GHZ;
///
/// // Configure the transmitting node for sending the wakeup packet.
/// sl_rail_idle(rail_handle, SL_RAIL_IDLE_ABORT, true);
/// sl_rail_config_rf_sense_selective_ook_wakeup_phy(rail_handle);
/// sl_rail_set_rf_sense_selective_ook_wakeup_payload(rail_handle, NUM_SYNC_WORD_BYTES, SYNC_WORD);
/// sl_rail_start_tx(rail_handle, channel, SL_RAIL_TX_OPTIONS_DEFAULT, NULL);
///
/// // Configure the receiving node (EFR32xG22) for RF Sense.
/// sl_rail_rf_sense_selective_ook_config_t config = {
///  .band = rf_band,
///  .sync_word_bytes = NUM_SYNC_WORD_BYTES,
///  .sync_word = SYNC_WORD,
///  .cb = NULL // Use SL_RAIL_EVENT_RF_SENSED event or poll sl_rail_is_rf_sensed()
/// };
/// sl_rail_start_selective_ook_rf_sense(rail_handle, &config);
/// @endcode
///
/// @note After RF energy has been sensed, the RF Sense is automatically
///   disabled and \ref sl_rail_start_selective_ook_rf_sense() must be called again to
///   reactivate. If RF energy has not been sensed and to manually disable
///   RF Sense, \ref sl_rail_start_selective_ook_rf_sense() must be called with band
///   specified as \ref SL_RAIL_RF_SENSE_OFF or with
///   \ref sl_rail_rf_sense_selective_ook_config_t as NULL.
///
/// @note Packet reception is not guaranteed to work correctly once RF Sense is
///   enabled, both in single protocol and multiprotocol RAIL.
///   To be safe, an application should turn this on only after idling
///   the radio to stop receive and turn it off before attempting to restart
///   receive. Since EM4 sleep causes the chip to come up through the reset
///   vector any wake from EM4 must also shut off RF Sense to ensure proper
///   receive functionality.
///
sl_rail_status_t sl_rail_start_selective_ook_rf_sense(sl_rail_handle_t rail_handle,
                                                      sl_rail_rf_sense_selective_ook_config_t *p_config);

/**
 * Switch to RF Sense Selective(OOK) PHY.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This function switches to the RF Sense Selective(OOK) PHY for transmitting a
 * packet to wake up a chip that supports Selective RF energy detection (OOK
 * mode). You may only call this function while the radio is idle. While the
 * radio is configured for this PHY, receive functionality should not be used.
 *
 * @note The user must also set up the transmit FIFO, via
 *   \ref sl_rail_set_rf_sense_selective_ook_wakeup_payload, post this function call to
 *   include the first byte as the Preamble byte, followed by the
 *   Sync word (1-4 bytes).
 *   See chip-specific documentation for more details.
 */
sl_rail_status_t sl_rail_config_rf_sense_selective_ook_wakeup_phy(sl_rail_handle_t rail_handle);

/**
 * Set the transmit payload for waking up a node configured for
 * RF Sense Selective(OOK).
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] sync_word_bytes Sync word length in bytes, 1-4 bytes.
 * @param[in] sync_word Sync word value.
 * @return Status code indicating success of the function call.
 *
 * @note You must call this function after the chip has been set up with the
 *   RF Sense Selective(OOK) PHY, using \ref sl_rail_config_rf_sense_selective_ook_wakeup_phy.
 */
sl_rail_status_t sl_rail_set_rf_sense_selective_ook_wakeup_payload(sl_rail_handle_t rail_handle,
                                                                   uint8_t sync_word_bytes,
                                                                   uint32_t sync_word);

/**
 * Check whether the RF was sensed.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if RF was sensed since the last call to \ref sl_rail_start_rf_sense().
 *   false otherwise.
 *
 * This function is useful if \ref sl_rail_start_rf_sense() is called with a NULL
 * callback. It is generally used after EM4 reboot but can be used any time.
 */
bool sl_rail_is_rf_sensed(sl_rail_handle_t rail_handle);

/**
 * The RF Sense high-sensitivity threshold value used by the next
 * \ref sl_rail_start_rf_sense() or \ref
 * sl_rail_start_selective_ook_rf_sense() call, when not specifying
 * a low-sensitivity \ref sl_rail_rf_sense_band_t.
 * The units are platform-dependent and range from 0 (highest sensitivity)
 * to 255 (lowest sensitivity). This should be less than \ref
 * sl_rail_rf_sense_low_sensitivity_value.
 * RAIL initializes this to a default setting.
 */
extern uint8_t sl_rail_rf_sense_high_sensitivity_value;

/**
 * The RF Sense low-sensitivity threshold value used by the next
 * \ref sl_rail_start_rf_sense() or \ref
 * sl_rail_start_selective_ook_rf_sense() call, when specifying
 * a low-sensitivity \ref sl_rail_rf_sense_band_t.
 * The units are platform-dependent and range from 0 (highest sensitivity)
 * to 255 (lowest sensitivity). This should be greater than \ref
 * sl_rail_rf_sense_high_sensitivity_value.
 * RAIL initializes this to a default setting.
 */
extern uint8_t sl_rail_rf_sense_low_sensitivity_value;

/** @} */ // end of group Rf_Sense

/******************************************************************************
 * RX Channel Hopping
 *****************************************************************************/
/**
 * @addtogroup Rx_Channel_Hopping RX Channel Hopping
 * @brief Hardware accelerated hopping between channels while waiting for a
 *   packet in receive.
 * @{
 *
 * Channel hopping provides a hardware accelerated method for
 * scanning across multiple channels quickly, as part of a receive protocol.
 * While it is possible to call \ref sl_rail_start_rx() on different channels,
 * back to back, and listen on many channels sequentially in that way, the
 * time it takes to switch channels with that method may be too long for some
 * protocols. This API pre-computes necessary channel change operations
 * for a given list of channels, so that the radio can move from channel
 * to channel much faster. Additionally, it leads to more succinct code
 * as channel changes will be done implicitly, without requiring numerous calls
 * to \ref sl_rail_start_rx(). Currently, while this feature is enabled, the radio
 * will hop channels in the given sequence each time it enters RX.
 * Note that RX Channel hopping and EFR32xG25's concurrent mode / collision
 * detection are mutually exclusive.
 *
 * The channel hopping buffer requires \ref SL_RAIL_CHANNEL_HOPPING_BUFFER_WORDS_PER_CHANNEL
 * number of 32-bit words of overhead per channel, plus 3 words overall plus the
 * twice the size of the \ref sl_rail_channel_config_t::phy_config_delta_subtract
 * of the whole radio configuration, plus the twice the sum of the sizes of all
 * the \ref sl_rail_channel_config_entry_t::phy_config_delta_add inall the channel
 * hopping channels.
 *
 * The following code gives an example of how to use
 * the RX Channel Hopping API.
 * @code{.c}
 * #define CHANNEL_HOPPING_NUMBER_OF_CHANNELS 4
 * #define CHANNEL_HOPPING_BUFFER_WORDS do {           \
 *   3 +                                               \
 *   (SL_RAIL_CHANNEL_HOPPING_BUFFER_WORDS_PER_CHANNEL \
 *    * CHANNEL_HOPPING_NUMBER_OF_CHANNELS) +          \
 *   2 * (SIZEOF_UINT32_DELTA_SUBTRACT +               \
 *   SIZEOF_UINT32_DELTA_ADD_0 +                       \
 *   SIZEOF_UINT32_DELTA_ADD_1 +                       \
 *   SIZEOF_UINT32_DELTA_ADD_2 +                       \
 *   SIZEOF_UINT32_DELTA_ADD_3)                        \
 * } while (0)
 *
 * sl_rail_rx_channel_hopping_config_entry_t channel_hopping_entries[CHANNEL_HOPPING_NUMBER_OF_CHANNELS];
 * uint32_t channel_hopping_buffer[CHANNEL_HOPPING_BUFFER_WORDS];
 *
 * sl_rail_rx_channel_hopping_config_t channel_hopping_config = {
 *   .buffer = channel_hopping_buffer,
 *   .buffer_words = CHANNEL_HOPPING_BUFFER_WORDS,
 *   .number_of_channels = CHANNEL_HOPPING_NUMBER_OF_CHANNELS,
 *   .entries = channel_hopping_entries,
 * };
 *
 * channel_hopping.entries[0].channel = 1;
 * channel_hopping.entries[1].channel = 2;
 * channel_hopping_entries[2].channel = 3;
 *
 * sl_rail_config_rx_channel_hopping(rail_handle, &channel_hopping_config);
 * sl_rail_enable_rx_channel_hopping(rail_handle, true, true)
 * @endcode
 */

/**
 * Configure RX channel hopping.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_rx_hopping_config A pointer to the configuration parameters for RX Channel Hopping.
 * @return Status code indicating success of the function call.
 *
 * Configure channel hopping channels, conditions, and parameters. This
 * API must be called before \ref sl_rail_enable_rx_channel_hopping(). This API must
 * never be called while the radio is on with RX Duty Cycle or Channel
 * Hopping enabled.
 *
 * @note Use the compile time symbol \ref SL_RAIL_SUPPORTS_CHANNEL_HOPPING or
 *   the runtime call \ref sl_rail_supports_channel_hopping() to check whether
 *   the platform supports this feature.
 *
 * @note Calling this function will overwrite any settings configured with
 *   \ref sl_rail_config_rx_duty_cycle().
 */
sl_rail_status_t sl_rail_config_rx_channel_hopping(sl_rail_handle_t rail_handle,
                                                   sl_rail_rx_channel_hopping_config_t *p_rx_hopping_config);

/**
 * Enable RX channel hopping.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] enable Enable (true) or disable (false) RX Channel Hopping.
 * @param[in] reset Start from the first channel of the channel hopping
 *   sequence (true) or from wherever hopping left off last time the code
 *   left RX.
 * @return Status code indicating success of the function call.
 *
 * Enable or disable Channel Hopping. Additionally, specify whether hopping
 * should be reset to start from the first channel index, or continue
 * from the channel last hopped to. The radio should not be on when
 * this API is called. \ref sl_rail_config_rx_channel_hopping() must be called
 * successfully before this API is called.
 *
 * @note Use the compile time symbol \ref SL_RAIL_SUPPORTS_CHANNEL_HOPPING or
 *   the runtime call \ref sl_rail_supports_channel_hopping() to check whether
 *   the platform supports this feature.
 *
 * @note RX Channel Hopping may not be enabled while Auto-Acking is enabled.
 *
 * @note Calling this function will overwrite any settings configured with
 *   \ref sl_rail_enable_rx_duty_cycle().
 */
sl_rail_status_t sl_rail_enable_rx_channel_hopping(sl_rail_handle_t rail_handle,
                                                   bool enable,
                                                   bool reset);

/**
 * Manually trigger an Rx channel hop.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This function facilitates a manual hop when using \ref
 * SL_RAIL_RX_CHANNEL_HOPPING_MODE_MANUAL. (It will also trigger a hop
 * in other automatic hop modes as well, so use with caution.)
 *
 * @note This function may cause \ref SL_RAIL_EVENT_RX_CHANNEL_HOPPING_COMPLETE
 *   to occur before it returns.
 */
sl_rail_status_t sl_rail_trigger_rx_channel_hop(sl_rail_handle_t rail_handle);

/**
 * Get RSSI in quarter-dBm of one channel in the channel hopping sequence, during
 * channel hopping.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] channel_index Index in the channel hopping sequence of the
 *   channel of interest.
 * @return Latest RSSI in quarter-dBm (dBm * 4) for the channel at the
 *   specified index.
 *
 * @note Use the compile time symbol \ref SL_RAIL_SUPPORTS_CHANNEL_HOPPING or
 *   the runtime call \ref sl_rail_supports_channel_hopping() to check whether
 *   the platform supports this feature.
 *
 * @note In multiprotocol, this function returns \ref SL_RAIL_RSSI_INVALID
 *   immediately if rail_handle is not the currently active \ref sl_rail_handle_t.
 *
 * @note \ref sl_rail_config_rx_channel_hopping() must be called successfully
 *   before this API is called.
 *
 * @note When the Z-Wave protocol is active, running
 *   \ref sl_rail_get_channel_hopping_rssi() on the 9.6 kbps PHY returns the RSSI
 *   measurement of the 40kpbs PHY. This is because the 9.6 kbps PHY has
 *   trouble with RSSI measurements on EFR32xG2x family of chips.
 */
int16_t sl_rail_get_channel_hopping_rssi(sl_rail_handle_t rail_handle,
                                         uint8_t channel_index);

///
/// Configure RX duty cycle mode.
///
/// @param[in] rail_handle A real RAIL instance handle.
/// @param[in] p_rx_duty_cycle_config A non-NULL pointer to a configuration structure specifying duty cycle parameters.
/// @return Status code indicating success of the function call.
///
/// Configure RX duty cycle mode. With this mode enabled, every time the radio
/// enters RX, it will duty cycle on and off to save power. The duty cycle
/// ratio can be altered dynamically and intelligently by the hardware by
/// staying on longer if a preamble or other packet segments are detected in
/// the air. This API must never be called while the radio is on with RX Duty
/// Cycle or Channel Hopping enabled.
/// For short delays (in the order of microseconds),
/// \ref sl_rail_rx_duty_cycle_config_t::delay_us, this can be used to save receive
/// current while having little impact on the radio performance, for protocols
/// with long preambles. For long delays (in the order of milliseconds or higher)
/// the chip can be put into EM2 energy mode before re-entering RX,
/// to save extra power, with some application hooks as shown below.
///
/// @code{.c}
/// #include "sl_rail.h"
///
/// extern sl_rail_handle_t rail_handle;
/// sl_rail_time_t periodic_wakeup_us;
///
/// volatile bool duty_cycle_end = true;
///
/// void rail_events_callback(sl_rail_handle_t rail_handle, sl_rail_events_t events)
/// {
///   // Omitting other event handlers
///   if (events & SL_RAIL_EVENT_RX_DUTY_CYCLE_RX_END) {
///     duty_cycle_end = true;
///   }
/// }
///
/// void main(void)
/// {
///   sl_rail_status_t status;
///   bool should_sleep = false;
///
///   initialize_rail(); // Call sl_rail_init() and such
///
///   // This function depends on your board/chip but it must enable the LFCLK
///   // you intend to use for RTCC sync before we configure sleep as that
///   // function will attempt to auto detect the clock.
///   Board_Setup_LFCLK();
///   // Initialize Power Manager module
///   sl_power_manager_init();
///   // Initialize RAIL Power Manager
///   sl_rail_init_power_manager();
///
///   // Configure sleep for timer synchronization
///   sl_rail_timer_sync_config_t timer_sync_config = SL_RAIL_TIMER_SYNC_DEFAULT;
///   status = sl_rail_config_sleep(rail_handle, &timer_sync_config);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///
///   // Configure RX duty-cycling
///   sl_rail_rx_duty_cycle_config_t duty_cycle_config = {
///     .mode = SL_RAIL_RX_CHANNEL_HOPPING_MODE_PREAMBLE_SENSE,
///     .parameter = DUTY_CYCLE_ON_TIME,
///     .delay_us = DUTY_CYCLE_OFF_TIME,
///     .delay_mode = SL_RAIL_RX_CHANNEL_HOPPING_DELAY_MODE_STATIC,
///     .options = SL_RAIL_RX_CHANNEL_HOPPING_OPTIONS_NONE,
///   };
///   status = sl_rail_config_rx_duty_cycle(rail_handle, &duty_cycle_config);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///   status = sl_rail_enable_rx_duty_cycle(rail_handle, true);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///
///   // Application main loop
///   while (1) {
///     if (duty_cycle_end) {
///       duty_cycle_end = false;
///       // Schedule the next receive.
///       sl_rail_scheduled_rx_config_t rx_config = {
///         .start = periodic_wakeup_us,
///         .start_mode = SL_RAIL_TIME_DELAY,
///         .end = 0U,
///         .end_mode = SL_RAIL_TIME_DISABLED,
///         .rx_transition_end_schedule = 0U,
///         .hard_window_end = 0U,
///       };
///       sl_rail_idle(rail_handle, SL_RAIL_IDLE_ABORT, true);
///       sl_rail_start_scheduled_rx(rail_handle, channel, &rx_config, NULL);
///     }
///     // ... do normal app stuff and set should_sleep when we want to sleep
///     if (should_sleep) {
///       // Let the CPU go to sleep if the system allows it.
///       sl_power_manager_sleep();
///     }
///   }
/// }
/// @endcode
///
/// @note Use the compile time symbol \ref SL_RAIL_SUPPORTS_CHANNEL_HOPPING or
///   the runtime call \ref sl_rail_supports_channel_hopping() to check whether
///   the platform supports this feature.
///
/// @note Calling this function will overwrite any settings configured with
///   \ref sl_rail_config_rx_channel_hopping().
///
/// @note On Series 3 parts, to enable RX duty cycling, use the
///    sl_rail_util_ieee802154_phy_select and
///    sl_rail_util_ieee802154_rx_duty_cycling components instead.
///
sl_rail_status_t sl_rail_config_rx_duty_cycle(sl_rail_handle_t rail_handle,
                                              const sl_rail_rx_duty_cycle_config_t *p_rx_duty_cycle_config);

/**
 * Enable RX duty cycle mode.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] enable Enable (true) or disable (false) RX Duty Cycling.
 * @return Status code indicating success of the function call.
 *
 * Enable or disable RX duty cycle mode. After this is called, the radio
 * will begin duty cycling each time it enters RX, based on the
 * configuration passed to \ref sl_rail_config_rx_duty_cycle(). This API must not
 * be called while the radio is on.
 *
 * @note Use the compile time symbol \ref SL_RAIL_SUPPORTS_CHANNEL_HOPPING or
 *   the runtime call \ref sl_rail_supports_channel_hopping() to check whether
 *   the platform supports this feature.
 *
 * @note calling this function will overwrite any settings
 *   configured with \ref sl_rail_enable_rx_channel_hopping().
 *
 * @note On Series 3 parts, to enable RX duty cycling, use the
 *   sl_rail_util_ieee802154_phy_select and
 *   sl_rail_util_ieee802154_rx_duty_cycling components instead.
 */
sl_rail_status_t sl_rail_enable_rx_duty_cycle(sl_rail_handle_t rail_handle,
                                              bool enable);

/**
 * Get the default RX duty cycle configuration.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_rx_duty_cycle_config An application-provided non-NULL pointer to store
 *   the default RX duty cycle configuration.
 * @return Status code indicating success of the function call.
 *   Note that \ref SL_RAIL_STATUS_INVALID_PARAMETER will be returned if the current
 *   channel's radio configuration does not support the requested information.
 *
 * To save power during RX, an application may want to go to low power as long as
 * possible by periodically waking up and trying to
 * "sense" if there are any incoming packets. This API returns the recommended
 * RX duty cycle configuration, so the application can enter low power mode
 * periodically without missing packets. To wake up
 * earlier, the application can reduce the delay parameter.
 * Note that these value might be different if any configuration / channel has
 * changed.
 **/
sl_rail_status_t sl_rail_get_default_rx_duty_cycle_config(sl_rail_handle_t rail_handle,
                                                          sl_rail_rx_duty_cycle_config_t *p_rx_duty_cycle_config);

/** @} */ // end of group Rx_Channel_Hopping

/******************************************************************************
 * Multiprotocol-Specific Functions
 *****************************************************************************/
/**
 * @addtogroup Multiprotocol
 * @brief Multiprotocol scheduler APIs to support multiple time-sliced PHYs.
 * @{
 */

/**
 * Yield the radio to other configurations.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * This function is used to indicate that the previous transmit or scheduled
 * receive operation has completed. It must be used in multiprotocol RAIL because
 * the scheduler assumes that any transmit or receive operation that is started
 * can go on indefinitely based on state transitions and your protocol.
 * RAIL will not allow a lower priority tasks to run until this is called so it
 * can negatively impact performance of those protocols if this is omitted or
 * delayed. It is also possible to call the \ref sl_rail_idle() API to
 * both terminate the operation and idle the radio. In single protocol RAIL
 * this API does nothing, however, if RAIL Power Manager is initialized,
 * calling \ref sl_rail_yield_radio() after scheduled TX/RX and instantaneous TX
 * completion, is required, to indicate to the Power Manager that the the radio
 * is no longer busy and can be idled for sleeping.
 *
 * See \ref rail_radio_scheduler_yield for more details.
 */
sl_rail_status_t sl_rail_yield_radio(sl_rail_handle_t rail_handle);

/**
 * Get the status of the RAIL scheduler, specific to the radio operation,
 * along with \ref sl_rail_status_t returned by RAIL API invoked by the
 * RAIL scheduler.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_scheduler_status An application-provided pointer to store
 *   \ref sl_rail_scheduler_status_t status. Can be NULL as long as p_rail_status
 *   is not NULL.
 * @param[out] p_rail_status An application-provided pointer to store
 *   \ref sl_rail_status_t of the RAIL API invoked by the RAIL scheduler.
 *   Can be NULL as long as p_scheduler_status is not NULL.
 * @return Status code indicating success of the function call.
 *
 * This function can only be called from a callback context after the
 * \ref SL_RAIL_EVENT_SCHEDULER_STATUS event occurs.
 */
sl_rail_status_t sl_rail_get_scheduler_status(sl_rail_handle_t rail_handle,
                                              sl_rail_scheduler_status_t *p_scheduler_status,
                                              sl_rail_status_t *p_rail_status);

/**
 * Change the priority of a specified task type in multiprotocol.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] priority Desired new priority for the rail_handle's active task
 * @param[in] task_type Type of task whose priority should be updated
 * @return Status code indicating success of the function call.
 *
 * While the application can use this function however it likes, a major use
 * case is being able to increase an infinite receive priority while receiving
 * a packet. In other words, a given \ref sl_rail_handle_t can maintain a very low
 * priority background receive, but upon getting a
 * \ref SL_RAIL_EVENT_RX_SYNC_0_DETECT_SHIFT or
 * \ref SL_RAIL_EVENT_RX_SYNC_1_DETECT_SHIFT event, the app can call this function
 * to increase the background RX priority to lower the risk another protocol
 * might preempt during packet reception.
 */
sl_rail_status_t sl_rail_set_task_priority(sl_rail_handle_t rail_handle,
                                           uint8_t priority,
                                           sl_rail_task_type_t task_type);

/**
 * Get the time needed to switch between protocols.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @return \ref sl_rail_time_t Time, in microseconds, needed to switch between protocols.
 *
 * @note This API provides an accurate transition time by accounting for
 * system overhead. The transition time determines how early the scheduler
 * starts processing a protocol switch relative to a scheduled event.
 */
sl_rail_time_t sl_rail_get_transition_time(sl_rail_handle_t radio_handle);

/**
 * Set the time needed to switch between protocols. Call this API
 * only once, before any protocol is initialized via
 * \ref sl_rail_init(). Changing this value during normal operation
 * can result in improper scheduling behavior.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @param[in] transition_time_us Time, in microseconds, needed to switch between protocols.
 * @return Status code indicating success of the function call.
 *
 * @note This API provides an accurate transition time by accounting for
 * system overhead. The transition time determines how early the scheduler starts processing
 * a protocol switch relative to a scheduled event. Setting this value too
 * low may cause scheduling issues or missed events.
 */
sl_rail_status_t sl_rail_set_transition_time(sl_rail_handle_t radio_handle,
                                             sl_rail_time_t transition_time_us);

/** @} */ // end of group Multiprotocol

/******************************************************************************
 * Diagnostic
 *****************************************************************************/
/**
 * @addtogroup Diagnostic
 * @brief APIs for diagnostic and test chip modes
 * @{
 */

/**
 * Configure direct mode for RAIL.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_direct_mode_config A pointer to a configuration structure to specify direct mode
 *   parameters. Default configuration will be used if NULL is passed.
 * @return \ref SL_RAIL_STATUS_NO_ERROR on success and an error code on failure.
 *
 * This API configures direct mode and should be called before
 * calling \ref sl_rail_enable_direct_mode(). If this function is not called, the
 * following default configuration will be used: \n
 * @code{.c}
 * sl_rail_direct_mode_config_t default_config = {
 *   .sync_rx   = false,
 *   .sync_tx   = false,
 *   .dout_port = SL_GPIO_PORT_A,
 *   .dout_pin  = 5,
 *   .din_port  = SL_GPIO_PORT_A,
 *   .din_pin   = 7,
 *   .dclk_port = SL_GPIO_PORT_A,
 *   .dclk_pin  = 6,
 * };
 * @endcode
 *
 * @warning This API is not safe to use in a multiprotocol app.
 */
sl_rail_status_t sl_rail_config_direct_mode(sl_rail_handle_t rail_handle,
                                            const sl_rail_direct_mode_config_t *p_direct_mode_config);

/**
 * Enable or disable direct mode for RAIL.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] enable_direct_tx Enable direct mode for data being transmitted out
 *   of the radio.
 * @param[in] enable_direct_rx Enable direct mode for data being received from
 *   the radio.
 * @return \ref SL_RAIL_STATUS_NO_ERROR on success or an error code on failure.
 *
 * This API enables or disables the modem and GPIOs for direct mode operation.
 * see \ref sl_rail_config_direct_mode() for information on selecting the
 * correct hardware configuration. If direct mode is enabled,
 * packets are output and input directly to the radio via GPIO
 * and RAIL packet handling is ignored.
 *
 * @note This feature is only available on certain chips.
 *   \ref sl_rail_supports_direct_mode() can be used to check if a particular
 *   chip supports this feature or not.
 *
 * @warning This API is not safe to use in a multiprotocol app.
 *
 * @warning As this function relies on GPIO access and RAIL is meant to run in
 *   TrustZone non-secure world, it is not supported if GPIO is configured as
 *   secure peripheral and it will return \ref SL_RAIL_STATUS_INVALID_CALL.
 */
sl_rail_status_t sl_rail_enable_direct_mode(sl_rail_handle_t rail_handle,
                                            bool enable_direct_tx,
                                            bool enable_direct_rx);

/**
 * Get the radio subsystem clock frequency in Hz.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Radio subsystem clock frequency in Hz.
 */
uint32_t sl_rail_get_radio_clock_freq_hz(sl_rail_handle_t rail_handle);

/**
 * Set the crystal tuning.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in] tune A chip-dependent crystal capacitor bank tuning parameter.
 * @return Status code indicating success of the function call.
 *
 * Tunes the crystal that the radio depends on to change the location of the
 * center frequency for transmitting and receiving. This function will only
 * succeed if the radio is idle at the time of the call.
 *
 * @note This function proportionally affects the entire chip's timing
 *   across all its peripherals, including radio tuning and channel spacing.
 *   It is recommended to call this function only when HFXO is not being used,
 *   as it can cause disturbance on the HFXO frequency.
 *   A separate function, \ref sl_rail_set_freq_offset(), can be used to adjust
 *   just the radio tuner without disturbing channel spacing or other chip
 *   peripheral timing.
 * @note This API sets CTUNEXIANA and internally
 *   CTUNEXOANA = CTUNEXIANA + delta where delta is set or changed by
 *   \ref sl_rail_set_tune_delta(). The default delta may not be 0 on some devices.
 */
sl_rail_status_t sl_rail_set_tune(sl_rail_handle_t rail_handle, uint32_t tune);

/**
 * Get the crystal tuning.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return A chip-dependent crystal capacitor bank tuning parameter.
 *
 * Retrieves the current tuning value used by the crystal that the radio
 * depends on.
 * @note This is the CTUNEXIANA value.
 */
uint32_t sl_rail_get_tune(sl_rail_handle_t rail_handle);

/**
 * Set the crystal tuning delta.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in] delta A chip-dependent crystal capacitor bank tuning delta.
 * @return Status code indicating success of the function call.
 *
 * Set the CTUNEXOANA delta for \ref sl_rail_set_tune() to use:
 * CTUNEXOANA = CTUNEXIANA + delta (subject to field-size limitations).
 * This function does not change CTUNE values;
 * call \ref sl_rail_set_tune() to put a new delta into effect.
 */
sl_rail_status_t sl_rail_set_tune_delta(sl_rail_handle_t rail_handle,
                                        int32_t delta);

/**
 * Get the crystal tuning delta.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return A chip-dependent crystal capacitor bank tuning delta.
 *
 * Retrieves the current tuning delta used by \ref sl_rail_set_tune().
 * @note The default delta if \ref sl_rail_set_tune_delta() has never been called
 *   is device-dependent and may not be 0.
 */
int32_t sl_rail_get_tune_delta(sl_rail_handle_t rail_handle);

/**
 * Get the frequency offset.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The measured frequency offset on a received packet.
 *   The units are described in the \ref sl_rail_frequency_offset_t
 *   documentation. If this returns \ref SL_RAIL_FREQUENCY_OFFSET_INVALID,
 *   it was called while the radio wasn't active and there is no way
 *   to get the frequency offset.
 *
 * Retrieves the measured frequency offset used during the previous
 * received packet, which includes the current radio frequency offset
 * (see \ref sl_rail_set_freq_offset()). If the chip has not been in RX,
 * it returns the nominal radio frequency offset.
 *
 * @note Changing to any non-idle radio state after reception can cause this
 *   value to be overwritten so it is safest to capture during packet reception.
 */
sl_rail_frequency_offset_t sl_rail_get_rx_freq_offset(sl_rail_handle_t rail_handle);

/**
 * Set the nominal radio frequency offset.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] freq_offset \ref sl_rail_frequency_offset_t parameter (signed, 2's
 *   complement).
 * @return Status code indicating success of the function call.
 *
 * This function is used to adjust the radio's tuning frequency slightly up or down.
 * It might be used in conjunction with \ref sl_rail_get_rx_freq_offset() after
 * receiving a packet from a peer to adjust the tuner to better match the
 * peer's tuned frequency.
 *
 * @note Unlike \ref sl_rail_set_tune(), which affects the entire chip's
 *   timing including radio tuning and channel spacing, this function
 *   only affects radio tuning without disturbing channel spacing or
 *   other chip peripheral timing.
 */
sl_rail_status_t sl_rail_set_freq_offset(sl_rail_handle_t rail_handle,
                                         sl_rail_frequency_offset_t freq_offset);

/**
 * Start transmitting a stream on a certain channel with the ability to select
 * an antenna.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] channel A channel on which to emit a stream.
 * @param[in] mode Choose the stream mode (PN9, and so on).
 * @param[in] tx_options Choose the TX Antenna option.
 *   Takes only \ref SL_RAIL_TX_OPTION_ANTENNA_0, \ref SL_RAIL_TX_OPTION_ANTENNA_1,
 *   \ref SL_RAIL_TX_OPTIONS_DEFAULT or \ref SL_RAIL_TX_OPTIONS_NONE from the
 *   \ref sl_rail_tx_options_t. If some other value is used then, transmission
 *   is possible on any antenna.
 * @return Status code indicating success of the function call.
 *
 * Begins streaming onto the given channel. The sources can either be an
 * unmodulated carrier wave or an encoded stream of bits from a PN9 source.
 * All ongoing radio operations will be stopped before transmission begins.
 */
sl_rail_status_t sl_rail_start_tx_stream(sl_rail_handle_t rail_handle,
                                         uint16_t channel,
                                         sl_rail_stream_mode_t mode,
                                         sl_rail_tx_options_t tx_options);

/**
 * Stop stream transmission and idle the radio.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * Halts the transmission started by \ref sl_rail_start_tx_stream().
 */
sl_rail_status_t sl_rail_stop_tx_stream(sl_rail_handle_t rail_handle);

/**
 * Stop infinite preamble transmission started and start transmitting the rest
 * of the packet.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call:
 *   \ref SL_RAIL_STATUS_NO_ERROR if infinite preamble was stopped;
 *   \ref SL_RAIL_STATUS_INVALID_CALL if the radio isn't configured for infinite
 *   preamble;
 *   \ref SL_RAIL_STATUS_INVALID_STATE if the radio isn't transmitting.
 *
 * This function is only useful for radio configurations that specify an
 * infinite preamble. Call this API only after \ref SL_RAIL_EVENT_TX_STARTED
 * has occurred and the radio is transmitting.
 */
sl_rail_status_t sl_rail_stop_infinite_preamble_tx(sl_rail_handle_t rail_handle);

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * Enable radio state change interrupt.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] enable Enable/disable Radio state change interrupt.
 * @return Status code indicating success of the function call. Returns
 *   \ref SL_RAIL_STATUS_NO_ERROR once the interrupt has been enabled or disabled.
 *
 * @note If enabled, state change events are reported through the separate
 *   \ref sl_railcb_radio_state_changed() callback.
 */
sl_rail_status_t sl_rail_enable_radio_state_changed(sl_rail_handle_t rail_handle,
                                                    bool enable);

/**
 * Callback on radio state changes.
 *
 * @param[in] rail_handle The currently active RAIL instance handle.
 * @param[in] state The current radio state.
 */
void sl_railcb_radio_state_changed(sl_rail_handle_t rail_handle, uint8_t state);

/**
 * Get the current radio state.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return An enumeration, \ref sl_rail_radio_state_efr32_t, for the current radio
 *   state.
 *
 */
sl_rail_radio_state_efr32_t sl_rail_get_radio_state_internal(sl_rail_handle_t rail_handle);

#endif//DOXYGEN_UNDOCUMENTED

/** @} */ // end of group Diagnostic

/******************************************************************************
 * Front End Module Voltage Detection (VDET)
 *****************************************************************************/
/**
 * @addtogroup VDET Front End Module Voltage Detection
 * @brief APIs for configuring and detecting a voltage from a Front End Module (VDET).
 *
 * The sl_rail_util_vdet plugin lets customers measure an output voltage from their attached
 * Front End Module at a specific time in a Tx packet. This information allows
 * optimizations to power configuration, and monitoring FEM performance.
 *
 * @note VDET is only supported with EFR32xG25 devices.
 * @{
 */

/**
 * Configure the VDET plugin.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @param[in] p_config A pointer to a \ref sl_rail_vdet_config_t struct that contains
 *   configuration data for the VDET.
 * @return \ref sl_rail_status_t
 *   \retval SL_RAIL_STATUS_NO_ERROR - All went well
 *   \retval SL_RAIL_STATUS_INVALID_STATE - VDET is enabled. Must be disabled first.
 *   \retval SL_RAIL_STATUS_INVALID_PARAMETER - mode/resolution/delay_us out-of-bounds.
 *
 * @warning As this function relies on GPIO access and RAIL is meant to run in
 *   TrustZone non-secure world, it is not supported if GPIO is configured as
 *   secure peripheral and it will return \ref SL_RAIL_STATUS_INVALID_CALL.
 */
sl_rail_status_t sl_rail_config_vdet(sl_rail_handle_t radio_handle,
                                     const sl_rail_vdet_config_t *p_config);

/**
 * Get the VDET plugin configuration.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @param[out] p_config A pointer to a \ref sl_rail_vdet_config_t struct that will
 *   return configuration data for the VDET.
 * @return \ref sl_rail_status_t
 *   \retval SL_RAIL_STATUS_NO_ERROR - All went well.
 */
sl_rail_status_t sl_rail_get_vdet_config(sl_rail_handle_t radio_handle,
                                         sl_rail_vdet_config_t *p_config);

/**
 * Enable the VDET plugin.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @param[in] enable Enable or disable \ref SL_RAIL_VDET_MODE_AUTOMATIC mode captures.
 *   Begins measurement if in \ref SL_RAIL_VDET_MODE_IMMEDIATE.
 * @return \ref sl_rail_status_t
 *   \retval SL_RAIL_STATUS_NO_ERROR - All went well, VDET is enabled or disabled.
 *   \retval SL_RAIL_STATUS_INVALID_STATE - VDET has not been configured or VDET was not idle.
 *     VDET is disabled.
 *
 * @warning As this function relies on HFXO access and RAIL is meant to run in
 *   TrustZone non-secure world, it is not supported if HFXO is configured as
 *   secure peripheral and it will return \ref SL_RAIL_STATUS_INVALID_CALL.
 */
sl_rail_status_t sl_rail_enable_vdet(sl_rail_handle_t radio_handle,
                                     bool enable);

/**
 * Check if the VDET plugin is enabled.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @return bool
 */
bool sl_rail_is_vdet_enabled(sl_rail_handle_t radio_handle);

/**
 * Get VDET measurement. A processed measurement in millivolts.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @param[out] p_vdet_mv A pointer to a uint32_t for returning the measurement.
 * @return \ref sl_rail_status_t
 *   \retval SL_RAIL_STATUS_NO_ERROR - All went well, value in *p_vdet_mv is valid.
 *   \retval SL_RAIL_STATUS_INVALID_CALL - VDET is in progress, wait until
 *     VDET capture is complete and try again.
 *   \retval SL_RAIL_STATUS_INVALID_STATE - VDET is not enabled or valid. Wait until next packet
       and try reading again.\n
 *       \ref SL_RAIL_VDET_MODE_AUTOMATIC - Wait until next packet and try reading again.\n
 *       \ref SL_RAIL_VDET_MODE_IMMEDIATE - Resend \ref sl_rail_enable_vdet().\n
 *   \retval SL_RAIL_STATUS_INVALID_PARAMETER - in \ref SL_RAIL_VDET_MODE_IMMEDIATE, resend \ref sl_rail_enable_vdet().\n
 *   \retval SL_RAIL_STATUS_SUSPENDED - Blocked by AuxADC contention. Wait until next packet
 *     and try reading again.
 *
 * @warning As this function relies on HFXO access and RAIL is meant to run in
 *   TrustZone non-secure world, it is not supported if HFXO is configured as
 *   secure peripheral and it will return \ref SL_RAIL_STATUS_INVALID_CALL.
 */
sl_rail_status_t sl_rail_get_vdet(sl_rail_handle_t radio_handle,
                                  uint32_t *p_vdet_mv);

/** @} */ // end of group VDET

/******************************************************************************
 * Thermal Protection
 *****************************************************************************/
/**
 * @addtogroup Thermal_Protection Thermal Protection
 * @{
 */

/**
 * Enable or disable the thermal protection if \ref SL_RAIL_SUPPORTS_THERMAL_PROTECTION
 * is defined and update the temperature threshold and cool down hysteresis preventing or
 * allowing transmissions.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @param[in] p_chip_temp_config A pointer to a \ref sl_rail_chip_temp_config_t that contains
 *   the configuration to be applied.
 * @return Status code indicating the result of the function call.
 *   Returns \ref SL_RAIL_STATUS_INVALID_PARAMETER if enable field from \ref sl_rail_chip_temp_config_t
 *   is set to false when an EFF is present on the board.
 *
 * When the temperature threshold minus a precise number of degrees
 * specified by the cool down hysteresis parameter is exceeded,
 * any future transmits are blocked until the temperature decreases below that limit.
 * Besides, if the temperature threshold is exceeded, any active transmit is aborted.
 *
 * By default the threshold is set to \ref SL_RAIL_CHIP_TEMP_THRESHOLD_MAX and
 * the cool down hysteresis is set to \ref SL_RAIL_CHIP_TEMP_COOLDOWN_DEFAULT.
 *
 * @note The thermal protection is automatically enabled when an EFF is present
 *   on the board. There is no use in calling this API in this case.
 */
sl_rail_status_t sl_rail_config_thermal_protection(sl_rail_handle_t radio_handle,
                                                   const sl_rail_chip_temp_config_t *p_chip_temp_config);

/**
 * Get the current thermal configuration parameter and status.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @param[out] p_chip_temp_config A non-NULL pointer to a \ref sl_rail_chip_temp_config_t that will
 *   be updated with the current configuration.
 * @return Status code indicating the result of the function call.
 */
sl_rail_status_t sl_rail_get_thermal_protection(sl_rail_handle_t radio_handle,
                                                sl_rail_chip_temp_config_t *p_chip_temp_config);

/** Number of temperature values provided for HFXO metrics */
#define SL_RAIL_HFXO_TEMP_MEASURE_COUNT (1U)

/** Total number of temperature values provided by \ref sl_rail_get_temperature(). */
#define SL_RAIL_TEMP_MEASURE_COUNT  (SL_RAIL_CHIP_TEMP_MEASURE_COUNT \
                                     + SL_RAIL_HFXO_TEMP_MEASURE_COUNT)

/**
 * Get the different temperature measurements in Kelvin done by sequencer or host.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @param[out] temp_buffer The array to be populated with temperatures (pointer).
 *    This array must have at least \ref SL_RAIL_TEMP_MEASURE_COUNT entries.
 * @param[in] reset true to reset the temperature statistics, false otherwise.
 * @return Status code indicating success of the function call.
 *
 * Temperatures, in Kelvin, are stored in temp_buffer[] such as:
 * - temp_buffer[0] is the chip temperature
 * - temp_buffer[1] is the minimal chip temperature
 * - temp_buffer[2] is the maximal chip temperature
 *
 * Values that are not populated yet or incorrect are set to 0.
 *
 * If \ref SL_RAIL_SUPPORTS_HFXO_COMPENSATION
 * temp_buffer[3] is the HFXO temperature
 */
sl_rail_status_t sl_rail_get_temperature(sl_rail_handle_t radio_handle,
                                         int16_t temp_buffer[SL_RAIL_TEMP_MEASURE_COUNT],
                                         bool reset);

/** @} */ // end of group Thermal_Protection

/******************************************************************************
 * Retiming
 *****************************************************************************/
/**
 * @addtogroup Retiming
 * @{
 * @brief EFR32-specific retiming capability.
 *
 * The EFR product families have many digital and analog modules that can run
 * in parallel with a radio. These combinations can cause interference and
 * degradation on the radio RX sensitivity. Retiming can
 * modify the clocking of the digital modules to reduce the interference.
 */

/**
 * Configure retiming options.
 *
 * @param[in] rail_handle A handle of RAIL instance.
 * @param[in] mask A bitmask containing which options should be modified.
 * @param[in] options A bitmask containing desired configuration settings.
 *   Bit positions for each option are found in the \ref sl_rail_retime_options_t.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_config_retime_options(sl_rail_handle_t rail_handle,
                                               sl_rail_retime_options_t mask,
                                               sl_rail_retime_options_t options);

/**
 * Get the currently configured retiming option.
 *
 * @param[in] rail_handle A handle of RAIL instance.
 * @param[out] p_options A pointer to configured retiming options
 *   bitmask indicating which are enabled.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_get_retime_options(sl_rail_handle_t rail_handle,
                                            sl_rail_retime_options_t *p_options);

/**
 * Indicate that the DCDC peripheral bus clock enable has changed allowing
 * RAIL to react accordingly.
 *
 * @return Status code indicating success of the function call.
 *
 * @note This should be called after DCDC has been enabled or disabled.
 */
sl_rail_status_t sl_rail_changed_dcdc(void);

/** @} */ // end of group Retiming

#ifndef DOXYGEN_UNDOCUMENTED

/******************************************************************************
 * Debug
 *****************************************************************************/
/**
 * @addtogroup Debug
 * @brief APIs for debugging
 * @{
 */

/**
 * Configure the debug mode for the radio library. Do not use this function
 * unless instructed by Silicon Labs.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] debug_mode Debug mode to enter.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_set_debug_mode(sl_rail_handle_t rail_handle,
                                        uint32_t debug_mode);

/**
 * Return the debug mode for the radio library. Do not use this function
 * unless instructed by Silicon Labs.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Debug mode for the radio library.
 */
uint32_t sl_rail_get_debug_mode(sl_rail_handle_t rail_handle);

/**
 * Override the radio base frequency.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] freq_hz A desired frequency in Hz.
 * @return Status code indicating success of the function call.
 *
 * Sets the radio to transmit at the frequency given. This function can only
 * be used while in \ref SL_RAIL_DEBUG_MODE_FREQ_OVERRIDE. The given frequency
 * needs to be close to the base frequency of the current PHY. After this
 * call, a full reset is needed to restore normal RAIL operation.
 */
sl_rail_status_t sl_rail_override_debug_frequency(sl_rail_handle_t rail_handle,
                                                  uint32_t freq_hz);

/**
 * Get the size of the radio's multiprotocol scheduler state buffer.
 *
 * @param[in] radio_handle A radio-generic RAIL handle.
 * @return Size, in bytes, of the radio's internal scheduler state buffer.
 *   Zero is returned if the handle is invalid or this is the singleprotocol
 *   library.
 */
uint32_t sl_rail_get_sched_buffer_bytes(sl_rail_handle_t radio_handle);

/** @} */ // end of group Debug

#endif//DOXYGEN_UNDOCUMENTED

/******************************************************************************
 * Assertion Callback
 *****************************************************************************/
/**
 * @addtogroup Assertions
 * @brief Callbacks called by assertions
 *
 * The assertion framework was implemented to not only
 * assert that certain conditions are true in a block of code, but also
 * to handle them more appropriately. In previous implementations,
 * the behavior upon a failed assert was to hang in a while(1) loop.
 * However, with the callback, each assert is given a unique error code so that
 * they can be handled on a more case-by-case basis. For documentation on each
 * of the errors, see the rail_assert_error_codes.h file.
 * \ref SL_RAIL_ASSERT_ERROR_MESSAGES[error_code] gives the explanation of the error.
 * With asserts built into the library, users can choose how to handle each
 * error inside the callback.
 *
 * @{
 */

/**
 * Callback called upon failed assertion.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in] error_code Value passed in by the calling assertion API indicating
 *   the RAIL assertion that failed.
 * @param[in] line An auxiliary value helping pinpoint which of several
 *   places the error_code assertion might have failed.
 */
void sl_railcb_assert_failed(sl_rail_handle_t rail_handle,
                             sl_rail_assert_error_codes_t error_code,
                             int line);

/** @} */ // end of group Assertions

/******************************************************************************
 * External_Thermistor
 *****************************************************************************/
/**
 * @addtogroup External_Thermistor External Thermistor
 * @brief APIs to measure temperature using an external thermistor
 *
 * This feature allows reading the temperature via an external thermistor on
 * chips that support it. This will require connecting the necessary components
 * and configuring the pins as required.
 *
 * @{
 */

/**
 * Start a thermistor measurement.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *   Returns \ref SL_RAIL_STATUS_INVALID_STATE if the thermistor is started while the
 *   radio is transmitting.
 *
 * To get the thermistor impedance, call the
 * function \ref sl_rail_get_thermistor_impedance(). On platforms having
 * \ref SL_RAIL_SUPPORTS_EXTERNAL_THERMISTOR, this function reconfigures
 * GPIO_THMSW_EN_PIN located in GPIO_THMSW_EN_PORT.
 * To locate this pin, refer to the data sheet or appropriate header files
 * of the device. For proper operation, \ref sl_rail_init() must be called before
 * using this function.
 *
 * @note When an EFF is attached, this function must not be called during
 *   transmit.
 *
 * @warning This API is not safe to use in a multiprotocol app.
 *
 * @warning As this function relies on EMU, GPIO and HFXO access and RAIL is
 *   meant to run in TrustZone non-secure world, it is not supported if EMU,
 *   GPIO or HFXO are configured as secure peripheral and it will return
 *   \ref SL_RAIL_STATUS_INVALID_CALL.
 */
sl_rail_status_t sl_rail_start_thermistor_measurement(sl_rail_handle_t rail_handle);

/**
 * Get the thermistor impedance measurement. Sets the output parameter to
 * \ref SL_RAIL_INVALID_THERMISTOR_VALUE if the thermistor is not properly
 * configured or the thermistor measurement is not ready.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[out] p_thermistor_impedance_ohms A pointer to a uint32_t updated
 *   with the current thermistor impedance measurement in Ohms, or
 *   \ref SL_RAIL_INVALID_THERMISTOR_VALUE on error.
 * @return Status code indicating success of the function call.
 *
 * @note This function is already called in \ref sl_rail_calibrate_hfxo().
 *   It does not need to be manually called during the compensation sequence.
 *
 * @note When HFXO compensation is enabled and a measurement is in progress,
 *   this function returns \ref SL_RAIL_STATUS_NO_ERROR with the most recent
 *   cached thermistor value.
 */
sl_rail_status_t sl_rail_get_thermistor_impedance(sl_rail_handle_t rail_handle,
                                                  uint32_t *p_thermistor_impedance_ohms);

/**
 * Callback to convert the thermistor impedance into temperature, in Celsius.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in] thermistor_impedance_ohms Current thermistor impedance measurement in
 *   Ohms.
 * @param[out] p_thermistor_temperature_8c A non-NULL pointer to an int16_t updated
 *   with the current thermistor temperature in eighth of Celsius degrees.
 * @return Status code indicating success of the function call.
 *
 * A version of this function is provided in the \ref rail_util_thermistor
 * plugin for Silicon Labs radio boards. For custom boards this function can be
 * modified and re-implemented as needed in the plugin.
 *
 * @note The \ref rail_util_thermistor plugin is mandatory on EFR32xG25 platforms.
 */
sl_rail_status_t sl_railcb_convert_thermistor_impedance(sl_rail_handle_t rail_handle,
                                                        uint32_t thermistor_impedance_ohms,
                                                        int16_t *p_thermistor_temperature_8c);

/**
 * Callback to compute the crystal PPM deviation from the thermistor
 * temperature.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in] crystal_temperature_c Current crystal temperature, in Celsius.
 * @param[out] p_crystal_error_ppm A non-NULL pointer to an int8_t updated
 *   with the current ppm error in ppm units.
 * @return Status code indicating success of the function call.
 *
 * A version of this function is provided in the \ref rail_util_thermistor
 * plugin for Silicon Labs radio boards. For custom boards this function can be
 * modified and re-implemented as needed in the plugin.
 *
 * @note The \ref rail_util_thermistor plugin is mandatory on EFR32xG25 platforms.
 */
sl_rail_status_t sl_railcb_compute_hfxo_error_ppm(sl_rail_handle_t rail_handle,
                                                  int16_t crystal_temperature_c,
                                                  int8_t *p_crystal_error_ppm);

/**
 * Configure the GPIO for thermistor usage.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @param[in] p_hfxo_thermistor_config A non-NULL pointer to the thermistor configuration
 *   indicating the GPIO port and pin to use.
 * @return Status code indicating the result of the function call.
 *
 * @note The port and pin that must be passed in \ref sl_rail_hfxo_thermistor_config_t
 *   are GPIO_THMSW_EN_PORT and GPIO_THMSW_EN_PIN respectively.
 *
 * @warning As this function relies on GPIO access and RAIL is meant to run in
 *   TrustZone non-secure world, it is not supported if GPIO is configured as
 *   secure peripheral and it will return \ref SL_RAIL_STATUS_INVALID_CALL.
 */
sl_rail_status_t sl_rail_config_hfxo_thermistor(sl_rail_handle_t rail_handle,
                                                const sl_rail_hfxo_thermistor_config_t *p_hfxo_thermistor_config);

/**
 * Configure the temperature parameters for HFXO compensation.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_hfxo_compensation_config A non-NULL pointer to HFXO compensation parameters
 *   indicating the temperature variations used to trigger a compensation.
 * @return Status code indicating the result of the function call.
 *
 * @note This function must be called after \ref sl_rail_config_hfxo_thermistor() to succeed.
 *
 * In \ref sl_rail_hfxo_compensation_config_t, delta_nominal_celsius and
 * delta_critical_celsius define the temperature variation triggering
 * a new compensation.
 * The field zone_temperature_celsius defines the temperature separating
 * the nominal case (below) from the critical one (above).
 *
 * When enabled and either delta_nominal_celsius or delta_critical_celsius
 * are exceeded, RAIL raises
 * event \ref SL_RAIL_EVENT_CAL_NEEDED with \ref SL_RAIL_CAL_TEMP_HFXO bit set.
 * The API \ref sl_rail_start_thermistor_measurement() must be called afterwards.
 * The latter will raise \ref SL_RAIL_EVENT_THERMISTOR_DONE with calibration bit
 * \ref SL_RAIL_CAL_COMPENSATE_HFXO set and a call to \ref sl_rail_calibrate_hfxo() must follow.
 *
 * @note Set delta_nominal_celsius and delta_critical_celsius to 0 to perform
 *   compensation after each transmit.
 */
sl_rail_status_t sl_rail_config_hfxo_compensation(sl_rail_handle_t rail_handle,
                                                  const sl_rail_hfxo_compensation_config_t *p_hfxo_compensation_config);

/**
 * Get the temperature parameters for HFXO compensation.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_hfxo_compensation_config A non-NULL pointer to HFXO
 *   compensation parameters filled in by the function.
 * @return Status code indicating the result of the function call.
 */
sl_rail_status_t sl_rail_get_hfxo_compensation_config(sl_rail_handle_t rail_handle,
                                                      sl_rail_hfxo_compensation_config_t *p_hfxo_compensation_config);

/**
 * Compute a frequency offset and compensate HFXO accordingly.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] crystal_error_ppm The current ppm error. Positive values indicate
 *   the HFXO frequency is too high; negative values indicate it's too low.
 * @return Status code indicating success of the function call.
 *
 * @note This function only works for platforms having
 *   \ref SL_RAIL_SUPPORTS_EXTERNAL_THERMISTOR alongside \ref SL_RAIL_SUPPORTS_HFXO_COMPENSATION.
 */
sl_rail_status_t sl_rail_compensate_hfxo(sl_rail_handle_t rail_handle,
                                         int8_t crystal_error_ppm);

/** @} */ // end of group External_Thermistor

/******************************************************************************
 * TrustZone
 *****************************************************************************/
/**
 * @addtogroup TrustZone
 * @brief APIs to use RAIL with TrustZone enabled and peripherals configured
 *   as secure.
 *
 * RAIL internally accesses CMU, EMU, GPIO, LDMA, HFXO, PRS and SYSRTC.
 * If some of them are configured as secure peripherals, some RAIL code must be
 * executed as secure code. To do so, callbacks gathered in
 * \ref sl_rail_tz_config_t must be implemented and passed to RAIL through
 * \ref sl_rail_tz_init_non_secure(). Each callback must do the
 * non-secure/secure transition, call
 * \ref sl_rail_tz_check_peripherals_secure_states() and then call the
 * corresponding RAIL API from secure world:
 *
 * | Secure peripheral |                                                                                                                                                         Callbacks to implement                                                                                                                                                               |
 * |-------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
 * |       CMU         | \ref sl_rail_tz_config_t::changed_dcdc_callback, \ref sl_rail_tz_config_t::config_antenna_gpio_callback, \ref sl_rail_tz_config_t::radio_clock_enable_callback, \ref sl_rail_tz_config_t::get_radio_clock_freq_hz_callback, \ref sl_rail_tz_config_t::rfeca_clock_enable_callback, \ref sl_rail_tz_config_t::rfeca_is_clock_enabled_callback |
 * |       EMU         | \ref sl_rail_tz_config_t::read_internal_temperature_callback, \ref sl_rail_tz_config_t::enable_secure_radio_irqs_callback, \ref sl_rail_tz_config_t::disable_secure_radio_irqs_callback                                                                                                                                                      |
 * |       GPIO        | \ref sl_rail_tz_config_t::config_antenna_gpio_callback                                                                                                                                                                                                                                                                                       |
 * |       HFXO        | \ref sl_rail_tz_config_t::configure_hfxo_callback                                                                                                                                                                                                                                                                                            |
 *
 * RAIL internally calls platform functions that access CMU, EMU, GPIO, LDMA
 * HFXO and PRS.
 * If some of them are configured as secure peripherals, some functions must be
 * executed as secure code. To do so, those functions are prepended with weak
 * symbols and must be overwritten to do the non-secure/secure transition and
 * call the corresponding platform function from secure world:
 *
 * | Secure peripheral |                                Platform functions to overwrite                                |
 * |-------------------|-----------------------------------------------------------------------------------------------|
 * |       CMU         | CMU_ClockEnable, EMU_DCDCSetPFMXTimeoutMaxCtrl (DCDC access: SL_TRUSTZONE_PERIPHERAL_DCDC_S)  |
 * |       HFXO        | CMU_HFXOCTuneSet, CMU_HFXOCTuneGet, CMU_HFXOCoreBiasCurrentCalibrate                          |
 * |       PRS         | PRS_SourceAsyncSignalSet, PRS_GetFreeChannel, PRS_ConnectConsumer, PRS_PinOutput, PRS_Combine |
 *
 * When there is a combination of secure and non-secure peripherals, defines
 * must be added in secure application slcp file so non-secure peripherals can
 * properly accessed by secure code. Example with only CMU non-secure:
 *
 * @code{.slcp}
 * define:
 *   - name: SL_TRUSTZONE_PERIPHERAL_CMU_S
 *     value: 0
 *     condition: [trustzone_secure]
 *   - name: SL_TRUSTZONE_PERIPHERAL_EMU_S
 *     value: 1
 *     condition: [trustzone_secure]
 *   - name: SL_TRUSTZONE_PERIPHERAL_GPIO_S
 *     value: 1
 *     condition: [trustzone_secure]
 *   - name: SL_TRUSTZONE_PERIPHERAL_LDMA_S
 *     value: 1
 *     condition: [trustzone_secure]
 *   - name: SL_TRUSTZONE_PERIPHERAL_HFXO0_S
 *     value: 1
 *     condition: [trustzone_secure]
 *   - name: SL_TRUSTZONE_PERIPHERAL_PRS_S
 *     value: 1
 *     condition: [trustzone_secure]
 *   - name: SL_TRUSTZONE_PERIPHERAL_SYSRTC_S
 *     value: 1
 *     condition: [trustzone_secure]
 * @endcode
 *
 * Some RAIL API are not suppoted with EMU, GPIO LDMA, HFXO, PRS or SYSRTC
 * configured secure:
 *
 * | Secure peripheral |                                                                                                             Unsupported RAIL API/features                                                                                                       |
 * |-------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
 * |       EMU         | \ref sl_rail_start_thermistor_measurement(), \ref sl_rail_init_power_manager()                                                                                                                                                                  |
 * |       GPIO        | \ref sl_rail_enable_direct_mode(), \ref sl_rail_enable_pti(), \ref sl_rail_config_pti(), \ref sl_rail_config_hfxo_thermistor(), \ref sl_rail_start_thermistor_measurement(), \ref sl_rail_config_vdet(), \ref sl_rail_ble_config_aox_antenna()  |
 * |       LDMA        | \ref SL_RAIL_IEEE802154_SUPPORTS_RX_CHANNEL_SWITCHING (\ref sl_rail_ieee802154_config_rx_channel_switching() and \ref SL_RAIL_RX_OPTION_CHANNEL_SWITCHING)                                                                                      |
 * |       HFXO        | \ref sl_rail_start_thermistor_measurement(), \ref sl_rail_enable_vdet(), \ref sl_rail_get_vdet()                                                                                                                                                |
 * |       PRS         | \ref sl_rail_enable_prs_lna_bypass()                                                                                                                                                                                                            |
 * |       SYSRTC      | \ref sl_rail_config_sleep() with \ref sl_rail_sleep_config_t::SL_RAIL_SLEEP_CONFIG_TIMERSYNC_ENABLED                                                                                                                                            |
 *
 * @{
 */

/**
 * Init RAIL TrustZone feature for non-secure world
 *
 * @param[in] p_tz_config A non-NULL pointer to a \ref sl_rail_tz_config_t
 *   structure.
 * @return Status code indicating success of the function call.
 *
 * @note This function must only be called from non-secure world (only if
 *   TrustZone is activated) on platforms having
 *   \ref SL_RAIL_SUPPORTS_TRUSTZONE_SECURE_PERIPHERALS. It must be called
 *   before \ref sl_rail_init() and it must be called again with updated
 *   \ref sl_rail_tz_config_t if peripherals secure configuration has changed.
 */
sl_rail_status_t sl_rail_tz_init_non_secure(const sl_rail_tz_config_t *p_tz_config);

/**
 * Init RAIL TrustZone feature for secure world
 *
 * @return Status code indicating success of the function call.
 *
 * @note This function must only be called from secure world (only if TrustZone
 *   is activated) on platforms having
 *   \ref SL_RAIL_SUPPORTS_TRUSTZONE_SECURE_PERIPHERALS. It must be called
 *   before starting the non-secure application.
 */
sl_rail_status_t sl_rail_tz_init_secure(void);

/**
 * Check the secure state of peripherals used by RAIL.
 *
 * @return Status code indicating success of the function call.
 *
 * @note This function must only be called from secure world and it must be
 *   called at the beginning of each RAIL TrustZone callbacks
 *   (\ref sl_rail_tz_config_t) secure code to avoid secure fault.
 */
sl_rail_status_t sl_rail_tz_check_peripherals_secure_states(void);

/**
 * Enable radio clocks.
 *
 * @return Status code indicating success of the function call.
 *
 * @note This function must only be called from secure world when CMU is
 *   configured as secure TrustZone peripheral.
 *
 */
sl_rail_status_t sl_rail_tz_radio_clock_enable(void);

/**
 * Enable RFECA clocks.
 *
 * @return Status code indicating success of the function call.
 *
 * @note This function must only be called from secure world when CMU is
 *   configured as secure TrustZone peripheral.
 *
 */
sl_rail_status_t sl_rail_tz_rfeca_clock_enable(void);

/**
 * Indicate whether RFECA clocks are enabled.
 *
 * @return true if RFECA clocks are enabled; false otherwise
 *
 * @note This function must only be called from secure world when CMU is
 *   configured as secure TrustZone peripheral.
 *
 */
bool sl_rail_tz_rfeca_is_clock_enabled(void);

/**
 * Read the internal temperature.
 *
 * @param[out] p_internal_temperature_kelvin A pointer to the internal
 *   temperature in Kelvin.
 * @param[in] enable_temperature_interrupts Indicate whether temperature
 * interrupts are enabled.
 * @return Status code indicating success of the function call.
 *
 * @note This function must only be called from secure world when EMU is
 *   configured as secure TrustZone peripheral.
 *
 */
sl_rail_status_t sl_rail_tz_read_internal_temperature(uint16_t *p_internal_temperature_kelvin,
                                                      bool enable_temperature_interrupts);

/**
 * Enable secure peripheral interrupts needed by the radio.
 *
 * @return Status code indicating success of the function call.
 *
 * @note This function must only be called from secure world when EMU is
 *   configured as secure TrustZone peripheral.
 *
 */
sl_rail_status_t sl_rail_tz_enable_secure_radio_irqs(void);

/**
 * Disable secure peripheral interrupts needed by the radio.
 *
 * @return Status code indicating success of the function call.
 *
 * @note This function must only be called from secure world when EMU is
 *   configured as secure TrustZone peripheral.
 *
 */
sl_rail_status_t sl_rail_tz_disable_secure_radio_irqs(void);

/**
 * Perform ldma transfer for the radio.
 *
 * @param[in] p_dest A pointer to the destination data.
 * @param[in] p_src A pointer to the source data.
 * @param[in] num_words Number of words to transfer.
 * @return Status code indicating success of the function call.
 *
 * @note This function must only be called from secure world when LDMA is
 *   configured as secure TrustZone peripheral.
 *
 */
sl_rail_status_t sl_rail_tz_radio_perform_m2m_ldma(uint32_t *p_dest,
                                                   const uint32_t *p_src,
                                                   uint32_t num_words);

/**
 * Configure HFXO.
 *
 * @return Status code indicating success of the function call.
 *
 * @note This function must only be called from secure world when HFXO is
 *   configured as secure TrustZone peripheral.
 *
 */
sl_rail_status_t sl_rail_tz_configure_hfxo(void);

/**
 * Set GPIO for antenna config.
 *
 * @param[in] p_config A pointer to a configuration structure applied to the relevant Antenna
 *   Configuration registers. A NULL configuration will produce undefined behavior.
 * @return Status code indicating success of the function call.
 *
 * @note This function must only be called from secure world when CMU or GPIO
 *   are configured as secure TrustZone peripherals.
 *
 */
sl_rail_status_t sl_rail_tz_config_antenna_gpio(const sl_rail_antenna_config_t *p_config);

/** @} */ // end of group TrustZone

/******************************************************************************
 * Scheduler
 *****************************************************************************/
/**
 * @addtogroup Scheduler_Series_3 Series 3 Scheduler
 * @brief APIs to use the new scheduler implementation in Series 3 devices.
 * @{
 */

/**
 * Cancel previously scheduled transactions
 *
 * This function cancels the scheduled transaction so a new transaction
 * can be scheduled, including "unscheduled" DMP transactions. Transactions
 * already started will complete.
 * This function is available for all series 3 devices.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 */
sl_rail_status_t sl_rail_cancel_scheduled_trx(sl_rail_handle_t rail_handle);

/** @} */ // end of group Scheduler

/******************************************************************************
 * Features
 *****************************************************************************/
/**
 * @addtogroup Features
 * @{
 */

/**
 * Indicate whether RAIL supports 2.4 GHz band operation on this chip.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if the 2.4 GHz band is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_2P4_GHZ_BAND.
 */
bool sl_rail_supports_2p4_ghz_band(sl_rail_handle_t rail_handle);

/**
 * Indicate whether RAIL supports Sub-GHz band operation on this chip.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if the Sub-GHz band is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_SUB_GHZ_BAND.
 */
bool sl_rail_supports_sub_ghz_band(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports dual 2.4 GHz and Sub-GHz band operation.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if the dual band is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_DUAL_BAND.
 */
bool sl_rail_supports_dual_band(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports bit masked address filtering.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if bit masked address filtering is supported; false otherwise.
 *
 * Runtime refinement of compile-time
 * \ref SL_RAIL_SUPPORTS_ADDR_FILTER_ADDRESS_BIT_MASK.
 */
bool sl_rail_supports_addr_filter_address_bit_mask(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports address filter mask information
 * for incoming packets in
 * \ref sl_rail_rx_packet_info_t::filter_mask and
 * \ref sl_rail_ieee802154_address_t::filter_mask.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if address filter information is supported; false otherwise
 *   (in which case \ref sl_rail_rx_packet_info_t::filter_mask value is undefined).
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_ADDR_FILTER_MASK.
 */
bool sl_rail_supports_addr_filter_mask(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports alternate TX power settings.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if alternate TX power settings are supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref
 * SL_RAIL_SUPPORTS_ALTERNATE_TX_POWER.
 */
bool sl_rail_supports_alternate_tx_power(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports antenna diversity.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if antenna diversity is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_ANTENNA_DIVERSITY.
 *
 * @note Certain radio configurations may not support this feature even
 *   if the chip in general claims to support it.
 */
bool sl_rail_supports_antenna_diversity(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports internal RF path diversity.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if RF path diversity is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_PATH_DIVERSITY.
 *
 * @note Certain radio configurations may not support this feature even
 *   if the chip in general claims to support it.
 */
bool sl_rail_supports_path_diversity(sl_rail_handle_t rail_handle);

/**
 * Indicate whether RAIL supports AUXADC measurements on this chip.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if AUXADC measurements are supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_AUXADC.
 */
bool sl_rail_supports_aux_adc(sl_rail_handle_t rail_handle);

/**
 * Indicate whether RAIL supports channel hopping on this chip.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if channel hopping is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_CHANNEL_HOPPING.
 */
bool sl_rail_supports_channel_hopping(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports direct mode.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if direct mode is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref
 * SL_RAIL_SUPPORTS_DIRECT_MODE.
 */
bool sl_rail_supports_direct_mode(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports dual sync words.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if dual sync words are supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_DUAL_SYNC_WORDS.
 *
 * @note Certain radio configurations may not support this feature even
 *   if the chip in general claims to support it.
 */
bool sl_rail_supports_dual_sync_words(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports start to start TX repeats.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if start to start TX repeats are supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref
 * SL_RAIL_SUPPORTS_TX_REPEAT_START_TO_START.
 */
bool sl_rail_supports_tx_repeat_start_to_start(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports VDET.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if VDET is supported; false otherwise.
 */
bool sl_rail_supports_vdet(sl_rail_handle_t rail_handle);

/**
 * Indicate whether RAIL supports thermistor measurements on this chip.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if thermistor measurements are supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_EXTERNAL_THERMISTOR.
 */
bool sl_rail_supports_external_thermistor(sl_rail_handle_t rail_handle);

/**
 * Indicate whether RAIL supports HFXO compensation on this chip.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if HFXO compensation is supported and
 *   \ref sl_rail_config_hfxo_thermistor() has been successfully called;
 *   false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_HFXO_COMPENSATION.
 */
bool sl_rail_supports_hfxo_compensation(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports MFM protocol.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if MFM protocol is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_MFM.
 */
bool sl_rail_supports_mfm(sl_rail_handle_t rail_handle);

/**
 * Indicate whether RAIL supports OFDM band operation on this chip.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if OFDM operation is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_OFDM_PA.
 */
bool sl_rail_supports_ofdm_pa(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports a high-precision LFRCO.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if high-precision LFRCO is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_PRECISION_LFRCO.
 */
bool sl_rail_supports_precision_lfrco(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports radio entropy.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if radio entropy is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_RADIO_ENTROPY.
 */
bool sl_rail_supports_radio_entropy(sl_rail_handle_t rail_handle);

/**
 * Indicate whether RAIL supports RF Sense Energy Detection Mode on this chip.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if RF Sense Energy Detection Mode is supported; false otherwise.
 *
 * Runtime refinement of compile-time
 * \ref SL_RAIL_SUPPORTS_RF_SENSE_ENERGY_DETECTION.
 */
bool sl_rail_supports_rf_sense_energy_detection(sl_rail_handle_t rail_handle);

/**
 * Indicate whether RAIL supports RF Sense Selective(OOK) Mode on this chip.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if RF Sense Selective(OOK) Mode is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_RF_SENSE_SELECTIVE_OOK.
 */
bool sl_rail_supports_rf_sense_selective_ook(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports configurable RSSI threshold
 * set by \ref sl_rail_set_rssi_detect_threshold().
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if setting configurable RSSI is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_RSSI_DETECT_THRESHOLD.
 */
bool sl_rail_supports_rssi_detect_threshold(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports RX direct mode data to FIFO.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if direct mode data to FIFO is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref
 * SL_RAIL_SUPPORTS_RX_DIRECT_MODE_DATA_TO_FIFO.
 */
bool sl_rail_supports_rx_direct_mode_data_to_fifo(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports raw RX data
 * sources other than \ref sl_rail_rx_data_source_t::SL_RAIL_RX_DATA_SOURCE_PACKET_DATA.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if direct mode is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_RX_RAW_DATA.
 */
bool sl_rail_supports_rx_raw_data(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports SQ-based PHY.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if the SQ-based PHY is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_SQ_PHY.
 */
bool sl_rail_supports_sq_phy(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports automatic TX to TX transitions.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if TX to TX transitions are supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_TX_TO_TX.
 */
bool sl_rail_supports_tx_to_tx(sl_rail_handle_t rail_handle);

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * Indicate whether RAIL supports the Sequencer User on this chip.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if Sequencer User is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_SEQUENCER_USER.
 */
bool sl_rail_supports_sequencer_user(sl_rail_handle_t rail_handle);
#endif//DOXYGEN_UNDOCUMENTED

/**
 * Indicate whether RAIL supports the BLE protocol on this chip.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if BLE is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_PROTOCOL_BLE.
 */
bool sl_rail_supports_protocol_ble(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports BLE 1 Mbps operation.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if BLE 1 Mbps operation is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_BLE_SUPPORTS_1_MBPS.
 */
bool sl_rail_ble_supports_1_mbps(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports BLE 2 Mbps operation.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if BLE 2 Mbps operation is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_BLE_SUPPORTS_2_MBPS.
 */
bool sl_rail_ble_supports_2_mbps(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports BLE Antenna Switching needed for
 * Angle-of-Arrival receives or Angle-of-Departure transmits.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if BLE Antenna Switching is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_BLE_SUPPORTS_ANTENNA_SWITCHING.
 */
bool sl_rail_ble_supports_antenna_switching(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports BLE Coded PHY used for Long-Range.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if BLE Coded PHY is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_BLE_SUPPORTS_CODED_PHY.
 */
bool sl_rail_ble_supports_coded_phy(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports BLE CTE (Constant Tone Extension)
 * needed for Angle-of-Arrival/Departure transmits.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if BLE CTE is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_BLE_SUPPORTS_CTE.
 */
bool sl_rail_ble_supports_cte(sl_rail_handle_t rail_handle);

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * Indicate whether this chip supports BLE CS.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if BLE CS is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_BLE_SUPPORTS_CS.
 */
bool sl_rail_ble_supports_cs(sl_rail_handle_t rail_handle);
#endif//DOXYGEN_UNDOCUMENTED

/**
 * Indicate whether this chip supports BLE IQ Sampling needed for
 * Angle-of-Arrival/Departure receives.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if BLE IQ Sampling is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_BLE_SUPPORTS_IQ_SAMPLING.
 */
bool sl_rail_ble_supports_iq_sampling(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports BLE PHY switch to RX
 * functionality, which is used to switch BLE PHYs at a specific time
 * to receive auxiliary packets.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if BLE PHY switch to RX is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_BLE_SUPPORTS_PHY_SWITCH_TO_RX.
 */
bool sl_rail_ble_supports_phy_switch_to_rx(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports the Quuppa PHY.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if the Quuppa is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_BLE_SUPPORTS_QUUPPA.
 */
bool sl_rail_ble_supports_quuppa(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports BLE signal identifier.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if signal identifier is supported; false otherwise.
 */
bool sl_rail_ble_supports_signal_identifier(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports BLE Simulscan PHY used for simultaneous
 * BLE 1 Mbps and Coded PHY reception.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if BLE Simulscan PHY is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_BLE_SUPPORTS_SIMULSCAN_PHY.
 */
bool sl_rail_ble_supports_simulscan_phy(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports the IEEE 802.15.4 protocol.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if the 802.15.4 protocol is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154.
 */
bool sl_rail_supports_protocol_ieee802154(sl_rail_handle_t rail_handle);

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * Indicate whether this chip supports the IEEE 802.15.4 2 Mbps PHY.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if the 802.15.4 2 Mbps PHY is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_IEEE802154_SUPPORTS_2_MBPS_PHY.
 */
bool sl_rail_ieee802154_supports_2_mbps_phy(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports the IEEE 802.15.4 2 Mbps
 * fast channel switching PHY.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if the 802.15.4 2 Mbps fast channel switching PHY is supported;
 * false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_IEEE802154_SUPPORTS_FCS_2_MBPS_PHY.
 */
bool sl_rail_ieee802154_supports_fcs_2_mbps_phy(sl_rail_handle_t rail_handle);
#endif //DOXYGEN_UNDOCUMENTED

/**
 * Indicate whether this chip supports the IEEE 802.15.4 Wi-Fi Coexistence PHY.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if the 802.15.4 COEX PHY is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_IEEE802154_SUPPORTS_COEX_PHY.
 */
bool sl_rail_ieee802154_supports_coex_phy(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports the IEEE 802.15.4 2.4 GHz band variant.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if IEEE 802.15.4 2.4 GHz band variant is supported;
 *   false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_IEEE802154_SUPPORTS_2P4_GHZ_BAND.
 */
bool sl_rail_ieee802154_supports_2p4_ghz_band(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports the thermal protection.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if thermal protection is supported;
 *   false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_THERMAL_PROTECTION.
 */
bool sl_rail_supports_thermal_protection(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports the IEEE 802.15.4 2.4 RX channel switching.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if IEEE 802.15.4 2.4 GHz RX channel switching is supported;
 *   false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_IEEE802154_SUPPORTS_RX_CHANNEL_SWITCHING.
 */
bool sl_rail_ieee802154_supports_rx_channel_switching(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports the IEEE 802.15.4
 * front end module optimized PHY.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if a front end module is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_IEEE802154_SUPPORTS_FEM_PHY.
 */
bool sl_rail_ieee802154_supports_fem_phy(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports canceling the frame-pending lookup
 * event \ref SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND when the radio
 * transitions to a state that renders the the reporting of this event moot
 * (i.e., too late for the stack to influence the outgoing Ack).
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if canceling the lookup event is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref
 * SL_RAIL_IEEE802154_SUPPORTS_CANCEL_FRAME_PENDING_LOOKUP.
 */
bool sl_rail_ieee802154_supports_cancel_frame_pending_lookup(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports early triggering of the frame-pending
 * lookup event \ref SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND
 * just after MAC address fields have been received.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if early triggering is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref
 * SL_RAIL_IEEE802154_SUPPORTS_EARLY_FRAME_PENDING_LOOKUP.
 */
bool sl_rail_ieee802154_supports_early_frame_pending_lookup(sl_rail_handle_t rail_handle);

/**
 * Indicate whether RAIL supports dual PA mode on this chip.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if the dual PA mode is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_IEEE802154_SUPPORTS_DUAL_PA_CONFIG.
 */
bool sl_rail_ieee802154_supports_dual_pa_config(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports IEEE 802.15.4E-2012 Enhanced Acking.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if 802.15.4E Enhanced Acking is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref
 * SL_RAIL_IEEE802154_SUPPORTS_E_ENHANCED_ACK.
 */
bool sl_rail_ieee802154_supports_e_enhanced_ack(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports IEEE 802.15.4E-2012 Multipurpose frame
 * reception.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if Multipurpose frame reception is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref
 * SL_RAIL_IEEE802154_SUPPORTS_E_MULTIPURPOSE_FRAMES.
 */
bool sl_rail_ieee802154_supports_e_multipurpose_frames(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports the IEEE 802.15.4E-2012 feature
 * subset needed for Zigbee R22 GB868.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if 802.15.4E GB868 subset is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref
 * SL_RAIL_IEEE802154_SUPPORTS_E_SUBSET_GB868.
 */
bool sl_rail_ieee802154_supports_e_subset_gb868(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports IEEE 802.15.4G-2012 reception and
 * transmission of frames with 4-byte CRC.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if 802.15.4G 4-byte CRC is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref
 * SL_RAIL_IEEE802154_SUPPORTS_G_4_BYTE_CRC.
 */
bool sl_rail_ieee802154_supports_g_4_byte_crc(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports IEEE 802.15.4G dynamic FEC.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if dynamic FEC is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref
 * SL_RAIL_IEEE802154_SUPPORTS_G_DYN_FEC.
 */
bool sl_rail_ieee802154_supports_g_dyn_fec(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports Wi-SUN.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if Wi-SUN is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref
 * SL_RAIL_SUPPORTS_PROTOCOL_WI_SUN.
 */
bool sl_rail_supports_protocol_wi_sun(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports Wi-SUN mode switching.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if Wi-SUN mode switching is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref
 * SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH.
 */
bool sl_rail_ieee802154_supports_g_mode_switch(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports IEEE 802.15.4G-2012 feature
 * subset needed for Zigbee R22 GB868.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if 802.15.4G GB868 subset is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref
 * SL_RAIL_IEEE802154_SUPPORTS_G_SUBSET_GB868.
 */
bool sl_rail_ieee802154_supports_g_subset_gb868(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports IEEE 802.15.4G-2012 reception
 * of unwhitened frames.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if 802.15.4G unwhitened frame reception is supported;
 *   false otherwise.
 *
 * Runtime refinement of compile-time \ref
 * SL_RAIL_IEEE802154_SUPPORTS_G_UNWHITENED_RX.
 */
bool sl_rail_ieee802154_supports_g_unwhitened_rx(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports IEEE 802.15.4G-2012 transmission
 * of unwhitened frames.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if 802.15.4G unwhitened frame transmit is supported;
 *   false otherwise.
 *
 * Runtime refinement of compile-time \ref
 * SL_RAIL_IEEE802154_SUPPORTS_G_UNWHITENED_TX.
 */
bool sl_rail_ieee802154_supports_g_unwhitened_tx(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports WMBUS simultaneous M2O RX of T and C modes.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if the WMBUS simultaneous M2O RX of T and C modes is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_WMBUS_SUPPORTS_SIMULTANEOUS_T_C_RX.
 */
bool sl_rail_wmbus_supports_simultaneous_t_c_rx(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports the Z-Wave protocol.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if the Z-Wave protocol is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_PROTOCOL_ZWAVE.
 */
bool sl_rail_supports_protocol_zwave(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports the Z-Wave concurrent PHY.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if the Z-Wave concurrent PHY is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_ZWAVE_SUPPORTS_CONC_PHY.
 */
bool sl_rail_zwave_supports_conc_phy(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports Z-Wave Region in PTI.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if ZWAVE Region in PTI is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_ZWAVE_SUPPORTS_REGION_PTI.
 */
bool sl_rail_zwave_supports_region_pti(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports IEEE 802.15.4 signal identifier.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if signal identifier is supported; false otherwise.
 */
bool sl_rail_ieee802154_supports_signal_identifier(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports fast RX-to-RX.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if fast RX-to-RX is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_FAST_RX_TO_RX.
 */
bool sl_rail_supports_fast_rx_to_rx(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports collision detection.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if collision detection is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_COLLISION_DETECTION.
 */
bool sl_rail_supports_collision_detection(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports Sidewalk protocol.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if Sidewalk protocol is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_PROTOCOL_SIDEWALK.
 */
bool sl_rail_supports_protocol_sidewalk(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports TrustZone secure configuration of
 * peripherals used by RAIL.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if secure mode is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_TRUSTZONE_SECURE_PERIPHERALS.
 */
bool sl_rail_supports_trustzone_secure_peripherals(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports automatic PRS LNA bypass for external
 * FEM.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if automatic PRS LNA bypass is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_PRS_LNA_BYPASS.
 */
bool sl_rail_supports_prs_lna_bypass(sl_rail_handle_t rail_handle);

/**
 * Indicate whether this chip supports RX duty cycling.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if RX duty cycling is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_RX_DUTY_CYCLING.
 */
bool sl_rail_supports_rx_duty_cycling(sl_rail_handle_t rail_handle);

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * Indicate whether RAIL supports the BTC protocol on this chip.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if BTC is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_PROTOCOL_BTC.
 */
bool sl_rail_supports_protocol_btc(sl_rail_handle_t rail_handle);
#endif//DOXYGEN_UNDOCUMENTED

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * Indicate whether RAIL supports the ANT protocol on this chip.
 *
 * @param[in] rail_handle A radio-generic or real RAIL instance handle.
 * @return true if ANT is supported; false otherwise.
 *
 * Runtime refinement of compile-time \ref SL_RAIL_SUPPORTS_PROTOCOL_ANT.
 */
bool sl_rail_supports_protocol_ant(sl_rail_handle_t rail_handle);
#endif//DOXYGEN_UNDOCUMENTED

/** @} */ // end of group Features

/** @} */ // end of group RAIL_API

#ifdef __cplusplus
}
#endif

#ifndef SLI_RAIL_2_X_REMOVE_COMPATIBILITY_LAYER
// Temporarily include RAIL 2.x APIs
#include "rail.h"
#endif

#endif // SL_RAIL_H
