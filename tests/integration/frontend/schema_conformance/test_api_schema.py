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

import allure
import pytest
import schemathesis
from hypothesis import HealthCheck, settings

from .conftest import SAFE_WRITE_OPERATION_IDS, SKIP_OPERATION_IDS

_base_url = os.getenv("WEB_BASE_URL", "http://10.0.4.20")
_schema = schemathesis.openapi.from_url(f"{_base_url}/openapi.yaml")

#TODO: Remove after https://flipper.atlassian.net/browse/FW-753 is fixed
_CHECKS = [schemathesis.checks.not_a_server_error]


# ---------------------------------------------------------------------------
# 1. GET conformance — all read-only operations
# ---------------------------------------------------------------------------


@allure.feature("5. Web Frontend")
@allure.story("Schema Conformance")
@allure.title("GET endpoints conform to OpenAPI schema")
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
    with allure.step(f"Call {case.method.upper()} {case.formatted_path}"):
        response = case.call(session=web_session)

    with allure.step(f"Validate response ({response.status_code})"):
        case.validate_response(response, checks=_CHECKS)


# ---------------------------------------------------------------------------
# 2. Safe write conformance — allowed POST operations
# ---------------------------------------------------------------------------


@allure.feature("5. Web Frontend")
@allure.story("Schema Conformance")
@allure.title("Safe write operations conform to OpenAPI schema")
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
    with allure.step(f"Call {case.method.upper()} {case.formatted_path}"):
        response = case.call(session=web_session)

    with allure.step(f"Validate response ({response.status_code})"):
        case.validate_response(response, checks=_CHECKS)
