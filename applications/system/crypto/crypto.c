#include "crypto.h"

#include <furi_hal_crypto_storage.h>
#include <furi_hal_crypto.h>

#include <furi.h>
#include <strint.h>
#include <sl_si91x_driver.h>

#include <cli/args.h>
#include <cli/cli_status.h>

#include <wifi/wifi_common.h>

static void print_status(FuriHalCryptoStatus status) {
    switch(status) {
    case FuriHalCryptoStatusOk:
        printf("Success\r\n");
        break;
    case FuriHalCryptoStatusFail:
        printf("Fail\r\n");
        break;
    case FuriHalCryptoStatusFailWrite:
        printf("Write error\r\n");
        break;
    case FuriHalCryptoStatusStorageFull:
        printf("Storage full\r\n");
        break;
    case FuriHalCryptoStatusDuplicate:
        printf("Duplicate key\r\n");
        break;
    case FuriHalCryptoStatusNotFound:
        printf("Key not found\r\n");
        break;
    case FuriHalCryptoStatusErrorCrc:
        printf("CRC error\r\n");
        break;
    default:
        printf("Unknown error\r\n");
        break;
    }
}

static void
    show_status(FuriHalCryptoStatus status, const FuriHalCryptoKeySlot* slot, const char* name) {
    furi_assert(slot);
    furi_assert(name);

    const FuriHalCryptoKeySlotHeader* header = &slot->header;
    const FuriHalCryptoPartition key_part = slot->address.partition;
    const FuriHalCryptoKeyType key_type = header->type;
    const uint32_t key_id = header->id;

    if(status == FuriHalCryptoStatusOk) {
        printf("Key %d:%d:%lX %s SUCCESS\r\n" CLI_STATUS_OK, key_part, key_type, key_id, name);
    } else {
        printf("Key %d:%d:%lX %s ERROR: ", key_part, key_type, key_id, name);

        print_status(status);

        printf(CLI_STATUS_ERROR);
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
    case FuriHalCryptoKeyTypeHmacSha1:
        return "FuriHalCryptoKeyTypeHmacSha1";
    case FuriHalCryptoKeyTypeHmacSha256:
        return "FuriHalCryptoKeyTypeHmacSha256";
    case FuriHalCryptoKeyTypeHmacSha384:
        return "FuriHalCryptoKeyTypeHmacSha384";
    case FuriHalCryptoKeyTypeHmacSha512:
        return "FuriHalCryptoKeyTypeHmacSha512";
    case FuriHalCryptoKeyTypeEcdsaPriv224:
        return "FuriHalCryptoKeyTypeEcdsaPriv224";
    case FuriHalCryptoKeyTypeEcdsaPriv256:
        return "FuriHalCryptoKeyTypeEcdsaPriv256";
    case FuriHalCryptoKeyTypeEcdsaPub224:
        return "FuriHalCryptoKeyTypeEcdsaPub224";
    case FuriHalCryptoKeyTypeEcdsaPub256:
        return "FuriHalCryptoKeyTypeEcdsaPub256";
    case FuriHalCryptoKeyTypeMatterAttestation:
        return "FuriHalCryptoKeyTypeMatterAttestation";
    case FuriHalCryptoKeyTypeMatterSetup:
        return "FuriHalCryptoKeyTypeMatterSetup";
    case FuriHalCryptoKeyTypeMatterDeviceInfo:
        return "FuriHalCryptoKeyTypeMatterDeviceInfo";
    case FuriHalCryptoKeyTypeCsrDerEcdsa256:
        return "FuriHalCryptoKeyTypeCsrDerEcdsa256";
    case FuriHalCryptoKeyTypeCrtDerEcdsa256:
        return "FuriHalCryptoKeyTypeCrtDerEcdsa256";
    default:
        return "Unknown type";
    }
}

void crypto_command_wipe(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);

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
                "Error: Failed to wipe NWP flash partition_main: "
                "0x%08lx\r\n",
                status);
            printf(CLI_STATUS_ERROR);
        } else {
            printf("Wipe NWP flash partition_main\r\n");
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
                "Error: Failed to wipe NWP flash partition_user: "
                "0x%08lx\r\n",
                status);
            printf(CLI_STATUS_ERROR);
        } else {
            printf("Wipe NWP flash partition_user\r\n");
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

    uint32_t temp = 0xFF;
    FuriHalCryptoKey* key = NULL;
    FuriHalCryptoPartition partition = FuriHalCryptoPartitionMax;
    uint32_t key_id = 0;
    FuriHalCryptoStatus status = FuriHalCryptoStatusOk;
    bool success = false;
    do {
        if(furi_string_size(args) == 0) {
            break;
        }
        char* args_cstr = (char*)furi_string_get_cstr(args);
        StrintParseError parse_err = StrintParseNoError;

        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &temp, 10);
        partition = (FuriHalCryptoPartition)temp;
        if(parse_err || (partition >= FuriHalCryptoPartitionMax)) {
            break;
        }

        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &temp, 10);
        FuriHalCryptoKeyType type = (FuriHalCryptoKeyType)temp;
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &key_id, 16);
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &temp, 16);
        bool wrap = temp != 0;
        uint16_t length = 0;
        parse_err |= strint_to_uint16(args_cstr, &args_cstr, &length, 10);

        if(length == 0 || length > FURI_HAL_CRYPTO_DATA_SIZE_MAX) {
            printf("Invalid data length\r\n");
            break;
        }

        uint8_t* buf = malloc(length);

        furi_string_printf(args, "%s", args_cstr);
        furi_string_trim(args);
        if(parse_err || !args_read_hex_bytes(args, buf, length)) {
            free(buf);
            break;
        }

        status = furi_hal_crypto_key_init_raw(&key, type, buf, length);
        explicit_bzero(buf, length);
        free(buf);
        if(status != FuriHalCryptoStatusOk) {
            printf("Invalid data length for selected key type\r\n");
            break;
        }

        do {
            if(wrap) {
                printf("Wrapping the key\r\n");
                FuriHalCryptoKey* wrapped_key = NULL;
                status = furi_hal_crypto_wrap_key(key, &wrapped_key);
                if(status == FuriHalCryptoStatusUnavailable) {
                    printf("Key wrapping is unavailable on this device\r\n");
                    break;
                } else if(status != FuriHalCryptoStatusOk) {
                    printf("Wrapping failed: %d\r\n", status);
                    break;
                } else {
                    furi_hal_crypto_key_free(key);
                    key = wrapped_key;
                }
            }

            FuriHalCryptoKeySlot slot;
            status = furi_hal_crypto_storage_write_ex(key, partition, key_id, &slot);

            show_status(status, &slot, "write");
        } while(false);
        furi_hal_crypto_key_free(key);
        success = status == FuriHalCryptoStatusOk;
    } while(false);

    if(!success) {
        cli_print_usage(
            "crypto write",
            "<partition> <type> <id: in HEX> <wrap: 0 or 1> <size> <data: in byte>\r\n",
            NULL);
        printf(CLI_STATUS_ERROR);
    }
}

void crypto_command_read(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);

    FuriHalCryptoPartition partition = FuriHalCryptoPartitionMax;
    FuriHalCryptoKeyType type = 0;
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

    FuriHalCryptoKey* key = NULL;
    // fill slot to show error
    FuriHalCryptoKeySlot slot = {
        .address = {.partition = partition}, .header = {.id = id, .type = type}};
    FuriHalCryptoStatus status = furi_hal_crypto_storage_read_ex(&key, &slot, partition, type, id);

    if(status == FuriHalCryptoStatusOk) {
        printf("partition: %d\r\n", partition);
        printf("magic_number: 0x%08lx\r\n", slot.header.magic_number);
        printf("key_reserved: 0x%04X\r\n", slot.header.reserved);
        printf("key_size: %d\r\n", key->length);
        printf("key_type: %ld\r\n", (uint32_t)key->type);
        printf("key_type_name: %s\r\n", crypto_command_show_type(key->type));
        printf("key_flags: 0x%08lX\r\n", (uint32_t)key->flags);
        printf("key_id: 0x%08lX\r\n", (uint32_t)slot.header.id);
        printf("key_reserved1: 0x%08lX\r\n", (uint32_t)slot.header.reserved1);
        printf("key_crc32: 0x%08lX\r\n", slot.header.crc32);
        printf("key_data:\r\n");
        for(uint32_t i = 0; i < key->length; i++) {
            if((i) % 32 == 0) printf("%08lx: ", i);
            printf("%02x ", key->data[i]);
            if((i + 1) % 32 == 0) {
                printf("\r\n");
            }
        }
        printf("\r\n");
        furi_hal_crypto_key_free(key);
    }

    show_status(status, &slot, "read");
}

void crypto_command_dump(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    UNUSED(args);

    sl_status_t status = SL_STATUS_FAIL;
    uint32_t address = FURI_HAL_CRYPTO_STORAGE_START_ADDRESS;
    uint8_t* buf = malloc(1024);

    for(uint32_t i = FURI_HAL_CRYPTO_STORAGE_START_ADDRESS;
        i < FURI_HAL_CRYPTO_STORAGE_END_ADDRESS;
        i += 1024) {
        status = sl_si91x_command_to_read_common_flash(i, 1024, buf);
        if(status != SL_STATUS_OK) {
            printf("Error: Failed to read from NWP flash:: 0x%08lx\r\n", status);
            free(buf);
            printf(CLI_STATUS_ERROR);
            return;
        }
        printf("Read data from NWP flash address:: 0x%08lx\r\n", i);

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

    FuriHalCryptoPartition partition = FuriHalCryptoPartitionMax;
    FuriHalCryptoKeyType type = 0;
    bool wrap = false;
    uint32_t id = 0;
    uint32_t temp = 0xFF;
    bool asymmetric_key = false;

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
        wrap = temp != 0;
        if(parse_err) {
            cli_print_usage(
                "crypto gen",
                "<partition> <type> <id: in HEX> <wrap: 0 or 1> Generate key from NWP flash.\r\n",
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

    FuriHalCryptoKey* key = NULL;

    switch(type) {
    case FuriHalCryptoKeyTypeEcdsaPriv224:
    case FuriHalCryptoKeyTypeEcdsaPriv256:
        asymmetric_key = true;
        break;
    default:
        break;
    }

    FuriHalCryptoStatus status = furi_hal_crypto_gen_random_key(&key, type);
    if(status == FuriHalCryptoStatusInvalidParameter) {
        printf("Error: Unsupported key type: %ld\r\n", (uint32_t)type);
        printf(CLI_STATUS_ERROR);
        return;
    } else if(status != FuriHalCryptoStatusOk) {
        printf("Error: Failed to generate random buffer:: %d\r\n", status);
        printf(CLI_STATUS_ERROR);
        return;
    }

    do {
        if(asymmetric_key) {
            // For asymmetric keys, we need to generate public key
            FuriHalCryptoKey* pub_key = NULL;
            do {
                status = furi_hal_crypto_gen_asymmetric_pub_key(key, &pub_key);
                if(status != FuriHalCryptoStatusOk) {
                    printf("Error: Failed to generate public key"
                           "\r\n");
                    break;
                }
                do {
                    FuriHalCryptoKeySlot slot;
                    status = furi_hal_crypto_storage_write_ex(pub_key, partition, id, &slot);
                    if(status != FuriHalCryptoStatusOk) {
                        printf("Error: Failed to write public key"
                               "\r\n");
                        show_status(status, &slot, "write");
                        break;
                    }
                    printf("Generated public key successfully\r\n");
                } while(false);
                furi_hal_crypto_key_free(pub_key);
            } while(false);
        }

        if(wrap) {
            FuriHalCryptoKey* wrapped_key = NULL;
            status = furi_hal_crypto_wrap_key(key, &wrapped_key);
            if(status == FuriHalCryptoStatusOk) {
                furi_hal_crypto_key_free(key);
                key = wrapped_key;
            } else if(status == FuriHalCryptoStatusUnavailable) {
                printf("Error: Key wrapping is unsupported\r\n");
                break;
            } else {
                printf("Error: Key wrapping failed\r\n");
                break;
            }
        }
        FuriHalCryptoKeySlot slot;
        status = furi_hal_crypto_storage_write_ex(key, partition, id, &slot);
        if(status == FuriHalCryptoStatusOk) {
            printf("Generated private key successfully\r\n");
        } else {
            printf("Error: Failed to write private key"
                   "\r\n");
        }
        show_status(status, &slot, "write");
    } while(false);

    furi_hal_crypto_key_free(key);
}

void crypto_command_gen_csr(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);

    FuriHalCryptoPartition partition = FuriHalCryptoPartitionMax;
    uint32_t id = 0;
    uint32_t temp = 0xFF;

    if(furi_string_size(args)) {
        char* args_cstr = (char*)furi_string_get_cstr(args);
        StrintParseError parse_err = StrintParseNoError;
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &temp, 10);
        partition = (FuriHalCryptoPartition)temp;
        if(parse_err || (partition >= FuriHalCryptoPartitionMax)) {
            cli_print_usage(
                "crypto gen_csr",
                "<partition> 0-partition_main, 1-partition_user\r\n",
                furi_string_get_cstr(args));
            printf(CLI_STATUS_ERROR);
            return;
        }
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &id, 16);
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &temp, 16);
        furi_string_printf(args, "%s", args_cstr);
        furi_string_trim(args);
        if(parse_err) {
            cli_print_usage(
                "crypto gen_csr",
                "<partition> <id: in HEX> <reserved: in HEX> <subject_name> Generate CSR from NWP flash.\r\n",
                furi_string_get_cstr(args));
            printf(CLI_STATUS_ERROR);
            return;
        }
    } else {
        cli_print_usage(
            "crypto gen_csr",
            "<partition> <id: in HEX> <reserved: in HEX> <subject_name> Generate CSR from NWP flash.\r\n",
            furi_string_get_cstr(args));
        printf(CLI_STATUS_ERROR);
        return;
    }

    FuriHalCryptoKey* priv_key = NULL;
    FuriHalCryptoKey* pub_key = NULL;
    FuriHalCryptoKey* csr_der_key = NULL;

    do {
        if(furi_string_size(args) == 0) {
            printf("Warning: Subject name is required for CSR generation\r\n");
            furi_string_set(args, "CN=Default Subject, O=Default Org, C=Default Country");
            printf("Using default subject name: %s\r\n", furi_string_get_cstr(args));
        }

        // Generate random private key
        FuriHalCryptoStatus status = furi_hal_crypto_gen_random_key(
            &priv_key,
            FuriHalCryptoKeyTypeEcdsaPriv256); // CSR is generated from private key
        if(status != FuriHalCryptoStatusOk) {
            printf("Error: Failed to generate random key: %d\r\n", status);
            printf(CLI_STATUS_ERROR);
            break;
        }

        FuriHalCryptoKeySlot slot;
        do {
            // generate public key
            status = furi_hal_crypto_gen_asymmetric_pub_key(priv_key, &pub_key);
            if(status != FuriHalCryptoStatusOk) {
                printf("Error: Failed to generate public key"
                       "\r\n");
                break;
            }
            do {
                status = furi_hal_crypto_storage_write_ex(pub_key, partition, id, &slot);
                show_status(status, &slot, "write pub key");
                if(status != FuriHalCryptoStatusOk) {
                    printf("Error: Failed to save public key"
                           "\r\n");
                    break;
                }
                printf("Generated public key successfully\r\n");

                // generate CSR
                status = furi_hal_crypto_gen_csr_der_ecdsa256(
                    priv_key, &csr_der_key, furi_string_get_cstr(args));
                if(status != FuriHalCryptoStatusOk) {
                    printf("Error: Failed to generate CSR"
                           "\r\n");
                    break;
                }
                do {
                    status = furi_hal_crypto_storage_write_ex(csr_der_key, partition, id, &slot);
                    show_status(status, &slot, "write CSR");
                    if(status != FuriHalCryptoStatusOk) {
                        break;
                    }
                    printf("Generated CSR successfully\r\n");

                    FuriHalCryptoKey* wrapped_key = NULL;
                    status = furi_hal_crypto_wrap_key(priv_key, &wrapped_key);
                    if(status == FuriHalCryptoStatusOk) {
                        status =
                            furi_hal_crypto_storage_write_ex(wrapped_key, partition, id, &slot);
                        furi_hal_crypto_key_free(wrapped_key);
                    } else {
                        if(status == FuriHalCryptoStatusUnavailable) {
                            printf("Key wrapping is unsupported\r\n");
                        } else {
                            printf("Error wrapping key: %d\r\n", status);
                        }
                        status = furi_hal_crypto_storage_write_ex(priv_key, partition, id, &slot);
                    }

                    if(status == FuriHalCryptoStatusOk) {
                        printf("Generated private key successfully\r\n");
                    }
                    show_status(status, &slot, "write private key");
                } while(false);
                furi_hal_crypto_key_free(csr_der_key);
            } while(false);
            furi_hal_crypto_key_free(pub_key);
        } while(false);
        furi_hal_crypto_key_free(priv_key);
    } while(false);
}

void crypto_command_list(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);

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
    } else {
        cli_print_usage(
            "crypto list",
            "<partition> List keys from NWP flash.\r\n",
            furi_string_get_cstr(args));
        printf(CLI_STATUS_ERROR);
        return;
    }

    FuriHalCryptoKeyIter iter = furi_hal_crypto_key_iter_init(partition);
    printf("\t<part>\t<type>\t<id>\r\n");
    FuriHalCryptoStatus status = FuriHalCryptoStatusFail;
    FuriHalCryptoKey* key = NULL;
    FuriHalCryptoKeySlot slot;
    while((status = furi_hal_crypto_key_iter_get_and_advance(&iter, &key, &slot)) ==
          FuriHalCryptoStatusOk) {
        printf("key:\t%d\t%d\t0x%08lX\r\n", slot.address.partition, key->type, slot.header.id);
        furi_hal_crypto_key_free(key);
    }

    if(status == FuriHalCryptoStatusNotFound || status == FuriHalCryptoStatusStorageFull) {
        printf(CLI_STATUS_OK);
    } else {
        show_status(status, &slot, "read");
    }
}

static void crypto_command_print_usage(void) {
    printf("Usage:\r\n");
    printf("crypto <cmd> <args>\r\n");
    printf("Cmd list:\r\n");

    printf("\tcrypto wipe <partition> Clear crypto storage.\r\n");
    printf("\tcrypto dump Dump crypto storage.\r\n");
    printf("\tcrypto read <partition> <type> <id: in HEX> Read key from NWP flash.\r\n");
    printf(
        "\tcrypto write <partition> <type> <id: in HEX> <wrap: 0 or 1> <size> <data: in Byte> Write key to NWP flash\r\n");
    printf(
        "\tcrypto gen <partition> <type> <id: in HEX> <wrap: 0 or 1> Generate key from NWP flash\r\n");
    printf(
        "\tcrypto gen_csr <partition> <id: in HEX> <reserved: in HEX> <subject_name> Generate CSR from NWP flash\r\n");
    printf("\tcrypto list <partition> List keys from NWP flash\r\n");
    printf("\t\t<partition> 0-partition_main, 1-partition_user.\r\n");
}

void crypto_command(PipeSide* pipe, FuriString* args, void* context) {
    furi_record_open(RECORD_WIFI);

    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            crypto_command_print_usage();
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
        if(furi_string_cmp_str(cmd, "gen_csr") == 0) {
            crypto_command_gen_csr(pipe, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "list") == 0) {
            crypto_command_list(pipe, args, context);
            break;
        }

        crypto_command_print_usage();
    } while(false);

    furi_string_free(cmd);

    furi_record_close(RECORD_WIFI);
}
