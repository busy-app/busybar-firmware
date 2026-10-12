# Command Line Interface {#cli}

Both MCUs run a command shell built on `lib/cli`. This page lists the transports and every command. Commands marked **U5** exist on the main MCU, commands marked **Si917** on the wireless co-processor and reachable through `sl_cli`.

# Transports

| Transport | MCU | How |
| --- | --- | --- |
| TCP socket (telnet style) | U5 | The `cli_socket` start-up hook listens on port 23 on every interface. Connections over the USB network are always accepted. The hook refuses connections over Wi-Fi unless `sysctl cli_wifi_enabled 1` was set. Each connection gets its own shell thread. This is the primary developer CLI: `telnet 10.0.4.20 23`. |
| Intercom | Both | The U5 command `sl_cli` spawns a shell on the Si917 and relays bytes in both directions over the `Cli` intercom channel. Only one session at a time. |
| UART1 | Si917 | The `cli_uart` service opens UART1 at 230400 baud. Only in the `hwtest` application set. |

There is no USB CDC shell. `cdc_echo` and `cdc_screen` under `applications/services/` are unused test services.

The U5 debug log console is a separate UART (USART6, 230400 baud) and is read-only. The `log` command streams the same output over the CLI.

# Library

`lib/cli/`:

- `CliRegistry`: a mutex protected dictionary of commands. `cli_registry_add_command(registry, name, flags, callback, context)` with a default 4 KiB command stack, or `_ex` with an explicit size. The `cli_on_system_start` hook publishes the main registry as `RECORD_CLI` and adds every `CLICMD` manifest entry.
- Command flags: `ParallelUnsafe`, `InsomniaSafe`, `DontAttachStdio`, `UseShellThread`, `Exclusive` (one instance at a time).
- Callback signature: `void (*)(PipeSide* pipe, FuriString* args, void* context)`. The pipe is installed as the command thread's stdio, so `printf` and `getchar` work. `cli_is_pipe_broken_or_is_etx_next_char()` detects Ctrl+C.
- `args.h`: helpers to read integers, strings and quoted strings from the argument string.
- `cli_status.h`: commands print `RET: 0` (`CLI_STATUS_OK`) or `RET: 1` (`CLI_STATUS_ERROR`) so scripts can parse the result.
- The shell (`shell/`) implements line editing, history, completion, `help`, `?` and `exit`.

On the U5 the `gpio` and `otp` commands exist only while the NVM debug flag is set (`sysctl debug 1`).

# Command reference

## Shell and diagnostics

| Command | MCU | Description |
| --- | --- | --- |
| `help`, `?` | Both | List commands |
| `exit` | Both | Close the session |
| `uptime` | Both | Print uptime |
| `log [error, warn, info, default, debug, trace, ?]` | Both | Stream the log to the session until Ctrl+C, with an optional temporary level |
| `top [interval_ms]` | Both | Thread table: application id, name, state, priority, stack, heap, CPU share. `0` prints once. |
| `free`, `free_blocks` | Both | Heap statistics, free block dump |
| `echo <text>` | Both | Print the arguments |
| `device_info` | Both | All device information pairs (firmware version, target, hashes, MACs, security bits, name) |
| `log_dump [-h] [path]` | U5 | Write device information plus the U5 and Si917 log rings to `/ext/log.txt` or the given path |
| `sl_cli` | U5 | Interactive shell on the Si917 |
| `crash` | Si917 | Crash on purpose (only in the `917_crash` application set) |

## System control

| Command | MCU | Description |
| --- | --- | --- |
| `sysctl debug <1 or 0>` | U5 | Set the NVM debug flag ("Dev mode"). Enables the debug applications menu, the update channel selector, demo mode, `gpio` and `otp`, and suppresses the automatic reboot on intercom failure. |
| `sysctl ui_debug <0, 1, 2>` | U5 | Draw widget bounding boxes on the main layer or all layers, after restart |
| `sysctl cli_wifi_enabled <1 or 0>` | U5 | Allow the CLI socket over Wi-Fi |
| `sysctl websrv_accesslog_level <0..3>` | U5 | Web server access log verbosity |
| `sysctl storage_bkp_unlock <1 or 0>` | U5 | Make `/bkp` writable (debug flag only) |
| `power info` | U5 | Charger and battery readings |
| `power off` | U5 | Power off (refused while USB is connected) |
| `power reboot [sw, hw, u5, 917]` | U5 | Reboot both MCUs, hardware reset through the charger, or one MCU only |
| `power boot <u5 or 917>` | U5 | Reboot into the DFU bootloader of the given MCU |
| `power ch <1 or 0>`, `power ch_limit <pct>`, `power ch_current <mA>` | U5 | Charger enable, charge limit (50..100, or 30..100 with the debug flag), charge current (1..1500) |
| `power pd_info`, `power pd_set <mV>` | U5 | USB-PD status, request a voltage |
| `date [iso8601]` | U5 | Read or set the RTC (input is UTC) |
| `timezone [name]` | U5 | Read or set the time zone |
| `gpio <pin_name> <0 or 1>` | U5 | Set a named GPIO (debug flag only) |
| `otp <dump or program> <OTP1..OTP4> [hex]` | U5 | Dump or irreversibly program an OTP block (debug flag only) |
| `factory_reset [-s]` | U5 | Factory reset after a y/n prompt. `-s` also enters shipping mode. |

## Storage and files

| Command | MCU | Description |
| --- | --- | --- |
| `storage list <path>`, `tree`, `stat`, `info`, `timestamp` | U5 | Inspect files and filesystems. Paths must start with `/ext` or `/bkp`. |
| `storage read <path>`, `write <path>`, `read_chunks <path> <n>`, `write_chunk <path> <n>` | U5 | Text and binary transfer (`write` reads until Ctrl+C) |
| `storage copy`, `rename`, `remove`, `mkdir`, `md5` | U5 | File operations |
| `storage extract <tar> <dst>` | U5 | Unpack a tar archive |
| `storage format <path>` | U5 | Format the partition that owns the path |
| `storage mkfs /` | U5 | Repartition and format both partitions (debug builds with the debug flag) |
| `tar <c or x> <archive> <directory>` | U5 | Create or extract an archive under `/ext` or `/bkp` |
| `storage_benchmark` | U5 | Throughput test (debug application set) |

`scripts/storage.py` and `scripts/flipper/storage_socket.py` drive these commands to transfer whole directories from a host.

## User interface and media

| Command | MCU | Description |
| --- | --- | --- |
| `loader open <name>`, `loader kill` | U5 | Start or stop an application by name or id |
| `input send <key> <type>`, `input dump` | U5 | Inject an input event, or print events as they arrive |
| `display show <front or back> <file>` | U5 | Load an image onto a display |
| `display brightness <0..100 or auto>` | U5 | Set brightness |
| `display gamma back <max> <gamma>` | U5 | Back display gamma table |
| `light_sensor` | U5 | Raw sensor channels and lux |
| `audio start <path>`, `audio stop` | U5 | Play or stop a `.snd` file |
| `fontstat` | U5 | Font cache statistics |
| `status_lights <r> <g> <b>` | Si917 | Set the RGB LED |

## Network and cloud

| Command | MCU | Description |
| --- | --- | --- |
| `netstat` | U5 | Dump lwIP TCP control blocks |
| `fetch [options] <url>` | U5 | HTTP client: `-o` output file, `-d` body, `-H` header, `-X` method, `-k` ignore the server certificate, `-a none/device/cert` client authentication, `-C` and `-K` custom certificate and key, `-v` verbose |
| `matter` | U5 | Sub-shell: `switch on/off`, `startup off/on/toggle/last`, `reset`, `comm`, `fabrics`, `cert <production, development, certification>` |
| `tls_crypto_test` | U5 | Stress the Si917 signer |
| `wifi_cli_test <wifi_init [mode], wifi_deinit, wifi_scan>` | Si917 | Silicon Labs console commands for the radio |

## Update

| Command | MCU | Description |
| --- | --- | --- |
| `update install <manifest>` | U5 | Prepare and apply an unpacked bundle |
| `update install_tar <tar>` | U5 | Unpack, prepare and apply |
| `update install_web <url>` | U5 | Download, unpack, prepare and apply |
| `update 917 <rps>`, `update 917_ta <rps>` | U5 | Flash the Si917 application or radio image directly |
| `update 917_probe` | U5 | Print the Si917 bootloader version (debug flag only) |

## Keys and crypto

| Command | MCU | Description |
| --- | --- | --- |
| `crypto init`, `list <partition>`, `read`, `write`, `gen`, `gen_csr`, `wipe`, `protect <0 or 1>`, `dump` | Si917 | Manage the crypto key storage. Partition 0 is `Main`, 1 is `User`. See @ref connectivity. |
| `crypto_backup create`, `remove`, `restore`, `verify` | U5 | Raw backup of the key storage to `/bkp/crypto_backup.bin` |
| `crypto_test` | Si917 | Sub-shell: `aes`, `ecdsa`, `hmac`, `sha`, `mbedtls_edsa`, `csr`, `aes_vectors` |
| `nvm_test` | Si917 | Sub-shell: `read`, `write`, `del`, `test`, `erase`, `purge` against NVM3 |

## JavaScript and tests

| Command | MCU | Description |
| --- | --- | --- |
| `js [-i app_id] [file]` | U5 | Run a script or open a REPL. `js -k` aborts the running script. See @ref javascript-applications. |
| `unit_tests` | U5 | Run the minunit suites (`unit_tests` application set) |

# Host tooling

`scripts/testops.py` wraps the CLI and the HTTP API for CI: `wait`, `get-version`, `power`, `input`, `device_info`, `sanity`, `uptime`, `debug`, `format`, `update-fw`, `update-bundle`, `unit-tests`. `scripts/update.py` and `scripts/storage.py` use the socket on port 23. See @ref tooling-tests-ci.
