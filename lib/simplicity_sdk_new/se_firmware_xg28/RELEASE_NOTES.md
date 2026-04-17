# Release Notes - XG28 SE Firmware

## v2.2.7

- Reseed AES DPA countermeasures upon exiting EM2/3.

## v2.2.6

- Fixed a bug where TAMPERRSTCAUSE did not get cleared properly in certain scenarios.
- Fixed a bug where trying to export a public key using a small custom ECC curve would cause the SE to
trigger a reset.

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

- Initial release for xG28 products

