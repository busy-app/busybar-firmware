# Wall Controller

The Z-Wave certified Wall Controller application shows a push-button switch panel implementation to control devices in the Z-Wave network from push buttons (physical or virtual) on a device that is meant to be mounted on a wall. Examples include scene and zone controller and wall-mounted AV controllers.

The Wall controller application implements a wall-mounted switch that has three individually configurable buttons. Each button has support for on/off and dimming.

The Wall Controller application is based on:

| <!-- -->                | <!-- -->                                                                                                          |
| ----------------------- | ----------------------------------------------------------------------------------------------------------------- |
| Role Type               | Always On End device (AOS)                                                                                        |
| Supporting Device Type  | Actuator                                                                                                          |
| Device Type             | Central Scene                                                                                                     |
| Generic Type            | Wall Controller                                                                                                   |
| Specific Type           | Not Used                                                                                                          |
| Requested security keys | S0, S2_UNAUTHENTICATED, and S2_AUTHENTICATED<br>The Wall Controller features do not depend on the security level. |

## Supported Command Classes

| Command Class             | Version | Required Security Class        |
| ------------------------- |:-------:| ------------------------------ |
| Association               |    2    | Highest granted Security Class |
| Association Group Info    |    3    | Highest granted Security Class |
| Central Scene             |    3    | Highest granted Security Class |
| Device Reset Locally      |    1    | Highest granted Security Class |
| Firmware Update Meta Data |    5    | Highest granted Security Class |
| Indicator                 |    3    | Highest granted Security Class |
| Manufacturer Specific     |    2    | Highest Granted Security Class |
| Multi-Channel Association |    3    | Highest granted Security Class |
| Powerlevel                |    1    | Highest granted Security Class |
| Security 0                |    1    | None                           |
| Security 2                |    1    | None                           |
| Supervision               |    1    | None                           |
| Transport Service         |    2    | None                           |
| Version                   |    3    | Highest granted Security Class |
| Z-Wave Plus Info          |    2    | None                           |

Use case:

The wall controller's three buttons are configurable to control nodes. Each button action is sent over the Lifeline as scene actions.

## Basic Command Class mapping

Basic Command Class is not mapped to any of the supported command classes.

## Association Groups

The following association groups are available for Z-Wave and Z-Wave Long Range respectively.

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
            <li>Central Scene: triggered by button press.</li>
            <li>Indicator Report: Triggered when LED0 changes state.</li>
        </ul>
    </td>
</tr><tr>
    <td>2</td>
    <td rowspan="2">KEY1</td>
    <td>Y</td>
    <td>Basic Set Command</td>
</tr><tr>
    <td>3</td>
    <td>Y</td>
    <td>Switch Multilevel CC</td>
</tr><tr>
    <td>4</td>
    <td rowspan="2">KEY2</td>
    <td>Y</td>
    <td>Basic Set Command</td>
</tr><tr>
    <td>5</td>
    <td>Y</td>
    <td>Switch Multilevel CC</td>
</tr><tr>
    <td>6</td>
    <td rowspan="2">KEY3</td>
    <td>Y</td>
    <td>Basic Set Command</td>
</tr><tr>
    <td>7</td>
    <td>Y</td>
    <td>Switch Multilevel CC</td>
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
    <td rowspan="3">BTN0</td>
    <td>Short Press</td>
    <td>Triggers association group 2: Basic Set</td>
</tr><tr>
    <td>Hold longer than Short Press</td>
    <td>Association group 3: Multilevel Switch Start Level Change</td>
</tr><tr>
    <td>Released after being held longer than Short Press</td>
    <td>Association group 3: Multilevel Switch Stop Level Change</td>
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
        Blinks with 1 Hz when learn mode is active.
        Used for Indicator Command Class.
    </td>
</tr>
</table>


## Firmware Update

This section will describe backward compatibility when upgrading the Wall Controller application from one SDK to a newer version.

Firmware update from previous versions should work without any additional steps.

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
    <th>send_central_scene_key</th>
    <td>
    First argument is the number of the key, which could be 1, 2 or 3. Second argument is used to determine the action of the specified key, which can be press, hold or release.
    </td>
    <td>Triggering central scene key</td>
</tr>
</table>
