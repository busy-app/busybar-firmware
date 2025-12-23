#pragma once

/**
 * UART channels
 */
typedef enum {
    FuriHalSerialIdUsart1,
    FuriHalSerialIdUsart2,
    FuriHalSerialIdUsart3,
    FuriHalSerialIdUart4,
    FuriHalSerialIdUart5,
    FuriHalSerialIdUsart6,

    FuriHalSerialIdMax,
} FuriHalSerialId;

typedef enum {
    FuriHalSerialDirectionNone = 0,
    FuriHalSerialDirectionTx = 1 << 0,
    FuriHalSerialDirectionRx = 1 << 1,
    FuriHalSerialDirectionTxRx = FuriHalSerialDirectionTx | FuriHalSerialDirectionRx,
} FuriHalSerialDirection;

typedef enum {
    FuriHalSerialPinTx,
    FuriHalSerialPinRx,
    FuriHalSerialPinRts,
    FuriHalSerialPinCts,

    FuriHalSerialPinMax,
} FuriHalSerialPin;

typedef enum {
    FuriHalSerialConfigDataBits7,
    FuriHalSerialConfigDataBits8,
    FuriHalSerialConfigDataBits9,
} FuriHalSerialConfigDataBits;

typedef enum {
    FuriHalSerialConfigParityNone,
    FuriHalSerialConfigParityEven,
    FuriHalSerialConfigParityOdd,
} FuriHalSerialConfigParity;

typedef enum {
    FuriHalSerialConfigStopBits_0_5,
    FuriHalSerialConfigStopBits_1,
    FuriHalSerialConfigStopBits_1_5,
    FuriHalSerialConfigStopBits_2,
} FuriHalSerialConfigStopBits;

typedef enum {
    FuriHalSerialTransferBitOrderLsbFirst,
    FuriHalSerialTransferBitOrderMsbFirst,
} FuriHalSerialTransferBitOrder;

typedef enum {
    FuriHalSerialBinaryDataLogicPositive,
    FuriHalSerialBinaryDataLogicNegative,
} FuriHalSerialBinaryDataLogic;

typedef struct FuriHalSerialHandle FuriHalSerialHandle;
