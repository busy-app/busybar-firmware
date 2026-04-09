#include "power_cli.h"

#include <furi_hal.h>
#include <cli/cli_command.h>
#include <cli/args.h>
#include <toolbox/property.h>
#include <power/power_service/power.h>

#include <brightness_control/brightness_control.h>
#include <front_display/front_display.h>
#include <gui/gui.h>
#include <gui/modules/canvas.h>

#ifndef POWER_CLI_DEBUG
#define POWER_CLI_DEBUG 1
#endif

static void
    power_cli_print_property(const char* key, const char* value, bool last, void* context) {
    UNUSED(last);
    UNUSED(context);
    printf("%-30s: %s\r\n", key, value);
}

static void power_cli_off(PipeSide* pipe, FuriString* args) {
    UNUSED(pipe);
    UNUSED(args);
    Power* power = furi_record_open(RECORD_POWER);
    printf("Disconnect USB for shutdown\r\n");

    furi_delay_ms(100);
    bool success = false;
    do {
        success = power_off(power);
        furi_delay_ms(1000);
    } while(!success);
}

static void power_cli_reboot(PipeSide* pipe, FuriString* args) {
    UNUSED(pipe);

    Power* power = furi_record_open(RECORD_POWER);

    FuriString* mode = furi_string_alloc();
    if(!args_read_string_and_trim(args, mode)) {
        power_reboot(power, PowerRebootNormal);
    } else {
        if(furi_string_cmp_str(mode, "sw") == 0) {
            power_reboot(power, PowerRebootNormal);
        } else if(furi_string_cmp_str(mode, "hw") == 0) {
            power_reboot(power, PowerRebootHardware);
        } else if(furi_string_cmp_str(mode, "u5") == 0) {
            power_reboot(power, PowerRebootNormalU5);
        } else if(furi_string_cmp_str(mode, "917") == 0) {
            power_reboot(power, PowerRebootNormal917);
        } else {
            cli_print_usage("power reboot", "<sw|hw|u5|917>", furi_string_get_cstr(args));
        }
    }

    furi_string_free(mode);
    furi_record_close(RECORD_POWER);
}

static void power_cli_reboot2dfu(PipeSide* pipe, FuriString* args) {
    UNUSED(pipe);

    Power* power = furi_record_open(RECORD_POWER);

    FuriString* mode = furi_string_alloc();
    bool args_error = true;
    if(args_read_string_and_trim(args, mode)) {
        if(furi_string_cmp_str(mode, "u5") == 0) {
            args_error = false;
            power_reboot(power, PowerRebootDfuU5);
        } else if(furi_string_cmp_str(mode, "917") == 0) {
            args_error = false;
            power_reboot(power, PowerRebootDfu917);
        }
    }

    if(args_error) {
        cli_print_usage("power boot", "<u5|917>", furi_string_get_cstr(args));
    }

    furi_string_free(mode);
    furi_record_close(RECORD_POWER);
}

static void power_cli_charger_on_off(PipeSide* pipe, FuriString* args) {
    UNUSED(pipe);

    bool args_error = true;
    int value = 0;

    Power* power = furi_record_open(RECORD_POWER);

    if(args_read_int_and_trim(args, &value)) {
        if((value == 0) || (value == 1)) {
            args_error = false;
            power_charge_enable(power, value);
        }
    }

    furi_record_close(RECORD_POWER);

    if(args_error) {
        cli_print_usage("power ch", "<1|0>", furi_string_get_cstr(args));
    }
}

static void power_cli_charger_current(PipeSide* pipe, FuriString* args) {
    UNUSED(pipe);

    bool args_error = true;
    int value = 0;

    Power* power = furi_record_open(RECORD_POWER);

    if(args_read_int_and_trim(args, &value)) {
        if((value > 0) && (value <= POWER_CHARGE_CURRENT_MAX)) {
            args_error = false;
            power_set_charge_current(power, value);
        }
    }

    furi_record_close(RECORD_POWER);

    if(args_error) {
        cli_print_usage("power ch_current", "<ma>", furi_string_get_cstr(args));
    }
}

static void power_cli_pd_info(PipeSide* pipe, FuriString* args) {
    UNUSED(args);

    Power* power = furi_record_open(RECORD_POWER);
    PowerPdInfo pd_info;
    power_get_pd_info(power, &pd_info);
    furi_record_close(RECORD_POWER);

    FuriString* value = furi_string_alloc();
    FuriString* key = furi_string_alloc();

    PropertyValueContext prop_ctx = {
        .key = key,
        .value = value,
        .out = power_cli_print_property,
        .sep = '.',
        .last = false,
        .context = pipe,
    };

    property_value_out(&prop_ctx, "%u", 2, "PD", "cc_line", pd_info.cc_line);
    property_value_out(
        &prop_ctx, "%u mA", 2, "PD", "passive_current_limit", pd_info.passive_mode_current);
    property_value_out(&prop_ctx, "%u mV", 2, "PD", "voltage", pd_info.voltage_set);
    property_value_out(&prop_ctx, "%u mA", 2, "PD", "current", pd_info.current_max);
    property_value_out(&prop_ctx, "%u", 2, "PD", "mode_id", pd_info.cap_id);
    property_value_out(&prop_ctx, "%u", 2, "PD", "capabilities_number", pd_info.cap_number);
    for(uint8_t i = 0; i < pd_info.cap_number; i++) {
        char cap_key[8];
        snprintf(cap_key, sizeof(cap_key), "cap_%u", pd_info.cap[i].pdo_id);
        if(pd_info.cap[i].is_fixed) {
            property_value_out(
                &prop_ctx,
                "FIX %u mV %u mA",
                2,
                "PD",
                cap_key,
                pd_info.cap[i].voltage_max,
                pd_info.cap[i].current_max);
        } else {
            property_value_out(
                &prop_ctx,
                "PPS %u-%u mV %u mA",
                2,
                "PD",
                cap_key,
                pd_info.cap[i].voltage_min,
                pd_info.cap[i].voltage_max,
                pd_info.cap[i].current_max);
        }
    }

    furi_string_free(value);
    furi_string_free(key);
}

static void power_cli_pd_request(PipeSide* pipe, FuriString* args) {
    UNUSED(pipe);

    bool args_error = true;
    int value = 0;

    Power* power = furi_record_open(RECORD_POWER);

    if(args_read_int_and_trim(args, &value)) {
        if((value > 0) && (value <= 20000)) {
            args_error = false;
            power_set_pd_mode(power, value);
        }
    }

    furi_record_close(RECORD_POWER);

    if(args_error) {
        cli_print_usage("power pd_set", "<mv>", furi_string_get_cstr(args));
    }
}

#if POWER_CLI_DEBUG == 1
static void power_cli_info_print_debug(PropertyValueContext* prop_ctx, const PowerInfo* info) {
    property_value_out(
        prop_ctx,
        "%02X, %02X, %02X, %02X, %02X",
        2,
        "charger",
        "status_raw",
        info->debug.charger_status.data[0],
        info->debug.charger_status.data[1],
        info->debug.charger_status.data[2],
        info->debug.charger_status.data[3],
        info->debug.charger_status.data[4]);
    property_value_out(
        prop_ctx,
        "%02X, %02X",
        2,
        "charger",
        "fault_raw",
        info->debug.charger_fault.data[0],
        info->debug.charger_fault.data[1]);

    const Bq25798ChargerFault* f = &(info->debug.charger_fault);
    property_value_out(prop_ctx, "%u", 3, "charger", "fault", "IBAT_REG", f->ibat_reg);
    property_value_out(prop_ctx, "%u", 3, "charger", "fault", "VBUS_OVP", f->vbus_ovp);
    property_value_out(prop_ctx, "%u", 3, "charger", "fault", "VBAT_OVP", f->vbat_ovp);
    property_value_out(prop_ctx, "%u", 3, "charger", "fault", "IBUS_OCP", f->ibus_ocp);
    property_value_out(prop_ctx, "%u", 3, "charger", "fault", "IBAT_OCP", f->ibat_ocp);
    property_value_out(prop_ctx, "%u", 3, "charger", "fault", "CONV_OCP", f->conv_ocp);
    property_value_out(prop_ctx, "%u", 3, "charger", "fault", "VAC2_OVP", f->vac2_ovp);
    property_value_out(prop_ctx, "%u", 3, "charger", "fault", "VAC1_OVP", f->vac1_ovp);

    property_value_out(prop_ctx, "%u", 3, "charger", "fault", "VSYS_SHORT", f->vsys_short);
    property_value_out(prop_ctx, "%u", 3, "charger", "fault", "VSYS_OVP", f->vsys_ovp);
    property_value_out(prop_ctx, "%u", 3, "charger", "fault", "OTG_OVP", f->otg_ovp);
    property_value_out(prop_ctx, "%u", 3, "charger", "fault", "OTG_UVP", f->otg_uvp);
    property_value_out(prop_ctx, "%u", 3, "charger", "fault", "THERM_SHUT", f->therm_shut);
}

static void power_cli_info_print_battery_soc(PropertyValueContext* prop_ctx, const BatterySocLevel* level) {
    if(level->flags & BatterySocLevelFlagNoData) {
        property_value_out(prop_ctx, "%s", 1, "battery", "unknown");
        return;
    }

    property_value_out(prop_ctx, "%.2f%%", 2, "battery", "state_of_charge", level->charge_percent);
    property_value_out(prop_ctx, "%s", 2, "battery", "algorithm", (level->flags & BatterySocLevelFlagChargeAccurate) ? "coulombs" : "volts");

    if(level->flags & BatterySocLevelFlagKnownHealth) {
        property_value_out(prop_ctx, "%.2f%%", 2, "battery", "health", level->health_percent);
    } else {
        property_value_out(prop_ctx, "%s", 2, "battery", "health", "unknown");
    }

    if(level->flags & BatterySocLevelFlagKnownDetails) {
        property_value_out(prop_ctx, "%.2f%%", 3, "battery", "soc", "v_based", level->detailed.voltage_based_percent);
        property_value_out(prop_ctx, "%.2f%%", 3, "battery", "soc", "c_based", level->detailed.charge_based_percent);
        property_value_out(prop_ctx, "%.2f%%", 3, "battery", "soc", "error", level->detailed.charge_error);
        property_value_out(prop_ctx, "%.2f%%", 3, "battery", "soc", "real_low", level->charge_percent * (1.0f - (level->detailed.charge_error / 100.0f)));
        property_value_out(prop_ctx, "%.2f%%", 3, "battery", "soc", "real_high", level->charge_percent * (1.0f + (level->detailed.charge_error / 100.0f)));
        property_value_out(prop_ctx, "%ld mAh", 3, "battery", "params", "chg_capacity", level->detailed.charge_capacity_mah);
        property_value_out(prop_ctx, "%ld mAh", 3, "battery", "params", "dischg_capacity", level->detailed.discharge_capacity_mah);
        property_value_out(prop_ctx, "%.2f%%", 3, "battery", "params", "coulombic_efficiency", level->detailed.efficiency);
        property_value_out(prop_ctx, "%.2f", 3, "battery", "params", "cycle_count", level->detailed.charge_cycles);
    } else {
        property_value_out(prop_ctx, "%s", 2, "battery", "soc", "unknown");
        property_value_out(prop_ctx, "%s", 2, "battery", "params", "unknown");
    }
}
#endif

static void power_cli_info(PipeSide* pipe, FuriString* args) {
    UNUSED(args);

    Power* power = furi_record_open(RECORD_POWER);
    PowerInfo info;
    power_get_info(power, &info);
    furi_record_close(RECORD_POWER);

    FuriString* value = furi_string_alloc();
    FuriString* key = furi_string_alloc();

    PropertyValueContext prop_ctx = {
        .key = key,
        .value = value,
        .out = power_cli_print_property,
        .sep = '.',
        .last = false,
        .context = pipe,
    };
    if(info.is_charging) {
        property_value_out(
            &prop_ctx, "%s", 1, "state", (info.is_full_charged) ? "charged" : "charging");
    } else {
        property_value_out(&prop_ctx, "%s", 1, "state", "discharging");
    }

    property_value_out(&prop_ctx, "%u%%", 2, "BAT", "level", info.charge);
    property_value_out(&prop_ctx, "%.0f mV", 2, "BAT", "voltage", info.voltage_battery);
    property_value_out(&prop_ctx, "%d mA", 2, "BAT", "current", info.current_battery);
    property_value_out(&prop_ctx, "%.1fC", 2, "BAT", "NTC", info.temperature_battery);

    property_value_out(&prop_ctx, "%.0f mV", 2, "USB", "voltage", info.voltage_usb);
    property_value_out(&prop_ctx, "%u mA", 2, "USB", "current", info.current_usb);
    property_value_out(&prop_ctx, "%u mA", 2, "USB", "current_limit", info.charge_ilim_usb);

    property_value_out(&prop_ctx, "%u", 2, "charger", "enabled", info.charge_enabled);
    property_value_out(
        &prop_ctx, "%u mA", 2, "charger", "current_limit", info.charge_ilim_battery);
    property_value_out(&prop_ctx, "%.1fC", 2, "charger", "temperature", info.temperature_charger);

#if POWER_CLI_DEBUG == 1
    power_cli_info_print_debug(&prop_ctx, &info);
    power_cli_info_print_battery_soc(&prop_ctx, &info.battery_details);
#endif

    furi_string_free(value);
    furi_string_free(key);
}

static void power_cli_drain(PipeSide* pipe, int i_requested) {
    Gui* gui = furi_record_open(RECORD_GUI);
    BrightnessControl* brightness_ctl = furi_record_open(RECORD_BRIGHTNESS_CONTROL);
    Power* power = furi_record_open(RECORD_POWER);

    power_switch_alerts(power, false);
    brightness_control_set_brightness_override(brightness_ctl, BrightnessControlModuleFrontDisplay, 100);

    Widget* root;
    Canvas* canvas;
    with_gui(gui, {
        root = gui_layer_get_root_widget(gui_get_layer(gui, GuiLayerIdTop), GuiDisplayIdFront);
        canvas = canvas_alloc(root, FRONT_DISPLAY_W, FRONT_DISPLAY_H);
    });

    int32_t brightness = 0;
    FuriWait last_print = 0;

    while(!cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
        PowerInfo power_info;
        power_get_info(power, &power_info);

        int i_battery = -power_info.current_battery;
        int i_error = i_battery - i_requested;
        if(abs(i_error) > 10) {
            if(i_error > 0) brightness--;
            if(i_error < 0) brightness++;
        }

        brightness = CLAMP(brightness, UINT8_MAX, 0);

        if((furi_get_tick() - last_print) >= 500) {
            printf("requested=%dmA, actual=%dmA, error=%dmA, brightness=%ld/255\r\n", i_requested, i_battery, i_error, brightness);
            last_print = furi_get_tick();
        }

        with_gui(gui, {
            canvas_draw_begin(canvas);
            canvas_set_line_color(canvas, (Color)COLOR_MAKE_RGB(brightness, brightness, brightness));
            canvas_set_fill_color(canvas, (Color)COLOR_MAKE_RGB(brightness, brightness, brightness));
            canvas_draw_rect(canvas, 0, 0, FRONT_DISPLAY_W, FRONT_DISPLAY_H, true);
            canvas_draw_end(canvas);
        });

        furi_delay_ms((abs(i_error) > 100) ? 25 : 200);
    }

    with_gui(gui, {
        canvas_free(canvas);
    });

    brightness_control_reset_brightness_override(brightness_ctl, BrightnessControlModuleFrontDisplay);
    power_switch_alerts(power, true);
    
    furi_record_close(RECORD_POWER);
    furi_record_close(RECORD_BRIGHTNESS_CONTROL);
    furi_record_close(RECORD_GUI);
}

static void power_cli_command_print_usage(void) {
    printf("Usage:\r\n");
    printf("power <cmd> <args>\r\n");
    printf("Cmd list:\r\n");

    printf("\tinfo\t - power information\r\n");
    printf("\toff\t - shutdown power\r\n");
    printf("\treboot\t - reboot\r\n");
    printf("\tboot\t - reboot to DFU bootloader\r\n");
    printf("\tch\t - charge on/off\r\n");
    printf("\tch_current\t - charge current limit\r\n");
    printf("\tpd_info\t - USB PD info\r\n");
    printf("\tpd_set\t - Request USB PD profile\r\n");
    printf("\tdrain <current in mA>\t - use front display to drain a specific current from the battery\r\n");
}

void power_cli_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            power_cli_command_print_usage();
            break;
        }

        if(furi_string_cmp_str(cmd, "off") == 0) {
            power_cli_off(pipe, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "reboot") == 0) {
            power_cli_reboot(pipe, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "boot") == 0) {
            power_cli_reboot2dfu(pipe, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "info") == 0) {
            power_cli_info(pipe, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "ch") == 0) {
            power_cli_charger_on_off(pipe, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "ch_current") == 0) {
            power_cli_charger_current(pipe, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "pd_info") == 0) {
            power_cli_pd_info(pipe, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "pd_list") == 0) {
            // power_cli_pd_list(pipe, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "pd_set") == 0) {
            power_cli_pd_request(pipe, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "drain") == 0) {
            int current_ma;
            if(!args_read_int_and_trim(args, &current_ma)) {
                power_cli_command_print_usage();
                break;
            }
            power_cli_drain(pipe, current_ma);
            break;
        }

        power_cli_command_print_usage();
    } while(false);

    furi_string_free(cmd);
}
