# Release Notes - XG21 SE Firmware

## v1.2.17

- Reseed AES DPA countermeasures upon exiting EM2/3.

## v1.2.16

- Improved stability for xG21-C and xG21-D when operating close to –40°C.
- Adjusted the upper temperature limit for the temperature tamper signal to reflect the maximum junc-
tion temperature (135°C) instead of the maximum ambient temperature (125°C).

## v1.2.15

- Reduced EM2 current consumption for the xG21-C and xG21-D family of devices.

## v1.2.14

- Prevent TrustZone specific debug lock configuration from being changed after debug access port lock
has been applied.
- Changed the TrustZone Root Key automatic renewal. The key is now only renewed on Device Erase.
- Fixed a bug where using custom domain curves larger than 40 bytes would lead to a reset.

## v1.2.13

- Security and stability fixes

## v1.2.12

- Added support for TrustZone Root Key which can be used by TrustZone secure applications for secure
storage. The key is renewed on OTP configuration, debug lock and device erase.

## v1.2.11

- Fixed downgrade attack vulnerability
- Increased internal watchdog timeout to prevent long operations from being interrupted
- Make the Device Erase command break the boot loop that could occur after a failed host upgrade
- Fixed an issue with the recovery logic following an interrupted host upgrade. The issue could quickly
lead to the device outspending its allowed attempts, leaving it in an unusable state.
- X25519 and Ed25519 algorithms, along with related key management functionality, is now supported on
Secure Vault Mid devices
- Added explicit validation of input and output length for commands that are sent without any in-
put/output buffers from the mailbox interface. This causes a change in behavior of the SE Manager
function `sl_se_roll_challenge`, which has been fixed in Gecko SDK v3.2.2. When applying the SE firm-
ware upgrade to a device with an application compiled with an older version of the SDK, be aware that
`sl_se_roll_challenge` will return an error code after the SE firmware upgrade, until the application
is recompiled with the updated SDK.
- GCM support for input lengths larger than 0.5 GB.

## v1.2.9

- Fixed an issue where the SE would fail to generate the correct public key for a given Ed25519 private
key, affecting the 'export public key',
'generate signature', and 'verify signature' operations when
called with a private key buffer. This bug exists in SE firmware versions between 1.2.2 and 1.2.8
(inclusive).
- Removed SE Manager function sl_se_upgrade_status_clear() and the corresponding functionality from the
SE firmware. This function behaves erratically on older versions and was not in use by Gecko SDK com-
ponents.

## v1.2.8

- Fixed potential issue with the strength of crypto countermeasures under certain conditions
- Fixed a bug where the SE would fault on generating keys of non-word-aligned size
- Security and stability fixes

## v1.2.6

- Critical stability fix to prevent devices becoming inoperable under certain conditions. A device
whose SE firmware has been successfully upgraded at least once can become inoperable if the upgrade
file is removed from flash after the upgrade and the device is subjected to a large cumulative number
of resets. The exact number of reset cycles before failure varies due to process variations and oper-
ating temperature but would typically range between 50,000 and 200,000 cycles. It is imperative that
this fix be applied to every device to avoid latent failures.
- Increase DCI output buffer size to be able to fit attestation tokens for all device configurations
- Fixes an issue where tamper levels are not set to default level after issuing the disable tamper com-
mand

## v1.2.4

- Allow EFR32xG21B devices to sign external content using the MCU identity key
- Fixed stability issue with the debug restriction command

## v1.2.3

- Fixed issue where a public key could not be derived from a P-521 private key
- Maximum size for a 'RAW' key is now 512 bytes (previously 612)
- Fixed an issue preventing HMAC with null input
- Improved error handling in AES encryption and decryption operations
- Added certificate read support to mailbox interface
- The ReadRSTCAUSE command will now always return the reset cause observed at the previous boot
- When tamper reset threshold is reached, the device is now held in debug mode
- Security and stability fixes

## v1.2.2

- Added an option to manually upgrade the (V)SE firmware when the debug interface is locked and Secure
Boot is enabled, as long as the device erase function has not been turned off.
- The error code returned over DCI on failure to validate the firmware during Secure Boot is now more
granular as to the reason for failing Secure Boot.
- Fixed potential fault in the JPakeGenSessionKey command related to invalid password length
- Improved robustness of wrapped and volatile key handling
- Fixed potential starvation issue on the SE command interfaces
- Support for EFR32xG21 with die revision A0 is now removed. These were early engineering samples, not
intended for production use. To check whether your device is revision A0, connect to the device using
Simplicity Commander.
- Added support for Montgomery-type keys for use in ECDH on Vault parts
- Security and stability fixes

## v1.2.1

- Fixed issue that could cause the device to become unrecoverable with both Secure Boot with RTSL and
Rollback Prevention enabled

## v1.2.0

- Improved handling of fatal errors during chip boot-up sequence
- Improved handling of run-time errors in the secure subsystem
- Volatile keys can now be used for multiple operations
- Access restrictions can now be set on the debug interface. The command DBG_LOCK_SET_RESTRICTION is
added and status of individual restrictions is added to the status commands.
- Improved stability for REGDIS enable/disable command
- Improved EM2 entry/exit delay
- Added DCI command SE_COMMAND_READ_OTP to read non-reconfigurable user settings from the SE for Secure
Boot with RTSL and Tamper Response
- Other security updates

## v1.1.8

- Improved root key entropy

## v1.1.7

- Security update

## v1.1.6

- Stability fixes

## v1.1.5

- Stability fixes

## 1.1.4

- Unauthenticated device recover will now also set all RAM words to 0

## v1.1.3

- Improved DCI error reporting on secure boot failure
- Increased reliability during boot-up sequence
- Added support for all allowed GCM tag lengths during GCM decrypt-and-verify
- Added functionality to keep SE awake on demand
- Added command to disable internal LDO (EFP support)
- Commands with reserved bits (i.e. unused bits from parameters/options) will now check these bits are
set to zero
- Added workaround for a Cortex-M33 errata when exercising TrustZone debug lock bits

## v1.1.2

- Added rollback prevention based on host application version number. The feature is enabled by the
anti-rollback flag is in OTP settings

## v1.1.1

- Fixed issue with having both Secure Boot and Secure Debug key installed, leading to problems with Se-
cure Debug
- Fixed issue preventing Secure Unlock to function properly on Secure Boot failure
- If the SE has successfully booted an image that was signed with a certificate with version > 0, di-
rect signing of a Secure Boot binary is no longer permitted by the SE. The
SECURE_BOOT_VERIFY_CERTIFICATE flag in OTP can be used to require the use of certificates and disa-
bles the option to use directly signed binaries.

## v1.1.0

- Added Secure Boot with RTSL (Root of Trust and Secure Loader) functionality
- Added extra protection against key leakage
- Added option to lock flash pages that have been validated by Secure Boot
- Other security fixes

## v1.0.2

- Fixed an issue were subsequent DCI commands could share input parameters
- J-PAKE commands returns more granular error codes
- Fixed an issue where a J-PAKE input buffer could be overflown
- Fixed an issue where a device erase command could crash the device
- NVM sub-system longevity improvement

## v1.0.1

- Security update fixing TRNG issue

## v1.0.0

- GA release

## v0.1.7

- Security update

## v0.1.6

- DCI 'Get Status' command now returns the actual status of the debug locks in addition to the debug
lock configuration

## v0.1.5

- An implementation error in the ECDH acceleration code in mbedTLS (used in e.g. BLE) will start to
produce an error in SE firmware v0.1.5. The update to ECDH in mbedTLS included in Gecko SDK Suite
v2.5.2 resolves the issue. Users of BLE or ECDH separately needs to update to Gecko SDK Suite v2.5.2
before upgrading to an SE firmware version beyond v0.1.4.
- Fix potential data corruption when installing keys through host mailbox interface
- Increased timeout for DCI operations
- Security and stability updates

## v0.1.4

- Improved responsiveness in EM2

## v0.1.3

- Improved power consumption while sleeping in EM1
- Security and stability updates

## v0.1.2

- Improved stability of device erase command

## v0.1.1

- Energy consumption in EM2 improved for typical temperature range
- Improved various status codes for debug commands
- When using secure debug unlock, the debug port maintains the unlocked state across soft reset

## v0.1.0

- Stability update (first release for EFR32xG21 products)

