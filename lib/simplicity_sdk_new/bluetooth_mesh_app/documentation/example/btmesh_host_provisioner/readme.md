# Bluetooth Mesh - Host Provisioner Example

The Host Provisioner example demonstrates using an NCP node connected to a PC as a provisioner.
Through this node the user can provision, configure and reset other nodes.
The BT Mesh network is created and handled by the NCP node, so network management options are also available.

## Components Required

1. **btmesh_host_provisioner** host example
   - Example host application for demonstrating provisioner capabilities.

2. **btmesh_ncp_empty** target sample app
   - NCP target sample app on the provisioner node.
   - **Note:** EFR32xG22 boards are not supported

## Prerequisites

- A POSIX/Mac, MSYS2, or MinGW64 platform is required

## Setup Steps

1. **NCP target**
   - The btmesh_ncp_empty target sample app must be programmed to the EFR32 chip:
     - Connect your WSTK to the PC
     - Open Simplicity Studio (with Bt Mesh SDK installed)
     - Select the btmesh_ncp_empty target sample app from Demos to flash your EFR32 device

2. **btmesh_host_provisioner**
   - Open a new terminal (MinGW64 terminal on Windows)
   - Navigate to `example_host/btmesh_host_provisioner` in Bt Mesh SDK
   - Generate the sample app using slc:
     - `slc generate` with `--with <OS>` is required. Available OS options are `macos`, `linux`, or `win32`.
     - The following additional build parameters are supported via `--with`:
       - `btmesh_host_app_prov_ui` - adds UI mode on top of the CLI application
       - `btmesh_host_app_remote_prov` - adds remote provisioning support
       - `btmesh_host_app_remote_prov_ui` - adds remote provisioning support with UI
       - `btmesh_host_app_prov_cbp` - adds certificate-based provisioning support
       - `btmesh_host_app_prov_oob` - adds out-of-band provisioning support
     - It is possible to combine components, i.e., `--with btmesh_host_app_prov_ui,btmesh_host_app_prov_cbp` generates CBP support with UI mode.
     - Full example command:
       ```bash
       slc generate -p btmesh_host_provisioner.slcp -d out --with macos,btmesh_host_app_prov_cbp,btmesh_host_app_remote_prov
       ```
   - Build the sample app:
     - Navigate to the generation folder
     - Run: `make -f btmesh_host_provisioner.Makefile -j`
     - **Note:** It is possible to *generate* a win32 target from anywhere. The build itself has to be done from MSYS2 or MinGW64.
   - Run the sample app with appropriate parameters:
     ```bash
     ./exe/btmesh_host_provisioner.exe -u /dev/ttyS4 --scan
     ```
     - **1st parameter:** `-u` selects connection to a UART serial port. Use `-t` for TCP/IP connection.
       - **Note:** On Windows, MinGW64 uses `COMx`, POSIX systems require `/dev/ttyX`
     - **2nd parameter:** `--scan` selects the scanning function to check if there are unprovisioned beaconing nodes available
     - If CBP support is added, UI mode will automatically use CBP to provision all capable nodes.
     - If command line mode is used, an extra `--cbp` parameter indicates that CBP shall be used. If this extra parameter is omitted, a normal provisioning procedure will follow even if the host is built with CBP enabled.

## Usage

The Host Provisioner example has two modes: CLI and UI.

### 1. UI Mode

The UI mode is accessible by starting the program without any Host Provisioner-related arguments, e.g.:

```bash
btmesh_host_provisioner.exe -u COM5
```

The user can choose from several commands in one session without exiting. In this case, the host example's database stores information about the nodes in the BT Mesh network and those found during scanning.
This database is updated while the program is running, but it is not guaranteed that it will be the same in the next run.

On Windows using MinGW64, `winpty` might be required for user input handling. In this case, just run the program as:

```bash
winpty exe/btmesh_host_provisioner.exe -u COM5
```

### 2. CLI Mode

The CLI mode is one command per run, selected by the appropriate parameter.
In this scenario, the host database is not preserved between runs, but node identifiers (e.g., UUID) stay the same provided the affected node has not been reset (with btmesh_node_reset or an NVM erase) in the meantime. This means that, e.g., a UUID found in a scanning session can be used in a provisioning session.

The following options are available for CLI mode:

- `--scan` - Scan for unprovisioned beaconing devices
- `--provision <UUID>` - Provision and configure the selected device
- `--nodelist` - List all nodes known to the provisioner (i.e., present in the provisioner's device database (DDB))
- `--nodeinfo <UUID>` - Print DCD information about the selected device
- `--remove <UUID>` - Unprovision the selected device
- `--key-refresh <timeout>` - Refresh the network key and app key
- `--reset` - Reset the provisioner to its factory state. It is recommended to remove all known devices before resetting
- `--help` - Print a help message about usage