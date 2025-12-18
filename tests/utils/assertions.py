"""
Common assertion helpers for BSB Test Automation.

Provides reusable assertion functions for common validation patterns.
"""

from __future__ import annotations

import re
from typing import Any, Pattern


def assert_has_fields(data: dict, *fields: str, msg: str = None) -> None:
    """
    Assert that a dictionary contains all specified fields.

    Args:
        data: Dictionary to check
        *fields: Required field names
        msg: Optional custom error message

    Raises:
        AssertionError: If any field is missing
    """
    missing = [f for f in fields if f not in data]
    if missing:
        error_msg = msg or f"Missing required fields: {missing}. Got: {list(data.keys())}"
        raise AssertionError(error_msg)


def assert_field_value(
    data: dict, field: str, expected: Any, msg: str = None
) -> None:
    """
    Assert that a dictionary field has the expected value.

    Args:
        data: Dictionary to check
        field: Field name
        expected: Expected value
        msg: Optional custom error message
    """
    if field not in data:
        raise AssertionError(f"Field '{field}' not found in data")

    actual = data[field]
    if actual != expected:
        error_msg = msg or f"Field '{field}': expected {expected!r}, got {actual!r}"
        raise AssertionError(error_msg)


def assert_field_in(
    data: dict, field: str, valid_values: list[Any], msg: str = None
) -> None:
    """
    Assert that a dictionary field value is one of the valid values.

    Args:
        data: Dictionary to check
        field: Field name
        valid_values: List of acceptable values
        msg: Optional custom error message
    """
    if field not in data:
        raise AssertionError(f"Field '{field}' not found in data")

    actual = data[field]
    if actual not in valid_values:
        error_msg = msg or f"Field '{field}': {actual!r} not in {valid_values}"
        raise AssertionError(error_msg)


def assert_field_type(
    data: dict, field: str, expected_type: type | tuple[type, ...]
) -> None:
    """
    Assert that a dictionary field has the expected type.

    Args:
        data: Dictionary to check
        field: Field name
        expected_type: Expected type or tuple of types
    """
    if field not in data:
        raise AssertionError(f"Field '{field}' not found in data")

    actual = data[field]
    if not isinstance(actual, expected_type):
        raise AssertionError(
            f"Field '{field}': expected type {expected_type}, got {type(actual).__name__}"
        )


def assert_field_matches(
    data: dict, field: str, pattern: str | Pattern, msg: str = None
) -> None:
    """
    Assert that a dictionary field matches a regex pattern.

    Args:
        data: Dictionary to check
        field: Field name
        pattern: Regex pattern (string or compiled)
        msg: Optional custom error message
    """
    if field not in data:
        raise AssertionError(f"Field '{field}' not found in data")

    actual = data[field]
    if not isinstance(actual, str):
        raise AssertionError(f"Field '{field}' is not a string: {type(actual).__name__}")

    if isinstance(pattern, str):
        pattern = re.compile(pattern)

    if not pattern.search(actual):
        error_msg = msg or f"Field '{field}': '{actual}' does not match pattern {pattern.pattern}"
        raise AssertionError(error_msg)


def assert_field_range(
    data: dict,
    field: str,
    min_value: int | float = None,
    max_value: int | float = None,
    msg: str = None,
) -> None:
    """
    Assert that a numeric field is within the specified range.

    Args:
        data: Dictionary to check
        field: Field name
        min_value: Minimum value (inclusive), None for no lower bound
        max_value: Maximum value (inclusive), None for no upper bound
        msg: Optional custom error message
    """
    if field not in data:
        raise AssertionError(f"Field '{field}' not found in data")

    actual = data[field]
    if not isinstance(actual, (int, float)):
        raise AssertionError(f"Field '{field}' is not numeric: {type(actual).__name__}")

    if min_value is not None and actual < min_value:
        error_msg = msg or f"Field '{field}': {actual} < minimum {min_value}"
        raise AssertionError(error_msg)

    if max_value is not None and actual > max_value:
        error_msg = msg or f"Field '{field}': {actual} > maximum {max_value}"
        raise AssertionError(error_msg)


def assert_non_empty(value: Any, name: str = "Value") -> None:
    """
    Assert that a value is not empty (non-zero length or truthy).

    Args:
        value: Value to check
        name: Name to use in error message
    """
    if not value:
        raise AssertionError(f"{name} should not be empty")


def assert_list_length(
    data: list,
    min_length: int = None,
    max_length: int = None,
    exact_length: int = None,
    name: str = "List",
) -> None:
    """
    Assert list length constraints.

    Args:
        data: List to check
        min_length: Minimum length (inclusive)
        max_length: Maximum length (inclusive)
        exact_length: Exact required length (overrides min/max)
        name: Name to use in error message
    """
    actual = len(data)

    if exact_length is not None:
        if actual != exact_length:
            raise AssertionError(f"{name}: expected length {exact_length}, got {actual}")
        return

    if min_length is not None and actual < min_length:
        raise AssertionError(f"{name}: length {actual} < minimum {min_length}")

    if max_length is not None and actual > max_length:
        raise AssertionError(f"{name}: length {actual} > maximum {max_length}")


def assert_all_have_fields(items: list[dict], *fields: str) -> None:
    """
    Assert that all dictionaries in a list have the specified fields.

    Args:
        items: List of dictionaries
        *fields: Required field names
    """
    for i, item in enumerate(items):
        missing = [f for f in fields if f not in item]
        if missing:
            raise AssertionError(
                f"Item {i} missing required fields: {missing}. Got: {list(item.keys())}"
            )


def assert_contains_text(text: str, *substrings: str, case_sensitive: bool = True) -> None:
    """
    Assert that text contains all specified substrings.

    Args:
        text: Text to search in
        *substrings: Required substrings
        case_sensitive: Whether comparison is case-sensitive
    """
    check_text = text if case_sensitive else text.lower()

    for substring in substrings:
        check_substring = substring if case_sensitive else substring.lower()
        if check_substring not in check_text:
            raise AssertionError(f"Text does not contain '{substring}'")


def assert_not_contains(text: str, *substrings: str, case_sensitive: bool = True) -> None:
    """
    Assert that text does not contain any of the specified substrings.

    Args:
        text: Text to search in
        *substrings: Forbidden substrings
        case_sensitive: Whether comparison is case-sensitive
    """
    check_text = text if case_sensitive else text.lower()

    for substring in substrings:
        check_substring = substring if case_sensitive else substring.lower()
        if check_substring in check_text:
            raise AssertionError(f"Text contains forbidden substring '{substring}'")
