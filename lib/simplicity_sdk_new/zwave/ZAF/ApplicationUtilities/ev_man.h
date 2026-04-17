/**
 * @file
 * @brief Framework event manager, handling all event types for the framework
 * @copyright 2018 Silicon Laboratories Inc.
 */

#ifndef _ev_man_H_
#define _ev_man_H_

#include <ZW_basis_api.h>

/**
 * @addtogroup ZAF
 * @{
 * @addtogroup EVMAN Event Manager
 * @{
 */

/**
 * Event groups for application state event machine
 */
#define DEFINE_EVENT_WAKEUP_NBR 0x01
#define DEFINE_EVENT_KEYPAD_NBR 0x10
#define DEFINE_EVENT_IR_NBR 0x20
#define DEFINE_EVENT_SYSTEM_NBR 0x30

/**
 * Event group for keys 0x40 - 0x6F
 */
#define DEFINE_EVENT_KEY_NBR 0x40

/**
 * Event group for zaf 0x80 - 0xFF
 */
#define DEFINE_EVENT_APP_NBR 0x70

/**
 * System events
 */
typedef enum _EVENT_SYSTEM_{
  EVENT_SYSTEM_RESET = DEFINE_EVENT_SYSTEM_NBR,
  EVENT_SYSTEM_LEARNMODE_START,      //Enqueue to manually start learn mode.
  EVENT_SYSTEM_LEARNMODE_FINISHED,   //Enqueued after learn process finished. Both on success and timeout.
  EVENT_SYSTEM_LEARNMODE_STOP,       //Enqueue to manually stop learn mode.
  EVENT_SYSTEM_LEARNMODE_TOGGLE,     //Enqueue to toggle the learn mode
  EVENT_SYSTEM_WATCHDOG_RESET,
  EVENT_SYSTEM_OTA_START,
  EVENT_SYSTEM_LEARNMODE_DSK_START,
  EVENT_SYSTEM_FLUSHMEM_READY,
  EVENT_SYSTEM_SMARTSTART_IN_PROGRESS,
  EVENT_SYSTEM_EMPTY
} EVENT_SYSTEM;

#define EventSchedulerInit ZAF_eventSchedulerInit
/**
 * @brief Initializes event scheduler.
 */
void ZAF_eventSchedulerInit(VOID_CALLBACKFUNC(pApplicationStateMachine)(uint8_t));

#define ZCB_EventSchedulerEventAdd ZCB_eventSchedulerEventAdd
/**
 * @brief Adds a given event to the event queue.
 * @param event A given event.
 * @return true if given event is added to queue, false if queue full.
 */
bool ZCB_eventSchedulerEventAdd(uint8_t event);

#define ZCB_EventScheduler ZCB_eventScheduler
/**
 * @brief Processes events.
 */
bool ZCB_eventScheduler(void);

#define ZCB_EventEnqueue ZAF_jobEnqueue
/**
 * @brief Adds a given event to the job queue.
 * @param event A given event.
 * @return true if given event is added to queue, false if queue full.
 */
bool ZAF_jobEnqueue(uint8_t event);

#define ZCB_EventDequeue ZAF_jobDequeue
/**
 * @brief Deque job queue
 * @param[out] pEvent return event from the queue.
 * @return false if queue is empty else true.
 */
bool ZAF_jobDequeue(uint8_t* pEvent);

/**
 * @brief Get number of events on queue
 * @return number of events on queue
 */
uint8_t ZAF_jobQueueCount(void);

/**
 * @} // EVMAN
 * @} // ZAF
 */

#endif /* _ev_man_H_ */
