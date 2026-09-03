"""
Wi-Fi connect/disconnect regression test for the state publisher.

Lives in its own file so a flaky runner-side network does not gate the
fast (REST-driven) state-publisher tests. Skips cleanly when the test
network credentials are not provisioned in the environment.
"""

from __future__ import annotations

import os

import allure
import pytest

from utils.wifi_helpers import (
    connect_to_test_network_or_fail,
    ensure_disconnected,
    wait_for_wifi_state,
)


pytestmark = [pytest.mark.regression, pytest.mark.api]


@allure.feature("State Publisher")
@allure.story("Published events")
@allure.title("Wi-Fi connect and disconnect transitions publish StateUpdates")
def test_wifi_connect_disconnect_publishes(state_publisher_ws, wifi_api):
    """AC9: Wi-Fi connect and disconnect each emit a ``wifi`` StateUpdate."""
    if not os.getenv("WIFI_SSID") or not os.getenv("WIFI_PASSWORD"):
        pytest.skip("WIFI_SSID / WIFI_PASSWORD env vars not set")

    # Establish a known starting point: disconnected.
    with allure.step("Establish a disconnected Wi-Fi baseline"):
        ensure_disconnected(wifi_api)

    state_publisher_ws.drain()
    with allure.step("Wi-Fi connect to test network"):
        connect_to_test_network_or_fail(wifi_api)

    with allure.step("Wait for the Wi-Fi connected StateUpdate"):
        connected_update = state_publisher_ws.wait_for(
            lambda u: u.WhichOneof("state") == "wifi"
            and u.wifi.WhichOneof("wifi_state") == "active"
            and u.wifi.active.status == 0,
            timeout=20.0,
        )

    with allure.step("Verify the published Wi-Fi connected state"):
        assert connected_update.WhichOneof("state") == "wifi"
        assert connected_update.wifi.WhichOneof("wifi_state") == "active"
        assert connected_update.wifi.active.status == 0

    state_publisher_ws.drain()
    with allure.step("Wi-Fi disconnect"):
        ensure_disconnected(wifi_api)
        wait_for_wifi_state(wifi_api, ["disconnected"], timeout=20)

    with allure.step("Wait for the Wi-Fi disconnected StateUpdate"):
        disconnected_update = state_publisher_ws.wait_for(
            lambda u: u.WhichOneof("state") == "wifi"
            and u.wifi.WhichOneof("wifi_state") == "inactive",
            timeout=5.0,
        )

    with allure.step("Verify the published Wi-Fi disconnected state"):
        assert disconnected_update.WhichOneof("state") == "wifi"
        assert disconnected_update.wifi.WhichOneof("wifi_state") == "inactive"
