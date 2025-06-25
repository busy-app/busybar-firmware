#include <furi_hal_info.h>

#include <furi_hal_version.h>
#include <furi.h>

#define FURI_HAL_917_MBR_SIZE                                496
#define FURI_HAL_917_MBR_ADDRESS                             0x081f0000
#define FURI_HAL_917_MBR_1_8_VERSION                         0x1F
#define FURI_HAL_917_MBR_1_6_VERSION                         0x1B
#define FURI_HAL_917_PACKAGE_TYPE_VALUES_OFFSET_COMMON_FLASH 0x81F0292
#define FURI_HAL_917_SILICON_REV_VALUES_OFFSET_COMMON_FLASH  0x81F0293
#define FURI_HAL_917_COMMON_FLASH_IPMU_VALUES_OFFSET         0x81F0258

typedef struct {
    uint8_t _reserved0[337];

    uint8_t r337_0_5                        : 6;
    uint8_t ta_secure_boot_enable           : 1;
    uint8_t ta_anti_roll_back               : 1;

    uint8_t ta_digital_signature_validation : 1;
    uint8_t m4_anti_roll_back               : 1;
    uint8_t m4_digital_signature_validation : 1;
    uint8_t r338_3_7                        : 5;

    uint8_t _reserved1[4];
    uint8_t r343_0_0            : 1;
    uint8_t ta_encrypt_firmware : 1;
    uint8_t r343_2_7            : 6;
    uint8_t _reserved2[1];

    uint8_t r345_0_3              : 4;
    uint8_t m4_secure_boot_enable : 1;
    uint8_t m4_encrypt_firmware   : 1;
    uint8_t r345_6_7              : 2;

    uint8_t _reserved3[8];

    uint8_t r354_0_4                       : 5;
    uint8_t disable_m4_access_frm_tass_sec : 1;
    uint8_t r354_6_7                       : 2;

    uint8_t _reserved4[2];

    uint8_t r357_0_3              : 4;
    uint8_t m4_fw_encryption_mode : 1;
    uint8_t r357_5_7              : 3;

    uint8_t _reserved6[46];
    uint8_t mbr_variant;
    uint8_t _reserved5[91];
} FURI_PACKED FuriHal917Mbr;
static_assert(sizeof(FuriHal917Mbr) == FURI_HAL_917_MBR_SIZE, "failed");

FURI_WEAK void furi_hal_info_get_api_version(uint16_t* major, uint16_t* minor) {
    *major = 0;
    *minor = 0;
}

void furi_hal_info_get(PropertyValueCallback out, char sep, void* context) {
    UNUSED(out);
    UNUSED(sep);
    UNUSED(context);

    FuriString* key = furi_string_alloc();
    FuriString* value = furi_string_alloc();

    PropertyValueContext property_context = {
        .key = key, .value = value, .out = out, .sep = sep, .last = false, .context = context};
    FuriHal917Mbr* furi_hal_917_mbr = (FuriHal917Mbr*)FURI_HAL_917_MBR_ADDRESS;

    // Firmware version
    const Version* firmware_version = furi_hal_version_get_firmware_version();
    if(firmware_version) {
        if(sep == '.') {
            property_value_out(
                &property_context,
                NULL,
                4,
                "917"
                "firmware",
                "commit",
                "hash",
                version_get_githash(firmware_version));
        } else {
            property_value_out(
                &property_context,
                NULL,
                3,
                "917",
                "firmware",
                "commit",
                version_get_githash(firmware_version));
        }

        property_value_out(
            &property_context,
            NULL,
            4,
            "917",
            "firmware",
            "commit",
            "dirty",
            version_get_dirty_flag(firmware_version) ? "true" : "false");

        if(sep == '.') {
            property_value_out(
                &property_context,
                NULL,
                5,
                "917",
                "firmware",
                "branch",
                "name",
                version_get_gitbranch(firmware_version));
        } else {
            property_value_out(
                &property_context,
                NULL,
                3,
                "917",
                "firmware",
                "branch",
                version_get_gitbranch(firmware_version));
        }

        property_value_out(
            &property_context,
            NULL,
            4,
            "917",
            "firmware",
            "branch",
            "num",
            version_get_gitbranchnum(firmware_version));
        property_value_out(
            &property_context,
            NULL,
            3,
            "917",
            "firmware",
            "version",
            version_get_version(firmware_version));
        property_value_out(
            &property_context,
            NULL,
            4,
            "917",
            "firmware",
            "build",
            "date",
            version_get_builddate(firmware_version));
        property_value_out(
            &property_context,
            "%d",
            3,
            "917",
            "firmware",
            "target",
            version_get_target(firmware_version));

        uint16_t api_version_major, api_version_minor;
        furi_hal_info_get_api_version(&api_version_major, &api_version_minor);
        property_value_out(
            &property_context, "%d", 4, "917", "firmware", "api", "major", api_version_major);
        property_value_out(
            &property_context, "%d", 4, "917", "firmware", "api", "minor", api_version_minor);

        property_value_out(
            &property_context,
            NULL,
            4,
            "917",
            "firmware",
            "origin",
            "fork",
            version_get_firmware_origin(firmware_version));

        property_value_out(
            &property_context,
            NULL,
            4,
            "917",
            "firmware",
            "origin",
            "git",
            version_get_git_origin(firmware_version));

        // MBR security flags
        property_value_out(
            &property_context,
            NULL,
            5,
            "917",
            "ta",
            "anti",
            "roll",
            "back",
            furi_hal_917_mbr->ta_anti_roll_back ? "true" : "false");
        property_value_out(
            &property_context,
            NULL,
            5,
            "917",
            "ta",
            "digital",
            "signature",
            "validation",
            furi_hal_917_mbr->ta_digital_signature_validation ? "true" : "false");
        property_value_out(
            &property_context,
            NULL,
            4,
            "917",
            "ta",
            "encrypt",
            "firmware",
            furi_hal_917_mbr->ta_encrypt_firmware ? "true" : "false");
        property_value_out(
            &property_context,
            NULL,
            4,
            "917",
            "ta",
            "secure",
            "boot",
            furi_hal_917_mbr->ta_secure_boot_enable ? "true" : "false");
        property_value_out(
            &property_context,
            NULL,
            5,
            "917",
            "m4",
            "anti",
            "roll",
            "back",
            furi_hal_917_mbr->m4_anti_roll_back ? "true" : "false");
        property_value_out(
            &property_context,
            NULL,
            5,
            "917",
            "m4",
            "digital",
            "signature",
            "validation",
            furi_hal_917_mbr->m4_digital_signature_validation ? "true" : "false");
        property_value_out(
            &property_context,
            NULL,
            4,
            "917",
            "m4",
            "encrypt",
            "firmware",
            furi_hal_917_mbr->m4_encrypt_firmware ? "true" : "false");
        property_value_out(
            &property_context,
            NULL,
            4,
            "917",
            "m4",
            "secure",
            "boot",
            furi_hal_917_mbr->m4_secure_boot_enable ? "true" : "false");
        property_value_out(
            &property_context,
            NULL,
            5,
            "917",
            "m4",
            "firmware",
            "encryption",
            "mode",
            furi_hal_917_mbr->m4_fw_encryption_mode ? "true" : "false");

        FuriString* ver_name = furi_string_alloc();
        furi_string_printf(
            ver_name,
            "%02X (%s)",
            furi_hal_917_mbr->mbr_variant,
            furi_hal_917_mbr->mbr_variant == FURI_HAL_917_MBR_1_8_VERSION ? "1.8 Mb" :
            furi_hal_917_mbr->mbr_variant == FURI_HAL_917_MBR_1_6_VERSION ? "1.6 Mb" :
                                                                            "Unknown");
        property_value_out(
            &property_context,
            NULL,
            4,
            "917",
            "mbr",
            "variant",
            "value",
            furi_string_get_cstr(ver_name));
        furi_string_free(ver_name);

        property_value_out(
            &property_context,
            NULL,
            7,
            "917",
            "disable",
            "m4",
            "access",
            "from",
            "tass",
            "sec",
            furi_hal_917_mbr->disable_m4_access_frm_tass_sec ? "true" : "false");
    }

    furi_string_free(key);
    furi_string_free(value);
}
