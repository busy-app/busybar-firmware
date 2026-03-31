#include <containers/pipe.h>
#include <cli/cli_ansi.h>
#include <cli/args.h>
#include <cli/cli_command.h>

void cli_command_uptime(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);
    uint32_t uptime = furi_get_tick() / furi_kernel_get_tick_frequency();
    printf(
        "Uptime: %02lud %02luh %02lum %02lus",
        uptime / 60 / 60 / 24,
        (uptime / 60 / 60) % 24,
        (uptime / 60) % 60,
        uptime % 60);
}

static void cli_command_log_tx_callback(const uint8_t* buffer, size_t size, void* context) {
    PipeSide* pipe = context;
    pipe_send(pipe, buffer, size);
}

static void cli_command_log_usage(void) {
    printf("\r\n");
    printf(ANSI_FG_GREEN "Usage:\r\n");
    printf(ANSI_FG_YELLOW "  log " ANSI_RESET
                          "- start logging according to current system settings\r\n");
    printf(ANSI_FG_YELLOW "  log " ANSI_FG_BR_WHITE "<level> " ANSI_RESET
                          "- log messages of <level> or below for all tags\r\n");
    printf(ANSI_FG_YELLOW "  log " ANSI_FG_BR_WHITE "<level> " ANSI_FG_YELLOW
                          "only " ANSI_FG_BR_WHITE "<tag1>,<tag2>,... " ANSI_RESET
                          "- log messages of <level> or below for comma-separated <tag>s\r\n");
    printf(ANSI_FG_YELLOW
           "  log " ANSI_FG_BR_WHITE "<level> " ANSI_FG_YELLOW "except " ANSI_FG_BR_WHITE
           "<tag1>,<tag2>,... " ANSI_RESET
           "- log messages of <level> or below for all tags except comma-separated <tag>s\r\n");
    printf(ANSI_FG_YELLOW
           "  log " ANSI_FG_BR_WHITE "<level1> " ANSI_FG_YELLOW "but " ANSI_FG_BR_WHITE
           "<level2> <tag1>,<tag2>,... " ANSI_RESET
           "- apply <level2> to specified tags, but <level1> to all other tags\r\n");
    printf(
        ANSI_FG_YELLOW
        "  log configure " ANSI_FG_BR_WHITE "<selector> " ANSI_RESET
        "- only configure the specified <selector> according to above rules, but don't log anything here\r\n");
    printf(ANSI_FG_BR_WHITE "  <level>: " ANSI_FG_YELLOW "error, warn, info, debug, trace\r\n");
    printf("\r\n");
    printf(ANSI_FG_GREEN "Examples:\r\n");
    printf(ANSI_FG_YELLOW
           "  log " ANSI_FG_BR_WHITE "debug " ANSI_RESET
           "- only messages with level 'error', 'warn', 'info' or 'debug'; but not 'trace'\r\n");
    printf(ANSI_FG_YELLOW "  log " ANSI_FG_BR_WHITE "debug " ANSI_FG_YELLOW
                          "only " ANSI_FG_BR_WHITE "Audio " ANSI_RESET
                          "- only messages from Audio, with level 'error'..'debug'\r\n");
    printf(ANSI_FG_YELLOW "  log " ANSI_FG_BR_WHITE "trace " ANSI_FG_YELLOW
                          "except " ANSI_FG_BR_WHITE "LightSensor,LightSensorData " ANSI_RESET
                          "- all messages except ones coming from the LightSensor service\r\n");
    printf(ANSI_FG_YELLOW
           "  log " ANSI_FG_BR_WHITE "info " ANSI_FG_YELLOW "but " ANSI_FG_BR_WHITE
           "trace Desktop " ANSI_RESET
           "- all messages coming from Desktop, and 'error'..'info' for all other tags\r\n");
    printf(ANSI_FG_YELLOW "  log " ANSI_FG_YELLOW "configure " ANSI_FG_BR_WHITE "trace " ANSI_RESET
                          "- all messages with all tags on the UART interface\r\n");
    printf(ANSI_RESET);
}

typedef enum {
    LogArgparseStatusNormal,
    LogArgparseStatusConfigureOnly,
    LogArgparseStatusError,
    LogArgparseStatusMAX,
} LogArgparseStatus;

static bool cli_command_log_parse_level(FuriString* specifier, FuriLogLevel* level) {
    bool success = false;
    FuriString* temp = furi_string_alloc();

    do {
        furi_string_set(temp, specifier);

        size_t space = furi_string_search_char(temp, ' ');
        if(space == FURI_STRING_FAILURE) {
            furi_string_reset(specifier);
        } else {
            furi_string_left(temp, space);
            furi_string_right(specifier, space + 1);
        }

        if(!furi_log_level_from_string(furi_string_get_cstr(temp), level)) {
            printf(ANSI_FG_RED "\"%s\" is not a valid log level\r\n", furi_string_get_cstr(temp));
            break;
        }

        success = true;
    } while(false);

    furi_string_free(temp);
    return success;
}

static bool cli_command_log_apply_specifier(FuriString* specifier) {
    if(furi_string_empty(specifier)) return true;

    bool success = false;

    do {
        FuriLogLevel main_level;
        if(!cli_command_log_parse_level(specifier, &main_level)) break;
        furi_log_set_level(main_level);

        bool parse_tag_list = false;
        FuriLogLevel exception_level = FuriLogLevelNone;
        FuriLogExceptionMode exception_mode = FuriLogExceptionModeInclude;

        if(furi_string_consume_left(specifier, "but ")) {
            if(!cli_command_log_parse_level(specifier, &exception_level)) break;
            parse_tag_list = true;

        } else if(furi_string_consume_left(specifier, "only ")) {
            exception_mode = FuriLogExceptionModeExclude;
            parse_tag_list = true;

        } else if(furi_string_consume_left(specifier, "except ")) {
            parse_tag_list = true;
        }

        if(parse_tag_list) {
            if(furi_string_search_char(specifier, ' ') != FURI_STRING_FAILURE) {
                printf(
                    ANSI_FG_RED "unrecognized extra arguments besides tag list: \"%s\"\r\n",
                    furi_string_get_cstr(specifier));
                break;
            }
            if(furi_string_end_with_str(specifier, ",")) {
                printf(
                    ANSI_FG_RED "tag list has trailing comma: \"%s\"\r\n",
                    furi_string_get_cstr(specifier));
                break;
            }

            furi_log_begin_level_exceptions(exception_level, exception_mode);

            FuriString* tag = furi_string_alloc();

            while(!furi_string_empty(specifier)) {
                size_t comma = furi_string_search_char(specifier, ',');
                furi_string_set_n(tag, specifier, 0, comma);
                furi_string_right(specifier, (comma == FURI_STRING_FAILURE) ? comma : (comma + 1));
                furi_log_add_level_exception(furi_string_get_cstr(tag));
            }
            furi_string_free(tag);

        } else if(!furi_string_empty(specifier)) {
            printf(
                ANSI_FG_RED "unrecognized extra arguments: \"%s\"\r\n",
                furi_string_get_cstr(specifier));
            break;
        }

        success = true;
    } while(false);

    return success;
}

static LogArgparseStatus cli_command_parse_args(FuriString* args) {
    if(furi_string_consume_left(args, "configure ")) {
        return cli_command_log_apply_specifier(args) ? LogArgparseStatusConfigureOnly :
                                                       LogArgparseStatusError;
    } else {
        return cli_command_log_apply_specifier(args) ? LogArgparseStatusNormal :
                                                       LogArgparseStatusError;
    }
}

void cli_command_log(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    FuriLogLevel previous_level = furi_log_get_level();

    bool restore_log_level = false;
    bool print_logs_here = false;

    LogArgparseStatus status = cli_command_parse_args(args);

    if(status == LogArgparseStatusError) {
        cli_command_log_usage();
        restore_log_level = true;

    } else if(status == LogArgparseStatusConfigureOnly) {
        printf("Log filter configured. Watch output on UART interface.\r\n");
        return;

    } else if(status == LogArgparseStatusNormal) {
        print_logs_here = true;
        restore_log_level = true;
    }

    if(print_logs_here) {
        FuriLogHandler log_handler = {
            .callback = cli_command_log_tx_callback,
            .context = pipe,
        };

        furi_log_add_handler(log_handler);

        printf("Press CTRL+C to stop...\r\n");
        while(!cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
            furi_delay_ms(100);
        }

        furi_log_remove_handler(log_handler);
    }

    if(restore_log_level) {
        // There will be strange behaviour if log level is set from settings while log command is running
        furi_log_set_level(previous_level);
        furi_log_begin_level_exceptions(FuriLogLevelNone, FuriLogExceptionModeInclude);
    }
}

void cli_command_top(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);

    int interval = 1000;
    args_read_int_and_trim(args, &interval);

    FuriThreadList* thread_list = furi_thread_list_alloc();
    while(!cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
        uint32_t tick = furi_get_tick();
        furi_thread_enumerate(thread_list);

        if(interval) printf(ANSI_CURSOR_POS("1", "1"));

        uint32_t uptime = tick / furi_kernel_get_tick_frequency();
        printf(
            "Threads: %zu, ISR Time: %0.2f%%, Uptime: %luh%lum%lus" ANSI_ERASE_LINE(
                ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n",
            furi_thread_list_size(thread_list),
            (double)furi_thread_list_get_isr_time(thread_list),
            uptime / 60 / 60,
            uptime / 60 % 60,
            uptime % 60);

        printf(
            "Heap: total %zu, free %zu, minimum %zu, max block %zu" ANSI_ERASE_LINE(
                ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n" ANSI_ERASE_LINE(ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n",
            memmgr_get_total_heap(),
            memmgr_get_free_heap(),
            memmgr_get_minimum_free_heap(),
            memmgr_heap_get_max_free_block());

        printf(
            "%-17s %-20s %-10s %5s %12s %6s %10s %7s %5s" ANSI_ERASE_LINE(
                ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n",
            "AppID",
            "Name",
            "State",
            "Prio",
            "Stack start",
            "Stack",
            "Stack Min",
            "Heap",
            "%CPU");

        for(size_t i = 0; i < furi_thread_list_size(thread_list); i++) {
            const FuriThreadListItem* item = furi_thread_list_get_at(thread_list, i);
            printf(
                "%-17s %-20s %-10s %5d   0x%08lx %6lu %10lu %7zu %5.1f" ANSI_ERASE_LINE(
                    ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n",
                item->app_id,
                item->name,
                item->state,
                item->priority,
                item->stack_address,
                item->stack_size,
                item->stack_min_free,
                item->heap,
                (double)item->cpu);
        }

        printf(ANSI_ERASE_DISPLAY(ANSI_ERASE_FROM_CURSOR_TO_END));
        fflush(stdout);

        if(interval > 0) {
            furi_delay_ms(interval);
        } else {
            break;
        }
    }
    furi_thread_list_free(thread_list);
}

void cli_command_free(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    printf("Free heap size: %zu\r\n", memmgr_get_free_heap());
    printf("Total heap size: %zu\r\n", memmgr_get_total_heap());
    printf("Minimum heap size: %zu\r\n", memmgr_get_minimum_free_heap());
    printf("Maximum heap block: %zu\r\n", memmgr_heap_get_max_free_block());

    printf("Pool free: %zu\r\n", memmgr_pool_get_free());
    printf("Maximum pool block: %zu\r\n", memmgr_pool_get_max_block());
}

void cli_command_free_blocks(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    memmgr_heap_printf_free_blocks();
}

void cli_command_echo(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);
    printf("%s\r\n", furi_string_get_cstr(args));
}
