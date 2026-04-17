# AoA locator



## Features

- Measures Azimuth and Elevation angles to AoA tags
- Connection oriented or connection less operation
- Calculates 3D location with multilocator system

## Requirements
- Mosquitto libraries with development files and
- Mosquitto MQTT broker
- CJSON library with development files
- An AoA antenna array board with Bluetooth - AoA NCP application

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

- Generate and compile the host application.
- Configure and run the MQTT broker.
- Connect the target with Bluetooth - AoA NCP application installed
- Edit the configuration file provided in the `config` folder

### Example invocation

```bash
bt_aoa_host_locator -u /dev/tty.usbmodem0004403482281 -c /path/to/config.json
```

If the MQTT broker is not running on your local machine, use the `-m` option
to provide a host and port. See -h for syntax.

## Resources

[Application Development with
Silicon Labs’ RTL Library](https://www.silabs.com/documents/public/application-notes/an1296-application-development-with-rtl-library.pdf)
