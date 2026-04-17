# SL SI91X WDT Manager

## Table of Contents

- [SL SI91X WDT Manager](#sl-si91x-wdt-manager)
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
  - [Expected Results](#expected-results)

## Purpose/Scope

This example demonstrates Watchdog Timer (WDT) manager integration with LED blinking functionality. The LED blinks periodically every 1 second for 10 times, then the system intentionally hangs to demonstrate the WDT manager's ability to detect an unresponsive system and automatically reset it.

## Overview

- **LED Blinking**: The application toggles the onboard LED0 every 1 second for 10 times using a sleeptimer.
- **WDT Manager Integration**: The WDT Manager service is automatically initialized and started to provide system reliability.
- **System Hang Simulation**: After 10 LED toggles, the system disables interrupts and enters an infinite loop to simulate a true hang condition.
- **WDT Reset Demonstration**: With interrupts disabled, the WDT timer cannot be kicked and will expire, causing a true WDT reset.
- **Automatic Recovery**: The system will restart and the cycle will repeat, showing continuous WDT protection.

## About Example Code

- [`wdt_manager.c`](wdt_manager.c) - This file contains the main WDT manager functionality with LED toggling using sleeptimer. After 10 LED toggles, it creates a system hang to demonstrate WDT reset functionality.
- [`app.c`](app.c) - Top-level application functions that initialize and process the WDT manager example.
- The WDT manager is integrated through the WDT Manager component, which automatically:
  - Initializes the watchdog timer
  - Starts the watchdog service
  - Monitors application health
  - Provides system reset recovery if needed

## Prerequisites/Setup Requirements

- Windows PC
- Silicon Labs Si917 Evaluation Kit [[BRD4002](https://www.silabs.com/development-tools/wireless/wireless-pro-kit-mainboard?tab=overview) + [BRD4338A](https://www.silabs.com/development-tools/wireless/wi-fi/siwx917-rb4338a-wifi-6-bluetooth-le-soc-radio-board?tab=overview) / [BRD4342A](https://www.silabs.com/development-tools/wireless/wi-fi/siwx91x-rb4342a-wifi-6-bluetooth-le-soc-radio-board?tab=overview) / [BRD4343A](https://www.silabs.com/development-tools/wireless/wi-fi/siw917y-rb4343a-wi-fi-6-bluetooth-le-8mb-flash-radio-board-for-module?tab=overview)]
- SiWx917 AC1 Module Explorer Kit (BRD2708A)

### Hardware Requirements

- Si91x development board (e.g., Si917 development kit)
- USB cable for programming and debugging
- Computer running Simplicity Studio

### Software Requirements

- Simplicity Studio 5.0 or later
- WiseConnect SDK 4.0.0 or later
- WDT Manager component (automatically included)

### Setup Diagram

![Setup Diagram](resources/readme/setupdiagram.png)

## Getting Started

1. **Open Simplicity Studio**: Launch Simplicity Studio and navigate to your workspace.

2. **Import the Example**:
   - Go to **File → Import → Simplicity Studio → Examples**.
   - Select **SL SI91X WDT Manager** from the Si91x examples.
   - Click **Finish** to import the project.

3. **Build and Flash**:
   - Right-click on the project in Project Explorer.
   - Select **Build Project** to compile the code.
   - Connect your development board via USB.
   - Right-click on the project and select **Flash to Device**.

## Application Build Environment

The example uses the following components:

- **sl_main**: Main application framework
- **sleeptimer**: For periodic LED toggling
- **sl_si91x_led_917**: LED driver for Si91x
- **sl_wdt_manager**: WDT manager for system reliability
- **syscalls**: System call interface
- **si91x_memory_default_config**: Default memory configuration

### Configuration Options

- **TOGGLE_DELAY_MS**: LED toggle interval (default: 1000 ms)
- **LED_INSTANCE**: LED instance to use (default: led_led0)

## Test the Application

1. **Power On**: Connect the development board to your computer via USB.

2. **Observe LED**: The onboard LED0 should start blinking every 1 second.

3. **Monitor Debug Output**: Open the console to see debug messages indicating LED toggles.

4. **WDT Manager**: The WDT manager runs automatically in the background, providing system reliability without any additional configuration.

## Expected Results

- **LED Behavior**: LED0 blinks every 1 second for exactly 10 times, then stops.
- **Debug Output**: Console shows initialization messages, LED toggle count, and hang notification.
- **System Hang**: After 10 toggles, system disables interrupts and enters infinite loop.
- **WDT Reset**: WDT timer expires (cannot be kicked) and resets the system.
- **Cycle Repeat**: System restarts and the cycle repeats, demonstrating continuous WDT protection.

### Debug Output Example

```text
WDT Manager Example Started
LED will toggle every 1000 ms
After 10 LED toggles, system will hang and WDT will reset it
WDT manager is active for system reliability
LED toggled - Count: 1
LED toggled - Count: 2
LED toggled - Count: 3
LED toggled - Count: 4
LED toggled - Count: 5
LED toggled - Count: 6
LED toggled - Count: 7
LED toggled - Count: 8
LED toggled - Count: 9
LED toggled - Count: 10
System hanging now - WDT will reset the system!
Watch for system reset in a few seconds...
[System resets here due to WDT]
[Cycle repeats after reset]
```

## Key Features

- **LED Blinking with Count**: Demonstrates basic GPIO/LED control with toggle counting.
- **WDT Manager Integration**: Shows how to integrate WDT manager for system reliability.
- **System Hang Simulation**: Intentionally creates system hang to test WDT functionality.
- **WDT Reset Demonstration**: Shows WDT manager detecting and recovering from system hang.
- **Automatic WDT Management**: No manual WDT configuration required.
- **Continuous Protection**: System automatically restarts and repeats the cycle.
- **Low Power**: Uses sleeptimer for efficient power management.

## Troubleshooting

- **LED Not Blinking**: Check if the board is properly connected and flashed.
- **No Debug Output**: Ensure console is properly configured in Simplicity Studio.
- **System Resets**: If the system resets unexpectedly, check for infinite loops or blocking operations in your code.
- **WDT Manager Issues**: The WDT manager is automatically managed, but ensure no other components are interfering with the watchdog timer.
