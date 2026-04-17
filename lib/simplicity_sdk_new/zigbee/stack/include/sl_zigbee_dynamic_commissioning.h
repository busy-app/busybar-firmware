/***************************************************************************//**
 * @file sl_zigbee_dynamic_commissioning.h
 * @brief implements hooks for performing device interview during dynamic
 * commissioning
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef SL_ZIGBEE_DYNAMIC_COMMISSIONING_H
#define SL_ZIGBEE_DYNAMIC_COMMISSIONING_H

#include "sl_enum.h"
#include "stack/include/sl_zigbee_address_info.h"
#include "stack/include/sl_zigbee_types.h"

SL_ENUM_GENERIC(sl_zigbee_dynamic_commissioning_event_t, uint16_t) {
  /// The trust center has received notification of a dynamic commissioning request, which kicks off a Dynamic Link Key (DLK) process
  SL_ZIGBEE_DYNAMIC_COMMISSIONING_EVENT_OPEN_REQUEST,
  /// DLK has completed - allow the device on the network or conduct further interview, determined by sl_zigbee_dynamic_commissioning_is_open_for_interview()
  SL_ZIGBEE_DEVICE_INTERVIEW_EVENT_READY,
  /// The device interview is continuing. The stack refreshes its internal timers for the device.
  SL_ZIGBEE_DEVICE_INTERVIEW_EVENT_CONTINUE,
  /// The interview was concluded as non-success - commissioning failed
  SL_ZIGBEE_DEVICE_INTERVIEW_EVENT_REJECTED,
  /// The interview has timed out
  SL_ZIGBEE_DEVICE_INTERVIEW_EVENT_TIMEOUT,
  /// A message was received for a downstream child
  SL_ZIGBEE_DEVICE_INTERVIEW_EVENT_DOWNSTREAM_MIDPOINT,
  /// The interview has concluded and the device should be allowed on the network. The device is sent the network key.
  SL_ZIGBEE_DYNAMIC_COMMISSIONING_EVENT_ACCEPTED,
  /// An error was encountered during commissioning
  SL_ZIGBEE_DYNAMIC_COMMISSIONING_EVENT_ERROR,
  /// For parent routers, the dynamic commissioning is complete with the downstream joiner. The device is on the network and has sent a network-encrypted frame.
  SL_ZIGBEE_DYNAMIC_COMMISSIONING_EVENT_COMPLETE,
};

/**
 * @brief An application-defined callback that allows a user to monitor the dynamic
 *  commissioning process, which includes the Dynamic Link Key procedure along with
 *  the device interview process. A user application may implement this function to
 *  be updated when certain events occur.
 *  Events begin from the start of the Dynamic Link Key (DLK) process
 *  (SL_ZIGBEE_DYNAMIC_COMMISSIONING_EVENT_OPEN_REQUEST) and can end with either the
 *  device being rejected from the network or admitted. In the event that the device
 *  is admitted, it is sent the network key in event
 *  SL_ZIGBEE_DYNAMIC_COMMISSIONING_EVENT_ACCEPTED. Most events occur on the Trust
 *  Center device, but certain events also appear on parent routers of joining devices.
 *
 * @param[in] ids  The address info of the device associated with the interview event
 * @param[in] event The type of interview event that triggered the alert
 *
 * @note The default behavior of the stack is to perform DLK and then proceed to send
 *  the network key to the joining device. User applications that wish to conduct a
 *  device interview before telling the stack to admit the device may use the
 *  ::sl_zigbee_dynamic_commissioning_set_open_for_interview API to do so. In such a
 *  scenario, when a device joins, the user will be alerted that the device interview
 *  stage is ready with the SL_ZIGBEE_DEVICE_INTERVIEW_EVENT_READY event. The user
 *  must then alert the stack by calling ::sl_zigbee_device_interview_status_update
 *  with either SL_ZIGBEE_DEVICE_INTERVIEW_EVENT_REJECTED to not admit the device, or
 *  SL_ZIGBEE_DYNAMIC_COMMISSIONING_EVENT_ACCEPTED to accept the device on the network
 *  and send the network key.
 *
 * @return None
 */
void sl_zigbee_dynamic_commissioning_alert_callback(sl_zigbee_address_info *ids,
                                                    sl_zigbee_dynamic_commissioning_event_t event);

/**
 * @brief An API to have the application decide if a joining device should be admitted
 *  onto the network or not. This API should be called during the device interview
 *  stage, after ::sl_zigbee_dynamic_commissioning_alert_callback alerts the user that
 *  the event is SL_ZIGBEE_DEVICE_INTERVIEW_EVENT_READY. This stack only accepts
 *  SL_ZIGBEE_DYNAMIC_COMMISSIONING_EVENT_ACCEPTED or
 *  SL_ZIGBEE_DEVICE_INTERVIEW_EVENT_REJECTED for the event argument, which determines
 *  if the joining device should be sent the network key or not. Other event inputs are
 *  ignored. This API should only be called on the Trust Center. Calling this on any other
 *  device role will result in a SL_ZIGBEE_DYNAMIC_COMMISSIONING_EVENT_ERROR event issued
 *  on the app.
 *
 *  If SL_ZIGBEE_DYNAMIC_COMMISSIONING_EVENT_ACCEPTED is passed in for the event argument,
 *  then the device is admitted onto the network and is sent the Network Key.
 *
 *  If SL_ZIGBEE_DEVICE_INTERVIEW_EVENT_REJECTED is passed in for the event argument,
 *  then any APS key produced as a result of the Dynamic Link Key process is deleted.
 *
 * @param[in] ids A structure which holds the short and long address
 * @param[in] event An event to pass to the application to indicate the state of commissioning.
 *
 * @return None
 *
 * @note The default behavior of the stack is to admit the device onto the network once DLK has completed.
 *  The user application does not need to call this function if this default behavior is desired. This API
 *  is only to be invoked if the device interview process has been enabled via the
 *  ::sl_zigbee_dynamic_commissioning_set_open_for_interview API, whereby the stack will hold
 *  off in sending the network key to the joining device in order to allow the user application
 *  to further interview the device.
 */
void sl_zigbee_device_interview_status_update(sl_zigbee_address_info *ids,
                                              sl_zigbee_dynamic_commissioning_event_t event);

/**
 * @brief Set the policy for whether to keep the commissioning window open
 *  to perform device interview procedures. If the input argument is false, the stack
 *  will send the network key after Dynamic Link Key completes with the joining device.
 *  If the input argument is true, the stack will not send the network key after DLK
 *  completes. The stack will await for the user to invoke
 *  ::sl_zigbee_device_interview_status_update with either
 *  SL_ZIGBEE_DYNAMIC_COMMISSIONING_EVENT_ACCEPTED or
 *  SL_ZIGBEE_DEVICE_INTERVIEW_EVENT_REJECTED.
 *
 * @param[in] open_interview A boolean indicating whether to keep commissioning open after DLK completes
 *
 * @return None
 *
 * @note The default behavior of the stack is to admit the device onto the network once DLK has completed.
 */
void sl_zigbee_dynamic_commissioning_set_open_for_interview(bool open_interview);

/**
 * @brief Get the policy for whether the stack will keep the commissioning window open
 *  to perform device interview procedures before deciding to admit the device
 *  onto the network.
 *
 * @return A boolean indicating whether commissioning is kept open after DLK completes. If
 * true, the user is required to call ::sl_zigbee_device_interview_status_update with either
 * SL_ZIGBEE_DYNAMIC_COMMISSIONING_EVENT_ACCEPTED or SL_ZIGBEE_DEVICE_INTERVIEW_EVENT_REJECTED to
 * either admit or reject the device onto/from the network.
 *
 * @note The default behavior of the stack is to admit the device onto the network once DLK has completed.
 */
bool sl_zigbee_dynamic_commissioning_is_open_for_interview(void);

#endif // SL_ZIGBEE_DYNAMIC_COMMISSIONING_H
