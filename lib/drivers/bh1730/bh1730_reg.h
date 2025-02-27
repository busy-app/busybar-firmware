#pragma once

#include <assert.h>
#include <core/common_defines.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BH1730_I2C_ADDRESS (0x52)
#define BH1730_I2C_TIMEOUT (50)

#define BH1730_REG_CONTROL   (0x00)
#define BH1730_REG_TIMING    (0x01)
#define BH1730_REG_INTERRUPT (0x02)
#define BH1730_REG_THLLOW    (0x03)
#define BH1730_REG_THLHIGH   (0x04)
#define BH1730_REG_THHLOW    (0x05)
#define BH1730_REG_THHHIGH   (0x06)
#define BH1730_REG_GAIN      (0x07)
#define BH1730_REG_ID        (0x12)
#define BH1730_REG_DATA0LOW  (0x14)
#define BH1730_REG_DATA0HIGH (0x15)
#define BH1730_REG_DATA1LOW  (0x16)
#define BH1730_REG_DATA1HIGH (0x17)

#define BH1730_REG_GAIN_X1   (0x00)
#define BH1730_REG_GAIN_X2   (0x01)
#define BH1730_REG_GAIN_X64  (0x02)
#define BH1730_REG_GAIN_X128 (0x03)

typedef struct {
    uint8_t POWER     : 1;
    uint8_t ADC_EN    : 1;
    uint8_t DATA_SEL  : 1;
    uint8_t ONE_TIME  : 1;
    uint8_t ADC_VALID : 1;
    uint8_t ADC_INTR  : 1;
    uint8_t _RSVD     : 2;
} Bh1730RegControl;

static_assert(sizeof(Bh1730RegControl) == 1, "Bh1730RegControl size mismatch");

typedef struct {
    uint8_t PERSIST  : 4;
    uint8_t INT_EN   : 1;
    uint8_t _RSVD1   : 1;
    uint8_t INT_STOP : 1;
    uint8_t _RSVD0   : 1;
} Bh1730RegInterrupt;

static_assert(sizeof(Bh1730RegInterrupt) == 1, "Bh1730RegInterrupt size mismatch");

typedef struct {
    uint8_t _RSVD : 5;
    uint8_t GAIN  : 3;
} Bh1730RegGain;

static_assert(sizeof(Bh1730RegGain) == 1, "Bh1730RegGain size mismatch");

typedef struct {
    uint8_t PART_NUM : 4;
    uint8_t REV_ID   : 4;
} Bh1730RegId;

static_assert(sizeof(Bh1730RegId) == 1, "Bh1730RegId size mismatch");

#ifdef __cplusplus
}
#endif
