#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Notificationtt Types */
typedef enum {
    NotificationttTypeSetColor, /**< Set color event */
    NotificationttTypeSetPreset, /**< Set preset event */
    NotificationttTypeStop, /**< Stop event */
    InputTypeMAX, /**< Special value for exceptional */
} NotificationttType;

/** Notificationt Event*/
typedef struct {
    NotificationttType type;
    union {
        int8_t brightness;
        int8_t red;
        int8_t green;
        int8_t blue;
        uint16_t preset;
    };
} InputEvent;

#ifdef __cplusplus
}
#endif
