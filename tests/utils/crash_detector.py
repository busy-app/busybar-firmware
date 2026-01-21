"""
Crash detection utilities for BSB firmware tests.

Monitors device crashes by checking /tmp/crash_detected.flag.
"""

import json
import logging
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Optional, TYPE_CHECKING

import allure

if TYPE_CHECKING:
    from .device_flasher import DeviceFlasher

logger = logging.getLogger("bsb_automation.crash_detector")

CRASH_FLAG_PATH = "/tmp/crash_detected.flag"
TRACE_FILE_WAIT_TIMEOUT = 15.0
TRACE_FILE_POLL_INTERVAL = 1


@dataclass
class CrashInfo:
    """Information about a detected crash."""
    processor: str
    timestamp: str
    trace_file: str
    crash_line: str

    @classmethod
    def from_dict(cls, data: dict) -> "CrashInfo":
        return cls(
            processor=data.get("processor", "unknown"),
            timestamp=data.get("timestamp", ""),
            trace_file=data.get("trace_file", ""),
            crash_line=data.get("crash_line", ""),
        )


class CrashDetector:
    """
    Monitors device crashes by tracking /tmp/crash_detected.flag.

    Usage in pytest fixture:
        detector = CrashDetector()
        detector.capture_initial_state()
        yield
        crash_info = detector.check_for_crash()
        if crash_info:
            # handle crash (attach report, flash firmware, etc.)
    """

    def __init__(self, crash_flag_path: str = CRASH_FLAG_PATH):
        self.crash_flag_path = Path(crash_flag_path)
        self._initial_state: Optional[dict] = None
        self._initial_mtime: Optional[float] = None

    def _read_crash_flag(self) -> Optional[dict]:
        """Read and parse the crash flag file."""
        if not self.crash_flag_path.exists():
            return None
        try:
            content = self.crash_flag_path.read_text()
            return json.loads(content)
        except (json.JSONDecodeError, IOError) as e:
            logger.warning(f"Failed to read crash flag: {e}")
            return None

    def _get_file_mtime(self) -> Optional[float]:
        """Get modification time of the crash flag file."""
        if not self.crash_flag_path.exists():
            return None
        return self.crash_flag_path.stat().st_mtime

    def capture_initial_state(self) -> None:
        """Capture the initial state of the crash flag file before test runs."""
        self._initial_state = self._read_crash_flag()
        self._initial_mtime = self._get_file_mtime()
        if self._initial_state:
            logger.debug(
                f"Initial crash flag state: processor={self._initial_state.get('processor')}, "
                f"timestamp={self._initial_state.get('timestamp')}"
            )
        else:
            logger.debug("No crash flag file present at test start")

    def check_for_crash(
        self,
        flasher: Optional["DeviceFlasher"] = None,
    ) -> Optional[CrashInfo]:
        """
        Check if a crash occurred during the test.

        Args:
            flasher: Optional DeviceFlasher to use for recovery.

        Returns:
            CrashInfo if crash detected, None otherwise.
        """
        current_state = self._read_crash_flag()
        current_mtime = self._get_file_mtime()

        # No crash file exists
        if current_state is None:
            logger.debug("No crash flag file present after test")
            return None

        # Check if file was created or updated
        crash_detected = False

        with allure.step("Checking for device crash"):
            if self._initial_state is None:
                # File was created during test
                crash_detected = True
                logger.error("Crash flag file was created during test!")
            elif current_mtime != self._initial_mtime:
                # File was updated during test
                crash_detected = True
                logger.error("Crash flag file was updated during test!")
            elif current_state.get("timestamp") != self._initial_state.get("timestamp"):
                # Timestamp changed (extra safety check)
                crash_detected = True
                logger.error("Crash timestamp changed during test!")

        if not crash_detected:
            return None

        with allure.step("Processing crash information"):
            crash_info = CrashInfo.from_dict(current_state)
            logger.error(
                f"CRASH DETECTED! Processor: {crash_info.processor}, "
                f"Line: {crash_info.crash_line}"
            )
            self._attach_crash_info(crash_info)

            self._attach_trace_file(crash_info.trace_file)

            if flasher:
                flasher.reset_and_wait()

            return crash_info

    def _attach_crash_info(self, crash_info: CrashInfo) -> None:
        """Attach crash information to allure report."""
        crash_summary = (
            f"Processor: {crash_info.processor}\n"
            f"Timestamp: {crash_info.timestamp}\n"
            f"Crash Line: {crash_info.crash_line}\n"
            f"Trace File: {crash_info.trace_file}"
        )
        allure.attach(
            crash_summary,
            name="Crash Summary",
            attachment_type=allure.attachment_type.TEXT,
        )
        logger.info("Attached crash summary to allure report")

    def _attach_trace_file(
        self,
        trace_filename: str,
        timeout: float = TRACE_FILE_WAIT_TIMEOUT,
        poll_interval: float = TRACE_FILE_POLL_INTERVAL,
    ) -> None:
        """
        Wait for trace file to appear and attach to allure report.

        Args:
            trace_filename: Name of the trace file in /tmp.
            timeout: Maximum time to wait for trace file (default 15s).
            poll_interval: Time between file existence checks.
        """
        if not trace_filename:
            logger.warning("No trace file specified in crash info")
            return

        trace_path = Path("/tmp") / trace_filename

        start_time = time.time()
        while time.time() - start_time < timeout:
            if trace_path.exists():
                break
            logger.debug(f"Waiting for trace file: {trace_path}")
            time.sleep(poll_interval)
        else:
            logger.warning(f"Trace file not found after {timeout}s: {trace_path}")
            allure.attach(
                f"Trace file not found after waiting {timeout}s: {trace_path}",
                name="Trace File Error",
                attachment_type=allure.attachment_type.TEXT,
            )
            return

        time.sleep(0.2)

        try:
            trace_content = trace_path.read_text()
            allure.attach(
                trace_content,
                name=f"Crash Trace ({trace_filename})",
                attachment_type=allure.attachment_type.TEXT,
            )
            logger.info(f"Attached trace file to allure report: {trace_filename}")
        except IOError as e:
            logger.error(f"Failed to read trace file: {e}")
            allure.attach(
                f"Failed to read trace file: {e}",
                name="Trace File Error",
                attachment_type=allure.attachment_type.TEXT,
            )