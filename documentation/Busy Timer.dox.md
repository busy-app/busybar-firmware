# Busy Timer {#busy-timer}

The busy timer is the core product feature. It is a focus timer that shows a status on the front display, mirrors its state to the cloud, and can drive a smart home switch. The code is split into the `busy_timer` service, which owns the model, and the `busy` application, which owns the screens.

# Model: the `busy_timer` service

Service: `applications/services/busy_timer/` (record `busy_timer`, order 190, U5).

## Modes and states

| Mode | Meaning |
| --- | --- |
| `Infinite` (shown as "Off") | No countdown. Stays in Work until stopped. |
| `Simple` | One Work interval of `total_time_ms` |
| `Interval` | Work and Rest cycles, `cycles_count` times |

States: `Idle`, `Work`, `Rest`. Transitions: Idle to Work on start; Work to Rest while cycles remain, otherwise Work to Idle; Rest to Work. When an interval ends without autostart the timer stops and publishes `IntervalEnded`; with autostart, or on a forced skip, it continues.

Limits: time 5 min to 24 h in 5 min steps, work and rest 5 min to 8 h, cycles 2 to 35. Defaults: 20 min simple, 20/5 min times 3 interval, autostart off.

## Profiles

Two profiles exist, `Busy` and `Custom`. A profile is the timer configuration plus the application configuration (`theme_name`, `is_smart_home_enabled`, `is_show_work_only_enabled`), metadata (`sort_order`, `title`, `card_id`) and a timestamp.

| Profile | Default | Theme | Show work only | Title |
| --- | --- | --- | --- | --- |
| `busy` | Interval 20/5 times 3 | `busy` (built in) | No | BUSY |
| `custom` | Infinite | `keep_out` | Yes | ZEN |

The service stores the profiles in `/ext/apps_data/busy_timer/settings_busy.json` and `settings_custom.json`, and the last snapshot in `state.json`. At boot it restores the snapshot. An active snapshot relaunches the `busy` application in timer mode.

## Ticking

A 33 ms poll timer reads the RTC and advances whole seconds. The timer therefore follows wall-clock time and survives RTC jumps in both directions. Infinite mode does not run the poll timer. `busy_timer_add_time(minutes)` rounds to 5 min multiples and clamps to the limits. The service ignores it while paused. In Infinite mode it converts the timer to Simple with 5 min remaining. A demo mode (visible only with the NVM debug flag) accelerates the countdown.

## API and events

Public API (`busy_timer.h`, synchronous through a 4 deep queue): `start(profile_id)`, `stop`, `toggle` (pause and resume), `skip`, `finalize`, `add_time`, `get_run_info`, `get_snapshot`, `set_snapshot`, `get_profile`, `set_profile`, `get_preset`, `set_preset`, `get_mode_names`.

Events on `busy_timer_get_pubsub()`: `Tick{elapsed, remaining}`, `ModeChanged`, `StateChanged`, `IntervalEnded{is_forced}`, `Paused{is_paused}`, `ProfileChanged{id, profile}`, `SnapshotCreated{snapshot}`.

## Snapshots and cloud sync

A snapshot describes the current run: `type` (`NOT_STARTED`, `INFINITE`, `SIMPLE`, `INTERVAL`), `card_id`, `is_paused`, per-type progress fields, the application configuration and a timestamp. The service captures a snapshot on every start, stop, toggle, skip, finalize and add-time.

Synchronization with the cloud goes over MQTT (see @ref connectivity), all at QoS 1:

| Topic | Direction | Content |
| --- | --- | --- |
| `busy/snapshot` | Both | Snapshot JSON, debounced at 250 ms on publish |
| `busy/profiles/busy`, `busy/profiles/custom` | Both | Profile JSON |

The service ignores incoming snapshots that are older than or equal to the last known one. It accepts incoming profiles only if they are valid and strictly newer. It clamps future timestamps to now. The HTTP API exposes the same JSON on `/api/busy/snapshot` and `/api/busy/profiles/{slot}` (see @ref http-api). A remote start launches the `busy` application with the argument `timer`. The `state_publisher` service also streams both objects to WebSocket, BLE and MQTT clients.

## Smart home coupling

When the running profile has `is_smart_home_enabled`, the Matter On/Off switch mirrors the timer: Work while running is On, everything else is Off. In the other direction, a Matter On from Idle applies the busy profile, forces smart home on for the session, brings the application to the foreground and starts the timer. On while paused resumes. On during Rest skips to Work. Off stops the timer and exits the application.

## Status lights

Work is static red, Rest is static green, Idle, paused and interval-ended turn the lights off.

# Screens: the `busy` application

Application: `applications/main/busy/` (appid `busy`, type `APP`, order 10, 4 KiB stack). The BUSY and CUSTOM switch positions run the same binary. CUSTOM passes the `custom` argument, which selects the `Custom` profile, its header image, start animation and MQTT topic. The `timer` argument (used by remote starts) skips the Start scene and waits for a `ShowTimer` request.

## Scene flow

1. **Start**: the profile logo animation and a two-option animated menu on the front, a Start/Setup menu on the back. Start loads the profile and goes to Overview (Interval mode) or straight to Timer. Setup opens the setup menus. Back does nothing here.
2. **Setup**: Timer (mode, time, work, rest, cycles, autostart, "Show work phase only", demo mode with the debug flag), Theme (front display theme picker with a live mirror on the back), Smart home (one switch). The scene writes changes back to the profile on exit.
3. **Overview** (Interval only): shows work and rest minutes, auto-advances after 2.25 s.
4. **Timer**: the running state. Up and Down add or remove 5 min, OK skips the interval, Start pauses and resumes, Back stops (or resumes when paused). Ticks drive a progress indicator and a countdown label. A tick sound plays at 3 s and a finish sound at 0. The front display shows the theme background (an image or animation) for custom themes and hides the countdown for 15 s out of every 20 s. With "Show work only" the application blanks the front display outside Work. While the timer is active the application raises its loader priority to blocking, which refuses HTTP canvas draws, and pauses automatic firmware updates.
5. **Progress** (Interval): "done/total" dots for 2 s, then Next.
6. **Next** (Interval, autostart off): prompts to start the next BUSY or REST interval.
7. **Ending** (Interval, all cycles done): particle animation and a cycle summary.
8. **Finish**: confetti and a summary, plays `session_completed.snd`. Start or Back finalizes the timer and returns to Start.

Transitions between scenes use the `transition_overlay` widget with presets such as fade to black, oval mask, press effect and flash.

## Themes

The theme picker lists the built-in `busy` theme plus every directory under `/ext/apps_assets/busy/themes/<name>/theme.json` (`bg_path`, `order`). Shipped themes, in order: `keep_out`, `dnd`, `meeting`, `on_call`, `lunch`, `back_soon`, `booked`, `flow`, `chill_time`, `on_air`, `coding`, `low_social_battery`. Each points at a 72x16 animation in the shared animations directory.

## Cross-thread API

`busy_api.c` exposes `busy_set_config()`, `busy_show_timer()` and `busy_request_exit()` through the record `busy_app`, which exists only while the application runs. The `busy_timer` service uses them (`busy_timer_start_app()`, `busy_timer_exit_app()`) when a remote snapshot or a Matter switch starts or stops the timer. If the application is not running, the service launches it with `desktop_replace_current_app("busy", "timer")`.
