# Wi-Fi Coex - Wi-Fi Client BLE Throughput App

## Table of Contents

- [Wi-Fi Coex - Wi-Fi Client BLE Throughput App](#wi-fi-coex---wi-fi-client-ble-throughput-app)
  - [Table of Contents](#table-of-contents)
  - [Purpose/Scope](#purposescope)
  - [Prerequisites / Setup Requirements](#prerequisites--setup-requirements)
    - [Hardware Requirements](#hardware-requirements)
    - [Software Requirements](#software-requirements)
    - [Setup Diagram](#setup-diagram)
  - [Getting Started](#getting-started)
  - [Application Build Environment](#application-build-environment)
    - [Common Steps](#common-steps)
      - [WLAN throughputs: UDP/TCP/TLS unidirectional](#wlan-throughputs-udptcptls-unidirectional)
  - [Test the Application](#test-the-application)

## Purpose/Scope

The coex application demonstrates throughput measurement of Wi-Fi while BLE is in connection.

The coex application has WLAN and BLE tasks and acts as an interface between a smartphone and a PC. A smartphone interacts with the BLE task, while both PC and Silicon Labs device would be connected to a Wireless Access Point, thus both are connected together wirelessly when a smartphone connects to Silicon Labs device. Data transfer will happen between the Station and AP.

## Prerequisites / Setup Requirements

### Hardware Requirements

- Windows PC with Host interface (UART / SPI).
- SiWx91x Wi-Fi Evaluation Kit. The SiWx91x supports multiple operating modes. See [Operating Modes]() for details.
- **SoC Mode**:
  - Standalone
    - BRD4002A Wireless pro kit mainboard [SI-MB4002A]
    - Radio Boards
      - BRD4338A [SiWx917-RB4338A]
      - BRD4339B [SiWx917-RB4339B]
      - BRD4340A [SiWx917-RB4340A]
      - BRD4343A [SiWx917-RB4343A]
  - Kits
    - SiWx917 Pro Kit [Si917-PK6031A](https://www.silabs.com/development-tools/wireless/wi-fi/siwx917-pro-kit?tab=overview)
    - SiWx917 Pro Kit [Si917-PK6032A]
    - SiWx917 AC1 Module Explorer Kit (BRD2708A)
  	
- **NCP Mode**:
  - Standalone
    - BRD4002A Wireless pro kit mainboard [SI-MB4002A]
    - EFR32xG24 Wireless 2.4 GHz +10 dBm Radio Board [xG24-RB4186C](https://www.silabs.com/development-tools/wireless/xg24-rb4186c-efr32xg24-wireless-gecko-radio-board?tab=overview)
    - NCP Expansion Kit with NCP Radio boards
      - (BRD4346A + BRD8045A) [SiWx917-EB4346A]
      - (BRD4357A + BRD8045A) [SiWx917-EB4357A]
  - Kits
    - EFR32xG24 Pro Kit +10 dBm [xG24-PK6009A](https://www.silabs.com/development-tools/wireless/efr32xg24-pro-kit-10-dbm?tab=overview)
  - Interface and Host MCU Supported
    - SPI - EFR32

- Wireless Access Point
- Smart phone/tablet with BLE Application (Ex: Light Blue / BLE Connect App)
- Windows PC with iPerf and openssl applications.

### Software Requirements

- Simplicity Studio
- Download and install iPerf from this link: [iPerf Application](https://sourceforge.net/projects/iperf2/files/iperf-2.0.8-win.zip/download). iPerf is a tool for active measurements of the maximum achievable bandwidth on IP networks. It supports tuning of various parameters related to timing, buffers and protocols (TCP and UDP with IPv4 and IPv6).
- Download and install OpenSSL from this link: [OpenSSL](http://ufpr.dl.sourceforge.net/project/gnuwin32/openssl/0.9.8h-1/openssl-0.9.8h-1-bin.zip) to download openssl in remote PC.
- Download and install the Silicon Labs [Simplicity Connect App(formerly EFR Connect App) or other BLE Central/Peripheral app.](https://www.silabs.com/developers/simplicity-connect-mobile-app ) in the android smart phones for testing BLE applications. Users can also use their choice of BLE apps available in Android/iOS smart phones.

### Setup Diagram

![Setup Diagram for WLAN Throughput BT SPP BLE Dual Role](resources/readme/wifi_throughput_ble_throughput_app_soc_ncp.png)

## Getting Started

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

- [Install Simplicity Studio](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#install-simplicity-studio)
- [Install WiSeConnect extension](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#install-the-wi-se-connect-extension)
- [Connect your device to the computer](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#connect-si-wx91x-to-computer)
- [Upgrade your connectivity firmware ](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#update-si-wx91x-connectivity-firmware)
- [Create a Studio project ](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#create-a-project)

For details on the project folder structure, see the [WiSeConnect Examples](https://docs.silabs.com/wiseconnect/latest/wiseconnect-examples/#example-folder-structure) page.

## Application Build Environment

The application can be configured to suit your requirements and development environment. Read through the following sections and make any changes needed.

1. Open `wifi_app_config.h` file and edit the following parameters:


    - **Wi-Fi Configuration**

      Configure the following parameters to enable your Silicon Labs Wi-Fi device to connect to your Wi-Fi network.

      ```c
      #define SSID           "SILABS_AP"      // Wi-Fi Network Name
      #define PSK            "1234567890"     // Wi-Fi Password
      #define SECURITY_TYPE  SL_WIFI_WPA2     // Wi-Fi Security Type: SL_WIFI_OPEN / SL_WIFI_WPA / RSI_WPA2
      ```

    - **Client/Server IP Settings**

      ```c
      #define DEVICE_PORT        <local_port>   // Local port to use
      #define SERVER_PORT        <remote_port>  // Remote server port
      #define SERVER_IP_ADDRESS  "192.168.0.100"    // Remote server IP address
      #define BUF_SIZE           1400               //! Memory length for send buffer
      #define SOCKET_ASYNC_FEATURE 1                // Type of Socket used. Synchronous = 0, Asynchronous = 1
      ```

    - **Throughput Measurement Types**

      The application may be configured to measure throughput using UDP, TCP, TLS packets. Choose the measurement type using the `THROUGHPUT_TYPE` macro.

      ```c
      #define THROUGHPUT_TYPE  TCP_TX     // Selects the throughput option; see the following diagrams. 
      #define TCP_TX           0          // SiWx91x transmits packets to remote TCP client
      #define TCP_RX           1          // SiWx91x receives packets from remote TCP server
      #define UDP_TX           2          // SiWx91x transmits packets to remote UDP client
      #define UDP_RX           3          // SiWx91x receives packets from remote UDP server
      #define TLS_TX           4          // SiWx91x transmits packets to remote TLS client
      #define TLS_RX           5          // SiWx91x receives packets from remote TLS server
      ```

2. To Load certificate to device flash. (The Certificate could be loaded once and need not be loaded for every boot up.)

   ```c
   #define  LOAD_CERTIFICATE                         0x01
   ```

3. Open `ble_app.c` file and update/modify following macros:

   - `RSI_BLE_CHAR_SERV_UUID` refers to the attribute type of the characteristics to be added in a service.

      ```c
      #define  RSI_BLE_CHAR_SERV_UUID                         0x2803
      ```

   - `RSI_BLE_CLIENT_CHAR_UUID` refers to the attribute type of the client characteristics descriptor to be added in a service.

     ```c
     #define RSI_BLE_CLIENT_CHAR_UUID                        0x2902
     ```

   - `RSI_BLE_NEW_SERVICE_UUID` refers to the attribute value of the newly created service.

      ```c
      #define  RSI_BLE_NEW_SERVICE_UUID                       0xAABB
      ```

   - `RSI_BLE_ATTRIBUTE_1_UUID` refers to the attribute type of the first attribute under this service (RSI_BLE_NEW_SERVICE_UUID).

      ```c
      #define  RSI_BLE_ATTRIBUTE_1_UUID                        0x1AA1
      ```

   - `RSI_BLE_ATTRIBUTE_2_UUID` refers to the attribute type of the second attribute under this service (RSI_BLE_NEW_SERVICE_UUID).

      ```c
      #define RSI_BLE_ATTRIBUTE_2_UUID                         0x1BB1
      ```

   - `RSI_BLE_MAX_DATA_LEN` refers to the Maximum length of the attribute data.

      ```c
      #define RSI_BLE_MAX_DATA_LEN                             20
      ```

   - `RSI_BLE_APP_DEVICE_NAME` refers to the name of the SiWx91x EVK to appear during scanning by remote devices.

      ```c
      #define  RSI_BLE_APP_DEVICE_NAME                         "WLAN_BLE_APP"
      ```

   The following are the **non-configurable** macros in the application.

   - `RSI_BLE_ATT_PROPERTY_READ` is used to set the READ property to an attribute value.

      ```c
      #define  RSI_BLE_ATT_PROPERTY_READ                       0x02
      ```

   - `RSI_BLE_ATT_PROPERTY_WRITE` is used to set the WRITE property to an attribute value.

      ```c
      #define RSI_BLE_ATT_PROPERTY_WRITE                       0x08
      ```

   - `RSI_BLE_ATT_PROPERTY_NOTIFY` is used to set the NOTIFY property to an attribute value.

      ```c
      #define  RSI_BLE_ATT_PROPERTY_NOTIFY                     0x10
      ```

### Common Steps

#### WLAN throughputs: UDP/TCP/TLS unidirectional

1. Compile the project and flash the binary onto STM32

2. To measure **WLAN throughput**, run the below iPerf commands or tls scripts.

3. To measure **UDP Tx** throughput, configure module as UDP client and open UDP server in remote port using following command. To establish UDP Server on remote PC, open [iPerf Application](https://sourceforge.net/projects/iperf2/files/iperf-2.0.8-win.zip/download) and run the below command from the installed folder's path in the command prompt.

    ```sh
    iperf.exe -s -u -p <SERVER_PORT> -i 1
    ```

   Example: iperf.exe -s -u -p 5001 -i 1 
    ![](resources/readme/remote_screen4.png)

4. To measure **UDP Rx** througput, configure module as UDP server and open UDP client in remote port using below command

    ```sh
    iperf.exe -c <Module_IP> -u -p <DEVICE_PORT> -i 1 -b<Bandwidth> -t <duration in sec>
    ```
  
    Example: iperf.exe -c 192.168.0.1 -u -p 5001 -i 1 -b50M -t 100
  
    ![](resources/readme/remote_screen5.png)

5. To measure **TCP Tx** throughput, configure module as TCP client and open TCP server in remote port using below command. To establish TCP Server on remote PC, open [iPerf Application](https://sourceforge.net/projects/iperf2/files/iperf-2.0.8-win.zip/download) and run the below command from the installed folder's path in the command prompt.

    ```sh
    iperf.exe -s -p <SERVER_PORT> -i 1
    ```
  
   Example: iperf.exe -s -p 5001 -i 1

   ![](resources/readme/remote_screen6.png)

6. To measure **TCP Rx** througput, configure module as TCP server and open TCP client in remote port using the following command:

    ```sh
            iperf.exe -c <Module_IP> -p <DEVICE_PORT> -i 1 -t <duration in sec>
    ```

   Example: iperf.exe -c 192.168.0.1 -p 5001 -i 1 -t 100

    ![](resources/readme/remote_screen7.png)

> **Note:** The client must set the SNI extension when connecting to an AWS server using TLS 1.3 version.

7. To measure **TLS Tx** throughput, configure module in TLS client and follow below steps to run TLS server in windows

   - Copy SSL_Server_throughput_d.py from release/resources/scripts/ to release/resources/certificates/

   - Open command prompt in folder release/resources/certificates/ and run below command

      ```sh
              python SSL_Server_throughput_d.py
      ```

    ![](resources/readme/remote_screen8.png)

8. To measure **TLS Rx** throughput, configure module in TLS client and follow below steps to run TLS server in windows

   - Copy SSL_tx_throughput.py from **release/resources/scripts/** to **release/resources/certificate**.

   - Change the port number from "5001" to the value configured in "TLS_RX_SERVER_PORT".

   - Open the command prompt in folder release/resources/certificates/ and run the following command:

      > **Note:**
      > Python version: 3.9.0

      ```sh
          python SSL_tx_throughput.py
      ```

      ![](resources/readme/remote_screen9.png)

> **Note**: For recommended settings, please refer the [recommendations guide](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-prog-recommended-settings/).

## Test the Application

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

1. Build the application.
2. Flash, run and debug the application

3. Observe the output prints on serial terminal  

   ![](resources/readme/output1.png)