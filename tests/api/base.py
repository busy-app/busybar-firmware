"""
Base API client with HTTP methods, logging, and Allure integration.

All domain-specific API clients inherit from BaseAPI to get:
- Automatic request/response logging
- Allure step wrapping
- Pydantic response validation
- Consistent error handling
"""

from __future__ import annotations

import json
import logging
from typing import Any, Type, TypeVar

import allure
import requests
from pydantic import BaseModel

T = TypeVar("T", bound=BaseModel)


class APIError(Exception):
    """Exception for API errors with response details."""

    def __init__(self, response: requests.Response, message: str = None):
        self.response = response
        self.status_code = response.status_code
        self.message = message or f"API error: {response.status_code}"
        super().__init__(self.message)


class BaseAPI:
    """
    Base API client with HTTP methods, logging, and Allure integration.

    All domain-specific API clients should inherit from this class.

    Example:
        class SystemAPI(BaseAPI):
            def get_version(self) -> VersionResponse:
                return self.get("/api/version", VersionResponse)
    """

    def __init__(self, session: requests.Session, base_url: str):
        self.session = session
        self.base_url = base_url.rstrip("/")
        self.logger = logging.getLogger(self.__class__.__name__)

    def _request(
        self,
        method: str,
        endpoint: str,
        response_model: Type[T] = None,
        step_name: str = None,
        timeout: int = 10,
        **kwargs,
    ) -> T | dict | requests.Response:
        """
        Make HTTP request with automatic logging and Allure step.

        Args:
            method: HTTP method (GET, POST, PUT, DELETE, OPTIONS)
            endpoint: API endpoint (e.g., "/api/version")
            response_model: Optional Pydantic model for response validation
            step_name: Custom step name for Allure report
            timeout: Request timeout in seconds
            **kwargs: Additional arguments passed to requests

        Returns:
            Validated Pydantic model if response_model provided,
            otherwise dict from JSON response or raw Response for errors.
        """
        url = f"{self.base_url}{endpoint}"
        step = step_name or f"{method.upper()} {endpoint}"

        with allure.step(step):
            self.logger.debug(f"{method.upper()} {url}")

            # Attach request to Allure
            self._attach_request(method, url, **kwargs)

            response = self.session.request(
                method, url, timeout=timeout, **kwargs
            )

            self._attach_response(response)
            self.logger.debug(f"Response: {response.status_code}")

            if response_model:
                return self._validate(response, response_model)

            if response.ok:
                try:
                    return response.json()
                except json.JSONDecodeError:
                    return response
            return response

    def get(
        self,
        endpoint: str,
        model: Type[T] = None,
        step_name: str = None,
        **kwargs,
    ) -> T | dict | requests.Response:
        """Make GET request."""
        return self._request("GET", endpoint, model, step_name, **kwargs)

    def post(
        self,
        endpoint: str,
        model: Type[T] = None,
        step_name: str = None,
        **kwargs,
    ) -> T | dict | requests.Response:
        """Make POST request."""
        return self._request("POST", endpoint, model, step_name, **kwargs)

    def put(
        self,
        endpoint: str,
        model: Type[T] = None,
        step_name: str = None,
        **kwargs,
    ) -> T | dict | requests.Response:
        """Make PUT request."""
        return self._request("PUT", endpoint, model, step_name, **kwargs)

    def delete(
        self,
        endpoint: str,
        model: Type[T] = None,
        step_name: str = None,
        **kwargs,
    ) -> T | dict | requests.Response:
        """Make DELETE request."""
        return self._request("DELETE", endpoint, model, step_name, **kwargs)

    def options(
        self,
        endpoint: str,
        model: Type[T] = None,
        step_name: str = None,
        **kwargs,
    ) -> T | dict | requests.Response:
        """Make OPTIONS request."""
        return self._request("OPTIONS", endpoint, model, step_name, **kwargs)

    def get_raw(self, endpoint: str, **kwargs) -> requests.Response:
        """
        Make GET request and return raw Response (for error testing).

        Use this method when testing error responses (400, 404, etc.)
        that shouldn't be validated against a model.
        """
        url = f"{self.base_url}{endpoint}"
        with allure.step(f"GET {endpoint} (raw)"):
            self._attach_request("GET", url, **kwargs)
            response = self.session.get(url, **kwargs)
            self._attach_response(response)
            return response

    def post_raw(self, endpoint: str, **kwargs) -> requests.Response:
        """Make POST request and return raw Response (for error testing)."""
        url = f"{self.base_url}{endpoint}"
        with allure.step(f"POST {endpoint} (raw)"):
            self._attach_request("POST", url, **kwargs)
            response = self.session.post(url, **kwargs)
            self._attach_response(response)
            return response

    def _validate(self, response: requests.Response, model: Type[T]) -> T:
        """Validate response against Pydantic model."""
        with allure.step(f"Validate response as {model.__name__}"):
            if not response.ok:
                raise APIError(
                    response,
                    f"Expected successful response, got {response.status_code}",
                )

            try:
                data = response.json()
            except json.JSONDecodeError as e:
                raise APIError(response, f"Invalid JSON response: {e}")

            return model.model_validate(data)

    def _attach_request(self, method: str, url: str, **kwargs) -> None:
        """Attach request details to Allure report."""
        request_info = {
            "method": method.upper(),
            "url": url,
        }

        # Add query parameters
        if "params" in kwargs and kwargs["params"]:
            request_info["params"] = kwargs["params"]

        # Add headers (excluding sensitive ones)
        if "headers" in kwargs and kwargs["headers"]:
            safe_headers = {
                k: v for k, v in kwargs["headers"].items()
                if k.lower() not in ("authorization", "x-api-token", "cookie")
            }
            if safe_headers:
                request_info["headers"] = safe_headers

        # Add JSON body
        if "json" in kwargs and kwargs["json"]:
            request_info["body"] = kwargs["json"]

        # Add form data (if not binary)
        if "data" in kwargs and kwargs["data"]:
            data = kwargs["data"]
            if isinstance(data, bytes):
                request_info["body"] = f"<binary data: {len(data)} bytes>"
            elif isinstance(data, str):
                request_info["body"] = data
            else:
                request_info["body"] = str(data)

        allure.attach(
            json.dumps(request_info, indent=2, default=str),
            name="Request",
            attachment_type=allure.attachment_type.JSON,
        )

    def _attach_response(self, response: requests.Response) -> None:
        """Attach response to Allure report."""
        try:
            data = response.json()
            allure.attach(
                json.dumps(data, indent=2),
                name=f"Response ({response.status_code})",
                attachment_type=allure.attachment_type.JSON,
            )
        except (json.JSONDecodeError, ValueError):
            content = response.text or "(empty response)"
            # Truncate very long responses
            if len(content) > 10000:
                content = content[:10000] + f"\n... (truncated, total {len(content)} chars)"
            allure.attach(
                content,
                name=f"Response ({response.status_code})",
                attachment_type=allure.attachment_type.TEXT,
            )

    def assert_status(
        self, response: requests.Response, expected: int | list[int]
    ) -> None:
        """Assert response has expected status code."""
        expected_list = [expected] if isinstance(expected, int) else expected
        with allure.step(f"Assert status is {expected}"):
            assert response.status_code in expected_list, (
                f"Expected status {expected}, got {response.status_code}"
            )
