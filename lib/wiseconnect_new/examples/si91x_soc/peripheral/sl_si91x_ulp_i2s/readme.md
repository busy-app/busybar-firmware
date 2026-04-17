# SI91x - SL_ULP_I2S

## Table of Contents

- [SI91x - SL\_ULP\_I2S](#si91x---sl_ulp_i2s)
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

This application demonstrates how to configure ULP_I2S in PS2 state and master mode. This example connects the TX pin to RX pin in loopback mode.

The Master transmits data on the TX pin using DMA and receives the same data on a RX pin using DMA.

The data received should match the transmitted data.

## Overview

- The I2S_2CH supports two stereo channels, while the ULP_I2S and NWP/Security subsystem I2S support one stereo channel.
- Supported programmable audio data resolutions are 16-, 24-, and 32-bits.
- Supported audio sampling rates are 8, 11.025, 16, 22.05, 24, 32, 44.1, 48, 88.2, 96, and 192 kHz.
- Supports Master and Slave modes.
- Full-duplex communication due to the independence of transmitter and receiver.
- Programmable FIFO thresholds with maximum FIFO depth of 8 and support for DMA.
- Supports generation of interrupts for different events.

## About Example Code

- This example fetches current I2S version using [sl_si91x_i2s_get_version](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/i2-s#sl-si91x-i2s-get-version).
- Initializes I2S peripheral and stores driver handle in i2s_driver_handle using [sl_si91x_i2s_init](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/i2-s#sl-si91x-i2s-init).
- Gets the transfer status of I2S peripheral using [sl_si91x_i2s_get_status](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/i2-s#sl-si91x-i2s-get-status).
- Configures ARM power mode to full power using [sl_si91x_i2s_configure_power_mode](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/i2-s#sl-si91x-i2s-configure-power-mode).
- Registers the user callback using [sl_si91x_i2s_register_event_callback](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/i2-s#sl-si91x-i2s-register-event-callback).
- Configures transmitter and receiver transfer parameters for i2s using [sl_si91x_i2s_transmit_receive_config](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/i2-s#sl-si91x-i2s-transmit-receive-config).
- Configures receive DMA channel and prepares I2S for data receivings.
- Configures transmit DMA channel and sends data.
- When send data is received by the receiver channel, it compares the data received with transferred data.
- Loopback test will be passed after successful data comparison.
- Switches the power state from PS4 to PS2.
- Performs another loopback test.
- Switches the power state from PS2 to PS4.
- Performs another loopback test,

> **Note:** When utilizing the I2S1 instance in high-power mode with DMA enabled, it is advisable to allocate buffers in the ULP Memory block.

**Note!**

1. `sl_i2s_xfer_config_t` has following parameters:
   (a) **mode** - Configure I2S device in Primary (Master) /Secondary (Slave) mode.
   (b) **sync** - I2S synchronous mode (4-pin mode: SCK and WS signals are shared between I2S transmit and receive blocks) and asynchronous mode (requires SCK and WS pins). Currently, the driver only supports ASYNC mode.
   (c) **protocol** - I2S/PCM protocol. Currently, the driver only supports I2S protocol.
   (d) **resolution** - Audio data resolutions (16-, 24-, and 32-bit).
   (e) **data_size** - Transfer buffer data type (8-, 16- and 32-bit).
   (f) **sampling_rate** - Audio sampling rate.
   (g) **transfer_type** - Transfer type (Transmit, Receive, Transmit abort, and Receive abort).
2. Transfers with 16-bit resolution must use a `uint16_t` data type buffer and pass SL_I2S_DATA_SIZE16 to the data_size parameter in `sl_i2s_xfer_config_t` while configuring the transfer.
3. Transfers with 24-bit and 32-bit resolutions must use a `uint32_t` data type buffer and pass `SL_I2S_DATA_SIZE32` to the data_size parameter in `sl_i2s_xfer_config_t` while configuring the transfer.
   - For **24-bit resolution**, only the lower 24 bits of each 32-bit word are valid; the upper 8 bits must be set to **zero**.  
   - Buffers should be aligned to **32-bit boundaries**.  
   - **Transfer-size rules:** 24-bit → multiples of 4 (see Point 6); 32-bit → even numbers (see Point 5).  
   - **Buffer size constraints (example):**  

     ```c
     #define I2S_LOWPOWER_BUFFER_SIZE   1024  // Samples per buffer in 16-bit mode (uint16_t)
     #define ULP_I2S_INSTANCE           1     // I2S instance index
     #define I2S_ULP_BANK_OFFSET        0x800 // 2048 bytes per bank
     #define I2S_TX_BUF_MEMORY          (ULP_SRAM_START_ADDR + (1 * I2S_ULP_BANK_OFFSET))
     #define I2S_RX_BUF_MEMORY          (ULP_SRAM_START_ADDR + (2 * I2S_ULP_BANK_OFFSET))
     // Each bank = 0x800 = 2048 bytes (2 KB)
     ```

     - With **16-bit** data (2 bytes/sample), one 2 KB bank holds **1024 samples** → `I2S_LOWPOWER_BUFFER_SIZE = 1024`.  
     - With **24/32-bit** data (4 bytes/sample), one **2 KB** bank holds **512 samples** → `I2S_LOWPOWER_BUFFER_SIZE = 512`.
        - Because the ULP memory is fixed at **4 KB total** in this example (2 KB TX + 2 KB RX), when using 32-bit (or 24-bit packed in 32-bit) **set `I2S_LOWPOWER_BUFFER_SIZE` to 512 samples per buffer**. Do **not** increase the buffer size beyond this under `SL_I2S_DATA_SIZE32`. 
4. Since 8-bit resolution is not supported, a `uint8_t` data type buffer can use 16-bit resolution for transfers and pass SL_I2S_DATA_SIZE8 to the data_size parameter in  `sl_i2s_xfer_config_t` while configuring the transfer. While performing this operation, the data buffer should be typecast to `(uint16_t *)`, and the transfer size should be half of the 8-bit data type buffer. (Refer to the I2S loopback application for more details.) For 8-bit transfers, the transfer size should be multiples of 4 (8,12,16,20...).
5. Any I2S transfers with 16-bit and 32-bit resolutions should only have an even transfer size (8,10,12,14...).
6. Any I2S transfers with 24-bit resolution should only have transfer size as multiples of 4 (8,12,16,20...).
7. The `I2S1_LOOP_BACK` macro is used only for I2S loopback applications to avoid clock generation from the receiver block during transfer.
8. SCK frequency is calculated using `SCK = 2 * bit_width * sampling rate`. By default, I2S0 uses I2S_PLL_CLK as a clock source. This can generate any frequency range mentioned in section 6.11.7 of the Si91x HRM.

   >**Note:**
   >
   > - The exact I2S clock frequency may not be achieved as intended because the integral part of the calculated division factor is written into register ignoring decimal part.
   >
9. By default, ULP_I2S/I2S1 uses ULP_I2S_REF_CLK to support I2S operation in low-power states. This limits the maximum supported sampling frequency of ULP_I2S to 48kHz (32 MHz RC trims to 20MHz in  ULP_State).

## Prerequisites/Setup Requirements

### Hardware Requirements

- Windows PC
- Silicon Labs Si917 Evaluation Kit [[BRD4002](https://www.silabs.com/development-tools/wireless/wireless-pro-kit-mainboard?tab=overview) + [BRD4338A](https://www.silabs.com/development-tools/wireless/wi-fi/siwx917-rb4338a-wifi-6-bluetooth-le-soc-radio-board?tab=overview) / [BRD4342A](https://www.silabs.com/development-tools/wireless/wi-fi/siwx91x-rb4342a-wifi-6-bluetooth-le-soc-radio-board?tab=overview) / [BRD4343A](https://www.silabs.com/development-tools/wireless/wi-fi/siw917y-rb4343a-wi-fi-6-bluetooth-le-8mb-flash-radio-board-for-module?tab=overview)]
- SiWx917 AC1 Module Explorer Kit [BRD2708A](https://www.silabs.com/development-tools/wireless/wi-fi/siw917y-ek2708a-explorer-kit)

### Software Requirements

- Simplicity Studio
- Serial console setup
  - For serial console setup instructions, see the [Console Input and Output](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#console-input-and-output) section in the *WiSeConnect Developer's Guide*.

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

   ![Figure: i2s_lowpower_uc_screen](resources/uc_screen/i2s_lowpower_uc_screen.png)

- Open the **sl_si91x_ulp_i2s.slcp** project file, select the **Software Component** tab, and search for **I2S** in the search bar.
- Using the configuration wizard, you configure different parameters like:

  - **General Configuration**

    - SL_ULP_I2S_RESOLUTION: ULP_I2S resolution can be configured through this macro. Valid resolution values are 16-, 24-, and 32-bit.
    - SL_ULP_I2S_SAMPLING_RATE: ULP_I2S sampling rate can be configured through this macro. Valid sampling rate values are 8kHz, 11.025kHz, 16kHz, 22.05kHz, 24kHz, 32kHz, 44.1kHz, 48kHz, 88.2kHz, 96kHz, and 192kHz.
    - Configuration files are generated in **config folder**. If not changed, the code will run on default UC values.
    - Configure the following macros in the [`ulp_i2s_example.c`](https://github.com/SiliconLabs/wiseconnect/blob/v4.0.1-content-for-docs/examples/si91x_soc/peripheral/sl_si91x_ulp_i2s/ulp_i2s_example.c) file and update/modify following macros, if required.

      ```C
      #define I2S_LOWPOWER_BUFFER_SIZE 1024    ///< Transmit/Receive buffer size
      ```

  - **Pin Configuration**

      | Description      | SiWx917 GPIO | WPK [BRD4002A](https://www.silabs.com/development-tools/wireless/wireless-pro-kit-mainboard)           | Explorer Kit [BRD2708A](https://www.silabs.com/development-tools/wireless/wi-fi/siw917y-ek2708a-explorer-kit)         |
      | ---------------- | ------------ | ------------------------ | ------------------------------- |
      | I2S DOUT         | ULP_GPIO_1   | P16                      | EXP_HEADER-5                    |
      | I2S DIN          | ULP_GPIO_6   | EXP_HEADER-16            | RX                              |

### Pin Description

**Note!** Make sure pin configuration is set in `RTE_Device_917.h` file. (path: /$project/config/RTE_Device_917.h)

> **Note**: For recommended settings, please refer the [recommendations guide](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-prog-recommended-settings/).

## Test the Application

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

1. Connect DOUT to DIN for loopback connection.
2. Compile and run the application.
3. When the application runs, it sends and receives data in loopback.
4. Data send/receive success and data comparison success prints can be seen on serial console.
5. After successful program execution, the prints in serial console looks as shown below.

   >![Figure: output](resources/readme/output.png)

>**Note:**
>
>- The required files for low-power state are moved to RAM and the rest of the application is executed from flash.
>- In this application, we are changing the power state from PS4 to PS2 and vice - versa.
>
> **Note:**
>
> - Interrupt handlers are implemented in the driver layer, and user callbacks are provided for custom code. If you want to write your own interrupt handler instead of using the default one, make the driver interrupt handler a weak handler. Then, copy the necessary code from the driver handler to your custom interrupt handler.
>
> **Note:**
>
>- This application is intended for demonstration purposes only to showcase the ULP peripheral functionality. It should not be used as a reference for real-time use case project development, because the wireless shutdown scenario is not supported in the current SDK.
