#include <furi_hal_resources.h>
#include <furi_hal_bus.h>
#include <stm32u5xx_ll_rcc.h>

const GpioPin gpio_swdio = {.port = GPIOA, .pin = LL_GPIO_PIN_13};
const GpioPin gpio_swclk = {.port = GPIOA, .pin = LL_GPIO_PIN_14};

const GpioPin gpio_log_usart_tx = {.port = GPIOC, .pin = LL_GPIO_PIN_3};
const GpioPin gpio_log_usart_rx = {.port = GPIOC, .pin = LL_GPIO_PIN_2};

const GpioPin gpio_usart1_tx = {.port = GPIOB, .pin = LL_GPIO_PIN_6};
const GpioPin gpio_usart1_rx = {.port = GPIOB, .pin = LL_GPIO_PIN_7};
const GpioPin gpio_usart1_rts = {.port = GPIOB, .pin = LL_GPIO_PIN_3};
const GpioPin gpio_usart1_cts = {.port = GPIOB, .pin = LL_GPIO_PIN_4};

const GpioPin gpio_lpuart1_tx = {.port = GPIOA, .pin = LL_GPIO_PIN_2};
const GpioPin gpio_lpuart1_rx = {.port = GPIOA, .pin = LL_GPIO_PIN_3};

const GpioPin gpio_917_rst = {.port = GPIOA, .pin = LL_GPIO_PIN_4};
const GpioPin gpio_917_irq = {.port = GPIOC, .pin = LL_GPIO_PIN_6};

const GpioPin gpio_bq25798_qon = {.port = GPIOC, .pin = LL_GPIO_PIN_13};
const GpioPin gpio_bq25798_irq = {.port = GPIOC, .pin = LL_GPIO_PIN_0};

const GpioPin gpio_sd_card_d0 = {.port = GPIOC, .pin = LL_GPIO_PIN_8};
const GpioPin gpio_sd_card_d1 = {.port = GPIOC, .pin = LL_GPIO_PIN_9};
const GpioPin gpio_sd_card_d2 = {.port = GPIOC, .pin = LL_GPIO_PIN_10};
const GpioPin gpio_sd_card_d3 = {.port = GPIOC, .pin = LL_GPIO_PIN_11};
const GpioPin gpio_sd_card_ck = {.port = GPIOC, .pin = LL_GPIO_PIN_12};
const GpioPin gpio_sd_card_cmd = {.port = GPIOD, .pin = LL_GPIO_PIN_2};

const GpioPin gpio_usb_dm = {.port = GPIOA, .pin = LL_GPIO_PIN_11};
const GpioPin gpio_usb_dp = {.port = GPIOA, .pin = LL_GPIO_PIN_12};
const GpioPin gpio_ucpd_cc1 = {.port = GPIOA, .pin = LL_GPIO_PIN_15};
const GpioPin gpio_ucpd_cc2 = {.port = GPIOB, .pin = LL_GPIO_PIN_15};

const GpioPin gpio_oled_spi_sdin = {.port = GPIOA, .pin = LL_GPIO_PIN_7};
const GpioPin gpio_oled_spi_sclk = {.port = GPIOA, .pin = LL_GPIO_PIN_5};
const GpioPin gpio_oled_cs = {.port = GPIOA, .pin = LL_GPIO_PIN_6};
const GpioPin gpio_oled_dc = {.port = GPIOC, .pin = LL_GPIO_PIN_4};
const GpioPin gpio_oled_fr = {.port = GPIOC, .pin = LL_GPIO_PIN_5};
const GpioPin gpio_oled_vcc_en = {.port = GPIOA, .pin = LL_GPIO_PIN_0};

const GpioPin gpio_i2c_scl = {.port = GPIOB, .pin = LL_GPIO_PIN_8};
const GpioPin gpio_i2c_sda = {.port = GPIOB, .pin = LL_GPIO_PIN_9};

const GpioPin gpio_led_power_en = {.port = GPIOB, .pin = LL_GPIO_PIN_12};
const GpioPin gpio_led_scan_sdi = {.port = GPIOC, .pin = LL_GPIO_PIN_1};
const GpioPin gpio_led_scan_clk = {.port = GPIOB, .pin = LL_GPIO_PIN_13};
const GpioPin gpio_led_scan_latch = {.port = GPIOA, .pin = LL_GPIO_PIN_1};
const GpioPin gpio_led_gclk = {.port = GPIOB, .pin = LL_GPIO_PIN_2};
const GpioPin gpio_led_sdi_ospi_d0 = {.port = GPIOB, .pin = LL_GPIO_PIN_1};
const GpioPin gpio_led_le_ospi_d1 = {.port = GPIOB, .pin = LL_GPIO_PIN_0};
const GpioPin gpio_led_dclk_ospi_clk = {.port = GPIOB, .pin = LL_GPIO_PIN_10};

const GpioPin gpio_i2s_fs = {.port = GPIOA, .pin = LL_GPIO_PIN_9};
const GpioPin gpio_i2s_sck = {.port = GPIOA, .pin = LL_GPIO_PIN_8};
const GpioPin gpio_i2s_sd = {.port = GPIOA, .pin = LL_GPIO_PIN_10};

const GpioPin gpio_audio_en_and_917_swo = {.port = GPIOC, .pin = LL_GPIO_PIN_7};

const GpioPinRecord gpio_pins[] = {};

const size_t gpio_pins_count = COUNT_OF(gpio_pins);

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
