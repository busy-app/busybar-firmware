#pragma once

#include <furi.h>

#define CRYPTO_BACKUP_COMMON_BUFFER_SIZE   512
#define CRYPTO_BACKUP_COMMON_USERDATA_SIZE 1024 * 20
static_assert(
    CRYPTO_BACKUP_COMMON_USERDATA_SIZE % CRYPTO_BACKUP_COMMON_BUFFER_SIZE == 0,
    "User data size must be a multiple of buffer size");

typedef enum {
    CryptoBackupCmdRead,
    CryptoBackupCmdWrite,
    CryptoBackupCmdNwpInit,
    CryptoBackupCmdNwpDeinit,
    CryptoBackupCmdUserDataWipe,
    CryptoBackupCmdAsk,
    CryptoBackupCmdNack,
    CryptoBackupCmdMax,
    CryptoBackupCmdError = 0xFFFFFFFF, /**< Special value for error handling */
} CryptoBackupCmd;

typedef struct FURI_PACKED {
    CryptoBackupCmd cmd; /**< Command type */
    uint32_t address; /**< Address for the command */
    uint32_t data_size; /**< Size of the data to be sent or received */
    uint8_t data[CRYPTO_BACKUP_COMMON_BUFFER_SIZE]; /**< Pointer to the data buffer */
} CryptoBackupEvent;

static_assert(
    sizeof(CryptoBackupEvent) == CRYPTO_BACKUP_COMMON_BUFFER_SIZE + 12,
    "CryptoBackupEvent size mismatch");
