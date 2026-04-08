#include "nwp_session.h"
#include "../helpers/rps.h"

#include <sl_info/sl_info.h>

#include <formatters/sl_rps/sl_rps.h>

static bool nwp_get_active_version(const char** active_version) {
#ifdef SRV_SL_INFO
    SlInfo* sl_info = furi_record_open(RECORD_SL_INFO);

    SlInfoStatus version_get_status =
        sl_info_get_value(sl_info, "sl_nwp_firmware", active_version);

    furi_record_close(RECORD_SL_INFO);
    return version_get_status == SlInfoStatusOk;
#else /* SRV_SL_INFO */
    UNUSED(active_version);

    return false;
#endif /* SRV_SL_INFO */
}

bool updater_nwp_session_is_current_version(const char* nwp_rps_path) {
    bool should_update = true;

#ifdef SRV_SL_INFO
    const char* active_version_string;
    if(nwp_get_active_version(&active_version_string)) {
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

            should_update = !furi_string_equal(update_version_string, active_version_string);
            furi_string_free(update_version_string);
        } while(false);

        storage_file_free(nwp_rps_file);
        furi_record_close(RECORD_STORAGE);
    }
#else /* SRV_SL_INFO */
    UNUSED(nwp_rps_path);
    UNUSED(nwp_get_active_version);
#endif /* SRV_SL_INFO */

    return should_update;
}
