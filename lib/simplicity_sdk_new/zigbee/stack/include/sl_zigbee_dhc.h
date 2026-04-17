/***************************************************************************//**
 * @brief Zigbee Dynamic Hardware Configuration (DHC) API
 *******************************************************************************
 * # License
 * <b>Copyright 2025  Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_ZIGBEE_DHC_H
#define SL_ZIGBEE_DHC_H

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"
#include "stack/include/sl_zigbee_types.h"

/**
 * @addtogroup zigbee_dhc
 * @brief Host - NCP APIs for PA calibration descriptors, curve/table data, CTUNE,
 * RSSI offset, PA mode, voltage, versions and signature.
 * @{ */

// Top-level DHC protocol / dataset version (matches readDhcVersion/writeDhcVersion frames)
#define SL_ZIGBEE_DHC_VERSION               0x01

// Algorithm identifiers (descriptor.algorithm)
#define SL_ZIGBEE_DHC_ALGO_CURVE            0u
#define SL_ZIGBEE_DHC_ALGO_TABLE            1u

// Fixed sizes per YAML (wire format constants)
#define SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT   9u
#define SL_ZIGBEE_DHC_TABLE_ENTRY_COUNT     16u

// Max PA descriptors (must match NCP SL_RAIL_NVM_PA_COUNT; host validates before write)
#define SL_ZIGBEE_DHC_MAX_PA_DESCRIPTORS    2u

// Convenience macro: validate descriptor counts at compile time where possible
#define SL_ZIGBEE_DHC_DESCRIPTOR_IS_CURVE(d) ((d)->algorithm == SL_ZIGBEE_DHC_ALGO_CURVE)
#define SL_ZIGBEE_DHC_DESCRIPTOR_IS_TABLE(d) ((d)->algorithm == SL_ZIGBEE_DHC_ALGO_TABLE)

// Metadata (mirrors sl_zigbee_dhc_pa_metadata_t in ezsp.yaml)
typedef struct {
  uint8_t  version;         // Calibration metadata / dataset version
  uint8_t  num_descriptors; // Number of PA descriptors present
  uint16_t pa_voltage;      // Supply voltage (e.g. mV)
  uint32_t signature;       // Integrity signature / CRC
} sl_zigbee_dhc_pa_metadata_t;

// Descriptor (sl_zigbee_dhc_pa_descriptor_t)
typedef struct {
  uint8_t  algorithm;              // 0=curve, 1=table
  uint8_t  num_segments_or_entries;// 9 for curve, 16 for table
  int16_t  min_ddbm;               // Signed (may be negative)
  int16_t  max_ddbm;               // Signed (may be negative)
} sl_zigbee_dhc_pa_descriptor_t;

// Curve segment (field name 'maxPowerLevel' must match YAML for generated marshal code)
typedef struct {
  uint8_t maxPowerLevel;  // Segment upper bound power level
  int32_t slope;          // Signed slope coefficient
  int32_t intercept;      // Signed intercept coefficient
} sl_zigbee_dhc_pa_curve_segment_t;

// Full curve (fixed-size segments array)
typedef struct {
  int16_t curve_min_ddbm;
  int16_t curve_max_ddbm;
  sl_zigbee_dhc_pa_curve_segment_t segments[SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT];
} sl_zigbee_dhc_pa_curve_t;

// Table (fixed-size entries array)
typedef struct {
  int16_t ddbm_values[SL_ZIGBEE_DHC_TABLE_ENTRY_COUNT];
} sl_zigbee_dhc_pa_table_t;

// Dataset version wrapper
typedef struct {
  uint8_t pa_version;
} sl_zigbee_dhc_pa_version_t;

// Signature wrapper
typedef struct {
  uint32_t pa_signature;
} sl_zigbee_dhc_pa_signature_t;

// Mode wrapper
typedef struct {
  uint8_t pa_mode;
} sl_zigbee_dhc_pa_mode_t;

// RSSI offset wrapper
typedef struct {
  int8_t rssi_offset;
} sl_zigbee_dhc_rssi_offset_t;

// CTUNE wrapper
typedef struct {
  uint32_t ctune;
} sl_zigbee_dhc_ctune_t;

#ifndef ZNET_HEADER_SCRIPT
/**
 * @brief Initialize Dynamic Hardware Configuration from a JSON file.
 *
 * Call this once after bringing up the EZSP link (so the underlying write
 * frames succeed) and before other radio / calibration dependent subsystems
 * query DHC values. The function parses the file and applies any recognized
 * calibration data via the public DHC write APIs.
 *
 * This call is explicit; there is no hidden environment-variable or lazy
 * automatic behavior.
 *
 * @param path Filesystem path to a JSON file (UTF-8, NULL-terminated).
 * @param dry_run If true, validate only (no writes applied).
 * @param stop_on_error If true, abort on first error; otherwise best-effort.
 * @return SL_STATUS_OK on success (or successful validation if dry_run).
 * @return SL_STATUS_NULL_POINTER if path is NULL.
 * @return SL_STATUS_IO / SL_STATUS_NOT_FOUND if the file cannot be opened.
 * @return SL_STATUS_INVALID_PARAMETER for malformed JSON.
 * @return Other codes propagated from lower-level write operations.
 */
sl_status_t sl_zigbee_dhc_init_from_json(const char *path, bool dry_run, bool stop_on_error);
/**
 * @brief Read PA metadata block (version, descriptor count, voltage, signature).
 * @param[out] metadata Pointer to output structure to populate.
 * @return SL_STATUS_OK on success.
 * @return SL_STATUS_NULL_POINTER if metadata is NULL.
 * @return Implementation-specific error if underlying transport/storage fails.
 */
sl_status_t sl_zigbee_dhc_read_pa_metadata(sl_zigbee_dhc_pa_metadata_t *metadata);

/**
 * @brief Write PA metadata block.
 *
 * Caller reads the metadata, modifies fields (like version or signature),
 * then calls sl_zigbee_dhc_write_pa_metadata() to store the updated values.
 *
 * @param[in] metadata Pointer to structure containing new metadata values.
 * @return SL_STATUS_OK on success.
 * @return SL_STATUS_NULL_POINTER if metadata is NULL.
 * @return SL_STATUS_INVALID_PARAMETER if num_descriptors appears inconsistent.
 */
sl_status_t sl_zigbee_dhc_write_pa_metadata(sl_zigbee_dhc_pa_metadata_t *metadata);

/**
 * @brief Read a PA descriptor by index.
 *
 * @param[in] index Descriptor index (0 .. num_descriptors-1).
 * @param[out] descriptor Filled with descriptor contents on success.
 * @return SL_STATUS_OK on success.
 * @return SL_STATUS_NULL_POINTER if descriptor is NULL.
 * @return SL_STATUS_INVALID_INDEX if index is out of range.
 */
sl_status_t sl_zigbee_dhc_read_pa_descriptor(uint8_t index, sl_zigbee_dhc_pa_descriptor_t *descriptor);

/**
 * @brief Write a PA descriptor by index.
 *
 * Caller must set algorithm and num_segments_or_entries consistent with the
 * intended data (curve vs table). Does not automatically resize data arrays.
 *
 * @param[in] index Descriptor index (0 .. num_descriptors-1).
 * @param[in] descriptor New descriptor data.
 * @return SL_STATUS_OK on success.
 * @return SL_STATUS_NULL_POINTER if descriptor is NULL.
 * @return SL_STATUS_INVALID_INDEX if index is out of range.
 * @return SL_STATUS_INVALID_PARAMETER if fields are inconsistent.
 */
sl_status_t sl_zigbee_dhc_write_pa_descriptor(uint8_t index, sl_zigbee_dhc_pa_descriptor_t *descriptor);

/**
 * @brief Read full PA curve (all segments) for a curve descriptor.
 * @param[in] index Descriptor index whose algorithm must be curve.
 * @param[out] curve Curve output structure.
 * @return SL_STATUS_OK on success.
 * @return SL_STATUS_INVALID_INDEX if index invalid.
 * @return SL_STATUS_INVALID_TYPE if descriptor is not a curve (impl-specific).
 */
sl_status_t sl_zigbee_dhc_read_pa_curve(uint8_t index, sl_zigbee_dhc_pa_curve_t *curve);

/**
 * @brief Write full PA curve (all segments) for a curve descriptor.
 * @param[in] index Descriptor index.
 * @param[in] curve Curve structure to store.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sl_zigbee_dhc_write_pa_curve(uint8_t index, sl_zigbee_dhc_pa_curve_t *curve);

/**
 * @brief Read a single curve segment.
 * @param[in] index Descriptor index (curve algorithm).
 * @param[in] segment_index Segment index (0 .. SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT-1).
 * @param[out] segment Output segment.
 * @return SL_STATUS_OK on success.
 * @return SL_STATUS_INVALID_INDEX if either index out of range.
 */
sl_status_t sl_zigbee_dhc_read_pa_curve_segment(uint8_t index, uint8_t segment_index, sl_zigbee_dhc_pa_curve_segment_t *segment);

/**
 * @brief Write a single curve segment.
 * @param[in] index Descriptor index.
 * @param[in] segment_index Segment index.
 * @param[in] segment New segment values.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sl_zigbee_dhc_write_pa_curve_segment(uint8_t index, uint8_t segment_index, sl_zigbee_dhc_pa_curve_segment_t *segment);

/**
 * @brief Read full PA table (all ddbm entries) for a table descriptor.
 * @param[in] index Descriptor index whose algorithm must be table.
 * @param[out] table Output table structure.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sl_zigbee_dhc_read_pa_table(uint8_t index, sl_zigbee_dhc_pa_table_t *table);

/**
 * @brief Write full PA table.
 * @param[in] index Descriptor index (table algorithm).
 * @param[in] table Table data to write.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sl_zigbee_dhc_write_pa_table(uint8_t index, sl_zigbee_dhc_pa_table_t *table);

/**
 * @brief Read RSSI offset scalar.
 * @param[out] rssi_offset Output structure (signed dB offset).
 * @return SL_STATUS_OK on success.
 */
sl_status_t sl_zigbee_dhc_read_rssi_offset(sl_zigbee_dhc_rssi_offset_t *rssi_offset);

/**
 * @brief Write RSSI offset scalar.
 * @param[in] rssi_offset New value wrapper.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sl_zigbee_dhc_write_rssi_offset(sl_zigbee_dhc_rssi_offset_t *rssi_offset);

/**
 * @brief Read PA voltage scalar (mV) from metadata storage.
 * @param[out] pa_voltage Pointer to uint16_t to receive voltage.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sl_zigbee_dhc_read_pa_voltage(uint16_t *pa_voltage);

/**
 * @brief Write PA voltage scalar (mV).
 * @param[in] pa_voltage Millivolt supply reference.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sl_zigbee_dhc_write_pa_voltage(uint16_t pa_voltage);

/**
 * @brief Read PA mode.
 * @param[out] pa_mode Mode wrapper.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sl_zigbee_dhc_read_pa_mode(sl_zigbee_dhc_pa_mode_t *pa_mode);

/**
 * @brief Write PA mode.
 * @param[in] pa_mode New mode wrapper.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sl_zigbee_dhc_write_pa_mode(sl_zigbee_dhc_pa_mode_t *pa_mode);

/**
 * @brief Read CTUNE calibration value.
 * @param[out] ctune Output wrapper (uint32 value).
 * @return SL_STATUS_OK on success.
 */
sl_status_t sl_zigbee_dhc_read_ctune(sl_zigbee_dhc_ctune_t *ctune);

/**
 * @brief Write CTUNE calibration value.
 * @param[in] ctune New ctune wrapper.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sl_zigbee_dhc_write_ctune(sl_zigbee_dhc_ctune_t *ctune);

/**
 * @brief Read transient top-level DHC version (distinct from metadata.version).
 * @param[out] dhc_version Byte to receive version.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sl_zigbee_dhc_read_dhc_version(uint8_t *dhc_version);

/**
 * @brief Write transient top-level DHC version.
 * @param[in] dhc_version New value.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sl_zigbee_dhc_write_dhc_version(uint8_t dhc_version);

/**
 * @brief Read PA dataset version wrapper (pa_version).
 * @param[out] pa_version Output structure.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sl_zigbee_dhc_read_pa_version(sl_zigbee_dhc_pa_version_t *pa_version);


/**
 * @brief Read PA signature wrapper.
 * @param[out] pa_signature Output signature.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sl_zigbee_dhc_read_pa_signature(sl_zigbee_dhc_pa_signature_t *pa_signature);

/**
 * @brief Write PA signature wrapper.
 * @param[in] pa_signature New signature.
 * @return SL_STATUS_OK on success.
 */
sl_status_t sl_zigbee_dhc_write_pa_signature(sl_zigbee_dhc_pa_signature_t *pa_signature);

#endif /* ZNET_HEADER_SCRIPT */

/** @} (end addtogroup zigbee_dhc) */
#endif // SL_ZIGBEE_DHC_H
