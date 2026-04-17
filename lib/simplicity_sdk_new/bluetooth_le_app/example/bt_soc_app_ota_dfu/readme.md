# SoC - Application OTA DFU

 This example project demonstrates the Application Over-the-Air Device Firmware Upgrade (OTA DFU) service, which enables firmware update during application runtime without resetting the device into 'OTA DFU mode' and without installing any application loader utility on the device. The downloaded firmware is stored in dedicated flash storage (slot 0). Once the download has finished, the bootloader is configured to update the firmware on the device. During the reboot, the new firmware is copied to the application space in flash, and the new application is loaded.

 > Note: this example expects a specific Gecko Bootloader to be present on your device. For details see the [Troubleshooting](#troubleshooting) section.

## Getting Started

To learn the Bluetooth technology basics, see [UG103.14: Bluetooth® LE Fundamentals](https://www.silabs.com/documents/public/user-guides/ug103-14-fundamentals-ble.pdf).

To get started with Bluetooth and Simplicity Studio, see [QSG169: Bluetooth® Quick-Start Guide for SDK v3.x and Higher](https://www.silabs.com/documents/public/quick-start-guides/qsg169-bluetooth-sdk-v3x-quick-start-guide.pdf).

OTA DFU means the firmware upgrade image is sent to the device via a Bluetooth connection. The new firmware can be uploaded either by Apploader (a standalone Bluetooth application that runs in the bootloader or on its own) or by the user application. Each solution has its own advantages and disadvantages.
* Apploader runs independently of the user application and can overwrite the old application directly, without an internal step. This is useful if there is no extra storage space. It is also a safe solution because, even if there is a serious bug in the user application, the device can still be started in DFU mode and new firmware can be installed without issues.
* Application OTA DFU means that the image is uploaded during the user application's runtime. This needs extra storage space (since the application cannot overwrite itself). On the other hand, it allows checking the validity of the firmware upgrade image before installation and offers more configurability and interactivity during the DFU sequence.

![OTA DFU with Apploader and Application OTA DFU](image/readme_img1.png)

This example project demonstrates Application OTA DFU, i.e. how to upload and install a new firmware image from the user application using the proper software components.

To learn more about Device Firmware Upgrade, see [AN1086: Using the Gecko Bootloader with the Silicon Labs Bluetooth® Applications](https://www.silabs.com/documents/public/application-notes/an1086-gecko-bootloader-bluetooth.pdf).


## Features Implemented in the Example

This example demonstrates how to use the Application OTA DFU software component.

* A simple application is implemented in the event handler function that starts advertising on boot (and on connection_closed event). This makes it possible for remote devices to find the device and connect to it.

* A simple serial port-based logger component is added to the project, which is useful for debugging and testing the application OTA features.

* A simple GATT database is defined by adding Generic Access and Device Information services. This allows remote devices to read some basic information, such as the device name.

* The application OTA software component is added, which extends both the event handlers (see sl_app_ota.c) and the GATT database (OTA DFU service). This enables OTA application updates during runtime without any additional application code.

By default, the Application OTA DFU software component manages the entire OTA DFU process. It manages the upload process and interfaces with the bootloader. The main application (app.c) only extends this component with additional features, such as progress reporting and error handling, and handles the device reboot. The developer is free to modify this code.

## Testing the Example

This is a minimal example with the application OTA service that allows it to do a firmware update during runtime. There is no need to change to 'OTA mode' and no Apploader utility is required. Once the new firmware is downloaded to the device, a restart will happen and the new firmware will overwrite the original application content. The application OTA example has to be uploaded to a device that already has an **internal storage bootloader** with at least **one configured flash storage** (slot 0) with enough capacity to store the new firmware. When started, the application first checks this flash storage and if necessary it will erase its content. When the OTA update finished and reboot is done, this storage will be cleared again if necessary. To test this update feature do the following:

1. Build the example solution and flash the combined bootloader + application binary to your device. (See [Troubleshooting](#troubleshooting) section.)
1. Modify the code so that you can differentiate the old and the new firmware (e.g., add an app_log message in the *app_init()* function or change the device name in the GATT database).
1. Optionally, you can modify the *create_gbl* step in the *Post-Build Editor (PBE)*. You can learn more about generating GBL files using the *Post-Build Editor* in [Simplicity Studio 5 Users Guide: Post-Build Editor (PBE)](https://docs.silabs.com/simplicity-studio-5-users-guide/latest/ss-5-users-guide-building-and-flashing/post-build-editor).
1. Build the project again, but do not flash.
1. Observe that a GBL (Gecko Bootloader) file is generated automatically along with the build artifacts. The filename is something similar to *bt_soc_app_ota_dfu.gbl* but may vary based on the project variant. Copy this file to your smartphone.
1. Open a terminal program and connect to your radio board via the JLink adapter to see the debug messages.
1. Download the **Simplicity Connect** smartphone application available on [iOS](https://apps.apple.com/us/app/simplicity-connect/id1030932759) and [Android](https://play.google.com/store/apps/details?id=com.siliconlabs.bledemo&hl=en&gl=US).
1. Open the app and choose the [Scan] option.
   ![Simplicity Connect start scanning](image/readme_img2.png)
1. Now you should find your device advertising as "Application OTA". Tap **Connect**.
   ![Scan results](image/readme_img3.png)
1. The connection is opened, and the GATT database is automatically discovered. Find the device name characteristic under Generic Access service and try to read out the device name.
1. Select **OTA Firmware** option.
   ![GATT database of the device](image/readme_img4.png)
1. Use the Partial OTA tab (default)
   ![OTA DFU popup](image/readme_img5.png)
1. Select the *bt_soc_app_ota_dfu.gbl* file and tap **Upload** to start the OTA transfer.
1. Once it's done, tap **END** to finalize the process.
1. Press push button 0 on your kit to restart your device. The bootloader will automatically install the new firmware image.
1. Verify that the new firmware has started by checking the modified device name in Simplicity Connect or by reviewing the updated logs in the terminal.


## Troubleshooting

### Bootloader Issues

This example solution includes 2 projects: the application example and a compatible bootloader.
When flashing this solution to the device, make sure to select the combined binary from the `artifact` folder.
This combined binary is named after the example solution, e.g. `bt_soc_app_ota_dfu_btl.s37`.

Flashing only the application binary (e.g., `bt_soc_app_ota_dfu.s37`) will work only if a compatible bootloader is already present on the device.


### Programming the Radio Board

Before programming the radio board mounted on the mainboard, make sure the power supply switch is in the AEM position (right side) as shown below.

![Radio board power supply switch](image/readme_img0.png)



## Resources

[Bluetooth Documentation](https://docs.silabs.com/bluetooth/latest/)

[UG103.14: Bluetooth® LE Fundamentals](https://www.silabs.com/documents/public/user-guides/ug103-14-fundamentals-ble.pdf)

[QSG169: Bluetooth® Quick-Start Guide for SDK v3.x and Higher](https://www.silabs.com/documents/public/quick-start-guides/qsg169-bluetooth-sdk-v3x-quick-start-guide.pdf)

[UG434: Silicon Labs Bluetooth ® C Application Developer's Guide for SDK v3.x](https://www.silabs.com/documents/public/user-guides/ug434-bluetooth-c-soc-dev-guide-sdk-v3x.pdf)

[AN1086: Using the Gecko Bootloader with the Silicon Labs Bluetooth® Applications](https://www.silabs.com/documents/public/application-notes/an1086-gecko-bootloader-bluetooth.pdf)

[Bluetooth Training](https://www.silabs.com/support/training/bluetooth)

[Uploading Firmware Images Using OTA DFU](https://docs.silabs.com/bluetooth/latest/general/firmware-upgrade/uploading-firmware-images-using-ota-dfu)



## Report Bugs & Get Support

You are always encouraged and welcome to report any issues you found to us via [Silicon Labs Community](https://www.silabs.com/community).
