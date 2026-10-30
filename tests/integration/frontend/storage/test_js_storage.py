"""Storage sandbox checks executed by installed JavaScript applications."""

from __future__ import annotations

import json
from textwrap import dedent
import uuid

import allure
import pytest

from clients.api import AppsAPI, StorageAPI
from utils.js_app_package import build_app_package
from utils.wait import wait_for


SANDBOX_ROOT = "/ext/apps_data/js_apps"
JS_FETCH_HEADERS = {"X-busybar-fetch": "js"}
RESULT_KEY = "storage_test_result"
pytestmark = [pytest.mark.api, pytest.mark.frontend]


def _storage_script(body: str) -> bytes:
    helpers = """
        function assert(condition, message) {
            if (!condition) throw new Error(message);
        }

        async function storage(method, operation, path, expected, body, newPath) {
            let url = "http://127.0.0.1/api/storage/" + operation;
            if (path !== null) url += "?path=" + encodeURIComponent(path);
            if (newPath !== undefined) {
                url += "&new_path=" + encodeURIComponent(newPath);
            }
            const options = {method: method};
            if (body !== undefined) options.body = body;
            try {
                const response = await fetch(url, options);
                const text = await response.text();
                assert(response.status === expected,
                    "HTTP " + response.status + " " + text);
                return text;
            } catch (error) {
                throw new Error(method + " " + url + ": " + error);
            }
        }
    """
    return (
        dedent(helpers)
        + "\nasync function runStorageTest() {\n"
        + dedent(body)
        + "\n}\n"
        + "runStorageTest().then(function() {\n"
        + f"    localStorage.setItem({json.dumps(RESULT_KEY)}, 'PASS');\n"
        + "}).catch(function(error) {\n"
        + f"    localStorage.setItem({json.dumps(RESULT_KEY)}, String(error));\n"
        + "});\n"
        + "globalThis.storageTestUnbind = listen('input', function() {});\n"
    ).encode("utf-8")


@pytest.fixture
def storage_app_runner(
    apps_api: AppsAPI,
    storage_api: StorageAPI,
    busy_timer_stopped,
):
    installed: dict[str, bytes] = {}

    def run(body: str, *, app_id: str | None = None) -> str:
        app_id = app_id or f"test.storage.{uuid.uuid4().hex[:12]}"
        report_path = f"/ext/apps_data/jsrunner/{app_id}.localstorage.json"
        source = _storage_script(body)

        with allure.step(f"Prepare storage test application {app_id}"):
            if app_id not in installed:
                installed[app_id] = source
                package, _ = build_app_package(app_id, main_script=source)
                staged = apps_api.stage(package)
                result = apps_api.install(staged.install_key)
                assert result.result == "OK", f"Install result: {result!r}"
            else:
                assert (
                    installed[app_id] == source
                ), f"Relaunch of {app_id!r} must use the same script"
            storage_api.remove_raw(report_path)
            allure.attach(
                source.decode(), "JavaScript source", allure.attachment_type.TEXT
            )

        def read_result():
            response = storage_api.read(report_path)
            if response.status_code in {400, 404}:
                return None
            assert (
                response.status_code == 200
            ), f"Report returned HTTP {response.status_code}: {response.text!r}"
            return response.json().get("data", {}).get(RESULT_KEY)

        try:
            with allure.step("Run storage requests inside the installed application"):
                result = apps_api.launch(app_id)
                assert result.result == "OK", f"Launch result: {result!r}"
                report = wait_for(
                    "JavaScript storage test result",
                    read_result,
                    lambda value: value is not None,
                    timeout=20,
                    interval=0.2,
                )
                assert report == "PASS", f"JavaScript storage test failed: {report!r}"
        finally:
            stopped = apps_api.quit_raw()
            assert stopped.status_code in {
                200,
                409,
            }, f"Failed to stop {app_id!r}: HTTP {stopped.status_code}, {stopped.text!r}"
        return app_id

    try:
        yield run
    finally:
        with allure.step("Remove test applications and their storage"):
            for app_id in reversed(installed):
                response = apps_api.delete_app_raw(app_id)
                assert response.status_code in {
                    200,
                    404,
                }, f"Failed to delete {app_id!r}: HTTP {response.status_code}, {response.text!r}"
                for path in (
                    f"{SANDBOX_ROOT}/{app_id}",
                    f"/ext/apps_data/jsrunner/{app_id}.localstorage.json",
                ):
                    response = storage_api.remove_raw(path)
                    assert (
                        response.status_code == 200
                    ), f"Failed to remove {path!r}: HTTP {response.status_code}, {response.text!r}"


@pytest.fixture
def protected_storage_dir(storage_api: StorageAPI):
    path = f"/ext/data_test_{uuid.uuid4().hex[:12]}"
    try:
        storage_api.mkdir(path)
        response = storage_api.write(f"{path}/sentinel.txt", b"protected")
        assert (
            response.status_code == 200
        ), f"Failed to seed sentinel: HTTP {response.status_code}, {response.text!r}"
        yield path
    finally:
        response = storage_api.remove_raw(path)
        assert (
            response.status_code == 200
        ), f"Failed to remove {path!r}: HTTP {response.status_code}, {response.text!r}"


@allure.title("JS storage creates and maps the application's data root")
@pytest.mark.parametrize("virtual_root", ["/ext/data", "/ext/data/"])
def test_js_storage_maps_root(
    storage_app_runner, storage_api: StorageAPI, virtual_root
):
    app_id = storage_app_runner(
        f"""
        const listing = JSON.parse(await storage('GET', 'list', {json.dumps(virtual_root)}, 200));
        assert(listing.list.length === 0, 'New sandbox is not empty: ' + JSON.stringify(listing));
        """
    )
    with allure.step("Verify that the physical application directory was created"):
        listing = storage_api.list(f"{SANDBOX_ROOT}/{app_id}")
        assert listing.list == [], f"Unexpected sandbox contents: {listing!r}"


@allure.title("JS storage supports file and directory operations inside the sandbox")
def test_js_storage_crud(storage_app_runner, storage_api: StorageAPI):
    app_id = storage_app_runner(
        """
        const root = '/ext/data';
        const directory = root + '/sub dir';
        const path = directory + '/file.txt';
        const renamed = directory + '/renamed.txt';
        await storage('POST', 'mkdir', directory, 200, '');
        await storage('POST', 'write', path, 200, 'initial content');
        assert(await storage('GET', 'read', path, 200) === 'initial content', 'Write mismatch');
        await storage('POST', 'write', path, 200, 'short');
        assert(await storage('GET', 'read', path, 200) === 'short', 'Overwrite left stale bytes');
        const append = await fetch('http://127.0.0.1/api/storage/write?path=' +
            encodeURIComponent(path) + '&append=1', {method: 'POST', body: ' appended'});
        await append.text();
        assert(append.status === 200, 'Append status=' + append.status);
        assert(await storage('GET', 'read', path, 200) === 'short appended', 'Append mismatch');
        await storage('POST', 'rename', path, 200, '', renamed);
        await storage('GET', 'read', path, 400);
        assert(await storage('GET', 'read', renamed, 200) === 'short appended', 'Rename lost data');
        const listing = JSON.parse(await storage('GET', 'list', directory, 200)).list;
        assert(listing.length === 1 && listing[0].name === 'renamed.txt' &&
            listing[0].type === 'file', 'Unexpected listing: ' + JSON.stringify(listing));
        await storage('DELETE', 'remove', renamed, 200);
        await storage('GET', 'read', renamed, 400);
        await storage('POST', 'write', path, 200, 'recursive delete');
        await storage('DELETE', 'remove', directory, 200);
        const empty = JSON.parse(await storage('GET', 'list', root, 200)).list;
        assert(empty.length === 0, 'Recursive remove left files: ' + JSON.stringify(empty));
        """
    )
    with allure.step("Verify the sandbox is empty after the application's cleanup"):
        listing = storage_api.list(f"{SANDBOX_ROOT}/{app_id}")
        assert listing.list == [], f"Unexpected remaining files: {listing!r}"


@allure.title(
    "JS storage rejects paths outside its virtual mount without changing files"
)
@pytest.mark.skip(reason="FW-1271: https://flipper.atlassian.net/browse/FW-1271")
@pytest.mark.parametrize("path_kind", ["prefix_collision", "traversal", "relative"])
def test_js_storage_rejects_outside_paths(
    storage_app_runner,
    storage_api: StorageAPI,
    protected_storage_dir: str,
    path_kind: str,
):
    paths = {
        "prefix_collision": protected_storage_dir,
        "traversal": "/ext/data/../../../"
        + protected_storage_dir.removeprefix("/ext/"),
        "relative": protected_storage_dir.lstrip("/"),
    }
    forbidden = json.dumps(paths[path_kind])
    try:
        storage_app_runner(
            f"""
            const forbidden = {forbidden};
            const protectedFile = forbidden + '/sentinel.txt';
            const ownFile = '/ext/data/own.txt';
            await storage('POST', 'write', ownFile, 200, 'own');
            const checks = [
                ['GET', 'read', protectedFile, 400],
                ['GET', 'list', forbidden, 400],
                ['POST', 'write', protectedFile, 400, 'corrupted'],
                ['POST', 'mkdir', forbidden + '/created', 400, ''],
                ['DELETE', 'remove', protectedFile, 400],
                ['POST', 'rename', protectedFile, 400, '', '/ext/data/stolen.txt'],
                ['POST', 'rename', ownFile, 400, '', protectedFile]
            ];
            const errors = [];
            for (let index = 0; index < checks.length; index++) {{
                try {{
                    await storage.apply(null, checks[index]);
                }} catch (error) {{
                    errors.push(String(error));
                }}
            }}
            assert(await storage('GET', 'read', ownFile, 200) === 'own', 'Rejected rename moved source');
            await storage('GET', 'read', '/ext/data/stolen.txt', 400);
            assert(errors.length === 0, JSON.stringify(errors));
            """
        )
    finally:
        with allure.step("Verify the external sentinel and directory were not changed"):
            response = storage_api.read(f"{protected_storage_dir}/sentinel.txt")
            assert (
                response.status_code == 200
            ), f"Sentinel returned HTTP {response.status_code}: {response.text!r}"
            assert (
                response.content == b"protected"
            ), f"Sentinel changed: {response.content!r}"
            listing = storage_api.list(protected_storage_dir).list
            assert [entry.name for entry in listing] == [
                "sentinel.txt"
            ], f"Rejected requests changed the directory: {listing!r}"


@allure.title(
    "JS storage keeps the same virtual filename isolated between applications"
)
def test_js_storage_isolates_applications(storage_app_runner, storage_api: StorageAPI):
    first_id = storage_app_runner(
        "await storage('POST', 'write', '/ext/data/shared.txt', 200, 'first');"
    )
    second_id = storage_app_runner(
        f"""
        await storage('GET', 'read', '/ext/data/shared.txt', 400);
        await storage('GET', 'read', {json.dumps(f'{SANDBOX_ROOT}/{first_id}/shared.txt')}, 400);
        await storage('POST', 'write', '/ext/data/shared.txt', 200, 'second');
        assert(await storage('GET', 'read', '/ext/data/shared.txt', 200) === 'second', 'Own read mismatch');
        """
    )
    with allure.step(
        "Verify independent physical files contain each application's data"
    ):
        for app_id, expected in ((first_id, b"first"), (second_id, b"second")):
            response = storage_api.read(f"{SANDBOX_ROOT}/{app_id}/shared.txt")
            assert (
                response.status_code == 200
            ), f"{app_id!r} file returned HTTP {response.status_code}: {response.text!r}"
            assert (
                response.content == expected
            ), f"{app_id!r} file changed: expected {expected!r}, got {response.content!r}"


@allure.title("JS storage preserves files when the same application is relaunched")
def test_js_storage_persists_across_launches(
    storage_app_runner, storage_api: StorageAPI
):
    body = """
        const listing = JSON.parse(await storage('GET', 'list', '/ext/data', 200)).list;
        if (listing.length === 0) {
            await storage('POST', 'write', '/ext/data/persist.txt', 200, 'persisted');
        } else {
            assert(await storage('GET', 'read', '/ext/data/persist.txt', 200) === 'persisted',
                'File did not survive relaunch');
            await storage('POST', 'write', '/ext/data/persist.txt', 200, 'reopened');
        }
    """
    app_id = storage_app_runner(body)
    storage_app_runner(body, app_id=app_id)
    with allure.step("Verify the second launch reopened the persistent file"):
        response = storage_api.read(f"{SANDBOX_ROOT}/{app_id}/persist.txt")
        assert (
            response.status_code == 200
        ), f"Persistent file returned HTTP {response.status_code}: {response.text!r}"
        assert (
            response.content == b"reopened"
        ), f"Unexpected persistent content: {response.content!r}"


@allure.title("JS-marked storage requests require a running application")
@pytest.mark.parametrize(
    ("method", "operation"),
    [
        ("GET", "read"),
        ("GET", "list"),
        ("POST", "write"),
        ("POST", "mkdir"),
        ("POST", "rename"),
        ("DELETE", "remove"),
    ],
)
def test_js_storage_requires_running_application(
    apps_api: AppsAPI, web_session, web_base_url, method, operation
):
    stopped = apps_api.quit_raw()
    assert stopped.status_code in {
        200,
        409,
    }, f"Failed to stop application: HTTP {stopped.status_code}, {stopped.text!r}"
    path = f"/ext/data/test_{uuid.uuid4().hex[:12]}"
    with allure.step("Send a JS-marked request without a running application"):
        response = web_session.request(
            method,
            f"{web_base_url}/api/storage/{operation}",
            params={"path": path, "new_path": path + "_renamed"},
            headers=JS_FETCH_HEADERS,
            data=b"test" if method == "POST" else None,
        )
        assert (
            response.status_code == 400
        ), f"Expected HTTP 400, got {response.status_code}: {response.text!r}"
