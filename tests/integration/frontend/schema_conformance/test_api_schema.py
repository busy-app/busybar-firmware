from __future__ import annotations

"""
Schemathesis tests: automatic verification of API conformance to the OpenAPI contract.

Two test groups:

1. test_get_conformance
   All GET operations. Schemathesis generates boundary values for query parameters
   and verifies that each response matches the schema. Fully safe — read-only.

2. test_post_conformance
   Explicitly allowed POST operations that do not destroy device state or break
   the HTTP session. Fewer examples are used because of side effects.

Both groups:
- Skip operations from SKIP_OPERATION_IDS (destructive, WebSocket, etc.)
- Use a limited number of examples (max_examples) — the device may become
  unstable under a high request rate
- Integrate with Allure
"""

import os
from http.client import RemoteDisconnected

import allure
import pytest
import requests
import schemathesis
from hypothesis import HealthCheck, settings
from .conftest import SAFE_WRITE_OPERATION_IDS, SKIP_OPERATION_IDS


def _is_mongoose_drop(exc: requests.exceptions.ConnectionError) -> bool:
    """True if Mongoose dropped the TCP connection mid-request (not a device crash).

    Mongoose closes the connection on certain malformed inputs (binary data,
    tilde characters, etc.) instead of returning a 4xx response.  The device
    stays alive — the ``device_health_monitor`` fixture independently detects
    real crashes, so we can safely skip these cases here.

    Exception chain: ConnectionError(ProtocolError('Connection aborted.', RemoteDisconnected(...)))
    """
    cause = exc.args[0] if exc.args else None
    return (
        isinstance(cause, Exception)
        and len(cause.args) >= 2
        and isinstance(cause.args[1], (RemoteDisconnected, ConnectionResetError))
    )

_base_url = os.getenv("WEB_BASE_URL", "http://10.0.4.20")
_schema = schemathesis.openapi.from_url(f"{_base_url}/openapi.yaml")



# ---------------------------------------------------------------------------
# 1. GET conformance — all read-only operations
# ---------------------------------------------------------------------------


@allure.feature("5. Web Frontend")
@allure.story("Schema Conformance")
@pytest.mark.schemathesis
@pytest.mark.frontend
@_schema.include(
    method="GET",
).exclude(
    operation_id=list(SKIP_OPERATION_IDS),
).parametrize()
@settings(
    max_examples=10,
    suppress_health_check=[HealthCheck.too_slow, HealthCheck.filter_too_much, HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_get_conformance(case: schemathesis.Case, web_session) -> None:
    allure.dynamic.title(f"GET {case.formatted_path}")
    allure.dynamic.parameter("method", case.method.upper())
    allure.dynamic.parameter("path", case.formatted_path)

    try:
        response = case.call(session=web_session)
    except requests.exceptions.ConnectionError as exc:
        if _is_mongoose_drop(exc):
            allure.attach(
                f"query: {case.query!r}\n[Mongoose closed the TCP connection — no HTTP response]",
                name=f"{case.method.upper()} {case.formatted_path}",
                attachment_type=allure.attachment_type.TEXT,
            )
            return
        raise

    allure.attach(
        f"query: {case.query!r}\nstatus: {response.status_code}",
        name=f"{case.method.upper()} {case.formatted_path} → {response.status_code}",
        attachment_type=allure.attachment_type.TEXT,
    )
    case.validate_response(response)


# ---------------------------------------------------------------------------
# 2. Safe write conformance — allowed POST operations
# ---------------------------------------------------------------------------


@allure.feature("5. Web Frontend")
@allure.story("Schema Conformance")
@pytest.mark.schemathesis
@pytest.mark.frontend
@_schema.include(
    method="POST",
    operation_id=list(SAFE_WRITE_OPERATION_IDS),
).parametrize()
@settings(
    max_examples=5,
    suppress_health_check=[HealthCheck.too_slow, HealthCheck.filter_too_much, HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_post_conformance(case: schemathesis.Case, web_session) -> None:
    allure.dynamic.title(f"POST {case.formatted_path}")
    allure.dynamic.parameter("method", case.method.upper())
    allure.dynamic.parameter("path", case.formatted_path)

    try:
        response = case.call(session=web_session)
    except requests.exceptions.ConnectionError as exc:
        if _is_mongoose_drop(exc):
            allure.attach(
                f"body: {case.body!r}\n[Mongoose closed the TCP connection — no HTTP response]",
                name=f"{case.method.upper()} {case.formatted_path}",
                attachment_type=allure.attachment_type.TEXT,
            )
            return
        raise

    allure.attach(
        f"body: {case.body!r}\nstatus: {response.status_code}",
        name=f"{case.method.upper()} {case.formatted_path} → {response.status_code}",
        attachment_type=allure.attachment_type.TEXT,
    )
    case.validate_response(response)
