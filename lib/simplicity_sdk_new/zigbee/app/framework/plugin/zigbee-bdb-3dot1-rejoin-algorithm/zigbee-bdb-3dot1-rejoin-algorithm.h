/***************************************************************************//**
 * @file
 * @brief Definitions for the BDB 3.1 rejoin algorithm.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SILABS_ZIGBEE_BDB_3DOT1_REJOIN_ALGORITHM_H
#define SILABS_ZIGBEE_BDB_3DOT1_REJOIN_ALGORITHM_H

#include PLATFORM_HEADER

/**
 * @brief Various states of the network steering process.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum sl_zigbee_af_bdb_3dot1_rejoin_state_t
#else
enum
#endif //DOXYGEN_SHOULD_SKIP_THIS
{
  SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_NONE                             = 0x00,  /**< (0x00) Steady state. Do nothing. */
  SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_SECURE_REJOIN_ON_CURRENT_CHANNEL = 0x01,  /**< (0x01) Issue a secure rejoin on the current channel */
  SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_TC_REJOIN_ON_CURRENT_CHANNEL     = 0x02,  /**< (0x02) Issue a TC rejoin on the current channel */
  SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_REJOIN_ON_PRIMARY_CHANNELS       = 0x03,  /**< (0x03) Issue a TC rejoin on the primary channel mask */
  SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_REJOIN_ON_SECONDARY_CHANNELS     = 0x04,  /**< (0x04) Issue a TC rejoin on the secondary channel mask */
  SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_REJOIN_LAST_STATE,
};
typedef uint8_t sl_zigbee_af_bdb_3dot1_rejoin_state_t;

typedef struct {
  bool enabled;
  sl_zigbee_af_bdb_3dot1_rejoin_state_t state;
  bool tc_keepalive_failure;
  bool poll_control_failure;
  uint32_t rejoin_attempts;
} sl_zigbee_af_bdb_rejoin_state_t;

/**
 * @defgroup bdb-3dot1-rejoin-algorithm BDB 3.1 Rejoin Algorithm
 * @ingroup component
 * @brief API and Callbacks for the BDB 3.1 Rejoin Algorithm Component
 *
 * This component defines routines for handling the rejoin mechanism as described
 * in the BDB 3.1 specification.
 *
 */

/**
 * @addtogroup bdb-3dot1-rejoin-algorithm
 * @{
 */

/** @brief Enable or disable the BDB 3.1 rejoin algorithm. The rejoin algorithm is enabled by default, and
 *   the enable state is not persisted across reboots.
 *
 * @param[in] true to enable the rejoin algorithm, false to disable it. If the algorithm is disabled,
 *  the application will not automatically attempt to rejoin upon a network down event. In such a case,
 *  it will be up to the user application to restore network functionality.
 *
 * @return void
 */
void sl_zigbee_af_bdb_3dot1_enable_rejoin_algorithm(bool enable);

/** @} END addtogroup */

#endif // SILABS_ZIGBEE_BDB_3DOT1_REJOIN_ALGORITHM_H
