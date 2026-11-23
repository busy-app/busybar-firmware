"""Application queue lifecycle, cancellation and final WebSocket outcomes."""

from __future__ import annotations

import allure
import pytest

from clients.state_pb import app_queue_pb2
from utils.js_app_package import build_non_app_package
from utils.app_queue import (
    AppQueueHarness,
    app_queue_assert_installed as _assert_installed,
    app_queue_assert_absent as _assert_absent,
    app_queue_assert_snapshot as _assert_queue,
    app_queue_assert_cancel_not_found as _assert_cancel_not_found,
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

BURST_OPERATIONS_COUNT = 12


@allure.epic("BusyBar & Local Control")
@allure.feature("JavaScript applications")
@allure.story("Application management queue")
class TestAppQueue:
    @allure.title("An empty application queue can be cleared repeatedly")
    def test_clear_empty_queue(self, app_queue: AppQueueHarness):
        with allure.step("Read and clear the empty queue twice"):
            app_queue.snapshot([])
            app_queue.cancel()
            app_queue.cancel()
            app_queue.snapshot([])
        app_queue.assert_no_more_results()

    @allure.title("An empty operation batch is accepted as a no-op per OpenAPI")
    def test_empty_batch_is_noop(self, app_queue: AppQueueHarness):
        with allure.step("Enqueue the empty batch permitted by minItems: 0"):
            item_ids = app_queue.enqueue()
            assert item_ids == [], f"Empty batch created queue items: {item_ids!r}"
            app_queue.snapshot([])
        app_queue.assert_no_more_results()

    @allure.title("Queue install, update and remove produce matching final WS results")
    @pytest.mark.parametrize(
        "include_version", [True, False], ids=["version", "no-version"]
    )
    def test_app_lifecycle(
        self, app_queue: AppQueueHarness, storage_api, include_version
    ):
        original = app_queue.package(held=True)
        operation = original.install(include_version=include_version)
        with allure.step("Hold the initial download and inspect the active operation"):
            item_ids = app_queue.enqueue(operation)
            original.download.wait_started()
            _assert_queue(app_queue, item_ids, [operation])
            _assert_absent(app_queue, original)

        with allure.step("Finish installation and verify its files"):
            original.download.release.set()
            app_queue.wait_results(dict.fromkeys(item_ids, app_queue_pb2.OK))
            app_queue.wait_idle()
            _assert_installed(app_queue, storage_api, original)

        with allure.step("Update the installed application through the queue"):
            updated = app_queue.package(app_id=original.app_id, version="2.0.0")
            update_ids = app_queue.enqueue(updated.install())
            app_queue.wait_results(dict.fromkeys(update_ids, app_queue_pb2.OK))
            app_queue.wait_idle()
            _assert_installed(app_queue, storage_api, updated)

        with allure.step(
            "Remove the application and reject cancellation of its finished item"
        ):
            remove_ids = app_queue.enqueue(updated.remove())
            app_queue.wait_results(dict.fromkeys(remove_ids, app_queue_pb2.OK))
            app_queue.wait_idle()
            _assert_absent(app_queue, updated)
            _assert_cancel_not_found(app_queue, remove_ids[0])
        app_queue.assert_no_more_results()

    @allure.title("A mixed batch executes in FIFO order with one active operation")
    def test_batch_fifo(self, app_queue: AppQueueHarness, storage_api):
        first = app_queue.package(held=True)
        second = app_queue.package(held=True)
        operations = [first.install(), second.install(), first.remove()]

        with allure.step("Queue two installs followed by removal of the first app"):
            item_ids = app_queue.enqueue(*operations)
            first.download.wait_started()
            _assert_queue(app_queue, item_ids, operations)
            assert (
                not second.download.requested.is_set()
            ), f"Second download started before the first finished: {second.download.url}"

        with allure.step("Release the first install while keeping the second blocked"):
            first.download.release.set()
            app_queue.wait_results({item_ids[0]: app_queue_pb2.OK})
            second.download.wait_started()
            _assert_queue(app_queue, item_ids[1:], operations[1:])
            _assert_installed(app_queue, storage_api, first)
            _assert_absent(app_queue, second)

        with allure.step("Complete the batch and verify sequential final events"):
            second.download.release.set()
            app_queue.wait_results(dict.fromkeys(item_ids, app_queue_pb2.OK))
            app_queue.wait_idle()
            _assert_absent(app_queue, first)
            _assert_installed(app_queue, storage_api, second)
            assert app_queue.result_order == item_ids, (
                f"Final WS events are out of order: {app_queue.result_order!r}; "
                f"expected={item_ids!r}"
            )
        app_queue.assert_no_more_results()

    @allure.title(
        "Cancelling a pending operation preserves its app and the remaining queue"
    )
    @pytest.mark.parametrize("operation_type", ["install", "remove"])
    def test_cancel_pending(
        self, app_queue: AppQueueHarness, storage_api, operation_type
    ):
        target = app_queue.package()
        if operation_type == "remove":
            initial_ids = app_queue.enqueue(target.install())
            app_queue.wait_results(dict.fromkeys(initial_ids, app_queue_pb2.OK))
            app_queue.wait_idle()
            _assert_installed(app_queue, storage_api, target)

        blocker = app_queue.package(held=True)
        survivor = app_queue.package()
        target_operation = (
            target.install() if operation_type == "install" else target.remove()
        )
        operations = [blocker.install(), target_operation, survivor.install()]
        item_ids = app_queue.enqueue(*operations)
        blocker.download.wait_started()
        _assert_queue(app_queue, item_ids, operations)

        with allure.step("Cancel only the pending middle operation"):
            app_queue.cancel(item_ids[1])
            app_queue.wait_results({item_ids[1]: app_queue_pb2.CANCELLED})
            _assert_queue(
                app_queue, [item_ids[0], item_ids[2]], [operations[0], operations[2]]
            )
            _assert_cancel_not_found(app_queue, item_ids[1])

        with allure.step(
            "Complete the surviving operations and verify cancellation had no side effects"
        ):
            blocker.download.release.set()
            app_queue.wait_results(
                dict.fromkeys([item_ids[0], item_ids[2]], app_queue_pb2.OK)
            )
            app_queue.wait_idle()
            _assert_installed(app_queue, storage_api, blocker)
            _assert_installed(app_queue, storage_api, survivor)
            if operation_type == "remove":
                _assert_installed(app_queue, storage_api, target)
            else:
                _assert_absent(app_queue, target)
                assert (
                    not target.download.requested.is_set()
                ), f"Cancelled pending install was downloaded: {target.download.url}"
        app_queue.assert_no_more_results()

    @allure.title(
        "Cancelling an active update preserves the old version and advances the queue"
    )
    def test_cancel_active_download(self, app_queue: AppQueueHarness, storage_api):
        original = app_queue.package()
        initial_ids = app_queue.enqueue(original.install())
        app_queue.wait_results(dict.fromkeys(initial_ids, app_queue_pb2.OK))
        app_queue.wait_idle()
        _assert_installed(app_queue, storage_api, original)

        update = app_queue.package(app_id=original.app_id, version="2.0.0", held=True)
        survivor = app_queue.package()
        operations = [update.install(), survivor.install()]
        item_ids = app_queue.enqueue(*operations)
        update.download.wait_started()
        _assert_queue(app_queue, item_ids, operations)

        with allure.step(
            "Cancel the incomplete update and wait for both final outcomes"
        ):
            app_queue.cancel(item_ids[0])
            app_queue.wait_results(
                {item_ids[0]: app_queue_pb2.CANCELLED, item_ids[1]: app_queue_pb2.OK}
            )
            app_queue.wait_idle()

        with allure.step(
            "Verify the original app survived and the next install succeeded"
        ):
            _assert_installed(app_queue, storage_api, original)
            _assert_installed(app_queue, storage_api, survivor)
            _assert_cancel_not_found(app_queue, item_ids[0])
        app_queue.assert_no_more_results()

    @allure.title(
        "Clearing the queue emits CANCELLED for active and pending items and allows reuse"
    )
    def test_cancel_all(self, app_queue: AppQueueHarness, storage_api):
        active = app_queue.package(held=True)
        pending = app_queue.package()
        operations = [active.install(), pending.install(), pending.remove()]
        item_ids = app_queue.enqueue(*operations)
        active.download.wait_started()
        _assert_queue(app_queue, item_ids, operations)

        with allure.step(
            "Cancel all operations, including pending removal of an absent app"
        ):
            app_queue.cancel()
            app_queue.wait_results(dict.fromkeys(item_ids, app_queue_pb2.CANCELLED))
            app_queue.wait_idle()
            _assert_absent(app_queue, active, pending)
            assert (
                not pending.download.requested.is_set()
            ), f"Cleared pending install was downloaded: {pending.download.url}"
            app_queue.cancel()

        with allure.step("Install a new application after clearing the queue"):
            recovery = app_queue.package()
            recovery_ids = app_queue.enqueue(recovery.install())
            app_queue.wait_results(dict.fromkeys(recovery_ids, app_queue_pb2.OK))
            app_queue.wait_idle()
            _assert_installed(app_queue, storage_api, recovery)
        app_queue.assert_no_more_results()

    @allure.title(
        "An installation failure emits its WS error and does not block the next operation"
    )
    @pytest.mark.parametrize(
        "package_options, expected_error",
        [
            pytest.param({"truncated": True}, app_queue_pb2.DOWNLOAD, id="download"),
            pytest.param({"held": True}, app_queue_pb2.DOWNLOAD, id="download-timeout"),
            pytest.param(
                {"body": b"invalid archive"}, app_queue_pb2.UNPACK, id="unpack"
            ),
            pytest.param(
                {"manifest_id": "test.queue.other"},
                app_queue_pb2.MANIFEST,
                id="manifest",
            ),
            pytest.param(
                {"api_version": "999.0.0"}, app_queue_pb2.API_VERSION, id="api-version"
            ),
            pytest.param({"body": b""}, app_queue_pb2.UNPACK, id="empty-package"),
            pytest.param(
                {"body": build_non_app_package()},
                app_queue_pb2.MANIFEST,
                id="non-app-archive",
            ),
            pytest.param(
                {"include_main": False}, app_queue_pb2.MANIFEST, id="missing-main"
            ),
            pytest.param(
                {"missing_manifest_fields": ("id",)},
                app_queue_pb2.MANIFEST,
                id="missing-manifest-id",
            ),
            pytest.param(
                {"manifest_updates": {"format_version": "invalid"}},
                app_queue_pb2.MANIFEST,
                id="manifest-format",
            ),
        ],
    )
    def test_failure_advances_queue(
        self, app_queue: AppQueueHarness, storage_api, package_options, expected_error
    ):
        invalid = app_queue.package(**package_options)
        valid = app_queue.package(held=True)

        with allure.step("Queue a failing install followed by a valid install"):
            item_ids = app_queue.enqueue(invalid.install(), valid.install())
            app_queue.wait_results({item_ids[0]: expected_error})
            valid.download.wait_started()
            _assert_queue(app_queue, item_ids[1:], [valid.install()])
            _assert_absent(app_queue, invalid, valid)

        with allure.step("Complete the next operation successfully"):
            valid.download.release.set()
            app_queue.wait_results({item_ids[1]: app_queue_pb2.OK})
            app_queue.wait_idle()
            _assert_installed(app_queue, storage_api, valid)
            assert (
                app_queue.result_order == item_ids
            ), f"Wrong final event order: {app_queue.result_order!r}; expected={item_ids!r}"
        app_queue.assert_no_more_results()

    @allure.title(
        "Fast operations retain every final WS event in order without duplicates"
    )
    def test_fast_results_are_sequential(self, app_queue: AppQueueHarness):
        absent = app_queue.package()

        with allure.step("Queue a burst of removals of an absent application"):
            item_ids = app_queue.enqueue(
                *[absent.remove() for _ in range(BURST_OPERATIONS_COUNT)]
            )
            app_queue.wait_results(dict.fromkeys(item_ids, app_queue_pb2.NOT_FOUND))
            app_queue.wait_idle()

        with allure.step("Verify one ordered final event for every accepted operation"):
            assert (
                app_queue.result_order == item_ids
            ), f"Final WS event sequence differs: {app_queue.result_order!r}; expected={item_ids!r}"
            _assert_absent(app_queue, absent)
            assert (
                not absent.download.requested.is_set()
            ), f"Removal unexpectedly downloaded an application: {absent.download.url}"
        app_queue.assert_no_more_results()
