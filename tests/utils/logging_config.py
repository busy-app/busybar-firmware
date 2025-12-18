"""
Logging configuration for BSB Test Automation
"""

import logging
import os
import sys
from datetime import datetime
from pathlib import Path

import colorlog
import structlog


def setup_logging(log_level: str = "INFO", log_to_file: bool = True) -> logging.Logger:
    """
    Setup structured logging for the test framework

    Args:
        log_level: Logging level (DEBUG, INFO, WARNING, ERROR)
        log_to_file: Whether to also log to file

    Returns:
        Configured logger instance
    """

    # Create logs directory
    logs_dir = Path("logs")
    logs_dir.mkdir(exist_ok=True)

    # Configure structlog
    structlog.configure(
        processors=[
            structlog.contextvars.merge_contextvars,
            structlog.processors.add_log_level,
            structlog.processors.TimeStamper(fmt="iso"),
            structlog.dev.set_exc_info,
            structlog.processors.JSONRenderer()
            if log_to_file
            else structlog.dev.ConsoleRenderer(),
        ],
        wrapper_class=structlog.make_filtering_bound_logger(
            getattr(logging, log_level.upper())
        ),
        logger_factory=structlog.WriteLoggerFactory(),
        cache_logger_on_first_use=True,
    )

    # Setup standard logging
    root_logger = logging.getLogger()
    root_logger.setLevel(getattr(logging, log_level.upper()))

    # Clear any existing handlers
    root_logger.handlers.clear()

    # Console handler with colors
    console_handler = colorlog.StreamHandler(sys.stdout)
    console_handler.setLevel(getattr(logging, log_level.upper()))

    console_format = colorlog.ColoredFormatter(
        "%(log_color)s%(asctime)s - %(name)s - %(levelname)s - %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
        log_colors={
            "DEBUG": "cyan",
            "INFO": "green",
            "WARNING": "yellow",
            "ERROR": "red",
            "CRITICAL": "red,bg_white",
        },
    )
    console_handler.setFormatter(console_format)
    root_logger.addHandler(console_handler)

    # File handler (if enabled)
    if log_to_file:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        log_file = logs_dir / f"bsb_tests_{timestamp}.log"

        file_handler = logging.FileHandler(log_file)
        file_handler.setLevel(logging.DEBUG)  # Always DEBUG level for file

        file_format = logging.Formatter(
            "%(asctime)s - %(name)s - %(levelname)s - %(filename)s:%(lineno)d - %(message)s",
            datefmt="%Y-%m-%d %H:%M:%S",
        )
        file_handler.setFormatter(file_format)
        root_logger.addHandler(file_handler)

    # Setup specific loggers

    # Reduce noise from external libraries
    logging.getLogger("urllib3").setLevel(logging.WARNING)
    logging.getLogger("requests").setLevel(logging.WARNING)
    logging.getLogger("asyncio").setLevel(logging.WARNING)

    # Create main logger for the framework
    logger = logging.getLogger("bsb_automation")

    return logger


def get_test_logger(test_name: str) -> logging.Logger:
    """Get a logger for a specific test"""
    return logging.getLogger(f"bsb_automation.test.{test_name}")


def get_cli_logger() -> logging.Logger:
    """Get logger for CLI operations"""
    return logging.getLogger("bsb_automation.cli")


def get_web_logger() -> logging.Logger:
    """Get logger for web operations"""
    return logging.getLogger("bsb_automation.web")


def get_setup_logger() -> logging.Logger:
    """Get logger for setup operations"""
    return logging.getLogger("bsb_automation.setup")


# Context manager for test logging
class TestLogContext:
    """Context manager to add test context to logs"""

    def __init__(self, test_name: str, test_id: str = None):
        self.test_name = test_name
        self.test_id = test_id
        self.logger = get_test_logger(test_name)

    def __enter__(self):
        context = {"test_name": self.test_name}
        if self.test_id:
            context["test_id"] = self.test_id

        structlog.contextvars.clear_contextvars()
        structlog.contextvars.bind_contextvars(**context)

        self.logger.info("Test started", test_name=self.test_name, test_id=self.test_id)
        return self.logger

    def __exit__(self, exc_type, exc_val, exc_tb):
        if exc_type:
            self.logger.error(
                "Test failed",
                test_name=self.test_name,
                error=str(exc_val),
                exc_info=True,
            )
        else:
            self.logger.info("Test completed", test_name=self.test_name)

        structlog.contextvars.clear_contextvars()


# Logging utilities for tests
def log_cli_command(command: str, response: str, duration: float = None):
    """Log CLI command execution"""
    logger = get_cli_logger()

    response_preview = response[:100] + "..." if len(response) > 100 else response
    duration_str = f", took {round(duration * 1000, 2)}ms" if duration else ""

    logger.info(
        f"CLI command executed: '{command}' -> {len(response)} chars{duration_str}"
    )


def log_web_request(method: str, url: str, status_code: int, duration: float = None):
    """Log web request"""
    logger = get_web_logger()

    duration_str = f", took {round(duration * 1000, 2)}ms" if duration else ""
    level = "info" if 200 <= status_code < 400 else "warning"

    getattr(logger, level)(
        f"Web request: {method} {url} -> {status_code}{duration_str}"
    )


def log_test_step(step_name: str, **kwargs):
    """Log test step with context"""
    logger = logging.getLogger("bsb_automation.test_step")
    logger.info(f"Step: {step_name}")


def log_assertion(description: str, passed: bool, expected=None, actual=None):
    """Log test assertion"""
    logger = logging.getLogger("bsb_automation.assertion")

    status = "PASSED" if passed else "FAILED"
    expected_str = f", expected: {expected}" if expected is not None else ""
    actual_str = f", actual: {actual}" if actual is not None else ""

    level = "info" if passed else "error"
    getattr(logger, level)(
        f"Assertion {status}: {description}{expected_str}{actual_str}"
    )
