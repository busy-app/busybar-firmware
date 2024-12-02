#include <furi_hal_resources.h>
#include <furi_hal_bus.h>

#include <sl_si91x_gpio_common.h>

#define PADSELECTION_ALL_M4  (0x3FFDFEUL) // GPIO 6...15, 46...57
#define PADSELECTION1_ALL_M4 (0x000FCEUL) // ULP GPIO 1...3, 6...11

#define TAG "FuriHalResources"

const GpioPin gpio_6 = {.type = GpioTypeHp, .pin = 6};
const GpioPin gpio_pwm_red = {.type = GpioTypeHp, .pin = 7};
const GpioPin gpio_ulp_uart_rx = {.type = GpioTypeHp, .pin = 8};
const GpioPin gpio_ulp_uart_tx = {.type = GpioTypeHp, .pin = 9};
const GpioPin gpio_10 = {.type = GpioTypeHp, .pin = 10};
const GpioPin gpio_pwm_green = {.type = GpioTypeHp, .pin = 11};
const GpioPin gpio_12 = {.type = GpioTypeHp, .pin = 12};
const GpioPin gpio_pwm_blue = {.type = GpioTypeHp, .pin = 15};
const GpioPin gpio_25 = {.type = GpioTypeHp, .pin = 25};
const GpioPin gpio_26 = {.type = GpioTypeHp, .pin = 26};
const GpioPin gpio_27 = {.type = GpioTypeHp, .pin = 27};
const GpioPin gpio_28 = {.type = GpioTypeHp, .pin = 28};
const GpioPin gpio_29 = {.type = GpioTypeHp, .pin = 29};
const GpioPin gpio_30 = {.type = GpioTypeHp, .pin = 30};
const GpioPin gpio_46 = {.type = GpioTypeHp, .pin = 40};
const GpioPin gpio_47 = {.type = GpioTypeHp, .pin = 40};
const GpioPin gpio_48 = {.type = GpioTypeHp, .pin = 40};
const GpioPin gpio_49 = {.type = GpioTypeHp, .pin = 40};
const GpioPin gpio_sw_busy = {.type = GpioTypeHp, .pin = 50};
const GpioPin gpio_sw_settings = {.type = GpioTypeHp, .pin = 51};
const GpioPin gpio_52 = {.type = GpioTypeHp, .pin = 52};
const GpioPin gpio_usart0_rts = {.type = GpioTypeHp, .pin = 53};
const GpioPin gpio_usart0_tx = {.type = GpioTypeHp, .pin = 54};
const GpioPin gpio_usart0_rx = {.type = GpioTypeHp, .pin = 55};
const GpioPin gpio_usart0_cts = {.type = GpioTypeHp, .pin = 56};
const GpioPin gpio_57 = {.type = GpioTypeHp, .pin = 57};

const GpioPin gpio_i_64 = {.type = GpioTypeHp, .pin = 64};
const GpioPin gpio_i_65 = {.type = GpioTypeHp, .pin = 65};
const GpioPin gpio_i_66 = {.type = GpioTypeHp, .pin = 66};
const GpioPin gpio_i_67 = {.type = GpioTypeHp, .pin = 67};
const GpioPin gpio_i_68 = {.type = GpioTypeHp, .pin = 68};
const GpioPin gpio_i_69 = {.type = GpioTypeHp, .pin = 69};
const GpioPin gpio_i_70 = {.type = GpioTypeHp, .pin = 70};
const GpioPin gpio_i_71 = {.type = GpioTypeHp, .pin = 71};
const GpioPin gpio_i_uart1_rx = {.type = GpioTypeHp, .pin = 72};
const GpioPin gpio_i_encoder_a = {.type = GpioTypeHp, .pin = 73};
const GpioPin gpio_i_encoder_b = {.type = GpioTypeHp, .pin = 74};
const GpioPin gpio_i_uart1_tx = {.type = GpioTypeHp, .pin = 75};

const GpioPin gpio_ulp_0 = {.type = GpioTypeUlp, .pin = 0};
const GpioPin gpio_irq = {.type = GpioTypeUlp, .pin = 1};
const GpioPin gpio_ulp_2 = {.type = GpioTypeUlp, .pin = 2};
const GpioPin gpio_ulp_i_3 = {.type = GpioTypeUlp, .pin = 3};
const GpioPin gpio_ulp_4 = {.type = GpioTypeUlp, .pin = 4};
const GpioPin gpio_ulp_5 = {.type = GpioTypeUlp, .pin = 5};
const GpioPin gpio_sw_apps = {.type = GpioTypeUlp, .pin = 6};
const GpioPin gpio_sw_status = {.type = GpioTypeUlp, .pin = 7};
const GpioPin gpio_uart1_rx = {.type = GpioTypeUlp, .pin = 8};
const GpioPin gpio_encoder_a = {.type = GpioTypeUlp, .pin = 9};
const GpioPin gpio_encoder_b = {.type = GpioTypeUlp, .pin = 10};
const GpioPin gpio_uart1_tx = {.type = GpioTypeUlp, .pin = 11};

const GpioPin gpio_sw_off = {.type = GpioTypeUulp, .pin = 0};
const GpioPin gpio_sw_back = {.type = GpioTypeUulp, .pin = 1};
const GpioPin gpio_sw_start_pause = {.type = GpioTypeUulp, .pin = 2};
const GpioPin gpio_sw_ok = {.type = GpioTypeUulp, .pin = 3};

const InputPin input_pins[] = {
    {
        .gpio = &gpio_sw_ok,
        .key = InputKeyOk,
        .inverted = true,
        .name = "Ok",
        .condition = GpioConditionRiseFall,
    },
    {
        .gpio = &gpio_sw_back,
        .key = InputKeyBack,
        .inverted = true,
        .name = "Back",
        .condition = GpioConditionRiseFall,
    },
    {
        .gpio = &gpio_sw_start_pause,
        .key = InputKeyStart,
        .inverted = true,
        .name = "Start/Pause",
        .condition = GpioConditionRiseFall,
    },
    {
        .gpio = &gpio_sw_busy,
        .key = InputKeySwitch,
        .inverted = true,
        .name = "Switch Busy",
        .switch_position = InputSwitchPositionBusy,
        .condition = GpioConditionFall,
    },
    {
        .gpio = &gpio_sw_status,
        .key = InputKeySwitch,
        .inverted = true,
        .name = "Switch Status",
        .switch_position = InputSwitchPositionStatus,
        .condition = GpioConditionFall,
    },
    {
        .gpio = &gpio_sw_off,
        .key = InputKeySwitch,
        .inverted = true,
        .name = "Switch Off",
        .switch_position = InputSwitchPositionOff,
        .condition = GpioConditionFall,
    },
    {
        .gpio = &gpio_sw_apps,
        .key = InputKeySwitch,
        .inverted = true,
        .name = "Switch Apps",
        .switch_position = InputSwitchPositionApps,
        .condition = GpioConditionFall,
    },
    {
        .gpio = &gpio_sw_settings,
        .key = InputKeySwitch,
        .inverted = true,
        .name = "Switch Settings",
        .switch_position = InputSwitchPositionSettings,
        .condition = GpioConditionFall,
    },
};

const size_t input_pins_count = COUNT_OF(input_pins);

static void furi_hal_resources_init_input_pins(GpioMode mode) {
    for(size_t i = 0; i < input_pins_count; i++) {
        const InputPin* pin = &input_pins[i];
        const GpioPin* gpio = pin->gpio;
        const GpioPull pull = gpio->type == GpioTypeUulp ? GpioPullNo :
                              pin->inverted              ? GpioPullUp :
                                                           GpioPullDown;
        furi_hal_gpio_init(gpio, mode, pull, GpioSpeedLow);
    }
}

void furi_hal_resources_init_early(void) {
    // Enable GPIO clock
    furi_hal_bus_enable(FuriHalBusEGPIO_CLK);
    // Enable ULP GPIO clock
    furi_hal_bus_enable(FuriHalBusUlpEGPIO_CLK_EN);
    // Control HP GPIO pads from M4
    PADSELECTION = PADSELECTION_ALL_M4;
    // Control ULP GPIO pads from M4
    PADSELECTION_1 = PADSELECTION1_ALL_M4;

    furi_hal_resources_init_input_pins(GpioModeInput);
}

void furi_hal_resources_deinit_early(void) {
    // TODO: No implementation GpioModeAnalog
    // furi_hal_resources_init_input_pins(GpioModeAnalog);
}

void furi_hal_resources_init(void) {
    NVIC_SetPriority(EGPIO_PIN_0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EGPIO_PIN_0_IRQn);

    NVIC_SetPriority(EGPIO_PIN_1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EGPIO_PIN_1_IRQn);

    NVIC_SetPriority(EGPIO_PIN_2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EGPIO_PIN_2_IRQn);

    NVIC_SetPriority(EGPIO_PIN_3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EGPIO_PIN_3_IRQn);

    NVIC_SetPriority(EGPIO_PIN_4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EGPIO_PIN_4_IRQn);

    NVIC_SetPriority(EGPIO_PIN_5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EGPIO_PIN_5_IRQn);

    NVIC_SetPriority(EGPIO_PIN_6_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EGPIO_PIN_6_IRQn);

    NVIC_SetPriority(EGPIO_PIN_7_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EGPIO_PIN_7_IRQn);

    NVIC_SetPriority(ULP_EGPIO_PIN_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(ULP_EGPIO_PIN_IRQn);

    NVIC_SetPriority(
        NPSS_TO_MCU_GPIO_INTR_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(NPSS_TO_MCU_GPIO_INTR_IRQn);

    FURI_LOG_I(TAG, "Init OK");
}
