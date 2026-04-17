# SL UART

## Table of Contents

- [SL UART](#sl-uart)
  - [Purpose/Scope](#purposescope)
  - [Overview](#overview)
  - [About Example Code](#about-example-code)
  - [Prerequisites/Setup Requirements](#prerequisitessetup-requirements)
    - [Hardware Requirements](#hardware-requirements)
    - [Software Requirements](#software-requirements)
    - [Setup Diagram](#setup-diagram)
  - [Getting Started](#getting-started)
  - [Application Build Environment](#application-build-environment)
  - [Pin Configuration](#pin-configuration)
  - [Flow Control Configuration](#flow-control-configuration)
  - [Test the Application](#test-the-application)
  - [Configuring higher clock](#configuring-higher-clock)

## Purpose/Scope

 This application demonstrates how to configure Universal Asynchronous Receiver-Transmitter (UART). In asynchronous mode, it will send data to the serial console and receives from the serial console.

## Overview

- UART is used in communication through wired medium in asynchronous fashion. It enables the device to communicate using serial protocols.
- This application is configured with following configurations:
  - Tx and Rx enabled
  - Asynchronous mode
  - 8 Bit data transfer
  - Stop bits 1
  - No Parity
  - No Auto Flow control
  - Baud Rate - 115200

## About Example Code

- [`uart_example.c`](https://github.com/SiliconLabs/wiseconnect/blob/v4.0.1-content-for-docs/examples/si91x_soc/peripheral/sl_si91x_uart/uart_example.c) - This example code demonstrates how to configure the UART to send and receive data.
- In this example, first UART gets initialized if it is not initialized already with clock and DMA configurations if DMA is enabled using [sl_si91x_usart_init](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/usart#sl-si91x-usart-init).  
**Note:** If the UART/USART instance is already selected for debug output logs, initialization will return `SL_STATUS_NOT_AVAILABLE`.
- After UART initialization, the UART is configured with the default configurations from UC along with the UART transmit and receive lines using [sl_si91x_usart_set_configuration](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/usart#sl-si91x-usart-set-configuration).
- The register user event callback for send and receive complete notification is set using [sl_si91x_usart_multiple_instance_register_event_callback](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/usart#sl-si91x-usart-multiple-instance-register-event-callback).
- After setting the user event callback, the data send and receive can happen through [sl_si91x_usart_send_data](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/usart#sl-si91x-usart-send-data) and [sl_si91x_usart_receive_data](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/usart#sl-si91x-usart-receive-data) respectively.
- Once the receive data event is triggered, the transmitted and received buffer data is compared to confirm that the received data is the same.

## Prerequisites/Setup Requirements

### Hardware Requirements

- Windows PC
- Silicon Labs Si917 Evaluation Kit [WPK(4002A) + BRD4338A]
- SiWx917 AC1 Module Explorer Kit [BRD2708A](https://www.silabs.com/development-tools/wireless/wi-fi/siw917y-ek2708a-explorer-kit)

### Software Requirements

- Simplicity Studio
- Serial console setup
  - For Serial Console setup instructions, refer to [link name](https://docs.silabs.com/wiseconnect/latest/wiseconnec-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#console-input-and-output)..

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

Configuration of UART at UC (Universal Configuration):

- Configure UC from the slcp component.
- Open the **sl_si91x_uart.slcp** project file, select the **Software Component** tab, and search for **UART** in the search bar.
- You can use the configuration wizard to configure different parameters, as required.

  > ![Figure: Selecting UC](resources/uc_screen/uart_uc.png)

## Pin Configuration

  | UART PINS     |  GPIO   |  917 Breakout pin |  Explorer kit Breakout pin |  UART-TTL cable |
  | ------------- | ------- | ----------------- | ------------------------- |---------------- |
  | UART1_TX_PIN  | GPIO_7  |          P20      |             [SCL]         |    RX pin       |
  | UART1_RX_PIN  | GPIO_6  |          P19      |             [SDA]         |    TX Pin       |

## Flow Control Configuration

- Set the SL_USART_FLOW_CONTROL_TYPE parameter to SL_USART_FLOW_CONTROL_RTS_CTS to enable UART flow control.
- Make sure the following two macros in `RTE_Device_917.h(path: /$project/config/RTE_Device_917.h)` are set to '1' to map RTS and CTS pins to WSTK/WPK Main Board EXP header or breakout pins.

  ```C
  #define RTE_UART1_CTS_PORT_ID    1
  #define RTE_UART1_RTS_PORT_ID    1
  ```

  | USART PINS     | GPIO    |   Breakout pin  | Explorer kit Breakout pin|
  | -------------- | ------- | --------------- | ------------------------ |
  | UART_CTS_PIN   | GPIO_28 |     P31         |           [CS]           |
  | UART_RTS_PIN   | GPIO_27 |     P29         |           [MOSI]         |

## Test the Application

1. Connect the TX pin (GPIO_7) of the board to the RX pin of the UART-TTL cable, and the RX pin (GPIO_6) of the board to the TX pin of the UART-TTL cable.
2. Power on the board and run the application.
3. Use a serial terminal on your PC to receive the transmitted data and send it back to the board.
4. The application will transmit 1024 bytes over UART to the serial port and wait for the user to send back 1024 bytes for data comparison.
5. Observe the UART data transfer in the serial terminal. The application will verify that the received data matches the transmitted data.
6. The output on the serial terminal should resemble the example shown below.

    > ![Figure: expected result](resources/readme/output_console_uart.png)
>
> **Note**:
>
>- Add data_in buffer to watch window for checking receive data.

## Configuring higher clock

- To achieve baud rates exceeding 2 million bps, you need to modify the clock source to INTF PLL CLK in the UC.

> **Note:**
>
> - Interrupt handlers are implemented in the driver layer, and user callbacks are provided for custom code. If you want to write your own interrupt handler instead of using the default one, make the driver interrupt handler a weak handler. Then, copy the necessary code from the driver handler to your custom interrupt handler.
> - By default, Request to Send (RTS) and Clear to Send (CTS) flow control signals are disabled in the UART driver UC, and their corresponding GPIO pins are not assigned in the Pintool. If you enable RTS/CTS in the Driver UC, you must manually configure and assign the appropriate GPIO pins in the Pintool to ensure proper hardware flow control functionality.
