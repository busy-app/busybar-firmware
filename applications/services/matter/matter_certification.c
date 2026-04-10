#include "matter_i.h"

#include <storage/storage.h>

#define TAG "MatterCert"

#define CERT_TYPE_NAME_LEN_MAX   (64)
#define CERTIFICATE_PATH_LEN_MAX (128)

#define CONFIG_FILE_DIR          EXT_PATH("apps_data/matter")
#define CONFIG_FILE_PATH         EXT_PATH("apps_data/matter/cd_selection.txt")
#define CERTIFICATE_PATH(format) EXT_PATH("apps_assets/matter/cd-" format ".der")

// defines fallback order
static const char* const certification_name_table[MatterCertificationTypeMax] = {
    [MatterCertificationTypeProduction] = "production",
    [MatterCertificationTypeDevelopment] = "dev",
    [MatterCertificationTypeProvisional] = "certification",
};

static MatterStatus matter_certification_read_config_file(char* buf, size_t buf_len) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    const size_t read_size =
        storage_simply_read_entire_file(storage, CONFIG_FILE_PATH, buf, buf_len);

    furi_record_close(RECORD_STORAGE);

    return (read_size > 0) ? MatterStatusOk : MatterStatusFsError;
}

static MatterStatus matter_certification_check_cd_file(const char* file_path) {
    MatterStatus status = MatterStatusFsError;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            break;
        }

        if(storage_file_size(file) == 0) {
            break;
        }

        status = MatterStatusOk;
    } while(false);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return status;
}

static MatterStatus
    matter_certification_name_to_type(const char* name, MatterCertificationType* cert_type) {
    MatterStatus status = MatterStatusBadConfig;

    for(MatterCertificationType i = 0; i < MatterCertificationTypeMax; ++i) {
        if(strncmp(name, certification_name_table[i], CERT_TYPE_NAME_LEN_MAX) == 0) {
            *cert_type = i;
            status = MatterStatusOk;
            break;
        }
    }

    return status;
}

static void matter_certification_get_cd_path_by_type(
    MatterCertificationType cert_type,
    char* cd_path,
    size_t cd_path_size) {
    snprintf(cd_path, cd_path_size, CERTIFICATE_PATH("%s"), certification_name_table[cert_type]);
}

static MatterStatus matter_certification_load_wanted_config(MatterCertificationType* cert_type) {
    MatterStatus status;

    do {
        char cert_type_name[CERT_TYPE_NAME_LEN_MAX + 1];

        status = matter_certification_read_config_file(cert_type_name, sizeof(cert_type_name));

        if(status != MatterStatusOk) {
            break;
        }

        status = matter_certification_name_to_type(cert_type_name, cert_type);

    } while(false);

    return status;
}

static MatterStatus matter_certification_load_actual_config(
    MatterCertificationType* cert_type,
    MatterCertificationType search_from_type) {
    MatterStatus status = MatterStatusFsError;

    for(MatterCertificationType i = search_from_type; i < MatterCertificationTypeMax; ++i) {
        char cd_file_path[CERTIFICATE_PATH_LEN_MAX + 1];
        matter_certification_get_cd_path_by_type(i, cd_file_path, sizeof(cd_file_path));

        status = matter_certification_check_cd_file(cd_file_path);

        if(status == MatterStatusOk) {
            *cert_type = i;
            break;
        }
    }

    return status;
}

// ========= Internal API =========

MatterStatus matter_certification_read_config(MatterCertificationConfig* cert_info) {
    MatterStatus status;

    do {
        status = matter_certification_load_wanted_config(&cert_info->wanted);

        if(status != MatterStatusOk) {
            FURI_LOG_W(TAG, "Certification type not configured or invalid, falling back");
            cert_info->wanted = MatterCertificationTypeMax;
        }

        const MatterCertificationType start_from_type =
            (cert_info->wanted < MatterCertificationTypeMax) ? cert_info->wanted :
                                                               MatterCertificationTypeProduction;

        status = matter_certification_load_actual_config(&cert_info->actual, start_from_type);

        if(status != MatterStatusOk) {
            FURI_LOG_E(TAG, "No certification data available");
            cert_info->actual = MatterCertificationTypeMax;
            break;
        }

        FURI_LOG_I(TAG, "Config loaded");

    } while(false);

    return status;
}

MatterStatus matter_certification_get_cd(
    MatterCertificationType cert_type,
    MatterCertificateDeclaration* cd) {
    furi_assert(cert_type < MatterCertificationTypeMax);

    MatterStatus status;

    char cd_file_path[CERTIFICATE_PATH_LEN_MAX + 1];
    matter_certification_get_cd_path_by_type(cert_type, cd_file_path, sizeof(cd_file_path));

    Storage* storage = furi_record_open(RECORD_STORAGE);

    cd->length =
        storage_simply_read_entire_file(storage, cd_file_path, cd->data, sizeof(cd->data));

    furi_record_close(RECORD_STORAGE);

    if(cd->length > 0) {
        FURI_LOG_D(TAG, "Certification data file read success");
        status = MatterStatusOk;
    } else {
        FURI_LOG_E(TAG, "Failed to read certification data file");
        status = MatterStatusFsError;
    }

    return status;
}

// TODO: Port to SettingProvider
MatterStatus matter_certification_set_config(MatterCertificationType cert_type) {
    furi_assert(cert_type < MatterCertificationTypeMax);

    MatterStatus status;
    const char* cert_type_name = certification_name_table[cert_type];

    Storage* storage = furi_record_open(RECORD_STORAGE);

    storage_simply_mkdir(storage, CONFIG_FILE_DIR);

    const bool file_write_success = storage_simply_write_entire_file(
        storage, CONFIG_FILE_PATH, cert_type_name, strlen(cert_type_name));

    furi_record_close(RECORD_STORAGE);

    if(file_write_success) {
        FURI_LOG_I(TAG, "Config set successfully");
        status = MatterStatusOk;
    } else {
        FURI_LOG_E(TAG, "Failed to set config");
        status = MatterStatusFsError;
    }

    return status;
}
