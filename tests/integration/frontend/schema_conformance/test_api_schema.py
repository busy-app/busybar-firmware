from __future__ import annotations

"""
Schemathesis tests: automatic verification of API conformance to the OpenAPI contract.

Two test groups:

1. test_get_conformance
   All GET operations. Schemathesis generates boundary values for query parameters
   and verifies that each response matches the schema. Fully safe — read-only.

2. test_post_conformance
   All POST operations except those in SKIP_OPERATION_IDS / SKIP_PATHS.
   New operations are covered automatically — only explicitly dangerous ones
   need a SKIP entry in conftest.py.

Both groups:
- Use a limited number of examples (max_examples) — the device may become
  unstable under a high request rate
- Integrate with Allure
"""

import os
import time

import allure
import pytest
import schemathesis
from hypothesis import HealthCheck, settings
from .conftest import SKIP_OPERATION_IDS, SKIP_PATHS_RE


_base_url = os.getenv("WEB_BASE_URL", "http://10.0.4.20")
_schema = schemathesis.openapi.from_url(f"{_base_url}/openapi.yaml")


# ---------------------------------------------------------------------------
# 1. GET conformance — all read-only operations
# ---------------------------------------------------------------------------


@allure.feature("5. Web Frontend")
@allure.story("Schema Conformance")
@pytest.mark.schemathesis
@pytest.mark.regression
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

    time.sleep(0.5)
    response = case.call(session=web_session)

    allure.attach(
        f"query: {case.query!r}\nstatus: {response.status_code}",
        name=f"{case.method.upper()} {case.formatted_path} → {response.status_code}",
        attachment_type=allure.attachment_type.TEXT,
    )
    case.validate_response(response)


# ---------------------------------------------------------------------------
# 2. POST conformance — all write operations except explicitly skipped ones
# ---------------------------------------------------------------------------


@allure.feature("5. Web Frontend")
@allure.story("Schema Conformance")
@pytest.mark.schemathesis
@pytest.mark.regression
@pytest.mark.frontend
@_schema.include(
    method="POST",
).exclude(
    operation_id=list(SKIP_OPERATION_IDS),
).exclude(
    path_regex=SKIP_PATHS_RE,
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

    time.sleep(0.5)
    response = case.call(session=web_session)

    allure.attach(
        f"body: {case.body!r}\nstatus: {response.status_code}",
        name=f"{case.method.upper()} {case.formatted_path} → {response.status_code}",
        attachment_type=allure.attachment_type.TEXT,
    )
    case.validate_response(response)
