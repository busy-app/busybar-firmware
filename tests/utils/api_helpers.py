"""
API Helper utilities for BSB Test Automation.

Provides common operations for API testing:
- Response validation
- Request builders
- Status checking and conditional skipping
"""

from __future__ import annotations

import json
from typing import Any, Callable

import allure
import pytest
import requests


class APIResponse:
    """Wrapper around requests.Response with validation helpers."""

    def __init__(self, response: requests.Response):
        self._response = response
        self._json_cache: dict | None = None

    @property
    def response(self) -> requests.Response:
        """Access the underlying requests.Response object."""
        return self._response

    @property
    def status_code(self) -> int:
        return self._response.status_code

    @property
    def headers(self) -> dict:
        return dict(self._response.headers)

    @property
    def content_type(self) -> str:
        return self._response.headers.get("content-type", "").lower()

    def json(self) -> dict:
        """Parse and cache JSON response."""
        if self._json_cache is None:
            self._json_cache = self._response.json()
        return self._json_cache

    def text(self) -> str:
        return self._response.text

    # Validation methods

    def assert_status(self, expected: int | list[int], msg: str = None) -> "APIResponse":
        """Assert response status code matches expected value(s)."""
        expected_list = [expected] if isinstance(expected, int) else expected
        actual = self.status_code
        if actual not in expected_list:
            error_msg = msg or f"Expected status {expected}, got {actual}"
            raise AssertionError(error_msg)
        return self

    def assert_ok(self) -> "APIResponse":
        """Assert response status is 200 OK."""
        return self.assert_status(200)

    def assert_created(self) -> "APIResponse":
        """Assert response status is 201 Created."""
        return self.assert_status(201)

    def assert_no_content(self) -> "APIResponse":
        """Assert response status is 204 No Content."""
        return self.assert_status(204)

    def assert_bad_request(self) -> "APIResponse":
        """Assert response status is 400 Bad Request."""
        return self.assert_status(400)

    def assert_not_found(self) -> "APIResponse":
        """Assert response status is 404 Not Found."""
        return self.assert_status(404)

    def assert_json_content_type(self) -> "APIResponse":
        """Assert response has JSON content type."""
        if "application/json" not in self.content_type:
            raise AssertionError(
                f"Expected JSON content type, got '{self.content_type}'"
            )
        return self

    def assert_has_fields(self, *fields: str) -> "APIResponse":
        """Assert JSON response contains specified fields."""
        data = self.json()
        missing = [f for f in fields if f not in data]
        if missing:
            raise AssertionError(
                f"Response missing required fields: {missing}. Got: {list(data.keys())}"
            )
        return self

    def assert_field_value(
        self, field: str, expected: Any, msg: str = None
    ) -> "APIResponse":
        """Assert a specific field has expected value."""
        data = self.json()
        if field not in data:
            raise AssertionError(f"Field '{field}' not found in response")
        actual = data[field]
        if actual != expected:
            error_msg = msg or f"Field '{field}': expected {expected!r}, got {actual!r}"
            raise AssertionError(error_msg)
        return self

    def assert_field_in(
        self, field: str, valid_values: list[Any], msg: str = None
    ) -> "APIResponse":
        """Assert a field value is one of the valid values."""
        data = self.json()
        if field not in data:
            raise AssertionError(f"Field '{field}' not found in response")
        actual = data[field]
        if actual not in valid_values:
            error_msg = msg or f"Field '{field}': {actual!r} not in {valid_values}"
            raise AssertionError(error_msg)
        return self

    def assert_field_type(
        self, field: str, expected_type: type | tuple[type, ...]
    ) -> "APIResponse":
        """Assert a field has the expected type."""
        data = self.json()
        if field not in data:
            raise AssertionError(f"Field '{field}' not found in response")
        actual = data[field]
        if not isinstance(actual, expected_type):
            raise AssertionError(
                f"Field '{field}': expected type {expected_type}, got {type(actual)}"
            )
        return self

    def get_field(self, field: str, default: Any = None) -> Any:
        """Get a field value from JSON response."""
        return self.json().get(field, default)

    def attach_to_allure(self, name: str = "Response") -> "APIResponse":
        """Attach JSON response to Allure report."""
        try:
            data = self.json()
            allure.attach(
                json.dumps(data, indent=2),
                name=name,
                attachment_type=allure.attachment_type.JSON,
            )
        except (ValueError, json.JSONDecodeError):
            allure.attach(
                self.text(),
                name=name,
                attachment_type=allure.attachment_type.TEXT,
            )
        return self


def api_get(
    session: requests.Session,
    base_url: str,
    endpoint: str,
    timeout: int = 10,
    **kwargs,
) -> APIResponse:
    """Make a GET request and return wrapped response."""
    url = f"{base_url}{endpoint}"
    response = session.get(url, timeout=timeout, **kwargs)
    return APIResponse(response)


def api_post(
    session: requests.Session,
    base_url: str,
    endpoint: str,
    timeout: int = 10,
    **kwargs,
) -> APIResponse:
    """Make a POST request and return wrapped response."""
    url = f"{base_url}{endpoint}"
    response = session.post(url, timeout=timeout, **kwargs)
    return APIResponse(response)


def api_put(
    session: requests.Session,
    base_url: str,
    endpoint: str,
    timeout: int = 10,
    **kwargs,
) -> APIResponse:
    """Make a PUT request and return wrapped response."""
    url = f"{base_url}{endpoint}"
    response = session.put(url, timeout=timeout, **kwargs)
    return APIResponse(response)


def api_delete(
    session: requests.Session,
    base_url: str,
    endpoint: str,
    timeout: int = 10,
    **kwargs,
) -> APIResponse:
    """Make a DELETE request and return wrapped response."""
    url = f"{base_url}{endpoint}"
    response = session.delete(url, timeout=timeout, **kwargs)
    return APIResponse(response)


def api_options(
    session: requests.Session,
    base_url: str,
    endpoint: str,
    timeout: int = 10,
    **kwargs,
) -> APIResponse:
    """Make an OPTIONS request and return wrapped response."""
    url = f"{base_url}{endpoint}"
    response = session.options(url, timeout=timeout, **kwargs)
    return APIResponse(response)


def skip_unless_status(
    session: requests.Session,
    base_url: str,
    endpoint: str,
    field: str,
    required_values: list[Any],
    reason_template: str = "Device {field} is {actual}, required: {required}",
) -> dict:
    """
    Check device status and skip test if not in required state.

    Args:
        session: API session
        base_url: Base URL for API
        endpoint: Status endpoint to check
        field: Field name to check in response
        required_values: List of acceptable values
        reason_template: Skip reason template with {field}, {actual}, {required}

    Returns:
        The status response data

    Raises:
        pytest.skip: If device is not in required state
    """
    response = api_get(session, base_url, endpoint)
    response.assert_ok()
    data = response.json()

    actual = data.get(field)
    if actual not in required_values:
        reason = reason_template.format(
            field=field, actual=actual, required=required_values
        )
        pytest.skip(reason)

    return data


class APITestContext:
    """
    Context manager for API tests with automatic cleanup.

    Example:
        with APITestContext(session, base_url) as ctx:
            ctx.upload_asset("/api/assets", "test.png", content)
            # ... use asset ...
        # Cleanup happens automatically
    """

    def __init__(self, session: requests.Session, base_url: str):
        self.session = session
        self.base_url = base_url
        self._cleanup_actions: list[Callable[[], None]] = []

    def __enter__(self) -> "APITestContext":
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        for action in reversed(self._cleanup_actions):
            try:
                action()
            except Exception:
                pass  # Best effort cleanup
        return False

    def add_cleanup(self, action: Callable[[], None]) -> None:
        """Register a cleanup action to run on exit."""
        self._cleanup_actions.append(action)

    def upload_asset(
        self,
        endpoint: str,
        filename: str,
        content: bytes,
        params: dict = None,
    ) -> APIResponse:
        """Upload an asset and register cleanup."""
        response = api_post(
            self.session,
            self.base_url,
            endpoint,
            params={"path": filename, **(params or {})},
            data=content,
        )

        if response.status_code in (200, 201):
            self.add_cleanup(
                lambda: api_delete(
                    self.session, self.base_url, endpoint, params={"path": filename}
                )
            )

        return response

    def get(self, endpoint: str, **kwargs) -> APIResponse:
        return api_get(self.session, self.base_url, endpoint, **kwargs)

    def post(self, endpoint: str, **kwargs) -> APIResponse:
        return api_post(self.session, self.base_url, endpoint, **kwargs)

    def put(self, endpoint: str, **kwargs) -> APIResponse:
        return api_put(self.session, self.base_url, endpoint, **kwargs)

    def delete(self, endpoint: str, **kwargs) -> APIResponse:
        return api_delete(self.session, self.base_url, endpoint, **kwargs)
