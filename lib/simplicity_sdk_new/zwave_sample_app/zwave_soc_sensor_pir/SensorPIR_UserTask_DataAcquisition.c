/**
 * @file
 * See header file for description!
 *
 * @copyright 2020 Silicon Laboratories Inc.
 */

#include <SensorPIR_UserTask_DataAcquisition.h>
#include <zaf_event_distributor_soc.h>
#include "events.h"
#include <SizeOf.h>
#include "zpal_log.h"

/****************************************************************
 * CONFIGURATIONS OF THIS MODULE
 ***************************************************************/

/****************************************************************
 * DEFINITIONS
 ***************************************************************/

/****************************************************************
 * TYPEDEF and CONSTANTS
 ***************************************************************/

/****************************************************************
 * MACROS
 ***************************************************************/

#define USER_TASK_WAKEUP_PERIOD   30000

/****************************************************************
 * FORWARD DECLARATIONS (none preferred)
 ***************************************************************/

/****************************************************************
 * STATIC VARIABLES
 ***************************************************************/

/****************************************************************
 * EXTERNAL VARIABLES (none preferred)
 ***************************************************************/

/****************************************************************
 * STATIC FUNCTIONS
 ***************************************************************/

/**
 * This function does the main operation of this task, which is defined by the User.
 */
NO_RETURN static void executeThread(void)
{
  for (;;) {
    // force acquisition of the lock with relock

    ////////////////////////////////////
    //Do something user specific
    ////////////////////////////////////

    zaf_event_distributor_enqueue_app_event(EVENT_APP_USERTASK_DATA_ACQUISITION_FINISHED);  // An event to be send to the main app.
    vTaskDelay(pdMS_TO_TICKS(USER_TASK_WAKEUP_PERIOD));
  }
}

/****************************************************************
 * API FUNCTIONS
 ***************************************************************/

/****************************************************************
 * THREAD FUNCTION
 ***************************************************************/

/**
 * A pointer to this function is passed to ZW_UserTask_CreateTask().
 *
 * ATTENTION: This task context shall never call a ZAF API function!
 *
 * The pointer passed to this function is user defined (void-pointer),
 * but here casted to SApplicationHandles-pointer.
 */
NO_RETURN void
SensorPIR_DataAcquisitionTask(__attribute__((unused)) void* pUserTaskParam)
{
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "\r\nSensorPIR Data Acquisition UserTask started!");

  // Generate event that says the Data acquisition UserTask has started!
  if (zaf_event_distributor_enqueue_app_event(EVENT_APP_USERTASK_DATA_ACQUISITION_READY)) {
    ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "\r\nDataAcquisitionTask: Ready event is send to main app!\r\n");
  }

  executeThread();
}
