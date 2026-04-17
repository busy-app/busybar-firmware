# USB CDC ACM FreeRTOS Application
This example project demonstrates use of the USB stack CDC ACM class with FreeRTOS.
The application will create a COM port on your PC. 
Once connected on the port using a terminal tool, a menu is displayed as shown below and lets you echo any character you typed in.
```c
===== USB CDC ACM Serial Emulation Demo ======

1. Echo 1 demo.
2. Echo N demo.
3. Echo N asynchronously demo.

Option:
```
Choose any one of the 3 options priovided for the demo.

**Note!** 
1.  For the option 2 and option 3 from the serial terminal, you can send up to 512 bytes at once to the USB device.
2.  If the data payload is an exact multiple of the max packet size of Bulk OUT (at full speed, max packet size is 64 bytes for Bulk transfers as per USB 2.0 spec), the demo will block forever. This is due to the serial terminal behavior which does not send automatically a zero length packet (ZLP). This is a limitation of the demo.
