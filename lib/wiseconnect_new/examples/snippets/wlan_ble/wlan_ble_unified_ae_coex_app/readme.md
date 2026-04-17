# WLAN - BLE Unified AE Coex App

## Table of Contents

- [WLAN - BLE Unified AE Coex App](#wlan---ble-unified-ae-coex-app)
  - [Table of Contents](#table-of-contents)
  - [Purpose/Scope](#purposescope)
  - [Prerequisites/Setup Requirements](#prerequisitessetup-requirements)
    - [Hardware Requirements](#hardware-requirements)
    - [Software Requirements](#software-requirements)
    - [Setup Diagram](#setup-diagram)
  - [Getting Started](#getting-started)
  - [Application Build Environment](#application-build-environment)
    - [Configure the BLE Parameters](#configure-the-ble-parameters)
    - [Extended Advertising Configurations](#extended-advertising-configurations)
    - [Extended Scanning Configurations](#extended-scanning-configurations)
  - [WLAN throughputs: UDP/TCP/TLS unidirectional](#wlan-throughputs-udptcptls-unidirectional)
  - [Test the Application](#test-the-application)
    - [Central Connection](#central-connection)
    - [Pairing Procedure](#pairing-procedure)
    - [Peripheral Connection](#peripheral-connection)
    - [BLE Data Transfer](#ble-data-transfer)
  - [Limitations for this App](#limitations-for-this-app)

## Purpose/Scope

This application demonstrates how to configure SiWx91x in Extended Advertising/Scanning mode with BLE (1 -Central,1-Peripheral) connections along with BLE data transfer and WLAN TCP data transfer.

## Prerequisites/Setup Requirements

### Hardware Requirements

- Windows PC
- TCP Server
- SoC Mode:
  - Silicon Labs [BRD4325A, BRD4325B, BRD4325C, BRD4325G, BRD4338A, BRD4339B, BRD4343A](https://www.silabs.com/)
  - Kits
    - SiWx91x AC1 Module Explorer Kit (BRD2708A)
- NCP Mode:
  - Silicon Labs [BRD4180B](https://www.silabs.com/);
  - Host MCU Eval Kit. This example has been tested with:
    - Silicon Labs [WSTK + EFR32MG21](https://www.silabs.com/development-tools/wireless/efr32xg21-bluetooth-starter-kit)
    - NCP Expansion Kit with NCP Radio boards
      - (BRD4346A + BRD8045A) [SiWx917-EB4346A]
      - (BRD4357A + BRD8045A) [SiWx917-EB4357A]
  - Interface and Host MCU Supported 
    - SPI - EFR32 
- Smartphone configured as BLE peripheral/central which supports extended advertising.
- Access point
- Windows PC (to run the TCP server).

### Software Requirements

- Simplicity Studio IDE
- Download and install the Silicon Labs [Simplicity Connect App(formerly EFR Connect App) or other BLE Central/Peripheral app.](https://www.silabs.com/developers/simplicity-connect-mobile-app ) in the android smart phones for testing BLE applications. Users can also use their choice of BLE apps available in Android/iOS smart phones.
- [iPerf Application](https://sourceforge.net/projects/iperf2/files/iperf-2.0.8-win.zip/download). iPerf is a tool for active measurements of the maximum achievable bandwidth on IP networks. It supports tuning of various parameters related to timing, buffers, and protocols (TCP and UDP with IPv4 and IPv6).
- [Python Environment](https://www.python.org/downloads/)
- Serial Terminal - [Docklight](https://docklight.de/)/[Tera Term](https://ttssh2.osdn.jp/index.html.en) (to be used with Keil IDE)

### Setup Diagram

  ![Figure: Setup Diagram NCP and SOC Mode for BLE Unified AE Coex Example](resources/readme/wlan_ble_ae_setup.png)	
   		
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

- The WLAN_TASK_ENABLE macro is a configuration option that controls whether the WLAN (Wi-Fi) task is enabled in the application.
- When WLAN_TASK_ENABLE is set to 1, the application will start and run the WLAN task, allowing the device to scan for Wi-Fi networks, connect to an access point, and perform Wi-Fi-related operations.
- If it is set to 0, the WLAN task will be disabled, and Wi-Fi functionality will not be configured in the application.

**Configure the WLAN parameters**

    ```c
    #define WLAN_TASK_ENABLE 1
    ```

2. In the Project Explorer pane, expand the **wifi** folder and open the `wifi_app_config.h` file and update/modify following macros:

   - Enter the AP Connectivity essentials configurations as the value to SSID, SECURITY_TYPE, CHANNEL_NO and PSK

      ```c
      #define SSID          "YOUR_AP_SSID"
      #define SECURITY_TYPE SL_WIFI_WPA2
      #define CHANNEL_NO    0
      #define PSK           "YOUR_AP_PASSPHRASE"
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

   - IP address of remote server:

      ```c
      #define     SERVER_IP          "192.168.0.234"
      ```

   - Port number of module:

      ```c
      #define     DEVICE_PORT                5005
      ```

   - Port number of remote server

      ```c
      #define     SERVER_PORT                5000
      ```

   - To select the ip, configure below macros:

      ```c
      #define     DHCP_MODE                  1
      ```

   - Enable the following macro only if the user wants to ensure that the Wi-Fi thread continuously scans for APs and prints the scan results:

      ```c
      #define WIFI_CONTINUOUS_SCAN_MODE_ONLY 0
      ```

   - Enable this macro only if the user wants to connect WiFi to the AP, print the IP address, and then suspend the connection

      ```c
      #define WIFI_CONNECTION_ONLY 0
      ```

   - Enable the below macro to use asynchronous socket APIs. If disabled, the application will use synchronous socket APIs.

      ```c
      #define SOCKET_ASYNC_FEATURE 1
       ```

### Configure the BLE Parameters

- Open `ble_config.h` file and update/modify following macros:

  - Configure the following macros to enable extended advertsing and scanning by default respectively:

       ```c
       #define ADV_ENABLED_DEFAULT    1
       #define SCAN_ENABLED_DEFAULT   1
       ```

  - Set the following macro to enable second advertising set:

       ```c
       #define ADV_SET2    1
       ```

  - Configure the following macros to set the number of peripheral and central connections that can be made:

       ```c
       #define RSI_BLE_MAX_NBR_SLAVES    1
       #define RSI_BLE_MAX_NBR_MASTERS   1
       ```

  - Fill the name of the peripheral to be connected to in the following macro:

       ```c
       #define RSI_REMOTE_DEVICE_NAME1   AEdevice1
       ```

  - Configure following macro to enable secure connection for central and peripheral respectively:

       ```c
       #define SMP_ENABLE_C1    1
       #define SMP_ENABLE_P1    1
       ```

  - Set following macro to receive 'gatt notifications' from remote device:

       ```c
       #define RX_NOTIFICATIONS_FROM_C1    1
       #define RX_NOTIFICATIONS_FROM_P1    1
       ```
  
    > **Note**: Max limit for number of peripheral connections is 1 and central connections is 1

  - Power save configuration:

    - By default, The application is configured without power save.

       ```c
       #define ENABLE_NWP_POWER_SAVE 0
       ```

    - If user wants to run the application in power save, modify the following configuration.

       ```c
       #define ENABLE_NWP_POWER_SAVE 1
       ```

  - The desired parameters are provided following. User can also modify the parameters as per their needs and requirements.

  ### Extended Advertising Configurations

    - The length of the advertising data needs to be filled in the following macro
       ```c
       #define BLE_AE_ADV_DATA_LEN    0x19
       ```

    - Follow mentioned macro needs to be populated with data:

       ```c
       #define BLE_AE_ADV_DATA    "AE_PERIPHERAL_DATA_1"
       ```
  - The Extended advertising handle for set 1 and 2 can be configured with the following macros respectively
       ```c
       #define BLE_AE_ADV_HNDL_SET_1          0x00
       #define BLE_AE_ADV_HNDL_SET_2          0x01
       ```
  - The minimum advertising interval for set 1 and 2 can be configured with the following macros respectively
       ```c
       #define BLE_AE_ADV_INT_MIN_SET_1       0x20
       #define BLE_AE_ADV_INT_MIN_SET_2       0x30
       ```
  - The maximum advertising interval for set 1 and 2 can be configured with the following macros respectively
       ```c
       #define BLE_AE_ADV_INT_MAX_SET_1       0x20
       #define BLE_AE_ADV_INT_MAX_SET_2       0x30
       ```
  - The Extended advertising channel map for set 1 and 2 can be configured with the following macros respectively
       ```c
       #define BLE_AE_ADV_CHANNEL_MAP_SET_1   0x07
       #define BLE_AE_ADV_CHANNEL_MAP_SET_2   0x07
       ```
  - The Extended advertising filter policy for set 1 and 2 can be configured with the following macros respectively
       ```c
       #define BLE_AE_ADV_FILTER_POLICY_SET_1 0x00
       #define BLE_AE_ADV_FILTER_POLICY_SET_2 0x00
       ```
  - The Extended advertising TX Power for set 1 and 2 can be configured with the following macros respectively
       ```c
       #define BLE_AE_ADV_TX_PWR_SET_1        0x7f
       #define BLE_AE_ADV_TX_PWR_SET_2        0x7f
       ```
  - The primary advertising phy for set 1 and 2 can be configured with the following macros respectively
       ```c
       #define BLE_AE_PRIMARY_ADV_PHY_SET_1   0x01
       #define BLE_AE_PRIMARY_ADV_PHY_SET_2   0x01
       ```
  - The Extended advertising max skip for set 1 and 2 can be configured with the following macros respectively
       ```c
       #define BLE_AE_SEC_ADV_MAX_SKIP_SET_1  0x00
       #define BLE_AE_SEC_ADV_MAX_SKIP_SET_2  0x00
       ```
  - The secondary advertising phy for set 1 and 2 can be configured with the following macros respectively
       ```c
       #define BLE_AE_SECONDARY_ADV_PHY_SET_1 0x01
       #define BLE_AE_SECONDARY_ADV_PHY_SET_2 0x01
       ```
  - The Extended advertising SID for set 1 and 2 can be configured with the following macros respectively
       ```c
       #define BLE_AE_ADV_SID_SET_1           0x00
       #define BLE_AE_ADV_SID_SET_2           0x01
       ```
  - The Extended advertising scan request notification enable for set 1 and 2 can be configured with the following macros respectively
       ```c
       #define BLE_AE_SCAN_REQ_NOTIF_EN_SET_1 0x01
       #define BLE_AE_SCAN_REQ_NOTIF_EN_SET_2 0x01
       ```
  - The Extended advertising event properties for set 1 and 2 can be configured with the following macros respectively
       ```c
       #define BLE_AE_ADV_EVNT_PROP_SET_1     (BLE_CONNECTABLE_ADV)
       #define BLE_AE_ADV_EVNT_PROP_SET_2     0x00
       ```
  - The Extended advertising duration for set 1 and 2 can be configured with the following macros respectively
       ```c
       #define BLE_AE_ADV_DUR_SET_1           0x00
       #define BLE_AE_ADV_DUR_SET_2           0x00
       ```
  - The maximum extended advertising events for set 1 and 2 can be configured with the following macros respectively
       ```c
       #define BLE_AE_ADV_MAX_AE_EVENTS_SET_1 0x00
       #define BLE_AE_ADV_MAX_AE_EVENTS_SET_2 0x00
       ```
   ### Extended Scanning Configurations
  - Extended scanning filter type can be set with the following macro
       ```c
       #define BLE_AE_SCAN_FILTER_TYPE       SCAN_FILTER_TYPE_ALL
       ```

  - The scan type for primary phy can be set using following macro:

       ```c
       #define PRI_PHY_BLE_AE_SCAN_TYPE      SCAN_TYPE_ACTIVE
       ```

  - The scan type for secondary phy can be set using following macro:

       ```c
       #define SEC_PHY_BLE_AE_SCAN_TYPE      SCAN_TYPE_ACTIVE
       ```

  - The primary phy extended scan interval can be set using following macro:

       ```c
       #define PRI_PHY_LE_AE_SCAN_INTERVAL   0x100
       ```

  - The primary phy extended scan window can be set using following macro:

       ```c
       #define PRI_PHY_LE_AE_SCAN_WINDOW     0x50
       ```

  - The secondary phy extended scan interval can be set using following macro:

       ```c
       #define SEC_PHY_LE_AE_SCAN_INTERVAL   0x100
       ```

  - The secondary phy extended scan window can be set using following macro:

       ```c
       #define SEC_PHY_LE_AE_SCAN_WINDOW     0x50
       ```

  - Extended scanning filter duplicates can be set using following macro:

       ```c
       #define BLE_AE_SCAN_ENABLE_FILTER_DUP 0x00
       ```

  - Extended scan duration can be set using following macro

       ```c
       #define BLE_AE_SCAN_DUR               0x00
       ```

  - Extended scan period can be set using following macro

       ```c
       #define BLE_AE_SCAN_PERIOD            0x00
       ```
  - Random address can be set for advertising sets using following macro
       ```c
       #define RSI_BLE_SET_RAND_ADDR       "CC:44:33:44:55:CC"
       ```
Following are the non-configurable macros in the application.

    - The event properties bits:

       ```c
       #define BLE_CONNECTABLE_ADV        (1 << 0)
       #define BLE_SCANNABLE_ADV          (1 << 1)
       #define BLE_LOW_DUTY_DIR_CONN_ADV  (1 << 2)
       #define BLE_HIGH_DUTY_DIR_CONN_ADV (1 << 3)
       #define BLE_LEGACY_ADV             (1 << 4)
       #define BLE_ANONYMOUS_ADV          (1 << 5)
       #define BLE_TX_WR_ADV              (1 << 6)
       ```
  
  - Phy rate bits:
       ```c
       #define PHY_1M       BIT(0)
       #define PHY_2M       BIT(1)
       #define PHY_LE_CODED BIT(2)
       #define RSI_BLE_PWR_INX          30
       #define RSI_BLE_PWR_SAVE_OPTIONS 0    
       #define BLE_DISABLE_DUTY_CYCLING 0
       #define BLE_DUTY_CYCLING         1
       #define BLR_DUTY_CYCLING         2
       #define BLE_4X_PWR_SAVE_MODE     4
       ```

 >**Note:** `ble_config.h` files are already set with desired configuration in respective example folders user need not change for each example.
   
## WLAN throughputs: UDP/TCP/TLS unidirectional

To measure **WLAN throughput**, run the following commands.

> **Note:** The "wlan_ble_unified_ae_coex_app" application has been verified only with TCP_TX.

1. To measure **UDP Tx** throughput, configure the SiWx91x device as a UDP client and open the UDP server on the remote PC using the following iPerf command. To establish UDP Server on remote PC, open [iPerf Application](https://sourceforge.net/projects/iperf2/files/iperf-2.0.8-win.zip/download) and run the below command from the installed folder's path in the command prompt.

    ```sh
    iperf.exe -s -u -p <SERVER_PORT> -i 1
    ```

   Example: iperf.exe -s -u -p 5001 -i 1
    ![](resources/readme/udp_tx.png)

2. To measure **UDP Rx** throughput, configure the SiWx91x device as a UDP server and open the UDP client on the remote PC using below iPerf command

    ```sh
    iperf.exe -c <Module_IP> -u -p <DEVICE_PORT> -i 1 -b<Bandwidth> -t <duration in sec>
    ```

    Example: iperf.exe -c 192.168.0.245 -u -p 5005 -i 1 -b50M -t 20

    ![](resources/readme/udp_rx.png)

3. To measure **TCP Tx** throughput, configure the SiWx91x device as a TCP client and open the TCP server on the remote PC using below iPerf command. To establish TCP Server on remote PC, open [iPerf Application](https://sourceforge.net/projects/iperf2/files/iperf-2.0.8-win.zip/download) and run the below command from the installed folder's path in the command prompt.

    ```sh
    iperf.exe -s -p <SERVER_PORT> -i 1
    ```

   Example: iperf.exe -s -p 5001 -i 1

   ![](resources/readme/tcp_tx.png)

4. To measure **TCP Rx** throughput, configure the SiWx91x device as a TCP server and open TCP client on remote PC using the following iPerf command:

    ```sh
            iperf.exe -c <Module_IP> -p <DEVICE_PORT> -i 1 -t <duration in sec>
    ```

   Example: iperf.exe -c 192.168.0.245 -p 5005 -i 1 -t 20

    ![](resources/readme/tcp_rx.png)

> **Note:** The client must set the SNI extension when connecting to an AWS server using TLS 1.3 version.

5. To measure **TLS Tx** throughput, configure the SiWx91x device as a TLS client and follow the steps below to run a TLS server on Windows machine.

   - Copy SSL_Server_throughput_d.py from **release/resources/scripts/** to **release/resources/certificates/**.

   - Open the command prompt in the release/resources/certificates/ folder and run the following command:
     > **Note:**
     > Python version: 3.9.0

      ```sh
              python SSL_Server_throughput_d.py
      ```

    ![](resources/readme/tls_tx.png)

6. To measure **TLS Rx** throughput, configure the SiWx91x device as a TLS client and follow the steps below to run a TLS server on Windows machine.

   - Copy SSL_tx_throughput.py from **release/resources/scripts/** to **release/resources/certificate**.

   - Change the port number from "5001" to the value configured in "TLS_RX_SERVER_PORT".

   - Open the command prompt in the release/resources/certificates/ folder and run the following command.:

      ```sh
          python SSL_tx_throughput.py
      ```

      ![](resources/readme/tls_rx.png)

> **Note**: For recommended settings, please refer the [recommendations guide](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-prog-recommended-settings/).

## Test the Application

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

- Build the application.
- Flash, run and debug the application.

Follow the steps as mentioned for the successful execution of the application:

1. Set all the configurations necessary as described in sections "Configure the BLE parameters" and "Configure the WLAN parameters".

   **Note:** According to the application flow, both WLAN and BLE tasks are running in separate threads. Depending on the user's choice, either the WLAN connection will establish first or the BLE connection will establish first.

2. By default, the Si91x device is configured for two extended advertising sets and extended scanning. Its advertising can be validated with any remote device that supports the AE feature. The following image shows the Si91x device's extended advertising sets as seen in the SiConnect app.

     ![](resources/readme/remote_scanner.png)

### Central Connection

1. To make remote central connection, scan from any BLE scanner application (SiConnect app) search for the device (if no random address is set it will advertise with the public address).
2. Default application advertises with a random address for BLE_AE_ADV_HNDL_SET_1 and with a public address for BLE_AE_ADV_HNDL_SET_2.
3. As shown in step 2, to establish the connection, click on the **CONNECT** option of the device for the required advertising set.
4. AFter the physical level connection is established, and if SMP is enabled, the application will initiate the SMP process, as shown in the images below.

### Pairing Procedure

1. A pairing request pop-up will appear as shown in the image below. Click on the **Pair and connect** option.

     ![](resources/readme/pairing_prompt_on_conn.png)

2. A subsequent pairing pop-up will appear as shown in the image below. Click on the **Pair** option.

     ![](resources/readme/pairing_prompt.png)

3. The passkey will be displayed in the serial terminal.

     ![](resources/readme/passkey_display_term_log.png)

4. Enter the passkey as shown in the following image and click on the "OK" option to proceed.

     ![](resources/readme/passkey_entry.png)

5. Once the SMP pairing process is successful, the connection status is displayed as follows:

     ![](resources/readme/active_connection.png)

### Peripheral Connection

   1. If extended scanning is enabled, the Si91x device will scan for the advertising devices using the configured scan parameters.

   2. To make a remote peripheral connection, advertise the remote device with the specific name defined in the RSI_REMOTE_DEVICE_NAME1 macro.

      **Note:**  See [Create a New Advertisement Set](https://docs.silabs.com/mobile-apps/latest/mobile-apps-explore/03-ble-configure-view) for creating the advertising set in the Simplicity Connect mobile App (formerly EFR Connect App).

   3. Add required services (for example, heartrate service) as shown in the following below and advertise from the remote peripheral device.

       ![](resources/readme/add_heartrate_gatt_service.png)

   4. If SMP is enabled for this connection, the bonding process is initiated after connection is established as mentioned in Pairing procedure.

### BLE Data Transfer

   1. Click on the **Notify** property to enable notifications for continuous data transfer from the Si91x device.
    When notifications are enabled, the **Notify** option will be highlighted as shown in the following image.

       ![](resources/readme/notification_enable.png)

**To Set Up the WLAN Connection**

   1. If `WLAN_TASK_ENABLE` is set to 1, the device initializes the Wi-Fi module using the configuration parameters defined in the application (such as SSID and security type).
   2. It then scans for available Wi-Fi networks, connects to the designated access point, and performs Wi-Fi-related operations as specified in the application.

      ![Prints in docklight window](resources/readme/output_wlan_init.png)
      ![Prints in docklight window](resources/readme/output_wlan_scan_conn.png)

   3. To enable Wi-Fi TCP data transfer, the application has the TCP client functionality. The user needs to run the TCP server on the PC that is connected to the same AP.

      **Note:**  Refer to the commands mentioned in the section "WLAN throughputs: UDP/TCP/TLS unidirectional" point 3 to configure iperf as TCP server.

   4. On the Wi-Fi side, the SiWx91x device is configured as a UDP/TCP/TLS server or client. When the SiWx91x device is configured as a server, it transmits data to the iperf client. Similarly, when the SiWx91x device is configured as a client, it receives data from the iperf server for a specified interval of 30 seconds. After the transmission or reception is complete, the final average application throughput will be displayed in the serial console.
     ![Prints in docklight window](resources/readme/output_throughput.png)

   5. Refer the following images for examples of console prints:
      ![Prints in docklight window](resources/readme/output_log1.png)
      ![Prints in docklight window](resources/readme/output_log2.png)
      ![Prints in docklight window](resources/readme/output_log3.png)
     ![Prints in docklight window](resources/readme/output_log4.png)

## Limitations for this App

- PSRAM is not supported for this feature.
- This feature is supported only with TA_480K_M4SS_192K and TA_416K_M4SS_256K RAM configurations.
- This feature is supported only with power save enabled.
- TWT(Target Wake Times) based wakeup is not supported.
- A maximum of 2 BLE connections are supported when the AE+CoEx feature is enabled.
- The scan interval and window are recommended to be in a 3:1 ratio.
- Maximum of 2 advertising sets are supported by this app.
- SMP pairing is limited to IOCapabilities 0x01 and 0x03.
- DUT does not save bonding information.
- Bonding information needs to be removed from remote device before issuing reconnection.
- WLAN/BLE throughputs will be less in coexistence cases due to BLE flow control missing in SDK.
