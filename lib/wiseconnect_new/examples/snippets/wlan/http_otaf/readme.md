# Wi-Fi - HTTP/HTTPS OTAF Update

## Table of Contents

- [Wi-Fi - HTTP/HTTPS OTAF Update](#wi-fi---httphttps-otaf-update)
  - [Table of Contents](#table-of-contents)
  - [Purpose/Scope](#purposescope)
  - [Prerequisites/Setup Requirements](#prerequisitessetup-requirements)
    - [Hardware Requirements](#hardware-requirements)
    - [Software Requirements](#software-requirements)
    - [Setup Diagram](#setup-diagram)
  - [Getting Started](#getting-started)
  - [Application Build Environment](#application-build-environment)
  - [Test the Application](#test-the-application)
  - [Additional Information](#additional-information)
    - [Configuring an AWS S3 Bucket](#configuring-an-aws-s3-bucket)
    - [Configuring Azure Blob Storage](#configuring-azure-blob-storage)
    - [Configuring and Uploading Firmware on Apache HTTP](#configuring-and-uploading-firmware-on-apache-http)
    - [Configuring and Uploading Firmware on Apache HTTPs](#configuring-and-uploading-firmware-on-apache-https)

## Purpose/Scope

This application shows how to update the NWP or M4 firmware of a device via Wi-Fi by downloading an update from a remote HTTP/HTTPS server. The server can be run on a local PC (Apache server) or hosted on a cloud service like Amazon AWS or Microsoft Azure. The update process goes through the following steps:

- **Connection**: The device connects to a Wi-Fi network and acts as a HTTP/HTTPS client.
- **Request**: The device sends a request to the HTTP/HTTPS server for the firmware update file.
- **Download**: The server sends the firmware file to the device.
- **Update**: The device writes the new firmware to its memory and then restarts to complete the update.

This process allows the device to update its software OTA without needing a physical connection.

> **Note:** By enabling the `HTTPS_SUPPORT` flag in the `app.c` file, the same HTTP Over-the-Air (OTA) application can be used for HTTPS OTA.

## Prerequisites/Setup Requirements

### Hardware Requirements

- Windows PC
- Wireless Access point
- SiWx91x Wi-Fi Evaluation Kit. The SiWx91x supports multiple operating modes. See [Operating Modes]() for details.
- **SoC Mode**:
  - Standalone
    - BRD4002A Wireless pro kit mainboard [SI-MB4002A]
    - Radio Boards
      - BRD4338A [SiWx917-RB4338A]
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
    - UART - EFR32

### Software Requirements

- Simplicity Studio

### Setup Diagram

![Figure: Setup Diagram SoC Mode for http_otaf Example](resources/readme/setup_soc_ncp.png)

## Getting Started

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

- [Install Simplicity Studio](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#install-simplicity-studio)
- [Install WiSeConnect extension](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#install-the-wi-se-connect-extension)
- [Connect your device to the computer](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#connect-si-wx91x-to-computer)
- [Upgrade your connectivity firmware](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#update-si-wx91x-connectivity-firmware)
- [Create a Studio project](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#create-a-project)

For details on the project folder structure, see the [WiSeConnect Examples](https://docs.silabs.com/wiseconnect/latest/wiseconnect-examples/#example-folder-structure) page.

## Application Build Environment

The application can be configured to suit your requirements and the development environment. Read through the following sections and make any changes needed.

- The application uses the default configurations as provided in the **DEFAULT_WIFI_CLIENT_PROFILE** in **sl_net_default_values.h** and you can choose to configure these parameters as needed.

- In the Project explorer pane, expand the **config** folder and open the ``sl_net_default_values.h`` file. Configure the following parameters to enable SiWx91x to connect to your Wi-Fi network.

  - STA instance related parameters:

    - DEFAULT_WIFI_CLIENT_PROFILE_SSID refers to the name with which the Wi-Fi network will be advertised and Si91X module is connected to it.

      ```c
      #define DEFAULT_WIFI_CLIENT_PROFILE_SSID               "YOUR_AP_SSID"      
      ```

    - DEFAULT_WIFI_CLIENT_CREDENTIAL refers to the secret key if the access point is configured in WPA-PSK/WPA2-PSK security modes.

      ```c
      #define DEFAULT_WIFI_CLIENT_CREDENTIAL                 "YOUR_AP_PASSPHRASE" 
      ```

    - DEFAULT_WIFI_CLIENT_SECURITY_TYPE refers to the security type if the access point is configured in WPA/WPA2 or mixed security modes.

      ```c
      #define DEFAULT_WIFI_CLIENT_SECURITY_TYPE              SL_WIFI_WPA2 
      ```

  - Other STA instance configurations can be modified if required in **DEFAULT_WIFI_CLIENT_PROFILE** configuration structure.

- The following configurations in the ``app.c`` file can be configured as per requirements:

  - Select HTTPS CERTIFICATE INDEX

    - For HTTPS Certificate index select, the default value of the CERTIFICATE_INDEX is set to '0'. To set SL_SI91X_HTTPS_CERTIFICATE_INDEX_1, modify the CERTIFICATE_INDEX to '1' and to set SL_SI91X_HTTPS_CERTIFICATE_INDEX_2, modify the CERTIFICATE_INDEX to '2'.
  
      ```c
      //! set 1 for selecting SL_SI91X_HTTPS_CERTIFICATE_INDEX_1, set 2 for selecting SL_SI91X_HTTPS_CERTIFICATE_INDEX_2
      #define CERTIFICATE_INDEX 0
      ```

  - Select Firmware update type.

    - For NWP firmware upgrade, set FW_UPDATE_TYPE to TA_FW_UPDATE and for M4 firmware upgrade, set FW_UPDATE_TYPE to M4_FW_UPDATE. For Combined firmware upgrade, set FW_UPDATE_TYPE to COMBINED_FW_UPDATE.

      ```c
      //! Type of FW update
      #define M4_FW_UPDATE       0
      #define TA_FW_UPDATE       1
      #define COMBINED_FW_UPDATE 2

      //! Set FW update type
      #define FW_UPDATE_TYPE TA_FW_UPDATE
      ```

      > **Note:**
      > - Use a combined image to update the firmware whenever possible. A combined image includes both the Network Processor (NWP) and Application Processor (M4) images in a single package.
      > - When generating a combined image, use the NWP and M4 images from the same release package. Using different versions may result in undefined behavior due to a version mismatch.
      > - For devices with 4 MB flash, updating with a combined image is not supported because of memory limitations. In this case, update the NWP image first, and then update the M4 image.
      > - For NCP mode, update the NWP image first, followed by the host image from the same release version.

  - Steps to Create a Combined Image

    - **Case 1: When Security is Disabled**
      1. Navigate to the Commander directory.
      2. Copy the NWP firmware image and M4 image into the Commander directory.
      3. Create the nwp_combined_image.rps file:

         ```sh
         commander rps convert <nwp_combined_image.rps> --taapp <original non-encrypted TA rps> --combinedimage
         ```

      4. Create the m4_combined_image.rps file:

         ```sh
         commander rps convert <m4_combined_image.rps> --app <original non-encrypted M4 rps> --combinedimage
         ```

      5. Create the final combined image:

         ```sh
         commander rps convert <combined_image.rps> --app <m4_combined_image.rps> --taapp <nwp_combined_image.rps>
         ```

    - **Case 2: When Security is Enabled**
      - For devices with security enabled, additional signing and encryption steps are required. Follow the detailed instructions in **Section 6 - Combined Image (NWP + M4)** of the [UG574 SiWx917 SoC Manufacturing Utility User Guide](https://www.silabs.com/documents/public/user-guides/ug574-siwx917-soc-manufacturing-utility-user-guide.pdf#page=24).

  - Based on the type of server (Apache/AWS S3 bucket/Azure Blob Storage) from which the firmware files are downloaded, configure the following parameters.
      - Configure FLAGS to choose the version and security type to be enabled.

        Valid configurations are:

        ```c
        #define HTTPS_SUPPORT    BIT(0)         // Set HTTPS_SUPPORT to use HTTPS feature
        #define HTTPV6           BIT(3)         // Enable IPv6. Set this bit in FLAGS. Default is IPv4
        #define HTTP_V_1_1       BIT(6)         // Set HTTP_V_1_1 to use HTTP version 1.1
        ```

      - In the application, the **AWS_ENABLE** macro is enabled by default. Depending on the requirement, the user can enable downloading firmware from Azure Blob storage (Enable Macro **AZURE_ENABLE**).
      - Else, if both **AWS_ENABLE** and **AZURE_ENABLE** macros are disabled, HTTP/HTTPS Apache server can be used to download the firmware.
      - In the application, the following parameters should be configured:
        - HTTP_PORT refers to HTTP Server port number.
        - HTTP_SERVER_IP_ADDRESS refers to HTTP Server IP address.
        - HTTP_URL refers to HTTP resource name.
        - HTTP_HOSTNAME refers to HTTP server hostname.
        - HTTP_EXTENDED_HEADER refers to HTTP extended header. If NULL, the default extented header is filled.
        
          The purpose of this macro is to append user configurable header fields to the default HTTP/HTTPS header.
          The extended header may contain multiple header fields, with each field terminated by "\r\n" (0x0D 0x0A).
        
          Example: key1:value1"\r\n"key2:value2"\r\n"
        
        - USERNAME refers to the username to be used to access the HTTP resource.
        - PASSWORD refers to the password to be used to access the HTTP resource.

      - For **Apache HTTP Server**:

        - Provide the PC IP where the Apache server is running in HTTP_SERVER_IP_ADDRESS
        - Provide the firmware package name uploaded in the Apache server in HTTP_URL

          ```c
          //Sample configurations
          #define FLAGS                   0
          #define HTTP_PORT               80
          #define HTTP_SERVER_IP_ADDRESS  "192.168.xxx.xxx"
          #define HTTP_URL                "Firmware/firmware.rps" //Firmware file name to download
          #define HTTP_HOSTNAME           "192.168.xxx.xxx"
          #define USERNAME                "admin"
          #define PASSWORD                "admin"
          ```

          > **Note:** Refer to the [Configuring and Uploading Firmware on Apache HTTP](#configuring-and-uploading-firmware-on-apache-http) section for information on setting up the Apache Server.

      - For **Apache HTTPS Server**:
        - Include Root certificate pem file for SSL connection.

          ```c
          //Sample configurations
          #include "cacert.pem.h"
          #define FLAGS                   HTTPS_SUPPORT
          #define HTTP_PORT               443
          #define HTTP_SERVER_IP_ADDRESS  "192.168.xxx.xxx"
          #define HTTP_URL                "Firmware/firmware.rps" //Firmware file name to download
          #define HTTP_HOSTNAME           "192.168.xxx.xxx"
          #define USERNAME                "admin"
          #define PASSWORD                "admin"
          ```

      - For **AWS S3 Bucket**:

        - Include Starfield root certificate file for SSL connection.

          > **Note:** The certificate authority for Amazon AWS S3 is Starfield, so we need to include Starfield Root certification for SSL connection to be successful. This certificate is already included in the SDK in linear array format ``aws_starfield_ca.pem.h``, which can be directly used for SSL connection to AWS S3.

        - Extract the hostname from AWS S3 bucket URL `https://<Your-S3-Bucket-name>.s3.<Your-nearest-S3-location>.amazonaws.com/firmware.rps` and provide it in **hostname**.

          > Example: For S3 bucket URL <https://example.s3.ap-south-1.amazonaws.com/firmware.rps>", hostname will be "example.s3.ap-south-1.amazonaws.com"

        - Extract the firmware package name from URL `https://<Your-S3-Bucket-name>.s3.<Your-nearest-S3-location>.amazonaws.com/firmware.rps` and provide it in **HTTP_URL**

          > Example: For S3 bucket URL "<https://example.s3.ap-south-1.amazonaws.com/firmware.rps>", HTTP_URL will be "firmware.rps"

        - Configurations for AWS S3 bucket

          ```c
          //Sample configurations
          #include "aws_starfield_ca.pem.h"          //CA certificate
          #define FLAGS                              HTTPS_SUPPORT
          #define HTTP_PORT                          443
          #define HTTP_URL                           "firmware.rps" //firmware file name to download
          #define USERNAME                           ""
          #define PASSWORD                           ""
          char *hostname                             ="example.s3.ap-south-1.amazonaws.com";
          ```

          > **Note:** The `USERNAME` and `PASSWORD` is provided as empty string "" because the S3 bucket URL that was created has public access provided. Refer to [Configuring AWS S3 Bucket](#configuring-an-aws-s3-bucket) section on how to upload Firmware in AWS S3 Bucket.

          - For Private resource: While trying to download the private resource, make sure to create the pre-signed URL and use it in the HTTP_URL as shown below.

          > Example: For a presigned URL
          "<https://example.s3.us-east-1.amazonaws.com/m4_binary/wifi_wlan_throughput_soc_isp.bin?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAQRVCCU6GSVCJRSZD%2F20240812%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20240812T092007Z&X-Amz-Expires=3600&X-Amz-SignedHeaders=host&X-Amz-Signature=68696bd185a364a28a5a7189227ced33c2539c7ecd9790833b2c1d411d10bfac>",
          - **hostname**: `example.s3.us-east-1.amazonaws.com`
          - **HTTP_URL**: `m4_binary/wifi_wlan_throughput_soc_isp.bin?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAQRVCCU6GSVCJRSZD%2F20240812%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20240812T092007Z&X-Amz-Expires=3600&X-Amz-SignedHeaders=host&X-Amz-Signature=68696bd185a364a28a5a7189227ced33c2539c7ecd9790833b2c1d411d10bfac`

      - For **Azure Blob Storage**:

        - Include **DigiCert Global Root G2** certificate file for SSL connection.
    
          > **Note:** The `silabs_dgcert_ca.pem.h` file is included in the SDK as a linear array and can be used directly for SSL connections to Azure Blob Storage.

          > **Deprecation Notice:** The Baltimore CyberTrust Root certificate is being deprecated by Azure. Devices must use DigiCert Global Root G2 and Microsoft RSA Root Certificate Authority 2017 to maintain connectivity after September 30, 2024. See [Azure TLS certificate migration](https://learn.microsoft.com/azure/iot-hub/migrate-tls-certificate) for details.

        - Extract the hostname from the Azure Blob Storage URL `https://<Your-Azure-Storage-Account-name>.blob.core.windows.net/<Your-container-name>/firmware.rps` and provide it in **hostname**.

          > Example: For Azure Blob Storage URL `https://example1.blob.core.windows.net/example2/firmware.rps`, the hostname will be "example..blob.core.windows.net".

        - Extract the firmware package name from the URL `<Your-Azure-Storage-Account-name>.blob.core.windows.net/<Your-container-name>/firmware.rps` and provide it in **HTTP_URL**.

          > Example: For Azure Blob Storage URL `https://example1.blob.core.windows.net/example2/firmware.rps`, the HTTP_URL will be "example2/firmware.rps".

        - Configurations for Azure Blob Storage

          ```c
          //Sample configurations
          #include "silabs_dgcert_ca.pem.h"        //DigiCert Global Root G2 CA
          #define FLAGS                             HTTPS_SUPPORT
          #define HTTP_PORT                         443
          #define HTTP_URL                          "rps/firmware.rps" //Firmware file name to download
          #define USERNAME                          ""
          #define PASSWORD                          ""
          char *hostname                            ="example.blob.core.windows.net";
          ```

          > **Note:** The `USERNAME` and `PASSWORD` is provided as an empty string "" because the Azure Blob storage URL that was created has public access provided. Refer to [Configuring Azure Blob Storage](#configuring-azure-blob-storage) for information on how to upload firmware in Azure Blob storage.

  - Modify the **station_init_configuration** values from `app.c` as per the requirements below.

    - For **Apache HTTP Server**:

      ```c
      // station_init_configuration structure should contain the following configurations:
      .tcp_ip_feature_bit_map     = (TCP_IP_FEAT_DHCPV4_CLIENT | TCP_IP_FEAT_HTTP_CLIENT | TCP_IP_FEAT_EXTENSION_VALID)
      .ext_tcp_ip_feature_bit_map = EXT_FEAT_HTTP_OTAF_SUPPORT
      ```

    - For **Apache HTTPS Server**:

      ```c
      // station_init_configuration structure should contain the following configurations:
      .tcp_ip_feature_bit_map     = (TCP_IP_FEAT_DHCPV4_CLIENT | TCP_IP_FEAT_HTTP_CLIENT| TCP_IP_FEAT_EXTENSION_VALID | TCP_IP_FEAT_SSL)

      .ext_tcp_ip_feature_bit_map = EXT_FEAT_HTTP_OTAF_SUPPORT
      ```

    - For **AWS S3 Bucket** and **Azure Blob Storage**:

      ```c
      // station_init_configuration structure should contain the following configurations:
      .tcp_ip_feature_bit_map     = (TCP_IP_FEAT_DHCPV4_CLIENT | TCP_IP_FEAT_HTTP_CLIENT| TCP_IP_FEAT_EXTENSION_VALID | TCP_IP_FEAT_SSL | TCP_IP_FEAT_DNS_CLIENT)

      .ext_tcp_ip_feature_bit_map = (EXT_FEAT_HTTP_OTAF_SUPPORT | EXT_TCP_IP_SSL_16K_RECORD)
      ```

  - Modify the credential loading values from app.c as per the requirements below:
    - The **[sl_net_set_credential()](https://docs.silabs.com/wiseconnect/3.0.13/wiseconnect-api-reference-guide-nwk-mgmt/net-credential-functions#sl-net-set-credential)** API expects the certificate in the form of a linear array. Convert the pem certificate into linear array form using the python script provided in the SDK `<SDK>/resources/scripts/certificate_script.py`.

    - For example : If the certificate is ca-certificate.pem, enter the command in the following way:
    `python certificate_script.py ca-certificate.pem`
      - The script will generate ca-certificate.pem in which one linear array named ca-certificate contains the certificate.

    - Root CA certificate needs to be converted as mentioned above.

    - After the conversion, place the converted file in the `<SDK>/resources/certificates/` path and include the certificate file in ``app.c``

    - For **HTTPS Apache Server**:

      ```c
      // Certificate includes
      #include "ca-certificate.pem.h"
      
      // Load Security Certificates
      status = sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(0), SL_NET_SIGNING_CERTIFICATE, ca-certificate, sizeof(ca-certificate) - 1);
      ```

    - For **AWS Server**:

      ```c
      // Certificate includes
      #include "aws_starfield_ca.pem.h"

      // Load Security Certificates
      status = sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(0), SL_NET_SIGNING_CERTIFICATE, aws_starfield_ca, (sizeof(aws_starfield_ca) - 1));
      ```

    - For **Azure Server**:

      ```c
      // Certificate includes
      #include "silabs_dgcert_ca.pem.h"

      // Load Security Certificates
      status = sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(0), SL_NET_SIGNING_CERTIFICATE, silabs_dgcert_ca, (sizeof(silabs_dgcert_ca) - 1));
      ```

  >**Note:**
  > AWS has announced that there will be changes in their root CA chain. More details can be found in the reference links [https://aws.amazon.com/blogs/security/acm-will-no-longer-cross-sign-certificates-with-starfield-class-2-starting-august-2024/](https://aws.amazon.com/blogs/security/acm-will-no-longer-cross-sign-certificates-with-starfield-class-2-starting-august-2024/).
  > We are providing both root CAs (Starfield class-2 and Starfield G2) in aws_starfield_ca.pem.h, which is located in the WiSeConnect directory `<SDK>/resources/certificates/aws_starfield_ca.pem.h`.
  > Alternate certification chains support is added. With this, as opposed to requiring full chain validation, only the peer certificate must validate to a trusted certificate. This allows loading intermediate root CAs as trusted.
  > The default CA certificate is the Starfield Combined CA certificate. To use the Intermediate Amazon Root CA 1 certificate, define the `SL_SI91X_AWS_IOT_ROOT_CA1` macro in the application.

> **Note**: For recommended settings, please refer the [recommendations guide](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-prog-recommended-settings/).

## Test the Application

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

1. Build the application.
2. Flash, run, and debug the application.

   - Application prints with AWS S3 bucket

     ![Run](resources/readme/application_prints_aws.png)

   - Application prints with Azure Blob Storage

     ![Run](resources/readme/application_prints_azure.png)

   - Application prints with Apache Server

     ![Run](resources/readme/application_prints_apache.png)

## Additional Information

### Configuring an AWS S3 Bucket

1. Sign into the Amazon S3 console at <https://console.aws.amazon.com/s3/>.
2. Choose **Create bucket**.

   ![Create bucket](resources/readme/image388.png)

3. Enter a bucket name.

   ![Bucket name](resources/readme/image389.png)

4. Enable ACLs.

   ![ACL](resources/readme/aws_bucket_acl_enable.png)

5. Under **Bucket settings for Block Public Access**, uncheck **Block all public access**.

   ![Block all public access](resources/readme/aws_bucket_public_access.png)

6. Under **Bucket Versioning**, select **Enable** to keep all versions in the same bucket.

   ![Select Enable to keep all versions in the same bucket](resources/readme/image391.png)

7. Choose **Create bucket**.

   ![Create bucket](resources/readme/image392.png)

8. Choose the created bucket from the list of buckets in the console.

9. Upload the file in creating bucket.

   ![Upload the file](resources/readme/image393.png)

10. Add the file to the bucket.

    ![Add the file to the bucket](resources/readme/image394.png)

11. Set permission to public access.

    ![Setting permission to public access](resources/readme/image395.png)

12. After uploading the file, click on the file.

    ![S3_URL_1](resources/readme/aws_bucket_getting_url_1.png)

13. Get the object URL inside bucket/properties, like below:

    ![S3_URL_2](resources/readme/aws_bucket_getting_url_2.png)

14. To get the presigned URL, go to **Object actions** tab as shown below and select the option **Share with a presigned URL** as follows:

    ![Presigned_URL](resources/readme/Presigned_URL.png)

15. Now select the **Time interval** for accessing the presigned URL as follows:

    ![Presigned_URL_1](resources/readme/Time_interval_presigned_URL.png)

16. Once the presigned URL is created, it is copied to our clipboard as shown below:

    ![Presigned_URL_2](resources/readme/Presigned_URL_created.png)

### Configuring Azure Blob Storage

1. Login to your Azure account and go to **Storage Account** or search for **Storage Account**.

   ![Search for Storage Account](resources/readme/image396.png)

2. Open storage account and create a new storage.

   ![Create a new storage](resources/readme/image397.png)

3. While creating a storage account, select your common Resource Group you have already created and provide a storage account name.
4. Select preferred location. For the account kind, select Blob-Storage and Replication select LRS.

   ![Select preferred location](resources/readme/image398.png)

5. Review and create your storage account.
6. Now download the Windows Storage Explorer here.
7. After installing the storage explorer, open **Azure Storage Explorer** in your Windows machine and navigate to Account management and add your Azure account.

   ![Open Azure Storage Explorer](resources/readme/image399.png)

   ![Add your Azure account](resources/readme/image400.png)

8. Click on the **Open connect** dialog, where you need to select a resource from the list as shown below.

   ![Select a Resource from the list](resources/readme/image401.png)

9. Select Storage account or service, then select connection method as **Connection String**.

   ![Select connection method as Connection String](resources/readme/image402.png)

10. In the Azure Portal, navigate to your newly created storage account and select **Access Keys**. Copy the connection string for Key1.

    ![Copy the connection string for Key1](resources/readme/image403.png)

11. The connection string has to be given in the local Azure Storage Explorer app.
12. Upon successfully adding, you should now see the EXPLORER tab on your Azure Storage Explorer displaying all the storages available in your account.

    ![Display all the storages available in your account](resources/readme/image404.png)

13. In the Azure Portal, search for **Storage Explorer** and perform the same steps as above. However, this option is in preview, so it is better to use Windows Azure Storage Explorer.
14. Create a new blob container as shown below:

    ![Create a new blob container](resources/readme/image405.png)

15. The route folder name you provide is quite important as all the further connections happen from here. For this example, we chose a file extension. The name used here is “rps”.
16. This should create a new folder, which looks like this:

    ![New folder looks like this](resources/readme/image406.png)

17. Change the Public Access Level by right-clicking on the new folder and selecting **Set Container Access Level**.

    ![Select Set Container Access Level](resources/readme/image407.png)

18. We can upload the Device Update File:

    ![Upload the Device Update File](resources/readme/image408.png)

19. Once done uploading, we can see the file:

    ![We can see the file](resources/readme/image409.png)

20. Right-click on the uploaded file, then select properties. You will find a URL path.

    ![Find a URL path](resources/readme/image410.png)

21. Copy the URL path as this link is used for accessing our device update files.

    ![This link is used for accessing our device update files](resources/readme/image411.png)

22. By accessing this URL, you can download the **Device Update** files in the application.

### Configuring and Uploading Firmware on Apache HTTP

1. Download and Install Wamp-Apache Server
    - Open the following link in your system browser:
      Wamp Server - (<https://www.wampserver.com/en/>)
    - Under Downloads, download the latest version of WAMP server for 32-bit or 64-bit machine.
    - Install Wamp-Apache server with all the default settings.
    - Make sure the Wamp-Apache server is present in the C:\ directory.
    - Configure an HTTP server.
    - Navigate to C:\wamp64\bin\apache\apache2.4.46\conf.
    - Open httpd.conf file with an editor.
    - Change the following lines into system IP address:

      ```sh
      Listen {System-IP-Address}:80
      ServerName {System-IP-Address}:80
      Eg: Listen 192.168.1.4:80
          ServerName 192.168.1.4:80
      ```

2. Save the file and exit.
3. Open command prompt and run with Administrator Privileges.
4. Navigate to directory C:\wamp64\bin\apache\apache2.4.46\bin.
5. Add Apache as a Windows Service:

    `httpd.exe -k install`

6. While the install is in progress, you will be directed to the Windows Network Access page as shown below. Make sure you allow both Private and Public network access.

   ![Windows Network Access page](resources/readme/image412.png)

7. Start Apache Service in Windows:
    - Open RUN in windows using WIN+R button.
    - Input "services.msc" into RUN.
    - This will open your Windows System Services.
    - In the list of services running, you can find Apache2.x.
    - Start the service as shown below:

    ![Start the service](resources/readme/image413.png)

8. Now that your Apache has started and is running, check it by using your browser. Open a Web browser and type the machine IP in the address bar and hit Enter. You should see the window below, if the server has started successfully.

   ![If server has started successfully.](resources/readme/image414.png)

   As you can see, the connection is "Not Secure", meaning it is running the HTTP server.
9. Configure the HTTP Wamp-Apache Server to Download firmware:
   - Go to the Wamp Root directory. In this example, it is C:\wamp64 and navigate to "www" folder C:\wamp64\www.
   - Create a new folder in that directory. In this example, I created a folder named "Firmware" [Folder Structure: C:\wamp64\www\Firmware].
   - In the "Firmware" folder, create an "index.html" file and write the following contents to the file.

      ```html
          <!DOCTYPE html>
          <html>
              <body>
                  <h2>Using a Relative File Path</h2>
                  <a href="firmware.rps" download>Download_Version_6</a><br><br>
                  <a href="firmware1.rps" download>Download_Version_4</a>
              </body>
          </html>
      ```

     This code will link your resources to the Apache server so those files can be downloaded.

     You can edit href values in the index.html to your firmware file names.
10. Make sure to copy all the firmware files into the present directory, C:\wamp64\www\Firmware. Save the file and Exit.

    ```html
    <a href="<your-firmware-file>-1.rps" download>Download_Version_6</a><br><br>
    <a href="<your-firmware-file>-2.rps" download>Download_Version_4</a>
    ```

11. Configure the HTTPD.conf file for Wamp-Apache Server.
    - Open httpd.conf file in C:\wamp64\bin\apache\apache2.4.46\conf\httpd.conf.
    - Search or Find "DocumentRoot" and change it to the following configuration. Save the file and Exit.

      ```sh
      "${INSTALL_DIR}/www/Firmware"
      ```

12. Restart Apache Service
    - Open Windows services, "WIN+R" → "services.msc" → ENTER.
    - Check for Apache service and restart the service.
    - In the above configuration, we have created a resource for our server in the "Firmware" folder.
    - Our access resource URL looks as shown in following:

      `http://<your-ip-address>/<Sub-Resource-Directory>/<Resources>`
      >
      > For example:
      >- <http://192.168.1.4/Firmware/firmware.rps>
      >- <http://192.168.1.4/Firmware/firmware1.rps>

13. Entering `http://<your-ip-address>/<Sub-Resource-Directory>` in the browser should load the window shown in following image. Clicking on any link should download the firmware files.

    ![Webpage in browser](resources/readme/image415.png)

14. Get the resource information and test with application.
    - In the SiWx91x FOTA application, make the changes below and test the application; this should start downloading firmware.

      ```c
      #define HTTP_SERVER_IP_ADDRESS "192.168.1.4"              //Replace this values related to your requirements
      #define HTTP_URL               "Firmware/firmware_file.rps"          //HTTP GET request resource name
      #define HTTP_HOSTNAME          "192.168.1.4"//Replace this values related to your requirements
      ```

       > **Warning:** Make sure that you are able to access the WAMP-Apache Server (with its IP Address), and that it is accessible to other systems in the same network. If not, follow the Changing PHP Configurations. Otherwise, proceed with the next steps.

15. Changing PHP Configuration
    - Complete the following steps only if you are unable to access the WAMP-Server from other machines in the network, as shown in the image below:

      ![Issue of not able to access the WAMP-Server from other machines](resources/readme/image416.png)

    - Give permissions to the newly created resource `"<Sub-Resource-Directory>"`.
    - Open "phpmyadmin.conf" file in "C:\wamp64\alias\phpmyadmin.conf".
    - Add the line shown in the following image to the end of the "phpmyadmin.conf" file. Save and exit the file.

      ```sh
      <Directory "c:/wamp64/www/<Sub-Resource-Directory>/">

      For example,
      <Directory "c:/wamp64/www/Firmware/">
      Options +Indexes +FollowSymLinks +MultiViews
      AllowOverride all
      Require all granted
      </Directory>
      ```

16. Restart the Apache service and now the resource should be accessible from any systems connected in the same network.

> **Note:** Even if you are not able to access the `http://<your-ip-address>/` page directly, you will be able to access your resources from here: `http://<your-ip-address>/<Sub-Resource-Directory>/`.

### Configuring and Uploading Firmware on Apache HTTPs

The HTTPs server configuration for Apache requires the Wamp server. If you have not installed it, follow the procedures in **"Step 1: Download and Install Wamp-Apache Server"** and continue with the HTTPS steps in this document.

1. Download and install OPENSSL for Windows:
   - OpenSSL for windows from here (<https://slproweb.com/products/Win32OpenSSL.html>).
   - Choose default install for OpenSSL.
   - We can only run OpenSSL using command prompt, and for that we need to first find the openssl.exe file.
   - Normally, it will be in "C:\Program Files\OpenSSL-Win64\bin\openssl.exe".

2. Generate required certs:
   > **Note:** If you already have the required certs to run the server, you can skip the **Generate required certs** step. Copy your certs to `C:\wamp64\bin\apache\apache2.4.46\conf` directory and update the `httpd-ssl.conf` file with the certificate paths shown in the **HTTPD configuration** step.

   - Open Command Prompt in Administrator privileges.
   - Change directory to your openssl.exe file "cd C:\Program Files\OpenSSL-Win64\bin\".
   - Execute the following command to generate a private.key file with AES 256 encryption.

      ```sh
      openssl.exe genrsa -aes256 -out private.key 2048
      openssl.exe rsa -in private.key -out private.key
      openssl.exe req -new -x509 -nodes -sha1 -key private.key -out certificate.crt -days 36500 -config <your-wamp-apache-openssl.conf-file-path>
      openssl.exe req -new -x509 -nodes -sha1 -key private.key -out certificate.crt -days 36500 -config C:\wamp64\bin\apache\apache2.4.46\conf\openssl.conf
      ```

   - Now there will be two files created [Private.key and certificate.crt] in the "C:\Program Files\OpenSSL-Win64\bin\" directory. Copy them to "C:\wamp64\bin\apache\apache2.4.46\conf".

3. Update HTTPD configuration:
    - Open the ``httpd.conf`` file in "C:\wamp64\bin\apache\apache2.4.46\conf".
    - Uncomment the lines shown below in that file. Save and Exit.

      ```sh
      LoadModule ssl_module modules/mod_ssl.so
      Include conf/extra/httpd-ssl.conf
      LoadModule socache_shmcb_module modules/mod_socache_shmcb.so
      ```

    - Open "php.ini" file in "C:\wamp64\bin\php\php5.6.40" and uncomment the following line in the file: 
    
      `extension=php_openssl.dll`
            
    - Open the ``httpd-ssl.conf`` file in "C:\wamp64\bin\apache\apache2.4.46\conf\extra" and update the below paths with proper information (i.e., provide system relative paths).

      ``` sh
      <VirtualHost default:443>
      General setup for the virtual host
      DocumentRoot "C:/wamp64/www"
      ServerName 192.168.43.85:443
      ServerAdmin admin@example.com
      ErrorLog "C:/wamp64/bin/apache/apache2.4.46/logs/error.log"
      TransferLog "C:/wamp64/bin/apache/apache2.4.46/logs/access.log"

      SSLCertificateFile "C:/wamp64/bin/apache/apache2.4.46/conf/certificate.crt"
      SSLCertificateKeyFile "C:/wamp64/bin/apache/apache2.4.46/conf/private.key" 
      ```

    - Run the command below to check if the configurations given above are correct. If the configurations are correct, the command will return "Syntax OK".

      `httpd.exe -t`

4. Configure HTTPS Wamp-Apache Server to download firmware:
    - Go to the Wamp Root directory "C:\wamp64" and navigate to "www".
    - Create a new folder in that directory "Firmware". [Folder Structure: C:\wamp64\www\Firmware]
    - In the "Firmware" folder, create an "index.html" file and write the following contents to the file:

      ```html
      <!DOCTYPE html>
       <html>
        <body>
          <h2>Using a Relative File Path</h2>
          <a href="firmware.rps" download>Download_Version_6</a><br><br>
          <a href="firmware1.rps" download>Download_Version_4</a>
        </body>
      </html>
      ```

    - This code will link your resources to the Apache server so that those files can be downloaded.

    - Make sure to copy all the firmware files into the present directory, C:\wamp64\www\Firmware.

    - You can edit the href values in the index.html to your firmware file names.

      ```html
      <a href="<your-firmware-file>-1.rps" download>Download_Version_6</a><br><br>
      <a href="<your-firmware-file>-2.rps" download>Download_Version_4</a>
      ```

    - Save the file and Exit.
5. Restart the server:
    - Open RUN, "WIN+R" → "services.msc" → ENTER.
    - Restart the Apache service.
    - Open your browser and enter your Apache server URL.

      ![Give your Apache server URL](resources/readme/image418.png)

    - Click on **Advanced Settings** and then click on **Proceed to 192.168.43.85 (unsafe)**.
    - You will be able to access the page and resources in HTTPS.

      ![Access the page and resources in HTTPS](resources/readme/image419.png)

    > **Note:** Make sure to check your HTTPS server from other local machines present in the same network. It should be accessible.

    > **NOTE :** Support for the SNI extension has been added to the AWS SDK, ensuring it is set by the client when connecting to an AWS server using TLS 1.3. This is handled internally by the AWS SDK and does not affect compatibility with other TLS versions.


