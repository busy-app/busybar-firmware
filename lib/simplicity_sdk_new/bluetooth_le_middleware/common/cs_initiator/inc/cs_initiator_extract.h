/***************************************************************************//**
 * @file
 * @brief CS initiator - extract header
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
#ifndef CS_INITIATOR_EXTRACT_H
#define CS_INITIATOR_EXTRACT_H

// -----------------------------------------------------------------------------
// Includes

#include "cs_initiator_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

// -----------------------------------------------------------------------------
// Function declarations

/******************************************************************************
 * Reset subevent data and synchronization for an initiator instance.
 *
 * @param[in] initiator initiator instance reference
 * @param[in] init true if reset was done in the initialization phase
 *****************************************************************************/
void reset_subevent_data(cs_initiator_t *initiator, bool init);

/******************************************************************************
 * Extract cs results (step data, subevent data) and hand over to RTL Lib
 * instance to calculate distance out of it
 *
 * @param[in] initiator initiator instance reference
 * @param[in] cs_result_content pointer to the CS result content
 *
 * @return true if procedure restart required, false otherwise
 *****************************************************************************/
cs_procedure_state_t extract_cs_result_data(cs_initiator_t *initiator,
                                            cs_result_data_t *cs_result_content);

/******************************************************************************
 * Check if ranging data is complete
 *
 * @param[in] data Ranging data
 * @param[in] size Size of ranging data
 * @param[in] is_initiator True for initiator
 * @param[in] antenna_path_num Number of antenna paths
 *
 * @return CS_PROCEDURE_STATE_COMPLETED if procedure is completed
 * @return CS_PROCEDURE_STATE_ABORTED otherwise
 *****************************************************************************/
cs_procedure_state_t ranging_data_is_complete(uint8_t *data,
                                              uint32_t size,
                                              bool is_initiator,
                                              uint8_t antenna_path_num);

#ifdef __cplusplus
}
#endif

#endif // CS_INITIATOR_EXTRACT_H
