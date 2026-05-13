"""
Crash detection utilities for BSB firmware tests.

Monitors device crashes by checking crash_detected.flag in SESSION_LOG_DIR.
"""

import hashlib
import json
import logging
import os
import re
import tempfile
import time
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import Optional
from uuid import uuid4

import allure

from config.config import Config

logger = logging.getLogger("bsb_automation.crash_detector")

TRACE_COMPLETION_TIMEOUT = 180.0
TRACE_COMPLETION_POLL = 1.0
FORCE_TRACE_REQUEST_FILENAME = "force_trace_request.flag"
FORCED_TRACE_TIMEOUT = 180.0
FORCED_TRACE_POLL = 1.0
FORCED_TRACE_S3_WAIT = 30.0
FORCED_TRACE_S3_POLL = 0.5
_TRACE_PENDING_STATUSES = {"pending", "in_progress"}


def _session_log_dir() -> Optional[Path]:
    log_dir = Config.SESSION_LOG_DIR or os.getenv("SESSION_LOG_DIR", "")
    return Path(log_dir) if log_dir else None


def _crash_flag_path() -> Path:
    if os.getenv("CRASH_FLAG_PATH"):
        return Path(os.environ["CRASH_FLAG_PATH"])
    log_dir = _session_log_dir()
    if log_dir:
        return log_dir / "crash_detected.flag"
    return Path(Config.CRASH_FLAG_PATH)


def _read_json_file(path: Path) -> Optional[dict]:
    if not path.exists():
        return None
    try:
        return json.loads(path.read_text())
    except (json.JSONDecodeError, OSError) as exc:
        logger.warning("Failed to read %s: %s", path, exc)
        return None


def _looks_like_legacy_crash_flag(data: dict) -> bool:
    return bool(
        data.get("processor")
        and (
            data.get("crash_line")
            or data.get("trace_file_u5")
            or data.get("trace_file_si917")
            or data.get("trace_results")
        )
    )


def request_forced_trace(
    reason: str,
    test_name: str,
    primary_processor: str = "u5",
    timeout: float = FORCED_TRACE_TIMEOUT,
) -> Optional[dict]:
    """Ask serial_logger to collect a GDB trace and wait for completion."""
    log_dir = _session_log_dir()
    if not log_dir:
        logger.warning("SESSION_LOG_DIR is unset; forced GDB trace not requested")
        return None
    if not log_dir.is_dir():
        logger.warning("SESSION_LOG_DIR does not exist: %s", log_dir)
        return None

    processor = (primary_processor or "u5").lower()
    if processor not in {"u5", "si917"}:
        logger.warning(
            "Unknown forced trace processor %r; defaulting to u5",
            primary_processor,
        )
        processor = "u5"

    request_id = f"force-{uuid4().hex}"
    request_path = log_dir / FORCE_TRACE_REQUEST_FILENAME
    crash_flag_path = _crash_flag_path()
    request_data = {
        "request_id": request_id,
        "reason": reason,
        "source": "pytest_device_health_monitor",
        "test_name": test_name,
        "timestamp": datetime.now().isoformat(),
        "primary_processor": processor,
    }

    tmp_path = None
    try:
        fd, tmp_name = tempfile.mkstemp(
            dir=log_dir,
            prefix=f".{FORCE_TRACE_REQUEST_FILENAME}.",
            suffix=".tmp",
        )
        tmp_path = Path(tmp_name)
        with os.fdopen(fd, "w") as tmp_file:
            json.dump(request_data, tmp_file)
        os.replace(tmp_path, request_path)
    except OSError as exc:
        logger.warning("Failed to write forced trace request %s: %s", request_path, exc)
        if tmp_path and tmp_path.exists():
            try:
                tmp_path.unlink()
            except OSError:
                pass
        return None

    logger.warning(
        "Requested forced GDB trace: request_id=%s processor=%s reason=%s",
        request_id,
        processor,
        reason,
    )

    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        flag_data = _read_json_file(crash_flag_path)
        if flag_data and flag_data.get("request_id") == request_id:
            status = flag_data.get("trace_status", "unknown")
            if status not in _TRACE_PENDING_STATUSES:
                flag_data.setdefault("primary_processor", processor)
                flag_data.setdefault("reason", reason)
                logger.info(
                    "Forced GDB trace completed: request_id=%s status=%s",
                    request_id,
                    status,
                )
                _wait_for_forced_trace_s3_urls(request_id, flag_data, log_dir)
                return flag_data
            logger.debug(
                "Waiting for forced GDB trace completion: request_id=%s status=%s",
                request_id,
                status,
            )
        time.sleep(FORCED_TRACE_POLL)

    logger.warning(
        "Forced GDB trace timed out after %.0fs: request_id=%s processor=%s reason=%s",
        timeout,
        request_id,
        processor,
        reason,
    )
    return None


def _forced_trace_flag_path(request_id: str, log_dir: Optional[Path] = None) -> Optional[Path]:
    base = log_dir or _session_log_dir()
    if not base:
        return None
    return base / "forced_traces" / f"{request_id}.flag"


def _wait_for_forced_trace_s3_urls(
    request_id: str,
    flag_data: dict,
    log_dir: Optional[Path] = None,
    timeout: float = FORCED_TRACE_S3_WAIT,
) -> None:
    """Best-effort: merge `s3_urls` into `flag_data` once notification_service
    has uploaded the trace artifacts. The wait is bounded — if S3 is disabled
    or the upload takes too long, we leave `s3_urls` absent and let the caller
    render the comment without trace links."""
    flag_path = _forced_trace_flag_path(request_id, log_dir=log_dir)
    if not flag_path:
        return

    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        per_event = _read_json_file(flag_path)
        if per_event and per_event.get("s3_urls"):
            flag_data["s3_urls"] = per_event["s3_urls"]
            return
        time.sleep(FORCED_TRACE_S3_POLL)

    logger.info(
        "No s3_urls for forced trace within %.0fs: request_id=%s flag=%s",
        timeout,
        request_id,
        flag_path,
    )


def serial_crash_recently_handled(lookback_seconds: float = 60.0) -> bool:
    """Return true when serial_logger recently handled a natural crash trace."""
    crash_flag_path = _crash_flag_path()
    try:
        mtime = crash_flag_path.stat().st_mtime
    except OSError:
        return False

    if time.time() - mtime > lookback_seconds:
        return False

    flag_data = _read_json_file(crash_flag_path)
    if not flag_data:
        return False

    event_type = flag_data.get("event_type")
    if event_type == "forced_trace":
        return False
    if event_type == "crash":
        return True
    if event_type:
        return False
    return _looks_like_legacy_crash_flag(flag_data)


def resolve_forced_trace_processor(node) -> str:
    """Return the primary processor to trace for a pytest node."""
    if node.get_closest_marker("uses_si917"):
        return "si917"
    return "u5"


def record_forced_trace(node, trace: Optional[dict], phase: str) -> None:
    """Store a forced trace result on a pytest node without overwriting prior traces."""
    if not trace:
        return
    forced_traces = getattr(node, "_forced_traces", None)
    if forced_traces is None:
        forced_traces = []
        node._forced_traces = forced_traces

    trace_record = dict(trace)
    trace_record.setdefault("pytest_phase", phase)
    forced_traces.append(trace_record)
    node._forced_trace = trace_record
    _write_forced_trace_sidecar(node, trace_record, phase)


def _sanitize_nodeid_for_filename(nodeid: str) -> str:
    raw = nodeid or "unknown"
    cleaned = re.sub(r"[^\w.\-]+", "_", raw).strip("_") or "unknown"
    if len(cleaned) <= 180:
        return cleaned
    digest = hashlib.sha1(raw.encode()).hexdigest()[:10]
    return f"{cleaned[:170]}_{digest}"


def _junit_id_from_nodeid(nodeid: str) -> str:
    """Mirror pytest's JUnit `classname::name` format so report_test_results.py
    can match failed-test names against forced-trace sidecars."""
    if not nodeid or "::" not in nodeid:
        return nodeid
    path_part, _, rest = nodeid.partition("::")
    if path_part.endswith(".py"):
        path_part = path_part[:-3]
    classname_path = path_part.replace("\\", "/").replace("/", ".")
    pieces = rest.split("::")
    name = pieces[-1]
    extra = ".".join(pieces[:-1])
    classname = f"{classname_path}.{extra}" if extra else classname_path
    return f"{classname}::{name}"


def _write_forced_trace_sidecar(node, trace: dict, phase: str) -> None:
    """Append a thin record to a per-nodeid sidecar so report_test_results.py
    can resolve failed-test names to forced-trace request_ids (and from there,
    to per-request flag files containing `s3_urls`)."""
    log_dir = _session_log_dir()
    if not log_dir:
        return
    nodeid = getattr(node, "nodeid", "") or ""
    if not nodeid:
        return

    sidecar_dir = log_dir / "forced_traces"
    try:
        sidecar_dir.mkdir(parents=True, exist_ok=True)
    except OSError as exc:
        logger.warning("Failed to create forced_traces dir %s: %s", sidecar_dir, exc)
        return

    sidecar_path = sidecar_dir / f"{_sanitize_nodeid_for_filename(nodeid)}.sidecar.json"

    record = {
        "nodeid": nodeid,
        "junit_id": _junit_id_from_nodeid(nodeid),
        "phase": phase,
        "request_id": trace.get("request_id"),
        "processor": _forced_trace_processor(trace),
        "trace_status": trace.get("trace_status"),
        "reason": trace.get("reason"),
        "timestamp": trace.get("timestamp") or datetime.now().isoformat(),
    }

    existing: list = []
    if sidecar_path.exists():
        try:
            loaded = json.loads(sidecar_path.read_text())
            if isinstance(loaded, list):
                existing = loaded
            elif isinstance(loaded, dict):
                existing = [loaded]
        except (OSError, json.JSONDecodeError) as exc:
            logger.warning("Discarding unreadable sidecar %s: %s", sidecar_path, exc)

    existing.append(record)
    try:
        sidecar_path.write_text(json.dumps(existing, indent=2, sort_keys=True))
    except OSError as exc:
        logger.warning("Failed to write forced trace sidecar %s: %s", sidecar_path, exc)


def request_forced_trace_for_node(node, reason: str, phase: str) -> Optional[dict]:
    """Request and record a forced trace for a pytest node when no real crash handled it."""
    if serial_crash_recently_handled():
        return None

    trace = request_forced_trace(
        reason=reason,
        test_name=node.nodeid,
        primary_processor=resolve_forced_trace_processor(node),
    )
    record_forced_trace(node, trace, phase=phase)
    return trace


def _forced_trace_processor(trace: dict) -> str:
    processor = (
        trace.get("processor")
        or trace.get("primary_processor")
        or trace.get("requested_processor")
    )
    if processor:
        return str(processor)

    trace_results = trace.get("trace_results") or {}
    if len(trace_results) == 1:
        return next(iter(trace_results))
    return "unknown"


def _forced_trace_path(rel_path: str) -> Optional[str]:
    if not rel_path:
        return None
    if os.path.isabs(rel_path):
        return rel_path
    log_dir = Config.SESSION_LOG_DIR or os.getenv("SESSION_LOG_DIR", "")
    if not log_dir:
        return None
    return os.path.join(log_dir, rel_path)


def _iter_forced_trace_files(trace: dict):
    seen = set()
    trace_results = trace.get("trace_results") or {}
    for proc, entry in trace_results.items():
        if not isinstance(entry, dict):
            continue
        rel_path = entry.get("file")
        if rel_path and rel_path not in seen:
            seen.add(rel_path)
            yield str(proc), rel_path

    for proc, key in (("u5", "trace_file_u5"), ("si917", "trace_file_si917")):
        rel_path = trace.get(key)
        if rel_path and rel_path not in seen:
            seen.add(rel_path)
            yield proc, rel_path


def attach_forced_traces_to_allure(item, report, append_longrepr) -> None:
    """Attach recorded forced trace results during setup/teardown reports."""
    forced_traces = getattr(item, "_forced_traces", [])
    if not forced_traces or report.when not in ("setup", "teardown"):
        return

    attached = getattr(item, "_forced_trace_attached_ids", set())
    item._forced_trace_attached_ids = attached

    for forced_trace in forced_traces:
        phase = forced_trace.get("pytest_phase", "teardown")
        if phase != report.when and report.when != "teardown":
            continue

        request_id = forced_trace.get("request_id") or f"{phase}:{id(forced_trace)}"
        if request_id in attached:
            continue
        attached.add(request_id)

        processor = _forced_trace_processor(forced_trace)
        status = forced_trace.get("trace_status", "unknown")
        reason = forced_trace.get("crash_line") or forced_trace.get("reason", "")

        allure.dynamic.tag("FORCED_TRACE")
        allure.dynamic.tag(f"forced_trace:{processor}")
        allure.attach(
            json.dumps(forced_trace, indent=2, sort_keys=True),
            name=f"Forced Trace Summary ({phase}, {processor})",
            attachment_type=allure.attachment_type.JSON,
        )

        for proc, rel_path in _iter_forced_trace_files(forced_trace):
            path = _forced_trace_path(rel_path)
            if path and os.path.exists(path):
                allure.attach.file(
                    path,
                    name=f"GDB trace ({proc})",
                    extension="log",
                )

        if report.failed or report.when == "teardown":
            append_longrepr(
                "FORCED TRACE collected: "
                f"phase={phase}, processor={processor}, status={status}, "
                f"request_id={request_id}, reason={reason}"
            )


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

        if current_state.get("event_type") == "forced_trace":
            logger.info(
                "Ignoring forced_trace flag update in crash detector: request_id=%s",
                current_state.get("request_id"),
            )
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
