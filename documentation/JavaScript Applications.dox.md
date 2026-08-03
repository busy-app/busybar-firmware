# JavaScript Applications {#javascript-applications}

This is a stub page about the JavaScript applications. Expand it with new aspects as they become established.

# Introduction

JavaScript applications are a locally-run form of [HTTP API](https://docs.busy.app/bar/dev/http-api) applications.
To achieve this, the @ref firmware includes a JavaScript [interpreter](https://jerryscript.net/) to run user scripts
and a set of standard and custom interfaces to be used by these scripts.

Additionally, the firmware is able to enumerate and list currently installed applications and display them
in the [APPS menu](https://docs.busy.app/bar/apps-and-integrations) for easy access.

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

To be decided
