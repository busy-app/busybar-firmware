#include <furi_hal_usb.h>

#include <furi_hal_bus.h>
#include <furi_hal_cortex.h>
#include <furi_hal_interrupt.h>
#include <furi_hal_resources.h>

#include <stm32u5xx_ll_rcc.h>
#include <stm32u5xx_ll_pwr.h>

#define USB_OTG_DEV     ((USB_OTG_DeviceTypeDef*)(USB_OTG_HS_BASE + USB_OTG_DEVICE_BASE))
#define USB_OTG_PCGCCTL (*(volatile uint32_t*)(USB_OTG_HS_BASE + USB_OTG_PCGCCTL_BASE))

#define USB_RESET_TIMEOUT_US (10000U)

static void furi_hal_usb_disable_global_interrupt(void) {
    CLEAR_BIT(USB_OTG_HS->GAHBCFG, USB_OTG_GAHBCFG_GINT);
}

static void furi_hal_usb_wait_ahb_idle(void) {
    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(USB_RESET_TIMEOUT_US);

    do {
        if(READ_BIT(USB_OTG_HS->GRSTCTL, USB_OTG_GRSTCTL_AHBIDL) != 0) {
            break;
        }
    } while(!furi_hal_cortex_timer_is_expired(timer));
}

static void furi_hal_usb_core_reset(void) {
    furi_hal_usb_wait_ahb_idle();

    SET_BIT(USB_OTG_HS->GRSTCTL, USB_OTG_GRSTCTL_CSRST);

    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(USB_RESET_TIMEOUT_US);

    do {
        if(READ_BIT(USB_OTG_HS->GRSTCTL, USB_OTG_GRSTCTL_CSRST) == 0) {
            break;
        }
    } while(!furi_hal_cortex_timer_is_expired(timer));

    furi_hal_usb_wait_ahb_idle();
}

static void furi_hal_usb_flush_fifos(void) {
    furi_hal_usb_wait_ahb_idle();

    SET_BIT(
        USB_OTG_HS->GRSTCTL,
        USB_OTG_GRSTCTL_RXFFLSH | USB_OTG_GRSTCTL_TXFFLSH | USB_OTG_GRSTCTL_TXFNUM_4);

    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(USB_RESET_TIMEOUT_US);

    do {
        if(READ_BIT(USB_OTG_HS->GRSTCTL, USB_OTG_GRSTCTL_RXFFLSH | USB_OTG_GRSTCTL_TXFFLSH) == 0) {
            break;
        }
    } while(!furi_hal_cortex_timer_is_expired(timer));
}

void furi_hal_usb_init(void) {
    // USB Clock
    LL_RCC_SetUSBPHYClockSource(LL_RCC_USBPHYCLKSOURCE_HSE);
    // TODO: HSE value
    MODIFY_REG(
        SYSCFG->OTGHSPHYCR,
        SYSCFG_OTGHSPHYCR_CLKSEL,
        SYSCFG_OTGHSPHYCR_CLKSEL_0 | SYSCFG_OTGHSPHYCR_CLKSEL_1);

    furi_hal_bus_enable(FuriHalBusOTG_HS);
    furi_hal_bus_enable(FuriHalBusUSBPHY);

    LL_PWR_EnableVddUSB();
    LL_PWR_EnableUSBPowerSupply();
    LL_PWR_EnableUSBEPODBooster();

    furi_hal_usb_disable_global_interrupt();
    furi_hal_usb_core_reset();

    furi_hal_gpio_init_ex(
        &gpio_usb_dm, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedHigh, GpioAltFn10USB_HS);
    furi_hal_gpio_init_ex(
        &gpio_usb_dp, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedHigh, GpioAltFn10USB_HS);

    // Configuring the SYSCFG registers OTG_HS PHY
    SYSCFG->OTGHSPHYCR |= SYSCFG_OTGHSPHYCR_EN;

    // Disable VBUS sense (B device)
    USB_OTG_HS->GCCFG &= ~USB_OTG_GCCFG_VBDEN;
    USB_OTG_HS->GCCFG &= ~USB_OTG_GCCFG_PULLDOWNEN;

    // B-peripheral session valid override enable
    USB_OTG_HS->GCCFG |= USB_OTG_GCCFG_VBVALEXTOEN;
    USB_OTG_HS->GCCFG |= USB_OTG_GCCFG_VBVALOVAL;

    USB_OTG_DEV->DCTL |= USB_OTG_DCTL_SDIS;

    CLEAR_REG(USB_OTG_PCGCCTL);

    furi_hal_usb_flush_fifos();

    CLEAR_REG(USB_OTG_DEV->DIEPMSK);
    CLEAR_REG(USB_OTG_DEV->DOEPMSK);
    CLEAR_REG(USB_OTG_DEV->DAINTMSK);

    CLEAR_REG(USB_OTG_HS->GINTMSK);
    WRITE_REG(USB_OTG_HS->GINTSTS, 0xBFFFFFFFUL);
}

void furi_hal_usb_set_irq(FuriHalInterruptISR usb_isr, void* isr_ctx) {
    furi_hal_interrupt_set_isr_ex(
        FuriHalInterruptIdUSBHS, FuriHalInterruptPriorityNormal, usb_isr, isr_ctx);
}
