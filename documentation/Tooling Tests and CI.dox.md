# Tooling, Tests and CI {#tooling-tests-ci}

This page complements @ref build-system with the host-side scripts, the bundle and resource wiring, the test suite and the CI workflows.

# Build wiring

## Layers and targets

`fbt-project.json` declares three build layers, all git submodules: `fbtng` (the build tool), `core_libs` (the furi runtime) and `freertos`. `project.scons` at the repository root adds the project specific variables (`U5_TARGET_HW` 22, `SIL_TARGET_HW` 65, `SIL917_INSECURE` true, `INTERCOM_FORCE_VERSION`) and decides what to build: with `TARGET_HW` unset it builds the Si917 target, then the U5 target, then the `bsbdist` components that produce bundles and `dist/`; with `TARGET_HW` set it builds one target only.

`bsb_common_env_init.scons` registers every library under `lib/` as a component (`lib/register_bsb_common_libs.scons`), adds `applications/` to the lint sources, and injects the `INTERCOM_FORCE_VERSION` define.

The custom SCons tools in `scripts/fbt_tools/` and the environment modules in `scripts/fbt_env_modules/` supply: asset converters (`bsb_assets.py`), the JavaScript application copier (`bsb_apps.py`), CA bundle refresh (`bsb_certs.py`), update bundles and flash over HTTP (`bsb_update_bundle.py`), external application support (`fbt_extapps.py`), Si917 image conversion, signing and flashing (`fwbin_silabs.py`, `fwsign_silabs.py`, `fwflash_silabs.py`).

## Targets not covered by the build system page

| Target | Effect |
| --- | --- |
| `dist_signed` | Signed Si917 images and `*_signed.tgz` bundles. Exists only when `SI917_SIGN_KEYSTORE` or all three `SI917_SIGN_SERVICE_*` variables are set. |
| `update_bundle` | Build the update bundle without installing it into `dist/` |
| `openapi_spec`, `openapi_dist` | Merge the OpenAPI segments, install into the web root or `dist/` |
| `faps`, `fap_<appid>` | Build external applications (`.fap`). The tooling exists but no manifest in the tree uses it. |
| `sdk_tree`, `api_check`, `api_table`, `get_apiversion` | SDK header tree and API symbol table for external applications |
| `proto` | Compile `assets/proto` with nanopb |
| `crypto_provision`, `matter_provision`, `mqtt_provision`, `crypto_wipe`, `mqtt_wipe` | Provisioning scripts against the connected device |
| `update_cacert` | Refresh `assets/shared/ca/cacert.pem` from curl.se |

Build variables not on the build system page: `SIL917_INSECURE`, `SI917_NWP_STACK` (path of the radio image), `SI917_SIGN_KEYSTORE`, `SI917_SIGN_SERVICE_URL`, `SI917_SIGN_SERVICE_TOKEN`, `SI917_SIGN_SERVICE_PROFILE`, `COMMANDER_CLI`, `SI917_PORT`. Application sets not listed there: `917_crash`, `hwtest`, `factory_testing`.

`flash_usb` details: one invocation accepts one preset only, the bundle never contains backup partition resources, and `--signed` with a Si917 image included requires the signing configuration.

## Artifacts

Build products land in `fbt_layers/fbtng/build/f<target>-<firmware or updater>-<C, D or CD>/`. The `dist` target installs `dist/f<U5>-D/busybar-f<U5>-<name>-<DIST_SUFFIX>.<ext>` for `update.tgz`, `bkp.tgz`, `firmware.elf`, `firmware.dfu`, `firmware.bin`, `updater.elf`, `updater.bin`, `sil_firmware.rps`, `sil_firmware.elf`, `sil_nwp.rps`, `openapi.yaml`, plus the signed variants. CI adds a recovery DFU and a SHA-256 sum file.

# Scripts

All device-facing scripts default to `10.0.4.20`: the CLI socket on port 23 and HTTP on port 80.

| Script | Purpose |
| --- | --- |
| `testops.py` | CI and QA device operations: `wait`, `get-version`, `power`, `input`, `device_info`, `sanity`, `uptime`, `debug`, `format`, `update-fw` (HTTP upload with retries and optional OpenOCD reset), `update-bundle`, `unit-tests` |
| `update.py` | `u5 <dfu or hex> [--to-dfu]` flashes the U5 with `dfu-util` or `STM32_Programmer_CLI`; `917 <rps> [--nwp]` uploads and flashes a Si917 image through the CLI |
| `update_bundle.py` | Builds a bundle directory, tar or tgz from stage, DFU, RPS and resource inputs (see @ref updater) |
| `update_over_http.py` | Uploads a bundle to `POST /api/update` with an optional intercom version gate |
| `storage.py` | Remote filesystem operations over the CLI socket: `send`, `receive`, `list`, `mkdir`, `remove`, `format_ext`, `stress` |
| `run.py` (`./run`) | Legacy build and flash driver for `f20` and `f21`. Superseded by the `flash_usb` targets. |
| `streaming.py` | Live display viewer. Targets a WebSocket route that no longer exists. |
| `swagger.py`, `openapi_merge.py` | OpenAPI merge and Swagger UI generation |
| `seq2anim.py`, `image.py`, `audio.py`, `ttf2font.py`, `sound_test.py` | Asset converters (see @ref user-interface) |
| `bsbotp.py` | Build and verify OTP blocks (see @ref hardware) |
| `bin2rps.py`, `sign_silabs_rps.py`, `flashrps.py` | Si917 image wrapping, signing and serial flashing |
| `matter_provision.py`, `mqtt_provision.py`, `vault_provision.py`, `credentials.py`, `crypto_storage.py` | Key and certificate provisioning (see @ref connectivity) |
| `map_analyse_upload.py` | Uploads ELF section sizes and map files to the size analyzer in CI |
| `check-submodules.py` | Verifies that every submodule is on the branch listed in `.github/expected-submodule-branches.txt` |
| `get_silabs_lib.sh` | Fetches a WiseConnect or Simplicity SDK package through Conan |
| `battery_calibration/` | Off-device rig (Arduino plus INA219) and plotting scripts that produce `factory.bat_cal` |
| `debug/platforms/` | OpenOCD and SVD descriptors and GDB helpers |

Shared Python helpers live in `scripts/flipper/`: `app.py` (argparse base), `cli.py` (TCP stream to the CLI), `storage_socket.py` (file transfer over the CLI).

# Tests

The `tests/` directory is a pytest suite that runs against a real device. `tests/README.md` and `tests/AGENTS.md` describe it.

## Set-up

Python 3.12 with Poetry (`pyproject.toml`), or `requirements.txt` for CI. `bootstrap_local.sh` creates the virtual environment, copies `config/.env.example` to `config/.env`, probes the device and runs two smoke tests. Configuration keys (`config/config.py`): `BUSYBAR_IP` (10.0.4.20), `DEVICE_CHECK_PORT` (80), `DAPLINK_U5_ID`, `DAPLINK_917_ID`, `BSB_FIRMWARE_PATH` (a checkout with the toolchain and debug descriptors, for crash traces), `SESSION_LOG_DIR`, plus cloud, Wi-Fi, BLE, Home Assistant and Allure settings.

## Harness

`tests/conftest.py` defines session fixtures (`web_base_url`, `device_flasher`, an autouse fixture that dismisses the first-boot screen) and function fixtures: a logged `requests.Session`, one typed client per API area (`system_api`, `wifi_api`, `storage_api`, `assets_api`, `account_api`, `ble_api`, `settings_api`, `input_api`, `streaming_api`, `update_api`, `busy_api`, `smart_home_api`), CLI connections, and a device information cache.

An autouse health monitor probes `GET /api/version` before and after every test. On failure it classifies the problem (port unreachable, API unhealthy, tolerated 503, crash detected through the bench serial logger), requests a GDB trace, resets the device through the DAPLink probe and fails the test with the reason.

## Markers and coverage

`pyproject.toml` declares the markers: `cli`, `frontend`, `api`, `mqtt`, `uses_cloud`, `external_service`, `state_publisher`, `uses_ble`, `matter`, `mdns`, `rate_limiter`, `long_running`, `regression`, `uses_si917`, `mtls`, `schemathesis` and others. `regression` tests run only on `-rc` tags or when a commit message contains `[run regression]`.

| Directory | Coverage |
| --- | --- |
| `integration/cli/` | Configuration, diagnostics, `fetch` (including mutual TLS against a local server), JavaScript `fetch` and `localStorage`, peripherals, power, shell, storage, submenus, system |
| `integration/frontend/` | Every HTTP API area: account, assets and display screenshots against reference frames, BLE, busy timer, token authentication over USB and Wi-Fi, CORS for every documented method, display drawing and priorities, input, Matter, schema conformance with schemathesis against the served OpenAPI document, settings and tokens, state publisher, storage, streaming, system, update, web UI, Wi-Fi |
| `integration/discovery/` | mDNS name, port, TXT records, re-announce on rename |
| `integration/matter/` | Commissioning into a real Home Assistant (skipped without `HA_URL`) |
| `integration/mqtt/` | Broker connectivity, cloud initiated unlink |
| `integration/state_publisher/` | Protobuf contract, events and rate limiting over WebSocket, MQTT and BLE |

Unit tests on the device are a separate thing: the `unit_tests` application set adds the `unit_tests` CLI command with minunit suites (argparse, crypto, datetime, device name, HTTP, JavaScript application settings, JavaScript, JSON helper, pipe, rate limiter, record, RLE, RTC, setting provider, state, storage, tar, timer, URL, XPM).

# CI

Workflows live in `.github/workflows/`. Builds run on a self-hosted `BsbShell` runner, device tests on `BusyBarTest` and `BusyBarBrickTest` benches. An indexer receives the artifacts and serves them from `https://update.busy.app/builds/busybar-firmware/<branch>/`. Pull requests from forks never upload.

| Workflow | Trigger | What it does |
| --- | --- | --- |
| `build.yml` | Push to `dev`, tags, pull requests | Builds the `22_65` pair (and `21_64` on demand) with `dist` and `dist_signed`, checks that generated files are unchanged, uploads bundles, ELFs and size reports, comments on the pull request, then runs the updater, integration, unit and brick test jobs and the `tests-passed` gate |
| `compact-build.yml` | Push to `dev`, pull requests | Release profile build (`DEBUG=0 COMPACT=1`) to prove the size optimized image links |
| `updater-test.yml` | Called by build | Flashes the bundle on a bench (`power reboot`, `debug 1`, `format`, `update-fw`), runs `sanity`, updates again with the same package |
| `unit-test.yml` | Called by build | Flashes the `unit_tests` build and runs the `unit_tests` CLI command |
| `integration-tests.yml` | Called by build, manual | Flashes, runs `pytest` with the marker filter, uploads results to Allure TestOps and the serial run log to S3, comments a summary |
| `brick-test.yml` | Called by build | Flashes the `917_crash` build, verifies that `sanity` fails, recovers with the normal bundle through the storage and CLI fallback path |
| `lint.yml` | Pull requests | Submodule branch check, `reuse lint`, `./fbt lint` for `f21` and `f64` |
| `docs.yml` | Push to `dev`, tags, pull requests | `./fbt doxygen` with warnings treated as errors |
| `frontend-build.yml` | Pull requests touching `assets/frontend/` | Rebuilds the web UI and fails if the committed build differs |
| `openapi-diff.yml` | Changes to the OpenAPI sources | `oasdiff` breaking-change report, semantic version bump enforcement, `API_VERSION` consistency check |
| `copilot-code-review.yml` | Pull request ready for review | Requests an automated review |
| `reindex.yml` | Release published | Asks the indexer to rebuild the release directory |

Release profile: a git tag builds with `DEBUG=0 COMPACT=1`, signs the Si917 images when the secrets exist, and uploads. Tags containing `-rc` run the full regression suite. Publishing the GitHub release triggers the reindex.

# Licensing and style tooling

- `REUSE.toml` and `LICENSES/` declare the licenses: GPL-2.0-or-later for the firmware sources, MIT for `applications_js/`, CC-BY-SA-4.0 for assets and documentation, OFL-1.1 for fonts, Apache-2.0 for the Matter glue and Swagger UI, and a Silicon Labs license for the radio firmware. `reuse lint` runs in CI. `LICENSE.md` still says CC-BY 4.0 for assets, which disagrees with `REUSE.toml`.
- `.clang-format` is enforced by `./fbt lint` and applied by `./fbt format` (see @ref code-style). `.clangd` adjusts flags for the language server.
- `./fbt doxygen` builds this documentation from `documentation/doxygen/Doxyfile.cfg`; `./fbt doxy` opens it (see @ref docs-howto).
