from __future__ import annotations

import re
import uuid

import allure
import pytest

from clients.api import AppInfo, AppsAPI, StorageAPI
from utils.js_app_package import (
    APP_AUTHOR,
    APP_DESCRIPTION,
    APP_NAME,
    build_app_package as _build_app_package,
    build_non_app_package as _build_non_app_package,
)
from utils.wait import wait_for, wait_for_stable


APP_ID_PATTERN = re.compile(r"^[a-zA-Z0-9_\-][a-zA-Z0-9_\-.]{0,31}$")


def _find_app(apps: list[AppInfo], app_id: str) -> AppInfo | None:
    return next((app for app in apps if app.id == app_id), None)


def _assert_app_metadata(
    app: AppInfo,
    app_id: str,
    version: str,
) -> None:
    assert app.id == app_id, f"Unexpected app id: {app.id!r}"
    assert app.name == APP_NAME, f"Unexpected app name: {app.name!r}"
    assert app.version == version, (
        f"Unexpected app version: {app.version!r}"
    )
    assert app.author == APP_AUTHOR, (
        f"Unexpected app author: {app.author!r}"
    )
    assert app.description == APP_DESCRIPTION, (
        f"Unexpected app description: {app.description!r}"
    )
    assert app.is_debug is False, (
        f"Unexpected debug flag: {app.is_debug!r}"
    )
    assert app.icon_path.startswith("/ext/"), (
        f"Unexpected icon path: {app.icon_path!r}"
    )


@pytest.fixture
def test_app_id(apps_api: AppsAPI):
    app_id = f"test.{uuid.uuid4().hex[:12]}"
    yield app_id

    response = apps_api.delete_app_raw(app_id)
    assert response.status_code in {200, 404}, (
        f"Failed to clean up {app_id!r}: HTTP {response.status_code}, "
        f"body={response.text[:200]!r}"
    )


@allure.feature("5. Web Frontend")
@allure.story("JavaScript Applications")
@pytest.mark.api
@pytest.mark.frontend
class TestAppsAPI:
    @allure.title(
        "GET /api/apps/list returns valid application metadata"
    )
    def test_list_apps(self, apps_api: AppsAPI):
        response = apps_api.list_apps()

        with allure.step("Validate application IDs and metadata URLs"):
            app_ids = [app.id for app in response.apps]
            assert len(app_ids) == len(set(app_ids)), (
                f"Duplicate application IDs returned: {app_ids!r}"
            )
            for app in response.apps:
                assert APP_ID_PATTERN.fullmatch(app.id), (
                    f"Invalid application ID returned: {app.id!r}"
                )
                assert app.name, f"Empty name returned for {app.id!r}"
                assert app.version, (
                    f"Empty version returned for {app.id!r}"
                )
                assert app.icon_path.startswith("/ext/"), (
                    f"Invalid icon path for {app.id!r}: {app.icon_path!r}"
                )

    @allure.title("Stage, install, list and delete a JavaScript app")
    def test_app_lifecycle(
        self,
        apps_api: AppsAPI,
        storage_api: StorageAPI,
        test_app_id: str,
    ):
        package, main_script = _build_app_package(test_app_id)

        with allure.step("Stage a valid application package"):
            staged = apps_api.stage(package)
            assert staged.result == "OK", (
                f"Unexpected stage result: {staged.result!r}"
            )
            _assert_app_metadata(staged.staged, test_app_id, "1.0.0")
            assert staged.installed is None, (
                f"Fresh app unexpectedly reported as installed: "
                f"{staged.installed!r}"
            )

        with allure.step("Verify staging does not install the app"):
            before_install = apps_api.list_apps()
            assert _find_app(before_install.apps, test_app_id) is None, (
                f"Staged app already appears installed: {before_install!r}"
            )

        with allure.step("Install the app using the returned key"):
            installed_result = apps_api.install(staged.install_key)
            assert installed_result.result == "OK", (
                f"Unexpected install result: {installed_result.result!r}"
            )

        with allure.step("Verify the install key is single-use"):
            repeated = apps_api.install_raw(staged.install_key)
            assert repeated.status_code == 400, (
                f"Reused install key returned HTTP {repeated.status_code}: "
                f"{repeated.text[:200]!r}"
            )

        with allure.step("Verify registry metadata and installed script"):
            listed = apps_api.list_apps()
            installed = _find_app(listed.apps, test_app_id)
            assert installed is not None, (
                f"Installed app is absent from list: {listed!r}"
            )
            _assert_app_metadata(installed, test_app_id, "1.0.0")

            script_response = storage_api.read(
                f"/ext/user_assets/{test_app_id}/scripts/main.js"
            )
            assert script_response.status_code == 200, (
                f"Installed main.js returned HTTP "
                f"{script_response.status_code}: "
                f"{script_response.text[:200]!r}"
            )
            assert script_response.content == main_script, (
                f"Installed main.js differs: {script_response.content!r}"
            )

        with allure.step("Delete the app and verify registry removal"):
            deleted = apps_api.delete_app(test_app_id)
            assert deleted.result == "OK", (
                f"Unexpected delete result: {deleted.result!r}"
            )
            after_delete = apps_api.list_apps()
            assert _find_app(after_delete.apps, test_app_id) is None, (
                f"Deleted app remains in list: {after_delete!r}"
            )
            repeated_delete = apps_api.delete_app_raw(test_app_id)
            assert repeated_delete.status_code == 404, (
                f"Repeated delete returned HTTP "
                f"{repeated_delete.status_code}: "
                f"{repeated_delete.text[:200]!r}"
            )

    @allure.title("An installed application is available to launch")
    @pytest.mark.cli
    def test_installed_app_can_be_launched(
        self,
        apps_api: AppsAPI,
        storage_api: StorageAPI,
        streaming_api,
        persistent_cli_connection,
        test_app_id: str,
    ):
        launch_token = uuid.uuid4().hex
        storage_path = (
            "/ext/apps_data/jsrunner/"
            f"{test_app_id}.localstorage.json"
        )
        script = (
            "localStorage.setItem("
            f"'integration_launch', '{launch_token}'"
            ");\n"
        ).encode("utf-8")
        package, _ = _build_app_package(
            test_app_id,
            main_script=script,
        )

        storage_api.remove_raw(storage_path)
        try:
            with allure.step(
                "Install an application with an observable script"
            ):
                staged = apps_api.stage(package)
                apps_api.install(staged.install_key)

            with allure.step("Open the installed app in the device launcher"):
                initial_frame = streaming_api.front_frame()
                output = persistent_cli_connection.execute_command(
                    f"loader open js_app_launcher {test_app_id}"
                )
                assert "Failed to launch" not in output, (
                    f"Launcher rejected the installed app: {output!r}"
                )

                initial_digest = initial_frame.digest()
                launcher_frame = wait_for_stable(
                    "stable JS app launcher screen",
                    streaming_api.front_frame,
                    lambda frame: frame.digest(),
                    predicate=lambda frame: frame.digest() != initial_digest,
                    stable_samples=3,
                    timeout=5,
                    interval=0.2,
                )
                launcher_frame.attach("Installed JS app launcher")

            with allure.step("Start the application and observe main.js"):
                start_output = persistent_cli_connection.execute_command(
                    "input send InputKeyStart InputTypeShort"
                )
                assert "Usage:" not in start_output, (
                    f"Failed to inject Start selection: {start_output!r}"
                )
                marker_response = wait_for(
                    "installed main.js launch marker",
                    lambda: storage_api.read(storage_path),
                    lambda response: response.status_code == 200,
                    timeout=10,
                    interval=0.2,
                )
                payload = marker_response.json()
                assert payload == {
                    "format_version": 1,
                    "data": {"integration_launch": launch_token},
                }, f"Unexpected launch marker: {payload!r}"
        finally:
            persistent_cli_connection.execute_command("loader kill")
            storage_api.remove_raw(storage_path)

    @allure.title("A plain TAR application package can be installed")
    def test_plain_tar_package(
        self,
        apps_api: AppsAPI,
        test_app_id: str,
    ):
        package, _ = _build_app_package(test_app_id, gzip=False)

        with allure.step("Stage and install a plain TAR package"):
            staged = apps_api.stage(package)
            assert staged.staged.id == test_app_id, (
                f"Unexpected staged app: {staged.staged!r}"
            )
            installed = apps_api.install(staged.install_key)
            assert installed.result == "OK", (
                f"Unexpected install result: {installed.result!r}"
            )

        with allure.step("Verify the TAR application is listed"):
            listed = _find_app(
                apps_api.list_apps().apps,
                test_app_id,
            )
            assert listed is not None, (
                "Application installed from plain TAR is absent"
            )

    @allure.title("Staging an update reports and replaces the installed app")
    def test_replace_installed_app(
        self,
        apps_api: AppsAPI,
        test_app_id: str,
    ):
        version_one, _ = _build_app_package(
            test_app_id,
            version="1.0.0",
        )
        version_two, _ = _build_app_package(
            test_app_id,
            version="2.0.0",
        )

        with allure.step("Install version 1"):
            first_stage = apps_api.stage(version_one)
            apps_api.install(first_stage.install_key)

        with allure.step("Stage version 2 and inspect replacement metadata"):
            second_stage = apps_api.stage(version_two)
            _assert_app_metadata(
                second_stage.staged,
                test_app_id,
                "2.0.0",
            )
            assert second_stage.installed is not None, (
                "Stage response omitted metadata for the installed version"
            )
            _assert_app_metadata(
                second_stage.installed,
                test_app_id,
                "1.0.0",
            )

            before_install = _find_app(
                apps_api.list_apps().apps,
                test_app_id,
            )
            assert before_install is not None, (
                "Installed version disappeared while update was staged"
            )
            assert before_install.version == "1.0.0", (
                f"Staging changed installed version: {before_install!r}"
            )

        with allure.step("Install and verify version 2"):
            apps_api.install(second_stage.install_key)
            after_install = _find_app(
                apps_api.list_apps().apps,
                test_app_id,
            )
            assert after_install is not None, (
                "Updated application is absent from the registry"
            )
            _assert_app_metadata(
                after_install,
                test_app_id,
                "2.0.0",
            )

    @allure.title("Install allows a same or older application version")
    @pytest.mark.parametrize("candidate_version", ["2.0.0", "1.9.9"])
    def test_install_allows_non_newer_version(
        self,
        apps_api: AppsAPI,
        test_app_id: str,
        candidate_version: str,
    ):
        baseline_package, _ = _build_app_package(
            test_app_id,
            version="2.0.0",
        )
        candidate_package, _ = _build_app_package(
            test_app_id,
            version=candidate_version,
        )

        with allure.step("Install the current application version"):
            baseline_stage = apps_api.stage(baseline_package)
            apps_api.install(baseline_stage.install_key)

        with allure.step(
            f"Install non-newer version {candidate_version}"
        ):
            candidate_stage = apps_api.stage(candidate_package)
            response = apps_api.install(candidate_stage.install_key)
            assert response.result == "OK", (
                f"Unexpected install result: {response.result!r}"
            )

        with allure.step("Verify the selected version replaced the app"):
            installed = _find_app(
                apps_api.list_apps().apps,
                test_app_id,
            )
            assert installed is not None, (
                "Application disappeared after replacing its version"
            )
            assert installed.version == candidate_version, (
                f"Installed version is {installed.version!r}, "
                f"expected {candidate_version!r}"
            )

    @allure.title("Invalid update keeps the installed application intact")
    def test_invalid_update_preserves_installed_app(
        self,
        apps_api: AppsAPI,
        storage_api: StorageAPI,
        test_app_id: str,
    ):
        baseline_package, baseline_script = _build_app_package(
            test_app_id,
            version="1.0.0",
        )
        invalid_update, _ = _build_app_package(
            test_app_id,
            version="2.0.0",
            include_main=False,
        )

        with allure.step("Install a valid baseline application"):
            baseline_stage = apps_api.stage(baseline_package)
            apps_api.install(baseline_stage.install_key)

        with allure.step("Reject an invalid update package"):
            response = apps_api.stage_raw(invalid_update)
            assert response.status_code == 400, (
                f"Invalid update returned HTTP {response.status_code}: "
                f"{response.text[:200]!r}"
            )

        with allure.step("Verify the baseline metadata and script remain"):
            installed = _find_app(
                apps_api.list_apps().apps,
                test_app_id,
            )
            assert installed is not None, (
                "Installed application disappeared after invalid update"
            )
            assert installed.version == "1.0.0", (
                f"Installed version changed after invalid update: "
                f"{installed.version!r}"
            )
            script_response = storage_api.read(
                f"/ext/user_assets/{test_app_id}/scripts/main.js"
            )
            assert script_response.status_code == 200, (
                f"Baseline main.js returned HTTP "
                f"{script_response.status_code}: "
                f"{script_response.text[:200]!r}"
            )
            assert script_response.content == baseline_script, (
                "Baseline main.js changed after invalid update: "
                f"{script_response.content!r}"
            )

    @allure.title("Optional manifest fields use documented defaults")
    def test_optional_manifest_defaults(
        self,
        apps_api: AppsAPI,
        test_app_id: str,
    ):
        package, _ = _build_app_package(
            test_app_id,
            include_optional_manifest_fields=False,
        )

        with allure.step("Stage a manifest containing required fields only"):
            staged = apps_api.stage(package)
            assert staged.staged.author == "", (
                f"Unexpected default author: {staged.staged.author!r}"
            )
            assert staged.staged.description == "", (
                "Unexpected default description: "
                f"{staged.staged.description!r}"
            )
            assert staged.staged.is_debug is False, (
                f"Unexpected default debug flag: {staged.staged.is_debug!r}"
            )

        with allure.step("Install and verify the default metadata"):
            apps_api.install(staged.install_key)
            installed = _find_app(
                apps_api.list_apps().apps,
                test_app_id,
            )
            assert installed is not None, (
                "Application with a minimal manifest was not installed"
            )
            assert installed.author == "", (
                f"Unexpected installed author: {installed.author!r}"
            )
            assert installed.description == "", (
                "Unexpected installed description: "
                f"{installed.description!r}"
            )
            assert installed.is_debug is False, (
                f"Unexpected installed debug flag: {installed.is_debug!r}"
            )

    @allure.title("A newer staged package invalidates the previous key")
    def test_new_stage_invalidates_previous_install_key(
        self,
        apps_api: AppsAPI,
    ):
        first_id = f"test.stage.{uuid.uuid4().hex[:12]}"
        second_id = f"test.stage.{uuid.uuid4().hex[:12]}"
        first_package, _ = _build_app_package(first_id)
        second_package, _ = _build_app_package(second_id)

        try:
            with allure.step("Stage two different packages"):
                first_stage = apps_api.stage(first_package)
                second_stage = apps_api.stage(second_package)
                assert first_stage.install_key != second_stage.install_key, (
                    "Consecutive stages returned the same install key: "
                    f"{first_stage.install_key}"
                )

            with allure.step("Reject the superseded key"):
                superseded = apps_api.install_raw(
                    first_stage.install_key
                )
                assert superseded.status_code == 400, (
                    f"Superseded key returned HTTP "
                    f"{superseded.status_code}: "
                    f"{superseded.text[:200]!r}"
                )

            with allure.step("Accept the current key only"):
                apps_api.install(second_stage.install_key)
                app_ids = {
                    app.id for app in apps_api.list_apps().apps
                }
                assert first_id not in app_ids, (
                    f"Superseded app was installed: {app_ids!r}"
                )
                assert second_id in app_ids, (
                    f"Current staged app was not installed: {app_ids!r}"
                )
        finally:
            for app_id in (first_id, second_id):
                cleanup = apps_api.delete_app_raw(app_id)
                assert cleanup.status_code in {200, 404}, (
                    f"Failed to clean up {app_id!r}: "
                    f"HTTP {cleanup.status_code}, "
                    f"body={cleanup.text[:200]!r}"
                )

    @allure.title("POST /api/apps/install validates the installation key")
    @pytest.mark.parametrize(
        "install_key",
        [None, 0, -1, "not-a-number", "123suffix", "4294967296"],
    )
    def test_install_key_validation(
        self,
        apps_api: AppsAPI,
        install_key: int | str | None,
    ):
        response = apps_api.install_raw(install_key)

        assert response.status_code == 400, (
            f"install_key={install_key!r} returned HTTP "
            f"{response.status_code}: {response.text[:200]!r}"
        )

    @allure.title("A wrong key does not invalidate the current staged app")
    def test_wrong_key_preserves_current_install_key(
        self,
        apps_api: AppsAPI,
        test_app_id: str,
    ):
        package, _ = _build_app_package(test_app_id)
        staged = apps_api.stage(package)
        wrong_key = staged.install_key + 1

        with allure.step("Reject a key that does not match the staged app"):
            rejected = apps_api.install_raw(wrong_key)
            assert rejected.status_code == 400, (
                f"Wrong key returned HTTP {rejected.status_code}: "
                f"{rejected.text[:200]!r}"
            )

        with allure.step("Accept the original installation key"):
            installed = apps_api.install(staged.install_key)
            assert installed.result == "OK", (
                f"Original key was not accepted: {installed!r}"
            )
            app_ids = {app.id for app in apps_api.list_apps().apps}
            assert test_app_id in app_ids, (
                f"Installed app is absent from list: {app_ids!r}"
            )

    @allure.title("A failed stage invalidates the previous install key")
    def test_failed_stage_invalidates_previous_key(
        self,
        apps_api: AppsAPI,
    ):
        app_id = f"test.failed.stage.{uuid.uuid4().hex[:7]}"
        package, _ = _build_app_package(app_id)

        with allure.step("Stage a valid application"):
            staged = apps_api.stage(package)

        with allure.step("Start a new stage operation that fails"):
            failed_stage = apps_api.stage_raw(b"invalid replacement archive")
            assert failed_stage.status_code == 400, (
                f"Invalid stage returned HTTP {failed_stage.status_code}: "
                f"{failed_stage.text[:200]!r}"
            )

        with allure.step("Reject the key belonging to cleared staging data"):
            stale_key = apps_api.install_raw(staged.install_key)
            assert stale_key.status_code == 400, (
                f"Stale key returned HTTP {stale_key.status_code}: "
                f"{stale_key.text[:200]!r}"
            )
            app_ids = {app.id for app in apps_api.list_apps().apps}
            assert app_id not in app_ids, (
                f"App from a cleared stage was installed: {app_ids!r}"
            )

    @allure.title("POST /api/apps/stage rejects invalid packages")
    @pytest.mark.parametrize(
        "case",
        [
            "empty",
            "not_archive",
            "no_application",
            "missing_main",
            "mismatched_id",
            "invalid_id",
            # TODO: Re-enable with the separate manifest validation work.
            # "invalid_version",
        ],
    )
    def test_stage_rejects_invalid_package(
        self,
        apps_api: AppsAPI,
        case: str,
    ):
        valid_id = f"test.invalid.{uuid.uuid4().hex[:10]}"
        if case == "empty":
            package = b""
            expected_error_code = None
        elif case == "not_archive":
            package = b"this is not a tar archive"
            expected_error_code = "unpack_error"
        elif case == "no_application":
            package = _build_non_app_package()
            expected_error_code = "manifest_error"
        elif case == "missing_main":
            package, _ = _build_app_package(
                valid_id,
                include_main=False,
            )
            expected_error_code = "manifest_error"
        elif case == "mismatched_id":
            package, _ = _build_app_package(
                valid_id,
                manifest_id=f"other.{uuid.uuid4().hex[:10]}",
            )
            expected_error_code = "manifest_error"
        elif case == "invalid_id":
            package, _ = _build_app_package(".hidden")
            expected_error_code = "manifest_error"
        elif case == "invalid_version":
            package, _ = _build_app_package(
                valid_id,
                version="not-semver",
            )
            expected_error_code = "manifest_error"
        else:
            raise AssertionError(f"Unknown invalid package case: {case!r}")

        response = None
        try:
            response = apps_api.stage_raw(package)

            with allure.step(f"Verify rejection for {case}"):
                assert response.status_code == 400, (
                    f"Invalid package {case!r} returned HTTP "
                    f"{response.status_code}: {response.text[:200]!r}"
                )
                payload = response.json()
                assert payload.get("error"), (
                    f"Invalid package response has no error: {payload!r}"
                )
                if expected_error_code is not None:
                    assert payload.get("error_code") == expected_error_code, (
                        f"Unexpected error code for {case!r}: {payload!r}"
                    )
        finally:
            if response is not None and response.status_code == 200:
                apps_api.stage_raw(b"cleanup invalid archive")

    @allure.title("Manifest requires every mandatory field")
    @pytest.mark.parametrize(
        "missing_field",
        ["format_version", "id", "name", "version"],
    )
    def test_manifest_required_fields(
        self,
        apps_api: AppsAPI,
        missing_field: str,
    ):
        app_id = f"test.required.{uuid.uuid4().hex[:9]}"
        package, _ = _build_app_package(
            app_id,
            missing_manifest_fields=(missing_field,),
        )

        response = apps_api.stage_raw(package)

        assert response.status_code == 400, (
            f"Manifest without {missing_field!r} returned HTTP "
            f"{response.status_code}: {response.text[:200]!r}"
        )
        assert response.json().get("error_code") == "manifest_error", (
            f"Unexpected response for missing {missing_field!r}: "
            f"{response.text[:200]!r}"
        )

    @allure.title("Manifest heap size enforces the documented boundaries")
    @pytest.mark.parametrize(
        ("heap_size_kib", "expected_status"),
        [(1, 200), (512, 200), (0, 400), (513, 400)],
    )
    def test_manifest_heap_size_boundaries(
        self,
        apps_api: AppsAPI,
        heap_size_kib: int,
        expected_status: int,
    ):
        app_id = f"test.heap.{uuid.uuid4().hex[:12]}"
        package, _ = _build_app_package(
            app_id,
            manifest_updates={"heap_size_kib": heap_size_kib},
        )

        try:
            response = apps_api.stage_raw(package)

            assert response.status_code == expected_status, (
                f"heap_size_kib={heap_size_kib} returned HTTP "
                f"{response.status_code}, expected {expected_status}: "
                f"{response.text[:200]!r}"
            )
            if expected_status == 400:
                assert response.json().get("error_code") == (
                    "manifest_error"
                ), f"Unexpected rejection: {response.text[:200]!r}"
        finally:
            # Clear a successfully staged boundary package.
            apps_api.stage_raw(b"cleanup invalid archive")

    @allure.title("POST /api/apps/stage rejects parent path segments")
    def test_stage_rejects_parent_path_segments(
        self,
        apps_api: AppsAPI,
        storage_api: StorageAPI,
    ):
        app_id = f"test.traversal.{uuid.uuid4().hex[:8]}"
        marker_name = f"traversal-{uuid.uuid4().hex[:12]}.txt"
        marker_path = f"/ext/js_app_installer/{marker_name}"
        archive_path = f"{app_id}/../../{marker_name}"
        marker_content = b"integration traversal marker"
        package, _ = _build_app_package(
            app_id,
            extra_files={archive_path: marker_content},
        )

        try:
            with allure.step(
                "Verify the marker does not exist before staging"
            ):
                before = storage_api.read(marker_path)
                assert before.status_code == 400, (
                    f"Traversal marker already exists at {marker_path!r}: "
                    f"HTTP {before.status_code}"
                )

            response = apps_api.stage_raw(package)

            with allure.step("Verify ../../ cannot escape the staging folder"):
                marker = storage_api.read(marker_path)
                issues = []
                if response.status_code != 400:
                    issues.append(
                        f"archive path {archive_path!r} returned HTTP "
                        f"{response.status_code}; body={response.text[:200]!r}"
                    )
                if marker.status_code == 200:
                    issues.append(
                        f"archive created {marker_path!r} outside staging "
                        f"with content {marker.content!r}"
                    )
                elif marker.status_code != 400:
                    issues.append(
                        f"marker lookup returned unexpected HTTP "
                        f"{marker.status_code}: {marker.text[:200]!r}"
                    )
                assert not issues, (
                    "Archive path traversal was not safely rejected:\n- "
                    + "\n- ".join(issues)
                )
        finally:
            storage_api.remove_raw(marker_path)
            apps_api.stage_raw(b"cleanup invalid archive")

    @allure.title("Application IDs enforce the documented 32-char limit")
    def test_app_id_length_boundary(self, apps_api: AppsAPI):
        valid_id = "t" + uuid.uuid4().hex[:31]
        invalid_id = f"{valid_id}x"
        valid_package, _ = _build_app_package(valid_id)
        invalid_package, _ = _build_app_package(invalid_id)

        try:
            with allure.step("Accept a 32-character application ID"):
                accepted = apps_api.stage(valid_package)
                assert accepted.staged.id == valid_id, (
                    f"Unexpected staged ID: {accepted.staged.id!r}"
                )
                apps_api.install(accepted.install_key)

            with allure.step("Reject a 33-character application ID"):
                rejected = apps_api.stage_raw(invalid_package)
                assert rejected.status_code == 400, (
                    f"33-character ID returned HTTP "
                    f"{rejected.status_code}: {rejected.text[:200]!r}"
                )
                assert rejected.json().get("error_code") == (
                    "manifest_error"
                ), f"Unexpected rejection: {rejected.text[:200]!r}"
        finally:
            cleanup = apps_api.delete_app_raw(valid_id)
            assert cleanup.status_code in {200, 404}, (
                f"Failed to clean up {valid_id!r}: "
                f"HTTP {cleanup.status_code}, "
                f"body={cleanup.text[:200]!r}"
            )

    @allure.title("DELETE /api/apps validates IDs and missing applications")
    def test_delete_validation(self, apps_api: AppsAPI):
        cases = (
            (None, 400, "missing id"),
            (".hidden", 404, "leading period"),
            ("bad/id", 404, "path separator"),
            ("x" * 33, 404, "id longer than 32 characters"),
            (
                f"test.absent.{uuid.uuid4().hex[:10]}",
                404,
                "valid missing id",
            ),
        )

        for app_id, expected_status, reason in cases:
            with allure.step(f"Delete rejects {reason}"):
                response = apps_api.delete_app_raw(app_id)
                assert response.status_code == expected_status, (
                    f"DELETE app_id={app_id!r} returned HTTP "
                    f"{response.status_code}, expected {expected_status}: "
                    f"{response.text[:200]!r}"
                )
