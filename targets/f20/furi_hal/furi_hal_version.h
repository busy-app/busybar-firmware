/**
 * @file furi_hal_version.h
 * Version HAL API
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <furi.h>
#include <version/version.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Get hardware target
 *
 * @return     Hardware Target
 */
uint8_t furi_hal_version_get_hw_target(void);

/** Get BLE MAC address
 *
 * @return     pointer to BLE MAC address
 */
const uint8_t* furi_hal_version_get_ble_mac(void);

/** Get address of version structure of firmware.
 *
 * @return     Address of firmware version structure.
 */
const struct Version* furi_hal_version_get_firmware_version(void);

/** Get device serial number (UID)
  *
  * @param[in]  serial     a string to store the value
  */
void furi_hal_version_get_uid_str(FuriString* serial);

#ifdef __cplusplus
}
#endif
