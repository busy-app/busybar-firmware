/***************************************************************************//**
 * @file
 * @brief Zigbee Dynamic Hardware Configuration (DHC) NCP API
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: LicenseRef-MSLA
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of the Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement
 * By installing, copying or otherwise using this software, you agree to the
 * terms of the MSLA.
 *
 ******************************************************************************/

#ifndef DHC_NCP_H
#define DHC_NCP_H

#include <stdint.h>
#include "sl_zigbee_dhc.h"
#include "sl_status.h"

// --- Adapter prototypes expected by generated command handlers (indexed API) ---
// These convert between the wire/public DHC structs (sl_zigbee_dhc_*) and the
// underlying RAIL NVM storage (sl_rail_nvm_*).
#include "sl_zigbee_dhc.h"

/**
 * @brief Read PA metadata from underlying RAIL/NVM backing store.
 * @param[out] metadata Output metadata structure.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_read_pa_metadata(sl_zigbee_dhc_pa_metadata_t *metadata);
/**
 * @brief Write PA metadata to backing store (overwrites full block).
 * @param[in] metadata New metadata values.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_write_pa_metadata(sl_zigbee_dhc_pa_metadata_t *metadata);
/**
 * @brief Read descriptor at index.
 * @param[in] index Descriptor index.
 * @param[out] descriptor Descriptor output.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_read_pa_descriptor(uint8_t index, sl_zigbee_dhc_pa_descriptor_t *descriptor);
/**
 * @brief Write descriptor at index.
 * @param[in] index Descriptor index.
 * @param[in] descriptor Descriptor input.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_write_pa_descriptor(uint8_t index, sl_zigbee_dhc_pa_descriptor_t *descriptor);
/**
 * @brief Read full curve (all segments) for given descriptor index.
 * @param[in] index Curve descriptor index.
 * @param[out] curve Curve output.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_read_pa_curve(uint8_t index, sl_zigbee_dhc_pa_curve_t *curve);
/**
 * @brief Write full curve (all segments) for given descriptor index.
 * @param[in] index Curve descriptor index.
 * @param[in] curve Curve input data.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_write_pa_curve(uint8_t index, sl_zigbee_dhc_pa_curve_t *curve);
/**
 * @brief Read single curve segment.
 * @param[in] index Curve descriptor index.
 * @param[in] segment_index Segment index (0..N-1).
 * @param[out] segment Segment output.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_read_pa_curve_segment(uint8_t index, uint8_t segment_index, sl_zigbee_dhc_pa_curve_segment_t *segment);
/**
 * @brief Write single curve segment.
 * @param[in] index Curve descriptor index.
 * @param[in] segment_index Segment index.
 * @param[in] segment Segment data to store.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_write_pa_curve_segment(uint8_t index, uint8_t segment_index, sl_zigbee_dhc_pa_curve_segment_t *segment);
/**
 * @brief Read full PA table for given descriptor index.
 * @param[in] index Table descriptor index.
 * @param[out] table Table output.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_read_pa_table(uint8_t index, sl_zigbee_dhc_pa_table_t *table);
/**
 * @brief Write full PA table for given descriptor index.
 * @param[in] index Table descriptor index.
 * @param[in] table Table input data.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_write_pa_table(uint8_t index, sl_zigbee_dhc_pa_table_t *table);
/**
 * @brief Read RSSI offset scalar.
 * @param[out] rssi_offset Output value wrapper.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_read_rssi_offset(sl_zigbee_dhc_rssi_offset_t *rssi_offset);
/**
 * @brief Write RSSI offset scalar.
 * @param[in] rssi_offset New offset.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_write_rssi_offset(sl_zigbee_dhc_rssi_offset_t *rssi_offset);
/**
 * @brief Read PA voltage (mV) from metadata backing store.
 * @param[out] pa_voltage Pointer to receive voltage.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_read_pa_voltage(uint16_t *pa_voltage);
/**
 * @brief Write PA voltage (mV) into metadata backing store.
 * @param[in] pa_voltage Millivolt value.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_write_pa_voltage(uint16_t pa_voltage);
/**
 * @brief Read PA mode scalar.
 * @param[out] pa_mode Output mode wrapper.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_read_pa_mode(sl_zigbee_dhc_pa_mode_t *pa_mode);
/**
 * @brief Write PA mode scalar.
 * @param[in] pa_mode New mode wrapper.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_write_pa_mode(sl_zigbee_dhc_pa_mode_t *pa_mode);
/**
 * @brief Read CTUNE calibration value.
 * @param[out] ctune Output ctune wrapper.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_read_ctune(sl_zigbee_dhc_ctune_t *ctune);
/**
 * @brief Write CTUNE calibration value.
 * @param[in] ctune New ctune wrapper.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_write_ctune(sl_zigbee_dhc_ctune_t *ctune);
/**
 * @brief Read top-level (transient) DHC version.
 * @param[out] dhc_version Output version byte.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_read_dhc_version(uint8_t *dhc_version);
/**
 * @brief Write top-level (transient) DHC version.
 * @param[in] dhc_version New version byte.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_write_dhc_version(uint8_t dhc_version);
/**
 * @brief Read PA dataset version wrapper.
 * @param[out] pa_version Output struct.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_read_pa_version(sl_zigbee_dhc_pa_version_t *pa_version);
/**
 * @brief Read PA signature wrapper.
 * @param[out] pa_signature Output signature struct.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_read_pa_signature(sl_zigbee_dhc_pa_signature_t *pa_signature);
/**
 * @brief Write PA signature wrapper.
 * @param[in] pa_signature New signature struct.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sli_zigbee_stack_write_pa_signature(sl_zigbee_dhc_pa_signature_t *pa_signature);

#endif // DHC_NCP_H
