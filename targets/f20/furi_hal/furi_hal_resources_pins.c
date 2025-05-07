#include <furi_hal_resources.h>

const GpioPin gpio_swdio = {.port = GPIOA, .pin = LL_GPIO_PIN_13};
const GpioPin gpio_swclk = {.port = GPIOA, .pin = LL_GPIO_PIN_14};

const GpioPin gpio_log_usart_tx = {.port = GPIOC, .pin = LL_GPIO_PIN_3};
const GpioPin gpio_log_usart_rx = {.port = GPIOC, .pin = LL_GPIO_PIN_2};

const GpioPin gpio_usart1_tx = {.port = GPIOB, .pin = LL_GPIO_PIN_6};
const GpioPin gpio_usart1_rx = {.port = GPIOB, .pin = LL_GPIO_PIN_7};
const GpioPin gpio_usart1_rts = {.port = GPIOB, .pin = LL_GPIO_PIN_3};
const GpioPin gpio_usart1_cts = {.port = GPIOB, .pin = LL_GPIO_PIN_4};

const GpioPin gpio_usart2_tx = {.port = GPIOA, .pin = LL_GPIO_PIN_2};
const GpioPin gpio_usart2_rx = {.port = GPIOA, .pin = LL_GPIO_PIN_3};

const GpioPin gpio_917_rst = {.port = GPIOA, .pin = LL_GPIO_PIN_4};
const GpioPin gpio_917_swo = {.port = GPIOC, .pin = LL_GPIO_PIN_7};
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

const GpioPin gpio_back_display_spi_sdin = {.port = GPIOA, .pin = LL_GPIO_PIN_7};
const GpioPin gpio_back_display_spi_sclk = {.port = GPIOA, .pin = LL_GPIO_PIN_5};
const GpioPin gpio_back_display_cs = {.port = GPIOA, .pin = LL_GPIO_PIN_6};
const GpioPin gpio_back_display_dc = {.port = GPIOC, .pin = LL_GPIO_PIN_4};
const GpioPin gpio_back_display_fr = {.port = GPIOC, .pin = LL_GPIO_PIN_5};
const GpioPin gpio_back_display_vcc_en = {.port = GPIOA, .pin = LL_GPIO_PIN_0};

const GpioPin gpio_i2c_scl = {.port = GPIOB, .pin = LL_GPIO_PIN_8};
const GpioPin gpio_i2c_sda = {.port = GPIOB, .pin = LL_GPIO_PIN_9};

const GpioPin gpio_front_display_power_en = {.port = GPIOB, .pin = LL_GPIO_PIN_12};
const GpioPin gpio_front_display_scan_sdi = {.port = GPIOC, .pin = LL_GPIO_PIN_1};
const GpioPin gpio_front_display_scan_clk = {.port = GPIOB, .pin = LL_GPIO_PIN_13};
const GpioPin gpio_front_display_scan_latch = {.port = GPIOA, .pin = LL_GPIO_PIN_1};
const GpioPin gpio_front_display_gclk = {.port = GPIOB, .pin = LL_GPIO_PIN_2};
const GpioPin gpio_front_display_sdi_ospi_d0 = {.port = GPIOB, .pin = LL_GPIO_PIN_1};
const GpioPin gpio_front_display_le_ospi_d1 = {.port = GPIOB, .pin = LL_GPIO_PIN_0};
const GpioPin gpio_front_display_dclk_ospi_clk = {.port = GPIOB, .pin = LL_GPIO_PIN_10};

const GpioPin gpio_i2s_fs = {.port = GPIOA, .pin = LL_GPIO_PIN_9};
const GpioPin gpio_i2s_sck = {.port = GPIOA, .pin = LL_GPIO_PIN_8};
const GpioPin gpio_i2s_sd = {.port = GPIOA, .pin = LL_GPIO_PIN_10};

// Note that it is the same pin as the 917 SWDIO pin in the f20 target
const GpioPin gpio_audio_en = {.port = GPIOC, .pin = LL_GPIO_PIN_7};

const GpioPinRecord gpio_pins[] = {};

const size_t gpio_pins_count = COUNT_OF(gpio_pins);
