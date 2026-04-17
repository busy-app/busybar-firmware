/***************************************************************************/ /**
 * @file app_iostream_usart.c
 * @brief iostream usart examples functions
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
#include "sl_iostream.h"
#include "sl_iostream_init_instances.h"
#include "sl_iostream_handles.h"
#include "app_iostream_usart.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#ifndef BUFSIZE
#define BUFFER_SIZE 80 //Input buffer size
#endif

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

/* Input buffer */
static char buffer[BUFFER_SIZE];

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************/ /**
 * Initialize example.
 ******************************************************************************/
void app_iostream_usart_init(void)
{
  /* Prevent buffering of output/input.*/
#if !defined(__CROSSWORKS_ARM) && defined(__GNUC__)
  setvbuf(stdout, NULL, _IONBF, 0); /*Set unbuffered mode for stdout (newlib)*/
  setvbuf(stdin, NULL, _IONBF, 0);  /*Set unbuffered mode for stdin (newlib)*/
#endif

  /* Output on vcom usart instance */
  const char str1[] = "IOstream USART example\r\n\r\n";
  sl_iostream_write(sl_iostream_vcom_handle, str1, strlen(str1));

  /* Setting default stream */
  sl_iostream_set_default(sl_iostream_vcom_handle);
  const char str2[] = "This is output on the default stream\r\n";
  sl_iostream_write(SL_IOSTREAM_STDOUT, str2, strlen(str2));

  /* Using printf */
  /* Writing ASCII art to the VCOM iostream */
  printf("Printf uses the default stream, as long as iostream_retarget_stdio is included.\r\n> ");
}

/***************************************************************************/ /**
 * Example ticking function.
 ******************************************************************************/
void app_iostream_usart_process_action(void)
{
  char c               = 0;
  static uint8_t index = 0;

  /* Retrieve characters, print local echo and full line back */
  sl_iostream_getchar(SL_IOSTREAM_STDIN, &c);
  if (c > 0) {
    if ((c == '\r') || (c == '\n')) {
      buffer[index] = '\0';
      printf("\r\nYou wrote: %s\r\n> ", buffer);
      index = 0;
    } else {
      if (index < BUFFER_SIZE - 1) {
        buffer[index] = (char)c;
        index++;
      }
      /* Local echo */
      sl_iostream_putchar(SL_IOSTREAM_STDOUT, (char)c);
    }
  }
}
