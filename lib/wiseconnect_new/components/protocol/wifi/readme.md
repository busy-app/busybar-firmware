<!-- Parent: Test -->
<!-- Title: SL Wi-Fi Internal Documentation -->

# SL Wi-Fi API

API files

 inc/sl_wifi.h

# Client connect API Sequence Diagram

```mermaid
sequenceDiagram
    participant AppThread
    participant WiFiLib
    participant RTOS
    participant HostInterface
    participant TargetDevice

    AppThread->>WiFiLib: sl_wifi_connect
    rect rgb(255, 200, 180)

    WiFiLib->>WiFiLib: Check if interface is valid
    alt if Interface is Invalid
        WiFiLib->>AppThread: Return Error
    end

    WiFiLib->>WiFiLib: Check if interface is Up
    alt if Interface is down
        WiFiLib->>AppThread: Return Error
    end

    WiFiLib->>WiFiLib: Check the state of command processor
    alt if state is not to allow
        WiFiLib->>AppThread: Return Busy Error
    end

    WiFiLib->>WiFiLib: Change state to in use
    WiFiLib->>WiFiLib: Fetch the credential using given credential ID

    alt if unable to fetch credential using given credential ID
        WiFiLib->>AppThread: Return Invalid argument error 
    end

    WiFiLib->>WiFiLib: Check the status of given interface
    alt if interface is already connected
        WiFiLib->>AppThread: Return Invalid operation Error
    end
    end

    WiFiLib->>RTOS: Queue Scan_Without_Results command
    Note over WiFiLib: Wait on semaphore
    Note over RTOS: DeQueue Scan_Without_Results command
    RTOS->>HostInterface: Send Scan_Without_Results command
    HostInterface->>HostInterface: Serialize the command packet
    HostInterface->>TargetDevice: Send the command on SPI Bus
    Note over TargetDevice: Scan for existing networks <br />Return status over SPI bus
    TargetDevice->>HostInterface: Send the return status on SPI Bus
    HostInterface->>RTOS: ISR for cmd response
    Note over RTOS: Update the Scan_Without_Results<br />command status with the<br />status  returned by device
    Note over RTOS: Update the semaphore
    RTOS-->>WiFiLib: Wake up semaphore
    Note over WiFiLib: Check the status of the command
    alt if the command failed
        WiFiLib->>AppThread: Return error status
    end
    Note over WiFiLib: Check the security type

    rect rgb(200, 200, 255)
    alt if security type is Enterprise security
        Note over WiFiLib: Create enterprise security<br />config packet with given<br />credentials
        WiFiLib->>RTOS: Queue EAP_Config command
        Note over WiFiLib: Wait on semaphore
        RTOS->>HostInterface: DeQueue EAP_Config command
        HostInterface->>HostInterface: Serialize the command packet
        HostInterface->>TargetDevice: Send the command on SPI Bus
        TargetDevice->>TargetDevice: Store the EAP Configuration
        TargetDevice->>HostInterface: Send the return status on SPI Bus
        HostInterface->>RTOS: ISR for cmd response
        Note over RTOS: Update the EAP_Config<br />command status with the<br />status returned by device
        Note over RTOS: Update the semaphore
        RTOS-->>WiFiLib: Wake up semaphore
        Note over WiFiLib: Check the status of the command
        alt if the command failed
            WiFiLib->>AppThread: Return error status
        end
    end
    end

    rect rgb(200, 255, 200)
    alt else if security type is WPS PIN and not in Enterprise security operation mode
        Note over WiFiLib: Create WPS method<br />config packet with given<br />credentials
        WiFiLib->>RTOS: Queue WPS_Config command
        Note over WiFiLib: Wait on semaphore
        RTOS->>HostInterface: DeQueue WPS_Config command
        HostInterface->>HostInterface: Serialize the command packet
        HostInterface->>TargetDevice: Send the command on SPI Bus
        TargetDevice->>TargetDevice: Store the WPS Configuration
        TargetDevice->>HostInterface: Send the return status on SPI Bus
        HostInterface->>RTOS: ISR for cmd response
        Note over RTOS: Update the WPS_Config<br />command status with the<br />status returned by device
        Note over RTOS: Update the semaphore
        RTOS-->>WiFiLib: Wake up semaphore
        Note over WiFiLib: Check the status of the command
        alt if the command failed
            WiFiLib->>AppThread: Return error status
        end
    end
    end

    rect rgb(255, 200, 255)
    alt else if security type is WEP
        Note over WiFiLib: Create WEP config<br />packet with given<br />credentials
        WiFiLib->>RTOS: Queue WEP_Config command
        Note over WiFiLib: Wait on semaphore
        RTOS->>HostInterface: DeQueue WEP_Config command
        HostInterface->>HostInterface: Serialize the command packet
        HostInterface->>TargetDevice: Send the command on SPI Bus
        TargetDevice->>TargetDevice: Store the WEP Configuration
        TargetDevice->>HostInterface: Send the return status on SPI Bus
        HostInterface->>RTOS: ISR for cmd response
        Note over RTOS: Update the WEP_Config<br />command status with the<br />status returned by device
        Note over RTOS: Update the semaphore
        RTOS-->>WiFiLib: Wake up semaphore
        Note over WiFiLib: Check the status of the command
        alt if the command failed
            WiFiLib->>AppThread: Return error status
        end
    end
    end

    rect rgb(200, 255, 255)
    alt else if security type is WPA
        Note over WiFiLib: Create WPA config<br />packet with given<br />credentials
        WiFiLib->>RTOS: Queue WPA_Config command
        Note over WiFiLib: Wait on semaphore
        RTOS->>HostInterface: DeQueue WPA_Config command
        HostInterface->>HostInterface: Serialize the command packet
        HostInterface->>TargetDevice: Send the command on SPI Bus
        TargetDevice->>TargetDevice: Store the WPA Configuration
        TargetDevice->>HostInterface: Send the return status on SPI Bus
        HostInterface->>RTOS: ISR for cmd response
        Note over RTOS: Update the WPA_Config<br />command status with the<br />status returned by device
        Note over RTOS: Update the semaphore
        RTOS-->>WiFiLib: Wake up semaphore
        Note over WiFiLib: Check the status of the command
        alt if the command failed
            WiFiLib->>AppThread: Return error status
        end
    end
    end

    Note over WiFiLib: Create a join command packet
    Note over WiFiLib: Fill the security type
    Note over WiFiLib: Fill the ssid in join command packet
    WiFiLib->>RTOS: Queue join command
    Note over RTOS: DeQueue join command
    RTOS->>HostInterface: Send join command
    HostInterface->>HostInterface: Serialize the command packet
    HostInterface->>TargetDevice: Send the command on SPI Bus
    Note over TargetDevice: Try to join the specified ssid<br />Return status over SPI bus
    TargetDevice->>HostInterface: Send the return status on SPI Bus
    HostInterface->>RTOS: ISR for cmd response

    RTOS->>WiFiLib: Invoke event handler with connect status

    alt if default callback handler is present
        WiFiLib->>AppThread: Invoke Connect status handler
    end

    alt if default callback handler is not present
        WiFiLib->>AppThread: Invoke event handler with connect status
    end
```

# Client disconnect API Sequence Diagram

```mermaid
sequenceDiagram
    participant AppThread
    participant WiFiLib
    participant RTOS
    participant HostInterface
    participant TargetDevice

    AppThread->>WiFiLib: sl_wifi_connect
    rect rgb(255, 200, 180)

    WiFiLib->>WiFiLib: Check if interface is valid
    alt if Interface is Invalid
        WiFiLib->>AppThread: Return Error
    end

    WiFiLib->>WiFiLib: Check if interface is Up
    alt if Interface is down
        WiFiLib->>AppThread: Return Error
    end

    WiFiLib->>WiFiLib: Check the state of command processor
    alt if state is not to allow
        WiFiLib->>AppThread: Return Busy Error
    end

    WiFiLib->>WiFiLib: Change state to in use

    Note over WiFiLib: Check the status of given interface
    alt if interface is not connected
        WiFiLib->>AppThread: Return Invalid operation Error
    end
    end

    rect rgb(200, 255, 200)
    Note over WiFiLib: Create a Disconnect command packet
    WiFiLib->>RTOS: Queue REQ_DISCONNECT command
    Note over RTOS: DeQueue REQ_DISCONNECT command
    RTOS->>HostInterface: Send REQ_DISCONNECT command
    HostInterface->>HostInterface: Serialize the command packet
    HostInterface->>TargetDevice: Send the command on SPI Bus
    Note over TargetDevice: Try to disconnect from the AP<br />Return status over SPI bus
    TargetDevice->>HostInterface: Send the return status on SPI Bus
    HostInterface->>RTOS: ISR for cmd response

    RTOS->>WiFiLib: Invoke event handler with connection status
    end

    alt if default callback handler is present
        WiFiLib->>AppThread: Invoke Connection status handler with connection<br />status as disconnected
    end

    alt if default callback handler is not present
        WiFiLib->>AppThread: Invoke event handler with connection<br />status as disconnected
    end
```

# Client Get Signal Strength API Sequence Diagram

```mermaid
sequenceDiagram
    participant AppThread
    participant WiFiLib
    participant RTOS
    participant HostInterface
    participant TargetDevice

    AppThread->>WiFiLib: sl_wifi_connect
    rect rgb(255, 200, 180)

    WiFiLib->>WiFiLib: Check if interface is valid
    alt if Interface is Invalid
        WiFiLib->>AppThread: Return Error
    end

    WiFiLib->>WiFiLib: Check if rssi parameter is valid
    alt if rssi parameter is null
        WiFiLib->>AppThread: Return Error
    end

    WiFiLib->>WiFiLib: Check if interface is Up
    alt if Interface is down
        WiFiLib->>AppThread: Return Error
    end

    WiFiLib->>WiFiLib: Check the state of command processor
    alt if state is not to allow
        WiFiLib->>AppThread: Return Busy Error
    end

    WiFiLib->>WiFiLib: Change state to in use

    Note over WiFiLib: Check the status of given interface
    alt if interface is not connected
        WiFiLib->>AppThread: Return Invalid operation Error
    end
    end

    rect rgb(200, 255, 200)
    Note over WiFiLib: Create a REQ_RSSI command packet
    WiFiLib->>RTOS: Queue REQ_RSSI command
    Note over WiFiLib: Wait on semaphore
    Note over RTOS: DeQueue REQ_RSSI command
    RTOS->>HostInterface: Send REQ_RSSI command
    HostInterface->>HostInterface: Serialize the command packet
    HostInterface->>TargetDevice: Send the command on SPI Bus
    Note over TargetDevice: Fetch the RSSI of connected AP<br />Return status over SPI bus
    TargetDevice->>HostInterface: Send the return status on SPI Bus
    HostInterface->>RTOS: ISR for cmd response
    Note over RTOS: Update the EAP_Config<br />command status with the<br />status returned by device
    Note over RTOS: Update the semaphore
    RTOS-->>WiFiLib: Wake up semaphore
    Note over WiFiLib: Collect the status of the command
    end

    Note over WiFiLib: Store the rssi value in the user provided rssi pointer
    WiFiLib->>WiFiLib: Change state to allow
    WiFiLib->>AppThread: Return status of the request
```

# Wi-Fi Host Interface

The Wi-Fi driver can make very few assumptions about the host system it is running on and so is designed to work in both single-threaded and multi-threaded environments, and will operate on devices from multiple vendors.

## Host buffers

The Wi-Fi driver has no knowledge of the buffer management system used by the host and interacts with the host using "buffer tokens" and a small number of buffer related API.

 sl_status_t wifi_host_allocate_buffer(wifi_host_buffer_t *buffer, sl_wifi_buffer_type_t type, uint32_t buffer_size, uint32_t wait_duration_ms);
 void* wifi_host_get_buffer_data( wifi_host_buffer_t buffer, uint16_t offset, uint16_t* data_length, uint16_t* total_length );
 sl_status_t wifi_host_free_buffer(wifi_host_buffer_t buffer, sl_wifi_buffer_type_t type);
 
The driver assumes that buffers can be chained together and may need to query multiple times to retrieve all the data represented by a buffer token.

# Sequence diagrams for wifi common apis

## sl_wifi_enable_multicast_address and sl_wifi_disable_multicast_address

```mermaid
 sequenceDiagram 
     #sl_status_t sl_wifi_enable_multicast_address( sl_wifi_interface_t interface, const sl_mac_address_t* mac)
     #sl_status_t sl_wifi_disable_multicast_address( sl_wifi_interface_t interface, const sl_mac_address_t* mac)
  participant app as Application
  participant wifi as SL WIFI
  participant bus as Bus thread  
  participant fw as Firmware
        
  app ->> wifi: sl_wifi_enable_multicast_address

  alt  If device is not in right state
   wifi ->> app: return error
  else Interface is invalid
   wifi ->> app: return error
  else mac is NULL
   wifi ->> app: return error
  else
   Note over wifi: Construct enable multicast address command packet with timeout = ?? 
   wifi ->> fw:Send RSI_WLAN_REQ_MULTICAST device command

            par Firmware to Bus thread
                Note over bus: Waiting for NCP_HOST_RX_EVENT and rx pending packet
                fw -->> bus: Send NCP_HOST_RX_EVENT event
                alt if interrupt status SLI_RX_PKT_PENDING received and queue_id == SLI_WLAN_MGMT_Q and frameType == SLI_WLAN_RSP_MULTICAST
                    Note over bus: Add buffer to NCP_COMMAND_RESPONSE_QUEUE and set event to NCP_HOST_COMMAND_RESPONSE_EVENT
                end
            end

   par Bus thread to SL WIFI
    Note over wifi: Waiting for NCP_HOST_COMMAND_RESPONSE_EVENT event
    bus -->> wifi: Send NCP_HOST_COMMAND_RESPONSE_EVENT event
    alt if command success
     Note over wifi: Free command response buffer
     wifi ->> app: return success
    else 
     wifi ->> app: Return Error
    end
   end
  end

  app ->> wifi: sl_wifi_disable_multicast_address

  alt  If device is not in right state
   wifi ->> app: return error
  else Interface is invalid
   wifi ->> app: return error
  else mac is NULL
   wifi ->> app: return error
  else
   Note over wifi: Construct enable multicast address command packet with timeout = ?? 
   wifi ->> fw:Send RSI_WLAN_REQ_MULTICAST device command

            par Firmware to Bus thread
                Note over bus: Waiting for NCP_HOST_RX_EVENT and rx pending packet
                fw -->> bus: Send NCP_HOST_RX_EVENT event
                alt if interrupt status SLI_RX_PKT_PENDING received and queue_id == SLI_WLAN_MGMT_Q and frameType == SLI_WLAN_RSP_MULTICAST
                    Note over bus: Add buffer to NCP_COMMAND_RESPONSE_QUEUE and set event to NCP_HOST_COMMAND_RESPONSE_EVENT
                end
            end

   par Bus thread to SL WIFI
    Note over wifi: Waiting for NCP_HOST_COMMAND_RESPONSE_EVENT event
    bus -->> wifi: Send NCP_HOST_COMMAND_RESPONSE_EVENT event
    alt if command success
     Note over wifi: Free command response buffer
     wifi ->> app: return success
    else 
     wifi ->> app: Return Error
    end
   end
  end 
```

## sl_wifi_get_firmware_version

```mermaid
 sequenceDiagram 
     #sl_status_t sl_wifi_get_firmware_version( sl_wifi_firmware_version_t* version )
  participant app as Application
  participant wifi as SL WIFI
  participant bus as Bus thread  
  participant fw as Firmware

        
  app ->> wifi: sl_wifi_get_firmware_version

  alt if version is NULL
   wifi ->> app: return error
  else If device is not in right state
   wifi ->> app: return error
  else
   Note over wifi: Construct fw version command packet with timeout = ?? 
   wifi ->> fw:Send SLI_WLAN_REQ_FW_VERSION device command

            par Firmware to Bus thread
                Note over bus: Waiting for NCP_HOST_RX_EVENT and rx pending packet
                fw -->> bus: Send NCP_HOST_RX_EVENT event
                alt if interrupt status SLI_RX_PKT_PENDING received and queue_id == SLI_WLAN_MGMT_Q and frameType == RSI_WLAN_RSP_FW_VERSION
                    Note over bus: Add buffer to NCP_COMMAND_RESPONSE_QUEUE and set event to NCP_HOST_COMMAND_RESPONSE_EVENT
                end
            end

   par Bus thread to SL WIFI
    Note over wifi: Waiting for NCP_HOST_COMMAND_RESPONSE_EVENT event
    bus -->> wifi: Send NCP_HOST_COMMAND_RESPONSE_EVENT event
    alt if command success
     Note over wifi: Read the response from NCP_COMMAND_RESPONSE_QUEUE and copy it to user's provided memory
     Note over wifi: Free command response buffer
     wifi ->> app: return success
    else 
     wifi ->> app: Return Error
    end
   end
  end
```

## sl_wifi_get_mac_address

```mermaid
 sequenceDiagram 
     # sl_status_t sl_wifi_get_mac_address(sl_wifi_interface_t interface, sl_mac_address_t *mac)
  participant app as Application
  participant wifi as SL WIFI
  participant bus as Bus thread  
  participant fw as Firmware

  app ->> wifi: sl_wifi_get_mac_address

  alt  If device is not in right state
   wifi ->> app: return error
  else Interface is invalid
   wifi ->> app: return error
  else mac is NULL
   wifi ->> app: return error
  else
   Note over wifi: Construct get mac address command packet with timeout = ?? 
   wifi ->> fw:Send SLI_WIFI_REQ_MAC_ADDRESS device command

            par Firmware to Bus thread
                Note over bus: Waiting for NCP_HOST_RX_EVENT and rx pending packet
                fw -->> bus: Send NCP_HOST_RX_EVENT event
                alt if interrupt status SLI_RX_PKT_PENDING received and queue_id == SLI_WLAN_MGMT_Q and frameType == RSI_WLAN_RSP_MAC_ADDRESS
                    Note over bus: Add buffer to NCP_COMMAND_RESPONSE_QUEUE and set event to NCP_HOST_COMMAND_RESPONSE_EVENT
                end
            end

   par Bus thread to SL WIFI
    Note over wifi: Waiting for NCP_HOST_COMMAND_RESPONSE_EVENT event
    bus -->> wifi: Send NCP_HOST_COMMAND_RESPONSE_EVENT event
    alt if command success
     Note over wifi: Read mac address from buffer and copy it to user's buffer
     Note over wifi: Free command response buffer
     wifi ->> app: return success
    else 
     wifi ->> app: Return Error
    end
   end
  end
```

## sl_wifi_get_statistics

```mermaid
 sequenceDiagram 
     #sl_wifi_statistics_t sl_wifi_get_statistics(sl_wifi_interface_t interface);
  participant app as Application
  participant wifi as SL WIFI
  participant bus as Bus thread  
  participant fw as Firmware

        
  app ->> wifi: sl_wifi_get_statistics

  alt  If device is not in right state
   wifi ->> app: return default statistics
  else Interface is invalid
   wifi ->> app: return default statistics
  else
   Note over wifi: Construct get statistics command packet with timeout = ?? 
   wifi ->> fw:Send RSI_WLAN_REQ_GET_STATS device command

            par Firmware to Bus thread
                Note over bus: Waiting for NCP_HOST_RX_EVENT and rx pending packet
                fw -->> bus: Send NCP_HOST_RX_EVENT event
                alt if interrupt status SLI_RX_PKT_PENDING received and queue_id == SLI_WLAN_MGMT_Q and frameType == RSI_WLAN_RSP_GET_STATS
                    Note over bus: Add buffer to NCP_COMMAND_RESPONSE_QUEUE and set event to NCP_HOST_COMMAND_RESPONSE_EVENT
                end
            end

   par Bus thread to SL WIFI
    Note over wifi: Waiting for NCP_HOST_COMMAND_RESPONSE_EVENT event
    bus -->> wifi: Send NCP_HOST_COMMAND_RESPONSE_EVENT event
    alt if command success
     Note over wifi: Read statistics from buffer and copy it to user's buffer
     Note over wifi: Free command response buffer
     wifi ->> app: return statistics
    else 
     wifi ->> app: Return default statistics
    end
   end
  end
```

## sl_wifi_get_status

```mermaid
 sequenceDiagram 
     #sl_status_t sl_wifi_get_status(sl_wifi_status_t* wifi_status)
  participant app as Application
  participant wifi as SL WIFI
  participant bus as Bus thread  
  participant fw as Firmware

        
  app ->> wifi: sl_wifi_get_status

  alt  If device is not in right state
   wifi ->> app: return error
  else wifi_status is NULL
   wifi ->> app: return error
  else
   Note over wifi: Construct get status command packet with timeout = ?? 
   wifi ->> fw:Send RSI_WLAN_REQ_GET_STATUS device command

            par Firmware to Bus thread
                Note over bus: Waiting for NCP_HOST_RX_EVENT and rx pending packet
                fw -->> bus: Send NCP_HOST_RX_EVENT event
                alt if interrupt status SLI_RX_PKT_PENDING received and queue_id == SLI_WLAN_MGMT_Q and frameType == RSI_WLAN_RSP_GET_STATUS
                    Note over bus: Add buffer to NCP_COMMAND_RESPONSE_QUEUE and set event to NCP_HOST_COMMAND_RESPONSE_EVENT
                end
            end

   par Bus thread to SL WIFI
    Note over wifi: Waiting for NCP_HOST_COMMAND_RESPONSE_EVENT event
    bus -->> wifi: Send NCP_HOST_COMMAND_RESPONSE_EVENT event
    alt if command success
     Note over wifi: Read buffer and copy to status to user's buffer
     Note over wifi: Free command response buffer
     wifi ->> app: Return success
    else 
     wifi ->> app: Return Error
    end
   end
  end
```

## sl_wifi_is_interface_up

```mermaid
 sequenceDiagram 
     #bool sl_wifi_is_interface_up(sl_wifi_interface_t interface);
  participant app as Application
  participant wifi as SL WIFI
  participant bus as Bus thread  
  participant fw as Firmware
        
  app ->> wifi: sl_wifi_is_interface_up

  alt  If device is not in right state
   wifi ->> app: return false
  else Interface is invalid
   wifi ->> app: return false
  else
   Note over wifi: Construct is interface up command packet with timeout = ?? 
   wifi ->> fw:Send RSI_WLAN_REQ_GET_INTERFACE_STATUS device command

            par Firmware to Bus thread
                Note over bus: Waiting for NCP_HOST_RX_EVENT and rx pending packet
                fw -->> bus: Send NCP_HOST_RX_EVENT event
                alt if interrupt status SLI_RX_PKT_PENDING received and queue_id == SLI_WLAN_MGMT_Q and frameType == RSI_WLAN_RSP_GET_INTERFACE_STATUS
                    Note over bus: Add buffer to NCP_COMMAND_RESPONSE_QUEUE and set event to NCP_HOST_COMMAND_RESPONSE_EVENT
                end
            end

   par Bus thread to SL WIFI
    Note over wifi: Waiting for NCP_HOST_COMMAND_RESPONSE_EVENT event
    bus -->> wifi: Send NCP_HOST_COMMAND_RESPONSE_EVENT event
    alt if command success
     Note over wifi: Read interface status from buffer
     Note over wifi: Free command response buffer
     wifi ->> app: return interface status 
    else 
     wifi ->> app: Return false
    end
   end
  end
```

## sl_wifi_set_mac_address

```mermaid
 sequenceDiagram 
     #sl_status_t sl_wifi_set_mac_address(sl_wifi_interface_t interface, const sl_mac_address_t *mac)
  participant app as Application
  participant wifi as SL WIFI
  participant bus as Bus thread  
  participant fw as Firmware

        
  app ->> wifi: sl_wifi_set_mac_address

  alt  If device is not in right state
   wifi ->> app: return error
  else Interface is invalid
   wifi ->> app: return error
  else mac is NULL
   wifi ->> app: return error
  else
   Note over wifi: Construct set mac address command packet with timeout = ?? 
   wifi ->> fw:Send RSI_WLAN_REQ_SET_MAC_ADDRESS device command

            par Firmware to Bus thread
                Note over bus: Waiting for NCP_HOST_RX_EVENT and rx pending packet
                fw -->> bus: Send NCP_HOST_RX_EVENT event
                alt if interrupt status SLI_RX_PKT_PENDING received and queue_id == SLI_WLAN_MGMT_Q and frameType == SLI_WIFI_RSP_SET_MAC_ADDRESS
                    Note over bus: Add buffer to NCP_COMMAND_RESPONSE_QUEUE and set event to NCP_HOST_COMMAND_RESPONSE_EVENT
                end
            end

   par Bus thread to SL WIFI
    Note over wifi: Waiting for NCP_HOST_COMMAND_RESPONSE_EVENT event
    bus -->> wifi: Send NCP_HOST_COMMAND_RESPONSE_EVENT event
    alt if command success
     Note over wifi: Free command response buffer
     wifi ->> app: return success
    else 
     wifi ->> app: Return Error
    end
   end
  end
```

## sl_si91x_deinit

```mermaid
 sequenceDiagram 
     #sl_status_t sl_si91x_deinit(void)
  participant app as Application
  participant wifi as SL WIFI
        
  app ->> wifi: sl_si91x_deinit

  alt  Check for generic errors 
   wifi ->> app: return appropriate error
  else 
   Note over wifi: 1. Deinitialise si91x_event_handler 
   Note over wifi: 2. Call to sl_si91x_host_deinit()
   Note over wifi: 3. sl_si91x_host_delay_ms()
   Note over wifi: 4. Call to si91x_bus_deinit()
   Note over wifi: 5. Disable bus interrupt, sl_si91x_host_disable_bus_interrupt
   Note over wifi: 6. Disable device bus
   Note over wifi: 7. Disable host bus

  end
```