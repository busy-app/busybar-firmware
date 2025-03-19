#include "cli_commands.h"
#include "cli_command_gpio.h"
#include "cli_command_display.h"
#include "cli_command_status_lights.h"
#include "cli_command_light_sensor.h"
#include "cli_command_audio.h"

#include <core/thread.h>
#include <core/thread_list.h>
#include <furi_hal.h>
#include <task_control_block.h>
#include <time.h>
#include <loader/loader.h>
#include <intercom/intercom.h>
#include <toolbox/args.h>

void cli_command_help(Cli* cli, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);
    printf("Commands available:");

    // Command count
    const size_t commands_count = CliCommandTree_size(cli->commands);
    const size_t commands_count_mid = commands_count / 2 + commands_count % 2;

    // Use 2 iterators from start and middle to show 2 columns
    CliCommandTree_it_t it_left;
    CliCommandTree_it(it_left, cli->commands);
    CliCommandTree_it_t it_right;
    CliCommandTree_it(it_right, cli->commands);
    for(size_t i = 0; i < commands_count_mid; i++)
        CliCommandTree_next(it_right);

    // Iterate throw tree
    for(size_t i = 0; i < commands_count_mid; i++) {
        printf("\r\n");
        // Left Column
        if(!CliCommandTree_end_p(it_left)) {
            printf("%-30s", furi_string_get_cstr(*CliCommandTree_ref(it_left)->key_ptr));
            CliCommandTree_next(it_left);
        }
        // Right Column
        if(!CliCommandTree_end_p(it_right)) {
            printf("%s", furi_string_get_cstr(*CliCommandTree_ref(it_right)->key_ptr));
            CliCommandTree_next(it_right);
        }
    };

    if(furi_string_size(args) > 0) {
        cli_nl(cli);
        printf("`");
        printf("%s", furi_string_get_cstr(args));
        printf("` command not found");
    }
}

void cli_command_uptime(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(args);
    UNUSED(context);
    uint32_t uptime = furi_get_tick() / furi_kernel_get_tick_frequency();
    printf(
        "Uptime: %02lud %02luh %02lum %02lus",
        uptime / 60 / 60 / 24,
        uptime / 60 / 60,
        uptime / 60 % 60,
        uptime % 60);
}

#define CLI_COMMAND_LOG_RING_SIZE   2048
#define CLI_COMMAND_LOG_BUFFER_SIZE 64

void cli_command_log_tx_callback(const uint8_t* buffer, size_t size, void* context) {
    furi_stream_buffer_send(context, buffer, size, 0);
}

bool cli_command_log_level_set_from_string(FuriString* level) {
    FuriLogLevel log_level;
    if(furi_log_level_from_string(furi_string_get_cstr(level), &log_level)) {
        furi_log_set_level(log_level);
        return true;
    } else {
        printf("<log> — start logging using the current level from the system settings\r\n");
        printf("<log error> — only critical errors and other important messages\r\n");
        printf("<log warn> — non-critical errors and warnings including <log error>\r\n");
        printf("<log info> — non-critical information including <log warn>\r\n");
        printf("<log default> — the default system log level (equivalent to <log info>)\r\n");
        printf(
            "<log debug> — debug information including <log info> (may impact system performance)\r\n");
        printf(
            "<log trace> — system traces including <log debug> (may impact system performance)\r\n");
    }
    return false;
}

void cli_command_log(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    FuriStreamBuffer* ring = furi_stream_buffer_alloc(CLI_COMMAND_LOG_RING_SIZE, 1);
    uint8_t buffer[CLI_COMMAND_LOG_BUFFER_SIZE];
    FuriLogLevel previous_level = furi_log_get_level();
    bool restore_log_level = false;

    if(furi_string_size(args) > 0) {
        if(!cli_command_log_level_set_from_string(args)) {
            furi_stream_buffer_free(ring);
            return;
        }
        restore_log_level = true;
    }

    const char* current_level;
    furi_log_level_to_string(furi_log_get_level(), &current_level);
    printf("Current log level: %s\r\n", current_level);

    FuriLogHandler log_handler = {
        .callback = cli_command_log_tx_callback,
        .context = ring,
    };

    furi_log_add_handler(log_handler);

    printf("Use <log ?> to list available log levels\r\n");
    printf("Press CTRL+C to stop...\r\n");
    while(!cli_cmd_interrupt_received(cli)) {
        size_t ret = furi_stream_buffer_receive(ring, buffer, CLI_COMMAND_LOG_BUFFER_SIZE, 50);
        cli_write(cli, buffer, ret);
    }

    furi_log_remove_handler(log_handler);

    if(restore_log_level) {
        // There will be strange behaviour if log level is set from settings while log command is running
        furi_log_set_level(previous_level);
    }

    furi_stream_buffer_free(ring);
}

static void cli_command_top(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);

    int interval = 1000;
    args_read_int_and_trim(args, &interval);

    FuriThreadList* thread_list = furi_thread_list_alloc();
    while(!cli_cmd_interrupt_received(cli)) {
        uint32_t tick = furi_get_tick();
        furi_thread_enumerate(thread_list);

        if(interval) printf("\e[2J\e[0;0f"); // Clear display and return to 0

        uint32_t uptime = tick / furi_kernel_get_tick_frequency();
        printf(
            "Threads: %zu, ISR Time: %0.2f%%, Uptime: %luh%lum%lus\r\n",
            furi_thread_list_size(thread_list),
            (double)furi_thread_list_get_isr_time(thread_list),
            uptime / 60 / 60,
            uptime / 60 % 60,
            uptime % 60);

        printf(
            "Heap: total %zu, free %zu, minimum %zu, max block %zu\r\n\r\n",
            memmgr_get_total_heap(),
            memmgr_get_free_heap(),
            memmgr_get_minimum_free_heap(),
            memmgr_heap_get_max_free_block());

        printf(
            "%-17s %-20s %-10s %5s %12s %6s %10s %7s %5s\r\n",
            "AppID",
            "Name",
            "State",
            "Prio",
            "Stack start",
            "Stack",
            "Stack Min",
            "Heap",
            "CPU");

        for(size_t i = 0; i < furi_thread_list_size(thread_list); i++) {
            const FuriThreadListItem* item = furi_thread_list_get_at(thread_list, i);
            printf(
                "%-17s %-20s %-10s %5d   0x%08lx %6lu %10lu %7zu %5.1f\r\n",
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

        if(interval > 0) {
            furi_delay_ms(interval);
        } else {
            break;
        }
    }
    furi_thread_list_free(thread_list);
}

void cli_command_free(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(args);
    UNUSED(context);

    printf("Free heap size: %zu\r\n", memmgr_get_free_heap());
    printf("Total heap size: %zu\r\n", memmgr_get_total_heap());
    printf("Minimum heap size: %zu\r\n", memmgr_get_minimum_free_heap());
    printf("Maximum heap block: %zu\r\n", memmgr_heap_get_max_free_block());

    printf("Pool free: %zu\r\n", memmgr_pool_get_free());
    printf("Maximum pool block: %zu\r\n", memmgr_pool_get_max_block());
}

void cli_command_free_blocks(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(args);
    UNUSED(context);

    memmgr_heap_printf_free_blocks();
}

void cli_command_echo(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);
    printf("%s\r\n", furi_string_get_cstr(args));
}

void cli_command_917_rx_callback(const void* data, size_t data_size, void* context) {
    furi_check(data);
    furi_check(context);

    FuriStreamBuffer* stream_buff = context;

    furi_check(
        furi_stream_buffer_send(stream_buff, data, data_size, FuriWaitForever) == data_size);
}

typedef struct {
    Intercom* intercom;
    FuriStreamBuffer* stream_buffer;
    uint8_t data[1024];
} CliCommand917Context;

static CliCommand917Context contex = {};

void cli_command_917(Cli* cli, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    CliCommand917Context* instance = context;

    printf("Starting 917 cli...\r\n");
    printf("Press Ctrl+] to exit\r\n");

    while(true) {
        uint8_t symbol = 0;
        size_t read_bytes = cli_read_timeout(cli, &symbol, 1, 10);
        if(read_bytes == 1) {
            size_t tx_bytes = intercom_tx(
                instance->intercom, IntercomChannelControl, &symbol, 1, FuriWaitForever);
            furi_check(tx_bytes == 1);
            if(symbol == CliSymbolAsciiETX) {
                printf("\r\nEnd of 917 cli session...\r\n\r\n");
                break;
            }

            size_t rx_bytes = furi_stream_buffer_bytes_available(instance->stream_buffer);
            if(rx_bytes) {
                furi_check(
                    furi_stream_buffer_receive(
                        instance->stream_buffer, instance->data, rx_bytes, 0) == rx_bytes);
                cli_write(cli, instance->data, rx_bytes);
            }
        }
    }
}

void cli_commands_init(Cli* cli) {
    //cli_add_command(cli, "!", CliCommandFlagParallelSafe, cli_command_info, (void*)true);
    //cli_add_command(cli, "info", CliCommandFlagParallelSafe, cli_command_info, NULL);
    //cli_add_command(cli, "device_info", CliCommandFlagParallelSafe, cli_command_info, (void*)true);

    cli_add_command(cli, "?", CliCommandFlagParallelSafe, cli_command_help, NULL);
    cli_add_command(cli, "help", CliCommandFlagParallelSafe, cli_command_help, NULL);

    cli_add_command(cli, "uptime", CliCommandFlagParallelSafe, cli_command_uptime, NULL);
    cli_add_command(cli, "log", CliCommandFlagParallelSafe, cli_command_log, NULL);
    cli_add_command(cli, "top", CliCommandFlagParallelSafe, cli_command_top, NULL);
    cli_add_command(cli, "free", CliCommandFlagParallelSafe, cli_command_free, NULL);
    cli_add_command(cli, "free_blocks", CliCommandFlagParallelSafe, cli_command_free_blocks, NULL);

    cli_add_command(cli, "gpio", CliCommandFlagDefault, cli_command_gpio, NULL);

    cli_add_command(cli, "echo", CliCommandFlagDefault, cli_command_echo, NULL);
    cli_add_command(cli, "display", CliCommandFlagParallelSafe, cli_command_display, NULL);
    cli_add_command(
        cli, "status_lights", CliCommandFlagParallelSafe, cli_command_status_lights, NULL);
    cli_add_command(
        cli, "light_sensor", CliCommandFlagParallelSafe, cli_command_light_sensor, NULL);
    cli_add_command(cli, "audio", CliCommandFlagParallelSafe, cli_command_audio, NULL);

    memset(&contex, 0, sizeof(CliCommand917Context));
    contex.intercom = furi_record_open(RECORD_INTERCOM);
    contex.stream_buffer = furi_stream_buffer_alloc(1024, 1);

    intercom_set_rx_callback(
        contex.intercom, IntercomChannelControl, cli_command_917_rx_callback, contex.stream_buffer);

    cli_add_command(cli, "917", CliCommandFlagParallelSafe, cli_command_917, &contex);
}
