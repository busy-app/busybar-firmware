# Zigbee Minimal Host


The Zigbee Minimal Host application provides a minimal functional subset to perform Zigbee 3.0 network creation and to serve as a starting point for users wishing to build their own Zigbee Host applications. The application is configured to operate as a Zigbee Coordinator/Router.

As configured, only network-layer functionality is provided; no Zigbee Cluster Library (ZCL) application-layer functionality is implemented. The application's ZCL ProfileId and DeviceId are set to undefined value 0xFFFF, and no ZCL clusters are enabled. Refer to the other sample applications for examples of how this layer of functionality is configured and enabled.

The current debug printing settings in this application are to aid the user to understand and debug this sample scenario. Debug printing should be turned off in order to save code size on a final product.
