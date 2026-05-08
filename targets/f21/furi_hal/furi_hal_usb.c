#include <furi_hal_usb.h>

#include <furi_hal_bus.h>
#include <furi_hal_interrupt.h>
#include <furi_hal_resources.h>

#include <stm32u5xx_ll_rcc.h>
#include <stm32u5xx_ll_pwr.h>

#include <toolbox/timers.h>

#define USB_OTG_DEV ((USB_OTG_DeviceTypeDef*)(USB_OTG_HS_BASE + USB_OTG_DEVICE_BASE))

#define USB_TIMEOUT_US (10000U)

#define TAG "FuriHalUsb"

static bool furi_hal_usb_wait_for_condition(TimerConditionCallback callback) {
    PreciseTimer timer = precise_timer_create(USB_TIMEOUT_US);
    return precise_timer_wait_for(timer, callback, NULL);
}

static void furi_hal_usb_disable_global_interrupt(void) {
    CLEAR_BIT(USB_OTG_HS->GAHBCFG, USB_OTG_GAHBCFG_GINT);
}

static bool furi_hal_usb_is_ahb_idle(void* context) {
    UNUSED(context);
    return READ_BIT(USB_OTG_HS->GRSTCTL, USB_OTG_GRSTCTL_AHBIDL) != 0;
}

static bool furi_hal_usb_is_core_reset(void* context) {
    UNUSED(context);
    return READ_BIT(USB_OTG_HS->GRSTCTL, USB_OTG_GRSTCTL_CSRST) == 0;
}

static bool furi_hal_usb_core_reset(void) {
    bool success = false;

    do {
        if(!furi_hal_usb_wait_for_condition(furi_hal_usb_is_ahb_idle)) {
            FURI_LOG_E(TAG, "AHB is not idle before core reset");
            break;
        }

        SET_BIT(USB_OTG_HS->GRSTCTL, USB_OTG_GRSTCTL_CSRST);

        if(!furi_hal_usb_wait_for_condition(furi_hal_usb_is_core_reset)) {
            FURI_LOG_E(TAG, "Failed to reset USB core");
            break;
        }

        if(!furi_hal_usb_wait_for_condition(furi_hal_usb_is_ahb_idle)) {
            FURI_LOG_E(TAG, "AHB is not idle after core reset");
            break;
        }

        success = true;
    } while(false);

    return success;
}

static bool furi_hal_usb_is_epod_booster_enabled(void* context) {
    UNUSED(context);
    return LL_PWR_IsActiveFlag_BOOST() != 0;
}

static bool furi_hal_usb_is_usbepod_booster_enabled(void* context) {
    UNUSED(context);
    return LL_PWR_IsActiveFlag_USBBOOST() != 0;
}

static bool furi_hal_usb_enable_power(void) {
    bool success = false;

    do {
        LL_PWR_EnableVddUSB();
        LL_PWR_EnableEPODBooster();

        if(!furi_hal_usb_wait_for_condition(furi_hal_usb_is_epod_booster_enabled)) {
            FURI_LOG_E(TAG, "Failed to enable EPOD booster");
            break;
        }

        LL_PWR_EnableUSBPowerSupply();
        LL_PWR_EnableUSBEPODBooster();

        if(!furi_hal_usb_wait_for_condition(furi_hal_usb_is_usbepod_booster_enabled)) {
            FURI_LOG_E(TAG, "Failed to enable USBEPOD booster");
            break;
        }

        success = true;
    } while(false);

    return success;
}

static void furi_hal_usb_enable_phy(void) {
    LL_RCC_SetUSBPHYClockSource(LL_RCC_USBPHYCLKSOURCE_HSE);

    MODIFY_REG(
        SYSCFG->OTGHSPHYCR,
        SYSCFG_OTGHSPHYCR_CLKSEL | SYSCFG_OTGHSPHYCR_EN,
        SYSCFG_OTGHSPHYCR_CLKSEL_0 | SYSCFG_OTGHSPHYCR_CLKSEL_1 | SYSCFG_OTGHSPHYCR_EN);

    furi_hal_gpio_init_ex(
        &gpio_usb_dm, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedHigh, GpioAltFn10USB_HS);
    furi_hal_gpio_init_ex(
        &gpio_usb_dp, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedHigh, GpioAltFn10USB_HS);
}

static void furi_hal_usb_disable_vbus_sense(void) {
    CLEAR_BIT(USB_OTG_HS->GCCFG, USB_OTG_GCCFG_PULLDOWNEN | USB_OTG_GCCFG_VBDEN);
}

static void furi_hal_usb_set_b_session_override(void) {
    SET_BIT(USB_OTG_HS->GCCFG, USB_OTG_GCCFG_VBVALEXTOEN | USB_OTG_GCCFG_VBVALOVAL);
}

static void furi_hal_usb_set_software_disconnect(void) {
    SET_BIT(USB_OTG_DEV->DCTL, USB_OTG_DCTL_SDIS);
}

void furi_hal_usb_init(void) {
    do {
        furi_hal_bus_enable(FuriHalBusUSBPHY);
        furi_hal_bus_enable(FuriHalBusOTG_HS);

        furi_hal_usb_enable_phy();

        if(!furi_hal_usb_enable_power()) {
            break;
        }

        furi_hal_usb_disable_global_interrupt();

        if(!furi_hal_usb_core_reset()) {
            break;
        }

        furi_hal_usb_disable_vbus_sense();
        furi_hal_usb_set_b_session_override();
        furi_hal_usb_set_software_disconnect();

        FURI_LOG_I(TAG, "Init OK");
    } while(false);
}

void furi_hal_usb_set_irq(FuriHalInterruptISR usb_isr, void* isr_ctx) {
    furi_hal_interrupt_set_isr_ex(
        FuriHalInterruptIdUSBHS, FuriHalInterruptPriorityNormal, usb_isr, isr_ctx);
}
