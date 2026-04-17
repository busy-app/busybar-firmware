/***************************************************************************/ /**
 * @file
 * @brief SI91x IO Stream Log Component.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_si91x_iostream_log.h"
#include "sl_iostream.h"
#include "sl_si91x_iostream_log_config.h"

#ifdef SL_CATALOG_DEBUG_SWO_SI91X_PRESENT
#include "sl_si91x_debug_swo.h"
#endif

#ifdef SL_CATALOG_SEGGER_RTT_PRESENT
#include "SEGGER_RTT.h"
#include "sl_rtt_buffer_index.h"
#endif

#ifdef SL_CATALOG_IOSTREAM_RTT_SI91X_PRESENT
#include "sl_si91x_iostream_rtt.h"
#endif

#ifdef SL_CATALOG_IOSTREAM_VUART_SI91X_PRESENT
#include "sl_si91x_iostream_vuart.h"
#endif

#ifdef SL_CATALOG_IOSTREAM_SWO_SI91X_PRESENT
#include "sl_si91x_iostream_swo.h"
#endif

/*******************************************************************************
  *************************** LOCAL VARIABLES *******************************
  ******************************************************************************/

sl_iostream_instance_info_t sl_si91x_log_stream_info;

/*******************************************************************************
  *************************** GLOBAL VARIABLES *******************************
  ******************************************************************************/

#ifdef SL_CATALOG_IOSTREAM_RTT_SI91X_PRESENT
extern sl_iostream_instance_info_t sl_si91x_iostream_instance_rtt_info;
#endif

#ifdef SL_CATALOG_IOSTREAM_VUART_SI91X_PRESENT
extern sl_iostream_instance_info_t sl_si91x_iostream_instance_vuart_info;
#endif

#ifdef SL_CATALOG_IOSTREAM_SWO_SI91X_PRESENT
extern sl_iostream_instance_info_t sl_si91x_iostream_instance_swo_info;
#endif

/*******************************************************************************
 ***************************Local Function Prototypes *****************************
 ******************************************************************************/

#if defined(SL_CATALOG_IOSTREAM_SWO_SI91X_PRESENT) && (IOSTREAM_LOG_TYPE == SL_SI91X_IOSTREAM_SWO_LOG)
/***************************************************************************/ /**
 * Si91x iostream SWO init function.
 *
 * @return 
 *         SL_STATUS_OK   - SWO initialized succesfully
 *         SL_STATUS_FAIL - Unable to initialize SWO
 ******************************************************************************/
__STATIC_INLINE sl_status_t sli_si91x_iostream_log_swo_init(void);

#elif defined(SL_CATALOG_IOSTREAM_RTT_SI91X_PRESENT) && (IOSTREAM_LOG_TYPE == SL_SI91X_IOSTREAM_RTT_LOG)
/***************************************************************************/ /**
 * Si91x iostream RTT init function.
 *
 * @return 
 *         SL_STATUS_OK   - RTT initialized succesfully
 *         SL_STATUS_FAIL - Unable to initialize RTT
 ******************************************************************************/
__STATIC_INLINE sl_status_t sli_si91x_iostream_log_rtt_init(void);

#elif defined(SL_CATALOG_IOSTREAM_VUART_SI91X_PRESENT) && (IOSTREAM_LOG_TYPE == SL_SI91X_IOSTREAM_VUART_LOG)
/***************************************************************************/ /**
 * Si91x iostream Vuart init function.
 *
 * @return 
 *         SL_STATUS_OK   - Vuart initialized succesfully
 *         SL_STATUS_FAIL - Unable to initialize Vuart
 ******************************************************************************/
__STATIC_INLINE sl_status_t sli_si91x_iostream_log_vuart_init(void);
#else
#define INVALID_LOG_TYPE
#endif

#ifndef INVALID_LOG_TYPE
sl_status_t sl_si91x_set_log_iostream(void);
static sl_status_t sli_si91x_iostream_log_init(void);
#endif

/*******************************************************************************
  *************************** FUNCTIONS *******************************
  ******************************************************************************/

#if (IOSTREAM_LOG_TYPE == SL_SI91X_IOSTREAM_SWO_LOG)
#ifdef SL_CATALOG_IOSTREAM_SWO_SI91X_PRESENT
/*******************************************************************************
 * This function initializes iostream swo module
  *******************************************************************************/
sl_status_t sli_si91x_iostream_log_swo_init()
{
  sl_status_t status = SL_STATUS_OK;
  status             = sl_si91x_debug_swo_init();
  if (status != SL_STATUS_OK) {
    return status;
  }
  status = sl_si91x_iostream_swo_init();
  if (status != SL_STATUS_OK) {
    return status;
  }
  return status;
}
/*******************************************************************************
 * This function initalizes swo and sets it as log stream
  *******************************************************************************/
sl_status_t sli_si91x_iostream_log_init(void)
{
  sl_si91x_log_stream_info = sl_si91x_iostream_instance_swo_info;
  return sli_si91x_iostream_log_swo_init();
}
#else
#error IOSTREAM SWO component not installed
#endif
#endif

#if (IOSTREAM_LOG_TYPE == SL_SI91X_IOSTREAM_RTT_LOG)
#ifdef SL_CATALOG_IOSTREAM_RTT_SI91X_PRESENT
/*******************************************************************************
 * This function initializes iostream rtt module
  *******************************************************************************/
sl_status_t sli_si91x_iostream_log_rtt_init()
{
  sl_status_t status = SL_STATUS_OK;
  status             = sl_si91x_iostream_rtt_init();
  if (status != SL_STATUS_OK) {
    return status;
  }
  return status;
}
/*******************************************************************************
 * This function initalizes rtt and sets it as log stream
  *******************************************************************************/
sl_status_t sli_si91x_iostream_log_init(void)
{
  sl_si91x_log_stream_info = sl_si91x_iostream_instance_rtt_info;
  return sli_si91x_iostream_log_rtt_init();
}
#else
#error IOSTREAM RTT component not installed
#endif
#endif

#if (IOSTREAM_LOG_TYPE == SL_SI91X_IOSTREAM_VUART_LOG)
#ifdef SL_CATALOG_IOSTREAM_VUART_SI91X_PRESENT
/*******************************************************************************
 * This function initializes iostream vuart module
  *******************************************************************************/
sl_status_t sli_si91x_iostream_log_vuart_init()
{
  sl_status_t status = SL_STATUS_OK;
  status             = sl_si91x_debug_swo_init();
  if (status != SL_STATUS_OK) {
    return status;
  }
  status = sl_si91x_iostream_vuart_init();
  if (status != SL_STATUS_OK) {
    return status;
  }
  return status;
}
/*******************************************************************************
 * This function initalizes vuart and sets it as log stream
  *******************************************************************************/
sl_status_t sli_si91x_iostream_log_init(void)
{
  sl_si91x_log_stream_info = sl_si91x_iostream_instance_vuart_info;
  return sli_si91x_iostream_log_vuart_init();
}

#else
#error IOSTREAM VUART component not installed
#endif
#endif

#ifndef INVALID_LOG_TYPE
/*******************************************************************************
 * This function disables the stdout buffer and initializes the iostream
  *******************************************************************************/
sl_status_t sl_si91x_iostream_log_init()
{
  sl_status_t status = SL_STATUS_OK;
  setvbuf(stdout, NULL, _IONBF, 0);       //disable STDOUT stream buffer
  status = sli_si91x_iostream_log_init(); //initialize iostream
  if (status != SL_STATUS_OK) {
    return status;
  }
  return status;
}

/*******************************************************************************
 * This function sets stream selected in UC as default iostream
  *******************************************************************************/
sl_status_t sl_si91x_set_log_iostream()
{

  sl_iostream_set_system_default(sl_si91x_log_stream_info.handle);
  return SL_STATUS_OK;
}
#endif