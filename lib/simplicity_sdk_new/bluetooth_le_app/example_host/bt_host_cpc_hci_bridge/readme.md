# CPC HCI Bridge

## Features

- Creates a bridge between the CPC daemon and the Bluetooth stack of the host.


## Requirements

- CPC daemon
- Target device with Bluetooth - RCP example loaded
- Linux OS as host platform with BlueZ stack
- Package dependencies
    - bluetooth
    - bluez
    - bluez-tools
    - rfkill
    - libbluetooth-dev

## Limitations

Since CPCd only runs on Linux, this application can only built and run on that platform.

## Usage

- Install the packages
- Build and install CPCd. This is a standard cmake procedure
- Generate and build the Bluetooth - CPC HCI bridge application
- Start CPCd
- Start the Bluetooth - CPC HCI bridge application

Running cpc-hci-bridge connects to CPCd using the standard instance name cpcd_0.


### Example invocation

```bash
bt_host_cpc_hci_bridge
```

opens a CPC endpoint to the BLE RCP running on
the EFR, and creates a numbered virtual serial device on the host, for example /dev/
pts/2. The actual number may vary.

Next, you should attach the Bluetooth stack of the host to the bridge.

```bash
sudo hciattach <device> any
```

After attaching the Bluetooth stack, you can start the Bluetooth CLI utility.

```
sudo bluetoothctl
```

## Resources

- [Multiprotocol Co-Processor](https://www.silabs.com/documents/public/application-notes/an1333-concurrent-protocols-with-802-15-4-rcp.pdf)
- [Using the CPC daemon](https://www.silabs.com/documents/public/application-notes/an1351-using-co-processor-communication_daemon.pdf)
- [CPC daemon repository](https://github.com/SiliconLabs/cpc-daemon)

