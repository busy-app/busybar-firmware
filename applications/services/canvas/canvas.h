#pragma once

#include <furi.h>
#include <m-array.h>
#include <gui/gui.h>
#include <gui/modules/countdown.h>
#include <gui/modules/anim_player.h>
#include <gui/modules/image.h>
#include <loader/loader.h>
#include <time.h>

#define RECORD_CANVAS       "CANVAS"
#define CANVAS_MAX_PRIORITY LOADER_MAX_PRIORITY
#define CANVAS_MAX_ELEMENTS 100

typedef struct CanvasSrv CanvasSrv;

typedef enum {
    CanvasResultOk = 0,
    CanvasResultBadParameters,
    CanvasResultLowPriority,
    CanvasResultEmptyScreen,
    CanvasResultTooManyElements,
    CanvasResultNonexistentElementId,
    CanvasResultWrongAppId,

    CanvasResultMax,
} CanvasResult;

typedef enum {
    CanvasElementTypeImage,
    CanvasElementTypeAnimPlayer,
    CanvasElementTypeText,
    CanvasElementTypeCountdown,
    CanvasElementTypeRectangle,
    CanvasElementTypeRawImage,

    CanvasElementTypeMax,
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
    int32_t z_index;

    union {
        struct {
            FuriString* file_path;
            uint8_t opacity;
        } image;

        struct {
            FuriString* file_path;
            FuriString* section;
            AnimFilePlayFlag flags;
            uint8_t opacity;
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

        struct {
            size_t width;
            size_t height;
            size_t radius;
            size_t border_width;
            enum {
                RectangleFillNone,
                RectangleFillSolid,
                RectangleFillGradientH,
                RectangleFillGradientV,

                RectangleFillMax,
            } fill;
            Color fill_color[2];
            Color border_color;
        } rectangle;

        struct {
            void* data;
            size_t data_size;
            uint32_t width;
            uint32_t height;
            ImageColorFormat format;
            uint8_t opacity;
        } raw_image;
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
    } else if(obj->type == CanvasElementTypeRawImage) {
        if(obj->raw_image.data) free(obj->raw_image.data);
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
    } else if(src->type == CanvasElementTypeRawImage) {
        if(src->raw_image.data) {
            obj->raw_image.data = malloc(src->raw_image.data_size);
            memcpy(obj->raw_image.data, src->raw_image.data, src->raw_image.data_size);
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
 * `app_id`.
 * 
 * Elements are further filtered by the `element_ids` list. If the list is
 * provided, element IDs that are not in the list are kept. The list is
 * terminated with a NULL string pointer.
 * 
 * If no elements are left after this possibly selective delete, the Canvas
 * closes itself.
 */
CanvasResult
    canvas_delete_elements(CanvasSrv* canvas, const char* app_id, const char* const* element_ids);

CanvasResult canvas_get_app_id(CanvasSrv* canvas, FuriString* string);

/**
 * @brief Maximum length of the app id stored in CanvasOwnershipInfo.
 */
#define CANVAS_OWNER_APP_ID_MAX (64)

/**
 * @brief Current canvas ownership (which HTTP API application owns the canvas).
 *
 * Published as a FuriState (see `canvas_get_ownership_state()`) whenever the
 * canvas goes from idle to owned, the owner changes, or the canvas is released.
 */
typedef struct {
    bool is_active; /**< Canvas is currently showing content (has an owner) */
    char app_id[CANVAS_OWNER_APP_ID_MAX + 1]; /**< App id from the draw calls, empty when inactive */
    size_t priority; /**< Draw priority of the current owner (0 when inactive) */
} CanvasOwnershipInfo;

/**
 * @brief Get the canvas ownership FuriState.
 *
 * The state stores a `CanvasOwnershipInfo` value.
 *
 * @param[in] canvas canvas service instance
 * @returns FuriState handle
 */
FuriState* canvas_get_ownership_state(CanvasSrv* canvas);
