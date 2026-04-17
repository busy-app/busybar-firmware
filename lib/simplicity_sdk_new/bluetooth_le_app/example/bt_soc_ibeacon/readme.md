# SoC - iBeacon

An iBeacon device is an implementation that sends non-connectable advertisements in iBeacon format. The iBeacon Service gives Bluetooth accessories a simple and convenient way to send iBeacon to smartphones. This example can be tested together with the Simplicity Connect mobile app.

## Getting Started

Introduced in iOS 7, iBeacon enables new location awareness possibilities for apps. Leveraging Bluetooth Low Energy (BLE), a device with iBeacon technology can be used to establish a region around an object. This allows an iOS device to determine when it has entered or left the region, along with an estimation of proximity to a beacon.

For more beacon information, see [Apple iBeacon](https://developer.apple.com/ibeacon/)

To get started with Silicon Labs Bluetooth and Simplicity Studio, see [QSG169: Bluetooth SDK v3.x Quick Start Guide](https://www.silabs.com/documents/public/quick-start-guides/qsg169-bluetooth-sdk-v3x-quick-start-guide.pdf).

To run this example, you will need:

- Any Bluetooth Low Energy-compatible [radio board](https://www.silabs.com/wireless/bluetooth),
- A smartphone with [Simplicity Connect app](https://www.silabs.com/developer-tools/simplicity-connect-mobile-app) installed. (Note: On Android, the distance will not be calculated.)

The following picture shows the system view of how it works.

![SoC iBeacon system overview](image/readme_img1.png)

Follow these steps to set up the project:

1. Create the SoC-iBeacon project based on your hardware, then build and flash the image to your board. Alternatively, you could flash the pre-built demo image.

   ![Create project](image/readme_img2.png)

2. Open the *Simplicity Connect* app on your smartphone and allow the permission requested the first time it is opened.

3. Click [Scan]. You will see a list of nearby devices that are sending Bluetooth advertisement. Find the one with "iBeacon" tag.

   ![Scan](image/readme_img3.png)

   You can also see the Minor, Major, and UUID number, and the EFR app will estimate the distance. (Note: On Android, the distance will not be calculated.)

## Troubleshooting

### Programming the Radio Board

Before programming the radio board mounted on the mainboard, make sure the power supply switch is in the AEM position (right side) as shown below.

![Radio board power supply switch](image/readme_img0.png)


## Resources

[Bluetooth Documentation](https://docs.silabs.com/bluetooth/latest/)

[UG103.14: Bluetooth LE Fundamentals](https://www.silabs.com/documents/public/user-guides/ug103-14-fundamentals-ble.pdf)

[QSG169: Bluetooth SDK v3.x Quick Start Guide](https://www.silabs.com/documents/public/quick-start-guides/qsg169-bluetooth-sdk-v3x-quick-start-guide.pdf)

[UG434: Silicon Labs Bluetooth ® C Application Developer's Guide for SDK v3.x](https://www.silabs.com/documents/public/user-guides/ug434-bluetooth-c-soc-dev-guide-sdk-v3x.pdf)

[Bluetooth Training](https://www.silabs.com/support/training/bluetooth)

[Getting-Started-with-iBeacon](https://developer.apple.com/ibeacon/Getting-Started-with-iBeacon.pdf)

## Report Bugs & Get Support

You are always encouraged and welcome to report any issues you found to us via [Silicon Labs Community](https://www.silabs.com/community).
