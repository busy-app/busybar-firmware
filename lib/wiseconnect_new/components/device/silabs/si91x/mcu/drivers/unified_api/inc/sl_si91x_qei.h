/******************************************************************************
* @file sl_si91x_qei.h
* @brief QEI (Quadrature Encoder Interface) API header file
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SI91X_QEI_H
#define SL_SI91X_QEI_H

#ifdef __cplusplus
extern "C" {
#endif

//// Includes
#include "rsi_qei.h"
#include "sl_status.h"
#include "sl_qei_board.h"
/***************************************************************************/ /**
 * @addtogroup QEI Quadrature Encoder
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 * 
 ******************************************************************************/
/*******************************************************************************
 ***************************  DEFINES / MACROS  ********************************
 ******************************************************************************/
#define QEI_SOFT_RST_MASK                  0          ///< software reset of the QEI bit
#define QEI_SWAP_PHASE_AB_MASK             1          ///< swapping phase A and B signals bit
#define QEI_POS_CNT_RST_WITH_INDEX_EN_MASK 2          ///< position count reset with index signal bit
#define QEI_POS_CNT_DIRECTION_CTRL_MASK    4          ///< position count direction bit
#define QEI_POS_CNT_DIR_FROM_REG_MASK      5          ///< position count direction from a register bit
#define QEI_INDEX_CNT_RST_EN_MASK          8          ///< enable index count reset bit
#define QEI_DIGITAL_FILTER_BYPASS_MASK     9          ///< bypass the digital filter bit
#define QEI_TIMER_MODE_MASK                10         ///< setting the timer mode of the QEI bit
#define QEI_START_VELOCITY_CNTR_MASK       11         ///< starting the velocity counter bit
#define QEI_STOP_IN_IDLE_MASK              12         ///< stop the QEI in idle state bit
#define QEI_POS_CNT_MODE_MASK              13         ///< selecting the position count mode bit
#define QEI_POS_CNT_RST_MASK               14         ///< resetting the position count bit
#define QEI_INDEX_CNT_RST_MASK             15         ///< resetting the index count bit
#define QEI_ENCODING_MODE_MASK             (0x3 << 0) ///< Encoding mode (2 bits)
#define QEI_INDEX_MATCH_VALUE_MASK         (0x3 << 4) ///< Index match value (2 bits)
#define QEI_DIGITAL_FILTER_DIV_MASK        (0xF << 6) ///< Digital filter division (4 bits)
#define QEI_UNIDIRECTIONAL_VELOCITY_MASK   10         ///< Unidirectional velocity (1 bit)
#define QEI_UNIDIRECTIONAL_INDEX_MASK      11         ///< Unidirectional index (1 bit)
#define QEI_INDEX_CNT_INIT_MASK            12         ///< Index count initialization (1 bit)

/*******************************************************************************
 ********************************   Local Variables   **************************
 ******************************************************************************/
/** @brief Callback function type for QEI events. */
typedef void (*sl_qei_callback_t)(void *callback_flag);
/*******************************************************************************
 ***************************  Global  VARIABLES ********************************
 ******************************************************************************/

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/
/***************************************************************************/
/**
 * @brief Enumeration to control the QEI module.
 *
 * @details This enumeration manages the setting and clearing of the module and control configuration registers.
 */
typedef enum {
  SL_QEI_RESET,     ///< Reset the configurations of the module and control registers.
  SL_QEI_SET,       ///< Set the configurations of the module and control registers.
  SL_QEI_STATE_LAST ///< Last member of the enum for validation purposes.
} sl_qei_control_t;

/**
 * @brief Enumeration for QEI phase swap control.
 *
 * @details This enumeration manages the phase A and phase B input swap select bit.
 */
typedef enum {
  SL_QEI_NO_SWAP_AB, ///< No swap for phase A and B.
  SL_QEI_SWAP_AB,    ///< Swap phase A and B.
  SL_QEI_SWAP_LAST   ///< Last member of the enum for validation purposes.
} sl_qei_swap_ab_t;

/**
 * @brief Enumeration for QEI timer modes.
 *
 * @details This enumeration selects the mode for the quadrature encoder or timer.
 */
typedef enum {
  SL_QEI_ENCODER_MODE, ///< Set QEI to encoder mode.
  SL_QEI_TIMER_MODE,   ///< Set QEI to timer mode.
  SL_QEI_MODE_LAST     ///< Last member of the enum for validation purposes.
} sl_qei_timer_mode_t;

/**
 * @brief Enumeration for QEI encoding modes.
 *
 * @details This enumeration controls the encoding modes for programmable 1x, 2x, or 4x position counting.
 */
typedef enum {
  SL_QEI_ENCODE_MODE_1X,  ///< 1x position counting mode.
  SL_QEI_ENCODE_MODE_2X,  ///< 2x position counting mode.
  SL_QEI_ENCODE_MODE_4X,  ///< 4x position counting mode.
  SL_QEI_ENCODE_MODE_LAST ///< Last member of the enum for validation purposes.
} sl_qei_encoding_mode_t;

/**
 * @brief Enumeration for QEI direction.
 *
 * @details This enumeration defines the possible directions for the Quadrature Encoder
 *          Interface (QEI).
 */
typedef enum {
  SL_QEI_NEG_DIRECTION, ///< Negative direction of rotation (e.g., counterclockwise).
  SL_QEI_POS_DIRECTION, ///< Positive direction of rotation (e.g., clockwise).
  SL_QEI_DIRECTION_LAST ///< Last member of the enum for validation purposes
} sl_qei_direction_t;

/**
 * @brief Enumeration for QEI position counting modes.
 *
 * @details This enumeration defines the position counting mode(16/32-bit) in the
 *          Quadrature Encoder Interface (QEI).
 */
typedef enum {
  SL_QEI_POS_CNT_32,  ///< Mode for 32-bit position counting.
  SL_QEI_POS_CNT_16,  ///< Mode for 16-bit position counting.
  SL_QEI_POS_CNT_LAST ///< Last member of the enum for validation purposes.
} sl_qei_pos_cnt_mode_t;

/**
 * @brief Enumeration for QEI digital filter settings.
 *
 * @details This enumeration defines the settings for digital filtering
 *          in the Quadrature Encoder Interface (QEI).
 */
typedef enum {
  SL_QEI_DIGITAL_FILTER,     ///< Enable digital filtering.
  SL_QEI_BYPASS_FILTER,      ///< digital filtering (bypass mode).
  SL_QEI_DIGITAL_FILTER_LAST ///< Last member of the enum for validation purposes.
} sl_qei_digital_filter_t;

/**
 * @brief Enumeration for unmasking QEI interrupts.
 *
 * @details This enumeration manages the unmasking of QEI interrupts.
 */
typedef enum {
  SL_QEI_POS_CNT_RESET_INTR_UNMASK      = BIT(0), ///< Unmask position counter reset interrupt.
  SL_QEI_IDX_CNT_MATCH_INTR_UNMASK      = BIT(1), ///< Unmask index count match interrupt.
  SL_QEI_POS_CNT_ERR_INTR_UNMASK        = BIT(2), ///< Unmask position counter error interrupt.
  SL_QEI_VELOCITY_LESS_INTR_UNMASK      = BIT(3), ///< Unmask velocity less than interrupt.
  SL_QEI_POS_CNT_MATCH_INTR_UNMASK      = BIT(4), ///< Unmask position count match interrupt.
  SL_QEI_VELOCITY_COMP_OVER_INTR_UNMASK = BIT(5), ///< Unmask velocity computation overflow interrupt.
  SL_QEI_PARAM_UNMASK_LAST                        ///< Last member of the enum for validation purposes.
} sl_qei_intr_unmask_config_t;

/**
 * @brief Enumeration for masking QEI interrupts.
 *
 * @details This enumeration manages the masking of QEI interrupts.
 */
typedef enum {
  SL_QEI_POS_CNT_RESET_INTR_MASK      = BIT(0), ///< Mask position counter reset interrupt.
  SL_QEI_IDX_CNT_MATCH_INTR_MASK      = BIT(1), ///< Mask index count match interrupt.
  SL_QEI_POS_CNT_ERR_INTR_MASK        = BIT(2), ///< Mask position counter error interrupt.
  SL_QEI_VELOCITY_LESS_INTR_MASK      = BIT(3), ///< Mask velocity less than interrupt.
  SL_QEI_POS_CNT_MATCH_INTR_MASK      = BIT(4), ///< Mask position count match interrupt.
  SL_QEI_VELOCITY_COMP_OVER_INTR_MASK = BIT(5), ///< Mask velocity computation overflow interrupt.
  SL_QEI_PARAM_MASK_LAST                        ///< Last member of the enum for validation purposes.
} sl_qei_intr_mask_config_t;

/**
 * @brief Enumeration for clearing QEI interrupts.
 *
 * @details This enumeration manages the clearing of QEI interrupts.
 */
typedef enum {
  SL_QEI_POS_CNT_RESET_INTR_LVL      = BIT(0), ///< Clear position counter reset interrupt.
  SL_QEI_IDX_CNT_MATCH_INTR_LVL      = BIT(1), ///< Clear index count match interrupt.
  SL_QEI_POS_CNT_ERR_INTR_LVL        = BIT(2), ///< Clear position counter error interrupt.
  SL_QEI_VELOCITY_LESS_INTR_LVL      = BIT(3), ///< Clear velocity less than interrupt.
  SL_QEI_POS_CNT_MATCH_INTR_LVL      = BIT(4), ///< Clear position count match interrupt.
  SL_QEI_VELOCITY_COMP_OVER_INTR_LVL = BIT(5), ///< Clear velocity computation overflow interrupt.
  SL_QEI_PARAM_CLEAR_LAST                      ///< Last member of the enum for validation purposes.
} sl_qei_intr_clear_config_t;

/**
 * @brief Enumeration for QEI control initialization configuration.
 *
 * @details This enumeration specifies the initialization configuration for QEI control.
 */
typedef enum {
  SL_QEI_UNIDIRECTIONAL_VELOCITY = QEI_UNIDIRECTIONAL_VELOCITY, ///< Enable unidirectional velocity.
  SL_QEI_UNIDIRECTIONAL_INDEX    = QEI_UNIDIRECTIONAL_INDEX,    ///< Enable unidirectional index.
  SL_QEI_INDEX_COUNT_INIT        = QEI_INDEX_CNT_INIT,          ///< Initialize index counter.
  SL_QEI_CTRL_INIT_LAST                                         ///< Last member of the enum for validation purposes.
} sl_qei_ctrl_init_config_t;

/**
 * @brief Enumeration for QEI control configuration.
 *
 * @details This enumeration specifies the control configuration parameters for QEI.
 */
typedef enum {
  SL_QEI_DIGITAL_FILTER_BYPASS   = QEI_DIGITAL_FILTER_BYPASS,   ///< Bypass digital filter.
  SL_QEI_INDEX_COUNT_RESET       = QEI_IDX_CNT_RST,             ///< Index counter reset.
  SL_QEI_IDX_CNT_RST_EN          = QEI_IDX_CNT_RST_EN,          ///< Enable index counter reset.
  SL_QEI_POS_CNT_16_BIT_MDE      = QEI_POS_CNT_16_BIT_MDE,      ///< Set position counter to 16-bit mode.
  SL_QEI_POS_CNT_DIR_FRM_REG     = QEI_POS_CNT_DIR_FRM_REG,     ///< Set position counter direction from register.
  SL_QEI_POS_CNT_DIR_CTRL        = QEI_POS_CNT_DIR_CTRL,        ///< Control direction of the position counter.
  SL_QEI_POS_CNT_RST             = QEI_POS_CNT_RST,             ///< Reset position counter.
  SL_QEI_POS_CNT_RST_WITH_IDX_EN = QEI_POS_CNT_RST_WITH_IDX_EN, ///< Reset position counter with index enable.
  SL_QEI_START_VELOCITY_CNTR     = QEI_START_VELOCITY_CNTR,     ///< Start velocity counter.
  SL_QEI_STOP_IN_IDLE            = QEI_STOP_IN_IDLE,            ///< Stop in idle state.
  SL_QEI_SWAP_PHASE_AB_B         = QEI_SWAP_PHASE_AB_B,         ///< Swap phases A and B.
  SL_QEI_INTERFACE_TIMER_MODE    = QEI_TIMER_MODE,              ///< Set to timer mode.
  SL_QEI_CONTROL_CONFIG_LAST                                    ///< Last member of the enum for validation purposes.
} sl_qei_control_config_t;

/**
 * @brief Enumeration for QEI digital filter clock divide select bits.
 *
 * @details This enumeration specifies the control configuration digital filter clock divide parameter for QEI.
 */
typedef enum {
  SL_QEI_CLK_DIV_1    = QEI_CLK_DIV_1,    ///< 1:1 Clock divide for Index, position A & B
  SL_QEI_CLK_DIV_2    = QEI_CLK_DIV_2,    ///< 1:2 Clock divide for Index, position A & B
  SL_QEI_CLK_DIV_4    = QEI_CLK_DIV_4,    ///< 1:4 Clock divide for Index, position A & B
  SL_QEI_CLK_DIV_8    = QEI_CLK_DIV_8,    ///< 1:8 Clock divide for Index, position A & B
  SL_QEI_CLK_DIV_16   = QEI_CLK_DIV_16,   ///< 1:16 Clock divide for Index, position A & B
  SL_QEI_CLK_DIV_32   = QEI_CLK_DIV_32,   ///< 1:32 Clock divide for Index, position A & B
  SL_QEI_CLK_DIV_64   = QEI_CLK_DIV_64,   ///< 1:64 Clock divide for Index, position A & B
  SL_QEI_CLK_DIV_128  = QEI_CLK_DIV_128,  ///< 1:128 Clock divide for Index, position A & B
  SL_QEI_CLK_DIV_256  = QEI_CLK_DIV_256,  ///< 1:256 Clock divide for Index, position A & B
  SL_QEI_CLK_DIV_512  = QEI_CLK_DIV_512,  ///< 1:512 Clock divide for Index, position A & B
  SL_QEI_CLK_DIV_1024 = QEI_CLK_DIV_1024, ///< 1:1024 Clock divide for Index, position A & B
  SL_QEI_CLK_DIV_LAST                     ///< Last member of the enum for validation purposes.
} sl_qei_clk_div_t;

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/
/***************************************************************************/
/**
 * @brief Structure to hold the different versions of the peripheral API.
 *
 * @details This structure defines the versioning information for the peripheral API.
 *          It includes the release, major, and minor version numbers.
 */
typedef struct {
  uint8_t release; ///< Release version number.
  uint8_t major;   ///< Major version number.
  uint8_t minor;   ///< Minor version number.
} sl_qei_version_t;

/**
 * @brief QEI configuration structure.
 *
 * @details This structure contains configuration parameters and the initialization settings
 *           for the Quadrature Encoder Interface (QEI)
 */
typedef struct {
  uint32_t config_param; ///< This have QEI configuration parameters like encoder soft reset, swap A, B phases etc
  uint32_t
    ctrl_param; ///< This have QEI control initialization parameters like Positing counting modes, digital filter clock divide etc
} sl_qei_config_t;

/**
 * @brief QEI interrupt acknowledge structure.
 *
 * @details This structure contains flags for acknowledging different interrupt levels for the QEI.
 */
typedef struct {
  uint32_t qei_position_cnt_reset_intr_lev;        ///< Position count reset interrupt level.
  uint32_t qei_index_cnt_match_intr_lev;           ///< Index count match interrupt level.
  uint32_t qei_position_cntr_err_intr_lev;         ///< Position counter error interrupt level.
  uint32_t qei_velocity_less_than_intr_lev;        ///< Velocity less than interrupt level.
  uint32_t qei_position_cnt_match_intr_lev;        ///< Position count match interrupt level.
  uint32_t qei_velocity_computation_over_intr_lev; ///< Velocity computation over interrupt level.
} sl_qei_intr_ack_t;

/**
 * @brief QEI interrupt mask structure.
 *
 * @details This structure defines masks for various QEI interrupts, controlling which interrupts are enabled.
 */
typedef struct {
  uint32_t qei_position_cnt_reset_intr_mask;        ///< Mask for position count reset interrupt.
  uint32_t qei_index_cnt_match_intr_mask;           ///< Mask for index count match interrupt.
  uint32_t qei_position_cntr_err_intr_mask;         ///< Mask for position counter error interrupt.
  uint32_t qei_velocity_less_than_intr_mask;        ///< Mask for velocity less than interrupt.
  uint32_t qei_position_cnt_match_intr_mask;        ///< Mask for position count match interrupt.
  uint32_t qei_velocity_computation_over_intr_mask; ///< Mask for velocity computation over interrupt.
} sl_qei_intr_mask_t;

/**
 * @brief QEI interrupt unmask structure.
 *
 * @details This structure defines unmasking options for various QEI interrupts, allowing them to be enabled.
 */
typedef struct {
  uint32_t qei_position_cnt_reset_intr_unmask;        ///< Unmask for position count reset interrupt.
  uint32_t qei_index_cnt_match_intr_unmask;           ///< Unmask for index count match interrupt.
  uint32_t qei_velocity_less_than_intr_unmask;        ///< Unmask for velocity less than interrupt.
  uint32_t qei_position_cntr_err_intr_unmask;         ///< Unmask for position counter error interrupt.
  uint32_t qei_position_cnt_match_intr_unmask;        ///< Unmask for position count match interrupt.
  uint32_t qei_velocity_computation_over_intr_unmask; ///< Unmask for velocity computation over interrupt.
} sl_qei_intr_unmask_t;

/**
 * @brief Structure to hold GPIO pin configuration parameters.
 *
 * @details This structure defines the configuration settings for a GPIO pin, including
 *          its port, pin number, multiplexing function, and pad configuration.
 *          It is used to configure GPIO pins for various functionalities in the system.
 */
typedef struct {
  uint8_t port; ///< GPIO port. Specifies the port number for the GPIO pin.
  uint8_t pin;  ///< GPIO pin. Specifies the pin number within the specified port.
  uint8_t mux;  ///< GPIO mux. Defines the multiplexing function for the pin.
  uint8_t pad;  ///< GPIO pad. Indicates the pad configuration for the pin.
} sl_gpio_pin_config_t;

/**
 * @brief Structure to hold the port and pin configurations for different events.
 *
 * @details This structure defines the configuration parameters for the QEI GPIO's.
 *          It includes settings for the port, pin, mux, and pad for DIR, IDX, PHASEA, PHASEB
 */
typedef struct {
  sl_gpio_pin_config_t dir;    ///< DIR GPIO configuration.
  sl_gpio_pin_config_t idx;    ///< IDX GPIO configuration.
  sl_gpio_pin_config_t phasea; ///< PHASE_A GPIO configuration.
  sl_gpio_pin_config_t phaseb; ///< PHASE_B GPIO configuration.
} sl_qei_init_t;
/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************/
/***************************************************************************/
/**
 * @brief Initialize the QEI module.
 *
 * @details This function initializes the QEI module, setting it up for operation.
 *          It should be called before using any other QEI functions.
 *
 * @param[in] qei_init Pointer to the structure of type \ref sl_qei_init_t.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_NULL_POINTER (0x0022) - The parameter is a null pointer.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_init(sl_qei_init_t *qei_init);

/***************************************************************************/
/**
 * @brief De-initialize the QEI module.
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_qei_init
 *
 * @details This function cleans up and deinitializes the QEI module, ensuring
 *          that all resources are released and the module is no longer active.
 ***************************************************************************/
void sl_si91x_qei_deinit(void);

/***************************************************************************/
/**
 * @brief Set the position counter value.
 *
 * @details This function sets the value of the position counter. The position counter
 *          is used to track the current position in applications such as rotary encoders.
 *
 * @param[in] pos_cnt_value Position counter value to set.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_set_position_counter(uint32_t pos_cnt_value);

/***************************************************************************/
/**
 * @brief Start the velocity counter.
 *
 * @details This function starts the velocity counter. This is required for managing
 *          when velocity calculations should take place.
 *
 * @param[in] period Delta time value to set. This timing information is essential for
 *            calculating the velocity of a rotating shaft or moving part by determining
 *            how quickly the position changes over a specified period.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_start_velocity(uint32_t period);

/***************************************************************************/
/**
 * @brief Get the current position counter value.
 *
 * @details This function retrieves the current value of the position counter,
 *          which reflects the position state of the QEI module.
 *
 * @return Returns the current position counter value.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
__STATIC_INLINE uint32_t sl_si91x_qei_get_position_counter(void)
{
  // Get the current position counter value
  return RSI_QEI_GetPosition(QEI);
}

/***************************************************************************/
/**
 * @brief Set the encoding mode for the QEI.
 *
 * @details This function configures the encoding mode of the Quadrature Encoder Interface
 *          (QEI). Different encoding modes can impact how position and direction are computed.
 *
 * @param[in] mode Encoding mode to set of type \ref sl_qei_encoding_mode_t.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_set_encoding_mode(sl_qei_encoding_mode_t mode);

/***************************************************************************/
/**
 * @brief Get the current encoding mode.
 *
 * @details This function retrieves the currently configured encoding mode for the QEI.
 *          This is useful for understanding how the position and direction are being interpreted.
 *
 * @return Returns the current encoding mode.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
__STATIC_INLINE sl_qei_encoding_mode_t sl_si91x_qei_get_encoding_mode(void)
{
  // Get the current encoding mode
  return RSI_QEI_GetMode(QEI);
}

/***************************************************************************/
/**
 * @brief Set the direction for the QEI.
 *
 * @details This function sets the direction of the QEI. This is critical for applications
 *          where direction affects how the position is interpreted (e.g., clockwise vs.
 *          counterclockwise).
 *
 * @param[in] direction Direction to set of type \ref sl_qei_direction_t.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_set_pos_direction(sl_qei_direction_t direction);

/***************************************************************************/
/**
 * @brief Get the current direction of the QEI.
 *
 * @details This function retrieves the current direction setting of the QEI, indicating
 *          whether the encoder is configured to interpret movement as forward or backward.
 *
 * @return Returns the current direction.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
__STATIC_INLINE sl_qei_direction_t sl_si91x_qei_get_direction(void)
{
  // Get the current direction
  return RSI_QEI_GetDirection(QEI);
}

/***************************************************************************/
/**
 * @brief Set the index counter value.
 *
 * @details This function sets the index counter value, which can be used in applications
 *          that require indexing or referencing specific positions.
 *
 * @param[in] index_count Index counter value to set.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_set_index_counter(uint32_t index_count);

/***************************************************************************/
/**
 * @brief Get the current index counter value.
 *
 * @details This function retrieves the current index counter value, which can be useful
 *          for tracking indexed positions or for synchronization purposes.
 *
 * @return Returns the current index counter value.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
__STATIC_INLINE uint32_t sl_si91x_qei_get_index_counter(void)
{
  // Get the current index counter value
  return RSI_QEI_GetIndex(QEI);
}

/***************************************************************************/
/**
 * @brief Configure the delta time frequency for velocity calculation.
 *
 * @details This function sets up the delta time frequency and period to be used in
 *          velocity calculations. Proper configuration ensures accurate velocity readings.
 *
 * @param[in] freq Frequency for delta time.
 * @param[in] period Period for delta time.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_configure_delta_time_frequency(uint32_t freq, uint32_t period);

/***************************************************************************/
/**
 * @brief Set the delta time for velocity calculation.
 *
 * @details This function sets the delta time value, which is critical for calculating
 *          velocity based on the position changes over time.
 *
 * @param[in] delta_time Delta time to set.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_set_delta_time(uint32_t delta_time);

/***************************************************************************/
/**
 * @brief Get the current delta time.
 *
 * @details This function retrieves the current delta time value being used for velocity
 *          calculations, providing insight into the timing configuration.
 *
 * @return Returns the current delta time.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
__STATIC_INLINE uint32_t sl_si91x_qei_get_delta_time(void)
{
  // Get the current delta time
  return RSI_QEI_GetDeltaTime(QEI);
}

/***************************************************************************/
/**
 * @brief Configure the stop state for the velocity counter.
 *
 * @details This function stops the velocity counter.
 ***************************************************************************/
__STATIC_INLINE void sl_si91x_qei_stop_velocity(void)
{
  RSI_QEI_StopVelocityCounter(QEI);
}

/***************************************************************************/
/**
 * @brief Get the current velocity value.
 *
 * @details This function retrieves the current calculated velocity, which can be used in
 *          applications requiring real-time speed monitoring.
 *
 * @return Returns the current velocity.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
__STATIC_INLINE uint32_t sl_si91x_qei_get_velocity(void)
{
  // Get the current velocity
  return RSI_QEI_GetVelocity(QEI);
}

/***************************************************************************/
/**
 * @brief Set the position match value.
 *
 * @details This function sets a specific position match value. The QEI can generate
 *          interrupts or events when this position is reached.
 *
 * @param[in] pos_match Position match value to set.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_set_position_match(uint32_t pos_match);

/***************************************************************************/
/**
 * @brief Get the current position match value.
 *
 * @details This function retrieves the currently set position match value,
 *          which helps in understanding the QEI's tracking and event
 *          generation capabilities.
 *
 * @return  Returns the current position match value.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
__STATIC_INLINE uint32_t sl_si91x_qei_get_position_match(void)
{
  // Get the current position match value
  return RSI_QEI_GetPosMatch(QEI);
}

/***************************************************************************/
/**
 * @brief Set a value for velocity comparison.
 *
 * @details This function sets a threshold velocity value, which can be used
 *          for comparing against the current velocity to trigger events or actions.
 *
 * @param[in] velocity_value Velocity value to compare against.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_set_velocity_value_to_compare(uint32_t velocity_value);

/***************************************************************************/
/**
 * @brief Set the maximum value for the index counter.
 *
 * @details This function sets the maximum allowable value for the index counter,
 *          which can prevent overflow and ensure proper operation.
 *
 * @param[in] max_index Maximum index counter value to set.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_set_index_maximum_counter(uint32_t max_index);

/***************************************************************************/
/**
 * @brief Get the current maximum value for the index counter.
 *
 * @details This function retrieves the current maximum index counter value,
 *          which helps in validating the index counter's limits and configuration.
 *
 * @return  Returns current maximum index counter value.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
__STATIC_INLINE uint32_t sl_si91x_qei_get_index_maximum_counter(void)
{
  // Get the maximum index value
  return RSI_QEI_GetMaxIndex(QEI);
}

/***************************************************************************/
/**
 * @brief Get the current interrupt status.
 *
 * @details This function retrieves the status of the interrupts related to the QEI module,
 *          providing insights into any triggered conditions or events.
 *
 * @return  Returns the current interrupt status.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
__STATIC_INLINE uint32_t sl_si91x_qei_get_interrupt_status(void)
{
  // Get the interrupt status
  return RSI_QEI_GetIntrStatus(QEI);
}

/***************************************************************************/
/**
 * @brief Swap phases A and B for the QEI.
 *
 * @details This function allows swapping the A and B phases of the encoder signals.
 *          This can be useful for correcting phase issues without hardware changes.
 *
 * @param[in] swap_select Selection for swapping phases of type \ref sl_qei_swap_ab_t
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_swap_a_b(sl_qei_swap_ab_t swap_select);

/***************************************************************************/
/**
 * @brief Configure the QEI to operate in timer mode.
 *
 * @details This function sets the QEI to operate in timer mode, allowing for more precise
 *          timing and position tracking applications.
 *
 * @param[in] timer_mode Timer mode to set of type \ref sl_qei_timer_mode_t
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_configure_timer_mode(sl_qei_timer_mode_t timer_mode);

/***************************************************************************/
/**
 * @brief Clear the interrupt set.
 *
 * @details This function clears interrupts in the QEI module.
 *
 * @param[in] int_mask Masked interrupt to clear
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_clear_interrupt(uint32_t int_mask);

/***************************************************************************/
/**
 * @brief Get the current interrupt clear settings.
 *
 * @details This function retrieves the current settings for clearing interrupts in the QEI
 *          module, useful for managing event handling.
 *
 * @return  Returns the current interrupt clear settings.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
__STATIC_INLINE uint32_t sl_si91x_qei_get_interrupt_clear(void)
{
  // Get the clear interrupt status
  return RSI_QEI_GetClrIntrStatus(QEI);
}

/***************************************************************************/
/**
 * @brief Get the status of the QEI module.
 *
 * @details This function retrieves the operational status of the QEI module,
 *          which can be useful for diagnostics and monitoring its state.
 *
 * @return Returns the current module status. This will read direct value from
 *          position signal geneartor, position counter direction, counter error status flag.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
__STATIC_INLINE uint32_t sl_si91x_qei_get_status(void)
{
  // Get the module status
  return RSI_QEI_GetStatus(QEI);
}

/***************************************************************************/
/**
 * @brief Set the frequency for the QEI module.
 *
 * @details This function sets the operational frequency for the QEI module.
 *          The frequency affects the responsiveness and timing accuracy of the module.
 *
 * @param[in] frequency Frequency value to set.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_set_module_frequency(uint32_t frequency);

/***************************************************************************/
/**
 * @brief Get the current frequency of the QEI module.
 *
 * @details This function retrieves the current operational frequency for the QEI module,
 *          which can help in performance tuning and analysis.
 *
 * @@return  Returns the current frequency value.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
__STATIC_INLINE uint32_t sl_si91x_qei_get_module_frequency(void)
{
  // Get the module frequency
  return RSI_QEI_GetModuleFreq(QEI);
}

/***************************************************************************/
/**
 * @brief Set the digital filter clock division.
 *
 * @details This function sets the clock division factor for the digital filter in the QEI
 *          module, which can help in reducing noise and improving signal integrity.
 *
 * @param[in] clock_division Clock division value to set \ref sl_qei_clk_div_t
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_set_digital_filter_clock_division(sl_qei_clk_div_t clock_division);

/***************************************************************************/
/**
 * @brief Get the current digital filter clock division.
 *
 * @details This function retrieves the current clock division value for the digital filter
 *          in the QEI module, useful for validating filter settings.
 *
 * @@return  current clock division value.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
__STATIC_INLINE sl_qei_clk_div_t sl_si91x_qei_get_digital_filter_clock_division(void)
{
  // Get the clock division
  return RSI_QEI_GetDigitalFilterClkDiv(QEI);
}

/***************************************************************************/
/**
 * @brief Register a callback function for QEI events.
 *
 * @details This function allows users to register a callback function that will be called
 *          on specific QEI events. This is essential for responsive applications that need
 *          to react to state changes.
 *
 * @param[in] callback Callback function to register of type \ref sl_qei_callback_t
 * @param[in] callback_flag_value Pointer to value passed to the callback.
 * @param[in] intr_flags Pointer to interrupt unmask settings of type \ref sl_qei_intr_unmask_t
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_NULL_POINTER (0x0022) - The parameter is a null pointer.
 *         - SL_STATUS_BUSY (0x0004) - Busy
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_register_callback(sl_qei_callback_t callback,
                                           void *callback_flag_value,
                                           sl_qei_intr_unmask_t *intr_flags);

/***************************************************************************/
/**
 * @brief Unregister a callback function for QEI events.
 *
 * @details This function removes a previously registered callback function, stopping it from
 *          being called on QEI events.
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_qei_register_callback
 *
 * @param[in] intr_mask Pointer to interrupt mask settings of type \ref sl_qei_intr_mask_t
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_NULL_POINTER (0x0022) - The parameter is a null pointer.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_unregister_callback(sl_qei_intr_mask_t *intr_mask);

/***************************************************************************/
/**
 * @brief Set the maximum position counter value.
 *
 * @details This function sets a maximum value for the position counter,
 *          helping to prevent overflow and ensuring reliable operation of
 *          the QEI in applications. Position match value to compare the position
 *          counter. When it is matched with position counter, interrupt is raised.
 *
 * @param[in] max_pos_cnt Maximum position counter value to set.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_set_maximum_position_counter(uint32_t max_pos_cnt);

/***************************************************************************/
/**
 * @brief Get the current maximum position counter value.
 *
 * @details This function retrieves the maximum position counter value that has been set,
 *          which can be useful for validating limits and configurations.
 *
 * @@return  Returns the current maximum position counter value.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
__STATIC_INLINE uint32_t sl_si91x_qei_get_maximum_position_counter(void)
{
  // Get the maximum position counter value
  return RSI_QEI_GetMaxPosCnt(QEI);
}

/***************************************************************************/
/**
 * @brief Set configuration parameters for the QEI module.
 *
 * @details This function applies specific configuration parameters to the QEI module,
 *          allowing for customized operation based on the needs of the application.
 *
 * @param[in] state State to reset/set module configurations of type \ref sl_qei_control_t.
 * @param[in] config Pointer to configuration structure of type \ref sl_qei_config_t.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *         - SL_STATUS_NULL_POINTER (0x0022) - The parameter is a null pointer.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_set_configuration(sl_qei_control_t state, sl_qei_config_t *config);

/***************************************************************************/
/**
 * @brief Get configuration parameters for the QEI module.
 *
 * @details This function retrieves the current configuration parameters of the QEI module,
 *          enabling verification of settings and operational parameters.
 *
 * @param[in] state Get module configurations of type \ref sl_qei_control_t.
 * @param[out] config Pointer to store configuration parameters \ref sl_qei_config_t
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - The parameter is an invalid argument.
 *         - SL_STATUS_NULL_POINTER (0x0022) - The parameter is a null pointer.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_qei_get_configuration(sl_qei_control_t state, sl_qei_config_t *config);

/***************************************************************************/
/**
 * @brief To get the API version of the QEI module.
 *
 * @details This API retrieves the version information of the QEI module,
 *          which includes the release, SQA, and DEV version numbers.
 *
 * @return Returns a structure of type \ref sl_qei_version_t.
 ***************************************************************************/
sl_qei_version_t sl_si91x_qei_get_version(void);

/// @} end group QEI ********************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************

/**  @addtogroup QEI Quadrature Encoder
 * @{
 *
 * @details
 *
 * @section QEI_Intro Introduction
 *
 * A Quadrature Encoder (QE), also known as a 2-channel incremental encoder, converts angular displacement into
 * two pulse signals. These two pulses are positioned 90 degrees out of phase. By monitoring both the number of
 * pulses and the relative phase of the two signals, the user code can track the position, direction of rotation,
 * and velocity. The index signal can also be used to reset the position counter.
 *
 * The quadrature encoder decodes the digital pulses from a quadrature encoder wheel to integrate position over
 * time and determine direction of rotation. Additionally, the QEI can capture the velocity of the encoder wheel.
 * The QEI is present in MCU HP peripherals.
 *
 * **Features of QEI**
 * 1. Tracks encoder wheel position \ref sl_si91x_qei_get_position_counter()
 * 2. Programmable for 1x, 2x, or 4x position counting, incrementing or decrementing depending on direction \ref sl_si91x_qei_set_encoding_mode()
 * 3. Includes an index counter for revolution counting \ref sl_si91x_qei_get_index_counter()
 * 4. Captures velocity using a built-in timer \ref sl_si91x_qei_get_velocity()
 * 5. Supports position counter reset for rollover/underflow or index pulse \ref sl_si91x_qei_set_configuration()
 * 6. Provides position, index, and velocity compare registers with interrupts \ref sl_si91x_qei_register_callback()
 * 7. Allows logically swapping the A and B inputs \ref sl_si91x_qei_swap_a_b()
 * 8. Accepts decoded signal inputs (clock and direction) in timer mode \ref sl_si91x_qei_configure_timer_mode()
 */
/** @} end group QEI */

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_QEI_H */
