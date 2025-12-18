#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    FuriHalNvmFlagDebug,
    FuriHalNvmFlagDetailedFilename,
    FuriHalNvmFlagStorageFormatInternal,
    FuriHalNvmFlagRebootIntoShippingMode,
    FuriHalNvmFlagCount, /**< Keep this last */
} FuriHalNvmFlag;

typedef enum {
    FuriHalNvmBootModeNormal, /**< Normal boot mode, default value */
    FuriHalNvmBootModeUpdate, /**< Update boot mode */
} FuriHalNvmBootMode;

/** Perform early NVM initialization and state verification
 */
void furi_hal_nvm_init_early(void);

/** Perform NVM initialization
 *
 * This function is called after the system is initialized.
 */
void furi_hal_nvm_init(void);

/** Discard ALL NVM contents */
void furi_hal_nvm_reset(void);

/** Set NVM Flag
 *
 * @param[in]  flag  The flag to set
 */
void furi_hal_nvm_set_flag(FuriHalNvmFlag flag);

/** Clear NVM Flag
 *
 * @param[in]  flag  The flag to clear
 */
void furi_hal_nvm_reset_flag(FuriHalNvmFlag flag);

/** Check if NVM Flag is set
 *
 * @param[in]  flag  The flag to check
 *
 * @return     true if set
 */
bool furi_hal_nvm_is_flag_set(FuriHalNvmFlag flag);

/** Get NVM boot mode
 *
 * @return     The NVM boot mode.
 */
FuriHalNvmBootMode furi_hal_nvm_get_boot_mode(void);

/** Set NVM boot mode
 *
 * @param[in]  mode  The boot mode to set
 */
void furi_hal_nvm_set_boot_mode(FuriHalNvmBootMode mode);

/** Store fault data
 *
 * @return     The fault data.
 */
void furi_hal_nvm_set_fault_data(uint32_t value);

void furi_hal_nvm_store_switch_pos(uint32_t pos);

uint32_t furi_hal_nvm_get_switch_pos(void);
