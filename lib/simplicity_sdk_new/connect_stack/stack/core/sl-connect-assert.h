/***************************************************************************//**
 * @brief Connect custom assert.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef __SL_CONNECT_ASSERT_H__
#define __SL_CONNECT_ASSERT_H__

#include <stdio.h>

#ifdef NDEBUG
#define CONNECT_STACK_ASSERT(exp) (void)(exp)
#else
#define CONNECT_STACK_ASSERT(exp)                        \
  do {                                                   \
    if (!(exp)) {                                        \
      printf("ASSERT in %s:%d\r\n", __FILE__, __LINE__); \
      while (1) {}                                       \
    }                                                    \
  } while (0)
#endif // NDEBUG

#endif // __SL_CONNECT_ASSERT_H__
