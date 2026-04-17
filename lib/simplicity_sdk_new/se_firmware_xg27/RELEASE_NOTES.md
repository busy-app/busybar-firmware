# Release Notes - XG27 SE Firmware

## v2.2.6

- Fixed a bug where TAMPERRSTCAUSE did not get cleared properly in certain scenarios.
- Fixed a bug where trying to export a public key using a small custom ECC curve would cause the SE to
trigger a reset.
- Fixed a bug that made it impossible to read the public keys stored in the SE.

## v2.2.4

- Fixed a bug where flashing a new application was not possible after a device erase, unless the device
was reset beforehand.

## v2.2.2

- Fixed an issue where data in internal VSE flash was encrypted using an uninitialized IV.
- Fixed an issue where certain debug restrictions could lead to a boot error on soft resets.
- Make the provisioned Secure Boot public key available to User code at runtime.

## v2.2.1

- Prevent TrustZone specific debug lock configuration from being changed after debug access port lock
has been applied.
- Changed the TrustZone Root Key automatic renewal. The key is now only renewed on Device Erase.
- Added OTP version to the status output field of the VSE mailbox.

## v2.2.0

- Initial release for EFR32xG27 products.

