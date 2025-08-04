#include "cli_command_otp.h"
#include <cli/args.h>
#include <furi_hal_flash_otp.h>

static uint32_t cli_otp_parse_addr(FuriString* args) {
    uint32_t addr = 0;
    FuriString* region_arg = furi_string_alloc();
    do {
        if(!args_read_string_and_trim(args, region_arg)) {
            break;
        }

        if(furi_string_cmp_str(region_arg, "OTP1") == 0) {
            addr = FURI_HAL_OTP_BLOCK1;
        } else if(furi_string_cmp_str(region_arg, "OTP2") == 0) {
            addr = FURI_HAL_OTP_BLOCK2;
        } else if(furi_string_cmp_str(region_arg, "OTP3") == 0) {
            addr = FURI_HAL_OTP_BLOCK3;
        } else if(furi_string_cmp_str(region_arg, "OTP4") == 0) {
            addr = FURI_HAL_OTP_BLOCK4;
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
            if(((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'F'))) {
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
    for(size_t i = 0; i < furi_string_size(data_str); i += 2) {
        uint8_t byte_temp = 0;
        char c = str_buf[i];
        if((c >= '0') && (c <= '9')) byte_temp |= (c - '0') << 4;
        if((c >= 'A') && (c <= 'F')) byte_temp |= (c - 'A' + 0xA) << 4;

        c = str_buf[i + 1];
        if((c >= '0') && (c <= '9')) byte_temp |= (c - '0');
        if((c >= 'A') && (c <= 'F')) byte_temp |= (c - 'A' + 0xA);
        buf[i / 2] = byte_temp;
    }

    furi_string_free(data_str);
    return buf;
}

static void cli_command_otp_program(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    uint32_t addr = cli_otp_parse_addr(args);

    size_t len = 0;
    uint8_t* data = cli_otp_parse_data(args, &len);
    if((addr == 0) || (!data) || (len == 0) || (len > FURI_HAL_OTP_BLOCK_SIZE)) {
        if(data) {
            free(data);
        }
        printf("Usage:\r\n");
        printf("otp dump <OTP1/OTP2/OTP3/OTP4>  <data>\r\n");
        return;
    }

    printf("Warning! This operation is irreversible! Are you sure? y/n\r\n");

    while(true) {
        char answer;
        if(pipe_receive(pipe, &answer, sizeof(answer)) != sizeof(answer)) break;
        if(answer == 'n' || answer == 'N') {
            printf("\r\nCancelled.");
            break;
        } else if(answer == 'y' || answer == 'Y') {
            printf("Programming OTP...\r\n");

            bool success = furi_hal_flash_program_otp(addr, data, len);
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

    uint32_t addr = cli_otp_parse_addr(args);
    if(addr == 0) {
        printf("Usage:\r\n");
        printf("otp dump <OTP1/OTP2/OTP3/OTP4>\r\n");
        return;
    }

    size_t len_remain = FURI_HAL_OTP_BLOCK_SIZE;
    const size_t row_len_max = 16;
    for(size_t offset = 0; offset < FURI_HAL_OTP_BLOCK_SIZE; offset += row_len_max) {
        uint8_t* ptr = (uint8_t*)(addr + offset);
        size_t row_len = MIN(len_remain, row_len_max);
        for(uint8_t i = 0; i < row_len; i++) {
            printf("%02X ", ptr[i]);
        }
        len_remain -= row_len;
        printf("\r\n");
    }
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

        if(furi_string_cmp_str(cmd, "dump") == 0) {
            cli_command_otp_dump(pipe, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "program") == 0) {
            cli_command_otp_program(pipe, args, context);
            break;
        }

        cli_command_otp_print_usage();
    } while(false);

    furi_string_free(cmd);
}
