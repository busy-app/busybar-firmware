"""Device configuration commands: reversible `sysctl` toggles, `status_lights`, and
the debug-gated commands that `sysctl debug` registers.

Every toggle restores its previous value. Coverage matrix:
scratchpad/cli_coverage_matrix.md.
"""

import re

import allure
import pytest

from utils.cli_helpers import DEBUG_ONLY_COMMANDS, command_set

pytestmark = pytest.mark.cli


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Commands Check")
class TestCLIConfig:
    """Reversible config toggles and status_lights."""

    @allure.title("CLI. Command status_lights (usage).")
    def test_status_lights_usage(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("status_lights")
        assert "status_lights" in response and "0-255" in response, response

    @allure.title("CLI. Command status_lights set (off).")
    def test_status_lights_set(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("status_lights 0 0 0")
        assert "Usage" not in response

    @allure.title("CLI. Command sysctl (usage).")
    def test_sysctl_usage(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("sysctl")
        assert "sysctl <cmd>" in response and "Cmd list:" in response, response
        for sub in ("debug", "ui_debug", "cli_wifi_enabled", "websrv_accesslog_level"):
            assert sub in response, f"missing subcommand {sub}: {response!r}"

    @allure.title("CLI. Command sysctl debug (toggle, restored to enabled).")
    def test_sysctl_debug_toggle(self, persistent_cli_connection):
        # the device must be left in debug mode (see the cli_debug fixture), so check
        # the disable path first and always come back to enabled
        # what the flag actually gates is checked in TestCLIDebugGatedCommands
        cli = persistent_cli_connection
        try:
            off = cli.execute_command("sysctl debug 0")
            assert "Debug disabled" in off, off
        finally:
            on = cli.execute_command("sysctl debug 1")
            assert "Debug enabled" in on, on

    @allure.title("CLI. Command sysctl ui_debug (toggle-and-restore).")
    def test_sysctl_ui_debug_toggle(self, persistent_cli_connection):
        # ui_debug <0|1|2>; reversible, restore to 0 (off) afterwards
        cli = persistent_cli_connection
        try:
            on = cli.execute_command("sysctl ui_debug 1")
            assert "illegal option" not in on and "usage:" not in on.lower(), on
        finally:
            cli.execute_command("sysctl ui_debug 0")

    @allure.title("CLI. Command sysctl websrv_accesslog_level (toggle-and-restore).")
    def test_sysctl_websrv_accesslog_toggle(self, persistent_cli_connection):
        # websrv_accesslog_level <0|1|2|3>; reversible, restore to 0 afterwards
        cli = persistent_cli_connection
        try:
            on = cli.execute_command("sysctl websrv_accesslog_level 1")
            assert "illegal option" not in on and "usage:" not in on.lower(), on
        finally:
            cli.execute_command("sysctl websrv_accesslog_level 0")

    @allure.title("CLI. Command sysctl cli_wifi_enabled (toggle).")
    def test_sysctl_cli_wifi_enabled_toggle(self, persistent_cli_connection):
        # cli_wifi_enabled <1|0> controls CLI over WiFi; this session runs over
        # USB-Ethernet, so toggling does not drop it
        cli = persistent_cli_connection
        try:
            off = cli.execute_command("sysctl cli_wifi_enabled 0")
            assert "disabled" in off.lower(), off
        finally:
            on = cli.execute_command("sysctl cli_wifi_enabled 1")
            assert "enabled" in on.lower(), on


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Commands Check")
class TestCLIDebugGatedCommands:
    """Debug mode and what it gates (the `cli_debug` fixture keeps it on).

    `sysctl debug` re-registers the gated commands right away, no reboot needed. Only
    the read-only and usage paths are exercised: `gpio <pin> <0|1>` drives real
    hardware, `otp program` burns fuses and a bare `factory_reset` wipes the device —
    all L3.
    """

    @allure.title("CLI. Debug mode adds exactly the debug-gated commands to `?`.")
    def test_debug_command_list_diff(self, persistent_cli_connection):
        cli = persistent_cli_connection
        try:
            cli.execute_command("sysctl debug 0")
            without_debug = command_set(cli)
        finally:
            cli.execute_command("sysctl debug 1")
        with_debug = command_set(cli)

        assert with_debug - without_debug == DEBUG_ONLY_COMMANDS, (
            "debug mode should add exactly "
            f"{sorted(DEBUG_ONLY_COMMANDS)}, it added {sorted(with_debug - without_debug)}"
        )
        assert not without_debug - with_debug, (
            "debug mode must not hide commands, "
            f"it removed {sorted(without_debug - with_debug)}"
        )

    @allure.title("CLI. Command sysctl gates storage_bkp_unlock behind debug mode.")
    def test_sysctl_debug_gated_subcommand(self, persistent_cli_connection):
        # storage_bkp_unlock is gated inside `sysctl` itself, so it never shows up in
        # the top-level `?` diff above
        cli = persistent_cli_connection
        try:
            cli.execute_command("sysctl debug 0")
            assert "storage_bkp_unlock" not in cli.execute_command("sysctl")
        finally:
            cli.execute_command("sysctl debug 1")
        assert "storage_bkp_unlock" in cli.execute_command("sysctl")

    @allure.title("CLI. Command gpio (usage).")
    def test_gpio_usage(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("gpio")
        assert "gpio <pin_name> <0|1>" in response, response
        assert "Pins:" in response and "en_audio" in response, response

    @allure.title("CLI. Command otp (usage).")
    def test_otp_usage(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("otp")
        assert "otp <cmd>" in response and "Cmd list:" in response, response
        assert "dump" in response and "program" in response, response

    @allure.title("CLI. Command otp dump.")
    def test_otp_dump(self, persistent_cli_connection):
        # read-only; an unprovisioned bank reads back as all-ff, still a valid dump
        response = persistent_cli_connection.execute_command("otp dump OTP1")
        assert re.search(r"\b[0-9a-f]{32,}\b", response), f"expected a hex dump: {response!r}"

    @allure.title("CLI. Command factory_reset (help).")
    def test_factory_reset_help(self, persistent_cli_connection):
        # `-h` prints usage and returns; a bare `factory_reset` prompts 'Are you sure? y/n'
        # and would eat the next command as the answer -> never run it here
        response = persistent_cli_connection.execute_command("factory_reset -h")
        assert "Usage: factory_reset" in response, response
        assert "--shipping-mode" in response, response

    @allure.title("CLI. Command sysctl storage_bkp_unlock (toggle-and-restore).")
    def test_sysctl_storage_bkp_unlock(self, persistent_cli_connection):
        cli = persistent_cli_connection
        try:
            unlocked = cli.execute_command("sysctl storage_bkp_unlock 1")
            assert "Backup storage unlocked" in unlocked, unlocked
        finally:
            locked = cli.execute_command("sysctl storage_bkp_unlock 0")
            assert "Backup storage locked" in locked, locked
