#pragma once

#include <furi.h>
#include <m-array.h>
#include <gui/gui.h>
#include <gui/modules/countdown.h>
#include <time.h>

#define RECORD_CANVAS "CANVAS"

typedef struct CanvasApp CanvasApp;

typedef enum {
    CanvasElementTypeImage,
    CanvasElementTypeText,
    CanvasElementTypeCountdown,
} CanvasElementType;

typedef struct {
    char* id;
    uint32_t timeout;
    time_t display_until;
    int16_t x;
    int16_t y;
    GuiDisplayId display;
    Align align;
    CanvasElementType type;
    union {
        struct {
            FuriString* file_path;
        } image;
        struct {
            char* text_str;
            GuiFont font;
            Color color;
            size_t width;
            size_t scroll_rate_cpm;
        } text;
        struct {
            time_t timestamp;
            Color color;
            CountdownDirection direction;
            CountdownShowHour hours;
        } countdown;
    };
} CanvasElement;

static inline void canvas_element_clear(CanvasElement* obj) {
    if(obj->id) {
        free(obj->id);
        obj->id = NULL;
    }
    if(obj->type == CanvasElementTypeImage) {
        if(obj->image.file_path) furi_string_free(obj->image.file_path);
    } else if(obj->type == CanvasElementTypeText) {
        if(obj->text.text_str) free(obj->text.text_str);
    }
}

static inline void canvas_element_clone(CanvasElement* obj, const CanvasElement* src) {
    memcpy(obj, src, sizeof(CanvasElement));
    if(src->id) obj->id = strdup(src->id);
    if(src->type == CanvasElementTypeImage) {
        if(src->image.file_path) {
            obj->image.file_path = furi_string_alloc_set(src->image.file_path);
        }
    } else if(src->type == CanvasElementTypeText) {
        if(src->text.text_str) {
            obj->text.text_str = strdup(src->text.text_str);
        }
    }
}

ARRAY_DEF(
    CanvasElementsArray,
    CanvasElement,
    M_OPEXTEND(
        M_POD_OPLIST,
        CLEAR(API_2(canvas_element_clear)),
        INIT_SET(API_6(canvas_element_clone))))

bool canvas_show_elements(CanvasApp* canvas, const char* app_id, CanvasElementsArray_t elements);

/**
 * @brief Delete elements by filter and possibly terminate Canvas
 * 
 * Deletes ALL elements (`app_id` is NULL) or elements related to a non-NULL
 * `app_id`. If no elements are left after this possibly selective delete, the
 * Canvas terminates itself.
 */
bool canvas_delete_elements(CanvasApp* canvas, const char* app_id);
