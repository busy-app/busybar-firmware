from __future__ import annotations

import allure
import pytest

from clients.api import StorageAPI, TelemetryAPI


TELEMETRY_SETTINGS_PATH = "/ext/apps_data/telemetry/telemetry_settings.json"


@allure.feature("5. Web Frontend")
@allure.story("Telemetry")
@pytest.mark.api
@pytest.mark.frontend
class TestTelemetryAPI:
    @allure.title("GET and PUT /api/telemetry round-trip both consent states")
    def test_telemetry_status_round_trip(
        self,
        telemetry_api: TelemetryAPI,
        telemetry_state_guard,
    ):
        for expected in (False, True):
            with allure.step(f"Set telemetry enabled={expected}"):
                updated = telemetry_api.set_enabled(expected)
                assert updated.enabled is expected, (
                    f"PUT returned enabled={updated.enabled!r}, "
                    f"expected {expected!r}"
                )

            with allure.step("Read the persisted in-memory state back"):
                current = telemetry_api.get_status()
                assert current.enabled is expected, (
                    f"GET returned enabled={current.enabled!r}, "
                    f"expected {expected!r}"
                )

    @allure.title(
        "PUT /api/telemetry rejects invalid JSON without changing consent"
    )
    @pytest.mark.parametrize(
        "request_kwargs",
        [
            pytest.param({"data": b""}, id="empty-body"),
            pytest.param(
                {
                    "data": b'{"enabled":',
                    "headers": {"Content-Type": "application/json"},
                },
                id="malformed-json",
            ),
            pytest.param({"json": []}, id="array"),
            pytest.param({"json": {}}, id="missing-enabled"),
            pytest.param({"json": {"enabled": None}}, id="null-enabled"),
            pytest.param({"json": {"enabled": "true"}}, id="string-enabled"),
            pytest.param({"json": {"enabled": 1}}, id="integer-enabled"),
            pytest.param(
                {"json": {"unexpected": False}},
                id="unknown-property",
            ),
            pytest.param(
                {"json": {"enabled": False, "unexpected": True}},
                id="additional-property",
            ),
        ],
    )
    def test_telemetry_rejects_invalid_body_without_state_change(
        self,
        telemetry_api: TelemetryAPI,
        telemetry_state_guard,
        request_kwargs,
    ):
        before = telemetry_api.get_status().enabled

        with allure.step("Submit an invalid telemetry settings document"):
            response = telemetry_api.set_enabled_raw(**request_kwargs)
            assert response.status_code == 400, (
                f"Expected HTTP 400, got {response.status_code}: "
                f"{response.text[:200]!r}"
            )

        with allure.step("Verify the invalid request did not change consent"):
            after = telemetry_api.get_status().enabled
            assert after is before, (
                f"Telemetry state changed from {before!r} to {after!r} "
                "after rejected PUT"
            )

    @allure.title("Telemetry consent survives a device reboot")
    @pytest.mark.long_running
    def test_telemetry_enabled_state_persists_after_reboot(
        self,
        telemetry_api: TelemetryAPI,
        telemetry_state_guard,
        persistent_cli_connection,
        web_base_url,
    ):
        with allure.step("Enable telemetry"):
            updated = telemetry_api.set_enabled(True)
            assert updated.enabled is True, (
                f"Unexpected PUT response: {updated!r}"
            )

        with allure.step("Reboot the device and wait for its HTTP API"):
            recovered = persistent_cli_connection.reboot_and_wait_for_api(
                web_base_url,
                timeout=90,
            )
            assert recovered, "Device did not come back after CLI reboot"

        with allure.step("Verify telemetry is still enabled"):
            current = telemetry_api.get_status()
            assert current.enabled is True, (
                "Telemetry consent was not persisted: "
                f"enabled={current.enabled!r}"
            )

    @allure.title("Telemetry defaults to opt-out when no setting is stored")
    @pytest.mark.long_running
    def test_telemetry_defaults_to_disabled_without_saved_setting(
        self,
        telemetry_api: TelemetryAPI,
        telemetry_state_guard,
        storage_api: StorageAPI,
        persistent_cli_connection,
        web_base_url,
    ):
        with allure.step("Create and then remove the persisted consent file"):
            telemetry_api.set_enabled(True)
            response = storage_api.remove_raw(TELEMETRY_SETTINGS_PATH)
            assert response.status_code == 200, (
                "Failed to remove telemetry settings: "
                f"HTTP {response.status_code}, body={response.text[:200]!r}"
            )

        with allure.step("Reboot the device and wait for its HTTP API"):
            recovered = persistent_cli_connection.reboot_and_wait_for_api(
                web_base_url,
                timeout=90,
            )
            assert recovered, "Device did not come back after CLI reboot"

        with allure.step("Verify fresh telemetry state is opted out"):
            current = telemetry_api.get_status()
            assert current.enabled is False, (
                "Telemetry must default to disabled without stored consent, "
                f"got enabled={current.enabled!r}"
            )
