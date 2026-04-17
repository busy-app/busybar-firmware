# Wi-Fi - TLS Client

## Table of Contents

- [Wi-Fi - TLS Client](#wi-fi---tls-client)
  - [Table of Contents](#table-of-contents)
  - [Purpose/Scope](#purposescope)
  - [Prerequisites/Setup Requirements](#prerequisitessetup-requirements)
    - [Hardware Requirements](#hardware-requirements)
    - [Software Requirements](#software-requirements)
    - [Setup Diagram](#setup-diagram)
  - [Getting Started](#getting-started)
  - [Application Build Environment](#application-build-environment)
    - [TLS\_EXTENSION\_ENABLE Macro](#tls_extension_enable-macro)
  - [Test the Application](#test-the-application)

## Purpose/Scope

This application demonstrates how to open a TCP client socket on SiWx917 module and use this TCP client socket with a secure connection using SSL and transmit data over the socket.

- Two TCP sockets are opened over SSL. One socket uses SSL TLS v1.0 protocol and the other socket uses SSL TLS v1.2 protocol.
- CA certificate and device certificates are loaded into module flash, as they are necessary for the key exchanges involved while using SSL.

## Prerequisites/Setup Requirements

### Hardware Requirements

- A Windows PC.
- SoC Mode:
  - Silicon Labs [BRD4388A](https://www.silabs.com/)
- NCP Mode:
  - Silicon Labs [BRD4180B](https://www.silabs.com/) **AND**
  - Host MCU Eval Kit. This example has been tested with:
    - Silicon Labs [WSTK + EFR32MG21](https://www.silabs.com/development-tools/wireless/efr32xg21-bluetooth-starter-kit)
  - Interface and Host MCU Supported
    - SPI - EFR32 
    
### Software Requirements

- Simplicity Studio
- Windows PC (Remote PC) with openssl application

### Setup Diagram

  ![Figure: Setup Diagram SoC and NCP Mode for TLS Client Example](resources/readme/tls_client_soc_ncp.png)

## Getting Started

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

- [Install Simplicity Studio](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#install-simplicity-studio)
- [Install WiSeConnect extension](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#install-the-wi-se-connect-extension)
- [Connect your device to the computer](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#connect-si-wx91x-to-computer)
- [Upgrade your connectivity firmware](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#update-si-wx91x-connectivity-firmware)
- [Create a Studio project](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#create-a-project)

For details on the project folder structure, see the [WiSeConnect Examples](https://docs.silabs.com/wiseconnect/latest/wiseconnect-examples/#example-folder-structure) page.

## Application Build Environment

The application can be configured to suit user requirements and development environment. Read through the following sections and make any changes needed.

- The application uses the default configurations as provided in the **default_wifi_ap_profile** in ``sl_net_default_values.h`` and user can choose to configure these parameters as needed.
- In the Project Explorer pane, expand the **config** folder and open the **sl_net_default_values.h** file. Configure the following parameters to enable your Silicon Labs Wi-Fi device to connect to your Wi-Fi network.

  - STA instance related parameters

    - DEFAULT_WIFI_CLIENT_PROFILE_SSID refers to the name with which Wi-Fi network that shall be advertised and Si91X module is connected to it.

      ```c
      #define DEFAULT_WIFI_CLIENT_PROFILE_SSID               "YOUR_AP_SSID"      
      ```

    - DEFAULT_WIFI_CLIENT_CREDENTIAL refers to the secret key if the Access point is configured in WPA-PSK/WPA2-PSK security modes.

      ```c
      #define DEFAULT_WIFI_CLIENT_CREDENTIAL                 "YOUR_AP_PASSPHRASE" 
      ```

    - DEFAULT_WIFI_CLIENT_SECURITY_TYPE refers to the security type if the Access point is configured in WPA/WPA2 or mixed security modes.

      ```c
      #define DEFAULT_WIFI_CLIENT_SECURITY_TYPE              SL_WIFI_WPA2 
      ```
  
  - Other STA instance configurations can be modified if required in `default_wifi_client_profile` configuration structure.

  - Configure the following parameters in ``app.c`` to test tls client app as per requirements

  - Client/Server IP Settings

    ```c
    #define SERVER_PORT1        <remote_port>      // Remote server port
    #define SERVER_PORT2        <remote_port>      // Remote server port
    #define SERVER_IP          "192.168.0.100"     // Remote server IP address
    ```

- `sl_wifi_device_configuration_t` from `app.c` should be modified as per below requirements

  **[sl_net_set_credential()](https://docs.silabs.com/wiseconnect/3.0.13/wiseconnect-api-reference-guide-nwk-mgmt/net-credential-functions#sl-net-set-credential)** API expects the certificate in the form of linear array. Convert the pem certificate into linear array form using python script provided in the SDK `<SDK>/resources/scripts/certificate_script.py`.

   For example : If the certificate is ca-certificate.pem, enter the command in the following way:
   python certificate_script.py ca-certificate.pem 
   The script will generate ca-certificate.pem in which one linear array named ca-certificate contains the certificate.

  Root CA certificate needs to be converted as mentioned above.

  After the conversion, place the converted file in `<SDK>/resources/certificates/` path and include the certificate file in app.c

  ```c
  // Certificate includes
  #include "ca-certificate.pem.h"
  
  // Load Security Certificates
  status = sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(0), SL_NET_SIGNING_CERTIFICATE, ca-certificate, sizeof(ca-certificate) - 1);
  ```

> **Note** :
 The included self-signed certificates will work for local OpenSSL server. For cloud servers, using default certificates for a cloud connection does not work. Replace the default certificates with valid certificates when connecting to Cloud Server.

---

### TLS_EXTENSION_ENABLE Macro

The `TLS_EXTENSION_ENABLE` macro controls whether advanced TLS extensions—specifically SNI (Server Name Indication) and ALPN (Application-Layer Protocol Negotiation)—are enabled in the TLS client example.

When `TLS_EXTENSION_ENABLE` is set to 1:
- The application will include and use SNI and ALPN extensions during the TLS handshake.
- SNI allows the client to specify the hostname it is connecting to, which is required by many modern servers that host multiple domains on a single IP address.
- ALPN allows the client and server to negotiate the application protocol (such as HTTP/1.1 or HTTP/2) to use over the secure connection.

When `TLS_EXTENSION_ENABLE` is set to 0:
- The application will not use SNI or ALPN extensions.
- The TLS handshake will be simpler, but connections to servers that require SNI or ALPN may fail.

**Example:**
```c
#define TLS_EXTENSION_ENABLE 1 // Enable TLS protocol extensions (e.g., SNI)
```

> **Note**: For recommended settings, please refer the [recommendations guide](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-prog-recommended-settings/).

## Test the Application

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

- Build the application
- Flash, run and debug the application

  ![TLS Client_Output](resources/readme/tls_client_output.png)

- After making any custom configuration changes required, build, download and run the application as below.
- Copy the certificates server-cert and server-key into Openssl/bin folder in the Windows PC2 (Remote PC).
- In Windows PC2 (Remote PC) which is connected to AP, run the Openssl and run two SSL servers by giving the following command:

  ```sh
      > Openssl.exe s_server -accept<SERVER_PORT> -cert <server_certificate_file_path> -key <server_key_file_path> -tls<tls_version>

      Example:
      > openssl.exe s_server -accept 5002 -cert server-cert.pem -key server-key.pem -tls1
  ```
> **Note:** The client must set the SNI extension when connecting to an AWS server using TLS 1.3 version.

- After the program gets executed, SiWx91x EVK would be connected to access point having the configuration same as that of in the application and get IP.
- The SiWx91x EVK which is configured as SSL client will connect to remote SSL server and sends number of packets configured in NUMBER_OF_PACKETS.

- EVK connects to remote SSL server-1

  ![EVK connects to remote SSL server-1](resources/readme/server1.png)
    
- EVK connects to remote SSL server-2
  
  ![EVK connects to remote SSL server-2](resources/readme/server2.png)
