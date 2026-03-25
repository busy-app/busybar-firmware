"""Fixtures for cloud account linking integration tests."""

import pytest
import requests

from clients.cloud.auth import CloudAuthClient
from clients.cloud.bars import CloudBarAPI
from config.config import Config


@pytest.fixture(scope="module")
def cloud_auth() -> CloudAuthClient:
    """Authenticate with cloud and return auth client."""
    if not Config.CLOUD_EMAIL or not Config.CLOUD_PASSWORD:
        pytest.skip("CLOUD_EMAIL / CLOUD_PASSWORD not configured")

    basic_auth = None
    if Config.CLOUD_BASIC_USER and Config.CLOUD_BASIC_PASSWORD:
        basic_auth = (Config.CLOUD_BASIC_USER, Config.CLOUD_BASIC_PASSWORD)

    client = CloudAuthClient(Config.CLOUD_BASE_URL, basic_auth=basic_auth)
    client.authenticate(Config.CLOUD_EMAIL, Config.CLOUD_PASSWORD)
    return client


@pytest.fixture(scope="module")
def cloud_session(cloud_auth: CloudAuthClient) -> requests.Session:
    """Authenticated requests.Session for cloud API calls."""
    return cloud_auth.get_authenticated_session()


@pytest.fixture(scope="module")
def cloud_bar_api(cloud_session: requests.Session) -> CloudBarAPI:
    """Cloud bar API client with authenticated session."""
    return CloudBarAPI(cloud_session, Config.CLOUD_BASE_URL)
