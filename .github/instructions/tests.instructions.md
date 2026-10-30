---
applyTo: "tests/**"
---

# Integration test review rules

Read `tests/AGENTS.md` first; it describes the environment, layout and fixtures.

- Import the existing Pydantic models from `clients/api` instead of building request and response dicts by hand.
- Use the provided fixtures (`web_session`, `api_session`, the typed `*_api` fixtures, `persistent_cli_connection`) rather than creating clients or raw sockets ad hoc.
- Do not weaken the autouse fixtures (`device_health_monitor`, `skip_hello_screen`) or swallow their failures, and do not hide failures with a blanket `except Exception`.
- Avoid fixed `sleep` calls; use the existing timing helpers so tests stay deterministic.
- Tests run against a real device on dedicated CI runners. Review for correctness, cleanup and isolation, and do not claim that a test passes: there is no device, broker or emulator in this review runtime.
