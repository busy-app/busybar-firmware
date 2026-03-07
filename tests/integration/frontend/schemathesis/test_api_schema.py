"""
Schemathesis tests: automatic verification of API conformance to the OpenAPI contract.

Two test groups:

1. TestGetConformance
   All GET operations. Schemathesis generates boundary values for query parameters
   and verifies that each response matches the schema. Fully safe — read-only.

2. TestSafeWriteConformance
   Explicitly allowed POST operations that do not destroy device state or break
   the HTTP session. Fewer examples are used because of side effects.

Both groups:
- Skip operations from SKIP_OPERATION_IDS (destructive, WebSocket, etc.)
- Use a limited number of examples (max_examples) — the device may become
  unstable under a high request rate
- Integrate with Allure
"""

import allure
import pytest
import schemathesis
from hypothesis import HealthCheck, settings

from .conftest import SAFE_WRITE_OPERATION_IDS, SKIP_OPERATION_IDS

# Lazy schema: resolves the 'schemathesis_schema' fixture from conftest.py
_schema = schemathesis.from_pytest_fixture("schemathesis_schema")


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _skip_if_dangerous(case: schemathesis.Case) -> None:
    """Skip the test if the operation is in the dangerous list."""
    op_id = case.operation.definition.raw.get("operationId", "")
    if op_id in SKIP_OPERATION_IDS:
        pytest.skip(f"Skipped dangerous operation: {op_id}")


def _skip_if_not_safe_write(case: schemathesis.Case) -> None:
    """Skip POST operations not in the explicit safe-write allowlist."""
    op_id = case.operation.definition.raw.get("operationId", "")
    if op_id not in SAFE_WRITE_OPERATION_IDS:
        pytest.skip(f"Not in safe-write allowlist: {op_id}")


# ---------------------------------------------------------------------------
# 1. GET conformance — all read-only operations
# ---------------------------------------------------------------------------


@allure.feature("5. Web Frontend")
@allure.story("Schema Conformance")
class TestGetConformance:
    """
    Verifies all GET endpoints: responses must conform to the OpenAPI schema.

    Schemathesis automatically:
    - generates boundary values for query/path parameters
    - validates the JSON response structure against $ref schemas
    - flags both documented and undocumented status codes
    """

    @allure.id("SCHEMA-001")
    @allure.title("GET endpoints conform to OpenAPI schema")
    @pytest.mark.schemathesis
    @pytest.mark.frontend
    @_schema.parametrize(method="GET")
    @settings(
        max_examples=10,
        suppress_health_check=[HealthCheck.too_slow, HealthCheck.filter_too_much],
        deadline=None,
    )
    def test_get_conformance(self, case: schemathesis.Case) -> None:
        """
        Each GET endpoint must:
        - return an HTTP status code documented in the schema
        - return a response body that matches the schema (when a body is documented)
        """
        _skip_if_dangerous(case)

        with allure.step(f"Call {case.method.upper()} {case.formatted_path}"):
            response = case.call()

        with allure.step(f"Validate response ({response.status_code})"):
            case.validate_response(response)


# ---------------------------------------------------------------------------
# 2. Safe write conformance — allowed POST operations
# ---------------------------------------------------------------------------


@allure.feature("5. Web Frontend")
@allure.story("Schema Conformance")
class TestSafeWriteConformance:
    """
    Verifies POST operations from SAFE_WRITE_OPERATION_IDS.

    Fewer examples are used (max_examples=5) because even safe writes have
    side effects (changing settings, sending key presses, etc.).

    What is verified:
    - 400 Bad Request on invalid input (never 500!)
    - Successful response structure matches the schema
    """

    @allure.id("SCHEMA-002")
    @allure.title("Safe write operations conform to OpenAPI schema")
    @pytest.mark.schemathesis
    @pytest.mark.frontend
    @_schema.parametrize(method="POST")
    @settings(
        max_examples=5,
        suppress_health_check=[HealthCheck.too_slow, HealthCheck.filter_too_much],
        deadline=None,
    )
    def test_post_conformance(self, case: schemathesis.Case) -> None:
        """
        POST operations from the allowlist: for any generated input the server
        must not return a 5xx response.
        """
        _skip_if_not_safe_write(case)

        with allure.step(f"Call {case.method.upper()} {case.formatted_path}"):
            response = case.call()

        with allure.step(f"Validate response ({response.status_code})"):
            case.validate_response(response)
