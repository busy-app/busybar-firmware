#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    FuriHalSdVersion1,
    FuriHalSdVersion2,
} FuriHalSdVersion;

typedef enum {
    FuriHalSdTypeSC, /*!< SD Standard Capacity <2G */
    FuriHalSdTypeHCXC, /*!< SD High Capacity <32G, SD Extended Capacity <2T */
    FuriHalSdTypeMMCLowCapacity, /*!< MMC Low Capacity */
    FuriHalSdTypeMMCHighCapacity, /*!< MMC High Capacity */
} FuriHalSdType;

typedef enum {
    FuriHalSdSpeedNormal, /*!< Normal Speed Card <12.5Mb/s, Spec Version 1.01 */
    FuriHalSdSpeedHigh, /*!< High Speed Card <25Mb/s , Spec version 2.00 */
    FuriHalSdSpeedUltraHigh, /*!< UHS-I SD Card <50Mb/s for SDR50, DDR5 Cards and <104Mb/s for SDR104, Spec version 3.01 */
} FuriHalSdSpeed;

typedef struct {
    uint32_t logical_block_count; /*!< logical capacity in blocks */
    uint32_t logical_block_size; /*!< logical block size in bytes */

    FuriHalSdVersion version; /*!< SD version */
    FuriHalSdType type; /*!< SD type */
    FuriHalSdSpeed speed; /*!< SD speed */

    uint8_t manufacturer_id; /*!< manufacturer ID */
    char oem_id[3]; /*!< OEM ID, 2 characters + null terminator */
    char product_name[6]; /*!< product name, 5 characters + null terminator */
    uint8_t product_revision_major; /*!< product revision major */
    uint8_t product_revision_minor; /*!< product revision minor */
    uint32_t product_serial_number; /*!< product serial number */
    uint8_t manufacturing_month; /*!< manufacturing month */
    uint16_t manufacturing_year; /*!< manufacturing year */
} FuriHalSdInfo;

typedef void (*FuriHalSdMmcPresentCallback)(void* context);

void furi_hal_sdmmc_init(bool have_rtos);

void furi_hal_sdmmc_set_presence_callback(FuriHalSdMmcPresentCallback callback, void* context);

bool furi_hal_sdmmc_is_sd_present(void);

bool furi_hal_sdmmc_init_card(void);

void furi_hal_sdmmc_deinit_card(void);

bool furi_hal_sdmmc_read_blocks(
    uint8_t* buffer,
    uint32_t address,
    uint32_t count,
    size_t timeout_ms_per_block);

bool furi_hal_sdmmc_write_blocks(
    const uint8_t* buffer,
    uint32_t address,
    uint32_t count,
    size_t timeout_ms);

bool furi_hal_sdmmc_get_card_info(FuriHalSdInfo* info);

bool furi_hal_sd_alive(void);
