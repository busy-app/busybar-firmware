#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stm32u5xx.h>

#define FURI_HAL_OTP_TOTAL_SIZE (FLASH_OTP_SIZE)

// OTP area (512 bytes) is split into 4x 128-byte blocks
#define FURI_HAL_OTP_BLOCK_SIZE (128)

// OTP1 - board info
#define FURI_HAL_OTP_BLOCK1 (FLASH_OTP_BASE)
// OTP2 - device info
#define FURI_HAL_OTP_BLOCK2 (FLASH_OTP_BASE + FURI_HAL_OTP_BLOCK_SIZE)
// OTP3,4 - reserved
#define FURI_HAL_OTP_BLOCK3 (FLASH_OTP_BASE + 2 * FURI_HAL_OTP_BLOCK_SIZE)
#define FURI_HAL_OTP_BLOCK4 (FLASH_OTP_BASE + 3 * FURI_HAL_OTP_BLOCK_SIZE)

bool furi_hal_flash_program_otp(const uint32_t addr, const uint8_t* data, uint16_t length);
