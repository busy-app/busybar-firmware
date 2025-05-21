#include "crypto.h"
#include <furi.h>
#include <args.h>
#include <cli_worker.h>
#include "sl_si91x_driver.h"

#define NWP_FLASH_START_ADDRESS   0
#define NWP_FLASH_END_ADDRESS     (1024 * 20)
#define SIZE_BUF                  (1024 * 4)
#define FLASH_SECTOR_ERASE_ENABLE 0
#define MAGIC_NUMBER_KEY          0x464c4950
#define MAX_KEY_SLOT              32

typedef struct {
    uint32_t magic_number;
    uint16_t key_slot;
    uint16_t key_size;
    uint32_t key_type;
    uint32_t key_flags;
    uint8_t key_data[112];
} FURI_PACKED FuriHalCryptoKey;
_Static_assert(sizeof(FuriHalCryptoKey) == 128, "Size check for 'FuriHalCryptoKey' failed.");

static bool crypto_check_key_slot_is_free(uint32_t key_slot) {
    sl_status_t status = 0;
    uint32_t address = NWP_FLASH_START_ADDRESS + key_slot * sizeof(FuriHalCryptoKey);
    FuriHalCryptoKey* key = malloc(sizeof(FuriHalCryptoKey));
    bool ret = false;
    uint32_t i = 0;
    do {
        if(key == NULL) {
            printf("Failed to allocate memory\r\n");
            break;
        }
        status = sl_si91x_command_to_read_common_flash(
            address, sizeof(FuriHalCryptoKey), (uint8_t*)key);
        if(status != SL_STATUS_OK) {
            printf("Failed to read from NWP flash: 0x%lx\r\n", status);
            break;
        }
        uint8_t* p_key = (uint8_t*)key;
        while(i < sizeof(FuriHalCryptoKey)) {
            if(p_key[i] != 0xFF) {
                break;
            }
            i++;
        }

        if(i == sizeof(FuriHalCryptoKey)) {
            ret = true;
        }

    } while(false);
    free(key);
    return ret;
}

static bool crypto_write_key(FuriHalCryptoKey* key) {
    furi_check(key);

    if(key->key_slot >= MAX_KEY_SLOT) {
        printf("Key slot %d is out of range\r\n", key->key_slot);
        return false;
    }

    if(!crypto_check_key_slot_is_free(key->key_slot)) {
        printf("Key slot %d is not free\r\n", key->key_slot);
        return false;
    }

    sl_status_t status = 0;
    uint32_t address = NWP_FLASH_START_ADDRESS + key->key_slot * sizeof(FuriHalCryptoKey);
    status = sl_si91x_command_to_write_common_flash(
        address, (uint8_t*)key, sizeof(FuriHalCryptoKey), 0);
    if(status != SL_STATUS_OK) {
        printf("Failed to write to NWP flash: 0x%lx\r\n", status);
        return false;
    }
    return true;
}

static bool crypto_read_key(FuriHalCryptoKey* key) {
    furi_check(key);

    if(key->key_slot >= MAX_KEY_SLOT) {
        printf("Key slot %d is out of range\r\n", key->key_slot);
        return false;
    }
    uint16_t key_slot = key->key_slot;

    sl_status_t status = 0;
    uint32_t address = NWP_FLASH_START_ADDRESS + key->key_slot * sizeof(FuriHalCryptoKey);
    status =
        sl_si91x_command_to_read_common_flash(address, sizeof(FuriHalCryptoKey), (uint8_t*)key);
    if(status != SL_STATUS_OK) {
        printf("Failed to read from NWP flash: 0x%lx\r\n", status);
        return false;
    }
    if(key->magic_number != MAGIC_NUMBER_KEY) {
        printf("Key slot %d is not valid\r\n", key_slot);
        return false;
    }
    return true;
}

void crypto_command_wipe(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(cli);

    UNUSED(args);

    sl_status_t status = sl_si91x_command_to_write_common_flash(
        NWP_FLASH_START_ADDRESS, NULL, NWP_FLASH_END_ADDRESS, 1);
    if(status != SL_STATUS_OK) {
        printf("Failed to wipe NWP flash: 0x%lx\r\n", status);
        return;
    }
    printf("Wipe NWP flash\r\n");
}

void crypto_command_write_all(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(cli);

    UNUSED(args);
    sl_status_t status = 0;
    uint32_t address = NWP_FLASH_START_ADDRESS;
    uint8_t* buf = malloc(SIZE_BUF);
    if(buf == NULL) {
        printf("Failed to allocate memory\r\n");
        return;
    }
    for(int i = 0; i < SIZE_BUF; i++) {
        buf[i] = i % 256;
    }

    for(uint32_t i = 0; i < 20; i++) {
        status = sl_si91x_command_to_write_common_flash(
            NWP_FLASH_START_ADDRESS + i * 1024, buf, 1024, 0);
        if(status != SL_STATUS_OK) {
            printf("Failed to write to NWP flash: 0x%lx\r\n", status);
            free(buf);
            return;
        }
        printf("Write data to NWP flash:\r\n");

        for(int i = 0; i < 1024; i++) {
            if((i) % 32 == 0) printf("%08lx :", address);
            printf("%02x ", buf[i]);
            if((i + 1) % 32 == 0) {
                printf("\r\n");
                address += 32;
            }
        }
        printf("\r\n");
    }

    free(buf);
}

void crypto_command_write_key(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(cli);
    UNUSED(args);

    FuriHalCryptoKey* key = malloc(sizeof(FuriHalCryptoKey));
    if(key == NULL) {
        printf("Failed to allocate memory\r\n");
        return;
    }

    key->magic_number = MAGIC_NUMBER_KEY;
    key->key_slot = 1;
    key->key_size = 100;
    key->key_type = 0;
    key->key_flags = 0;
    for(int i = 0; i < key->key_size; i++) {
        key->key_data[i] = i % 256;
    }

    if(!crypto_write_key(key)) {
        printf("Failed to write key\r\n");
    } else {
        printf("Write key to NWP flash slot: %d \r\n", key->key_slot);
    }
    free(key);
}

void crypto_command_read_key(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(cli);
    UNUSED(args);

    FuriHalCryptoKey* key = malloc(sizeof(FuriHalCryptoKey));
    if(key == NULL) {
        printf("Failed to allocate memory\r\n");
        return;
    }

    key->key_slot = 1;
    if(!crypto_read_key(key)) {
        printf("Failed to read key\r\n");
    } else {
        printf("Read key from NWP flash slot: %d \r\n", key->key_slot);
        printf("Magic number: %lx\r\n", key->magic_number);
        printf("Key slot: %d\r\n", key->key_slot);
        printf("Key size: %d\r\n", key->key_size);
        printf("Key type: %ld\r\n", key->key_type);
        printf("Key flags: %ld\r\n", key->key_flags);
        printf("Key data:\r\n");
        for(uint32_t i = 0; i < key->key_size; i++) {
            if((i) % 32 == 0) printf("%08lx :", i);
            printf("%02x ", key->key_data[i]);
            if((i + 1) % 32 == 0) {
                printf("\r\n");
            }
        }
        printf("\r\n");
    }
    free(key);
}

void crypto_command_dump(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(cli);
    UNUSED(args);
    sl_status_t status = 0;
    uint32_t address = NWP_FLASH_START_ADDRESS;
    uint8_t* buf = malloc(SIZE_BUF);
    if(buf == NULL) {
        printf("Failed to allocate memory\r\n");
        return;
    }
    for(uint32_t i = 0; i < 20; i++) {
        status =
            sl_si91x_command_to_read_common_flash(NWP_FLASH_START_ADDRESS + i * 1024, 1024, buf);
        if(status != SL_STATUS_OK) {
            printf("Failed to read from NWP flash: 0x%lx\r\n", status);
            free(buf);
            return;
        }
        printf("Read data from NWP flash:\r\n");

        for(int i = 0; i < 1024; i++) {
            if((i) % 32 == 0) printf("%08lx :", address);

            printf("%02x ", buf[i]);

            if((i + 1) % 32 == 0) {
                printf("\r\n");
                address += 32;
            }
        }
    }
    free(buf);
}

static void crypto_command_print_usage(void) {
    printf("Usage:\r\n");
    printf("crypto <cmd> <args>\r\n");
    printf("Cmd list:\r\n");

    printf("\tcrypro wipe\r\n");
    printf("\tcrypro dump\r\n");
    printf("\tcrypro write_all\r\n");
    printf("\tcrypro read_key\r\n");
    printf("\tcrypro write_key\r\n");
}

static void crypto_command(Cli* cli, FuriString* args, void* context) {
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            crypto_command_print_usage();
            break;
        }

        if(furi_string_cmp_str(cmd, "write_all") == 0) {
            crypto_command_write_all(cli, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "dump") == 0) {
            crypto_command_dump(cli, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "wipe") == 0) {
            crypto_command_wipe(cli, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "write_key") == 0) {
            crypto_command_write_key(cli, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "read_key") == 0) {
            crypto_command_read_key(cli, args, context);
            break;
        }

        crypto_command_print_usage();
    } while(false);

    furi_string_free(cmd);
}

void crypto_system_start(void) {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);

    cli_add_command(cli, "crypto", CliCommandFlagParallelSafe, crypto_command, NULL);

    furi_record_close(RECORD_CLI);
#else
    UNUSED(crypto_command);
#endif
}
