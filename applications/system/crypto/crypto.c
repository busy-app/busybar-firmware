#include "crypto.h"
#include <furi_hal_crypto_storage.h>
#include <furi.h>
#include <args.h>
#include <strint.h>
#include <cli_worker.h>
#include "sl_si91x_driver.h"

#define CRYPTO_SIZE_BUF (1024 * 1)

void crypto_command_wipe(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(cli);

    UNUSED(args);

    sl_status_t status = sl_si91x_command_to_write_common_flash(
        FURI_HAL_CRYPTO_STORAGE_START_ADDRESS, NULL, FURI_HAL_CRYPTO_STORAGE_END_ADDRESS, 1);
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
    sl_status_t status = SL_STATUS_FAIL;
    uint32_t address = FURI_HAL_CRYPTO_STORAGE_START_ADDRESS;
    uint8_t* buf = malloc(CRYPTO_SIZE_BUF);
    if(buf == NULL) {
        printf("Failed to allocate memory\r\n");
        return;
    }
    for(int i = 0; i < CRYPTO_SIZE_BUF; i++) {
        buf[i] = i % 256;
    }

    for(uint32_t i = 0; i < 20; i++) {
        status = sl_si91x_command_to_write_common_flash(
            FURI_HAL_CRYPTO_STORAGE_START_ADDRESS + i * 1024, buf, 1024, 0);
        if(status != SL_STATUS_OK) {
            printf("Failed to write to NWP flash: 0x%lx\r\n", status);
            free(buf);
            return;
        }
        printf("Write data to NWP flash:\r\n");

        for(int i = 0; i < 1024; i++) {
            if((i) % 32 == 0) printf("%08lx: ", address);
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

    key->magic_number = FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY;

    if(furi_string_size(args)) {
        char* args_cstr = (char*)furi_string_get_cstr(args);
        StrintParseError parse_err = StrintParseNoError;
        parse_err |= strint_to_uint16(args_cstr, &args_cstr, &key->key_slot, 10);
        parse_err |= strint_to_uint16(args_cstr, &args_cstr, &key->key_size, 10);
        if(!parse_err && (key->key_size > sizeof(key->key_data))) {
            cli_print_usage(
                "crypto write_key", "<key_size> of range\r\n", furi_string_get_cstr(args));
            free(key);
            return;
        }
        uint32_t temp = 0;
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &temp, 10);
        key->key_type = (FuriHalCryptoKeyType)temp;
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &temp, 16);
        key->key_flags = (FuriHalCryptoKeyFlag)temp;
        furi_string_printf(args, "%s", args_cstr);
        furi_string_trim(args);
        if(parse_err || !args_read_hex_bytes(args, key->key_data, key->key_size)) {
            cli_print_usage(
                "crypto write_key",
                "<key_slot><key_size><key_type><key_flag: in HEX><key_data: in byte>\r\n",
                furi_string_get_cstr(args));
            free(key);
            return;
        }
    } else {
        cli_print_usage(
            "crypto write_key",
            "<key_slot><key_size><key_type><key_flag: in HEX><key_data: in byte>\r\n",
            furi_string_get_cstr(args));
        free(key);
        return;
    }

    if(!furi_hal_crypto_storage_write_key(key)) {
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

    if(furi_string_size(args)) {
        char* args_cstr = (char*)furi_string_get_cstr(args);
        StrintParseError parse_err = StrintParseNoError;
        parse_err |= strint_to_uint16(args_cstr, &args_cstr, &key->key_slot, 10);
        if(parse_err) {
            cli_print_usage(
                "crypto read_key",
                "<key_slot> Read key from NWP flash slot\r\n",
                furi_string_get_cstr(args));
            free(key);
            return;
        }
    } else {
        cli_print_usage(
            "crypto read_key",
            "<key_slot> Read key from NWP flash slot\r\n",
            furi_string_get_cstr(args));
        free(key);
        return;
    }

    if(!furi_hal_crypto_storage_read_key(key)) {
        printf("Failed to read key\r\n");
    } else {
        printf("Read key from NWP flash slot: %d \r\n", key->key_slot);
        printf("Magic number: %lx\r\n", key->magic_number);
        printf("Key slot: %d\r\n", key->key_slot);
        printf("Key size: %d\r\n", key->key_size);
        printf("Key type: %ld\r\n", (uint32_t)key->key_type);
        printf("Key flags: 0x%08lX\r\n", (uint32_t)key->key_flags);
        printf("Key data:\r\n");
        for(uint32_t i = 0; i < key->key_size; i++) {
            if((i) % 32 == 0) printf("%08lx: ", i);
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
    sl_status_t status = SL_STATUS_FAIL;
    uint32_t address = FURI_HAL_CRYPTO_STORAGE_START_ADDRESS;
    uint8_t* buf = malloc(CRYPTO_SIZE_BUF);
    if(buf == NULL) {
        printf("Failed to allocate memory\r\n");
        return;
    }
    for(uint32_t i = 0; i < 20; i++) {
        status = sl_si91x_command_to_read_common_flash(
            FURI_HAL_CRYPTO_STORAGE_START_ADDRESS + i * 1024, 1024, buf);
        if(status != SL_STATUS_OK) {
            printf("Failed to read from NWP flash: 0x%lx\r\n", status);
            free(buf);
            return;
        }
        printf("Read data from NWP flash:\r\n");

        for(int i = 0; i < 1024; i++) {
            if((i) % 32 == 0) printf("%08lx: ", address);

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

    printf("\tcrypto wipe Clear crypto storage\r\n");
    printf("\tcrypto dump Dump crypto storage\r\n");
    printf("\tcrypto write_all Write random date to crypto storage\r\n");
    printf("\tcrypto read_key <key_slot> Read key from NWP flash slot\r\n");
    printf(
        "\tcrypto write_key <key_slot><key_size><key_type><key_flag: in HEX><key_data: in Byte> Write key from NWP flash slot\r\n");
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
