import pytest

from clients.api import SettingsAPI


@pytest.fixture(autouse=True)
def preserve_settings(settings_api: SettingsAPI):
    """Save and restore device settings around each test."""
    name = settings_api.get_name().name
    access = settings_api.get_access()
    brightness = settings_api.get_brightness().value
    volume = settings_api.get_volume().volume

    yield

    settings_api.set_name(name)
    # When mode is "key", we can't restore the key (GET doesn't return it).
    # Fall back to "enabled" to keep some access protection.
    if access.mode == "key":
        settings_api.set_access("enabled")
    else:
        settings_api.set_access(access.mode)
    settings_api.set_brightness(value=brightness)
    settings_api.set_volume(int(volume))
