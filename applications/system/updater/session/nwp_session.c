#include "nwp_session.h"
#include "../helpers/rps.h"

#include <storage/storage.h>
#include <cli_u5/cli_command_sl_cli.h>

#include <containers/pipe_util.h>
#include <formatters/sl_rps/sl_rps.h>

#define NWP_VERSION_CLI_THREAD_NAME       "updater_nwp_version_cli"
#define NWP_VERSION_CLI_THREAD_STACK_SIZE 512

#define NWP_VERSION_CLI_PIPE_CAPACITY   128
#define NWP_VERSION_RECEIVE_BUFFER_SIZE 64
#define NWP_VERSION_DRAIN_BUFFER_SIZE   64

static int32_t nwp_get_active_version_thread_entry(void* context) {
    PipeSide* tx_pipe = context;

    bool success = cli_command_sl_cli_send_command_get_response(tx_pipe, "device_info");
    pipe_free(tx_pipe);
    return success ? 0 : -1;
}

static bool nwp_get_active_version(FuriString* nwp_version) {
    PipeSideBundle pipe_bundle = pipe_alloc(NWP_VERSION_CLI_PIPE_CAPACITY, 1);
    PipeSide* rx_pipe = pipe_bundle.alices_side;
    PipeSide* tx_pipe = pipe_bundle.bobs_side;

    FuriThread* get_version_thread = furi_thread_alloc_ex(
        NWP_VERSION_CLI_THREAD_NAME,
        NWP_VERSION_CLI_THREAD_STACK_SIZE,
        nwp_get_active_version_thread_entry,
        tx_pipe);

    furi_thread_start(get_version_thread);

    if(pipe_copy_until(rx_pipe, NULL, "sl_nwp_firmware") && pipe_copy_until(rx_pipe, NULL, ": ")) {
        char receive_buffer[NWP_VERSION_RECEIVE_BUFFER_SIZE];

        size_t bytes_read;
        while((bytes_read = pipe_receive(rx_pipe, receive_buffer, sizeof(receive_buffer))) > 0) {
            size_t idx;
            for(idx = 0; idx < bytes_read; idx++) {
                if(receive_buffer[idx] == '\r' || receive_buffer[idx] == '\n') {
                    receive_buffer[idx] = '\0';
                    break;
                }
            }

            furi_string_cat_str(nwp_version, receive_buffer);

            if(idx < bytes_read) {
                break;
            }
        }
    }

    char drain_buffer[NWP_VERSION_DRAIN_BUFFER_SIZE];
    while(pipe_state(rx_pipe) != PipeStateBroken) {
        pipe_receive(rx_pipe, drain_buffer, sizeof(drain_buffer));
    }

    pipe_free(rx_pipe);
    furi_thread_join(get_version_thread);
    furi_thread_free(get_version_thread);

    return furi_string_size(nwp_version) > 0;
}

bool updater_nwp_session_is_current_version(const char* nwp_rps_path) {
    bool should_update = true;

    FuriString* active_version_string = furi_string_alloc();
    if(nwp_get_active_version(active_version_string)) {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        File* nwp_rps_file = storage_file_alloc(storage);

        do {
            if(!storage_file_open(nwp_rps_file, nwp_rps_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
                break;
            }

            UpdaterRpsFwVersion update_version;
            UpdaterRpsExtFwVersion update_ext_version;
            if(!updater_rps_read_version(nwp_rps_file, &update_version, &update_ext_version)) {
                break;
            }

            FuriString* update_version_string = furi_string_alloc();
            sl_rps_format_nwp_version(
                update_version_string,
                &(SlRpsNwpVersion){
                    .major = update_version.major,
                    .minor = update_version.minor,
                    .patch = update_ext_version.patch,
                    .build = update_version.build,
                    .security = update_version.security,
                    .rom_id = update_ext_version.rom_id,
                    .chip_id = update_ext_version.chip_id,
                    .customer_id = update_ext_version.customer_id,
                });

            should_update = !furi_string_equal(active_version_string, update_version_string);
            furi_string_free(update_version_string);
        } while(false);

        storage_file_free(nwp_rps_file);
        furi_record_close(RECORD_STORAGE);
    }

    furi_string_free(active_version_string);

    return should_update;
}
