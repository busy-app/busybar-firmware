"""
Crash detection utilities for BSB firmware tests.

Monitors device crashes by checking crash_detected.flag in SESSION_LOG_DIR.
"""

import json
import logging
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

import allure

from config.config import Config

logger = logging.getLogger("bsb_automation.crash_detector")

TRACE_COMPLETION_TIMEOUT = 180.0
TRACE_COMPLETION_POLL = 1.0


@dataclass
class CrashInfo:
    """Information about a detected crash."""
    processor: str
    timestamp: str
    trace_file_u5: str
    trace_file_si917: str
    crash_line: str
    trace_status: str

    @classmethod
    def from_dict(cls, data: dict) -> "CrashInfo":
        return cls(
            processor=data.get("processor", "unknown"),
            timestamp=data.get("timestamp", ""),
            trace_file_u5=data.get("trace_file_u5", ""),
            trace_file_si917=data.get("trace_file_si917", ""),
            crash_line=data.get("crash_line", ""),
            trace_status=data.get("trace_status", "unknown"),
        )


class CrashDetector:
    """
    Monitors device crashes by tracking crash_detected.flag.

    Usage in pytest fixture:
        detector = CrashDetector()
        detector.capture_initial_state()
        yield
        crash_info = detector.check_for_crash()
        if crash_info:
            # handle crash (attach report, flash firmware, etc.)
    """

    def __init__(self, crash_flag_path: str = None):
        self.crash_flag_path = Path(crash_flag_path or Config.CRASH_FLAG_PATH)
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

    def _wait_for_trace_completion(self, timeout: float = TRACE_COMPLETION_TIMEOUT) -> Optional[dict]:
        """Poll flag file until trace_status leaves 'pending'/'in_progress'.

        Returns the final flag data, or None if file disappeared.
        Treats missing trace_status ('unknown') as complete for backward compat.
        """
        start_time = time.time()
        while time.time() - start_time < timeout:
            flag_data = self._read_crash_flag()
            if flag_data is None:
                logger.warning("Crash flag file disappeared while waiting for trace")
                return None

            status = flag_data.get("trace_status", "unknown")
            if status not in ("pending", "in_progress"):
                logger.info(f"Trace completed with status: {status}")
                return flag_data

            elapsed = time.time() - start_time
            logger.debug(f"Waiting for trace completion ({elapsed:.0f}s / {timeout:.0f}s)...")
            time.sleep(TRACE_COMPLETION_POLL)

        logger.warning(f"Trace did not complete within {timeout}s, proceeding anyway")
        return self._read_crash_flag()

    def check_for_crash(self) -> Optional[CrashInfo]:
        """
        Check if a crash occurred during the test.

        Device recovery is handled separately by device_health_monitor.

        Returns:
            CrashInfo if crash detected, None otherwise.
        """
        current_state = self._read_crash_flag()
        current_mtime = self._get_file_mtime()

        # No crash file exists
        if current_state is None:
            logger.debug("No crash flag file present after test")
            return None

        crash_detected = False
        with allure.step("Checking for device crash"):
            if self._initial_state is None:
                crash_detected = True
                logger.error("Crash flag file was created during test!")
            elif current_mtime != self._initial_mtime:
                crash_detected = True
                logger.error("Crash flag file was updated during test!")
            elif current_state.get("timestamp") != self._initial_state.get("timestamp"):
                crash_detected = True
                logger.error("Crash timestamp changed during test!")

        if not crash_detected:
            return None

        with allure.step("Waiting for crash trace to complete"):
            final_state = self._wait_for_trace_completion()
            if final_state:
                current_state = final_state

        with allure.step("Processing crash information"):
            crash_info = CrashInfo.from_dict(current_state)
            logger.error(
                f"CRASH DETECTED! Processor: {crash_info.processor}, "
                f"Line: {crash_info.crash_line}"
            )
            self._attach_crash_info(crash_info)
            self._attach_trace_files(crash_info)

            return crash_info

    def _attach_crash_info(self, crash_info: CrashInfo) -> None:
        """Attach crash information to allure report."""
        crash_summary = (
            f"Processor: {crash_info.processor}\n"
            f"Timestamp: {crash_info.timestamp}\n"
            f"Crash Line: {crash_info.crash_line}\n"
            f"Trace Status: {crash_info.trace_status}\n"
            f"Trace File U5: {crash_info.trace_file_u5}\n"
            f"Trace File Si917: {crash_info.trace_file_si917}"
        )
        allure.attach(
            crash_summary,
            name="Crash Summary",
            attachment_type=allure.attachment_type.TEXT,
        )
        logger.info("Attached crash summary to allure report")

    def _attach_trace_files(self, crash_info: CrashInfo) -> None:
        """Attach both trace files to allure report (trace already complete)."""
        trace_files = [
            ("U5", crash_info.trace_file_u5),
            ("Si917", crash_info.trace_file_si917),
        ]

        for proc_name, trace_filename in trace_files:
            if not trace_filename:
                logger.debug(f"No trace file for {proc_name}")
                continue

            trace_path = self.crash_flag_path.parent / trace_filename
            if not trace_path.exists():
                logger.warning(f"Trace file not found: {trace_path}")
                allure.attach(
                    f"Trace file not found: {trace_path}",
                    name=f"Trace File Error ({proc_name})",
                    attachment_type=allure.attachment_type.TEXT,
                )
                continue

            try:
                trace_content = trace_path.read_text()
                allure.attach(
                    trace_content,
                    name=f"Crash Trace {proc_name} ({trace_filename})",
                    attachment_type=allure.attachment_type.TEXT,
                )
                logger.info(f"Attached {proc_name} trace file: {trace_filename}")
            except IOError as e:
                logger.error(f"Failed to read trace file {trace_path}: {e}")
                allure.attach(
                    f"Failed to read trace file: {e}",
                    name=f"Trace File Error ({proc_name})",
                    attachment_type=allure.attachment_type.TEXT,
                )
