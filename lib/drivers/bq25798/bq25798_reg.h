#pragma once

#include <stdint.h>
#include <assert.h>

#include <core/common_defines.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BQ25798_I2C_ADDRESS (0xd6)
#define BQ25798_I2C_TIMEOUT (50)

#define BQ25798_REG00_MINIMAL_SYSTEM_VOLTAGE (0x00u)
#define BQ25798_REG01_CHARGE_VOLTAGE_LIMIT   (0x01u)
#define BQ25798_REG03_CHARGE_CURRENT_LIMIT   (0x03u)
#define BQ25798_REG05_INPUT_VOLTAGE_LIMIT    (0x05u)
#define BQ25798_REG06_INPUT_CURRENT_LIMIT    (0x06u)
#define BQ25798_REG08_PRECHARGE_CONTROL      (0x08u)
#define BQ25798_REG09_TERMINATION_CONTROL    (0x09u)
#define BQ25798_REG0A_RECHARGE_CONTROL       (0x0Au)
#define BQ25798_REG0B_VOTG_REGULATION        (0x0Bu)
#define BQ25798_REG0D_IOTG_REGULATION        (0x0Du)
#define BQ25798_REG0E_TIMER_CONTROL          (0x0Eu)
#define BQ25798_REG0F_CHARGER_CONTROL_0      (0x0Fu)
#define BQ25798_REG10_CHARGER_CONTROL_1      (0x10u)
#define BQ25798_REG11_CHARGER_CONTROL_2      (0x11u)
#define BQ25798_REG12_CHARGER_CONTROL_3      (0x12u)
#define BQ25798_REG13_CHARGER_CONTROL_4      (0x13u)
#define BQ25798_REG14_CHARGER_CONTROL_5      (0x14u)
#define BQ25798_REG15_MPPT_CONTROL           (0x15u)
#define BQ25798_REG16_TEMPERATURE_CONTROL    (0x16u)
#define BQ25798_REG17_NTC_CONTROL_0          (0x17u)
#define BQ25798_REG18_NTC_CONTROL_1          (0x18u)
#define BQ25798_REG19_ICO_CURRENT_LIMIT      (0x19u)
#define BQ25798_REG1B_CHARGER_STATUS_0       (0x1Bu)
#define BQ25798_REG1C_CHARGER_STATUS_1       (0x1Cu)
#define BQ25798_REG1D_CHARGER_STATUS_2       (0x1Du)
#define BQ25798_REG1E_CHARGER_STATUS_3       (0x1Eu)
#define BQ25798_REG1F_CHARGER_STATUS_4       (0x1Fu)
#define BQ25798_REG20_FAULT_STATUS_0         (0x20u)
#define BQ25798_REG21_FAULT_STATUS_1         (0x21u)
#define BQ25798_REG22_CHARGER_FLAG_0         (0x22u)
#define BQ25798_REG23_CHARGER_FLAG_1         (0x23u)
#define BQ25798_REG24_CHARGER_FLAG_2         (0x24u)
#define BQ25798_REG25_CHARGER_FLAG_3         (0x25u)
#define BQ25798_REG26_FAULT_FLAG_0           (0x26u)
#define BQ25798_REG27_FAULT_FLAG_1           (0x27u)
#define BQ25798_REG28_CHARGER_MASK_0         (0x28u)
#define BQ25798_REG29_CHARGER_MASK_1         (0x29u)
#define BQ25798_REG2A_CHARGER_MASK_2         (0x2Au)
#define BQ25798_REG2B_CHARGER_MASK_3         (0x2Bu)
#define BQ25798_REG2C_FAULT_MASK_0           (0x2Cu)
#define BQ25798_REG2D_FAULT_MASK_1           (0x2Du)
#define BQ25798_REG2E_ADC_CONTROL            (0x2Eu)
#define BQ25798_REG2F_ADC_FUNCTION_DISABLE_0 (0x2Fu)
#define BQ25798_REG30_ADC_FUNCTION_DISABLE_1 (0x30u)
#define BQ25798_REG31_IBUS_ADC               (0x31u)
#define BQ25798_REG33_IBAT_ADC               (0x33u)
#define BQ25798_REG35_VBUS_ADC               (0x35u)
#define BQ25798_REG37_VAC1_ADC               (0x37u)
#define BQ25798_REG39_VAC2_ADC               (0x39u)
#define BQ25798_REG3B_VBAT_ADC               (0x3Bu)
#define BQ25798_REG3D_VSYS_ADC               (0x3Du)
#define BQ25798_REG3F_TS_ADC                 (0x3Fu)
#define BQ25798_REG41_TDIE_ADC               (0x41u)
#define BQ25798_REG43_DP_ADC                 (0x43u)
#define BQ25798_REG45_DM_ADC                 (0x45u)
#define BQ25798_REG47_DPDM_DRIVER            (0x47u)
#define BQ25798_REG48_PART_INFORMATION       (0x48u)

typedef enum {
    Bq25798ChargerStatusVbusStatNoInput = 0x0, // No Input or BHOT or BCOLD in OTG mode
    Bq25798ChargerStatusVbusStatSdp = 0x1, // USB SDP (500mA)
    Bq25798ChargerStatusVbusStatCdp = 0x2, // USB CDP (1.5A)
    Bq25798ChargerStatusVbusStatDcp = 0x3, // USB DCP (3.25A)
    Bq25798ChargerStatusVbusStatHVDCP = 0x4, // Adjustable High Voltage DCP (HVDCP) (1.5A)
    Bq25798ChargerStatusVbusStatUnknown = 0x5, //Unknown adaptor (3A)
    Bq25798ChargerStatusVbusStatNonStandard = 0x6, // Non-Standard Adapter (1A/2A/2.1A/2.4A)
    Bq25798ChargerStatusVbusStatOtg = 0x7, // In OTG mode
    Bq25798ChargerStatusVbusStatNotQualified = 0x8, // Not qualified adaptor
    Bq25798ChargerStatusVbusStatVbus = 0xB, // Device directly powered from VBUS
    Bq25798ChargerStatusVbusStatBackup = 0xC, // Backup Mode
} Bq25798ChargerStatusVbusStat;

typedef enum {
    Bq25798ChargerStatusChargeStatNot = 0x0, // Not Charging
    Bq25798ChargerStatusChargeStatTrickle = 0x1, // Trickle Charge
    Bq25798ChargerStatusChargeStatPre = 0x2, // Pre-charge
    Bq25798ChargerStatusChargeStatFast = 0x3, // Fast charge (CC mode)
    Bq25798ChargerStatusChargeStatTaper = 0x4, // Taper Charge (CV mode)
    Bq25798ChargerStatusChargeStatTopOff = 0x6, // Top-off Timer Active Charging
    Bq25798ChargerStatusChargeStatTermination = 0x7, // Charge Termination Done
} Bq25798ChargerStatusChargeStat;

typedef enum {
    Bq25798ChargerStatusIcoDisabled = 0x0, // ICO disabled
    Bq25798ChargerStatusIcoOptimization = 0x1, // ICO optimization in progress
    Bq25798ChargerStatusIcoMaximum = 0x2, // Maximum input current detected
} Bq25798ChargerStatusIco;

typedef struct {
    union {
        struct {
            // STATUS 0
            bool vbus_present                       : 1;
            bool ac1_present                        : 1;
            bool ac2_present                        : 1;
            bool power_good                         : 1;
            uint8_t dummy0                          : 1;
            bool wd                                 : 1;
            bool vindpm_votg                        : 1;
            bool iindpm_iotg                        : 1;
            // STATUS 1
            bool bc1_2_done_stat                    : 1;
            Bq25798ChargerStatusVbusStat vbus_stat  : 4;
            Bq25798ChargerStatusChargeStat chg_stat : 3;
            // STATUS 2
            bool vbat_present_stat                  : 1;
            bool dpdm_stat                          : 1;
            bool treg_stat                          : 1;
            uint8_t dummy1                          : 3;
            Bq25798ChargerStatusIco ico_stat        : 2;
            // STATUS 3

            // STATUS 4
        } FURI_PACKED;
        uint8_t data[5];
    };
} FURI_PACKED Bq25798ChargerStatus;

typedef struct {
    union {
        struct {
            // FAULT 0
            uint8_t vac1_ovp   : 1;
            uint8_t vac2_ovp   : 1;
            uint8_t conv_ocp   : 1;
            uint8_t ibat_ocp   : 1;
            uint8_t ibus_ocp   : 1;
            uint8_t vbat_ovp   : 1;
            uint8_t vbus_ovp   : 1;
            uint8_t ibat_reg   : 1;
            // FAULT 1
            uint8_t            : 2;
            uint8_t therm_shut : 1;
            uint8_t            : 1;
            uint8_t otg_uvp    : 1;
            uint8_t otg_ovp    : 1;
            uint8_t vsys_ovp   : 1;
            uint8_t vsys_short : 1;
        } FURI_PACKED;
        uint8_t data[2];
    };
} FURI_PACKED Bq25798ChargerFault;

typedef struct {
    uint16_t bat_v; // Battery voltage (mV)
    int16_t bat_i; // Battery current (mA)
    uint16_t usb_v; // USB voltage (mV)
    uint16_t usb_i; // USB voltage (mA)
    uint16_t sys_v; // System voltage (mV)
    float temp_bat_pct; // Battery NTC (%)
    float temp_charger; // Charger die temperature (°C)
} Bq25798AdcValues;

typedef enum {
    // Flag 0 reg
    Bq25798ChargerFlagVbusPresent = (1UL << 0),
    Bq25798ChargerFlagAC1Present = (1UL << 1),
    Bq25798ChargerFlagAC2Present = (1UL << 2),
    Bq25798ChargerFlagPowerGood = (1UL << 3),
    Bq25798ChargerFlagPoorSrc = (1UL << 4),
    Bq25798ChargerFlagWd = (1UL << 5),
    Bq25798ChargerFlagVindpmVotg = (1UL << 6),
    Bq25798ChargerFlagIindpmIotg = (1UL << 7),
    // Flag 1 reg
    Bq25798ChargerFlagBC12Done = (1UL << 8),
    Bq25798ChargerFlagVbatPresent = (1UL << 9),
    Bq25798ChargerFlagThermReg = (1UL << 10),
    Bq25798ChargerFlagVbusStatus = (1UL << 12),
    Bq25798ChargerFlagICOStatus = (1UL << 14),
    Bq25798ChargerFlagChargeStatus = (1UL << 15),
    // Flag 2 reg
    Bq25798ChargerFlagTopOffTmr = (1UL << 16),
    Bq25798ChargerFlagPreChgTmr = (1UL << 17),
    Bq25798ChargerFlagTrickleChgTmr = (1UL << 18),
    Bq25798ChargerFlagFastChgTmr = (1UL << 19),
    Bq25798ChargerFlagVsysMinReg = (1UL << 20),
    Bq25798ChargerFlagAdcDone = (1UL << 21),
    Bq25798ChargerFlagDpDmDone = (1UL << 22),
    // Flag 3 reg
    Bq25798ChargerFlagTsHot = (1UL << 24),
    Bq25798ChargerFlagTsWarm = (1UL << 25),
    Bq25798ChargerFlagTsCool = (1UL << 26),
    Bq25798ChargerFlagTsCold = (1UL << 27),
    Bq25798ChargerFlagVbatOtgLow = (1UL << 28),
} Bq25798ChargerFlag;

typedef enum {
    Bq25798PowerIdle = 0, /** Normal mode, power is On */
    Bq25798PowerShutdown = 1, /** Total shutdown (wake up by USB cable only) */
    Bq25798PowerOff = 2, /** Power OFF (Ship mode) */
    Bq25798PowerReset = 3, /** System Power Reset */
} Bq25798PowerSwitch;

typedef struct {
    uint8_t ITERM : 5; /** RW:5h: Termination current */
    uint8_t
        STOP_WD_CHG : 1; /** RW:0h: Defines whether a watchdog timer expiration will disable charging */
    uint8_t REG_RST : 1; /** RW:0h: Reset registers to default values and reset timer */
    uint8_t _RSVD   : 1; /** RO:0h: reserved spaces */
} Bq25798Reg09TerminationControl;

typedef struct {
    uint8_t EN_BACKUP       : 1;
    uint8_t EN_TERM         : 1;
    uint8_t EN_HIZ          : 1;
    uint8_t FORCE_ICO       : 1;
    uint8_t EN_ICO          : 1;
    uint8_t EN_CHG          : 1;
    uint8_t FORCE_IBATDIS   : 1;
    uint8_t EN_AUTO_IBATDIS : 1;
} Bq25798Reg0FChargerControl0;

typedef struct {
    uint8_t WATCHDOG    : 3;
    uint8_t WD_RST      : 1;
    uint8_t VAC_OVP     : 2;
    uint8_t VBUS_BACKUP : 2;
} Bq25798Reg10ChargerControl1;

typedef struct {
    uint8_t SDRV_DLY      : 1;
    uint8_t SDRV_CTRL     : 2;
    uint8_t HVDCP_EN      : 1;
    uint8_t EN_9V         : 1;
    uint8_t EN_12V        : 1;
    uint8_t AUTO_INDET_EN : 1;
    uint8_t FORCE_INDET   : 1;
} Bq25798Reg11ChargerControl2;

typedef struct {
    uint8_t DIS_FWD_OOA : 1;
    uint8_t DIS_OTG_OOA : 1;
    uint8_t DIS_LDO     : 1;
    uint8_t WKUP_DLY    : 1;
    uint8_t PFM_FWD_DIS : 1;
    uint8_t PFM_OTG_DIS : 1;
    uint8_t EN_OTG      : 1;
    uint8_t DIS_ACDRV   : 1;
} Bq25798Reg12ChargerControl3;

typedef struct {
    uint8_t EN_BATOC     : 1;
    uint8_t EN_EXTILIM   : 1;
    uint8_t EN_IINDPM    : 1;
    uint8_t IBAT_REG     : 2;
    uint8_t EN_IBAT      : 1;
    uint8_t              : 1;
    uint8_t SFET_PRESENT : 1;
} Bq25798Reg14ChargerControl5;

typedef struct {
    uint8_t              : 2;
    uint8_t ADC_AVG_INIT : 1;
    uint8_t ADC_AVG      : 1;
    uint8_t ADC_SAMPLE   : 2;
    uint8_t ADC_RATE     : 1;
    uint8_t ADC_EN       : 1;
} Bq25798Reg2EADCControl;

typedef struct {
    uint8_t DEV_REV : 3; /** RO:0b001: Device revision */
    uint8_t PN      : 3; /** RO:0b011: Part Number */
    uint8_t _RSVD   : 2; /** RO:0b0: Reserved */
} Bq25798Reg48PartInformation;

static_assert(sizeof(Bq25798ChargerStatus) == 5, "Bq25798ChargerStatus size mismatch");
static_assert(sizeof(Bq25798ChargerFault) == 2, "Bq25798ChargerFault size mismatch");
static_assert(
    sizeof(Bq25798Reg09TerminationControl) == 1,
    "Bq25798Reg09TerminationControl size mismatch");
static_assert(
    sizeof(Bq25798Reg0FChargerControl0) == 1,
    "Bq25798Reg0FChargerControl0 size mismatch");
static_assert(
    sizeof(Bq25798Reg10ChargerControl1) == 1,
    "Bq25798Reg10ChargerControl1 size mismatch");
static_assert(
    sizeof(Bq25798Reg11ChargerControl2) == 1,
    "Bq25798Reg11ChargerControl2 size mismatch");
static_assert(
    sizeof(Bq25798Reg12ChargerControl3) == 1,
    "Bq25798Reg12ChargerControl3 size mismatch");
static_assert(
    sizeof(Bq25798Reg14ChargerControl5) == 1,
    "Bq25798Reg14ChargerControl5 size mismatch");
static_assert(sizeof(Bq25798Reg2EADCControl) == 1, "Bq25798Reg2EADCControl size mismatch");
static_assert(
    sizeof(Bq25798Reg48PartInformation) == 1,
    "Bq25798Reg48PartInformation size mismatch");

#ifdef __cplusplus
}
#endif
