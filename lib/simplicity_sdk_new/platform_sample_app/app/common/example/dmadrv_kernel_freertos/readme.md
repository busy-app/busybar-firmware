# FreeRTOS DMADRV Example

This is a FreeRTOS-based implementation of the DMADRV demonstrates how to use DMADRV for DMA transfers in a multitasking environment with FreeRTOS.
This example project shows the use of the command line interface component in a FreeRTOS Task.
## Features
- DMA-based data transfer using DMADRV.
- FreeRTOS tasks and semaphores for synchronization.
- Echo functionality to send back received data.

## Files
- **app.c**: Application entry point that initializes and starts FreeRTOS.
- **app.h**: Header file for app.c.
- **dmadrv_app.c**: Contains the DMA logic and FreeRTOS task for processing.
- **dmadrv_app.h**: Header file for dmadrv_app.c.

## How to Run
1. Build the project with FreeRTOS and DMADRV included.
2. Flash onto a Silicon Labs development board.
3. Use a serial terminal to interact with the application.
