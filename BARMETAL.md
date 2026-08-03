# BarMetal

An independent, community firmware fork for the **BUSY Bar**, maintained by
[@nastea1](https://github.com/nastea1) alongside the
[BarPilot](https://github.com/nastea1/barpilot) control app.

> **Not affiliated.** BarMetal is a community fork of the
> [official BUSY Bar firmware](https://github.com/busy-app/busybar-firmware) by Flipper
> Devices. It is not affiliated with, endorsed by, or sponsored by Flipper Devices Inc.
> Upstream copyright and licensing (GPLv2-or-later for most first-party code, MIT for
> `furi`) are preserved — see `LICENSE.md`.

## Why this fork exists

BarPilot is a controller for the bar's local HTTP API. Building it surfaced device
behaviour that could not be fixed from the API side:

* the Wi-Fi subsystem could wedge **permanently** — stuck reporting `connecting`, with
  every radio command returning `Command timed out`, unrecoverable by forgetting the
  network or power-cycling, and eventually crashing the device;
* `POST /api/display/brightness` appeared to do nothing at the low end, and `0` could not
  turn the display off;
* there was **no software reboot**, so a wedged device required physical recovery — in one
  case an escape from the STM32 bootloader via `dfu-util`.

`WIFI-WEDGE-ANALYSIS.md` documents the root causes with citations into this source tree.
BarMetal fixes them.

## What BarMetal changes

| Area | Change |
|---|---|
| **Wi-Fi watchdog** | Every request dispatched to the Wi-Fi co-processor is now bounded (30 s). On expiry the caller is released, the API semaphore is freed and the state machine returns to `disconnected` — instead of the service hanging forever on `FuriWaitForever` |
| **Wi-Fi escalation** | After 3 consecutive backend timeouts the link is torn down and re-initialised, using the service's existing deinit/init paths |
| **Forget always works** | `Forget` is no longer gated on `disconnected`; the one recovery the UI offers is now available in exactly the state that needs it |
| **Brightness** | Step 0 is now genuinely **off** (front and back); the low end of the curve is spread out instead of collapsing 0–14 % onto one value. Auto-brightness is clamped to ≥ 1 step so a dark room never blanks the display |
| **Software reboot** | New `POST /api/system/reboot?target=all\|mcu\|wifi`. `wifi` restarts only the radio — remote recovery for a wedged co-processor without disturbing the main MCU |

## Compatibility

BarMetal tracks upstream and keeps the HTTP API compatible: everything the official
firmware serves still works, and BarPilot (or any other client) runs unmodified against
it. The only API addition is `/api/system/reboot`.

## Building

Same as upstream — see `README.md`:

```shell
git clone --recursive https://github.com/nastea1/barmetal.git
cd barmetal
./fbt
```

## Status

Early. Changes are written against the upstream sources and reviewed line by line;
see the commit history for what has been built and what has been flashed to hardware.
Treat unflashed changes as untested on a device.
