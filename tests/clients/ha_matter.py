"""
Home Assistant client for Matter tests.

Talks to an existing Home Assistant instance (REST + WebSocket API) whose
Matter Server add-on shares L2 with the device's WiFi network:
- commission a device by pairing code (HA does mDNS discovery),
- find the resulting HA device/switch entity,
- toggle/read the switch,
- remove the device (sends RemoveFabric to the device, no reboot).

Uses the already-installed `websocket-client` and `requests` deps.
"""

from __future__ import annotations

import itertools
import json
import time

import requests
import websocket


class HAError(Exception):
    """Error response from Home Assistant."""


class HAMatterClient:
    def __init__(self, base_url: str, token: str, timeout: float = 30.0):
        self.base_url = base_url.rstrip("/")
        self._headers = {"Authorization": f"Bearer {token}"}
        self._http = requests.Session()
        self._http.headers.update(self._headers)
        self._ids = itertools.count(1)

        ws_url = self.base_url.replace("http", "ws", 1) + "/api/websocket"
        self.ws = websocket.create_connection(ws_url, timeout=timeout)
        greeting = json.loads(self.ws.recv())
        if greeting.get("type") != "auth_required":
            raise HAError(f"unexpected HA WebSocket greeting: {greeting}")
        self.ws.send(json.dumps({"type": "auth", "access_token": token}))
        reply = json.loads(self.ws.recv())
        if reply.get("type") != "auth_ok":
            raise HAError(f"HA WebSocket auth failed: {reply}")

        self.matter_entry_id = self._find_matter_entry()

    def close(self) -> None:
        self.ws.close()
        self._http.close()

    # === Low-level ===

    def ws_cmd(self, payload: dict, timeout: float = 60.0):
        message_id = next(self._ids)
        self.ws.settimeout(timeout)
        self.ws.send(json.dumps({"id": message_id, **payload}))
        deadline = time.monotonic() + timeout
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                break
            self.ws.settimeout(remaining)
            try:
                msg = json.loads(self.ws.recv())
            except websocket.WebSocketTimeoutException:
                break
            if msg.get("id") != message_id or msg.get("type") != "result":
                continue  # subscription events etc.
            if not msg.get("success"):
                raise HAError(f"{payload.get('type')}: {msg.get('error')}")
            return msg.get("result")
        raise TimeoutError(f"no reply to {payload.get('type')} within {timeout}s")

    def _find_matter_entry(self) -> str:
        for entry in self.ws_cmd({"type": "config_entries/get"}):
            if entry.get("domain") == "matter":
                return entry["entry_id"]
        raise HAError("no Matter config entry in this Home Assistant")

    # === Matter operations ===

    def commission(self, code: str, timeout: float = 120.0) -> None:
        self.ws_cmd({"type": "matter/commission", "code": code}, timeout=timeout)

    def device_by_serial(self, serial: str) -> dict | None:
        """Matter-entry device whose serial_number matches, or None."""
        for dev in self.ws_cmd({"type": "config/device_registry/list"}):
            if (
                dev.get("serial_number") == serial
                and self.matter_entry_id in dev.get("config_entries", [])
            ):
                return dev
        return None

    def switch_entity(self, device_id: str) -> str:
        """The device's OnOff entity — HA exposes the BUSY Bar as a light."""
        for domain in ("light.", "switch."):
            for ent in self.ws_cmd({"type": "config/entity_registry/list"}):
                if ent.get("device_id") == device_id and ent.get("entity_id", "").startswith(domain):
                    return ent["entity_id"]
        raise HAError(f"no light/switch entity for device {device_id}")

    def remove_device(self, device_id: str) -> None:
        self.ws_cmd({
            "type": "config/device_registry/remove_config_entry",
            "device_id": device_id,
            "config_entry_id": self.matter_entry_id,
        })

    # === Entity state (REST) ===

    def get_state(self, entity_id: str) -> str:
        r = self._http.get(f"{self.base_url}/api/states/{entity_id}", timeout=15)
        r.raise_for_status()
        return r.json()["state"]

    def set_switch(self, entity_id: str, on: bool) -> None:
        service = "turn_on" if on else "turn_off"
        # domain-agnostic service works for both light.* and switch.*
        r = self._http.post(
            f"{self.base_url}/api/services/homeassistant/{service}",
            json={"entity_id": entity_id},
            timeout=15,
        )
        r.raise_for_status()
