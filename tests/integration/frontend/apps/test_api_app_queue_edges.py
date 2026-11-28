"""Queue capacity, interleavings, rollback and stream boundary scenarios."""

from __future__ import annotations

import json
import string
import uuid
from urllib.parse import quote, unquote, urlsplit, urlunsplit

import allure
import pytest

from clients.api import AppQueueCancelRequest, AppSettingsDocument
from clients.state_pb import apps_pb2
from clients.state_publisher import StatePublisherClient
from utils.app_queue import (
    AppQueueHarness,
    app_queue_assert_absent,
    app_queue_assert_installed,
    app_queue_assert_snapshot,
)
from utils.js_app_package import build_app_package


PENDING_LIMIT = 100
PROGRESS_PAYLOAD_SIZE = 512 * 1024
RUNNING_SCRIPT = (
    b"localStorage.setItem('ready', '1');\n"
    b"globalThis.unbind = listen('input', function() {});\n"
)
pytestmark = [
    pytest.mark.api,
    pytest.mark.frontend,
    pytest.mark.state_publisher,
    pytest.mark.bsb_metadata(
        suite="Integration",
        level="Integration",
        intent="Product",
        interfaces=("Device API", "WebSocket"),
        transports=("LAN REST", "WebSocket"),
    ),
]


def _install(instance, *apps):
    item_ids = instance.enqueue(*(app.install() for app in apps))
    instance.wait_results(dict.fromkeys(item_ids, apps_pb2.QUEUE_ERROR_NONE))
    instance.wait_idle()
    return item_ids


def _block(instance):
    blocker = instance.package(held=True)
    item_ids = instance.enqueue(blocker.install())
    blocker.download.wait_started()
    return blocker, item_ids


def _cancel_all(instance):
    instance.cancel()
    instance.wait_results(
        dict.fromkeys(instance.accepted_ids, apps_pb2.QUEUE_ERROR_CANCELLED)
    )
    instance.wait_idle()
    instance.assert_no_more_results()


def _observe_download_progress(instance, *, known_length):
    # Progress is coalesced; keep enough uncompressed data before the release gate.
    app_id = "test.queue." + uuid.uuid4().hex[:12]
    app = instance.package(
        app_id=app_id,
        held=True,
        known_length=known_length,
        gzip=False,
        extra_files={f"{app_id}/resources/payload.bin": b"x" * PROGRESS_PAYLOAD_SIZE},
    )
    item_ids = instance.enqueue(app.install())
    app.download.wait_started()

    def is_detail(update):
        if update.WhichOneof("state") != "apps_state":
            return False
        state = update.apps_state
        if state.WhichOneof("value") != "queue_operation_detail":
            return False
        detail = state.queue_operation_detail
        return (
            detail.item_id == item_ids[0]
            and detail.step == "downloading"
            and (
                detail.HasField("progress") and 0 < detail.progress < 100
                if known_length
                else not detail.HasField("progress")
            )
        )

    with allure.step("Observe download progress and active queue details"):
        update = instance.ws.wait_for(is_detail, timeout=8)
        detail = update.apps_state.queue_operation_detail
        assert (
            detail.HasField("progress") == known_length
        ), f"Unexpected progress presence: {detail!r}"
        snapshot = instance.snapshot(item_ids)
        assert (
            snapshot.result[0].detail is not None
        ), f"Active operation has no detail: {snapshot!r}"
    return app, item_ids


@allure.epic("BusyBar & Local Control")
@allure.feature("JavaScript applications")
@allure.story("Application queue boundaries")
class TestAppQueueEdges:
    @allure.title(
        "A maximum-size batch fills 100 pending slots in addition to active work"
    )
    def test_maximum_batch(self, app_queue):
        _, active_ids = _block(app_queue)
        absent = app_queue.package()
        with allure.step("Enqueue all 100 pending operations in one valid request"):
            pending_ids = app_queue.enqueue(
                *[absent.remove() for _ in range(PENDING_LIMIT)]
            )
            app_queue.snapshot(active_ids + pending_ids)
        with allure.step("Clear all 101 items and retain every terminal event"):
            _cancel_all(app_queue)
            pending_order = [
                item_id for item_id in app_queue.result_order if item_id in pending_ids
            ]
            assert (
                pending_order == pending_ids
            ), f"Pending cancellation event order differs: {pending_order!r}"

    @allure.title(
        "Queue-full batches report accepted, queue_full and skipped items and allow slot reuse"
    )
    def test_partial_batch_and_capacity_recovery(self, app_queue):
        _, active_ids = _block(app_queue)
        absent = app_queue.package()
        initial_ids = app_queue.enqueue(
            *[absent.remove() for _ in range(PENDING_LIMIT - 2)]
        )
        with allure.step("Submit four operations with space for only the first two"):
            result = app_queue.submit(*[absent.remove() for _ in range(4)]).result
            assert [item.error for item in result] == [
                None,
                None,
                "queue_full",
                "skipped",
            ], f"Unexpected partial result: {result!r}"
            accepted = [item.item_id for item in result if item.item_id is not None]
            full_ids = active_ids + initial_ids + accepted
            app_queue.snapshot(full_ids)
            rejected = app_queue.submit(absent.remove()).result
            assert (
                rejected[0].error == "queue_full"
            ), f"Full queue accepted an operation: {rejected!r}"
        with allure.step(
            "Cancel a pending item and append a replacement in its freed slot"
        ):
            cancelled = initial_ids[len(initial_ids) // 2]
            app_queue.cancel(cancelled)
            app_queue.wait_results({cancelled: apps_pb2.QUEUE_ERROR_CANCELLED})
            replacement = app_queue.enqueue(absent.remove())
            app_queue.snapshot(
                [item_id for item_id in full_ids if item_id != cancelled] + replacement
            )
        _cancel_all(app_queue)

    @allure.title(
        "Cancelling first, middle or last pending install keeps the other items in order"
    )
    @pytest.mark.parametrize("position", [0, 1, 2], ids=["first", "middle", "last"])
    def test_cancel_pending_positions(self, app_queue, storage_api, position):
        blocker, active_ids = _block(app_queue)
        pending = [app_queue.package() for _ in range(3)]
        pending_ids = app_queue.enqueue(*(app.install() for app in pending))
        with allure.step("Remove the selected pending item without moving the others"):
            cancelled = pending_ids[position]
            app_queue.cancel(cancelled)
            app_queue.wait_results({cancelled: apps_pb2.QUEUE_ERROR_CANCELLED})
            survivors = [item_id for item_id in pending_ids if item_id != cancelled]
            app_queue.snapshot(active_ids + survivors)
            assert not pending[
                position
            ].download.requested.is_set(), (
                f"Cancelled download started: {pending[position].download.url}"
            )
        blocker.download.release.set()
        app_queue.wait_results(
            dict.fromkeys(active_ids + survivors, apps_pb2.QUEUE_ERROR_NONE)
        )
        app_queue.wait_idle()
        app_queue_assert_absent(app_queue, pending[position])
        for index, app in enumerate(pending):
            if index != position:
                app_queue_assert_installed(app_queue, storage_api, app)
        actual = [item_id for item_id in app_queue.result_order if item_id in survivors]
        assert actual == survivors, f"Survivor completion order differs: {actual!r}"

    @allure.title("Repeated cancellation of active work has one terminal result")
    @pytest.mark.parametrize("clear_all", [False, True], ids=["single", "all"])
    def test_repeated_active_cancellation(
        self, app_queue, app_queue_contract, clear_all
    ):
        _, item_ids = _block(app_queue)
        if clear_all:
            app_queue.cancel()
            app_queue.cancel()
        else:
            app_queue.cancel(item_ids[0])
            response = app_queue.api.cancel_queue_raw(
                AppQueueCancelRequest(item_id=item_ids[0])
            )
            app_queue_contract.assert_response(
                "/api/apps/queue/cancel", "post", response
            )
            assert response.status_code in {
                202,
                404,
            }, f"Unexpected repeated cancel: {response.status_code}, {response.text!r}"
        app_queue.wait_results(dict.fromkeys(item_ids, apps_pb2.QUEUE_ERROR_CANCELLED))
        app_queue.wait_idle()
        app_queue.assert_no_more_results()

    @allure.title(
        "Cancellation rejects conflicting fields and accepts unrelated fields"
    )
    @pytest.mark.parametrize(
        "extra,expected_status",
        [({"client_tag": "qa"}, 202), ({"all": False}, 400), ({"item_id": None}, 400)],
        ids=["extra-field", "cancel-all-false", "clear-null-id"],
    )
    def test_cancellation_additional_fields(
        self,
        app_queue,
        app_queue_contract,
        web_session,
        web_base_url,
        extra,
        expected_status,
    ):
        _, item_ids = _block(app_queue)
        body = (
            {"all": True, **extra}
            if "item_id" in extra
            else {"item_id": item_ids[0], **extra}
        )
        response = web_session.post(web_base_url + "/api/apps/queue/cancel", json=body)
        with allure.step("Verify cancellation field handling"):
            assert (
                response.status_code == expected_status
            ), f"Unexpected cancel status: {response.status_code}, expected={expected_status}, {response.text!r}"
            if expected_status == 202:
                app_queue_contract.assert_response(
                    "/api/apps/queue/cancel", "post", response
                )
                app_queue.wait_results(
                    dict.fromkeys(item_ids, apps_pb2.QUEUE_ERROR_CANCELLED)
                )
                app_queue.wait_idle()
            else:
                app_queue_contract.assert_error(response)
                app_queue.snapshot(item_ids)

    @allure.title(
        "Application IDs at both documented length boundaries install successfully"
    )
    @pytest.mark.parametrize("length", [1, 32])
    def test_app_id_boundaries(self, app_queue, storage_api, length):
        existing = {item.id for item in app_queue.api.list_apps().apps}
        app_id = (
            next(
                value
                for value in string.ascii_lowercase + string.digits
                if value not in existing
            )
            if length == 1
            else ("test.queue." + uuid.uuid4().hex)[:length]
        )
        app = app_queue.package(app_id=app_id)
        _install(app_queue, app)
        app_queue_assert_installed(app_queue, storage_api, app)

    @allure.title(
        "Optional version metadata supports zero and uint32 boundary components"
    )
    @pytest.mark.parametrize(
        "version", ["0.0.0", "1.2.3", "4294967295.4294967295.4294967295"]
    )
    def test_version_metadata_boundaries(self, app_queue, version):
        blocker, active_ids = _block(app_queue)
        app = app_queue.package()
        operation = app.install()
        operation.operation.version = version
        item_ids = app_queue.enqueue(operation)
        app_queue_assert_snapshot(
            app_queue, active_ids + item_ids, [blocker.install(), operation]
        )
        snapshot = app_queue.snapshot(active_ids + item_ids)
        assert (
            snapshot.result[1].operation.version == version
        ), f"Version metadata changed: {snapshot!r}"
        _cancel_all(app_queue)

    @allure.title(
        "The installed manifest is authoritative when optional version metadata differs"
    )
    def test_manifest_version(self, app_queue, storage_api):
        app = app_queue.package(version="1.2.3")
        operation = app.install()
        operation.operation.version = "9.9.9"
        item_ids = app_queue.enqueue(operation)
        app_queue.wait_results(dict.fromkeys(item_ids, apps_pb2.QUEUE_ERROR_NONE))
        app_queue.wait_idle()
        app_queue_assert_installed(app_queue, storage_api, app)

    @allure.title("Plain TAR and close-delimited downloads install successfully")
    @pytest.mark.parametrize(
        "options",
        [{"gzip": False}, {"known_length": False}],
        ids=["plain-tar", "unknown-length"],
    )
    def test_package_framing(self, app_queue, storage_api, options):
        app = app_queue.package(**options)
        _install(app_queue, app)
        app_queue_assert_installed(app_queue, storage_api, app)

    @allure.title("An unknown numeric manifest format remains forward compatible")
    def test_unknown_manifest_format(self, app_queue, storage_api):
        app = app_queue.package(manifest_updates={"format_version": 99})
        _install(app_queue, app)
        app_queue_assert_installed(app_queue, storage_api, app)

    @allure.title(
        "Every punctuation character permitted in application IDs is supported"
    )
    @pytest.mark.parametrize("prefix", ["_", "-", "queue_1-a.b-"])
    def test_app_id_characters(self, app_queue, storage_api, prefix):
        app = app_queue.package(app_id=prefix + uuid.uuid4().hex[:12])
        _install(app_queue, app)
        app_queue_assert_installed(app_queue, storage_api, app)

    @allure.title("Source URLs preserve spaces, UTF-8 and escapes")
    @pytest.mark.parametrize(
        "filename,encoded",
        [
            ("with space.tgz", False),
            ("РїР°РєРµС‚.tgz", False),
            ("with space.tgz", True),
        ],
        ids=["space", "unicode", "escaped-space"],
    )
    def test_source_url_encoding(self, app_queue, storage_api, filename, encoded):
        app = app_queue.package(filename=filename)
        source = urlsplit(app.download.url)
        url = urlunsplit(
            (
                source.scheme,
                source.netloc,
                quote(source.path, safe="/") if encoded else source.path,
                "",
                "",
            )
        )
        operation = app.install()
        operation.operation.url = url
        item_ids = app_queue.enqueue(operation)
        app_queue.wait_results(dict.fromkeys(item_ids, apps_pb2.QUEUE_ERROR_NONE))
        app_queue.wait_idle()
        app_queue_assert_installed(app_queue, storage_api, app)
        actual = app.download.request_target
        assert actual is not None, f"Source URL was never requested: {url!r}"
        assert (
            unquote(urlsplit(actual).path) == source.path
        ), f"URL path changed: {actual!r}, source={url!r}"

    @allure.title("Failed updates preserve the existing manifest and script")
    @pytest.mark.parametrize(
        "options,error",
        [
            ({"truncated": True}, apps_pb2.QUEUE_ERROR_DOWNLOAD),
            ({"body": b"invalid archive"}, apps_pb2.QUEUE_ERROR_UNPACK),
            ({"manifest_id": "test.queue.other"}, apps_pb2.QUEUE_ERROR_MANIFEST),
            ({"api_version": "999.0.0"}, apps_pb2.QUEUE_ERROR_API_VERSION),
        ],
        ids=["download", "unpack", "manifest", "api-version"],
    )
    def test_failed_update_preserves_app(self, app_queue, storage_api, options, error):
        original = app_queue.package()
        _install(app_queue, original)
        update = app_queue.package(app_id=original.app_id, version="2.0.0", **options)
        item_ids = app_queue.enqueue(update.install())
        app_queue.wait_results(dict.fromkeys(item_ids, error))
        app_queue.wait_idle()
        app_queue_assert_installed(app_queue, storage_api, original)
        recovery = app_queue.package(app_id=original.app_id, version="3.0.0")
        _install(app_queue, recovery)
        app_queue_assert_installed(app_queue, storage_api, recovery)

    @allure.title("Running application protection is evaluated when queued work starts")
    @pytest.mark.parametrize("operation_type", ["install", "remove"])
    @pytest.mark.parametrize(
        "launch_when_pending",
        [False, True],
        ids=["already-running", "launched-while-pending"],
    )
    def test_running_app_protection(
        self,
        app_queue,
        storage_api,
        queue_launch_app,
        operation_type,
        launch_when_pending,
    ):
        original = app_queue.package(main_script=RUNNING_SCRIPT)
        _install(app_queue, original)
        if not launch_when_pending:
            queue_launch_app(original)
        blocker, active_ids = _block(app_queue)
        update = app_queue.package(app_id=original.app_id, version="2.0.0")
        operation = (
            update.install() if operation_type == "install" else original.remove()
        )
        item_ids = app_queue.enqueue(operation)
        if launch_when_pending:
            queue_launch_app(original)
        blocker.download.release.set()
        app_queue.wait_results(
            {
                active_ids[0]: apps_pb2.QUEUE_ERROR_NONE,
                item_ids[0]: apps_pb2.QUEUE_ERROR_APP_RUNNING,
            }
        )
        app_queue.wait_idle()
        app_queue_assert_installed(app_queue, storage_api, original)
        assert (
            not update.download.requested.is_set()
        ), f"Protected update downloaded: {update.download.url}"

    @allure.title(
        "An app launched during an update download is protected before replacement"
    )
    @pytest.mark.xfail(reason="FW-1321", raises=AssertionError, strict=False)
    def test_app_launch_during_update_download(
        self, app_queue, storage_api, queue_launch_app
    ):
        original = app_queue.package(main_script=RUNNING_SCRIPT)
        _install(app_queue, original)
        update = app_queue.package(app_id=original.app_id, version="2.0.0", held=True)
        item_ids = app_queue.enqueue(update.install())
        update.download.wait_started()
        queue_launch_app(original)
        update.download.release.set()
        app_queue.wait_results(
            dict.fromkeys(item_ids, apps_pb2.QUEUE_ERROR_APP_RUNNING)
        )
        app_queue.wait_idle()
        app_queue_assert_installed(app_queue, storage_api, original)

    @allure.title("Queued update, remove and reinstall preserve persistent settings")
    def test_settings_survive_queue_lifecycle(self, app_queue):
        app_id = "test.queue." + uuid.uuid4().hex[:12]
        schema = {
            "format_version": 1,
            "version": 1,
            "fields": {
                "level": {
                    "label": "Level",
                    "type": "integer",
                    "default": 4,
                    "min": 0,
                    "max": 10,
                    "step": 1,
                }
            },
        }
        extras = {f"{app_id}/appmeta/settings.json": json.dumps(schema).encode()}
        app_queue.data_paths.add(f"/ext/apps_data/jsrunner/{app_id}.settings.json")
        original = app_queue.package(app_id=app_id, extra_files=extras)
        _install(app_queue, original)
        document = AppSettingsDocument(version=1, values={"level": 7})
        app_queue.api.set_settings(app_id, document)
        updated = app_queue.package(app_id=app_id, version="2.0.0", extra_files=extras)
        _install(app_queue, updated)
        assert (
            app_queue.api.get_settings(app_id) == document
        ), f"Settings lost on update: {app_queue.api.get_settings(app_id)!r}"
        item_ids = app_queue.enqueue(updated.remove())
        app_queue.wait_results(dict.fromkeys(item_ids, apps_pb2.QUEUE_ERROR_NONE))
        app_queue.wait_idle()
        _install(app_queue, updated)
        actual = app_queue.api.get_settings(app_id)
        assert actual == document, f"Settings lost on remove/reinstall: {actual!r}"

    @allure.title(
        "Reconnect recovers the queue through GET and receives future terminal events"
    )
    def test_ws_reconnect(self, app_queue, web_base_url):
        blocker, active_ids = _block(app_queue)
        candidate = app_queue.package()
        item_ids = app_queue.enqueue(candidate.install())
        with StatePublisherClient(web_base_url) as first:
            first.wait_for(lambda update: update.WhichOneof("state") == "frame")
        app_queue.snapshot(active_ids + item_ids)
        with StatePublisherClient(web_base_url) as second:
            second.wait_for(lambda update: update.WhichOneof("state") == "frame")
            second.drain()
            observer = AppQueueHarness(app_queue.api, app_queue.server, second)
            observer.accepted_ids.update(active_ids + item_ids)
            blocker.download.release.set()
            expected = dict.fromkeys(active_ids + item_ids, apps_pb2.QUEUE_ERROR_NONE)
            app_queue.wait_results(expected)
            observer.wait_results(expected)
            observer.assert_no_more_results()
        app_queue.wait_idle()

    @allure.title("Known-length download reports partial progress and completes")
    def test_download_progress_known_length(self, app_queue):
        app, item_ids = _observe_download_progress(app_queue, known_length=True)
        app.download.release.set()
        app_queue.wait_results(dict.fromkeys(item_ids, apps_pb2.QUEUE_ERROR_NONE))
        app_queue.wait_idle()

    @allure.title("Unknown-length download omits progress and can be cancelled")
    def test_download_progress_unknown_length(self, app_queue):
        _, item_ids = _observe_download_progress(app_queue, known_length=False)
        app_queue.cancel(item_ids[0])
        app_queue.wait_results(dict.fromkeys(item_ids, apps_pb2.QUEUE_ERROR_CANCELLED))
        app_queue.wait_idle()

    @allure.title(
        "Legacy stage/install/delete are blocked by active queue work and recover afterwards"
    )
    @pytest.mark.parametrize("operation_type", ["stage", "install", "delete"])
    def test_legacy_interlocks(self, app_queue, storage_api, operation_type):
        app = app_queue.package()
        package, _ = build_app_package(app.app_id)
        staged = app_queue.api.stage(package) if operation_type == "install" else None
        if operation_type == "delete":
            _install(app_queue, app)
        _, item_ids = _block(app_queue)
        with allure.step("Reject conflicting synchronous operations"):
            if operation_type == "stage":
                response = app_queue.api.stage_raw(package)
            elif operation_type == "install":
                response = app_queue.api.install_raw(staged.install_key)
            else:
                response = app_queue.api.delete_app_raw(app.app_id)
            expected_status = 409 if operation_type == "delete" else 400
            assert (
                response.status_code == expected_status
            ), f"Unexpected legacy conflict: {response.status_code}, {response.text!r}"
            app_queue.snapshot(item_ids)
        app_queue.cancel()
        app_queue.wait_results(dict.fromkeys(item_ids, apps_pb2.QUEUE_ERROR_CANCELLED))
        app_queue.wait_idle()
        with allure.step("Retry after the queue is empty"):
            if operation_type == "stage":
                staged = app_queue.api.stage(package)
                response = app_queue.api.install(staged.install_key)
            elif operation_type == "install":
                response = app_queue.api.install(staged.install_key)
            else:
                response = app_queue.api.delete_app(app.app_id)
            assert (
                response.result == "OK"
            ), f"Legacy operation did not recover: {response!r}"
            if operation_type != "delete":
                app_queue_assert_installed(app_queue, storage_api, app)

    @allure.title("Add requests retain metadata and permit additional OpenAPI fields")
    def test_additional_enqueue_fields(
        self, app_queue, app_queue_contract, web_session, web_base_url
    ):
        app = app_queue.package(held=True)
        item = app.install().model_dump(exclude_none=True)
        item["client_tag"] = "item"
        item["operation"]["client_tag"] = "operation"
        body = {"operations": [item], "client_tag": "batch"}
        app_queue_contract.assert_request("/api/apps/queue", body, valid=True)
        response = web_session.post(web_base_url + "/api/apps/queue", json=body)
        from_response = response.json()
        if response.status_code == 202:
            item_ids = [
                result["item_id"]
                for result in from_response["result"]
                if "item_id" in result
            ]
            app_queue.accepted_ids.update(item_ids)
        app_queue_contract.assert_response("/api/apps/queue", "post", response)
        assert (
            response.status_code == 202
        ), f"Valid additional fields rejected: {response.status_code}, {response.text!r}"
        app.download.wait_started()
        app.download.release.set()
        app_queue.wait_results(dict.fromkeys(item_ids, apps_pb2.QUEUE_ERROR_NONE))
        app_queue.wait_idle()
