"""Generic poll-until-condition helper shared by tests and harnesses."""

from __future__ import annotations

import time
from typing import Any, Callable, TypeVar


_T = TypeVar("_T")


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


def wait_for_stable(
    description: str,
    getter: Callable[[], _T],
    identity: Callable[[_T], Any],
    *,
    predicate: Callable[[_T], bool] | None = None,
    stable_samples: int = 2,
    timeout: float,
    interval: float = 1.0,
) -> _T:
    """Poll until an accepted value has the same identity repeatedly."""
    if stable_samples < 1:
        raise ValueError("stable_samples must be at least 1")

    unset = object()
    previous_identity: Any = unset
    observed_stable_samples = 0

    def is_stable(value: _T) -> bool:
        nonlocal previous_identity, observed_stable_samples

        if predicate is not None and not predicate(value):
            previous_identity = unset
            observed_stable_samples = 0
            return False

        current_identity = identity(value)
        if previous_identity is not unset and current_identity == previous_identity:
            observed_stable_samples += 1
        else:
            previous_identity = current_identity
            observed_stable_samples = 1

        return observed_stable_samples >= stable_samples

    return wait_for(
        description,
        getter,
        is_stable,
        timeout=timeout,
        interval=interval,
    )
