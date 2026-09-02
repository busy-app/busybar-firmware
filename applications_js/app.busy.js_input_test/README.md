# JS Input Test

This app verifies that a JavaScript app receives Busy Bar physical-input events.
It registers handlers for the JavaScript input names:

- `startPause`
- `dial`
- `ok`

The front display shows the latest event and an incrementing event counter. The
LED notification color also changes by control.

Start/Pause and OK emit one `press` and one `release` event. The firmware's
internal `short`, `long`, and `repeat` events are intentionally not forwarded
to JavaScript. Back and the mode selector remain owned by the firmware and are
not exposed to JavaScript applications.

The app intentionally creates no timer or interval. Its registered control
handlers are the only thing keeping the JavaScript runtime alive, so leaving
`READY` visible is also a test of handler-based lifetime management.

The Setup screen also exercises declarative JavaScript app settings:

- nested Display and Input groups
- switches, selectors, a spinbox, and a timebox
- group sublabels
- conditional visibility of Uppercase based on Event format

The script reads the complete nested settings object with `Settings.load()` when
it starts. Applications save a validated nested object with
`Settings.save(values)`.

## Upload

From the firmware repository root, run:

```bash
./scripts/upload_js_input_test.sh
```

The default device address is `10.0.4.20`. To use another address:

```bash
BUSYBAR_DEVICE_IP=192.168.1.50 ./scripts/upload_js_input_test.sh
```

The script validates the local app, checks that the device is running firmware
built from the current repository commit, and creates a USTAR-compatible TGZ. It
then exercises the current application-management API by installing the package,
listing it, reading its nested settings, and launching it immediately:

- `POST /api/apps/install`
- `GET /api/apps/list`
- `GET /api/apps/settings?application_name=app.busy.js_input_test`
- `POST /api/apps/launch?application_name=app.busy.js_input_test`

The firmware only replaces an installed app when the package has a newer SemVer.
If the app is already installed at the same or a newer version, bump `version` in
`appmeta/manifest.json` before running the script again.

## Test on the bar

1. Run the upload script; it installs and launches **JS Input Test** directly.
2. Rotate the dial and press Start/Pause and OK.
3. Press Back to verify that the firmware closes the application.
4. Open **Apps**, select **JS Input Test**, and use **Setup** to edit its nested
   settings.
5. Choose **Start** to test the physical Apps-menu launch path as well.

Short-pressing Back stops the running app and returns to its Start/Setup screen.
Moving the mode selector is handled entirely by the firmware.
