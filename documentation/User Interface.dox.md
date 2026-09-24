# User Interface {#user-interface}

This page covers the display pipeline, the GUI framework, input, the application life cycle (desktop and loader), animations, status indicators, brightness and audio. All of it runs on the U5 except the input backend and the status lights backend, which run on the Si917.

# Displays

| | Front | Back |
| --- | --- | --- |
| Panel | 72x16 RGB LED matrix | SSD1320 160x80 grayscale OLED |
| Color depth | 24 bit (`LV_COLOR_FORMAT_RGB888`) | 8 bit in LVGL, packed to 4 bit for the panel |
| Draw buffer | 3456 bytes, direct render mode | 12800 bytes, direct render mode |
| Service | `front_display` (order 80) | `back_display` (order 70) |
| Usable area for applications | Whole panel | 148x80. The rightmost 12 px column holds the status bar. |

## Front display service

`front_display_draw()` copies an RGB888 frame and sends it through the LED driver pipeline: an index lookup table maps frame buffer pixels to the transmit order of the three chained driver ICs, a 256 entry gamma table (gamma 2.8, flattened toward 2.5 at low brightness) converts each channel to 16 bit, and the encoded stream goes out over OCTOSPI1 by DMA. The TIM5 scan interrupt starts the transfer right after VSYNC so data loads between scan frames. The driver enables output after 10 blank frames.

Other API: `front_display_set_brightness()` (0..100, regenerates the gamma table), `front_display_set_blanked()` (fades to black over 200 ms and drops draws while blanked), `front_display_sleep_mode()` (cuts the panel rail once the current DMA transfer ends). The service waits until the power service reports the battery present before it powers the panel, because the matrix draws from the battery rail.

## Back display service

`back_display_draw()` converts the L8 buffer to packed L4 and marks it dirty. The actual SPI transfer happens on the next tearing-effect interrupt from the panel, so updates are frame synchronized. The API also offers `back_display_set_contrast()` (default 25), `back_display_set_gamma_table()` (15 entries) and a reference counted `back_display_sleep_mode()`.

## GUI service

The `gui` service (order 200, record `gui`, high priority thread) owns a single LVGL 9 instance with one `lv_display_t` per panel. It ticks LVGL every 8 ms, refreshes every 16 ms, registers the `C:` filesystem driver over the storage service, attaches one theme per display, and funnels input events from the `input_events` pubsub into LVGL.

LVGL is compiled with the software renderer only, a 1 MiB image cache, the baked `busy_regular_5` font as default, the `bar`, `canvas`, `image` and `label` widgets, flex and grid layouts, the PNG decoder and the QR code library. The build patches the binary image extension from `bin` to `image`, which is why external images ship as `*.image`. Configuration is in `targets/f21/config/lv_conf.h`.

### Layers

Each display has four layers, top to bottom: `System` (status bar, desktop transition overlay, supervisor warnings), `Top` (dialogs, the HTTP canvas overlay), `Main` (the running application) and `Bottom`. Every layer has one root widget per display. The GUI offers input layer by layer from `System` down. The first layer that consumes an event stops propagation.

### Widget model

`Widget` (`applications/services/gui/widget.h`) is an `lv_obj_t` subclass. Every module follows the same pattern: an opaque structure whose first member is the base widget, an LVGL class derived from `widget_lvgl_class`, `x_alloc(parent)`, `x_free()` and `x_get_base()`. Class data carries an optional input callback and a per-display style callback, so one widget renders correctly on both panels.

An application gets a screen by opening `RECORD_GUI`, taking the `Main` layer root for each display and allocating widgets under it. All widget calls must happen under `gui_lock()` (the `with_gui()` macro). There is no view dispatcher: the application owns its LVGL objects and frees them before it exits.

Modules in `applications/services/gui/modules/`:

| Module | Purpose |
| --- | --- |
| `label` | Text with theme font sizes, alignment, wrap, dots, scroll and circular scroll modes, inline color commands |
| `image` | Image from a path (cached), uncached, or from raw pixels |
| `anim_player`, `anim_menu` | Animation playback and a two-option animated menu driven by named sections |
| `menu`, `submenu` | Vertical lists with icons and sublabels |
| `dialog` | Two-option dialog with an optional icon |
| `var_item_list` | Settings list: switch, selector, spinbox, timebox |
| `flex_layout`, `flex_box` | Row and column containers |
| `nav_bar` | Back display header with a breadcrumb stack |
| `title_card`, `anim_title_card` | Icon plus title, optionally animated |
| `status_view` | Icon plus primary and secondary text |
| `progress_bar`, `qr_code`, `countdown`, `canvas` | Self describing |
| `snap_image` | Blurred or dimmed capture of the current frame, used behind modals |
| `transition_overlay`, `overlap_fader` | Color or mask overlays and edge fades |
| `mirror_card`, `front_display_mirror` | Live copy of the front display on the back display |

`scene_manager.h` is a plain scene stack (`enter`, `exit`, `event` callbacks, custom, back and tick events) that most applications use. It is not tied to LVGL.

### Themes and fonts

Two themes (`lib/lvgl_addons/themes/`) style every widget class per display. Front: black background, gray text, white when focused, fonts `busy_regular_5` and `busy_regular_7`. Back: gray text, white when focused, fonts `busy_regular_5`, `busy_regular_9` and `busy_bold_10`.

The `font_registry` start-up hook loads fonts by path from `/ext/apps_assets/shared/fonts/`, caches up to 7 with reference counting, and returns baked in-flash fonts for `busy_regular_5` and `busy_regular_9`. Available fonts: `busy_bold_7`, `busy_bold_10`, `busy_condensed_7`, `busy_regular_5/7/9/14`, `busy_superscript_7`, `busy_tiny`, `lana_pixel_regular_11`. The `fontstat` CLI command prints the cache.

# Input

## Hardware side (Si917)

`applications/services/input/input_f64.c` reads three buttons (OK, Back, Start/Pause), the five mode switch contacts (Busy, Status, Off, Apps, Settings) and the rotary encoder through the QEI peripheral. Pin interrupts start a 2 ms debounce timer that waits for 10 stable samples before it emits an event. The backend publishes events locally and sends them over the `Input` intercom channel. At start-up it sends every already-pressed pin as a press so the U5 learns the initial switch position.

## Logical side (U5)

`applications/services/input/input.c` (record `input`, pubsub `input_events`) turns intercom events into `InputEvent{key, type, sequence}`:

- Keys: `Up`, `Down`, `Right`, `Left`, `Ok`, `Back`, `Start`, `Busy`, `Custom`, `Off`, `Apps`, `Settings`. Encoder rotation maps to `Up` and `Down`, one step per event.
- Types: `Press`, `Release`, `Short` (released before the long threshold), `Long` (held 300 ms), `Repeat` (every 150 ms after long while held).
- The mode switch position is a `FuriState` (`input_get_switch_pos()`). It defaults to Busy if nothing arrives within 1.5 s of boot.

Widgets react to `Short` presses. `Up` and `Down` navigate, `Ok` and `Start` both confirm, `Back` is left to the application. `input_key_press/release/toggle()` inject software events; the `input` CLI command and `POST /api/input` use them.

# Desktop and loader

## Loader

The `loader` service (record `loader`) runs one application at a time. `loader_start(name, args)` finds the application in the generated tables, allocates a thread with the manifest stack size and application id, sets the loader priority to 10 and starts it. `loader_stop()` sends `FuriSignalExit` to the application thread; an application must install a signal handler, release its widgets and records, and return. `loader_lock()` blocks application starts without running anything.

Loader priority (`loader.h`): stub 0, passthrough 9, default 10, max application 90, max 100, blocking 101. Only the running application may change it. The canvas service uses it to decide whether an HTTP draw request may cover the current application. CLI: `loader open <name>`, `loader kill`.

## Desktop

The `desktop` service (record `desktop`) maps the mode switch to applications and animates the transition:

| Switch position | Application | Argument |
| --- | --- | --- |
| BUSY | `busy` | |
| CUSTOM | `busy` | `custom` |
| OFF | `soft_off` | |
| APPS | `apps_menu` | |
| SETTINGS | `settings_menu` | |

Flow on a switch change: the desktop shows a 100 ms fade to black on the front display, sends `FuriSignalAboutToExit` to the running application so it can play an exit animation, debounces the switch for 100 ms, stops the application through the loader, starts the new one, then plays the mask animation `horizontal_mask_transition_72x16.anim` (section `right_to_left` or `left_to_right` depending on the direction, no animation when going to OFF). If the start fails, the `message` application shows the loader error.

`desktop_replace_current_app(name, args)` lets applications replace each other, for example a settings sub-application returning to the settings menu. A pending programmatic request is not overridden by a switch request. The start-up application is `power_on`.

# Animations

## The `.anim` file format

Custom format with the signature `bicycle1`. It supports true color, true color with alpha and 4 bit grayscale, per-frame change masks (none, all, run length, bitmap), per-frame pixel encodings (raw, run length, a QOI-like encoding), inter-frame compression, and **named sections** (ranges of frames). The encoder forces a key frame at every section start so the decoder can enter any section without decoding earlier frames. Section 0 is always `default`. The @ref file_formats page describes the full rationale and the buffer pipeline. The structures are in `lib/anim_file/anim_file_format.h`.

Source animations are zip files `name_WxH.zip` with `frame_N.png` files and a `meta.json` (`fps`, `color_mode`, `sections[]`), converted by `scripts/seq2anim.py` during the resources build. `assets/animations/README.md` documents the archive rules.

## Library and widgets

`lib/anim_file/anim_file.h`: `anim_file_alloc(storage, path, options)`, `anim_file_set_out_buf()`, `anim_file_frame()` (decodes the next frame and returns flags such as `Last`, `Finished`, `Looping`, `NoChange`), `anim_file_set_section(flags, name)` with `FinishCurrent` and `Loop` flags, and `anim_file_set_offset(x, y)` for sub-pixel translation (requires the intermediate buffer option, and renders fractional offsets through a 3x3 bilinear kernel).

`AnimPlayer` wraps the decoder in an `lv_canvas` with a timer at `1000/fps` ms and adds `set_source`, `set_section`, `start`, `pause`, `set_offset` and a frame callback. `AnimMenu` drives a menu of N options from sections named `item-X` and `transition-X-to-Y`.

# Status bar and status lights

## Status bar

The `status_bar` service places a 12 px wide column on the back display `System` layer with, top to bottom, BLE, Wi-Fi, audio and USB indicators and a battery indicator with a numeric label. It listens to the power pubsub, the audio pubsub, the Wi-Fi state and the BLE pubsub. Images come from `/ext/apps_assets/status_bar/images/`.

## Status lights

The RGB LED lives on the Si917. The U5 `status_lights` service forwards `StatusLightsCommand` frames over the intercom; the Si917 backend runs the preset and drives the PWM. API: `status_lights_run_preset(preset, color)`, `status_lights_set_brightness(0..100)`.

Presets: `Off`, `StaticColor`, `Fade` (triangle), `RainbowGradient`, `Blink` (500 ms), `Notification` (three blinks at full brightness, used by the HTTP draw API). The backend scales each channel with a gamma of 1.85. CLI on the Si917: `status_lights <r> <g> <b>`.

# Brightness

The `light_sensor` service samples the BH1730 once per second, keeps a 5 sample mean of the lux value clamped to 1..10000, and quantizes it on a logarithmic scale to a level 0..15 with hysteresis. The `light_sensor` CLI command prints the raw channels.

The `brightness_control` service (record `brightness_control`) owns one logical brightness for three sinks. Mode `auto` uses `level/15`, mode `manual` uses `brightness/100`. Each sink maps the value through its own curve:

| Sink | Range | Curve |
| --- | --- | --- |
| Front display | 1..100 | Ease-in (power 0.5) |
| Back display contrast | 1..71 | Linear |
| Status lights | 5..90 | Linear |

Temporary per-sink overrides exist for the settings screens (`brightness_control_set_brightness_override()`). The state is a `FuriState` with mode, effective brightness and the setting. CLI: `display brightness <0-100 or auto>`, `display show <front or back> <file>`, `display gamma back <max> <gamma>`.

# Audio

Sound files are headerless raw PCM, mono, 44100 Hz, signed 16 bit little endian (`.snd`). `scripts/audio.py` converts any source with ffmpeg: mono downmix, loudness normalization to -6 LUFS, and a speaker compensation EQ from `scripts/audio_eq.txt`.

The `audio` service (record `audio`) plays one file at a time through SAI1 with an 8192 sample ping-pong DMA buffer. A 100 ms fade-in applies at start and a 100 ms fade-out at stop. Playing a file while another plays fades the current one out first. There is no mixing.

API: `audio_enable()` and `audio_disable()` are reference counted and control the amplifier with a 100 ms settle time. `audio_play_file(path)` requires at least one enable holder. `audio_stop()` stops playback. `audio_set_volume(0.0..1.0)` persists to `/ext/apps_data/audio/audio.json`. Events: `AudioEventVolumeUpdate`, `AudioEventPlayEnd`. CLI: `audio start <path>`, `audio stop`.

# Canvas (remote drawing)

The `canvas` service (record `CANVAS`, order 65) backs `POST /api/display/draw` (see @ref http-api). It keeps up to 100 elements (image, animation, text, countdown, rectangle, raw image) with an id, position, alignment, display, z-index and a timeout or an absolute expiry. Elements render on the `Top` layer of both displays, sized like the `Main` root so the status bar stays visible. While a canvas is shown, a mirror card on the back display shows the front display when the back has no elements of its own. The canvas consumes every input. `Back` or any switch change closes the canvas.

A draw request carries a priority. The service rejects it (`LowPriority`) when the loader priority of the running application is higher. It retries a rejected asynchronous draw for 1.5 s on every priority change. If the running application raises its priority above the canvas, the service clears the canvas. This is how the BUSY timer blocks remote drawing during a session.

# Assets

`./fbt resources` builds the resource tree that lands at `/ext/apps_assets` on the device. The first directory level under each asset type becomes the application namespace. Names carry the pixel size as a suffix (`front_power_on_72x16`, `battery_8x18`).

| Type | Source | Converter | Output |
| --- | --- | --- | --- |
| Animation | `assets/animations/<app>/name_WxH.zip` | `seq2anim.py` | `<app>/animations/name_WxH.anim` |
| External image | `assets/images/external/<app>/name_WxH.png` | `image.py -f BIN` (LVGL binary) | `<app>/images/name_WxH.image` |
| Internal image | `assets/images/internal/*.png` | `image.py -f C` | Linked `I_<name>` descriptors, used for fallback icons |
| Sound | `assets/sounds/<app>/*.wav` | `audio.py` | `<app>/sounds/*.snd` |
| Font | `assets/shared/fonts/*.font` | Copy (TTF conversion is not yet in the toolchain) | `shared/fonts/*.font` |
| Web UI, OpenAPI, Swagger UI | `assets/frontend-build/public`, `applications/services/web_server/openapi` | gzip, merge | `web_server/www/` |
| JavaScript applications | `applications_js/<id>/` | Copy | `/ext/user_assets/<id>/` |

Application manifests can add a `resources` directory that the build merges into the tree, for example the BUSY themes in `applications/main/busy/resources/`.
