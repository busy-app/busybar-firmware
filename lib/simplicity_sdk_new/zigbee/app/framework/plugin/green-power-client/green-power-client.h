/***************************************************************************//**
 * @file
 * @brief Bookkeeping for Commissioning related info.
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

#include "green-power-client-config.h"

/**
 * @defgroup green-power-client Green Power Client
 * @ingroup component cluster
 * @brief API and Callbacks for the Green Power Cluster Client Component
 *
 * A component implementing the client-side functionality of the Green Power cluster.
 */

/**
 * @addtogroup green-power-client
 * @{
 */

/** @brief Shows if we are on transmit channel (1) or on operational channel (0) */
#define GP_CLIENT_ON_TRANSMIT_CHANNEL_MASK BIT(0)
/** @brief Shows if operational channel is same as transmit channel */
#define GP_CLIENT_TRANSMIT_SAME_AS_OPERATIONAL_CHANNEL_MASK BIT(1)
/** @brief Shows if there is a channel request pending */
#define GP_CLIENT_ADDITIONAL_CHANNEL_REQUEST_PENDING BIT(2)

/**
 * @brief Options for MAC Sequence Number Capability
 */
typedef enum  {
  SL_ZIGBEE_GP_GPD_MAC_SEQ_NUM_CAP_SEQUENTIAL  = 0x00, /**< Sequence Number is sequential */
  SL_ZIGBEE_GP_GPD_MAC_SEQ_NUM_CAP_RANDOM      = 0x01, /**< Sequence Number is random */
} sl_zigbee_gp_gpd_mac_seq_num_cap_t;

/**
 * @brief GP Client Commissioning Mode exit flags
 */
typedef enum {
  SL_ZIGBEE_AF_GPC_COMMISSIONING_EXIT_ON_COMMISSIONING_WINDOW_EXP = 0x1, /**< On Commissioning Window expiration */
  SL_ZIGBEE_AF_GPC_COMMISSIONING_EXIT_ON_FIRST_PAIRING_SUCCESS = 0x2, /**< On first Pairing success */
  SL_ZIGBEE_AF_GPC_COMMISSIONING_EXIT_ON_GP_PROXY_COMMISSIONING_MODE_EXIT = 0x4, /**< On GP Proxy Commissioning Mode (exit) */
  SL_ZIGBEE_AF_GPC_COMMISSIONING_EXIT_MODE_MAX = 0xFF, /**< Upper bound for exit mode mask; not used as an actual exit condition */
} sl_zigbee_af_green_power_client_commissioning_exit_mode_t;

/**
 * @brief GP Client Commissioning State
 */
typedef struct {
  bool inCommissioningMode; /**< If GP Client is in Commissioning Mode */
  sl_zigbee_af_green_power_client_commissioning_exit_mode_t exitMode; /**< GP Client Commissioning Mode exit flag */
  uint16_t gppCommissioningWindow; /**< GP Proxy Commissioning Window */
  uint8_t channel; /**< GP Client current channel */
  bool unicastCommunication; /**< If Commissioning is unicast or broadcast */
  sl_802154_short_addr_t commissioningSink; /**< Address of GP Commissioning Sink */
  uint8_t channelStatus; /**< GP Client status flags. Bit 0 shows if we are on transmit channel(1) or on operational channel(0). bit 1 shows if operational channel is same as transmit channel (1) */
} sl_zigbee_af_green_power_client_commissioning_state_t;

/**
 * @brief GP address duplicate filters
 */
typedef struct {
  sl_zigbee_gp_address_t addrs[SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_ADDR_ENTRIES]; /**< GP address entries */
  uint8_t randomSeqNums[SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_ADDR_ENTRIES][SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_SEQ_NUM_ENTRIES_PER_ADDR]; /**< Sequence Numbers per address entries */
  uint32_t expirationTimes[SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_ADDR_ENTRIES][SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_SEQ_NUM_ENTRIES_PER_ADDR]; /**< Sequence Number expiration times */
} sl_zigbee_af_green_power_duplicate_filter_t;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
bool sli_zigbee_af_gp_message_checking(sl_zigbee_gp_address_t *gpAddr, uint8_t sequenceNumber);
#endif //DOXYGEN_SHOULD_SKIP_THIS

/**
 * @name API
 * @{
 */

/** @brief Clear the proxy table.
 *
 * This function clears the proxy table.
 *
 */
void sl_zigbee_af_green_power_client_clear_proxy_table(void);

/** @} */ // end of name API

/**
 * @name Callbacks
 * @{
 */

/**
 * @defgroup gp_client_cb Green Power Client
 * @ingroup af_callback
 * @brief Callbacks for Green Power Client Component
 *
 */

/**
 * @addtogroup gp_client_cb
 * @{
 */

/** @brief Green power client Sink table based forward callback.
 *
 * This function is called by the Green Power client before forwarding the GP notification.
 * This callback provides the pointer to the group list for the paired gpd.
 * In case of a combo application, where the green power server is also present, this callback
 * is consumed by the green power server to update the sinklist from the sink table.
 *
 * @param[in] addr gpd address
 * @param[in] sinkList sink list pointer that can be used to update the sink list
 * @param[in] maxNumberEntries maximum number of sink list entries
 */
void sl_zigbee_af_green_power_client_gpdf_sink_table_based_forward_cb(sl_zigbee_gp_address_t *addr,
                                                                      sl_zigbee_gp_sink_list_entry_t *sinkList,
                                                                      uint8_t maxNumberEntries);

/** @} */ // end of gp_client_cb
/** @} */ // end of name Callbacks

/** @} */ // end of green-power-client
