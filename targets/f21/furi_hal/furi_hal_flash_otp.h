#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <core/common_defines.h>

#ifdef __cplusplus
extern "C" {
#endif

// OTP area (512 bytes) is split into 4x 128-byte blocks
#define FURI_HAL_FLASH_OTP_BLOCK_SIZE (128)

#define FURI_HAL_FLASH_OTP_MAGIC (0x3713)

/** OTP block identifiers */
typedef enum {
    FuriHalOtpBlockOtp1 = 1,
    FuriHalOtpBlockOtp2 = 2,
    FuriHalOtpBlockOtp3 = 3,
    FuriHalOtpBlockOtp4 = 4,
} FuriHalFlashOtpBlock;

typedef struct {
    uint16_t magic; // Magic value (FURI_HAL_FLASH_OTP_MAGIC)
    uint8_t index; // OTP block index (FuriHalOtpBlock)
    uint8_t version;
} FURI_PACKED FuriHalFlashOtpHeader;

_Static_assert(sizeof(FuriHalFlashOtpHeader) == 4, "OTP header size mismatch");

/**
 * @brief Check if an OTP header is valid
 * @param header Pointer to OTP header
 * @param expected_block Expected block identifier
 * @return true if magic and index are valid
 */
static inline bool furi_hal_flash_otp_header_is_valid(
    const FuriHalFlashOtpHeader* header,
    FuriHalFlashOtpBlock expected_block) {
    return (header->magic == FURI_HAL_FLASH_OTP_MAGIC) &&
           (header->index == (uint8_t)expected_block);
}

/**
 * @brief Get OTP block base address
 * @param block OTP block identifier
 * @return Base address of the OTP block
 */
uint32_t furi_hal_flash_otp_get_block_address(FuriHalFlashOtpBlock block);

/**
 * @brief Program data to OTP area
 * @param block OTP block identifier
 * @param data Data to program
 * @param length Length of data
 * @return true on success, false if OTP area is not empty or on error
 */
bool furi_hal_flash_otp_program(FuriHalFlashOtpBlock block, const uint8_t* data, uint16_t length);

#ifdef __cplusplus
}
#endif
