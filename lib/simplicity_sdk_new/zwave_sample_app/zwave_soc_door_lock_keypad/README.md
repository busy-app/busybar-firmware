# Door Lock

The Z-Wave certified Door Lock application shows a lock implementation. It supports user codes to open a door and thereby eliminate the need for traditional keys. It is possible to both lock and unlock the door remotely through the Z-Wave protocol.

The Door Lock application is based on:

| <!-- -->                | <!-- -->                                                                |
| ----------------------- | ----------------------------------------------------------------------- |
| Role Type               | Listening Sleeping End device (LSS / FLiRS)                             |
| Supporting Device Type  | Actuator                                                                |
| Device Type             | Lock                                                                    |
| Generic Type            | Entry Control                                                           |
| Specific Type           | Door Lock                                                               |
| Requested security keys | S0 and S2_ACCESS<br>The Door Lock features depend on the security level |

The application will only work when securely added to a network by a controller supporting security.
The controller MUST support security S2_Access_Control or S0 in order to be able to control the door lock.

**Not** implemented Door lock functionality:

- Timed Operation mode
- Door Lock condition
- Inside Door Handle State
- Functionality handling Lock timeout
- Target mode
- Auto-Relock, Hold And Release, Block to Block, and Twist Assist

## Supported Command Classes

The Door Lock application implements mandatory and some optional command classes. The table below lists the supported Command Classes, their version, and their required Security class, if any.

| Command Class             | Version | Required Security Class |
| ------------------------- |:-------:| ----------------------- |
| Association               | 2       | S0 or Access Control    |
| Association Group Info    | 3       | S0 or Access Control    |
| Basic                     | 2       | S0 or Access Control    |
| Battery                   | 1       | S0 or Access Control    |
| Device Reset Locally      | 1       | S0 or Access Control    |
| Door Lock                 | 4       | S0 or Access Control    |
| Firmware Update Meta Data | 5       | S0 or Access Control    |
| Indicator                 | 3       | S0 or Access Control    |
| Manufacturer Specific     | 2       | S0 or Access Control    |
| Multi-Channel Association | 3       | S0 or Access Control    |
| Powerlevel                | 1       | S0 or Access Control    |
| Security 0                | 1       | None                    |
| Security 2                | 1       | None                    |
| Supervision               | 1       | None                    |
| Transport Service         | 2       | None                    |
| User Code                 | 1       | S0 or Access Control    |
| User Credential\*         | 1       | S0 or Access Control    |
| Version                   | 3       | S0 or Access Control    |
| Z-Wave Plus Info          | 2       | None                    |

\*: The User Credential Command Class is only available on specific boards. See the section [User Credential Command Class](#user-credential-command-class).

### User Credential Command Class

The User Credential Command Class is enabled by default on the following boards:
 - BRD2705A
 - BRD4400B
 - BRD4400C
 - BRD4401B
 - BRD4401C

Support for this command class can be disabled if the User Code Command Class is sufficient, by disabling the *User Credential Command Class* component.

To enable this command class on other boards, due to flash memory constraints, it is recommended to first disable the *Z-Wave CLI Common* component before enabling the *User Credential Command Class* component.

The default credentials are created when the inclusion process finishes.

- If the User Code Command Class is enabled:
    - User Code 1 is created with a value of 3494.
- If the User Credential Command Class is enabled:
    - A Programming User with the unique identifier 1, called "Admin" is created.
    - A PIN Code associated with user 1 is created with a value of 3494.

To operate the door bolt with the default credentials, the end device must first
be included in a Z-Wave network.

## Basic Command Class mapping

The Basic Command Class is mapped according to the table below.


| Basic Command                       | Mapped Command                                     |
| ----------------------------------- | ---------------------------------------------------|
| Basic Set (Value)                   | Door Lock Operation Set (Door Lock Mode)           |
| Basic Report (Current Value = 0x00) | Door Lock Operation Report (Door Lock Mode = 0x00) |
| Basic Report (Current Value = 0xFF) | Door Lock Operation Report (Door Lock Mode = 0xFF) |

## Association Groups

The table below shows the available association groups, including supported command classes for Z-Wave and Z-Wave Long Range respectively.

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
            <li>Battery: Triggered upon low battery.</li>
            <li>Door Lock: Triggered upon a change in door lock configuration.</li>
            <li>Door Lock: Triggered upon a change in door lock operation.</li>
            <li>Indicator Report: Triggered when LED0 changes state.</li>
            <li>User Code: Triggered when a user code record is modified.</li>
            <li>User Credential*: Triggered when a Duress User enters a Credential.</li>
        </ul>
    </td>
</tr>
</table>

X: For Z-Wave node count is equal to 5 and for Z-Wave Long Range it is 1.\
\*: The User Credential Command Class is only available on specific boards. See the section [User Credential Command Class](#user-credential-command-class).


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
    <td>Simulates entering a PIN Code on a keypad via the User Credential API.<br>
        The entered PIN Code is hard-coded with the value of 3494.<br>
        A valid Credential entry (i.e., button press) toggles the Door Lock Mode between Secured and Unsecured,
        with a delay of 2 seconds, to simulate timed hardware operation.<br>
        If Credential data for User UID 1, Slot 1 is changed to something other than the default PIN Code, the Door Lock Mode can no longer be toggled by pressing this button.
    </td>
</tr><tr>
    <td>Medium Press</td>
    <td>Sends Battery Report</td>
</tr><tr>
    <td>Long Press</td>
    <td>Changes the outside door handle mode to active if it was inactive and vice versa.</td>
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
    <th>Functionality</th>
    <th>Description</th>
</tr><tr>
    <td>LED0</td>
    <td>Indicator:</td>
    <td>
        Blinks with 1 Hz when learn mode is active.<br>
        Used for Indicator Command Class.
    </td>
</tr><tr>
    <td>LED1</td>
    <td>Bolt:</td>
    <td>
        <ul>
            <li>Led off -> bolt unlocked</li>
            <li>Led on -> bolt locked</li>
        </ul>
    </td>
</tr>
</table>

## Firmware Update

This section will describe backward compatibility when upgrading the Door Lock application from one SDK to a newer version. 

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
    <td>Toggling the learn mode functionality. In case the learn mode started, but inclusion does not happen, learn mode will automatically stop after about 30 seconds</td>
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
    <th>enter_user_code</th>
    <td>Your current user code. It must be between 4 and 10 digits.</td>
    <td>Entering a minimum 4 digit long user code</td>
</tr>
<tr>
    <th>set_new_user_code</th>
    <td>Your new user code. It must be between 4 and 10 digits.</td>
    <td>Setting a new user code wich's lenght must be between 4 and 10 digits</td>
</tr>
<tr>
    <th>set_doorhandle_state</th>
    <td>State of the door handle as string. Possible values are deactivate and activate</td>
    <td>Updating the doorhandle state to locked or unlocked</td>
</tr>
<tr>
    <th>get_doorhandle_state</th>
    <td>-</td>
    <td>Shows the state of the door handle. Can be "pressed" or "released"</td>
</tr>
<tr>
    <th>get_doorbolt_state</th>
    <td>-</td>
    <td>Shows the state of the door bolt. Can be "unlocked" or "locked"</td>
</tr>
<tr>
    <th>get_doorlatch_state</th>
    <td>-</td>
    <td>Shows the state of the door latch. Can be "closed" or "open"</td>
</tr>
</table>

### User Credential Related CLI commands

The following commands exist only if User Credential Command class is available. See the section [User Credential Command Class](#user-credential-command-class).

<table>
<tr>
    <th>Command</th>
    <th>Arguments</th>
    <th>Description</th>
</tr>
<tr>
    <th>u3c_add_user</th>
    <td><ul><li>[string] Name of the user</li></ul>
    </td>
    <td>This command will create a new user in U3C database. The user's UUID will be autogenerated.</td>
</tr>
<tr>
    <th>u3c_add_credential</th>
    <td>
        <ul>
            <li>[uint16] Associated User UID</li>
            <li>[string] Type of the new credential. Can be "pin" or "password"</li>
            <li>[uint16] Slot number of the new credential</li>
            <li>[string] Credential data. E.g. 3494</li>
        </ul>
    </td>
    <td>This command will create a new credential and assign it to an existing user.</td>
</tr>
<tr>
    <th>u3c_modify_user</th>
    <td>
        <ul>
            <li>[uint16] User UID to modify</li>
            <li>[string] Attribute to modify. Can be "type", "name" or "active"</li>
            <li>[string] New value of the attribute</li>
        </ul>
    </td>
    <td>This command modifies an attribute of an existing user</td>
</tr>
<tr>
    <th>u3c_modify_credential</th>
    <td>
        <ul>
            <li>[string] Type of the credential. Can be "pin" or "password"</li>
            <li>[uint16] Slot of the credential</li>
            <li>[string] Attribute to change. Can be "data", "uuid" or "slot"</li>
            <li>[string] New value of the attribute</li>
        </ul>
    </td>
    <td>This command modifies an existing credential.</td>
</tr>
<tr>
    <th>u3c_delete_user</th>
    <td>
        <ul>
            <li>[uint16] User UID of an existing user to delete</li>
        </ul>
    </td>
    <td>This command deletes an existing user and all of the assigned credentials.</td>
</tr>
<tr>
    <th>u3c_delete_credential</th>
    <td>
        <ul>
            <li>[string] Type of the credential. Can be "pin" or "password"</li>
            <li>[uint16] Slot of the credential</li>
        </ul>
    </td>
    <td>This command deletes an existing credential from the database.</td>
</tr>
<tr>
    <th>u3c_get_user</th>
    <td>
        <ul>
            <li>[uint16] User UID of an existing user</li>
        </ul>
    </td>
    <td>This command prints the information of an existing user.</td>
</tr>
<tr>
    <th>u3c_get_credential</th>
    <td>
        <ul>
            <li>[string] Type of the credential. Can be "pin" or "password"</li>
            <li>[uint16] Slot of the credential</li>
        </ul>
    </td>
    <td>This command prints the information of an existing credential.</td>
</tr>
<tr>
    <th>u3c_enter_credential</th>
    <td>
        <ul>
            <li>[string] Type of the credential. Can be "pin" or "password"</li>
            <li>[string] Credential data</li>
        </ul>
    </td>
    <td>This command enters an existing credential and validates it.</td>
</tr>
<tr>
    <th>u3c_credential_learn_start</th>
    <td>
        <ul>
            <li>[uint16] Learn target UUID</li>
            <li>[string] Type of the credential. Can be "pin" or "password"</li>
            <li>[uint16] Learn target slot</li>
        </ul>
    </td>
    <td>Initiates the Credential Learn process locally.</td>
</tr>
<tr>
    <th>u3c_credential_learn</th>
    <td>
        <ul>
            <li>[string] Credential data. E.g. 3494</li>
        </ul>
    </td>
    <td>Enter credential data for User Credential Learn process</td>
</tr>
</table>