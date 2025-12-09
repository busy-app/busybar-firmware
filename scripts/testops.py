#!/usr/bin/env python3
"""
BusyBar Operations Tool (class-based commands)

Usage:
  python testops.py wait --host HOST --port PORT -t TIMEOUT
  python testops.py get-version [--host HOST] [--telnet-port PORT] [-t TIMEOUT]
  python testops.py power [COMMAND ...] [--host HOST] [--telnet-port PORT] [-t TIMEOUT]
  python testops.py update-bundle PATH.json [--host HOST] [--telnet-port PORT] [--update-timeout SECONDS]
  python testops.py unit-tests [--host HOST] [--telnet-port PORT] [-t TIMEOUT]
  python testops.py device_info [--host HOST] [--telnet-port PORT] [-t TIMEOUT]
  python testops.py update-fw TAR_PATH [--name NAME] [--host HOST] [-t TIMEOUT]

Environment:
  BUSYBAR_IP   Default host if --host is not provided (default: 10.0.4.20)
  BUSIBAR_PORT Default port if --port is not provided (default: 23)
  LOG_LEVEL    Logging level name (e.g., INFO, DEBUG). Default: INFO
"""

import argparse
import dataclasses
import logging
import os
import re
import socket
import subprocess
import sys
import telnetlib
import time
import urllib.request
import urllib.error
from time import sleep
from typing import List, Optional, Tuple


def _normalize_input_key(raw_key: str) -> str:
    """
    Accept flexible key names (with/without InputKey prefix) and normalize to InputKeyXxx.
    """
    cleaned = raw_key.strip()
    if not cleaned:
        raise ValueError("Key name cannot be empty")

    suffix = cleaned[8:] if cleaned.lower().startswith("inputkey") else cleaned
    suffix = suffix.strip()
    if not suffix:
        raise ValueError("Key name missing after InputKey prefix")

    parts = re.split(r"[_\s-]+", suffix)
    normalized_parts = []
    for part in parts:
        if not part:
            continue
        normalized_parts.append(part[:1].upper() + part[1:])

    if not normalized_parts:
        raise ValueError(f"Unable to normalize key name '{raw_key}'")

    return "InputKey" + "".join(normalized_parts)


def _setup_root_logger() -> logging.Logger:
    level_name = os.getenv("LOG_LEVEL", "INFO").upper()
    level = getattr(logging, level_name, logging.INFO)
    logger = logging.getLogger("busybar")
    if not logger.handlers:
        logger.setLevel(level)
        handler = logging.StreamHandler()
        fmt = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
        handler.setFormatter(fmt)
        logger.addHandler(handler)
    return logger


LOG = _setup_root_logger()


@dataclasses.dataclass(frozen=True)
class TelnetSettings:
    host: str = os.getenv("BUSYBAR_IP", "10.0.4.20")
    port: int = int(os.getenv("BUSYBAR_TELNET_PORT", "23"))
    timeout: int = 10  # seconds
    prompt_patterns: Tuple[re.Pattern, ...] = dataclasses.field(
        default_factory=lambda: (
            re.compile(rb">:"),  # '>:'
        )
    )


@dataclasses.dataclass(frozen=True)
class BusyBarConfig:
    http_port: int = 80
    telnet: TelnetSettings = dataclasses.field(default_factory=TelnetSettings)


@dataclasses.dataclass
class CommandResult:
    ok: bool
    command: str
    stdout: str
    duration_sec: float


class TelnetClient:
    def __init__(self, settings: TelnetSettings, logger: Optional[logging.Logger] = None):
        self.settings = settings
        self._tn: Optional[telnetlib.Telnet] = None
        self._logger = logger or LOG.getChild("telnet")
        self._welcome: Optional[bytes] = None

    def __enter__(self) -> "TelnetClient":
        self.connect()
        return self

    def __exit__(self, exc_type, exc, tb):
        self.close()

    @property
    def is_connected(self) -> bool:
        return self._tn is not None

    def connect(self) -> None:
        try:
            self._logger.debug(
                f"Connecting to {self.settings.host}:{self.settings.port} (timeout={self.settings.timeout})")
            self._tn = telnetlib.Telnet(self.settings.host, self.settings.port, timeout=self.settings.timeout)
            self._welcome = self._read_until_prompt(timeout=3.0)
        except Exception as e:
            self._logger.error(f"Failed to connect to {self.settings.host}:{self.settings.port} - {e}")
            raise

    def close(self) -> None:
        if self._tn:
            try:
                self._tn.close()
            finally:
                self._tn = None

    def read_welcome(self) -> str:
        if self._welcome is None:
            try:
                self._welcome = self._read_until_prompt(timeout=3.0)
            except Exception as e:
                self._logger.debug(f"No welcome captured: {e}")
                self._welcome = b""
        return self._welcome.decode("utf-8", errors="ignore")

    def sendline(self, command: str) -> None:
        if not self._tn:
            raise RuntimeError("Telnet not connected")
        self._logger.debug(f">> {command}")
        self._tn.write(command.encode("utf-8") + b"\r\n")

    def run(self, command: str, timeout: Optional[float] = 10) -> CommandResult:
        if not self._tn:
            raise RuntimeError("Telnet not connected")

        start = time.perf_counter()
        self.sendline(command)
        try:
            raw = self._read_until_prompt(timeout=timeout or self.settings.timeout)
        except TimeoutError as e:
            self._logger.error(f"Timeout waiting for command '{command}' to complete: {e}")
            return CommandResult(ok=False, command=command, stdout="", duration_sec=time.perf_counter() - start)
        duration = time.perf_counter() - start
        cleaned = self._clean_command_output(raw, command)
        self._logger.debug(f"<< {cleaned.strip()}")
        end = time.perf_counter()
        return CommandResult(ok=True, command=command, stdout=cleaned, duration_sec=duration)

    def _read_until_prompt(self, timeout: float) -> bytes:
        """
        Wait for any configured prompt regex. Returns buffer INCLUDING the matched prompt.
        """
        if not self._tn:
            raise RuntimeError("Telnet not connected")

        idx, match, text = self._tn.expect(list(self.settings.prompt_patterns), timeout)
        if idx == -1:
            self._logger.warning("Prompt not detected within timeout; returning buffered data")
            return text
        return text

    @staticmethod
    def _clean_command_output(buffer: bytes, command: str) -> str:
        """
        Remove command echo and trailing prompt from the buffer.
        """
        text = buffer.decode("utf-8", errors="ignore").replace("\r", "")
        lines = text.split("\n")

        if lines and lines[0].strip() == command.strip():
            lines = lines[1:]

        if lines:
            lines[-1] = re.sub(r"(>:\s*|\.>\s*|>\s*)$", "", lines[-1])

        if lines and lines[-1].strip() == "":
            lines = lines[:-1]

        while lines and lines[0].strip() == "":
            lines = lines[1:]
        while lines and lines[-1].strip() == "":
            lines = lines[:-1]

        return "\n".join(lines)

    def run_until_pattern(
            self,
            command: str,
            patterns: str | list[str],
            timeout: float,
            error_patterns: list[str] | None = None
    ) -> CommandResult:
        """
        Send a command and wait for one or more specific patterns in the output instead of waiting for prompt.
        Useful for commands that don't return to prompt (like update commands).

        Args:
            command: The command to send
            pattern: A text pattern or list of patterns to wait for in the output
            timeout: Maximum time to wait for any of the patterns
            error_patterns: List of error patterns that should cause immediate failure

        Returns:
            CommandResult with ok=True if any success pattern found,
            ok=False if timeout or an error pattern is found.
        """
        if not self._tn:
            raise RuntimeError("Telnet not connected")

        start = time.perf_counter()
        self.sendline(command)

        accumulated = b""
        patterns_bytes = [pattern.encode("utf-8") for pattern in patterns]
        error_patterns_bytes = [err.encode("utf-8") for err in (error_patterns or [])]

        while time.perf_counter() - start < timeout:
            try:
                # Read any available data without blocking
                data = self._tn.read_very_eager()
                if data:
                    accumulated += data
                    self._logger.debug(f"Read: {data.decode('utf-8', errors='ignore')}")

                    # Check for error patterns first
                    for error_pattern in error_patterns_bytes:
                        if error_pattern in accumulated:
                            duration = time.perf_counter() - start
                            cleaned = self._clean_command_output(accumulated, command)
                            self._logger.error(f"Error pattern '{error_pattern.decode('utf-8')}' found after {duration:.2f}s")
                            return CommandResult(ok=False, command=command, stdout=cleaned, duration_sec=duration)

                    # Check if we found the success pattern
                    for pattern in patterns_bytes:
                        if pattern in accumulated:
                            duration = time.perf_counter() - start
                            cleaned = self._clean_command_output(accumulated, command)
                            self._logger.info(f"Pattern '{pattern}' found after {duration:.2f}s")
                            return CommandResult(ok=True, command=command, stdout=cleaned, duration_sec=duration)

                time.sleep(0.1)  # Small delay to avoid busy waiting
            except Exception as e:
                self._logger.debug(f"Error reading data: {e}")
                break

        # Timeout occurred
        duration = time.perf_counter() - start
        cleaned = self._clean_command_output(accumulated, command) if accumulated else ""
        self._logger.warning(f"Pattern '{pattern}' not found within {timeout}s timeout")
        return CommandResult(ok=False, command=command, stdout=cleaned, duration_sec=duration)


    def run_interactive(self, command: str, responses: dict, timeout: float = None) -> CommandResult:
        """
        Run a command that requires interactive responses.

        Args:
            command: The initial command to send
            responses: Dict mapping expected prompts (as regex patterns) to responses
            timeout: Overall timeout for the entire interaction

        Returns:
            CommandResult with accumulated output
        """
        if not self._tn:
            raise RuntimeError("Telnet not connected")

        start = time.perf_counter()
        timeout = timeout or self.settings.timeout
        self.sendline(command)

        accumulated = b""

        # Compile response patterns
        response_patterns = [(re.compile(pattern.encode('utf-8')), response)
                             for pattern, response in responses.items()]

        while time.perf_counter() - start < timeout:
            try:
                data = self._tn.read_very_eager()
                if data:
                    accumulated += data
                    decoded = accumulated.decode('utf-8', errors='ignore')
                    self._logger.debug(f"Read: {data.decode('utf-8', errors='ignore')}")

                    for pattern, response in response_patterns:
                        if pattern.search(accumulated):
                            self._logger.info(f"Found prompt pattern, sending: {response}")
                            self.sendline(response)
                            # Remove this pattern from list after responding
                            response_patterns.remove((pattern, response))
                            break

                    # Check if we're back at the main prompt (command complete)
                    for prompt_pattern in self.settings.prompt_patterns:
                        if prompt_pattern.search(accumulated):
                            duration = time.perf_counter() - start
                            cleaned = self._clean_command_output(accumulated, command)
                            self._logger.info(f"Command completed after {duration:.2f}s")
                            return CommandResult(ok=True, command=command, stdout=cleaned, duration_sec=duration)

                time.sleep(0.1)
            except Exception as e:
                self._logger.error(f"Error during interactive command: {e}")
                break

        duration = time.perf_counter() - start
        cleaned = self._clean_command_output(accumulated, command) if accumulated else ""
        self._logger.warning(f"Interactive command timed out after {timeout}s")
        return CommandResult(ok=False, command=command, stdout=cleaned, duration_sec=duration)


class BusyBarDevice:
    """
    High-level device operations executed via a TelnetClient transport.
    """

    _VERSION_PATTERNS: Tuple[re.Pattern, ...] = (
        re.compile(r"Firmware version:\s*([^\n\r]+)", re.IGNORECASE),
        re.compile(r"Version:\s*([^\n\r]+)", re.IGNORECASE),
        re.compile(r"\bv(?P<v>\d+\.\d+\.\d+(?:\.\d+)?)\b", re.IGNORECASE),
    )
    _STATUS_RE = re.compile(r"^\s*Status:\s*(?:\x1b\[[0-9;]*m)?([A-Za-z]+)(?:\x1b\[[0-9;]*m)?\b", re.IGNORECASE | re.MULTILINE)

    def __init__(self, config: BusyBarConfig):
        self.config = config
        self._logger = LOG.getChild("device")

    def _telnet(self, timeout: Optional[int] = None) -> TelnetClient:
        settings = TelnetSettings(
            host=self.config.telnet.host,
            port=self.config.telnet.port,
            timeout=timeout if timeout is not None else self.config.telnet.timeout,
            prompt_patterns=self.config.telnet.prompt_patterns,
        )
        return TelnetClient(settings, logger=LOG.getChild("telnet"))

    def get_version_from_text(self, text: str) -> Optional[str]:
        for pat in self._VERSION_PATTERNS:
            m = pat.search(text)
            if m:
                return m.group(1) if m.groups() else m.group(0)
        return None

    def get_version(self, timeout: int) -> Tuple[bool, str]:
        """
        Connects, reads welcome, extracts version. Falls back to issuing 'version'
        if welcome lacks version.
        """
        with self._telnet(timeout=timeout) as tn:
            welcome = tn.read_welcome()
            version = self.get_version_from_text(welcome)

            if version:
                return True, version

            try:
                res = tn.run("version", timeout=timeout)
                v = self.get_version_from_text(res.stdout) or res.stdout.strip()
                if v:
                    return True, v
            except Exception as e:
                self._logger.debug(f"'version' command failed or absent: {e}")

            return False, f"Version not found. Welcome:\n{welcome}"

    def power(self, power_args: List[str], timeout: int) -> Tuple[bool, str]:
        """
        Runs 'power' or 'power ...'.
        """
        cmd = "power" if not power_args else f"power {' '.join(power_args)}"
        with self._telnet(timeout=timeout) as tn:
            _ = tn.read_welcome()  # discard
            res = tn.run(cmd, timeout=timeout)
            return res.ok, (res.stdout or "(Command executed successfully, no output)")

    def update_bundle(self, bundle_path: str, timeout: int = 10) -> Tuple[bool, str]:
        """
        Runs 'update install <bundle.json>' and waits for 'Update preparation successful, rebooting...' message.
        Returns immediately with error if 'Update prepare install failed:' is detected.
        Closes connection immediately after seeing success message since device won't return to prompt.
        """
        cmd = f"update install {bundle_path}"
        with self._telnet(timeout=timeout) as tn:
            _ = tn.read_welcome()
            self._logger.info(f"Executing: {cmd}")

            # Define error patterns that should cause immediate failure
            error_patterns = [
                "Update prepare install failed:"
            ]
            ok_patterns = [
                "Updater configuration valid",
                "Update preparation successful"
            ]

            # Use the method with error pattern detection
            res = tn.run_until_pattern(cmd, ok_patterns, timeout=timeout, error_patterns=error_patterns)

            if res.ok:
                return True, res.stdout if res.stdout else "Update initiated successfully - 'Update preparation successful, rebooting...' detected"
            else:
                # Check if the error was due to invalid manifest path
                if "Failed to load updater configuration: Manifest path invalid" in res.stdout:
                    return False, f"Update prepare install failed:. Output: {res.stdout}"
                elif res.stdout:
                    return False, f"Update command failed or timed out. Output: {res.stdout}"
                else:
                    return False, "Update command timed out with no output"

    def run_unit_tests(self, timeout: int) -> Tuple[bool, str]:
        """
        Run device 'unit_tests' and return (passed, full_output_or_message).
        passed=True -> tests passed (Status: PASSED or 'PASSED' token detected)
        """
        with self._telnet(timeout=timeout) as tn:
            res = tn.run("unit_tests", timeout=timeout)
            out = res.stdout

            # Prefer explicit "Status: X"
            m = self._STATUS_RE.search(out)
            if m:
                status = m.group(1).upper()
                if status == "PASSED":
                    return True, out  # Return full output even when passed
                return False, out

            # Fallback heuristic if explicit status is absent
            if re.search(r"\bPASSED\b", out):
                return True, out  # Return full output even when passed

            return False, out

    def get_device_info(self, timeout: int) -> Tuple[bool, str]:
        """
        Runs 'device_info' and returns the output.
        """
        with self._telnet(timeout=timeout) as tn:
            res = tn.run("device_info", timeout=timeout)
            return res.ok, (res.stdout or "(No output received)")

    def uptime(self, timeout: int) -> Tuple[bool, str]:
        """
        Runs 'uptime' and returns the output.
        """
        with self._telnet(timeout=timeout) as tn:
            res = tn.run("uptime", timeout=timeout)
            return res.ok, (res.stdout or "(No output received)")

    def send_tar(
        self,
        tar_path: str,
        name: str = "firmware",
        timeout: int = 120,
        retries: int = 0,
        retry_delay: float = 5.0,
    ) -> Tuple[bool, str]:
        """
        Upload a tar file to the device via HTTP POST.

        Args:
            tar_path: Local path to the tar file
            name: Update name parameter (default: firmware)
            timeout: HTTP timeout in seconds (default: 120)
            retries: Number of retry attempts if host is unavailable (default: 0)
            retry_delay: Delay in seconds between retries (default: 5.0)

        Returns:
            Tuple of (success, output_message)
        """
        if not os.path.isfile(tar_path):
            return False, f"File not found: {tar_path}"

        file_size = os.path.getsize(tar_path)
        url = f"http://{self.config.telnet.host}:{self.config.http_port}/api/update?name={name}"

        self._logger.info(f"Uploading {tar_path} ({file_size} bytes) to {url}")

        with open(tar_path, "rb") as f:
            data = f.read()

        last_error = None
        for attempt in range(retries + 1):
            if attempt > 0:
                self._logger.info(f"Retry {attempt}/{retries} after {retry_delay}s...")
                time.sleep(retry_delay)

            try:
                req = urllib.request.Request(
                    url,
                    data=data,
                    method="POST",
                    headers={
                        "Content-Type": "application/octet-stream",
                        "Connection": "keep-alive",
                        "Content-Length": str(file_size),
                        "DNT": "1",
                    },
                )

                with urllib.request.urlopen(req, timeout=timeout) as response:
                    status = response.status
                    body = response.read().decode("utf-8", errors="ignore")
                    self._logger.info(f"Response: {status} - {body}")
                    if status == 200:
                        return True, f"Upload successful ({file_size} bytes). Response: {body}"
                    else:
                        return False, f"Upload failed with status {status}. Response: {body}"

            except urllib.error.HTTPError as e:
                body = e.read().decode("utf-8", errors="ignore") if e.fp else ""
                return False, f"HTTP error {e.code}: {e.reason}. Response: {body}"
            except urllib.error.URLError as e:
                last_error = f"URL error: {e.reason}"
                self._logger.warning(f"Attempt {attempt + 1}/{retries + 1} failed: {last_error}")
            except (socket.timeout, TimeoutError) as e:
                last_error = f"Connection timeout: {e}"
                self._logger.warning(f"Attempt {attempt + 1}/{retries + 1} failed: {last_error}")
            except OSError as e:
                last_error = f"Connection error: {e}"
                self._logger.warning(f"Attempt {attempt + 1}/{retries + 1} failed: {last_error}")

        return False, f"Upload failed after {retries + 1} attempts. Last error: {last_error}"

    def storage_format(self, path: str = "/ext", timeout: int = 30) -> Tuple[bool, str]:
        """
        Format storage partition with confirmation.

        Args:
            path: Path to format (default: /ext)
            timeout: Timeout in seconds (default: 30)

        Returns:
            Tuple of (success, output_message)
        """
        cmd = f"storage format {path}"

        # Define expected prompts and responses
        responses = {
            r"Are you sure \(y/n\)\?": "y"
        }

        with self._telnet(timeout=timeout) as tn:
            _ = tn.read_welcome()
            self._logger.info(f"Executing: {cmd}")

            res = tn.run_interactive(cmd, responses, timeout=timeout)

            if res.ok:
                # Check for success message in output
                if "successfully formatted" in res.stdout.lower():
                    return True, res.stdout
                else:
                    return False, f"Format command completed but success not confirmed. Output: {res.stdout}"
            else:
                return False, f"Format command failed or timed out. Output: {res.stdout}"

    def send_input_sequences(
            self,
            sequences: List[Tuple[str, List[str]]],
            timeout: int = 10,
            delay: float = 0.0
    ) -> Tuple[bool, str]:
        """
        Sends one or more input sequences via telnet. Each sequence contains a key and
        the list of InputType events to send in order.
        """
        if not sequences:
            return False, "No input sequences specified"

        with self._telnet(timeout=timeout) as tn:
            _ = tn.read_welcome()
            outputs: List[str] = []
            for idx, (key, input_types) in enumerate(sequences):
                if not input_types:
                    continue
                for input_type in input_types:
                    cmd = f"input send {key} {input_type}"
                    self._logger.info(f"Sending input: {cmd}")
                    res = tn.run(cmd, timeout=timeout)
                    if not res.ok:
                        return False, res.stdout or f"Command '{cmd}' failed or timed out"
                    outputs.append(res.stdout.strip() or f"{cmd} (ok)")

                if delay > 0 and idx < len(sequences) - 1:
                    time.sleep(delay)

            joined = "\n".join(outputs)
            return True, joined or "Input sequence(s) sent"


class BusyBarWaiter:
    def __init__(self):
        self._logger = LOG.getChild("wait")

    @staticmethod
    def _ping_host(host: str, timeout: int = 1) -> bool:
        try:
            result = subprocess.run(
                ["ping", "-c", "1", "-W", str(timeout), host],
                capture_output=True,
                timeout=timeout + 1,
                check=False,
            )
            return result.returncode == 0
        except Exception:
            return False

    @staticmethod
    def _check_port(host: str, port: int, timeout: int = 1) -> bool:
        try:
            with socket.create_connection((host, port), timeout=timeout):
                return True
        except Exception:
            return False

    def wait_for_busybar(self, host: str, port: int, timeout: int) -> bool:
        sleep(2)  # Initial wait to avoid immediate success
        start = time.time()
        self._logger.info(f"Waiting for BusyBar at {host}:{port} (timeout {timeout}s)...")

        while True:
            elapsed = int(time.time() - start)
            if elapsed >= timeout:
                self._logger.error(f"ERROR: BusyBar did not come online after {timeout} seconds")
                return False

            if self._ping_host(host):
                self._logger.info("BusyBar is online (ICMP).")
                if self._check_port(host, port, timeout=2):
                    self._logger.info(f"BusyBar service ready on {host}:{port} (elapsed {elapsed}s).")
                    return True
                else:
                    self._logger.info(f"Host responds but port {port} not ready yet (elapsed {elapsed}s).")
            else:
                self._logger.info(f"No response yet... (elapsed {elapsed}s).")

            time.sleep(2)


class BusyBarTestOps:
    """
    Encapsulates ALL CLI command logic and wiring.
    Transport is handled by BusyBarDevice (which uses TelnetClient).
    """

    def __init__(self) -> None:
        self._logger = LOG.getChild("ops")
        self.default_host = os.getenv("BUSYBAR_IP", "10.0.4.20")
        self.default_port = int(os.getenv("BUSYBAR_TELNET_PORT", "23"))

    def build_parser(self) -> argparse.ArgumentParser:
        parser = argparse.ArgumentParser(
            description="BusyBar Operations Tool",
            formatter_class=argparse.RawDescriptionHelpFormatter,
        )
        subparsers = parser.add_subparsers(
            title="commands",
            description="Available commands",
            help="Command help",
            dest="command",
            required=True,
        )

        p_wait = subparsers.add_parser(
            "wait",
            help="Wait for BusyBar to come online",
            description="Wait for BusyBar service to become available",
        )
        p_wait.add_argument(
            "--host",
            default=self.default_host,
            help="BusyBar IP address or hostname",
            dest="host",
        )
        p_wait.add_argument("--port", type=int, default=80, help="Port to check (default: 80)")
        p_wait.add_argument("-t", "--timeout", type=int, default=60, help="Timeout in seconds (default: 60)")
        p_wait.set_defaults(func=self._cmd_wait)

        # get-version
        p_ver = subparsers.add_parser(
            "get-version",
            help="Get firmware version from device via telnet",
            description="Connect to device via telnet and extract firmware version from welcome message",
        )
        p_ver.add_argument("--host", default=self.default_host, help="Device IP/host")
        p_ver.add_argument("--telnet-port", type=int, default=self.default_port, help="Telnet port (default: 23)")
        p_ver.add_argument("-t", "--timeout", type=int, default=10, help="Connection timeout in seconds (default: 10)")
        p_ver.set_defaults(func=self._cmd_get_version)

        p_power = subparsers.add_parser(
            "power",
            help="Execute power-related commands on device",
            description="Execute power commands (info, off, reboot, boot, ch, ch_current, pd_info, pd_set) via telnet",
        )
        p_power.add_argument("power_cmd", nargs="*",
                             help='Power command and arguments (e.g., "reboot", "ch on", "pd_info")')
        p_power.add_argument("--host", default=self.default_host, help="Device IP/host")
        p_power.add_argument("--telnet-port", type=int, default=self.default_port, help="Telnet port (default: 23)")
        p_power.add_argument("-t", "--timeout", type=int, default=10,
                             help="Connection timeout in seconds (default: 10)")
        p_power.set_defaults(func=self._cmd_power)

        p_input = subparsers.add_parser(
            "input",
            help="Send key events via telnet",
            description="Inject key presses using the BusyBar 'input send' command.",
        )
        p_input.add_argument("key", nargs="?", help="Key name (e.g., InputKeyStart or 'start')")
        p_input.add_argument(
            "-s",
            "--sequence",
            nargs="+",
            metavar="KEY",
            help="Send a series of short presses for the provided keys"
        )
        p_input.add_argument("--long", action="store_true", help="Use InputTypeLong instead of Short")
        p_input.add_argument("--only", action="store_true", help="Send only a single Short/Long event")
        pr_group = p_input.add_mutually_exclusive_group()
        pr_group.add_argument("--press", action="store_true", help="Send only a Press event")
        pr_group.add_argument("--release", action="store_true", help="Send only a Release event")
        p_input.add_argument(
            "-t",
            "--delay",
            dest="sequence_delay",
            type=float,
            default=0.0,
            help="Delay in seconds between sequential presses when using -s"
        )
        p_input.add_argument("--host", default=self.default_host, help="Device IP/host")
        p_input.add_argument("--telnet-port", type=int, default=self.default_port, help="Telnet port (default: 23)")
        p_input.add_argument("--timeout", type=int, default=10,
                             help="Connection timeout in seconds (default: 10)")
        p_input.set_defaults(func=self._cmd_input)

        p_upd = subparsers.add_parser(
            "update-bundle",
            help="Update firmware with bundle file",
            description="Execute firmware update with specified bundle file via telnet",
        )
        p_upd.add_argument("bundle_path", default="/ext/tmp/upd_bundle", help="Path to the bundle.json file")
        p_upd.add_argument("--host", default=self.default_host, help="Device IP/host")
        p_upd.add_argument("--telnet-port", type=int, default=self.default_port, help="Telnet port (default: 23)")
        p_upd.add_argument("--update-timeout", type=int, default=120, help="Update timeout in seconds (default: 120)")
        p_upd.set_defaults(func=self._cmd_update_bundle)

        p_tests = subparsers.add_parser(
            "unit-tests",
            help="Run on-device unit_tests and report PASSED or full output",
            description="Runs 'unit_tests' via telnet; prints 'PASSED' if tests passed else prints full output",
            aliases=["unit_tests", "tests"],
        )
        p_tests.add_argument("--host", default=self.default_host, help="Device IP/host")
        p_tests.add_argument("--telnet-port", type=int, default=self.default_port, help="Telnet port (default: 23)")
        p_tests.add_argument("-t", "--timeout", type=int, default=120, help="Timeout in seconds (default: 120)")
        p_tests.set_defaults(func=self._cmd_unit_tests)

        p_devinfo = subparsers.add_parser(
            "device_info",
            help="Get device information via telnet",
            description="Runs 'device_info' command via telnet and prints the output",
            aliases=["device-info", "info"],
        )
        p_devinfo.add_argument("--host", default=self.default_host, help="Device IP/host")
        p_devinfo.add_argument("--telnet-port", type=int, default=self.default_port, help="Telnet port (default: 23)")
        p_devinfo.add_argument("-t", "--timeout", type=int, default=10, help="Timeout in seconds (default: 10)")
        p_devinfo.set_defaults(func=self._cmd_device_info)

        p_sanity = subparsers.add_parser(
            "sanity-check",
            help="Perform a sanity check on the device",
            description="Checks device_info and version for expected fields",
            aliases=["sanity", "check"],
        )
        p_sanity.add_argument("--host", default=self.default_host, help="Device IP/host")
        p_sanity.add_argument("--telnet-port", type=int, default=self.default_port, help="Telnet port (default: 23)")
        p_sanity.add_argument("-t", "--timeout", type=int, default=10, help="Timeout in seconds (default: 10)")
        p_sanity.set_defaults(func=self._cmd_sanity_check)

        p_uptime = subparsers.add_parser(
            "uptime",
            help="Get device uptime via telnet",
            description="Runs 'uptime' command via telnet and prints the output",
            aliases=["up"],
        )
        p_uptime.add_argument("--host", default=self.default_host, help="Device IP/host")
        p_uptime.add_argument("--telnet-port", type=int, default=self.default_port, help="Telnet port (default: 23)")
        p_uptime.add_argument("-t", "--timeout", type=int, default=10, help="Timeout in seconds (default: 10)")
        p_uptime.set_defaults(func=self._cmd_uptime)

        p_format = subparsers.add_parser(
            "storage-format",
            help="Format storage partition (with automatic confirmation)",
            description="Executes 'storage format' command and automatically confirms the operation",
            aliases=["format"],
        )
        p_format.add_argument(
            "--path",
            default="/ext",
            help="Path to format (default: /ext)"
        )
        p_format.add_argument("--host", default=self.default_host, help="Device IP/host")
        p_format.add_argument("--telnet-port", type=int, default=self.default_port, help="Telnet port (default: 23)")
        p_format.add_argument("-t", "--timeout", type=int, default=30, help="Timeout in seconds (default: 30)")
        p_format.add_argument(
            "--no-confirm",
            action="store_true",
            help="Skip confirmation prompt (for testing - not recommended)"
        )
        p_format.set_defaults(func=self._cmd_storage_format)

        p_send_tar = subparsers.add_parser(
            "update-fw",
            help="Upload a firmware tar file to the device via HTTP",
            description="Uploads a firmware tar archive to the device update endpoint",
            aliases=["update_fw"],
        )
        p_send_tar.add_argument("tar_path", help="Path to the tar file to upload")
        p_send_tar.add_argument(
            "--name",
            default="firmware",
            help="Update name parameter (default: firmware)"
        )
        p_send_tar.add_argument("--host", default=self.default_host, help="Device IP/host")
        p_send_tar.add_argument("-t", "--timeout", type=int, default=120, help="HTTP timeout in seconds (default: 120)")
        p_send_tar.add_argument(
            "-r", "--retries",
            type=int,
            default=0,
            help="Number of retry attempts if host is unavailable (default: 0)"
        )
        p_send_tar.add_argument(
            "--retry-delay",
            type=float,
            default=5.0,
            help="Delay in seconds between retries (default: 5.0)"
        )
        p_send_tar.set_defaults(func=self._cmd_send_tar)

        return parser

    @staticmethod
    def _make_device_from_args(host: str, telnet_port: int, timeout: int) -> BusyBarDevice:
        config = BusyBarConfig(
            http_port=80,
            telnet=TelnetSettings(host=host, port=telnet_port, timeout=timeout),
        )
        return BusyBarDevice(config)

    def _cmd_wait(self, args: argparse.Namespace) -> int:
        waiter = BusyBarWaiter()
        ok = waiter.wait_for_busybar(host=args.host, port=args.port, timeout=args.timeout)
        return 0 if ok else 1

    def _cmd_get_version(self, args: argparse.Namespace) -> int:
        device = self._make_device_from_args(args.host, args.telnet_port, args.timeout)
        ok, msg = device.get_version(timeout=args.timeout)
        if ok:
            print(msg)
            return 0
        print(msg, file=sys.stderr)
        return 1

    def _cmd_power(self, args: argparse.Namespace) -> int:
        device = self._make_device_from_args(args.host, args.telnet_port, args.timeout)
        ok, msg = device.power(power_args=args.power_cmd or [], timeout=args.timeout)
        print(msg if msg else "(Command executed successfully, no output)")
        return 0 if ok else 1

    def _cmd_input(self, args: argparse.Namespace) -> int:
        sequences: List[Tuple[str, List[str]]] = []
        sequence_delay = 0.0

        if args.sequence:
            if args.key:
                print("Positional key cannot be used with -s/--sequence", file=sys.stderr)
                return 1
            if args.long or args.only or args.press or args.release:
                print("-s/--sequence only supports short presses; remove other modifiers", file=sys.stderr)
                return 1
            if args.sequence_delay < 0:
                print("Delay must be non-negative", file=sys.stderr)
                return 1

            try:
                normalized_keys = [_normalize_input_key(key) for key in args.sequence]
            except ValueError as exc:
                print(f"Invalid key: {exc}", file=sys.stderr)
                return 1

            sequences = [
                (key, ["InputTypePress", "InputTypeShort", "InputTypeRelease"])
                for key in normalized_keys
            ]
            sequence_delay = args.sequence_delay
        else:
            if not args.key:
                print("Key argument is required unless using -s/--sequence", file=sys.stderr)
                return 1

            if args.sequence_delay:
                print("-t/--delay is only valid with -s/--sequence", file=sys.stderr)
                return 1

            try:
                key = _normalize_input_key(args.key)
            except ValueError as exc:
                print(f"Invalid key: {exc}", file=sys.stderr)
                return 1

            if args.only and (args.press or args.release):
                print("--only cannot be combined with --press or --release", file=sys.stderr)
                return 1

            if args.long and (args.press or args.release):
                print("--long cannot be combined with --press or --release", file=sys.stderr)
                return 1

            if args.press:
                sequence = ["InputTypePress"]
            elif args.release:
                sequence = ["InputTypeRelease"]
            elif args.only:
                sequence = ["InputTypeLong" if args.long else "InputTypeShort"]
            else:
                middle = "InputTypeLong" if args.long else "InputTypeShort"
                sequence = ["InputTypePress", middle, "InputTypeRelease"]

            sequences = [(key, sequence)]

        device = self._make_device_from_args(args.host, args.telnet_port, args.timeout)
        ok, msg = device.send_input_sequences(
            sequences=sequences,
            timeout=args.timeout,
            delay=sequence_delay
        )
        print(msg if msg else "(No output received)")
        return 0 if ok else 1

    def _cmd_update_bundle(self, args: argparse.Namespace) -> int:
        device = self._make_device_from_args(args.host, args.telnet_port, args.update_timeout)
        print(f"Executing: update install {args.bundle_path}")
        ok, msg = device.update_bundle(bundle_path=args.bundle_path, timeout=args.update_timeout)
        print(msg if msg else "(Update command sent successfully, no immediate output)")
        return 0 if ok else 1

    def _cmd_unit_tests(self, args: argparse.Namespace) -> int:
        device = self._make_device_from_args(args.host, args.telnet_port, args.timeout)
        passed, output = device.run_unit_tests(timeout=args.timeout)
        print(output if output else "(No output received)")
        return 0 if passed else 1

    def _cmd_device_info(self, args: argparse.Namespace) -> int:
        device = self._make_device_from_args(args.host, args.telnet_port, args.timeout)
        ok, msg = device.get_device_info(timeout=args.timeout)
        print(msg if msg else "(No output received)")
        return 0 if ok else 1

    def _cmd_sanity_check(self, args: argparse.Namespace) -> int:
        device = self._make_device_from_args(args.host, args.telnet_port, args.timeout)
        ok, msg = device.get_device_info(timeout=args.timeout)
        if not ok:
            return 1

        u5_commit = re.search(r"u5_firmware_commit\s*:\s*([0-9a-fA-F]+)", msg or "")
        print(f"U5 commit: {u5_commit.group(1)}" if u5_commit else "(u5 commit not found)")

        sl_commit = re.search(r"sl_firmware_commit\s*:\s*([0-9a-fA-F]+)", msg or "")
        print(f"SL commit: {sl_commit.group(1)}" if sl_commit else "(sl commit not found)")

        sl_intercom_ok = re.search(r"sl_intercom_status\s*:\s*ok", msg or "", re.IGNORECASE)
        print(f"{sl_intercom_ok.group(0)}" if sl_intercom_ok else "(sl intercom status not OK)")

        if ok and not (u5_commit and sl_commit and sl_intercom_ok):
            print("Sanity check failed: missing expected fields", file=sys.stderr)
            return 1
        return 0

    def _cmd_uptime(self, args: argparse.Namespace) -> int:
        device = self._make_device_from_args(args.host, args.telnet_port, args.timeout)
        ok, msg = device.uptime(timeout=args.timeout)
        if not ok:
            print("Failed to get uptime", file=sys.stderr)
            return 1
        print(msg if msg else "(No output received)")
        return 0

    def _cmd_storage_format(self, args: argparse.Namespace) -> int:
        if not args.no_confirm:
            print(f"WARNING: This will format {args.path} and ALL DATA WILL BE LOST!")
            response = input("Are you sure you want to continue? (yes/no): ")
            if response.lower() != "yes":
                print("Format cancelled by user")
                return 1

        device = self._make_device_from_args(args.host, args.telnet_port, args.timeout)
        ok, msg = device.storage_format(path=args.path, timeout=args.timeout)

        if ok:
            print(f"Successfully formatted {args.path}")
            print(msg)
            return 0
        else:
            print(f"Failed to format {args.path}", file=sys.stderr)
            print(msg, file=sys.stderr)
            return 1

    def _cmd_send_tar(self, args: argparse.Namespace) -> int:
        config = BusyBarConfig(
            http_port=80,
            telnet=TelnetSettings(host=args.host),
        )
        device = BusyBarDevice(config)
        retry_info = f" (retries: {args.retries})" if args.retries > 0 else ""
        print(f"Uploading {args.tar_path} to {args.host}...{retry_info}")
        ok, msg = device.send_tar(
            tar_path=args.tar_path,
            name=args.name,
            timeout=args.timeout,
            retries=args.retries,
            retry_delay=args.retry_delay,
        )
        print(msg)
        return 0 if ok else 1


def main() -> int:
    ops = BusyBarTestOps()
    parser = ops.build_parser()
    args = parser.parse_args()
    try:
        return args.func(args)
    except KeyboardInterrupt:
        print("\nInterrupted by user", file=sys.stderr)
        return 130
    except Exception as e:
        LOG.exception("Unhandled error")
        print(f"Error: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
