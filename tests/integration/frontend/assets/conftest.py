import os

import pytest


def pytest_addoption(parser):
    parser.addoption(
        "--update-refs",
        action="store_true",
        default=False,
        help="Update reference screenshots instead of comparing",
    )


@pytest.fixture
def update_refs(request) -> bool:
    """True when running in reference-update mode."""
    return request.config.getoption("--update-refs") or bool(
        os.environ.get("UPDATE_REFS")
    )
