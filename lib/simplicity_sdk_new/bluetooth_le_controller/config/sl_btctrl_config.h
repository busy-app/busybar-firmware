#ifndef SL_BTCTRL_CONFIG_H
#define SL_BTCTRL_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Bluetooth Controller Configuration

// <o SL_BT_CONTROLLER_BUFFER_MEMORY> Bluetooth Controller Buffer Memory
// <i> Default: SL_BT_CONTROLLER_BUFFER_MEMORY
// <i> Define the Amount of memory to allocate for tx/rx buffers in Bluetooth Controller
#ifndef SL_BT_CONTROLLER_BUFFER_MEMORY
#define SL_BT_CONTROLLER_BUFFER_MEMORY     (8192)
#endif

// <o SL_BT_CONTROLLER_LE_BUFFER_SIZE_MAX> Bluetooth Controller ACL data packets that can be stored
// <i> Default: SL_BT_CONTROLLER_LE_BUFFER_SIZE_MAX
// <i> Define the total number of the maximum sized ACL data packets that can be received from the host
#ifndef SL_BT_CONTROLLER_LE_BUFFER_SIZE_MAX
#define SL_BT_CONTROLLER_LE_BUFFER_SIZE_MAX     (3)
#endif

// <o SL_BT_CONFIG_MAX_QUEUED_ADV_REPORTS> Maximum number of queued advertisement reports <1-255>
// <i> Default: 10
// <i> Define the maximum number of queued advertisement reports. Additional advertisement reports are dropped.
#ifndef SL_BT_CONFIG_MAX_QUEUED_ADV_REPORTS
#define SL_BT_CONFIG_MAX_QUEUED_ADV_REPORTS     (10)
#endif

// <q SL_BT_CONTROLLER_SCANNER_RECEPTION_EARLY_ABORT> Bluetooth Controller Scanner Reception Early Abort.
// <i> Default: Disabled
// <i> Enable or disable the scanner reception early abort feature.
// <i> This feature allows the controller to control the scanner to abort the reception of a packet if it will conflinct with another scheduled higher priority task.
#ifndef SL_BT_CONTROLLER_SCANNER_RECEPTION_EARLY_ABORT
#define SL_BT_CONTROLLER_SCANNER_RECEPTION_EARLY_ABORT (0)
#endif

// <o SL_BT_CONTROLLER_LINKLAYER_IRQ_PRIORITY> Linklayer interrupt priority in baremetal applications <1..7:1>
// <i> Default: 5
// <i> Define the ISR priority for linklayer interrupts in baremetal applications.
// <i> The value of SL_BT_CONTROLLER_LINKLAYER_IRQ_PRIORITY must be higher than SL_BT_CONTROLLER_RADIO_IRQ_PRIORITY.
// <i>
// <i> NOTE: SL_BT_CONTROLLER_LINKLAYER_IRQ_PRIORITY is only applicable in baremetal applications.
// <i> For RCP applications that include an RTOS, Link Layer thread priority is determined by SL_BTCTRL_RTOS_LINK_LAYER_TASK_PRIORITY,
// <i> defined in config/sl_btctrl_rtos_config.h.
// <i> For NCP/SoC applications that include an RTOS, Link Layer thread priority is determined by SL_BT_RTOS_LINK_LAYER_TASK_PRIORITY,
// <i> defined in config/sl_bt_rtos_config.h.
#ifndef SL_BT_CONTROLLER_LINKLAYER_IRQ_PRIORITY
#define SL_BT_CONTROLLER_LINKLAYER_IRQ_PRIORITY  (5)
#endif

// <o SL_BT_CONTROLLER_RADIO_IRQ_PRIORITY> Radio interrupt priority <1..7:1>
// <i> Default: 4
// <i> Define the ISR priority for radio interrupts.
#ifndef SL_BT_CONTROLLER_RADIO_IRQ_PRIORITY
#define SL_BT_CONTROLLER_RADIO_IRQ_PRIORITY      (4)
#endif

// <h> TX Power Levels

// <e SL_BT_CONTROLLER_MIN_POWER_LEVEL_OVERRIDE> Override Default Minimum Supported TX Power Level
// <i> Default: 0
// <i> Define the minimum radiated TX power level.
#ifndef SL_BT_CONTROLLER_MIN_POWER_LEVEL_OVERRIDE
#define SL_BT_CONTROLLER_MIN_POWER_LEVEL_OVERRIDE (0)
#endif

// <o SL_BT_CONTROLLER_MIN_POWER_LEVEL> Minimum radiated TX power level in 0.1dBm unit
// <i> Default: 0
// <i> Define the minimum radiated TX power level.
// <i> If the configured power level is lower than what the radio supports, the minimum supported level will be used.
// <i> For NCP and SoC applications, the TX power configuration is located in the Bluetooth Core component.
// <i> This value will nonetheless be applied, provided that SL_BT_CONTROLLER_MIN_POWER_LEVEL_OVERRIDE is enabled, and the corresponding minimum TX power value in the Bluetooth Core component remains at the default value.
// <i> For RCP applications, this value will be applied if SL_BT_CONTROLLER_MIN_POWER_LEVEL_OVERRIDE is enabled. If SL_BT_CONTROLLER_MIN_POWER_LEVEL_OVERRIDE is disabled, this value will be ignored and the minimum supported TX power level will be used.
#ifndef SL_BT_CONTROLLER_MIN_POWER_LEVEL
#define SL_BT_CONTROLLER_MIN_POWER_LEVEL (0)
#endif
// </e>

// <e SL_BT_CONTROLLER_MAX_POWER_LEVEL_OVERRIDE> Override Default Maximum Supported TX Power Level
// <i> Default: 0
// <i> Define the maximum radiated TX power level.
#ifndef SL_BT_CONTROLLER_MAX_POWER_LEVEL_OVERRIDE
#define SL_BT_CONTROLLER_MAX_POWER_LEVEL_OVERRIDE (0)
#endif

// <o SL_BT_CONTROLLER_MAX_POWER_LEVEL> Maximum radiated TX power level in 0.1dBm unit
// <i> Default: 0
// <i> Define the maximum radiated TX power level.
// <i> If the configured power level is higher than what the radio supports, the maximum supported level will be used.
// <i> For NCP and SoC applications, the TX power configuration is located in the Bluetooth Core component.
// <i> This value will nonetheless be applied, provided that SL_BT_CONTROLLER_MAX_POWER_LEVEL_OVERRIDE is enabled, and the corresponding maximum TX power value in the Bluetooth Core component remains at the default value.
// <i> For RCP applications, this value will be applied if SL_BT_CONTROLLER_MAX_POWER_LEVEL_OVERRIDE is enabled. If SL_BT_CONTROLLER_MAX_POWER_LEVEL_OVERRIDE is disabled, this value will be ignored and the maximum supported TX power level will be used.
#ifndef SL_BT_CONTROLLER_MAX_POWER_LEVEL
#define SL_BT_CONTROLLER_MAX_POWER_LEVEL (0)
#endif
// </e>

// </h> End TX Power Levels

// <h> Bluetooth Controller Configuration for LE Connection
// <o SL_BT_CONTROLLER_COMPLETED_PACKETS_THRESHOLD> Total transmitted packets threshold for all connections to send the Number Of Completed Packets HCI event to the host <1-255>
// <i> Default: 4
// <i> Define the number of transmitted air interface ACL packets to trigger the Number Of Completed Packets HCI event.
#ifndef SL_BT_CONTROLLER_COMPLETED_PACKETS_THRESHOLD
#define SL_BT_CONTROLLER_COMPLETED_PACKETS_THRESHOLD     (4)
#endif

// <o SL_BT_CONTROLLER_COMPLETED_PACKETS_EVENTS_TIMEOUT> Number of connection events to send the Number Of Completed Packets HCI event to the host <1-255>
// <i> Default: 3
// <i> Define the maximum number of connection events since the previous Number Of Completed Packets HCI event to trigger reporting of any unreported completed ACL packets.
#ifndef SL_BT_CONTROLLER_COMPLETED_PACKETS_EVENTS_TIMEOUT
#define SL_BT_CONTROLLER_COMPLETED_PACKETS_EVENTS_TIMEOUT     (3)
#endif

// <o SL_BT_CONTROLLER_CONN_EVENT_LENGTH_MIN> Bluetooth Controller Minimum Connection Event Duration
// <i> Default: SL_BT_CONTROLLER_CONN_EVENT_LENGTH_MIN
// <i> Define the minimum connection event length in 0.625 ms units. This value is used to provide a hint to the linklayer scheduler.
// <i> User could set a larger minimum connection event length when creating a connection or when updating connection parameters.
#ifndef SL_BT_CONTROLLER_CONN_EVENT_LENGTH_MIN
#define SL_BT_CONTROLLER_CONN_EVENT_LENGTH_MIN  (3)
#endif

// <q SL_BT_CONTROLLER_CONN_EVENT_LENGTH_EXTENSION> Bluetooth Controller Connection Event Extension
// <i> Default: Disabled
// <i> Enable or disable the connection event extension feature.
// <i> This allows connections to overrun lower priority tasks as long as there is data to transmit or receive on the connection,
// and the maximum connection event length is not reached.
#ifndef SL_BT_CONTROLLER_CONN_EVENT_LENGTH_EXTENSION
#define SL_BT_CONTROLLER_CONN_EVENT_LENGTH_EXTENSION (0)
#endif
// </h> Bluetooth Controller Configuration for LE Connection

// <q SL_BT_CONTROLLER_USE_LEGACY_VENDOR_SPECIFIC_EVENT_CODE> Use legacy event code 0x3e for vendor specific events
// <i> Default: 0
#ifndef SL_BT_CONTROLLER_USE_LEGACY_VENDOR_SPECIFIC_EVENT_CODE
#define SL_BT_CONTROLLER_USE_LEGACY_VENDOR_SPECIFIC_EVENT_CODE     (0)
#endif

// <o SL_BT_CONTROLLER_ADAPTIVITY_MODE> Adaptive Frequency Hopping operation mode
//   <SL_BTCTRL_CHANNELMAP_FLAG_ACTIVE_ADAPTIVITY=> Active AFH
//   <SL_BTCTRL_CHANNELMAP_FLAG_PASSIVE_ADAPTIVITY=> Passive AFH
// <i> Choose between active AFH and passive AFH
// <i> Default: Active AFH
// <d> SL_BTCTRL_CHANNELMAP_FLAG_ACTIVE_ADAPTIVITY
#ifndef SL_BT_CONTROLLER_ADAPTIVITY_MODE
#define SL_BT_CONTROLLER_ADAPTIVITY_MODE     (SL_BTCTRL_CHANNELMAP_FLAG_ACTIVE_ADAPTIVITY)
#endif

// <h> Advertising Configuration
// <e SL_BT_CONTROLLER_PRIMARY_EXT_PACKET_INCLUDE_TX_POWER> Include TX Power in the extended header of primary extended advertising packets
// <i> This is default setting to all advertising sets unless it is specifically overridden by VS_SiliconLabs_Set_Advertising_Config_Bits.
// <i> Default: 0
// <i> Enabling this option takes effect only if permitted by Le_Set_Extended_Advertising_Parameters
// <i> According to Link Layer specification, Vol 6, Part B, Table 2.4 and Table 2.5:
// <i> Optional in 1M PHY when the advertiser is connectable or scannable, or with auxiliary pointer.
// <i> In other cases, the Extended Advertising TX Power is always optional by settings.
#define SL_BT_CONTROLLER_PRIMARY_EXT_PACKET_INCLUDE_TX_POWER     (0)
// </e>
// <e SL_BT_CONTROLLER_PRIMARY_EXT_PACKET_INCLUDE_ADDRESS> Include Advertiser Address in the header of primary extended advertising packets
// <i> This is default setting to all advertising sets unless it is specifically overridden by VS_SiliconLabs_Set_Advertising_Config_Bits.
// <i> Default: 0
// <i> Enabling this option takes effect only if permitted by Le_Set_Extended_Advertising_Parameters
// <i> According to Link Layer specification, Vol 6, Part B, Table 2.4 and Table 2.5:
// <i> Optional in 1M PHY when the advertiser is non-connectable and non-scannable, with auxiliary pointer.
// <i> Mandatory when the advertiser is non-connectable and non-scannable, without auxiliary pointer.
// <i> Forbidden when the advertiser is connectable or scannable.
#define SL_BT_CONTROLLER_PRIMARY_EXT_PACKET_INCLUDE_ADDRESS      (0)
// </e>
// </h> Advertising Configuration
// </h> Bluetooth Controller Configuration

// <<< end of configuration section >>>

#endif // SL_BTCTRL_CONFIG_H
