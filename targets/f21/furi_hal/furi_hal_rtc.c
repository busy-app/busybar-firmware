#include <furi.h>
#include <furi_hal_rtc.h>
#include <furi_hal_cortex.h>

#include <stm32u5xx_ll_pwr.h>
#include <stm32u5xx_ll_rtc.h>
#include <stm32u5xx_ll_rcc.h>
#include <stm32u5xx_ll_bus.h>

#define TAG "FuriHalRtc"

#define FURI_HAL_RTC_HEADER_MAGIC    0x10F1
#define FURI_HAL_RTC_SYNC_TIMEOUT_US (1000 * 1000)

static void furi_hal_rtc_start_clock_and_switch(void) {
    // Check if the RTC clock source is already LSE
    if(LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE) {
        FlagStatus pwrclkchanged = RESET;
        /* Update LSE configuration in Backup Domain control register */
        /* Requires to enable write access to Backup Domain if necessary */
        if(LL_AHB3_GRP1_IsEnabledClock(LL_AHB3_GRP1_PERIPH_PWR) != 1U) {
            /* Enables the PWR Clock and Enables access to the backup domain */
            LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_PWR);
            pwrclkchanged = SET;
        }
        if(LL_PWR_IsEnabledBkUpAccess() != 1U) {
            /* Enable write access to Backup domain */
            LL_PWR_EnableBkUpAccess();
            while(LL_PWR_IsEnabledBkUpAccess() == 0U) {
            }
        }
        LL_RCC_ForceBackupDomainReset();
        LL_RCC_ReleaseBackupDomainReset();
        LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_LOW);
        LL_RCC_LSE_Enable();

        /* Wait till LSE is ready */
        while(LL_RCC_LSE_IsReady() != 1) {
        }
        LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
        /* Restore clock configuration if changed */
        if(pwrclkchanged == SET) {
            LL_AHB3_GRP1_DisableClock(LL_AHB3_GRP1_PERIPH_PWR);
        }
    }
}

void furi_hal_rtc_clock_enable(void) {
    /* Peripheral clock enable */
    LL_RCC_EnableRTC();
    LL_APB3_GRP1_EnableClock(LL_APB3_GRP1_PERIPH_RTCAPB);
    LL_SRDAMR_GRP1_EnableAutonomousClock(LL_SRDAMR_GRP1_PERIPH_RTCAPBAMEN);
}

void furi_hal_rtc_init_early(void) {
    if(LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE) {
        // Start LSE and switch to it
        furi_hal_rtc_start_clock_and_switch();
    }
    // Enable RTC clock
    furi_hal_rtc_clock_enable();
}

void furi_hal_rtc_deinit_early(void) {
}

void furi_hal_rtc_init(void) {
    LL_RTC_InitTypeDef RTC_InitStruct;
    RTC_InitStruct.HourFormat = LL_RTC_HOURFORMAT_24HOUR;
    RTC_InitStruct.AsynchPrescaler = 127;
    RTC_InitStruct.SynchPrescaler = 255;
    LL_RTC_Init(RTC, &RTC_InitStruct);
    LL_RTC_SetBackupRegisterPrivilege(RTC, LL_RTC_PRIVILEGE_BKUP_ZONE_NONE);
    LL_RTC_SetBackupRegProtection(RTC, LL_RTC_BKP_DR0, LL_RTC_BKP_DR0);
    LL_RTC_SetRtcPrivilege(RTC, LL_RTC_PRIVILEGE_FULL_NO);

    if(LL_RTC_BKP_GetRegister(RTC, LL_RTC_BKP_DR0) != FURI_HAL_RTC_HEADER_MAGIC) {
        LL_RTC_BKP_SetRegister(RTC, LL_RTC_BKP_DR0, FURI_HAL_RTC_HEADER_MAGIC);
        DateTimeMs datetime = {
            .dt =
                {
                    .date = utz_date_init(2026, 1, 1),
                    .time = {0},
                },
            .millis = 0,
        };
        furi_hal_rtc_set_datetime(&datetime);
    }
    FURI_LOG_I(TAG, "Init OK");
}

DateTimeMs furi_hal_rtc_get_datetime(void) {
    furi_check(!FURI_IS_IRQ_MODE());

    uint32_t prescaler, sub_second, time, date;

    for(;;) {
        /* only check timeout for consecutive RS wait cycles */
        FuriHalCortexTimer timeout_timer = furi_hal_cortex_timer_get(FURI_HAL_RTC_SYNC_TIMEOUT_US);
        while(!LL_RTC_IsActiveFlag_RS(RTC)) {
            furi_thread_yield();

            if(furi_hal_cortex_timer_is_expired(timeout_timer)) {
                furi_crash("RTC sync timeout: RS flag not set in time");
            }
        }

        FURI_CRITICAL_ENTER();

        if(LL_RTC_IsActiveFlag_RS(RTC)) {
            prescaler = LL_RTC_GetSynchPrescaler(RTC);
            sub_second = LL_RTC_TIME_GetSubSecond(RTC);
            time = LL_RTC_TIME_Get(RTC); /* xx-HH-MM-SS */
            date = LL_RTC_DATE_Get(RTC); /* WW-DD-MM-YY */

            LL_RTC_ClearFlag_RS(RTC);

            FURI_CRITICAL_EXIT();
            break;
        }

        FURI_CRITICAL_EXIT();
    }

    DateTimeMs datetime;

    datetime.dt.second = __LL_RTC_CONVERT_BCD2BIN((time >> 0) & 0xFF);
    datetime.dt.minute = __LL_RTC_CONVERT_BCD2BIN((time >> 8) & 0xFF);
    datetime.dt.hour = __LL_RTC_CONVERT_BCD2BIN((time >> 16) & 0xFF);
    datetime.dt.year = __LL_RTC_CONVERT_BCD2BIN((date >> 0) & 0xFF) + 2000;
    datetime.dt.month = __LL_RTC_CONVERT_BCD2BIN((date >> 8) & 0xFF);
    datetime.dt.dayofmonth = __LL_RTC_CONVERT_BCD2BIN((date >> 16) & 0xFF);
    datetime.dt.dayofweek = __LL_RTC_CONVERT_BCD2BIN((date >> 24) & 0xFF);

    /* special case for getting time right after set operation */
    if(sub_second > prescaler) {
        time_t timestamp = datetime_datetime_to_timestamp(&datetime.dt);
        datetime.dt = datetime_timestamp_to_datetime(timestamp - 1);

        sub_second %= (prescaler + 1);
    }

    datetime.millis = 1000 * (prescaler - sub_second) / (prescaler + 1);
    return datetime;
}

time_t furi_hal_rtc_get_timestamp(void) {
    DateTimeMs datetime = furi_hal_rtc_get_datetime();

    return datetime_datetime_to_timestamp(&datetime.dt);
}

time_t furi_hal_rtc_get_timestamp_ms(void) {
    DateTimeMs datetime = furi_hal_rtc_get_datetime();

    return datetime_datetime_to_timestamp_ms(&datetime);
}

void furi_hal_rtc_set_datetime(const DateTimeMs* datetime) {
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
        __LL_RTC_CONVERT_BIN2BCD(datetime->dt.hour),
        __LL_RTC_CONVERT_BIN2BCD(datetime->dt.minute),
        __LL_RTC_CONVERT_BIN2BCD(datetime->dt.second));

    /* Set date */
    LL_RTC_DATE_Config(
        RTC,
        datetime->dt.dayofweek,
        __LL_RTC_CONVERT_BIN2BCD(datetime->dt.dayofmonth),
        __LL_RTC_CONVERT_BIN2BCD(datetime->dt.month),
        __LL_RTC_CONVERT_BIN2BCD(datetime->dt.year - 2000));

    /* Exit Initialization mode */
    LL_RTC_DisableInitMode(RTC);
    while(!LL_RTC_IsActiveFlag_RS(RTC)) {
    }

    /* Pend milliseconds set */
    uint32_t prescaler = LL_RTC_GetSynchPrescaler(RTC);
    uint32_t shift = prescaler - datetime->millis * (prescaler + 1) / 1000;
    LL_RTC_TIME_Synchronize(RTC, LL_RTC_SHIFT_SECOND_ADVANCE, shift);

    /* Enable write protection */
    LL_RTC_EnableWriteProtection(RTC);
    FURI_CRITICAL_EXIT();
}
