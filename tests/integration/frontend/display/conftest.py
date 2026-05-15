"""
Fixtures for display/canvas priority integration tests.

Busy-timer state fixtures (busy_state_guard, busy_timer_stopped, etc.) are
defined in the parent frontend/conftest.py and are available here automatically.
This file only contains display-specific autouse fixtures.
"""

from __future__ import annotations

import time
import pytest
import requests

from utils.busy_timer import (
    STATE_SETTLE_S as _STATE_SETTLE_S,
    get_snapshot as _get_snapshot,
    next_timestamp as _next_timestamp,
    set_snapshot as _set_snapshot,
)


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
        {
            "User-Agent": "BSB-AutoTest/1.0",
            "Accept": "application/json",
        }
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
# busy_state_guard, busy_timer_stopped, busy_timer_active, and busy_timer_paused
# are defined in the parent frontend/conftest.py and are automatically available
# to all tests in this directory.  No re-declaration needed here.
