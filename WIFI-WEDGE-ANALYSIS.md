# Why the Wi-Fi subsystem wedges permanently (fw 1.1.1)

Root-cause analysis from the firmware source, driven by a device that wedged in the
field and stayed wedged across two "forget network" attempts, three power cycles, a
full Wi-Fi clear, a firmware crash, and a bootloader (DFU) cold start.

**Every claim below cites code in this repository.**

## Field symptoms

| # | Symptom |
|---|---|
| 1 | `GET /api/wifi/status` -> `{"state":"connecting"}` forever, including 30 s after a cold boot with the network forgotten |
| 2 | `POST /api/wifi/disconnect` -> HTTP 400 `Command timed out` |
| 3 | `GET /api/wifi/networks` (scan) -> empty, times out, or 503 |
| 4 | `POST /api/smart_home/pairing` -> 503 in ~5 s, sometimes hangs 45 s+ with no response |
| 5 | Unrelated endpoints (draws, uploads) fail intermittently while wedged |
| 6 | Eventually: "System error - restart device", API down |
| 7 | No `/api/system/reboot` exists, so recovery is physical |

## The mechanism

### 1. The API waits forever for a co-processor that may never answer

Wi-Fi runs on a **SiWx917 co-processor**; the STM32 host talks to it over the
`intercom` channel. Every blocking Wi-Fi API call goes through:

`applications/services/wifi/wifi_api.c:10`
```c
static WifiStatus wifi_api_blocking_request(Wifi* instance, WifiMessage* message) {
    ...
    const FuriStatus sem_status =
        furi_semaphore_acquire(instance->api_semaphore, furi_ms_to_ticks(WIFI_API_TIMEOUT_MS));
    if(sem_status == FuriStatusOk) {
        wifi_api_send_message(instance, message);
        api_lock_wait_unlock_and_free(message->lock);   // <-- unbounded
```

and `lib/toolbox/api_lock.h:33`
```c
#define api_lock_wait_unlock(_lock) \
    furi_event_flag_wait(_lock, API_LOCK_EVENT, FuriFlagWaitAny, FuriWaitForever)
```

`FuriWaitForever`. The semaphore acquire is bounded (5 s); **the wait for the answer is
not**. If the co-processor never responds:

* the calling thread (an HTTP worker) blocks **permanently** -> symptom 5;
* `wifi_api_unlock()` never runs, so `api_semaphore` is **never released**
  (`wifi_api.c:59`) -> every later call fails its 5 s acquire and returns
  `WifiStatusTimeout` -> HTTP 400 `Command timed out` (`http_api/api_wifi.c:38`) -> symptom 2.

One unanswered request disables the entire Wi-Fi API for the rest of the uptime.

### 2. Nothing times out the `connecting` state

`wifi_state.c:8` implements the state machine; transitions out of `WifiStateConnecting`
happen **only** when the backend reports connected/disconnected
(`wifi.c:240` `wifi_process_backend_info_response`). There is no connect timer, no retry
limit, no fallback to `WifiStateDisconnected`. If the co-processor goes silent mid-connect,
the host reports `connecting` forever -> symptom 1.

### 3. The wedge blocks its own escape hatches

`wifi_state.c:97` gates requests on the current state:

```c
} else if(request_type == WifiRequestTypeScan) {
    if(current_state != WifiStateDisconnected) status = WifiStatusScanNotPossible;
} else if(request_type == WifiRequestTypeForget) {
    if(current_state != WifiStateDisconnected) status = WifiStatusError;
```

While stuck in `connecting`:

* **scan is refused** - it requires `Disconnected` -> symptom 3;
* **forget is refused** - it requires `Disconnected`.

That last one is why the field device could not be recovered: the only supported way to
clear the network is unavailable exactly when it is needed. `Disconnect` *is* permitted
from `Connecting`, but it is dispatched to the unresponsive co-processor and times out -
so the state can never reach `Disconnected`. **The wedge is self-sealing.**

### 4. Forget never reaches the co-processor

`wifi.c:125`
```c
} else if(request_type == WifiRequestTypeForget) {
    FURI_LOG_I(TAG, "Forgetting saved network");
    wifi_settings_reset(NULL);
    break; // No backend request necessary
```

Forget clears **host-side settings only**. Any profile/auto-connect state on the
co-processor is untouched, so a "forgotten" device can still come up attempting to
associate.

### 5. Boot re-enters the trap automatically

`wifi.c:59` `wifi_apply_settings_pending_callback()` loads settings at startup and calls
`wifi_schedule_connect_request()` whenever an SSID is present. With an unhealthy
co-processor, every boot immediately re-enters `Connecting` and re-wedges -> symptom 1
"from a cold boot".

### 6. Unexpected transitions are deliberate panics

`wifi_state.c` contains **17** `furi_crash()` calls, e.g.
```c
} else {
    furi_crash("Invalid transition from WifiStateConnecting");
}
```
Any state/event combination the author did not anticipate - plausible once host and
co-processor disagree - is a hard crash, i.e. "System error - restart device" -> symptom 6.

## Proposed fixes

1. **Bound the wait.** Give `api_lock_wait_unlock` a timeout variant and use it in
   `wifi_api_blocking_request` (e.g. 30 s), releasing the semaphore and returning
   `WifiStatusTimeout` on expiry. *This alone converts a permanent wedge into a
   recoverable error.*
2. **Time out `Connecting`/`Reconnecting`.** Start a timer on entry; on expiry force a
   transition to `Disconnected` and tear down the backend request.
3. **Always allow `Forget` and `Disconnect`.** Forget should be legal from any state and
   should also clear co-processor-side profiles; disconnect should force the host state
   to `Disconnected` even when the backend does not answer.
4. **Add a co-processor watchdog.** If N consecutive requests time out, power-cycle the
   SiWx917 (`wifi_power.c`) and re-init instead of waiting forever.
5. **Replace `furi_crash` on unexpected transitions with recovery** - log, reset to
   `Unknown`, and re-init the subsystem. Crashing the whole device because the radio
   misbehaved is disproportionate.
6. **Expose `POST /api/system/reboot`.** There is currently no software reboot; a wedged
   device requires physical intervention (and, in one case, a DFU escape).

## Bonus: `/api/display/brightness` looks inert because the low end is flat

`applications/services/brightness_control/brightness_conv.c:7`
```c
static const uint8_t brightness_conv_front_table[] = {25, 25, 28, 31, 37, 43, 52, 61, 73, 85, 100};
```
User brightness 0-100 is quantised to 11 internal steps
(`brightness_conv_user_to_internal`), then mapped through this table. Consequences:

* **`value=0` does not switch the front display off - it sets 25 %.** There is no way to
  turn the display off via this API.
* Every value from 0 to ~14 lands on `25` (steps 0 and 1 are both 25), so the whole low
  range is visually identical - which reads as "the setting does nothing".
* `GET` returns `brightness_setting` (the stored user value, `brightness_control.c:292`),
  so a stored `1` reads back while the panel sits at 25 % - the setting appears accepted
  and ignored.

Suggested: make step 0 mean off (or add an explicit off), and spread the low end.
