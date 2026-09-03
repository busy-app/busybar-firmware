"""
Shared fixtures for frontend integration tests.

Busy-timer state fixtures are defined here (not inside display/conftest.py)
so they are available to all test suites under frontend/, not only to
tests that live in the display/ subdirectory.
"""

from __future__ import annotations

import time
from collections.abc import Callable, Iterator

import allure
import pytest
import requests

from clients.api import MintedAccessToken, SettingsAPI
from utils.busy_timer import (
    WORK_CARD_UUID,
    STATE_SETTLE_S,
    get_snapshot,
    next_timestamp,
    set_snapshot,
    wait_for_snapshot_type,
)


@pytest.fixture
def access_token_factory(
    settings_api: SettingsAPI,
) -> Iterator[Callable[[str], MintedAccessToken]]:
    """Mint tokens and revoke only the tokens created by the current test."""
    created_short_ids: list[str] = []

    def mint(name: str) -> MintedAccessToken:
        token = settings_api.mint_access_token(name)
        created_short_ids.append(token.short_id)
        return token

    yield mint

    with allure.step("Revoke tokens created by the test"):
        for short_id in reversed(created_short_ids):
            response = settings_api.revoke_access_token_raw(short_id)
            assert response.status_code in (200, 404), (
                f"Failed to clean up token {short_id!r}: "
                f"HTTP {response.status_code}, body={response.text!r}"
            )


@pytest.fixture
def busy_state_guard(api_session: requests.Session, web_base_url: str):
    """
    Capture the current busy-timer snapshot before each test and
    restore it afterwards, regardless of whether the test passes or fails.

    Yields the captured original snapshot dict so tests can inspect it.
    """
    original = get_snapshot(api_session, web_base_url)
    yield original
    try:
        restore = dict(original)
        restore["snapshot_timestamp_ms"] = next_timestamp(api_session, web_base_url)
        set_snapshot(api_session, web_base_url, restore)
        # Settle before the autouse device_health_monitor probes /api/version:
        # a snapshot write immediately followed by a probe can trip the device's
        # overload guard (503). Poll instead of a fixed sleep so we wait only as
        # long as the device actually needs.
        original_type = original.get("snapshot", {}).get("type")
        if original_type:
            wait_for_snapshot_type(api_session, web_base_url, original_type)
    except Exception:
        pass  # Best-effort restore; device_health_monitor will recover on failure


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
        "snapshot_timestamp_ms": next_timestamp(api_session, web_base_url),
    }
    set_snapshot(api_session, web_base_url, stopped_body)
    wait_for_snapshot_type(api_session, web_base_url, "NOT_STARTED")
    yield


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
            "card_id": WORK_CARD_UUID,
            "is_paused": False,
            "busy_bar_settings": settings,
        },
        "snapshot_timestamp_ms": next_timestamp(api_session, web_base_url),
    }
    set_snapshot(api_session, web_base_url, active_body)
    wait_for_snapshot_type(api_session, web_base_url, "INFINITE")
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
            "card_id": WORK_CARD_UUID,
            "is_paused": False,
            "busy_bar_settings": settings,
        },
        "snapshot_timestamp_ms": next_timestamp(api_session, web_base_url),
    }
    set_snapshot(api_session, web_base_url, active_body)
    time.sleep(STATE_SETTLE_S)

    # Step 2: pause
    paused_body = {
        "snapshot": {
            "type": "INFINITE",
            "card_id": WORK_CARD_UUID,
            "is_paused": True,
            "busy_bar_settings": settings,
        },
        "snapshot_timestamp_ms": next_timestamp(api_session, web_base_url),
    }
    set_snapshot(api_session, web_base_url, paused_body)
    time.sleep(STATE_SETTLE_S)
    yield
