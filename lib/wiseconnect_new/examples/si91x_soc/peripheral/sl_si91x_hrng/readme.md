# SL HRNG (Hardware Random Number Generator)

## Table of Contents

- [SL HRNG (Hardware Random Number Generator)](#sl-hrng-hardware-random-number-generator)
  - [Table of Contents](#table-of-contents)
  - [Purpose/Scope](#purposescope)
  - [Overview](#overview)
  - [About Example Code](#about-example-code)
  - [Prerequisites/Setup Requirements](#prerequisitessetup-requirements)
    - [Hardware Requirements](#hardware-requirements)
    - [Software Requirements](#software-requirements)
    - [Setup Diagram](#setup-diagram)
  - [Getting Started](#getting-started)
  - [Application Build Environment](#application-build-environment)
  - [Test the Application](#test-the-application)

## Purpose/Scope

- This application demonstrates the use of the Hardware Random Number Generator (HRNG) module to generate high-quality random numbers using hardware-based entropy sources.

## Overview

- The HRNG module provides an interface to generate random numbers using the hardware capabilities of the SI91x series micro-controllers.
- Key features of the HRNG module include:
  - High-quality random number generation
  - Hardware-based entropy source
  - **Pseudo-random number:** Pseudo-random number rely on mathematical algorithms to generate sequences of numbers that resemble randomness. These algorithms are deterministic, meaning they always produce the same sequence of numbers when initialized with the same starting value, referred to as a seed.
  - **True Random Number :** True Random Number create random numbers using physical processes like electronic noise, radioactive decay, or other unpredictable natural events.
  - Easy-to-use API

## About Example Code

This section provides instructions on how to get started with the Hardware Random Number Generator (HRNG) example.

- **Initialize the HRNG Peripheral**: Before using the HRNG, you need to initialize it. This can be done using the [`sl_si91x_hrng_init()`](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/hrng#sl-si91x-hrng-init) function.
- **Soft Reset Demonstration**: This example demonstrates the soft reset functionality by executing three test scenarios in sequence:
  1. **Soft Reset Enabled State**: Uses [`sl_si91x_hrng_soft_reset_set()`](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/hrng#sl-si91x-hrng-soft-reset-set) to put the HRNG peripheral in reset state. While in this state, the HRNG is held in reset and subsequent operations (start, get_bytes) will fail or produce no data, demonstrating the effect of the reset state.
  2. **PSEUDO_RANDOM Mode**: Uses [`sl_si91x_hrng_soft_reset_clear()`](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/hrng#sl-si91x-hrng-soft-reset-clear) to release the HRNG from reset state, then demonstrates PSEUDO_RANDOM mode operation. In this state, the HRNG operates normally and produces random data.
  3. **TRUE_RANDOM Mode**: Demonstrates TRUE_RANDOM mode operation, which uses hardware-based entropy sources for generating random numbers.
  
  This sequence helps users understand the difference between the reset and operational states of the HRNG peripheral, as well as the two available random number generation modes.
- **Start the HRNG**: After initialization, start the HRNG using the [`sl_si91x_hrng_start()`](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/hrng#sl-si91x-hrng-start) function with the desired mode (TRUE_RANDOM or PSEUDO_RANDOM).
- **Generate Random Numbers**: Use the [`sl_si91x_hrng_get_bytes()`](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/hrng#sl-si91x-hrng-get-bytes) function to generate random numbers. The example reads 10 random 32-bit values and validates that no duplicates are present.
- **Stop the HRNG**: Once you are done generating random numbers, stop the HRNG using the [`sl_si91x_hrng_stop()`](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/hrng#sl-si91x-hrng-stop) function.
- **Deinitialize the HRNG**: Finally, deinitialize the HRNG using the [`sl_si91x_hrng_deinit()`](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/hrng#sl-si91x-hrng-deinit) function to release any allocated resources and disable the peripheral clocks.

> **Note**: When the soft reset bit is set, the HRNG peripheral is held in reset and cannot generate random numbers. To use the HRNG after setting the soft reset, you must first clear the soft reset bit using `sl_si91x_hrng_soft_reset_clear()`.

## Prerequisites/Setup Requirements

### Hardware Requirements

- Windows PC
- Silicon Labs Si917 Evaluation Kit [[BRD4002](https://www.silabs.com/development-tools/wireless/wireless-pro-kit-mainboard?tab=overview) + [BRD4338A](https://www.silabs.com/development-tools/wireless/wi-fi/siwx917-rb4338a-wifi-6-bluetooth-le-soc-radio-board?tab=overview) / [BRD4342A](https://www.silabs.com/development-tools/wireless/wi-fi/siwx91x-rb4342a-wifi-6-bluetooth-le-soc-radio-board?tab=overview) / [BRD4343A](https://www.silabs.com/development-tools/wireless/wi-fi/siw917y-rb4343a-wi-fi-6-bluetooth-le-8mb-flash-radio-board-for-module?tab=overview)]

### Software Requirements

- Si91x
- Simplicity Studio
- Serial console Setup
  - For Serial Console setup instructions, refer to [link name](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#console-input-and-output).

### Setup Diagram

> ![Figure: setupdiagram](resources/readme/setupdiagram.png)

## Getting Started

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

- [Install Simplicity Studio](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#install-simplicity-studio)
- [Install WiSeConnect extension](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#install-the-wiseconnect-3-extension)
- [Connect your device to the computer](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#connect-siwx91x-to-computer)
- [Upgrade your connectivity firmware](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#update-siwx91x-connectivity-firmware)
- [Create a Studio project](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#create-a-project)

For details on the project folder structure, see the [WiSeConnect Examples](https://docs.silabs.com/wiseconnect/latest/wiseconnect-examples/#example-folder-structure) page.

## Application Build Environment

- Include the HRNG header file in your project:
  
  #include ["sl_si91x_hrng.h"](https://github.com/SiliconLabs/wiseconnect/blob/v4.0.1-content-for-docs/components/device/silabs/si91x/mcu/drivers/unified_api/inc/sl_si91x_hrng.h)
  

> **Note**: For recommended settings, please refer the [recommendations guide](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-prog-recommended-settings/).

 ## Test the Application

1. Compile and run the application. 
2. After successful program execution the prints in serial console looks as shown below.

   > ![Figure: output](resources/readme/output_hrng.png)

