# Host AoA positioning

## Features

- Subscribes to MQTT, and gets AoA data
- Calculates 3D positions from AoA data using the RTL

## Requirements

- At least 3 configured AoA locator
- 3D positions of the locators
- At least one AoA tag
- MQTT broker connectivity
- Mosquitto libraries with development files and
- Mosquitto MQTT broker
- CJSON library with development files

On MacOS, you can install libmosquitto and the mosquitto MQTT broker
by issuing

```bash
brew install mosquitto
```

On Linux, you need several packages.

```bash
apt install libmosquitto-dev mosquitto
```

To install libcjson on macos:

```bash
brew install cjson
```

On linux

```bash
apt install libcjson-dev
```

## Usage

The positions and addresses are defined in a configuration file named
`positioning_config.json`. Edit the file and supply its path using the `-c`
command line option.

- locator_id (string): must match the locator’s BLE address in the form
ble-<ADDRESS_TYPE>-<BLE_ADDRESS> where:
  * <ADDRESS_TYPE> = sr (static random) or pd (public device)
  * <BLE_ADDRESS> = 6‑byte address, no separators, uppercase
Example: ble-pd-842E1431C72A [AN1296 locator IDs]
- coordinates (x, y, z) for each locator in a right‑handed local coordinate system.
- Orientation/rotation (as defined in the example file). [AN1296 locator coords]

### Example invocation

```bash
bt_host_positioning.exe -c ./config/positioning_config.json
```
For complete list of command line arguments, use the `-h` option.

### Output

Upon successful configuration, the application will publish the following format to the MQTT broker.

```
{
  "x": 1.509856,
  "x_stdev": 0.321395,
  "y": 0.329126,
  "y_stdev": 0.309377,
  "z": 1.650664,
  "z_stdev": 0.218991,
  "sequence": 29076
}
```

## Resources

[Application Development with Silicon Labs’ RTL Library](https://www.silabs.com/documents/public/application-notes/an1296-application-development-with-rtl-library.pdf)