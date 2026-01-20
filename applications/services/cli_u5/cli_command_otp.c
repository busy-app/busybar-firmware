#include "cli_command_otp.h"
#include <cli/args.h>
#include <furi_hal_flash_otp.h>
#include <furi_hal_flash.h>

static uint32_t cli_otp_parse_addr(FuriString* args, FuriHalFlashOtpBlock* block) {
    uint32_t addr = 0;
    *block = 0;
    FuriString* region_arg = furi_string_alloc();
    do {
        if(!args_read_string_and_trim(args, region_arg)) {
            break;
        }

        if(furi_string_cmpi_str(region_arg, "OTP1") == 0) {
            *block = FuriHalOtpBlockOtp1;
            addr = furi_hal_flash_otp_get_block_address(FuriHalOtpBlockOtp1);
        } else if(furi_string_cmpi_str(region_arg, "OTP2") == 0) {
            *block = FuriHalOtpBlockOtp2;
            addr = furi_hal_flash_otp_get_block_address(FuriHalOtpBlockOtp2);
        } else if(furi_string_cmpi_str(region_arg, "OTP3") == 0) {
            *block = FuriHalOtpBlockOtp3;
            addr = furi_hal_flash_otp_get_block_address(FuriHalOtpBlockOtp3);
        } else if(furi_string_cmpi_str(region_arg, "OTP4") == 0) {
            *block = FuriHalOtpBlockOtp4;
            addr = furi_hal_flash_otp_get_block_address(FuriHalOtpBlockOtp4);
        }
    } while(0);
    furi_string_free(region_arg);
    return addr;
}

static uint8_t* cli_otp_parse_data(FuriString* args, size_t* len) {
    if(furi_string_size(args) == 0) {
        return NULL;
    }

    FuriString* data_str = furi_string_alloc();
    size_t hex_char_count = 0;
    do {
        if(!args_read_string_and_trim(args, data_str)) {
            break;
        }

        const char* str_buf = furi_string_get_cstr(data_str);
        for(size_t i = 0; i < furi_string_size(data_str); i++) {
            char c = str_buf[i];
            if(((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'F')) ||
               ((c >= 'a') && (c <= 'f'))) {
                hex_char_count++;
            } else {
                hex_char_count = 0;
                break;
            }
        }
    } while(0);

    if((hex_char_count % 2) || (hex_char_count == 0)) {
        furi_string_free(data_str);
        return NULL;
    }
    *len = hex_char_count / 2;

    uint8_t* buf = malloc(*len);
    const char* str_buf = furi_string_get_cstr(data_str);
    for(size_t i = 0; i < hex_char_count; i += 2) {
        uint8_t byte_temp = 0;
        char c = str_buf[i];
        if((c >= '0') && (c <= '9'))
            byte_temp |= (c - '0') << 4;
        else if((c >= 'A') && (c <= 'F'))
            byte_temp |= (c - 'A' + 0xA) << 4;
        else if((c >= 'a') && (c <= 'f'))
            byte_temp |= (c - 'a' + 0xa) << 4;

        c = str_buf[i + 1];
        if((c >= '0') && (c <= '9'))
            byte_temp |= (c - '0');
        else if((c >= 'A') && (c <= 'F'))
            byte_temp |= (c - 'A' + 0xA);
        else if((c >= 'a') && (c <= 'f'))
            byte_temp |= (c - 'a' + 0xa);
        buf[i / 2] = byte_temp;
    }

    furi_string_free(data_str);
    return buf;
}

static void cli_command_otp_program(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);

    FuriHalFlashOtpBlock block = 0;
    uint32_t addr = cli_otp_parse_addr(args, &block);

    size_t len = 0;
    uint8_t* data = cli_otp_parse_data(args, &len);
    if((addr == 0) || (!data) || (len == 0) || (len > FURI_HAL_FLASH_OTP_BLOCK_SIZE)) {
        if(data) {
            free(data);
        }
        printf("Usage:\r\n");
        printf("otp program <OTP1/OTP2/OTP3/OTP4> <data>\r\n");
        return;
    }

    // Validate OTP header (magic and index)
    if(len < sizeof(FuriHalFlashOtpHeader)) {
        printf(
            "Error: Data too short for OTP header (min %zu bytes)\r\n",
            sizeof(FuriHalFlashOtpHeader));
        free(data);
        return;
    }

    const FuriHalFlashOtpHeader* header = (const FuriHalFlashOtpHeader*)data;
    if(header->magic != FURI_HAL_FLASH_OTP_MAGIC) {
        printf(
            "Error: Invalid magic value 0x%04X, expected 0x%04X\r\n",
            header->magic,
            FURI_HAL_FLASH_OTP_MAGIC);
        free(data);
        return;
    }

    if(header->index != (uint8_t)block) {
        printf(
            "Error: OTP index mismatch - data contains index %d, but writing to OTP%d\r\n",
            header->index,
            block);
        free(data);
        return;
    }

    printf(
        "OTP header validated: magic=0x%04X, index=%d, version=%d\r\n",
        header->magic,
        header->index,
        header->version);
    printf("Warning! This operation is irreversible! Are you sure? y/n\r\n");

    while(true) {
        char answer;
        if(pipe_receive(pipe, &answer, sizeof(answer)) != sizeof(answer)) break;
        if(answer == 'n' || answer == 'N') {
            printf("\r\nCancelled.");
            break;
        } else if(answer == 'y' || answer == 'Y') {
            printf("Programming OTP...\r\n");

            bool success = furi_hal_flash_otp_program(block, data, len);
            if(success) {
                printf("Done\r\n");
            } else {
                printf("Programming error\r\n");
            }

            break;
        }
    }
    free(data);
}

static void cli_command_otp_dump(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    FuriHalFlashOtpBlock block = 0;
    uint32_t addr = cli_otp_parse_addr(args, &block);
    if(addr == 0) {
        printf("Usage:\r\n");
        printf("otp dump <OTP1/OTP2/OTP3/OTP4>\r\n");
        return;
    }

    for(size_t i = 0; i < FURI_HAL_FLASH_OTP_BLOCK_SIZE; i++) {
        printf("%02x", *(uint8_t*)(addr + i));
    }
    printf("\r\n");
}

static void cli_command_otp_print_usage() {
    printf("Usage:\r\n");
    printf("otp <cmd>\r\n");
    printf("Cmd list:\r\n");
    printf("\tdump - dump OTP content\r\n");
    printf("\tprogram - program OTP\r\n");
}

void cli_command_otp(PipeSide* pipe, FuriString* args, void* context) {
    FuriString* cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            cli_command_otp_print_usage();
            break;
        }

        if(furi_string_cmpi_str(cmd, "dump") == 0) {
            cli_command_otp_dump(pipe, args, context);
            break;
        }
        if(furi_string_cmpi_str(cmd, "program") == 0) {
            cli_command_otp_program(pipe, args, context);
            break;
        }

        cli_command_otp_print_usage();
    } while(false);

    furi_string_free(cmd);
}
