# JavaScript Applications {#javascript-applications}

# Introduction

JavaScript applications are a locally-run form of [HTTP API](https://docs.busy.app/bar/dev/http-api) applications.
To achieve this, the @ref firmware includes a JavaScript [interpreter](https://jerryscript.net/) to run user scripts
and a set of standard and custom interfaces to be used by these scripts.

Additionally, the firmware is able to enumerate and list currently installed applications and display them
in the [APPS menu](https://docs.busy.app/bar/apps-and-integrations) for easy access.

# Quick start

To facilitate application development, [dedicated tooling](https://github.com/busy-app/busy-tools) is available.
New developers are advised to try it first, then return to this page for more information as needed.

# Application structure

## Applications directory

The firmware is looking for JavaScript applications in the `/ext/user_assets` directory on the EMMC storage.

Every subfolder in this directory will be searched for a valid application file structure.

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

The only allowed special characters are: `_` (underscore), `-` (minus) and `.` (period). The name cannot start with a period.

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

Application manifest file is a JSON file that describes all of the application's properties that are necessary to run it.

| Field            | Purpose                       | Required? | Default |
| ---------------- | ----------------------------- | :-------: | :-----: |
| `format_version` | File schema version           | YES [1]   | -       |
| `id`             | Unique application identifier | YES [2]   | -       |
| `name`           | Display name (shown in GUI)   | YES       | -       |
| `version`        | Application version (`x.y.z`) | YES       | -       |
| `api_version`    | Minimum HTTP API version      | YES [3]   | -       |
| `description`    | Free form description         | NO        | `""`    |
| `author`         | Application author            | NO        | `""`    |
| `heap_size_kib`  | Memory size for dynamic data  | NO  [4]   | `128`   |
| `debug`          | Hide if developer mode is off | NO        | `false` |

- [1] Defined by the firmware version. Currently equal to 1.
- [2] Must be the same as the root directory name.
- [3] Firmware only checks that the expected API version is not newer than the available version.
- [4] Increase if encountering "out of memory" errors. 512 KiB is the current maximum.

See @subpage javascript-applications-manifest-schema for the complete file format description.

## Settings file

Settings file is a JSON file that describes the settings relevant for the application.

An individual setting entry can be of one of the following types:

| Type          | Explanation                   |
| ------------- | ----------------------------- |
| `boolean`     | Boolean value (`true` or `false`) |
| `integer`     | Signed integer number         |
| `string`      | Arbitrary text string         |
| `enum`        | List of pre-defined options   |
| `color`       | Colour in `RGB` or `RGBA` format  |
| `time`        | Time in `HH:MM:SS` format     |
| `geolocation` | Geographic coordinates        |

An arbitrary number of settings is supported, each of which can be any of the supported types. Additionally,
settings can be grouped via the `group` directive, with a maximum of 4 nested levels.

The graphical interface for viewing and editing the settings is handled by the firmware, so the application
does not need to implement it by itself. However, not all settings types (namely, `color` and `geolocation`)
can be edited via @bsb graphical interface due to display and input limitations. Full editing capability is
available from the Web UI or programmaticaly via the HTTP API.

While running, the application only has the ability to read its settings via the `GET /api/apps/settings` HTTP request.

See @subpage javascript-applications-settings-schema for the complete file format description.

## HTTP Requests

Requests to the [HTTP API](https://docs.busy.app/bar/dev/http-api) are performed via the built-in `fetch` function.

It supports the same syntax as its regular counterpart from browsers with several limitations.
See [MDN](https://developer.mozilla.org/en-US/docs/Web/API/Window/fetch) for more info on the usage.

Applications must make requests to `http://localhost/api/...` in order to interact with the on-device web server.

Example snippet that outputs "Hello world!" on the front display:

```js
fetch("http://localhost/api/display/draw", {
    method: "POST",
    body: JSON.stringify({
        application_name: "app.hello.world",
        elements: [
            {
                id: "0",
                type: "text",
                x: 72 / 2,
                y: 16 / 2,
                align: "center",
                text: "Hello World!",
                font: "small"
            }
        ]
    })
});
```

For application isolation purposes, only the following API endpoints are accessible from within the applications:

| Endpoint             | Allowed method(s) |
| -------------------- | :---------------: |
| `apps/settings`      | `GET` [1]         |
| `display/brightness` | `GET`, `POST`     |
| `display/draw`       | `POST`, `DELETE`  |
| `audio/play`         | `POST`, `DELETE`  |
| `busy`               | `GET`, `PUT`      |
| `name`               | `GET`             |
| `audio/volume`       | `GET`, `POST`     |
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

- [1] The request is only allowed if the `app_id` request parameter matches the running application ID.

The `fetch` function can also be used to access remote HTTP(S) resources in the same manner, provided there is a working Wi-Fi connection.

## Input handling

Applications that need to react to user input must use the built-in `listen` function:

```js
const unbind = listen("input", inputHandler);

function inputHandler(event) {
    switch(event.key) {
        // Various handlers
    }
}

// Other logic

// Input not needed anymore
unbind();
```

The received `event` object will have the following properties:

| Property | Explanation                                  |
| -------- | -------------------------------------------- |
| `key`    | Name of the control that produced the event  |
| `action` | Type of the event that occurred              |
| `delta`  | For the encoder wheel only: number of clicks |

The `key` property may take one of the following values:

| Value     | Hardware control type   |
| --------- | ----------------------- |
| `encoder` | Encoder wheel           |
| `ok`      | Encoder button          |
| `start`   | Start/Stop (big button) |
| `back`    | Back button             |

The `action` property may take one of the following values:

| Value              | Explanation                           |
| ------------------ | ------------------------------------- |
| `press`            | Button was pressed [1]                |
| `release`          | Button was released [1]               |
| `short`            | Button was pressed shortly [1]        |
| `long`             | Button was held for a longer time [1] |
| `repeat`           | Button is being held continuously [1] |
| `clockwise`        | Rotated clockwise [2]                 |
| `counterclockwise` | Rotated counterclockwise [2]          |

- [1] Only for buttons
- [2] Only for encoder wheel

Additionally, if the `key` is equal to `encoder`, then an additional `delta` property will be present,
which represents the number of clicks that occurred (currently, only 1). The sign denotes rotation direction (negative is counterclockwise).

It is recommended that applications react to the `short` action as a normal button press and to `long` action as the long button press.
Raw `press` and `release` actions may be useful in games or other applications that need to track explicit press and release events.

See @subpage javascript-applications-input-event-schema for the complete event format description.

### Back button behaviour
A long press of the Back button is handled outside of the application and is used to force-quit from an application.

@page javascript-applications-manifest-schema Appendix 1: Manifest File Schema

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
            "pattern": "^[a-zA-Z0-9_\\-][a-zA-Z0-9_\\-.]{0,31}$",
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
        "api_version": {
            "type": "string",
            "pattern": "^[0-9]+\.[0-9]+\.[0-9]+$",
            "description": "Minimum required HTTP API version"
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
            "maximum": 512,
            "default": 128,
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
        "version",
        "api_version"
    ]
}
```

@page javascript-applications-settings-schema Appendix 2: Settings File Schema

```json
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "type": "object",
    "properties": {
        "format_version": {
            "type": "number",
            "const": 1,
            "description": "Settings file format version"
        },
        "version": {
            "type": "integer",
            "minimum": 1,
            "description": "Settings schema version (bump to discard previously stored values)"
        },
        "fields": {
            "$ref": "#/$defs/fields",
            "description": "List of settings entries"
        }
    },
    "required": [
        "format_version",
        "version",
        "fields"
    ],
    "$defs": {
        "fields": {
            "type": "object",
            "minProperties": 1,
            "propertyNames": {
                "pattern": "^[a-z][a-z0-9_]*$"
            },
            "additionalProperties": {
                "$ref": "#/$defs/node"
            },
            "description": "Map of settings entries, keyed by the entry ID (must be unique on the same level)"
        },
        "node": {
            "type": "object",
            "properties": {
                "type": {
                    "enum": ["boolean", "integer", "string", "enum", "color", "time", "geolocation", "group"],
                    "description": "Entry type"
                },
                "label": {
                    "type": "string",
                    "description": "Display name to be shown in the UI"
                },
                "description": {
                    "type": "string",
                    "description": "Help text to be shown in the UI"
                }
            },
            "required": [
                "type",
                "label"
            ],
            "oneOf": [
                { "$ref": "#/$defs/boolean" },
                { "$ref": "#/$defs/integer" },
                { "$ref": "#/$defs/string" },
                { "$ref": "#/$defs/enum" },
                { "$ref": "#/$defs/color" },
                { "$ref": "#/$defs/time" },
                { "$ref": "#/$defs/geolocation" },
                { "$ref": "#/$defs/group" }
            ]
        },
        "boolean": {
            "properties": {
                "type": { "const": "boolean" },
                "default": {
                    "type": "boolean",
                    "description": "Default value"
                }
            },
            "required": ["default"]
        },
        "integer": {
            "properties": {
                "type": { "const": "integer" },
                "default": {
                    "type": "integer",
                    "minimum": -2147483648,
                    "maximum": 2147483647,
                    "description": "Default value (must be within [min, max])"
                },
                "min": {
                    "type": "integer",
                    "minimum": -2147483648,
                    "maximum": 2147483647,
                    "default": -2147483648,
                    "description": "Minimum allowed value, inclusive (must not exceed max)"
                },
                "max": {
                    "type": "integer",
                    "minimum": -2147483648,
                    "maximum": 2147483647,
                    "default": 2147483647,
                    "description": "Maximum allowed value, inclusive"
                },
                "step": {
                    "type": "integer",
                    "minimum": 1,
                    "default": 1,
                    "description": "Value increment in the UI (max - min must be evenly divisible by step)"
                }
            },
            "required": ["default"]
        },
        "string": {
            "properties": {
                "type": { "const": "string" },
                "default": {
                    "type": "string",
                    "description": "Default value (length must be within [min_length, max_length])"
                },
                "min_length": {
                    "type": "integer",
                    "minimum": 0,
                    "maximum": 255,
                    "default": 0,
                    "description": "Minimum value length in bytes (must not exceed max_length)"
                },
                "max_length": {
                    "type": "integer",
                    "minimum": 0,
                    "maximum": 255,
                    "default": 64,
                    "description": "Maximum value length in bytes"
                },
                "sensitive": {
                    "type": "boolean",
                    "default": false,
                    "description": "If true, mask the value in the UI"
                }
            },
            "required": ["default"]
        },
        "enum": {
            "properties": {
                "type": { "const": "enum" },
                "default": {
                    "type": "string",
                    "description": "Default value (must match one of the option values)"
                },
                "options": {
                    "type": "array",
                    "minItems": 1,
                    "maxItems": 32,
                    "items": {
                        "type": "object",
                        "properties": {
                            "value": {
                                "type": "string",
                                "description": "Option value to be stored (must be unique within the options)"
                            },
                            "label": {
                                "type": "string",
                                "description": "Option name to be shown in the UI"
                            }
                        },
                        "required": ["value", "label"]
                    },
                    "description": "List of available options"
                }
            },
            "required": ["default", "options"]
        },
        "color": {
            "properties": {
                "type": { "const": "color" },
                "default": {
                    "type": "string",
                    "pattern": "^#[0-9A-Fa-f]{6}([0-9A-Fa-f]{2})?$",
                    "description": "Default value in #RRGGBB or #RRGGBBAA format (#RRGGBB is fully opaque)"
                }
            },
            "required": ["default"]
        },
        "time": {
            "properties": {
                "type": { "const": "time" },
                "default": {
                    "type": "string",
                    "pattern": "^([01][0-9]|2[0-3]):[0-5][0-9](:[0-5][0-9])?$",
                    "description": "Default value in HH:MM or HH:MM:SS format"
                }
            },
            "required": ["default"]
        },
        "geolocation": {
            "properties": {
                "type": { "const": "geolocation" },
                "default": {
                    "type": "object",
                    "properties": {
                        "mode": {
                            "enum": ["auto", "fixed"],
                            "description": "Location source: determined by the device (auto) or explicit coordinates (fixed)"
                        },
                        "name": {
                            "type": "string",
                            "minLength": 1,
                            "maxLength": 128,
                            "description": "Location name to be shown in the UI (1 to 128 bytes)"
                        },
                        "lat": {
                            "type": "number",
                            "minimum": -90,
                            "maximum": 90,
                            "description": "Latitude in degrees (fixed mode only)"
                        },
                        "lon": {
                            "type": "number",
                            "minimum": -180,
                            "maximum": 180,
                            "description": "Longitude in degrees (fixed mode only)"
                        }
                    },
                    "required": ["mode", "name"],
                    "oneOf": [
                        {
                            "properties": { "mode": { "const": "auto" } },
                            "not": { "anyOf": [{ "required": ["lat"] }, { "required": ["lon"] }] }
                        },
                        {
                            "properties": { "mode": { "const": "fixed" } },
                            "required": ["lat", "lon"]
                        }
                    ],
                    "description": "Default value"
                }
            },
            "required": ["default"]
        },
        "group": {
            "properties": {
                "type": { "const": "group" },
                "fields": {
                    "$ref": "#/$defs/fields",
                    "description": "List of nested settings entries (groups can be nested up to 4 levels deep)"
                }
            },
            "required": ["fields"]
        }
    }
}
```

@page javascript-applications-input-event-schema Appendix 3: Input Event Schema

```json
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "type": "object",
    "properties": {
        "key": {
            "type": "string",
            "enum": ["encoder", "ok", "start", "back"],
            "description": "Name of the control that produced the event"
        },
        "action": {
            "type": "string",
            "enum": ["press", "release", "short", "long", "repeat", "clockwise", "counterclockwise"],
            "description": "Type of the event that occurred"
        },
        "delta": {
            "type": "integer",
            "description": "For the encoder wheel only: number of clicks (negative is counterclockwise)"
        }
    },
    "required": ["key", "action"],
    "oneOf": [
        { "$ref": "#/$defs/button" },
        { "$ref": "#/$defs/encoder" }
    ],
    "$defs": {
        "button": {
            "properties": {
                "key": { "enum": ["ok", "start", "back"] },
                "action": { "enum": ["press", "release", "short", "long", "repeat"] }
            },
            "not": { "required": ["delta"] },
            "if": {
                "properties": { "key": { "const": "back" } }
            },
            "then": {
                "properties": { "action": { "not": { "const": "long" } } }
            }
        },
        "encoder": {
            "properties": {
                "key": { "const": "encoder" },
                "action": { "enum": ["clockwise", "counterclockwise"] }
            },
            "required": ["delta"],
            "if": {
                "properties": { "action": { "const": "clockwise" } }
            },
            "then": {
                "properties": { "delta": { "const": 1 } }
            },
            "else": {
                "properties": { "delta": { "const": -1 } }
            }
        }
    }
}
```
