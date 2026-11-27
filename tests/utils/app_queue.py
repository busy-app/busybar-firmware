"""Queue lifecycle helpers shared by application integration tests."""

from __future__ import annotations

import time
import uuid
from dataclasses import dataclass
from http import HTTPStatus

import allure

from clients.api import (
    AppQueueAddRequest,
    AppQueueAddResponse,
    AppQueueCancelRequest,
    AppQueueClearRequest,
    AppQueueGetResponse,
    AppQueueRequestItem,
    AppQueueRequestOperation,
    AppsAPI,
    StorageAPI,
)
from clients.state_pb import apps_pb2
from clients.state_publisher import StatePublisherClient
from utils.app_package_server import AppPackageDownload, AppPackageServer
from utils.js_app_package import build_app_package
from utils.wait import wait_for


QUEUE_TIMEOUT_S = 30
QUEUE_POLL_INTERVAL_S = 0.2
WS_QUIET_PERIOD_S = 0.5


@dataclass
class QueueTestApp:
    app_id: str
    version: str
    main_script: bytes
    download: AppPackageDownload

    def install(self, *, include_version: bool = True) -> AppQueueRequestItem:
        return AppQueueRequestItem(
            app_id=self.app_id,
            operation=AppQueueRequestOperation(
                type="install",
                url=self.download.url,
                version=self.version if include_version else None,
            ),
        )

    def remove(self) -> AppQueueRequestItem:
        return AppQueueRequestItem(
            app_id=self.app_id,
            operation=AppQueueRequestOperation(type="remove"),
        )


class AppQueueHarness:
    def __init__(
        self, api: AppsAPI, server: AppPackageServer, ws: StatePublisherClient
    ) -> None:
        self.api = api
        self.server = server
        self.ws = ws
        self.app_ids: set[str] = set()
        self.accepted_ids: set[int] = set()
        self.results: dict[int, int] = {}
        self.result_order: list[int] = []
        self.data_paths: set[str] = set()
        self.has_launched_app = False

    def package(
        self,
        *,
        app_id: str | None = None,
        version: str = "1.0.0",
        held: bool = False,
        body: bytes | None = None,
        status: int = HTTPStatus.OK,
        truncated: bool = False,
        filename: str | None = None,
        known_length: bool = True,
        **package_options,
    ) -> QueueTestApp:
        app_id = app_id or f"test.queue.{uuid.uuid4().hex[:12]}"
        self.app_ids.add(app_id)
        package, main_script = build_app_package(
            app_id, version=version, **package_options
        )
        download = self.server.add_package(
            package if body is None else body,
            held=held,
            status=status,
            truncated=truncated,
            filename=filename,
            known_length=known_length,
        )
        return QueueTestApp(app_id, version, main_script, download)

    def submit(self, *operations: AppQueueRequestItem) -> AppQueueAddResponse:
        response = self.api.add_to_queue(
            AppQueueAddRequest(operations=list(operations))
        )
        assert len(response.result) == len(
            operations
        ), f"Unexpected enqueue response for {operations!r}: {response!r}"
        for requested, result in zip(operations, response.result):
            assert (
                result.app_id == requested.app_id
            ), f"Enqueue response changed application: {result!r}"
            expected_operation = requested.operation.model_dump(exclude={"url"})
            assert result.operation.model_dump() == expected_operation, (
                f"Enqueue response changed operation: {result!r}, "
                f"requested={requested!r}"
            )
            if result.item_id is not None:
                assert (
                    result.item_id not in self.accepted_ids
                ), f"Queue reused an active test's item ID: {result.item_id}"
                self.accepted_ids.add(result.item_id)
        return response

    def enqueue(self, *operations: AppQueueRequestItem) -> list[int]:
        response = self.submit(*operations)
        item_ids: list[int] = []
        for result in response.result:
            assert result.item_id is not None, f"Operation was not accepted: {result!r}"
            item_ids.append(result.item_id)
        return item_ids

    def snapshot(self, item_ids: list[int]) -> AppQueueGetResponse:
        response = self.api.get_queue()
        actual_ids = [item.item_id for item in response.result]
        assert (
            actual_ids == item_ids
        ), f"Unexpected queue order: {response!r}; expected IDs={item_ids!r}"
        statuses = [item.status for item in response.result]
        expected = (
            ["in_progress"] + ["pending"] * (len(item_ids) - 1) if item_ids else []
        )
        assert (
            statuses == expected
        ), f"Unexpected operation statuses: {response!r}; expected={expected!r}"
        return response

    def wait_idle(self) -> None:
        wait_for(
            "application queue to become empty",
            self.api.get_queue,
            lambda response: not response.result,
            timeout=QUEUE_TIMEOUT_S,
            interval=QUEUE_POLL_INTERVAL_S,
        )

    def cancel(self, item_id: int | None = None) -> None:
        request = (
            AppQueueClearRequest()
            if item_id is None
            else AppQueueCancelRequest(item_id=item_id)
        )
        response = self.api.cancel_queue(request)
        assert (
            response.result == "OK"
        ), f"Unexpected cancel acknowledgement: {response!r}"

    def _record_result(self, update) -> None:
        result = update.apps_event.queue_operation_result
        item_id = result.item_id
        assert item_id in self.accepted_ids, (
            f"WS result has an unknown item ID: {result!r}; "
            f"accepted={self.accepted_ids!r}"
        )
        assert item_id not in self.results, f"Duplicate WS result: {result!r}"
        self.results[item_id] = result.error
        self.result_order.append(item_id)

    @staticmethod
    def _is_result(update) -> bool:
        return (
            update.WhichOneof("state") == "apps_event"
            and update.apps_event.WhichOneof("value") == "queue_operation_result"
        )

    def wait_finished(self, item_ids) -> None:
        expected = set(item_ids)
        deadline = time.monotonic() + QUEUE_TIMEOUT_S
        while not expected <= self.results.keys():
            remaining = max(0, deadline - time.monotonic())
            try:
                update = self.ws.wait_for(self._is_result, timeout=remaining)
            except TimeoutError as error:
                raise AssertionError(
                    f"Missing WS results: {expected - self.results.keys()}; "
                    f"received={self.results!r}"
                ) from error
            self._record_result(update)

    def wait_results(self, expected: dict[int, int]) -> None:
        with allure.step(f"Wait for final WS results: {expected!r}"):
            self.wait_finished(expected)

            for item_id, error in expected.items():
                actual = self.results[item_id]
                assert actual == error, (
                    f"Wrong result for item_id={item_id}: "
                    f"{apps_pb2.QueueError.Name(actual)}, "
                    f"expected {apps_pb2.QueueError.Name(error)}"
                )

    def assert_no_more_results(self) -> None:
        try:
            update = self.ws.wait_for(self._is_result, timeout=WS_QUIET_PERIOD_S)
        except TimeoutError:
            return
        raise AssertionError(f"Unexpected additional WS result: {update!r}")


def app_queue_assert_installed(
    instance: AppQueueHarness, storage_api: StorageAPI, app: QueueTestApp
) -> None:
    with allure.step(f"Verify installed metadata and script for {app.app_id}"):
        listed = instance.api.list_apps()
        installed = next((item for item in listed.apps if item.id == app.app_id), None)
        assert installed is not None, f"Installed application is missing: {listed!r}"
        assert (
            installed.version == app.version
        ), f"Wrong installed version: {installed!r}; expected={app.version!r}"
        script = storage_api.read(f"/ext/user_assets/{app.app_id}/scripts/main.js")
        assert (
            script.status_code == HTTPStatus.OK
        ), f"Cannot read installed script: HTTP {script.status_code}, {script.text[:200]!r}"
        assert (
            script.content == app.main_script
        ), f"Installed script differs: {script.content!r}; expected={app.main_script!r}"


def app_queue_assert_absent(instance: AppQueueHarness, *apps: QueueTestApp) -> None:
    listed = instance.api.list_apps()
    unexpected = {app.app_id for app in apps} & {item.id for item in listed.apps}
    assert not unexpected, f"Applications unexpectedly remain installed: {unexpected!r}"


def app_queue_assert_snapshot(instance: AppQueueHarness, item_ids, operations) -> None:
    snapshot = instance.snapshot(item_ids)
    for item, operation in zip(snapshot.result, operations):
        assert (
            item.app_id == operation.app_id
        ), f"Queue application differs: {item!r}; expected={operation!r}"
        assert item.operation.model_dump() == operation.operation.model_dump(
            exclude={"url"}
        ), f"Queue operation differs: {item!r}; expected={operation!r}"


def app_queue_assert_cancel_not_found(instance: AppQueueHarness, item_id: int) -> None:
    response = instance.api.cancel_queue_raw(AppQueueCancelRequest(item_id=item_id))
    assert response.status_code == HTTPStatus.NOT_FOUND, (
        f"Cancelling item {item_id} returned HTTP {response.status_code}: "
        f"{response.text[:200]!r}"
    )
