"""
End-to-end test for linking a runner to a cloud account.

Flow:
1. Authenticate with cloud (PKCE)
2. Get OTP from runner (POST /api/account/link)
3. Submit OTP to cloud (POST /api/v0/bars/link)
4. Verify link on both sides
5. Cleanup: unlink
"""

import time

import allure
import pytest

from clients.api.account import AccountAPI
from clients.cloud.bars import CloudBarAPI


def _wait_for(predicate, timeout=10, interval=1, description="condition"):
    """Poll until predicate returns truthy, or raise on timeout."""
    deadline = time.time() + timeout
    last_result = None
    while time.time() < deadline:
        last_result = predicate()
        if last_result:
            return last_result
        time.sleep(interval)
    raise AssertionError(f"Timed out after {timeout}s waiting for {description}")


@allure.feature("5. Web Frontend")
@allure.story("Cloud Linking")
class TestCloudLinkFlow:
    """End-to-end test for linking a bar to a cloud account."""

    @allure.title("Full cloud link/verify/unlink flow")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.mqtt
    @pytest.mark.cloud_link
    def test_cloud_link_full_flow(
        self,
        account_api: AccountAPI,
        cloud_bar_api: CloudBarAPI,
    ):
        # Step 1: Wait for MQTT to be connected
        with allure.step("Wait for MQTT connection"):
            deadline = time.time() + 15
            while time.time() < deadline:
                status = account_api.get_status()
                if status.status == "connected":
                    break
                time.sleep(1)
            else:
                pytest.skip(f"MQTT did not connect within 15s (state: {status.status})")

        # Step 2: Cleanup — unlink if already linked
        with allure.step("Ensure device is not already linked"):
            info = account_api.get_info()
            if info.linked:
                account_api.unlink()
                _wait_for(
                    lambda: not account_api.get_info().linked,
                    timeout=10,
                    description="device to unlink",
                )
                # Wait for MQTT to re-establish connected (not linked) state
                _wait_for(
                    lambda: account_api.get_status().status == "connected",
                    timeout=15,
                    description="MQTT to reconnect after unlink",
                )

        # Step 3: Request OTP and submit to cloud (with retry for cloud timing)
        with allure.step("Link device via OTP"):
            cloud_resp = None
            for attempt in range(3):
                with allure.step(f"Attempt {attempt + 1}: request OTP"):
                    link_response = account_api.link()
                    otp_code = link_response.code
                    allure.attach(
                        f"OTP: {otp_code}, expires_at: {link_response.expires_at}",
                        name=f"Link OTP (attempt {attempt + 1})",
                        attachment_type=allure.attachment_type.TEXT,
                    )
                    assert otp_code, "OTP code should not be empty"
                    assert link_response.expires_at > time.time(), "OTP should not be expired"

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

        # Step 5: Verify runner shows linked
        with allure.step("Verify runner reports linked"):
            def _check_linked():
                result = account_api.get_info()
                return result if result.linked else None

            info = _wait_for(
                _check_linked,
                timeout=15,
                description="runner to report linked=true",
            )
            assert info.linked is True
            assert info.email, "Linked account should have email"
            allure.attach(
                f"linked={info.linked}, email={info.email}, user_id={info.user_id}",
                name="Runner Account Info",
                attachment_type=allure.attachment_type.TEXT,
            )

        # Step 6: Verify cloud shows bar in list
        with allure.step("Verify cloud shows bar in list"):
            bar_list = cloud_bar_api.list_bars()
            assert len(bar_list.bars) > 0, "Cloud should have at least one bar linked"
            allure.attach(
                str([{"id": b.id, "label": b.label} for b in bar_list.bars]),
                name="Cloud Bar List",
                attachment_type=allure.attachment_type.TEXT,
            )

        # Step 7: Cleanup — unlink from runner side
        with allure.step("Cleanup: unlink device"):
            account_api.unlink()
            _wait_for(
                lambda: not account_api.get_info().linked,
                timeout=10,
                description="device to unlink after cleanup",
            )
            verify = account_api.get_info()
            assert verify.linked is False, "Device should be unlinked after cleanup"
