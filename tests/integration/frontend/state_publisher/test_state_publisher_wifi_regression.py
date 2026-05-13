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

# Reuse the existing helpers — do not duplicate.
from integration.frontend.wifi.test_api_wifi import (
    connect_to_test_network_or_fail,
    ensure_disconnected,
    wait_for_wifi_state,
)


pytestmark = [pytest.mark.regression, pytest.mark.api]


def test_wifi_connect_disconnect_publishes(state_publisher_ws, wifi_api):
    """AC9: Wi-Fi connect and disconnect each emit a ``wifi`` StateUpdate."""
    if not os.getenv("WIFI_SSID") or not os.getenv("WIFI_PASSWORD"):
        pytest.skip("WIFI_SSID / WIFI_PASSWORD env vars not set")

    # Establish a known starting point: disconnected.
    ensure_disconnected(wifi_api)

    # --- connect ---
    state_publisher_ws.drain()
    with allure.step("Wi-Fi connect to test network"):
        connect_to_test_network_or_fail(wifi_api)

    connected_update = state_publisher_ws.wait_for(
        lambda u: u.WhichOneof("state") == "wifi"
        and u.wifi.WhichOneof("wifi_state") == "connected",
        timeout=20.0,
    )
    assert connected_update.WhichOneof("state") == "wifi"
    assert connected_update.wifi.WhichOneof("wifi_state") == "connected"

    # --- disconnect ---
    state_publisher_ws.drain()
    with allure.step("Wi-Fi disconnect"):
        ensure_disconnected(wifi_api)
        wait_for_wifi_state(wifi_api, ["disconnected"], timeout=20)

    disconnected_update = state_publisher_ws.wait_for(
        lambda u: u.WhichOneof("state") == "wifi"
        and u.wifi.WhichOneof("wifi_state") == "disconnected",
        timeout=5.0,
    )
    assert disconnected_update.WhichOneof("state") == "wifi"
    assert disconnected_update.wifi.WhichOneof("wifi_state") == "disconnected"
