/**
 * @file font_registry.h
 * @brief Loads fonts from storage
 */

#include <lvgl.h>
#include "fonts.h"

#define RECORD_FONT_REGISTRY "font_registry"

typedef struct FontRegistry FontRegistry;

/**
 * @param[inout] instance FontRegistry instance
 * @param[in] font_path Path to the font in Storage in LVGL format
 * 
 * @returns The requested font, or a fallback font if it couldn't be loaded
 * 
 * @warning Only use this function from widgets internally
 */
const lv_font_t* font_registry_load_font(FontRegistry* instance, const char* font_path);

/**
 * @param[inout] instance FontRegistry instance
 * @param[in] font Previously loaded font
 * 
 * @warning Only use this function from widgets internally
 */
void font_registry_unload_font(FontRegistry* instance, const lv_font_t* font);
