# Hardware targets

## Files and directories

| Item | Purpose |
| ---- | ------- |
| `target.json` | Target description file. |
| `config`   | Target-specific configuration `.h` files. |
| `fbt_conf` | Target-specific build system configuration. |
| `furi_hal` | Target-specific HAL implementation. |
| `src`  | Low-level target-specific sources (e.g. startup and main files). |
| `*.ld` | Target-specific linker script. |


## STM32U595 targets

`STM32U595` is the main microcontroller. It handles most of the tasks except for the wireless connectivity and buttons. The corresponding target numbers are `f2x`.

| Target | Note |
| ------ | ---- |
| `f20`  | Earlest prototype devices that are still supported. |
| `f21`  | Manufacturing prototype. |
| `f22`  | Production version. |

## SiWG917 targets

`SiWG917` is the wireless co-processor microcontroller. It handles wireless communications, smart home (Matter) functionality and acts as a secure crypto enclave. The tar

| Target | Pairs with | Note |
| ------ | ---------- | ---- |
| `f64`  | `f20`, `f21` | |
| `f65`  | `f22` | DFU enter button is `OK` (was `Start`). |
