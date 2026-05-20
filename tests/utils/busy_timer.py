"""
Shared busy-timer state helpers for integration tests.

Used by both display/conftest.py and frontend/conftest.py so that busy-timer
fixtures are available to all frontend test suites (not just display/).
"""

from __future__ import annotations

import time

import requests

# Sentinel UUID used when creating synthetic work-session snapshots.
WORK_CARD_UUID = "00000000-0000-0000-0000-000000000001"

# How far (ms) ahead of the device's last-known timestamp we stamp new snapshots.
# busy_timer rejects any snapshot whose timestamp <= last_known_snapshot.timestamp_ms.
TS_ADVANCE_MS = 2000

# Time (seconds) to wait after setting a busy timer snapshot so the busy app
# processes the state change and updates the loader priority.  0.3 s was too
# short in CI environments.
STATE_SETTLE_S = 1.0


def get_snapshot(session: requests.Session, base_url: str) -> dict:
    resp = session.get(f"{base_url}/api/busy/snapshot", timeout=10)
    resp.raise_for_status()
    return resp.json()


def next_timestamp(session: requests.Session, base_url: str) -> int:
    """Return a timestamp strictly greater than whatever the device currently holds."""
    current = get_snapshot(session, base_url)
    device_ts = current.get("snapshot_timestamp_ms", 0)
    return max(device_ts, int(time.time() * 1000)) + TS_ADVANCE_MS


def set_snapshot(session: requests.Session, base_url: str, body: dict) -> None:
    resp = session.put(f"{base_url}/api/busy/snapshot", json=body, timeout=10)
    resp.raise_for_status()
