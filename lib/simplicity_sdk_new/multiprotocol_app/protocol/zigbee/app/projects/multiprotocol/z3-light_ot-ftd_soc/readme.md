# CMP Zigbee+OpenThread application


This application demonstrates Zigbee and OpenThread Concurrent Multiprotocol functionality. It features a router on the Zigbee side and a Full Thread Device (FTD) on the OpenThread side.

At a high-level, the protocol stacks operate per usual with their own RAIL handles. Behind the scenes, they multiplex a common RAIL handle with a single-protocol RAIL library. It is critical to ensure that the Zigbee and OpenThread networks are both on the same channel. Not doing so can cause unpredictable operation.

It is important to appropriately size the heap and stacks used in this application to account for the two wireless protocols.

Protocol tasks are managed using an RTOS. Zigbee and OpenThread RTOS tasks are both set to the same priority. CLI task runs at a lower priority. It is important to note that Zigbee and OpenThread APIs are not thread-safe and therefore, any CLI command handler needs to make sure to NOT call Zigbee and OpenThread APIs directly in order to prevent race conditions and unpredictable operation.

Form and join operations on Zigbee and OpenThread can be accomplished using the available CLI commands. Note that OpenThread CLI is limited to a few commands used to commission the OpenThread device out-of-band. Zigbee CLI is comparable to other Zigbee sample applications like the zigbee_z3_light.

OpenThread CLI is implemented in the file ot_up_cli.c and can be extended using the examples provided.
To get this node to join an existing OpenThread network out-of-band, you will need to use the following CLI commands:

- dataset_networkkey {OpenThreadNetworkKey}
- dataset_commit_active
- ifconfig_up
- thread_start
- thread_state

This application can be extended to add Bluetooth LE. This requires the addition of the following components to the project:
- bluetooth_stack
- gatt_configuration
- bluetooth_feature_legacy_advertiser
- bluetooth_feature_connection
- bluetooth_feature_gatt
- bluetooth_feature_gatt_server
- bluetooth_feature_legacy_scanner
- bluetooth_feature_sm
- bluetooth_feature_system
- zigbee_ble_dmp_cli
