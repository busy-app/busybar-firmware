#pragma once

/**
 * UART channels
 */
typedef enum {
    FuriHalSerialIdUsart0,
    FuriHalSerialIdUart1,
    FuriHalSerialIdUlpuart,
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

typedef struct FuriHalSerialHandle FuriHalSerialHandle;
