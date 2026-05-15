"""CLI client for the BSB device — wraps a telnet connection and exposes
command execution plus 917-CLI mode helpers."""

import os
import re
import socket
import telnetlib  # TODO: Replace with alternative before Python 3.13 (deprecated)
import time
from typing import Optional

import requests

from utils.logging_config import get_cli_logger, log_cli_command


class SimpleCLIConnection:
    """Simple CLI connection using standard telnetlib"""

    def __init__(self, host: str = None, port: int = None):
        self.host = host or os.getenv("CLI_HOST", "10.0.4.20")
        self.port = int(port or os.getenv("CLI_PORT", "23"))
        self.tn: Optional[telnetlib.Telnet] = None
        self.connected = False
        self.logger = get_cli_logger()
        self._in_sl_cli = False  # For 917 CLI mode tracking

    def connect(self, timeout: float = 10.0) -> bool:
        """Connect to CLI via telnet"""
        try:
            self.logger.info(f"Connecting to {self.host}:{self.port}")

            # Create telnet connection
            self.tn = telnetlib.Telnet(self.host, self.port, timeout=timeout)

            # Read welcome message until we see prompt
            welcome = self.tn.read_until(b">: ", timeout=5.0)
            welcome_str = welcome.decode("utf-8", errors="ignore")

            self.logger.info(f"Connected! Welcome message: {len(welcome_str)} chars")
            self.logger.debug(f"Welcome (last 50 chars): {repr(welcome_str[-50:])}")

            self.connected = True
            return True

        except Exception as e:
            self.logger.error(f"Connection failed: {type(e).__name__}: {e}")
            self.connected = False
            if self.tn:
                try:
                    self.tn.close()
                except:
                    pass
                self.tn = None
            return False

    def execute_command(
        self, command: str, timeout: float = 5.0, slow_command: bool = False
    ) -> str:
        """Execute a command and return its response.

        Auto-recovers from a dead telnet socket: reconnects on entry if the
        previous session was killed externally (e.g. by a `power reboot`),
        retries once on IO error, and probes the socket after success so a
        silently peer-closed connection is surfaced before the next call
        rather than failing on the next write.
        """
        # Increase timeout for slow commands (only if custom timeout not provided)
        if slow_command and timeout == 5.0:  # Default timeout
            timeout = 15.0

        prompt = b"917>: " if self._in_sl_cli else b">: "
        response_str = ""

        for attempt in (1, 2):
            if not self.connected or not self.tn:
                self.logger.info("CLI not connected — (re)connecting")
                if not self.connect():
                    return ""
            try:
                self.logger.debug(f"Executing command: {repr(command)}")
                self.tn.write(f"{command}\r\n".encode("utf-8"))

                if command.strip() == "device_info":
                    response_str = self.tn.read_until(prompt, timeout=5.0).decode(
                        "utf-8", "ignore"
                    )
                    if (
                        "u5_firmware" in response_str
                        and "sl_firmware" not in response_str
                    ):
                        self.logger.debug("Got u5_ fields, waiting for sl_ fields...")
                        response_str += self.tn.read_until(
                            prompt, timeout=max(timeout - 5.0, 0.5)
                        ).decode("utf-8", "ignore")
                else:
                    response_str = self.tn.read_until(prompt, timeout=timeout).decode(
                        "utf-8", "ignore"
                    )
                break
            except (EOFError, OSError) as exc:
                self.logger.warning(
                    f"CLI IO error on {repr(command)} (attempt {attempt}/2): "
                    f"{type(exc).__name__}: {exc}"
                )
                try:
                    self.tn.close()
                except Exception:
                    pass
                self.tn = None
                self.connected = False
                self._in_sl_cli = False
            except Exception as exc:
                self.logger.error(
                    f"Command execution failed: {type(exc).__name__}: {exc}"
                )
                return ""
        else:
            return ""

        self.logger.debug(
            f"Raw response ({len(response_str)} chars): {repr(response_str[:100])}"
        )

        cleaned = self._clean_response(response_str, command)

        if command == "sl_cli":
            clean_response = re.sub(r"\x1b\[[0-9;]*[A-Za-z]", "", response_str)
            clean_response = re.sub(r"\x1b\([A-Z]", "", clean_response)
            clean_response = re.sub(r"\x1b[>=]", "", clean_response)
            if (
                "Welcome to BUSY Bar 917" in clean_response
                or "917 Command Line Interface" in clean_response
            ):
                self._in_sl_cli = True
                self.logger.debug("Entered 917 CLI mode")
            else:
                self.logger.warning(
                    f"sl_cli executed but no welcome message found. Clean response: {repr(clean_response[:200])}"
                )
        elif command == "exit" and self._in_sl_cli:
            self._in_sl_cli = False
            self.logger.debug("Exited 917 CLI mode")

        log_cli_command(command, cleaned, timeout)

        # Peer-closed socket -> reset the flag so the next call reconnects
        # instead of falling into the IO-error retry path.
        if self.tn is not None:
            try:
                self.tn.sock.setblocking(False)
                if self.tn.sock.recv(1, socket.MSG_PEEK) == b"":
                    self.logger.info("CLI peer closed socket; will reconnect next call")
                    self.connected = False
                self.tn.sock.setblocking(True)
            except BlockingIOError:
                self.tn.sock.setblocking(True)
            except OSError:
                self.connected = False

        return cleaned

    def _clean_response(self, response: str, command: str) -> str:
        """Clean response by removing command echo and prompts"""
        if not response:
            return ""

        # Remove ANSI escape sequences
        cleaned = re.sub(r"\x1b\[[0-9;]*[A-Za-z]", "", response)
        cleaned = re.sub(r"\x1b\([A-Z]", "", cleaned)
        cleaned = re.sub(r"\x1b[>=]", "", cleaned)

        # Split into lines
        lines = cleaned.split("\n")
        cleaned_lines = []
        command_seen = False

        for line in lines:
            line = line.strip("\r").strip()

            if not line:
                continue

            # Skip the command echo (only first occurrence)
            if not command_seen and line == command:
                command_seen = True
                continue

            # Skip prompt-only lines
            if line in [
                ">:",
                "917>:",
                ">",
                "917>",
                "busybar>:",
                "busybar>",
            ] or line.endswith(">:"):
                continue

            # Remove prompt if it's at the end of a line with content
            if line.endswith(">:"):
                line = line[:-2].strip()

            if line:  # Only add non-empty lines
                cleaned_lines.append(line)

        return "\n".join(cleaned_lines)

    # 917 CLI helper methods
    def enter_sl_cli(self) -> str:
        """Enter 917 CLI mode"""
        return self.execute_command("sl_cli", slow_command=True)

    def exit_sl_cli(self) -> str:
        """Exit 917 CLI mode"""
        if not self._in_sl_cli:
            raise RuntimeError("Not in 917 CLI mode")
        return self.execute_command("exit")

    def execute_917_command(self, command: str) -> str:
        """Execute command in 917 CLI mode"""
        if not self._in_sl_cli:
            raise RuntimeError("Not in 917 CLI mode")
        return self.execute_command(command, slow_command=True)

    def disconnect(self):
        """Disconnect from CLI"""
        if self.tn:
            self.logger.info("Disconnecting from CLI")
            try:
                # Exit 917 mode if we're in it
                if self._in_sl_cli:
                    try:
                        self.execute_command("exit")
                    except:
                        pass
                self.tn.close()
            except:
                pass
            self.tn = None
        self.connected = False
        self._in_sl_cli = False

    def reboot_and_wait_for_api(self, base_url: str, timeout: float = 60.0) -> bool:
        """Send `power reboot sw` and wait for the HTTP API to come back.

        Sends the command without reading the prompt back (the prompt won't
        return — the device reboots), then closes the telnet socket and
        polls `${base_url}/api/version` until it sees the device drop and
        return. Re-establishes the CLI on success so the caller can keep
        using the same fixture-scoped instance afterwards.
        """
        if not self.connected or not self.tn:
            if not self.connect():
                return False

        self.logger.info("Sending `power reboot sw`...")
        try:
            self.tn.write(b"power reboot sw\r\n")
            time.sleep(0.3)
        except Exception as exc:
            self.logger.error(f"Failed to send reboot command: {exc}")
            return False
        finally:
            try:
                self.tn.close()
            except Exception:
                pass
            self.tn = None
            self.connected = False
            self._in_sl_cli = False

        # Wait for the device to drop and come back. Reuse one local HTTP
        # session without forcing server-side Connection: close; otherwise the
        # firmware can accumulate TCP PCBs while reboot polling.
        t0 = time.monotonic()
        gone = False
        with requests.Session() as session:
            session.headers.update({"User-Agent": "BSB-AutoTest/1.0"})
            while time.monotonic() - t0 < timeout:
                try:
                    with session.get(f"{base_url}/api/version", timeout=2) as response:
                        if gone and response.status_code == 200:
                            self.logger.info(
                                f"API recovered after {time.monotonic() - t0:.1f}s"
                            )
                            # Re-establish the CLI for downstream uses.
                            self.connect()
                            return True
                except requests.RequestException:
                    gone = True
                time.sleep(0.5)
        self.logger.error(f"Device did not come back within {timeout}s")
        return False
