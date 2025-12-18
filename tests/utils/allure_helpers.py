"""
Allure helper utilities for BSB Test Automation.

Provides wrappers and utilities for Allure reporting integration.
"""

from __future__ import annotations

import json
from contextlib import contextmanager
from functools import wraps
from typing import Any, Callable

import allure


def attach_json(data: Any, name: str = "Response") -> None:
    """
    Attach JSON data to Allure report.

    Args:
        data: Data to serialize and attach (dict, list, or JSON string)
        name: Attachment name
    """
    if isinstance(data, str):
        try:
            json.loads(data)
            json_str = data
        except json.JSONDecodeError:
            json_str = json.dumps({"raw": data}, indent=2)
    else:
        json_str = json.dumps(data, indent=2, default=str)

    allure.attach(json_str, name=name, attachment_type=allure.attachment_type.JSON)


def attach_text(text: str, name: str = "Output") -> None:
    """
    Attach text to Allure report.

    Args:
        text: Text content to attach
        name: Attachment name
    """
    allure.attach(text, name=name, attachment_type=allure.attachment_type.TEXT)


def attach_html(html: str, name: str = "HTML") -> None:
    """
    Attach HTML content to Allure report.

    Args:
        html: HTML content to attach
        name: Attachment name
    """
    allure.attach(html, name=name, attachment_type=allure.attachment_type.HTML)


def attach_image(data: bytes, name: str = "Image", image_type: str = "png") -> None:
    """
    Attach image to Allure report.

    Args:
        data: Image binary data
        name: Attachment name
        image_type: Image type (png, jpg, gif, etc.)
    """
    type_map = {
        "png": allure.attachment_type.PNG,
        "jpg": allure.attachment_type.JPG,
        "jpeg": allure.attachment_type.JPG,
        "gif": allure.attachment_type.GIF,
        "svg": allure.attachment_type.SVG,
    }
    attachment_type = type_map.get(image_type.lower(), allure.attachment_type.PNG)
    allure.attach(data, name=name, attachment_type=attachment_type)


def attach_request(
    method: str,
    url: str,
    headers: dict = None,
    body: Any = None,
    name: str = "Request",
) -> None:
    """
    Attach HTTP request details to Allure report.

    Args:
        method: HTTP method
        url: Request URL
        headers: Request headers
        body: Request body
        name: Attachment name
    """
    request_info = {
        "method": method,
        "url": url,
    }
    if headers:
        request_info["headers"] = dict(headers)
    if body:
        request_info["body"] = body

    attach_json(request_info, name=name)


def attach_response(
    status_code: int,
    headers: dict = None,
    body: Any = None,
    name: str = "Response",
) -> None:
    """
    Attach HTTP response details to Allure report.

    Args:
        status_code: Response status code
        headers: Response headers
        body: Response body
        name: Attachment name
    """
    response_info = {"status_code": status_code}
    if headers:
        response_info["headers"] = dict(headers)
    if body:
        response_info["body"] = body

    attach_json(response_info, name=name)


@contextmanager
def step(name: str, attach_result: bool = False):
    """
    Context manager for Allure steps with optional result attachment.

    Args:
        name: Step name
        attach_result: Whether to attach the yielded result

    Yields:
        Dict for storing step data that can be attached

    Example:
        with step("Verify response", attach_result=True) as data:
            data["status"] = response.status_code
            data["fields"] = list(response.json().keys())
    """
    step_data = {}
    with allure.step(name):
        yield step_data
        if attach_result and step_data:
            attach_json(step_data, name=f"{name} - Result")


def step_decorator(name: str = None):
    """
    Decorator to wrap a function as an Allure step.

    Args:
        name: Step name (defaults to function name)

    Example:
        @step_decorator("Validate API response")
        def validate_response(response):
            assert response.status_code == 200
    """

    def decorator(func: Callable) -> Callable:
        step_name = name or func.__name__.replace("_", " ").title()

        @wraps(func)
        def wrapper(*args, **kwargs):
            with allure.step(step_name):
                return func(*args, **kwargs)

        return wrapper

    return decorator


def parametrize_with_ids(argnames: str, argvalues: list, id_func: Callable = None):
    """
    Wrapper around pytest.mark.parametrize with automatic ID generation.

    Args:
        argnames: Comma-separated argument names
        argvalues: List of argument values
        id_func: Optional function to generate test IDs

    Example:
        @parametrize_with_ids("value", ["a", "b", "c"])
        def test_values(value):
            ...
    """
    import pytest

    if id_func:
        ids = [id_func(v) for v in argvalues]
    else:
        ids = [str(v) if not isinstance(v, tuple) else "-".join(str(x) for x in v) for v in argvalues]

    return pytest.mark.parametrize(argnames, argvalues, ids=ids)


class AllureTestCase:
    """
    Mixin class for test classes with common Allure operations.

    Example:
        class TestAPI(AllureTestCase):
            def test_endpoint(self, api_session, web_base_url):
                response = api_session.get(f"{web_base_url}/api/status")
                self.attach_response(response)
    """

    def attach_response(self, response, name: str = "Response") -> None:
        """Attach requests.Response to Allure report."""
        try:
            body = response.json()
        except (ValueError, AttributeError):
            body = getattr(response, "text", str(response))

        attach_response(
            status_code=getattr(response, "status_code", 0),
            headers=dict(getattr(response, "headers", {})),
            body=body,
            name=name,
        )

    def attach_json(self, data: Any, name: str = "Data") -> None:
        """Attach JSON data to Allure report."""
        attach_json(data, name=name)

    def attach_text(self, text: str, name: str = "Output") -> None:
        """Attach text to Allure report."""
        attach_text(text, name=name)

    def step(self, name: str):
        """Create an Allure step context."""
        return allure.step(name)
