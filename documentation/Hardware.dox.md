# Hardware {#hardware}

The @bsb is built around two microcontrollers. The STM32U595 ("U5") is the main MCU. It drives both displays, the storage, the USB port, the audio path, the charger and the whole application layer. The SiWG917 ("Si917") is the wireless co-processor. It owns the Wi-Fi and BLE radio, the Matter node, the secure key storage, the physical buttons, the mode switch, the rotary encoder and the RGB status lights. The two MCUs communicate over a high speed UART link called the intercom (see @ref architecture).

# Hardware targets

Each firmware image targets one hardware target. Target descriptors live in `targets/<target>/target.json`, target specific HAL code in `targets/<target>/furi_hal/`, and target configuration in `targets/<target>/config/`. See `targets/README.md` for the directory layout.

| Target | MCU | Role | Notes |
| --- | --- | --- | --- |
| `f20` | STM32U595 | Main MCU | Earliest prototypes that are still supported. Inherits `f21` with different pin assignments. |
| `f21` | STM32U595 | Main MCU | Manufacturing prototype. Base target for all U5 code. |
| `f22` | STM32U595 | Main MCU | Production device. Inherits `f21`. Differs only in the expected OTP hardware target value. |
| `f64` | SiWG917M111 | Wireless co-processor | Pairs with `f20` and `f21`. |
| `f65` | SiWG917M111 | Wireless co-processor | Pairs with `f22`. The DFU button is OK instead of Start. |

The default build pair is `f22` + `f65` (`U5_TARGET_HW=22`, `SIL_TARGET_HW=65` in `project.scons`).

## Differences between targets

| Item | f20 | f21 / f22 |
| --- | --- | --- |
| Si917 interrupt line into the U5 | PC6 | PA0 |
| BQ25798 charger interrupt | PC0 | PC6 |
| Back display VCC enable | PA0 | PC0 |
| Audio amplifier enable | PC7 (shared with the Si917 SWO line) | PH3 |
| Front display power enable output | Push-pull | Open drain |
| Expected OTP `hw_target` | 20 | 21 or 22 |

`f64` and `f65` differ only in the DFU boot button (Start on `f64`, OK on `f65`).

# STM32U595 main MCU

## Core and clocks

- Cortex-M33 with FPU, TrustZone disabled.
- HSE 16 MHz crystal, LSE 32.768 kHz for the RTC.
- PLL1 from HSE gives a 160 MHz system clock. AHB and all APB buses run at 160 MHz. PLL1P (160 MHz) also feeds SDMMC1 and SAI1.
- Instruction cache and flash prefetch are enabled. The data cache is off by default.
- The DWT cycle counter supplies FreeRTOS runtime statistics.

Clock setup lives in `targets/f21/furi_hal/furi_hal_clock.c`.

## Memory map

| Region | Origin | Size | Use |
| --- | --- | --- | --- |
| Flash | `0x08000000` | 4 MiB (linker reserves 2 MiB for the image) | Firmware. `__free_flash_start__` marks the end of the image. |
| RAM | `0x20000000` | 2496 KiB | Data, BSS, heap, 4 KiB main stack |
| SRAM4 | `0x28000000` | 16 KiB | Declared, unused |
| Backup SRAM | `0x40036400` | 2 KiB | Persistent NVM block (flags, boot mode, switch position, version pointer) |
| OTP | Flash OTP area | 512 bytes | Factory provisioning, four 128 byte blocks |

Three linker scripts exist: `STM32U595xx_FLASH.ld` for the normal image, `STM32U595xx_RAM.ld` for the RAM resident updater stage, and `application_ext.ld` for relocatable external applications. Flash pages are 8 KiB. The linker sizes the FreeRTOS heap (heap_4).

The MPU guards the first 1 MiB (null pointer accesses), a 32 byte read-only region at the base of the current task stack (updated on every context switch), and marks the backup SRAM as normal non-cacheable memory.

## Peripherals

| Function | Device | Interface | Pins |
| --- | --- | --- | --- |
| Front display | 72x16 RGB LED matrix, 3 chained constant-current PWM LED driver ICs, 24 scan blocks | OCTOSPI1 in dual-line mode fed by GPDMA (pixel data), TIM8 (grayscale clock, 138 pulses per scan), TIM5 (scan cadence), SPI2 with DMA (row select shift register) | SDI PB1, LE PB0, DCLK PB10, GCLK PB2, scan SDI PC1, scan CLK PB13, latch PA1, power enable PB12 |
| Back display | SSD1320 160x80 4 bpp grayscale OLED | SPI1, simplex TX | SDIN PA7, SCLK PA5, CS PA6, D/C PC4, frame sync PC5, VCC enable PC0 |
| Ambient light sensor | ROHM BH1730 | I2C1, address 0x52 | SCL PB8, SDA PB9 |
| Charger | TI BQ25798 buck-boost charger with ADC | I2C1, address 0xD6 | Interrupt PC6, QON PC13 |
| USB-PD | STM32U5 UCPD1 | | CC1 PA15, CC2 PB15 |
| Audio | SAI1 block A, 44.1 kHz 16 bit mono, DMA | NS4168 class-D amplifier with single-wire enable | FS PA9, SCK PA8, SD PA10, amplifier enable PH3 |
| Storage | eMMC (SD fallback) | SDMMC1, 4 bit bus, IDMA, MMC high speed 52 MHz | D0..D3 PC8..PC11, CK PC12, CMD PD2 |
| USB | USB OTG HS with on-chip HS PHY, TinyUSB, CDC-NCM only | | DM PA11, DP PA12 |
| Intercom | USART1 with RTS/CTS, 11.25 Mbaud, DMA | Si917 USART0 | TX PB6, RX PB7, RTS PB3, CTS PB4 |
| Si917 console | USART2 | Si917 ULP-UART: log capture at 230400 baud and the ROM bootloader at 115200 baud during updates | TX PA2, RX PA3 |
| U5 debug console | USART6, 230400 baud | | TX PC3, RX PC2 |
| Si917 control | GPIO | Reset PA4, SWO (boot select) PC7, interrupt PA0 | |
| SWD | | Only reachable on the debug board | SWDIO PA13, SWCLK PA14 |
| RTC | On LSE, millisecond resolution | | |
| RNG | Hardware RNG on HSI48 | | |

There is no QSPI flash and no PSRAM. OCTOSPI1 acts as a DMA-fed shifter for the LED matrix, not as a memory interface. There are no buttons on the U5. The only U5 side input is a fallback confirm pin (PH3) that is polled when the firmware is built without the intercom service.

## Power

The BQ25798 controls the battery path, the 3V3 rails, hardware reset, power off and shipping mode. The power service (see @ref services) programs 4200 mV charge voltage and a 1500 mA maximum charge current, starts with a 500 mA input limit and raises it after USB-PD negotiation (a 9 V request is issued when the source offers more than one PDO). An NVM flag requests shipping mode, which the device enters once VBUS is absent.

The power service computes the state of charge from the charger ADC with a calibration table stored at `/bkp/recovery/resources/power/factory.bat_cal`. The tooling that produces this table is in `scripts/battery_calibration/`.

Low power modes are not implemented. The FreeRTOS tickless idle hook executes `WFI` only.

## Watchdog and crash handling

No IWDG or WWDG is configured. On `furi_crash()` the crash handler (`targets/f21/src/crash_handler.c`) reboots the device unless a debugger is attached, or, in debug builds, the NVM debug flag is set.

## OTP provisioning

The 512 byte OTP area is split into four 128 byte blocks, each starting with the magic `0x3713`, a block index and a version:

| Block | Content |
| --- | --- |
| OTP1 | Production timestamp, USB MAC, model string (for example `BB.1`), hardware version, target (20, 21, 22), body and connector codes |
| OTP2 | QC timestamp, color, region |
| OTP3 | Curve id and the 56 byte secp224r1 public key |
| OTP4 | MCU UID and two ECDSA signatures that bind OTP1 and OTP2 to this MCU |

`scripts/bsbotp.py` builds and verifies the blobs (`genkey`, `create-otp1..4`, `load`, `verify`, `dump-all`). The write happens on the device with the CLI command `otp program`, which is available only when the NVM debug flag is set and refuses to overwrite a non-blank block. `furi_hal_version.c` parses the blocks and derives the hardware version string (`4.F22.B7.C2` style), the USB MAC (fallback prefix `0C:FA:22` plus UID), and the FCC and IC identifiers.

# SiWG917 wireless co-processor

## Core and clocks

- SiWG917M111MGTBA. The Cortex-M4F application core (M4) runs this firmware. The network processor (NWP) core runs the Silicon Labs connectivity firmware, a vendor binary shipped in `lib/wiseconnect_extra/connectivity_firmware/`.
- 40 MHz crystal, M4 PLL at 180 MHz, interface PLL at 160 MHz, QSPI flash clock 80 MHz.
- The M4 gets 320 KiB of RAM (`SL_SI91X_SI917_RAM_MEM_CONFIG=3`).

## Memory map

| Region | Origin | Size | Use |
| --- | --- | --- | --- |
| Flash | `0x08202000` | 2040 KiB | M4 application image in the shared 8 MiB flash |
| RAM | `0x0000000C` | 320 KiB less 1 KiB | Data, BSS, heap, 512 byte stack |
| uDMA descriptors | Top 1 KiB of RAM | 1 KiB | |
| NVM3 | End of the M4 region, before the last 4 KiB | 40 KiB | Key-value store used by Matter and the BLE bonding data |
| NWP MBR | `0x081F0000` | 496 bytes | Secure boot, anti-rollback and encryption flags of the TA and M4 images |
| Crypto key storage | NWP managed common flash | 16 KiB main partition + 4 KiB user partition | Certificates, private keys, Matter credentials |

Code that runs while flash is reprogrammed (`critical.o`, `furi_hal_interrupt.o`, `furi_hal_qspi_flash.o`, the WiseConnect RAM functions) is forced into RAM.

## Peripherals

| Function | Pins |
| --- | --- |
| Intercom (USART0 to U5 USART1) | TX GPIO54, RX GPIO55, RTS GPIO53, CTS GPIO56 |
| ULP-UART (log console at 230400 baud, ROM bootloader UART) | RX GPIO8, TX GPIO9 |
| UART1 (CLI in the `hwtest` configuration) | ULP GPIO8 RX, ULP GPIO11 TX |
| Status RGB LED (MCPWM, 3 kHz) | Red GPIO7, green GPIO11, blue GPIO15 |
| Rotary encoder (QEI) | ULP GPIO9 A, ULP GPIO10 B |
| Buttons | UULP0 Off, UULP1 OK, UULP2 Start/Pause, UULP3 Back |
| Mode switch contacts | GPIO50 Busy, GPIO51 Settings, ULP GPIO6 Apps, ULP GPIO7 Status (Custom), UULP0 Off |
| Interrupt to U5 | ULP GPIO1 |
| U5 SWD pass-through | SWCLK GPIO10, SWDIO GPIO47 |

Early init pulls up unused pins. The Si917 crash handler never reboots on its own.

## Wireless and security

- 2.4 GHz Wi-Fi client with BLE coexistence (`targets/f64/config/wifi_config.c`). The firmware bypasses the NWP TCP/IP offload, so raw Ethernet frames reach the M4.
- Hardware crypto through the NWP: AES 128/192/256, ECDSA sign and verify, HMAC, SHA, TRNG and key wrapping. Wrapped private keys can sign, but nothing can read them out. Key wrapping requires secure boot. The `SIL917_INSECURE` build variable (default true) disables wrapping during provisioning for lab devices.
- The M4 `HWRNG` block feeds `furi_hal_random`.

# Boot sequence

## STM32U5

1. `Reset_Handler` calls `SystemInit()` (FPU on, RCC reset to MSI 4 MHz, vector table at flash or SRAM for the RAM image), initializes data and BSS, then calls `main()`.
2. `main()` (`targets/f21/src/main.c`) runs `furi_hal_init_early()`, which validates or resets the backup SRAM NVM block, enables debug features when the NVM debug flag is set, and parses the OTP blocks.
3. `main()` checks the boot mode from NVM. In `Normal` mode the Init thread starts the scheduler, runs `furi_hal_init()` and starts every service (see @ref services).
4. In `Update` mode (set by the updater before a reboot) the mode is cleared and `boot_update.c` runs before the scheduler: it mounts both eMMC partitions with FatFs, reads the manifest path from `/ext/.sys_update.txt`, loads the `updater_stage` binary named in `update.json`, verifies its CRC32, copies it to SRAM1 and jumps to it. Any failure falls back to a power reset.
5. The updater stage is the same firmware linked for RAM execution with the `update_executor` application set. It flashes the U5 DfuSe image, then the Si917 M4 and NWP images, then unpacks the resources. See @ref updater.
6. The ST ROM DFU bootloader at `0x0BF90000` is reachable only through the power service (`power boot u5` on the CLI). There is no button combination for it.

The flash driver clears the `PA15_PUPEN` option bit if it is set, because PA15 is the UCPD CC1 line. No read-out or write protection is configured in code.

## SiWG917

The Silicon Labs ROM bootloader and MBR run first. `main()` (`targets/f64/src/main.c`) runs `furi_hal_init_early()`, then the Init thread runs `furi_hal_init()` and starts the services. There is no NVM boot mode logic on this side.

Holding the SWO line low during reset enters the ROM bootloader. The U5 does this with `furi_hal_power_reset_917(true)`. A host can do the same by grounding `JTAG_TDO_SWO` before flashing with `scripts/flashrps.py`. When the DFU button is held at boot, the `startup_dfu_hook` service blinks the status LED orange for 3 s, then turns it solid red and waits for the intercom so the U5 can take over.

# Si917 firmware image (RPS)

- The build turns the M4 ELF into `.bin` and then `.rps` with `scripts/bin2rps.py`. The RPS header carries the magic `0x900D900D`, the image size, the flash address, a boot descriptor and a CRC-32 (polynomial `0xD95EAAE5`) at offset 0x14. This image has no signature and no encryption.
- `scripts/sign_silabs_rps.py` signs an image either locally with Simplicity `commander-cli` and a keystore, or through a signing service (`SI917_SIGN_SERVICE_URL`, `SI917_SIGN_SERVICE_TOKEN`, `SI917_SIGN_SERVICE_PROFILE`). Secure-boot devices accept only signed images.
- From a host, `scripts/flashrps.py` (or `./fbt flash` on `f64`/`f65`) drives the ROM bootloader text menu over the ULP-UART at 115200 baud, switches to 921600 baud, selects the M4 or NWP image slot, and streams the file with the Kermit protocol.
- From the U5, the updater does the same over USART2 with an embedded Kermit implementation, trying baud rates from 921600 down to 9600.

# FreeRTOS configuration

| Setting | U5 | Si917 |
| --- | --- | --- |
| Tick | 1000 Hz | 1000 Hz |
| Priorities | 32 | 32 |
| Heap | heap_4, size from linker | heap_4, size from linker |
| Tickless idle | Custom hook, `WFI` only | Off |
| Stack overflow check | Off, MPU guard instead | Off |
| Task return address | `furi_thread_catch` | `furi_thread_catch` |
| `configASSERT` | `furi_crash("FreeRTOS Assert")` in debug builds | Same |

Both MCUs use 4 priority bits with the syscall ceiling at 5. Furi interrupt priorities map to NVIC 7..13 (Normal is 10). The `KamiSama` level maps to NVIC 4, above the syscall ceiling, and serves interrupts before the scheduler starts. EXTI lines run at 10 on the U5. Pin interrupts run at 5 on the Si917.

# HAL modules

The HAL is the `furi_hal_*` family in `targets/<target>/furi_hal/`.

| Module | U5 | Si917 |
| --- | --- | --- |
| `furi_hal` | Early and main init sequences | Early and main init sequences |
| `furi_hal_bus` | RCC clock gating per peripheral | M4 clock gates |
| `furi_hal_clock` | PLL setup, kernel clock queries | PLL setup |
| `furi_hal_cortex` | Caches, DWT, system reset, jumps to DFU, SRAM or flash | DWT |
| `furi_hal_mpu` | Null guard, stack guard, backup SRAM attributes | |
| `furi_hal_interrupt` | ISR registration by id, priorities, ISR time accounting | Timers and QEI |
| `furi_hal_gpio`, `furi_hal_resources` | Pin tables, EXTI | HP, ULP and UULP pins |
| `furi_hal_dma` | GPDMA and LPDMA channel allocation | uDMA channels |
| `furi_hal_flash` | Page program and erase, option bytes, OTP | Base and free end only |
| `furi_hal_nvm` | Backup SRAM block | Stub |
| `furi_hal_rtc`, `furi_hal_random` | RTC, RNG | RNG |
| `furi_hal_version`, `furi_hal_info` | OTP parsing, UID, MACs, device info | NWP firmware version, MACs, MBR security bits, enclave check |
| `furi_hal_serial`, `furi_hal_serial_control` | USART driver with DMA and autobaud, handle arbitration | Same |
| `furi_hal_spi`, `furi_hal_i2c` | Bus and handle model | |
| `furi_hal_sdmmc` | eMMC and SD driver | |
| `furi_hal_sai`, `furi_hal_dac` | Audio output paths | |
| `furi_hal_light_sensor`, `furi_hal_display` | BH1730, front display power pin | |
| `furi_hal_usb` | OTG HS bring-up | |
| `furi_hal_power` | Reset, Si917 reset and DFU line control | Reset |
| `furi_hal_crypto`, `furi_hal_crypto_storage` | | NWP crypto services, key slot storage |
| `furi_hal_pwm`, `furi_hal_qei` | | RGB LED, rotary encoder |

# Drivers

| Driver (`lib/drivers/`) | Chip | Bus |
| --- | --- | --- |
| `bh1730` | ROHM BH1730 ambient light sensor | I2C1 |
| `bq25798` | TI BQ25798 charger with ADC | I2C1 |
| `ns4168` | NS4168 class-D amplifier | One GPIO, pulse count selects the high-pass filter |
| `ssd1320` | SSD1320 OLED controller | SPI1 |

# Debug tooling

`scripts/debug/platforms/` holds OpenOCD and SVD descriptors: `stm32u595.json` (flash at `0x08000000`, `stm32u5x.cfg`, `STM32U595.svd`) and `siw917.json` (flash at `0x08202000`, `siw917.cfg`, `SiWx917.svd`). The GDB extensions in `scripts/debug/platforms/stm32u5/` read the running firmware version through the backup SRAM NVM block. The U5 SWD pins are only reachable with the debug board attached (see @ref quick-start).
