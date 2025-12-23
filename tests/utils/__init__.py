"""
Utilities package for BSB Test Automation

This package provides common utilities for testing:
- Logging configuration
- API helpers for HTTP request/response handling
- Assertion helpers for common validation patterns
- Allure reporting helpers
- CLI output parsing helpers
"""

from .logging_config import (TestLogContext, get_cli_logger, get_setup_logger,
                             get_test_logger, get_web_logger, log_assertion,
                             log_cli_command, log_test_step, log_web_request,
                             setup_logging)

from .api_helpers import (
    APIResponse,
    APITestContext,
    api_delete,
    api_get,
    api_options,
    api_post,
    api_put,
    skip_unless_status,
)

from .assertions import (
    assert_all_have_fields,
    assert_contains_text,
    assert_field_in,
    assert_field_matches,
    assert_field_range,
    assert_field_type,
    assert_field_value,
    assert_has_fields,
    assert_list_length,
    assert_non_empty,
    assert_not_contains,
)

from .allure_helpers import (
    AllureTestCase,
    attach_html,
    attach_image,
    attach_json,
    attach_request,
    attach_response,
    attach_text,
    parametrize_with_ids,
    step,
    step_decorator,
)

from .cli_helpers import (
    CLIOutput,
    extract_section,
    parse_cli_output,
    parse_table_output,
)


__all__ = [
    # Logging
    "setup_logging",
    "get_test_logger",
    "get_cli_logger",
    "get_web_logger",
    "get_setup_logger",
    "TestLogContext",
    "log_cli_command",
    "log_web_request",
    "log_test_step",
    "log_assertion",
    # API helpers
    "APIResponse",
    "APITestContext",
    "api_get",
    "api_options",
    "api_post",
    "api_put",
    "api_delete",
    "skip_unless_status",
    # Assertions
    "assert_has_fields",
    "assert_field_value",
    "assert_field_in",
    "assert_field_type",
    "assert_field_matches",
    "assert_field_range",
    "assert_non_empty",
    "assert_list_length",
    "assert_all_have_fields",
    "assert_contains_text",
    "assert_not_contains",
    # Allure helpers
    "attach_json",
    "attach_text",
    "attach_html",
    "attach_image",
    "attach_request",
    "attach_response",
    "step",
    "step_decorator",
    "parametrize_with_ids",
    "AllureTestCase",
    # CLI helpers
    "CLIOutput",
    "parse_cli_output",
    "parse_table_output",
    "extract_section",
]
