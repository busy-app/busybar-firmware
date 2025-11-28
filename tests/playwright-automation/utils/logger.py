"""Logging configuration for test automation."""

import logging
import sys
from datetime import datetime
from pathlib import Path
from typing import Optional

import colorlog
from rich.console import Console
from rich.logging import RichHandler

# Create logs directory
LOGS_DIR = Path("artifacts/logs")
LOGS_DIR.mkdir(parents=True, exist_ok=True)


class TestLogger:
    """Custom logger for test automation."""

    _instance: Optional["TestLogger"] = None
    _loggers = {}

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
        return cls._instance

    def __init__(self):
        if not hasattr(self, "initialized"):
            self.console = Console()
            self.log_file = LOGS_DIR / f"test_run_{datetime.now():%Y%m%d_%H%M%S}.log"
            self.initialized = True

    def get_logger(
        self,
        name: str,
        level: int = logging.INFO,
        use_color: bool = True,
        use_rich: bool = False,
    ) -> logging.Logger:
        """
        Get or create a logger with specified configuration.

        Args:
            name: Logger name (usually __name__)
            level: Logging level
            use_color: Use colored output
            use_rich: Use rich handler for better formatting

        Returns:
            Configured logger instance
        """
        if name in self._loggers:
            return self._loggers[name]

        logger = logging.getLogger(name)
        logger.setLevel(level)
        logger.handlers.clear()

        # Console handler
        if use_rich:
            console_handler = RichHandler(
                console=self.console, show_time=True, show_path=False
            )
            console_handler.setFormatter(logging.Formatter("%(message)s"))
        elif use_color:
            console_handler = colorlog.StreamHandler(sys.stdout)
            console_handler.setFormatter(
                colorlog.ColoredFormatter(
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
            )
        else:
            console_handler = logging.StreamHandler(sys.stdout)
            console_handler.setFormatter(
                logging.Formatter(
                    "%(asctime)s - %(name)s - %(levelname)s - %(message)s",
                    datefmt="%Y-%m-%d %H:%M:%S",
                )
            )

        console_handler.setLevel(level)
        logger.addHandler(console_handler)

        # File handler
        file_handler = logging.FileHandler(self.log_file, encoding="utf-8")
        file_handler.setFormatter(
            logging.Formatter(
                "%(asctime)s - %(name)s - %(levelname)s - %(funcName)s:%(lineno)d - %(message)s",
                datefmt="%Y-%m-%d %H:%M:%S",
            )
        )
        file_handler.setLevel(logging.DEBUG)  # Always log everything to file
        logger.addHandler(file_handler)

        self._loggers[name] = logger
        return logger

    def log_test_start(self, test_name: str) -> None:
        """Log test start with formatting."""
        logger = self.get_logger("TEST")
        logger.info("=" * 80)
        logger.info(f"TEST STARTED: {test_name}")
        logger.info("=" * 80)

    def log_test_end(self, test_name: str, status: str, duration: float) -> None:
        """Log test end with formatting."""
        logger = self.get_logger("TEST")
        logger.info("-" * 80)
        logger.info(f"TEST {status.upper()}: {test_name} (Duration: {duration:.2f}s)")
        logger.info("=" * 80)

    def log_step(self, step_name: str, level: int = 1) -> None:
        """Log test step with indentation."""
        logger = self.get_logger("STEP")
        indent = "  " * (level - 1)
        logger.info(f"{indent}▶ {step_name}")

    def log_assertion(self, message: str, passed: bool = True) -> None:
        """Log assertion result."""
        logger = self.get_logger("ASSERT")
        if passed:
            logger.info(f"✓ {message}")
        else:
            logger.error(f"✗ {message}")

    def log_action(self, action: str, target: str = None) -> None:
        """Log UI action."""
        logger = self.get_logger("ACTION")
        if target:
            logger.info(f"🎯 {action}: {target}")
        else:
            logger.info(f"🎯 {action}")

    def log_network(self, method: str, url: str, status: int = None) -> None:
        """Log network request."""
        logger = self.get_logger("NETWORK")
        if status:
            logger.debug(f"🌐 {method} {url} -> {status}")
        else:
            logger.debug(f"🌐 {method} {url}")

    def log_error(self, error: Exception, context: str = None) -> None:
        """Log error with context."""
        logger = self.get_logger("ERROR")
        if context:
            logger.error(f"❌ Error in {context}: {str(error)}")
        else:
            logger.error(f"❌ Error: {str(error)}")
        logger.debug(f"Stack trace:", exc_info=True)

    def get_log_file_path(self) -> Path:
        """Get current log file path."""
        return self.log_file


# Singleton instance
test_logger = TestLogger()


# Convenience functions
def get_logger(name: str = __name__) -> logging.Logger:
    """Get logger for module."""
    return test_logger.get_logger(name)


def log_test_start(test_name: str) -> None:
    """Log test start."""
    test_logger.log_test_start(test_name)


def log_test_end(test_name: str, status: str, duration: float) -> None:
    """Log test end."""
    test_logger.log_test_end(test_name, status, duration)


def log_step(step_name: str, level: int = 1) -> None:
    """Log test step."""
    test_logger.log_step(step_name, level)


def log_action(action: str, target: str = None) -> None:
    """Log UI action."""
    test_logger.log_action(action, target)


def log_error(error: Exception, context: str = None) -> None:
    """Log error."""
    test_logger.log_error(error, context)
