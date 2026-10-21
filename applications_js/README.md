# JavaScript Applications

JavaScript Applications (JS Apps for short) are the intended way of building BUSY Bar applications by third-party developers.

## Principle of operation

JS Apps are plain JavaScript files with additional metadata, packaged into a `tar.gz` archive for distribution. The scripts are interpreted using
the on-device [JerryScript](https://jerryscript.net/)-based engine, and the interaction with the firmware is done via loopback `fetch` requests
using the [HTTP API](https://docs.busy.app/bar/dev/http-api). Input events are handled separately, using a built-in `listen` function.

For more information, refer to the [online documentation](https://developer.dev.busy.app/javascript-applications.html) or build and view it locally by running `./fbt doxy`.

## Creating a new application

[BUSY Tools](https://github.com/busy-app/busy-tools) contains the necessary tooling for bootstrapping a BUSY Bar application.

Alternatively, see [JS App Example](app.busy.js_example) for an example of a manually-written, plain JS app.

## Application management

The operations described below can also be performed from the web interface or (partially) via the device UI.

### Installation

Installing an application is a 2-part process:

1. Stage the application archive for installation:

```sh
curl -X 'POST' 'http://10.0.4.20/api/apps/stage' --data-binary '@com.your.app.tgz'
```

If successful, the response will include an install key:
```json
{
  "result": "OK",
  "install_key": 1234567890,
  "staged": {
    ...
  }
}
```

2. Install the staged application, providing the install key from the previous step:

```sh
curl -X 'POST' 'http://10.0.4.20/api/apps/install?install_key=1234567890'
```

On success, the response will contain the `OK` result:
```json
{
  "result": "OK"
}
```

### Uninstallation

It is possible to uninstall a JS App via a single request:

```sh
curl -X 'DELETE' 'http://10.0.4.20/api/apps?app_id=com.your.app'
```

### Launching and closing

It is possible to launch an app via the HTTP API:

```sh
curl -X 'POST' 'http://10.0.4.20/api/apps/launch?app_id=com.your.app'
```

Quitting from a running app is done in the same manner:

```sh
curl -X 'POST' 'http://10.0.4.20/api/apps/quit'
```

No application id is needed for this request, as it will automatically close the currently running app.

See the [on-device OpenAPI documentation](http://10.0.4.20/docs/#/Apps) for a complete list of available endpoints.
