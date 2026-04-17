/*******************************************************************************
 * @file
 * @brief iostream usart examples functions
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <stdio.h>
#include <string.h>
#include "sl_iostream.h"
#include "sl_iostream_init_instances.h"
#include "sl_iostream_handles.h"
#include "FreeRTOS.h"
#include "task.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#ifndef BUFSIZE
#define BUFSIZE    80
#endif

#ifndef TERMINAL_TASK_STACK_SIZE
#define TERMINAL_TASK_STACK_SIZE      256
#endif

#ifndef TERMINAL_TASK_PRIO
#define TERMINAL_TASK_PRIO            20
#endif

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

/* Input buffer */
static char buffer[BUFSIZE];

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
void app_iostream_terminal_task(void *pvParameters);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/*******************************************************************************
 * Initialize example.
 ******************************************************************************/
void app_iostream_eusart_init(void)
{
  /* Prevent buffering of output/input. */
#if !defined(__CROSSWORKS_ARM) && defined(__GNUC__)
  setvbuf(stdout, NULL, _IONBF, 0);   /* Set unbuffered mode for stdout (newlib) */
  setvbuf(stdin, NULL, _IONBF, 0);    /* Set unbuffered mode for stdin (newlib) */
#endif

  // Create terminal task (FreeRTOS)
  BaseType_t result = xTaskCreate(
    app_iostream_terminal_task,     // Task function
    "iostream terminal task",       // Name
    TERMINAL_TASK_STACK_SIZE,       // Stack size in words
    NULL,                           // Task parameter
    TERMINAL_TASK_PRIO,             // Task priority
    NULL                            // Task handle (optional)
    );
  configASSERT(result == pdPASS);    // Assert task creation succeeded
}

/*******************************************************************************
 * Terminal task.
 ******************************************************************************/
void app_iostream_terminal_task(void *pvParameters)
{
  int8_t c = 0;
  uint8_t index = 0;

  (void)pvParameters;

  /* Output on vcom usart instance */
  const char str1[] = "IOstream EUSART example\r\n\r\n";
  sl_iostream_write(sl_iostream_vcom_handle, str1, strlen(str1));

  /* Setting default stream */
  sl_iostream_set_default(sl_iostream_vcom_handle);
  const char str2[] = "This is output on the default stream\r\n";
  sl_iostream_write(SL_IOSTREAM_STDOUT, str2, strlen(str2));

  /* Using printf */
  /* Writing ASCII art to the VCOM iostream */
  printf("Printf uses the default stream, as long as iostream_retarget_stdio is included.\r\n");

  printf("> ");
  /* Retrieve characters, print local echo and full line back */
  while (1) {
    c = getchar();
    if (c > 0) {
      if (c == '\r' || c == '\n') {
        buffer[index] = '\0';
        printf("\r\nYou wrote: %s\r\n>", buffer);
        index = 0;
      } else {
        if (index < BUFSIZE - 1) {
          buffer[index] = c;
          index++;
        }

        /* Local echo */
        putchar(c);
      }
    }
  }
}
