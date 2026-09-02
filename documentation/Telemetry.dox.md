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
            "t": "timer.session.end",
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
| `events` | Up to 32 events per batch: `t` = type, `ts` = event time (ms), `p` = priority, `d` = payload |

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
| `device.state`                    | 0 | `charge`, `charging`, `charge_limit`, `matter_fabrics`, `matter_commissioned`, `account_linked`, `dev_mode` (composite, per flush)                                                                                             |
| `fw.update`                       | 2 | `from_version`, `outcome` (`success`\|`failure`)                                                                                                                                                                               |
| `timer.session.start`             | 1 | `source` (`device`\|`http_api`\|`integration:matter`\|`integration:mqtt`), `profile`, `theme`, `mode` (`infinite`\|`simple`\|`interval`), `demo`; simple: `duration_ms`; interval: `work_ms`, `rest_ms`, `cycles`, `autostart` |
| `timer.session.end`               | 2 | `outcome` (`completed`\|`stopped`\|`interrupted`), `source`, `duration_s`, `cycles`                                                                                                                                            |
| `timer.theme`                     | 0 | `profile`, `theme`                                                                                                                                                                                                             |
| `app.start` / `app.stop`          | 1 | `app` (app id)                                                                                                                                                                                                                 |
| `canvas.acquire`                  | 1 | `app` (HTTP API `application_name`), `priority`                                                                                                                                                                               |
| `canvas.release`                  | 1 | `app` (the released owner's `application_name`)                                                                                                                                                                               |
| `setting.brightness`              | 0 | `value` (0–100), `mode` (`auto`\|`manual`)                                                                                                                                                                                     |
| `setting.volume`                  | 0 | `volume` (0.0–1.0)                                                                                                                                                                                                             |
| `input.switch`                    | 0 | `pos` (`busy`\|`status`\|`off`\|`apps`\|`settings`)                                                                                                                                                                            |
| `input.counts`                    | 0 | `ok`, `back`, `start`, `wheel_up`, `wheel_down` (composite, per flush window)                                                                                                                                                  |
| `power.transition`                | 1 | `charging`, `charge`, `charge_limit`                                                                                                                                                                                           |
| `net.online` / `net.offline`      | 2 | (empty)                                                                                                                                                                                                                        |
| `net.offline_duration`            | 2 | `duration_ms`                                                                                                                                                                                                                  |
| `account.link` / `account.unlink` | 2 | `linked` (bool)                                                                                                                                                                                                                |

> `device.state` and `input.counts` are composite events generated at flush time
> and are not part of the per-type enum reported by the `telemetry stats` CLI.
