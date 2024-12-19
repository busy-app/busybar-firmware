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

typedef struct FuriHalSerialHandle FuriHalSerialHandle;
