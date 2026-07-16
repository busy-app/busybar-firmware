"""
Shared busy-timer state helpers for integration tests.

Used by both display/conftest.py and frontend/conftest.py so that busy-timer
fixtures are available to all frontend test suites (not just display/).
"""

from __future__ import annotations

import time
from datetime import datetime

import requests

# Sentinel UUID used when creating synthetic work-session snapshots.
WORK_CARD_UUID = "00000000-0000-0000-0000-000000000001"

# How far (ms) ahead of the device's last-known timestamp we stamp new snapshots.
# busy_timer rejects any snapshot whose timestamp <= last_known_snapshot.timestamp_ms.
TS_ADVANCE_MS = 2000
TS_MIN_ADVANCE_MS = 1

# Firmware rejects snapshots more than 60 seconds ahead of RTC.
TS_MAX_FUTURE_MS = 60 * 1000
TS_FUTURE_WAIT_TIMEOUT_S = 20.0
TS_FUTURE_POLL_INTERVAL_S = 0.1

# A reflash (or a mid-run reset) leaves the RTC at 2000-01-01 until NTP re-syncs.
# Any year below this means the clock is unset, not merely lagging.
MIN_SANE_YEAR = 2020

# Time (seconds) to wait after setting a busy timer snapshot so the busy app
# processes the state change and updates the loader priority.  0.3 s was too
# short in CI environments.
STATE_SETTLE_S = 1.0


def get_snapshot(session: requests.Session, base_url: str) -> dict:
    resp = session.get(f"{base_url}/api/busy/snapshot", timeout=10)
    resp.raise_for_status()
    return resp.json()


def device_now_ms(session: requests.Session, base_url: str) -> int:
    """Return the device's current RTC time in ms (GET /api/time).

    The busy_timer future-window check compares against the DEVICE RTC, not
    the test host clock. Right after a reflash the RTC may lag the runner by
    more than the 60 s tolerance until NTP syncs, so host time is unusable.
    """
    resp = session.get(f"{base_url}/api/time", timeout=10)
    resp.raise_for_status()
    return int(datetime.fromisoformat(resp.json()["timestamp"]).timestamp() * 1000)


def ensure_device_clock_synced(session: requests.Session, base_url: str) -> None:
    """Set the device clock to host time if its RTC is unset (year < 2020).

    After a reflash — or after a mid-run reset, e.g. the web server being wedged
    by the schema-conformance fuzzer — the RTC drops back to 2000-01-01 and only
    NTP re-syncs it. Until then a busy snapshot cannot be timestamped: the stored
    snapshot sits decades ahead of the RTC, so no value is both newer than it and
    within the 60 s future window, and next_timestamp() would just time out.

    The device timezone is stored separately from the clock and survives the
    reset, so GET /api/time still reports the right offset. Stamp host time *in
    that offset* — the firmware sets the RTC from the wall-clock part, so matching
    the offset keeps the resulting epoch aligned with the runner's.
    """
    resp = session.get(f"{base_url}/api/time", timeout=10)
    resp.raise_for_status()
    device_dt = datetime.fromisoformat(resp.json()["timestamp"])
    if device_dt.year >= MIN_SANE_YEAR:
        return

    host_local = datetime.now(device_dt.tzinfo).replace(microsecond=0)
    resp = session.post(
        f"{base_url}/api/time/timestamp",
        params={"timestamp": host_local.isoformat()},
        data=b"",
        timeout=10,
    )
    resp.raise_for_status()


def next_timestamp(session: requests.Session, base_url: str) -> int:
    """Return a valid timestamp strictly greater than the current device snapshot.

    The firmware rejects snapshots from more than 60 seconds ahead of its own
    RTC. If the current device snapshot is already close to that limit,
    advance it by the smallest possible amount instead of pushing the test
    state past the accepted future window.
    """
    ensure_device_clock_synced(session, base_url)
    deadline = time.monotonic() + TS_FUTURE_WAIT_TIMEOUT_S

    while True:
        current = get_snapshot(session, base_url)
        device_ts = current.get("snapshot_timestamp_ms", 0)
        now_ms = device_now_ms(session, base_url)
        future_limit_ms = now_ms + TS_MAX_FUTURE_MS

        candidate = max(device_ts + TS_MIN_ADVANCE_MS, now_ms + TS_ADVANCE_MS)
        if candidate <= future_limit_ms:
            return candidate

        if time.monotonic() >= deadline:
            raise AssertionError(
                "Cannot build a fresh BUSY snapshot timestamp within the "
                f"{TS_MAX_FUTURE_MS}ms future limit: "
                f"device_ts={device_ts}, now_ms={now_ms}"
            )

        time.sleep(TS_FUTURE_POLL_INTERVAL_S)


def set_snapshot(session: requests.Session, base_url: str, body: dict) -> None:
    resp = session.put(f"{base_url}/api/busy/snapshot", json=body, timeout=10)
    if not resp.ok:
        # Surface the device's error body — a bare 400 is undiagnosable.
        raise requests.HTTPError(
            f"{resp.status_code} for PUT /api/busy/snapshot: {resp.text}",
            response=resp,
        )


# After the device reports the new snapshot type, wait a short residual for the
# busy app to finish propagating the loader priority. Polling the snapshot can
# observe the new type slightly before that effect lands, so a small fixed tail
# is still needed — but far less than the old blanket STATE_SETTLE_S.
STATE_PROPAGATE_S = 0.3
STATE_POLL_INTERVAL_S = 0.1


def wait_for_snapshot_type(
    session: requests.Session,
    base_url: str,
    expected_type: str,
    *,
    timeout: float = STATE_SETTLE_S * 3,
    propagate: float = STATE_PROPAGATE_S,
) -> None:
    """Block until the busy snapshot reports ``expected_type``.

    Drop-in replacement for ``time.sleep(STATE_SETTLE_S)`` after ``set_snapshot``:
    settles as fast as the device actually applies the state instead of always
    waiting a full second, then adds a small residual for the loader-priority
    effect to land. Best-effort — returns after ``timeout`` regardless, matching
    the old fixed-sleep semantics.
    """
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            current = get_snapshot(session, base_url)
        except requests.RequestException:
            time.sleep(STATE_POLL_INTERVAL_S)
            continue
        if current.get("snapshot", {}).get("type") == expected_type:
            break
        time.sleep(STATE_POLL_INTERVAL_S)
    if propagate:
        time.sleep(propagate)
