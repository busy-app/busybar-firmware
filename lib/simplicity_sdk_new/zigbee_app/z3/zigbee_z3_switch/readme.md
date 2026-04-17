# Zigbee 3.0 Gateway, Light and Switch


Zigbee 3.0 provides a foundation of commissioning and network management mechanisms to be used in all Zigbee applications. The sample scenario presented here demonstrates the flexibility that the Zigbee 3.0 specification provides to applications. It also acts as an excellent starting point for users wishing to build their own Zigbee 3.0 applications.

These applications can take on three possible roles.
1. zigbee_z3_gateway, the gateway can form a centralized network, and the light and the switch can join the centralized network by performing network steering.
2. zigbee_z3_light, acting as a router, can form a distributed network, and the zigbee_z3_switch, acting as an end device, can join the distributed network.
3. zigbee_z3_light, acting as a Touchlink target, can Touchlink with the switch, acting as a Touchlink initiator.

The gateway provides a CLI command application interface to the user. The CLI command "plugin network-creator start 1" can be issued to form a centralized network.   
The gateway application can then be triggered to allow other devices onto the network with the CLI command  "plugin network-creator-security open-network". Devices can then join the network using the ZigBeeAlliance09 link key, or by manually entering the install code-derived link key into the gateway using the CLI command "plugin network-creator-security set-joining-link-key".  
The CLI command "plugin network-creator-security close-network" will close the network and no longer allow devices onto the gateway's network.

The light provides a very simplistic user interface. On power up, the light will perform network steering automatically. If the light does not find a suitable network to join, it forms its own distributed network. The light will not open its network for joining as soon as the network comes up, so this must be done manually. Once a distributed network is formed and opened, the switch may join its network by performing network steering itself or through Touchlink commissioning.  Once the light is on a network, it will set its COMMISSIONING_STATUS_LED. When the light starts identifying as a find-and-bind target, it will blink its COMMISSIONING_STATUS_LED. The light will start identifying as a find-and-bind target automatically when it brings its network up, but users can reopen the find-and-bind target window by pushing either button on the light at any time when it is on a network.

The switch provides a simple application interface to the user. When the switch is not on a network, it can initiate network steering to join a network using button 0. The switch can also initiate Touchlink commissioning using button 1. After the switch has successfully joined a network, it will perform the finding and binding procedure for an initiator. After this takes place, the switch should have at least two bindings to the light in its binding table: one for the On/Off cluster and one for the Level Control cluster. While the switch is performing its network commissioning, it will blink its COMMISSIONING_STATUS_LED. When the switch is active on the network, it will set its COMMISSIONING_STATUS_LED. Once the switch has finished finding and binding, users can use buttons 0 and 1 to send On/Off Toggle and Level Control Move to Level commands to the light, respectively.

This application also demonstrates functionality of a Green Power proxy application.

The current debug printing settings in these applications are only for the purpose of aiding users in understanding and debugging this sample scenario. Debug printing should be turned off in order to save code size on a final product.
