# JavaScript Applications {#javascript-applications}

# Introduction

JavaScript applications are a locally-run form of [HTTP API](https://docs.busy.app/bar/dev/http-api) applications. The @ref firmware includes a JavaScript interpreter ([JerryScript](https://jerryscript.net/)) and a small set of standard web interfaces. A script draws on the displays, plays sounds and blinks the lights by calling the HTTP API of the device over loopback with `fetch()`. Requests from `127.0.0.1` need no API token (see @ref http-api).

The firmware enumerates installed applications and lists them in the [APPS menu](https://docs.busy.app/bar/apps-and-integrations).

What the runtime does **not** offer at this time: a native display, image, font or animation API, button events, a LED or sound API, file system access, a busy timer API, `require()`, and a way for the script to read its own settings values. The Back key always stops the application.

# Application structure

## Applications directory

The firmware looks for JavaScript applications in `/ext/user_assets` on the eMMC. The registry checks every direct subdirectory for a valid application file structure. The directory name must equal the manifest `id`.

## Application file structure

```
/ext/user_assets
└── org.author.example_app
    ├── appmeta
    │   ├── manifest.json
    │   ├── settings.json
    │   ├── icon_front_8x8.png
    │   └── icon_back_11x11.png
    │
    └── scripts
        └── main.js
```

### Root directory

Required. The name must be at most 32 characters long and contain only ASCII alphanumeric characters plus `_`, `-` and `.`. The runtime currently rejects ids that contain `-`, so use `[A-Za-z0-9._]` only. A reverse domain name scheme is the recommended form.

### appmeta directory

Required. Contains the application metadata.

| File | Purpose | Required |
| --- | --- | --- |
| `manifest.json` | Application manifest | Yes |
| `settings.json` | Application settings schema | No. Without it the application has no Setup screen. |
| `icon_front_8x8.png` | 8x8 color icon for the front display | No. A default icon is used. |
| `icon_back_11x11.png` | 11x11 grayscale icon for the back display | No. A default icon is used. |

### scripts directory

Required. `main.js` is the entry point and is always evaluated as an ES module. Relative import specifiers resolve other `.js` files against the `scripts` directory.

### Additional directories

The root directory may contain any number of other directories, for example `images`, `animations` or `sounds`. The full path has a limit of 256 characters. Files under the application directory are addressable from the display API through `path` relative to the application root.

## Manifest file

Parser: `lib/js_app/js_app_manifest.c`. The file must be 1 to 512 bytes.

```json
{
    "format_version": 1,
    "id": "app.busy.js_example",
    "name": "JS App Example",
    "version": "0.1.0",
    "description": "A simple JS application example",
    "author": "BUSY Bar team",
    "heap_size_kib": 128,
    "debug": false
}
```

| Key | Type | Required | Rules | Default |
| --- | --- | --- | --- | --- |
| `format_version` | number | Yes | Must be 1. Other values log a warning. | |
| `id` | string | Yes | Must equal the directory name | |
| `name` | string | Yes | Shown in the APPS menu and the navigation bar | |
| `version` | string | Yes | `major.minor.patch` | |
| `description` | string | No | | `""` |
| `author` | string | No | | `""` |
| `heap_size_kib` | number | No | 1..512. The whole JavaScript heap. | 128 |
| `debug` | boolean | No | `true` hides the application unless Dev mode is on | `false` |

## Settings file

`appmeta/settings.json` declares settings that the user can edit on the device in the Setup screen. Parser: `lib/js_app/js_app_settings.c`.

Root keys: `format_version` (must be 1), `version` (number greater than 0, the schema version stored with the values), `fields` (non-empty object of field nodes). Field ids start with `[a-z]` and continue with `[a-z0-9_]`. Every field has `label` (required), `description` (optional) and `type`:

| Type | Extra keys | Rules |
| --- | --- | --- |
| `boolean` | `default` | |
| `integer` | `default`, `min`, `max`, `step` | `min <= max`, `step` not 0, `(max - min)` divisible by `step`, default in range |
| `string` | `default`, `sensitive`, `min_length`, `max_length` | `max_length` up to 255 (default 64). Sensitive values are shown as `***`. |
| `enum` | `default`, `options` | 1..32 options of `{value, label}` with unique values. The default must be an option value. |
| `color` | `default` | `#RRGGBB` or `#RRGGBBAA` |
| `time` | `default` | `HH:MM` or `HH:MM:SS` |
| `geolocation` | `default` | `{mode: "auto" or "fixed", name, lat, lon}`. `fixed` requires both coordinates. |
| `group` | `fields` | Nested fields, four levels deep at most |

Example:

```json
{
    "format_version": 1,
    "version": 1,
    "fields": {
        "enabled": { "label": "Enabled", "type": "boolean", "default": true },
        "refresh": { "label": "Refresh (s)", "type": "integer", "default": 60, "min": 10, "max": 600, "step": 10 },
        "mode": { "label": "Mode", "type": "enum", "default": "a",
                  "options": [ { "value": "a", "label": "Mode A" }, { "value": "b", "label": "Mode B" } ] },
        "api_key": { "label": "API key", "type": "string", "default": "", "sensitive": true, "max_length": 128 },
        "accent": { "label": "Accent", "type": "color", "default": "#FF6D16" },
        "alarm": { "label": "Alarm", "type": "time", "default": "07:30" }
    }
}
```

The launcher stores the values in `/ext/apps_data/jsrunner/<id>.settings.json` as `{"version": N, "values": {...}}`. Groups nest, enums store the option value, color and time store their formatted strings. On the device only boolean, integer and enum fields are editable. The other types are read-only. **No API currently passes these values to the script.**

# Installation

Upload every file with `POST /api/assets/upload?application_name=<id>&file=<relative path>` and the file content as the body. Parent directories are created. `DELETE /api/assets/upload?application_name=<id>` removes the application. See @ref http-api. The build bundles applications placed in `applications_js/` in the source tree into the resources.

JavaScript applications appear in the APPS menu only when the flag file `/ext/apps_data/apps_menu/js_apps_enabled` exists. Create it with the storage CLI or the storage API.

# Life cycle

1. The APPS menu launches `js_app_launcher` with the application id.
2. The launcher shows the Start screen (icon, name, Start and Setup). Setup edits the settings.
3. Start allocates a runtime context with the manifest heap size, parses `main.js` as a module, links imports and evaluates it. Console output goes to the device log (`console.log` at debug level, `info` at info, `error` at error, tag `JsAppLauncher`).
4. The application stays alive while it has pending timers or fetches. When none remain, the runtime reports termination and the launcher returns to the Start screen.
5. Back aborts the script (the VM throws `aborted` inside busy loops, the runtime stops fetches and frees timers) and returns to Start. From Start, Back returns to the APPS menu.

Errors: a parse or link failure shows "Syntax error, check script.". A missing file or an invalid id shows "App loading failed, reinstall it.". An uncaught exception in top-level code terminates the application silently and logs `JsRunner: Error running script`.

Limits: one JavaScript context firmware-wide, an 8 KiB application thread stack, 250 KiB per script file, the import root is valid only during the initial evaluation (do all static imports at top level).

# Runtime API

Implemented in `applications/services/js_runner/`. Native functions throw `TypeError` on too few arguments or an invalid `this`.

## Globals

| Global | Notes |
| --- | --- |
| `console.log`, `console.info`, `console.error` | The runtime stringifies the arguments and joins them with spaces. Objects print as `[object Object]`, use `JSON.stringify`. |
| `setTimeout(fn, ms)`, `setInterval(fn, ms)` | Exactly two arguments. The runtime raises delays below 10 ms to 10 ms. No extra callback arguments. The runtime prints an exception in a callback as `Uncaught: ...` and the application continues. |
| `clearTimeout(id)`, `clearInterval(id)` | Same function. |
| `fetch(input, init)` | See below. |
| `Request`, `Response`, `Headers`, `URL` | See below. |
| `localStorage` | Persistent string store. |
| `AbortController`, `DOMException`, `FormData` | Empty stubs so that libraries can feature-detect them. They do nothing. |

Language: ES modules, `Promise`, `Array`, `Date` (uses the device time and time zone), `JSON`, `Math`, `RegExp`, `String`, `BigInt`, `Map`, `Set`, `WeakMap`, `WeakSet`, `WeakRef`, `DataView`, typed arrays, `SharedArrayBuffer`, `Atomics`, `Proxy`, `globalThis`. Not available: `Reflect`, Annex B functions (`escape`, `substr`, `__proto__` accessors), Unicode case conversion, `TextEncoder`, `atob`, `structuredClone`, `queueMicrotask`, `crypto`, `WebSocket`, `XMLHttpRequest`, `Blob`.

## fetch

`fetch(input, init)` returns a Promise that resolves when the response headers arrive. Rejections are **strings**, not `Error` objects.

- `input`: a URL string or a `Request`. `http://` and `https://` are supported. TLS uses the device CA bundle.
- `init.method`: string, default GET.
- `init.headers`: a plain object. At most 10 headers. The runtime drops extra ones. `fetch` does not accept a `Headers` instance here.
- `init.body`: converted to a string. Binary bodies are not supported.
- `init.dispatcher.connect.useDeviceKey`: when true, the request presents the device TLS client certificate (mutual TLS with BUSY cloud services).
- Ignored: `signal`, `credentials`, `mode`, `redirect`, `cache`, `keepalive`. There is no timeout.

Each fetch runs in its own thread and keeps the application alive until it completes.

## Request

`new Request(input, init)`. `input` is a URL string or another `Request`. The result is a plain object with `url`, `method`, `headers`, `body` and `dispatcher` properties. There is no `clone()`.

## Response

Produced by `fetch`. Properties: `status`, `statusText`, `ok`, `headers` (a `Headers`), `body` (a readable stream), `bodyUsed`, `type` (`"basic"`), `url`.

Body methods, each returning a Promise, one consumer per response: `text()`, `json()` (rejects with the parse error), `arrayBuffer()`, `bytes()`. `blob()` and `formData()` reject with `"unimplemented"`.

`response.body.getReader()` returns a reader with `read()` (resolves `{done, value}` with `Uint8Array` chunks as they arrive), `cancel()` and `closed`. The body is also an async iterable, so `for await (const chunk of response.body)` works. An example is installed at `/ext/apps_assets/js_runner/example/fetch.js`.

## Headers

`new Headers()` creates an empty set. Methods: `get(name)` (case-insensitive), `set(name, value)`, `has(name)`, `entries()`, `keys()`, `values()`, `forEach(callback)`. The `Headers` object itself is not iterable; iterate over `entries()`. There is no `append` or `delete`.

## URL

`new URL(string)`. No base argument. Read-only getters: `href`, `origin`, `protocol`, `host`, `hostname`, `port`, `pathname`, `search`, and `toString()`. No `hash`, `username`, `password` or `searchParams`.

## localStorage

Strings only, persisted per application id in `/ext/apps_data/jsrunner/<id>.localstorage.json`. Members: `length`, `key(index)`, `getItem(key)`, `setItem(key, value)`, `removeItem(key)`, `clear()`. Non-string arguments throw `TypeError`. Every write rewrites the whole file, so keep it small. Property style access (`localStorage.foo`) is not supported.

# Example

`scripts/main.js`:

```js
import { draw } from "./ui.js";

const KEY = "last_temp";
let temp = localStorage.getItem(KEY);

async function refresh() {
    try {
        const r = await fetch("https://api.example.com/now", { headers: { "Accept": "application/json" } });
        if (!r.ok) { console.error("HTTP", r.status, r.statusText); return; }
        const j = await r.json();
        temp = String(j.temp);
        localStorage.setItem(KEY, temp);
    } catch (e) {
        console.error("fetch failed:", String(e));
    }
    await draw("com.example.weather", temp ?? "--");
}

refresh();
setInterval(refresh, 60000);
```

`scripts/ui.js`:

```js
export async function draw(app, text) {
    const body = JSON.stringify({ application_name: app, elements: [
        { id: "t", type: "text", x: 36, y: 8, align: "center", font: "normal", text, display: "front" },
        { id: "b", type: "text", x: 0, y: 0, align: "top_left", font: "small", text, display: "back" }
    ]});
    const r = await fetch("http://127.0.0.1/api/display/draw", { method: "POST", body,
                          headers: { "Content-Type": "application/json" } });
    if (!r.ok) console.error("draw:", await r.text());
}
```

The draw request's `application_name` is not bound to the JavaScript application id. Use your own id so that clearing elements does not affect other clients.

# CLI

```
js [-i app_id] [file]   run a script file, or open a REPL when no file is given
js -k                   abort the running script
```

The REPL evaluates each line as a classic script and prints the result. `js -i <id> /ext/user_assets/<id>/scripts/main.js` runs an installed application with its own `localStorage` file and prints console output to the terminal instead of the device log. The CLI context uses a 96 KiB heap and the application id `app.busy.cli`. Because only one context may exist, the command fails with "Out of resources" while the launcher runs an application.

# Debugging checklist

1. The application is not in the APPS menu: check the `js_apps_enabled` flag file, that the directory name equals the manifest id, that the manifest is at most 512 bytes and parses (log tag `JsAppManifest`), that `scripts/main.js` exists, and that `debug` is false or Dev mode is on.
2. "Syntax error, check script.": the device log shows `JsRunner: Error parsing script: <file>:<line>: <message>`. A missing module shows `JSGlue: Cannot open file ...`.
3. "App loading failed, reinstall it.": file error, an id containing `-`, or another JavaScript context running (`js -k`).
4. The application returns to Start immediately: top-level code finished without pending timers or fetches, or an uncaught exception occurred.
5. Draw calls do nothing: watch the web server log for `... is required` style validation messages and check `application_name` and `display`.
6. Out of memory: the log shows `JSGlue: jerryscript fatal error` followed by a crash. Raise `heap_size_kib` (up to 512) or reduce live data.

# Host C API

`applications/services/js_runner/js_runner.h`, for firmware code that embeds scripts:

| Function | Purpose |
| --- | --- |
| `js_runner_context_alloc(runner, app_id, heap_size, console_callback, context)` | Start the application thread and the JerryScript context |
| `js_runner_run(handle, path, termination_callback, context)` | Parse, link and start evaluating a module file. Returns after parse and link. |
| `js_runner_run_snippet(handle, code, print_result, ...)` | Evaluate a classic script string |
| `js_runner_join(exec_handle, timeout)` | Wait until no timers or fetches remain |
| `js_runner_abort(exec_handle)`, `js_runner_abort_all(runner)` | Request termination |
| `js_runner_context_free(handle)` | Stop the thread |

The JerryScript port lives in `targets/f21/jerryscript/jerryscript_glue.c` (context allocation, time and time zone, module file reading, fatal errors). Build flags are in `lib/jerryscript.scons`. The engine is the `flipperdevices/jerryscript` fork, branch `bsb`.
