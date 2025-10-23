#include "rps.h"

#define RPS_FW_VERSION_OFFSET 0x0C
#define RPS_FW_VERSION_SIZE   4

#define RPS_EXT_FW_VERSION_OFFSET 0x2C
#define RPS_EXT_FW_VERSION_SIZE   4

static_assert(sizeof(UpdaterRpsFwVersion) == RPS_FW_VERSION_SIZE);
static_assert(sizeof(UpdaterRpsExtFwVersion) == RPS_EXT_FW_VERSION_SIZE);

bool updater_rps_read_version(
    File* rps_file,
    UpdaterRpsFwVersion* version,
    UpdaterRpsExtFwVersion* ext_version) {
    furi_assert(rps_file);
    furi_assert(version);
    furi_assert(ext_version);

    bool is_success = false;
    do {
        size_t bytes_read;

        if(!storage_file_seek(rps_file, RPS_FW_VERSION_OFFSET, true)) {
            break;
        }

        bytes_read = storage_file_read(rps_file, version, sizeof(*version));
        if(bytes_read != sizeof(*version)) {
            break;
        }

        if(!storage_file_seek(rps_file, RPS_EXT_FW_VERSION_OFFSET, true)) {
            break;
        }

        bytes_read = storage_file_read(rps_file, ext_version, sizeof(*ext_version));
        if(bytes_read != sizeof(*ext_version)) {
            break;
        }

        is_success = true;
    } while(false);

    return is_success;
}
