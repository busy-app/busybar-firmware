"""CLI shell itself: the `?` command list, help/usage, UI rendering, and 917
connection robustness. Not tied to any one device command.

These carry TestOps ids and run outside the `regression` sweep.
"""

import allure
import pytest

pytestmark = pytest.mark.cli


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Commands Check")
class TestCLIShell:
    """The `?` command list, exit, and the help/usage mechanism."""

    @allure.id("2047")
    @allure.title("CLI. Command ?.")
    @pytest.mark.story_commands_check
    def test_cli_command_question_mark(self, persistent_cli_connection, test_logger):
        with allure.step("Execute ? command"):
            response = persistent_cli_connection.execute_command("?", timeout=20)

        with allure.step("Verify ? command provides help"):
            test_logger.debug(f"Response from ? command: {response}")
            assert (
                "Available commands:" in response
            ), "? command should return available commands list"

            expected_commands = [
                "loader",
                "power",
                "input",
                "audio",
                "update",
                "display",
                "log",
                "echo",
                "status_lights",
                "free_blocks",
                "device_info",
                "sysctl",
                "light_sensor",
                "top",
                "sl_cli",
                "date",
                "uptime",
                "crypto_backup",
                "free",
                "storage",
                "help",
                "exit",
            ]
            response_lower = response.lower()
            missing = [cmd for cmd in expected_commands if cmd not in response_lower]
            assert not missing, f"Missing expected commands: {missing}"

    @allure.id("2046")
    @allure.title("CLI. Command Exit.")
    @pytest.mark.story_commands_check
    def test_cli_command_exit(self, persistent_cli_connection):
        help_response = persistent_cli_connection.execute_command("?", timeout=20)
        assert "exit" in help_response.lower(), "Exit command should be available in help"

    @allure.id("2045")
    @allure.title("CLI. Command Help.")
    @pytest.mark.story_commands_check
    def test_cli_command_help(self, persistent_cli_connection):
        # the usage/help mechanism, exercised through `power`
        response = persistent_cli_connection.execute_command("power")
        assert "Usage:" in response, "Should contain guidance on command usage"
        assert "power <cmd> <args>" in response, "Should explain command arguments"
        assert "Cmd list:" in response, "Should list all sub-commands"


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("UI Validation")
class TestCLIUI:
    """CLI UI rendering."""

    @allure.id("2048")
    @allure.title("CLI. UI. Render")
    @pytest.mark.story_ui_validation
    def test_cli_ui_render(self, fresh_cli_connection):
        response = fresh_cli_connection.execute_command("?")
        assert response.strip(), "CLI should render help properly"
        assert len(response) > 100, "Help output should be substantial"

    @allure.id("2152")
    @allure.title("CLI. UI. Welcome message.")
    @pytest.mark.story_ui_validation
    def test_cli_ui_welcome_message(self, fresh_cli_connection):
        assert fresh_cli_connection.connected, "CLI should be connected and show welcome"
        response = fresh_cli_connection.execute_command("?")
        assert "Available commands:" in response, "CLI should respond properly to commands"


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Connection Management")
class TestCLIConnectionManagement:
    """917 CLI entry/exit robustness."""

    @allure.id("2727")
    @allure.title("CLI. 917 CLI. Multiple Entries.")
    @pytest.mark.story_commands_check
    def test_cli_917_multiple_entries(self, fresh_cli_connection):
        cli = fresh_cli_connection
        for attempt in range(1, 4):
            with allure.step(f"Enter/use/exit 917 CLI — attempt {attempt}"):
                response = cli.enter_sl_cli()
                assert (
                    "Welcome to BUSY Bar 917" in response
                ), f"Should enter 917 CLI on attempt {attempt}"
                try:
                    assert cli.execute_917_command(
                        "?"
                    ), f"917 CLI should respond on attempt {attempt}"
                finally:
                    cli.exit_sl_cli()
                assert not cli._in_sl_cli, f"Should exit 917 CLI on attempt {attempt}"
