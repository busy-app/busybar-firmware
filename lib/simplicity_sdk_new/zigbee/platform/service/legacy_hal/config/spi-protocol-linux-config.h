/***************************************************************************//**
 * @brief SPI Protocol Linux Config
 *\n*******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

// <<< Use Configuration Wizard in Context Menu >>>

// <h> SPI and GPI configurations

// <s NCP_SPI_DEVICE> SPI device file
// <i> Default: "/dev/spidev0.0"
// <i> SPI device file
#define NCP_SPI_DEVICE              "/dev/spidev0.0"

// <s NCP_CHIP_SELECT_GPIO> SPI chip select pin
// <i> Default: "8"
// <i> SPI chip select
#define NCP_CHIP_SELECT_GPIO          "8"

// <s NCP_HOST_INT_GPIO> SPI host interrupt pin
// <i> Default: "22"
// <i> SPI host interrupt pin
#define NCP_HOST_INT_GPIO             "22"

// <s NCP_RESET_GPIO> SPI reset pin
// <i> Default: "23"
// <i> SPI reset pin
#define NCP_RESET_GPIO                "23"

// <s NCP_WAKE_GPIO> SPI wakeup pin
// <i> Default: "24"
// <i> SPI wakeup pin
#define NCP_WAKE_GPIO                 "24"

// </h>

// <<< end of configuration section >>>
