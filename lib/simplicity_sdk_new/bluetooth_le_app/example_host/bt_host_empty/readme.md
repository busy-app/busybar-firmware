# Host empty


## Features

- Demonstration of the NCP host application
- Starts advertising, and waiting for connection
- Contains a GATT table that can be edited by the GATT editor


## Requirements

- A target device running the Bluetooth - NCP example

## Usage

- Generate and build the example

### Typical invocation and response

```bash
./out/build/debug/bt_host_empty -u /dev/tty.usbmodem0004403482281
```

For complete command line option list, use the -h option.

### Output

If the NCP target is found, the application outputs the following.
This example shows a successful connection opened to a remote device.

```
[I] NCP host initialised.
[I] Press Crtl+C to quit

[I] Rebooting NCP target (0)...
[I] Bluetooth stack booted: v11.0.0+915ab9a7
[I] Bluetooth public device address: 54:DC:E9:1D:3E:5E
[I] Started advertising.
[I] Connection opened.
```

## Resources

https://www.silabs.com/documents/public/application-notes/an1259-bt-ncp-mode-sdk-v3x.pdf
