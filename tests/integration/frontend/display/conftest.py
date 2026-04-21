"""
Fixtures for display/canvas priority integration tests.
"""

from __future__ import annotations

import time
import pytest
import requests

# Sentinel UUID used when creating synthetic snapshots that need a card_id
_WORK_CARD_UUID = "00000000-0000-0000-0000-000000000001"

# How far (ms) ahead of the device's last-known timestamp we stamp new snapshots.
# busy_timer rejects any snapshot whose timestamp <= last_known_snapshot.timestamp_ms.
_TS_ADVANCE_MS = 2000

# Time (seconds) to wait after setting a busy timer snapshot so the busy app
# processes the state change and updates the loader priority.  0.3 s was too
# short in CI environments.
_STATE_SETTLE_S = 1.0


def _get_snapshot(session: requests.Session, base_url: str) -> dict:
    resp = session.get(f"{base_url}/api/busy/snapshot", timeout=10)
    resp.raise_for_status()
    return resp.json()


def _next_timestamp(session: requests.Session, base_url: str) -> int:
    """Return a timestamp strictly greater than whatever the device currently holds."""
    current = _get_snapshot(session, base_url)
    device_ts = current.get("snapshot_timestamp_ms", 0)
    return max(device_ts, int(time.time() * 1000)) + _TS_ADVANCE_MS


def _set_snapshot(session: requests.Session, base_url: str, body: dict) -> None:
    resp = session.put(f"{base_url}/api/busy/snapshot", json=body, timeout=10)
    resp.raise_for_status()


# ---------------------------------------------------------------------------
# Session-level teardown: restore busy timer to NOT_STARTED and clear display
# ---------------------------------------------------------------------------


@pytest.fixture(autouse=True, scope="session")
def _busy_session_teardown(web_base_url: str):
    """
    After ALL display/priority tests in this directory have completed,
    restore the busy timer to NOT_STARTED and clear the display canvas.

    Uses its own requests.Session because session-scoped fixtures cannot
    depend on function-scoped ones (like ``api_session``).
    """
    yield
    # --- teardown ---
    sess = requests.Session()
    sess.headers.update(
        {"User-Agent": "BSB-AutoTest/1.0", "Accept": "application/json"}
    )
    try:
        snapshot = _get_snapshot(sess, web_base_url)
        settings = snapshot.get("snapshot", {}).get("busy_bar_settings", {})
        stopped_body = {
            "snapshot": {
                "type": "NOT_STARTED",
                "busy_bar_settings": settings,
            },
            "snapshot_timestamp_ms": _next_timestamp(sess, web_base_url),
        }
        _set_snapshot(sess, web_base_url, stopped_body)
        time.sleep(_STATE_SETTLE_S)
    except Exception:
        pass  # best-effort

    try:
        sess.delete(f"{web_base_url}/api/display/draw", timeout=10)
    except Exception:
        pass  # best-effort
    finally:
        sess.close()


# ---------------------------------------------------------------------------
# Per-test display cleanup
# ---------------------------------------------------------------------------


@pytest.fixture(autouse=True)
def _clear_display_after_test(api_session: requests.Session, web_base_url: str):
    """Clear the display canvas after every test to avoid stale draw state."""
    yield
    try:
        api_session.delete(f"{web_base_url}/api/display/draw", timeout=10)
    except Exception:
        pass


# ---------------------------------------------------------------------------
# Busy timer state fixtures
# ---------------------------------------------------------------------------


@pytest.fixture
def busy_state_guard(api_session: requests.Session, web_base_url: str):
    """
    Capture the current busy-timer snapshot before each test and
    restore it afterwards, regardless of whether the test passes or fails.

    Yields the captured original snapshot dict so tests can inspect it.
    """
    original = _get_snapshot(api_session, web_base_url)
    yield original
    try:
        # Stamp the restore body with a fresh timestamp so the device accepts it.
        restore = dict(original)
        restore["snapshot_timestamp_ms"] = _next_timestamp(api_session, web_base_url)
        _set_snapshot(api_session, web_base_url, restore)
        # Allow the busy app to react and update the loader priority
        time.sleep(_STATE_SETTLE_S)
    except Exception:
        pass  # Best-effort restore; device_health_monitor will recover on failure


@pytest.fixture
def busy_timer_active(
    api_session: requests.Session,
    web_base_url: str,
    busy_state_guard: dict,
):
    """
    Put the busy timer into an INFINITE (active, unpaused) work session.
    The busy app calls loader_set_priority(90) in this state.

    Yields nothing – just ensures the timer is active for the duration
    of the test. The original state is restored by busy_state_guard.
    """
    current = busy_state_guard
    settings = current.get("snapshot", {}).get("busy_bar_settings", {})
    active_body = {
        "snapshot": {
            "type": "INFINITE",
            "card_id": _WORK_CARD_UUID,
            "is_paused": False,
            "busy_bar_settings": settings,
        },
        "snapshot_timestamp_ms": _next_timestamp(api_session, web_base_url),
    }
    _set_snapshot(api_session, web_base_url, active_body)
    time.sleep(_STATE_SETTLE_S)
    yield


@pytest.fixture
def busy_timer_paused(
    api_session: requests.Session,
    web_base_url: str,
    busy_state_guard: dict,
):
    """
    Put the busy timer into a paused INFINITE work session.
    The busy app calls loader_set_priority(LOADER_DEFAULT_APP_PRIORITY) in
    this state (is_paused=True suppresses the active-priority promotion).

    Transition path: whatever → ACTIVE → PAUSED.
    Going through ACTIVE first ensures the busy app is fully running before
    the pause is applied, avoiding the NOT_STARTED → paused race (FW-832)
    where notify_initial_state fires before the scene subscribes to pubsub.

    Yields nothing – the original state is restored by busy_state_guard.
    """
    current = busy_state_guard
    settings = current.get("snapshot", {}).get("busy_bar_settings", {})

    # Step 1: activate — ensures the app is running before we pause
    active_body = {
        "snapshot": {
            "type": "INFINITE",
            "card_id": _WORK_CARD_UUID,
            "is_paused": False,
            "busy_bar_settings": settings,
        },
        "snapshot_timestamp_ms": _next_timestamp(api_session, web_base_url),
    }
    _set_snapshot(api_session, web_base_url, active_body)
    time.sleep(_STATE_SETTLE_S)

    # Step 2: pause
    paused_body = {
        "snapshot": {
            "type": "INFINITE",
            "card_id": _WORK_CARD_UUID,
            "is_paused": True,
            "busy_bar_settings": settings,
        },
        "snapshot_timestamp_ms": _next_timestamp(api_session, web_base_url),
    }
    _set_snapshot(api_session, web_base_url, paused_body)
    time.sleep(_STATE_SETTLE_S)
    yield


@pytest.fixture
def busy_timer_stopped(
    api_session: requests.Session,
    web_base_url: str,
    busy_state_guard: dict,
):
    """
    Put the busy timer into NOT_STARTED state.
    The busy app calls loader_set_priority(LOADER_DEFAULT_APP_PRIORITY=10).

    Yields nothing.
    """
    current = busy_state_guard
    settings = current.get("snapshot", {}).get("busy_bar_settings", {})
    stopped_body = {
        "snapshot": {
            "type": "NOT_STARTED",
            "busy_bar_settings": settings,
        },
        "snapshot_timestamp_ms": _next_timestamp(api_session, web_base_url),
    }
    _set_snapshot(api_session, web_base_url, stopped_body)
    time.sleep(_STATE_SETTLE_S)
    yield
