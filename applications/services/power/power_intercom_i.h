#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PowerIntercomMessageTypeDeepSleepWithWakeupOnModeSwitchOutOfOffPosition,
} PowerIntercomMessageType;

typedef struct {
    PowerIntercomMessageType type;
} PowerIntercomMessage;

#ifdef __cplusplus
}
#endif
