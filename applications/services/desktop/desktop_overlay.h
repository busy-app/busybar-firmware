#include <gui/gui.h>

typedef struct DesktopOverlay DesktopOverlay;

DesktopOverlay* desktop_overlay_alloc(Gui* gui);
void desktop_overlay_show(DesktopOverlay* instance);
void desktop_overlay_hide(DesktopOverlay* instance);
bool desktop_overlay_show_requested(const DesktopOverlay* instance);
