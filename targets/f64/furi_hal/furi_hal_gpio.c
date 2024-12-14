#include <furi.h>
#include <furi_hal_gpio.h>

#include <sl_si91x_gpio_common.h>

#define HP_ULP_PERIPH_COUNT (2UL)

#define HP_ULP_INTERRUPT_COUNT (8UL)
#define UULP_INTERRUPT_COUNT   (4UL)

#define DRIVE_STRENGTH_2MA     (0UL)
#define DRIVE_STRENGTH_4MA     (1UL)
#define DRIVE_STRENGTH_8MA     (2UL)
#define DRIVE_STRENGTH_12MA    (3UL)
#define GPIO_INTR_STATUS_CLEAR (7UL)

typedef struct {
    const GpioPin* gpio;
    GpioExtiCallback callback;
    void* context;
} GpioHpUlpInterrupt;

typedef struct {
    GpioExtiCallback callback;
    void* context;
} GpioUulpInterrupt;

typedef struct {
    GpioHpUlpInterrupt hp_ulp[HP_ULP_PERIPH_COUNT][HP_ULP_INTERRUPT_COUNT];
    GpioUulpInterrupt uulp[UULP_INTERRUPT_COUNT];
} GpioInterrupt;

static EGPIO_Type* const gpio_hp_ulp_peripheral[HP_ULP_PERIPH_COUNT] = {
    [GpioTypeHp] = GPIO,
    [GpioTypeUlp] = ULP_GPIO,
};

static volatile GpioInterrupt gpio_interrupt;

void furi_hal_gpio_init_simple(const GpioPin* gpio, const GpioMode mode) {
    furi_hal_gpio_init(gpio, mode, GpioPullNo, GpioSpeedLow);
}

void furi_hal_gpio_init(
    const GpioPin* gpio,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed) {
    furi_hal_gpio_init_ex(gpio, mode, pull, speed, GpioAltFnUnused);
}

static void
    furi_hal_gpio_init_hp_ulp(const GpioPin* gpio, const GpioMode mode, const GpioAltFn alt_fn) {
    furi_assert(gpio->type < HP_ULP_PERIPH_COUNT);

    EGPIO_Type* periph = gpio_hp_ulp_peripheral[gpio->type];

    periph->PIN_CONFIG[gpio->pin].GPIO_CONFIG_REG_b.MODE = alt_fn;

    if(mode == GpioModeInput) {
        periph->PIN_CONFIG[gpio->pin].GPIO_CONFIG_REG_b.DIRECTION = 1;
    } else if(mode == GpioModeOutputPushPull) {
        periph->PIN_CONFIG[gpio->pin].GPIO_CONFIG_REG_b.DIRECTION = 0;
    } else if(mode == GpioModeOutputOpenDrain) {
        periph->PIN_CONFIG[gpio->pin].GPIO_CONFIG_REG_b.DIRECTION = 1;
        periph->PIN_CONFIG[gpio->pin].BIT_LOAD_REG = 0;
    } else {
        furi_crash();
    }
}

static void
    furi_hal_gpio_init_uulp(const GpioPin* gpio, const GpioMode mode, const GpioAltFn alt_fn) {
    UULP_GPIO->NPSS_GPIO_CNTRL[gpio->pin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_REN = 1;
    UULP_GPIO->NPSS_GPIO_CNTRL[gpio->pin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_MODE = alt_fn;

    if(mode == GpioModeInput) {
        UULP_GPIO->NPSS_GPIO_CNTRL[gpio->pin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_OEN = 1;

    } else if(mode == GpioModeOutputPushPull) {
        UULP_GPIO->NPSS_GPIO_CNTRL[gpio->pin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_OEN = 0;

    } else if(mode == GpioModeOutputOpenDrain) {
        UULP_GPIO->NPSS_GPIO_CNTRL[gpio->pin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_OEN = 1;
        UULP_GPIO->NPSS_GPIO_CNTRL[gpio->pin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_OUT = 0;

    } else {
        furi_crash();
    }
}

void furi_hal_gpio_init_ex(
    const GpioPin* gpio,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed,
    const GpioAltFn alt_fn) {
    // Configure gpio with interrupts disabled
    FURI_CRITICAL_ENTER();

    if(gpio->type == GpioTypeHp) {
        PAD_REG(gpio->pin)->GPIO_PAD_CONFIG_REG_b.PADCONFIG_REN = 1;
        PAD_REG(gpio->pin)->GPIO_PAD_CONFIG_REG_b.PADCONFIG_SR = speed;
        PAD_REG(gpio->pin)->GPIO_PAD_CONFIG_REG_b.PADCONFIG_P1_P2 = pull;
        PAD_REG(gpio->pin)->GPIO_PAD_CONFIG_REG_b.PADCONFIG_E1_E2 = DRIVE_STRENGTH_2MA;

        furi_hal_gpio_init_hp_ulp(gpio, mode, alt_fn);

    } else if(gpio->type == GpioTypeUlp) {
        ULP_PAD_CONFIG2_REG->ULP_PAD_CONFIG_REG2 |= 1UL << gpio->pin;

        // NOTE: Speed and Pull-Up settings are co-dependent for pins 0...3, 4...8, 8...11
        if(gpio->pin < 4) {
            ULP_PAD_CONFIG0_REG->ULP_GPIO_PAD_CONFIG_REG_0.PADCONFIG_SR_1 = speed;
            ULP_PAD_CONFIG0_REG->ULP_GPIO_PAD_CONFIG_REG_0.PADCONFIG_P1_P2_1 = pull;
            ULP_PAD_CONFIG0_REG->ULP_GPIO_PAD_CONFIG_REG_0.PADCONFIG_E1_E2_1 = DRIVE_STRENGTH_2MA;
        } else if(gpio->pin < 8) {
            ULP_PAD_CONFIG0_REG->ULP_GPIO_PAD_CONFIG_REG_0.PADCONFIG_SR_2 = speed;
            ULP_PAD_CONFIG0_REG->ULP_GPIO_PAD_CONFIG_REG_0.PADCONFIG_P1_P2_2 = pull;
            ULP_PAD_CONFIG0_REG->ULP_GPIO_PAD_CONFIG_REG_0.PADCONFIG_E1_E2_2 = DRIVE_STRENGTH_2MA;
        } else {
            ULP_PAD_CONFIG1_REG->ULP_GPIO_PAD_CONFIG_REG_1.PADCONFIG_SR_1 = speed;
            ULP_PAD_CONFIG1_REG->ULP_GPIO_PAD_CONFIG_REG_1.PADCONFIG_P1_P2_1 = pull;
            ULP_PAD_CONFIG1_REG->ULP_GPIO_PAD_CONFIG_REG_1.PADCONFIG_E1_E2_1 = DRIVE_STRENGTH_2MA;
        }

        furi_hal_gpio_init_hp_ulp(gpio, mode, alt_fn);

    } else if(gpio->type == GpioTypeUulp) {
        // No way of setting the pullup/down resistors, crash to avoid false expectations
        furi_check(pull == GpioPullNo);
        furi_hal_gpio_init_uulp(gpio, mode, alt_fn);

    } else {
        furi_crash();
    }

    FURI_CRITICAL_EXIT();
}

void furi_hal_gpio_enable_ulp_on_hp(const GpioPin* ulp_gpio, const GpioAltFn alt_fn) {
    furi_check(ulp_gpio);
    furi_check(ulp_gpio->type == GpioTypeUlp);

    ULPCLK->ULP_SOC_GPIO_MODE_REG[ulp_gpio->pin].ULP_SOC_GPIO_MODE_REG_b.ULP_SOC_GPIO_MODE_REG =
        alt_fn;
}

void furi_hal_gpio_write(const GpioPin* gpio, const bool state) {
    switch(gpio->type) {
    case GpioTypeHp:
        GPIO->PIN_CONFIG[gpio->pin].BIT_LOAD_REG = state;
        break;
    case GpioTypeUlp:
        ULP_GPIO->PIN_CONFIG[gpio->pin].BIT_LOAD_REG = state;
        break;
    case GpioTypeUulp:
        UULP_GPIO->NPSS_GPIO_CNTRL[gpio->pin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_OUT = state;
        break;
    default:
        furi_crash();
    }
}

void furi_hal_gpio_write_open_drain(const GpioPin* gpio, const bool state) {
    switch(gpio->type) {
    case GpioTypeHp:
        GPIO->PIN_CONFIG[gpio->pin].GPIO_CONFIG_REG_b.DIRECTION = state;
        break;
    case GpioTypeUlp:
        ULP_GPIO->PIN_CONFIG[gpio->pin].GPIO_CONFIG_REG_b.DIRECTION = state;
        break;
    case GpioTypeUulp:
        UULP_GPIO->NPSS_GPIO_CNTRL[gpio->pin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_OEN = state;
        break;
    default:
        furi_crash();
    }
}

bool furi_hal_gpio_read(const GpioPin* gpio) {
    switch(gpio->type) {
    case GpioTypeHp:
        return GPIO->PIN_CONFIG[gpio->pin].BIT_LOAD_REG;
    case GpioTypeUlp:
        return ULP_GPIO->PIN_CONFIG[gpio->pin].BIT_LOAD_REG;
    case GpioTypeUulp:
        return FURI_BIT(UULP_GPIO_STATUS, gpio->pin);
    default:
        furi_crash();
    }
}

static uint32_t furi_hal_gpio_get_free_interrupt_index_hp_ulp(const GpioPin* gpio) {
    furi_assert(gpio->type < HP_ULP_PERIPH_COUNT);

    for(uint32_t i = 0; i < HP_ULP_INTERRUPT_COUNT; ++i) {
        if(gpio_interrupt.hp_ulp[gpio->type][i].callback == NULL) {
            return i;
        }
    }

    furi_crash("Maximum HP/ULP interrupt count exceeded");
}

static uint32_t furi_hal_gpio_get_configured_interrupt_index_hp_ulp(const GpioPin* gpio) {
    furi_assert(gpio->type < HP_ULP_PERIPH_COUNT);

    for(uint32_t i = 0; i < HP_ULP_INTERRUPT_COUNT; ++i) {
        if(gpio_interrupt.hp_ulp[gpio->type][i].gpio == gpio) {
            return i;
        }
    }

    furi_crash("HP/ULP Gpio not configured as interrupt source");
}

static void furi_hal_gpio_add_int_callback_hp_ulp(
    const GpioPin* gpio,
    GpioCondition cond,
    GpioExtiCallback cb,
    void* ctx) {
    furi_assert(gpio->type < HP_ULP_PERIPH_COUNT);

    EGPIO_Type* periph = gpio_hp_ulp_peripheral[gpio->type];
    const uint32_t idx = furi_hal_gpio_get_free_interrupt_index_hp_ulp(gpio);

    gpio_interrupt.hp_ulp[gpio->type][idx].gpio = gpio;
    gpio_interrupt.hp_ulp[gpio->type][idx].callback = cb;
    gpio_interrupt.hp_ulp[gpio->type][idx].context = ctx;

    periph->INTR[idx].GPIO_INTR_CTRL_b.PORT_NUMBER = gpio->pin / 16;
    periph->INTR[idx].GPIO_INTR_CTRL_b.PIN_NUMBER = gpio->pin % 16;

    if(cond == GpioConditionRise) {
        periph->INTR[idx].GPIO_INTR_CTRL_b.RISE_EDGE_ENABLE = 1;
    } else if(cond == GpioConditionFall) {
        periph->INTR[idx].GPIO_INTR_CTRL_b.FALL_EDGE_ENABLE = 1;
    } else if(cond == GpioConditionRiseFall) {
        periph->INTR[idx].GPIO_INTR_CTRL_b.RISE_EDGE_ENABLE = 1;
        periph->INTR[idx].GPIO_INTR_CTRL_b.FALL_EDGE_ENABLE = 1;
    } else {
        furi_crash();
    }

    periph->INTR[idx].GPIO_INTR_STATUS_b.MASK_CLEAR = 1;
}

static void furi_hal_gpio_add_int_callback_uulp(
    const GpioPin* gpio,
    GpioCondition cond,
    GpioExtiCallback cb,
    void* ctx) {
    furi_assert(gpio->type == GpioTypeUulp);
    furi_assert(gpio->pin < UULP_INTERRUPT_COUNT);

    gpio_interrupt.uulp[gpio->pin].callback = cb;
    gpio_interrupt.uulp[gpio->pin].context = ctx;

    const uint32_t bit = 1UL << gpio->pin;

    if(cond == GpioConditionRise) {
        GPIO_NPSS_GPIO_CONFIG_REG |= bit;
    } else if(cond == GpioConditionFall) {
        GPIO_NPSS_GPIO_CONFIG_REG |= bit << 8;
    } else if(cond == GpioConditionRiseFall) {
        GPIO_NPSS_GPIO_CONFIG_REG |= bit | (bit << 8);
    } else {
        furi_crash();
    }

    GPIO_NPSS_INTERRUPT_MASK_CLR_REG = bit << 1;
}

void furi_hal_gpio_add_int_callback(
    const GpioPin* gpio,
    GpioCondition cond,
    GpioExtiCallback cb,
    void* ctx) {
    furi_check(gpio);
    furi_check(cb);

    FURI_CRITICAL_ENTER();

    if(gpio->type == GpioTypeHp || gpio->type == GpioTypeUlp) {
        furi_hal_gpio_add_int_callback_hp_ulp(gpio, cond, cb, ctx);
    } else if(gpio->type == GpioTypeUulp) {
        furi_hal_gpio_add_int_callback_uulp(gpio, cond, cb, ctx);
    } else {
        furi_crash();
    }

    FURI_CRITICAL_EXIT();
}

void furi_hal_gpio_enable_int_callback(const GpioPin* gpio) {
    furi_check(gpio);

    FURI_CRITICAL_ENTER();

    if(gpio->type == GpioTypeHp || gpio->type == GpioTypeUlp) {
        EGPIO_Type* periph = gpio_hp_ulp_peripheral[gpio->type];
        const uint32_t idx = furi_hal_gpio_get_configured_interrupt_index_hp_ulp(gpio);

        periph->INTR[idx].GPIO_INTR_STATUS_b.MASK_CLEAR = 1;

    } else if(gpio->type == GpioTypeUulp) {
        GPIO_NPSS_INTERRUPT_MASK_CLR_REG = (1UL << gpio->pin) << 1;

    } else {
        furi_crash();
    }

    FURI_CRITICAL_EXIT();
}

void furi_hal_gpio_disable_int_callback(const GpioPin* gpio) {
    furi_check(gpio);

    FURI_CRITICAL_ENTER();

    if(gpio->type == GpioTypeHp || gpio->type == GpioTypeUlp) {
        EGPIO_Type* periph = gpio_hp_ulp_peripheral[gpio->type];
        const uint32_t idx = furi_hal_gpio_get_configured_interrupt_index_hp_ulp(gpio);

        periph->INTR[idx].GPIO_INTR_STATUS_b.MASK_SET = 1;

    } else if(gpio->type == GpioTypeUulp) {
        GPIO_NPSS_INTERRUPT_MASK_SET_REG = (1UL << gpio->pin) << 1;

    } else {
        furi_crash();
    }

    FURI_CRITICAL_EXIT();
}

void furi_hal_gpio_remove_int_callback(const GpioPin* gpio) {
    furi_check(gpio);

    FURI_CRITICAL_ENTER();

    if(gpio->type == GpioTypeHp || gpio->type == GpioTypeUlp) {
        EGPIO_Type* periph = gpio_hp_ulp_peripheral[gpio->type];
        const uint32_t idx = furi_hal_gpio_get_configured_interrupt_index_hp_ulp(gpio);

        periph->INTR[idx].GPIO_INTR_STATUS_b.MASK_SET = 1;
        periph->INTR[idx].GPIO_INTR_CTRL_b.RISE_EDGE_ENABLE = 0;
        periph->INTR[idx].GPIO_INTR_CTRL_b.FALL_EDGE_ENABLE = 0;

        gpio_interrupt.hp_ulp[gpio->type][idx].gpio = NULL;
        gpio_interrupt.hp_ulp[gpio->type][idx].callback = NULL;
        gpio_interrupt.hp_ulp[gpio->type][idx].context = NULL;

    } else if(gpio->type == GpioTypeUulp) {
        const uint32_t bit = 1UL << gpio->pin;

        GPIO_NPSS_GPIO_CONFIG_REG &= ~(bit | (bit << 8));
        GPIO_NPSS_INTERRUPT_MASK_SET_REG = bit << 1;

        gpio_interrupt.uulp[gpio->pin].callback = NULL;
        gpio_interrupt.uulp[gpio->pin].context = NULL;

    } else {
        furi_crash();
    }

    FURI_CRITICAL_EXIT();
}

FURI_ALWAYS_INLINE static void
    furi_hal_gpio_int_call_hp_ulp(const GpioType gpio_type, uint32_t index) {
    volatile GpioHpUlpInterrupt* hp_ulp_interrupt = &gpio_interrupt.hp_ulp[gpio_type][index];
    if(hp_ulp_interrupt->callback) {
        hp_ulp_interrupt->callback(hp_ulp_interrupt->context);
    }
}

FURI_ALWAYS_INLINE static void furi_hal_gpio_int_call_uulp(uint32_t index) {
    volatile GpioUulpInterrupt* uulp_interrupt = &gpio_interrupt.uulp[index];
    if(uulp_interrupt->callback) {
        uulp_interrupt->callback(uulp_interrupt->context);
    }
}

FURI_ALWAYS_INLINE static bool
    furi_hal_gpio_get_int_flag_hp_ulp(const EGPIO_Type* periph, uint32_t index) {
    return periph->INTR[index].GPIO_INTR_STATUS_b.INTERRUPT_STATUS;
}

FURI_ALWAYS_INLINE static void
    furi_hal_gpio_clear_int_flag_hp_ulp(EGPIO_Type* periph, uint32_t index) {
    periph->INTR[index].GPIO_INTR_STATUS = GPIO_INTR_STATUS_CLEAR;
}

FURI_ALWAYS_INLINE static bool furi_hal_gpio_get_int_flag_uulp(uint32_t flag) {
    return (GPIO_NPSS_INTERRUPT_STATUS_REG >> 1) & flag;
}

FURI_ALWAYS_INLINE static void furi_hal_gpio_clear_int_flag_uulp(uint32_t flag) {
    GPIO_NPSS_INTERRUPT_CLEAR_REG = (flag << 1);
}

/* Interrupt handlers */

// 50: GPIO Group Interrupt0
void GRP_IRQ0_Handler(void) {
    furi_crash("Not implemented");
}

// 51: GPIO Group Interrupt1
void GRP_IRQ1_Handler(void) {
    furi_crash("Not implemented");
}

// 52: GPIO Pin Interrupt0
void PIN_IRQ0_Handler(void) {
    furi_hal_gpio_int_call_hp_ulp(GpioTypeHp, PIN_INTR_0);
    furi_hal_gpio_clear_int_flag_hp_ulp(GPIO, PIN_INTR_0);
}

// 53: GPIO Pin Interrupt1
void PIN_IRQ1_Handler(void) {
    furi_hal_gpio_int_call_hp_ulp(GpioTypeHp, PIN_INTR_1);
    furi_hal_gpio_clear_int_flag_hp_ulp(GPIO, PIN_INTR_1);
}

// 54: GPIO Pin Interrupt2
void PIN_IRQ2_Handler(void) {
    furi_hal_gpio_int_call_hp_ulp(GpioTypeHp, PIN_INTR_2);
    furi_hal_gpio_clear_int_flag_hp_ulp(GPIO, PIN_INTR_2);
}

// 55: GPIO Pin Interrupt3
void PIN_IRQ3_Handler(void) {
    furi_hal_gpio_int_call_hp_ulp(GpioTypeHp, PIN_INTR_3);
    furi_hal_gpio_clear_int_flag_hp_ulp(GPIO, PIN_INTR_3);
}

// 56: GPIO Pin Interrupt4
void PIN_IRQ4_Handler(void) {
    furi_hal_gpio_int_call_hp_ulp(GpioTypeHp, PIN_INTR_4);
    furi_hal_gpio_clear_int_flag_hp_ulp(GPIO, PIN_INTR_4);
}

// 57: GPIO Pin Interrupt5
void PIN_IRQ5_Handler(void) {
    furi_hal_gpio_int_call_hp_ulp(GpioTypeHp, PIN_INTR_5);
    furi_hal_gpio_clear_int_flag_hp_ulp(GPIO, PIN_INTR_5);
}

// 58: GPIO Pin Interrupt6
void PIN_IRQ6_Handler(void) {
    furi_hal_gpio_int_call_hp_ulp(GpioTypeHp, PIN_INTR_6);
    furi_hal_gpio_clear_int_flag_hp_ulp(GPIO, PIN_INTR_6);
}

// 59: GPIO Pin Interrupt7
void PIN_IRQ7_Handler(void) {
    furi_hal_gpio_int_call_hp_ulp(GpioTypeHp, PIN_INTR_7);
    furi_hal_gpio_clear_int_flag_hp_ulp(GPIO, PIN_INTR_7);
}

// 18: ULP Processor Interrupt18
void ULP_PIN_IRQ_Handler(void) {
    if(furi_hal_gpio_get_int_flag_hp_ulp(ULP_GPIO, ULP_PIN_INTR_0)) {
        furi_hal_gpio_int_call_hp_ulp(GpioTypeUlp, ULP_PIN_INTR_0);
        furi_hal_gpio_clear_int_flag_hp_ulp(ULP_GPIO, ULP_PIN_INTR_0);
    }
    if(furi_hal_gpio_get_int_flag_hp_ulp(ULP_GPIO, ULP_PIN_INTR_1)) {
        furi_hal_gpio_int_call_hp_ulp(GpioTypeUlp, ULP_PIN_INTR_1);
        furi_hal_gpio_clear_int_flag_hp_ulp(ULP_GPIO, ULP_PIN_INTR_1);
    }
    if(furi_hal_gpio_get_int_flag_hp_ulp(ULP_GPIO, ULP_PIN_INTR_2)) {
        furi_hal_gpio_int_call_hp_ulp(GpioTypeUlp, ULP_PIN_INTR_2);
        furi_hal_gpio_clear_int_flag_hp_ulp(ULP_GPIO, ULP_PIN_INTR_2);
    }
    if(furi_hal_gpio_get_int_flag_hp_ulp(ULP_GPIO, ULP_PIN_INTR_3)) {
        furi_hal_gpio_int_call_hp_ulp(GpioTypeUlp, ULP_PIN_INTR_3);
        furi_hal_gpio_clear_int_flag_hp_ulp(ULP_GPIO, ULP_PIN_INTR_3);
    }
    if(furi_hal_gpio_get_int_flag_hp_ulp(ULP_GPIO, ULP_PIN_INTR_4)) {
        furi_hal_gpio_int_call_hp_ulp(GpioTypeUlp, ULP_PIN_INTR_4);
        furi_hal_gpio_clear_int_flag_hp_ulp(ULP_GPIO, ULP_PIN_INTR_4);
    }
    if(furi_hal_gpio_get_int_flag_hp_ulp(ULP_GPIO, ULP_PIN_INTR_5)) {
        furi_hal_gpio_int_call_hp_ulp(GpioTypeUlp, ULP_PIN_INTR_5);
        furi_hal_gpio_clear_int_flag_hp_ulp(ULP_GPIO, ULP_PIN_INTR_5);
    }
    if(furi_hal_gpio_get_int_flag_hp_ulp(ULP_GPIO, ULP_PIN_INTR_6)) {
        furi_hal_gpio_int_call_hp_ulp(GpioTypeUlp, ULP_PIN_INTR_6);
        furi_hal_gpio_clear_int_flag_hp_ulp(ULP_GPIO, ULP_PIN_INTR_6);
    }
    if(furi_hal_gpio_get_int_flag_hp_ulp(ULP_GPIO, ULP_PIN_INTR_7)) {
        furi_hal_gpio_int_call_hp_ulp(GpioTypeUlp, ULP_PIN_INTR_7);
        furi_hal_gpio_clear_int_flag_hp_ulp(ULP_GPIO, ULP_PIN_INTR_7);
    }
}

// 19: ULP Processor Interrupt19
void ULP_GROUP_IRQ_Handler(void) {
    furi_crash("Not implemented");
}

// 21: UULP Interrupt1
void UULP_PIN_IRQ_Handler(void) {
    if(furi_hal_gpio_get_int_flag_uulp(UULP_INTR_1)) {
        furi_hal_gpio_int_call_uulp(0);
        furi_hal_gpio_clear_int_flag_uulp(UULP_INTR_1);
    }
    if(furi_hal_gpio_get_int_flag_uulp(UULP_INTR_2)) {
        furi_hal_gpio_int_call_uulp(1);
        furi_hal_gpio_clear_int_flag_uulp(UULP_INTR_2);
    }
    if(furi_hal_gpio_get_int_flag_uulp(UULP_INTR_3)) {
        furi_hal_gpio_int_call_uulp(2);
        furi_hal_gpio_clear_int_flag_uulp(UULP_INTR_3);
    }
    if(furi_hal_gpio_get_int_flag_uulp(UULP_INTR_4)) {
        furi_hal_gpio_int_call_uulp(3);
        furi_hal_gpio_clear_int_flag_uulp(UULP_INTR_4);
    }
    if(furi_hal_gpio_get_int_flag_uulp(UULP_INTR_5)) {
        furi_crash("Not present");
    }
}
