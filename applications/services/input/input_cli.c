#include <furi/furi.h>

#include <input/input.h>
#include <cli/cli.h>
#include <toolbox/args.h>

static void input_cli_print_usage(Cli* cli, void* context) {
    UNUSED(cli);
    UNUSED(context);

    printf("Usage: input <command>\r\n");
    printf("Commands:\r\n");
    printf("\tdump - Dumps incoming input events\r\n");
    printf("\tsend <key> <type> - Inject input\r\n");
}

static void input_cli_dump_events_callback(const void* value, void* ctx) {
    furi_assert(value);
    furi_assert(ctx);
    FuriMessageQueue* input_queue = ctx;
    furi_message_queue_put(input_queue, value, FuriWaitForever);
}

static void input_cli_dump(Cli* cli, FuriPubSub* input_events) {
    FuriMessageQueue* input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    FuriPubSubSubscription* input_subscription =
        furi_pubsub_subscribe(input_events, input_cli_dump_events_callback, input_queue);

    InputEvent input_event;
    printf("Press CTRL+C to stop\r\n");
    while(!cli_cmd_interrupt_received(cli)) {
        if(furi_message_queue_get(input_queue, &input_event, 100) == FuriStatusOk) {
            printf(
                "key: %s type: %s\r\n",
                input_get_key_name(input_event.key),
                input_get_type_name(input_event.type));
            fflush(stdout);
        }
    }

    furi_pubsub_unsubscribe(input_events, input_subscription);
    furi_message_queue_free(input_queue);
}

static void input_cli_command_send(Cli* cli, FuriString* args, FuriPubSub* input_events) {
    UNUSED(cli);
    UNUSED(args);

    InputKey key = InputKeyUp;
    InputType type = InputTypePress;
    FuriString* str_tmp = furi_string_alloc();

    bool args_parsed = false;
    do {
        if(!args_read_string_and_trim(args, str_tmp)) break;
        size_t i = 0;
        for(i = 0; i < InputKeyMAX; i++) {
            if(furi_string_cmp_str(str_tmp, input_get_key_name(i)) == 0) break;
        }
        if(i == InputKeyMAX) break;
        key = i;

        if(!args_read_string_and_trim(args, str_tmp)) break;
        for(i = 0; i < InputTypeMAX; i++) {
            if(furi_string_cmp_str(str_tmp, input_get_type_name(i)) == 0) break;
        }
        if(i == InputTypeMAX) break;
        type = i;

        args_parsed = true;
    } while(false);

    if(!args_parsed) {
        input_cli_print_usage(cli, NULL);
    } else {
        InputEvent event = {
            .key = key,
            .type = type,
        };
        furi_pubsub_publish(input_events, &event);
    }

    furi_string_free(str_tmp);
}

static void input_cli_command(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(args);
    UNUSED(context);

    FuriString* cmd = furi_string_alloc();
    FuriPubSub* input_events = furi_record_open(RECORD_INPUT_EVENTS);

    bool cmd_parsed = false;
    do {
        if(!args_read_string_and_trim(args, cmd)) break;
        if(furi_string_cmp_str(cmd, "dump") == 0) {
            input_cli_dump(cli, input_events);
        } else if(furi_string_cmp_str(cmd, "send") == 0) {
            input_cli_command_send(cli, args, input_events);
        } else {
            break;
        }

        cmd_parsed = true;
    } while(false);

    if(!cmd_parsed) {
        input_cli_print_usage(cli, NULL);
    }

    furi_string_free(cmd);
    furi_record_close(RECORD_INPUT_EVENTS);
}

void input_on_system_start(void) {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, "input", CliCommandFlagParallelSafe, input_cli_command, NULL);
    furi_record_close(RECORD_CLI);
#else
    UNUSED(nfc_cli);
#endif
}
