"""Power settings API client and response model."""

from typing import Literal

from pydantic import BaseModel, Field, model_validator

from .base import BaseAPI
from .system import ResultResponse


class SleepSettings(BaseModel):
    """Sleep policy; delayed sleep requires a holdoff in milliseconds."""

    sleep: Literal["never", "after_holdoff"]
    sleep_holdoff: int | None = Field(default=None, ge=0)

    @model_validator(mode="after")
    def validate_holdoff(self):
        if self.sleep == "after_holdoff" and self.sleep_holdoff is None:
            raise ValueError("after_holdoff requires sleep_holdoff")
        return self


class PowerAPI(BaseAPI):
    """Read and update the OFF-mode sleep policy."""

    def get_sleep(self) -> SleepSettings:
        return self.get("/api/power/sleep", SleepSettings)

    def set_sleep(self, settings: SleepSettings) -> ResultResponse:
        return self.post(
            "/api/power/sleep",
            ResultResponse,
            json=settings.model_dump(exclude_none=True),
        )
