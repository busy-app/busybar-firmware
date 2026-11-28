"""Boundary and request-validation coverage for the application queue API."""

from __future__ import annotations

from copy import deepcopy
import json

import allure
import pytest

from clients.api import AppQueueAddResponse
from clients.state_pb import apps_pb2
from utils.app_queue import AppQueueHarness


QUEUE_PATH = "/api/apps/queue"
CANCEL_PATH = "/api/apps/queue/cancel"
UINT32_MAX = (1 << 32) - 1
MISSING = object()

pytestmark = [
    pytest.mark.api,
    pytest.mark.frontend,
    pytest.mark.bsb_metadata(
        suite="Contract",
        level="Integration",
        intent="Product",
        interfaces=("Device API", "WebSocket"),
        transports=("LAN REST", "WebSocket"),
    ),
]


def _blocked_queue(instance: AppQueueHarness):
    blocker = instance.package(held=True)
    item_ids = instance.enqueue(blocker.install())
    blocker.download.wait_started()
    return item_ids


def _replace(item, path, value):
    result = deepcopy(item)
    parent = result
    for key in path[:-1]:
        parent = parent[key]
    if value is MISSING:
        parent.pop(path[-1])
    else:
        parent[path[-1]] = value
    return result


def _post(instance, session, base_url, **kwargs):
    response = session.post(base_url + QUEUE_PATH, **kwargs)
    if response.status_code == 202:
        accepted = AppQueueAddResponse.model_validate(response.json())
        instance.accepted_ids.update(
            item.item_id for item in accepted.result if item.item_id is not None
        )
    return response


INVALID_ROOTS = [
    pytest.param(None, id="null"),
    pytest.param([], id="array"),
    pytest.param(True, id="boolean", marks=pytest.mark.regression),
    pytest.param(1, id="number", marks=pytest.mark.regression),
    pytest.param("operations", id="string", marks=pytest.mark.regression),
    pytest.param({}, id="missing-operations"),
    pytest.param({"operations": None}, id="null-operations"),
    pytest.param({"operations": {}}, id="object-operations"),
    pytest.param(
        {"operations": "remove"}, id="string-operations", marks=pytest.mark.regression
    ),
    pytest.param({"operations": [None]}, id="null-item"),
    pytest.param({"operations": [[]]}, id="array-item", marks=pytest.mark.regression),
    pytest.param({"operations": [1]}, id="number-item", marks=pytest.mark.regression),
]

INVALID_FIELDS = [
    pytest.param(("app_id",), MISSING, id="missing-id"),
    pytest.param(("app_id",), None, id="null-id"),
    pytest.param(("app_id",), 1, id="number-id"),
    pytest.param(("app_id",), "", id="empty-id"),
    pytest.param(("app_id",), "Test.queue", id="uppercase-id"),
    pytest.param(("app_id",), ".hidden", id="leading-dot"),
    pytest.param(("app_id",), "a" * 33, id="id-too-long"),
    pytest.param(("app_id",), "../outside", id="parent-path"),
    pytest.param(("app_id",), "test/queue", id="slash-id"),
    pytest.param(("app_id",), "test\\queue", id="backslash-id"),
    pytest.param(("operation",), MISSING, id="missing-operation"),
    pytest.param(("operation",), None, id="null-operation"),
    pytest.param(("operation",), [], id="array-operation"),
    pytest.param(("operation", "type"), MISSING, id="missing-type"),
    pytest.param(("operation", "type"), None, id="null-type"),
    pytest.param(("operation", "type"), 1, id="number-type"),
    pytest.param(("operation", "type"), "update", id="unknown-type"),
    pytest.param(("operation", "type"), "INSTALL", id="uppercase-type"),
    pytest.param(("operation", "url"), MISSING, id="missing-url"),
    pytest.param(("operation", "url"), None, id="null-url"),
    pytest.param(("operation", "url"), 1, id="number-url"),
    pytest.param(("operation", "url"), [], id="array-url"),
    pytest.param(("operation", "version"), None, id="null-version"),
    pytest.param(("operation", "version"), 1, id="number-version"),
    pytest.param(("operation", "version"), {}, id="object-version"),
]

INVALID_CANCEL = [
    pytest.param(None, id="null"),
    pytest.param([], id="array", marks=pytest.mark.regression),
    pytest.param({}, id="empty-object"),
    pytest.param({"all": False}, id="all-false"),
    pytest.param({"all": None}, id="all-null", marks=pytest.mark.regression),
    pytest.param({"all": 1}, id="all-number", marks=pytest.mark.regression),
    pytest.param({"all": "true"}, id="all-string", marks=pytest.mark.regression),
    pytest.param({"item_id": None}, id="id-null", marks=pytest.mark.regression),
    pytest.param({"item_id": True}, id="id-boolean"),
    pytest.param({"item_id": 0.5}, id="id-fraction"),
    pytest.param({"item_id": "1"}, id="id-string", marks=pytest.mark.regression),
    pytest.param({"item_id": []}, id="id-array", marks=pytest.mark.regression),
    pytest.param({"item_id": {}}, id="id-object", marks=pytest.mark.regression),
    pytest.param({"item_id": -1}, id="id-negative"),
    pytest.param({"item_id": UINT32_MAX + 1}, id="id-above-uint32"),
    pytest.param({"all": True, "item_id": 1}, id="both-alternatives"),
]


@allure.epic("BusyBar & Local Control")
@allure.feature("JavaScript applications")
@allure.story("Application queue contract")
class TestAppQueueContract:
    @allure.title("Invalid queue root is rejected without changing existing work")
    @pytest.mark.parametrize("body", INVALID_ROOTS)
    def test_invalid_roots(
        self, app_queue, app_queue_contract, web_session, web_base_url, body
    ):
        app_queue_contract.assert_request(QUEUE_PATH, body, valid=False)
        item_ids = _blocked_queue(app_queue)
        with allure.step("Reject malformed request and preserve the held operation"):
            response = _post(
                app_queue,
                web_session,
                web_base_url,
                data=json.dumps(body),
                headers={"Content-Type": "application/json"},
            )
            app_queue.snapshot(item_ids)
            app_queue_contract.assert_error(response)
            app_queue_contract.assert_response(QUEUE_PATH, "post", response)

    @allure.title("Invalid operation fields are rejected atomically")
    @pytest.mark.parametrize("path,value", INVALID_FIELDS)
    def test_invalid_fields(
        self, app_queue, app_queue_contract, web_session, web_base_url, path, value
    ):
        item_ids = _blocked_queue(app_queue)
        candidate = app_queue.package()
        item = candidate.install().model_dump(exclude_none=True)
        body = {"operations": [_replace(item, path, value)]}
        app_queue_contract.assert_request(QUEUE_PATH, body, valid=False)
        with allure.step("Reject invalid item and preserve the original queue"):
            response = _post(app_queue, web_session, web_base_url, json=body)
            app_queue.snapshot(item_ids)
            app_queue_contract.assert_error(response)
            app_queue_contract.assert_response(QUEUE_PATH, "post", response)

    @allure.title("Version strings must follow the documented x.y.z format")
    @pytest.mark.parametrize(
        "version",
        [
            "",
            pytest.param("1", marks=pytest.mark.regression),
            "1.2",
            pytest.param("1.2.3.4", marks=pytest.mark.regression),
            pytest.param("a.2.3", marks=pytest.mark.regression),
            "-1.2.3",
            pytest.param("1.2.-3", marks=pytest.mark.regression),
            "1.2.3suffix",
            "\u0661.2.3",
        ],
        ids=[
            "empty",
            "major-only",
            "missing-patch",
            "extra-component",
            "non-numeric",
            "negative-major",
            "negative-patch",
            "suffix",
            "non-ascii-digit",
        ],
    )
    def test_invalid_version_strings(
        self, app_queue, app_queue_contract, web_session, web_base_url, version
    ):
        item_ids = _blocked_queue(app_queue)
        candidate = app_queue.package()
        item = candidate.install().model_dump(exclude_none=True)
        item["operation"]["version"] = version
        response = _post(
            app_queue, web_session, web_base_url, json={"operations": [item]}
        )
        with allure.step("Reject a non-x.y.z version without adding its operation"):
            assert (
                response.status_code == 400
            ), f"Invalid version accepted: {version!r}, {response.status_code}, {response.text!r}"
            app_queue.snapshot(item_ids)
            app_queue_contract.assert_response(QUEUE_PATH, "post", response)

    @allure.title("Remove operations ignore optional download fields")
    @pytest.mark.parametrize(
        "field,value",
        [
            ("url", None),
            pytest.param("url", 1, marks=pytest.mark.regression),
            pytest.param("version", None, marks=pytest.mark.regression),
            pytest.param("version", 1, marks=pytest.mark.regression),
        ],
    )
    def test_remove_ignores_download_fields(
        self, app_queue, app_queue_contract, web_session, web_base_url, field, value
    ):
        item_ids = _blocked_queue(app_queue)
        candidate = app_queue.package()
        item = candidate.remove().model_dump(exclude_none=True)
        item["operation"][field] = value
        body = {"operations": [item]}
        response = _post(app_queue, web_session, web_base_url, json=body)
        with allure.step("Enqueue removal without interpreting URL or version"):
            assert (
                response.status_code == 202
            ), f"Remove returned {response.status_code}: {response.text!r}"
            app_queue_contract.assert_response(QUEUE_PATH, "post", response)
            result = AppQueueAddResponse.model_validate(response.json()).result
            assert len(result) == 1, f"Unexpected remove response: {result!r}"
            item = result[0]
            assert item.item_id is not None, f"Remove was not enqueued: {item!r}"
            assert item.app_id == candidate.app_id, f"Remove changed app ID: {item!r}"
            assert item.operation.model_dump(exclude_none=True) == {
                "type": "remove"
            }, f"Ignored fields leaked into queue metadata: {item!r}"
            app_queue.snapshot(item_ids + [item.item_id])
        app_queue.cancel(item_ids[0])
        app_queue.wait_results(
            {
                item_ids[0]: apps_pb2.QUEUE_ERROR_CANCELLED,
                item.item_id: apps_pb2.QUEUE_ERROR_NOT_FOUND,
            }
        )
        app_queue.wait_idle()

    @allure.title("Special characters in restricted operation fields are rejected")
    @pytest.mark.parametrize(
        "path,prefix",
        [
            (("app_id",), "test.queue"),
            (("operation", "type"), "install"),
            (("operation", "version"), "1.2.3"),
        ],
        ids=["app-id", "operation-type", "version"],
    )
    @pytest.mark.parametrize(
        "character",
        [
            "\n",
            pytest.param("\r", marks=pytest.mark.regression),
            pytest.param("\t", marks=pytest.mark.regression),
            pytest.param('"', marks=pytest.mark.regression),
            pytest.param("'", marks=pytest.mark.regression),
            pytest.param("\\", marks=pytest.mark.regression),
            pytest.param("%", marks=pytest.mark.regression),
            "\u2603",
        ],
        ids=[
            "newline",
            "carriage-return",
            "tab",
            "double-quote",
            "single-quote",
            "backslash",
            "percent",
            "unicode",
        ],
    )
    def test_special_characters(
        self,
        app_queue,
        app_queue_contract,
        web_session,
        web_base_url,
        path,
        prefix,
        character,
    ):
        item_ids = _blocked_queue(app_queue)
        candidate = app_queue.package()
        item = candidate.install().model_dump(exclude_none=True)
        body = {"operations": [_replace(item, path, prefix + character + "suffix")]}
        response = _post(app_queue, web_session, web_base_url, json=body)
        with allure.step("Reject the special character without queue mutation"):
            assert (
                response.status_code == 400
            ), f"Special character accepted in {path!r}: {character!r}, {response.status_code}, {response.text!r}"
            app_queue.snapshot(item_ids)
            app_queue_contract.assert_error(response)

    @allure.title("Invalid source URL strings are rejected before queue mutation")
    @pytest.mark.parametrize(
        "url",
        ["", "not a URL", "http://", "http://127.0.0.1:65536/app.tgz"],
        ids=["empty", "no-scheme", "no-host", "port-overflow"],
    )
    def test_invalid_url_strings(
        self, app_queue, app_queue_contract, web_session, web_base_url, url
    ):
        item_ids = _blocked_queue(app_queue)
        candidate = app_queue.package()
        item = candidate.install().model_dump(exclude_none=True)
        item["operation"]["url"] = url
        response = _post(
            app_queue, web_session, web_base_url, json={"operations": [item]}
        )
        with allure.step("Reject an invalid URL and preserve the original operation"):
            assert (
                response.status_code == 400
            ), f"Invalid URL accepted: {url!r}, {response.status_code}, {response.text!r}"
            app_queue.snapshot(item_ids)
            app_queue_contract.assert_response(QUEUE_PATH, "post", response)

    @allure.title(
        "A later invalid item prevents every operation in its batch from being enqueued"
    )
    @pytest.mark.parametrize(
        "position",
        [
            pytest.param(0, marks=pytest.mark.regression),
            1,
            pytest.param(2, marks=pytest.mark.regression),
        ],
        ids=["first", "middle", "last"],
    )
    def test_batch_validation_is_atomic(
        self, app_queue, app_queue_contract, web_session, web_base_url, position
    ):
        item_ids = _blocked_queue(app_queue)
        candidate = app_queue.package()
        items = [candidate.install().model_dump(exclude_none=True) for _ in range(3)]
        items[position] = {"app_id": candidate.app_id, "operation": {"type": "unknown"}}
        body = {"operations": items}
        app_queue_contract.assert_request(QUEUE_PATH, body, valid=False)
        with allure.step("Reject the entire batch before any queue mutation"):
            response = _post(app_queue, web_session, web_base_url, json=body)
            app_queue.snapshot(item_ids)
            assert (
                not candidate.download.requested.is_set()
            ), f"Invalid batch started a download: {candidate.download.url}"
            app_queue_contract.assert_response(QUEUE_PATH, "post", response)
            assert (
                response.status_code == 400
            ), f"Invalid batch returned {response.status_code}: {response.text!r}"

    @allure.title("A batch beyond OpenAPI maxItems is rejected without partial enqueue")
    def test_batch_size_above_limit(
        self, app_queue, app_queue_contract, web_session, web_base_url
    ):
        item_ids = _blocked_queue(app_queue)
        candidate = app_queue.package()
        body = {"operations": [candidate.remove().model_dump(exclude_none=True)] * 101}
        app_queue_contract.assert_request(QUEUE_PATH, body, valid=False)
        response = _post(app_queue, web_session, web_base_url, json=body)
        with allure.step("Verify no operation from the oversized batch was added"):
            app_queue.snapshot(item_ids)
            app_queue_contract.assert_error(response)
            app_queue_contract.assert_response(QUEUE_PATH, "post", response)

    @allure.title("Malformed JSON is rejected without affecting active work")
    @pytest.mark.parametrize(
        "body",
        [
            pytest.param(b"", marks=pytest.mark.regression),
            b"{",
            pytest.param(b'{"operations":', marks=pytest.mark.regression),
            pytest.param(b'{"operations":[,]}', marks=pytest.mark.regression),
        ],
    )
    def test_malformed_json(self, app_queue, web_session, web_base_url, body):
        item_ids = _blocked_queue(app_queue)
        response = _post(
            app_queue,
            web_session,
            web_base_url,
            data=body,
            headers={"Content-Type": "application/json"},
        )
        with allure.step("Verify parse error and queue isolation"):
            assert (
                response.status_code == 400
            ), f"Malformed JSON returned {response.status_code}: {response.text!r}"
            app_queue.snapshot(item_ids)

    @allure.title("Invalid cancellation cannot modify the active queue")
    @pytest.mark.parametrize("body", INVALID_CANCEL)
    def test_invalid_cancellation(
        self, app_queue, app_queue_contract, web_session, web_base_url, body
    ):
        item_ids = _blocked_queue(app_queue)
        response = web_session.post(
            web_base_url + CANCEL_PATH,
            data=json.dumps(body),
            headers={"Content-Type": "application/json"},
        )
        with allure.step("Verify a client error and unchanged active operation"):
            assert (
                response.status_code == 400
            ), f"Invalid cancellation returned {response.status_code}: {response.text!r}"
            app_queue.snapshot(item_ids)
            app_queue_contract.assert_error(response)

    @allure.title("Malformed cancellation JSON returns Bad Request")
    def test_cancellation_malformed_json(
        self, app_queue, app_queue_contract, web_session, web_base_url
    ):
        response = web_session.post(
            web_base_url + CANCEL_PATH,
            data=b"{",
            headers={"Content-Type": "application/json"},
        )
        app_queue.snapshot([])
        assert (
            response.status_code == 400
        ), f"Malformed cancellation returned {response.status_code}: {response.text!r}"
        app_queue_contract.assert_error(response)

    @allure.title("Unknown uint32 IDs return 404 without queue mutation")
    @pytest.mark.parametrize(
        "item_id",
        [0, UINT32_MAX],
        ids=["zero", "uint32-max"],
    )
    def test_unknown_integer_id(
        self, app_queue, app_queue_contract, web_session, web_base_url, item_id
    ):
        body = {"item_id": item_id}
        app_queue_contract.assert_request(CANCEL_PATH, body, valid=True)
        response = web_session.post(web_base_url + CANCEL_PATH, json=body)
        with allure.step("Verify documented not-found response for an absent ID"):
            app_queue.snapshot([])
            assert (
                response.status_code == 404
            ), f"Valid absent item ID returned {response.status_code}: {response.text!r}"
            app_queue_contract.assert_response(CANCEL_PATH, "post", response)

    @allure.title("Unsupported queue methods return 405 and the documented Allow set")
    @pytest.mark.parametrize(
        "path,method",
        [
            (QUEUE_PATH, "PUT"),
            pytest.param(QUEUE_PATH, "DELETE", marks=pytest.mark.regression),
            (CANCEL_PATH, "GET"),
            pytest.param(CANCEL_PATH, "PUT", marks=pytest.mark.regression),
            pytest.param(CANCEL_PATH, "DELETE", marks=pytest.mark.regression),
        ],
    )
    def test_wrong_methods(
        self, app_queue, app_queue_contract, web_session, web_base_url, path, method
    ):
        response = web_session.request(method, web_base_url + path)
        with allure.step("Verify routing contract without executing an operation"):
            app_queue.snapshot([])
            assert (
                response.status_code == 405
            ), f"{method} {path}: {response.status_code}, {response.text!r}"
            allowed = {verb.upper() for verb in app_queue_contract.paths[path]}
            actual = {
                verb.strip()
                for verb in response.headers.get("Allow", "").split(",")
                if verb.strip()
            }
            assert actual == allowed, f"Allow={actual!r}, expected={allowed!r}"
