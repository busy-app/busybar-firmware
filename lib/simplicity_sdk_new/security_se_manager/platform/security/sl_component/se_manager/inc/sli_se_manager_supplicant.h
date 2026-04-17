/***************************************************************************//**
 * @file
 * @brief Silicon Labs Secure Engine Manager API.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef SLI_SE_MANAGER_SUPPLICANT_H
#define SLI_SE_MANAGER_SUPPLICANT_H

#include "sli_se_manager_features.h"

#if defined(SLI_MAILBOX_COMMAND_SUPPORTED)

/// @addtogroup sl_se_manager
/// @{

/***************************************************************************//**
 * @addtogroup sli_se_manager_supplicant Supplicant
 *
 * @brief
 *   Supplicant acceleration APIs.
 *
 * @details
 *   APIs for accelerating various Dragonfly operations.
 *
 * @{
 ******************************************************************************/

  #include "sl_se_manager_key_handling.h"
  #include "sl_se_manager_types.h"
  #include "sli_se_manager_mailbox.h"
  #include "sl_status.h"

  #ifdef __cplusplus
    extern "C" {
  #endif

  typedef struct {
    uint8_t *scalar;
    uint8_t *element;
    sl_se_buffer_t pwe;
    sl_se_buffer_t rand;
    sl_se_key_descriptor_t kck;
  } sli_se_sae_ephemeral_context_t;

  typedef struct {
    sl_se_key_descriptor_t pmk;
    sli_se_sae_ephemeral_context_t ephemeral_ctx;
    sl_se_key_type_t group;
    const void *domain;
  } sli_se_sae_context_t;

// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************//**
 * @brief
 *   This function generates a Password Element(PWE),
 *   a commit scalar and a commit element.
 *
 * @note
 *   Only the NIST256 Weierstrass curve is supported.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 *
 * @param[in] sae_ctx
 *   Pointer to sli_se_sae_context_t structure.
 *
 * @param[in] mac_ap
 *   MAC address of the access point (6 bytes).
 *
 * @param[in] mac_client
 *   MAC address of the client (6 bytes).
 *
 * @param[in] password
 *   The password used to derive the PWE.
 *
 * @param[in] password_len
 *   Length of the password.
 *
 * @param[in] h2e
 *   Boolean indicating whether to use H2E (Hash-to-Element) method for PWE derivation.
 *
 * @param[in] identifier
 *   Identifier used in H2E method to derive PWE (if h2e is true).
 *
 * @param[in] identifier_len
 *   Length of the identifier.
 *
 * @param[in] ssid
 *   SSID used in H2E method to derive PT (if h2e is true).
 *
 * @param[in] ssid_len
 *   Length of the SSID.
 *
 * @param[in] iterations
 *   Number of iterations to use in the Hunting-and-Pecking process. Hardcoded to 40 for now.
 *
 * @return
 *   SL_STATUS_OK when the command was executed successfully, otherwise an
 *   appropriate error code (@ref sl_status.h).
 ******************************************************************************/
  sl_status_t sli_se_sae_prepare_commit(sl_se_command_context_t *cmd_ctx,
                                        sli_se_sae_context_t     *sae_ctx,
                                        const uint8_t           *mac_ap,
                                        const uint8_t           *mac_client,
                                        size_t                  mac_len,
                                        const uint8_t           *password,
                                        size_t                  password_len,
                                        bool                    h2e,
                                        const uint8_t           *identifier, // for h2e to derive pwe
                                        size_t                  identifier_len,
                                        uint8_t                 *ssid, // for h2e to derive pt
                                        size_t                  ssid_len,
                                        uint32_t                iterations);

/***************************************************************************//**
 * @brief
 *   This function generates a SAE Key Conformation Key(SAE-KCK) and a
 *   Pairwise Master Key(PMK) from the received peer commit scalar and
 *   element, the locally generated commit scalar and the PWE.
 *
 * @note
 *   Only the NIST256 Weierstrass curve is supported.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 *
 * @param[in] sae_ctx
 *   Pointer to sli_se_sae_context_t structure.
 *
 * @param[in] peer_scalar
 *   The received peer scalar.
 *
 * @param[in] peer_element
 *   The received peer element.
 *
 * @return
 *   SL_STATUS_OK when the command was executed successfully, otherwise an
 *   appropriate error code (@ref sl_status.h).
 ******************************************************************************/
  sl_status_t sli_se_sae_process_commit(sl_se_command_context_t *cmd_ctx,
                                        sli_se_sae_context_t     *sae_ctx,
                                        const uint8_t           *peer_scalar,
                                        const uint8_t           *peer_element);


  #ifdef __cplusplus
    }
  #endif

/// @} (end addtogroup sli_se_manager_supplicant)
/// @} (end addtogroup sli_se_manager)

#endif // defined(SLI_MAILBOX_COMMAND_SUPPORTED)

#endif // SLI_SE_MANAGER_SUPPLICANT_H
