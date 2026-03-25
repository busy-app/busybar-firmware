"""
Test cloud link flow through password-protected API via public (Wi-Fi) IP.

Verifies that:
1. Access protection (key mode) blocks unauthenticated requests via public IP
2. X-API-Token header grants access through public IP
3. Full cloud link flow works through the protected public API
"""

import time

import allure
import pytest
import requests

from clients.api.account import AccountAPI
from clients.api.settings import SettingsAPI
from clients.api.wifi import WifiAPI
from clients.cloud.bars import CloudBarAPI

TEST_ACCESS_KEY = "5678"


def _wait_for(predicate, timeout=10, interval=1, description="condition"):
    """Poll until predicate returns truthy, or raise on timeout."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        result = predicate()
        if result:
            return result
        time.sleep(interval)
    raise AssertionError(f"Timed out after {timeout}s waiting for {description}")


@pytest.fixture
def public_ip(wifi_api: WifiAPI) -> str:
    """Get the device's Wi-Fi IP address, skip if not connected."""
    status = wifi_api.get_status()
    if status.state != "connected":
        pytest.skip(f"Wi-Fi not connected (state: {status.state})")
    assert status.ip_config and status.ip_config.address, "No IP address in wifi status"
    return status.ip_config.address


@pytest.fixture
def protected_access(settings_api: SettingsAPI):
    """Enable key-based access protection, restore original mode after test."""
    original = settings_api.get_access()

    settings_api.set_access("key", TEST_ACCESS_KEY)

    yield TEST_ACCESS_KEY

    # Restore original mode (via USB — always has access)
    if original.mode == "key":
        settings_api.set_access("key", TEST_ACCESS_KEY)
    else:
        settings_api.set_access(original.mode)


@pytest.fixture
def public_account_api(public_ip: str, protected_access: str) -> AccountAPI:
    """AccountAPI client targeting public IP with X-API-Token."""
    session = requests.Session()
    session.headers.update({
        "User-Agent": "BSB-AutoTest/1.0",
        "Accept": "application/json",
        "X-API-Token": protected_access,
    })
    return AccountAPI(session, f"http://{public_ip}")


@allure.feature("5. Web Frontend")
@allure.story("Cloud Linking")
class TestProtectedApiAccess:
    """Test API access protection via public IP."""

    @allure.title("Public IP rejects requests without token in key mode")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.cloud_link
    def test_public_ip_rejects_without_token(
        self,
        public_ip: str,
        protected_access: str,
    ):
        resp = requests.get(
            f"http://{public_ip}/api/account/status",
            timeout=5,
        )
        assert resp.status_code == 403, f"Expected 403, got {resp.status_code}"

    @allure.title("Public IP allows requests with valid token in key mode")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.cloud_link
    def test_public_ip_allows_with_token(
        self,
        public_ip: str,
        protected_access: str,
    ):
        resp = requests.get(
            f"http://{public_ip}/api/account/status",
            headers={"X-API-Token": protected_access},
            timeout=5,
        )
        assert resp.status_code == 200, f"Expected 200, got {resp.status_code}"

    @allure.title("Public IP rejects requests with wrong token")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.cloud_link
    def test_public_ip_rejects_wrong_token(
        self,
        public_ip: str,
        protected_access: str,
    ):
        resp = requests.get(
            f"http://{public_ip}/api/account/status",
            headers={"X-API-Token": "0000"},
            timeout=5,
        )
        assert resp.status_code == 403, f"Expected 403, got {resp.status_code}"

    @allure.title("Whitelisted endpoints accessible without token")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.cloud_link
    def test_whitelisted_endpoints_accessible(
        self,
        public_ip: str,
        protected_access: str,
    ):
        for endpoint in ["/api/version", "/api/access"]:
            with allure.step(f"GET {endpoint}"):
                resp = requests.get(f"http://{public_ip}{endpoint}", timeout=5)
                assert resp.status_code == 200, (
                    f"{endpoint}: expected 200, got {resp.status_code}"
                )


@allure.feature("5. Web Frontend")
@allure.story("Cloud Linking")
class TestCloudLinkProtected:
    """Cloud link flow through protected public API."""

    @allure.title("Cloud link/verify/unlink via protected public IP")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.mqtt
    @pytest.mark.cloud_link
    def test_cloud_link_via_protected_api(
        self,
        public_account_api: AccountAPI,
        cloud_bar_api: CloudBarAPI,
    ):
        # Step 1: Wait for MQTT
        with allure.step("Wait for MQTT connection"):
            deadline = time.time() + 15
            while time.time() < deadline:
                status = public_account_api.get_status()
                if status.status == "connected":
                    break
                time.sleep(1)
            else:
                pytest.skip(f"MQTT did not connect within 15s (state: {status.status})")

        # Step 2: Unlink if needed
        with allure.step("Ensure device is not already linked"):
            info = public_account_api.get_info()
            if info.linked:
                public_account_api.unlink()
                _wait_for(
                    lambda: not public_account_api.get_info().linked,
                    timeout=10,
                    description="device to unlink",
                )
                _wait_for(
                    lambda: public_account_api.get_status().status == "connected",
                    timeout=15,
                    description="MQTT to reconnect after unlink",
                )

        # Step 3: Request OTP and submit to cloud (with retry for cloud timing)
        with allure.step("Link device via OTP"):
            cloud_resp = None
            for attempt in range(3):
                with allure.step(f"Attempt {attempt + 1}: request OTP"):
                    link_response = public_account_api.link()
                    otp_code = link_response.code
                    allure.attach(
                        f"OTP: {otp_code}, expires_at: {link_response.expires_at}",
                        name=f"Link OTP (attempt {attempt + 1})",
                        attachment_type=allure.attachment_type.TEXT,
                    )

                with allure.step(f"Submit OTP '{otp_code}' to cloud"):
                    cloud_resp = cloud_bar_api.link_bar(otp_code)
                    if cloud_resp.status_code == 204:
                        break
                    if cloud_resp.status_code == 422 and attempt < 2:
                        time.sleep(3)
                        continue
                    assert cloud_resp.status_code == 204, (
                        f"Expected 204, got {cloud_resp.status_code}: {cloud_resp.text}"
                    )

        # Step 5: Verify link via protected public API
        with allure.step("Verify runner reports linked via public API"):
            def _check_linked():
                result = public_account_api.get_info()
                return result if result.linked else None

            info = _wait_for(
                _check_linked,
                timeout=15,
                description="runner to report linked=true",
            )
            assert info.linked is True
            assert info.email, "Linked account should have email"

        # Step 6: Verify cloud side
        with allure.step("Verify cloud shows bar in list"):
            bar_list = cloud_bar_api.list_bars()
            assert len(bar_list.bars) > 0, "Cloud should have at least one bar linked"

        # Step 7: Cleanup
        with allure.step("Cleanup: unlink device"):
            public_account_api.unlink()
            _wait_for(
                lambda: not public_account_api.get_info().linked,
                timeout=10,
                description="device to unlink after cleanup",
            )
