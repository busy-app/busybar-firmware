"""Process/system commands: top, log, loader, update.

Coverage matrix and plan: scratchpad/cli_coverage_matrix.md.
"""

import re

import allure
import pytest

from utils.cli_helpers import run_streaming

pytestmark = pytest.mark.cli


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Commands Check")
class TestCLISystemCommands:
    """Process/system commands: top, log, loader, update."""

    @allure.title("CLI. Command top (single snapshot).")
    def test_top_snapshot(self, persistent_cli_connection):
        # `top <interval>` with interval 0 prints one frame and returns — no CTRL+C needed
        response = persistent_cli_connection.execute_command("top 0", timeout=15)
        assert re.search(r"Threads:\s+\d+", response), response
        assert "ISR Time:" in response and "Uptime:" in response, response
        assert re.search(r"Heap: total \d+, free \d+", response), response
        assert "AppID" in response and "Stack Min" in response, "missing the thread table header"
        # the table always lists at least the CLI shell and the storage service
        assert "CliShell" in response and "StorageSrv" in response, response

    @allure.title("CLI. Command top (streaming, CTRL+C).")
    @pytest.mark.regression  # streams frames at 1s intervals until two are seen
    def test_top_streaming(self, persistent_cli_connection):
        # default interval (1s) streams frames until ETX; stop as soon as two arrive
        response = run_streaming(
            persistent_cli_connection,
            "top",
            run_seconds=6.0,
            until=lambda t: t.count("Threads:") >= 2,
        )
        assert response.count("Threads:") >= 2, f"expected repeated frames: {response!r}"

    @allure.title("CLI. Command log (levels listing).")
    def test_log_levels(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("log ?")
        for level in ("error", "warn", "info", "debug", "trace"):
            assert f"log {level}" in response, f"level {level} not listed: {response!r}"

    @allure.title("CLI. Command log (streaming, CTRL+C).")
    @pytest.mark.regression  # streams until the level banner and CTRL+C notice appear
    def test_log_streaming(self, persistent_cli_connection):
        response = run_streaming(
            persistent_cli_connection,
            "log info",
            run_seconds=4.0,
            until=lambda t: "Current log level: info" in t and "Press CTRL+C to stop" in t,
        )
        assert "Current log level: info" in response, response
        assert "Press CTRL+C to stop" in response, response

    @allure.title("CLI. Command loader (usage).")
    def test_loader_usage(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("loader")
        assert "loader <cmd> <args>" in response, response
        assert "open" in response and "kill" in response, response

    @allure.title("CLI. Command loader kill.")
    def test_loader_kill(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("loader kill")
        assert (
            "App stopped successfully" in response or "No app running" in response
        ), response

    @allure.title("CLI. Command update (usage).")
    def test_update_usage(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("update")
        assert "update <917|917_ta" in response, response
        for sub in ("install", "install_tar", "install_web"):
            assert sub in response, f"missing subcommand {sub}: {response!r}"
