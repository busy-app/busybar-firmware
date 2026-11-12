# AGENTS.md

Instructions for coding agents working on the BUSY Bar JavaScript applications.

## Ground rules

- **Do not modify the BUSY Bar firmware** when writing a JavaScript application. No changes to `applications/`,
  `lib/`, `targets/`, `fbt_layers/` or any other firmware code, and no `./fbt` builds. JavaScript apps run on
  stock firmware. If a feature seems to need a firmware change, stop and tell the user instead.
- Start from [busy-tools](https://github.com/busy-app/busy-tools). It scaffolds new apps, builds them into
  device packages and installs, runs and stops them on a connected BUSY Bar. Read its `AGENTS.md`, README and
  `packages/cli/README.md` before you start.
- Apps talk to the device only through its [HTTP API](https://docs.busy.app/bar/dev/http-api), by calling
  `fetch` on `http://localhost/api/...` from inside the script. Only a subset of the API is available to apps
  (see [Available HTTP API](#available-http-api)).
- The runtime, package layout, manifest and settings schemas, `fetch` limitations and input events are
  described in [JavaScript Applications](../documentation/JavaScript%20Applications.dox.md). That document
  is the reference. Read it when something here is not enough.
- The default button press event is `short`, not `press`. Use it unless explicitly told otherwise.

## What is in this directory

| Directory             | Contents |
| --------------------- | -------- |
| `app.busy.js_example` | Minimal plain-JS app in the on-device layout: input handling, drawing, settings. |
| `app.busy.clock`      | `app.json` pointing to an external repository and version, not the app sources. |
| `app.busy.weather`    | Same as above. |

New apps live in their own repository ("one app, one repository"), scaffolded with busy-tools. Do not add
app sources here unless the user asks for it.

## Development loop

Requires Node 24 (see `.nvmrc` in the generated app; Node 26 is not supported) and a BUSY Bar connected over
USB, or reachable over Wi-Fi.

### 1. Scaffold

```sh
npm create @busy-app/app my-app -- --author "Jane Doe" --id app.jane_doe.my_app \
    --name "My App" --description "..." --api-version 27.12.0
cd my-app
```

Pass every flag when running non-interactively, otherwise the generator prompts. `--no-git` and
`--no-install` skip `git init` and the dependency install. `pnpm`, `yarn` and `bun create` work the same way.

### 2. Write the app

Project layout (`busy-cli` only looks at the folder it runs in, or its `src/`):

```
my-app/
├── src/                          the app, the only folder the build scans
│   ├── main.ts                   entry point (main.ts or main.js, exactly one)
│   ├── appmeta/
│   │   ├── manifest.json         required: format_version, id, name, version, api_version
│   │   ├── settings.json         optional: settings description
│   │   ├── icon_front_8x8.png    optional: 8x8 colour icon
│   │   └── icon_back_11x11.png   optional: 11x11 greyscale icon
│   ├── images/                   .png, .image
│   ├── animations/               .anim
│   └── sounds/                   .snd
├── shared/                       helpers imported as @shared/* (device client, settings, dates)
├── types/env.d.ts                globals provided by the device and the build (listen, __BUSY_ADDR__)
└── package.json
```

- The entry point is executed top to bottom, nothing is imported or called. It must start the app itself
  (the template calls `main()` at the end) and **must not export anything**, or the build fails. Imported
  modules may use exports normally.
- The manifest `id` must match the package root directory name: at most 32 characters, ASCII letters,
  digits, `_`, `-` and `.`, not starting with `.`. Use a reverse-domain id (`app.<author>.<app>`), and
  replace a placeholder `app.example` before publishing.
- `api_version` is the minimum HTTP API version the app needs. Installing or launching on firmware with an
  older API fails.
- Raise `heap_size_kib` (default 128, max 512) if the app runs out of memory.
- Draw with the layout helpers from `@busy-app/busy-lib` (`row` / `column` / `stack` → `render()` →
  `device.DisplayDraw()`). The front display is 72x16 pixels.
- Handle input with `listen("input", handler)`, which returns an unbind function. A long press of Back is
  reserved by the firmware to force-quit the app.
- The app ends once nothing keeps it alive: clear timers and call the unbind function to exit cleanly.
- Resources in other formats (`.jpg`, `.svg`, `.wav`, `.mp3`, ...) end up in `resources/` and the device
  cannot decode them. Convert them to `.png`, `.anim` or `.snd`.
- Type-check with `npm run typecheck`.

### 3. Build

```sh
npm run build                     # dist/<id>.tgz
npm run build -- --no-minify      # readable output, for debugging on the device
npm run build -- --emit dir       # dist/<id>/ instead of the archive, to inspect the package
```

The package has the on-device layout: `<id>/appmeta/`, `<id>/scripts/main.js` (plus one file per imported
module unless `--bundle` is used), `<id>/images/`, `animations/`, `sounds/` and `resources/`. `dist/` is
emptied on every build.

### 4. Deploy, start and stop

```sh
npm run app:install               # uploads dist/<id>.tgz and installs it (does not build)
npm run app:run                   # launches the installed app
npm run app:stop                  # quits the running app
npm run app:uninstall             # removes the app, keeps its settings
```

These call `busy-cli app install|run|stop|uninstall`, which use the device's `apps` HTTP API endpoints. Run
them from the project folder to pick up the id and package automatically. Otherwise pass the `.tgz` path or
app id explicitly.

- Over USB the device is at `10.0.4.20` and no password is needed.
- Over Wi-Fi pass `--addr <host>`, plus `--password <password>` if password protection is enabled, e.g.
  `npm run app:install -- --addr 192.168.1.42 --password secret`. HTTP API access over Wi-Fi must be enabled
  in the device web UI (**Network** → **HTTP API** → **Over Wi-Fi**).
- A typical iteration is `npm run build && npm run app:install && npm run app:run`. Installing over a
  running app or a previous version is fine.
- Never pass `--addr` to `npm run build` unless the user asks: it changes the address the *app* connects to,
  which must stay `localhost` for an app running on the device.

## Available HTTP API

Apps run in isolation and can only reach these endpoints (relative to `http://localhost/api/`). Any other
endpoint is rejected, even if it is listed in the full [HTTP API reference](https://docs.busy.app/bar/dev/http-api).
Do not design an app around an endpoint that is not on this list.

| Endpoint             | Allowed method(s)           |
| -------------------- | :-------------------------: |
| `apps/settings`      | `GET` [1]                   |
| `display/brightness` | `GET`, `POST`               |
| `display/draw`       | `POST`, `DELETE`            |
| `audio/play`         | `POST`, `DELETE`            |
| `audio/volume`       | `GET`, `POST`               |
| `busy`               | `GET`, `PUT`                |
| `name`               | `GET`                       |
| `time`               | `GET`                       |
| `wifi/status`        | `GET`                       |
| `version`            | `GET`                       |
| `access`             | `GET`                       |
| `transport`          | `GET`                       |
| `status`             | `GET`                       |
| `status/device`      | `GET`                       |
| `status/firmware`    | `GET`                       |
| `status/system`      | `GET`                       |
| `status/power`       | `GET`                       |
| `storage`            | `GET`, `POST`, `DELETE` [2] |

- [1] Only with `app_id` equal to the running app's id. Apps can read their settings but not write them.
- [2] Requests made to Storage API from a JS application can only access application's sandboxed storage space mapped to `/ext/data`.

This list mirrors [JavaScript Applications](../documentation/JavaScript%20Applications.dox.md#http-requests),
which is authoritative. Check it if the two ever differ. Remote HTTP(S) resources can be fetched with the same
`fetch` when Wi-Fi is connected. Handle network failures there without stopping the app.

## Design guidelines

Apply these by default. They are recommendations, not hard requirements; if the user asks otherwise, follow the user.

### Principles
BUSY Bar stands on the user's desk while they work. Design for a quick look from the side.
- **Glanceable:** a user who looks at the screen for 1 second understands the main information. Make the key value stand out; details come second.
- **Calm:** the screen does not distract from work. Change, move or flash something only when it is important to the user.
- **Predictable:** the user already knows the controls from other apps. Give each control its standard meaning (see Controls).

### Screens
- **Front (72x16):** the main content. Show what the task needs. Fewer elements make the key value easier to read. Make the key value the largest element. Keep at least a 1 px margin from the edges.
- **Back (160x80):** by default, do not draw on the back screen. BUSY Bar mirrors the front screen there, and this is enough for most apps.
  - Draw your own back screen only when it adds something: extra info, a QR code, longer text, or a separate mechanic, for example a game where the other player must not see the front screen.
  - Keep both screens in sync: the same values and the same state at the same time, unless the app needs them to differ, for example in a game.
- **States:** design only the states the app actually has, for example error, no network or empty. In error and empty states, say what happened and what to do next.
- **Temporary states** such as pause: when the user needs to see where they are to continue, prefer keeping the content visible. Show the state inside the interface, for example with a small icon or label, a color change, or dimmed content with an overlay.
- **Switching:** switch screens and states in one step, with no empty moment in between. When the app shows nothing, BUSY Bar shows its own "Loading..." text.
- **Loading:** if the app waits for data, pick the simplest placeholder that fits:
  - dashes in place of a value, such as `--°` or `$--`;
  - a small loader in place of the content;
  - a skeleton of the layout, when several elements load at once.

  Keep the placeholder in the place of the content it loads, so nothing jumps when the content appears. Use a separate loading screen only when there is no layout to show yet.

### Spacing
- Place related elements close together: an icon and its value, a label and its number.
- Separate unrelated groups with clearly more space than inside a group, at least twice as much.
- Use the same gap for the same kind of relation across the screen.
- Align elements to a shared edge or center line.
- Keep at least 2 px between an icon and text, and at least 1 px between lines, so elements do not touch. These are minimums, not targets: when there is room, give elements more space so the screen does not feel cramped.
- Center the content block vertically on the screen, with equal space above and below. Do not push text to the top edge.
- Keep the same element in the same place on every screen of the app, for example the status label always on the second line.
- Align elements that sit in a row, such as an icon, a value and text, to each other: by their top edges or by their vertical centers. Pick one and use it on every screen of the app.
- A group aligned to the left edge is fine; it does not have to be centered on the screen.

### Controls
- `start`: launch; switch screens or modes. When the app has one main quick action, `start` can trigger it instead. For example, in a tracker app, `start` can add a new entry, such as one more glass of water. Choose by context.
- `ok`: confirm; open.
- `encoder`: move through lists, values and screens.
- `back`: go back; exit.
- Avoid extra actions and button combinations. If the app needs one, describe the behavior of every control in the app description.
- Make every press change something visible.
- When a screen waits for the user to press something, show which control to press, for example a short hint such as "Press start". Do not leave the user guessing.

### Text and color
- Use at most 2 lines of text on the front screen.
- Use short, plain words every user knows: "Connect", "Try again", "25 Dec", "CPU 38%". Do not use jargon or error codes.
- Keep common symbols and units that show what a value is, such as `$`, `%`, `°`.
- Keep a short label or unit that says what a number counts, such as "5 tasks", "3 km" or "CPU 38%". A bare number is hard to understand at a glance.
- For a process in progress, you can use a word with an ellipsis: "Loading...", "Connecting...", "Building...". Use it only when there is no visual loader for that process.
- Scroll text only when it cannot fit otherwise, and scroll slowly.
- For changes and trends, `+` and `-` work, and arrows often read faster: ▲ 2.4% in green, ▼ 1.1% in red. Pick what fits the screen.
- Keep strong contrast between text and background.
- Use color to add character and make items recognizable: brand colors of services, sports teams or currencies are welcome.
- When the screen shows a status color (good / moderate / poor), prefer putting it on one element only, such as the level label or a status symbol. Keep other text white.
- Illustrations, such as a sun, a leaf or a cloud, should keep their natural colors. Prefer not to recolor them to show a status.
- Status symbols, such as a checkmark, a cross, a question mark, an exclamation mark or an arrow, can use status colors.
- Prefer sentence case. Use caps mainly where the Fonts section suggests it.
- When the meaning matters, prefer not to rely on color alone: back it up with a symbol, text or shape.

### Fonts
BUSY Bar has nine fonts. Use them freely: the roles below are starting points, not rules. Experiment and check the result on the device. The back screen notes apply only when the app draws its own back screen; a mirrored front screen is always fine.
- `small` (line height 9): the base font for main text and menus. Reads well and fits two lines on the front screen. Works for informational, technical and decorative text. Back screen: captions, breadcrumbs and small elements; reads better in caps.
- `normal` (line height 11): one-line text, such as timer digits or a heading. Works for informational, technical and decorative text, numbers and timers. Does not fit two lines on the front screen. Back screen: captions and numbers.
- `bold` (line height 11): accent for the main info on screen, clock and timer digits. Back screen: accent text for titles, numbers, pairing codes and timers.
- `large` (line height 13): accented large text on the front screen. Back screen: the main text for body copy and numbers.
- `extra_large` (line height 14): statuses, big short one-line text, the most important number or time. Caps only.
- `condensed` (line height 11): compact digits for timers, PIN codes and numbers. Back screen: fine, but not for accent numbers.
- `superscript` (line height 11): small numbers and time, and raised marks such as `m²`. Back screen: only when needed; on its own it reads poorly.
- `tiny` (line height 6): minor captions only, preferably in caps. Do not use it on the back screen: it reads poorly there.
- `global` (line height 11): text in other languages and non-Latin alphabets. The other fonts cover basic Latin only. It is heavy: use it only for text that needs it, and check that the app runs stably on the device.

Good starting points:
- front screen, one line: `extra_large` in caps, `large` or `normal`;
- front screen, two lines: `small` on both lines, or `small` + `bold`;
- back screen: `large` for main text, `bold` for titles and key numbers, `small` in caps for captions.

Use 2–3 fonts per screen at most, and build hierarchy with clear differences in size and weight.

### Motion
- Animations that stay on screen: slow and calm. Do not flash or blink unless the user's attention is needed.
- One-time animations, such as a celebration, a transition or a game event, can be bold. A good pattern: play the animation as an overlay on the whole screen, with the content underneath strongly dimmed, then return to the content. Do not tone these animations down to look calm.
- Every animation must look smooth and even, with no stutter or pauses between frames. Check it on the device. If it stutters, simplify it: fewer moving elements, shorter paths, evenly timed frames. A simple smooth animation is better than a complex choppy one.
- For a long or complex animation, prepare it as an `.anim` file and play it, instead of redrawing frames from code.

### Pixel shapes
These apply to every icon and graphic, in the app and in the menu.
- Draw circles and round shapes with the midpoint circle algorithm, or by hand with the same result:
  - the shape is identical in all four quadrants;
  - runs of pixels shrink smoothly from the flat top and sides toward the diagonals, for example 3, 2, 1, 1;
  - no single stray pixels and no uneven steps such as 2, 1, 2.
- For a circle 5 px wide or smaller, use a rounded square or a plus shape instead of trying to draw a true circle.
- Draw rays, spokes and similar details as straight lines at 0°, 45° and 90° only, with the same length, evenly spaced around the center, and with a 1 px gap from the body.
- After drawing, check geometric shapes, such as circles, rays and rounded squares, mirrored left to right and top to bottom. Fix any pixel that breaks the symmetry.
- Organic objects, such as a flower, a leaf or a cloud, keep their natural asymmetry and the details that make them recognizable, such as a stem. Do not turn them into geometric patterns.

### App icon: front (8x8)
These rules apply to the app icon in the menu only.
- Always draw both app icons, `src/appmeta/icon_front_8x8.png` and `src/appmeta/icon_back_11x11.png`, even though the format marks them as optional. Without them, BUSY Bar shows a default icon.
- Pick an object that shows what this specific app does, for example a tomato for a Pomodoro timer, a light bulb for smart home control, a cup for a coffee counter. Do not draw a generic shape, such as a plain circle, square or badge.
- Keep the details that make the object recognizable, such as the stem of a tomato, the handle of a cup or the teeth of a gear. Shading adds volume; it must not turn the object into a plain ball or block.
- Draw one real object, or two when the idea needs both: a sun behind a cloud, a house with signal waves, a cup with steam. Use filled silhouettes. No outlines, no text.
- Use a transparent background. Every pixel is fully opaque or fully transparent.
- Use 3–4 real material colors: wood is brown, leaves are green, metal is a cool slate gray. Make the silhouette stand out on a black background; avoid dark base colors.
- Make the icon volumetric, not flat: draw every color in 3 tones of one hue. Light comes from the top-right: highlight on the top and right edges, shadow on the bottom and left, base tone in between.
- Make the tones strongly contrasting: the highlight is much lighter and the shadow much darker than the base. Subtle steps disappear at 8x8 and the icon looks flat.
- At 8x8 every pixel counts. Simplify details instead of dropping them: draw fewer and shorter ones, but keep those that make the object recognizable. If the object still does not fit cleanly, pick a simpler object for the same idea.
- Prefer objects with a compact, solid silhouette that fills most of the 8x8, and show them from the angle where the shape is clearest. For a complex shape, keep the volume simple: one lit side and one shaded side.
- Plan the icon as an 8x8 grid of pixels, row by row, before you draw it. Check each row for merged parts and stray pixels.
- Separate parts that touch with a 1 px gap or a clearly different tone, so they do not merge into one blob.
- Keep shapes clean and proportional: symmetric where the object is symmetric, smooth curves, no stray single pixels.
- Draw the object itself. Do not place it on a background shape such as a circle or square.
- If the icon has a sign on it, such as a letter, `$`, `₿`, `?`, `!` or `%`, keep strong contrast between the sign and the object, so the sign stays readable.

### App icon: back (11x11)
- Use one color, `#888888`, with no shading.
- Draw the same object as the front icon, with fewer details, using the full 11x11.
- Fill solid objects. Draw symbols and thin shapes as lines; draw diagonals as a staircase of 2 px per row, shifted 1 px each row.

### Icons inside the app
- No fixed style. Choose size, colors and detail to fit the space and the visual weight of the screen.
- Match the icon size to the layout:
  - two lines of content: a small icon per line, no taller than the line;
  - one content block with free space: a large, detailed icon up to the full screen height minus margins. Prefer it over a small icon when it fits together with the gaps from the Spacing section.
- Every icon must be readable at its real size on the device:
  - Use a widely known symbol with a simple, clear silhouette.
  - Leave out details smaller than 2 px; they turn into noise.
  - Keep strong contrast between the icon and the background, and between its parts.
  - If the icon has a sign on it, such as a letter, `$`, `?`, `!` or `%`, make the sign contrast strongly with the object.
  - If an icon cannot be read at the available size, use text instead.
- Draw icons and other graphics inside the app with volume by default, not flat: 2–3 tones per color, lighter on the top-right, darker on the bottom-left. Keep a shape flat only when it is too small for the shading to read.
- Keep icons consistent with each other within the app.
- Colors by meaning: blue = info, red = error, yellow = warning, green = success.

### Icon states
Optional patterns for icons inside the app. Skip this section if the app does not show states with icons.
- Keep the same base object in every state.
- **Off or unavailable:** strike the object through or add a `×` badge. Gray alone is not enough.
- **Result:** add a badge in the bottom-right corner, attached to the object with no gap. On the back screen it is `#888888`; on the front screen:
  - error or no connection: red `×`;
  - success: green checkmark;
  - critical: red `!`;
  - warning: yellow `!`.
- **In progress:** add signal arcs on the right of the object.
- **Level:** on the front, change the color of the indicator part only. On the back, change the number of elements: sound 50% = 1 wave, 100% = 2 waves.
- Combine at most 2 objects; the second is smaller, on the right or bottom-right.

## Before finishing

1. No firmware files were changed.
2. `npm run typecheck` and `npm run build` succeed.
3. The manifest is valid, its `id` follows the rules above, and `api_version` matches the endpoints used.
4. Only the endpoints listed above are used, with the allowed methods.
5. The app was installed and launched on a device with `npm run app:install` and `npm run app:run`, if one
   is available. If not, say so.
