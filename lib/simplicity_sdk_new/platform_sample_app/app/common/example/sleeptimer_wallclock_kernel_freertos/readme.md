# Sleeptimer Wallclock Kernel FreeRTOS

This example application demonstrates the use of the sleeptimer's wall clock interface in a FreeRTOS environment. The application uses the low frequency RTC (Real Time Clock) peripheral to set and get the date and time, with operations accessible over a virtual COM serial port.

All wallclock logic is managed in a FreeRTOS task, allowing other tasks to run concurrently.

Three types of time can be set and get in this application:

* Unix time (Shown as integer)
* Network Protocol Time (Shown as integer)
* Date and time (Shown in YYYY:MM:DD HH:MM:SS format)

## CLI Commands

Help		- Prints list of supported commands
get_unix_time 	- Prints unix time
set_unix_time 	- Can be used to set unix time (Ex: set_unix_time 17)
get_ntp_time	- Prints ntp time
set_ntp_time	- Can be used to set ntp time (Ex: set_ntp_time 2208988825)
get_datetime	- Prints date and time
set_datetime	- Can be used to set date and time (Ex: set_datetime YYYY-MM-DD HH:MM:SS)

## Requirements

- Silicon Labs board with RTC peripheral
- FreeRTOS kernel enabled

## Resources
* [AN0014: EFM32 Timers](https://www.silabs.com/documents/public/application-notes/AN0014.pdf)
* [FreeRTOS Kernel Documentation](https://www.freertos.org/)
