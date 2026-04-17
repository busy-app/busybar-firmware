# Z3 Light GP Combo


Apart from being a Z3 Light, this application demonstrates functionality of a Green Power combo device that has proxy and sink instances together in one application.

A Green Power Combo (GPC) has a device id = 0x66, the Green Power endpoint(242), profile Id = 0xA1E0 in this application configuration. The Green Power Cluster Id = 0x0021 holds both server and client instances. The configuration includes Green Power components for server, client and the common to handle the required combo functionality.

GPD Commissioning:  
The Green Power commissioning is invoked by either a user action locally (CLI command or button press) or by a remote commissioning tool with Green Power ZCL command Sink Commissioning Mode with action Enter. This command specifies if proxies in the network need to participate. If set, then the sink will generate Proxy commissioning Mode with action = Enter, Exit criteria, commissioning communication mode etc. to the proxies in the network. In a combo scenario, the proxy on the combo node will participate. Once the sink and proxy are in commissioning mode, then the GPD starts to send the commission frames and completes the commissioning process. At the end of successful commissioning, the proxy and sink tables are populated.

Commissioning Tool Commissioning:  
A commissioning tool using GpPairingConfiguration command can be used to commission a GPD.

Sink and Proxy Tables:  
Both of these tables are used by Green Power server and client, respectively. These tables are represented as attributes of respective Green Power cluster side, which can be read with the ZCL read attribute command.

GpNotification:  
The proxy table is used by the Green Power client to validate an incoming GPDF and generate a GP Notification based on communication mode. It could happen that the proxy side (client) will generate a GP Notification for the sink (server) on the same node. The GP Notification, when received by the server side, uses sink table to validate the source GPD address and sink destination and forwards.

Command Translation:  
The combo can execute a GPD operational command such as toggle, or accept a report from a GPD when it is paired to a GPD that has sent the GPDF and it has a translation available for the command for a Zigbee Cluster Command and endpoint. The Green Power server component considers both the configured application endpoints 1 and 2 for translation purpose. For example, if the commissioning endpoint = 0xFF in SinkCommissioningModeEnter command it puts both the end points into commissioning, and creates a translation table for both endpoints.

Using this application :
1. Either by the Green Power component CLI or by pushing the button PB0, the SinkCommisisoningModeEnter coammnd puts the combo into commissioning. When using Button press to put the sink in commissioning mode, endpoint 1 is considered for GP commissioning endpoint. That the sink is in commissioning mode is indicated by the LED1 is ON.
2. Use the GPD Switch (GPD DeviceId = 2) sample application to start the commissioning.
3. Once the commissioning is successful, the LED1 will turn off, indicating a successful pairing.
4. Use the GPD operational command to see the LED0 indicate the execution of ON/OFF Cluster command on endpoint 1.

**Note:**   
Bidirectional commissioning process has a rx off sent and a rx window to receive the channel configuration and commissioning reply. These windows are affected by the debug prints. Therefore it is recommended to turn the debug printing off for the Green Power Cluster. The current debug printing settings in these applications are only for the purpose of aiding users in understanding and debugging this sample scenario. Debug printing should be turned off in order to save code size on a final product.
