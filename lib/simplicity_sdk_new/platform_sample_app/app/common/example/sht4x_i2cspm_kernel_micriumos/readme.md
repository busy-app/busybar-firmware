# I2C SPM Micrium OS Application


This example project shows how to use the I2C Simple Polled Master driver using the SHT4X Relative Humidity and Temperature Sensor in a Micrium OS Task.

In this application, the initial temperature is read from the SHT4x sensor. Upper and lower limits are then set based on the defined TEMPERATURE\_BAND\_C. The program continuously monitors the temperature and responds as follows:

If two LEDs are available:

- When the temperature exceeds the upper limit, LED0 is turned on to indicate high temperature. A message, "Temperature is high", is printed to the VCOM serial console.

- When the temperature falls below the lower limit, LED1 is turned on to indicate low temperature. A message, "Temperature is low", is printed to the VCOM serial console.

If only the console is available:

- Regardless of the LED status, the application will output "Temperature is high" or "Temperature is low" on the VCOM serial console whenever the temperature goes above the upper limit or below the lower limit, respectively.

