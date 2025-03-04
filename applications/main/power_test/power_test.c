#include <furi_hal.h>
#include <furi.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>

#include <power_simple/power.h>

#define TAG "PowerTest"

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
} PowerTest;

typedef enum {
    PowerViewSubmenu,
} PowerView;

typedef enum {
    PowerTestSubmenuInfo,
    PowerTestSubmenuPdDebug,
    PowerTestSubmenuOff,
    PowerTestSubmenuShutdown,
    PowerTestSubmenuReset,
} PowerTestSubmenu;

static void power_test_submenu_callback(void* context, uint32_t index) {
    PowerTest* instance = (PowerTest*)context;
    UNUSED(instance);

    if(index == PowerTestSubmenuOff) {
        Power* power = furi_record_open(RECORD_POWER);
        power_off(power);
        furi_record_close(RECORD_POWER);
    } else if(index == PowerTestSubmenuShutdown) {
        Power* power = furi_record_open(RECORD_POWER);
        power_shutdown(power);
        furi_record_close(RECORD_POWER);
    } else if(index == PowerTestSubmenuReset) {
        Power* power = furi_record_open(RECORD_POWER);
        power_reboot(power, PowerRebootHardware);
        furi_record_close(RECORD_POWER);
    } else {
        furi_crash();
    }
}

static uint32_t power_test_exit_callback(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

PowerTest* power_test_alloc(void) {
    PowerTest* instance = malloc(sizeof(PowerTest));

    View* view = NULL;

    instance->gui = furi_record_open(RECORD_GUI);
    instance->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(
        instance->view_dispatcher, instance->gui, ViewDispatcherTypeFullscreen);

    // Menu
    instance->submenu = submenu_alloc();
    view = submenu_get_view(instance->submenu);
    view_set_previous_callback(view, power_test_exit_callback);
    view_dispatcher_add_view(instance->view_dispatcher, PowerViewSubmenu, view);
    // submenu_add_item(
    //     instance->submenu, "Info", PowerTestSubmenuInfo, power_test_submenu_callback, instance);
    // submenu_add_item(
    //     instance->submenu,
    //     "PD Debug",
    //     PowerTestSubmenuPdDebug,
    //     power_test_submenu_callback,
    //     instance);
    submenu_add_item(
        instance->submenu,
        "Off (Ship mode)",
        PowerTestSubmenuOff,
        power_test_submenu_callback,
        instance);
    submenu_add_item(
        instance->submenu,
        "Shutdown",
        PowerTestSubmenuShutdown,
        power_test_submenu_callback,
        instance);
    submenu_add_item(
        instance->submenu, "Reset", PowerTestSubmenuReset, power_test_submenu_callback, instance);

    return instance;
}

void power_test_free(PowerTest* instance) {
    view_dispatcher_remove_view(instance->view_dispatcher, PowerViewSubmenu);
    submenu_free(instance->submenu);

    view_dispatcher_free(instance->view_dispatcher);
    furi_record_close(RECORD_GUI);

    free(instance);
}

int32_t power_test_run(PowerTest* instance) {
    view_dispatcher_switch_to_view(instance->view_dispatcher, PowerViewSubmenu);
    view_dispatcher_run(instance->view_dispatcher);
    return 0;
}

int32_t power_test_app(void* p) {
    UNUSED(p);

    PowerTest* instance = power_test_alloc();

    int32_t ret = power_test_run(instance);

    power_test_free(instance);

    return ret;
}
