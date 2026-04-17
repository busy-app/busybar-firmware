# SL I2S LOOPBACK

## Table of Contents

- [SL I2S LOOPBACK](#sl-i2s-loopback)
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
    - [Pin Description](#pin-description)
  - [Test the Application](#test-the-application)

## Purpose/Scope

This application demonstrates the I2S transfer using loopback mechanism.

## Overview

- The I2S_2CH supports two stereo channels, while the ULP_I2S and the NWP/Security subsystem I2S support one stereo channel.
- Supported programmable audio data resolutions are 16-, 24- and 32-bits.
- Supported audio sampling rates are 8, 11.025, 16, 22.05, 24, 32, 44.1, 48, 88.2, 96 and 192 kHz
- Support for Master and Slave modes.
- Full-duplex communication due to the independence of transmitter and receiver.
- Programmable FIFO thresholds with maximum FIFO depth of 8 and support for DMA.
- Supports generation of interrupts for different events.

## About Example Code

- This example fetches current I2S version using [sl_si91x_i2s_get_version](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/i2-s#sl-si91x-i2s-get-version)().
- Initializes I2S peripheral and store driver handle in i2s_driver_handle using [sl_si91x_i2s_init](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/i2-s#sl-si91x-i2s-init)().
- Get the transfer status of I2S peripheral using [sl_si91x_i2s_get_status](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/i2-s#sl-si91x-i2s-get-status)().
- Configure ARM power mode to full power using [sl_si91x_i2s_configure_power_mode](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/i2-s#sl-si91x-i2s-configure-power-mode)().
- Register user callback using [sl_si91x_i2s_register_event_callback](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/i2-s#sl-si91x-i2s-register-event-callback)().
- Configure transmitter and receiver transfer parameters for i2s using [sl_si91x_i2s_transmit_receive_config](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/i2-s#sl-si91x-i2s-transmit-receive-config)().
- Configure receive DMA channel and prepare I2S for data receiving.
- Configure transmit DMA channel and send data.
- When send data is received by receiver channel, it compares the data received with transferred data
- Loopback test will be pass after successful data comparison.
- After comparison, the send and receive operations are terminated using [sl_si91x_i2s_end_transfer](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/i2-s#sl-si91x-i2s-end-transfer)() API.

**Note!**

1. sl_i2s_xfer_config_t has the following parameters:

   (a) mode - Configure I2S device in Primary(Master)/Secondary(Slave) mode
   (b) sync - I2S synchronous mode (4-pin mode: SCK and WS signals are shared between I2S transmit and receive blocks)
   and asynchronous mode (requires SCK and WS pins). Currently, the driver only supports ASYNC mode.
   (c) protocol - I2S/PCM protocol. Currently, the driver only supports I2S protocol.
   (d) resolution - Audio data resolutions (16-bit, 24-bit and 32-bit)
   (e) data_size - Transfer buffer data type (8-bit, 16-bit and 32-bit)
   (f) sampling_rate - Audio sampling rate
   (g) transfer_type - Transfer type (Transmit, Receive, Transmit abort, and Receive abort)

2. Transfers with 16-bit resolution must use `uint16_t` data type buffer and pass SL_I2S_DATA_SIZE16 to data_size parameter in sl_i2s_xfer_config_t while configuring the transfer.
3. Transfers with 24-bit and 32-bit resolutions must use `uint32_t` data type buffer and pass SL_I2S_DATA_SIZE32 to data_size parameter in sl_i2s_xfer_config_t while configuring the transfer.
4. Since 8-bit resolution is not supported, `uint8_t` data type buffer can use 16-bit resolution for transfer and pass SL_I2S_DATA_SIZE8 to data_size parameter in sl_i2s_xfer_config_t while configuring the transfer. While performing this operation, the data buffer should be typecast to (`uint16_t *`) and the transfer size should be half of the 8-bit data type buffer. (Refer to the I2S loopback application for more details). For 8-bit transfers, transfer size should be multiples of 4 (8,12,16,20...).
5. Any I2S transfers with 16-bit and 32-bit resolutions should only have an even transfer size (8,10,12,14...).
6. Any I2S transfers with 24-bit resolutions should only have transfer size as multiples of 4 (8,12,16,20...).
7. I2S_LOOP_BACK macro is only used for I2S loopback applications to avoid clock generation from the receiver block during transfer.
8. SCK frequency is calculated using `SCK = 2 * bit_width * sampling_frequency`. By default, I2S0 uses I2S_PLL_CLK as a clock source. This can generate any frequency range mentioned in section 6.11.7 of Si91x HRM.
9. By default ULP_I2S/I2S1 uses ULP_MHZ_RC_CLK to support I2S operation in low-power states. This limits the maximum supported sampling frequency of ULP_I2S to 48kHz (32 MHz RC trims to 20MHz in sleep).

> **Note:**
>
>- In I2S Loopback mode, it is advisable to use the sl_si91x_i2s_end_transfer() API to conclude the current transfer before starting a new one.

## Prerequisites/Setup Requirements

### Hardware Requirements

- Windows PC
- Silicon Labs Si917 Evaluation Kit [[BRD4002](https://www.silabs.com/development-tools/wireless/wireless-pro-kit-mainboard?tab=overview) + [BRD4338A](https://www.silabs.com/development-tools/wireless/wi-fi/siwx917-rb4338a-wifi-6-bluetooth-le-soc-radio-board?tab=overview) / [BRD4342A](https://www.silabs.com/development-tools/wireless/wi-fi/siwx91x-rb4342a-wifi-6-bluetooth-le-soc-radio-board?tab=overview) / [BRD4343A](https://www.silabs.com/development-tools/wireless/wi-fi/siw917y-rb4343a-wi-fi-6-bluetooth-le-8mb-flash-radio-board-for-module?tab=overview)]
- SiWx917 AC1 Module Explorer Kit [BRD2708A](https://www.silabs.com/development-tools/wireless/wi-fi/siw917y-ek2708a-explorer-kit)

### Software Requirements

- Simplicity Studio
- Serial console setup
  - For serial Console setup instructions, see the [Console Input and Output](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#console-input-and-output) section in the *WiSeConnect Developer's Guide*.

### Setup Diagram

![Figure: setupdiagram](resources/readme/setupdiagram.png)

## Getting Started

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

- [Install Simplicity Studio](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#install-simplicity-studio)
- [Install WiSeConnect extension](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#install-the-wiseconnect-3-extension)
- [Connect your device to the computer](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#connect-siwx91x-to-computer)
- [Upgrade your connectivity firmware](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#update-siwx91x-connectivity-firmware)
- [Create a Studio project](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#create-a-project)

For details on the project folder structure, see the [WiSeConnect Examples](https://docs.silabs.com/wiseconnect/latest/wiseconnect-examples/#example-folder-structure) page.

## Application Build Environment

- Configure UC from the slcp component.

   ![Figure: i2s_loopback_uc_screen](resources/uc_screen/i2s_loopback_uc_screen.png)

- Open the **sl_si91x_i2s_loopback.slcp** project file, select the **Software Component** tab, and search for **I2S** in the search bar.
- Using the configuration wizard, you can configure different parameters like:

  - **General Configuration**

    - **SL_I2S0_RESOLUTION**: I2S0 resolution can be configured through this macro. Valid resolution values are 16, 24 and 32 bit.
    - **SL_I2S0_SAMPLING_RATE**: I2S0 sampling rates can be configured through this macro. Valid sampling rate values are 8kHz, 11.025kHz, 16kHz, 22.05kHz, 24kHz, 32kHz, 44.1kHz, 48kHz, 88.2kHz, 96kHz and 192kHz.
    - **SL_I2S0_CHANNEL**: I2S0 channel number (0-channel no 0, 1-channel no 1)
    - Configuration files are generated in **config folder**. If not changed, the code will run on default UC values.
    - Configure the following macros in [`i2s_loopback_example.c`](https://github.com/SiliconLabs/wiseconnect/blob/v4.0.1-content-for-docs/examples/si91x_soc/peripheral/sl_si91x_i2s_loopback/i2s_loopback_example.c) file and update/modify following macros if required.

      ```C
      #define I2S_BUFFER_SIZE 1024    ///< Transmit/Receive buffer size
      ```

  - **Pin Configuration**

      |     GPIO        | Explorer kit GPIO | Description            |
      | --------------- | ----------------- |----------------------- |
      | GPIO_25 [P25]   |    GPIO_25 [SCK]  | I2S frequency          |
      | GPIO_26 [P27]   |    GPIO_26 [MISO] | I2S Sampling frequency |
      | GPIO_28 [P31]   |    GPIO_28 [CS]   | I2S DOUT          |
      | GPIO_27 [P29]   |    GPIO_27 [MOSI] | I2S DIN           |

### Pin Description

>**Note:** Make sure that the pin configuration exists in the `RTE_Device_xxx.h` file.
>
> - SiWx917: RTE_Device_917.h (path: /$project/config/RTE_Device_917.h)

> **Note**: For recommended settings, please refer the [recommendations guide](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-prog-recommended-settings/).

## Test the Application

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

1. Connect GPIO_28 to GPIO_27 for loopback connection.
2. Compile and run the application.
3. When the application runs, it sends and receives data in loopback.
4. After successful program execution, the prints in serial console looks as shown below.

   ![ Figure: output](resources/readme/output.png)

> **Note:**
>
> - Interrupt handlers are implemented in the driver layer, and user callbacks are provided for custom code. If you want to write your own interrupt handler instead of using the default one, make the driver interrupt handler a weak handler. Then, copy the necessary code from the driver handler to your custom interrupt handler.
