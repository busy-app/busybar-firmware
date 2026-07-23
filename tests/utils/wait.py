"""Generic poll-until-condition helper shared by tests and harnesses."""

from __future__ import annotations

import time
from typing import Any, Callable


def wait_for(
    description: str,
    getter: Callable[[], Any],
    predicate: Callable[[Any], bool],
    *,
    timeout: float,
    interval: float = 1.0,
) -> Any:
    """Poll a getter until its result satisfies predicate, preserving context."""
    deadline = time.monotonic() + timeout
    last_value: Any = None
    last_error: Exception | None = None
    while time.monotonic() < deadline:
        try:
            last_value = getter()
            last_error = None
            if predicate(last_value):
                return last_value
        except Exception as exc:  # polling must tolerate services restarting
            last_error = exc
        time.sleep(interval)

    if last_error is not None:
        raise AssertionError(
            f"Timed out waiting for {description}; last error: {last_error}"
        ) from last_error
    raise AssertionError(
        f"Timed out waiting for {description}; last value: {last_value!r}"
    )
