"""Fixtures specific to the state-publisher WebSocket regression tests."""

from __future__ import annotations

import pytest

from clients.state_publisher import StatePublisherClient


@pytest.fixture(scope="function")
def state_publisher_ws(web_base_url):
    """Open a fresh state-publisher WebSocket per test, drained and ready.

    Function-scoped so each test starts with an empty queue. The drain
    happens here (immediately after the opt-in handshake) and tests are
    expected to drain again right before their mutation to discard any
    snapshot-shaped traffic that may have raced in.
    """
    with StatePublisherClient(web_base_url) as ws:
        ws.drain()
        yield ws
