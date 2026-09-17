from __future__ import annotations

from copy import deepcopy
from dataclasses import dataclass
import json
import uuid

import allure
import pytest

from clients.api import AppsAPI, AssetsAPI, StorageAPI
from utils.js_app_package import build_app_package


SETTINGS_VERSION = 7
SETTINGS_SCHEMA = {
    "format_version": 1,
    "version": SETTINGS_VERSION,
    "fields": {
        "enabled": {
            "label": "Enabled",
            "type": "boolean",
            "default": True,
        },
        "level": {
            "label": "Level",
            "type": "integer",
            "default": 4,
            "min": 0,
            "max": 10,
            "step": 2,
        },
        "username": {
            "label": "Username",
            "type": "string",
            "default": "admin",
            "sensitive": True,
            "min_length": 2,
            "max_length": 16,
        },
        "mode": {
            "label": "Mode",
            "type": "enum",
            "default": "fast",
            "options": [
                {"value": "slow", "label": "Slow"},
                {"value": "fast", "label": "Fast"},
                {"value": "turbo", "label": "Turbo"},
            ],
        },
        "accent": {
            "label": "Accent",
            "type": "color",
            "default": "#12345678",
        },
        "alarm": {
            "label": "Alarm",
            "type": "time",
            "default": "09:30:05",
        },
        "location": {
            "label": "Location",
            "type": "geolocation",
            "default": {
                "mode": "fixed",
                "name": "Office",
                "lat": 12.5,
                "lon": -45.25,
            },
        },
        "advanced": {
            "label": "Advanced",
            "type": "group",
            "fields": {
                "retries": {
                    "label": "Retries",
                    "type": "integer",
                    "default": 2,
                    "min": 0,
                    "max": 5,
                },
                "label": {
                    "label": "Label",
                    "type": "string",
                    "default": "primary",
                    "max_length": 16,
                },
            },
        },
    },
}

DEFAULT_VALUES = {
    "enabled": True,
    "level": 4,
    "username": "admin",
    "mode": "fast",
    "accent": "#12345678",
    "alarm": "09:30:05",
    "location": {
        "mode": "fixed",
        "name": "Office",
        "lat": 12.5,
        "lon": -45.25,
    },
    "advanced": {
        "retries": 2,
        "label": "primary",
    },
}

UPDATED_VALUES = {
    "enabled": False,
    "level": 8,
    "username": "secret-token",
    "mode": "turbo",
    "accent": "#ABCDEF80",
    "alarm": "22:10",
    "location": {
        "mode": "fixed",
        "name": "Remote",
        "lat": -20.5,
        "lon": 40.25,
    },
    "advanced": {
        "retries": 5,
        "label": "secondary",
    },
}


@dataclass(frozen=True)
class InstalledSettingsApp:
    app_id: str
    package: bytes
    data_path: str


def _settings_schema_bytes(schema: dict | None = None) -> bytes:
    return json.dumps(
        schema or SETTINGS_SCHEMA,
        separators=(",", ":"),
    ).encode("utf-8")


def _settings_package(
    app_id: str,
    *,
    schema: dict | None = None,
    app_version: str = "1.0.0",
) -> bytes:
    package, _ = build_app_package(
        app_id,
        version=app_version,
        extra_files={
            f"{app_id}/appmeta/settings.json": (
                _settings_schema_bytes(schema)
            )
        },
    )
    return package


def _document(
    values: dict,
    version: int = SETTINGS_VERSION,
) -> dict:
    return {"version": version, "values": deepcopy(values)}


def _install(apps_api: AppsAPI, package: bytes) -> None:
    staged = apps_api.stage(package)
    installed = apps_api.install(staged.install_key)
    assert installed.result == "OK", (
        f"Unexpected install result: {installed.result!r}"
    )


def _assert_storage_document(
    storage_api: StorageAPI,
    path: str,
    expected: dict,
) -> None:
    response = storage_api.read(path)
    assert response.status_code == 200, (
        f"Settings file {path!r} returned HTTP "
        f"{response.status_code}: {response.text[:200]!r}"
    )
    assert response.json() == expected, (
        f"Unexpected persisted settings: {response.text[:500]!r}"
    )


@pytest.fixture
def settings_app(
    apps_api: AppsAPI,
    storage_api: StorageAPI,
) -> InstalledSettingsApp:
    app_id = f"test.settings.{uuid.uuid4().hex[:9]}"
    package = _settings_package(app_id)
    data_path = f"/ext/apps_data/jsrunner/{app_id}.settings.json"

    storage_api.remove_raw(data_path)
    try:
        _install(apps_api, package)
        yield InstalledSettingsApp(app_id, package, data_path)
    finally:
        cleanup = apps_api.delete_app_raw(app_id)
        assert cleanup.status_code in {200, 404}, (
            f"Failed to clean up {app_id!r}: HTTP "
            f"{cleanup.status_code}, body={cleanup.text[:200]!r}"
        )
        storage_api.remove_raw(data_path)


@allure.feature("5. Web Frontend")
@allure.story("JavaScript Application Settings")
@pytest.mark.api
@pytest.mark.frontend
class TestAppSettingsAPI:
    @allure.title("GET materializes complete application defaults")
    def test_get_materializes_defaults(
        self,
        apps_api: AppsAPI,
        storage_api: StorageAPI,
        settings_app: InstalledSettingsApp,
    ):
        with allure.step("Verify settings have not been materialized yet"):
            before = storage_api.read(settings_app.data_path)
            assert before.status_code != 200, (
                f"Settings unexpectedly exist: {before.text[:200]!r}"
            )

        with allure.step("Read defaults through the application API"):
            document = apps_api.get_settings(settings_app.app_id)
            expected = _document(DEFAULT_VALUES)
            assert document.model_dump() == expected

        with allure.step("Verify every default was persisted"):
            _assert_storage_document(
                storage_api,
                settings_app.data_path,
                expected,
            )

    @allure.title("PUT stores, normalizes and returns all setting types")
    def test_put_roundtrip_and_normalization(
        self,
        apps_api: AppsAPI,
        storage_api: StorageAPI,
        settings_app: InstalledSettingsApp,
    ):
        request_values = deepcopy(UPDATED_VALUES)
        request_values["accent"] = "#abcdef80"
        request_values["unknown"] = "drop me"
        request_values["advanced"]["unknown_nested"] = 123

        with allure.step("Store a complete document with unknown fields"):
            result = apps_api.set_settings(
                settings_app.app_id,
                _document(request_values),
            )
            assert result.result == "OK"

        with allure.step("Read the normalized document"):
            expected = _document(UPDATED_VALUES)
            actual = apps_api.get_settings(settings_app.app_id)
            assert actual.model_dump() == expected
            assert actual.values["username"] == "secret-token"
            assert "unknown" not in actual.values
            assert "unknown_nested" not in actual.values["advanced"]

        with allure.step("Verify normalized values are persisted"):
            _assert_storage_document(
                storage_api,
                settings_app.data_path,
                expected,
            )

    @allure.title("Invalid PUT documents are rejected atomically")
    def test_invalid_put_preserves_settings(
        self,
        apps_api: AppsAPI,
        settings_app: InstalledSettingsApp,
    ):
        baseline = _document(UPDATED_VALUES)
        apps_api.set_settings(settings_app.app_id, baseline)

        missing_field = deepcopy(UPDATED_VALUES)
        missing_field.pop("level")

        invalid_level = deepcopy(UPDATED_VALUES)
        invalid_level["level"] = 11

        short_string = deepcopy(UPDATED_VALUES)
        short_string["username"] = "x"

        invalid_enum = deepcopy(UPDATED_VALUES)
        invalid_enum["mode"] = "missing"

        invalid_location = deepcopy(UPDATED_VALUES)
        invalid_location["location"] = {
            "mode": "fixed",
            "name": "Broken",
            "lat": 1.0,
        }

        cases = (
            ("malformed JSON", b"{"),
            ("missing version", {"values": deepcopy(UPDATED_VALUES)}),
            ("missing values", {"version": SETTINGS_VERSION}),
            ("missing field", _document(missing_field)),
            ("older version", _document(UPDATED_VALUES, 6)),
            ("future version", _document(UPDATED_VALUES, 8)),
            ("integer above maximum", _document(invalid_level)),
            ("short sensitive string", _document(short_string)),
            ("unknown enum option", _document(invalid_enum)),
            ("incomplete geolocation", _document(invalid_location)),
        )

        for name, payload in cases:
            with allure.step(f"Reject {name}"):
                if isinstance(payload, bytes):
                    response = apps_api.set_settings_raw(
                        settings_app.app_id,
                        body=payload,
                    )
                else:
                    response = apps_api.set_settings_raw(
                        settings_app.app_id,
                        payload,
                    )
                assert response.status_code == 400, (
                    f"{name} returned HTTP {response.status_code}: "
                    f"{response.text[:200]!r}"
                )
                current = apps_api.get_settings(settings_app.app_id)
                assert current.model_dump() == baseline, (
                    f"{name} changed settings: {current.model_dump()!r}"
                )

    @pytest.mark.skip(reason="Pending agreement on installed app validation semantics")
    @allure.title("Settings endpoints reject uninstalled asset directories")
    def test_settings_reject_uninstalled_asset_directory(
        self,
        apps_api: AppsAPI,
        assets_api: AssetsAPI,
        storage_api: StorageAPI,
    ):
        app_id = f"test.assets.{uuid.uuid4().hex[:10]}"
        data_path = f"/ext/apps_data/jsrunner/{app_id}.settings.json"

        try:
            with allure.step("Create a settings schema without installing an app"):
                assets_api.upload_asset(
                    app_id,
                    "appmeta/settings.json",
                    _settings_schema_bytes(),
                )

            with allure.step("Reject settings operations for the asset directory"):
                responses = {
                    "GET": apps_api.get_settings_raw(app_id),
                    "PUT": apps_api.set_settings_raw(
                        app_id,
                        _document(UPDATED_VALUES),
                    ),
                    "DELETE": apps_api.reset_settings_raw(app_id),
                }
                for method, response in responses.items():
                    assert response.status_code == 404, (
                        f"{method} returned HTTP {response.status_code}: "
                        f"{response.text[:200]!r}"
                    )
        finally:
            assets_api.delete_assets(app_id)
            storage_api.remove_raw(data_path)

    @allure.title("DELETE resets every setting to its schema default")
    def test_delete_resets_defaults(
        self,
        apps_api: AppsAPI,
        storage_api: StorageAPI,
        settings_app: InstalledSettingsApp,
    ):
        apps_api.set_settings(
            settings_app.app_id,
            _document(UPDATED_VALUES),
        )

        with allure.step("Reset application settings"):
            result = apps_api.reset_settings(settings_app.app_id)
            assert result.result == "OK"

        with allure.step("Verify defaults through API and storage"):
            expected = _document(DEFAULT_VALUES)
            actual = apps_api.get_settings(settings_app.app_id)
            assert actual.model_dump() == expected
            _assert_storage_document(
                storage_api,
                settings_app.data_path,
                expected,
            )

    @allure.title("GET heals incomplete and corrupted stored settings")
    def test_get_heals_stored_document(
        self,
        apps_api: AppsAPI,
        storage_api: StorageAPI,
        settings_app: InstalledSettingsApp,
    ):
        apps_api.get_settings(settings_app.app_id)
        partial = {
            "version": SETTINGS_VERSION,
            "values": {
                "enabled": False,
                "level": 11,
            },
        }
        write = storage_api.write(
            settings_app.data_path,
            json.dumps(partial).encode("utf-8"),
        )
        assert write.status_code == 200

        with allure.step("Heal missing and invalid fields"):
            expected_values = deepcopy(DEFAULT_VALUES)
            expected_values["enabled"] = False
            expected = _document(expected_values)
            healed = apps_api.get_settings(settings_app.app_id)
            assert healed.model_dump() == expected
            _assert_storage_document(
                storage_api,
                settings_app.data_path,
                expected,
            )

        with allure.step("Recover defaults from malformed storage"):
            write = storage_api.write(settings_app.data_path, b"{")
            assert write.status_code == 200
            expected = _document(DEFAULT_VALUES)
            healed = apps_api.get_settings(settings_app.app_id)
            assert healed.model_dump() == expected
            _assert_storage_document(
                storage_api,
                settings_app.data_path,
                expected,
            )

    @allure.title(
        "Settings survive reinstall and reset when schema version changes"
    )
    def test_reinstall_and_schema_version_change(
        self,
        apps_api: AppsAPI,
        settings_app: InstalledSettingsApp,
    ):
        apps_api.set_settings(
            settings_app.app_id,
            _document(UPDATED_VALUES),
        )

        with allure.step("Delete and reinstall the same application"):
            apps_api.delete_app(settings_app.app_id)
            absent = apps_api.get_settings_raw(settings_app.app_id)
            assert absent.status_code == 404
            _install(apps_api, settings_app.package)
            preserved = apps_api.get_settings(settings_app.app_id)
            assert preserved.model_dump() == _document(UPDATED_VALUES)

        with allure.step("Install an application with a newer schema"):
            schema_v8 = deepcopy(SETTINGS_SCHEMA)
            schema_v8["version"] = 8
            schema_v8["fields"]["enabled"]["default"] = False
            schema_v8["fields"]["level"]["default"] = 6
            package_v8 = _settings_package(
                settings_app.app_id,
                schema=schema_v8,
                app_version="2.0.0",
            )
            _install(apps_api, package_v8)

        with allure.step("Verify the old document was reset"):
            defaults_v8 = deepcopy(DEFAULT_VALUES)
            defaults_v8["enabled"] = False
            defaults_v8["level"] = 6
            reset = apps_api.get_settings(settings_app.app_id)
            assert reset.model_dump() == _document(defaults_v8, 8)

    @allure.title("Settings endpoints report identifier and schema errors")
    def test_settings_errors(
        self,
        apps_api: AppsAPI,
        storage_api: StorageAPI,
    ):
        invalid_ids = (None, ".hidden", "bad/id", "x" * 33)
        for app_id in invalid_ids:
            with allure.step(f"Reject app_id={app_id!r}"):
                response = apps_api.get_settings_raw(app_id)
                assert response.status_code == 400, (
                    f"app_id={app_id!r} returned HTTP "
                    f"{response.status_code}: {response.text[:200]!r}"
                )

        with allure.step("Report a missing application"):
            missing_id = f"test.absent.{uuid.uuid4().hex[:10]}"
            response = apps_api.get_settings_raw(missing_id)
            assert response.status_code == 404

        app_id = f"test.schema.{uuid.uuid4().hex[:11]}"
        data_path = f"/ext/apps_data/jsrunner/{app_id}.settings.json"
        try:
            with allure.step("Report an application without settings"):
                package, _ = build_app_package(app_id)
                _install(apps_api, package)
                response = apps_api.get_settings_raw(app_id)
                assert response.status_code == 404

            with allure.step("Report an invalid settings schema"):
                invalid_package, _ = build_app_package(
                    app_id,
                    version="2.0.0",
                    extra_files={
                        f"{app_id}/appmeta/settings.json": b"{",
                    },
                )
                _install(apps_api, invalid_package)
                response = apps_api.get_settings_raw(app_id)
                assert response.status_code == 503
        finally:
            cleanup = apps_api.delete_app_raw(app_id)
            assert cleanup.status_code in {200, 404}
            storage_api.remove_raw(data_path)
