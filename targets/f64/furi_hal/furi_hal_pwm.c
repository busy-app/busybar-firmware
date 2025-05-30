#include "furi_hal_pwm.h"
#include <furi.h>
#include "rsi_pwm.h"
#include "rsi_error.h"
#include "furi_hal_resources.h"
#include "furi_hal_bus.h"
#include "furi_hal_interrupt.h"

#define TAG "PWM"

/***************************************************************************/
/**
 * @brief Enumeration for PWM mode.
 *
 * @details This enumeration defines the possible modes for the PWM module.
 *          It includes options for independent and complementary modes.
 */
typedef enum {
    FuriHalPwmModeIndependent, ///< PWM independent mode
    FuriHalPwmModeComplementary, ///< PWM complementary mode
    FuriHalPwmModeLast, ///< Last member of the enum for validation
} FuriHalPwmMode;

/***************************************************************************/
/**
 * @brief Enumeration for PWM time base output post scale bits.
 *
 * @details This enumeration defines the possible post scale values for the PWM time base output.
 *          It includes options ranging from 1:1 to 1:16 post scale.
 */
typedef enum {
    FuriHalPwmTimePeriodPostScale_1_1, ///< 1:1 post scale
    FuriHalPwmTimePeriodPostScale_1_2, ///< 1:2 post scale
    FuriHalPwmTimePeriodPostScale_1_3, ///< 1:3 post scale
    FuriHalPwmTimePeriodPostScale_1_4, ///< 1:4 post scale
    FuriHalPwmTimePeriodPostScale_1_5, ///< 1:5 post scale
    FuriHalPwmTimePeriodPostScale_1_6, ///< 1:6 post scale
    FuriHalPwmTimePeriodPostScale_1_7, ///< 1:7 post scale
    FuriHalPwmTimePeriodPostScale_1_8, ///< 1:8 post scale
    FuriHalPwmTimePeriodPostScale_1_9, ///< 1:9 post scale
    FuriHalPwmTimePeriodPostScale_1_10, ///< 1:10 post scale
    FuriHalPwmTimePeriodPostScale_1_11, ///< 1:11 post scale
    FuriHalPwmTimePeriodPostScale_1_12, ///< 1:12 post scale
    FuriHalPwmTimePeriodPostScale_1_13, ///< 1:13 post scale
    FuriHalPwmTimePeriodPostScale_1_14, ///< 1:14 post scale
    FuriHalPwmTimePeriodPostScale_1_15, ///< 1:15 post scale
    FuriHalPwmTimePeriodPostScale_1_16, ///< 1:16 post scale
    FuriHalPwmTimePeriodPostScale_1_Last, ///< Last member of the enum for validation
} FuriHalPwmTimePeriodPostScale;

/***************************************************************************/
/**
 * @brief Enumeration for PWM input clock pre-scale select value.
 *
 * @details This enumeration defines the possible pre-scale values for the PWM input clock.
 *          It includes options ranging from 1x to 64x input clock periods.
 */
typedef enum {
    FuriHalPwmTimePeriodPreScale_1, ///< 1x input clock period
    FuriHalPwmTimePeriodPreScale_2, ///< 2x input clock period
    FuriHalPwmTimePeriodPreScale_4, ///< 4x input clock period
    FuriHalPwmTimePeriodPreScale_8, ///< 8x input clock period
    FuriHalPwmTimePeriodPreScale_16, ///< 16x input clock period
    FuriHalPwmTimePeriodPreScale_32, ///< 32x input clock period
    FuriHalPwmTimePeriodPreScale_64, ///< 64x input clock period
    FuriHalPwmTimePeriodPreScale_Last, ///< Last member of the enum for validation
} FuriHalPwmTimePeriodPreScale;

/***************************************************************************/
/**
 * @brief Enumeration for PWM base timer modes.
 *
 * @details This enumeration defines the possible modes for the PWM base timer.
 *          It includes options for free run, single event, down count, up/down, and up/down double update modes.
 */
typedef enum {
    FuriHalPwmBaseTimerModeFreeRunMode = 0, ///< PWM free run mode
    FuriHalPwmBaseTimerModeSingleEventMode = 1, ///< PWM single event mode
    FuriHalPwmBaseTimerModeDownCountMode = 2, ///< PWM down count mode
    FuriHalPwmBaseTimerModeUpDownMode = 4, ///< PWM up/down mode
    FuriHalPwmBaseTimerModeUpDownDoubleUpdate = 5, ///< PWM up/down double update mode
    FuriHalPwmBaseTimerModeLastT = 6, ///< Last member of the enum for validation
} FuriHalPwmBaseTimerMode;

/***************************************************************************/
/**
 * @brief Enumeration for PWM timer.
 *
 * @details This enumeration defines the possible timer configurations for the PWM module.
 *          It includes options for using a separate timer for each channel or a single timer for all channels.
 */
typedef enum {
    FuriHalPwmBaseTimerEachChannel, ///< PWM timer for each channel
    FuriHalPwmBaseTimerAllChannel, ///< PWM timer for all channels
    FuriHalPwmBaseTimerLast, ///< Last member of the enum for validation
} FuriHalPwmBaseTimer;

/***************************************************************************/
/**
 * @brief Enumeration for PWM polarity low.
 *
 * @details This enumeration defines the possible states for controlling the polarity of the low side in the PWM module.
 *          It includes options for setting the polarity to low or high.
 */
typedef enum {
    FuriHalPwmPolarityLow, ///< PWM output polarity for low side (L0-L3) - low
    FuriHalPwmPolarityHigh, ///< PWM output polarity for low side (H0-H3) - high
    FuriHalPwmPolarityLast, ///< Last member of the enum for validation
} FuriHalPwmPolarity;

#define FURI_HAL_PWM_FREQ                             (3000U)
#define FURI_HAL_PWM_RATE                             (SystemCoreClock / FURI_HAL_PWM_FREQ)
#define FURI_HAL_PWM_POLARITY_LOW                     FuriHalPwmPolarityHigh
#define FURI_HAL_PWM_POLARITY_HIGH                    FuriHalPwmPolarityHigh
#define FURI_HAL_PWM_MODE                             FuriHalPwmModeIndependent
#define FURI_HAL_PWM_DUTY_CYCLE_DEFAULT               (0U)
#define FURI_HAL_PWM_BASE_TIMER_MODE                  FuriHalPwmBaseTimerModeFreeRunMode
#define FURI_HAL_PWM_BASE_TIMER_COUNTER_INITIAL_VALUE (0U)
#define FURI_HAL_PWM_CHENNEL_TIMER_SELECTION          FuriHalPwmBaseTimerAllChannel
#define FURI_HAL_PWM_DUTY_CYCLE_UPDATE                0x01 // Enable duty cycle updating bit in register

#define FURI_HAL_PWM_CHANNEL_RED   PWM_CHNL_0
#define FURI_HAL_PWM_CHANNEL_GREEN PWM_CHNL_2
#define FURI_HAL_PWM_CHANNEL_BLUE  PWM_CHNL_3

static inline void furi_hal_pwm_channel_config(uint32_t chnlNum) {
    furi_check(chnlNum <= PWM_CHNL_3);

    mcpwm_set_time_period(
        MCPWM,
        chnlNum,
        (uint16_t)FURI_HAL_PWM_RATE,
        (uint16_t)FURI_HAL_PWM_BASE_TIMER_COUNTER_INITIAL_VALUE);

    mcpwm_set_output_mode(MCPWM, FURI_HAL_PWM_MODE, chnlNum);
    uint32_t ticks = (uint32_t)((FURI_HAL_PWM_RATE * FURI_HAL_PWM_DUTY_CYCLE_DEFAULT) >> 8);
    // Set Duty cycle value for channel
    RSI_MCPWM_SetDutyCycle(MCPWM, ticks, chnlNum);
    mcpwm_set_base_timer_mode(MCPWM, FURI_HAL_PWM_BASE_TIMER_MODE, chnlNum);
    RSI_MCPWM_BaseTimerSelect(MCPWM, FURI_HAL_PWM_CHENNEL_TIMER_SELECTION);
}

static inline void furi_hal_pwm_configure_duty_cycle(uint32_t chnlNum) {
    furi_check(chnlNum <= PWM_CHNL_3);
    RSI_MCPWM_DutyCycleControlSet(MCPWM, FURI_HAL_PWM_DUTY_CYCLE_UPDATE, chnlNum);
    RSI_MCPWM_DutyCycleControlReset(MCPWM, FURI_HAL_PWM_DUTY_CYCLE_UPDATE, chnlNum);
}

void furi_hal_pwm_init(void) {
    // Enable PWM clock
    furi_hal_bus_enable(FuriHalBusMCPWM_PCLK);

    // Initialize PWM
    mcpwm_set_output_polarity(MCPWM, FURI_HAL_PWM_POLARITY_LOW, FURI_HAL_PWM_POLARITY_HIGH);

    furi_hal_pwm_channel_config(FURI_HAL_PWM_CHANNEL_RED);
    furi_hal_pwm_channel_config(FURI_HAL_PWM_CHANNEL_GREEN);
    furi_hal_pwm_channel_config(FURI_HAL_PWM_CHANNEL_BLUE);

    mcpwm_period_control_config(
        MCPWM,
        FuriHalPwmTimePeriodPostScale_1_1,
        FuriHalPwmTimePeriodPreScale_1,
        FURI_HAL_PWM_CHANNEL_RED);
    mcpwm_period_control_config(
        MCPWM,
        FuriHalPwmTimePeriodPostScale_1_1,
        FuriHalPwmTimePeriodPreScale_1,
        FURI_HAL_PWM_CHANNEL_GREEN);
    mcpwm_period_control_config(
        MCPWM,
        FuriHalPwmTimePeriodPostScale_1_1,
        FuriHalPwmTimePeriodPreScale_1,
        FURI_HAL_PWM_CHANNEL_BLUE);

    furi_hal_pwm_configure_duty_cycle(FURI_HAL_PWM_CHANNEL_RED);
    furi_hal_pwm_configure_duty_cycle(FURI_HAL_PWM_CHANNEL_GREEN);
    furi_hal_pwm_configure_duty_cycle(FURI_HAL_PWM_CHANNEL_BLUE);
}

void furi_hal_pwm_deinit(void) {
    // Disable PWM clock
    furi_hal_bus_disable(FuriHalBusMCPWM_PCLK);
    // Deinit GPIO
    //Todo: Deinit GPIO
    furi_hal_gpio_init_simple(&gpio_pwm_red, GpioModeInput);
    furi_hal_gpio_init_simple(&gpio_pwm_green, GpioModeInput);
    furi_hal_gpio_init_simple(&gpio_pwm_blue, GpioModeInput);
}

void furi_hal_pwm_start(void) {
    // Reset the counter disable
    RSI_PWM_Counter_Reset_Disable(MCPWM, FURI_HAL_PWM_CHANNEL_RED);
    RSI_PWM_Counter_Reset_Disable(MCPWM, FURI_HAL_PWM_CHANNEL_GREEN);
    RSI_PWM_Counter_Reset_Disable(MCPWM, FURI_HAL_PWM_CHANNEL_BLUE);
    // Start the PWM
    mcpwm_start(MCPWM, FURI_HAL_PWM_CHANNEL_RED);
    mcpwm_start(MCPWM, FURI_HAL_PWM_CHANNEL_GREEN);
    mcpwm_start(MCPWM, FURI_HAL_PWM_CHANNEL_BLUE);
    // Initialise GPIO
    furi_hal_gpio_init_ex(
        &gpio_pwm_red, GpioModeOutputPushPull, GpioPullNo, GpioSpeedHigh, GpioAltFn10PWM_0H);
    furi_hal_gpio_init_ex(
        &gpio_pwm_green, GpioModeOutputPushPull, GpioPullNo, GpioSpeedHigh, GpioAltFn10PWM_2H);
    furi_hal_gpio_init_ex(
        &gpio_pwm_blue, GpioModeOutputPushPull, GpioPullNo, GpioSpeedHigh, GpioAltFn10PWM_3H);
}

void furi_hal_pwm_stop(void) {
    // Deinitialise GPIO
    furi_hal_gpio_init_simple(&gpio_pwm_red, GpioModeInput);
    furi_hal_gpio_init_simple(&gpio_pwm_green, GpioModeInput);
    furi_hal_gpio_init_simple(&gpio_pwm_blue, GpioModeInput);
    // Stop the PWM
    mcpwm_stop(MCPWM, FURI_HAL_PWM_CHANNEL_RED);
    mcpwm_stop(MCPWM, FURI_HAL_PWM_CHANNEL_GREEN);
    mcpwm_stop(MCPWM, FURI_HAL_PWM_CHANNEL_BLUE);
    // Reset the counter
    mcpwm_counter_reset(MCPWM, FURI_HAL_PWM_CHANNEL_RED);
    mcpwm_counter_reset(MCPWM, FURI_HAL_PWM_CHANNEL_GREEN);
    mcpwm_counter_reset(MCPWM, FURI_HAL_PWM_CHANNEL_BLUE);
}

void furi_hal_pwm_set_rgb(uint8_t red, uint8_t green, uint8_t blue) {
    uint32_t ticks = 0;

    // Set the duty cycle for the red channel
    ticks = (uint32_t)((FURI_HAL_PWM_RATE * red) >> 8);
    MCPWM->PWM_DUTYCYCLE_REG_WR_VALUE_b[FURI_HAL_PWM_CHANNEL_RED].PWM_DUTYCYCLE_REG_WR_VALUE_CH =
        ticks;

    // Set the duty cycle for the green channel
    ticks = (uint32_t)((FURI_HAL_PWM_RATE * green) >> 8);
    MCPWM->PWM_DUTYCYCLE_REG_WR_VALUE_b[FURI_HAL_PWM_CHANNEL_GREEN].PWM_DUTYCYCLE_REG_WR_VALUE_CH =
        ticks;

    // Set the duty cycle for the blue channel
    ticks = (uint32_t)((FURI_HAL_PWM_RATE * blue) >> 8);
    MCPWM->PWM_DUTYCYCLE_REG_WR_VALUE_b[FURI_HAL_PWM_CHANNEL_BLUE].PWM_DUTYCYCLE_REG_WR_VALUE_CH =
        ticks;
}
