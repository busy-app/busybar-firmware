"""`power` command: usage plus the read-only info subcommands.

Charging subcommands (ch/ch_current/ch_limit) and off/reboot/boot are L2/L3 and
not exercised here. Coverage matrix: scratchpad/cli_coverage_matrix.md.
"""

import allure
import pytest

pytestmark = pytest.mark.cli


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Commands Check")
class TestCLIPower:
    """Read-only power queries and usage."""

    @allure.id("2026")
    @allure.title("CLI. Command Power.")
    @pytest.mark.story_commands_check
    def test_cli_command_power(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("power")
        assert "Usage:" in response, "Power command should provide help information"
        assert "power <cmd> <args>" in response, "Should show usage format"
        assert "Cmd list:" in response, "Should show available subcommands"

    @allure.title("CLI. Command power info.")
    def test_power_info(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("power info")
        for field in ("state", "BAT.level", "BAT.voltage", "USB.voltage", "charger.enabled"):
            assert field in response, f"missing field {field}: {response!r}"

    @allure.title("CLI. Command power pd_info.")
    def test_power_pd_info(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("power pd_info")
        for field in ("PD.cc_line", "PD.voltage", "PD.current"):
            assert field in response, f"missing field {field}: {response!r}"
