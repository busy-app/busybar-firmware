"""Verify next_timestamp() recovers a device whose RTC has been reset to 2000.

Reproduces the CI failure mode: after a reflash/reset the RTC drops to 2000-01-01
while a 2026 snapshot is still stored, so no fresh timestamp is valid until the
clock is fixed. Standalone so it can be run on its own against the bench.
"""

import time
from datetime import datetime

import allure
import pytest

from utils.busy_timer import (
    MIN_SANE_YEAR,
    WORK_CARD_UUID,
    device_now_ms,
    ensure_device_clock_synced,
    next_timestamp,
    set_snapshot,
)


def _device_year(session, base_url):
    resp = session.get(f"{base_url}/api/time", timeout=10)
    resp.raise_for_status()
    return datetime.fromisoformat(resp.json()["timestamp"]).year


@pytest.fixture
def restore_clock(api_session, web_base_url):
    """Guarantee the device is left with a sane clock — a test that skews it to
    2000 and then fails would otherwise poison every later time-sensitive test."""
    yield
    ensure_device_clock_synced(api_session, web_base_url)


@allure.feature("5. Web Frontend")
@allure.story("Busy Timer")
@allure.title("Snapshot timestamp generation recovers a year-2000 device clock")
def test_next_timestamp_recovers_year_2000_rtc(
    api_session, web_base_url, restore_clock
):
    with allure.step("Skew the device clock to the year 2000"):
        # keep the device's real offset, only wind the clock back to 2000
        now = datetime.fromisoformat(
            api_session.get(f"{web_base_url}/api/time", timeout=10).json()["timestamp"]
        )
        skewed = now.replace(
            year=2000, month=1, day=1, hour=0, minute=1, second=0, microsecond=0
        )
        resp = api_session.post(
            f"{web_base_url}/api/time/timestamp",
            params={"timestamp": skewed.isoformat()},
            data=b"",
            timeout=10,
        )
        resp.raise_for_status()
        time.sleep(0.5)
        assert (
            _device_year(api_session, web_base_url) < MIN_SANE_YEAR
        ), "clock was not skewed"

    with allure.step(
        "Generate the next timestamp and verify the clock is resynchronized"
    ):
        # next_timestamp must transparently repair the clock and return a usable value
        ts = next_timestamp(api_session, web_base_url)
        assert (
            _device_year(api_session, web_base_url) >= MIN_SANE_YEAR
        ), "clock was not resynced"

    with allure.step("Submit and accept a snapshot with the recovered timestamp"):
        # and the device must actually accept a snapshot stamped with it
        settings = (
            api_session.get(f"{web_base_url}/api/busy/snapshot", timeout=10)
            .json()
            .get("snapshot", {})
            .get("busy_bar_settings", {})
        )
        set_snapshot(
            api_session,
            web_base_url,
            {
                "snapshot": {
                    "type": "INFINITE",
                    "card_id": WORK_CARD_UUID,
                    "is_paused": False,
                    "busy_bar_settings": settings,
                },
                "snapshot_timestamp_ms": ts,
            },
        )


@allure.feature("5. Web Frontend")
@allure.story("Busy Timer")
@allure.title("Clock synchronization is a no-op when the device clock is already sane")
def test_ensure_clock_noop_when_synced(api_session, web_base_url):
    with allure.step("Establish and read a synchronized device clock"):
        ensure_device_clock_synced(api_session, web_base_url)
        before = device_now_ms(api_session, web_base_url)

    with allure.step("Synchronize again and verify the clock is not moved"):
        ensure_device_clock_synced(
            api_session, web_base_url
        )  # already sane -> no change
        after = device_now_ms(api_session, web_base_url)
        assert abs(after - before) < 5000, "a synced clock must not be moved"
