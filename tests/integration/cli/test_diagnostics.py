"""Read-only diagnostic commands — device/system info that only reports state.

Coverage matrix and plan: scratchpad/cli_coverage_matrix.md.
"""

import re

import allure
import pytest

pytestmark = pytest.mark.cli


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Commands Check")
class TestCLIDiagnostics:
    """Read-only diagnostics: nothing here changes device state."""

    @allure.title("CLI. Command netstat.")
    def test_netstat(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("netstat")
        assert "Proto" in response and "Local Address" in response, response
        assert "LISTEN" in response, "expected at least one listening socket"

    @allure.title("CLI. Command fontstat.")
    def test_fontstat(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("fontstat")
        assert "Loaded:" in response and "fonts" in response, response
        assert "Font" in response and "Size" in response, "expected a font table"

    @allure.title("CLI. Command light_sensor.")
    def test_light_sensor(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("light_sensor")
        assert "data0:" in response and "data1:" in response, response
        assert "lux" in response, "expected an illuminance value"

    @allure.title("CLI. Command date.")
    def test_date(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("date")
        # ISO-8601, e.g. 2026-06-29T12:37:41+01:00
        assert re.search(
            r"\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}", response
        ), f"expected an ISO-8601 timestamp, got: {response!r}"

    @allure.title("CLI. Command timezone.")
    def test_timezone(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("timezone")
        assert response.strip(), "timezone should return a non-empty zone name"

    @allure.id("2043")
    @allure.title("CLI. Command Free.")
    @pytest.mark.story_commands_check
    def test_cli_command_free(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("free")

        for field in (
            "Free heap size:",
            "Total heap size:",
            "Minimum heap size:",
            "Maximum heap block:",
            "Pool free:",
            "Maximum pool block:",
        ):
            assert field in response, f"Response should contain '{field}'"

        free_heap = re.search(r"Free heap size:\s*(\d+)", response)
        assert free_heap, f"Could not parse free heap size: {response!r}"
        assert int(free_heap.group(1)) > 25000, f"Free heap too low: {free_heap.group(1)}"

    @allure.id("2034")
    @allure.title("CLI. Command Free_blocks.")
    @pytest.mark.story_commands_check
    def test_cli_command_free_blocks(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("free_blocks")
        # heap block dump: lines like 'A 200B2660 S 1821080'
        assert re.search(r"A\s+[0-9A-Fa-f]+\s+S\s+\d+", response), response

    @allure.id("2041")
    @allure.title("CLI. Command Uptime.")
    @pytest.mark.story_commands_check
    def test_cli_command_uptime(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("uptime")
        assert response.strip(), "Uptime command should return system uptime"
        assert any(
            unit in response.lower() for unit in ("d", "h", "m", "s")
        ), f"Uptime should contain time units, got: {response}"

    @allure.id("2035")
    @allure.title("CLI. Command Device_info.")
    @pytest.mark.story_commands_check
    def test_cli_command_device_info(self, persistent_cli_connection):
        # two parts: u5_* fields land at once, sl_* fields after a ~2s round-trip to 917
        response = persistent_cli_connection.execute_command(
            "device_info", timeout=20.0, slow_command=True
        )
        assert (
            "u5_firmware_origin_fork       : Official" in response
        ), "Should include the correct origin fork"
        origin_git = re.search(r"u5_firmware_origin_git\s*:\s*(\S+)", response)
        assert origin_git, "Should include firmware origin git information"

    @allure.id("2031")
    @allure.title("CLI. Command Echo.")
    @pytest.mark.story_commands_check
    def test_cli_command_echo(self, persistent_cli_connection):
        message = "Hello BSB Test"
        response = persistent_cli_connection.execute_command(f'echo "{message}"')
        assert message in response, f"Echo should return the input message: {message}"
