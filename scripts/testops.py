#!/usr/bin/env python3
"""
BusyBar Operations Tool (class-based commands)

Usage:
  python testops.py wait --host HOST --port PORT -t TIMEOUT
  python testops.py get-version [--host HOST] [--telnet-port PORT] [-t TIMEOUT]
  python testops.py power [COMMAND ...] [--host HOST] [--telnet-port PORT] [-t TIMEOUT]
  python testops.py update-bundle PATH.json [--host HOST] [--telnet-port PORT] [--update-timeout SECONDS]

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
from typing import List, Optional, Tuple

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
            re.compile(rb">:"),       # '>:'
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
            self._logger.debug(f"Connecting to {self.settings.host}:{self.settings.port} (timeout={self.settings.timeout})")
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

    def run(self, command: str, timeout: Optional[float] = None) -> CommandResult:
        if not self._tn:
            raise RuntimeError("Telnet not connected")

        start = time.perf_counter()
        self.sendline(command)
        raw = self._read_until_prompt(timeout=timeout or self.settings.timeout)
        duration = time.perf_counter() - start
        cleaned = self._clean_command_output(raw, command)
        self._logger.debug(f"<< {cleaned.strip()}")
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

class BusyBarDevice:
    """
    High-level device operations executed via a TelnetClient transport.
    """

    _VERSION_PATTERNS: Tuple[re.Pattern, ...] = (
        re.compile(r"Firmware version:\s*([^\n\r]+)", re.IGNORECASE),
        re.compile(r"Version:\s*([^\n\r]+)", re.IGNORECASE),
        re.compile(r"\bv(?P<v>\d+\.\d+\.\d+(?:\.\d+)?)\b", re.IGNORECASE),
    )
    _STATUS_RE = re.compile(r"^\s*Status:\s*([A-Za-z]+)\b", re.IGNORECASE | re.MULTILINE)

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

    def update_bundle(self, bundle_path: str, timeout: int) -> Tuple[bool, str]:
        """
        Runs 'update install <bundle.json>' and waits for prompt again.
        """
        cmd = f"update install {bundle_path}"
        with self._telnet(timeout=timeout) as tn:
            _ = tn.read_welcome()
            self._logger.info(f"Executing: {cmd}")
            res = tn.run(cmd, timeout=timeout)
            return res.ok, (res.stdout or "(Update command sent successfully, no immediate output)")

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
                    return True, "PASSED"
                return False, out

            # Fallback heuristic if explicit status is absent
            if re.search(r"\bPASSED\b", out):
                return True, "PASSED"

            return False, out


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

        # wait
        p_wait = subparsers.add_parser(
            "wait",
            help="Wait for BusyBar to come online",
            description="Wait for BusyBar service to become available",
        )
        p_wait.add_argument(
            "--host",
            default=os.getenv("BUSYBAR_IP", "10.0.4.20"),
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
        p_ver.add_argument("--host", default=os.getenv("BUSYBAR_IP", "10.0.4.20"), help="Device IP/host")
        p_ver.add_argument("--telnet-port", type=int, default=23, help="Telnet port (default: 23)")
        p_ver.add_argument("-t", "--timeout", type=int, default=10, help="Connection timeout in seconds (default: 10)")
        p_ver.set_defaults(func=self._cmd_get_version)

        # power
        p_power = subparsers.add_parser(
            "power",
            help="Execute power-related commands on device",
            description="Execute power commands (info, off, reboot, boot, ch, ch_current, pd_info, pd_set) via telnet",
        )
        p_power.add_argument("power_cmd", nargs="*", help='Power command and arguments (e.g., "reboot", "ch on", "pd_info")')
        p_power.add_argument("--host", default=os.getenv("BUSYBAR_IP", "10.0.4.20"), help="Device IP/host")
        p_power.add_argument("--telnet-port", type=int, default=23, help="Telnet port (default: 23)")
        p_power.add_argument("-t", "--timeout", type=int, default=10, help="Connection timeout in seconds (default: 10)")
        p_power.set_defaults(func=self._cmd_power)

        # update-bundle
        p_upd = subparsers.add_parser(
            "update-bundle",
            help="Update firmware with bundle file",
            description="Execute firmware update with specified bundle file via telnet",
        )
        p_upd.add_argument("bundle_path", help="Path to the bundle.json file")
        p_upd.add_argument("--host", default=os.getenv("BUSYBAR_IP", "10.0.4.20"), help="Device IP/host")
        p_upd.add_argument("--telnet-port", type=int, default=23, help="Telnet port (default: 23)")
        p_upd.add_argument("--update-timeout", type=int, default=120, help="Update timeout in seconds (default: 120)")
        p_upd.set_defaults(func=self._cmd_update_bundle)

        p_tests = subparsers.add_parser(
            "unit-tests",
            help="Run on-device unit_tests and report PASSED or full output",
            description="Runs 'unit_tests' via telnet; prints 'PASSED' if tests passed else prints full output",
            aliases=["unit_tests", "tests"],
        )
        p_tests.add_argument("--host", default=os.getenv("BUSYBAR_IP", "10.0.4.20"), help="Device IP/host")
        p_tests.add_argument("--telnet-port", type=int, default=23, help="Telnet port (default: 23)")
        p_tests.add_argument("-t", "--timeout", type=int, default=120, help="Timeout in seconds (default: 120)")
        p_tests.set_defaults(func=self._cmd_unit_tests)

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

    def _cmd_update_bundle(self, args: argparse.Namespace) -> int:
        device = self._make_device_from_args(args.host, args.telnet_port, args.update_timeout)
        print(f"Executing: update install {args.bundle_path}")
        ok, msg = device.update_bundle(bundle_path=args.bundle_path, timeout=args.update_timeout)
        print(msg if msg else "(Update command sent successfully, no immediate output)")
        return 0 if ok else 1

    def _cmd_unit_tests(self, args: argparse.Namespace) -> int:
        device = self._make_device_from_args(args.host, args.telnet_port, args.timeout)
        passed, output = device.run_unit_tests(timeout=args.timeout)
        if passed:
            print("PASSED")
            return 0
        # Not passed or status unknown: print full output
        print(output if output else "(No output received)")
        return 1


# ----------------------------
# Main
# ----------------------------

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
