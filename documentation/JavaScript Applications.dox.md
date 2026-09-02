# JavaScript Applications {#javascript-applications}

This is a stub page about the JavaScript applications. Expand it with new aspects as they become established.

# Introduction

JavaScript applications are a locally-run form of [HTTP API](https://docs.busy.app/bar/dev/http-api) applications.
To achieve this, the @ref firmware includes a JavaScript [interpreter](https://jerryscript.net/) to run user scripts
and a set of standard and custom interfaces to be used by these scripts.

Additionally, the firmware is able to enumerate and list currently installed applications and display them
in the [APPS menu](https://docs.busy.app/bar/apps-and-integrations) for easy access.

JavaScript applications are enabled by default and do not require an enable flag on the device.

# Application structure

## Applications directory

The firmware is looking for JavaScript applications in the `/ext/user_assets` directory on the EMMC storage.

Every subfolder in this directory will be searched for a valid application file structure.

Applications may be installed over HTTP as TAR or TGZ packages. Package `appmeta/`, `scripts/`,
and any resource directories directly at the archive root; do not wrap them in an additional app
directory. The firmware derives the app ID from `appmeta/manifest.json`.

Installation is staged under `/ext/tmp/app_install/`, on the same filesystem as
`/ext/user_assets/`. Archive paths and sizes are validated before the staged directory is promoted
with a filesystem rename. When updating an existing app, the old directory is first renamed to a
backup and restored if promotion fails. Only a package with a newer SemVer replaces an installed
version. Saved settings are preserved across updates.

The application management endpoints are:

| Method | Endpoint | Purpose |
| ------ | -------- | ------- |
| `POST` | `/api/apps/install` | Upload and install a TAR or TGZ binary body |
| `GET` | `/api/apps/list` | List valid installed applications |
| `POST` | `/api/apps/launch?application_name=<id>` | Launch an app immediately, bypassing Start/Setup |
| `DELETE` | `/api/apps/remove?application_name=<id>` | Remove an app and its saved settings |
| `GET`, `PUT` | `/api/apps/settings?application_name=<id>` | Read or update settings |

The current storage layout does not distinguish firmware-bundled JavaScript apps from uploaded
ones, so removal protection for stock apps will be added when those sources move to a separate
read-only location.

## Firmware source directory

JavaScript applications maintained with the firmware live in `applications_js/<app-id>/`.
Application sources place `main.ts` or `main.js` at the application root, with metadata in
`appmeta/`. Shared build and development tooling lives in `applications_js/.platform/`.

The JavaScript application builder produces the deployed package described below, including the
compiled `scripts/main.js`, under `assets/applications_js/<app-id>/`.

## Application file structure

Every JavaScript application must implement the following basic file structure:

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

The firmware will look for these hardcoded file names when looking for applications.

### Root directory

Required. The name must be at most 32 characters long and contain only ASCII alphanumeric characters.

The only allowed special characters are: `_` (underscore), `-` (minus) and `.` (period).

To improve the name uniqueness, a reverse domain name scheme is recommended, but is not required.

### appmeta directory

Required. Contains the application metadata. See the table below for contents summary.

| File                  | Purpose                              | Required? |
| --------------------- | ------------------------------------ | :-------: |
| `manifest.json`       | Application manifest (see below)     | YES       |
| `settings.json`       | Application settings description     | NO [1]    |
| `icon_front_8x8.png`  | Icon to be shown in front display UI | NO [2]    |
| `icon_back_11x11.png` | Icon to be shown in back display UI  | NO [3]    |

- [1] If not present, it is assumed that the application does not have any settings.
- [2] Must be a 8x8px PNG colour image. If not present, a default icon will be used.
- [3] Must be a 11x11px PNG greyscale image. If not present, a default icon will be used.

### scripts directory

Required. Contains the JavaScript script files.

The `main.js` file is the application entry point and is required. Any number of additional `.js` files may be present if required.

### Additional directories

The root directory may also contain any number of additional directories. The maximum length of the full path is limited to 256 characters and is subject to the same character restrictions as the root directory name, with the addition of the directory separator `/` (forward slash).

It is recommended, but not required, to group the resource files necessary for the application to comprehensively named directories, e.g.

- `resources`, `data` -- any kind of resource files used by the application
- `images`, `graphics`, `gfx` -- graphic resources (such as icons or background images)
- `animations` -- self explanatory
- `sounds` -- self explanatory, etc.

## Manifest file

Application manifest file is a JSON file that matches the following schema:

```json
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "type": "object",
    "properties": {
        "format_version": {
            "type": "number",
            "description": "Manifest file format version"
        },
        "id": {
            "type": "string",
            "minLength": 1,
            "maxLength": 32,
            "pattern": "^[a-zA-Z0-9._-]+$",
            "description": "Unique application ID (must match the root directory name)"
        },
        "name": {
            "type": "string",
            "description": "Display name to be shown in the UI"
        },
        "version": {
            "type": "string",
            "pattern": "^[0-9]+\.[0-9]+\.[0-9]+$",
            "description": "Application version"
        },
        "description": {
            "type": "string",
            "default": "",
            "description": "Free form application description"
        },
        "author": {
            "type": "string",
            "default": "",
            "description": "Application author (individual, company, etc.)"
        },
        "heap_size_kib": {
            "type": "number",
            "minimum": 1,
            "maximum": 256,
            "default": 32,
            "description": "The amount of memory reserved for application heap, in KiB"
        },
        "debug": {
            "type": "boolean",
            "default": false,
            "description": "If true, display in UI only when developer mode is enabled"
        }
    },
    "required": [
        "format_version",
        "id",
        "name",
        "version"
    ]
}
```

## Settings file

The optional `appmeta/settings.json` file describes settings that are rendered by the native Setup
screen and persisted separately from the application package. Setting names use lowercase letters,
numbers, and underscores. A name must begin with a lowercase letter.

Each settings level must contain either groups or values. Mixing group rows and editable value rows
at the same level is not supported. Groups may be nested up to four levels deep.

Supported types correspond to the native variable-item list:

| Type | Value | Additional properties |
| ---- | ----- | --------------------- |
| `group` | Nested object | `settings`, optional `sub_label_setting` |
| `switch` | Boolean | `default` |
| `selector` | String | `default`, `options`, optional `suffix` |
| `spinbox` | Integer | `default`, `min`, `max`, `step`, optional `suffix` |
| `timebox` | Integer minutes | `default`, `min`, `max`, `step` |

For numeric settings, the range must be evenly divisible by `step`, and `min` must be aligned to
that step.

Any setting, group or value, may declare `visible_if`. Its `setting` names another value in the
same group, and `equals` is the value that makes the setting visible. A group hidden this way hides
everything inside it. A setting may not refer to itself.

```json
{
    "format_version": 1,
    "version": 1,
    "settings": [
        {
            "name": "timer",
            "label": "Timer",
            "type": "group",
            "sub_label_setting": "mode",
            "settings": [
                {
                    "name": "mode",
                    "label": "Mode",
                    "type": "selector",
                    "default": "simple",
                    "options": [
                        {"value": "simple", "label": "Simple"},
                        {"value": "interval", "label": "Interval"}
                    ]
                },
                {
                    "name": "cycles",
                    "label": "Cycles",
                    "type": "spinbox",
                    "default": 3,
                    "min": 2,
                    "max": 35,
                    "step": 1,
                    "visible_if": {
                        "setting": "mode",
                        "equals": "interval"
                    }
                }
            ]
        }
    ]
}
```

Values are stored as typed, nested JSON in
`/ext/apps_data/jsrunner/<app-id>.json`. They are not stored in `localStorage` and are not
removed when the application package is updated.

The descriptor's `version` is copied into the stored settings. Changing it discards incompatible
stored values and recreates the nested object from defaults.

The global `Settings` object exposes the current values through Promise-based batch operations:

```js
const config = await Settings.load();
const values = config.values;

values.timer.mode = "interval";
values.timer.cycles = 4;
await Settings.save(values);
```

`Settings.load()` resolves to `{format_version, version, values}`, where `values` is the complete
nested object. `Settings.save(values)` validates the supplied nested batch before saving it and
rejects its Promise if a path or value is invalid. Nested groups, selector options, numeric ranges,
and numeric steps are validated by the same implementation used by the Setup screen and HTTP API.

The values are loaded when the application starts. Changes made from Setup or the HTTP API become
visible to a running application the next time it calls `Settings.load()`.

# Input

The global `Input` object lets an application react to its available physical controls. Registering a
handler keeps the application alive until the application is closed. Registering another handler
for the same control replaces the previous handler.

```js
Input.on("dial", ({direction, delta}) => {
    console.log(direction, delta);
});

Input.on("startPause", ({action}) => {
    console.log(action);
});

Input.on("ok", ({action}) => {
    console.log(action);
});
```

Supported control names and event values:

| Control | Event properties |
| ------- | ---------------- |
| `dial` | `direction`: `clockwise` or `counterclockwise`; `delta`: `1` or `-1` |
| `startPause` | `action`: `press` or `release` |
| `ok` | `action`: `press` or `release` |

Button handlers receive physical state changes only. The input service's internal `short`, `long`,
and `repeat` events are not forwarded because they describe the same physical interaction again.
An application that needs custom hold or repeat behavior can start and stop its own interval from
the `press` and `release` events.

Back and the mode selector are reserved by the firmware and are never delivered to JavaScript.
Pressing Back stops the application and performs normal cleanup.
