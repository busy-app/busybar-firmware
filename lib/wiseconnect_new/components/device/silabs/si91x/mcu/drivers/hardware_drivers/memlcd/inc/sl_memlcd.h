/***************************************************************************/ /**
 * @file
 * @brief Memory LCD interface
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

#ifndef SL_MEMLCD_H
#define SL_MEMLCD_H

#include "rsi_timers.h"
#include "sl_status.h"

#if !defined(MEMLCD_CUSTOM_DRIVER)
#include "sl_memlcd_spi.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// Macros for MemLCD color modes.
#define SL_MEMLCD_COLOR_MODE_MONOCHROME 1 ///< monochrome color mode for the MemLCD.
#define SL_MEMLCD_COLOR_MODE_RGB_3BIT   2 ///< RGB 3-bit color mode for the MemLCD.

/** @addtogroup MEMLCD
 * @ingroup SI91X_HARDWARE_DRIVER_APIS
 *
 * The memory LCD interface is built to use SPI communication either with USART 
 * or EUSART. Configurations for three standard display are already supported 
 * (lpm013m126a, ls013b7dh03 and ls013b7dh06), but the @ref sl_memlcd_t 
 * structure can also be configured with a custom LCD's specifications.
 * To implement a fully custom memory LCD interface.
 *
 * @{
 */

/**
 * General memory LCD data structure.
 */
typedef struct sl_memlcd_t {
  unsigned short width;  ///< Display pixel width.
  unsigned short height; ///< Display pixel height.
  uint8_t bpp;           ///< Bits per pixel.
  uint8_t color_mode;    ///< Color mode.
  int spi_freq;          ///< SPI clock frequency.
  uint8_t extcomin_freq; ///< Extcomin pin toggle frequency.
  uint8_t setup_us;      ///< SPI CS setup time.
  uint8_t hold_us;       ///< SPI CS hold time.
  void *custom_data;     ///< Custom LCD's data.
} sl_memlcd_t;
/// @brief Enumeration to represent the ulp-timer instances.

/***************************************************************************/
/**
 * @brief Configures the memory LCD device.
 * 
 * @details This API configures the memory LCD device. It must be called to configure the memory LCD device 
 * before starting to use the memory LCD.
 * 
 * @param[in] device Memory LCD Display device pointer.
 * 
 * @return Status code of the operation:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Failure.
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_memlcd_configure(struct sl_memlcd_t *device);
/***************************************************************************/
/**
 * @brief To enable the memory LCD display.
 *
 * @details This API must be called to enable MEMLCD display.
******************************************************************************/
void sl_memlcd_display_enable(void);

/***************************************************************************/
/**
 * @brief To disable the memory LCD display.
 *
 * @details This API must be called to disable MEMLCD display.
******************************************************************************/
void sl_memlcd_display_disable(void);

/***************************************************************************/
/**
 * @brief To enable or disable the display.
 *
 * @details This API enables or disables the display. Disabling the display 
 * does not result in data loss. Note that this API does not control the 
 * DISP pin on the display. This pin is managed by board-specific code.
 *
 * @param[in] device Display device pointer.
 *
 * @param[in] on Set this parameter to 'true' to enable the display. Set to 'false' in
 * order to disable the display.
 *
 * @return status code of the operation.
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Failure.
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
******************************************************************************/

sl_status_t sl_memlcd_power_on(const struct sl_memlcd_t *device, bool on);

/***************************************************************************/
/**
 * @brief Handles MEMLCD initialization after wakeup.
 * 
 * @details This API handles the initialization of the MEMLCD after the system wakes up 
 * from a low-power state.
 * 
 * @return Status code of the operation:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Failure.
 *  For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_memlcd_post_wakeup_init(void);

/***************************************************************************/
/**
 * @brief Clears the display.
 *
 * @details This API will clear the display of memory LCD.
 * @param[in] device Memory LCD Display device pointer.
 *
 * @return Status code of the operation.
 *  For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
******************************************************************************/
sl_status_t sl_memlcd_clear(const struct sl_memlcd_t *device);

/***************************************************************************/
/**
 * @brief Draws a number of rows to the memory LCD display.
 * 
 * @details This API will draw a specified number of rows to the memory LCD display.
 * The `data` parameter points to the pixel matrix buffer to draw, and its format 
 * depends on the color mode of the memory LCD display.
 * 
 * @param[in] device Memory LCD Display device pointer.
 * @param[in] data Pointer to the pixel matrix buffer to draw. The format of the buffer
 *                 depends on the color mode of the memory LCD display.
 * @param[in] row_start Start row on the display where to start drawing. The first row is 0.
 * @param[in] row_count Number of rows to draw.
 * 
 * @return SL_STATUS_OK if there are no errors.
 *  For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_memlcd_draw(const struct sl_memlcd_t *device,
                           const void *data,
                           unsigned int row_start,
                           unsigned int row_count);

/***************************************************************************/
/**
 * @brief Refreshes the display device.
 * 
 * This API refreshes the memory LCD display. It is typically used to re-initialize 
 * the SPI clock rate after a clock configuration change.
 * 
 * @param[in] device Memory LCD Display device pointer.
 * 
 * @return Status code of the operation:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Failure.
 *  For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_memlcd_refresh(const struct sl_memlcd_t *device);

/***************************************************************************/
/**
 * @brief Gets a handle to the memory LCD.
 * 
 * This function returns a handle to the memory LCD structure. If no memory LCD 
 * is initialized yet, it returns NULL.
 * 
 * @return Pointer to the memory LCD structure or NULL if no memory LCD is initialized.
 ******************************************************************************/
const sl_memlcd_t *sl_memlcd_get(void);

/// @} end group MEMLCD ********************************************************/

// ***************************************************************************/
/**
* @addtogroup MEMLCD
* @{
*
*  @details
*
*   @section MEMLCD_Custom Custom-Memory-LCD
*
*   The custom memory LCD module enables the implementation of an
*   LCD driver that is incompatible with existing interfaces. To
*   accomplish this, the following conditions must be fulfilled.
*
*   The LCD driver and the communication needs to be implemented manually.
*   However, some APIs must be implemented and the @ref sl_memlcd_t
*   structure must be initialized for the GLIB library to work properly.
*   The APIs to implement are [sl_memlcd_init](https://docs.silabs.com/gecko-platform/latest/platform-hardware-driver/memlcd#sl-memlcd-init), @ref sl_memlcd_power_on,
*   @ref sl_memlcd_draw and @ref sl_memlcd_get and they must follow the same
*   declaration as shown in the documentation.
*
*   In the [sl_memlcd_init](https://docs.silabs.com/gecko-platform/latest/platform-hardware-driver/memlcd#sl-memlcd-init) API, it is important to initialize a
*   @ref sl_memlcd_t type variable and fill in the "height" and "width" fields
*   with the real specifications of the LCD display. This same variable must be
*   accessible from the @ref sl_memlcd_get API as the GLIB library will
*   fetch the height and width of the display.
*
*   The @ref sl_memlcd_t structure contains a "custom_data" field to store any
*   custom structure needed for the implementation of the driver.
*
* @} end group MEMLCD ********************************************************/
#ifdef __cplusplus
}
#endif

#endif
