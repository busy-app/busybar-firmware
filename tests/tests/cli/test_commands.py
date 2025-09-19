import allure
import pytest


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Commands Check")
class TestCLICommandsSession:
    """Test cases for CLI commands using session-scoped connection"""

    @allure.id("2712")
    @allure.title("CLI. Command ?. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    def test_cli_command_question_mark(self, persistent_cli_connection, test_logger):
        """Test CLI. Command ?. [Draft]"""
        with allure.step("Execute ? command"):
            response = persistent_cli_connection.execute_command("?", timeout=20)

        with allure.step("Verify ? command provides help"):
            test_logger.debug(f"Response from ? command: {response}")
            assert (
                "Available commands:" in response
            ), "? command should return available commands list"
            assert "?" in response, "? command should list itself as available"

            # Check for key expected commands from your PuTTY output
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
            missing_commands = []
            for cmd in expected_commands:
                if cmd not in response_lower:
                    missing_commands.append(cmd)

            assert (
                len(missing_commands) == 0
            ), f"Missing expected commands: {missing_commands}"

    @allure.id("2713")
    @allure.title("CLI. Command Exit. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    def test_cli_command_exit(self, persistent_cli_connection):
        """Test CLI. Command Exit. [Draft]"""
        with allure.step("Check exit command availability"):
            help_response = persistent_cli_connection.execute_command("?", timeout=20)
            assert (
                "exit" in help_response.lower()
            ), "Exit command should be available in help"

    @allure.id("2714")
    @allure.title("CLI. Command Free. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    def test_cli_command_free(self, persistent_cli_connection):
        """Test CLI. Command Free. [Draft]"""
        with allure.step("Execute free command"):
            response = persistent_cli_connection.execute_command("free")

        with allure.step("Verify free command output"):
            assert (
                "Free heap size:" in response
            ), "Response should contain 'Free heap size:'"
            assert (
                "Total heap size:" in response
            ), "Response should contain 'Total heap size:'"
            assert (
                "Minimum heap size:" in response
            ), "Response should contain 'Minimum heap size:'"
            assert (
                "Maximum heap block:" in response
            ), "Response should contain 'Maximum heap block:'"
            assert "Pool free:" in response, "Response should contain 'Pool free:'"
            assert (
                "Maximum pool block:" in response
            ), "Response should contain 'Maximum pool block:'"

            # Extract and validate free heap size
            try:
                free_heap_line = [
                    line for line in response.split("\n") if "Free heap size:" in line
                ][0]
                free_heap_value = int(
                    free_heap_line.split("Free heap size:")[1].split()[0]
                )
                assert (
                    free_heap_value > 25000
                ), f"Free heap size should be > 25000, got {free_heap_value}"
            except (IndexError, ValueError) as e:
                pytest.fail(f"Could not parse free heap size: {e}")

    @allure.id("2715")
    @allure.title("CLI. Command Help. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    def test_cli_command_help(self, persistent_cli_connection):
        """Test CLI. Command Help. [Draft]"""
        with allure.step("Execute help command via power command"):
            response = persistent_cli_connection.execute_command("power")

        with allure.step("Verify help command output"):
            assert "Usage:" in response, "Should contain guidance on command usage"
            assert "power <cmd> <args>" in response, "Should explain command arguments"
            assert "Cmd list:" in response, "Should list all sub-commands"

    @allure.id("2716")
    @allure.title("CLI. Command Storage. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    def test_cli_command_storage(self, persistent_cli_connection):
        """Test CLI. Command Storage. [Draft]"""
        with allure.step("Execute storage command"):
            response = persistent_cli_connection.execute_command("storage")

        with allure.step("Verify storage command output"):
            assert response.strip(), "Storage command should return storage information"
            assert len(response.strip()) > 10, "Storage output should be substantial"

    @allure.id("2717")
    @allure.title("CLI. Command Sl_cli. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    def test_cli_command_sl_cli(self, persistent_cli_connection):
        """Test CLI. Command Sl_cli. [Draft] - Enter and exit 917 CLI"""
        with allure.step("Execute sl_cli command to enter 917 CLI"):
            response = persistent_cli_connection.enter_sl_cli()

        with allure.step("Verify 917 CLI entry"):
            assert (
                "Welcome to BUSY Bar 917 Command Line Interface!" in response
            ), "Should enter 917 CLI with welcome message"
            assert persistent_cli_connection._in_sl_cli, "Should be in 917 CLI mode"

        try:
            with allure.step("Test 917 CLI help command"):
                help_response = persistent_cli_connection.execute_917_command("?")
                assert help_response.strip(), "917 CLI should respond to help command"

        finally:
            with allure.step("Exit sl_cli mode"):
                exit_response = persistent_cli_connection.exit_sl_cli()
                assert (
                    not persistent_cli_connection._in_sl_cli
                ), "Should have exited 917 CLI mode"

    @allure.id("2718")
    @allure.title("CLI. Command Uptime. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    def test_cli_command_uptime(self, persistent_cli_connection):
        """Test CLI. Command Uptime. [Draft]"""
        with allure.step("Execute uptime command"):
            response = persistent_cli_connection.execute_command("uptime")

        with allure.step("Verify uptime command output"):
            assert response.strip(), "Uptime command should return system uptime"
            # Look for time units (days, hours, minutes, seconds)
            has_time_units = any(
                unit in response.lower()
                for unit in ["d", "h", "m", "s", "day", "hour", "min", "sec"]
            )
            assert has_time_units, f"Uptime should contain time units, got: {response}"

    @allure.id("2719")
    @allure.title("CLI. Command Device_info. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    def test_cli_command_device_info(self, persistent_cli_connection):
        """Test CLI. Command Device_info. [Draft] - SLOW COMMAND (uses 917 chip)"""
        with allure.step("Execute device_info command"):
            # device_info has two parts: u5_* fields (immediate) and sl_* fields (after 2s delay)
            # Use longer timeout to ensure we get the complete response
            response = persistent_cli_connection.execute_command(
                "device_info", timeout=20.0, slow_command=True
            )

        with allure.step("Verify device_info command output"):
            assert (
                response.strip()
            ), "Device_info command should return device information"
            assert (
                "u5_firmware_origin_fork       : Official" in response
            ), "Should include the correct origin fork"
            # Check that git origin is present (URL may vary based on deployment)
            assert (
                "u5_firmware_origin_git" in response
            ), "Should include firmware origin git information"
            # Optionally verify it contains a git URL format
            git_lines = [line for line in response.split("\n") if "u5_firmware_origin_git" in line]
            if git_lines:
                git_url = git_lines[0].split(":", 1)[1].strip()
                assert git_url, "Git URL should not be empty"

    @allure.id("2720")
    @allure.title("CLI. Command Audio. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    def test_cli_command_audio(self, persistent_cli_connection):
        """Test CLI. Command Audio. [Draft]"""
        with allure.step("Execute audio command"):
            response = persistent_cli_connection.execute_command("audio")

        with allure.step("Verify audio command executes"):
            assert response is not None, "Audio command should execute without error"

    @allure.id("2721")
    @allure.title("CLI. Command Display. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    def test_cli_command_display(self, persistent_cli_connection):
        """Test CLI. Command Display. [Draft]"""
        with allure.step("Execute display command"):
            response = persistent_cli_connection.execute_command("display")

        with allure.step("Verify display command executes"):
            assert response is not None, "Display command should execute without error"

    @allure.id("2722")
    @allure.title("CLI. Command Echo. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    def test_cli_command_echo(self, persistent_cli_connection):
        """Test CLI. Command Echo. [Draft]"""
        test_message = "Hello BSB Test"
        with allure.step(f"Execute echo command with message: {test_message}"):
            response = persistent_cli_connection.execute_command(
                f'echo "{test_message}"'
            )

        with allure.step("Verify echo command output"):
            assert (
                test_message in response
            ), f"Echo should return the input message: {test_message}"

    @allure.id("2723")
    @allure.title("CLI. Command Free_blocks. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    def test_cli_command_free_blocks(self, persistent_cli_connection):
        """Test CLI. Command Free_blocks. [Draft]"""
        with allure.step("Execute free_blocks command"):
            response = persistent_cli_connection.execute_command("free_blocks")

        with allure.step("Verify free_blocks command output"):
            assert response is not None, "Free_blocks command should execute"

    @allure.id("2724")
    @allure.title("CLI. Command Power. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    def test_cli_command_power(self, persistent_cli_connection):
        """Test CLI. Command Power. [Draft]"""
        with allure.step("Check if power command is available"):
            # First check if power command exists by looking at available commands
            help_response = persistent_cli_connection.execute_command("?", timeout=20)
            if "power" not in help_response.lower():
                pytest.skip("Power command not available in this firmware version")

        with allure.step("Execute power command to get help"):
            response = persistent_cli_connection.execute_command("power")

        with allure.step("Verify power command help"):
            assert "Usage:" in response, "Power command should provide help information"
            assert "power <cmd> <args>" in response, "Should show usage format"
            assert "Cmd list:" in response, "Should show available subcommands"


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("UI Validation")
class TestCLIUI:
    """Test cases for CLI UI - Story: UI validation"""

    @allure.id("2725")
    @allure.title("CLI. UI. Render [Draft]")
    @pytest.mark.story_ui_validation
    @pytest.mark.cli
    def test_cli_ui_render(self, fresh_cli_connection):
        """Test CLI. UI. Render [Draft]"""
        with allure.step("Check CLI UI rendering"):
            response = fresh_cli_connection.execute_command("?")
            assert response.strip(), "CLI should render help properly"
            assert len(response) > 100, "Help output should be substantial"

    @allure.id("2726")
    @allure.title("CLI. UI. Welcome message. [Draft]")
    @pytest.mark.story_ui_validation
    @pytest.mark.cli
    def test_cli_ui_welcome_message(self, fresh_cli_connection):
        """Test CLI. UI. Welcome message. [Draft]"""
        with allure.step("Verify CLI connection shows welcome"):
            assert (
                fresh_cli_connection.connected
            ), "CLI should be connected and show welcome message"

        with allure.step("Test basic command to verify CLI responsiveness"):
            response = fresh_cli_connection.execute_command("?")
            assert (
                "Available commands:" in response
            ), "CLI should respond properly to commands"


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Connection Management")
class TestCLIConnectionManagement:
    """Test connection management and 917 CLI"""

    @allure.id("2727")
    @allure.title("CLI. 917 CLI. Multiple Entries. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    def test_cli_917_multiple_entries(self, fresh_cli_connection):
        """Test entering and exiting 917 CLI multiple times"""
        for i in range(3):
            with allure.step(f"Enter 917 CLI - attempt {i + 1}"):
                response = fresh_cli_connection.enter_sl_cli()
                assert (
                    "Welcome to BUSY Bar 917" in response
                ), f"Should enter 917 CLI on attempt {i + 1}"

            with allure.step(f"Test 917 CLI functionality - attempt {i + 1}"):
                help_response = fresh_cli_connection.execute_917_command("?")
                assert (
                    help_response is not None
                ), f"917 CLI should respond to commands on attempt {i + 1}"

            with allure.step(f"Exit 917 CLI - attempt {i + 1}"):
                fresh_cli_connection.exit_sl_cli()
                assert (
                    not fresh_cli_connection._in_sl_cli
                ), f"Should exit 917 CLI on attempt {i + 1}"
