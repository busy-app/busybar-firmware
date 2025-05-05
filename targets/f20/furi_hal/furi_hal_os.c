#include <furi_hal_os.h>
#include <furi.h>

#include <FreeRTOS.h>
#include <task.h>

#define TAG "FuriHalOs"

void furi_hal_os_init(void) {
    FURI_LOG_I(TAG, "Init OK");
}

// extern void xPortSysTickHandler(void);

extern void SysTick_Handler(void);

void furi_hal_os_tick(void) {
    if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        SysTick_Handler();
    }
}
