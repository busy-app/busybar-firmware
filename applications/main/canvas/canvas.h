#pragma once

#include <furi.h>
#include <m-array.h>
#include <gui/gui.h>

#define RECORD_CANVAS "CANVAS"

typedef struct CanvasApp CanvasApp;

typedef enum {
    CanvasElementTypeImage,
    CanvasElementTypeText,
} CanvasElementType;

typedef struct {
    char* app_scoped_id;
    uint32_t timeout;
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
    };
} CanvasElement;

static inline void canvas_element_clear(CanvasElement* obj) {
    if(obj->app_scoped_id) free(obj->app_scoped_id);
    if(obj->type == CanvasElementTypeImage) {
        if(obj->image.file_path) furi_string_free(obj->image.file_path);
    } else if(obj->type == CanvasElementTypeText) {
        if(obj->text.text_str) free(obj->text.text_str);
    }
}

ARRAY_DEF(
    CanvasElementsArray,
    CanvasElement,
    M_OPEXTEND(M_POD_OPLIST, CLEAR(API_2(canvas_element_clear))))

bool canvas_show_elements(CanvasApp* canvas, const char* app_id, CanvasElementsArray_t elements);

/**
 * @brief Delete elements by filter and possibly terminate Canvas
 * 
 * Deletes ALL elements (`app_id` is NULL) or elements related to a non-NULL
 * `app_id`. If no elements are left after this possibly selective delete, the
 * Canvas terminates itself.
 */
void canvas_delete_elements(CanvasApp* canvas, const char* app_id);
