import pytest
import allure
import asyncio


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Commands Check - Automated")
class TestCLICommands:
    """Test cases for CLI commands - Story: Commands Check"""

    @allure.testcase("2047", "CLI. Command ?. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_command_question_mark(self, cli, test_logger):
        """Test CLI. Command ?. [Draft]"""
        with allure.step("Execute ? command"):
            response = await cli.execute_command("?")
            
        with allure.step("Verify ? command provides help"):
            test_logger.debug(f"Response from ? command: {response}")
            assert "Available commands:" in response, "? command should return available commands list"
            assert "?" in response, "? command should list itself as available"
            assert "power" in response.lower(), "Help should mention power command"
            assert "audio" in response.lower(), "Help should mention audio command"
            assert "echo" in response.lower(), "Help should mention echo command"
            assert "device_info" in response.lower(), "Help should mention device_info command"
            assert "top" in response.lower(), "Help should mention top command"
            assert "uptime" in response.lower(), "Help should mention uptime command"
            assert "storage" in response.lower(), "Help should mention storage command"
            assert "loader" in response.lower(), "Help should mention loader command"
            assert "update" in response.lower(), "Help should mention update command"
            assert "log" in response.lower(), "Help should mention log command"
            assert "free_blocks" in response.lower(), "Help should mention free_blocks command"
            assert "light_sensor" in response.lower(), "Help should mention light_sensor command"
            assert "crypto_backup" in response.lower(), "Help should mention crypto_backup command"
            assert "free" in response.lower(), "Help should mention free command"
            assert "help" in response.lower(), "Help should mention help command"
            assert "exit" in response.lower(), "Help should mention exit command"
            assert "input" in response.lower(), "Help should mention input command"
            assert "display" in response.lower(), "Help should mention display command"
            assert "status_lights" in response.lower(), "Help should mention status_lights command"
            assert "sysctl" in response.lower(), "Help should mention sysctl command"
            assert "sl_cli" in response.lower(), "Help should mention sl_cli command"
            # Add specific assertions based on expected output

    @allure.testcase("2046", "CLI. Command Exit. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_command_exit(self, cli):
        """Test CLI. Command Exit. [Draft]"""
        with allure.step("Check exit command availability"):
            exists = await cli.check_command_exists("exit")
            assert exists, "Exit command should be available"
            # TODO: Implement actual exit command test if feasible via flag in cli fixture

    @allure.testcase("2043", "CLI. Command Free. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_command_free(self, cli):
        """Test CLI. Command Free. [Draft]"""
        with allure.step("Execute free command"):
            response = await cli.execute_command("free")
            
        with (allure.step("Verify free command output")):
            assert "Free heap size:" in response, "Response should contain 'Free heap size:'"
            assert "Total heap size:" in response, "Response should contain 'Total heap size:'"
            assert "Minimum heap size:" in response, "Response should contain 'Minimum heap size:'"
            assert "Maximum heap block:" in response, "Response should contain 'Maximum heap block:'"
            assert "Pool free:" in response, "Response should contain 'Pool free:'"
            assert "Maximum pool block:" in response, "Response should contain 'Maximum pool block:'"
            assert "Free heap size:" in response and int(response.split("Free heap size:")[1].split()[0]) > 25000, \
                "Response should contain 'Free heap size' greater than 25000"

    @allure.testcase("2045", "CLI. Command Help. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_command_help(self, cli):
        """Test CLI. Command Help. [Draft]"""
        with allure.step("Execute help command"):
            response = await cli.execute_command("power help")
            
        with allure.step("Verify help command output"):
            assert "Usage:" in response, "Contains guidance on command usage"
            assert "power <cmd> <args>" in response, "Explains command arguments"
            assert "Cmd list:" in response, "Lists all sub-commands"
            # TODO: remove? its same as ? maybe parametrize that test instead

    @allure.testcase("2044", "CLI. Command Storage. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_command_storage(self, cli):
        """Test CLI. Command Storage. [Draft]"""
        with allure.step("Execute storage command"):
            response = await cli.execute_command("storage")
            
        with allure.step("Verify storage command output"):
            assert response, "Storage command should return storage information"
            # TODO: add entire class for storage tests

    @allure.testcase("2040", "CLI. Command Sl_cli. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_command_sl_cli(self, cli):
        """Test CLI. Command Sl_cli. [Draft]"""
        with allure.step("Execute sl_cli command"):
            response = await cli.execute_command("sl_cli")
            
        with allure.step("Verify sl_cli command"):
            assert "Welcome to BUSY Bar 917 Command Line Interface!" in response is not None, "917 is reached"

        # with allure.step("Exit sl_cli mode"):
        #     response = await cli.execute_command("exit")
        #     assert response, "Exited sl_cli mode"

    @allure.testcase("2041", "CLI. Command Uptime. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_command_uptime(self, cli):
        """Test CLI. Command Uptime. [Draft]"""
        with allure.step("Execute uptime command"):
            response = await cli.execute_command("uptime")
            
        with allure.step("Verify uptime command output"):
            assert response and any(int(part[:-1]) > 0 for part in response.split() if part[-1] in "dhms"), \
                "Uptime command should return system uptime with at least one non-zero value"

    @allure.testcase("2035", "CLI. Command Device_info. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_command_device_info(self, cli):
        """Test CLI. Command Device_info. [Draft]"""
        with allure.step("Execute device_info command"):
            response = await cli.execute_command("device_info")
            
        with allure.step("Verify device_info command output"):
            assert response, "Device_info command should return device information"
            assert "u5_firmware_origin_fork       : Official" in response, "Device_info should include the correct origin fork"
            assert "u5_firmware_origin_git        : https://github.com/flipperdevices/bsb-firmware" in response, "Device_info should include the correct origin git"
            # TODO: Add more checks for other fields in the future

    @allure.testcase("2028", "CLI. Command Audio. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_command_audio(self, cli):
        """Test CLI. Command Audio. [Draft]"""
        with allure.step("Execute audio command"):
            response = await cli.execute_command("audio")
            
        with allure.step("Verify audio command"):
            assert response is not None, "Audio command should execute"
            # TODO: does nothing 8/26

    @allure.testcase("2030", "CLI. Command Display. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_command_display(self, cli):
        """Test CLI. Command Display. [Draft]"""
        with allure.step("Execute display command"):
            response = await cli.execute_command("display")
            
        with allure.step("Verify display command"):
            assert response is not None, "Display command should execute"

    @allure.testcase("2031", "CLI. Command Echo. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_command_echo(self, cli):
        """Test CLI. Command Echo. [Draft]"""
        test_message = "Hello BSB Test"
        with allure.step(f"Execute echo command with message: {test_message}"):
            response = await cli.execute_command(f'echo "{test_message}"')
            
        with allure.step("Verify echo command output"):
            assert test_message in response, f"Echo should return the input message: {test_message}"

    @allure.testcase("2034", "CLI. Command Free_blocks. [Draft]")
    @pytest.mark.story_commands_check
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_command_free_blocks(self, cli):
        """Test CLI. Command Free_blocks. [Draft]"""
        with allure.step("Execute free_blocks command"):
            response = await cli.execute_command("free_blocks")
            
        with allure.step("Verify free_blocks command output"):
            assert response is not None, "Free_blocks command should execute"

    # Add more command tests following the same pattern...


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("UI validation")
class TestCLIUI:
    """Test cases for CLI UI - Story: UI validation"""

    @allure.testcase("2048", "CLI. UI. Render [Draft]")
    @pytest.mark.story_ui_validation
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_ui_render(self, cli):
        """Test CLI. UI. Render [Draft]"""
        with allure.step("Check CLI UI rendering"):
            # Test basic rendering by checking if we get proper responses
            response = await cli.execute_command("help")
            assert response, "CLI should render help properly"

    @allure.testcase("2049", "CLI. UI. Version [Draft]")
    @pytest.mark.story_ui_validation
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_ui_version(self, cli):
        """Test CLI. UI. Version [Draft]"""
        with allure.step("Execute version command"):
            response = await cli.execute_command("version")
            
        with allure.step("Verify version information"):
            assert response, "Version command should return version information"

    @allure.testcase("2050", "CLI. UI. Build Info. [Draft]")
    @pytest.mark.story_ui_validation
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_ui_build_info(self, cli):
        """Test CLI. UI. Build Info. [Draft]"""
        with allure.step("Execute build info command"):
            # This might be a different command, adjust as needed
            response = await cli.execute_command("build_info")
            
        with allure.step("Verify build info output"):
            assert response is not None, "Build info command should execute"

    @allure.testcase("2152", "CLI. UI. Welcome message. [Draft]")
    @pytest.mark.story_ui_validation
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_ui_welcome_message(self, cli):
        """Test CLI. UI. Welcome message. [Draft]"""
        with allure.step("Verify welcome message on connection"):
            # The welcome message should be captured during connection
            # This test verifies the initial connection worked and we got a welcome
            response = await cli.execute_command("")  # Send empty command to see prompt
            assert cli.connected, "CLI should be connected and show welcome message"

    @allure.testcase("2127", "CLI. Commands. History. [Draft]")
    @pytest.mark.story_ui_validation
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_commands_history(self, cli):
        """Test CLI. Commands. History. [Draft]"""
        with allure.step("Execute a command"):
            await cli.execute_command("help")
            
        with allure.step("Check command history"):
            response = await cli.execute_command("history")
            assert response is not None, "History command should work"

    @allure.testcase("2129", "CLI. Commands. Aliases. [Draft]")
    @pytest.mark.story_ui_validation
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_commands_aliases(self, cli):
        """Test CLI. Commands. Aliases. [Draft]"""
        with allure.step("Check for command aliases"):
            response = await cli.execute_command("alias")
            assert response is not None, "Alias command should execute"

    @allure.testcase("2128", "CLI. Commands. Tab Completion. [Draft]")
    @pytest.mark.story_ui_validation
    @pytest.mark.cli
    @pytest.mark.asyncio
    async def test_cli_commands_tab_completion(self, cli):
        """Test CLI. Commands. Tab Completion. [Draft]"""
        with allure.step("Test tab completion functionality"):
            # Tab completion testing through telnet is complex
            # For now, just verify basic command recognition
            assert await cli.check_command_exists("help"), "Basic commands should be available for completion"