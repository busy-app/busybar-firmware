
#include "furi_hal_qei.h"
#include <furi.h>
#include "rsi_qei.h"
#include "furi_hal_resources.h"
#include "furi_hal_bus.h"
#include "furi_hal_interrupt.h"

#define TAG "QEI"

#define FURI_HAL_QEI_VELOCITY_FREQ_CALCULATION_POS      512 * 1000000 // 512Hz max frequency * 1000000us
#define FURI_HAL_QEI_VELOCITY_DELTA_TIME_CALCULATION_US 10000

typedef enum {
    HalQeiStateIDLE,
    HalQeiStateStart,
    HalQeiStateRotation,
    HalQeiStateStop,
} FuriHalQeiState;

typedef struct {
    FuriHalQeiState state;
    int pos;
    FuriHalQeiDeltaPosCallback delta_pos_callback;
    void* context;
} FuriHalQei;

static FuriHalQei furi_hal_qei = {.pos = 0, .state = HalQeiStateIDLE};

STATIC INLINE void furi_hal_qei_set_pos_cnt(volatile QEI_Type* pstcQei, uint32_t Position) {
    // configure least significant word (LSW)
    pstcQei->QEI_POSITION_CNT_REG = (0xFFFF & Position);
    pstcQei->QEI_POSITION_CNT_REG |= (0xFFFF & (Position >> 16)) << 16;
}

STATIC INLINE void furi_hal_qei_set_velocity(volatile QEI_Type* pstcQei, uint32_t data) {
    pstcQei->QEI_VELOCITY_REG = data;
}

static void furi_hal_qei_process(uint16_t pos) {
    if(furi_hal_qei.state == HalQeiStateIDLE) {
        furi_hal_qei.pos = 0;
    } else {
        int16_t delta_pos = (int16_t)(pos - furi_hal_qei.pos);
        if(delta_pos) {
            if(furi_hal_qei.delta_pos_callback) {
                furi_hal_qei.delta_pos_callback(delta_pos, furi_hal_qei.context);
            }
        }
        furi_hal_qei.pos = pos;
    }
}

static void furi_qei_irq_callback(void* context) {
    UNUSED(context);

    if(RSI_QEI_GetIntrStatus(QEI) & (QEI_POS_CNT_RST_INTR_LVL | QEI_POS_CNT_MAT_INTR_LVL)) {
        if(furi_hal_qei.state == HalQeiStateIDLE) {
            furi_hal_qei.state = HalQeiStateStart;
            furi_hal_qei_process(RSI_QEI_GetPosition(QEI));
            //Mask interrupts on transition through 1 and 0
            RSI_QEI_IntrMask(QEI, (QEI_POS_CNT_RST_INTR_LVL | QEI_POS_CNT_MAT_INTR_LVL));
            //Start interrupts on timeout while rotating and on rotation stop
            RSI_QEI_IntrUnMask(
                QEI, QEI_VELO_LESS_THAN_INTR_LVL | VELOCITY_COMPUTATION_OVER_INTR_LVL);
            //Start velocity calculation
            RSI_QEI_StartVelocityCounter(QEI);
            RSI_QEI_ClrIntrStatus(
                QEI, (QEI_VELO_LESS_THAN_INTR_LVL | VELOCITY_COMPUTATION_OVER_INTR_LVL));
        }
        RSI_QEI_ClrIntrStatus(QEI, (QEI_POS_CNT_RST_INTR_LVL | QEI_POS_CNT_MAT_INTR_LVL));
    }

    if(RSI_QEI_GetIntrStatus(QEI) & VELOCITY_COMPUTATION_OVER_INTR_LVL) {
        //If the rotation interrupt has been triggered
        furi_hal_qei.state = HalQeiStateRotation;
        furi_hal_qei_process(RSI_QEI_GetPosition(QEI));
        RSI_QEI_ClrIntrStatus(QEI, VELOCITY_COMPUTATION_OVER_INTR_LVL);
    }

    if(RSI_QEI_GetIntrStatus(QEI) & QEI_VELO_LESS_THAN_INTR_LVL) {
        //Rotation has stopped
        furi_hal_qei.state = HalQeiStateIDLE;
        furi_hal_qei_process(RSI_QEI_GetPosition(QEI));
        RSI_QEI_StopVelocityCounter(QEI);
        RSI_QEI_ClrIntrStatus(
            QEI, QEI_VELO_LESS_THAN_INTR_LVL | VELOCITY_COMPUTATION_OVER_INTR_LVL);

        //Stop interrupts on timeout while rotating and on rotation stop
        RSI_QEI_IntrMask(QEI, (QEI_VELO_LESS_THAN_INTR_LVL | VELOCITY_COMPUTATION_OVER_INTR_LVL));
        //Set the position to 0
        furi_hal_qei_set_pos_cnt(QEI, 0);
        //Start interrupts on transition through 0 and 1
        RSI_QEI_IntrUnMask(QEI, (QEI_POS_CNT_RST_INTR_LVL | QEI_POS_CNT_MAT_INTR_LVL));
    }
}

void furi_hal_qei_init(void) {
    furi_hal_gpio_init_ex(
        &gpio_encoder_a, GpioModeInput, GpioPullUp, GpioSpeedLow, GpioAltFn6SOCPERH_ON_ULP_GPIO_9);
    // Init virtual (multiplexed) pins
    furi_hal_gpio_init_ex(
        &gpio_i_encoder_a, GpioModeInput, GpioPullNo, GpioSpeedLow, GpioAltFn3QEI_PHA);

    furi_hal_gpio_init_ex(
        &gpio_encoder_b, GpioModeInput, GpioPullUp, GpioSpeedLow, GpioAltFn6SOCPERH_ON_ULP_GPIO_10);
    // Init virtual (multiplexed) pins
    furi_hal_gpio_init_ex(
        &gpio_i_encoder_b, GpioModeInput, GpioPullNo, GpioSpeedLow, GpioAltFn3QEI_PHB);

    // Enable QEI clock
    furi_hal_bus_enable(FuriHalBusQEI_PCLK);

    RSI_QEI_SetMode(QEI, QEI_ENCODING_MODE_2X);
    RSI_QEI_ConfigureDeltaTimeAndFreq(
        QEI,
        FURI_HAL_QEI_VELOCITY_FREQ_CALCULATION_POS,
        FURI_HAL_QEI_VELOCITY_DELTA_TIME_CALCULATION_US);
    //Set comparison with 1 at the beginning of the smart
    RSI_QEI_SetPosMatch(QEI, 1);
    // Set the minimum rotation speed
    furi_hal_qei_set_velocity(QEI, 1);
    // Enable an interrupt to compare transitions 0 and 1
    RSI_QEI_IntrUnMask(QEI, QEI_POS_CNT_RST_INTR_LVL | QEI_POS_CNT_MAT_INTR_LVL);
    //Mask the interruption
    RSI_QEI_IntrMask(QEI, VELOCITY_COMPUTATION_OVER_INTR_LVL);

    furi_hal_interrupt_set_isr(FuriHalInterruptIdQEI, furi_qei_irq_callback, NULL);
}

void furi_hal_qei_deinit(void) {
    //Disable QEI interrupt
    furi_hal_interrupt_set_isr(FuriHalInterruptIdQEI, NULL, NULL);

    RSI_QEI_IntrMask(
        QEI,
        (QEI_POS_CNT_RST_INTR_LVL | QEI_POS_CNT_MAT_INTR_LVL | QEI_POS_CNT_ERR_INTR_LVL |
         QEI_VELO_LESS_THAN_INTR_LVL | QEI_IDX_CNT_MAT_INT_LVL |
         VELOCITY_COMPUTATION_OVER_INTR_LVL));
    RSI_QEI_ClrIntrStatus(
        QEI,
        (QEI_POS_CNT_RST_INTR_LVL | QEI_POS_CNT_MAT_INTR_LVL | QEI_POS_CNT_ERR_INTR_LVL |
         QEI_VELO_LESS_THAN_INTR_LVL | QEI_IDX_CNT_MAT_INT_LVL |
         VELOCITY_COMPUTATION_OVER_INTR_LVL));

    // Disable QEI clock
    furi_hal_bus_disable(FuriHalBusQEI_PCLK);

    furi_hal_qei.context = NULL;
    furi_hal_qei.delta_pos_callback = NULL;
    furi_hal_qei.pos = 0;
    furi_hal_qei.state = HalQeiStateIDLE;

    //Todo: Deinit GPIO
    furi_hal_gpio_init_simple(&gpio_encoder_a, GpioModeInput);
    furi_hal_gpio_init_simple(&gpio_i_encoder_a, GpioModeInput);
}

void furi_hal_qei_set_delta_pos_callback(FuriHalQeiDeltaPosCallback callback, void* context) {
    furi_check(callback, "FuriHalQei callback pointer NULL");
    furi_hal_qei.delta_pos_callback = callback;
    furi_hal_qei.context = context;
}
