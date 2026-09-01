/**
 * @file mqtt.h
 * @brief MQTT & BUSY account service API.
 *
 * The MQTT service is responsible for:
 * - Initiating and managing connections to the remote MQTT broker,
 * - BUSY account handling (linking & unlinking),
 * - Subscribing and publishing to session topics (see below).
 *
 * Limitations:
 * - Wildcard subscriptions are NOT supported,
 * - Attempts to publish when offline or not linked will result in an error,
 * - Unsubscribing from a topic will cause a reconnection to the MQTT broker.
 *
 * Topic format:
 * With the below API, subscription/publishing is only possible on the session topic.
 *
 * Internally, MQTT session topics have the following format:
 * - `sessions/aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee/down/v1/topic/name` for subscriptions
 * - `sessions/aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee/up/v1/topic/name` for publishing
 *
 * where `aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee` is a unique session identifier.
 *
 * Device-scope publishing (see @ref mqtt_publish_device_scope) uses the following format:
 * - `devices/<device_serial>/up/v1/topic/name` for publishing
 *
 * Device-scope topics do NOT require an account to be linked to the device.
 *
 * The MQTT service API abstracts it so the application must use only the `topic/name` part when interacting with it.
 * This rule also applies to some MQTT message properties such as Response Topic (see below).
 *
 * Data lifetime:
 * - Output functions (e.g. @ref mqtt_publish) will block briefly until the provided data is copied and sent internally.
 * It is safe to delete the data afterwards.
 * - Data returned in callbacks is only valid inside the callback. No pointers should be stored upon exiting from it
 * and all necessary data must be copied to recipient-owned storage.
 */
#pragma once

#include "mqtt_config.h"

#include <core/pubsub.h>
#include <core/string.h>

#include <time.h>

/**
 * @brief The string key for MQTT instance access
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_MQTT);`
 */
#define RECORD_MQTT "mqtt"

/**
 * @brief The length of the link PIN string.
 */
#define MQTT_LINK_PIN_LEN (4)

/**
 * @brief Opaque data type for the MQTT service instance.
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_MQTT);`
 */
typedef struct Mqtt Mqtt;

/**
 * @brief Opaque data type for the MQTT message object.
 */
typedef struct MqttMessage MqttMessage;

/**
 * @brief Opaque data type for the MQTT subscription object.
 */
typedef struct MqttSubscription MqttSubscription;

/**
 * @brief Subscription callback function type.
 *
 * A function with this signature may be used to deliver MQTT messages via invoking
 * the mqtt_subscribe() API.
 *
 * @note The value pointed to by the @p message parameter is only valid within the
 *       context of the callback it was delivered in.
 *
 * @param[in] message pointer to the delivered message
 * @param[in,out] context pointer to the user-specific object (set during subscribing)
 */
typedef void (*MqttSubscriptionCallback)(const MqttMessage* message, void* context);

/**
 * @brief Enumeration of available property types.
 *
 * @note Not all possible properties are implemented. Add new ones as needed.
 *
 * See https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901029 for reference.
 */
typedef enum {
    MqttPropertyTypeExpiryInterval, /**< Data type: integer */
    MqttPropertyTypeResponseTopic, /**< Data type: string */
    MqttPropertyTypeCorrelationData, /**< Data type: string */
    /* Add more property types as needed */
    MqttPropertyTypeMax, /**< Special value, internal use */
} MqttPropertyType;

/**
 * @brief Data type for a single MQTT property.
 */
typedef struct {
    MqttPropertyType type; /**< A type from the MqttPropertyType enum */
    union {
        int32_t integer; /**< Integer value, valid only for integer properties */
        const char* string; /**< String value, valid only for string properties */
    } value;
} MqttProperty;

/**
 * @brief Enumeration of possible event types emitted by the MQTT service.
 */
typedef enum {
    MqttEventTypeStatusChanged, /**< A status change has occurred */
    MqttEventTypeLinkPinReceived, /**< An account linking PIN has been received in response to a request */
    MqttEventTypeLinkDone, /**< Account linking has been successfully completed */
    MqttEventTypeUnlinked, /**< Account was unlinked from the device */
    MqttEventTypeMax, /**< Special value, internal use */
} MqttEventType;

/**
 * @brief Data type describing the event of type @c MqttEventTypeLinkPinReceived.
 *
 * @note The `pin` pointer is valid only inside the event delivery callback.
 */
typedef struct {
    const char* pin; /**< Pointer to a string of length @c MQTT_LINK_PIN_LEN */
    time_t expires_at; /**< Timestamp past which the PIN will no longer be valid */
} MqttEventLinkPinReceived;

/**
 * @brief Enumeration of possible states that the MQTT service can be in.
 */
typedef enum {
    MqttStatusError, /**< An error has occurred, usually because clent certificates are missing */
    MqttStatusNotConnected, /**< Not connected to MQTT broker */
    MqttStatusConnectedNotLinked, /**< Connected to MQTT broker, but not linked to an account */
    MqttStatusConnectedLinked, /**< Connected to MQTT broker, and linked to an account */
    MqttStatusMax, /**< Special value, internal use */
} MqttStatus;

/**
 * @brief Data type describing the event of type @c MqttEventTypeStatusChanged.
 */
typedef struct {
    MqttStatus status; /**< New status value from the MqttStatus enum */
} MqttEventStatusChanged;

/**
 * @brief Data type describing the generic MQTT service event, complete with type and specific data.
 */
typedef struct {
    MqttEventType type; /**< A value determining the specific data type */
    union {
        /** Data for an event of type @c MqttEventTypeStatusChanged */
        MqttEventStatusChanged status_changed;
        /** Data for an event of type @c MqttEventTypeLinkPinReceived */
        MqttEventLinkPinReceived link_pin_received;
    };
} MqttEvent;

/**
 * @brief Enumeration of possible MQTT QoS (Quality of Service) levels.
 *
 * See https://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718099 for reference.
 */
typedef enum {
    MqttQosAtMostOnce = 0, /**< Deliver at most once (lowest) */
    MqttQosAtLeastOnce = 1, /**< Deliver at least once */
    MqttQosExactlyOnce = 2, /**< Deliver exactly once (highest) */
    MqttQosMax, /**< Special value, internal use */
} MqttQos;

/**
 * @brief Get the pointer to the associated FuriPubSub object.
 *
 * Use furi_pubsub_subscribe() to subscribe to the MQTT service events.
 * The delivered events will be of type MqttEvent.
 *
 * @param[in,out] instance pointer to the MQTT service instance
 * @returns pointer to the FuriPubSub object
 */
FuriPubSub* mqtt_get_pubsub(Mqtt* instance);

/**
 * @brief Get the current status of the MQTT service.
 *
 * @param[in] instance pointer to the MQTT service instance to be queried
 * @returns numeric value corresponding to the current status
 */
MqttStatus mqtt_get_status(Mqtt* instance);

// =========================== Account management ==================================

/**
 * @brief Data type for a linked account information
 */
typedef struct {
    FuriString*
        session_id; /**< Session ID string pointer, should be allocated before. Set to NULL if not used */
    FuriString*
        email; /**< Linked account email string pointer, should be allocated before. Set to NULL if not used */
    FuriString*
        user_id; /**< Linked account user ID string pointer, should be allocated before. Set to NULL if not used */
    bool is_valid; /**< true if the account link data presents, false otherwise */
} MqttSessionInfo;

/**
 * @brief Request the account linking PIN from the BUSY cloud.
 *
 * @note The requested PIN will be delivered asynchronously via the @c MqttEventTypeLinkPinReceived event.
 *
 * @param[in,out] instance pointer to the MQTT service instance
 * @returns true if the request was sent successfully, false otherwise
 */
bool mqtt_request_link_pin(Mqtt* instance);

/**
 * @brief Unlink the device from the BUSY account.
 *
 * @param[in,out] instance pointer to the MQTT service instance
 */
void mqtt_unlink(Mqtt* instance);

/**
 * @brief Get current session information.
 *
 * @param[in] instance pointer to the MQTT service instance to be queried
 * @param[in,out] info pointer to the @c MqttSessionInfo structure
 */
void mqtt_get_session_info(Mqtt* instance, MqttSessionInfo* info);

// =========================== Profile management ==================================

/**
 * @brief Get the current backend configuration.
 *
 * @param[in] instance pointer to the MQTT service instance to be queried
 * @param[out] config pointer to a MqttConfig structure to contain the result (must be allocated)
 */
void mqtt_get_config(Mqtt* instance, MqttConfig* config);

/**
 * @brief Set the backend configuration.
 *
 * @param[in,out] instance pointer to the MQTT service instance to be modified
 * @param[in] config pointer to a MqttConfig structure containing the configuration to be set
 * @returns @c true if the configuration could be successfully set, @c false otherwise
 */
bool mqtt_set_config(Mqtt* instance, const MqttConfig* config);

// =========================== Subscription management ==================================

/**
 * @brief Publish a message to a topic on the MQTT broker.
 *
 * @note The caller must provide only the unique topic
 *       part, e.g. `topic/name` in the above example
 *
 * @param[in,out] instance pointer to the MQTT service instance
 * @param[in] qos enum value from MqttQos corresponding to a QoS level
 * @param[in] topic C-string containing the specific topic part
 * @param[in] data pointer to arbitrary data to be published
 * @param[in] data_size size (or length) of the @p data to be published
 * @returns @c true if publishing was successful, false otherwise
 */
bool mqtt_publish(
    Mqtt* instance,
    MqttQos qos,
    const char* topic,
    const void* data,
    size_t data_size);

/**
 * @brief Publish a message to a topic on the MQTT broker with additional properties.
 *
 * @note The caller must provide only the unique topic
 *       part, e.g. `topic/name` in the above example
 *
 * @param[in,out] instance pointer to the MQTT service instance
 * @param[in] qos enum value from MqttQos corresponding to a QoS level
 * @param[in] topic C-string containing the specific topic part
 * @param[in] data pointer to arbitrary data to be published
 * @param[in] data_size size (or length) of the @p data to be published
 * @param[in] props pointer to the array of MQTT message properties
 * @param[in] props_count number of properties contained in the @p props array
 * @returns @c true if publishing was successful, false otherwise
 */
bool mqtt_publish_ex(
    Mqtt* instance,
    MqttQos qos,
    const char* topic,
    const void* data,
    size_t data_size,
    const MqttProperty* props,
    uint32_t props_count);

/**
 * @brief Publish a message to a device-scope topic on the MQTT broker.
 *
 * Device-scope topics use the `devices/<device_serial>/up/v1/topic/name` format and do NOT
 * require an account to be linked to the device (unlike session-scope publishing, which is
 * only allowed in the @c MqttStatusConnectedLinked state). Publishing is allowed in both
 * @c MqttStatusConnectedLinked and @c MqttStatusConnectedNotLinked states.
 *
 * @note The caller must provide only the unique topic
 *       part, e.g. `topic/name` in the above example
 *
 * @param[in,out] instance pointer to the MQTT service instance
 * @param[in] qos enum value from MqttQos corresponding to a QoS level
 * @param[in] topic C-string containing the specific topic part
 * @param[in] data pointer to arbitrary data to be published
 * @param[in] data_size size (or length) of the @p data to be published
 * @returns @c true if publishing was successful, false otherwise
 */
bool mqtt_publish_device_scope(
    Mqtt* instance,
    MqttQos qos,
    const char* topic,
    const void* data,
    size_t data_size);

/**
 * @brief Subscribe to a topic on the MQTT broker and start receiving messages.

 * @note The caller must provide only the unique topic
 *       part, e.g. `topic/name` in the above example
 *
 * @param[in,out] instance pointer to the MQTT service instance
 * @param[in] qos enum value from MqttQos corresponding to a QoS level
 * @param[in] topic C-string containing the specific topic part
 * @param[in] callback pointer to the function used to asynchronously deliver MQTT messages
 * @param[in,out] context pointer to a user-specific object (will be passed to the callback)
 * @returns pointer to the object uniquely identifying the resulting subscription
 */
MqttSubscription* mqtt_subscribe(
    Mqtt* instance,
    MqttQos qos,
    const char* topic,
    MqttSubscriptionCallback callback,
    void* context);

/**
 * @brief Unsubscribe from a topic on the MQTT broker and stop receiving messages.
 *
 * @warning Due to current technical limitations, calling this function
 *          will trigger a reconnection procedure.
 *
 * @param[in,out] instance pointer to the MQTT service instance
 * @param[in,out] subscription pointer to the target subscription object
 */
void mqtt_unsubscribe(Mqtt* instance, MqttSubscription* subscription);

// =========================== Message operations ==================================

/**
 * @brief Get the payload (data) contained in a MqttMessage object.
 *
 * @note objects of type MqttMessage are only valid within the context
 *       of the callback that delivered them.
 *
 * @note @p data_size may be @c NULL if it is not required.
 *
 * @param[in] message pointer to the MqttMessage object to be queried
 * @param[out] data_size pointer to the value to contain the size (length) of the data
 * @returns pointer to the data held by the message
 */
const void* mqtt_message_get_data(const MqttMessage* message, size_t* data_size);

/**
 * @brief Get the string property possibly attached to a MqttMessage object.
 *
 * @warning The value of @p property_type MUST correspond to the underlying string data type.
 *
 * @param[in] message pointer to the MqttMessage object to be queried
 * @param[in] property_type value from the MqttPropertyType enum denoting the property type
 * @param[out] value pointer to a FuriString object to contain the value (must be allocated)
 * @returns true if a property of @p property_type was found, false otherwise
 */
bool mqtt_message_get_string_property(
    const MqttMessage* message,
    MqttPropertyType property_type,
    FuriString* value);

/**
 * @brief Get the integer property possibly attached to a MqttMessage object.
 *
 * @warning The value of @p property_type MUST correspond to the underlying integer data type.
 *
 * @param[in] message pointer to the MqttMessage object to be queried
 * @param[in] property_type value from the MqttPropertyType enum denoting the property type
 * @param[out] value pointer to an integer variable to contain the value (must be allocated)
 * @returns true if a property of @p property_type was found, false otherwise
 */
bool mqtt_message_get_integer_property(
    const MqttMessage* message,
    MqttPropertyType property_type,
    uint32_t* value);
