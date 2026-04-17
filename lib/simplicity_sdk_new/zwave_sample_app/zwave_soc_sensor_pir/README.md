# Sensor PIR

The Z-Wave certified Sensor PIR application shows a presence/movement detector implementation for controlling other devices and for sending notifications.

| <!-- -->                  | <!-- -->                                       |
|---------------------------|------------------------------------------------|
| Role Type                 | Reporting Sleeping End device (RSS)            |
| Supporting Device Type    | Data reporting                                 |
| Device Type               | Notification sensor                            |
| Generic Type              | Sensor Notification                            |
| Specific Type             | Notification Sensor                            |
| Requested security keys   | S0, S2\_UNAUTHENTICATED, and S2\_AUTHENTICATED |

Sensor PIR transmits the following notification types/events:

-   Home Security
    -   Motion detection (unknown location)

Sensor PIR supports the "push mode" only of Notification CC.

## Supported Command Classes

Sensor PIR implements mandatory and some optional command classes. The table below lists the supported Command Classes, their version, and their required Security class, if any.

| Command Class             | Version | Required Security Class        |
| ------------------------- |:-------:| ------------------------------ |
| Association               |    2    | Highest granted Security Class | 
| Association Group Info    |    3    | Highest granted Security Class |
| Battery                   |    1    | Highest granted Security Class |
| Device Reset Locally      |    1    | Highest granted Security Class |
| Firmware Update Meta Data |    5    | Highest granted Security Class |
| Indicator                 |    3    | Highest granted Security Class |
| Manufacturer Specific     |    2    | Highest Granted Security Class |
| Multi-Channel Association |    3    | Highest granted Security Class |
| Notification              |    8    | Highest granted Security Class |
| Powerlevel                |    1    | Highest granted Security Class |
| Security 2                |    1    | None                           |
| Supervision               |    1    | None                           |
| Transport Service         |    2    | None                           |
| Version                   |    3    | Highest granted Security Class |
| Wake Up                   |    2    | Highest granted Security Class |
| Z-Wave Plus Info          |    2    | None                           |

## Basic Command Class mapping

Basic Command Class is not mapped to any of the supported command classes

## Association Group configuration

Application Association Group configuration

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
            <li>Battery: triggered upon low battery.</li>
            <li>
                Notification: triggered upon movement detection (simulated by
                Medium Press of BTN1). After a while, a cancel notification will be issued.
            </li>
            <li>Indicator Report: Triggered when LED0 changes state.</li>
        </ul>
    </td>
</tr><tr>
    <td>2</td>
    <td>Basic Set</td>
    <td>Y</td>
    <td>
        Upon a movement detection (simulated by Medium Press of BTN1), nodes
        associated in this group will first receive a Basic Set with 0xFF (turn on)
        and after a while receive a Basic Set with 0x00 (turn off).
    </td>
</tr>
</table>

X: For Z-Wave node count is equal to 5 and for Z-Wave Long Range it is 1.
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
    <td>Sends Battery Report</td>
</tr><tr>
    <td>Medium Press</td>
    <td>Simulates motion detection for 10 seconds. Notifications are generated when motion is detected and when the sensor returns to idle.</td>
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
    <th>Description</th>
</tr><tr>
    <td>LED0</td>
    <td>
        Blinks with 1 Hz when learn mode is active.<br>
        Used for Indicator Command Class.
    </td>
</tr>
</table>


## Firmware Update

This section will describe backward compatibility when upgrading the Switch On/Off application from one SDK to a newer version.


## CLI Support

In case CLI support is needed, please install zw_cli_common component to the project. Please note that the zw_cli_common component will significantly increase the power consumption of this application, as it must always be in an active state in order to receive CLI commands. The CLI cannot function during EM4 sleep mode. After a reset, the application stays awake for 10 seconds. During this timeframe, the sleeping disable CLI command can be issued to keep the application awake.

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
    <th>battery_report</th>
    <td>-</td>
    <td>Sending a battery report</td>
</tr>
<tr>
    <th>motion_detected</th>
    <td>-</td>
    <td>Triggering a motion detected event</td>
</tr>
<tr>
    <th>sleeping</th>
    <td>[string] "enable" or "disable"</td>
    <td>Enable or disable sleeping. After pushing the reset button (or resetting with commander) the device will be awake for a given amount of time if the CLI component is added to the project. During this time the user can prevent the sleeping. For more information check the `zw_cli_sleeping` component.</td>
</tr>
</table>