"""Regressions for the two Wi-Fi failures from the nightly run."""

import os
import unittest
from unittest.mock import Mock, create_autospec, patch

import pytest
import requests

from clients.api import WifiAPI, WifiStatusResponse
from integration.frontend.common import test_api_token_auth_wifi as token_auth_wifi
from integration.frontend.state_publisher import (
    test_state_publisher_wifi_regression as wifi_regression,
)
from utils import wifi_helpers


class WifiHelpersTests(unittest.TestCase):
    def setUp(self):
        self.api = create_autospec(WifiAPI, instance=True)
        self.connected = WifiStatusResponse(
            state="connected",
            ssid="test-network",
            ip_config={"address": "192.0.2.20/24"},
        )

    def test_token_fixture_accepts_connect_race_only_after_link_validation(self):
        self.api.get_status.return_value = WifiStatusResponse(state="disconnected")
        response = requests.Response()
        response.status_code = 400
        response._content = b'{"error":"Already connected"}'
        self.api.connect_to_test_network.return_value = response
        with (
            patch.object(token_auth_wifi, "WIFI_SSID", "test-network"),
            patch.object(
                wifi_helpers, "wait_for_wifi_link_stable", return_value=self.connected
            ) as wait,
        ):
            url = token_auth_wifi.wifi_external_base_url.__wrapped__(
                self.api, "http://10.0.4.20"
            )
        self.assertEqual(url, "http://192.0.2.20")
        self.api.connect_to_test_network.assert_called_once_with(timeout=30)
        wait.assert_called_once()

    def test_other_connect_errors_are_not_accepted(self):
        self.api.get_status.return_value = WifiStatusResponse(state="disconnected")
        response = requests.Response()
        response.status_code = 400
        response._content = b'{"error":"Authentication failed"}'
        self.api.connect_to_test_network.return_value = response
        with patch.object(wifi_helpers, "wait_for_wifi_link_stable") as wait:
            with self.assertRaises(pytest.fail.Exception):
                wifi_helpers.connect_to_test_network_or_fail(self.api)
        wait.assert_not_called()

    def test_existing_connection_keeps_the_fixture_fast_path(self):
        self.api.get_status.return_value = self.connected
        with (
            patch.object(token_auth_wifi, "WIFI_SSID", ""),
            patch.object(token_auth_wifi, "connect_to_test_network_or_fail") as connect,
        ):
            url = token_auth_wifi.wifi_external_base_url.__wrapped__(
                self.api, "http://10.0.4.20"
            )
        self.assertEqual(url, "http://192.0.2.20")
        connect.assert_not_called()

    def test_state_publisher_baseline_waits_before_disconnect(self):
        connected = Mock()
        connected.WhichOneof.return_value = "wifi"
        connected.wifi.WhichOneof.return_value = "active"
        connected.wifi.active.status = 0
        disconnected = Mock()
        disconnected.WhichOneof.return_value = "wifi"
        disconnected.wifi.WhichOneof.return_value = "inactive"
        ws = Mock()
        ws.wait_for.side_effect = [connected, disconnected]
        calls = []

        def wait_for_state(api, states, timeout):
            calls.append(tuple(states))
            return "connected" if "connected" in states else "disconnected"

        with (
            patch.dict(
                os.environ,
                {"WIFI_SSID": "test-network", "WIFI_PASSWORD": "test-password"},
            ),
            patch.object(
                wifi_regression, "wait_for_wifi_state", side_effect=wait_for_state
            ) as wait,
            patch.object(
                wifi_regression,
                "ensure_disconnected",
                side_effect=lambda api: calls.append("disconnect"),
            ),
            patch.object(
                wifi_regression,
                "connect_to_test_network_or_fail",
                side_effect=lambda api: calls.append("connect"),
            ),
        ):
            wifi_regression.test_wifi_connect_disconnect_publishes(ws, self.api)
        self.assertEqual(
            calls,
            [
                ("connected", "disconnected"),
                "disconnect",
                "connect",
                "disconnect",
                ("disconnected",),
            ],
        )
        self.assertEqual(wait.call_args_list[0].kwargs["timeout"], 40.0)
        self.assertEqual(ws.wait_for.call_count, 2)

    def test_stuck_baseline_fails_without_sending_disconnect(self):
        with (
            patch.dict(
                os.environ,
                {"WIFI_SSID": "test-network", "WIFI_PASSWORD": "test-password"},
            ),
            patch.object(
                wifi_regression,
                "wait_for_wifi_state",
                side_effect=AssertionError("State stayed connecting"),
            ),
            patch.object(wifi_regression, "ensure_disconnected") as disconnect,
        ):
            with self.assertRaisesRegex(AssertionError, "State stayed connecting"):
                wifi_regression.test_wifi_connect_disconnect_publishes(Mock(), self.api)
        disconnect.assert_not_called()


if __name__ == "__main__":
    unittest.main()
