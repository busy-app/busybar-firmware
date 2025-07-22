#include "crypto.h"
#include <furi_hal_crypto_storage.h>
#include <furi_hal_crypto.h>

#include <furi.h>
#include <cli/args.h>
#include <cli/cli_ansi.h>
#include <cli/cli_status.h>
#include <strint.h>
#include "sl_si91x_driver.h"

#include <sl_net.h>
#include "wifi_config.h"

#include "furi_hal_nwp.h"

static void crypto_command_show_status(FuriHalCryptoStatus status, const char* name) {
    switch(status) {
    case FuriHalCryptoStatusOk:
        printf(ANSI_FG_GREEN "Key %s successfully\r\n" ANSI_RESET, name);
        break;
    case FuriHalCryptoStatusFail:
        printf(ANSI_FG_RED "Error: Failed to %s key " ANSI_RESET "Fail\r\n", name);
        break;
    case FuriHalCryptoStatusFailWrite:
        printf(ANSI_FG_RED "Error: Failed to %s key " ANSI_RESET "Write error\r\n", name);
        break;
    case FuriHalCryptoStatusStorageFull:
        printf(ANSI_FG_RED "Error: Failed to %s key " ANSI_RESET "Storage full\r\n", name);
        break;
    case FuriHalCryptoStatusDuplicate:
        printf(ANSI_FG_RED "Error: Failed to %s key " ANSI_RESET "Duplicate key\r\n", name);
        break;
    case FuriHalCryptoStatusNotFound:
        printf(ANSI_FG_RED "Error: Failed to %s key " ANSI_RESET "Key not found\r\n", name);
        break;
    case FuriHalCryptoStatusErrorCrc:
        printf(ANSI_FG_RED "Error: Failed to %s key " ANSI_RESET "CRC error\r\n", name);
        break;
    default:
        printf(ANSI_FG_RED "Error: Failed to %s key " ANSI_RESET "Unknown error\r\n", name);
        break;
    }
    if(status != FuriHalCryptoStatusOk) {
        printf(CLI_STATUS_ERROR);
    } else {
        printf(CLI_STATUS_OK);
    }
}

static const char* crypto_command_show_type(FuriHalCryptoKeyType type) {
    switch(type) {
    case FuriHalCryptoKeyTypeAes128:
        return "FuriHalCryptoKeyTypeAes128";
    case FuriHalCryptoKeyTypeAes192:
        return "FuriHalCryptoKeyTypeAes192";
    case FuriHalCryptoKeyTypeAes256:
        return "FuriHalCryptoKeyTypeAes256";
        break;
    case FuriHalCryptoKeyTypeHmacSha1:
        return "FuriHalCryptoKeyTypeHmacSha1";
    case FuriHalCryptoKeyTypeHmacSha256:
        return "FuriHalCryptoKeyTypeHmacSha256";
    case FuriHalCryptoKeyTypeHmacSha384:
        return "FuriHalCryptoKeyTypeHmacSha384";
        break;
    case FuriHalCryptoKeyTypeHmacSha512:
        return "FuriHalCryptoKeyTypeHmacSha512";
    case FuriHalCryptoKeyTypeEcdsaPriv224:
        return "FuriHalCryptoKeyTypeEcdsaPriv224";
    case FuriHalCryptoKeyTypeEcdsaPriv256:
        return "FuriHalCryptoKeyTypeEcdsaPriv256";
        break;
    case FuriHalCryptoKeyTypeEcdsaPub224:
        return "FuriHalCryptoKeyTypeEcdsaPub224";
    case FuriHalCryptoKeyTypeEcdsaPub256:
        return "FuriHalCryptoKeyTypeEcdsaPub256";
    case FuriHalCryptoKeyTypeMatterDAC:
        return "FuriHalCryptoKeyTypeMatterDAC";
        break;
    case FuriHalCryptoKeyTypeMatterPAI:
        return "FuriHalCryptoKeyTypeMatterPAI";
    case FuriHalCryptoKeyTypeMatterCD:
        return "FuriHalCryptoKeyTypeMatterCD";
    case FuriHalCryptoKeyTypeMatterVID_PID:
        return "FuriHalCryptoKeyTypeMatterVID_PID";
        break;
    case FuriHalCryptoKeyTypeMatterSPAKE2:
        return "FuriHalCryptoKeyTypeMatterSPAKE2";
        break;
    default:
        return "Unknown type";
        break;
    }
}

static bool crypto_command_is_init(void) {
    bool ret = furi_hal_nwp_is_initialized();
    if(!ret) {
        printf(ANSI_FG_RED
               "Error: NWP is not initialized, please run 'crypto init' first\r\n" ANSI_RESET);
        printf(CLI_STATUS_ERROR);
    }
    return ret;
}

static void crypto_command_init(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    if(furi_hal_nwp_init()) {
        printf(ANSI_FG_GREEN "NWP initialized\r\n" ANSI_RESET);
        printf(CLI_STATUS_OK);
    } else {
        printf(ANSI_FG_RED "Error: Failed to initialize NWP\r\n" ANSI_RESET);
        printf(CLI_STATUS_ERROR);
    }
}

static void crypto_command_deinit(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    if(furi_hal_nwp_deinit()) {
        printf(ANSI_FG_GREEN "NWP deinitialized\r\n" ANSI_RESET);
        printf(CLI_STATUS_OK);
    } else {
        printf(ANSI_FG_RED "Error: Failed to deinitialize NWP\r\n" ANSI_RESET);
        printf(CLI_STATUS_ERROR);
    }
}

void crypto_command_wipe(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    if(!crypto_command_is_init()) {
        return;
    }
    uint16_t partition = FuriHalCryptoPartitionMax;
    bool is_valid = true;
    if(furi_string_size(args)) {
        char* args_cstr = (char*)furi_string_get_cstr(args);
        StrintParseError parse_err = StrintParseNoError;
        parse_err |= strint_to_uint16(args_cstr, &args_cstr, &partition, 10);
        if(parse_err) {
            is_valid = false;
        }
        if(partition >= FuriHalCryptoPartitionMax) {
            is_valid = false;
        }
    } else {
        is_valid = false;
    }
    if(!is_valid) {
        cli_print_usage(
            "crypto wipe",
            "<partition> 0-partition_main, 1-partition_user\r\n",
            furi_string_get_cstr(args));
        printf(CLI_STATUS_ERROR);
        return;
    }

    sl_status_t status = SL_STATUS_FAIL;
    switch(partition) {
    case FuriHalCryptoPartitionMain:
        status = sl_si91x_command_to_write_common_flash(
            FURI_HAL_CRYPTO_STORAGE_PARTITION_MAIN_START_ADDRESS,
            NULL,
            FURI_HAL_CRYPTO_STORAGE_PARTITION_MAIN_END_ADDRESS -
                FURI_HAL_CRYPTO_STORAGE_PARTITION_MAIN_START_ADDRESS + 1,
            1);
        if(status != SL_STATUS_OK) {
            printf(
                ANSI_FG_RED "Error: Failed to wipe NWP flash partition_main: " ANSI_RESET
                            "0x%08lx\r\n",
                status);
            printf(CLI_STATUS_ERROR);
        } else {
            printf(ANSI_FG_GREEN "Wipe NWP flash partition_main\r\n" ANSI_RESET);
            printf(CLI_STATUS_OK);
        }
        break;
    case FuriHalCryptoPartitionUser:
        status = sl_si91x_command_to_write_common_flash(
            FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_START_ADDRESS,
            NULL,
            FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_END_ADDRESS -
                FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_START_ADDRESS + 1,
            1);
        if(status != SL_STATUS_OK) {
            printf(
                ANSI_FG_RED "Error: Failed to wipe NWP flash partition_user: " ANSI_RESET
                            "0x%08lx\r\n",
                status);
            printf(CLI_STATUS_ERROR);
        } else {
            printf(ANSI_FG_GREEN "Wipe NWP flash partition_user\r\n" ANSI_RESET);
            printf(CLI_STATUS_OK);
        }
        break;
    default:
        furi_crash();
    }
}

void crypto_command_write(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    if(!crypto_command_is_init()) {
        return;
    }
    uint32_t temp = 0xFF;
    FuriHalCryptoKey* key = NULL;
    FuriHalCryptoPartition partition = FuriHalCryptoPartitionMax;
    if(furi_string_size(args)) {
        char* args_cstr = (char*)furi_string_get_cstr(args);
        StrintParseError parse_err = StrintParseNoError;

        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &temp, 10);
        partition = (FuriHalCryptoPartition)temp;
        if(parse_err || (partition >= FuriHalCryptoPartitionMax)) {
            cli_print_usage(
                "crypto write",
                "<partition> 0-partition_main, 1-partition_user\r\n",
                furi_string_get_cstr(args));
            printf(CLI_STATUS_ERROR);
            return;
        }

        key = furi_hal_crypto_storage_alloc(partition);

        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &temp, 10);
        key->header.type = (FuriHalCryptoKeyType)temp;
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &key->header.id, 16);
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &temp, 16);
        key->header.flags = (FuriHalCryptoKeyFlag)temp;
        parse_err |= strint_to_uint16(args_cstr, &args_cstr, &key->header.size, 10);
        if(!parse_err && (key->header.size > key->length)) {
            cli_print_usage("crypto write", "<size> of range\r\n", furi_string_get_cstr(args));
            furi_hal_crypto_storage_free(key);
            printf(CLI_STATUS_ERROR);
            return;
        }

        furi_string_printf(args, "%s", args_cstr);
        furi_string_trim(args);
        if(parse_err || !args_read_hex_bytes(args, key->data, key->header.size)) {
            cli_print_usage(
                "crypto write",
                "<partition> <type> <id: in HEX> <flags: in HEX> <size> <data: in byte>\r\n",
                furi_string_get_cstr(args));
            furi_hal_crypto_storage_free(key);
            printf(CLI_STATUS_ERROR);
            return;
        }
    } else {
        cli_print_usage(
            "crypto write",
            "<partition> <type> <id: in HEX> <flags: in HEX> <size> <data: in byte>\r\n",
            furi_string_get_cstr(args));
        printf(CLI_STATUS_ERROR);
        return;
    }
    furi_check(key);

    FuriHalCryptoStatus status = furi_hal_crypto_storage_write(key);

    crypto_command_show_status(status, "write");

    furi_hal_crypto_storage_free(key);
}

void crypto_command_read(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);

    if(!crypto_command_is_init()) {
        return;
    }
    FuriHalCryptoKey* key = NULL;
    FuriHalCryptoPartition partition = FuriHalCryptoPartitionMax;
    FuriHalCryptoKeyType type = FuriHalCryptoKeyTypeNone;
    uint32_t id = 0;
    uint32_t temp = 0xFF;

    if(furi_string_size(args)) {
        char* args_cstr = (char*)furi_string_get_cstr(args);
        StrintParseError parse_err = StrintParseNoError;
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &temp, 10);
        partition = (FuriHalCryptoPartition)temp;
        if(parse_err || (partition >= FuriHalCryptoPartitionMax)) {
            cli_print_usage(
                "crypto read",
                "<partition> 0-partition_main, 1-partition_user\r\n",
                furi_string_get_cstr(args));
            printf(CLI_STATUS_ERROR);
            return;
        }
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &temp, 10);
        type = (FuriHalCryptoKeyType)temp;
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &id, 16);
        if(parse_err) {
            cli_print_usage(
                "crypto read",
                "<partition> <type> <id: in HEX> Read key from NWP flash.\r\n",
                furi_string_get_cstr(args));
            printf(CLI_STATUS_ERROR);
            return;
        }
    } else {
        cli_print_usage(
            "crypto read",
            "<partition> <type> <id: in HEX> Read key from NWP flash.\r\n",
            furi_string_get_cstr(args));
        printf(CLI_STATUS_ERROR);
        return;
    }

    key = furi_hal_crypto_storage_alloc(partition);
    FuriHalCryptoStatus status = furi_hal_crypto_storage_read(key, type, id);

    if(status == FuriHalCryptoStatusOk) {
        printf("partition: %d\r\n", partition);
        printf("magic_number: 0x%08lx\r\n", key->header.magic_number);
        printf("key_reserved: 0x%04X\r\n", key->header.reserved);
        printf("key_size: %d\r\n", key->header.size);
        printf("key_type: %ld\r\n", (uint32_t)key->header.type);
        printf("key_type_name: %s\r\n", crypto_command_show_type(key->header.type));
        printf("key_flags: 0x%08lX\r\n", (uint32_t)key->header.flags);
        printf("key_id: 0x%08lX\r\n", (uint32_t)key->header.id);
        printf("key_reserved1: 0x%08lX\r\n", (uint32_t)key->header.reserved1);
        printf("key_crc32: 0x%08lX\r\n", key->header.crc32);
        printf("key_data:\r\n");
        for(uint32_t i = 0; i < key->header.size; i++) {
            if((i) % 32 == 0) printf("%08lx: ", i);
            printf("%02x ", key->data[i]);
            if((i + 1) % 32 == 0) {
                printf("\r\n");
            }
        }
        printf("\r\n");
    }

    crypto_command_show_status(status, "read");
    furi_hal_crypto_storage_free(key);
}

void crypto_command_dump(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    UNUSED(args);
    if(!crypto_command_is_init()) {
        return;
    }
    sl_status_t status = SL_STATUS_FAIL;
    uint32_t address = FURI_HAL_CRYPTO_STORAGE_START_ADDRESS;
    uint8_t* buf = malloc(1024);

    for(uint32_t i = FURI_HAL_CRYPTO_STORAGE_START_ADDRESS;
        i < FURI_HAL_CRYPTO_STORAGE_END_ADDRESS;
        i += 1024) {
        status = sl_si91x_command_to_read_common_flash(i, 1024, buf);
        if(status != SL_STATUS_OK) {
            printf(
                ANSI_FG_RED "Error: Failed to read from NWP flash: " ANSI_RESET "0x%08lx\r\n",
                status);
            free(buf);
            printf(CLI_STATUS_ERROR);
            return;
        }
        printf(ANSI_FG_GREEN "Read data from NWP flash address: " ANSI_RESET "0x%08lx\r\n", i);

        for(uint32_t ii = 0; ii < 1024; ii++) {
            if((ii) % 32 == 0) printf("%08lx: ", address);
            printf("%02x ", buf[ii]);
            if((ii + 1) % 32 == 0) {
                printf("\r\n");
                address += 32;
            }
        }
    }
    free(buf);
    printf(CLI_STATUS_OK);
}

void crypto_command_gen(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    if(!crypto_command_is_init()) {
        return;
    }

    FuriHalCryptoKey* key = NULL;
    FuriHalCryptoPartition partition = FuriHalCryptoPartitionMax;
    FuriHalCryptoKeyType type = FuriHalCryptoKeyTypeNone;
    FuriHalCryptoKeyFlag flags = FuriHalCryptoKeyFlagNone;
    uint32_t id = 0;
    uint32_t temp = 0xFF;

    if(furi_string_size(args)) {
        char* args_cstr = (char*)furi_string_get_cstr(args);
        StrintParseError parse_err = StrintParseNoError;
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &temp, 10);
        partition = (FuriHalCryptoPartition)temp;
        if(parse_err || (partition >= FuriHalCryptoPartitionMax)) {
            cli_print_usage(
                "crypto gen",
                "<partition> 0-partition_main, 1-partition_user\r\n",
                furi_string_get_cstr(args));
            printf(CLI_STATUS_ERROR);
            return;
        }
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &temp, 10);
        type = (FuriHalCryptoKeyType)temp;
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &id, 16);
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &temp, 16);
        flags = (FuriHalCryptoKeyFlag)temp;
        if(parse_err) {
            cli_print_usage(
                "crypto gen",
                "<partition> <type> <id: in HEX> <flags: in HEX> Generate key from NWP flash.\r\n",
                furi_string_get_cstr(args));
            printf(CLI_STATUS_ERROR);
            return;
        }
    } else {
        cli_print_usage(
            "crypto gen",
            "<partition> <type> <id: in HEX> <flags: in HEX> Generate key from NWP flash.\r\n",
            furi_string_get_cstr(args));
        printf(CLI_STATUS_ERROR);
        return;
    }

    key = furi_hal_crypto_storage_alloc(partition);

    switch(type) {
    case FuriHalCryptoKeyTypeAes128:
        key->header.size = FURI_HAL_CRYPTO_AES_KEY_SIZE_128;
        break;
    case FuriHalCryptoKeyTypeAes192:
        key->header.size = FURI_HAL_CRYPTO_AES_KEY_SIZE_192;
        break;
    case FuriHalCryptoKeyTypeAes256:
        key->header.size = FURI_HAL_CRYPTO_AES_KEY_SIZE_256;
        break;
    case FuriHalCryptoKeyTypeHmacSha1:
        key->header.size = FURI_HAL_CRYPTO_HMAC_SHA1_DIGEST_SIZE;
        break;
    case FuriHalCryptoKeyTypeHmacSha256:
        key->header.size = FURI_HAL_CRYPTO_HMAC_SHA256_DIGEST_SIZE;
        break;
    case FuriHalCryptoKeyTypeHmacSha384:
        key->header.size = FURI_HAL_CRYPTO_HMAC_SHA384_DIGEST_SIZE;
        break;
    case FuriHalCryptoKeyTypeHmacSha512:
        key->header.size = FURI_HAL_CRYPTO_HMAC_SHA512_DIGEST_SIZE;
        break;
    case FuriHalCryptoKeyTypeEcdsaPriv224:
        key->header.size = FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_224;
        break;
    case FuriHalCryptoKeyTypeEcdsaPriv256:
        key->header.size = FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256;
        break;
    default:
        printf(ANSI_FG_RED "Error: Unsupported key type: %ld\r\n" ANSI_RESET, (uint32_t)type);
        furi_hal_crypto_storage_free(key);
        printf(CLI_STATUS_ERROR);
        return;
        break;
    }
    key->header.type = type;
    key->header.flags = flags;
    key->header.id = id;

    uint8_t* buf = malloc(key->header.size);

    // Todo add generation asimetric keys

    FuriHalCryptoStatus status = furi_hal_crypto_storage_gen_random_buf(buf, key->header.size);
    if(status != FuriHalCryptoStatusOk) {
        printf(
            ANSI_FG_RED "Error: Failed to generate random buffer: " ANSI_RESET "%d\r\n", status);
        printf(CLI_STATUS_ERROR);
        free(buf);
        furi_hal_crypto_storage_free(key);
        return;
    }

    memcpy(key->data, buf, key->header.size);
    memset(buf, 0, key->header.size);
    free(buf);

    status = furi_hal_crypto_storage_write(key);
    if(status == FuriHalCryptoStatusOk) {
        printf(ANSI_FG_GREEN "Generated key successfully\r\n" ANSI_RESET);
    } else {
        printf(ANSI_FG_RED "Error: Failed to generate key" ANSI_RESET "\r\n");
    }

    crypto_command_show_status(status, "write");
    furi_hal_crypto_storage_free(key);
}

void crypto_command_list(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    if(!crypto_command_is_init()) {
        return;
    }

    FuriHalCryptoKey* key = NULL;
    FuriHalCryptoPartition partition = FuriHalCryptoPartitionMax;
    uint32_t temp = 0xFF;

    if(furi_string_size(args)) {
        char* args_cstr = (char*)furi_string_get_cstr(args);
        StrintParseError parse_err = StrintParseNoError;
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &temp, 10);
        partition = (FuriHalCryptoPartition)temp;
        if(parse_err || (partition >= FuriHalCryptoPartitionMax)) {
            cli_print_usage(
                "crypto list",
                "<partition> 0-partition_main, 1-partition_user\r\n",
                furi_string_get_cstr(args));
            printf(CLI_STATUS_ERROR);
            return;
        }
        if(parse_err) {
            cli_print_usage(
                "crypto list",
                "<partition> List keys from NWP flash.\r\n",
                furi_string_get_cstr(args));
            printf(CLI_STATUS_ERROR);
            return;
        }
    } else {
        cli_print_usage(
            "crypto list",
            "<partition> List keys from NWP flash.\r\n",
            furi_string_get_cstr(args));
        printf(CLI_STATUS_ERROR);
        return;
    }

    key = furi_hal_crypto_storage_alloc(partition);
    printf("\t<part>\t<type>\t<id>\r\n");
    bool read_next = true;
    FuriHalCryptoStatus status = FuriHalCryptoStatusFail;
    do {
        status = furi_hal_crypto_storage_get_next_key(key);
        if(status == FuriHalCryptoStatusOk) {
            printf("key:\t%d\t%d\t0x%08lX\r\n", key->partition, key->header.type, key->header.id);
            read_next = true;
        } else if(status == FuriHalCryptoStatusNotFound || status == FuriHalCryptoStatusStorageFull) {
            read_next = false;
        } else {
            crypto_command_show_status(status, "read");
            read_next = false;
        }
    } while(read_next);

    furi_hal_crypto_storage_free(key);
    if(status == FuriHalCryptoStatusNotFound || status == FuriHalCryptoStatusStorageFull) {
        printf(CLI_STATUS_OK);
    } else {
        printf(CLI_STATUS_ERROR);
    }
}

static void crypto_command_print_usage(void) {
    printf("Usage:\r\n");
    printf("crypto <cmd> <args>\r\n");
    printf("Cmd list:\r\n");

    printf("\tcrypto init Initialize NWP.\r\n");
    printf("\tcrypto wipe <partition> Clear crypto storage.\r\n");
    printf("\tcrypto dump Dump crypto storage.\r\n");
    printf("\tcrypto read <partition> <type> <id: in HEX> Read key from NWP flash.\r\n");
    printf(
        "\tcrypto write <partition> <type> <id: in HEX> <flags: in HEX> <size> <data: in Byte> Write key from NWP flash\r\n");
    printf(
        "\tcrypto gen <partition> <type> <id: in HEX> <flags: in HEX> Generate key from NWP flash\r\n");
    printf("\tcrypto list <partition> List keys from NWP flash\r\n");
    printf("\tcrypto deinit Deinitialize NWP.\r\n");
    printf("\t\t<partition> 0-partition_main, 1-partition_user.\r\n");
}

void crypto_command(PipeSide* pipe, FuriString* args, void* context) {
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            crypto_command_print_usage();
            break;
        }

        if(furi_string_cmp_str(cmd, "init") == 0) {
            crypto_command_init(pipe, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "deinit") == 0) {
            crypto_command_deinit(pipe, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "dump") == 0) {
            crypto_command_dump(pipe, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "wipe") == 0) {
            crypto_command_wipe(pipe, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "write") == 0) {
            crypto_command_write(pipe, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "read") == 0) {
            crypto_command_read(pipe, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "gen") == 0) {
            crypto_command_gen(pipe, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "list") == 0) {
            crypto_command_list(pipe, args, context);
            break;
        }

        crypto_command_print_usage();
    } while(false);

    furi_string_free(cmd);
}
