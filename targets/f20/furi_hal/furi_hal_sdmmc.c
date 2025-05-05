#include <core/log.h>
#include <furi_hal_bus.h>
#include <furi_hal_cortex.h>
#include <furi_hal_clock.h>
#include <furi_hal_interrupt.h>
#include <furi_hal_sdmmc.h>
#include <furi_hal_resources.h>
#include <stm32u5xx_ll_rcc.h>

// #include <stm32u5xx_hal_conf.h> // TODO
#include <stm32u5xx_ll_sdmmc.h> // FIXME

#define TAG                      "FuriHalSDMMC"
#define FURI_SDMMC_SWDATATIMEOUT ((uint32_t)1000U)
#define FURI_SDMMC_BLOCK         SDMMC1
#define FURI_SDMMC_BUS           FuriHalBusSDMMC1
#define FURI_SDMMC_IRQ           SDMMC1_IRQn
#define FURI_SDMMC_PIN_ALTFN     GpioAltFn12SDMMC1

#define SD_INIT_FREQ         400000U /* Initialization phase : 400 kHz max */
#define SD_NORMAL_SPEED_FREQ 25000000U /* Normal speed phase : 25 MHz max */
#define SD_HIGH_SPEED_FREQ   50000000U /* High speed phase : 50 MHz max */
#define SD_BLOCKSIZE         ((uint32_t)512U) /*!< Block size is 512 bytes */

#define MMC_BLOCKSIZE             ((uint32_t)512U) /*!< Block size is 512 bytes */
#define MMC_HIGH_VOLTAGE_RANGE    0x80FF8000U /*!< High voltage in byte mode    */
#define MMC_DUAL_VOLTAGE_RANGE    0x80FF8080U /*!< Dual voltage in byte mode    */
#define MMC_LOW_VOLTAGE_RANGE     0x80000080U /*!< Low voltage in byte mode     */
#define EMMC_HIGH_VOLTAGE_RANGE   0xC0FF8000U /*!< High voltage in sector mode  */
#define EMMC_DUAL_VOLTAGE_RANGE   0xC0FF8080U /*!< Dual voltage in sector mode  */
#define EMMC_LOW_VOLTAGE_RANGE    0xC0000080U /*!< Low voltage in sector mode   */
#define MMC_INVALID_VOLTAGE_RANGE 0x0001FF01U
#define MMC_HIGH_SPEED_FREQ       52000000U /* High speed phase : 52 MHz max */

#define MMC_EXT_CSD_PWR_CL_26_INDEX     201
#define MMC_EXT_CSD_PWR_CL_52_INDEX     200
#define MMC_EXT_CSD_PWR_CL_DDR_52_INDEX 238

#define MMC_EXT_CSD_PWR_CL_26_POS     8
#define MMC_EXT_CSD_PWR_CL_52_POS     0
#define MMC_EXT_CSD_PWR_CL_DDR_52_POS 16

#define SDMMC_INIT_CLOCK_EDGE            (SDMMC_CLOCK_EDGE_RISING)
#define SDMMC_INIT_CLOCK_POWER_SAVE      (SDMMC_CLOCK_POWER_SAVE_DISABLE)
#define SDMMC_INIT_HARDWARE_FLOW_CONTROL (SDMMC_HARDWARE_FLOW_CONTROL_ENABLE)

#define SDMMC_CLOCK_POWER_SAVE (SDMMC_CLOCK_POWER_SAVE_ENABLE)

#define SDMMC_REAL_DATATIMEOUT (FURI_SDMMC_SWDATATIMEOUT * 5000U)

typedef enum {
    FuriHalSdErrorNone = SDMMC_ERROR_NONE,
    FuriHalSdErrorDataCrcFail = SDMMC_ERROR_DATA_CRC_FAIL,
    FuriHalSdErrorDataTimeout = SDMMC_ERROR_DATA_TIMEOUT,
    FuriHalSdErrorTxUnderrun = SDMMC_ERROR_TX_UNDERRUN,
    FuriHalSdErrorRxOverrun = SDMMC_ERROR_RX_OVERRUN,
    FuriHalSdErrorAddrMisaligned = SDMMC_ERROR_ADDR_MISALIGNED,
    FuriHalSdErrorBlockLenErr = SDMMC_ERROR_BLOCK_LEN_ERR,
    FuriHalSdErrorEraseSeqErr = SDMMC_ERROR_ERASE_SEQ_ERR,
    FuriHalSdErrorBadEraseParam = SDMMC_ERROR_BAD_ERASE_PARAM,
    FuriHalSdErrorWriteProtViolation = SDMMC_ERROR_WRITE_PROT_VIOLATION,
    FuriHalSdErrorLockUnlockFailed = SDMMC_ERROR_LOCK_UNLOCK_FAILED,
    FuriHalSdErrorComCrcFailed = SDMMC_ERROR_COM_CRC_FAILED,
    FuriHalSdErrorIllegalCmd = SDMMC_ERROR_ILLEGAL_CMD,
    FuriHalSdErrorCardEccFailed = SDMMC_ERROR_CARD_ECC_FAILED,
    FuriHalSdErrorCcErr = SDMMC_ERROR_CC_ERR,
    FuriHalSdErrorGeneralUnknownErr = SDMMC_ERROR_GENERAL_UNKNOWN_ERR,
    FuriHalSdErrorStreamReadUnderrun = SDMMC_ERROR_STREAM_READ_UNDERRUN,
    FuriHalSdErrorStreamWriteOverrun = SDMMC_ERROR_STREAM_WRITE_OVERRUN,
    FuriHalSdErrorCidCsdOverwrite = SDMMC_ERROR_CID_CSD_OVERWRITE,
    FuriHalSdErrorWpEraseSkip = SDMMC_ERROR_WP_ERASE_SKIP,
    FuriHalSdErrorCardEccDisabled = SDMMC_ERROR_CARD_ECC_DISABLED,
    FuriHalSdErrorEraseReset = SDMMC_ERROR_ERASE_RESET,
    FuriHalSdErrorAkeSeqErr = SDMMC_ERROR_AKE_SEQ_ERR,
    FuriHalSdErrorInvalidVoltRange = SDMMC_ERROR_INVALID_VOLTRANGE,
    FuriHalSdErrorAddrOutOfRange = SDMMC_ERROR_ADDR_OUT_OF_RANGE,
    FuriHalSdErrorRequestNotApplicable = SDMMC_ERROR_REQUEST_NOT_APPLICABLE,
    FuriHalSdErrorParam = SDMMC_ERROR_INVALID_PARAMETER,
    FuriHalSdErrorUnsupportedFeature = SDMMC_ERROR_UNSUPPORTED_FEATURE,
    FuriHalSdErrorBusy = SDMMC_ERROR_BUSY,
    FuriHalSdErrorDma = SDMMC_ERROR_DMA,
    FuriHalSdErrorTimeout = SDMMC_ERROR_TIMEOUT,
} FuriHalSdError;

typedef struct {
    uint8_t CSDStruct; /*!< CSD structure */
    uint8_t SysSpecVersion; /*!< System specification version */
    uint8_t Reserved1; /*!< Reserved */
    uint8_t TAAC; /*!< Data read access time 1 */
    uint8_t NSAC; /*!< Data read access time 2 in CLK cycles */
    uint8_t MaxBusClkFrec; /*!< Max. bus clock frequency */
    uint16_t CardComdClasses; /*!< Card command classes */
    uint8_t RdBlockLen; /*!< Max. read data block length */
    uint8_t PartBlockRead; /*!< Partial blocks for read allowed */
    uint8_t WrBlockMisalign; /*!< Write block misalignment */
    uint8_t RdBlockMisalign; /*!< Read block misalignment */
    uint8_t DSRImpl; /*!< DSR implemented */
    uint8_t Reserved2; /*!< Reserved */
    uint32_t DeviceSize; /*!< Device Size */
    uint8_t MaxRdCurrentVDDMin; /*!< Max. read current @ VDD min */
    uint8_t MaxRdCurrentVDDMax; /*!< Max. read current @ VDD max */
    uint8_t MaxWrCurrentVDDMin; /*!< Max. write current @ VDD min */
    uint8_t MaxWrCurrentVDDMax; /*!< Max. write current @ VDD max */
    uint8_t DeviceSizeMul; /*!< Device size multiplier */
    uint8_t EraseGrSize; /*!< Erase group size */
    uint8_t EraseGrMul; /*!< Erase group size multiplier */
    uint8_t WrProtectGrSize; /*!< Write protect group size */
    uint8_t WrProtectGrEnable; /*!< Write protect group enable */
    uint8_t ManDeflECC; /*!< Manufacturer default ECC */
    uint8_t WrSpeedFact; /*!< Write speed factor */
    uint8_t MaxWrBlockLen; /*!< Max. write data block length */
    uint8_t WriteBlockPaPartial; /*!< Partial blocks for write allowed */
    uint8_t Reserved3; /*!< Reserved */
    uint8_t ContentProtectAppli; /*!< Content protection application */
    uint8_t FileFormatGroup; /*!< File format group */
    uint8_t CopyFlag; /*!< Copy flag (OTP */
    uint8_t PermWrProtect; /*!< Permanent write protection */
    uint8_t TempWrProtect; /*!< Temporary write protection */
    uint8_t FileFormat; /*!< File format */
    uint8_t ECC; /*!< ECC code */
    uint8_t CSD_CRC; /*!< CSD CRC */
    uint8_t Reserved4; /*!< Always 1 */
} CardCSDInfo;
_Static_assert(sizeof(CardCSDInfo) == sizeof(CardCSDInfo), "Size check for 'CardCSDInfo' failed.");

/*
    R:          Read only. 
    W:          One time programmable and not readable. 
    R/W:        One time programmable and readable. 
    W/E:        Multiple writable with value kept after power failure, H/W reset assertion and any CMD0 reset and not readable. 
    R/W/E:      Multiple writable with value kept after power failure, H/W reset assertion and any CMD0 reset and readable. 
    R/W/C_P:    Writable after value cleared by power failure and HW/rest assertion (the value not cleared by CMD0 reset) and readable. 
    R/W/E_P:    Multiple writable with value reset after power failure,  H/W reset assertion and any CMD0 reset and readable. 
    W/E_P:      Multiple writable with value reset after power failure,  H/W reset assertion and any CMD0 reset and not readable. 
*/
typedef struct {
    uint8_t Reserved24[32]; /*!< Reserved [TBD][31:0]*/
    uint8_t FlushCache; /*!< Flushing of the cache [W/E_P][32]*/
    uint8_t CacheCtrl; /*!< Control to turn the Cache ON/OFF [R/W/E_P][33]*/
    uint8_t PowerOffNotification; /*!< Power Off Notification [R/W/E_P][34]*/
    uint8_t PackedFailureIndex; /*!< Packed command failure index [R][35]*/
    uint8_t PackedCommandStatus; /*!< Packed command status [R][36]*/
    uint8_t ContextConf[15]; /*!< Context configuration [R/W/E_P][51:37]*/
    uint16_t ExtPartitionsAttribute; /*!< Extended Partitions Attribute [R/W][53:52]*/
    uint16_t ExceptionEventsStatus; /*!< Exception events status [R][55:54]*/
    uint16_t ExceptionEventsCtrl; /*!< Exception events control [R/W/E_P][57:56]*/
    uint8_t DyncapNeeded; /*!< Number of addressed group to be released [D][58]*/
    uint8_t Class6Ctrl; /*!< Class 6 commands control [R/W/E_P][59]*/
    uint8_t IniTimeoutEmu; /*!< 1st initialization after disabling sector size emulation [R][60]*/
    uint8_t DataSectorSize; /*!< Sector size [R][61]*/
    uint8_t UseNativeSector; /*!< Sector size emulation [R/W][62]*/
    uint8_t NativeSectorSize; /*!< Native sector size [R][63]*/
    uint8_t VendorSpecificField[64]; /*!< Vendor Specific Fields <vendor specific> [127:64]*/
    uint8_t Reserved23[2]; /*!< Reserved [TBD][129:128]*/
    uint8_t ProgramCidCsdDDRSupport; /*!< Program CID/CSD in DDR mode support [R][130]*/
    uint8_t PeriodicWakeup; /*!< Periodic Wake-up [R/W/E][131]*/
    uint8_t TCaseSupport; /*!< Package Case Temperature is controlled [W/E_P][132]*/
    uint8_t Reserved22[1]; /*!< Reserved [TBD][133]*/
    uint8_t SecBadBlkMgmnt; /*!< Bad Block Management mode [R/W][134]*/
    uint8_t Reserved21[1]; /*!< Reserved [TBD][135]*/
    uint8_t EnhStartAddr[4]; /*!< Enhanced User Data Start Address [R/W][139:136]*/
    uint8_t EnhSizeMult[3]; /*!< Enhanced User Data Area Size [R/W][142:140]*/
    uint8_t GPSizeMult[12]; /*!< General Purpose Partition Size [R/W][154:143]*/
    uint8_t PartitionSettingCompleted; /*!< Partitioning Setting [R/W][155]*/
    uint8_t PartitionsAttribute; /*!< Partitions attribute [R/W][156]*/
    uint8_t MaxEnhSizeMult[3]; /*!< Max Enhanced Area Size [R][159:157]*/
    uint8_t PartitioningSupport; /*!< Partitioning Support [R][160]*/
    uint8_t HPIMgmt; /*!< HPI management [R/W/E_P][161]*/
    uint8_t RSTnFunction; /*!< H/W reset function [R/W][162]*/
    uint8_t BKOPSEn; /*!< Enable background operations handshake [R/W][163]*/
    uint8_t BKOPSStart; /*!< Manually start background operations [W/E_P][164]*/
    uint8_t SanitizeStart; /*!< Start Sanitize operation [W/E_P][165]*/
    uint8_t WrRelParam; /*!< Write reliability parameter register [R][166]*/
    uint8_t WrRelSet; /*!< Write reliability setting register [R/W][167]*/
    uint8_t RpmbSizeMult; /*!< RPMB Size [R][168]*/
    uint8_t FWConfig; /*!< FW configuration [R/W][169]*/
    uint8_t Reserved20; /*!< Reserved [TBD][170]*/
    uint8_t UserWp; /*!< User area write protection register [R/W, R/W/C_P & R/W/E_P][171]*/
    uint8_t Reserved19; /*!< Reserved [TBD][172]*/
    uint8_t BootWp; /*!< Boot area write protection register [R/W & R/W/C_P][173]*/
    uint8_t BootWpStatus; /*!< Boot write protection status registers [R][174]*/
    uint8_t EraseGroupDef; /*!< High-density erase group definition [R/W/E_P][175]*/
    uint8_t Reserved18; /*!< Reserved [TBD][176]*/
    uint8_t BootBusConditions; /*!< Boot bus conditions [R/W/E][177]*/
    uint8_t BootConfigProt; /*!< Boot config protection [R/W & R/W/C_P][178]*/
    uint8_t PartitionConfig; /*!< Partition configuration [R/W/E & R/W/E_P][179]*/
    uint8_t Reserved17; /*!< Reserved [TBD][180]*/
    uint8_t ErasedMemCont; /*!< Erased memory content [R][181]*/
    uint8_t Reserved16; /*!< Reserved [TBD][182]*/
    uint8_t BusWidth; /*!< Bus width mode [R/W/E_P][183]*/
    uint8_t Reserved15; /*!< Reserved [TBD][184]*/
    uint8_t HSTiming; /*!< High-speed interface timing [R/W/E_P][185]*/
    uint8_t Reserved14; /*!< Reserved [TBD][186]*/
    uint8_t PowerClass; /*!< Power class [R/W/E_P][187]*/
    uint8_t Reserved13; /*!< Reserved [TBD][188]*/
    uint8_t CmdSetRev; /*!< Command set revision [R][189]*/
    uint8_t Reserved12; /*!< Reserved [TBD][190]*/
    uint8_t CmdSet; /*!< Command set [R/W/E_P][191]*/
    uint8_t ExtCSDRev; /*!< Extended CSD revision [R][192]*/
    uint8_t Reserved11; /*!< Reserved [TBD][193]*/
    uint8_t CSDStructVer; /*!< CSD structure version [R][194]*/
    uint8_t Reserved10; /*!< Reserved [TBD][195]*/
    uint8_t DeviceType; /*!< Device type [R][196]*/
    uint8_t DriverStrength; /*!< I/O Driver Strength [R][197]*/
    uint8_t OutOfInterruptTime; /*!< Out-of-interrupt busy timing [R][198]*/
    uint8_t PartitionSwitchTime; /*!< Partition switching timing [R][199]*/
    uint8_t PwrCl52_195; /*!< Power class for 52MHz at 1.95V [R][200]*/
    uint8_t PwrCl26_195; /*!< Power class for 26MHz at 1.95V [R][201]*/
    uint8_t PwrCl52_360; /*!< Power class for 52MHz at 3.6V [R][202]*/
    uint8_t PwrCl26_360; /*!< Power class for 26MHz at 3.6V [R][203]*/
    uint8_t Reserved9; /*!< Reserved [TBD][204]*/
    uint8_t MinPerfR_4_26; /*!< Minimum read performance for 4bit at 26MHz [R][205]*/
    uint8_t MinPerfW_4_26; /*!< Minimum write performance for 4bit at 26MHz [R][206]*/
    uint8_t
        MinPerfR_8_26_4_52; /*!< Minimum read performance for 8bit at 26MHz, for 4bit at 52MHz [R][207]*/
    uint8_t
        MinPerfW_8_26_4_52; /*!< Minimum write performance for 8bit at 26MHz, for 4bit at 52MHz [R][208]*/
    uint8_t MinPerfR_8_52; /*!< Minimum read performance for 8bit at 52MHz [R][209]*/
    uint8_t MinPerfW_8_52; /*!< Minimum write performance for 8bit at 52MHz [R][210]*/
    uint8_t Reserved8; /*!< Reserved [TBD][211]*/
    uint32_t SecCount; /*!< Sector Count [R][215:212]*/
    uint8_t Reserved7; /*!< Reserved [TBD][216]*/
    uint8_t SATimeout; /*!< Sleep/awake timeout [R][217]*/
    uint8_t Reserved6; /*!< Reserved [TBD][218]*/
    uint8_t SCVccQ; /*!< Sleep current (VCCQ) [R][219]*/
    uint8_t SCVcc; /*!< Sleep current (VCC) [R][220]*/
    uint8_t HCWriteProtectGrpSize; /*!< High-capacity write protect group size [R][221]*/
    uint8_t RelWrSecC; /*!< Reliable write sector count [R][222]*/
    uint8_t EraseTimeoutMult; /*!< High-capacity erase timeout [R][223]*/
    uint8_t HCEraseGrpSize; /*!< High-capacity erase unit size [R][224]*/
    uint8_t AccSize; /*!< Access size [R][225]*/
    uint8_t BootSizeMulti; /*!< Boot partition size [R][226]*/
    uint8_t Reserved5; /*!< Reserved [TBD][227]*/
    uint8_t BootInfo; /*!< Boot information [R][228]*/
    uint8_t SecTrimMult; /*!< Secure TRIM Multiplier [R][229]*/
    uint8_t SecEraseMult; /*!< Secure Erase Multiplier [R][230]*/
    uint8_t SecFeatureSupport; /*!< Secure feature support [R][231]*/
    uint8_t TrimMult; /*!< TRIM Multiplier [R][232]*/
    uint8_t Reserved4; /*!< Reserved [TBD][233]*/
    uint8_t MinPerfDdrR_8_52; /*!< Minimum read performance for 8bit at 52MHz in DDR mode [R][234]*/
    uint8_t MinPerfDdrW_8_52; /*!< Minimum write performance for 8bit at 52MHz in DDR mode [R][235]*/
    uint8_t PwrCl200_130; /*!< Power class for 200MHz at 1.3V  [R][236]*/
    uint8_t PwrCl200_195; /*!< Power class for 200MHz at 1.95V [R][237]*/
    uint8_t PwrClDdr52_195; /*!< Power class for 52MHz, DDR at 1.95V [R][238]*/
    uint8_t PwrClDdr52_360; /*!< Power class for 52MHz, DDR at 3.6V [R][239]*/
    uint8_t Reserved3; /*!< Reserved [TBD][240]*/
    uint8_t InitTimeAfterPart; /*!< 1st initialization time after partitioning [R][241]*/
    uint32_t CorrectlyPrgSectorsNum; /*!< Number of correctly programmed sectors [R][245:242]*/
    uint8_t BKOPSStatus; /*!< Background operations status [R][246]*/
    uint8_t PowerOffLongTime; /*!< Power off notification (long) timeout [R][247]*/
    uint8_t GenericCmd6Time; /*!< Generic CMD6 timeout [R][248]*/
    uint32_t CacheSize; /*!< Cache size [R][252:249]*/
    uint8_t Reserved2[241]; /*!< Reserved [TBD][493:253]*/
    uint8_t ExtSupport; /*!< Extended partitions attribute support [R][494]*/
    uint8_t LargeUnitSizeM1; /*!< Large unit size [R][495]*/
    uint8_t ContextCapabilities; /*!< Context management capabilities [R][496]*/
    uint8_t TagResourcesSize; /*!< Tag resources size [R][497]*/
    uint8_t TagUnitSize; /*!< Tag unit size [R][498]*/
    uint8_t DataTagSupport; /*!< Data tag support [R][499]*/
    uint8_t MaxPackedWrites; /*!< Max packed write commands [R][500]*/
    uint8_t MaxPackedReads; /*!< Max packed read commands [R][501]*/
    uint8_t BKOPSSupport; /*!< Background operations support [R][502]*/
    uint8_t HPIFeatures; /*!< HPI features [R][503]*/
    uint8_t SCmdSet; /*!< Supported command sets [R][504]*/
    uint8_t ExtSecurityErr; /*!< Extended security commands error [R][505]*/
    uint8_t Reserved1[6]; /*!< Reserved [TBD][511:506]*/
} FURI_PACKED CardExtendedCSDRegister;
_Static_assert(
    sizeof(CardExtendedCSDRegister) == 512,
    "Size check for 'CardExtendedCSDRegister' failed.");

typedef struct {
    uint8_t DataBusWidth; /*!< Shows the currently defined data bus width */
    uint8_t SecuredMode; /*!< Card is in secured mode of operation */
    uint16_t CardType; /*!< Carries information about card type */
    uint32_t ProtectedAreaSize; /*!< Carries information about the capacity of protected area */
    uint8_t SpeedClass; /*!< Carries information about the speed class of the card */
    uint8_t PerformanceMove; /*!< Carries information about the card's performance move */
    uint8_t AllocationUnitSize; /*!< Carries information about the card's allocation unit size */
    uint16_t EraseSize; /*!< Determines the number of AUs to be erased in one operation */
    uint8_t EraseTimeout; /*!< Determines the timeout for any number of AU erase */
    uint8_t EraseOffset; /*!< Carries information about the erase offset */
    uint8_t UhsSpeedGrade; /*!< Carries information about the speed grade of UHS card */
    uint8_t
        UhsAllocationUnitSize; /*!< Carries information about the UHS card's allocation unit size */
    uint8_t VideoSpeedClass; /*!< Carries information about the Video Speed Class of UHS card */
} CardStatus;

typedef struct {
    FuriHalSdMmcPresentCallback present_callback;
    void* context;

    CardCSDInfo csd;
    CardStatus status;

    FuriHalSdInfo info;
    uint32_t card_rca;
    bool card_alive;

    CardExtendedCSDRegister csd_ext;
} SdMmc;

static SdMmc sdmmc1 = {0};

typedef enum {
    SdMmcDmaStateEnabled = 1 << 0,
    SdMmcDmaStateRxMulti = 1 << 1,
    SdMmcDmaStateRxSingle = 1 << 2,
    SdMmcDmaStateTxMulti = 1 << 3,
    SdMmcDmaStateTxSingle = 1 << 4,
} SdMmcDmaState;

typedef enum {
    SdMmcDmaEventComplete = 1 << 0,
    SdMmcDmaEventError = 1 << 1,
} SdMmcDmaEvent;

typedef struct {
    uint8_t* rx_buffer;
    size_t rx_size;
    const uint8_t* tx_buffer;
    size_t tx_size;

    volatile uint32_t state;
    volatile FuriHalSdError error;
    FuriEventFlag* event;
} SdMmcDmaContext;

static SdMmcDmaContext sdmmc_dma_context = {0};

static void furi_hal_sdmmc_gpio_init(void) {
    // furi_hal_gpio_write(&gpio_sd_card_power_switch, 0);
    // furi_hal_gpio_init(
    //     &gpio_sd_card_power_switch, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    // furi_hal_gpio_init(&gpio_sd_card_detect, GpioModeInterruptRiseFall, GpioPullNo, GpioSpeedLow);
}

bool furi_hal_sdmmc_is_sd_present(void) {
    bool sd_present = true; //(furi_hal_gpio_read(&gpio_sd_card_detect) == 0);
    return sd_present;
}

void furi_hal_sdmmc_init(void) {
    furi_hal_sdmmc_gpio_init();
    sdmmc_dma_context.event = furi_event_flag_alloc();
    sdmmc1.card_alive = false;

    FURI_LOG_I(TAG, "Init OK");
}

// static void furi_hal_sdmmc_present_callback(void* context) {
//     SdMmc sdmmc = *(SdMmc*)context;
//     if(sdmmc.present_callback) {
//         sdmmc.present_callback(sdmmc.context);
//     }
// }

void furi_hal_sdmmc_set_presence_callback(FuriHalSdMmcPresentCallback callback, void* context) {
    sdmmc1.present_callback = callback;
    sdmmc1.context = context;

    // if(sdmmc1.present_callback) {
    //     furi_hal_gpio_add_int_callback(
    //         &gpio_sd_card_detect, furi_hal_sdmmc_present_callback, &sdmmc1);
    // } else {
    //     furi_hal_gpio_remove_int_callback(&gpio_sd_card_detect);
    // }
}

static void furi_hal_sdmmc_periph_init(void) {
    GpioSpeed speed = GpioSpeedMedium;

    furi_hal_gpio_init_ex(
        &gpio_sd_card_d0, GpioModeAltFunctionPushPull, GpioPullNo, speed, FURI_SDMMC_PIN_ALTFN);
    furi_hal_gpio_init_ex(
        &gpio_sd_card_d1, GpioModeAltFunctionPushPull, GpioPullNo, speed, FURI_SDMMC_PIN_ALTFN);
    furi_hal_gpio_init_ex(
        &gpio_sd_card_d2, GpioModeAltFunctionPushPull, GpioPullNo, speed, FURI_SDMMC_PIN_ALTFN);
    furi_hal_gpio_init_ex(
        &gpio_sd_card_d3, GpioModeAltFunctionPushPull, GpioPullNo, speed, FURI_SDMMC_PIN_ALTFN);
    furi_hal_gpio_init_ex(
        &gpio_sd_card_ck, GpioModeAltFunctionPushPull, GpioPullNo, speed, FURI_SDMMC_PIN_ALTFN);
    furi_hal_gpio_init_ex(
        &gpio_sd_card_cmd, GpioModeAltFunctionPushPull, GpioPullNo, speed, FURI_SDMMC_PIN_ALTFN);

    LL_RCC_SetSDMMCKernelClockSource(LL_RCC_SDMMC12_KERNELCLKSOURCE_PLL1);

    furi_hal_bus_enable(FURI_SDMMC_BUS);
}

static void furi_hal_sdmmc_periph_deinit(void) {
    furi_hal_bus_disable(FURI_SDMMC_BUS);
    furi_hal_gpio_init_simple(&gpio_sd_card_d0, GpioModeAnalog);
    furi_hal_gpio_init_simple(&gpio_sd_card_d1, GpioModeAnalog);
    furi_hal_gpio_init_simple(&gpio_sd_card_d2, GpioModeAnalog);
    furi_hal_gpio_init_simple(&gpio_sd_card_d3, GpioModeAnalog);
    furi_hal_gpio_init_simple(&gpio_sd_card_ck, GpioModeAnalog);
    furi_hal_gpio_init_simple(&gpio_sd_card_cmd, GpioModeAnalog);
}

static void furi_hal_sdmmc_card_enable_power(void) {
    // furi_hal_gpio_write(&gpio_sd_card_power_switch, 1);
    // we need about 1.2ms to stabilize the power
    // furi_delay_ms(2);

    // wait some time after reset to correctly mount the card
    // TODO: why?
    furi_delay_ms(35);
}

static void furi_hal_sdmmc_card_disable_power(void) {
    // furi_hal_gpio_write(&gpio_sd_card_power_switch, 0);
}

static uint32_t sdmmc_power_on(void) {
    uint32_t errorstate;

    /* CMD0: GO_IDLE_STATE */
    errorstate = SDMMC_CmdGoIdleState(FURI_SDMMC_BLOCK);
    if(errorstate != FuriHalSdErrorNone) {
        return errorstate;
    }

    /* CMD8: SEND_IF_COND: Command available only on V2.0 cards */
    errorstate = SDMMC_CmdOperCond(FURI_SDMMC_BLOCK);
    if(errorstate == SDMMC_ERROR_CMD_RSP_TIMEOUT) /* No response to CMD8 */
    {
        FURI_LOG_D(TAG, "No response to CMD8, assume SD card v1.x");
        sdmmc1.info.version = FuriHalSdVersion1;

        /* CMD0: GO_IDLE_STATE */
        errorstate = SDMMC_CmdGoIdleState(FURI_SDMMC_BLOCK);
        if(errorstate != FuriHalSdErrorNone) {
            return errorstate;
        }
    } else {
        FURI_LOG_D(TAG, "Response to CMD8, assume SD card v2.x");
        sdmmc1.info.version = FuriHalSdVersion2;

        /* SEND CMD55 APP_CMD with RCA as 0 */
        errorstate = SDMMC_CmdAppCommand(FURI_SDMMC_BLOCK, 0);
        if(errorstate != FuriHalSdErrorNone) {
            return FuriHalSdErrorUnsupportedFeature;
        }
    }

    uint32_t count = 0U;
    uint32_t response = 0U;

    /* SD CARD */
    /* Send ACMD41 SD_APP_OP_COND with Argument 0x80100000 */
    while((count < SDMMC_MAX_VOLT_TRIAL) && (!((response >> 31U) == 1U))) {
        /* SEND CMD55 APP_CMD with RCA as 0 */
        errorstate = SDMMC_CmdAppCommand(FURI_SDMMC_BLOCK, 0);
        if(errorstate != FuriHalSdErrorNone) {
            return errorstate;
        }

        /* Send CMD41 */
        errorstate = SDMMC_CmdAppOperCommand(
            FURI_SDMMC_BLOCK,
            SDMMC_VOLTAGE_WINDOW_SD | SDMMC_HIGH_CAPACITY | SD_SWITCH_1_8V_CAPACITY);
        if(errorstate != FuriHalSdErrorNone) {
            return FuriHalSdErrorUnsupportedFeature;
        }

        /* Get command response */
        response = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP1);

        count++;
    }

    if(count >= SDMMC_MAX_VOLT_TRIAL) {
        return FuriHalSdErrorInvalidVoltRange;
    }

    /* Check card type */
    sdmmc1.info.type = FuriHalSdTypeSC;
    if((response & SDMMC_HIGH_CAPACITY) == SDMMC_HIGH_CAPACITY) {
        sdmmc1.info.type = FuriHalSdTypeHCXC;
    }

    return FuriHalSdErrorNone;
}

static inline uint32_t sdmmc_get_flags(uint32_t mask) {
    return __SDMMC_GET_FLAG(FURI_SDMMC_BLOCK, mask);
}

static inline void sdmmc_clear_flags(uint32_t flags) {
    __SDMMC_CLEAR_FLAG(FURI_SDMMC_BLOCK, flags);
}

static inline void sdmmc_clear_static_flags(void) {
    sdmmc_clear_flags(SDMMC_STATIC_FLAGS);
}

static inline void sdmmc_clear_static_data_flags(void) {
    sdmmc_clear_flags(SDMMC_STATIC_DATA_FLAGS);
}

static inline void sdmmc_disable_it(uint32_t it) {
    __SDMMC_DISABLE_IT(FURI_SDMMC_BLOCK, it);
}

static inline void sdmmc_enable_it(uint32_t it) {
    __SDMMC_ENABLE_IT(FURI_SDMMC_BLOCK, it);
}

static bool sdmmc_parse_csd(CardCSDInfo* info, uint32_t csd[4]) {
    info->CSDStruct = (uint8_t)((csd[0] & 0xC0000000U) >> 30U);
    info->SysSpecVersion = (uint8_t)((csd[0] & 0x3C000000U) >> 26U);
    info->Reserved1 = (uint8_t)((csd[0] & 0x03000000U) >> 24U);
    info->TAAC = (uint8_t)((csd[0] & 0x00FF0000U) >> 16U);
    info->NSAC = (uint8_t)((csd[0] & 0x0000FF00U) >> 8U);
    info->MaxBusClkFrec = (uint8_t)(csd[0] & 0x000000FFU);
    info->CardComdClasses = (uint16_t)((csd[1] & 0xFFF00000U) >> 20U);
    info->RdBlockLen = (uint8_t)((csd[1] & 0x000F0000U) >> 16U);
    info->PartBlockRead = (uint8_t)((csd[1] & 0x00008000U) >> 15U);
    info->WrBlockMisalign = (uint8_t)((csd[1] & 0x00004000U) >> 14U);
    info->RdBlockMisalign = (uint8_t)((csd[1] & 0x00002000U) >> 13U);
    info->DSRImpl = (uint8_t)((csd[1] & 0x00001000U) >> 12U);
    info->Reserved2 = 0U;

    if(sdmmc1.info.type == FuriHalSdTypeSC) {
        info->DeviceSize = (((csd[1] & 0x000003FFU) << 2U) | ((csd[2] & 0xC0000000U) >> 30U));
        info->MaxRdCurrentVDDMin = (uint8_t)((csd[2] & 0x38000000U) >> 27U);
        info->MaxRdCurrentVDDMax = (uint8_t)((csd[2] & 0x07000000U) >> 24U);
        info->MaxWrCurrentVDDMin = (uint8_t)((csd[2] & 0x00E00000U) >> 21U);
        info->MaxWrCurrentVDDMax = (uint8_t)((csd[2] & 0x001C0000U) >> 18U);
        info->DeviceSizeMul = (uint8_t)((csd[2] & 0x00038000U) >> 15U);
    } else if(sdmmc1.info.type == FuriHalSdTypeHCXC) {
        info->DeviceSize = (((csd[1] & 0x0000003FU) << 16U) | ((csd[2] & 0xFFFF0000U) >> 16U));
    } else if(sdmmc1.info.type == FuriHalSdTypeMMCLowCapacity) {
        info->DeviceSize = (((csd[1] & 0x000003FFU) << 2U) | ((csd[2] & 0xC0000000U) >> 30U));
        info->MaxRdCurrentVDDMin = (uint8_t)((csd[2] & 0x38000000U) >> 27U);
        info->MaxRdCurrentVDDMax = (uint8_t)((csd[2] & 0x07000000U) >> 24U);
        info->MaxWrCurrentVDDMin = (uint8_t)((csd[2] & 0x00E00000U) >> 21U);
        info->MaxWrCurrentVDDMax = (uint8_t)((csd[2] & 0x001C0000U) >> 18U);
        info->DeviceSizeMul = (uint8_t)((csd[2] & 0x00038000U) >> 15U);
    } else if(sdmmc1.info.type == FuriHalSdTypeMMCHighCapacity) {
        info->DeviceSize = sdmmc1.csd_ext.SecCount;
    } else {
        furi_crash("Unknown SD/MMC type");
    }

    info->EraseGrSize = (uint8_t)((csd[2] & 0x00004000U) >> 14U);
    info->EraseGrMul = (uint8_t)((csd[2] & 0x00003F80U) >> 7U);
    info->WrProtectGrSize = (uint8_t)(csd[2] & 0x0000007FU);
    info->WrProtectGrEnable = (uint8_t)((csd[3] & 0x80000000U) >> 31U);
    info->ManDeflECC = (uint8_t)((csd[3] & 0x60000000U) >> 29U);
    info->WrSpeedFact = (uint8_t)((csd[3] & 0x1C000000U) >> 26U);
    info->MaxWrBlockLen = (uint8_t)((csd[3] & 0x03C00000U) >> 22U);
    info->WriteBlockPaPartial = (uint8_t)((csd[3] & 0x00200000U) >> 21U);
    info->Reserved3 = 0;
    info->ContentProtectAppli = (uint8_t)((csd[3] & 0x00010000U) >> 16U);
    info->FileFormatGroup = (uint8_t)((csd[3] & 0x00008000U) >> 15U);
    info->CopyFlag = (uint8_t)((csd[3] & 0x00004000U) >> 14U);
    info->PermWrProtect = (uint8_t)((csd[3] & 0x00002000U) >> 13U);
    info->TempWrProtect = (uint8_t)((csd[3] & 0x00001000U) >> 12U);
    info->FileFormat = (uint8_t)((csd[3] & 0x00000C00U) >> 10U);
    info->ECC = (uint8_t)((csd[3] & 0x00000300U) >> 8U);
    info->CSD_CRC = (uint8_t)((csd[3] & 0x000000FEU) >> 1U);
    info->Reserved4 = 1;

    return true;
}

static void sdmmc_parse_info(FuriHalSdInfo* info, CardCSDInfo* csd, uint32_t cid[4]) {
    if(info->type == FuriHalSdTypeSC) {
        uint32_t block_count = (csd->DeviceSize + 1U);
        block_count *= (1UL << ((csd->DeviceSizeMul & 0x07U) + 2U));
        uint32_t block_size = (1UL << (csd->RdBlockLen & 0x0FU));

        info->logical_block_count = (block_count) * ((block_size) / SD_BLOCKSIZE);
        info->logical_block_size = SD_BLOCKSIZE;
    } else if(sdmmc1.info.type == FuriHalSdTypeHCXC) {
        info->logical_block_count = ((csd->DeviceSize + 1U) * 1024U);
        info->logical_block_size = SD_BLOCKSIZE;
    } else if(sdmmc1.info.type == FuriHalSdTypeMMCLowCapacity) {
        uint32_t block_count = (csd->DeviceSize + 1U);
        block_count *= (1UL << ((csd->DeviceSizeMul & 0x07U) + 2U));
        uint32_t block_size = (1UL << (csd->RdBlockLen & 0x0FU));

        info->logical_block_count = (block_count) * ((block_size) / SD_BLOCKSIZE);
        info->logical_block_size = SD_BLOCKSIZE;
    } else if(sdmmc1.info.type == FuriHalSdTypeMMCHighCapacity) {
        info->logical_block_count = csd->DeviceSize;
        info->logical_block_size = MMC_BLOCKSIZE;
    } else {
        furi_crash("Unknown SD/MMC type");
    }

    info->manufacturer_id = (uint8_t)((cid[0] & 0xFF000000U) >> 24U);
    info->oem_id[0] = (char)((cid[0] & 0x00FF0000U) >> 16U);
    info->oem_id[1] = (char)((cid[0] & 0x0000FF00U) >> 8U);
    info->oem_id[2] = '\0';
    info->product_name[0] = (char)((cid[0] & 0x000000FFU) >> 0U);
    info->product_name[1] = (char)((cid[1] & 0xFF000000U) >> 24U);
    info->product_name[2] = (char)((cid[1] & 0x00FF0000U) >> 16U);
    info->product_name[3] = (char)((cid[1] & 0x0000FF00U) >> 8U);
    info->product_name[4] = (char)((cid[1] & 0x000000FFU) >> 0U);
    info->product_name[5] = '\0';
    info->product_revision_major = (uint8_t)((cid[2] & 0xFF000000U) >> 28U);
    info->product_revision_minor = (uint8_t)((cid[2] & 0xFF000000U) >> 24U) & 0x0FU;
    info->product_serial_number =
        (uint32_t)(((cid[2] & 0x00FFFFFFU) << 8U) | ((cid[3] & 0xFF000000U) >> 24U));
    info->manufacturing_month = (uint8_t)((cid[3] & 0x000FFF00U) >> 8U) & 0x0FU;
    info->manufacturing_year = 2000 + (uint16_t)((cid[3] & 0x000FFF00U) >> 12U);
}

static uint32_t sdmmc_init_card(void) {
    uint32_t errorstate;
    uint16_t rca = 0U;
    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(SDMMC_CMDTIMEOUT * 1000U);

    uint32_t CSD[4] = {0};
    uint32_t CID[4] = {0};

    /* Check the power State */
    if(SDMMC_GetPowerState(FURI_SDMMC_BLOCK) == 0U) {
        /* Power off */
        return FuriHalSdErrorRequestNotApplicable;
    }

    /* Send CMD2 ALL_SEND_CID */
    errorstate = SDMMC_CmdSendCID(FURI_SDMMC_BLOCK);
    if(errorstate != FuriHalSdErrorNone) {
        return errorstate;
    } else {
        /* Get Card identification number data */
        CID[0U] = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP1);
        CID[1U] = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP2);
        CID[2U] = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP3);
        CID[3U] = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP4);
    }

    /* Send CMD3 SET_REL_ADDR with argument 0 */
    /* SD Card publishes its RCA. */
    while(rca == 0U) {
        errorstate = SDMMC_CmdSetRelAdd(FURI_SDMMC_BLOCK, &rca);
        if(errorstate != FuriHalSdErrorNone) {
            return errorstate;
        }
        if(furi_hal_cortex_timer_is_expired(timer)) {
            return FuriHalSdErrorTimeout;
        }
    }

    /* Get the SD card RCA */
    sdmmc1.card_rca = rca;

    /* Send CMD9 SEND_CSD with argument as card's RCA */
    errorstate = SDMMC_CmdSendCSD(FURI_SDMMC_BLOCK, (uint32_t)(sdmmc1.card_rca << 16U));
    if(errorstate != FuriHalSdErrorNone) {
        return errorstate;
    } else {
        /* Get Card Specific Data */
        CSD[0U] = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP1);
        CSD[1U] = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP2);
        CSD[2U] = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP3);
        CSD[3U] = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP4);
    }

    /* Parse parameters */
    if(!sdmmc_parse_csd(&sdmmc1.csd, CSD)) {
        return FuriHalSdErrorTimeout;
    }

    sdmmc_parse_info(&sdmmc1.info, &sdmmc1.csd, CID);

    /* Select the Card */
    errorstate =
        SDMMC_CmdSelDesel(FURI_SDMMC_BLOCK, (uint32_t)(((uint32_t)sdmmc1.card_rca) << 16U));
    if(errorstate != FuriHalSdErrorNone) {
        return errorstate;
    }

    /* All cards are initialized */
    return FuriHalSdErrorNone;
}

static FuriHalSdError sdmmc_send_status_command(uint32_t* pSDstatus) {
    SDMMC_DataInitTypeDef config = {0};
    FuriHalSdError errorstate;
    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(SDMMC_REAL_DATATIMEOUT);
    uint32_t count;
    uint32_t* data = pSDstatus;

    /* Check SD response */
    if((SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP1) & SDMMC_CARD_LOCKED) ==
       SDMMC_CARD_LOCKED) {
        return FuriHalSdErrorLockUnlockFailed;
    }

    /* Set block size for card if it is not equal to current block size for card */
    errorstate = SDMMC_CmdBlockLength(FURI_SDMMC_BLOCK, 64U);
    if(errorstate != FuriHalSdErrorNone) {
        FURI_LOG_E(TAG, "SDMMC_CmdBlockLength failed with error 0x%08x", errorstate);
        return errorstate;
    }

    /* Send CMD55 */
    errorstate = SDMMC_CmdAppCommand(FURI_SDMMC_BLOCK, (uint32_t)(sdmmc1.card_rca << 16U));
    if(errorstate != FuriHalSdErrorNone) {
        FURI_LOG_E(TAG, "SDMMC_CmdAppCommand failed with error 0x%08x", errorstate);
        return errorstate;
    }

    /* Configure the SD DPSM (Data Path State Machine) */
    config.DataTimeOut = SDMMC_REAL_DATATIMEOUT;
    config.DataLength = 64U;
    config.DataBlockSize = SDMMC_DATABLOCK_SIZE_64B;
    config.TransferDir = SDMMC_TRANSFER_DIR_TO_SDMMC;
    config.TransferMode = SDMMC_TRANSFER_MODE_BLOCK;
    config.DPSM = SDMMC_DPSM_ENABLE;
    SDMMC_ConfigData(FURI_SDMMC_BLOCK, &config);

    /* Send ACMD13 (SD_APP_STAUS)  with argument as card's RCA */
    errorstate = SDMMC_CmdStatusRegister(FURI_SDMMC_BLOCK);
    if(errorstate != FuriHalSdErrorNone) {
        FURI_LOG_E(TAG, "SDMMC_CmdStatusRegister failed with error 0x%08x", errorstate);
        return errorstate;
    }

    /* Get status data */
    while(!sdmmc_get_flags(
        SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DATAEND)) {
        if(sdmmc_get_flags(SDMMC_FLAG_RXFIFOHF)) {
            for(count = 0U; count < 8U; count++) {
                *data = SDMMC_ReadFIFO(FURI_SDMMC_BLOCK);
                data++;
            }
        }

        if(furi_hal_cortex_timer_is_expired(timer)) {
            return FuriHalSdErrorTimeout;
        }
    }

    if(sdmmc_get_flags(SDMMC_FLAG_DTIMEOUT)) {
        return FuriHalSdErrorDataTimeout;
    } else if(sdmmc_get_flags(SDMMC_FLAG_DCRCFAIL)) {
        return FuriHalSdErrorDataCrcFail;
    } else if(sdmmc_get_flags(SDMMC_FLAG_RXOVERR)) {
        return FuriHalSdErrorRxOverrun;
    } else {
        /* Nothing to do */
    }

    while((sdmmc_get_flags(SDMMC_FLAG_DPSMACT))) {
        *data = SDMMC_ReadFIFO(FURI_SDMMC_BLOCK);
        data++;

        if(furi_hal_cortex_timer_is_expired(timer)) {
            return FuriHalSdErrorTimeout;
        }
    }

    /* Clear all the static status flags*/
    sdmmc_clear_static_data_flags();

    return FuriHalSdErrorNone;
}

static bool sd_mmc_get_card_status(CardStatus* card_status) {
    uint32_t sd_status[16] = {0};
    FuriHalSdError errorstate;
    bool status = true;

    errorstate = sdmmc_send_status_command(sd_status);
    if(errorstate != FuriHalSdErrorNone) {
        /* Clear all the static flags */
        sdmmc_clear_static_flags();
        FURI_LOG_E(TAG, "sdmmc_send_status_command failed with error 0x%08x", errorstate);
        status = false;
    } else {
        card_status->DataBusWidth = (uint8_t)((sd_status[0] & 0xC0U) >> 6U);
        card_status->SecuredMode = (uint8_t)((sd_status[0] & 0x20U) >> 5U);
        card_status->CardType = (uint16_t)(((sd_status[0] & 0x00FF0000U) >> 8U) |
                                           ((sd_status[0] & 0xFF000000U) >> 24U));
        card_status->ProtectedAreaSize =
            (((sd_status[1] & 0xFFU) << 24U) | ((sd_status[1] & 0xFF00U) << 8U) |
             ((sd_status[1] & 0xFF0000U) >> 8U) | ((sd_status[1] & 0xFF000000U) >> 24U));
        card_status->SpeedClass = (uint8_t)(sd_status[2] & 0xFFU);
        card_status->PerformanceMove = (uint8_t)((sd_status[2] & 0xFF00U) >> 8U);
        card_status->AllocationUnitSize = (uint8_t)((sd_status[2] & 0xF00000U) >> 20U);
        card_status->EraseSize =
            (uint16_t)(((sd_status[2] & 0xFF000000U) >> 16U) | (sd_status[3] & 0xFFU));
        card_status->EraseTimeout = (uint8_t)((sd_status[3] & 0xFC00U) >> 10U);
        card_status->EraseOffset = (uint8_t)((sd_status[3] & 0x0300U) >> 8U);
        card_status->UhsSpeedGrade = (uint8_t)((sd_status[3] & 0x00F0U) >> 4U);
        card_status->UhsAllocationUnitSize = (uint8_t)(sd_status[3] & 0x000FU);
        card_status->VideoSpeedClass = (uint8_t)((sd_status[4] & 0xFF000000U) >> 24U);
    }

    /* Set Block Size for Card */
    errorstate = SDMMC_CmdBlockLength(FURI_SDMMC_BLOCK, SD_BLOCKSIZE);
    if(errorstate != FuriHalSdErrorNone) {
        /* Clear all the static flags */
        sdmmc_clear_static_flags();
        FURI_LOG_E(TAG, "SDMMC_CmdBlockLength failed with error 0x%08x", errorstate);
        status = false;
    }

    return status;
}

static FuriHalSdError sdmmc_find_scr(uint32_t* pSCR) {
    SDMMC_DataInitTypeDef config = {0};
    FuriHalSdError errorstate;
    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(SDMMC_REAL_DATATIMEOUT);
    uint32_t index = 0U;
    uint32_t tempscr[2U] = {0};
    uint32_t* scr = pSCR;

    /* Set Block Size To 8 Bytes */
    errorstate = SDMMC_CmdBlockLength(FURI_SDMMC_BLOCK, 8U);
    if(errorstate != FuriHalSdErrorNone) {
        FURI_LOG_E(TAG, "SDMMC_CmdBlockLength failed with error 0x%08x", errorstate);
        return errorstate;
    }

    /* Send CMD55 APP_CMD with argument as card's RCA */
    errorstate = SDMMC_CmdAppCommand(FURI_SDMMC_BLOCK, (uint32_t)((sdmmc1.card_rca) << 16U));
    if(errorstate != FuriHalSdErrorNone) {
        FURI_LOG_E(TAG, "SDMMC_CmdAppCommand failed with error 0x%08x", errorstate);
        return errorstate;
    }

    config.DataTimeOut = SDMMC_REAL_DATATIMEOUT;
    config.DataLength = 8U;
    config.DataBlockSize = SDMMC_DATABLOCK_SIZE_8B;
    config.TransferDir = SDMMC_TRANSFER_DIR_TO_SDMMC;
    config.TransferMode = SDMMC_TRANSFER_MODE_BLOCK;
    config.DPSM = SDMMC_DPSM_ENABLE;
    SDMMC_ConfigData(FURI_SDMMC_BLOCK, &config);

    /* Send ACMD51 SD_APP_SEND_SCR with argument as 0 */
    errorstate = SDMMC_CmdSendSCR(FURI_SDMMC_BLOCK);
    if(errorstate != FuriHalSdErrorNone) {
        FURI_LOG_E(TAG, "SDMMC_CmdSendSCR failed with error 0x%08x", errorstate);
        return errorstate;
    }

    while(!sdmmc_get_flags(
        SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DBCKEND |
        SDMMC_FLAG_DATAEND)) {
        if((!sdmmc_get_flags(SDMMC_FLAG_RXFIFOE)) && (index == 0U)) {
            tempscr[0] = SDMMC_ReadFIFO(FURI_SDMMC_BLOCK);
            tempscr[1] = SDMMC_ReadFIFO(FURI_SDMMC_BLOCK);
            index++;
        }

        if(furi_hal_cortex_timer_is_expired(timer)) {
            FURI_LOG_E(TAG, "SDMMC_ReadFIFO failed with timeout");
            return FuriHalSdErrorTimeout;
        }
    }

    if(sdmmc_get_flags(SDMMC_FLAG_DTIMEOUT)) {
        sdmmc_clear_flags(SDMMC_FLAG_DTIMEOUT);
        FURI_LOG_E(TAG, "SDTIMEOUT");
        return FuriHalSdErrorDataTimeout;
    } else if(sdmmc_get_flags(SDMMC_FLAG_DCRCFAIL)) {
        sdmmc_clear_flags(SDMMC_FLAG_DCRCFAIL);
        FURI_LOG_E(TAG, "SDCRCFAIL");
        return FuriHalSdErrorDataCrcFail;
    } else if(sdmmc_get_flags(SDMMC_FLAG_RXOVERR)) {
        sdmmc_clear_flags(SDMMC_FLAG_RXOVERR);
        FURI_LOG_E(TAG, "SDRXOVERR");
        return FuriHalSdErrorRxOverrun;
    } else {
        /* No error flag set */
        /* Clear all the static flags */
        sdmmc_clear_static_data_flags();

        *scr =
            (((tempscr[1] & SDMMC_0TO7BITS) << 24U) | ((tempscr[1] & SDMMC_8TO15BITS) << 8U) |
             ((tempscr[1] & SDMMC_16TO23BITS) >> 8U) | ((tempscr[1] & SDMMC_24TO31BITS) >> 24U));
        scr++;
        *scr =
            (((tempscr[0] & SDMMC_0TO7BITS) << 24U) | ((tempscr[0] & SDMMC_8TO15BITS) << 8U) |
             ((tempscr[0] & SDMMC_16TO23BITS) >> 8U) | ((tempscr[0] & SDMMC_24TO31BITS) >> 24U));
    }

    return FuriHalSdErrorNone;
}

static FuriHalSdError sdmmc_wide_bus_enable(void) {
    uint32_t scr[2U] = {0};
    FuriHalSdError errorstate;

    if((SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP1) & SDMMC_CARD_LOCKED) ==
       SDMMC_CARD_LOCKED) {
        FURI_LOG_E(TAG, "SD Card unlock failed");
        return FuriHalSdErrorLockUnlockFailed;
    }

    /* Get SCR Register */
    errorstate = sdmmc_find_scr(scr);
    if(errorstate != FuriHalSdErrorNone) {
        FURI_LOG_E(TAG, "sdmmc_find_scr failed with error 0x%08x", errorstate);
        return errorstate;
    }

    /* If requested card supports wide bus operation */
    if((scr[1U] & SDMMC_WIDE_BUS_SUPPORT) != SDMMC_ALLZERO) {
        /* Send CMD55 APP_CMD with argument as card's RCA.*/
        errorstate = SDMMC_CmdAppCommand(FURI_SDMMC_BLOCK, (uint32_t)(sdmmc1.card_rca << 16U));
        if(errorstate != FuriHalSdErrorNone) {
            FURI_LOG_E(TAG, "SDMMC_CmdAppCommand failed with error 0x%08x", errorstate);
            return errorstate;
        }

        /* Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
        errorstate = SDMMC_CmdBusWidth(FURI_SDMMC_BLOCK, 2U);
        if(errorstate != FuriHalSdErrorNone) {
            FURI_LOG_E(TAG, "SDMMC_CmdBusWidth failed with error 0x%08x", errorstate);
            return errorstate;
        }

        return FuriHalSdErrorNone;
    } else {
        return FuriHalSdErrorRequestNotApplicable;
    }
}

static bool sdmmc_config_wide_bus_operation(uint32_t sdmmc_clk) {
    furi_assert(sdmmc_clk != 0U);

    SDMMC_InitTypeDef init = {0};
    FuriHalSdError errorstate;

    bool status = true;

    errorstate = sdmmc_wide_bus_enable();
    if(errorstate != FuriHalSdErrorNone) {
        /* Clear all the static flags */
        sdmmc_clear_static_flags();
        FURI_LOG_E(TAG, "sdmmc_wide_bus_enable failed with error 0x%08x", errorstate);
        status = false;
    }

    /* Configure the SDMMC peripheral */
    init.ClockEdge = SDMMC_INIT_CLOCK_EDGE;
    init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE;
    init.BusWide = SDMMC_BUS_WIDE_4B;
    init.HardwareFlowControl = SDMMC_INIT_HARDWARE_FLOW_CONTROL;
    init.ClockDiv = 0U;

    /* Check if user Clock div < Normal speed 25Mhz, no change in Clockdiv */
    if(init.ClockDiv >= (sdmmc_clk / (2U * SD_NORMAL_SPEED_FREQ))) {
        init.ClockDiv = init.ClockDiv;
    } else if(sdmmc1.info.speed == FuriHalSdSpeedUltraHigh) {
        /* UltraHigh speed SD card, user Clock div */
        init.ClockDiv = init.ClockDiv;
    } else if(sdmmc1.info.speed == FuriHalSdSpeedHigh) {
        /* High speed SD card, Max Frequency = 50Mhz */
        if(init.ClockDiv == 0U) {
            if(sdmmc_clk > SD_HIGH_SPEED_FREQ) {
                init.ClockDiv = sdmmc_clk / (2U * SD_HIGH_SPEED_FREQ);
            } else {
                init.ClockDiv = init.ClockDiv;
            }
        } else {
            if((sdmmc_clk / (2U * init.ClockDiv)) > SD_HIGH_SPEED_FREQ) {
                init.ClockDiv = sdmmc_clk / (2U * SD_HIGH_SPEED_FREQ);
            } else {
                init.ClockDiv = init.ClockDiv;
            }
        }
    } else {
        /* No High speed SD card, Max Frequency = 25Mhz */
        if(init.ClockDiv == 0U) {
            if(sdmmc_clk > SD_NORMAL_SPEED_FREQ) {
                init.ClockDiv = sdmmc_clk / (2U * SD_NORMAL_SPEED_FREQ);
            } else {
                init.ClockDiv = init.ClockDiv;
            }
        } else {
            if((sdmmc_clk / (2U * init.ClockDiv)) > SD_NORMAL_SPEED_FREQ) {
                init.ClockDiv = sdmmc_clk / (2U * SD_NORMAL_SPEED_FREQ);
            } else {
                init.ClockDiv = init.ClockDiv;
            }
        }
    }

    SDMMC_Init(FURI_SDMMC_BLOCK, init);

    /* Set Block Size for Card */
    errorstate = SDMMC_CmdBlockLength(FURI_SDMMC_BLOCK, SD_BLOCKSIZE);
    if(errorstate != FuriHalSdErrorNone) {
        /* Clear all the static flags */
        sdmmc_clear_static_flags();
        FURI_LOG_E(TAG, "SDMMC_CmdBlockLength failed with error 0x%08x", errorstate);
        status = false;
    }

    return status;
}

static FuriHalSdError sdmmc_send_status(uint32_t* pCardStatus) {
    FuriHalSdError errorstate;

    /* Send Status command */
    errorstate = SDMMC_CmdSendStatus(FURI_SDMMC_BLOCK, (uint32_t)(sdmmc1.card_rca << 16U));
    if(errorstate != FuriHalSdErrorNone) {
        FURI_LOG_E(TAG, "SDMMC_CmdSendStatus failed with error 0x%08x", errorstate);
        return errorstate;
    }

    /* Get SD card status */
    *pCardStatus = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP1);

    return FuriHalSdErrorNone;
}

typedef enum {
    SdCardStateReady = 0x00000001U, /*!< Card state is ready */
    SdCardStateIdentification = 0x00000002U, /*!< Card is in identification state */
    SdCardStateStandby = 0x00000003U, /*!< Card is in standby state */
    SdCardStateTransfer = 0x00000004U, /*!< Card is in transfer state */
    SdCardStateSending = 0x00000005U, /*!< Card is sending an operation */
    SdCardStateReceiving = 0x00000006U, /*!< Card is receiving operation information */
    SdCardStateProgramming = 0x00000007U, /*!< Card is in programming state */
    SdCardStateDisconnected = 0x00000008U, /*!< Card is disconnected */
    SdCardStateError = 0x000000FFU, /*!< Card response Error */
} SdCardState;

SdCardState sdmmc_get_card_state(void) {
    SdCardState cardstate;
    FuriHalSdError errorstate;
    uint32_t resp1 = 0;

    errorstate = sdmmc_send_status(&resp1);
    if(errorstate != FuriHalSdErrorNone) {
        FURI_LOG_E(TAG, "sdmmc_send_status failed with error 0x%08x", errorstate);
    }

    cardstate = ((resp1 >> 9U) & 0x0FU);
    return cardstate;
}

static bool sdmmc_wait_for_transfer_state(size_t timeout_ms) {
    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(timeout_ms);
    SdCardState card_state = sdmmc_get_card_state();
    while(card_state != SdCardStateTransfer) {
        if(furi_hal_cortex_timer_is_expired(timer)) {
            FURI_LOG_E(TAG, "sdmmc_get_card_state failed");
            return false;
        }
        card_state = sdmmc_get_card_state();
    }

    return true;
}

static bool sdmmc_read_blocks_dma(uint8_t* data, uint32_t address, uint32_t block_count) {
    SDMMC_DataInitTypeDef config = {0};
    FuriHalSdError errorstate;

    /* Initialize data control register */
    FURI_SDMMC_BLOCK->DCTRL = 0U;

    /* Update the SD transfer context */
    sdmmc_dma_context.rx_buffer = data;
    sdmmc_dma_context.rx_size = SD_BLOCKSIZE * block_count;

    if(sdmmc1.info.type != FuriHalSdTypeMMCHighCapacity) {
        address *= SD_BLOCKSIZE;
    }

    /* Configure the SD DPSM (Data Path State Machine) */
    config.DataTimeOut = SDMMC_REAL_DATATIMEOUT;
    config.DataLength = SD_BLOCKSIZE * block_count;
    config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
    config.TransferDir = SDMMC_TRANSFER_DIR_TO_SDMMC;
    config.TransferMode = SDMMC_TRANSFER_MODE_BLOCK;
    config.DPSM = SDMMC_DPSM_DISABLE;
    SDMMC_ConfigData(FURI_SDMMC_BLOCK, &config);

    __SDMMC_CMDTRANS_ENABLE(FURI_SDMMC_BLOCK);
    FURI_SDMMC_BLOCK->IDMABASER = (uint32_t)data;
    FURI_SDMMC_BLOCK->IDMACTRL = SDMMC_ENABLE_IDMA_SINGLE_BUFF;

    /* Read Blocks in DMA mode */
    if(block_count > 1U) {
        sdmmc_dma_context.state = SdMmcDmaStateEnabled | SdMmcDmaStateRxMulti;

        /* Read Multi Block command */
        errorstate = SDMMC_CmdReadMultiBlock(FURI_SDMMC_BLOCK, address);
    } else {
        sdmmc_dma_context.state = SdMmcDmaStateEnabled | SdMmcDmaStateRxSingle;

        /* Read Single Block command */
        errorstate = SDMMC_CmdReadSingleBlock(FURI_SDMMC_BLOCK, address);
    }

    if(errorstate != FuriHalSdErrorNone) {
        /* Clear all the static flags */
        sdmmc_clear_static_flags();
        FURI_LOG_E(TAG, "SDMMC_CmdReadSingle/MultiBlock failed with error 0x%08x", errorstate);
        return false;
    }

    /* Enable transfer interrupts */
    sdmmc_enable_it(SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_RXOVERR | SDMMC_IT_DATAEND);

    return true;
}

static bool sdmmc_write_blocks_dma(const uint8_t* data, uint32_t address, uint32_t block_count) {
    SDMMC_DataInitTypeDef config = {0};
    FuriHalSdError errorstate;

    /* Initialize data control register */
    FURI_SDMMC_BLOCK->DCTRL = 0U;

    sdmmc_dma_context.tx_buffer = data;
    sdmmc_dma_context.tx_size = SD_BLOCKSIZE * block_count;

    if(sdmmc1.info.type != FuriHalSdTypeMMCHighCapacity) {
        address *= SD_BLOCKSIZE;
    }

    /* Configure the SD DPSM (Data Path State Machine) */
    config.DataTimeOut = SDMMC_REAL_DATATIMEOUT;
    config.DataLength = SD_BLOCKSIZE * block_count;
    config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
    config.TransferDir = SDMMC_TRANSFER_DIR_TO_CARD;
    config.TransferMode = SDMMC_TRANSFER_MODE_BLOCK;
    config.DPSM = SDMMC_DPSM_DISABLE;
    SDMMC_ConfigData(FURI_SDMMC_BLOCK, &config);

    __SDMMC_CMDTRANS_ENABLE(FURI_SDMMC_BLOCK);

    FURI_SDMMC_BLOCK->IDMABASER = (uint32_t)data;
    FURI_SDMMC_BLOCK->IDMACTRL = SDMMC_ENABLE_IDMA_SINGLE_BUFF;

    /* Write Blocks in Polling mode */
    if(block_count > 1U) {
        sdmmc_dma_context.state = SdMmcDmaStateEnabled | SdMmcDmaStateRxMulti;

        /* Write Multi Block command */
        errorstate = SDMMC_CmdWriteMultiBlock(FURI_SDMMC_BLOCK, address);
    } else {
        sdmmc_dma_context.state = SdMmcDmaStateEnabled | SdMmcDmaStateRxSingle;

        /* Write Single Block command */
        errorstate = SDMMC_CmdWriteSingleBlock(FURI_SDMMC_BLOCK, address);
    }
    if(errorstate != FuriHalSdErrorNone) {
        /* Clear all the static flags */
        sdmmc_clear_static_flags();
        FURI_LOG_E(TAG, "sdmmc_write_blocks_dma failed with error 0x%08x", errorstate);
        return false;
    }

    /* Enable transfer interrupts */
    sdmmc_enable_it(SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR | SDMMC_IT_DATAEND);

    return true;
}

static void sdmmc_irq_handler(void* ctx) {
    UNUSED(ctx);

    uint32_t errorstate;

    if(sdmmc_get_flags(SDMMC_FLAG_DATAEND)) {
        sdmmc_clear_flags(SDMMC_FLAG_DATAEND);

        sdmmc_disable_it(
            SDMMC_IT_DATAEND | SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR |
            SDMMC_IT_RXOVERR);

        __SDMMC_CMDTRANS_DISABLE(FURI_SDMMC_BLOCK);

        if((sdmmc_dma_context.state & SdMmcDmaStateEnabled) != 0U) {
            FURI_SDMMC_BLOCK->DLEN = 0;
            FURI_SDMMC_BLOCK->DCTRL = 0;
            FURI_SDMMC_BLOCK->IDMACTRL = SDMMC_DISABLE_IDMA;

            /* Stop Transfer for Write Multi blocks or Read Multi blocks */
            if(((sdmmc_dma_context.state & SdMmcDmaStateRxMulti) != 0U) ||
               ((sdmmc_dma_context.state & SdMmcDmaStateTxMulti) != 0U)) {
                errorstate = SDMMC_CmdStopTransfer(FURI_SDMMC_BLOCK);
                if(errorstate != FuriHalSdErrorNone) {
                    sdmmc_dma_context.error |= errorstate;

                    furi_event_flag_set(sdmmc_dma_context.event, SdMmcDmaEventError);
                }
            }

            sdmmc_clear_static_data_flags();
            furi_event_flag_set(sdmmc_dma_context.event, SdMmcDmaEventComplete);
        }
    } else if(sdmmc_get_flags(
                  SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_RXOVERR |
                  SDMMC_FLAG_TXUNDERR)) {
        /* Set Error code */
        if(sdmmc_get_flags(SDMMC_IT_DCRCFAIL)) {
            sdmmc_dma_context.error |= FuriHalSdErrorDataCrcFail;
        }
        if(sdmmc_get_flags(SDMMC_IT_DTIMEOUT)) {
            sdmmc_dma_context.error |= FuriHalSdErrorDataTimeout;
        }
        if(sdmmc_get_flags(SDMMC_IT_RXOVERR)) {
            sdmmc_dma_context.error |= FuriHalSdErrorRxOverrun;
        }
        if(sdmmc_get_flags(SDMMC_IT_TXUNDERR)) {
            sdmmc_dma_context.error |= FuriHalSdErrorTxUnderrun;
        }

        /* Clear All flags */
        sdmmc_clear_static_data_flags();

        /* Disable all interrupts */
        sdmmc_disable_it(
            SDMMC_IT_DATAEND | SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR |
            SDMMC_IT_RXOVERR);

        __SDMMC_CMDTRANS_DISABLE(FURI_SDMMC_BLOCK);
        FURI_SDMMC_BLOCK->DCTRL |= SDMMC_DCTRL_FIFORST;
        FURI_SDMMC_BLOCK->CMD |= SDMMC_CMD_CMDSTOP;
        sdmmc_dma_context.error |= SDMMC_CmdStopTransfer(FURI_SDMMC_BLOCK);
        FURI_SDMMC_BLOCK->CMD &= ~(SDMMC_CMD_CMDSTOP);
        sdmmc_clear_flags(SDMMC_FLAG_DABORT);

        if((sdmmc_dma_context.state & SdMmcDmaStateEnabled) != 0U) {
            if(sdmmc_dma_context.error != FuriHalSdErrorNone) {
                /* Disable Internal DMA */
                FURI_SDMMC_BLOCK->IDMACTRL = SDMMC_DISABLE_IDMA;

                furi_event_flag_set(sdmmc_dma_context.event, SdMmcDmaEventError);
            }
        }
    }
}

static bool sdmmc_init_card_lowspeed(uint32_t sdmmc_clk) {
    furi_assert(sdmmc_clk != 0U);

    // init params
    SDMMC_InitTypeDef init = {0};
    init.ClockEdge = SDMMC_INIT_CLOCK_EDGE;
    init.ClockPowerSave = SDMMC_INIT_CLOCK_POWER_SAVE;
    init.BusWide = SDMMC_BUS_WIDE_1B;
    init.HardwareFlowControl = SDMMC_INIT_HARDWARE_FLOW_CONTROL;

    // set init clock (400 kHz)
    init.ClockDiv = sdmmc_clk / (2U * SD_INIT_FREQ);

    // init sdmmc block
    SDMMC_Init(FURI_SDMMC_BLOCK, init);

    // power on sdmmc block
    SDMMC_PowerState_ON(FURI_SDMMC_BLOCK);

    // wait 74 sd clock cycles
    furi_delay_us((1000000 * 74) / (sdmmc_clk / (2U * init.ClockDiv)));

    // identify card operating voltage
    FuriHalSdError errorstate = sdmmc_power_on();
    if(errorstate != FuriHalSdErrorNone) {
        FURI_LOG_E(TAG, "sdmmc_power_on failed with error 0x%08x", errorstate);
        return false;
    }

    // initialize card
    errorstate = sdmmc_init_card();
    if(errorstate != FuriHalSdErrorNone) {
        FURI_LOG_E(TAG, "sdmmc_init_card failed with error 0x%08x", errorstate);
        return false;
    }

    // set block size for card
    errorstate = SDMMC_CmdBlockLength(FURI_SDMMC_BLOCK, SD_BLOCKSIZE);
    if(errorstate != FuriHalSdErrorNone) {
        sdmmc_clear_static_flags();
        FURI_LOG_E(TAG, "SDMMC_CmdBlockLength failed with error 0x%08x", errorstate);
        return false;
    }

    return true;
}

static bool sdmmc_init_sdcard(uint32_t sdmmc_clk) {
    if(!sdmmc_init_card_lowspeed(sdmmc_clk)) {
        FURI_LOG_E(TAG, "sdmmc_init_card failed");
        return false;
    }

    if(sd_mmc_get_card_status(&sdmmc1.status) != true) {
        FURI_LOG_E(TAG, "sd_mmc_get_card_status failed");
        return false;
    }

    /* Get Initial Card Speed from Card Status */
    uint32_t speedgrade = sdmmc1.status.UhsSpeedGrade;
    uint32_t unitsize = sdmmc1.status.UhsAllocationUnitSize;
    if(sdmmc1.info.type == FuriHalSdTypeHCXC && ((speedgrade != 0U) || (unitsize != 0U))) {
        sdmmc1.info.speed = FuriHalSdSpeedUltraHigh;
    } else if(sdmmc1.info.type == FuriHalSdTypeHCXC) {
        sdmmc1.info.speed = FuriHalSdSpeedHigh;
    } else {
        sdmmc1.info.speed = FuriHalSdSpeedNormal;
    }

    /* Configure the bus wide */
    if(sdmmc_config_wide_bus_operation(sdmmc_clk) != true) {
        FURI_LOG_E(TAG, "sdmmc_config_wide_bus_operation failed");
        return false;
    }

    /* Verify that SD card is ready to use after Initialization */
    if(!sdmmc_wait_for_transfer_state(SDMMC_REAL_DATATIMEOUT)) {
        FURI_LOG_E(TAG, "sdmmc_wait_for_transfer_state failed");
        return false;
    }

    return true;
}

// emmc
static FuriHalSdError
    sdmmc_mmc_read_ext_csd(CardExtendedCSDRegister* ext_csd_reg, uint32_t timeout) {
    furi_check(ext_csd_reg);

    SDMMC_DataInitTypeDef config;
    FuriHalSdError errorstate = FuriHalSdErrorNone;
    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(timeout);
    uint32_t count;
    uint32_t* tmp_buf;

    /* Initialize data control register */
    FURI_SDMMC_BLOCK->DCTRL = 0;

    /* Initiaize the destination pointer */
    tmp_buf = (uint32_t*)ext_csd_reg;

    /* Configure the MMC DPSM (Data Path State Machine) */
    config.DataTimeOut = SDMMC_REAL_DATATIMEOUT;
    config.DataLength = MMC_BLOCKSIZE;
    config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
    config.TransferDir = SDMMC_TRANSFER_DIR_TO_SDMMC;
    config.TransferMode = SDMMC_TRANSFER_MODE_BLOCK;
    config.DPSM = SDMMC_DPSM_DISABLE;
    (void)SDMMC_ConfigData(FURI_SDMMC_BLOCK, &config);
    __SDMMC_CMDTRANS_ENABLE(FURI_SDMMC_BLOCK);

    /* Send ExtCSD Read command to Card */
    errorstate = SDMMC_CmdSendEXTCSD(FURI_SDMMC_BLOCK, (uint32_t)(sdmmc1.card_rca << 16U));
    if(errorstate != FuriHalSdErrorNone) {
        /* Clear all the static flags */
        sdmmc_clear_static_flags();
        return errorstate;
    }

    /* Poll on SDMMC flags */
    while(!sdmmc_get_flags(
        SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DATAEND)) {
        if(sdmmc_get_flags(SDMMC_FLAG_RXFIFOHF)) {
            /* Read data from SDMMC Rx FIFO */
            for(count = 0U; count < (SDMMC_FIFO_SIZE / 4U); count++) {
                *tmp_buf = SDMMC_ReadFIFO(FURI_SDMMC_BLOCK);
                tmp_buf++;
            }
        }

        if(furi_hal_cortex_timer_is_expired(timer)) {
            /* Clear all the static flags */
            sdmmc_clear_static_flags();
            return FuriHalSdErrorTimeout;
        }
    }

    __SDMMC_CMDTRANS_DISABLE(FURI_SDMMC_BLOCK);

    /* Get error state */
    if(sdmmc_get_flags(SDMMC_FLAG_DTIMEOUT)) {
        /* Clear all the static flags */
        sdmmc_clear_static_flags();
        return FuriHalSdErrorDataTimeout;
    } else if(sdmmc_get_flags(SDMMC_FLAG_DCRCFAIL)) {
        /* Clear all the static flags */
        sdmmc_clear_static_flags();
        return FuriHalSdErrorDataCrcFail;
    } else if(sdmmc_get_flags(SDMMC_FLAG_RXOVERR)) {
        /* Clear all the static flags */
        sdmmc_clear_static_flags();
        return FuriHalSdErrorRxOverrun;
    } else {
        /* Nothing to do */
    }

    /* Clear the static data flags */
    sdmmc_clear_static_data_flags();

    return errorstate;
}

static FuriHalSdError sdmmc_mmc_init_card(void) {
    FuriHalSdError errorstate;
    uint16_t rca = 2U;

    uint32_t CSD[4] = {0};
    uint32_t CID[4] = {0};

    /* Check the power State */
    if(SDMMC_GetPowerState(FURI_SDMMC_BLOCK) == 0U) {
        /* Power off */
        return FuriHalSdErrorRequestNotApplicable;
    }

    /* Send CMD2 ALL_SEND_CID */
    errorstate = SDMMC_CmdSendCID(FURI_SDMMC_BLOCK);
    if(errorstate != FuriHalSdErrorNone) {
        return errorstate;
    }

    /* Get Card identification number data */
    CID[0U] = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP1);
    CID[1U] = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP2);
    CID[2U] = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP3);
    CID[3U] = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP4);

    /* Send CMD3 SET_REL_ADDR with RCA = 2 (should be greater than 1) */
    /* MMC Card publishes its RCA. */
    errorstate = SDMMC_CmdSetRelAdd(FURI_SDMMC_BLOCK, &rca);
    if(errorstate != FuriHalSdErrorNone) {
        return errorstate;
    }

    /* Get the SD card RCA */
    sdmmc1.card_rca = rca;

    /* Send CMD9 SEND_CSD with argument as card's RCA */
    errorstate = SDMMC_CmdSendCSD(FURI_SDMMC_BLOCK, (uint32_t)(sdmmc1.card_rca << 16U));
    if(errorstate != FuriHalSdErrorNone) {
        return errorstate;
    } else {
        /* Get Card Specific Data */
        CSD[0U] = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP1);
        CSD[1U] = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP2);
        CSD[2U] = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP3);
        CSD[3U] = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP4);
    }

    /* Select the Card */
    errorstate =
        SDMMC_CmdSelDesel(FURI_SDMMC_BLOCK, (uint32_t)(((uint32_t)sdmmc1.card_rca) << 16U));
    if(errorstate != FuriHalSdErrorNone) {
        return errorstate;
    }

    /* While card is not ready for data and trial number for sending CMD13 is not exceeded */
    errorstate =
        SDMMC_CmdSendStatus(FURI_SDMMC_BLOCK, (uint32_t)(((uint32_t)sdmmc1.card_rca) << 16U));
    if(errorstate != FuriHalSdErrorNone) {
        return errorstate;
    }

    /* Get Extended CSD parameters */
    errorstate = sdmmc_mmc_read_ext_csd(&sdmmc1.csd_ext, SDMMC_CMDTIMEOUT * 1000U);
    if(errorstate != FuriHalSdErrorNone) {
        return errorstate;
    }

    /* Parse parameters */
    if(!sdmmc_parse_csd(&sdmmc1.csd, CSD)) {
        return FuriHalSdErrorTimeout;
    }

    sdmmc_parse_info(&sdmmc1.info, &sdmmc1.csd, CID);

    /* While card is not ready for data and trial number for sending CMD13 is not exceeded */
    errorstate =
        SDMMC_CmdSendStatus(FURI_SDMMC_BLOCK, (uint32_t)(((uint32_t)sdmmc1.card_rca) << 16U));
    if(errorstate != FuriHalSdErrorNone) {
        return errorstate;
    }

    /* All cards are initialized */
    return FuriHalSdErrorNone;
}

static FuriHalSdError sdmmc_mmc_power_on(void) {
    FuriHalSdError errorstate;

    /* CMD0: GO_IDLE_STATE */
    errorstate = SDMMC_CmdGoIdleState(FURI_SDMMC_BLOCK);
    if(errorstate != FuriHalSdErrorNone) {
        return errorstate;
    }

    uint32_t count = 0U;
    uint32_t response = 0U;
    uint32_t validvoltage = 0U;

    while(validvoltage == 0U) {
        if(count++ == SDMMC_MAX_VOLT_TRIAL) {
            return FuriHalSdErrorInvalidVoltRange;
        }

        /* SEND CMD1 APP_CMD with voltage range as argument */
        errorstate = SDMMC_CmdOpCondition(FURI_SDMMC_BLOCK, EMMC_HIGH_VOLTAGE_RANGE);
        if(errorstate != FuriHalSdErrorNone) {
            return FuriHalSdErrorUnsupportedFeature;
        }

        /* Get command response */
        response = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP1);

        /* Get operating voltage*/
        validvoltage = (((response >> 31U) == 1U) ? 1U : 0U);
    }

    /* When power routine is finished and command returns valid voltage */
    // TODO: new card types
    if(((response & (0xFF000000U)) >> 24) == 0xC0U) {
        // hmmc->MmcCard.CardType = MMC_HIGH_CAPACITY_CARD;
        FURI_LOG_D(TAG, "Found high capacity MMC");
        sdmmc1.info.type = FuriHalSdTypeMMCHighCapacity;
    } else {
        // hmmc->MmcCard.CardType = MMC_LOW_CAPACITY_CARD;
        FURI_LOG_D(TAG, "Found low capacity MMC");
        sdmmc1.info.type = FuriHalSdTypeMMCLowCapacity;
    }

    return FuriHalSdErrorNone;
}

static FuriHalSdError sdmm_mmc_pwr_class_update(uint32_t Wide, uint32_t Speed) {
    uint32_t count;
    uint32_t response = 0U;
    FuriHalSdError errorstate = FuriHalSdErrorNone;
    uint8_t power_class = sdmmc1.csd_ext.PowerClass;
    uint32_t supported_pwr_class;

    if((Wide == SDMMC_BUS_WIDE_8B) || (Wide == SDMMC_BUS_WIDE_4B)) {
        /* Get the supported PowerClass field of the Extended CSD register */
        if(Speed == SDMMC_SPEED_MODE_DDR) {
            /* Field PWR_CL_DDR_52_xxx [238 or 239] */
            supported_pwr_class = sdmmc1.csd_ext.PwrClDdr52_195;
        } else if(Speed == SDMMC_SPEED_MODE_HIGH) {
            /* Field PWR_CL_52_xxx [200 or 202] */
            supported_pwr_class = sdmmc1.csd_ext.PwrCl52_195;
        } else {
            /* Field PWR_CL_26_xxx [201 or 203] */
            supported_pwr_class = sdmmc1.csd_ext.PwrCl26_195;
        }

        if(Wide == SDMMC_BUS_WIDE_8B) {
            /* Bit [7:4]: power class for 8-bits bus configuration - Bit [3:0]: power class for 4-bits bus configuration */
            supported_pwr_class = (supported_pwr_class >> 4U);
        }

        if((power_class & 0x0FU) != (supported_pwr_class & 0x0FU)) {
            /* Need to change current power class */
            errorstate = SDMMC_CmdSwitch(
                FURI_SDMMC_BLOCK, (0x03BB0000U | ((supported_pwr_class & 0x0FU) << 8U)));

            if(errorstate == FuriHalSdErrorNone) {
                /* While card is not ready for data and trial number for sending CMD13 is not exceeded */
                count = SDMMC_MAX_TRIAL;
                do {
                    errorstate = SDMMC_CmdSendStatus(
                        FURI_SDMMC_BLOCK, (uint32_t)(((uint32_t)sdmmc1.card_rca) << 16U));
                    if(errorstate != FuriHalSdErrorNone) {
                        break;
                    }

                    /* Get command response */
                    response = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP1);
                    count--;
                } while(((response & 0x100U) == 0U) && (count != 0U));

                /* Check the status after the switch command execution */
                if((count != 0U) && (errorstate == FuriHalSdErrorNone)) {
                    /* Check the bit SWITCH_ERROR of the device status */
                    if((response & 0x80U) != 0U) {
                        errorstate = SDMMC_ERROR_UNSUPPORTED_FEATURE;
                    }
                } else if(count == 0U) {
                    errorstate = SDMMC_ERROR_TIMEOUT;
                } else {
                    /* Nothing to do */
                }
            }
            //* Update the current power class */
            errorstate = sdmmc_mmc_read_ext_csd(&sdmmc1.csd_ext, SDMMC_CMDTIMEOUT * 1000U);
        }
    }

    return errorstate;
}

static FuriHalSdError sdmmc_mmc_high_speed(FunctionalState state, uint32_t sdmmc_clk) {
    uint32_t errorstate = FuriHalSdErrorNone;
    uint32_t response = 0U;
    uint32_t count;
    SDMMC_InitTypeDef init;

    if(((FURI_SDMMC_BLOCK->CLKCR & SDMMC_CLKCR_BUSSPEED) != 0U) && (state == DISABLE)) {
        errorstate = sdmm_mmc_pwr_class_update(
            (FURI_SDMMC_BLOCK->CLKCR & SDMMC_CLKCR_WIDBUS), SDMMC_SPEED_MODE_DEFAULT);
        if(errorstate != FuriHalSdErrorNone) {
            return errorstate;
        }

        /* Index : 185 - Value : 0 */
        errorstate = SDMMC_CmdSwitch(FURI_SDMMC_BLOCK, 0x03B90000U);
    }

    if(((FURI_SDMMC_BLOCK->CLKCR & SDMMC_CLKCR_BUSSPEED) == 0U) && (state != DISABLE)) {
        errorstate = sdmm_mmc_pwr_class_update(
            (FURI_SDMMC_BLOCK->CLKCR & SDMMC_CLKCR_WIDBUS), SDMMC_SPEED_MODE_HIGH);
        if(errorstate != FuriHalSdErrorNone) {
            return errorstate;
        }

        /* Index : 185 - Value : 1 */
        errorstate = SDMMC_CmdSwitch(FURI_SDMMC_BLOCK, 0x03B90100U);
    }

    if(errorstate == FuriHalSdErrorNone) {
        /* While card is not ready for data and trial number for sending CMD13 is not exceeded */
        count = SDMMC_MAX_TRIAL;
        do {
            errorstate = SDMMC_CmdSendStatus(
                FURI_SDMMC_BLOCK, (uint32_t)(((uint32_t)sdmmc1.card_rca) << 16U));
            if(errorstate != FuriHalSdErrorNone) {
                break;
            }

            /* Get command response */
            response = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP1);
            count--;
        } while(((response & 0x100U) == 0U) && (count != 0U));

        /* Check the status after the switch command execution */
        if((count != 0U) && (errorstate == FuriHalSdErrorNone)) {
            /* Check the bit SWITCH_ERROR of the device status */
            if((response & 0x80U) != 0U) {
                errorstate = SDMMC_ERROR_UNSUPPORTED_FEATURE;
            } else {
                /* Configure high speed */
                init.ClockEdge = SDMMC_INIT_CLOCK_EDGE;
                init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE;
                init.BusWide = (FURI_SDMMC_BLOCK->CLKCR & SDMMC_CLKCR_WIDBUS);
                init.HardwareFlowControl = SDMMC_INIT_HARDWARE_FLOW_CONTROL;

                if(state == DISABLE) {
                    init.ClockDiv = sdmmc_clk / (2U * SD_INIT_FREQ);
                    (void)SDMMC_Init(FURI_SDMMC_BLOCK, init);

                    CLEAR_BIT(FURI_SDMMC_BLOCK->CLKCR, SDMMC_CLKCR_BUSSPEED);
                } else {
                    if(sdmmc_clk == 0U) {
                        errorstate = SDMMC_ERROR_INVALID_PARAMETER;
                    } else {
                        if(sdmmc_clk <= MMC_HIGH_SPEED_FREQ) {
                            init.ClockDiv = 0;
                        } else {
                            init.ClockDiv = (sdmmc_clk / (2U * MMC_HIGH_SPEED_FREQ)) + 1U;
                        }
                        (void)SDMMC_Init(FURI_SDMMC_BLOCK, init);

                        SET_BIT(FURI_SDMMC_BLOCK->CLKCR, SDMMC_CLKCR_BUSSPEED);
                    }
                }
            }
        } else if(count == 0U) {
            errorstate = SDMMC_ERROR_TIMEOUT;
        } else {
            /* Nothing to do */
        }
    }

    return errorstate;
}

static FuriHalSdError sdmmc_mmc_ddr_mode(FunctionalState state) {
    FuriHalSdError errorstate = FuriHalSdErrorNone;
    uint32_t response = 0U;
    uint32_t count;

    if(((FURI_SDMMC_BLOCK->CLKCR & SDMMC_CLKCR_DDR) != 0U) && (state == DISABLE)) {
        if((FURI_SDMMC_BLOCK->CLKCR & SDMMC_CLKCR_WIDBUS_0) != 0U) {
            errorstate = sdmm_mmc_pwr_class_update(SDMMC_BUS_WIDE_4B, SDMMC_SPEED_MODE_HIGH);
            if(errorstate != FuriHalSdErrorNone) {
                return errorstate;
            }

            /* Index : 183 - Value : 1 */
            errorstate = SDMMC_CmdSwitch(FURI_SDMMC_BLOCK, 0x03B70100U);

        } else {
            errorstate = sdmm_mmc_pwr_class_update(SDMMC_BUS_WIDE_8B, SDMMC_SPEED_MODE_HIGH);
            if(errorstate != FuriHalSdErrorNone) {
                return errorstate;
            }

            /* Index : 183 - Value : 2 */
            errorstate = SDMMC_CmdSwitch(FURI_SDMMC_BLOCK, 0x03B70200U);
        }
    }

    if(((FURI_SDMMC_BLOCK->CLKCR & SDMMC_CLKCR_DDR) == 0U) && (state != DISABLE)) {
        if((FURI_SDMMC_BLOCK->CLKCR & SDMMC_CLKCR_WIDBUS_0) != 0U) {
            errorstate = sdmm_mmc_pwr_class_update(SDMMC_BUS_WIDE_4B, SDMMC_SPEED_MODE_DDR);
            if(errorstate != FuriHalSdErrorNone) {
                return errorstate;
            }

            /* Index : 183 - Value : 5 */
            errorstate = SDMMC_CmdSwitch(FURI_SDMMC_BLOCK, 0x03B70500U);

        } else {
            errorstate = sdmm_mmc_pwr_class_update(SDMMC_BUS_WIDE_8B, SDMMC_SPEED_MODE_DDR);
            if(errorstate != FuriHalSdErrorNone) {
                return errorstate;
            }

            /* Index : 183 - Value : 6 */
            errorstate = SDMMC_CmdSwitch(FURI_SDMMC_BLOCK, 0x03B70600U);
        }
    }

    if(errorstate == FuriHalSdErrorNone) {
        /* While card is not ready for data and trial number for sending CMD13 is not exceeded */
        count = SDMMC_MAX_TRIAL;
        do {
            errorstate = SDMMC_CmdSendStatus(
                FURI_SDMMC_BLOCK, (uint32_t)(((uint32_t)sdmmc1.card_rca) << 16U));
            if(errorstate != FuriHalSdErrorNone) {
                break;
            }

            /* Get command response */
            response = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP1);
            count--;
        } while(((response & 0x100U) == 0U) && (count != 0U));

        /* Check the status after the switch command execution */
        if((count != 0U) && (errorstate == FuriHalSdErrorNone)) {
            /* Check the bit SWITCH_ERROR of the device status */
            if((response & 0x80U) != 0U) {
                errorstate = SDMMC_ERROR_UNSUPPORTED_FEATURE;
            } else {
                /* Configure DDR mode */
                if(state == DISABLE) {
                    CLEAR_BIT(FURI_SDMMC_BLOCK->CLKCR, SDMMC_CLKCR_DDR);
                } else {
                    SET_BIT(FURI_SDMMC_BLOCK->CLKCR, SDMMC_CLKCR_DDR);
                }
            }
        } else if(count == 0U) {
            errorstate = SDMMC_ERROR_TIMEOUT;
        } else {
            /* Nothing to do */
        }
    }

    return errorstate;
}

static FuriHalSdError sdmmc_mmc_config_speed_bus_mode(uint32_t sdmmc_clk) {
    FuriHalSdError errorstate = FuriHalSdErrorNone;

    /* Field DEVICE_TYPE [196] of Extended CSD register */
    uint32_t device_type = sdmmc1.csd_ext.DeviceType;

    // auto switch to high speed mode
    if(((FURI_SDMMC_BLOCK->CLKCR & SDMMC_CLKCR_WIDBUS) != 0U) && ((device_type & 0x04U) != 0U)) {
        /* High Speed DDR mode allowed */
        errorstate = sdmmc_mmc_high_speed(ENABLE, sdmmc_clk);
        if(errorstate != FuriHalSdErrorNone) {
            return errorstate;
        } else {
            if((FURI_SDMMC_BLOCK->CLKCR & SDMMC_CLKCR_CLKDIV) != 0U) {
                /* DDR mode not supported with CLKDIV = 0 */
                errorstate = sdmmc_mmc_ddr_mode(ENABLE);
                if(errorstate != FuriHalSdErrorNone) {
                    return errorstate;
                }
            }
        }
    } else if((device_type & 0x02U) != 0U) {
        /* High Speed mode allowed */
        errorstate = sdmmc_mmc_high_speed(ENABLE, sdmmc_clk);
        if(errorstate != FuriHalSdErrorNone) {
            return errorstate;
        }
    } else {
        /* Nothing to do : keep current speed */
    }

    /* Verify that MMC card is ready to use after Speed mode switch*/
    if(!sdmmc_wait_for_transfer_state(SDMMC_REAL_DATATIMEOUT)) {
        return FuriHalSdErrorTimeout;
    }

    return errorstate;
}

static FuriHalSdError sdmmc_mmc_wide_bus_mode(uint32_t WideMode) {
    uint32_t count;
    // SDMMC_InitTypeDef Init;
    FuriHalSdError errorstate;
    uint32_t response = 0U;

    /* Check and update the power class if needed */
    if((FURI_SDMMC_BLOCK->CLKCR & SDMMC_CLKCR_BUSSPEED) != 0U) {
        if((FURI_SDMMC_BLOCK->CLKCR & SDMMC_CLKCR_DDR) != 0U) {
            errorstate = sdmm_mmc_pwr_class_update(WideMode, SDMMC_SPEED_MODE_DDR);
        } else {
            errorstate = sdmm_mmc_pwr_class_update(WideMode, SDMMC_SPEED_MODE_HIGH);
        }
    } else {
        errorstate = sdmm_mmc_pwr_class_update(WideMode, SDMMC_SPEED_MODE_DEFAULT);
    }

    if(errorstate == FuriHalSdErrorNone) {
        if(WideMode == SDMMC_BUS_WIDE_8B) {
            errorstate = SDMMC_CmdSwitch(FURI_SDMMC_BLOCK, 0x03B70200U);
        } else if(WideMode == SDMMC_BUS_WIDE_4B) {
            errorstate = SDMMC_CmdSwitch(FURI_SDMMC_BLOCK, 0x03B70100U);
        } else if(WideMode == SDMMC_BUS_WIDE_1B) {
            errorstate = SDMMC_CmdSwitch(FURI_SDMMC_BLOCK, 0x03B70000U);
        } else {
            /* WideMode is not a valid argument*/
            errorstate = FuriHalSdErrorParam;
        }

        /* Check for switch error and violation of the trial number of sending CMD 13 */
        if(errorstate == FuriHalSdErrorNone) {
            /* While card is not ready for data and trial number for sending CMD13 is not exceeded */
            count = SDMMC_MAX_TRIAL;
            do {
                errorstate = SDMMC_CmdSendStatus(
                    FURI_SDMMC_BLOCK, (uint32_t)(((uint32_t)sdmmc1.card_rca) << 16U));
                if(errorstate != FuriHalSdErrorNone) {
                    break;
                }

                /* Get command response */
                response = SDMMC_GetResponse(FURI_SDMMC_BLOCK, SDMMC_RESP1);
                count--;
            } while(((response & 0x100U) == 0U) && (count != 0U));

            /* Check the status after the switch command execution */
            if((count != 0U) && (errorstate == FuriHalSdErrorNone)) {
                /* Check the bit SWITCH_ERROR of the device status */
                if((response & 0x80U) != 0U) {
                    errorstate = SDMMC_ERROR_GENERAL_UNKNOWN_ERR;
                } else {
                    /* Configure the SDMMC peripheral */
                    // Init = hmmc->Init;
                    // Init.BusWide = WideMode;
                    // (void)SDMMC_Init(FURI_SDMMC_BLOCK, Init);
                    uint32_t clkcr = FURI_SDMMC_BLOCK->CLKCR;
                    clkcr &= ~SDMMC_CLKCR_WIDBUS;
                    clkcr |= WideMode;
                    FURI_SDMMC_BLOCK->CLKCR = clkcr;
                }
            } else if(count == 0U) {
                errorstate = SDMMC_ERROR_TIMEOUT;
            } else {
                /* Nothing to do */
            }
        }
    }

    if(errorstate != FuriHalSdErrorNone) {
        /* Clear all the static flags */
        sdmmc_clear_static_flags();
    }

    return errorstate;
}

static bool sdmmc_init_mmc_lowspeed(uint32_t sdmmc_clk) {
    furi_assert(sdmmc_clk != 0U);

    // init params
    SDMMC_InitTypeDef init = {0};
    init.ClockEdge = SDMMC_INIT_CLOCK_EDGE;
    init.ClockPowerSave = SDMMC_INIT_CLOCK_POWER_SAVE;
    init.BusWide = SDMMC_BUS_WIDE_1B;
    init.HardwareFlowControl = SDMMC_INIT_HARDWARE_FLOW_CONTROL;

    // set init clock (400 kHz)
    init.ClockDiv = sdmmc_clk / (2U * SD_INIT_FREQ);

    // init sdmmc block
    SDMMC_Init(FURI_SDMMC_BLOCK, init);

    // power on sdmmc block
    SDMMC_PowerState_ON(FURI_SDMMC_BLOCK);

    // wait 74 sd clock cycles
    furi_delay_us((1000000 * 74) / (sdmmc_clk / (2U * init.ClockDiv)));

    // identify card operating voltage
    FuriHalSdError errorstate = sdmmc_mmc_power_on();
    if(errorstate != FuriHalSdErrorNone) {
        FURI_LOG_E(TAG, "sdmmc_mmc_power_on failed with error 0x%08x", errorstate);
        return false;
    }

    // initialize card
    errorstate = sdmmc_mmc_init_card();
    if(errorstate != FuriHalSdErrorNone) {
        FURI_LOG_E(TAG, "sdmmc_mmc_init_card failed with error 0x%08x", errorstate);
        return false;
    }

    // set block size for card
    errorstate = SDMMC_CmdBlockLength(FURI_SDMMC_BLOCK, SD_BLOCKSIZE);
    if(errorstate != FuriHalSdErrorNone) {
        sdmmc_clear_static_flags();
        FURI_LOG_E(TAG, "SDMMC_CmdBlockLength failed with error 0x%08x", errorstate);
        return false;
    }

    return true;
}

static bool sdmmc_init_mmc(uint32_t sdmmc_clk) {
    if(!sdmmc_init_mmc_lowspeed(sdmmc_clk)) {
        FURI_LOG_E(TAG, "sdmmc_init_mmc failed");
        return false;
    }

    if(sdmmc_mmc_wide_bus_mode(SDMMC_BUS_WIDE_4B) != FuriHalSdErrorNone) {
        FURI_LOG_E(TAG, "sdmmc_mmc_wide_bus_mode failed");
        return false;
    }

    // idk, what to set for eMMC?
    // sdmmc1.info.speed = FuriHalSdSpeedHigh;
    // sdmmc1.info.version = FuriHalSdVersion2;

    /* Configure the bus wide */
    if(sdmmc_mmc_config_speed_bus_mode(sdmmc_clk) != FuriHalSdErrorNone) {
        FURI_LOG_E(TAG, "sdmmc_mmc_config_speed_bus_mode failed");
        return false;
    }

    return true;
}

bool furi_hal_sdmmc_init_card(void) {
    sdmmc1.card_alive = false;

    // init sdmmc periph
    furi_hal_sdmmc_periph_init();

    // enable sdcard power
    furi_hal_sdmmc_card_enable_power();

    uint32_t sdmmc_clk = furi_hal_clock_get_freq(FuriHalClockHwSdMmc12);

    if(sdmmc_init_mmc(sdmmc_clk)) {
        sdmmc1.card_alive = true;
        return true;
    } else {
        FURI_LOG_E(TAG, "MMC init failed, trying SD card");
        if(sdmmc_init_sdcard(sdmmc_clk)) {
            sdmmc1.card_alive = true;
            return true;
        } else {
            FURI_LOG_E(TAG, "SD card init failed");
            return false;
        }
    }
}

void furi_hal_sdmmc_deinit_card(void) {
    furi_hal_sdmmc_card_disable_power();

    furi_hal_sdmmc_periph_deinit();

    FURI_LOG_I(TAG, "Card deinit OK");
    sdmmc1.card_alive = false;
}

bool furi_hal_sdmmc_read_blocks(
    uint8_t* buffer,
    uint32_t address,
    uint32_t count,
    size_t timeout_ms) {
    if((address + count) > (sdmmc1.info.logical_block_count)) {
        FURI_LOG_E(TAG, "Address out of range");
        return false;
    }

    sdmmc_disable_it(
        SDMMC_IT_DATAEND | SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR |
        SDMMC_IT_RXOVERR);
    sdmmc_clear_static_flags();

    furi_event_flag_clear(sdmmc_dma_context.event, SdMmcDmaEventComplete | SdMmcDmaEventError);
    furi_hal_interrupt_set_isr(FuriHalInterruptIdSdMmc1, sdmmc_irq_handler, &sdmmc_dma_context);

    bool status = sdmmc_read_blocks_dma(buffer, address, count);

    uint32_t flags = furi_event_flag_wait(
        sdmmc_dma_context.event,
        SdMmcDmaEventComplete | SdMmcDmaEventError,
        FuriFlagWaitAny,
        timeout_ms);

    furi_hal_interrupt_set_isr(FuriHalInterruptIdSdMmc1, NULL, NULL);

    if((flags == FuriFlagErrorTimeout) || (flags & SdMmcDmaEventError)) {
        if(flags == FuriFlagErrorTimeout) {
            FURI_LOG_E(TAG, "sdmmc_read_blocks_dma failed with timeout");
        } else {
            FURI_LOG_E(
                TAG, "sdmmc_read_blocks_dma failed with error 0x%08x", sdmmc_dma_context.error);
        }
        sdmmc_dma_context.error = FuriHalSdErrorNone;
        status = false;
        sdmmc_disable_it(
            SDMMC_IT_DATAEND | SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR |
            SDMMC_IT_RXOVERR);
    }

    if(status) {
        status = sdmmc_wait_for_transfer_state(timeout_ms);
        if(!status) {
            FURI_LOG_E(TAG, "sdmmc_wait_for_transfer_state failed");
        }
    }

    if(!status) {
        sdmmc1.card_alive = false;
    }

    return status;
}

bool furi_hal_sdmmc_write_blocks(
    const uint8_t* buffer,
    uint32_t address,
    uint32_t count,
    size_t timeout_ms) {
    if((address + count) > (sdmmc1.info.logical_block_count)) {
        FURI_LOG_E(TAG, "Address out of range");
        return false;
    }

    sdmmc_disable_it(
        SDMMC_IT_DATAEND | SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR |
        SDMMC_IT_RXOVERR);
    sdmmc_clear_static_flags();

    furi_event_flag_clear(sdmmc_dma_context.event, SdMmcDmaEventComplete | SdMmcDmaEventError);
    furi_hal_interrupt_set_isr(FuriHalInterruptIdSdMmc1, sdmmc_irq_handler, &sdmmc_dma_context);

    bool status = sdmmc_write_blocks_dma(buffer, address, count);

    uint32_t flags = furi_event_flag_wait(
        sdmmc_dma_context.event,
        SdMmcDmaEventComplete | SdMmcDmaEventError,
        FuriFlagWaitAny,
        timeout_ms);

    furi_hal_interrupt_set_isr(FuriHalInterruptIdSdMmc1, NULL, NULL);

    if((flags == FuriFlagErrorTimeout) || (flags & SdMmcDmaEventError)) {
        if(flags == FuriFlagErrorTimeout) {
            FURI_LOG_E(TAG, "sdmmc_write_blocks_dma failed with timeout");
        } else {
            FURI_LOG_E(
                TAG, "sdmmc_write_blocks_dma failed with error 0x%08x", sdmmc_dma_context.error);
        }

        sdmmc_dma_context.error = FuriHalSdErrorNone;
        status = false;
        sdmmc_disable_it(
            SDMMC_IT_DATAEND | SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR |
            SDMMC_IT_RXOVERR);
    }

    if(status) {
        status = sdmmc_wait_for_transfer_state(timeout_ms);
        if(!status) {
            FURI_LOG_E(TAG, "sdmmc_wait_for_transfer_state failed");
        }
    }

    if(!status) {
        sdmmc1.card_alive = false;
    }

    return status;
}

bool furi_hal_sdmmc_get_card_info(FuriHalSdInfo* info) {
    furi_check(info);
    memcpy(info, &sdmmc1.info, sizeof(FuriHalSdInfo));
    return true;
}

bool furi_hal_sd_alive(void) {
    return sdmmc1.card_alive;
}
