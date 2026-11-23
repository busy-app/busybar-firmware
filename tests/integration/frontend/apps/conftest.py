"""Isolated application queue and device-reachable package downloads."""

from __future__ import annotations

import socket
import threading
from pathlib import Path
from urllib.parse import urlsplit

import allure
import pytest

from clients.state_pb import app_queue_pb2, state_pb2
from clients.state_publisher import StatePublisherClient
from utils.app_package_server import AppPackageServer
from utils.app_queue import AppQueueHarness
from utils.app_queue_contract import AppQueueContract
from utils.wait import wait_for


@pytest.fixture(scope="session")
def app_queue_contract():
    return AppQueueContract(Path(__file__).resolve().parents[4])


@pytest.fixture
def app_package_server(web_base_url):
    target = urlsplit(web_base_url)
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as route_socket:
        route_socket.connect((target.hostname, target.port or 80))
        host_ip = route_socket.getsockname()[0]

    server = AppPackageServer(host_ip)
    server_thread = threading.Thread(target=server.serve_forever, daemon=True)
    server_thread.start()
    try:
        yield server
    finally:
        server.release_all()
        server.shutdown()
        server_thread.join(timeout=2)
        server.server_close()


@pytest.fixture
def app_queue(apps_api, storage_api, web_base_url, app_package_server):
    initial = apps_api.get_queue()
    assert not initial.result, f"Application queue is already in use: {initial!r}"
    assert (
        app_queue_pb2 is not None
    ), "Application queue protobuf bindings are unavailable"
    assert (
        "app_queue_event" in state_pb2.StateUpdate.DESCRIPTOR.fields_by_name
    ), "Regenerate protobuf bindings from the PR's assets/proto submodule"

    with StatePublisherClient(web_base_url) as ws:
        # The first streamed frame confirms that enable has been processed before
        # any fast operation can finish and publish its result.
        ws.wait_for(lambda update: update.WhichOneof("state") == "frame")
        ws.drain()
        instance = AppQueueHarness(apps_api, app_package_server, ws)
        try:
            yield instance
        finally:
            with allure.step("Cancel test operations and remove test applications"):
                cleanup_errors = []
                if instance.has_launched_app:
                    try:
                        response = apps_api.quit_raw()
                        assert response.status_code in {
                            200,
                            409,
                        }, f"HTTP {response.status_code}, {response.text!r}"
                    except Exception as error:
                        cleanup_errors.append(f"Quit test application: {error}")
                for description, action in [
                    ("Cancel test operations", instance.cancel),
                    ("Wait for empty queue", instance.wait_idle),
                    (
                        "Receive terminal events",
                        lambda: instance.wait_finished(instance.accepted_ids),
                    ),
                    (
                        "Check for duplicate terminal events",
                        instance.assert_no_more_results,
                    ),
                ]:
                    try:
                        action()
                    except Exception as error:
                        cleanup_errors.append(f"{description}: {error}")
                for app_id in sorted(instance.app_ids):
                    try:
                        response = apps_api.delete_app_raw(app_id)
                        assert response.status_code in {
                            200,
                            404,
                        }, f"HTTP {response.status_code}, {response.text[:200]!r}"
                    except Exception as error:
                        cleanup_errors.append(f"{app_id}: {error}")
                for path in sorted(instance.data_paths):
                    try:
                        response = storage_api.remove_raw(path)
                        assert response.status_code in {
                            200,
                            404,
                        }, f"HTTP {response.status_code}, {response.text[:200]!r}"
                    except Exception as error:
                        cleanup_errors.append(f"{path}: {error}")
                assert (
                    not cleanup_errors
                ), f"Application cleanup failed: {cleanup_errors!r}"


@pytest.fixture
def queue_launch_app(app_queue, storage_api, busy_timer_stopped):
    def launch(app):
        path = f"/ext/apps_data/jsrunner/{app.app_id}.localstorage.json"
        app_queue.data_paths.add(path)
        response = app_queue.api.launch(app.app_id)
        app_queue.has_launched_app = True
        assert response.result == "OK", f"Unexpected launch response: {response!r}"
        wait_for(
            "queued test application to publish its ready marker",
            lambda: storage_api.read(path),
            lambda reply: reply.status_code == 200
            and reply.json().get("data", {}).get("ready") == "1",
            timeout=8,
            interval=0.2,
        )

    return launch
