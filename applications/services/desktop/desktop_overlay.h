#include <gui/gui.h>

typedef struct DesktopOverlay DesktopOverlay;

typedef enum {
    DesktopOverlayTransitionTypeUp,
    DesktopOverlayTransitionTypeDown,

    DesktopOverlayTransitionTypeNone,

    DesktopOverlayTransitionTypesCount,
} DesktopOverlayTransitionType;

DesktopOverlay* desktop_overlay_alloc(Gui* gui);
void desktop_overlay_show(DesktopOverlay* instance, DesktopOverlayTransitionType type);
void desktop_overlay_hide(DesktopOverlay* instance, DesktopOverlayTransitionType type);
bool desktop_overlay_show_requested(const DesktopOverlay* instance);
