#include <furi.h>
#include <furi_hal_rtc.h>
// #include <stm32u5xx_ll_rcc.h>
// #include <stm32u5xx_ll_rtc.h>
//#include <stm32u5xx_ll_bus.h>
#include <furi_hal_bus.h>
#include <stm32u5xx_ll_pwr.h>
#include <stm32u5xx_ll_rtc.h>
#include <stm32u5xx_ll_utils.h>
#include <stm32u5xx_ll_rcc.h>
#include <stm32u5xx_ll_bus.h>

#define TAG "FuriHalRtc"

#define FURI_HAL_RTC_LSE_STARTUP_TIME 300

#define FURI_HAL_RTC_CLOCK_IS_READY() (LL_RCC_LSE_IsReady() && LL_RCC_LSI_IsReady())

#define FURI_HAL_RTC_HEADER_MAGIC   0x10F1
#define FURI_HAL_RTC_HEADER_VERSION 0

uint32_t furi_hal_rtc_get_timestamp(void) {
    return 0;
}

static bool furi_hal_rtc_start_clock_and_switch(void) {
    uint32_t tmpregister;
    /* Enable write access to Backup domain */
    LL_PWR_EnableBkUpAccess();
    /* Wait for Backup domain Write protection disable */
    LL_PWR_DisableBkUpAccess();
    LL_mDelay(2);

    /* Reset the Backup domain only if the RTC Clock source selection is modified from default */
    if(LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE) {
        /* Store the content of BDCR register before the reset of Backup Domain */
        tmpregister = READ_BIT(RCC->BDCR, ~(RCC_BDCR_RTCSEL));
        /* RTC Clock selection can be changed only if the Backup Domain is reset */
        LL_RCC_ForceBackupDomainReset();
        LL_RCC_ReleaseBackupDomainReset();
        /* Restore the Content of BDCR register */
        RCC->BDCR = tmpregister;
    }

    // /* Wait for LSE reactivation if LSE was enable prior to Backup Domain reset */

    // Enable LSI and LSE
    LL_RCC_LSI_Enable();
    LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_HIGH);
    LL_RCC_LSE_Enable();

    // Wait for LSI and LSE startup
    uint32_t c = 0;
    while(!FURI_HAL_RTC_CLOCK_IS_READY() && c < FURI_HAL_RTC_LSE_STARTUP_TIME) {
        LL_mDelay(1);
        c++;
    }

    if(FURI_HAL_RTC_CLOCK_IS_READY()) {
        LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
        LL_RCC_EnableRTC();
        return LL_RCC_GetRTCClockSource() == LL_RCC_RTC_CLKSOURCE_LSE;
    } else {
        return false;
    }
}

static void furi_hal_rtc_reset(void) {
    LL_RCC_ForceBackupDomainReset();
    LL_RCC_ReleaseBackupDomainReset();
}

static void furi_hal_rtc_recover(void) {
    DateTime datetime = {0};

    // Handle fixable LSE failure
    if(LL_RCC_LSE_IsCSSDetected()) {
        //furi_hal_light_sequence("rgb B");
        // Shutdown LSE and LSECSS
        LL_RCC_LSE_DisableCSS();
        LL_RCC_LSE_Disable();
    } else {
        // furi_hal_light_sequence("rgb R");
    }

    // Temporary switch to LSI
    LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSI);
    if(LL_RCC_GetRTCClockSource() == LL_RCC_RTC_CLKSOURCE_LSI) {
        // Get datetime before RTC Domain reset
        furi_hal_rtc_get_datetime(&datetime);
    }

    // Reset RTC Domain
    furi_hal_rtc_reset();

    // Start Clock
    if(!furi_hal_rtc_start_clock_and_switch()) {
        // Plan C: reset RTC and restart
        //furi_hal_light_sequence("rgb R.r.R.r.R.r");
        furi_hal_rtc_reset();
        NVIC_SystemReset();
    }

    // Set date if it valid
    if(datetime.year != 0) {
        furi_hal_rtc_set_datetime(&datetime);
    }
}

void furi_hal_rtc_init_early(void) {
    // Enable RTCAPB clock
    //furi_hal_bus_enable(FuriHalBusRTCAPB);
    // Enable RTCAPB clock
    LL_APB1_GRP1_EnableClock(LL_APB3_GRP1_PERIPH_RTCAPB);

    // Prepare clock
    if(!furi_hal_rtc_start_clock_and_switch()) {
        // Plan B: try to recover
        furi_hal_rtc_recover();
    }
    LL_SRDAMR_GRP1_EnableAutonomousClock(LL_SRDAMR_GRP1_PERIPH_RTCAPBAMEN);
}

void furi_hal_rtc_deinit_early(void) {
}

void furi_hal_rtc_init(void) {
    LL_RTC_InitTypeDef RTC_InitStruct;
    RTC_InitStruct.HourFormat = LL_RTC_HOURFORMAT_24HOUR;
    RTC_InitStruct.AsynchPrescaler = 127;
    RTC_InitStruct.SynchPrescaler = 255;
    LL_RTC_Init(RTC, &RTC_InitStruct);

    // furi_log_set_level(furi_hal_rtc_get_log_level());
    // furi_hal_serial_control_set_logging_config(
    //     furi_hal_rtc_log_devices[furi_hal_rtc_get_log_device()],
    //     furi_hal_rtc_log_baud_rates[furi_hal_rtc_get_log_baud_rate()]);

    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_rtc_get_datetime(DateTime* datetime) {
    furi_check(!FURI_IS_IRQ_MODE());
    furi_check(datetime);

    FURI_CRITICAL_ENTER();
    uint32_t time = LL_RTC_TIME_Get(RTC); // 0x00HHMMSS
    uint32_t date = LL_RTC_DATE_Get(RTC); // 0xWWDDMMYY
    FURI_CRITICAL_EXIT();

    datetime->second = __LL_RTC_CONVERT_BCD2BIN((time >> 0) & 0xFF);
    datetime->minute = __LL_RTC_CONVERT_BCD2BIN((time >> 8) & 0xFF);
    datetime->hour = __LL_RTC_CONVERT_BCD2BIN((time >> 16) & 0xFF);
    datetime->year = __LL_RTC_CONVERT_BCD2BIN((date >> 0) & 0xFF) + 2000;
    datetime->month = __LL_RTC_CONVERT_BCD2BIN((date >> 8) & 0xFF);
    datetime->day = __LL_RTC_CONVERT_BCD2BIN((date >> 16) & 0xFF);
    datetime->weekday = __LL_RTC_CONVERT_BCD2BIN((date >> 24) & 0xFF);
}

void furi_hal_rtc_sync_shadow(void) {
    if(!LL_RTC_IsShadowRegBypassEnabled(RTC)) {
        LL_RTC_ClearFlag_RS(RTC);
        while(!LL_RTC_IsActiveFlag_RS(RTC)) {
        };
    }
}

void furi_hal_rtc_set_datetime(DateTime* datetime) {
    furi_check(!FURI_IS_IRQ_MODE());
    furi_check(datetime);

    FURI_CRITICAL_ENTER();
    /* Disable write protection */
    LL_RTC_DisableWriteProtection(RTC);

    /* Enter Initialization mode and wait for INIT flag to be set */
    LL_RTC_EnableInitMode(RTC);
    while(!LL_RTC_IsActiveFlag_INIT(RTC)) {
    }

    /* Set time */
    LL_RTC_TIME_Config(
        RTC,
        LL_RTC_TIME_FORMAT_AM_OR_24,
        __LL_RTC_CONVERT_BIN2BCD(datetime->hour),
        __LL_RTC_CONVERT_BIN2BCD(datetime->minute),
        __LL_RTC_CONVERT_BIN2BCD(datetime->second));

    /* Set date */
    LL_RTC_DATE_Config(
        RTC,
        datetime->weekday,
        __LL_RTC_CONVERT_BIN2BCD(datetime->day),
        __LL_RTC_CONVERT_BIN2BCD(datetime->month),
        __LL_RTC_CONVERT_BIN2BCD(datetime->year - 2000));

    /* Exit Initialization mode */
    LL_RTC_DisableInitMode(RTC);

    furi_hal_rtc_sync_shadow();

    /* Enable write protection */
    LL_RTC_EnableWriteProtection(RTC);
    FURI_CRITICAL_EXIT();
}