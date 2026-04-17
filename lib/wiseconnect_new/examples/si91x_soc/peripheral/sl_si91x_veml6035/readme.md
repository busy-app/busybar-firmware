# SL VEML6035

## Table of Contents

- [SL VEML6035](#sl-veml6035)
  - [Purpose/Scope](#purposescope)
  - [About Example Code](#about-example-code)
  - [Prerequisites/Setup Requirements](#prerequisitessetup-requirements)
    - [Hardware Requirements](#hardware-requirements)
    - [Software Requirements](#software-requirements)
    - [Setup Diagram](#setup-diagram)
  - [Getting Started](#getting-started)
  - [Application Build Environment](#application-build-environment)
    - [Pin Configuration](#pin-configuration)
  - [Test the Application](#test-the-application)

## Purpose/Scope

 This application demonstrates the VEML6035 Ambient Light Sensor, which measures the Lux via I2C interface for every 2 seconds. The lux is the unit of illuminance, or luminous flux per unit area.

## About Example Code

This example demonstrates the measurement of Lux for every 2 seconds. It also shows how to use different APIs present via the I2C interface.

## Prerequisites/Setup Requirements

### Hardware Requirements

- Windows PC
- Silicon Labs SiWx917 Dev Kit [BRD2605A](https://www.silabs.com/development-tools/wireless/wi-fi/siwx917-dk2605a-wifi-6-bluetooth-le-soc-dev-kit?tab=overview)

### Software Requirements

- Simplicity Studio
- Serial console Setup
  - For Serial Console setup instructions, see the [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#console-input-and-output).

### Setup Diagram

![Figure: setupdiagram](resources/readme/setupdiagram.png)

## Getting Started

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

- [Install Simplicity Studio](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#install-simplicity-studio)
- [Install WiSeConnect extension](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#install-the-wiseconnect-3-extension)
- [Connect your device to the computer](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#connect-siwx91x-to-computer)
- [Upgrade your connectivity firmware](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#update-siwx91x-connectivity-firmware)
- [Create a Studio project](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#create-a-project)

## Application Build Environment

- Configure the following macros in [`veml6035_example.c`](https://github.com/SiliconLabs/wiseconnect/blob/v4.0.1-content-for-docs/examples/si91x_soc/peripheral/sl_si91x_veml6035/veml6035_example.c) file. .Update or  modify the following macros, if required.

  - `DELAY_PERIODIC_MS1` : Select the delay for data display. By default, the delay is kept as 2 seconds.

    ```C
    #define DELAY_PERIODIC_MS1 2000    // sleeptimer1 periodic timeout in ms
    ```

  - `I2C instance`: Select I2C instance for communication through UC from the VEML6035 slcp component. By default I2C2 is selected.

    ![Figure: Veml6035 UC](resources/uc_screen/veml6035_uc_screen.png)

### Pin Configuration

- The sensor is internally connected on SiWG917 Dev kit board on ULP_I2C.
- However, to use this sensor driver on Custom boards, the following pins are supported based on the instance selected in UC.

**I2C0:**

| PIN |  ULP GPIO PIN       |        Description          |
| --- | ------------------- | --------------------------- |
| SCL |  GPIO_7 [P20]       | Connect to Follower SCL pin |
| SDA |  GPIO_6 [P19]       | Connect to Follower SDA pin |

**I2C1:**

| PIN |     GPIO PIN        |      Description            |
| --- | ------------------- | --------------------------- |
| SCL |   GPIO_50 [P32]     | Connect to Follower SCL pin |
| SDA |   GPIO_51 [P34]     | Connect to Follower SDA pin |

**ULP_I2C:**

| PIN |        ULP GPIO PIN        |      Description            |
| --- | -------------------------- | --------------------------- |
| SCL | ULP_GPIO_7 [EXP_HEADER-15] | Connect to Follower SCL pin |
| SDA | ULP_GPIO_6 [EXP_HEADER-16] | Connect to Follower SDA pin |


## Test the Application

See the instructions [Creating a Project](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#create-a-project) section on the **Developing with Boards in SoC Mode** guide to:

1. Compile and run the application.
2. When the application runs, it measures **ALS channel lux** and **white channel lux** every 2 seconds.
3. After successful program execution, the output to the serial console should look similar to the following image:

   ![Figure: console_output_veml6035](resources/readme/console_output_veml6035.png)

> **Note:**
>
> - Interrupt handlers are implemented in the driver layer, and user callbacks are provided for custom code. If you want to write your own interrupt handler instead of using the default one, make the driver interrupt handler a weak handler. Then, copy the necessary code from the driver handler to your custom interrupt handler.
