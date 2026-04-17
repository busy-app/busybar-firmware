# Firmware Update Images and Bootloader

## General Information
This guide was originally written for Simplicity Studio V5.9.3.1 with SiSDK 2024.12.0. It has been updated for SiSDK 2025.12.0, which is distributed with Simplicity Studio v6. While file paths may differ between versions, the general procedures remain consistent.

**Note:** Detailed information regarding bootloaders for the 800 series is available [here](https://www.silabs.com/documents/public/user-guides/ug103-06-fundamentals-bootloading.pdf)
and [here](https://www.silabs.com/documents/public/user-guides/ug489-gecko-bootloader-user-guide-gsdk-4.pdf)

Before going further into this guide, it is important to know a few files and locations that are crucial in following the subsequent steps.

### Default Locations

- **Default Simplicity Studio Installation Location**: `C:\SiliconLabs\SimplicityStudio\v5`
- **Default SDK Installation Location**: `C:\Users\{Username}\SimplicityStudio\SDKs`
- **Default Workspace Location**: `C:\Users\{Username}\SimplicityStudio\v5_workspace`
- **Sample encryption keys**: - `sample_sign.key`, `sample_sign.key.pub`, `sample_sign.key-tokens.txt`, `sample_encrypt.key`
- **Sample keys location**: `{SDK Installation Location}\gecko_sdk\protocol\z-wave\platform\SiliconLabs\PAL\BootLoader\sample-keys`
- **Commander utility location**: `{Simplicity Studio Installation Location}\developer\adapter_packs\commander`
- **Sample project location**: `{Default Workspace Location}\zwave_soc_switch_on_off\`

>**Note:** The above paths are customizable by the user and as such must be adapted if they are different from the default paths given above.

## General information

The purpose of this section is to describe how to generate and manage firmware update images. The SDK provides two bootloaders for a given board type - OTA and OTW. The OTA bootloader is needed for all Z-Wave based devices that implement firmware updates; the OTW bootloader is for devices that update firmware using the serial port from another host controller. The OTA bootloader is triggered when an image has been transferred over the air using the FIRMWARE_UPDATE command class. The transferred image must be an image in Gecko Boootloader (GBL) format. The bootloaders provided in the SDK require the GBL image to be signed.

Three steps are needed for performing an OTA update:

1. The OTA bootloader must be flashed.
2. The signing keys and optionally an encryption key must be flashed.
3. A signed image must be transferred using the firmware update command class.

Further information about bootloaders can be found [here](https://www.silabs.com/documents/public/user-guides/ug266-gecko-bootloader-user-guide.pdf).

## Generate GBL files (manual steps)

To generate the GBL files needed for the OTA/OTW update, a signing keypair must first be created. It is the intention that a vendor will keep the signing keypair for the lifetime of the product. These keys are used to sign all the firmware versions for the whole lifetime of the product. An encryption key must also be created, this key is intended for encrypting the GBL file. Encryption makes it harder for a bootlegger to copy the product.

The signing keys can be created using the `Simplicity Commander`'s command line interface:

```bat
commander.exe gbl keygen --type ecc-p256 -o vendor_sign.key
```
This step will create 3 files:

**vendor_sign.key** - This is the private key and must be kept safe by the manufacturer.
**vendor_sign.pub.key** - This is the public key
**vendor_sign.key-tokens.txt** - This is the public key in another format which can be programmed into the device at manufacturing using simplicity commander.
A vendor may choose to have a keypair like this for all his products, one for each product type.

An encryption key can be generated as follows:
```bat
commander.exe gbl keygen --type aes-ccm -o vendor_encrypt.key
```
Once the two keys have been obtained, a GBL may be produced as follows:
```bat
commander.exe gbl create appname.gbl --app appname.hex --sign vendor_sign.key --encrypt vendor_encrypt.key --compress lzma
```
This should be done each time a new firmware is produced.


## Firmware update flow (Simplicity Studio)

1. Create a new `Solution Examples` project using the sample project as a template, such as `ZWave_SoC_SwitchOnOff_Solution`.
   This will create a new workspace with the bootloader and the Z-Wave application.
2. Open the <solution_name>.slpb (SL Postbuild Profile) file.
   This will open a GUI, where the encryption and signing keys can be added.
3. When the `Solution Examples` project is built, the GBL files are generated in the `artifacts` folder with the merged binary of the bootloader and the Z-Wave application.
4. Flash initial device firmware:
   ```bat
   commander.exe flash "{solution project location}\zwave_soc_switch_on_off.hex" --address 0x0 -s <board_jlink_serial>
   ```
5. Flash the encryption keys:
   ```bat
   commander.exe flash --tokengroup znet --tokenfile sample_encrypt.key --tokenfile sample_sign.keytokens.txt -s <board_jlink_serial>
   ```
6.  Reset device:
    ```bat
    commander.exe device reset -s <board_jlink_serial>
    ```
7. Connect a controller or a device running a controller firmware to the PC and start the PC controller application.
8. Include the node into the network and make sure the device is visible.
9. Initiate the OTA update in the PC controller application using an OTA-ready .gbl file with a firmware version greater than that of the current binary (see the *v255 generation* section below).

>**Note:** The automatic generation of GBL files also works with `Example Projects`, but in this case, the bootloader won't be merged with the Z-Wave application.

## v255 generation

A v255 binary is an application binary in .gbl format, the firmware version of which is set to `255.0.0`. This ensures that the image version is greater than the current firmware version on the device, which is a hard requirement for OTA updates. A suitable binary can be generated from a `Solution Example` or a `Project Example` in Simplicity Studio. The following steps are required to generate the v255 file:
1. Open `zw_version_config.h` in the GUI or in the text editor.
2. Set `USE_USER_APP_VERSION` to `1`.
3. Set `ZAF_VERSION_MAJOR` to `255`.
4. Set `ZAF_VERSION_MINOR` to `0`.
5. Set `ZAF_VERSION_PATCH` to `0`.

## Bootloader configuration

The bootloader resides at the start address `0x08000000` of the main flash and a fixed space of `24KB` is reserved for this. Z-Wave applications will start from address `0x08006000`.
The bootloader must be flashed first before the Z-Wave sample application is flashed. It is also possible to combine the bootloader and the Z-Wave application into a single image.
*One can use the pre-built bootloader images available in Simplicity Studio or build a bootloader image by themselves using the bootloader sample applications in Simplicity Studio.*
When building the bootloader, the OTA image storage information must be configured according to subsequent images.
Importantly, **this storage slot (slot0), start address and size must not be changed**.

![Bootloader 800 Storage Slot](bootloader_800_storageslot.png)
![Bootloader 800 OTA](bootloader_800_OTA.png)

In Simplicity Studio, the `Solution Examples` provide workspaces for the bootloader and the Z-Wave application. In the workspaces' postbuild profile the gbl files are generated and the bootloader and the Z-Wave application are combined into a single image (check `artifacts` folder).

## Linker Script for the 800 Series

The Z-Wave sample applications that are available in Simplicity Studio contain linker scripts that have been tuned to accommodate the OTA image-related configuration as well.
It is recommended not to modify this linker script when developing applications.

## Bootloader Compression for 800 Series

The bootloader compression type used for OTA is *lzma* compression. The corresponding component name to be selected in the studio is *bootloader_compression_lzma*.

## Application Upgrade Version

The studio component *bootloader_app_upgrade_version* has to be selected for checking the application version during upgrades.

## Erase and Read Manufacturing Tokens for the 800 Series

There is a dedicated space in the flash memory where the Manufacturing Tokens data can be stored.
This area can be written once during firmware running. To save the new region, the flash must be erased before.

Read token frequency:
```sh
commander tokendump --tokengroup znet --token MFG_ZWAVE_COUNTRY_FREQ
```

Write token frequency:
```sh
commander flash --tokengroup znet --token MFG_ZWAVE_COUNTRY_FREQ:0xFF
```

*0xFF* means this area is erased.
