# SL PWM

## Table of Contents

- [SL PWM](#sl-pwm)
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
    - [Application Configuration Parameters](#application-configuration-parameters)
    - [PWM Pin Configuration](#pwm-pin-configuration)
  - [Test the Application](#test-the-application)

## Purpose/Scope

This application demonstrates the Pulse Width Modulation (PWM) to generate a periodic pulse waveform, which is useful in motor control and power control applications.

## Overview

- Supports up to eight PWM outputs with four duty cycle generators. The output pins are grouped in pairs to facilitate driving the low side and high side of a power half bridge.
- Complementary and Independent output modes are supported.
- Dead time insertion in Complementary mode.
- Manual override option for PWM output pins. Output pin polarity is programmable.
- Supports generation of interrupts for different events.
- Supports two hardware fault input pins.

## About Example Code

- This example demonstrates the generation of periodic pulse waveform with 50% duty cycle and frequency of 25Khz.
- Four macros are present (that is, DEAD_TIME, OVERRIDE, FAULT, and SVT).
- If **DEAD_TIME** is enabled:
  - DEAD_TIME: To perform correct status change of the power switches in the inverter leg, a PWM generator should insert a small amount of time between required switching edges for top and bottom switch. This time is called dead-time.
  - Dead time mode is applied only in complementary mode.
    - Complementary mode: In Complementary PWM mode, PWM waveform output incorporates dead time (anti-short periods) to prevent overlap between the positive and anti phases.
    - Independent Mode: In Independent PWM Output mode, the PWM outputs (PWMxH and PWMxL) are phase shifted relative to each other.
  - Initialize the PWM using [sl_si91x_pwm_init()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-init).
  - Set the configuration using [sl_si91x_pwm_set_configuration()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-set-configuration) API.
  - Set the duty cycle using [sl_si91x_pwm_set_duty_cycle()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-set-duty-cycle) API.
  - Set the base timer mode using [sl_si91x_pwm_set_base_timer_mode()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-set-base-timer-mode) API.
  - Set the base time period control using [sl_si91x_pwm_control_period()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-control-period) API.
  - Register callbacks using [sl_si91x_pwm_register_callback()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-register-callback) API.
  - Start PWM using [sl_si91x_pwm_start()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-start) API.
  - Please refer below for Dead Time Insertion
    - Center Aligned :- Configure DT_COUNTER_A as 0x0
    ![Figure: Center Aligned](resources/readme/Center_Edge_Aligned.png)
    - Lead Edge Aligned :- Configure DT_COUNTER_A as 0x1
    ![Figure: Lead Edge Aligned](resources/readme/Lead_Edge_Aligned.png)
    - Tail Edge Aligned :- Configure DT_COUNTER_A as 0x10
    ![Figure: Tail Edge Aligned](resources/readme/Tail_Edge_Aligned.png)
- If **OVERRIDE** is enabled:
  - OVERRIDE: While overriding PWM outputs, the channel counters continue to run, only the PWM outputs are forced to user-defined values.
  - Initialize the PWM using [sl_si91x_pwm_init()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-init) API.
  - Set the configuration using [sl_si91x_pwm_set_configuration()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-set-configuration) API.
  - Set the base timer mode using [sl_si91x_pwm_set_base_timer_mode()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-set-base-timer-mode) API.
  - Set the duty cycle using [sl_si91x_pwm_set_duty_cycle()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-set-duty-cycle) API.
  - Set the base time period control using [sl_si91x_pwm_control_period()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-control-period) API.
  - Register callbacks using [sl_si91x_pwm_register_callback()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-register-callback) API.
  - Start PWM using [sl_si91x_pwm_start()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-start) API.
- If **FAULT** is enabled:
  - FAULT: There are two fault pins, FAULTxA and FAULTxB, associated with the MCPWM (Motor Control Pulse Width Modulation) module. When asserted, these pins can optionally drive each of the PWM I/O pins to a defined state.
  - Initialize the PWM using [sl_si91x_pwm_init()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-init) API, [sl_Si91x_pwm_fault_init()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-fault-init) API.
  - Set the configuration using [sl_si91x_pwm_set_configuration()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-set-configuration) API.
  - Set the base timer mode using [sl_si91x_pwm_set_base_timer_mode()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-set-base-timer-mode) API.
  - Set the duty cycle using [sl_si91x_pwm_set_duty_cycle()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-set-duty-cycle) API.
  - Set the base time period control using [sl_si91x_pwm_control_period()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-control-period) API.
  - Register callbacks using [sl_si91x_pwm_register_callback()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-register-callback) API.
  - Change the event generated to fault A/B flags.
  - Start PWM using [sl_si91x_pwm_start()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-start) API.
- If the user wants to work without UC configuration, they can use their own macros and structure configuration and pass it directly in the application.
- If **SVT** is enabled:
  - Initialize the PWM using [sl_si91x_pwm_init()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-init) API. Initialize Special Event Trigger based GPIO pin.
  - Set the configuration using [sl_si91x_pwm_set_configuration()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-set-configuration) API.
  - Enable the special event trigger using [sl_si91x_pwm_control_special_event_trigger()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-control-special-event-trigger) API.
  - Set the configuration for special event trigger using [sl_si91x_pwm_trigger_special_event()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-trigger-special-event) API.
  - Start PWM using [sl_si91x_pwm_start()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/pwm#sl-si91x-pwm-start) API.
  
>**Note:**
>
>1. PWM has four channels. The user can handle these channels using instances. Each channel has 2 PWM outputs (PWM_L (PWM output low) and PWM_H(PWM output high)).The usage of L,H depends on how the application uses them. For example, when driving something in PUSH-PULL configuration, PWM_H can drive the high-side switch, whereas PWM_L drives the low-side switch. This is only a reference of how L,H can be used. There might be other scenarios, which are dependent on settings done and how to achieve it.
>2. channel_0, channel_1, channel_2 and channel_3 are the names pre-defined for the PWM channels.
>3. For user-defined instances, you may have to define hardware-specific definitions in the `sl_si91x_pwm_init_channel_0_config.h` file (path: /$project/config/sl_si91x_pwm_init_channel_0_config.h).
>4. The user can directly use APIs in the application by passing appropriate structure members if they do not want to configure from UC.

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

### Application Configuration Parameters

- Configure the following macros in the `pwm_example.c` file and update/modify following macros, if required.

  - Global Parameters

    ```C
      #define EVENT_COUNT       10    // Count of events that can generate interrupt. 
      #define PRESCALE_A        0x100 // PWM Prescale_A value
      #define DEADTIME_A        0x08  // PWM deadtime_A
      #define DT_COUNTER_A      0x00  // Dead time counter A enable
      #define DUTY_CYCLE_UPDATE 0x01  // Enable duty cycle updating bit in register
    ```
  
  - Channel-specific Parameters

    ```C
    #define INTR_EVENT       0x01  /* Set channel-specific interrupts for time match and fault events.
                                      Use bitwise OR to combine flags for multiple channels. */
    ```

    ```C
    #define FAULT_A_ENABLE   0x11  /* Configure fault mode and output polarity per channel.
                                      Combine options with bitwise OR for desired channel settings. */
    ```

    ```C
    #define DT_ENABLE        0x01  /* Enable dead time per channel
                                      Use OR logic to set for multiple channels. */
    ```

- Configure UC from the slcp component.
- Open the **sl_si91x_pwm.slcp** project file, select the **Software Component** tab, and search for **PWM** in the search bar.
- By default, PWM has a channel 0 instance. If the base timer selection is selected as **Only one (0th) for All Channels**, to select any other PWM channel, one should have PWM channel-0 as reference. If separate PWM channels are to be considered, change the base timer selection to **One for Each Channel**.
- You can use the configuration wizard to configure different parameters:

  - **Common Configuration for all PWM Channels**
    - **Base Timer Selection**: The application provides two base timer options: one for each channel individually and one shared across all channels.
      - If you are using only Channel 0, select the **Only one (0th) for All Channels** option and ensure PWM Channel 0 is installed.
      - If you are using any channel other than Channel 0, select the **One for Each Channel** option to configure individual timers per channel.

  - **General Configuration for individual PWM Channel**
    - **Frequency**: Frequency changes from 500Hz to 200Khz. By default 25Khz is considered.
    - **Output Polarity Low**: There are 2 polarities: polarity low and polarity high. The difference can be observed in waveforms when polarity is changed. By default, it is set to polarity high.
    - **Output Polarity High**: There are 2 polarities: polarity low and polarity high. The difference can be observed in waveforms when polarity is changed. By default, it is set to polarity high.
    - **PWM Mode**: There are 2 modes: independent mode and complementary mode. The definitions for these modes are covered in the *About example code*. The difference can be seen in waveform when modes are changed.
    - **Timer Counter**: This is initial base time counter value to set. By default, it is set to 0.
    - **Duty Cycle**: By default, 50% duty cycle is taken. One can vary duty cycle from 0% to 100%.
    - **Base Timer Mode**: There are 6 different modes. By default, free run mode is selected.
    - **Ext Trigger**: In order to enable fault A, fault B, and other external triggers present, an ext trigger parameter is used.

  - **Pin Configuration for individual PWM Channel**
    - The pin configuration for PWM channel can be configured under SL_PWM_CHANNEL0 section.

      ![Figure: UC image](resources/uc_screen/pwm_uc_screen.png)

### PWM Pin Configuration

- PWM channel-0 pin configuration.

  | Description  | GPIO    | 917 Breakout pin |  Explorer kit Breakout pin |
  | ------------ | ------- | ---------------- | -------------------------- |
  | PWM_H        | GPIO_7  |        P20       |            [SCL]           |
  | PWM_L        | GPIO_6  |        P19       |            [SDA]           |

- PWM channel-1 pin configuration.

  | Description   | GPIO    | 917 Breakout pin |
  | ------------- | ------- | ---------------- |
  | PWM_H         | GPIO_9  |     F9           |
  | PWM_L         | GPIO_8  |     F8           |

- PWM channel-2 pin configuration.

  | Description   | GPIO    | 917 Breakout pin |
  | ------------- | ------- | ---------------- |
  | PWM_H         | GPIO_11 |     F13          |
  | PWM_L         | GPIO_10 |     F11          |

- PWM channel-3 pin configuration.

  | Description   | GPIO        | 917 Breakout pin | Explorer kit Breakout pin|
  | ------------- | ----------- | ---------------- | ------------------------ |
  | PWM_H         | ULP_GPIO_7  |EXP_15            |          [TX]            |
  | PWM_L         | ULP_GPIO_6  |EXP_16            |          [RX]            |

>**Note:** Make sure pin configuration are set in the `RTE_Device_917.h` file:
>
> - SiWx917: RTE_Device_917.h (path: /$project/config/RTE_Device_917.h)

## Test the Application

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

1. Compile and run the application.
2. When the application runs, it generates periodic pulse waveform with 50% duty cycle.
3. Connect oscilloscope to GPIO_6(P19) & GPIO_7(P20) and observe the PWM waveform.
4. After successful program execution, the prints in serial console looks as shown below.

   ![Figure:output1](resources/readme/output1.png)

   ![Figure: output2](resources/readme/output2.png)

> **Note:**
>
> - Use [sl_si91x_clock_manager_m4_set_core_clk()](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-services/clock-manager#sl-si91x-clock-manager-m4-set-core-clk) API to set system core clock  to 32MHz to achieve PWM frequency between 500Hz to 2700Hz.
> - Interrupt handlers are implemented in the driver layer, and user callbacks are provided for custom code. If you want to write your own interrupt handler instead of using the default one, make the driver interrupt handler a weak handler. Then, copy the necessary code from the driver handler to your custom interrupt handler.
