# Wi-Fi Coex - Wi-Fi Throughput BLE Dual Role

## Table of Contents

- [Wi-Fi Coex - Wi-Fi Throughput BLE Dual Role](#wi-fi-coex---wi-fi-throughput-ble-dual-role)
  - [Table of Contents](#table-of-contents)
  - [Purpose/Scope](#purposescope)
  - [Prerequisites / Setup Requirements](#prerequisites--setup-requirements)
    - [Hardware Requirements](#hardware-requirements)
    - [Software Requirements](#software-requirements)
    - [Setup Diagram](#setup-diagram)
  - [Getting Started](#getting-started)
  - [Application Build Environment](#application-build-environment)
    - [WLAN Throughput](#wlan-throughput)
  - [BLE throughput](#ble-throughput)
  - [Test the Application](#test-the-application)
    - [Common Steps](#common-steps)
    - [WLAN throughputs: UDP/TCP/SSL unidirectional](#wlan-throughputs-udptcpssl-unidirectional)
    - [BLE throughput](#ble-throughput-1)

## Purpose/Scope

This example demonstrates throughput measurements of WLAN/BLE in individual or with protocol combinations in the SiWx91x module.

The application can measure throughput ranges for TCP/UDP/SSL TX/RX to and from remote servers, while BLE data transfers are in progress. It has provision to control other protocol activities while measuring WLAN/BLE throughputs.

The Application can be configured for the individual protocol throughput measurement as well the combination of protocols throughputs (Wi-Fi, BLE, Wi-Fi+BLE).

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
- Download and install iPerf from this link [iPerf Application](https://sourceforge.net/projects/iperf2/files/iperf-2.0.8-win.zip/download). iPerf is a tool for active measurements of the maximum achievable bandwidth on IP networks. It supports tuning of various parameters related to timing, buffers and protocols (TCP and UDP with IPv4 and IPv6).
- Download and install OpenSSL from this link [OpenSSL](http://ufpr.dl.sourceforge.net/project/gnuwin32/openssl/0.9.8h-1/openssl-0.9.8h-1-bin.zip) to download openssl in remote PC.
- Download and install the Silicon Labs [Simplicity Connect App(formerly EFR Connect App) or other BLE Central/Peripheral app.](https://www.silabs.com/developers/simplicity-connect-mobile-app ) in the android smart phones for testing BLE applications. Users can also use their choice of BLE apps available in Android/iOS smart phones.

### Setup Diagram

  ![Setup Diagram for WLAN Throughput BT SPP BLE Dual Role](resources/readme/image1_soc.png)

## Getting Started

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

- [Install Simplicity Studio](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#install-simplicity-studio)
- [Install WiSeConnect extension](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#install-the-wi-se-connect-extension)
- [Connect your device to the computer](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#connect-si-wx91x-to-computer)
- [Upgrade your connectivity firmware](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#update-si-wx91x-connectivity-firmware)
- [Create a Studio project](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#create-a-project)

For details on the project folder structure, see the [WiSeConnect Examples](https://docs.silabs.com/wiseconnect/latest/wiseconnect-examples/#example-folder-structure) page.

## Application Build Environment

The application can be configured to suit your requirements and development environment. Read through the following sections and make any changes needed.

### WLAN Throughput

- Open `wifi_app_config.h` file and configure below macros.

  This section explains how to configure the application to measure TCP/UDP/SSL unidirectional throughput in alone or with combination of BT/BLE activities.

  - Set below macros to 1 to measure **WLAN** alone throughput

    ```c
    #define RSI_ENABLE_WLAN_TEST    1 //Set this to 0 to disable WLAN
    #define WLAN_THROUGHPUT_TEST    1 //Set this to 0 while measuring BLE throughput
    ```

  - Set below macros to 1 to measure **WLAN** throughput along with specific **BLE** activity.

    ```c
    #define RSI_ENABLE_WLAN_TEST    1 //Set this to 0 to disable WLAN
    #define WLAN_THROUGHPUT_TEST    1 //Set this to 0 while measuring BLE throughput
    #define RSI_ENABLE_BLE_TEST     1 //Set this to 0 to disable BLE
    ```

    > **Note:**
    >
    > While measuring WLAN throughput along with BLE activities, ensure 'BLE_THROUGHPUT_TEST' is set to '0'

  - Set below macros to 1 to measure **WLAN** throughput along with specific **BT** activity.

    ```c
    #define RSI_ENABLE_WLAN_TEST    1 //Set this to 0 to disable WLAN
    #define WLAN_THROUGHPUT_TEST    1 //Set this to 0 while measuring BLE throughput
    #define RSI_ENABLE_BT_TEST      1 //Set this to 0 to disable BT
    ```

  - Set below macros to 1 to measure **WLAN** throughput along with **BT and BLE** activities.

    ```c
    #define RSI_ENABLE_WLAN_TEST    1 //Set this to 0 to disable WLAN
    #define WLAN_THROUGHPUT_TEST    1 //Set this to 0 while measuring BLE throughput
    #define RSI_ENABLE_BLE_TEST     1 //Set this to 0 to disable BLE
    #define RSI_ENABLE_BT_TEST      1 //Set this to 0 to disable BT
    ```

    > **Note:**
    >
    > While measuring WLAN throughput with BLE activities ensure 'BLE_THROUGHPUT_TEST' is set to '0'

- If BLE is enabled, Open `app_common_config.h` and configure specific activity of BLE using below macros.

  - **BLE operations**: Set any one of below macros to choose desired BLE activity

    ```c
    #define BLE_INIT_DONE                       0           //! make it 1 for BLE init only
    #define BLE_CONNECTED                       0           //! make it 1 for BLE connection only
    #define BLE_DATA_TRANSFER_START             1           //! make it 1 for BLE data transfer
    ```

  - By default, BLE_DATA_TRANSFER_START is set to '1', that means WLAN measurement will start after BLE GATT data transfers choose the required **operational mode** of SiWx91x module.

    Valid Configurations are:

    - 0  - WLAN alone mode
    - 13 - WLAN + BLE mode

    > **Note:**
    >
    > By default, opermode is set to WLAN+BLE

- Select WLAN configurations in `wifi_app_config.h` file.

  - Enter the AP Connectivity essentials configs as the value to SSID, SECURITY_TYPE and PSK

    ```c
    #define     SSID                       "SILABS_AP"
    #define     SECURITY_TYPE              RSI_WPA2
    #define     PSK                        "12345678"
    ```

  - Channel no in which device should scan choose

    ```c
    #define     CHANNEL_NO                 0       //! 0 - scan all channels
    ```

  - Port number of remote server

    ```c
    #define     SERVER_PORT                5001
    ```

  - While verifying SSL RX throughput, port number of remote server should be configured below:

    ```c
    #define     SSL_SERVER_PORT         5002
    ```

  - IP address of remote server:

    ```c
    #define     SERVER_IP_ADDRESS          "192.168.0.102"
    ```

  - Port number of module:

    ```c
    #define     DEVICE_PORT                5001
    ```

  - To select the ip, configure below macros:

    ```c
    #define     DHCP_MODE                  1           // 0 enable or disable
    ```

  - Choose the throughput type by configuring below macro:

    ```c
    #define     THROUGHPUT_TYPE            TCP_TX 
    ```

    - Valid Configurations of THROUGHPUT_TYPE:

      - UDP_TX → UDP transmit
      - UDP_RX → UDP receive
      - TCP_TX → TCP transmit
      - TCP_RX → TCP receive
      - SSL_TX → SSL transmit
      - SSL_RX → SSL receive

  - Average time required to measure UDP_TX/TCP_TX throughput

    ```c
    #define     THROUGHPUT_AVG_TIME        60000    //60sec of throughput numbers average
    ```

  - Maximum no. of packets required to measure UDP_RX

    ```c
    #define     MAX_TX_PKTS                10000
    ```

  - Configure below macro to choose throughput measurement type . This option is valid while measuring WLAN alone throughput

    ```c
    #define     CONTINUOUS_THROUGHPUT      0
    ```

  - If **CONTINUOUS_THROUGHPUT** is set to '1', then application measures throughput for every interval of 'THROUGHPUT_AVG_TIME' independent of throughput type. If **CONTINUOUS_THROUGHPUT** is set to '0', then throughput is calculated only once for interval of 'THROUGHPUT_AVG_TIME' in case of UDP/TCP TX , for MAX_TX_PKTS in case of UDP RX/ SSL TX and for default interval provided by server in case of TCP RX/SSL RX.

    > **Note:**
    >
    > 1. By default, 'CONTINUOUS_THROUGHPUT' is set to '0'.
    > 2. If CONTINUOUS_THROUGHPUT is set to '1', then ensure that client/server should run with interval at least greater than THROUGHPUT_AVG_TIME.

- Open `ble_config.h` file

  - BLE Advertise name

    ```c
    #define     RSI_BLE_APP_GATT_TEST       (void *)"SI_COEX_MAX_DEMO" 
    ```

  - Configure BLE advertising interval

    ```c
    #define     RSI_BLE_ADV_INT_MIN         0x06a8 //! 1065ms
    #define     RSI_BLE_ADV_INT_MAX         0x06a8 //! 1065ms
    ```

  - Configure below macros to set connection interval, connection latency and connection supervision timeout

    ```c
    #define     CONN_INTERVAL_C1            1600    // connection interval:2s
    #define     CONN_LATENCY_C1             0       // latency : 0
    #define     CONN_SUPERVISION_TIMEOUT_C1 1600   
    ```

## BLE throughput

This section explains user how to configure the application for measuring BLE Tx/Rx throughput using sniffer in alone or with combination of WLAN/BT.

- Open `app_common_config.h` file

  - Set below macro to 1 to measure **BLE** alone throughput

    ```c
    #define     RSI_ENABLE_BLE_TEST        1 //Set this to 0 to disable BLE
    #define     BLE_THROUGHPUT_TEST        1 //Set this to 0 for normal BLE operation
    ```

  - Set below macros to 1 to measure **BLE** throughput along with specific **WLAN** activity.

    ```c
    #define     RSI_ENABLE_BLE_TEST        1 //Set this to 0 to disable BLE
    #define     BLE_THROUGHPUT_TEST        1 //Set this to 0 for normal BLE operation
    #define     RSI_ENABLE_WLAN_TEST       1 //Set this to 0 to disable WLAN
    ```

    > **Note:**
    > While measuring BLE throughput with WLAN activities, > ensure 'WLAN_THROUGHPUT_TEST' is set to '0'

  - Set below macros to 1 to measure **BLE** throughput.

    ```c
    #define     RSI_ENABLE_BLE_TEST        1 //Set this to 0 to disable BLE
    #define     BLE_THROUGHPUT_TEST        1 //Set this to 0 for normal BLE operation
    ```

  - Set below macros to 1 to measure **BLE** throughput and **WLAN** activities.

    ```c
    #define     RSI_ENABLE_BLE_TEST        1 //Set this to 0 to disable BLE
    #define     BLE_THROUGHPUT_TEST        1 //Set this to 0 for normal BLE operation
    #define     RSI_ENABLE_WLAN_TEST       1 //Set this to 0 to disable WLAN
    ```

    > **Note:**
    > While measuring BLE throughput with WLAN activities ensure 'WLAN_THROUGHPUT_TEST' is set to '0'.

  - If WLAN is enabled, configure specific activity of WLAN using below macros.

  - **WLAN operations:** set any one of below macros to choose desired **BLE** activity

    ```c
    #define     WLAN_SCAN_ONLY             0           //! make it 1 for WLAN scan only
    #define     WLAN_CONNECT_ONLY          0           //! make it 1 for WLAN connection only
    #define     WLAN_DATATRANSFER          1           //! make it 1 for WLAN TCP TX
    ```

- Open `ble_config.h` file

  - BLE Advertise name

    ```c
    #define     RSI_BLE_APP_GATT_TEST       (void *)"SI_COEX_MAX_DEMO"
    ```

  - Configure BLE advertising interval

    ```c
    #define     RSI_BLE_ADV_INT_MIN         0x06a8 //! 1065ms
    #define     RSI_BLE_ADV_INT_MAX         0x06a8 //! 1065ms
    ```

  - Configure below macros to set connection interval, connection latency and connection supervision timeout

    ```c
    #define     CONN_INTERVAL_C1            9   // connection interval:1.125ms
    #define     CONN_LATENCY_C1             0   // latency : 0
    #define     CONN_SUPERVISION_TIMEOUT_C1 1600   
    ```

  - Configure below macros to enable/disable data length extension mode

    ```c
    #define     DLE_ON_C1                   1   // 1- DLE ON, 0- DLE OFF 
    ```

  - Configure BLE data transfer type

    ```c
    #define     RX_NOTIFICATIONS_FROM_C1    0  //! set below macro to receive 'gatt notifications' from remote device
    #define     RX_INDICATIONS_FROM_C1      0  //! set below macro to receive 'gatt indications' from remote device
    #define     TX_NOTIFICATIONS_TO_C1      1  //! set below macro to Transmit 'gatt notifications' to remote device
    #define     TX_WRITES_TO_C1             0  //! set below macro to Transmit 'gatt write with response' to remote device
    #define     TX_WRITES_NO_RESP_TO_C1     0  //! set below macro to Transmit 'gatt write without response' to remote device
    #define     TX_INDICATIONS_TO_C1        0  //! set below macro to Transmit 'gatt indications' to remote device
    ```

    > **Note:**
    > By default, all BLE configurations are chosen to get high BLE throughput.

- Select WLAN configurations in `wifi_app_config.h`

  - Enter the AP Connectivity essentials configs as the value to SSID, SECURITY_TYPE and PSK

    ```c
    #define     SSID                        "SILABS_AP"
    #define     SECURITY_TYPE               RSI_WPA2
    #define     PSK                         "12345678"
    ```

  - Channel no in which device should scan choose

    ```c
    #define     CHANNEL_NO                  0       //! 0 - scan all channels
    ```

  - Configure throughput type as TCP_TX

    ```c
    #define     THROUGHPUT_TYPE          TCP_TX 
    ```

> **Note**: For recommended settings, please refer the [recommendations guide](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-prog-recommended-settings/).

## Test the Application

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

- Build the application.
- Flash, run and debug the application
- Observing the output prints on serial terminal

    ![](resources/readme/output1.png)

- WLAN thread wait for the BLE activity
- Scan for BLE advertise name **RSI_BLE_APP_GATT_TEST** using EFR connect

  ![](resources/readme/remote_screen1.png)

  ![](resources/readme/output2.png)
  
  ![](resources/readme/remote_screen2.png)

  ![](resources/readme/output3.png)
  
  ![](resources/readme/output4.png)

### Common Steps

### WLAN throughputs: UDP/TCP/SSL unidirectional

1. Compile the project and flash the binary onto STM32

2. To measure **WLAN throughput**, run the below iPerf commands or ssl scripts

   - To measure **UDP Tx** throughput, configure module as UDP client and open UDP server in remote port using below command. To establish UDP Server on remote PC, open [iPerf Application](https://sourceforge.net/projects/iperf2/files/iperf-2.0.8-win.zip/download) and run the below command from the installed folder's path in the command prompt.

    ```sh
            iperf.exe -s -u -p <SERVER_PORT> -i 1
    ```

    ex: iperf.exe -s -u -p 5001 -i 1

      ![](resources/readme/remote_screen4.png)

   - To measure **UDP Rx** througput, configure module as UDP server and open UDP client in remote port using below command

      ```sh
              iperf.exe -c <Module_IP> -u -p <DEVICE_PORT> -i 1 -b<Bandwidth> -t <duration in sec>
      ```

      ex: iperf.exe -c 192.168.0.1 -u -p 5001 -i 1 -b50M -t 100

        ![](resources/readme/remote_screen5.png)

   - To measure **TCP Tx** throughput, configure module as TCP client and open TCP server in remote port using below command. To establish TCP Server on remote PC, open [iPerf Application](https://sourceforge.net/projects/iperf2/files/iperf-2.0.8-win.zip/download) and run the below command from the installed folder's path in the command prompt.

      ```sh
              iperf.exe -s -p <SERVER_PORT> -i 1
      ```

      ex: iperf.exe -s -p 5001 -i 1

        ![](resources/readme/remote_screen6.png)

   - To measure **TCP Rx** througput, configure module as TCP server and open TCP client in remote port using below command

      ```sh
              iperf.exe -c <Module_IP> -p <DEVICE_PORT> -i 1 -t <duration in sec>
      ```

      ex: iperf.exe -c 192.168.0.1 -p 5001 -i 1 -t 100

        ![](resources/readme/remote_screen7.png)

   - To measure **SSL Tx** throughput, configure module in SSL client and follow below steps to run SSL server in windows

      - Copy SSL_Server_throughput_d.py from release/resources/scripts/ to release/resources/certificates/

      - Open command prompt in folder release/resources/certificates/ and run below command

        ```sh
                python SSL_Server_throughput_d.py
        ```

      ![](resources/readme/remote_screen8.png)

   - To measure **SSL Rx** throughput, configure module in SSL client and follow below steps to run SSL server in windows

      - Copy SSL_tx_throughput.py from release/resources/scripts/ to release/resources/certificate

      - Change port no. from "5001" to the value configured in "SSL_RX_SERVER_PORT"

      - Open command prompt in folder release/resources/certificates/ and run below command
      > **Note:**
      > Python version: 3.9.0

      ```sh
          python SSL_tx_throughput.py
      ```

      ![](resources/readme/remote_screen9.png)

3. After the program gets executed, Module scans for the configured Access point, connects to it.

4. Acquires the ip address and waits for ble activities to complete if configured.

5. Scan for BLE advertise name (RSI_BLE_APP_GATT_TEST) using EFR connect (Android app)/ BLE dongles and initiate ble connection if found.
  
    ![](resources/readme/remote_screen1.png)

6. For BLE data transfer, enable GATT notifications of Module on service characteristic RSI_BLE_ATTRIBUTE_1_UUID (0x1AA1) using EFR connect.

    ![](resources/readme/remote_screen2.png)

7. If Gatt Notification are enabled, module continuously transmits notifications per connection interval of size 232 bytes.

    ![](resources/readme/remote_screen3.png)

8. Once above activities are completed, Module transmits/receives packets based on the configuration selected and measures the WLAN throughput

> **Note:**
> Verify that all connections are stable and throughput is as expected.
> **Note:**
> In this release, due to low SPI frequency in EFR, wlan throughput are less compared to STM. Work is in progress to support the high frequency.

### BLE throughput

1. Compile the project and flash the binary onto STM32

2. If WLAN is configured, run the below iPerf command (Note: Ensure module is configured as TCP client)

   ```sh
     iperf.exe -s -p <SERVER_PORT> -i 1
   ```

   ex: iperf.exe -s -p 5001 -i 1

3. After the program gets executed, Module scans for the configured Access point, connects to it and acquires the ip address

4. After acquiring ip address, connects to remote server and starts tcp download

5. After successful connection, transmit the data using same android app to module.

6. Module receives the data transmitted by app and retransmits the same to BT SPP manager app.

7. After completion of WLAN, scan for BLE advertise name (RSI_BLE_APP_GATT_TEST) using EFR connect (Android app)/ BLE dongles and initiate ble connection if found.
  
    ![](resources/readme/remote_screen1.png)

8. After successful connection, enable Gatt notifications of Module on service characteristic RSI_BLE_ATTRIBUTE_1_UUID (0x1AA1) using EFR connect.
  
    ![](resources/readme/remote_screen2.png)

9. If GATT Notifications are enabled, module continuously transmits notifications per connection interval of size 232 bytes.
  
    ![](resources/readme/remote_screen3.png)

10. While transmitting, measure the throughput using BLE sniffer

> **Note:**
> Verify that all connections are stable and throughput is as expected.
