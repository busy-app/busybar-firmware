"""Sub-CLIs: the `matter` submenu and the 917 CLI entered via `sl_cli`.

Coverage matrix and plan: scratchpad/cli_coverage_matrix.md.
"""

import re

import allure
import pytest

from clients.cli import SimpleCLIConnection

pytestmark = pytest.mark.cli


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Commands Check")
class TestCLISlCli:
    """Entering and leaving the 917 sub-CLI (`sl_cli`)."""

    @allure.id("2040")
    @allure.title("CLI. Command Sl_cli.")
    @pytest.mark.story_commands_check
    def test_cli_command_sl_cli(self, fresh_cli_connection):
        # a fresh connection: sl_cli is exclusive and must not be entered on the
        # module-scoped one that the read-only command tests keep at the main prompt
        cli = fresh_cli_connection
        response = cli.enter_sl_cli()
        assert (
            "Welcome to BUSY Bar 917 Command Line Interface!" in response
        ), "Should enter 917 CLI with welcome message"
        assert cli._in_sl_cli, "Should be in 917 CLI mode"
        try:
            assert cli.execute_917_command("?").strip(), "917 CLI should respond to help"
        finally:
            cli.exit_sl_cli()
            assert not cli._in_sl_cli, "Should have exited 917 CLI mode"

    @allure.title("CLI. Command sl_cli is exclusive (only one instance allowed).")
    @pytest.mark.story_commands_check
    def test_cli_command_sl_cli_exclusive(self, fresh_cli_connection):
        """sl_cli is marked CliCommandFlagExclusive — a second concurrent instance
        from another shell must be rejected with the run-once notice."""
        first = fresh_cli_connection
        first.enter_sl_cli()
        assert first._in_sl_cli, "First connection should be in 917 CLI mode"
        try:
            second = SimpleCLIConnection()
            assert second.connect(), "second CLI connection failed"
            try:
                response = second.execute_command("sl_cli", slow_command=True)
                assert (
                    "can only be run once" in response
                ), f"Second sl_cli should be refused, got: {response!r}"
                assert (
                    "Welcome to BUSY Bar 917" not in response
                ), "Second sl_cli must not enter 917 CLI"
                assert not second._in_sl_cli, "Second connection must not be in 917 CLI"
            finally:
                second.disconnect()
        finally:
            first.exit_sl_cli()


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Commands Check")
class TestCLIMatterReadOnly:
    """Read-only matter submenu commands (enter/leave via execute_command)."""

    @staticmethod
    def _matter(cli, command):
        """Enter matter, run the command, leave. Returns the command response."""
        cli.execute_command("matter")
        try:
            return cli.execute_command(command)
        finally:
            cli.execute_command("exit")  # leave matter>: back to the main CLI

    @allure.title("CLI. matter. Command fabrics.")
    def test_matter_fabrics(self, fresh_cli_connection):
        response = self._matter(fresh_cli_connection, "fabrics")
        assert "fabric" in response.lower(), response

    @allure.title("CLI. matter. Command comm.")
    def test_matter_comm(self, fresh_cli_connection):
        response = self._matter(fresh_cli_connection, "comm")
        assert "pairing code" in response.lower(), response
        assert "QR code payload" in response, response

    @allure.title("CLI. matter. Command cd.")
    def test_matter_cd(self, fresh_cli_connection):
        response = self._matter(fresh_cli_connection, "cd")
        assert "available CDs" in response, response
        for cd in ("production", "development", "certification"):
            assert cd in response, f"missing CD {cd}: {response!r}"


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Commands Check")
@pytest.mark.uses_si917
class TestCLI917ReadOnlyCommands:
    """Read-only 917 CLI commands (entered via the sl_cli fixture)."""

    @allure.title("CLI. 917. Command device_info.")
    def test_917_device_info(self, sl_cli):
        response = sl_cli.execute_917_command("device_info")
        assert "sl_firmware_version" in response, response
        assert "sl_wifi_mac" in response and "sl_ble_mac" in response, response

    @allure.title("CLI. 917. Command free.")
    def test_917_free(self, sl_cli):
        response = sl_cli.execute_917_command("free")
        assert "Free heap size:" in response, response
        assert "Total heap size:" in response, response

    @allure.title("CLI. 917. Command netstat.")
    def test_917_netstat(self, sl_cli):
        response = sl_cli.execute_917_command("netstat")
        assert "Proto" in response and "Local Address" in response, response

    @allure.title("CLI. 917. Command uptime.")
    def test_917_uptime(self, sl_cli):
        response = sl_cli.execute_917_command("uptime")
        assert "Uptime:" in response, response

    @allure.title("CLI. 917. Command crypto (usage).")
    def test_917_crypto_usage(self, sl_cli):
        response = sl_cli.execute_917_command("crypto")
        assert "crypto" in response and "Cmd list:" in response, response
        assert "list" in response and "dump" in response, response

    @allure.title("CLI. 917. Command crypto list (main partition).")
    def test_917_crypto_list_main(self, sl_cli):
        response = sl_cli.execute_917_command("crypto list 0")
        assert "RET: 0" in response, response
        # main partition is populated -> at least one key row
        assert "key:" in response, f"expected key entries on partition 0: {response!r}"

    @allure.title("CLI. 917. Command crypto list (user partition).")
    def test_917_crypto_list_user(self, sl_cli):
        # user partition may be empty, but the command must succeed
        response = sl_cli.execute_917_command("crypto list 1")
        assert "RET: 0" in response, response

    @allure.title("CLI. 917. Command crypto dump.")
    @pytest.mark.regression  # ~1.9s: real 917-chip key dump
    def test_917_crypto_dump(self, sl_cli):
        response = sl_cli.execute_917_command("crypto dump")
        assert response.strip(), "crypto dump should return output"
        assert "illegal option" not in response and "usage:" not in response.lower(), response

    @allure.title("CLI. 917. Command free_blocks.")
    def test_917_free_blocks(self, sl_cli):
        response = sl_cli.execute_917_command("free_blocks")
        # heap block dump: lines like 'A 00041FC0 S 55864'
        assert re.search(r"A\s+[0-9A-Fa-f]+\s+S\s+\d+", response), response

    @allure.title("CLI. 917. Command echo.")
    def test_917_echo(self, sl_cli):
        response = sl_cli.execute_917_command("echo hi917")
        assert "hi917" in response, response

    @allure.title("CLI. 917. Command log (levels listing).")
    def test_917_log_levels(self, sl_cli):
        response = sl_cli.execute_917_command("log ?")
        for level in ("error", "warn", "info", "debug", "trace"):
            assert f"log {level}" in response, f"level {level} not listed: {response!r}"
