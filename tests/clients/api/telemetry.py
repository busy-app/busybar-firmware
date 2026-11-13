"""Telemetry API client and Pydantic models."""

from __future__ import annotations

import requests
from pydantic import BaseModel, ConfigDict, StrictBool

from .base import BaseAPI


class TelemetryStatus(BaseModel):
    """Response and request body for ``/api/telemetry``."""

    model_config = ConfigDict(extra="forbid")

    enabled: StrictBool


class TelemetryAPI(BaseAPI):
    """Typed client for the telemetry collection setting."""

    def get_status(self) -> TelemetryStatus:
        return self.get("/api/telemetry", TelemetryStatus)

    def set_enabled(self, enabled: bool) -> TelemetryStatus:
        body = TelemetryStatus(enabled=enabled)
        return self.put(
            "/api/telemetry",
            TelemetryStatus,
            json=body.model_dump(),
        )

    def set_enabled_raw(self, **kwargs) -> requests.Response:
        return self.put_raw("/api/telemetry", **kwargs)
