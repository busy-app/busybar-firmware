# Zigbee 4.0 Light and Switch


The Zigbee 4.0 light and switch applications build upon the existing Zigbee 3.0 light and switch applications. These 4.0 applications demonstrate the features as defined in the BDB 3.1 and Zigbee PRO Core revision 23 specifications.

The light acts as a dual role coordinator/router, which can either form a Zigbee 4.0 network or join (as a router) a Zigbee 4.0 network. When acting as a trust center, the light will perform discovery of all devices that join the network and will bind to the Poll Control cluster for any end devices that join the network. If the light joins a network, it will perform a trust center connectivity check every so often using the Trust Center Keepalive component. Upon TC connectivity loss, it will employ the BDB 3.1 rejoin algorithm to restore network connectivity.

The switch acts as a Zigbee 4.0 joining device, which also performs TC connectivity and acts with the rejoin algorithm if connectivity with the trust center is lost.

Both light and switch applications leverage the features in the R23 version of the PRO Core stack, such as Network Commissioning and Dynamic Link Key, as well as routines defined in the BDB 3.1 specification. DLK will automatically kick off when these Zigbee 4.0 devices join a network or have devices join their network.

Both Zigbee 4.0 applications are backwards compatible with existing Zigbee 3.0 networks. Zigbee 3.0 devices are also capable of joining Zigbee 4.0 networks created by these sample applications.
