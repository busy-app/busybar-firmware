# BLE - Long Range

## Table of Contents

- [BLE - Long Range](#ble---long-range)
  - [Table of Contents](#table-of-contents)
  - [Purpose/Scope](#purposescope)
  - [Prerequisites/Setup Requirements](#prerequisitessetup-requirements)
    - [Hardware Requirements](#hardware-requirements)
    - [Software Requirements](#software-requirements)
    - [Setup Diagram](#setup-diagram)
  - [Getting Started](#getting-started)
  - [Application Build Environment](#application-build-environment)
  - [Test the Application](#test-the-application)

## Purpose/Scope

This application demonstrates how to configure/connects SiWx91x as a Central and can be used to update the PHY rates either coded PHY(long range) or 2Mbps PHY rates. The PHY update Procedure is used to change the Transmit or receive PHYs, or both.

The procedure can be initiated either on a request by the host or autonomously by the Link Layer. Either the central or the peripheral may initiate this procedure at any time after entering the Connection State.

## Prerequisites/Setup Requirements

### Hardware Requirements

- Windows PC
- SoC Mode:
  - Silicon Labs [BRD4325A, BRD4325B, BRD4325C, BRD4325G, BRD4338A, BRD4339B, BRD4343A](https://www.silabs.com/)
  - Kits
    - SiWx917 AC1 Module Explorer Kit (BRD2708A)
- PSRAM Mode:  
  - Silicon Labs [BRD4340A, BRD4342A, BRD4325G](https://www.silabs.com/)
- NCP Mode:
  - Silicon Labs [BRD4180B](https://www.silabs.com/)
  - Host MCU Eval Kit. This example has been tested with:
    - Silicon Labs [WSTK + EFR32MG21](https://www.silabs.com/development-tools/wireless/efr32xg21-bluetooth-starter-kit)
    - NCP Expansion Kit with NCP Radio boards
      - (BRD4346A + BRD8045A) [SiWx917-EB4346A]
      - (BRD4357A + BRD8045A) [SiWx917-EB4357A]
- Interface and Host MCU Supported
  - SPI - EFR32 

### Software Requirements

- Simplicity Studio IDE
  
- Download and install the Silicon Labs [Simplicity Connect App(formerly EFR Connect App) or other BLE Central/Peripheral app.](https://www.silabs.com/developers/simplicity-connect-mobile-app ) in the android smart phones for testing BLE applications. Users can also use their choice of BLE apps available in Android/iOS smart phones.

### Setup Diagram

![](resources/readme/ble_longrange_2mbps_soc_ncp.png)

## Getting Started

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

- [Install Simplicity Studio](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#install-simplicity-studio)
- [Install WiSeConnect extension](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#install-the-wi-se-connect-extension)
- [Connect your device to the computer](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#connect-si-wx91x-to-computer)
- [Upgrade your connectivity firmware ](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#update-si-wx91x-connectivity-firmware)
- [Create a Studio project ](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#create-a-project)

For details on the project folder structure, see the [WiSeConnect Examples](https://docs.silabs.com/wiseconnect/latest/wiseconnect-examples/#example-folder-structure) page.

## Application Build Environment

The application can be configured to suit your requirements and development environment.

Read through the following sections and make any changes needed.

- Open `app.c` file and update/modify following macros,

  - `RSI_DEVICE_DATA_RATE` refers the PHY data rate of the Silicon Labs module to be selected.

    - If user configures, **LONG_RANGE**, Silicon Labs module will update PHY data rates to coded PHY data rate. By default it is configured to 500kbps PHY data rate.

    - If user configures, **BLE_2MBPS**, Silicon Labs module will update 2Mbps PHY data rates.

    ```c
    #define RSI_DEVICE_DATA_RATE                LONG_RANGE
    ```

  - `RSI_BLE_DEV_ADDR_TYPE` refers address type of the remote device to connect.

    ```c
    #define RSI_BLE_DEV_ADDR_TYPE                         LE_PUBLIC_ADDRESS
    ```

    - Based on the address of the advertising device, Valid configurations are

      - LE_RANDOM_ADDRESS
      - LE_PUBLIC_ADDRESS

    >  **Note:** Depends on the remote device, address type will be changed.

  - `RSI_BLE_DEV_ADDR` refers address of the remote device to connect.

    ```c
    #define RSI_BLE_DEV_ADDR                          "00:23:A7:80:70:B9"
    ```

  - `RSI_REMOTE_DEVICE_NAME` refers the name of remote device to which Silicon Labs device has to connect

    ```c
    #define RSI_REMOTE_DEVICE_NAME                              "SILABS_DEV"
    ```

    >  **Note:** Silicon Labs module can connect to remote device by referring either RSI_BLE_DEV_ADDR or  RSI_REMOTE_DEVICE_NAME of the remote device.

  - Power save configuration

    - By default, the application is configured without power save.

      ```c  
      #define ENABLE_NWP_POWER_SAVE 0
      ```

    - If user wants to run the application in power save, modify the following configuration.

      ```c  
      #define ENABLE_NWP_POWER_SAVE 1
      ```

  - The desired parameters are provided in following steps. User can also modify the parameters as per their needs and requirements.

  - Open `ble_config.h` file and update/modify following macros,

    ```c
    #define RSI_BLE_PWR_INX                                   30
    #define RSI_BLE_PWR_SAVE_OPTIONS                          0 
    ```

    > **Note:** `ble_config.h` files are already set with desired configuration in respective example folders user need not change for each example.

> **Note**: For recommended settings, please refer the [recommendations guide](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-prog-recommended-settings/).

## Test the Application

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

- Build the application in Studio.
- Flash, run and debug the application.
    > **Note:**  The provided mobile screenshots are from the 2.8.1 version of the Simplicity Connect App(formerly EFR Connect App), it is recommended to use the latest version.

Follow the steps for successful execution of the application:

1. Configure the remote BLE device in peripheral mode, add the complete local name record,  Enable the Scan response data, and connectable options to the advertising data. And keep it in the Advertising mode. Ensure that the specified remote device name in the RSI_REMOTE_DEVICE_NAME macro is proper.

    ![](resources/readme/blelradvertising.png)     

   > **Note:** Refer the [Creating New Advertisement Sets](https://docs.silabs.com/bluetooth/5.0/miscellaneous/mobile/efr-connect-mobile-app) for configuring the Simplicity Connect mobile App(formerly EFR Connect App) connect mobile APP as advertiser.

2. After the program gets executed, Silicon Labs device tries to connect with the remote device specified in **RSI_BLE_DEV_ADDR** **or RSI_REMOTE_DEVICE_NAME** macro.

3. Clicks on "YES AND OPEN" button and Observe that the connection is established between the desired device and Silicon Labs device.

    ![](resources/readme/blelrconnection.png) 

4. After connection, Silicon Labs device will read PHY rate of the remote device and set PHY rate of the remote device.

    ![](resources/readme/blelrconnection1.png)

5. Observe PHY update complete event after setting PHY rate.

    ![](resources/readme/output1.png)

6. When Silicon Labs module is configured as **LONG_RANGE** PHY data rate, the Slicon Labs module and remote device updates its PHY data rate to 500kbps and prints in docklight looks as shown below.

    ![](resources/readme/500kbps.png)

7. When Silicon Labs module is configured as **BLE_2MBPS** PHY data rate, the Slicon Labs module and remote device updates its PHY data rate to 2mbps and prints in docklight looks as shown below.

    ![](resources/readme/2mbps.png)

    


