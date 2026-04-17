# Power Strip

The Z-Wave certified Power Strip application shows an extension block implementation used to turn on 
several devices that are connected to power. Examples include lights, appliances, etc.

The Power Strip Z-Wave certified application implements several endpoints, which can be controlled 
individually.

The Power Strip application is based on:

| <!-- -->                 | <!-- -->                                     |
| ------------------------ | -------------------------------------------- |
| Role Type                | Always On End device (AOS)                   |
| Supporting Device Type   | Actuator                                     |
| Device Type              | Binary Switch                                |
| Generic Type             | Switch Binary                                |
| Specific Type            | Not Used                                     |
| Requested security keys  | S0, S2_UNAUTHENTICATED, and S2_AUTHENTICATED |
| Endpoint 1 Device Type   | Binary Switch                                |
| Endpoint 1 Generic Type  | Switch Binary                                |
| Endpoint 1 Specific Type | Not Used                                     |
| Endpoint 2 Device Type   | Multilevel Switch                            |
| Endpoint 2 Generic Type  | Switch Multilevel                            |
| Endpoint 2 Specific Type | Not Used                                     |

Power Strip supports the "push mode" only of Notification CC and transmits the following notification 
types/events:
- Power Management
  - Overload detected (both endpoint 1 and 2)


##  Supported Command Classes

The Power Strip application implements mandatory and some optional command classes. The table 
below lists the supported command classes, their version, and their required Security class, if any.

| Command Class             | Version | Required Security Class        |
| :------------------------ | :-----: | :----------------------------- |
| Association               |    2    | Highest Granted Security Class |
| Association Group Info    |    3    | Highest Granted Security Class |
| Basic                     |    2    | Highest Granted Security Class |
| Device Reset Locally      |    1    | Highest Granted Security Class |
| Firmware Update Meta Data |    5    | Highest Granted Security Class |
| Indicator                 |    3    | Highest Granted Security Class |
| Manufacturer Specific     |    2    | Highest Granted Security Class |
| Multi-Channel             |    4    | Highest Granted Security Class |
| Multi-Channel Association |    3    | Highest Granted Security Class |
| Notification              |    8    | Highest Granted Security Class |
| Powerlevel                |    1    | Highest Granted Security Class |
| Security 0                |    1    | None                           |
| Security 2                |    1    | None                           |
| Supervision               |    1    | None                           |
| Switch Binary             |    2    | Highest Granted Security Class |
| Transport Service         |    2    | None                           |
| Version                   |    3    | Highest Granted Security Class |
| Z-Wave Plus Info          |    2    | None                           |

###  Supported Command Classes: Endpoint 1

Endpoint 1 implements the following command classes.

| Command Class             | Version | Required Security Class        |
| :------------------------ | :-----: | :----------------------------- |
| Association               |    2    | Highest Granted Security Class |
| Association Group Info    |    3    | Highest Granted Security Class |
| Basic                     |    2    | Highest Granted Security Class |
| Multi-Channel Association |    3    | Highest Granted Security Class |
| Notification              |    8    | Highest Granted Security Class |
| Security 0                |    1    | None                           |
| Security 2                |    1    | None                           |
| Supervision               |    1    | None                           |
| Switch Binary             |    2    | Highest Granted Security Class |
| Z-Wave Plus Info          |    2    | None                           |

###  Supported Command Classes: Endpoint 2

Endpoint 2 implements the following command classes.

| Command Class             | Version | Required Security Class        |
| :------------------------ | :-----: | :----------------------------- |
| Association               |    2    | Highest Granted Security Class |
| Association Group Info    |    3    | Highest Granted Security Class |
| Basic                     |    2    | Highest Granted Security Class |
| Multi-Channel Association |    3    | Highest Granted Security Class |
| Notification              |    8    | Highest Granted Security Class |
| Security 0                |    1    | None                           |
| Security 2                |    1    | None                           |
| Supervision               |    1    | None                           |
| Multilevel Switch         |    4    | Highest Granted Security Class |
| Z-Wave Plus Info          |    2    | None                           |

## Basic Command Class mapping

The Basic Command Class is mapped according to the next table.

| Endpoin | Basic Command                          | Mapped Command                             |
| :------ | :------------------------------------- | :----------------------------------------- |
| 1       | Basic Set (Value)                      | Binary Switch Set (Value)                  |
| 1       | Basic Report (Current Value, Duration) | Binary Switch Report (Value, Duration)     |
| 2       | Basic Set (Value)                      | Multilevel Switch Set (Value)              |
| 2       | Basic Report (Current Value, Duration) | Multilevel Switch Report (Value, Duration) |

## Association Groups

The following three tables show the available association groups for Z-Wave and Z-Wave Long 
Range respectively.
Backward compatibility for non-Multi Channel devices forces the root device AGI table to contain all the 
association groups mentioned in each of the endpoints AGI tables except from group 1, the Lifeline 
group.

### Association groups for Root Device:

<table>
<tr>
    <th>ID</th>
    <th>Name</th>
    <th>Node Count</th>
    <th>Description</th>
</tr><tr>
    <td>1</td>
    <td>Lifeline</td>
    <td>X</td>
    <td>
        <p>Supports the following command classes:</p>
        <ul>
            <li>Device Reset Locally: triggered upon reset.</li>
            <li>Binary Switch Report: Triggered when the switch changes state.</li>
            <li>Notification: triggered by the endpoints</li>
            <li>Indicator Report: Triggered when LED0 changes state.</li>
        </ul>
    </td>
</tr><tr>
    <td>2</td>
    <td>alarm EP 1</td>
    <td>Y</td>
    <td>Mirror of endpoint 1, group 2</td>
</tr><tr>
    <td>3</td>
    <td>alarm EP 2</td>
    <td>Y</td>
    <td>Mirror of endpoint 2, group 2</td>
</tr>
</table>

X: For Z-Wave node count is equal to 5 and for Z-Wave Long Range it is 1.

Y: For Z-Wave node count is equal to 5 and for Z-Wave Long Range it is 0.

### Association groups for Endpoint 1:

<table>
<tr>
    <th>ID</th>
    <th>Name</th>
    <th>Node Count</th>
    <th>Description</th>
</tr><tr>
    <td>1</td>
    <td>Lifeline</td>
    <td>0</td>
    <td>
        <p>Supports the following command classes:</p>
        <ul>
            <li>Binary Switch Report: Triggered when the switch changes state.</li>
            <li>Notification: triggered by the endpoints</li>
        </ul>
    </td>
</tr><tr>
    <td>2</td>
    <td>alarm EP 1</td>
    <td>Y</td>
    <td>Notification report on overload.</td>
</tr>
</table>

Y: For Z-Wave node count is equal to 5 and for Z-Wave Long Range it is 0.

### Association groups for Endpoint 2:

<table>
<tr>
    <th>ID</th>
    <th>Name</th>
    <th>Node Count</th>
    <th>Description</th>
</tr><tr>
    <td>1</td>
    <td>Lifeline</td>
    <td>0</td>
    <td>
        <p>Supports the following command classes:</p>
        <ul>
            <li>Multilevel Switch Report: Triggered when the switch changes state.</li>
            <li>Notification: triggered by the endpoints</li>
        </ul>
    </td>
</tr><tr>
    <td>2</td>
    <td>alarm EP 2</td>
    <td>Y</td>
    <td>Notification report on overload.</td>
</tr>
</table>

Y: For Z-Wave node count is equal to 5 and for Z-Wave Long Range it is 0.

## Usage of Buttons and LED Status

We are differentiating four different types of button presses. The following types are the same for the BTN0 and BTN1 on the WSTK board. The duration values can be configured under the config directory in app_button_press_config.h file in each generated application/project.

Please note external wakeup is not supported on button 1 in case of brd2603a and brd2603b.

<table>
<tr>
    <th>Press Type</th>
    <th>Duration</th>
</tr><tr>
    <td>Short Press</td>
    <td>0 - 400 ms</td>
</tr><tr>
    <td>Medium Press</td>
    <td>401 - 1500 ms</td>
</tr><tr>
    <td>Long Press</td>
    <td>1501 - 5000 ms</td>
</tr><tr>
    <td>Very Long Press</td>
    <td>Every press longer than Long Press</td>
</tr>
</table>


<table>
<tr>
    <th>Button</th>
    <th>Action</th>
    <th>Description</th>
</tr><tr>
    <td>RST</td>
    <td>Press</td>
    <td>Resets the firmware of an application (like losing power). All volatile memory will be cleared.</td>
</tr><tr>
    <td rowspan="2">BTN0</td>
    <td>Short Press</td>
    <td>
    <p>
        Toggles the transmission of an "Overload detected" notification.
        The first transmission, when enabled, will always be the first in the following list.
      </p>
      <p>The notifications will be transmitted in the following pattern:</p>
      <ol>
        <li>Notification from endpoint 1: Overload detected</li>
        <li>Notification from endpoint 1: No event</li>
        <li>Notification from endpoint 2: Overload detected</li>
        <li>Notification from endpoint 2: No event</li>
      </ol>
      <p>One notification will be transmitted every 30 seconds.</p>
    </td>
</tr><tr>
    <td>Medium Press</td>
    <td>Toggles the binary switch</td>
</tr><tr>
    <td rowspan="2">BTN1</td>
    <td>Short Press</td>
    <td>Enter "learn mode" (sending node info frame) to add/remove the device.<br>
    Removing the device from a network will reset it.
    </td>
</tr><tr>
    <td>Very Long Press</td>
    <td>Perform a reset to factory default operation of the device, and a Device Reset Locally Notification Command is sent via Lifeline.
    </td>
</tr>
</table>


<table>
<tr>
    <th>LED</th>
    <th>Description for boards with RGB LED</th>
    <th>Description for boards without RGB LED<sup>1</sup></th>
</tr><tr>
    <td>LED0</td>
    <td colspan="2">
      Blinks with 1 Hz when learn mode is active.
      Used for Indicator Command Class.
    </td>
</tr><tr>
    <td>LED1</td>
    <td>Endpoint 1 (switch on/off)</td>
    <td>Endpoint 2 (dimmer, average of the three colors)</td>
</tr><tr>
    <td>RGB</td>
    <td>Endpoint 2 (dimmer)</td>
    <td>-</td>
</tr>
</table>

<sup>1</sup>: Some boards do not have an RGB LED. On such boards, the command line interface can be used to monitor the binary switch endpoint and all the levels of the multilevel switch endpoint.

## Firmware Update

This section will describe backward compatibility when upgrading the Power Strip application from one SDK to a newer version. 

## CLI Support

In case CLI support is needed, please install zw_cli_common component to the project. Please note that the zw_cli_common component will modify the power consumption for every application.

The default baud rate is 9600, which allows the underlying EUSART module to work in EM2. On some development kits (e.g. BRD2603A and BRD2705A), the following command should be applied in Simplicity Studio via the Admin console to adjust the baudrate: `serial vcom config speed 9600`.

### Available CLI commands

<table>
<tr>
    <th>Command</th>
    <th>Arguments</th>
    <th>Description</th>
</tr>
<tr>
    <th>help</th>
    <th>-</th>
    <th>Printing all supported CLI commands and their description.</th>
</tr>
<tr>
    <th>set_learn_mode</th>
    <td>-</td>
    <td>Toggling the learn mode functionality. In case the learn mode started, but inclusion does not happen,learn mode will automatically stop after about 30 seconds</td>
</tr>
<tr>
    <th>factory_reset</th>
    <td>-</td>
    <td>Executing factory reset</td>
</tr>
<tr>
    <th>get_dsk</th>
    <td>-</td>
    <td>Printing out the generated DSK of the device</td>
</tr>
<tr>
    <th>get_region</th>
    <td>-</td>
    <td>Printing out the set region of the application</td>
</tr>
<tr>
    <th>toggle_endpoint</th>
    <td>Number of your endpoint you want to toggle. Can be 1 or 2</td>
    <td>Toggling endpoint 1 or endpoint 2</td>
</tr>
<tr>
    <th>dim_endpoint</th>
    <td>New dimming level, it can be minimum 0 and maximum 99</td>
    <td>Setting the dimmin for endpoint 2</td>
</tr>
<tr>
    <th>toggle_notification_sending</th>
    <td>-</td>
    <td>Toggles the transmission of an "Overload detected" notification.
        The first transmission, when enabled, will always be the first in the following list.
    </td>
</tr>
<tr>
    <th>get_led_state</th>
    <td>-</td>
    <td>Get the state of the LED1. Can be "on" or "off".
    <br>Only available on hardware with an RGB LED (otherwise, LED1 is assigned
    to the multilevel switch and its state can be queried with get_rgb_values).</td>
</tr>
<tr>
    <th>get_rgb_values</th>
    <td>-</td>
    <td>Shows the current Red Green and Blue values. The range is 0-65535 for each component. In case the target does not have RGB LED it shows the LED intensity on a scale 0- 255.</td>
</tr>
</table>
