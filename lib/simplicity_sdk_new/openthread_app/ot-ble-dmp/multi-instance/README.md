# Introduction

The sample application OpenThread BLE DMP – SoC Free RTOS (multi-instance) is a test application that demonstrates the components that go into building a dynamic multiprotocol (DMP) application with OpenThread multi-instance support. It provides a command line interface (CLI) that allows the user to execute basic OpenThread and Bluetooth commands, while also supporting multiple OpenThread instances running simultaneously. It also demonstrates how the power manager component can be used to allow the device to enter into low power (EM2) mode in between activities.

The term 'dynamic' in DMP refers to the fact that both protocols are operating simultaneously. The radio scheduler takes care of multiplexing the transmitted and received packets over the radio. With multi-instance support, you can run multiple OpenThread networks simultaneously, each with its own state and configuration, while still maintaining full Bluetooth functionality.

This document assumes that you have installed Simplicity Studio 5 (SSv5) and the Simplicity SDK Suite (SiSDK) containing the OpenThread and Bluetooth SDKs, and that you are familiar with SSv5 and configuring, building, and flashing applications. If not, see _QSG170: Silicon Labs OpenThread Quick Start Guide_.

To get started quickly, in the SSv5 Launcher Perspective go to the DEMOS tab. Find the **OpenThread BLE DMP - SoC FreeRTOS (multi-instance)** demo and click RUN. This uploads the application image to your board.

## Hardware Requirements

- An EFR32 chip with at least 768 kB of flash.

## Building the Sample App

To build the ot-ble-dmp-multi-instance sample app from source you must have installed SSv5 and the SiSDK. The GNU ARM toolchain is installed with SSv5. The IAR-EWARM toolchain is not compatible with OpenThread.

1. With your target development hardware connected, open SSv5's File menu and select New > Silicon Labs Project Wizard. The Target, SDK, and Toolchain Selection dialog opens. Your target hardware should be populated. Click NEXT.

2. The Example Project Selection dialog opens. Use the Technology Type and Keyword filters to search for a specific example, in this case ot-ble-dmp-multi-instance. Select it and click NEXT.

Note that, if you do not see the application, your connected hardware may not be compatible. To verify, in the Launcher Perspective's My Products view enter EFR32MGxx and select one of the boards. Go to the Examples tab, filter by Thread technology and verify you can see the app.

3. The Project Configuration dialog opens. Here you can rename your project, change the default project file location, and determine if you will link to or copy project files. Note that if you change any linked resource, it is changed for any other project that references it. Unless you know you want to modify SDK resources, use the default selection. Click FINISH.

The Simplicity IDE opens with the ot-ble-dmp-multi-instance project open in the Project Configurator. You may now build the project. The ot-ble-dmp-multi-instance.s37 image will be located in the **GNU ARM <version>** directory, and may be uploaded to your board using an SSv5 tool such as the flash programmer or Simplicity Commander.

## CLI Commands

Type help at the prompt to see a list of CLI commands. A complete OpenThread CLI reference is available here:

https://github.com/openthread/openthread/blob/master/src/cli/README.md

A quick tutorial on using the CLI to form a two-node OpenThread network and send a ping is available here:

https://github.com/openthread/openthread/tree/master/examples/apps/cli

### Multi-Instance Commands

The ot-ble-dmp-multi-instance app adds multi-instance support with the following commands:

```
> instance
instance
list
get
set <index>
help
Done

> instance list
 Index: 0, Id: 1
*Index: 1, Id: 2
Done

> instance get
Current instance index: 1
Done

> instance set 0
Done
Switching from instance 1
Switched to instance 0
```

### Bluetooth Commands

The ot-ble-dmp-multi-instance app also includes all the Bluetooth commands from the original BLE DMP application. Type "ble" at the prompt to see a list of subcommands:

```
get_address
create_adv_set
set_adv_timing
set_adv_random_address
start_adv
stop_adv
start_discovery
set_conn_timing
conn_open
close_conn
update_conn_timing
conn_role
get_random_address
set_random_address
read_local_identity_address
read_conn_rssi
read_remote_used_features
read_remote_version
conn_tx_power
add_dev_to_whitelist
remove_dev_from_whitelist
clear_whitelist
set_adv_data
set_scan_response_data
```

## Multi-Instance Usage Examples

### Creating Multiple Thread Networks

1. Start with instance 0:

```
> instance set 0
> dataset init new
> dataset commit active
> ifconfig up
> thread start
```

2. Switch to instance 1 and create a different network:

```
> instance set 1
> dataset init new
> dataset commit active
> ifconfig up
> thread start
```

3. List instances to see both running:

```
> instance list
*Index: 0, Id: 1
 Index: 1, Id: 2
```

### Using Bluetooth Alongside Multiple Thread Networks

The Bluetooth functionality works independently of the OpenThread instances:

```
> ble get_address
BLE address: 84:2e:14:31:87:44
Done

> ble start_adv
Advertising started
Done
```

You can switch between OpenThread instances while Bluetooth operations continue in the background.

## Power Management

The application supports the same power management features as the original BLE DMP application. Press BTN0 to toggle between staying awake (EM1) and allowing deep sleep (EM2) modes.

## Implementation Notes

- The application creates 2 OpenThread instances by default (configurable via `OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_NUM`)
- Each instance maintains its own Thread network state, roles, and configurations
- The CLI operates on the currently selected instance
- Bluetooth functionality is shared across all instances
- All instances process their tasklets in the main application loop
- Power management applies to the entire application, not individual instances

For more detailed information about multi-instance OpenThread, refer to the OpenThread documentation and the multi-instance CLI example.
