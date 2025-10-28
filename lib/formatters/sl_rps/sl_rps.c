#include "sl_rps.h"

void sl_rps_format_nwp_version(FuriString* string, const SlRpsNwpVersion* data) {
    furi_check(string);
    furi_check(data);

    furi_string_printf(
        string,
        "%x%x.%u.%u.%u.%u.%u.%u",
        data->chip_id,
        data->rom_id,
        data->major,
        data->minor,
        data->security,
        data->patch,
        data->customer_id,
        data->build);
}
