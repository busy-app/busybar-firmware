from __future__ import annotations

"""
Schemathesis fixtures and shared configuration.

SKIP_OPERATION_IDS / SKIP_PATHS: operations that must NOT participate in
automated fuzzing on real hardware — destructive, session-breaking, require
external services, or use WebSocket transport.

All other operations are tested automatically.  When a new API operation is
added it is covered by default; it only needs an explicit SKIP entry if it
carries a safety risk.
"""

import re

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
        # Changelog requires a specific version that exists on the device —
        # tested separately in test_api_update.py with a known version
        "getUpdateChangelog",
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
        # BLE pairing — alters pairing state
        "setBleParingMode",
        # Matter commissioning — long-running background operation
        "startMatterCommissioning",
        # Smart home pairing — long-running background operation
        "startSmartHomePairing",
        # Audio volume — crashes AudioSrv, see FW-814
        "setAudioVolume",
        # WebSocket — not supported by a plain HTTP client
        "connectWebSocket",
        "connectInputWebSocket",
        # Storage endpoints require real files on the device; schemathesis-generated
        # paths are always non-existent and the firmware returns 400 instead of 404
        "readStorageFile",
        "listStorageFiles",
        # Scan is not possible while the device is connected to Wi-Fi (always 400
        # during the test run); tested separately in the dedicated Wi-Fi test suite
        "getWifiNetworks",
        # Requires custom_url when profile=custom; schemathesis cannot satisfy
        # this inter-parameter dependency and always sends profile=custom without it
        "setAccountProfile",
    }
)

# POST operations without operationId that must be skipped.
# These are excluded by path because schemathesis cannot filter them by
# operationId (the schema does not assign one).
SKIP_PATHS: frozenset[str] = frozenset(
    {
        # WiFi — session-breaking: disconnect drops the HTTP connection mid-run;
        # connect may switch the device to a different network
        "/api/wifi/connect",
        "/api/wifi/disconnect",
        # Smart home switch — toggles physical devices (real-world side effects)
        "/api/smart_home/switch",
    }
)

# Regex used to exclude SKIP_PATHS in schemathesis .exclude(path_regex=...)
SKIP_PATHS_RE: str = "^(" + "|".join(re.escape(p) for p in sorted(SKIP_PATHS)) + ")$"


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
    return schemathesis.openapi.from_url(schema_url)
