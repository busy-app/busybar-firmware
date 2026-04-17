# Release Notes - XG22 SE Firmware

## v1.2.14

- Prevent TrustZone specific debug lock configuration from being changed after debug access port lock
has been applied.
- Changed the TrustZone Root Key automatic renewal. The key is now only renewed on Device Erase.
- Patched vulnerability in security configuration of EFM32PG22 SoCs with date code earlier than 2239.
- Added OTP version to the status output field of the VSE mailbox.

## v1.2.12

- Added support for TrustZone Root Key which can be used by TrustZone secure applications for secure
storage. The key is renewed on OTP configuration, debug lock and device erase.

## v1.2.11

- Fixed downgrade attack vulnerability
- Make the Device Erase command break the boot loop that could occur after a failed host upgrade

## v1.2.7

- Security and stability fixes

## v1.2.6

- Critical stability fix to prevent devices becoming inoperable under certain conditions. A device
whose VSE firmware has been successfully upgraded at least once can become inoperable if the upgrade
file is removed from flash after the upgrade and the device is subjected to a large cumulative number
of resets. The exact number of reset cycles before failure varies due to process variations and oper-
ating temperature but would typically range between 50,000 and 200,000 cycles. It is imperative that
this fix be applied to every device to avoid latent failures.
- Increase DCI output buffer size to be able to fit attestation tokens for all device configurations.
- Hardened the boot up process against partial RAM retention. If a mailbox command is issued to the VSE
and followed by a momentary power failure whose duration is sufficient to cause only a few specific
RAM bits to lose state, the POR boot process can terminate in a hard fault.

## v1.2.5

- Added VSE mailbox support to enable debug lock. The debug lock status is stored in the status word
upon each boot and can be read using the EMLIB API SE_getConfigStatusBits(). The SE Manager functions
sl_se_apply_debug_lock() and sl_se_get_debug_lock_status() are also available for EFR32xG22 products
from Gecko SDK version 3.0.1.

## v1.2.2

- The output mailbox struct is populated with user OTP settings after reset.
- Added an option to manually upgrade the (V)SE firmware when the debug interface is locked and Secure
Boot is enabled, as long as the device erase function has not been turned off.
- The error code returned over DCI on failure to validate the firmware during Secure Boot is now more
granular as to the reason for failing Secure Boot.
- Security updates

## v1.2.1

- Fixed issue that could cause the device to become unrecoverable with both Secure Boot with RTSL and
Rollback Prevention enabled

## v1.2.0

- Improved handling of fatal errors during chip boot-up sequence
- Access restrictions can now be set on the debug interface. The command DBG_LOCK_SET_RESTRICTION is
added and status of individual restrictions is added to the status commands.

## v1.1.8

- None for this product

## v1.1.7

- Security update

## v1.1.6

- Added DCDC configuration retention across soft reset

## v1.1.5

- Stability fixes

## v1.1.4

- Unauthenticated device recover will now also set all RAM words to 0

## v1.1.3

- Commands with reserved bits (i.e. unused bits from parameters/options) will now check these bits are
set to zero
- Fixed issue that prevented use of the UserData page
- Added workaround for a Cortex-M33 errata when exercising TrustZone debug lock bits
- Aligned DCI success status codes with EFR32xG21 products
- Resolved debug lock issue

## v1.1.1
(first release for EFR32xG22 products)

- Fixed an issue where a device with Secure Boot turned on would not be able to be secure unlocked on
secure boot failure
- Fixed issue preventing Secure Unlock to function properly on Secure Boot failure
