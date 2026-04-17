# NCP UART Application with Multi-Rail


NCP UART Application with multi-RAIL library enabled for application-specific Green Power GPDF transmission scheduling.
This network co-processor (NCP) application is an extension to the standard zigbee_ncp_uart_hw sample application with following changes.
Application configuration:
 - Multi-RAIL library enabled instead of single RAIL (in multi-RAIL one handle is used by the Zigbee stack whereas the other is used by the application)
 - multirail-demo plugin enabled (this initializes the additional RAIL handle)
 - GP library with sink and proxy table set to non zero.  
  
This application implements a simple GP TX queue. Its size can be configured by defining EMBER_APPL_GP_BIDIRECTIONAL_TX_QUEUE_SIZE.

The queue works as follows. The host inititializes and submits the outgoing GPDF packets against a GPD address, and the queue holds it. When a GPDF is received (checked in emberPacketHandoffIncoming for MAC data type frames) from a GPD with the rxAfterTx bit set (in its GP NWK Ext FC) the queue is read and a transmission is scheduled using the additional RAIL handle for the rx offset time in the application GP_RX_OFFSET_USEC (i.e., 20000 micro seconds).

This application implements the following custom EZSP commands as the queue interface:  
SL_ZIGBEE_CUSTOM_EZSP_COMMAND_INIT_APP_GP_TX_QUEUE: Initializes and clears the application-specific GP outgoing tx queue.

SL_ZIGBEE_CUSTOM_EZSP_COMMAND_SET_APP_GP_TX_QUEUE: Sets (adds or overwrites) a GPDF frame in the queue for a given GPD.

SL_ZIGBEE_CUSTOM_EZSP_COMMAND_GET_APP_GP_TX_QUEUE: Gets (reads back) the content from the queue for a GPD. This is a test API for sending the raw command out using the additional RAIL handle.

SL_ZIGBEE_CUSTOM_EZSP_COMMAND_SEND_APP_GP_RAW: Sends a raw GP packet on a specific channel and time.
