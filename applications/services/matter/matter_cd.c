#include "matter_cd.h"

#define TAG                      "MatterCd"
#define CONFIG_FILE_PATH         EXT_PATH("apps_data/matter/cd_selection.txt")
#define CERTIFICATE_PATH(format) EXT_PATH("apps_assets/matter/cd-" format ".der")

// defines fallback order
static const char* const cd_certificates[] = {
    "production",
    "dev",
    "certification",
};

static_assert(COUNT_OF(cd_certificates) > 0);

void matter_cd_init(MatterCd* cd) {
    furi_assert(cd);
    cd->storage = furi_record_open(RECORD_STORAGE);
}

bool matter_cd_prepare_initialization_frame(MatterCd* cd, MatterIntercomFrame* frame) {
    furi_assert(cd);
    furi_assert(frame);

    storage_simply_read_entire_file(
        cd->storage, CONFIG_FILE_PATH, cd->wanted_selection, sizeof(cd->wanted_selection));

    int selected_idx = -1;
    for(size_t i = 0; i < COUNT_OF(cd_certificates); i++) {
        if(strcmp(cd->wanted_selection, cd_certificates[i]) == 0) {
            selected_idx = i;
            break;
        }
    }

    if(selected_idx == -1) {
        FURI_LOG_I(TAG, "no certificate selected or invalid selection - falling back");
        selected_idx = 0;
    }

    cd->de_facto_selection = NULL;
    frame->type = MatterIntercomFrameTypeCdCertificate;
    MatterIntercomCdCertificateFrame* cert_frame = &frame->cd_certificate;

    for(size_t i = selected_idx; i < COUNT_OF(cd_certificates); i++) {
        const char* candidate = cd_certificates[i];

        char candidate_path[128];
        snprintf(candidate_path, sizeof(candidate_path), CERTIFICATE_PATH("%s"), candidate);
        FURI_LOG_D(TAG, "trying \"%s\" (idx %zu) at \"%s\"", candidate, i, candidate_path);

        cert_frame->contents_length = storage_simply_read_entire_file(
            cd->storage, candidate_path, cert_frame->contents, sizeof(cert_frame->contents));
        bool candidate_valid = cert_frame->contents_length > 0;
        if(!candidate_valid) continue;

        FURI_LOG_D(TAG, "success");
        cd->de_facto_selection = candidate;
        break;
    }

    if(!cd->de_facto_selection) {
        FURI_LOG_E(TAG, "no certificates available");
        return false;
    }

    return true;
}

const char* matter_cd_get_wanted_selection(MatterCd* cd) {
    furi_assert(cd);
    return cd->wanted_selection;
}

bool matter_cd_set_wanted_selection(MatterCd* cd, const char* selection) {
    furi_assert(cd);
    furi_assert(selection);
    return storage_simply_write_entire_file(
        cd->storage, CONFIG_FILE_PATH, selection, strlen(selection));
}

const char* matter_cd_get_de_facto_selection(MatterCd* cd) {
    furi_assert(cd);
    furi_assert(cd->de_facto_selection);
    return cd->de_facto_selection;
}
