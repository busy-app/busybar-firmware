#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stm32u5xx.h>

#define FURI_HAL_OTP_TOTAL_SIZE (FLASH_OTP_SIZE)

// OTP area (512 bytes) is split into 4x 128-byte blocks
#define FURI_HAL_OTP_BLOCK_SIZE (128)

// OTP1 - board/hardware info (production data)
#define FURI_HAL_OTP_BLOCK1 (FLASH_OTP_BASE)
// OTP2 - device info (QC data: color, region)
#define FURI_HAL_OTP_BLOCK2 (FLASH_OTP_BASE + FURI_HAL_OTP_BLOCK_SIZE)
// OTP3 - public key storage for signature verification
#define FURI_HAL_OTP_BLOCK3 (FLASH_OTP_BASE + 2 * FURI_HAL_OTP_BLOCK_SIZE)
// OTP4 - signatures for OTP1 and OTP2
#define FURI_HAL_OTP_BLOCK4 (FLASH_OTP_BASE + 3 * FURI_HAL_OTP_BLOCK_SIZE)

// ============================================================================
// OTP Header Definition
// ============================================================================

// Magic value for OTP block validation (appears as "13 37" in hex dump due to little-endian)
#define FURI_HAL_OTP_MAGIC (0x3713)

// OTP block indices
#define FURI_HAL_OTP_INDEX_OTP1 (1)
#define FURI_HAL_OTP_INDEX_OTP2 (2)
#define FURI_HAL_OTP_INDEX_OTP3 (3)
#define FURI_HAL_OTP_INDEX_OTP4 (4)

// Common OTP header structure (4 bytes)
typedef struct __attribute__((packed)) {
    uint16_t magic; // Magic value (FURI_HAL_OTP_MAGIC)
    uint8_t index; // OTP block index (1-4)
    uint8_t version; // OTP structure version
} FuriHalOtpHeader;

_Static_assert(sizeof(FuriHalOtpHeader) == 4, "OTP header size mismatch");

/**
 * @brief Check if an OTP header is valid
 * @param header Pointer to OTP header
 * @param expected_index Expected block index (1-4)
 * @return true if magic and index are valid
 */
static inline bool
    furi_hal_otp_header_is_valid(const FuriHalOtpHeader* header, uint8_t expected_index) {
    return (header->magic == FURI_HAL_OTP_MAGIC) && (header->index == expected_index);
}

/**
 * @brief Program data to OTP area
 * @param addr OTP address (must be 16-byte aligned)
 * @param data Data to program
 * @param length Length of data
 * @return true on success, false if OTP area is not empty or on error
 */
bool furi_hal_flash_program_otp(const uint32_t addr, const uint8_t* data, uint16_t length);
