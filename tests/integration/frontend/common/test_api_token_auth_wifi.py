"""WiFi-specific access-token behavior kept separate from local API tests."""

from __future__ import annotations

from collections.abc import Callable, Iterator
from urllib.parse import urlsplit
from uuid import uuid4

import allure
import pytest
import requests

from clients.api import WIFI_SSID, MintedAccessToken, SettingsAPI, WifiAPI
from utils.wait import wait_for


_INITIAL_ACCESS_KEY = "73916482"
_CHANGED_ACCESS_KEY = "28461739"
_TOKEN_HEADER = "X-API-Token"


def _headers(token: str) -> dict[str, str]:
    return {_TOKEN_HEADER: token}


@pytest.fixture
def wifi_external_base_url(
    wifi_api: WifiAPI,
    web_base_url: str,
) -> str:
    """Return the device's external WiFi URL, connecting if configured."""
    with allure.step("Resolve the device WiFi address"):
        status = wifi_api.get_status()
        if status.state != "connected":
            if not WIFI_SSID:
                pytest.skip(
                    "WiFi authentication tests require an existing connection or "
                    "the WIFI_SSID test configuration"
                )
            response = wifi_api.connect_to_test_network(timeout=30)
            assert response.status_code == 200, (
                f"WiFi connect failed with HTTP {response.status_code}: "
                f"{response.text!r}"
            )
            status = wait_for(
                "WiFi connection with an assigned address",
                wifi_api.get_status,
                lambda value: (
                    value.state == "connected"
                    and value.ip_config is not None
                    and bool(value.ip_config.address)
                ),
                timeout=30,
                interval=1,
            )

        ip_config = status.ip_config
        assert ip_config is not None and ip_config.address, (
            "WiFi is connected without an IP address: "
            f"state={status.state!r}, ip_config={ip_config!r}"
        )
        address = ip_config.address.split("/", 1)[0]
        assert (
            address and address != "0.0.0.0"
        ), f"WiFi reported an unusable address: {ip_config.address!r}"

        local_host = urlsplit(web_base_url).hostname
        assert address != local_host and not address.startswith("127."), (
            "Expected an external WiFi address, got "
            f"address={address!r}, WEB_BASE_URL={web_base_url!r}"
        )
    return f"http://{address}"


@pytest.fixture
def wifi_token_api(
    settings_api: SettingsAPI,
    web_session: requests.Session,
    wifi_external_base_url: str,
) -> Iterator[SettingsAPI]:
    """Configure key mode and expose a client using the WiFi address."""
    original_access = settings_api.get_access()
    with allure.step("Configure key access mode for WiFi"):
        settings_api.set_access("key", _INITIAL_ACCESS_KEY)

    yield SettingsAPI(web_session, wifi_external_base_url)

    with allure.step("Restore original access mode"):
        if original_access.mode == "key":
            settings_api.set_access("enabled")
        else:
            settings_api.set_access(original_access.mode)


@allure.feature("5. Web Frontend")
@allure.story("API token authentication over WiFi")
@pytest.mark.api
@pytest.mark.frontend
@pytest.mark.uses_si917
class TestAPITokenAuthenticationWiFi:
    @allure.title("Key mode requires credentials and accepts generated token")
    def test_key_mode_authentication_flow(
        self,
        settings_api: SettingsAPI,
        wifi_token_api: SettingsAPI,
    ):
        with allure.step("Reject a protected WiFi request without credentials"):
            missing = wifi_token_api.get_raw("/api/status")
            assert missing.status_code == 403, (
                f"Expected missing WiFi credentials to return HTTP 403, got "
                f"{missing.status_code}: {missing.text!r}"
            )

        minted: MintedAccessToken | None = None
        try:
            with allure.step("Mint a token over WiFi using the numeric key"):
                minted = wifi_token_api.mint_access_token(
                    f"wifi-key-{uuid4().hex[:10]}",
                    headers=_headers(_INITIAL_ACCESS_KEY),
                )
            with allure.step("Authorize a WiFi request using the generated token"):
                authorized = wifi_token_api.get_raw(
                    "/api/status",
                    headers=_headers(minted.token),
                )
                assert authorized.status_code == 200, (
                    f"Expected generated token to return HTTP 200 over WiFi, got "
                    f"{authorized.status_code}: {authorized.text!r}"
                )
        finally:
            if minted is not None:
                settings_api.revoke_access_token_raw(minted.short_id)

    @allure.title("Token is inactive when access mode is disabled")
    def test_token_is_inactive_in_disabled_mode(
        self,
        settings_api: SettingsAPI,
        wifi_token_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
    ):
        with allure.step("Create a token and disable access"):
            token = access_token_factory(f"wifi-disabled-{uuid4().hex[:10]}")
            settings_api.set_access("disabled")

        with allure.step("Verify the token cannot authorize a WiFi request"):
            response = wifi_token_api.get_raw(
                "/api/status",
                headers=_headers(token.token),
            )
            assert response.status_code == 403, (
                f"Expected token to be inactive in disabled mode, got HTTP "
                f"{response.status_code}: {response.text!r}"
            )

    @allure.title("Enabled access mode allows anonymous WiFi requests")
    def test_enabled_mode_allows_anonymous_access(
        self,
        settings_api: SettingsAPI,
        wifi_token_api: SettingsAPI,
    ):
        with allure.step("Enable anonymous access"):
            settings_api.set_access("enabled")

        with allure.step("Verify a WiFi request succeeds without credentials"):
            response = wifi_token_api.get_raw("/api/status")
            assert response.status_code == 200, (
                f"Expected anonymous WiFi access in enabled mode, got HTTP "
                f"{response.status_code}: {response.text!r}"
            )

    @allure.title("Token remains valid after the numeric key changes")
    def test_token_survives_key_change(
        self,
        settings_api: SettingsAPI,
        wifi_token_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
    ):
        with allure.step("Create a token under the initial numeric key"):
            token = access_token_factory(f"wifi-key-change-{uuid4().hex[:10]}")

        with allure.step("Verify the token works before the key change"):
            before_change = wifi_token_api.get_raw(
                "/api/status",
                headers=_headers(token.token),
            )
            assert before_change.status_code == 200, (
                f"Expected token to work in key mode, got HTTP "
                f"{before_change.status_code}: {before_change.text!r}"
            )

        with allure.step("Change the numeric key"):
            settings_api.set_access("key", _CHANGED_ACCESS_KEY)

        with allure.step("Verify the existing token still works"):
            after_change = wifi_token_api.get_raw(
                "/api/status",
                headers=_headers(token.token),
            )
            assert after_change.status_code == 200, (
                f"Expected token to survive key change, got HTTP "
                f"{after_change.status_code}: {after_change.text!r}"
            )
