"""
Schemathesis fixtures and shared configuration.

SKIP_OPERATION_IDS: operations that must not participate in automated fuzzing
on real hardware — either destructive, session-breaking, require external
services, or use WebSocket transport.
"""

import pytest
import schemathesis

# ---------------------------------------------------------------------------
# Operations skipped during fuzzing on real hardware
# ---------------------------------------------------------------------------

SKIP_OPERATION_IDS: frozenset[str] = frozenset(
    {
        # Firmware update — could brick the device during the test run
        "installFirmwareUpdate",
        "abortFirmwareDownload",
        "updateFirmware",
        # Filesystem mutations — could corrupt FS state
        "writeStorageFile",
        "removeStorageFile",
        "createStorageDir",
        "RenameStorageFile",
        # Account operations — irreversible, require Cloud
        "unlinkAccount",
        "linkAccount",
        # Asset uploads — would overwrite application assets
        "uploadAssetWithAppId",
        "deleteAppAssets",
        # WiFi — disconnect would break the HTTP session used by the test runner
        "connectWifi",
        "disconnectWifi",
        # BLE pairing — alters pairing state
        "setBleParingMode",
        # Matter commissioning — long-running background operation
        "startMatterCommissioning",
        # WebSocket — not supported by a plain HTTP client
        "connectWebSocket",
        "connectInputWebSocket",
    }
)

# Operations explicitly allowed among POST/PUT (Safe Writes)
SAFE_WRITE_OPERATION_IDS: frozenset[str] = frozenset(
    {
        # System settings
        "setHttpAccess",
        # Display — reversible
        "setDisplayBrightness",
        "clearDisplay",
        # Audio — reversible
        "setAudioVolume",
        "stopAudio",
        # Time — reversible
        "setTimeTimestamp",
        "setTimeTimezone",
        # Account profile — local fields only
        "setAccountProfile",
        # Busy timer profiles
        "setBusyProfile",
        "setBusySnapshot",
        # Auto-update settings — configures policy only, does not trigger update
        "setAutoupdateSettings",
        # Input key — sends a button press (reversible)
        "setInputKey",
    }
)


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------


@pytest.fixture(scope="session")
def schemathesis_schema(web_base_url: str) -> schemathesis.BaseSchema:
    """
    Load the OpenAPI schema from the device.

    The schema is fetched from the running device (/openapi.yaml) rather than
    from the firmware source tree so that we test exactly what the server
    actually serves.

    validate_schema=False: the device may serve a schema with minor deviations
    from the OpenAPI 3.1 draft — we do not want schema meta-validation errors
    to abort the entire test run.
    """
    schema_url = f"{web_base_url}/openapi.yaml"
    return schemathesis.from_uri(
        schema_url,
        validate_schema=False,
    )
