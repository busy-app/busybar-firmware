"""
Utilities package for BSB Test Automation

This package provides common utilities for testing:
- Logging configuration
"""

from .logging_config import (
    TestLogContext,
    get_cli_logger,
    get_setup_logger,
    get_test_logger,
    get_web_logger,
    log_assertion,
    log_cli_command,
    log_test_step,
    log_web_request,
    setup_logging,
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
]
