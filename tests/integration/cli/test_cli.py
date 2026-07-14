"""CLI tests for BUSY Bar.

The base set (`TestCLICommandsSession`, `TestCLIUI`, `TestCLIConnectionManagement`)
runs everywhere; the full command sweep is marked `regression` per class and stays
out of PR/dev runs.

Everything here is Level 1: read-only diagnostics and reversible operations —
deterministic output, safe. Coverage matrix and plan: scratchpad/cli_coverage_matrix.md.
"""

import hashlib
import http.server
import re
import socketserver
import threading
import time

import allure
import pytest

from clients.api.streaming import raw_to_png
from clients.cli import SimpleCLIConnection

pytestmark = [pytest.mark.cli]

ANSI_RE = re.compile(r"\x1b\[[0-9;?]*[A-Za-z]")

# Standard assets, always present on /ext
SOUND_FILE = "/ext/apps_assets/busy/sounds/countdown_tick.snd"
IMAGE_FILE = "/ext/apps_assets/shared/images/dt_apple_red.image"

# The only commands debug mode adds to `?` — `cli_command_update_debug_mode()` in
# cli_u5/cli_commands.c registers exactly these two. `factory_reset` is listed either
# way (it is destructive, not debug-gated), and `sysctl storage_bkp_unlock` is gated
# inside `sysctl`, not in the top-level list.
DEBUG_ONLY_COMMANDS = {"gpio", "otp"}

TEXT_PAYLOAD = b"hello-busybar"
# Printable bytes only: the CLI runs over telnetlib, which swallows NUL/0x11 and
# escapes 0xFF, so a full 0..255 blob would not survive the round-trip intact.
BLOB_PAYLOAD = (bytes(range(0x20, 0x7F)) * 6)[:512]


@pytest.fixture(scope="module", autouse=True)
def cli_debug(persistent_cli_connection):
    """Debug mode on for the whole CLI module, and left on afterwards.

    The flag lives in NVM and survives reboots, so a suite that turned it off would
    leave the bench without the debug-gated commands (`gpio`, `otp`, `factory_reset`).
    Re-enable on teardown as well: `test_sysctl_debug_toggle` flips it off on purpose.
    """
    persistent_cli_connection.execute_command("sysctl debug 1")
    yield
    persistent_cli_connection.execute_command("sysctl debug 1")


@pytest.fixture(scope="class")
def sl_cli():
    """CLI in 917 (sl_cli) mode, shared by the whole 917 class.

    Its own connection, not `persistent_cli_connection`: `sl_cli` is exclusive and
    a test failing inside 917 mode must not leave the shared CLI at the `917>:`
    prompt. Entering 917 mode is slow, so do it once per class, not per test — the
    commands in there are read-only and cannot interfere with each other.
    """
    cli = SimpleCLIConnection()
    if not cli.connect():
        pytest.skip("Could not connect to CLI")
    try:
        cli.enter_sl_cli()
        yield cli
    finally:
        if cli._in_sl_cli:
            cli.exit_sl_cli()
        cli.disconnect()


def command_set(cli):
    """The command names `?` lists, as a set."""
    response = cli.execute_command("?", timeout=20)
    listing = response.split("Available commands:")[1].split("Find out more")[0]
    return set(listing.split())


def resync(cli):
    """Put the shared CLI back at a clean prompt, reconnecting if needed.

    A raw/streaming command that died mid-protocol (`read_chunks` waiting for the
    per-chunk ack, `storage write` still consuming raw keys) leaves the device
    eating whatever is sent next. On a module-scoped connection that would poison
    every later test, so prove the prompt answers and reconnect if it does not.
    """
    if "__sync__" not in cli.execute_command("echo __sync__"):
        cli.disconnect()
        cli.connect()
        # the device tears the old shell down asynchronously and only then closes the
        # file an aborted `storage write` held open — deleting it right away fails
        # with 'file is already open'
        time.sleep(1.0)


@pytest.fixture
def storage_dir(persistent_cli_connection):
    """Empty `/ext/cli_test`, wiped again afterwards — whatever the test did or
    left half-done. Cleanup runs even when the test fails, and starts with a
    resync so it still works if a raw-protocol test died mid-command."""
    cli = persistent_cli_connection
    path = "/ext/cli_test"

    def rm_rf(target):
        # `storage remove` only unlinks files and *empty* dirs, so walk the tree
        # depth-first (`extract` leaves a whole subtree behind in out/)
        listing = cli.execute_command(f"storage list {target}")
        for kind, name in re.findall(r"\[([DF])\]\s+(\S+)", listing):
            child = f"{target}/{name}"
            rm_rf(child) if kind == "D" else cli.execute_command(f"storage remove {child}")
        cli.execute_command(f"storage remove {target}")

    rm_rf(path)  # a previous run may have died before its own cleanup
    cli.execute_command(f"storage mkdir {path}")
    try:
        yield path
    finally:
        resync(cli)
        rm_rf(path)


def run_streaming(cli, command, run_seconds=1.5, timeout=10.0, during=None):
    """Run a command that streams until CTRL+C (`log`, `top`, `input dump`,
    `display show`) and return its output.

    Those commands loop on `cli_is_pipe_broken_or_is_etx_next_char()`, so they
    never reach the prompt on their own and `execute_command()` would just time
    out mid-stream, leaving the connection out of sync. Drive the socket
    directly: run for a while, send ETX (0x03), then drain back to the prompt.

    `during` is called while the command is still streaming — that is the only
    moment its effect is observable (a `display show` image, an injected input
    event). It runs under try/finally: if it raises, the ETX still goes out, so a
    broken screenshot request cannot leave the command streaming forever.
    """
    cli.tn.write(f"{command}\r\n".encode("utf-8"))
    try:
        time.sleep(run_seconds)
        if during is not None:
            during()
            time.sleep(0.5)
    finally:
        cli.tn.write(b"\x03")
        response = cli.tn.read_until(b">: ", timeout=timeout).decode("utf-8", "ignore")
    return ANSI_RE.sub("", response)


def read_chunks(cli, path, chunk_size):
    """Drive `storage read_chunks`: the device reports the file size, then prints
    'Ready?' before every chunk and waits for one byte back before dumping it.

    telnetlib cannot read an exact number of bytes and `read_some()` happily returns
    the next 'Ready?' along with the chunk, so keep the overflow in a local buffer.
    """
    buffer = b""

    def fill(until):
        nonlocal buffer
        while not until():
            buffer += cli.tn.read_some()

    cli.tn.write(f"storage read_chunks {path} {chunk_size}\r".encode("utf-8"))
    cli.tn.read_until(b"Size: ", timeout=5)
    size = int(cli.tn.read_until(b"\r\n", timeout=5).strip())

    data = b""
    while len(data) < size:
        fill(lambda: b"Ready?\r\n" in buffer)
        buffer = buffer.split(b"Ready?\r\n", 1)[1]
        cli.tn.write(b"y")

        want = min(chunk_size, size - len(data))
        fill(lambda: len(buffer) >= want)
        data, buffer = data + buffer[:want], buffer[want:]

    if b">: " not in buffer:
        cli.tn.read_until(b">: ", timeout=5)
    return data


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("All Commands Coverage")
@pytest.mark.regression
class TestCLIReadOnlyCommands:
    """Level 1 — read-only top-level commands (U5 CLI)."""

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

    @allure.title("CLI. Command status_lights (usage).")
    def test_status_lights_usage(self, persistent_cli_connection):
        # bare call prints usage; the visual result cannot be checked without a camera
        response = persistent_cli_connection.execute_command("status_lights")
        assert "status_lights" in response and "0-255" in response, response

    @allure.title("CLI. Command status_lights set (off).")
    def test_status_lights_set(self, persistent_cli_connection):
        # reversible: turn the lights off, the command must be accepted without error
        response = persistent_cli_connection.execute_command("status_lights 0 0 0")
        assert "Usage" not in response and "Incorect" not in response, response

    @allure.title("CLI. Command log (levels listing).")
    def test_log_levels(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("log ?")
        for level in ("error", "warn", "info", "debug", "trace"):
            assert f"log {level}" in response, f"level {level} not listed: {response!r}"

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

    @allure.title("CLI. Command sysctl cli_wifi_enabled (toggle, CLI reachable after restore).")
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

        # restoring the flag is not proof that the CLI came back — open a new
        # connection and make it answer before handing the device to the next test
        restored = SimpleCLIConnection()
        assert restored.connect(), "CLI does not accept connections after re-enabling it"
        try:
            assert "__wifi__" in restored.execute_command("echo __wifi__")
        finally:
            restored.disconnect()


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("All Commands Coverage")
@pytest.mark.regression
class TestCLISystemCommands:
    """Level 1 — process/system commands: top, free_blocks, log, loader, update."""

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
    def test_top_streaming(self, persistent_cli_connection):
        # default interval (1s) streams frames until ETX
        response = run_streaming(persistent_cli_connection, "top", run_seconds=2.5)
        assert response.count("Threads:") >= 2, f"expected repeated frames: {response!r}"

    @allure.title("CLI. Command log (streaming, CTRL+C).")
    def test_log_streaming(self, persistent_cli_connection):
        response = run_streaming(persistent_cli_connection, "log info", run_seconds=2.0)
        assert "Current log level: info" in response, response
        assert "Press CTRL+C to stop" in response, response

    @allure.title("CLI. Command loader (usage).")
    def test_loader_usage(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("loader")
        assert "loader <cmd> <args>" in response, response
        assert "open" in response and "kill" in response, response

    @allure.title("CLI. Command loader kill.")
    def test_loader_kill(self, persistent_cli_connection):
        # kills whatever app the desktop currently runs; the desktop restores it
        response = persistent_cli_connection.execute_command("loader kill")
        assert (
            "App stopped successfully" in response or "No app running" in response
        ), response

    @allure.title("CLI. Command update (usage).")
    def test_update_usage(self, persistent_cli_connection):
        # bare call prints usage; a real update is destructive -> L3, not here
        response = persistent_cli_connection.execute_command("update")
        assert "update <917|917_ta" in response, response
        for sub in ("install", "install_tar", "install_web"):
            assert sub in response, f"missing subcommand {sub}: {response!r}"

    @allure.title("CLI. Command sysctl (usage).")
    def test_sysctl_usage(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("sysctl")
        assert "sysctl <cmd>" in response and "Cmd list:" in response, response
        for sub in ("debug", "ui_debug", "cli_wifi_enabled", "websrv_accesslog_level"):
            assert sub in response, f"missing subcommand {sub}: {response!r}"


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("All Commands Coverage")
@pytest.mark.regression
class TestCLIDebugGatedCommands:
    """Level 1 — debug mode and what it gates (the `cli_debug` fixture keeps it on).

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
        # storage_bkp_unlock is gated inside `sysctl` itself (sysctl_visible_debug),
        # so it never shows up in the top-level `?` diff above
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
        # the pin list is what makes the usage useful — it names the drivable rails
        assert "Pins:" in response and "en_audio" in response, response

    @allure.title("CLI. Command otp (usage).")
    def test_otp_usage(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("otp")
        assert "otp <cmd>" in response and "Cmd list:" in response, response
        assert "dump" in response and "program" in response, response

    @allure.title("CLI. Command otp dump.")
    def test_otp_dump(self, persistent_cli_connection):
        # read-only; an unprovisioned bank reads back as all-ff, which is still a
        # valid dump — assert the shape, not the content
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


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("All Commands Coverage")
@pytest.mark.regression
class TestCLIPeripheralCommands:
    """Level 1 — peripherals: audio/display/input.

    The display is checked for real through `GET /api/screen`; sound is only checked
    for a clean run (audibility is verified separately, outside this suite)."""

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
    def test_display_show_screenshot(self, persistent_cli_connection, streaming_api):
        # `display show` holds the image on screen only while it runs, so the frame
        # has to be grabbed mid-stream; after CTRL+C the display goes back to the app
        before = streaming_api.get_screen_bytes(display=1)
        shown = []

        response = run_streaming(
            persistent_cli_connection,
            f"display show back {IMAGE_FILE}",
            run_seconds=2.0,
            during=lambda: shown.append(streaming_api.get_screen_bytes(display=1)),
        )
        assert "Error!" not in response, response

        allure.attach(
            raw_to_png(shown[0], 1), "Back display while shown", allure.attachment_type.PNG
        )
        assert shown[0] != before, "the shown image did not change the back display"

        after = streaming_api.get_screen_bytes(display=1)
        assert after != shown[0], "the image stayed on screen after CTRL+C"

    @allure.title("CLI. Command input dump (streaming, CTRL+C).")
    def test_input_dump_streaming(self, persistent_cli_connection):
        response = run_streaming(persistent_cli_connection, "input dump", run_seconds=1.5)
        assert "Press CTRL+C to stop" in response, response

    @allure.title("CLI. Command input send is observed by input dump.")
    def test_input_send_seen_by_dump(self, persistent_cli_connection, fresh_cli_connection):
        # dump on one connection, inject on another: the event must show up in the dump
        response = run_streaming(
            persistent_cli_connection,
            "input dump",
            run_seconds=0.5,
            during=lambda: fresh_cli_connection.execute_command(
                "input send InputKeyUp InputTypeRelease"
            ),
        )
        assert "key: InputKeyUp type: InputTypeRelease" in response, (
            f"the injected event never reached the input pubsub: {response!r}"
        )


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("All Commands Coverage")
@pytest.mark.regression
class TestCLIFetch:
    """Level 1 — `fetch` against an HTTP server served from the test host itself.

    The device reaches the host over the same link the CLI runs on, so bind the
    server to the local end of the telnet socket — no bench configuration needed.
    """

    PAYLOAD = b"busybar-fetch-test\n" * 10
    DEST = "/ext/fetch_test.bin"

    @pytest.fixture
    def http_server(self, persistent_cli_connection):
        payload = self.PAYLOAD

        class Handler(http.server.BaseHTTPRequestHandler):
            def do_GET(self):
                self.send_response(200)
                self.send_header("Content-Type", "application/octet-stream")
                self.send_header("Content-Length", str(len(payload)))
                self.end_headers()
                self.wfile.write(payload)

            def log_message(self, *args):
                pass

        class Server(http.server.ThreadingHTTPServer):
            # HTTPServer.server_bind() resolves the bound address with getfqdn(),
            # which stalls ~5s on the bench (no reverse DNS for the USB-net range)
            def server_bind(self):
                socketserver.TCPServer.server_bind(self)
                self.server_name, self.server_port = self.server_address[:2]

        host_ip = persistent_cli_connection.tn.sock.getsockname()[0]
        server = Server((host_ip, 0), Handler)
        threading.Thread(target=server.serve_forever, daemon=True).start()
        try:
            yield f"http://{host_ip}:{server.server_address[1]}/payload.bin"
        finally:
            server.shutdown()
            server.server_close()

    @allure.title("CLI. Command fetch (usage).")
    def test_fetch_usage(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("fetch")
        assert "fetch <url> [path]" in response, response

    @allure.title("CLI. Command fetch (download to stdout).")
    def test_fetch_to_stdout(self, persistent_cli_connection, http_server):
        response = persistent_cli_connection.execute_command(
            f"fetch {http_server}", timeout=25, slow_command=True
        )
        assert "HTTP/1.0 200 OK" in response, response
        assert "busybar-fetch-test" in response, response

    @allure.title("CLI. Command fetch (download to file).")
    def test_fetch_to_file(self, persistent_cli_connection, http_server):
        cli = persistent_cli_connection
        cli.execute_command(f"storage remove {self.DEST}")
        try:
            response = cli.execute_command(
                f"fetch {http_server} {self.DEST}", timeout=25, slow_command=True
            )
            assert f"File successfully saved to {self.DEST}" in response, response

            stat = cli.execute_command(f"storage stat {self.DEST}")
            assert f"size: {len(self.PAYLOAD)}b" in stat, stat
            md5 = cli.execute_command(f"storage md5 {self.DEST}")
            assert hashlib.md5(self.PAYLOAD).hexdigest() in md5, (
                f"downloaded file differs from what was served: {md5!r}"
            )
        finally:
            cli.execute_command(f"storage remove {self.DEST}")


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("All Commands Coverage")
@pytest.mark.regression
class TestCLIStorageReadOnly:
    """Level 1 — read-only storage subcommands on /ext (no filesystem mutations)."""

    @allure.title("CLI. storage. Command info.")
    def test_storage_info(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("storage info /ext")
        for field in ("Type:", "Total:", "Free:", "Used:"):
            assert field in response, f"missing field {field}: {response!r}"

    @allure.title("CLI. storage. Command list.")
    def test_storage_list(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("storage list /ext")
        # /ext always has at least one directory ([D]) or file ([F])
        assert "[D]" in response or "[F]" in response, response

    @allure.title("CLI. storage. Command stat.")
    def test_storage_stat(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("storage stat /ext")
        assert response.strip(), "stat should return directory info"
        assert "Usage" not in response and "Error" not in response, response

    @allure.title("CLI. storage. Command tree.")
    def test_storage_tree(self, persistent_cli_connection):
        # recursive listing of a small standard dir; 'Empty' is a valid result
        response = persistent_cli_connection.execute_command("storage tree /ext/update")
        assert response.strip(), "tree should return output"
        assert "Storage error" not in response and "Usage" not in response, response

    @allure.title("CLI. storage. Command read.")
    def test_storage_read(self, persistent_cli_connection):
        # small text file that always exists on /ext
        response = persistent_cli_connection.execute_command(
            "storage read /ext/.sys_update.txt"
        )
        assert "Size:" in response, f"expected a size header: {response!r}"
        assert "Storage error" not in response, response

    @allure.title("CLI. storage. Command timestamp.")
    def test_storage_timestamp(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command(
            "storage timestamp /ext/.sys_update.txt"
        )
        assert re.search(r"Timestamp\s+\d+", response), (
            f"expected 'Timestamp <number>': {response!r}"
        )


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("All Commands Coverage")
@pytest.mark.regression
class TestCLIStorageMutating:
    """Level 1 — reversible mutating operations on /ext (everything is cleaned up)."""

    @allure.title("CLI. storage. File round-trip (copy/md5/stat/rename/remove).")
    def test_storage_file_roundtrip(self, persistent_cli_connection, storage_dir):
        cli, d = persistent_cli_connection, storage_dir
        # self-contained source file: archive a standard directory
        tar = cli.execute_command(f"tar c {d}/seed.tar /ext/user_assets", timeout=10)
        assert "RET: 0" in tar, tar
        cp = cli.execute_command(f"storage copy {d}/seed.tar {d}/copy.bin")
        assert "error" not in cp.lower(), cp
        md5 = cli.execute_command(f"storage md5 {d}/copy.bin")
        assert re.search(r"\b[0-9a-f]{32}\b", md5), f"no md5 hash: {md5!r}"
        stat = cli.execute_command(f"storage stat {d}/copy.bin")
        assert "size" in stat.lower(), stat
        cli.execute_command(f"storage rename {d}/copy.bin {d}/renamed.bin")
        lst = cli.execute_command(f"storage list {d}")
        assert "renamed.bin" in lst, lst
        rm = cli.execute_command(f"storage remove {d}/renamed.bin")
        assert "error" not in rm.lower(), rm

    @allure.title("CLI. storage. Command extract (tar archive).")
    def test_storage_extract(self, persistent_cli_connection, storage_dir):
        cli, d = persistent_cli_connection, storage_dir
        c = cli.execute_command(f"tar c {d}/seed.tar /ext/user_assets", timeout=10)
        assert "RET: 0" in c, c
        ex = cli.execute_command(f"storage extract {d}/seed.tar {d}/out", timeout=10)
        assert "success" in ex.lower(), ex
        assert "Storage error" not in ex, ex

    @allure.title("CLI. Command tar (compress + extract).")
    def test_tar_compress_extract(self, persistent_cli_connection, storage_dir):
        cli, d = persistent_cli_connection, storage_dir
        c = cli.execute_command(f"tar c {d}/seed.tar /ext/user_assets", timeout=10)
        assert "RET: 0" in c and "success" in c.lower(), c
        x = cli.execute_command(f"tar x {d}/seed.tar {d}/out", timeout=10)
        assert "RET: 0" in x and "success" in x.lower(), x

    @allure.title("CLI. storage. Command write (interactive text).")
    def test_storage_write(self, persistent_cli_connection, storage_dir):
        cli = persistent_cli_connection
        path = f"{storage_dir}/written.txt"
        # CR only: `storage write` starts reading raw keys right after the command,
        # so the LF of a CRLF would land in the file as its first byte
        cli.tn.write(f"storage write {path}\r".encode("utf-8"))
        cli.tn.read_until(b"exit by Ctrl+C.", timeout=5)
        cli.tn.write(TEXT_PAYLOAD + b"\x03")  # ETX flushes the buffer and exits
        cli.tn.read_until(b">: ", timeout=10)

        stat = cli.execute_command(f"storage stat {path}")
        assert f"size: {len(TEXT_PAYLOAD)}b" in stat, stat
        md5 = cli.execute_command(f"storage md5 {path}")
        assert hashlib.md5(TEXT_PAYLOAD).hexdigest() in md5, f"content differs: {md5!r}"

    @allure.title("CLI. storage. Commands write_chunk + read_chunks (binary round-trip).")
    def test_storage_chunks_roundtrip(self, persistent_cli_connection, storage_dir):
        cli = persistent_cli_connection
        path = f"{storage_dir}/chunked.bin"

        # write_chunk <path> <size>: 'Ready' and then exactly <size> raw bytes
        cli.tn.write(f"storage write_chunk {path} {len(BLOB_PAYLOAD)}\r".encode("utf-8"))
        cli.tn.read_until(b"Ready\r\n", timeout=5)
        cli.tn.write(BLOB_PAYLOAD)
        cli.tn.read_until(b">: ", timeout=10)

        md5 = cli.execute_command(f"storage md5 {path}")
        assert hashlib.md5(BLOB_PAYLOAD).hexdigest() in md5, f"content differs: {md5!r}"

        data = read_chunks(cli, path, chunk_size=128)
        assert data == BLOB_PAYLOAD, "read_chunks returned different bytes than were written"

    @allure.title("CLI. Command log_dump.")
    def test_log_dump(self, persistent_cli_connection):
        cli = persistent_cli_connection
        try:
            response = cli.execute_command("log_dump", timeout=15, slow_command=True)
            assert "Log successfully saved to /ext/dump.log" in response, response
            stat = cli.execute_command("storage stat /ext/dump.log")
            assert re.search(r"size:\s*[1-9]\d*b", stat), f"empty log dump: {stat!r}"
        finally:
            cli.execute_command("storage remove /ext/dump.log")

    @allure.title("CLI. Command storage_benchmark.")
    def test_storage_benchmark(self, persistent_cli_connection):
        cli = persistent_cli_connection
        try:
            response = cli.execute_command("storage_benchmark", timeout=60, slow_command=True)
            assert "SD card is alive" in response, response
            assert re.search(r"Total files:\s+\d+", response), response
            # write+read pass for every block size, 512B .. 128KiB
            assert re.search(r"Write 512 bytes took .* speed .* KiB/s", response), response
            assert re.search(r"Read 131072 bytes took .* speed .* KiB/s", response), response
            assert "Failed to" not in response and "mismatch" not in response, response
        finally:
            # the benchmark always leaves its scratch file behind
            cli.execute_command("storage remove /ext/benchmark.bin")

    @allure.title("CLI. Command crypto_backup (create/verify cycle).")
    def test_crypto_backup_cycle(self, persistent_cli_connection):
        cli = persistent_cli_connection
        # `create` writes /bkp/crypto_backup.bin with O_CREATE_NEW, so a leftover
        # file from a prior run makes it fail (and permanently skip). Start clean.
        cli.execute_command("crypto_backup remove", timeout=10)
        try:
            created = cli.execute_command("crypto_backup create", timeout=25, slow_command=True)
            if "RET: 1" in created or "Error" in created:
                pytest.skip(
                    "crypto_backup unavailable on an unprovisioned device "
                    f"(enclave/OTP invalid): {created.strip()[:120]!r}"
                )
            assert "RET: 0" in created or "Progress: 100%" in created, created
            verified = cli.execute_command("crypto_backup verify", timeout=25, slow_command=True)
            assert "RET: 0" in verified, verified
        finally:
            cli.execute_command("crypto_backup remove", timeout=10)


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("All Commands Coverage")
@pytest.mark.regression
class TestCLIMatterReadOnly:
    """Level 1 — read-only matter submenu commands (enter/leave via execute_command)."""

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
@allure.story("All Commands Coverage")
@pytest.mark.regression
@pytest.mark.uses_si917
class TestCLI917ReadOnlyCommands:
    """Level 1 — read-only 917 CLI commands (entered via the sl_cli fixture)."""

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


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Commands Check")
class TestCLICommandsSession:
    """Base set — runs outside the regression sweep (these carry TestOps ids)."""

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

    @allure.id("2045")
    @allure.title("CLI. Command Help.")
    @pytest.mark.story_commands_check
    def test_cli_command_help(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("power")
        assert "Usage:" in response, "Should contain guidance on command usage"
        assert "power <cmd> <args>" in response, "Should explain command arguments"
        assert "Cmd list:" in response, "Should list all sub-commands"

    @allure.id("2044")
    @allure.title("CLI. Command Storage.")
    @pytest.mark.story_commands_check
    def test_cli_command_storage(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("storage")
        assert response.strip(), "Storage command should return storage information"
        assert len(response.strip()) > 10, "Storage output should be substantial"

    @allure.id("2040")
    @allure.title("CLI. Command Sl_cli.")
    @pytest.mark.story_commands_check
    def test_cli_command_sl_cli(self, fresh_cli_connection):
        # a fresh connection: sl_cli is exclusive and must not be entered on the
        # module-scoped one, which the rest of the file keeps at the main prompt
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

    @allure.id("2026")
    @allure.title("CLI. Command Power.")
    @pytest.mark.story_commands_check
    def test_cli_command_power(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("power")
        assert "Usage:" in response, "Power command should provide help information"
        assert "power <cmd> <args>" in response, "Should show usage format"
        assert "Cmd list:" in response, "Should show available subcommands"

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

    @allure.id("2034")
    @allure.title("CLI. Command Free_blocks.")
    @pytest.mark.story_commands_check
    def test_cli_command_free_blocks(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("free_blocks")
        # heap block dump: lines like 'A 200B2660 S 1821080'
        assert re.search(r"A\s+[0-9A-Fa-f]+\s+S\s+\d+", response), response


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("UI Validation")
class TestCLIUI:
    """Base set — CLI UI rendering."""

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
    """Base set — 917 CLI entry/exit robustness."""

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
