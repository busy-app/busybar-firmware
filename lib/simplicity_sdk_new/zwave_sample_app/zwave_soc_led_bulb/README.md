# LED Bulb

The Z-Wave LED Bulb application shows an implementation of a remotely controlled color 
switch. An example is a LED bulb or any other color capable device.

The LED Bulb application is based on:

| <!-- -->                | <!-- -->                                     |
| ----------------------- | -------------------------------------------- |
| Role Type               | Always On End device (AOS)                   |
| Supporting  Device Type | Actuator                                     |
| Device Type             | Color Switch                                 |
| Generic Type            | Switch Multilevel                            |
| Specific Type           | Color Tunable Multilevel                     |
| Requested security keys | S0, S2_UNAUTHENTICATED, and S2_AUTHENTICATED |

## Supported Command Classes

The LED Bulb application implements mandatory and some optional command classes. The table below 
lists the supported Command Classes, their version, and their required Security class

| Command Class             | Version | Required Security Class        |
| :------------------------ | :-----: | :----------------------------- |
| Association               |    2    | Highest granted Security Class |
| Association Group Info    |    3    | Highest granted Security Class |
| Basic                     |    2    | Highest granted Security Class |
| Color Switch              |    3    | Highest granted Security Class |
| Device Reset Locally      |    1    | Highest granted Security Class |
| Firmware Update Meta Data |    5    | Highest granted Security Class |
| Indicator                 |    3    | Highest granted Security Class |
| Manufacturer Specific     |    2    | Highest Granted Security Class |
| Multi-Channel Association |    3    | Highest granted Security Class |
| Multilevel Switch         |    4    | Highest granted Security Class |
| Powerlevel                |    1    | Highest granted Security Class |
| Security 0                |    1    | None                           |
| Security 2                |    1    | None                           |
| Supervision               |    1    | None                           |
| Transport Service         |    2    | None                           |
| Version                   |    3    | Highest granted Security Class |
| Z-Wave Plus Info          |    2    | None                           |


## Basic Command Class Mapping

The Basic Command Class is mapped according to the table below.

| Basic Command                          | Mapped Command                             |
| :------------------------------------- | :----------------------------------------- |
| Basic Set (Value)                      | Multilevel Switch Set (Value)              |
| Basic Report (Current Value, Duration) | Multilevel Switch Report (Value, Duration) |

## Association Groups

The following association groups shown in the table below are available for Z-Wave and Z-Wave Long 
Range respectively.

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
            <li>Color Switch Report: Triggered when the switch changes state.</li>
            <li>Multilevel Switch Report: Triggered when the switch changes state.</li>
            <li>Indicator Report: Triggered when LED0 changes state.</li>
        </ul>
    </td>
</tr>
</table>

X: For Z-Wave node count is equal to 5 and for Z-Wave Long Range it is 1.

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
    <th rowspan="2">LED</th>
    <th colspan="2">Description</th>
</tr><tr>
    <th>Boards with an RGB LED</th>
    <th>Boards without an RGB LED</th>
</tr><tr>
    <td>LED0</td>
    <td colspan="2">
        Blinks with 1 Hz when learn mode is active.<br>
        Used for Indicator Command Class.
    </td>
</tr><tr>
    <td>LED1</td>
    <td>-</td>
    <td>
        Controlled by CC Color Switch and CC Multilevel Switch
        (monochrome, brightness only)
    </td>
</tr><tr>
    <td>RGB</td>
    <td>Controlled by CC Color Switch (colors) and CC Multilevel Switch (brightness)</td>
    <td>-</td>
</tr>
</table>

## Firmware Update

SDK 7.14.0 is the first SDK, including LED Bulb. When there are new versions of LED Bulb application, backward compatibility notes will be added in this section.

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
    <th>get_rgb_values</th>
    <td>-</td>
    <td>Shows the current Red Green and Blue values. The range is 0-65535 for each component. In case the target does not have RGB LED it shows the LED intensity on a scale 0-255.</td>
</tr>
<tr>
    <th>toggle_led_bulb</th>
    <td>-</td>
    <td>Toggle "LED Bulb" on/off</td>
</tr>
</table>
