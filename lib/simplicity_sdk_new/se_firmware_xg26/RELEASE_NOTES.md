# Release Notes - XG26 SE Firmware

## v2.2.7

- Reseed AES DPA countermeasures upon exiting EM2/3.

## v2.2.6

- Fixed a bug where TAMPERRSTCAUSE did not get cleared properly in certain scenarios.
- Fixed a bug where trying to export a public key using a small custom ECC curve would cause the SE to
trigger a reset.

## v2.2.4

- Initial release for EFR32xG26 products.
