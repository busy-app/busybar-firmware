# Sleeptimer FreeRTOS

This example application demonstrates the use of timers in an RTOS-based environment using FreeRTOS. The application uses the low-frequency RTC (Real Time Clock) peripheral to create periodic and one-shot timers. The on-board buttons control the timers' stop and start operations. Status is shown on the virtual COM serial port console.

## Application Overview

The application creates **three timers**:
- **Periodic timer**: Toggles LED0 when its timeout occurs.
- **One-shot timer**: Toggles LED1 once when its timeout occurs.
- **Status timer**: Prints the remaining time of the periodic & one-shot timers every second.

When the application starts (after flashing), both the periodic and one-shot timers are started automatically.

- **Button0** is used to start/stop the periodic timer.
- **Button1** is used to start/stop the one-shot timer.
- The status timer continuously prints the status of the other two timers whenever its timeout occurs. It cannot be stopped.

## Requirements

- Silicon Labs board with **2 buttons** and **2 LEDs**.

## Resources
- [AN0014: EFM32 Timers](https://www.silabs.com/documents/public/application-notes/an0014-efm32-timers.pdf)
