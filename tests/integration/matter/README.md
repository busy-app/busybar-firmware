# Matter integration tests

BusyBar is commissioned over Matter into an **existing Home Assistant**
instance; the "lamp" is the BUSY Bar OnOff switch asserted from both sides
(HA switch service ↔ device `/api/smart_home/switch`).

## Topology (why HA and not a local controller)

The device advertises Matter (`_matterc._udp`, IPv6-only) **over WiFi only** —
not over the USB Ethernet link. A Matter controller must therefore share L2
with the device's WiFi network. On the bench that is the HA box on the device
WiFi LAN (its Matter Server add-on does mDNS discovery natively); the test
process itself only needs HTTP/WS access to HA — routed is fine.

Verified impossible alternatives (2026-07-17): controller on the
bsb-test-runner VM — b1-iot is routed, not L2-adjacent; no VLAN tags reach the
VM; the firmware Matter stack ignores IPv4 (direct-IP PASE over IPv4 times out
even on shared L2) and unicast mDNS.

## Env

```bash
export HA_URL=http://10.46.21.151        # unset → suite skips
export HA_TOKEN=<long-lived access token>
export WIFI_SSID=... WIFI_PASSWORD=... WIFI_SECURITY=WPA2   # device WiFi
pytest integration/matter/ -v
```

The HA user behind the token must be admin (device removal). The HA Matter
Server add-on must have **`enable_test_net_dcl: true`** — without it HA
rejects devices with test/development certificates (all bench BusyBars). The fixture
self-heals a leftover registration: if a device with our serial is already in
the HA Matter registry (previous aborted run), it is removed first — only the
device under test is ever touched.

Cleanup is controller-side (device removal → RemoveFabric on the device, no
reboot). `DELETE /api/smart_home/pairing` is intentionally not used — it
requires a reboot.
