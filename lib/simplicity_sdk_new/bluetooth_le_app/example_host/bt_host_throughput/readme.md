# Throughput test

This host example demonstrates how to measure Bluetooth LE throughput between the NCP target *EFR* device and a remote *EFR* device.
It is designed to help evaluate data transfer rates and optimize Bluetooth LE communication.

## Features

- Supports bidirectional throughput measurement
- Adjustable packet size, connection interval, and PHY type
- Selectable test modes: notification or indication
- Configurable MTU and data length
- Real-time throughput and statistics display
- Flexible connection options: TCP/IP, UART, or AF socket
- Customizable logging and configuration file support


## Limitations, known issues

The host application supports only the central role.


## Requirements

- An EFR MCU flashed with Bluetooth NCP example with bluetooth_feature_power_control feature enabled
- Host platform running Linux, Windows, or macOS
- Compatible Bluetooth LE device running Bluetooth - SoC Throughput example
- libcjson host library with headers installed by the user


## Usage

1. Build and flash a Bluetooth - SoC Throughput example or demo onto your Bluetooth LE device.
    Refer to the bt_soc_throughput documentation for details.
2. Generate a Bluetooth - NCP application.
    - Add the bluetooth_feature_power_control.
    - Build and flash to the target.
3. Connect the NCP target to your host.
4. Start the host application.
5. If no time or data length was specified, press Button 0
    on the board with the Bluetooth - SoC Throughput application.
6. For complete option list, use the -h option for help.

### Example invocation

```bash
bt_host_throughput -u /dev/tty.usbmodem0004403482281
```

This will create a connection to the Bluetooth - SoC Throughput application. To start a measurement,
press button 0 on the target. The result will be printed on the screen.



## Output

The script displays:
- Current throughput (bps)
- Total bytes sent/received
- Test duration
- Status

```
__________________
|ROLE: CENTRAL   |
|ST: Subscribed  |
|TX:  +10 dBm    |
|RSSI:  -42 dBm  |
|PHY: 1M         |
|INTERVAL: 0040  |
|PDU: 251        |
|MTU: 247        |
|DATA: 244       |
|NOTIFY: Yes     |
|INDICATE: Yes   |
|TH: 0024400 bps |
|CNT: 000000017  |
__________________
```


## Resources

- [Bluetooth LE SDK Documentation](https://docs.silabs.com/bluetooth/latest/)
- [Using the Silicon Labs Bluetooth® Stack v3.x and Higher in Network
Co-Processor Mode](https://www.silabs.com/documents/public/application-notes/an1259-bt-ncp-mode-sdk-v3x.pdf)
- [Bluetooth Host Development](https://docs.silabs.com/bluetooth/latest/bluetooth-network-coprocessor-mode/03-ncp-host-development)
