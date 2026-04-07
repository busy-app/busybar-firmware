#include "font_registry_i.h"

#include <cli/cli_command.h>
#include <cli/args.h>

typedef struct {
    bool help;
} FontstatCliArguments;

static const FontstatCliArguments fontstat_cli_default_arguments = {
    .help = false,
};

static void fontstat_cli_print_usage(void) {
    printf("Usage: fontstat [options]\r\n");
    printf("Print font registry cache state.\r\n");
    printf("Options:\r\n");
    printf("  -h, --help    Show this help message\r\n");
}

static void fontstat_cli_parse_arguments(FuriString* args_string, FontstatCliArguments* args) {
    *args = fontstat_cli_default_arguments;

    FuriString* arg = furi_string_alloc();
    while(args_read_string_and_trim(args_string, arg)) {
        if(furi_string_equal_str(arg, "-h") || furi_string_equal_str(arg, "--help")) {
            args->help = true;
        } else {
            printf("Unknown argument: %s\r\n", furi_string_get_cstr(arg));
            args->help = true;
            break;
        }
    }

    furi_string_free(arg);
}

void fontstat_cli_command_entry(PipeSide* pipe, FuriString* args_string, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    FontstatCliArguments args;
    fontstat_cli_parse_arguments(args_string, &args);

    if(args.help) {
        fontstat_cli_print_usage();
        return;
    }

    FontRegistry* registry = furi_record_open(RECORD_FONT_REGISTRY);
    FuriString* output_buffer = furi_string_alloc();

    furi_check(furi_mutex_acquire(registry->mutex, FuriWaitForever) == FuriStatusOk);

    size_t fonts_count = stbds_shlenu(registry->loaded_fonts);
    size_t total_estimated_memory_size = 0;

    for(size_t i = 0; i < fonts_count; i++) {
        FontRegistryLoadedFont* font = &registry->loaded_fonts[i];
        total_estimated_memory_size += font->value.estimated_memory_size;

        furi_string_cat_printf(
            output_buffer,
            "%-50s %4zu %8zu\r\n",
            font->key,
            font->value.references,
            font->value.estimated_memory_size);
    }

    furi_check(furi_mutex_release(registry->mutex) == FuriStatusOk);

    printf("Loaded: %zu fonts, %zu bytes\r\n", fonts_count, total_estimated_memory_size);
    printf("%-50s %4s %8s\r\n", "Font", "Refs", "Size");
    printf("%s", furi_string_get_cstr(output_buffer));

    furi_string_free(output_buffer);
    furi_record_close(RECORD_FONT_REGISTRY);
}
