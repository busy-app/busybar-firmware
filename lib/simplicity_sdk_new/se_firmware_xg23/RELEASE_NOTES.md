# Release Notes - XG23 SE Firmware

## v2.2.7

- Reseed AES DPA countermeasures upon exiting EM2/3.

## v2.2.6

- Fixed a bug where TAMPERRSTCAUSE did not get cleared properly in certain scenarios.
- Fixed a bug where trying to export a public key using a small custom ECC curve would cause the SE to
trigger a reset.

## v2.2.5

- Accept host upgrade attempts where the size of the upgrade binary includes the CRC32 checksum of
Gecko Bootloader. This is a mitigation for a defect in Gecko Bootloader present in GSDK versions
3.2.0 through 3.2.9 and 4.0.0 through 4.0.2.

## v2.2.4

- Improved current consumption of Active Mode. Making active mode only exitable through the interface
with which it was entered.
- Improved SE wakeup time from EM2/3.
- Time to execute a device erase has been reduced by ~50 percent.
- Fixed a bug where flashing a new application was not possible after a device erase, unless the device
was reset beforehand.
- Fixed a bug where the setting of "Enable M33 Lockup reset" bit in EMU led to timeout when issuing a
device erase command.
- Reduced secure boot time by 0.2-0.4 ms when considering a 0x6000 byte host application.

## v2.2.2

- Improved boot time by about 1 ms.
- Reduced duration of Secure Boot validation of host application by around 1 ms depending on applica-
tion size.
- Fixed an issue where data in internal SE flash was encrypted using an uninitialized IV.
- Fixed an issue where the get_tamper_reset_cause command would return an incorrect value when the tam-
per reset threshold in OTP was configured to 0.
- Fixed an issue where certain debug restrictions could lead to a boot error on soft resets.

## v2.2.1

- Prevent TrustZone specific debug lock configuration from being changed after debug access port lock
has been applied.
- Changed the TrustZone Root Key automatic renewal. The key is now only renewed on Device Erase.
- Added get_tamper_reset_cause command to output which tamper source caused a tamper reset. The command
is available over mailbox.

## v2.2.0

- Added support for TrustZone Root Key which can be used by TrustZone secure applications for secure
storage. The key is renewed on OTP configuration, debug lock and device erase.
- Fixed a bug where TrustZone specific debug locks where not retained across soft resets after being
opened through Secure Debug unlock.
- Fixed an issue where executing a host upgrade with a large image could cause a watchdog timeout.
- Security and stability updates.

## v2.1.7

- Fixed downgrade attack vulnerability
- Increased internal watchdog timeout to prevent long operations from being interrupted
- Make the Device Erase command break the boot loop that could occur after a failed host upgrade
- X25519 and Ed25519 algorithms, along with related key management functionality, is now supported on
Secure Vault Mid devices
- Randomize the operating frequency between every command also when active mode is enabled
- GCM support for input lengths larger than 0.5 GB

## v2.1.5

- Fixed an issue where Tamper module was not responding in EM1P. This issue could lead to a device re-
set upon exiting EM1P.
- Startup time improvements. For typical device configurations, this change could save 5-6 ms of boot
time. With Secure Boot enabled and debug ports locked, the estimated boot time savings can be as much
as 10 ms. Prior to this change, startup time could increase over time based on the history of the de-
vice. For example, performing a successful host upgrade or device erase command on a debug locked
part could permanently increase boot time by a few milliseconds. In a worst-case scenario, these ef-
fects could add up to a total of 300 ms additional boot time.

## v2.1.4

- Fixed a bug where using volatile keys together with a custom ECC curve did not work
- Added explicit validation of input and output length for commands that are sent without any in-
put/output buffers from the mailbox interface. This causes a change in behavior of the SE Manager
function `sl_se_roll_challenge`, which has been fixed in Gecko SDK v3.2.2. When applying the SE firm-
ware upgrade to a device with an application compiled with an older version of the SDK, be aware that
`sl_se_roll_challenge` will return an error code after the SE firmware upgrade, until the application
is recompiled with the updated SDK.
- Improved fault injection hardening

## v2.1.3

- Fixed bug where using custom domains failed for some commands

## v2.1.2

- Added support for new glitch detectors through the tamper response bits in SM InitOTP
- Added PBKDF2-CMAC support (option 0x10) to DeriveKey command
- Added custom curve support in ECDSA and ECDH
- Improved fault injection resistance
- Security and stability updates

## v2.1.1

- Production support update (Silicon Labs internal)

## v2.1.0

- Secure Vault High features (first release for EFR32xG23 products)
