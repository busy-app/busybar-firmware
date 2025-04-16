#include <furi_hal.h>
#include <furi_hal_interrupt.h>
#include <stm32u5xx_ll_pwr.h>

void furi_hal_usb_init(void) {
    // USB Clock
    LL_RCC_SetUSBPHYClockSource(LL_RCC_USBPHYCLKSOURCE_HSE);
    // TODO: HSE value
    MODIFY_REG(
        SYSCFG->OTGHSPHYCR,
        SYSCFG_OTGHSPHYCR_CLKSEL,
        SYSCFG_OTGHSPHYCR_CLKSEL_0 | SYSCFG_OTGHSPHYCR_CLKSEL_1);

    furi_hal_gpio_init_ex(
        &gpio_usb_dm, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedHigh, GpioAltFn10USB_HS);
    furi_hal_gpio_init_ex(
        &gpio_usb_dp, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedHigh, GpioAltFn10USB_HS);

    furi_hal_bus_enable(FuriHalBusOTG_HS);
    furi_hal_bus_enable(FuriHalBusUSBPHY);

    LL_PWR_EnableVddUSB();
    LL_PWR_EnableUSBPowerSupply();
    LL_PWR_EnableUSBEPODBooster();

    // Configuring the SYSCFG registers OTG_HS PHY
    SYSCFG->OTGHSPHYCR |= SYSCFG_OTGHSPHYCR_EN;

    // Disable VBUS sense (B device)
    USB_OTG_HS->GCCFG &= ~USB_OTG_GCCFG_VBDEN;

    // B-peripheral session valid override enable
    USB_OTG_HS->GCCFG |= USB_OTG_GCCFG_VBVALEXTOEN;
    USB_OTG_HS->GCCFG |= USB_OTG_GCCFG_VBVALOVAL;
}

void furi_hal_usb_set_irq(FuriHalInterruptISR usb_isr, void* isr_ctx) {
    furi_hal_interrupt_set_isr_ex(
        FuriHalInterruptIdUSBHS, FuriHalInterruptPriorityNormal, usb_isr, isr_ctx);
}
