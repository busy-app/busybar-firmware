from __future__ import annotations

import re

import allure
import pytest

from clients.api import TelemetryAPI

pytestmark = pytest.mark.cli


TELEMETRY_EVENT_NAMES = {
    "device.boot",
    "fw.update",
    "timer.session.start",
    "timer.session.end",
    "timer.theme",
    "app.start",
    "app.stop",
    "setting.brightness",
    "setting.volume",
    "input.switch",
    "power.transition",
    "net.online",
    "net.offline",
    "net.offline_duration",
    "net.wifi.connect",
    "net.wifi.disconnect",
    "net.wifi.reconfigure",
    "account.link",
    "account.unlink",
    "canvas.acquire",
    "canvas.release",
}


def _stat_value(output: str, label: str) -> int:
    pattern = rf"^\s*{re.escape(label)}:\s*(\d+)\s*$"
    match = re.search(pattern, output, re.MULTILINE)
    assert match, f"Missing numeric telemetry stat {label!r}:\n{output}"
    return int(match.group(1))


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Telemetry")
class TestTelemetryCLI:
    @allure.title("CLI telemetry consent commands agree with the HTTP API")
    def test_telemetry_toggle_and_status(
        self,
        persistent_cli_connection,
        telemetry_api: TelemetryAPI,
        telemetry_state_guard,
    ):
        cli = persistent_cli_connection

        with allure.step("Disable telemetry through CLI"):
            response = cli.execute_command("telemetry off")
            assert "Telemetry collection disabled" in response, response
            assert telemetry_api.get_status().enabled is False, (
                "HTTP API still reports telemetry enabled after "
                "`telemetry off`"
            )

        with allure.step("Enable telemetry through CLI"):
            response = cli.execute_command("telemetry on")
            assert "Telemetry collection enabled" in response, response
            assert telemetry_api.get_status().enabled is True, (
                "HTTP API still reports telemetry disabled after "
                "`telemetry on`"
            )

        with allure.step("Read telemetry status through CLI"):
            response = cli.execute_command("telemetry status")
            assert "Telemetry collection is enabled" in response, response

    @allure.title(
        "CLI telemetry stats expose a sane initialized service state"
    )
    def test_telemetry_stats_are_sane(self, persistent_cli_connection):
        output = persistent_cli_connection.execute_command("telemetry stats")

        with allure.step(
            "Verify the stats command is available in debug mode"
        ):
            assert "Telemetry stats:" in output, output
            assert re.search(
                r"^\s*enabled:\s*(true|false)\s*$",
                output,
                re.MULTILINE,
            ), output
            assert re.search(
                r"^\s*connected:\s*(true|false)\s*$",
                output,
                re.MULTILINE,
            ), output

        with allure.step("Verify buffer and delivery counter invariants"):
            buffered = _stat_value(output, "buffered events")
            batches_sent = _stat_value(output, "batches sent")
            events_sent = _stat_value(output, "events sent")
            _stat_value(output, "events dropped")

            assert 0 <= buffered <= 32, (
                f"Telemetry ring reports {buffered} buffered events; "
                "capacity is 32"
            )
            assert batches_sent <= events_sent, (
                f"batches sent ({batches_sent}) exceeds events sent "
                f"({events_sent})"
            )

        with allure.step(
            "Verify counters exist for every documented event type"
        ):
            missing = {
                name
                for name in TELEMETRY_EVENT_NAMES
                if not re.search(
                    rf"^\s*{re.escape(name)}\s+\d+\s*$",
                    output,
                    re.MULTILINE,
                )
            }
            assert not missing, (
                f"Missing telemetry event counters {sorted(missing)}:\n"
                f"{output}"
            )

    @allure.title("CLI telemetry command prints usage for missing arguments")
    def test_telemetry_usage(self, persistent_cli_connection):
        output = persistent_cli_connection.execute_command("telemetry")

        assert "Usage:" in output, output
        for command in ("status", "on", "off", "stats"):
            assert f"telemetry {command}" in output, (
                f"Missing telemetry {command!r} command in usage:\n{output}"
            )
