# Telemetry Format {#telemetry_format}

The device reports usage and device-state events to the backend as JSON over
MQTT. This page describes the upload envelope and the format of individual
events.

## Transport

- MQTT topic (device scope, **no account link required**):
  `devices/<device_serial>/up/v1/telemetry`
- QoS 1 (at-least-once)
- Batches are published on a 15-minute timer, when the in-RAM buffer reaches 32
  events, on a high-priority (push) event (rate-limited to 1 message / 5 s), and
  on MQTT reconnect (backlog flush).

## Upload envelope

```json
{
    "schema": 1,
    "ts": 1720000000000,
    "events": [
        {
            "t": "timer_stop",
            "ts": 1720000000000,
            "p": 2,
            "d": { "...": "..." }
        },
        { "t": "app.start", "ts": 1720000000001, "p": 1, "d": { "...": "..." } }
    ]
}
```

| Field    | Meaning                                                                                      |
| -------- | -------------------------------------------------------------------------------------------- |
| `schema` | Envelope format version (currently 1)                                                        |
| `ts`     | Batch creation time, epoch ms                                                                |
| `events` | Up to 34 events per batch: up to 32 buffered, plus the flush-time composites `device.state` and `input.counts`; `t` = type, `ts` = event time (ms), `p` = priority, `d` = payload |

### Priority (`p`)

- `2` — **push**: buffered even offline, sent immediately (rate-limited)
- `1` — **batch**: buffered, flushed on the 15-minute cadence
- `0` — **low**: aggregated/snapshots, dropped while offline

### Conventions

- All timestamps are Unix epoch milliseconds.
- Durations use seconds (`*_s`) or milliseconds (`*_ms`) as named.
- The device emits milestones/transitions, snapshots and aggregates only;
  durations over time (timer session length, screen ownership, mains vs battery
  share) are computed on the collection/analytics side.

## Events

| `t`                               | p | `d` fields                                                                                                                                                                                                                     |
| --------------------------------- | - | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `device.boot`                     | 2 | `serial`, `fw_version`, `fw_hash`, `fw_branch`, `fw_build_date`, `fw_target`, `fw_dirty`, `usb_mac`, `hw_version`                                                                                                              |
| `device.state`                    | 0 | `charge`, `charging`, `usb_connected`, `charge_limit`, `matter_fabrics`, `matter_commissioned`, `account_linked`, `dev_mode` (composite, per flush)                                                                            |
| `fw.update`                       | 2 | `from_version`, `outcome` (`success`\|`failure`); on success: `to_version`                                                                                                                                                     |
| `start_timer`                     | 2 | `uuid` (card id), `timer_session_id`, `timer_source`, `profile`, `theme`, timer fields, `demo`, `restored`                                                                                                                     |
| `timer_complete`                  | 2 | `uuid`, `timer_session_id`, `timer_source`, timer fields, `duration_s`, `cycles`; interval: `phase` (`work`\|`rest`)                                                                                                           |
| `timer_stop`                      | 2 | as `timer_complete`, plus `reason` (`USER_ACTION`\|`EXTERNAL`\|`INTERRUPTED`)                                                                                                                                                  |
| `change_timer_settings`           | 0 | `uuid`, `timer_source`, `profile`, `theme`, timer fields; not reported for profiles pushed from the cloud                                                                                                                      |
| `app.start` / `app.stop`          | 1 | `app` (app id)                                                                                                                                                                                                                 |
| `app.install` / `app.update`      | 1 | `app`, `version`, `hash` (when known)                                                                                                                                                                                          |
| `app.uninstall`                   | 1 | `app`, `version` (when known)                                                                                                                                                                                                  |
| `canvas.acquire`                  | 1 | `app` (HTTP API `application_name`), `priority`                                                                                                                                                                               |
| `canvas.release`                  | 1 | `app` (the released owner's `application_name`)                                                                                                                                                                               |
| `setting.brightness`              | 0 | `value` (0–100), `mode` (`auto`\|`manual`)                                                                                                                                                                                     |
| `setting.volume`                  | 0 | `volume` (0.0–1.0)                                                                                                                                                                                                             |
| `input.switch`                    | 0 | `pos` (`busy`\|`custom`\|`off`\|`apps`\|`settings`)                                                                                                                                                                            |
| `input.counts`                    | 0 | `ok`, `back`, `start`, `wheel_up`, `wheel_down` (composite, per flush window)                                                                                                                                                  |
| `power.transition`                | 2 | `usb_connected`, `charging`, `charge`, `charge_limit`                                                                                                                                                                          |
| `net.online` / `net.offline`      | 2 | (empty)                                                                                                                                                                                                                        |
| `net.offline_duration`            | 2 | `duration_ms`                                                                                                                                                                                                                  |
| `net.wifi.connect`                | 2 | `security` (`open`\|`wpa`\|`wpa2`\|`wpa_wpa2_mixed`\|`wpa3`\|`wpa3_transition`\|`wep`), `reconnect` (bool)                                                                          |
| `net.wifi.disconnect`             | 2 | `reason` (`requested`\|`connect_failed`\|`connection_lost`)                                                                                                                                                                    |
| `net.wifi.reconfigure`            | 2 | `action` (`connect`\|`forget`) — user-initiated                                                                                                                                                                                |
| `account.link` / `account.unlink` | 2 | `linked` (bool)                                                                                                                                                                                                                |
| `integration.connect` / `integration.disconnect` | 2 | `integration` (`matter`)                                                                                                                                                                                        |
| `telemetry.enable`                | 2 | `source`, `text_version` (as given in `PUT /api/telemetry`)                                                                                                                                                                    |
| `telemetry.disable`               | 2 | (empty)                                                                                                                                                                                                                        |

Timer fields: `timer_type` (`INFINITE`\|`SIMPLE`\|`INTERVAL`), `timer_total_time_ms`; interval:
`timer_work_time_ms`, `timer_rest_time_ms`, `timer_cycles_count`, `timer_is_autostart_enabled`.
`timer_source` is `BAR`, `HTTP_API`, `MATTER`, `CLOUD` or `UNKNOWN`. A start and its end share
`timer_session_id`.

> `device.state` and `input.counts` are composite events generated at flush time
> and are not part of the per-type enum reported by the `telemetry stats` CLI.

> `fw.update` is reported on the first boot after an update is installed, with
> `outcome` `success` or `failure`. An update that fails before the device
> reboots into the updater is reported right away with `outcome` `failure`.

> A timer that was running when the device rebooted is closed with
> `timer_stop` (`reason` `INTERRUPTED`), and the restored timer is reported as a
> new `start_timer` with `restored` set and `timer_source` `UNKNOWN`.

> `integration.connect` and `integration.disconnect` are reported when the
> device gets its first Matter controller and when it loses the last one.

> `telemetry.enable` is the first event after consent is given.
> `telemetry.disable` is the last event. Events not sent yet are dropped when
> telemetry is turned off. Without a connection, `telemetry.disable` is sent
> after the next connect.
