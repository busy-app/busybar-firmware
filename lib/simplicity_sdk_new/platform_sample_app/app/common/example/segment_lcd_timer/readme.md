# Segment LCD with Timer Functionality Example

## Summary

This project shows how to use the segment LCD peripheral to display
timer functions.

## Connections Required

Connect the board via a micro-USB cable to your PC to flash the example.

## Setup

Clone the repository with this project from GitHub onto your local machine.

From within the Simplicity Studio IDE, select Import -> MCU Project... from the 
Project menu. Click the Browse button and navigate to the local repository 
folder, then to the SimplicityStudio folder, select the .sls file for the 
board, click the Next button twice, and then click Finish.

Build and flash the hex image onto the board. Reset board and observe the
segment LCD displaying 00000. Press Push Button 0 to start the timer.

The push buttons also have the following functionalities:
Regular timer mode (default mode):

1. Push Button 0 -> start/stop the timer
2. Push Button 1 single click -> reset the timer

To configure the Compare mode:

1. Push Button 1 hold for 2 or more seconds -> enter compare mode
2. Push Button 0 -> configure the value of the timer to trigger compare match
3. Push Button 1 -> exit compare mode setup
4. Push Button 0 -> start timer and run the compare mode
5. When compare match happens, the Segment LCD will blink with the current value
6. Press Button 0 or 1 to exit compare mode when compare match happens

## How It Works

The segment LCD needs to be configured to display numerical values correctly
on the selected board. This is taken care of by the LCD driver library.
The LETIMER is configured to interrupt every 1 Hz and update the segment LCD
accordingly. The GPIOs for the push buttons are configured as inputs and will
interrupt once pressed to perform specific operations.

## How to Port to Another Part

Right click on the project and select "Properties" and navigate to "C/C++ 
Build" then "Board/Part/SDK". Select the new board or part to target and apply 
the changes. There may be some dependencies that need to be resolved when 
changing the target architecture. This project can only work out of the box
on the boards listed in the Hardware Required section of this readme.
