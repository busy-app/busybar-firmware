#!/usr/bin/env python3
"""
Debug script to test device reset behavior with detailed logging.

Run with: python debug_reset_test.py

This script will:
1. Reset the device via OpenOCD
2. Check connection status
3. Wait and poll for device availability
4. Repeat to identify issues
"""

import logging
import socket
import subprocess
import sys
import telnetlib
import time
from pathlib import Path

# Add parent to path for imports
sys.path.insert(0, str(Path(__file__).parent))

from config.config import Config

# Setup detailed logging - output only, no file
logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s.%(msecs)03d [%(levelname)s] %(message)s",
    datefmt="%H:%M:%S",
    stream=sys.stdout,
)
logger = logging.getLogger(__name__)


class ResetDebugger:
    def __init__(self):
        self.device_ip = Config.BUSYBAR_IP
        self.firmware_dir = Config.BSB_FIRMWARE_PATH
        self.serial = Config.DAPLINK_U5_ID or "auto"
        self.toolchain_env = Config.TOOLCHAIN_ENV
        self.openocd_interface = Config.OPENOCD_INTERFACE
        self.openocd_target = Config.OPENOCD_TARGET

        logger.info("=" * 60)
        logger.info("Reset Debugger Configuration:")
        logger.info(f"  Device IP: {self.device_ip}")
        logger.info(f"  Firmware dir: {self.firmware_dir}")
        logger.info(f"  DAPLink serial: {self.serial}")
        logger.info(f"  Toolchain env: {self.toolchain_env}")
        logger.info(f"  OpenOCD interface: {self.openocd_interface}")
        logger.info(f"  OpenOCD target: {self.openocd_target}")
        logger.info("=" * 60)

    def check_device_available(self, timeout: float = 2.0) -> bool:
        """Check if device is reachable via TCP connection to port 80."""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(timeout)
            start = time.time()
            result = sock.connect_ex((self.device_ip, 80))
            elapsed = time.time() - start
            sock.close()
            available = result == 0
            logger.debug(
                f"TCP check to {self.device_ip}:80 - "
                f"{'SUCCESS' if available else f'FAILED (code={result})'} "
                f"in {elapsed*1000:.1f}ms"
            )
            return available
        except socket.error as e:
            logger.debug(f"TCP check to {self.device_ip}:80 - EXCEPTION: {e}")
            return False

    def check_ping(self, count: int = 1) -> bool:
        """Check if device responds to ping."""
        try:
            result = subprocess.run(
                ["ping", "-c", str(count), "-W", "2", self.device_ip],
                capture_output=True,
                text=True,
                timeout=10,
            )
            success = result.returncode == 0
            logger.debug(f"Ping to {self.device_ip} - {'SUCCESS' if success else 'FAILED'}")
            if not success and result.stdout:
                logger.debug(f"Ping stdout: {result.stdout.strip()}")
            if result.stderr:
                logger.debug(f"Ping stderr: {result.stderr.strip()}")
            return success
        except Exception as e:
            logger.debug(f"Ping to {self.device_ip} - EXCEPTION: {e}")
            return False

    def check_arp(self) -> str:
        """Check ARP table entry for device."""
        try:
            result = subprocess.run(
                ["ip", "neigh", "show", self.device_ip],
                capture_output=True,
                text=True,
                timeout=5,
            )
            output = result.stdout.strip() or "no entry"
            logger.debug(f"ARP for {self.device_ip}: {output}")
            return output
        except Exception as e:
            logger.debug(f"ARP check failed: {e}")
            return f"error: {e}"

    def connect_cli(self, timeout: float = 10.0) -> tuple[bool, telnetlib.Telnet | None]:
        """Connect to device CLI via telnet."""
        try:
            logger.debug(f"Connecting to CLI at {self.device_ip}:23...")
            tn = telnetlib.Telnet(self.device_ip, 23, timeout=timeout)

            # Wait for prompt
            welcome = tn.read_until(b">: ", timeout=5.0)
            welcome_str = welcome.decode("utf-8", errors="ignore")

            logger.info(f"CLI connected, welcome message: {len(welcome_str)} chars")
            logger.debug(f"Welcome (last 100 chars): {repr(welcome_str[-100:])}")

            return True, tn
        except Exception as e:
            logger.error(f"CLI connection failed: {type(e).__name__}: {e}")
            return False, None

    def get_uptime(self, tn: telnetlib.Telnet) -> tuple[bool, str]:
        """Get device uptime via CLI."""
        try:
            logger.debug("Sending 'uptime' command...")
            tn.write(b"uptime\r\n")

            response = tn.read_until(b">: ", timeout=5.0)
            response_str = response.decode("utf-8", errors="ignore")

            # Parse uptime from response
            # Expected format: "Uptime: X days, HH:MM:SS" or similar
            lines = response_str.strip().split("\n")
            uptime_line = None
            for line in lines:
                line = line.strip()
                if line and line != "uptime" and not line.endswith(">:"):
                    uptime_line = line
                    break

            if uptime_line:
                logger.info(f"Device uptime: {uptime_line}")
                return True, uptime_line
            else:
                logger.warning(f"Could not parse uptime from response: {repr(response_str)}")
                return False, response_str

        except Exception as e:
            logger.error(f"Uptime command failed: {type(e).__name__}: {e}")
            return False, str(e)

    def check_cli_and_uptime(self) -> tuple[bool, str | None]:
        """Connect to CLI and get uptime."""
        logger.info("--- Checking CLI and uptime ---")

        connected, tn = self.connect_cli()
        if not connected or tn is None:
            return False, None

        try:
            success, uptime = self.get_uptime(tn)
            return success, uptime if success else None
        finally:
            try:
                tn.close()
            except Exception:
                pass

    def reset_device(self) -> tuple[bool, str, str]:
        """
        Reset the device via OpenOCD.
        Returns: (success, stdout, stderr)
        """
        logger.info("Executing OpenOCD reset...")

        reset_cmd = (
            f"cd {self.firmware_dir} && "
            f"source {self.toolchain_env} && "
            f"openocd "
            f"-f {self.openocd_interface} "
            f'-c "transport select swd" '
            f'-c "adapter serial {self.serial}" '
            f"-f {self.openocd_target} "
            f'-c "init" -c "reset run" -c "exit"'
        )
        logger.debug(f"Reset command: {reset_cmd}")

        try:
            start = time.time()
            result = subprocess.run(
                reset_cmd,
                shell=True,
                executable="/bin/bash",
                capture_output=True,
                text=True,
                timeout=30,
                cwd=self.firmware_dir,
            )
            elapsed = time.time() - start

            success = "reset run" in result.stderr or result.returncode == 0
            logger.info(
                f"OpenOCD reset {'SUCCESS' if success else 'FAILED'} "
                f"(exit={result.returncode}, took {elapsed:.1f}s)"
            )

            if result.stdout:
                logger.debug(f"OpenOCD stdout:\n{result.stdout}")
            if result.stderr:
                # Log each line for easier reading
                for line in result.stderr.split("\n"):
                    if line.strip():
                        level = logging.INFO if "Info :" in line else logging.DEBUG
                        logger.log(level, f"OpenOCD: {line}")

            return success, result.stdout, result.stderr

        except subprocess.TimeoutExpired:
            logger.error("OpenOCD reset TIMEOUT (30s)")
            return False, "", "timeout"
        except Exception as e:
            logger.error(f"OpenOCD reset EXCEPTION: {e}")
            return False, "", str(e)

    def wait_for_device(
        self,
        timeout: float = 60.0,
        poll_interval: float = 1.0,
    ) -> tuple[bool, float]:
        """
        Wait for device to become available.
        Returns: (success, elapsed_time)
        """
        logger.info(f"Waiting for device (timeout={timeout}s, poll={poll_interval}s)...")
        start = time.time()
        attempt = 0

        while time.time() - start < timeout:
            attempt += 1
            elapsed = time.time() - start

            # Check TCP first (faster)
            tcp_ok = self.check_device_available(timeout=2.0)

            if tcp_ok:
                logger.info(f"Device available after {elapsed:.1f}s (attempt {attempt})")
                return True, elapsed

            # Every 5 attempts, also check ping and ARP for debugging
            if attempt % 5 == 0:
                self.check_ping()
                self.check_arp()

            logger.debug(f"Attempt {attempt}: not available yet ({elapsed:.1f}s elapsed)")
            time.sleep(poll_interval)

        elapsed = time.time() - start
        logger.error(f"Device NOT available after {elapsed:.1f}s ({attempt} attempts)")
        return False, elapsed

    def run_reset_cycle(self, cycle_num: int) -> dict:
        """Run a single reset cycle and return results."""
        logger.info("")
        logger.info("=" * 60)
        logger.info(f"RESET CYCLE {cycle_num}")
        logger.info("=" * 60)

        results = {
            "cycle": cycle_num,
            "pre_check": None,
            "pre_uptime": None,
            "reset_success": None,
            "post_wait_success": None,
            "post_wait_time": None,
            "post_check": None,
            "post_uptime": None,
        }

        # Pre-reset status
        logger.info("--- Pre-reset status ---")
        results["pre_check"] = self.check_device_available()
        self.check_ping()
        self.check_arp()

        # Pre-reset CLI and uptime
        if results["pre_check"]:
            cli_ok, uptime = self.check_cli_and_uptime()
            results["pre_uptime"] = uptime

        # Reset
        logger.info("--- Executing reset ---")
        results["reset_success"], _, _ = self.reset_device()

        # Small delay before polling
        logger.info("--- Waiting 2s before polling ---")
        time.sleep(2)

        # Wait for device
        logger.info("--- Polling for device ---")
        results["post_wait_success"], results["post_wait_time"] = self.wait_for_device(
            timeout=30.0, poll_interval=1.0
        )

        # Post-reset status
        logger.info("--- Post-reset status ---")
        results["post_check"] = self.check_device_available()
        self.check_ping()
        self.check_arp()

        # Post-reset CLI and uptime
        if results["post_check"]:
            cli_ok, uptime = self.check_cli_and_uptime()
            results["post_uptime"] = uptime

        return results

    def run_test(self, num_cycles: int = 10):
        """Run multiple reset cycles and summarize results."""
        logger.info("")
        logger.info("#" * 60)
        logger.info(f"# Starting reset debug test with {num_cycles} cycles")
        logger.info("#" * 60)

        all_results = []

        for i in range(1, num_cycles + 1):
            results = self.run_reset_cycle(i)
            all_results.append(results)

            # Brief pause between cycles
            if i < num_cycles:
                logger.info("--- Waiting 5s before next cycle ---")
                time.sleep(5)

        # Summary
        logger.info("")
        logger.info("#" * 60)
        logger.info("# SUMMARY")
        logger.info("#" * 60)

        for r in all_results:
            status = "OK" if r["post_wait_success"] else "FAILED"
            wait_time = f"{r['post_wait_time']:.1f}s" if r["post_wait_time"] else "N/A"
            logger.info(
                f"Cycle {r['cycle']}: {status} "
                f"(pre={r['pre_check']}, reset={r['reset_success']}, "
                f"wait={wait_time}, post={r['post_check']})"
            )
            if r["pre_uptime"]:
                logger.info(f"  Pre-reset uptime:  {r['pre_uptime']}")
            if r["post_uptime"]:
                logger.info(f"  Post-reset uptime: {r['post_uptime']}")

        success_count = sum(1 for r in all_results if r["post_wait_success"])
        logger.info(f"Total: {success_count}/{num_cycles} cycles successful")

        return all_results


def main():
    debugger = ResetDebugger()
    results = debugger.run_test(num_cycles=30)

    # Exit with error if any cycle failed
    if not all(r["post_wait_success"] for r in results):
        sys.exit(1)


if __name__ == "__main__":
    main()
