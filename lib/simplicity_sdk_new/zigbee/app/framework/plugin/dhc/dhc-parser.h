/***************************************************************************/ /**
 * @file
 * @brief  Zigbee Dynamic Hardware Configuration (DHC) Parser API
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

#ifndef DHC_PARSER_H
#define DHC_PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"

// Keep historical macro for backward compatibility but prefer SL_ZIGBEE_DHC_VERSION
#ifndef DHC_VERSION
#define DHC_VERSION SL_ZIGBEE_DHC_VERSION
#endif

/**
 * @brief Parser control flags (bitmask) for DHC JSON ingestion.
 */
typedef enum {
  DHC_PARSE_FLAG_DRY_RUN       = 1u << 0,  //!< Validate only; perform no writes
  DHC_PARSE_FLAG_STOP_ON_ERROR = 1u << 1, //!< Abort on first error (instead of best-effort)
} dhc_parse_status_flags_t;

/**
 * @brief Parse a legacy DHC JSON buffer in memory.
 *
 * Auto-detects the legacy schema (presence of top-level keys such as
 * 'pa_curves') and applies recognized fields using the indexed DHC API.
 * Behavior is modified by @p flags (dry-run / stop-on-error).
 *
 * @param[in] json_text NULL-terminated JSON text buffer.
 * @param[in] flags Bitwise OR of dhc_parse_status_flags_t values.
 * @return SL_STATUS_OK on success (and all applicable writes unless DRY_RUN).
 * @return SL_STATUS_NULL_POINTER if json_text is NULL.
 * @return SL_STATUS_INVALID_PARAMETER if the JSON structure is malformed.
 * @return Other implementation-specific status codes if underlying writes fail.
 */
sl_status_t sl_zigbee_af_dhc_parse_json(const char *json_text, uint32_t flags);

/**
 * @brief Parse a legacy DHC JSON file from a filesystem path.
 *
 * Loads the file contents then delegates to @ref sl_zigbee_af_dhc_parse_json with the same
 * @p flags. Large files are processed entirely in memory.
 *
 * @param[in] path Path to JSON file (UTF-8, NULL-terminated).
 * @param[in] flags Bitwise OR of dhc_parse_status_flags_t values.
 * @return SL_STATUS_OK on success.
 * @return SL_STATUS_NULL_POINTER if path is NULL.
 * @return SL_STATUS_NOT_FOUND if the file cannot be opened/read.
 * @return Propagated status codes from @ref sl_zigbee_af_dhc_parse_file for parse errors.
 */
sl_status_t sl_zigbee_af_dhc_parse_file(const char *path, uint32_t flags);

#endif // DHC_PARSER_H
