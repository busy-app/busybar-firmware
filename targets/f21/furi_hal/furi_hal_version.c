#include <furi_hal_version.h>
#include <furi_hal_flash_otp.h>
#include <furi_hal_rtc.h>

#include <stm32u5xx_ll_utils.h>

#include <furi.h>

#define TAG "FuriHalVersion"

#define FLIPPER_MAC_0 0x0C
#define FLIPPER_MAC_1 0xFA
#define FLIPPER_MAC_2 0x22

#define OTP1_MAC_SIZE   (6)
#define OTP1_MODEL_SIZE (8)

#define HW_UID_SIZE (12)

typedef struct {
    bool otp1_valid;
    bool otp2_valid;
    bool otp3_valid;
    bool otp4_valid;

    // Cached/derived values
    char model_code[OTP1_MODEL_SIZE + 1]; // Null-terminated model
    uint8_t usb_mac[OTP1_MAC_SIZE];
    uint8_t hw_uid[HW_UID_SIZE];
} FuriHalVersionState;

static FuriHalVersionState furi_hal_version_state = {0};

// OTP1 Data Structure - Hardware/Board Info (Production)
// Layout: <H B B I 6s 8s B B B B (little-endian)
// Total size: 4 (header) + 22 (data) = 26 bytes

typedef struct {
    FuriHalFlashOtpHeader header; // Common header (magic, index=1, version)
    uint32_t hw_timestamp; // Production timestamp (Unix epoch)
    uint8_t u5_usb_mac[OTP1_MAC_SIZE]; // USB MAC address
    char hw_model[OTP1_MODEL_SIZE]; // Hardware model string (e.g., "BB.1")
    uint8_t hw_version; // Hardware version
    uint8_t hw_target; // Target ID
    uint8_t hw_body; // Body type
    uint8_t hw_connect; // Connect type
} FURI_PACKED Otp1Data;

_Static_assert(sizeof(Otp1Data) == 26, "OTP1 data structure size mismatch");

// OTP2 Data Structure - Device Info (QC)
// Layout: <H B B I B B (little-endian)
// Total size: 4 (header) + 6 (data) = 10 bytes
typedef struct {
    FuriHalFlashOtpHeader header; // Common header (magic, index=2, version)
    uint32_t hw_timestamp_qc; // QC timestamp (Unix epoch)
    uint8_t hw_color; // Device color
    uint8_t hw_region; // Device region
} FURI_PACKED Otp2Data;

_Static_assert(sizeof(Otp2Data) == 10, "OTP2 data structure size mismatch");

// OTP3 Data Structure - Public Key Storage
// Layout: <H B B B 56s (little-endian)
// Total size: 4 (header) + 57 (data) = 61 bytes
#define OTP3_PKEY_SIZE (56) // secp224r1: 28 bytes X + 28 bytes Y

typedef struct {
    FuriHalFlashOtpHeader header; // Common header (magic, index=3, version)
    uint8_t hw_otp3_curve; // EC curve identifier
    uint8_t hw_otp3_pkey[OTP3_PKEY_SIZE]; // Public key (X || Y)
} FURI_PACKED Otp3Data;

_Static_assert(sizeof(Otp3Data) == 61, "OTP3 data structure size mismatch");

// OTP4 Data Structure - Signatures
// Layout: <H B B 12s 56s 56s (little-endian)
// Total size: 4 (header) + 124 (data) = 128 bytes
#define OTP4_MCU_UID_SIZE   (12)
#define OTP4_SIGNATURE_SIZE (56) // secp224r1: 28 bytes R + 28 bytes S

typedef struct {
    FuriHalFlashOtpHeader header; // Common header (magic, index=4, version)
    uint8_t hw_otp4_mcu_uid[OTP4_MCU_UID_SIZE]; // MCU UID (for binding)
    uint8_t hw_otp1_signature[OTP4_SIGNATURE_SIZE]; // Signature over OTP1
    uint8_t hw_otp2_signature[OTP4_SIGNATURE_SIZE]; // Signature over OTP2
} FURI_PACKED Otp4Data;

_Static_assert(sizeof(Otp4Data) == 128, "OTP4 data structure size mismatch");

/*** OTP Access Helpers ***/

static const Otp1Data* otp_get_otp1(void) {
    return (const Otp1Data*)furi_hal_flash_otp_get_block_address(FuriHalOtpBlockOtp1);
}

static const Otp2Data* otp_get_otp2(void) {
    return (const Otp2Data*)furi_hal_flash_otp_get_block_address(FuriHalOtpBlockOtp2);
}

static const Otp3Data* otp_get_otp3(void) {
    return (const Otp3Data*)furi_hal_flash_otp_get_block_address(FuriHalOtpBlockOtp3);
}

static const Otp4Data* otp_get_otp4(void) {
    return (const Otp4Data*)furi_hal_flash_otp_get_block_address(FuriHalOtpBlockOtp4);
}

static bool otp_block_is_empty(const void* addr, size_t size) {
    const uint8_t* ptr = (const uint8_t*)addr;
    for(size_t i = 0; i < size; i++) {
        if(ptr[i] != 0xFF) {
            return false;
        }
    }
    return true;
}

static bool otp_is_otp1_provisioned(void) {
    const Otp1Data* otp1 = otp_get_otp1();
    if(otp_block_is_empty(otp1, sizeof(Otp1Data))) {
        return false;
    }
    return furi_hal_flash_otp_header_is_valid(&otp1->header, FuriHalOtpBlockOtp1);
}

static bool otp_is_otp2_provisioned(void) {
    const Otp2Data* otp2 = otp_get_otp2();
    if(otp_block_is_empty(otp2, sizeof(Otp2Data))) {
        return false;
    }
    return furi_hal_flash_otp_header_is_valid(&otp2->header, FuriHalOtpBlockOtp2);
}

static bool otp_is_otp3_provisioned(void) {
    const Otp3Data* otp3 = otp_get_otp3();
    if(otp_block_is_empty(otp3, sizeof(Otp3Data))) {
        return false;
    }
    return furi_hal_flash_otp_header_is_valid(&otp3->header, FuriHalOtpBlockOtp3);
}

static bool otp_is_otp4_provisioned(void) {
    const Otp4Data* otp4 = otp_get_otp4();
    if(otp_block_is_empty(otp4, sizeof(Otp4Data))) {
        return false;
    }
    return furi_hal_flash_otp_header_is_valid(&otp4->header, FuriHalOtpBlockOtp4);
}

/*** Initialization ***/

void furi_hal_version_init(void) {
    // Copy MCU UID, reversing byte order
    for(size_t i = 0; i < HW_UID_SIZE; i++) {
        furi_hal_version_state.hw_uid[i] = ((uint8_t*)UID_BASE_ADDRESS)[HW_UID_SIZE - 1 - i];
    }

    // Check OTP provisioning status
    furi_hal_version_state.otp1_valid = otp_is_otp1_provisioned();
    furi_hal_version_state.otp2_valid = otp_is_otp2_provisioned();
    furi_hal_version_state.otp3_valid = otp_is_otp3_provisioned();
    furi_hal_version_state.otp4_valid = otp_is_otp4_provisioned();

    // Initialize model name
    if(furi_hal_version_state.otp1_valid) {
        const Otp1Data* otp1 = otp_get_otp1();
        memcpy(furi_hal_version_state.model_code, otp1->hw_model, OTP1_MODEL_SIZE);
        furi_hal_version_state.model_code[OTP1_MODEL_SIZE] = '\0';
    } else {
        strncpy(
            furi_hal_version_state.model_code,
            "Unknown",
            sizeof(furi_hal_version_state.model_code) - 1);
    }

    // Initialize USB MAC address
    if(furi_hal_version_state.otp1_valid) {
        const Otp1Data* otp1 = otp_get_otp1();
        memcpy(furi_hal_version_state.usb_mac, otp1->u5_usb_mac, 6);
    } else {
        // Generate from MCU UID if not provisioned
        uint32_t uid[2] = {0};
        uid[0] = LL_GetUID_Word0();
        uid[1] = LL_GetUID_Word1();

        furi_hal_version_state.usb_mac[0] = FLIPPER_MAC_0;
        furi_hal_version_state.usb_mac[1] = FLIPPER_MAC_1;
        furi_hal_version_state.usb_mac[2] = FLIPPER_MAC_2;
        furi_hal_version_state.usb_mac[3] = (uid[0] >> 16) & 0xFF;
        furi_hal_version_state.usb_mac[4] = uid[0] & 0xFF;
        furi_hal_version_state.usb_mac[5] = (uid[1] >> 16) & 0xFF;
    }

    FURI_LOG_I(
        TAG,
        "OTP1: %s, OTP2: %s, OTP3: %s, OTP4: %s",
        furi_hal_version_state.otp1_valid ? "valid" : "empty",
        furi_hal_version_state.otp2_valid ? "valid" : "empty",
        furi_hal_version_state.otp3_valid ? "valid" : "empty",
        furi_hal_version_state.otp4_valid ? "valid" : "empty");
}

/***  Hardware Info from OTP1 ***/

uint8_t furi_hal_version_get_hw_version(void) {
    if(!furi_hal_version_state.otp1_valid) {
        return 0;
    }
    return otp_get_otp1()->hw_version;
}

uint8_t furi_hal_version_get_hw_target(void) {
    if(!furi_hal_version_state.otp1_valid) {
        return version_get_target(version_get());
    }
    return otp_get_otp1()->hw_target;
}

uint8_t furi_hal_version_get_hw_target_otp(void) {
    if(!furi_hal_version_state.otp1_valid) {
        return 0;
    }
    return otp_get_otp1()->hw_target;
}

uint8_t furi_hal_version_get_hw_body(void) {
    if(!furi_hal_version_state.otp1_valid) {
        return 0;
    }
    return otp_get_otp1()->hw_body;
}

uint8_t furi_hal_version_get_hw_connect(void) {
    if(!furi_hal_version_state.otp1_valid) {
        return 0;
    }
    return otp_get_otp1()->hw_connect;
}

uint32_t furi_hal_version_get_hw_timestamp(void) {
    if(!furi_hal_version_state.otp1_valid) {
        return 0;
    }
    return otp_get_otp1()->hw_timestamp;
}

const char* furi_hal_version_get_name_ptr(void) {
    return furi_hal_version_state.model_code;
}

/*** Device Info from OTP2 ***/

uint32_t furi_hal_version_get_hw_timestamp_qc(void) {
    if(!furi_hal_version_state.otp2_valid) {
        return 0;
    }
    return otp_get_otp2()->hw_timestamp_qc;
}

FuriHalVersionColor furi_hal_version_get_hw_color(void) {
    if(!furi_hal_version_state.otp2_valid) {
        return FuriHalVersionColorUnknown;
    }

    return (FuriHalVersionColor)otp_get_otp2()->hw_color;
}

uint8_t furi_hal_version_get_hw_region(void) {
    if(!furi_hal_version_state.otp2_valid) {
        return 0;
    }
    return otp_get_otp2()->hw_region;
}

/*** OTP3 - Public Key Data ***/

FuriHalVersionKeyCurve furi_hal_version_get_sign_curve(void) {
    if(!furi_hal_version_state.otp3_valid) {
        return FuriHalVersionKeyCurveNone;
    }
    return (FuriHalVersionKeyCurve)otp_get_otp3()->hw_otp3_curve;
}

const uint8_t* furi_hal_version_get_sign_pubkey(void) {
    if(!furi_hal_version_state.otp3_valid) {
        return NULL;
    }
    return otp_get_otp3()->hw_otp3_pkey;
}

size_t furi_hal_version_get_sign_pubkey_size(void) {
    return OTP3_PKEY_SIZE;
}

/*** OTP4 - Signature Data ***/

const uint8_t* furi_hal_version_get_otp_mcu_uid(void) {
    if(!furi_hal_version_state.otp4_valid) {
        return NULL;
    }
    return otp_get_otp4()->hw_otp4_mcu_uid;
}

const uint8_t* furi_hal_version_get_otp1_signature(void) {
    if(!furi_hal_version_state.otp4_valid) {
        return NULL;
    }
    return otp_get_otp4()->hw_otp1_signature;
}

const uint8_t* furi_hal_version_get_otp2_signature(void) {
    if(!furi_hal_version_state.otp4_valid) {
        return NULL;
    }
    return otp_get_otp4()->hw_otp2_signature;
}

size_t furi_hal_version_get_signature_size(void) {
    return OTP4_SIGNATURE_SIZE;
}

/*** Model and Regulatory Info ***/

const char* furi_hal_version_get_model_name(void) {
    return "BUSY Bar";
}

const char* furi_hal_version_get_model_code(void) {
    return furi_hal_version_state.model_code;
}

const char* furi_hal_version_get_fcc_id(void) {
    // TODO: Return proper FCC ID based on model
    return "";
}

const char* furi_hal_version_get_ic_id(void) {
    // TODO: Return proper IC ID based on model
    return "";
}

const char* furi_hal_version_get_mic_id(void) {
    // TODO: Return proper MIC ID based on model
    return "";
}

const char* furi_hal_version_get_srrc_id(void) {
    // TODO: Return proper SRRC ID based on model
    return "";
}

const char* furi_hal_version_get_ncc_id(void) {
    // TODO: Return proper NCC ID based on model
    return "";
}

const uint8_t* furi_hal_version_get_usb_mac(void) {
    return furi_hal_version_state.usb_mac;
}

/*** Hardware UID ***/

size_t furi_hal_version_uid_size(void) {
    return HW_UID_SIZE;
}

const uint8_t* furi_hal_version_uid(void) {
    return furi_hal_version_state.hw_uid;
}

/*** OTP Validity Status ***/

bool furi_hal_version_is_otp_valid(FuriHalFlashOtpBlock block) {
    switch(block) {
    case FuriHalOtpBlockOtp1:
        return furi_hal_version_state.otp1_valid;
    case FuriHalOtpBlockOtp2:
        return furi_hal_version_state.otp2_valid;
    case FuriHalOtpBlockOtp3:
        return furi_hal_version_state.otp3_valid;
    case FuriHalOtpBlockOtp4:
        return furi_hal_version_state.otp4_valid;
    default:
        return false;
    }
}
