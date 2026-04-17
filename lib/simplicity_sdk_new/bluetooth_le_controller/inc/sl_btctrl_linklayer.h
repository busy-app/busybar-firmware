/***************************************************************************//**
 * @brief Bluetooth Link Layer configuration API
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

#ifndef _SL_BTCTRL_LINKLAYER_H_
#define _SL_BTCTRL_LINKLAYER_H_
#include "sl_btctrl_linklayer_defs.h"

/**
 * Link Layer init
 */
sl_status_t sl_btctrl_init(void);
sl_status_t sl_btctrl_init_internal(struct sl_btctrl_config *config);
void sl_btctrl_deinit(void);

/**
 * Allocate controller memory
 *
 * @returns Status indicating success of allocation
 */
sl_status_t sl_btctrl_init_mem(const struct sl_btctrl_config *config);

/**
 * Release all memory allocated by controller
 */
void sl_btctrl_deinit_mem(void);

/**
 * Perform functional initialization of the controller
 *
 * This function requires that controller memory has been successfully allocated
 * with a prior call to @ref sl_btctrl_init_mem.
 *
 * @returns Status indicating success of initialization
 */
sl_status_t sl_btctrl_init_functional(struct sl_btctrl_config *config);

/**
 * Perform functional de-initialization of the controller
 *
 * To release all memory allocated by the controller, call @ref
 * sl_btctrl_deinit_mem after this function.
 */
void sl_btctrl_deinit_functional(void);

/**
 * Bgbuf init and link layer init
 *
 * @returns Status indicating success of allocation
 */
sl_status_t sl_btctrl_init_ll(const struct sl_btctrl_config *config);

/**
 * Bgbuf deinit and link layer deinit
 *
 * @returns Status indicating success of allocation
 */
sl_status_t sl_btctrl_deinit_ll(void);

/**
 *  Configures how many maximum sized ACL data packets
 *  can the controller store.
 */
void sl_btctrl_configure_le_buffer_size(uint8_t count);

void sli_btctrl_set_interrupt_priorities(const struct sl_btctrl_config *config);

void sli_btctrl_set_address(uint8_t *address);

//Initialize memory objects used by LinkLayer
//In future these should be configured individually
sl_status_t sl_btctrl_init_basic(uint8_t connections, uint8_t adv_sets, uint8_t whitelist);

void sli_btctrl_events_init(void);

/**
 * Initialize and enable adaptive frequency hopping
 */
sl_status_t sl_btctrl_init_afh(uint32_t flags);

/**
 * Enable high power use under appropriate conditions
 */
void sl_btctrl_init_highpower(void);

/**
 * @brief Initilize periodic advertiser
 */
void sl_btctrl_init_periodic_adv();

/**
 * @brief Initilize periodic advertiser
 */
void sl_btctrl_init_periodic_scan();

/**
 * @brief Configure transmit power to the radio.
 * The new transmit power becomes effective immediately. Configuration includes
 * power mode (amplifier) configuration if necessary. The configuration is not
 * permanent and the active Bluetooth procedures may change it at any time.
 * Therefore, this function is useful only for testing purposes.
 * @param[in] tx_power_ddbm The power level in units of deci-dBm to be used for
 *       transmission. The value must *not* be RF path compensated since
 *       compensation will be applied automatically. */
void sl_btctrl_set_tx_power(int16_t power_ddbm);

/**
 * @brief Enable and initialize support for the PAwR advertiser.
 * @param[in] pawr_adv_config PAwR advertiser configuration.
 * @return SL_STATUS_OK, or an appropriate error code. */
sl_status_t sl_btctrl_pawr_advertiser_configure(struct sl_btctrl_pawr_advertiser_config *pawr_adv_config);

/**
 * @brief Enable and initialize support for PAwR sync/receiver.
 * @param[in] pawr_sync_config PAwR synchronizer configuration.
 * @return SL_STATUS_OK, or an appropriate error code. */
sl_status_t sl_btctrl_pawr_synchronizer_configure(struct sl_btctrl_pawr_synchronizer_config *pawr_sync_config);

/**
 * @brief Allocate memory for synchronized scanners
 *
 * @param num_scan Number of Periodic Scanners Allowed
 * @return SL_STATUS_OK if allocation was succesfull, failure reason otherwise
 */
sl_status_t sl_btctrl_alloc_periodic_scan(uint8_t num_scan);

/**
 * @brief Allocate memory for periodic advertisers
 *
 * @param num_adv Number of advertisers to allocate
 */
sl_status_t sl_btctrl_alloc_periodic_adv(uint8_t num_adv);

/**
 * @brief Set maximum number of advertisement reports allowed to be queued
 *
 * @param num_adv Maximum number of advertisement reports allowed to be queued
 */
void sl_btctrl_configure_max_queued_adv_reports(uint8_t num_reports);

/**
 * Call to enable the even connection scheduling algorithm.
 * This function should be called before link layer initialization.
 */
void sl_btctrl_enable_even_connsch();

/**
 * Call to enable connection statistics collection.
 */
void sl_btctrl_init_conn_statistics(void);

/**
 * Call to initialize multiprotocol
 * in bluetooth controller
 */
void sl_btctrl_init_multiprotocol();

/**
 * Link with symbol to enable radio watchdog
 */
void sl_btctrl_enable_radio_watchdog();

/**
 * Get radio counters for troubleshooting purposes.
 * @return Pointer to the radio counters. */
const struct sl_btctrl_radio_counters *sl_btctrl_radio_get_counters(void);

/**
 * Reset radio counters. */
void sl_btctrl_radio_reset_counters(void);

/**
 * Initialize CTE receiver
 */
sl_status_t sl_btctrl_init_cte_receiver();

/**
 * Initialize CTE transmitter
 */
sl_status_t sl_btctrl_init_cte_transmitter();

/**
 * Initialize both CTE receiver and transmitter
 *
 * Note: This is for backward compatibility. It is recommend to
 * use sl_btctrl_init_cte_receiver and sl_btctrl_init_cte_transmitter
 * functions instead.
 */
sl_status_t sl_btctrl_init_cte();

/**
 * Initialize Channel Sounding
 */
sl_status_t sl_btctrl_init_cs(const struct sl_btctrl_cs_config *config);

/**
 * Check if event bitmap indicates pending events
 * @return bool pending events
 */
bool sli_pending_btctrl_events(void);

/**
 * Check if the controller supports Coded PHY.
 * @return True if Coded PHY is supported, false otherwise.
 */
bool sl_btctrl_radio_supports_coded_phy();

/**
 * Check if the controller supports 2M PHY.
 * @return True if 2M PHY is supported, false otherwise.
 */
bool sl_btctrl_radio_supports_2m_phy();

/**
 * Disable the support for Coded and Simulscan PHYs.
 */
void sl_btctrl_disable_coded_phy(void);

/**
 * Disable the support for 2M PHY.
 */
void sl_btctrl_disable_2m_phy(void);

/**
 * Initialize adv component
 */
void sl_btctrl_init_adv(void);

void sl_btctrl_init_conn(void);

void sl_btctrl_init_subrate(void);

sl_status_t sl_btctrl_allocate_conn_subrate_memory(uint8_t connectionsCount);

void sl_btctrl_init_channel_classification(void);

sl_status_t sl_btctrl_allocate_channel_classification_memory(uint8_t connectionsCount);

void sl_btctrl_init_phy(void);

void sl_btctrl_init_adv_ext(void);

/**
 * Set the advertiser address or tx power to be used for all advertisers
 * @param adv_config The advertiser configuration.
 * @return sl_status_ok, or an appropriate status.
 */
sl_status_t sl_btctrl_config_adv(struct sl_btctrl_adv_config *adv_config);

void sl_btctrl_init_privacy(void);

sl_status_t sl_btctrl_allocate_resolving_list_memory(uint8_t resolvingListSize);

void sl_btctrl_init_even_anchor_selection(void);

void sl_btctrl_init_empty_center_anchor_selection(void);

/**
 * @brief Initialize extended scanner state
 *
 */
void sl_btctrl_init_scan_ext(void);

/**
 * @brief Initialize radio early rx abort
 *
 */
void sl_btctrl_init_radio_early_rx_abort(void);

void sl_btctrl_init_scan(void);

/**
 * @brief return true if controller is initialized
 *
 */
bool sl_btctrl_is_initialized();

/**
 * @brief Sets PAST initiator feature bit,
 * and links in PAST sender and ll_adv_sync symbols to the project.
 */
void sl_btctrl_init_past_local_sync_transfer(void);

/**
 * @brief Sets PAST initiator feature bit,
 * and links in PAST sender, ll_scan_sync and ll_scan_sync_registry symbols to the project.
 */
void sl_btctrl_init_past_remote_sync_transfer(void);

/**
 * @brief Sets PAST receiver feature bit,
 * and links in PAST receiver, ll_scan_sync and ll_scan_sync_registry symbols to the project.
 */
void sl_btctrl_init_past_receiver(void);

/**
 * @brief Configure how often to send the Number Of Completed Packets HCI event.
 * @param[in] packets When the controller has transmitted this number of ACL data packets it will send
 * the Number Of Completed Packets HCI event to the host.
 * @param[in] events When this number of connection events have passed and the controller did not yet report
 * all the transmitted packets, then it will send the Number Of Completed Packets HCI event to the host.
 */
void sl_btctrl_configure_completed_packets_reporting(uint8_t packets, uint8_t events);

/**
 * @brief Configure the power control configuration. This configuration is used when the power control
 * feature is present.
 * @param[in] power_ctrl_cfg The power control configuration.
 */
sl_status_t sl_btctrl_init_power_control(const uint8_t *power_ctrl_cfg);

sl_status_t sl_bt_init_app_controlled_tx_power(void);

/**
 * Initial sniff, open to customer
 */
sl_status_t sl_btctrl_init_sniff(uint8_t num);
/**
 * Deinitial sniff, open to customer
 */
void sl_btctrl_deinit_sniff(void);

void sl_btctrl_init_config(struct sl_btctrl_config *config);

/**
 * @brief Configure the minimum and maximum TX power levels used by the device.
 * @param[in] min_power The maximum TX power level.
 * @param[in] max_power The minimum TX power level.
 */
sl_status_t sli_btctrl_set_min_max_tx_power(int16_t *min_power, int16_t *max_power);

void sli_btctrl_enable_peripheral_feature_exchange(bool enable);

/**
 * Get the radio context handle for Bluetooth
 * @return The radio context handle
 */
void* sli_btctrl_get_radio_context_handle(void);

/**
 * Get a snapshot of Link layer configuration flags
 * @param flags only flags specified here is returned
 */
uint32_t sl_btctrl_get_config_flags(uint32_t flags);

/**
 * Set Link Layer configuration flags
 */
void sl_btctrl_set_config_flags(uint32_t flags);

/**
 * Clear Link Layer configuration flags
 */
void sl_btctrl_clear_config_flags(uint32_t flags);

/**
 * For backward compatibility
 */
static inline uint32_t BTLE_LL_GetFlags(uint32_t flags)
{
  return sl_btctrl_get_config_flags(flags);
}

static inline void BTLE_LL_SetFlags(uint32_t flags)
{
  sl_btctrl_set_config_flags(flags);
}

static inline void BTLE_LL_ClearFlags(uint32_t flags)
{
  sl_btctrl_clear_config_flags(flags);
}
/**
 * @brief Set new radio priorities from config parameter
 * @param priorities priority table to copy parameters from
 * */
void sl_btctrl_configure_scheduler_priorities(sl_btctrl_ll_priorities *priorities);

/**
 * Linklayer tasklet initialization
 */
void sl_btctrl_init_tasklets(void);
void sl_btctrl_init_adv_tasklet(void);
void sl_btctrl_init_iso_conn_tasklet(void);
void sl_btctrl_init_conn_tasklet(void);
void sl_btctrl_init_dtm_tasklet(void);
void sl_btctrl_init_scan_tasklet(void);

/**
 * @brief Initializes event info reporting vendor specific feature.
 */
void sl_btctrl_init_event_info_report(void);

#endif
