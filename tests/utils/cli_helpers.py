"""Shared helpers and constants for the CLI test suite (integration/cli/).

Fixtures live in integration/cli/conftest.py; the plain functions and constants
here are imported directly by the test modules.
"""

from __future__ import annotations

import re
import time

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


def run_streaming(cli, command, run_seconds=1.5, timeout=10.0, during=None, until=None):
    """Run a command that streams until CTRL+C (`log`, `top`, `input dump`,
    `display show`) and return its output.

    Those commands loop on `cli_is_pipe_broken_or_is_etx_next_char()`, so they
    never reach the prompt on their own and `execute_command()` would just time
    out mid-stream, leaving the connection out of sync. Drive the socket
    directly: stream, send ETX (0x03), then drain back to the prompt.

    Tolerance instead of fixed sleeps: the output is read progressively and, when
    `until(text)` is given, reading stops as soon as it holds — `run_seconds` is
    only an upper bound. A slow device that emits the banner or the awaited line
    late no longer flakes; a genuinely broken command still fails, just after the
    cap. `during` fires once the command has had a moment to start (an injected
    input event, a screenshot grab); if `until` is set, reading then continues
    until the effect shows up. It runs under try/finally, so the ETX always goes
    out even if `during` raises.
    """
    # drop anything a previous command left buffered, so `until` can't match on it
    try:
        cli.tn.read_very_eager()
    except EOFError:
        pass

    cli.tn.write(f"{command}\r\n".encode("utf-8"))
    chunks = []

    def pump(max_seconds, predicate):
        deadline = time.monotonic() + max_seconds
        while time.monotonic() < deadline:
            chunks.append(cli.tn.read_very_eager().decode("utf-8", "ignore"))
            if predicate is not None and predicate(ANSI_RE.sub("", "".join(chunks))):
                return
            time.sleep(0.05)

    try:
        if during is None:
            pump(run_seconds, until)
        else:
            pump(min(run_seconds, 0.5), None)  # let the command start streaming
            during()
            if until is not None:
                pump(run_seconds, until)  # wait for the effect to show up
    finally:
        cli.tn.write(b"\x03")
        chunks.append(cli.tn.read_until(b">: ", timeout=timeout).decode("utf-8", "ignore"))
    return ANSI_RE.sub("", "".join(chunks))


def read_chunks(cli, path, chunk_size):
    """Drive `storage read_chunks`: the device reports the file size, then prints
    'Ready?' before every chunk and waits for one byte back before dumping it.

    telnetlib cannot read an exact number of bytes and `read_some()` happily returns
    the next 'Ready?' along with the chunk, so keep the overflow in a local buffer.
    """
    buffer = b""

    def fill(until, timeout=30.0):
        nonlocal buffer
        deadline = time.monotonic() + timeout
        while not until():
            chunk = cli.tn.read_eager()  # non-blocking; raises EOFError when closed
            if chunk:
                buffer += chunk
                deadline = time.monotonic() + timeout
            elif time.monotonic() > deadline:
                raise TimeoutError(
                    f"storage read_chunks {path}: no data for {timeout}s"
                )
            else:
                time.sleep(0.05)

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
