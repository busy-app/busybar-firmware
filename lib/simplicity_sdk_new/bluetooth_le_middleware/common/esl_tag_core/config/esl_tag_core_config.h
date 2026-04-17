/***************************************************************************//**
 * @file
 * @brief ESL Tag Core component configuration macros
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#ifndef ESL_TAG_CORE_CONFIG_H
#define ESL_TAG_CORE_CONFIG_H

/***********************************************************************************************//**
 * @addtogroup esl_tag_core
 * @{
 **************************************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <h> ESL Tag Core configuration

// <o ESL_TAG_MAX_SYNC_LOST_COUNT> Missed sync receives before timeout [intervals]<0-3276>
// <i> Set the maximum grace period for consecutive unsuccessful PAwR receive before sync timeout.
// <i> To use the maximum timeout by PAwR specification (163,84s) regardless of the actual interval please set to zero.
// <i> Otherwise, the timeout will be [intervals]* periodic interval [ms] or the maximum - whichever is smaller.
// <i> Default: 3
#define ESL_TAG_MAX_SYNC_LOST_COUNT               3

// <q ESL_TAG_VENDOR_OPCODES_ENABLED> ESL Vendor-specific tag support
// <i> Enables the ESL Vendor-specific tag support for TLV opcodes
// <i> Default: On
#define ESL_TAG_VENDOR_OPCODES_ENABLED            1

// <q ESL_TAG_EAD_REPLAY_MITIGATION_ENABLED> Encrypted Adv. Data replay mitigation
// <i> Prevents replay attacks by ignoring replay EAD packets.
// <i> Disable for backward compatibility with an ESLP v1.0 compatible AP that sets a new randomizer value
// <i> for each sync packet (as opposed to incrementing the randomizer, as specified in ESLP v1.0.1).
// <i> Default: On
#define ESL_TAG_EAD_REPLAY_MITIGATION_ENABLED     1

// <h> ESL Tag Advertiser configuration
// <o ESL_TAG_ADVERTISING_INTERVAL_MIN> Minimum connectable advertising interval <100-10000>
// <i> Default: 750 (Double with Intermittent advertising enabled to keep the average power consumption in the same range).
// <i> Define the minimum time interval of connectable advertising packets in milliseconds the ESL Tag needs.
// <i> The actual advertising interval will be randomized between min and max values for system scalability.
#define ESL_TAG_ADVERTISING_INTERVAL_MIN          750

// <o ESL_TAG_ADVERTISING_INTERVAL_MAX> Maximum connectable advertising interval <100-10000>
// <i> Default: 1500 (Double with Intermittent advertising enabled to keep the average power consumption in the same range).
// <i> Define the maximum time interval of connectable advertising packets in milliseconds the ESL Tag allows.
#define ESL_TAG_ADVERTISING_INTERVAL_MAX          1500

// <e ESL_TAG_INTERMITTENT_ADVERTISING> Intermittent advertising
// <i> Can improve connection establishment by broadcasting a secondary connectable advertisement that closely follows the primary ESL Service advertising.
// <i> The additional advertisings don't contain any data, its only purpose is to speed up the response to connection requests.
// <i> Increases the average power consumption in the Unassociated and Unsynchronized states, unless the advertisement interval limits are increased proportionally.
// <i> Default: Off
#define ESL_TAG_INTERMITTENT_ADVERTISING          0

// <o ESL_TAG_SECONDARY_ADVERTISING_DELAY> Secondary advertising delay <10-500>
// <i> Default: 225
// <i> Define the delay of the secondary connectable advertising in milliseconds to the primary advertising.
// <i> For best results, this delay should be 25-30 times the connection interval, which depend on the negotiation between the AP and the ESL.
#define ESL_TAG_SECONDARY_ADVERTISING_DELAY       225

// </e>

// </h>

// <e ESL_TAG_BUILTIN_BATTERY_MEASURE_ENABLE> Built-in battery level measurement
// <i> Enables the built-in battery level measurement and warning on low level.
// <i> Default: On
#define ESL_TAG_BUILTIN_BATTERY_MEASURE_ENABLE    1

// <o ESL_TAG_BATTERY_LEVEL_FULL_MILLIVOLTS> Battery full level [mV]<3000-4300>
// <i> Electric potential between battery terminals at fully charge in milli Volts unit. Reference value, only.
// <i> Default: 3200
#define ESL_TAG_BATTERY_LEVEL_FULL_MILLIVOLTS     3200

// <o ESL_TAG_BATTERY_LEVEL_LOW_MILLIVOLTS> Battery low warning level [mV]<2200-2999>
// <i> Electric potential between battery terminals at which the ESL Tag shall set the Service Needed Basic State flag.
// <i> Default: 2400
#define ESL_TAG_BATTERY_LEVEL_LOW_MILLIVOLTS      2400

// <o ESL_TAG_BATTERY_MEASUREMENT_INTERVAL_MIN> Measurement validity period [minutes]<1-2880>
// <i> Set the validity period of the battery voltage measurement - this is the time interval after which a new measurement will be taken on request.
// <i> Default: 10
#define ESL_TAG_BATTERY_MEASUREMENT_INTERVAL_MIN  10

// </e>

// <e ESL_TAG_POWER_DOWN_ENABLE> Power saving after timeout in Unassociated state
// <i> Allows the ESL Core to enter EM4 after a specified amount of time in the Unassociated state.
// <i> Note: ESL Core will emit 'esl_core_shutdown_hook()' event only if this feature is enabled.
// <i> Default: On
#define ESL_TAG_POWER_DOWN_ENABLE                 1

// <o ESL_TAG_POWER_DOWN_TIMEOUT_MIN> EM4 timeout [minutes]<2-1440>
// <i> Up to 24 hours timeout to enter EM4 from Unassociated state.
// <i> Default: 60 minutes
#define ESL_TAG_POWER_DOWN_TIMEOUT_MIN            60

// </e>

// <e ESL_TAG_SYNC_SCAN_ENABLE> Custom intermediate state for temporary loss of sync
// <i> Custom intermediate state to recover from temporary loss of sync. It can reduce advertising traffic caused by ESLs that have encountered an unexpected loss of PAwR sync.
// <i> Requires the 'bluetooth_feature_sync_scanner' feature to be manually added to the project if enabled.
// <i> Note: This is a non-standard ESL behavior that should be compatible with standard APs that are unaware of the feature.
// <i> Default: Off
#define ESL_TAG_SYNC_SCAN_ENABLE                 0

// <o ESL_TAG_SCAN_TIMEOUT_SEC> Scanning timeout [seconds]<2-163>
// <i> Specifies a custom grace period during which the ESL will attempt to resync by scanning after a PAwR train loss before entering the Unsynchronized state.
// <i> This value is more of a trade-off between fast fallback to advertising if the scan doesn't succeed and the energy budget required to scan.
// <i> If the value is considerably less than twice the typical PAwR interval of the target ESL network, the resynchronization attempt by scanning is likely to fail.
// <i> Default: 11 seconds
#define ESL_TAG_SCAN_TIMEOUT_SEC                 11

// <o ESL_TAG_SCAN_COUNT_LIMIT> Successive scanning limit <1-10>
// <i> Specifies a maximum limit for successive resynchronizations by scanning.
// <i> After this limit is reached, the ESL starts advertising on the next sync loss to keep the absolute timer counter bias under control.
// <i> Default: 3 times
#define ESL_TAG_SCAN_COUNT_LIMIT                 3

// <o ESL_TAG_SCAN_SWEEEP_COEFF> Scan sweep coefficient <3-11>
// <5=> PAwR / 5
// <7=> PAwR / 7
// <11=> PAwR / 11
// <i> Specifies a coefficient for calculating an appropriate scan interval for successfully finding the lost PAwR sync train, while ignoring most others.
// <i> A larger value implies a more frequent scan with an increased energy budget and also a higher probability to find the signal faster.
// <i> A higher value may also prevent sync-by-scan for extremely short PAwR intervals. In this case, the standard advertising starts immediately.
// <i> Default: one fifth of the lost PAwR interval
#define ESL_TAG_SCAN_SWEEEP_COEFF                5

// <o ESL_TAG_SCAN_WINDOW_MS> Scanning window [ms]<10-100>
// <i> Defines the scan window within the scan interval.
// <i> If this value exceeds the scan interval derived from the last PAwR by the sweep coefficient, the scan window is set equal to the interval.
// <i> Default: 30 ms
#define ESL_TAG_SCAN_WINDOW_MS                   30

// </e>

// </h>

// <<< end of configuration section >>>

// convert ESL_TAG_BATTERY_MEASUREMENT_INTERVAL_MIN given in [min] to [ms] unit
#define ESL_TAG_BATTERY_MEASUREMENT_INTERVAL_MS \
  (ESL_TAG_BATTERY_MEASUREMENT_INTERVAL_MIN * 60 * 1000)

// convert ESL_TAG_POWER_DOWN_TIMEOUT_MIN given in [min] to [ms] unit
#define ESL_TAG_POWER_DOWN_TIMEOUT_MS \
  (ESL_TAG_POWER_DOWN_TIMEOUT_MIN * 60 * 1000)

// provide secondary advertising storage space if ESL_TAG_INTERMITTENT_ADVERTISING is enabled
#if (ESL_TAG_INTERMITTENT_ADVERTISING == 0)
  #define ESL_TAG_ADVERTISERS_COUNT 1
#else
  #define ESL_TAG_ADVERTISERS_COUNT 2
#endif // ESL_TAG_INTERMITTENT_ADVERTISING
/** @} (end addtogroup esl_tag_core) */
#endif // ESL_TAG_CORE_CONFIG_H
