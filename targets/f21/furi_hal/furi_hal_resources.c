#include <furi_hal_resources.h>
#include <furi_hal_bus.h>
#include <stm32u5xx_ll_rcc.h>

static inline void furi_hal_nvic_enable(IRQn_Type irqn) {
    NVIC_SetPriority(irqn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 10, 0));
    NVIC_EnableIRQ(irqn);
}

void furi_hal_resources_init_early(void) {
    furi_hal_bus_enable(FuriHalBusGPIOA);
    furi_hal_bus_enable(FuriHalBusGPIOB);
    furi_hal_bus_enable(FuriHalBusGPIOC);
    furi_hal_bus_enable(FuriHalBusGPIOD);
    furi_hal_bus_enable(FuriHalBusGPIOE);
#if defined(GPIOF)
    furi_hal_bus_enable(FuriHalBusGPIOF);
#endif
    furi_hal_bus_enable(FuriHalBusGPIOG);
    furi_hal_bus_enable(FuriHalBusGPIOH);
#if defined(GPIOI)
    furi_hal_bus_enable(FuriHalBusGPIOI);
#endif
#if defined(GPIOJ)
    furi_hal_bus_enable(FuriHalBusGPIOJ);
#endif

    // SD Card stepdown control TODO:
    // furi_hal_gpio_write(&gpio_sd_card_power_switch, 0);
    // furi_hal_gpio_init(
    //     &gpio_sd_card_power_switch, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);

    // SD Card detect
    // furi_hal_gpio_init(&gpio_sd_card_detect, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    // Master Clock Output

    furi_hal_nvic_enable(EXTI0_IRQn);
    furi_hal_nvic_enable(EXTI1_IRQn);
    furi_hal_nvic_enable(EXTI2_IRQn);
    furi_hal_nvic_enable(EXTI3_IRQn);
    furi_hal_nvic_enable(EXTI4_IRQn);
    furi_hal_nvic_enable(EXTI5_IRQn);
    furi_hal_nvic_enable(EXTI6_IRQn);
    furi_hal_nvic_enable(EXTI7_IRQn);
    furi_hal_nvic_enable(EXTI8_IRQn);
    furi_hal_nvic_enable(EXTI9_IRQn);
    furi_hal_nvic_enable(EXTI10_IRQn);
    furi_hal_nvic_enable(EXTI11_IRQn);
    furi_hal_nvic_enable(EXTI12_IRQn);
    furi_hal_nvic_enable(EXTI13_IRQn);
    furi_hal_nvic_enable(EXTI14_IRQn);
    furi_hal_nvic_enable(EXTI15_IRQn);
}

void furi_hal_resources_deinit_early(void) {
    furi_hal_bus_disable(FuriHalBusGPIOA);
    furi_hal_bus_disable(FuriHalBusGPIOB);
    furi_hal_bus_disable(FuriHalBusGPIOC);
    furi_hal_bus_disable(FuriHalBusGPIOD);
    furi_hal_bus_disable(FuriHalBusGPIOE);
#if defined(GPIOF)
    furi_hal_bus_disable(FuriHalBusGPIOF);
#endif
    furi_hal_bus_disable(FuriHalBusGPIOG);
    furi_hal_bus_disable(FuriHalBusGPIOH);
#if defined(GPIOI)
    furi_hal_bus_disable(FuriHalBusGPIOI);
#endif
#if defined(GPIOJ)
    furi_hal_bus_disable(FuriHalBusGPIOJ);
#endif
}
