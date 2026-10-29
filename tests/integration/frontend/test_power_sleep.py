"""USB-powered API checks; entering sleep and physical wakeup need a hardware test."""

import allure
import pytest

from clients.api import PowerAPI, SleepSettings


SLEEP_ENDPOINT = "/api/power/sleep"
USB_CONNECTED_MIN_MV = 4000

pytestmark = [pytest.mark.frontend, pytest.mark.api]


@pytest.fixture
def power_api(api_factory):
    return api_factory(PowerAPI)


@pytest.fixture
def sleep_settings_guard(power_api, system_api):
    status = system_api.get_status()
    if status.power.usb_voltage < USB_CONNECTED_MIN_MV:
        pytest.skip("Sleep settings writes require a stable USB-powered connection")

    original = power_api.get_sleep()
    try:
        power_api.set_sleep(SleepSettings(sleep="never"))
        assert power_api.get_sleep() == SleepSettings(
            sleep="never"
        ), "Failed to disable sleep before testing"
        yield
    finally:
        with allure.step("Restore and verify the original sleep policy"):
            power_api.set_sleep(original)
            restored = power_api.get_sleep()
            assert (
                restored == original
            ), f"Sleep settings were not restored: {restored}, expected {original}"

    final_status = system_api.get_status()
    assert final_status.system.boot_time == status.system.boot_time, (
        f"Device rebooted: {status.system.boot_time} -> "
        f"{final_status.system.boot_time}"
    )


@allure.title("Sleep settings can be read without changing the policy")
def test_get_sleep_settings(power_api):
    first = power_api.get_sleep()
    second = power_api.get_sleep()
    assert second == first, f"GET changed sleep settings: {first} -> {second}"


@allure.title("Sleep policies round-trip while USB prevents deep sleep")
@pytest.mark.parametrize(
    "settings",
    [
        SleepSettings(sleep="never"),
        SleepSettings(sleep="after_holdoff", sleep_holdoff=0),
        SleepSettings(sleep="after_holdoff", sleep_holdoff=30000),
        SleepSettings(sleep="after_holdoff", sleep_holdoff=60000),
        SleepSettings(sleep="after_holdoff", sleep_holdoff=86400000),
    ],
    ids=["never", "zero-holdoff", "30-seconds", "60-seconds", "24-hours"],
)
def test_sleep_settings_round_trip(power_api, sleep_settings_guard, settings):
    with allure.step("Set the policy and verify readback"):
        power_api.set_sleep(settings)
        actual = power_api.get_sleep()
        assert actual == settings, f"Expected {settings}, got {actual}"


@allure.title("Sleep policy updates replace the mode and holdoff")
def test_sleep_settings_transitions(power_api, sleep_settings_guard):
    policies = [
        SleepSettings(sleep="after_holdoff", sleep_holdoff=30000),
        SleepSettings(sleep="after_holdoff", sleep_holdoff=90000),
        SleepSettings(sleep="never"),
        SleepSettings(sleep="after_holdoff", sleep_holdoff=60000),
    ]
    for settings in policies:
        with allure.step(f"Apply and read back {settings}"):
            power_api.set_sleep(settings)
            actual = power_api.get_sleep()
            assert actual == settings, f"Expected {settings}, got {actual}"


@allure.title("Invalid sleep settings are rejected without changing the policy")
@pytest.mark.parametrize(
    "payload",
    [
        {},
        {"sleep": None},
        {"sleep": True},
        {"sleep": 1},
        {"sleep": ""},
        {"sleep": "disabled"},
        {"sleep": "AFTER_HOLDOFF", "sleep_holdoff": 60000},
        {"sleep": "after_holdoff"},
        {"sleep": "after_holdoff", "sleep_holdoff": None},
        {"sleep": "after_holdoff", "sleep_holdoff": "60000"},
        {"sleep": "after_holdoff", "sleep_holdoff": True},
        {"sleep": "after_holdoff", "sleep_holdoff": []},
        {"sleep": "after_holdoff", "sleep_holdoff": {}},
        {"sleep": "after_holdoff", "sleep_holdoff": -1},
    ],
    ids=[
        "missing-mode",
        "null-mode",
        "boolean-mode",
        "numeric-mode",
        "empty-mode",
        "unknown-mode",
        "wrong-case",
        "missing-holdoff",
        "null-holdoff",
        "string-holdoff",
        "boolean-holdoff",
        "array-holdoff",
        "object-holdoff",
        "negative-holdoff",
    ],
)
def test_invalid_sleep_settings(
    power_api, sleep_settings_guard, web_session, web_base_url, payload
):
    power_api.set_sleep(SleepSettings(sleep="after_holdoff", sleep_holdoff=90000))
    before = power_api.get_sleep()
    with allure.step("Reject invalid JSON values and preserve the current policy"):
        response = web_session.post(f"{web_base_url}{SLEEP_ENDPOINT}", json=payload)
        assert (
            response.status_code == 400
        ), f"Expected 400 for {payload}, got {response.status_code}: {response.text}"
        after = power_api.get_sleep()
        assert (
            after == before
        ), f"Rejected request changed settings: {before} -> {after}"


@allure.title("Malformed JSON is rejected without changing sleep settings")
@pytest.mark.parametrize(
    "body",
    [
        b"",
        b"{",
        b'{"sleep":',
        b"null",
        b"[]",
    ],
)
def test_malformed_sleep_settings(
    power_api, sleep_settings_guard, web_session, web_base_url, body
):
    power_api.set_sleep(SleepSettings(sleep="after_holdoff", sleep_holdoff=60000))
    before = power_api.get_sleep()
    response = web_session.post(
        f"{web_base_url}{SLEEP_ENDPOINT}",
        data=body,
        headers={"Content-Type": "application/json"},
    )
    after = power_api.get_sleep()
    assert (response.status_code, after) == (400, before), (
        f"Expected 400 and unchanged settings for {body!r}; "
        f"got {response.status_code}: {response.text}; settings: {before} -> {after}"
    )
