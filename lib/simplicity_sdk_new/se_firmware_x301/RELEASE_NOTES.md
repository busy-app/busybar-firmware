# Release Notes - X301 SE Firmware

## v3.3.7

- Stability fix for flash accesses during flash erase/write operations. Flash read accesses (instruction or data fetches) from the host CPU at the exact same time as a flash write or erase operation was started by the SE could lead to bus faults, corrupted data or hang situations leading to lack of debug connection depending on flash encryption mode (AXiP, EXiP, None). This release fixes the issue by ensuring there are no ongoing read operations when flash write/erase operations are started. Note that the fix has the side-effect that the host bus will be frozen for 1-2 microseconds while the SE ensures that no transactions are in progress.

## v3.3.6

- Bug fix in the DeleteKey command for KSU keys. The command erroneously prevented the KSU from being used after DeleteKey was called. This issue is now fixed.
- Flash encryption stabilization. This fixes issues with flash encryption in either EXiP or AXiP modes. In either of the modes, data could be incorrectly encrypted before written to the external flash. This was a rare issue, only occuring once in a few thousand code region write attempts.
- Users are advised to upgrade their SE FW to v3.3.5 before performing application or bootloader upgrades. Bundling an SE FW upgrade into the same GBL as other upgrade files is also okay, as the SE FW upgrade is always applied first by the bootloader.

## v3.3.2

- Updates to the PSA Initial Attestation Token creation and Secure Debug unlock.
  - The lifecycle claim is updated to contain information on whether or not the SE CPU has been unlocked during manufacturing.
  - A Secure Debug unlock of the SE CPU after the device has experienced any customer init commands is now tracked permanently in OTP.
  - The device will erase its attestation keys and will never generate new attestation tokens.
- An EnterEOL command has been added. This command allows the user to trigger a full erasure of all SE protected secrets and optionally log a trace code in OTP. The device will not be operational after entering EOL.
- A command to wipe the host AES key used by the SiSDK Bootloader is added. This command allows users to erase their secrets from OTP without rendering the device non-functional.
- Upgrade to ROM patch 5.3
- Improved the polling operation for the external flash to optimize flash erase speed and response time for flash pause and resume commands.
- Improved handling of new commands when flash operations are in progress.
- Allow transfers to KSU if the DPA_REQUIRED flag is set in the key metadata, as all KSU consumers have DPA protections.

## v3.3.1

- Enables DPA countermeasures for EdDSA and Montgomery point multiplication.
  - With DPA CM enabled, only points on the curve are supported as public keys for ECDH (Curve25519). Attempting other values as public keys will result in unusable output. Valid ECDH public keys will always be points on the curve.

## v3.3.0

- Added non-blocking command options for flash write and erase commands. The non-blocking options make the command return immediately. To check the status of the flash operation, the user must check the GetFlashStatus command.
  - Commands that erase a code region require that the flash status is checked by the user before responding to any other commands.
  - While flash is busy, all SE commands not related to flash handling are unavailable.
- Added new commands related to flash handling: GetFlashStatus, PauseFlash and ResumeFlash.
- Updated key metadata format for KSU, including supporting crypto engine ID for transfers and imports to KSU.
- Fixed a bug in the HMAC multipart update command that reported BUS_ERROR

## v3.2.0

- Added support for HMAC-AES-MMO.
- GetOTPRollbackCounter command can now read out SE rollback counter.
- Added new return status: EOL(0xE). This status is returned when the SE has run out of OTP space to update its rollback counter.
- Fixed an issue where the SE could get into a bad state if Debug Lock was applied after both Device Erase and Secure Debug Unlock was disabled.
- Security updates.

## v3.1.1

- Improved bootup time for devices with a full MTP storage area.
- Added command for partial erase of host region.
- Added command for disabling code region AXiP IV roll.
- Support transferring NVM3 key to KSURAM.
- Fix an issue where the device would reset about 240 ms after waking up from EM4.
- Improved QSPI calibration logic.
- Updates ROM patch to 5.2.

## v3.1.0

- Support for Secure Boot, on-par with Series 2.
  - Note that the “narrow” and “wide” page lock options no longer make sense for Series 3 and are removed.
  - In order to validate the Secure Boot signature, SE FW requires that region 0 is locked. This can be achieved through “commander security closeregion 0” command in Simplicity Commander. Only close a region after flashing a validly signed application to region 0.
- EraseHostFlash command is now available through mailbox.
- ReadSecureTraceFlags command is now available through mailbox.
- Get/IncrementHostRollbackCounter command is now available through mailbox.
- KSU support.
- Fixed a bug that caused parts with Giantec flash to crash when ConfigureQSPIRefClk command was called.
- Unlocking a host code region is no longer possible to do through the ApplyCodeRegionConfig command. To unlock a region one must either erase the region over mailbox or trigger a device erase command.
- BUSLOCK is no longer applied when changing QSPI clock configuration.
- Re-calibration of QSPI delay line on delay tap drift.

## v3.0.1

- Fixed a bug where DeviceErase command could brick a device.

## v3.0.0

- B0 support.
- Fixed issue when attempting to lock a bank swapped host code region.
- Lengthened watchdog timers during long flash operations.
- ConfigureQSPIRefClock command will now re-apply previous config if FLPLL programming fails. If previous config
  also fails, device is reset.
- Fixed a bug in the CloseCodeRegion command where it did not allow a region version to be set properly.
- Improved erase time for large flash erase operations.

## v0.3.1

- Added host upgrade functionality.
- Added multi-part HMAC support.
- Fixed bug where one could not write to code region 2 using sl_se_code_region_write() API.

## v0.3.0

- Added SE command for retrieving QSPI FLPLL configurations.
- Added command for configuring QSPI drive strength.
- Added command for setting and getting host upgrade file version.
- Get data region location updated to return correct length based on AXiP config.
- Fixed bug where erase data page command accepts writes outside available flash space.
- This version includes a fix for a upgrade issue that is present on earlier versions of SE FW. Some devices with earlier versions of SE FW will not be able to upgrade.
- Added powering down of OTP whenever it is not in use to reduce EM0/1 current.
- Various other bug fixes.

## v0.2.2

- Increase flash clock speed from 20 to 100MHz out of reset
  - SE uses HFRCO as reference clock for the FLPLL at boot. HFRCO registers are not locked by the SE.
- The command for configuring the QSPI reference clock now supports:
  - Switching back to FSRCO and disabling the FLPLL.
  - Setting more configurations when enabling the FLPLL, to support fine tuning of the QSPI clock.
- Added SE Device Erase command.
- Fix bug causing SE hangs when enabling AXiP for host code regions.

## v0.2.1

- Revert to 20MHz flash clock speed.

## v0.2.0

- Fix a bug where the SE would quickly outspend its OTP rollback counter, leaving the device in a non-functional state.
- Implement a workaround for a bug in the L2CACHE.
  - The bug could lead to the SE or other CPUs being sent incorrect data from the L2CACHE.
  - SE FW needs to flush the entire L2CACHE just after receiving a command. This is expected to have performance implications for other code running from flash.
  - This is required for stability of operation.
- Increase flash clock speed from 20 to 100 MHz.
  - SE uses HFRCO as reference clock for the FLPLL at boot. HFRCO registers are locked by the SE out of reset.
  - Host code must send an SE command to switch to HFXO as reference clock. After that command HFXO registers are locked while HFRCO registers are available.
- Added a command to switch FLPLL reference clock from HFRCO to HFXO.
- Enabled AES commands, sign/verify commands and JPake commands.
- Support for AXiP of host code regions. This is not enabled by default.
- Added the SE Upgrade status command.
