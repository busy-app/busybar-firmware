#pragma once

#include <furi.h>
#include <m-array.h>
#include <gui/gui.h>
#include <gui/modules/countdown.h>
#include <gui/modules/anim_player.h>
#include <loader/loader.h>
#include <time.h>

#define RECORD_CANVAS       "CANVAS"
#define CANVAS_MAX_PRIORITY LOADER_MAX_PRIORITY

typedef struct CanvasSrv CanvasSrv;

typedef enum {
    CanvasResultOk = 0,
    CanvasResultBadParameters,
    CanvasResultLowPriority,
    CanvasResultEmptyScreen,

    CanvasResultMax,
} CanvasResult;

typedef enum {
    CanvasElementTypeImage,
    CanvasElementTypeAnimPlayer,
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
            FuriString* file_path;
            FuriString* section;
            AnimFilePlayFlag flags;
        } anim_player;

        struct {
            char* text_str;
            char* font_path;
            Color color;
            size_t width;
            size_t scroll_rate_cpm;
            size_t scroll_start_delay;
            size_t scroll_repeat_delay;
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
        if(obj->text.font_path) free(obj->text.font_path);
    } else if(obj->type == CanvasElementTypeAnimPlayer) {
        if(obj->anim_player.file_path) furi_string_free(obj->anim_player.file_path);
        if(obj->anim_player.section) furi_string_free(obj->anim_player.section);
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
        if(src->text.text_str) obj->text.text_str = strdup(src->text.text_str);
        if(src->text.font_path) obj->text.font_path = strdup(src->text.font_path);
    } else if(src->type == CanvasElementTypeAnimPlayer) {
        if(src->anim_player.file_path) {
            obj->anim_player.file_path = furi_string_alloc_set(src->anim_player.file_path);
        }
        if(src->anim_player.section) {
            obj->anim_player.section = furi_string_alloc_set(src->anim_player.section);
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

typedef void (*CanvasDrawCallback)(CanvasResult result, void* ctx);

CanvasResult canvas_show_elements(
    CanvasSrv* canvas,
    const char* app_id,
    size_t priority,
    CanvasElementsArray_t elements);

void canvas_show_elements_async(
    CanvasSrv* canvas,
    const char* app_id,
    size_t priority,
    CanvasElementsArray_t elements,
    CanvasDrawCallback callback,
    void* callback_ctx);

/**
 * @brief Delete elements by filter and possibly terminate Canvas
 * 
 * Deletes ALL elements (`app_id` is NULL) or elements related to a non-NULL
 * `app_id`. If no elements are left after this possibly selective delete, the
 * Canvas closes itself.
 */
CanvasResult canvas_delete_elements(CanvasSrv* canvas, const char* app_id);

CanvasResult canvas_get_app_id(CanvasSrv* canvas, FuriString* string);
