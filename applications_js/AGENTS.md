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

| Endpoint             | Allowed method(s) |
| -------------------- | :---------------: |
| `apps/settings`      | `GET` [1]         |
| `display/brightness` | `GET`, `POST`     |
| `display/draw`       | `POST`, `DELETE`  |
| `audio/play`         | `POST`, `DELETE`  |
| `audio/volume`       | `GET`, `POST`     |
| `busy`               | `GET`, `PUT`      |
| `name`               | `GET`             |
| `time`               | `GET`             |
| `wifi/status`        | `GET`             |
| `version`            | `GET`             |
| `access`             | `GET`             |
| `transport`          | `GET`             |
| `status`             | `GET`             |
| `status/device`      | `GET`             |
| `status/firmware`    | `GET`             |
| `status/system`      | `GET`             |
| `status/power`       | `GET`             |

- [1] Only with `app_id` equal to the running app's id. Apps can read their settings but not write them.

This list mirrors [JavaScript Applications](../documentation/JavaScript%20Applications.dox.md#http-requests),
which is authoritative. Check it if the two ever differ. Remote HTTP(S) resources can be fetched with the same
`fetch` when Wi-Fi is connected. Handle network failures there without stopping the app.

## Before finishing

1. No firmware files were changed.
2. `npm run typecheck` and `npm run build` succeed.
3. The manifest is valid, its `id` follows the rules above, and `api_version` matches the endpoints used.
4. Only the endpoints listed above are used, with the allowed methods.
5. The app was installed and launched on a device with `npm run app:install` and `npm run app:run`, if one
   is available. If not, say so.
