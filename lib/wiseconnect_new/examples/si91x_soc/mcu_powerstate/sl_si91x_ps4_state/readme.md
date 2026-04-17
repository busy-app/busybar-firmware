# PS4 State With NWP Shutdown

## Table of Contents

- [PS4 State With NWP Shutdown](#ps4-state-with-nwp-shutdown)
  - [Table of Contents](#table-of-contents)
  - [Purpose/Scope](#purposescope)
  - [About Example Code](#about-example-code)
  - [Prerequisites/Setup Requirements](#prerequisitessetup-requirements)
    - [Hardware Requirements](#hardware-requirements)
    - [Software Requirements](#software-requirements)
    - [Setup Diagram](#setup-diagram)
  - [Getting Started](#getting-started)
  - [Application Build Environment](#application-build-environment)
  - [Test the Application](#test-the-application)

## Purpose/Scope

- This application demonstrates the PS4 sleep and active state with RAM retention and NWP shutdown. 
  ```c
  Note: This application is designed to validate the MCU's power consumption values specified in the datasheet, and thus contain meticulous optimizations. This application should not be taken as reference for a real-time use case project bring up. This application does not support wake-up sources.
  ```

## About Example Code

- This example demonstrates the transition to the PS4 active and sleep states, with RAM retention and NWP shutdown. By default, our application is configured to retain 320KB RAM.
- Initially, the Power Manager service is initialized, transitioning the processor to the PS3 state with the clock set to 40 MHz (Power Save) using sl_si91x_power_manager_init.
- The power domains will be disabled and NWP will be shutdown.
- The application will then switch to the PS4 state and clock is at 180 MHz(performance mode).
- Finally, the application will enter the ps4 sleep mode.
- To ensure a continuous PS4 active state, configure the ACTIVE_STATE macro to 1 in the application. This will transition the application to the PS4 state with the clock running at 180 MHz.

## Prerequisites/Setup Requirements

- To use this application following Hardware, Software and Project Setup is required.

### Hardware Requirements

- Windows PC
- Silicon Labs Si917 Evaluation Kit [[BRD4002](https://www.silabs.com/development-tools/wireless/wireless-pro-kit-mainboard?tab=overview) + [BRD4338A](https://www.silabs.com/development-tools/wireless/wi-fi/siwx917-rb4338a-wifi-6-bluetooth-le-soc-radio-board?tab=overview) / [BRD4342A](https://www.silabs.com/development-tools/wireless/wi-fi/siwx91x-rb4342a-wifi-6-bluetooth-le-soc-radio-board?tab=overview) / [BRD4343A](https://www.silabs.com/development-tools/wireless/wi-fi/siw917y-rb4343a-wi-fi-6-bluetooth-le-8mb-flash-radio-board-for-module?tab=overview)]
  - The Serial Console setup instructions are provided below:
Refer to instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#console-input-and-output).

### Software Requirements

- Simplicity Studio
- Embedded Development Environment
  - For Silicon Labs Si91x, use the latest version of Simplicity Studio (refer **"Download and Install Simplicity Studio"** section in **getting-started-with-siwx917-soc** guide at **release_package/docs/index.html**)

### Setup Diagram

![Figure: Introduction](resources/readme/setupdiagram.png)

## Getting Started

Refer instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) for the following tasks:

- Install Studio and WiSeConnect extension
- Connect your device to the computer
- Upgrade your connectivity firmware
- Create a Studio project

For details on the project folder structure, see the [WiSeConnect Examples](https://docs.silabs.com/wiseconnect/latest/wiseconnect-examples/#example-folder-structure) page.

## Application Build Environment

The application can be configured to suit user requirements and development environment. Read through the following sections and make any changes needed.

Configure the following parameters to enable your PS4 Active or sleep state.

  - By default, the ACTIVE_STATE macro is set to 0, which means the application will enter the sleep state.
  
    ```c
    #define ACTIVE_STATE                                      0 
    ```

  - To enable the PS4 active state, configure the ACTIVE_STATE macro to 1.

    ```c
    #define ACTIVE_STATE                                      1
    ```

> **Note**: For recommended settings, please refer the [recommendations guide](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-prog-recommended-settings/).

## Test the Application

Refer instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) for the following tasks:

1. Compile and run the application.
2. By default it switches the NWP off, add the PS4 requirement and goes to sleep.
3. After successful program execution, the prints in the serial console look as shown below.
  ![Figure: Ps4 Sleep State](resources/readme/ps4_sleep_state.png)
4. For Active state, please set the ACTIVE_STATE macro as 1.
5. After successful program execution, the prints in the serial console look as shown below.
  ![Figure: PS4 Active State](resources/readme/ps4_active_state.png)
