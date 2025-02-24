#include <gui_lvgl/gui_lvgl.h>

typedef struct DesktopOverlay DesktopOverlay;

DesktopOverlay* desktop_overlay_alloc(GuiLvgl* gui);
void desktop_overlay_show(DesktopOverlay* instance);
void desktop_overlay_hide(DesktopOverlay* instance);
bool desktop_overlay_show_requested(const DesktopOverlay* instance);
