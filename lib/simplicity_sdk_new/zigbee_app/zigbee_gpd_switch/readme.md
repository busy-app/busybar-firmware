# Green Power Device Switch Application


This sample application demonstrates a green power switch device with following capability that is pre-configured in plugin GPD App Configuration.
- Device = GPD On/Off Switch (Device Id = 2).
- Application Id = 0 , 4 Byte Device Src Address.
- Fixed Location = True.
- Commissioning = Bidirectional with Key Request and Key Protected.
- Security Level = 3.
- Security Key type used = OOB Individual (Key Type 4).


This sample application maps its functionality with configured buttons as described here:
- BRD4183A Where there is one button available on the boards and is enabled as PB0:
  - PB0 - Commissioning, Toggle.
  - PB0 Long press for 5 seconds: Decommissioning.
- BRD4182A Where there are two buttons available on the boards and both are enabled as PB0 and PB1:  
  - PB0 - Commissioning, Decommissioning (together with PB1 pressed).
  - PB1 - Toggle.

GPD State Indication when the led plugin is enabled as follows:
- BRD4183A Where there is one LED available on the boards and is enabled as LED0:
  - LED0 Activity and Commissioning state - ON indicates not commissioned, OFF indicates Commissioned. Blinks for a button press or serial command. The ON time is the measurement active time to complete the command.
- BRD4182A Where there are two buttons available on the boards and both are enabled as PB0 and PB1:
  - LED1 Commissioning state - ON indicates GPD is not commissioned, OFF indicates GPD is commissioned.
  - LED0 Activity - Blinks for a button press or serial command. The ON time is the measurement active time to complete the command.

There are debug print available in the callback file to print the GPD state and activities on serial debug terminal using serial and ember-minimal-print plugins.

- **Commissioning the GPD with the Z3LightCombo is as follows**: 
1. Enter the Z3LightCombo into commissioning state ( follow descriptions in the  Z3LightCombo sample application to do so).
2. Then follow the series of button presses on the GPD:
    Keep clicking (press and release) the PB0 (commissioning button) leaving a time of more than 1 second in between until the Commissioning state indicator switches off indicating the GPD is commissioned.  
    Following series of commands will be sent out:  
      - I. Channel request: To find the channel of the Zigbee network and use that for commissioning requests.  
      - II. Commissioning command: Commissioning request.
      - III. Commissioning command: To collect the commissioning reply.
      - IV.  Commissioning command: To indicate the successful commissioning.
3. Then press the Toggle button on GPD to see the Z3LightCombo toggle on/off.
4. To decommission the the GPD and bring it to factory reset state press the PB0 (and PB1) button(s) to invoke decommissioning action.

The GPD supports a set of CLI to help testing. This CLI can be turned off from this .isc file. The sample application by default becomes a non-sleepy application when CLI enabled. Some of the useful CLIs are here:
  - **help**: Prints the command table.
  - **node info**: Prints the information.
  - **node comm <1/0>**: 1 - Sends a single comm command based on the gpd state, 0 - sends a decommissioning command.
  - **node comm 255**: This commands starts a self-triggering timer based state machine to complete all the states of commissioning.
  - **switch toggle**: Sends a toggle command.
  - **switch sleep**: enters sleepy behaviour, the CLI will stop working in this mode.

## Note: 
  1. In this sample application the RX window is kept as 80 milliseconds to accommodate the commissioning reply delay caused by debug prints in Z3LightCombo sample application. In specification it should be 8-10 milliseconds.
  2. This sample application needs user to check and configure (as needed) buttons and LEDs.
  3. For accurate power measurement, switch off the LEDs, switch off debug prints, CLI and power down on-board SPI flash.

