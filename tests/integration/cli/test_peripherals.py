"""Peripheral commands: audio, display, input.

The display is checked for real through `GET /api/screen`; sound is only checked
for a clean run (audibility is verified separately, outside this suite).

Coverage matrix and plan: scratchpad/cli_coverage_matrix.md.
"""

import time

import allure
import pytest

from clients.api.streaming import raw_to_png
from utils.cli_helpers import IMAGE_FILE, SOUND_FILE, run_streaming

pytestmark = pytest.mark.cli


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Commands Check")
class TestCLIPeripheralCommands:
    """Peripherals: audio/display/input."""

    @allure.id("2028")
    @allure.title("CLI. Command Audio.")
    @pytest.mark.story_commands_check
    def test_cli_command_audio(self, persistent_cli_connection):
        # a bare `audio` prints nothing at all; usage only comes with a bad subcommand
        response = persistent_cli_connection.execute_command("audio bogus")
        assert "Invalid command bogus" in response, response
        assert "audio start <path>" in response and "audio stop" in response, response

    @allure.id("2030")
    @allure.title("CLI. Command Display.")
    @pytest.mark.story_commands_check
    def test_cli_command_display(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("display")
        assert "Usage: display <action>" in response, response
        assert "show <front|back>" in response and "brightness" in response, response

    @allure.title("CLI. Command audio start (missing file).")
    def test_audio_start_missing_file(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("audio start /ext/nope.wav")
        assert "File /ext/nope.wav does not exist" in response, response

    @allure.title("CLI. Command audio start (real sound file).")
    def test_audio_start_real_file(self, persistent_cli_connection):
        # playback is fire-and-forget (the command returns at once); that the sound
        # is actually audible is checked separately, out of this suite
        response = persistent_cli_connection.execute_command(
            f"audio start {SOUND_FILE}", timeout=15, slow_command=True
        )
        assert "does not exist" not in response, response
        assert "Failed to play" not in response, response

    @allure.title("CLI. Command audio stop.")
    def test_audio_stop(self, persistent_cli_connection):
        # idempotent: stopping while nothing plays is accepted silently
        response = persistent_cli_connection.execute_command("audio stop")
        assert "Usage" not in response and "Invalid command" not in response, response

    @allure.title("CLI. Command display show (missing file).")
    def test_display_show_missing_file(self, persistent_cli_connection):
        # the missing-file path returns at once; the happy path streams until CTRL+C
        # and is covered by test_display_show_screenshot
        response = persistent_cli_connection.execute_command(
            "display show back /ext/nope.png"
        )
        assert "File not found" in response, response

    @allure.title("CLI. Command display brightness (set-and-restore).")
    def test_display_brightness(self, persistent_cli_connection):
        cli = persistent_cli_connection
        try:
            response = cli.execute_command("display brightness 50")
            assert "Error!" not in response and "Usage" not in response, response
        finally:
            cli.execute_command("display brightness auto")

    @allure.title("CLI. Command display brightness (invalid value).")
    def test_display_brightness_invalid(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("display brightness bogus")
        assert "Unable to parse 'bogus' as brightness value" in response, response

    @allure.title("CLI. Command input (usage).")
    def test_input_usage(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("input")
        assert "Usage: input" in response, response
        assert "dump" in response and "send" in response, response

    @allure.title("CLI. Command input send (invalid key/type).")
    def test_input_send_invalid(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("input send bogus bogus")
        assert "Usage: input" in response, response

    @allure.title("CLI. Command display show (screenshot of the back display).")
    @pytest.mark.regression  # streams the image, grabs the frame while it is shown
    def test_display_show_screenshot(self, persistent_cli_connection, streaming_api):
        # `display show` holds the image on screen only while it runs, so the frame
        # has to be grabbed mid-stream; after CTRL+C the display goes back to the app

        def screen_until(differs_from):
            # the image takes a moment to render, so poll rather than grab once
            frame = streaming_api.get_screen_bytes(display=1)
            for _ in range(20):
                if frame != differs_from:
                    return frame
                time.sleep(0.2)
                frame = streaming_api.get_screen_bytes(display=1)
            return frame

        before = streaming_api.get_screen_bytes(display=1)
        shown = []

        response = run_streaming(
            persistent_cli_connection,
            f"display show back {IMAGE_FILE}",
            run_seconds=5.0,
            during=lambda: shown.append(screen_until(before)),
        )
        assert "Error!" not in response, response

        allure.attach(
            raw_to_png(shown[0], 1), "Back display while shown", allure.attachment_type.PNG
        )
        assert shown[0] != before, "the shown image did not change the back display"

        after = screen_until(shown[0])
        assert after != shown[0], "the image stayed on screen after CTRL+C"

    @allure.title("CLI. Command input dump (streaming, CTRL+C).")
    @pytest.mark.regression  # streams until the CTRL+C banner appears
    def test_input_dump_streaming(self, persistent_cli_connection):
        response = run_streaming(
            persistent_cli_connection,
            "input dump",
            run_seconds=4.0,
            until=lambda t: "Press CTRL+C to stop" in t,
        )
        assert "Press CTRL+C to stop" in response, response

    @allure.title("CLI. Command input send is observed by input dump.")
    @pytest.mark.regression  # dumps on one connection while another injects an event
    def test_input_send_seen_by_dump(self, persistent_cli_connection, fresh_cli_connection):
        # dump on one connection, inject on another, then read until the event lands
        response = run_streaming(
            persistent_cli_connection,
            "input dump",
            run_seconds=4.0,
            during=lambda: fresh_cli_connection.execute_command(
                "input send InputKeyUp InputTypeRelease"
            ),
            until=lambda t: "key: InputKeyUp type: InputTypeRelease" in t,
        )
        assert "key: InputKeyUp type: InputTypeRelease" in response, (
            f"the injected event never reached the input pubsub: {response!r}"
        )
