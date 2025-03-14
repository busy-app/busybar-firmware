#include "power_cli.h"

#include <furi_hal.h>
#include <cli/cli.h>
#include <toolbox/args.h>
#include <toolbox/property.h>
#include <power/power_service/power.h>

static void
    power_cli_print_property(const char* key, const char* value, bool last, void* context) {
    UNUSED(last);
    UNUSED(context);
    printf("%-30s: %s\r\n", key, value);
}

static void power_cli_off(Cli* cli, FuriString* args) {
    UNUSED(cli);
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

static void power_cli_reboot(Cli* cli, FuriString* args) {
    UNUSED(cli);

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

static void power_cli_reboot2dfu(Cli* cli, FuriString* args) {
    UNUSED(cli);

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

static void power_cli_charger_on_off(Cli* cli, FuriString* args) {
    UNUSED(cli);

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

static void power_cli_charger_current(Cli* cli, FuriString* args) {
    UNUSED(cli);

    bool args_error = true;
    int value = 0;

    Power* power = furi_record_open(RECORD_POWER);

    if(args_read_int_and_trim(args, &value)) {
        if((value > 0) && (value <= CHARGE_CURRENT_MAX)) {
            args_error = false;
            power_set_charge_current(power, value);
        }
    }

    furi_record_close(RECORD_POWER);

    if(args_error) {
        cli_print_usage("power ch_current", "<ma>", furi_string_get_cstr(args));
    }
}

static void power_cli_pd_info(Cli* cli, FuriString* args) {
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
        .context = cli,
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

static void power_cli_pd_request(Cli* cli, FuriString* args) {
    UNUSED(cli);

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
        cli_print_usage("power ch_set", "<mv>", furi_string_get_cstr(args));
    }
}

static void power_cli_info(Cli* cli, FuriString* args) {
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
        .context = cli,
    };
    if(info.is_charging) {
        property_value_out(
            &prop_ctx, "%s", 1, "state", (info.is_full_charged) ? "charged" : "charging");
    } else {
        property_value_out(&prop_ctx, "%s", 1, "state", "discharging");
    }

    property_value_out(&prop_ctx, "%u%%", 2, "BAT", "level", info.charge);
    property_value_out(&prop_ctx, "%u mV", 2, "BAT", "voltage", info.voltage_battery);
    property_value_out(&prop_ctx, "%d mA", 2, "BAT", "current", info.current_battery);
    property_value_out(&prop_ctx, "%.1f%%", 2, "BAT", "NTC", info.temperature_battery);

    property_value_out(&prop_ctx, "%u mV", 2, "USB", "voltage", info.voltage_usb);
    property_value_out(&prop_ctx, "%u mA", 2, "USB", "current", info.current_usb);
    property_value_out(&prop_ctx, "%u mA", 2, "USB", "current_limit", info.charge_ilim_usb);

    property_value_out(&prop_ctx, "%u", 2, "charger", "enabled", info.charge_enabled);
    property_value_out(
        &prop_ctx, "%u mA", 2, "charger", "current_limit", info.charge_ilim_battery);
    property_value_out(&prop_ctx, "%.1f°C", 2, "charger", "temperature", info.temperature_charger);

    furi_string_free(value);
    furi_string_free(key);
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
}

static void power_cli(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            power_cli_command_print_usage();
            break;
        }

        if(furi_string_cmp_str(cmd, "off") == 0) {
            power_cli_off(cli, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "reboot") == 0) {
            power_cli_reboot(cli, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "boot") == 0) {
            power_cli_reboot2dfu(cli, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "info") == 0) {
            power_cli_info(cli, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "ch") == 0) {
            power_cli_charger_on_off(cli, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "ch_current") == 0) {
            power_cli_charger_current(cli, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "pd_info") == 0) {
            power_cli_pd_info(cli, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "pd_list") == 0) {
            // power_cli_pd_list(cli, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "pd_set") == 0) {
            power_cli_pd_request(cli, args);
            break;
        }

        power_cli_command_print_usage();
    } while(false);

    furi_string_free(cmd);
}

void power_on_system_start(void) {
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, "power", CliCommandFlagParallelSafe, power_cli, NULL);
    furi_record_close(RECORD_CLI);
}
