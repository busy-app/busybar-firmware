/***************************************************************************//**
 * @file
 * @brief Driver for the Invensense ICM40627 6-axis motion sensor
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

#include "sl_icm40627.h"
#include "sl_icm40627_config.h"

#include <stdint.h>
#include <stdio.h>

#include "sl_sleeptimer.h"
#include "sl_assert.h"
#include "sl_gpio.h"
#include "sl_clock_manager.h"
#include "em_device.h"

#if defined(_SILICON_LABS_32B_SERIES_2)
#include "em_eusart.h"
#else
#include "sl_hal_eusart.h"
#endif

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
static void sl_icm40627_chip_select_set(bool select);

/* Concatenate preprocessor tokens A and B. */
#define SL_CONCAT(A, B) A ## B

/* Generate the cmu clock symbol based on instance. */
#define ICM40627_SPI_CLK(N) SL_CONCAT(SL_BUS_CLOCK_EUSART, N)
/** @endcond */

/***************************************************************************//**
 *    Initializes the ICM40627 sensor. Enables the power supply and SPI lines,
 *    sets up the host SPI controller, configures the chip control interface,
 *    clock generator and interrupt line.
 ******************************************************************************/
sl_status_t sl_icm40627_init(void)
{
  uint8_t data;

  /* Enable and setup the SPI bus */
  sl_icm40627_spi_init();

  /* Issue reset */
  sl_icm40627_reset();

  /* Read Who am I register, should get 0x4E */
  sl_icm40627_read_register(ICM40627_REG_WHO_AM_I, 1, &data);

  /* If not - return */
  if (data != ICM40627_DEVICE_ID) {
    return SL_STATUS_INITIALIZATION;
  }

  /* Disable I2C interface, use SPI */
  sl_icm40627_read_register(ICM40627_REG_INTF_CONFIG0, 1, &data);
  data |= ICM40627_UI_SIFS_CFG_VALUE_1;
  sl_icm40627_write_register(ICM40627_REG_INTF_CONFIG0, data);

  /* Set clock select to automatic clock source selection -Select PLL when available, else select RC oscillator (default) */
  sl_icm40627_read_register(ICM40627_REG_INTF_CONFIG1, 1, &data);
  data |= ICM40627_CLKSEL_VALUE_1;
  sl_icm40627_write_register(ICM40627_REG_INTF_CONFIG1, data);

  /*Clear the sleep bit*/
  sl_icm40627_enable_sleep_mode(false);

  /* PLL startup time - maybe it is too long but better be on the safe side, no spec in the datasheet */
  sl_sleeptimer_delay_millisecond(30);

  /* INT pin: active low, open drain, IT status read clears. It seems that latched mode does not work, the INT pin cannot be cleared if set */
  sl_icm40627_write_register(ICM40627_REG_INT_CONFIG, ICM40627_BIT_INT1_DRIVE_CIRCUIT_OPEN_DRAIN | ICM40627_BIT_INT1_POLARITY_ACTL);

  /*Set INT_ASYNC_RESET to 0 from default setting of 1, for proper INT1 and INT2 pin operation */
  sl_icm40627_read_register(ICM40627_REG_INT_CONFIG1, 1, &data);
  data |= ICM40627_BIT_INT_ASYNC_RESET_0;
  sl_icm40627_write_register(ICM40627_REG_INT_CONFIG1, data);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    De-initializes the ICM40627 sensor.
 ******************************************************************************/
sl_status_t sl_icm40627_deinit(void)
{
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Reads the device ID of the ICM40627
 ******************************************************************************/
sl_status_t sl_icm40627_get_device_id(uint8_t *dev_id)
{
  sl_icm40627_read_register(ICM40627_REG_WHO_AM_I, 1, dev_id);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Reads register from the ICM40627 device
 ******************************************************************************/
sl_status_t sl_icm40627_read_register(uint16_t addr, uint16_t num_bytes, uint8_t *data)
{
  uint8_t regAddr;
  uint8_t bank;

  regAddr = (uint8_t) (addr & 0x7F);
  bank = (uint8_t) (addr >> 7);

  /*Select the User bank*/
  sl_icm40627_select_register_bank(bank);

  /* Enable chip select */
  sl_icm40627_chip_select_set(true);

#if defined(_SILICON_LABS_32B_SERIES_2)
  /* Set R/W bit to 1 - read */
  EUSART_Spi_TxRx(SL_ICM40627_SPI_EUSART_PERIPHERAL, (regAddr | 0x80));

  /* Transmit 0's to provide clock and read the data */
  while ( num_bytes-- ) {
    *data++ = EUSART_Spi_TxRx(SL_ICM40627_SPI_EUSART_PERIPHERAL, 0x00);
  }
#else
  *Set R / W bit to 1 - read
  * / sl_hal_eusart_spi_tx_rx(SL_ICM40627_SPI_EUSART_PERIPHERAL, (regAddr | 0x80));

  /* Transmit 0's to provide clock and read the data */
  while ( num_bytes-- ) {
    *data++ = sl_hal_eusart_spi_tx_rx(SL_ICM40627_SPI_EUSART_PERIPHERAL, 0x00);
  }
#endif
  /* Disable chip select */
  sl_icm40627_chip_select_set(false);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Writes a register in the ICM40627 device
 ******************************************************************************/
sl_status_t sl_icm40627_write_register(uint16_t addr, uint8_t data)
{
  uint8_t regAddr;
  uint8_t bank;

  regAddr = (uint8_t) (addr & 0x7F);
  bank = (uint8_t) (addr >> 7);

  /*Select the User bank*/
  sl_icm40627_select_register_bank(bank);

  /* Enable chip select */
  sl_icm40627_chip_select_set(true);

#if defined(_SILICON_LABS_32B_SERIES_2)
  /* clear R/W bit - write, send the address */
  EUSART_Spi_TxRx(SL_ICM40627_SPI_EUSART_PERIPHERAL, (regAddr & 0x7F));

  /* Send the data */
  EUSART_Spi_TxRx(SL_ICM40627_SPI_EUSART_PERIPHERAL, data);
#else
  /* clear R/W bit - write, send the address */
  sl_hal_eusart_spi_tx_rx(SL_ICM40627_SPI_EUSART_PERIPHERAL, (regAddr & 0x7F));

  /* Send the data */
  sl_hal_eusart_spi_tx_rx(SL_ICM40627_SPI_EUSART_PERIPHERAL, data);
#endif

  /* Disable chip select */
  sl_icm40627_chip_select_set(false);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Performs the software reset on the ICM40627 chip
 ******************************************************************************/
sl_status_t sl_icm40627_reset(void)
{
  uint32_t intStatus;

  /* Set Soft_reset bit to initiate software reset */
  sl_icm40627_write_register(ICM40627_REG_DEVICE_CONFIG, ICM40627_BIT_SOFT_RESET_CONFIG_ENABLE);

  /* Wait 100ms to complete the reset sequence */
  sl_sleeptimer_delay_millisecond(100);

  /* Check the Int Reset Done bit */
  sl_icm40627_read_interrupt_status(&intStatus);

  if ((intStatus & ICM40627_MASK_RESET_DONE_INT) == 0 ) {
    return SL_STATUS_FAIL;
  }
  /* Set endianess to Big Endian format for further processing */
  sl_icm40627_write_register(ICM40627_REG_INTF_CONFIG0, 0x30);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Initializes the SPI bus in order to communicate with the ICM40627
 ******************************************************************************/
sl_status_t sl_icm40627_spi_init(void)
{
  EUSART_TypeDef *eusart = SL_ICM40627_SPI_EUSART_PERIPHERAL;

#if defined(_SILICON_LABS_32B_SERIES_2)
  EUSART_SpiInit_TypeDef init = EUSART_SPI_MASTER_INIT_DEFAULT_HF;
  EUSART_SpiAdvancedInit_TypeDef advancedInit = EUSART_SPI_ADVANCED_INIT_DEFAULT;

  init.bitRate = 3300000UL;
  init.advancedSettings = &advancedInit;

  advancedInit.autoCsEnable = false;
  advancedInit.msbFirst = true;
#else
  sl_hal_eusart_spi_config_t init = SL_HAL_EUSART_SPI_MASTER_INIT_DEFAULT_HF;
  sl_hal_eusart_spi_advanced_config_t advancedInit = SL_HAL_EUSART_SPI_ADVANCED_INIT_DEFAULT;

  init.advanced_config = &advancedInit;

  advancedInit.auto_cs_enable = false;
  advancedInit.msb_first = true;
#endif
  /* Enabling clock to EUSART */
  sl_clock_manager_enable_bus_clock(ICM40627_SPI_CLK(SL_ICM40627_SPI_EUSART_PERIPHERAL_NO));
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);

  /* IO configuration */
  sl_gpio_set_pin_mode(&(sl_gpio_t) {SL_ICM40627_SPI_EUSART_TX_PORT, SL_ICM40627_SPI_EUSART_TX_PIN }, SL_GPIO_MODE_PUSH_PULL, 0);  /* TX - MOSI */
  sl_gpio_set_pin_mode(&(sl_gpio_t) {SL_ICM40627_SPI_EUSART_RX_PORT, SL_ICM40627_SPI_EUSART_RX_PIN }, SL_GPIO_MODE_PUSH_PULL, 0);  /* RX - MISO */
  sl_gpio_set_pin_mode(&(sl_gpio_t) {SL_ICM40627_SPI_EUSART_SCLK_PORT, SL_ICM40627_SPI_EUSART_SCLK_PIN }, SL_GPIO_MODE_PUSH_PULL, 0);  /* Clock */
  sl_gpio_set_pin_mode(&(sl_gpio_t) {SL_ICM40627_SPI_EUSART_CS_PORT, SL_ICM40627_SPI_EUSART_CS_PIN }, SL_GPIO_MODE_PUSH_PULL, 1);  /* CS */

  /* Initialize EUSART, in SPI master mode. */
#if defined(_SILICON_LABS_32B_SERIES_2)
  EUSART_SpiInit(eusart, &init);
#else
  sl_hal_eusart_init_spi(eusart, &init);
  sl_hal_eusart_enable(eusart);
  sl_hal_eusart_enable_tx(eusart);
  sl_hal_eusart_enable_rx(eusart);
#endif

  /* Enable pins at correct EUSART location. */
  GPIO->EUSARTROUTE[SL_ICM40627_SPI_EUSART_PERIPHERAL_NO].TXROUTE = ((SL_ICM40627_SPI_EUSART_TX_PORT << _GPIO_EUSART_TXROUTE_PORT_SHIFT) | (SL_ICM40627_SPI_EUSART_TX_PIN << _GPIO_EUSART_TXROUTE_PIN_SHIFT));
  GPIO->EUSARTROUTE[SL_ICM40627_SPI_EUSART_PERIPHERAL_NO].RXROUTE = ((SL_ICM40627_SPI_EUSART_RX_PORT << _GPIO_EUSART_RXROUTE_PORT_SHIFT) | (SL_ICM40627_SPI_EUSART_RX_PIN << _GPIO_EUSART_RXROUTE_PIN_SHIFT));
  GPIO->EUSARTROUTE[SL_ICM40627_SPI_EUSART_PERIPHERAL_NO].SCLKROUTE = ((SL_ICM40627_SPI_EUSART_SCLK_PORT << _GPIO_EUSART_SCLKROUTE_PORT_SHIFT) | (SL_ICM40627_SPI_EUSART_SCLK_PIN << _GPIO_EUSART_SCLKROUTE_PIN_SHIFT));
  GPIO->EUSARTROUTE[SL_ICM40627_SPI_EUSART_PERIPHERAL_NO].ROUTEEN = GPIO_EUSART_ROUTEEN_RXPEN | GPIO_EUSART_ROUTEEN_TXPEN | GPIO_EUSART_ROUTEEN_SCLKPEN;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Enables or disables the sleep mode of the device
 ******************************************************************************/
sl_status_t sl_icm40627_enable_sleep_mode(bool enable)
{
  uint8_t reg;

  sl_icm40627_read_register(ICM40627_REG_INT_STATUS3, 1, &reg);

  if ( enable ) {
    /* Sleep: Set the SLEEP interrupt bit and clear the WAKE interrupt bit */
    reg |= ICM40627_BIT_MASK_SLEEP_INT;
    reg &= ~(ICM40627_BIT_MASK_WAKE_INT);
  } else {
    /* Wake Up: Clear the SLEEP interrupt bit and set the wake interrupt bit */
    reg &= ~(ICM40627_BIT_MASK_SLEEP_INT);
    reg |= ICM40627_BIT_MASK_WAKE_INT;
  }

  sl_icm40627_write_register(ICM40627_REG_INT_STATUS3, reg);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Sets the sample rate for both the accelerometer and the gyroscope.
 ******************************************************************************/
float sl_icm40627_set_sample_rate(float sample_rate)
{
  uint8_t accel_sample_rate_setting;
  uint8_t gyro_sample_rate_setting;

  /* Map sample_rate to corresponding register values */
  if (sample_rate > 6000.0f) {
    accel_sample_rate_setting = ICM40627_ACCEL_ODR_VALUE_8000;
    gyro_sample_rate_setting = ICM40627_GYRO_ODR_VALUE_8000;
    sample_rate = 8000.0f;
  } else if (sample_rate > 3000.0f) {
    accel_sample_rate_setting = ICM40627_ACCEL_ODR_VALUE_4000;
    gyro_sample_rate_setting = ICM40627_GYRO_ODR_VALUE_4000;
    sample_rate = 4000.0f;
  } else if (sample_rate > 1500.0f) {
    accel_sample_rate_setting = ICM40627_ACCEL_ODR_VALUE_2000;
    gyro_sample_rate_setting = ICM40627_GYRO_ODR_VALUE_2000;
    sample_rate = 2000.0f;
  } else if (sample_rate > 750.0f) {
    accel_sample_rate_setting = ICM40627_ACCEL_ODR_VALUE_1000;
    gyro_sample_rate_setting = ICM40627_GYRO_ODR_VALUE_1000;
    sample_rate = 1000.0f;
  } else if (sample_rate > 375.0f) {
    accel_sample_rate_setting = ICM40627_ACCEL_ODR_VALUE_500;
    gyro_sample_rate_setting = ICM40627_GYRO_ODR_VALUE_500;
    sample_rate = 500.0f;
  } else if (sample_rate > 100.0f) {
    accel_sample_rate_setting = ICM40627_ACCEL_ODR_VALUE_200;
    gyro_sample_rate_setting = ICM40627_GYRO_ODR_VALUE_200;
    sample_rate = 200.0f;
  } else if (sample_rate > 75.0f) {
    accel_sample_rate_setting = ICM40627_ACCEL_ODR_VALUE_100;
    gyro_sample_rate_setting = ICM40627_GYRO_ODR_VALUE_100;
    sample_rate = 100.0f;
  } else if (sample_rate > 37.0f) {
    accel_sample_rate_setting = ICM40627_ACCEL_ODR_VALUE_50;
    gyro_sample_rate_setting = ICM40627_GYRO_ODR_VALUE_50;
    sample_rate = 50.0f;
  } else if (sample_rate > 18.0f) {
    accel_sample_rate_setting = ICM40627_ACCEL_ODR_VALUE_25;
    gyro_sample_rate_setting = ICM40627_GYRO_ODR_VALUE_25;
    sample_rate = 25.0f;
  } else if (sample_rate > 9.0f) {
    accel_sample_rate_setting = ICM40627_ACCEL_ODR_VALUE_12_5;
    gyro_sample_rate_setting = ICM40627_GYRO_ODR_VALUE_12_5;
    sample_rate = 12.5f;
  } else if (sample_rate > 6.0f) {
    accel_sample_rate_setting = ICM40627_ACCEL_ODR_VALUE_6_25;
    gyro_sample_rate_setting = ICM40627_GYRO_ODR_VALUE_12_5;
    sample_rate = 6.25f;
  } else {
    accel_sample_rate_setting = ICM40627_ACCEL_ODR_VALUE_3_125;
    gyro_sample_rate_setting = ICM40627_GYRO_ODR_VALUE_12_5;
    sample_rate = 3.125f;
  }

  /* Set the sample rate for gyroscope and accelerometer */
  sl_icm40627_gyro_set_sample_rate((sl_gyro_ODR_t)gyro_sample_rate_setting);
  sl_icm40627_accel_set_sample_rate((sl_accel_ODR_t)accel_sample_rate_setting);

  /* Return the actual sample rate set */
  return sample_rate;
}

/***************************************************************************//**
 *    Sets the Output Data Rate of the gyroscope
 ******************************************************************************/
sl_status_t sl_icm40627_gyro_set_sample_rate(sl_gyro_ODR_t gyro_ODR)
{
  uint8_t reg;

  /* Read the ICM40627_REG_GYRO_CONFIG0 register */
  sl_icm40627_read_register(ICM40627_REG_GYRO_CONFIG0, 1, &reg);
  reg &= ~(ICM40627_MASK_GYRO_ODR_VALUE);

  /* Write the new bandwidth value to the gyro config register */
  reg |= (gyro_ODR & ICM40627_MASK_GYRO_ODR_VALUE);
  sl_icm40627_write_register(ICM40627_REG_GYRO_CONFIG0, reg);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Sets the Output Data Rate of the accelerometer
 ******************************************************************************/
sl_status_t sl_icm40627_accel_set_sample_rate(sl_accel_ODR_t accel_ODR)
{
  uint8_t reg;

  /* Read the ICM40627_REG_ACCEL_CONFIG0 register */
  sl_icm40627_read_register(ICM40627_REG_ACCEL_CONFIG0, 1, &reg);
  reg &= ~(ICM40627_MASK_ACCEL_ODR_VALUE);

  /* Write the new bandwidth value to the accel config register */
  reg |= (accel_ODR & ICM40627_MASK_ACCEL_ODR_VALUE);
  sl_icm40627_write_register(ICM40627_REG_ACCEL_CONFIG0, reg);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Sets the Bandwidth of the gyroscope by choosing a UI Filter Bandwidth
 ******************************************************************************/
sl_status_t sl_icm40627_gyro_set_bandwidth(sl_gyro_BW_t gyro_Bw)
{
  uint8_t reg;

  /* Read the ICM40627_REG_GYRO_ACCEL_CONFIG0 register */
  sl_icm40627_read_register(ICM40627_REG_GYRO_ACCEL_CONFIG0, 1, &reg);
  reg &= ~(ICM40627_MASK_GYRO_UI_FILT_BW);

  /* Write the new bandwidth value to the gyro accel config register */
  reg |= (gyro_Bw & ICM40627_MASK_GYRO_UI_FILT_BW);
  sl_icm40627_write_register(ICM40627_REG_GYRO_ACCEL_CONFIG0, reg);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Sets the Bandwidth of the accelerometer by choosing a UI Filter Bandwidth
 ******************************************************************************/
sl_status_t sl_icm40627_accel_set_bandwidth(sl_accel_BW_t accel_Bw)
{
  uint8_t reg;

  /* Read the ICM40627_REG_GYRO_ACCEL_CONFIG0 register */
  sl_icm40627_read_register(ICM40627_REG_GYRO_ACCEL_CONFIG0, 1, &reg);
  reg &= ~(ICM40627_MASK_ACCEL_UI_FILT_BW);

  /* Write the new bandwidth value to the gyro accel config register */
  reg |= (accel_Bw & ICM40627_MASK_ACCEL_UI_FILT_BW);
  sl_icm40627_write_register(ICM40627_REG_GYRO_ACCEL_CONFIG0, reg);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Sets the full scale value of the accelerometer
 ******************************************************************************/
sl_status_t sl_icm40627_accel_set_full_scale(sl_Accel_FS_t accelFs)
{
  uint8_t reg;
  accelFs &= ICM40627_MASK_ACCEL_FS_SEL;

  /* Read the ICM40627_REG_ACCEL_CONFIG0 register */
  sl_icm40627_read_register(ICM40627_REG_ACCEL_CONFIG0, 1, &reg);
  reg &= ~(ICM40627_MASK_ACCEL_FS_SEL);

  /* Write the new full-scale value to the accel config register */
  reg |= accelFs;
  sl_icm40627_write_register(ICM40627_REG_ACCEL_CONFIG0, reg);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Sets the full scale value of the gyroscope
 ******************************************************************************/
sl_status_t sl_icm40627_gyro_set_full_scale(sl_gyro_FS_t gyroFs)
{
  uint8_t reg;

  gyroFs &= ICM40627_MASK_GYRO_FS_SEL;

  /* Read the ICM40627_REG_GYRO_CONFIG0 register */
  sl_icm40627_read_register(ICM40627_REG_GYRO_CONFIG0, 1, &reg);
  reg &= ~(ICM40627_MASK_GYRO_FS_SEL);

  /* Write the new full-scale value to the gyro config register */
  reg |= gyroFs;
  sl_icm40627_write_register(ICM40627_REG_GYRO_CONFIG0, reg);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Reads the raw acceleration value and converts to g value based on
 *    the actual resolution
 ******************************************************************************/
sl_status_t sl_icm40627_accel_read_data(float accel[3])
{
  uint8_t raw_data[6];
  int16_t temp;
  float accel_res;

  /* Retrieve the current resolution */
  sl_icm40627_accel_get_resolution(&accel_res);

  /* Read the six raw data registers into data array */
  sl_icm40627_read_register(ICM40627_REG_ACCEL_DATA_X1, 6, &raw_data[0]);

  /* Convert the MSB and LSB into a signed 16-bit value and multiply by the resolution to get the g value */
  temp = ( (int16_t) raw_data[0] << 8) | raw_data[1];
  accel[0] = (float) temp * accel_res;

  temp = ( (int16_t) raw_data[2] << 8) | raw_data[3];
  accel[1] = (float) temp * accel_res;

  temp = ( (int16_t) raw_data[4] << 8) | raw_data[5];
  accel[2] = (float) temp * accel_res;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Reads the raw gyroscope value and converts to deg/sec value based on
 *    the actual resolution
 ******************************************************************************/
sl_status_t sl_icm40627_gyro_read_data(float gyro[3])
{
  uint8_t raw_data[6];
  float gyro_res;
  int16_t temp;

  /* Retrieve the current resolution */
  sl_icm40627_gyro_get_resolution(&gyro_res);

  /* Read the six raw data registers into data array */
  sl_icm40627_read_register(ICM40627_REG_GYRO_DATA_X1, 6, &raw_data[0]);

  /* Convert the MSB and LSB into a signed 16-bit value and multiply by the resolution to get the dps value */
  temp = ( (int16_t) raw_data[0] << 8) | raw_data[1];
  gyro[0] = (float) temp * gyro_res;

  temp = ( (int16_t) raw_data[2] << 8) | raw_data[3];
  gyro[1] = (float) temp * gyro_res;

  temp = ( (int16_t) raw_data[4] << 8) | raw_data[5];
  gyro[2] = (float) temp * gyro_res;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Gets the actual resolution of the accelerometer
 ******************************************************************************/
sl_status_t sl_icm40627_accel_get_resolution(float *accel_res)
{
  uint8_t reg;

  /* Read the actual acceleration full scale setting */
  sl_icm40627_read_register(ICM40627_REG_ACCEL_CONFIG0, 1, &reg);
  reg &= ICM40627_MASK_ACCEL_FS_SEL;

  /* Calculate the resolution */
  switch ( reg ) {
    case ICM40627_ACCEL_FS_SEL_VALUE_2G:
      *accel_res = 2.0 / 32768.0;
      break;

    case ICM40627_ACCEL_FS_SEL_VALUE_4G:
      *accel_res = 4.0 / 32768.0;
      break;

    case ICM40627_ACCEL_FS_SEL_VALUE_8G:
      *accel_res = 8.0 / 32768.0;
      break;

    case ICM40627_ACCEL_FS_SEL_VALUE_16G:
      *accel_res = 16.0 / 32768.0;
      break;

    default:
      EFM_ASSERT(false);
      return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Gets the actual resolution of the gyroscope
 ******************************************************************************/
sl_status_t sl_icm40627_gyro_get_resolution(float *gyro_res)
{
  uint8_t reg;

  /* Read the actual gyroscope full scale setting */
  sl_icm40627_read_register(ICM40627_REG_GYRO_CONFIG0, 1, &reg);
  reg &= ICM40627_MASK_GYRO_FS_SEL;

  /* Calculate the resolution */
  switch ( reg ) {
    case ICM40627_GYRO_FS_SEL_VALUE_250:
      *gyro_res = 250.0 / 32768.0;
      break;

    case ICM40627_GYRO_FS_SEL_VALUE_500:
      *gyro_res = 500.0 / 32768.0;
      break;

    case ICM40627_GYRO_FS_SEL_VALUE_1000:
      *gyro_res = 1000.0 / 32768.0;
      break;

    case ICM40627_GYRO_FS_SEL_VALUE_2000:
      *gyro_res = 2000.0 / 32768.0;
      break;

    case ICM40627_GYRO_FS_SEL_VALUE_125:
      *gyro_res = 125.0 / 32768.0;
      break;

    case ICM40627_GYRO_FS_SEL_VALUE_62_5:
      *gyro_res = 62.5 / 32768.0;
      break;

    case ICM40627_GYRO_FS_SEL_VALUE_31_25:
      *gyro_res = 31.25 / 32768.0;
      break;

    case ICM40627_GYRO_FS_SEL_VALUE_15_625:
      *gyro_res = 15.625 / 32768.0;
      break;

    default:
      EFM_ASSERT(false);
      return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Enables or disables the sensors in the ICM40627 chip

 ******************************************************************************/
sl_status_t sl_icm40627_enable_sensor(bool accel, bool gyro, bool temp)
{
  uint8_t pwrManagement = 0x00;

  sl_icm40627_read_register(ICM40627_REG_PWR_MGMT0, 1, &pwrManagement);

  /* To enable the accelerometer */
  if (accel) {
    /*Set the accelerometer to the low-noise mode*/
    pwrManagement |= ICM40627_PWR_MGMT0_ACCEL_MODE_LN;
  } else {
    pwrManagement &= ICM40627_PWR_MGMT0_ACCEL_MODE_DIS;
  }

  /* To enable gyro */
  if (gyro) {
    /*Set the gyro to the low-noise mode*/
    pwrManagement |= ICM40627_PWR_MGMT0_GYRO_MODE_LN;

    /*Set value to 0 when turning off gyroscope.*/
    sl_icm40627_write_register(ICM40627_REG_GYRO_ON_OFF_CONFIG, 0x40);
  } else {
    pwrManagement &= ICM40627_PWR_MGMT0_GYRO_MODE_DIS;

    /*Set value to 1 when turning on gyroscope.*/
    sl_icm40627_write_register(ICM40627_REG_GYRO_ON_OFF_CONFIG, 0x00);
  }

  /* To enable the temperature sensor */
  if (temp) {
    pwrManagement &= ICM40627_PWR_MGMT0_TEMP_MODE_EN;
  } else {
    pwrManagement |= ICM40627_PWR_MGMT0_TEMP_MODE_DIS;
  }

  /* Write back the modified values */
  sl_icm40627_write_register(ICM40627_REG_PWR_MGMT0, pwrManagement);

  /* Wait 200ms to complete the write */
  sl_sleeptimer_delay_millisecond(200);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Enables or disables the interrupts in the ICM40627 chip
 ******************************************************************************/
sl_status_t sl_icm40627_enable_interrupt(bool dataReadyEnable, bool womEnable)
{
  uint8_t intEnable;
  uint8_t data;

  /* All interrupts are disabled by default */
  intEnable = 0;
  data = 0;

  sl_icm40627_read_register(ICM40627_REG_INT_SOURCE1, 1, &data);

  /* Enable one or both of the interrupt sources if required */
  if ( womEnable ) {
    intEnable = (data | ICM40627_BIT_MASK_WOM_X_INT1_EN | ICM40627_BIT_MASK_WOM_Y_INT1_EN | ICM40627_BIT_MASK_WOM_Z_INT1_EN);
  }
  /* Write value to register */
  sl_icm40627_write_register(ICM40627_REG_INT_SOURCE1, intEnable);

  /* All interrupts are disabled by default */
  intEnable = 0;
  data = 0;

  sl_icm40627_read_register(ICM40627_REG_INT_SOURCE0, 1, &data);

  if ( dataReadyEnable ) {
    intEnable = data | ICM40627_MASK_UI_DRDY_INT1_EN;
  }
  /* Write value to register */
  sl_icm40627_write_register(ICM40627_REG_INT_SOURCE0, intEnable);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Reads the interrupt status registers of the ICM40627 chip
 ******************************************************************************/
sl_status_t sl_icm40627_read_interrupt_status(uint32_t *intStatus)
{
  uint8_t reg_status;
  uint8_t reg_status2;
  uint8_t reg_status3;

  sl_icm40627_read_register(ICM40627_REG_INT_STATUS, 1, &reg_status);

  sl_icm40627_read_register(ICM40627_REG_INT_STATUS2, 1, &reg_status2);

  sl_icm40627_read_register(ICM40627_REG_INT_STATUS3, 1, &reg_status3);

  *intStatus = (uint32_t) reg_status;
  *intStatus |= ( ( (uint32_t) reg_status2) << 8);
  *intStatus |= ( ( (uint32_t) reg_status3) << 16);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Checks if new data is available for read
 ******************************************************************************/
bool sl_icm40627_is_data_ready(void)
{
  uint8_t status;
  bool ret;

  ret = false;
  sl_icm40627_read_register(ICM40627_REG_INT_STATUS, 1, &status);

  if ( status & ICM40627_MASK_DATA_RDY_INT ) {
    ret = true;
  }

  return ret;
}

/***************************************************************************//**
 *    Accelerometer and gyroscope calibration function. Reads the gyroscope
 *    and accelerometer values while the device is at rest and in level. The
 *    resulting values are loaded to the accel and gyro bias registers to cancel
 *    the static offset error.
 ******************************************************************************/
sl_status_t sl_icm40627_calibrate_accel_and_gyro(float *accel_bias_scaled, float *gyro_bias_scaled)
{
  uint8_t data[16];
  uint16_t i, packet_count, fifo_count;
  int32_t gyro_bias[3] = { 0, 0, 0 };
  int32_t accel_bias[3] = { 0, 0, 0 };
  int32_t accel_temp[3];
  int32_t gyro_temp[3];
  int32_t accel_bias_factory[3];
  int32_t gyro_bias_stored[3];
  float gyro_res, accel_res;

  /*Disable all the sensors*/
  sl_icm40627_enable_sensor(false, false, false);

  /* Set the most sensitive range: 2G full scale and 250dps full scale */
  sl_icm40627_accel_set_full_scale(sl_accelFS_2g);
  sl_icm40627_gyro_set_full_scale(sl_gyroFS_250dps);

  /*Set the output data rate as 1kHz*/
  sl_icm40627_accel_set_sample_rate(sl_accelODR_1000Hz);
  sl_icm40627_gyro_set_sample_rate(sl_gyroODR_1000Hz);

  /*Configure the bandwidths to 2kHz*/
  sl_icm40627_accel_set_bandwidth(sl_accelBW_ODR_MUL_8);
  sl_icm40627_gyro_set_bandwidth(sl_gyroBW_ODR_MUL_8);

  /* Retrieve the resolution per bit */
  sl_icm40627_accel_get_resolution(&accel_res);
  sl_icm40627_gyro_get_resolution(&gyro_res);

  /* Disable the FIFO-Bypass mode */
  sl_icm40627_write_register(ICM40627_REG_FIFO_CONFIG, ICM40627_FIFO_MODE_BYPASS_MODE);

  /* Enable the accelerometer, gyro and temp sensors */
  sl_icm40627_enable_sensor(true, true, true);

  /* The accel sensor needs max 30ms, the gyro max 35ms to fully start */
  /* Experiments show that the gyro needs more time to get reliable results */
  sl_sleeptimer_delay_millisecond(50);

  /* Reset the FIFO-Stream to FIFO mode */
  sl_icm40627_write_register(ICM40627_REG_FIFO_CONFIG, ICM40627_FIFO_MODE_STREAM_TO_FIFO);

  // Enable the different sensors to write to the FIFO
  sl_icm40627_write_register(ICM40627_REG_FIFO_CONFIG1, ICM40627_MASK_FIFO_TMST_FSYNC_EN
                             | ICM40627_MASK_FIFO_TEMP_EN
                             | ICM40627_MASK_FIFO_GYRO_EN
                             | ICM40627_MASK_FIFO_ACCEL_EN);

  /* The max FIFO size is 2048 bytes, but is limited to 512. */
  fifo_count = 0;
  while ( fifo_count < FIFO_SAMPLE_COUNT ) {
    sl_sleeptimer_delay_millisecond(5);
    /* Read FIFO sample count */
    sl_icm40627_read_register(ICM40627_REG_FIFO_COUNTH, 2, &data[0]);
    /* Convert to a 16 bit value */
    fifo_count = ( (uint16_t) (data[0] << 8) | data[1]);
  }

  /* Disable accelerometer and gyro to store the data in FIFO */
  sl_icm40627_write_register(ICM40627_REG_FIFO_CONFIG1, 0x00);

  /* Read FIFO sample count */
  sl_icm40627_read_register(ICM40627_REG_FIFO_COUNTH, 2, &data[0]);

  /* Convert to a 16 bit value */
  fifo_count = ( (uint16_t) (data[0] << 8) | data[1]);

  /* Calculate the number of data sets */
  /* Each FIFO packet: [Header(1)|Accel(6)|Gyro(6)|Temp(1)|Timestamp(2)] = 16 bytes */
  packet_count = fifo_count / 16;

  /* Retrieve the data from the FIFO */
  for ( i = 0; i < packet_count; i++ ) {
    sl_icm40627_read_register(ICM40627_REG_FIFO_DATA, sizeof(data), &data[0]);
    /* Convert to 16 bit signed accel and gyro x,y and z values */

    accel_temp[0] = (int16_t)((data[1] << 8) | data[2]);
    accel_temp[1] = (int16_t)((data[3] << 8) | data[4]);
    accel_temp[2] = (int16_t)((data[5] << 8) | data[6]);

    gyro_temp[0] = (int16_t)((data[7] << 8) | data[8]);
    gyro_temp[1] = (int16_t)((data[9] << 8) | data[10]);
    gyro_temp[2] = (int16_t)((data[11] << 8) | data[12]);


    /* Sum the values */
    accel_bias[0] += accel_temp[0];
    accel_bias[1] += accel_temp[1];
    accel_bias[2] += accel_temp[2];
    gyro_bias[0] += gyro_temp[0];
    gyro_bias[1] += gyro_temp[1];
    gyro_bias[2] += gyro_temp[2];
  }

  /* Divide by packet count to get the average */
  accel_bias[0] /= packet_count;
  accel_bias[1] /= packet_count;
  accel_bias[2] /= packet_count;
  gyro_bias[0] /= packet_count;
  gyro_bias[1] /= packet_count;
  gyro_bias[2] /= packet_count;

  /* Accelerometer: add or remove (depending on the orientation of the chip) 1G (gravity) from the Z axis value */
  if ( accel_bias[2] > 0L ) {
    accel_bias[2] -= (int32_t) (1.0f / accel_res);
  } else {
    accel_bias[2] += (int32_t) (1.0f / accel_res);
  }

  /* Convert the values to degrees per sec for displaying */
  gyro_bias_scaled[0] = (float) gyro_bias[0] * gyro_res;
  gyro_bias_scaled[1] = (float) gyro_bias[1] * gyro_res;
  gyro_bias_scaled[2] = (float) gyro_bias[2] * gyro_res;

  /* Read stored gyro trim values. After reset these values are all 0 */

  /* ICM40627_REG_OFFSET_USER0 contains the Lower bits of X-gyro offset[0:7] */
  sl_icm40627_read_register(ICM40627_REG_OFFSET_USER0, 1, &data[0]);
  gyro_bias_stored[0] = (int16_t)data[0];

  /* ICM40627_REG_OFFSET_USER1 contains the Upper bits of Y-gyro offset[11:8] and Upper bits of X-Gyro offset[11:8] */
  sl_icm40627_read_register(ICM40627_REG_OFFSET_USER1, 1, &data[0]);
  uint8_t temp = data[0];
  gyro_bias_stored[0] |= (int16_t)((temp & 0x0F) << 8);
  gyro_bias_stored[1] = (int16_t)((temp & 0xF0) << 4);

  /* ICM40627_REG_OFFSET_USER2 contains the Lower bits of Y-gyro offset[7:0] */
  sl_icm40627_read_register(ICM40627_REG_OFFSET_USER2, 1, &data[0]);
  gyro_bias_stored[1] |= (int16_t)data[0];

  /* ICM40627_REG_OFFSET_USER3 contains the Lower bits of Z-gyro offset[7:0] */
  sl_icm40627_read_register(ICM40627_REG_OFFSET_USER3, 1, &data[0]);
  gyro_bias_stored[2] = (int16_t)data[0];

  /*ICM40627_REG_OFFSET_USER4 contains the Upper bits of X-accel offset[11:8] and Upper bits of Z-gyro offset[11:8] */
  sl_icm40627_read_register(ICM40627_REG_OFFSET_USER4, 1, &data[0]);
  temp = data[0];
  gyro_bias_stored[2] |= (int16_t)((temp & 0x0F) << 8);

  /* The gyro bias should be stored in 1000dps full scaled format. We measured in 250dps to get */
  /* the best sensitivity, so need to divide by 4 */
  /* Subtract from the stored calibration value */
  gyro_bias_stored[0] -= gyro_bias[0] / 4;
  gyro_bias_stored[1] -= gyro_bias[1] / 4;
  gyro_bias_stored[2] -= gyro_bias[2] / 4;

  /* Calculate the accelerometer bias values to store in the hardware accelerometer bias registers. These registers contain */
  /* factory trim values which must be added to the calculated accelerometer biases; on boot up these registers will hold */
  /* non-zero values. In addition, bit 0 of the lower byte must be preserved since it is used for temperature */
  /* compensation calculations(? the datasheet is not clear). Accelerometer bias registers expect bias input */
  /* as 2048 LSB per g, so that the accelerometer biases calculated above must be divided by 8. */

  /* Read factory accelerometer trim values */
  accel_bias_factory[0] = (int16_t)((temp & 0xF0) << 4);

  /* ICM40627_REG_OFFSET_USER5 contains the Lower bits of X-accel offset[7:0] */
  sl_icm40627_read_register(ICM40627_REG_OFFSET_USER5, 1, &data[0]);
  accel_bias_factory[0] |= (int16_t)data[0];

  /* ICM40627_REG_OFFSET_USER6 contains the Lower bits of Y-accel offset[7:0] */
  sl_icm40627_read_register(ICM40627_REG_OFFSET_USER6, 1, &data[0]);
  accel_bias_factory[1] = (int16_t)data[0];

  /* ICM40627_REG_OFFSET_USER7 contains the Upper bits of Z-accel offset[11:8] and Upper bits of Y-accel offset[11:8]*/
  sl_icm40627_read_register(ICM40627_REG_OFFSET_USER7, 1, &data[0]);
  temp = data[0];
  accel_bias_factory[1] |= (int16_t)((temp & 0x0F) << 8);
  accel_bias_factory[2] = (int16_t)((temp & 0xF0) << 4);

  /* ICM40627_REG_OFFSET_USER8 contains the Lower bits of Z-accel offset[7:0] */
  sl_icm40627_read_register(ICM40627_REG_OFFSET_USER8, 1, &data[0]);
  accel_bias_factory[2] |= (int16_t)data[0];

  /* Construct total accelerometer bias, including calculated average accelerometer bias from above */
  /* Scale the 2g full scale (most sensitive range) results to 16g full scale - divide by 8 */
  /* Clear the last bit (temperature compensation? - the datasheet is not clear) */
  /* Subtract from the factory calibration value */

  accel_bias_factory[0] -= ( (accel_bias[0] / 8) & ~1);
  accel_bias_factory[1] -= ( (accel_bias[1] / 8) & ~1);
  accel_bias_factory[2] -= ( (accel_bias[2] / 8) & ~1);

  /* Split the values into two bytes */
  /*Lower bits of X-gyro offset*/
  data[0] = gyro_bias_stored[0] & 0xFF;
  /*Upper bits of Y-gyro offset and Upper bits of X-gyro offset*/
  data[1] = (((gyro_bias_stored[0]) >> 8) & 0x0F) | (((gyro_bias_stored[1] >> 8) & 0x0F) << 4);
  /*Lower bits of Y-gyro offset */
  data[2] = gyro_bias_stored[1] & 0xFF;
  /*Lower bits of Z-gyro offset*/
  data[3] = gyro_bias_stored[2] & 0xFF;
  /*Upper bits of X-accel offset and Upper bits of Z-gyro offset*/
  data[4] = (((gyro_bias_stored[2]) >> 8) & 0x0F) | (((accel_bias_factory[0] >> 8) & 0x0F) << 4);
  /*Lower bits of X-accel offset*/
  data[5] = accel_bias_factory[0] & 0xFF;
  /*Lower bits of Y-accel offset */
  data[6] = accel_bias_factory[1] & 0xFF;
  /*Upper bits of Z-accel and Upper bits of Y-accel offset*/
  data[7] = (((accel_bias_factory[1]) >> 8) & 0x0F) | (((accel_bias_factory[2] >> 8) & 0x0F) << 4);
  /*Lower bits of Z-accel offset*/
  data[8] = accel_bias_factory[2] & 0xFF;

  /* Write the  gyro and accel bias values to the chip */
  sl_icm40627_write_register(ICM40627_REG_OFFSET_USER0, data[0]);
  sl_icm40627_write_register(ICM40627_REG_OFFSET_USER1, data[1]);
  sl_icm40627_write_register(ICM40627_REG_OFFSET_USER2, data[2]);
  sl_icm40627_write_register(ICM40627_REG_OFFSET_USER3, data[3]);
  sl_icm40627_write_register(ICM40627_REG_OFFSET_USER4, data[4]);
  sl_icm40627_write_register(ICM40627_REG_OFFSET_USER5, data[5]);
  sl_icm40627_write_register(ICM40627_REG_OFFSET_USER6, data[6]);
  sl_icm40627_write_register(ICM40627_REG_OFFSET_USER7, data[7]);
  sl_icm40627_write_register(ICM40627_REG_OFFSET_USER8, data[8]);

  /* Convert the values to G for displaying */
  accel_bias_scaled[0] = (float) accel_bias[0] * accel_res;
  accel_bias_scaled[1] = (float) accel_bias[1] * accel_res;
  accel_bias_scaled[2] = (float) accel_bias[2] * accel_res;

  /* Turn off FIFO */
  sl_icm40627_write_register(ICM40627_REG_FIFO_CONFIG, ICM40627_FIFO_MODE_BYPASS_MODE);

  /* Disable all sensors */
  sl_icm40627_enable_sensor(false, false, false);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Gyroscope calibration function. Reads the gyroscope
 *    values while the device is at rest and in level. The
 *    resulting values are loaded to the gyro bias registers to cancel
 *    the static offset error.
 ******************************************************************************/
sl_status_t sl_icm40627_calibrate_gyro(float *gyro_bias_scaled)
{
  uint8_t data[16];
  uint16_t i, packet_count, fifo_count;
  int32_t gyro_bias[3] = { 0, 0, 0 };
  int32_t gyro_temp[3];
  int32_t accel_bias_factory[1];
  int32_t gyro_bias_stored[3];
  float gyro_res;

  /*Disable all the sensors*/
  sl_icm40627_enable_sensor(false, false, false);

  /* Set the most sensitive range: 2G full scale and 250dps full scale */
  sl_icm40627_gyro_set_full_scale(sl_gyroFS_250dps);

  /*Set the output data rate as 1kHz*/
  sl_icm40627_gyro_set_sample_rate(sl_gyroODR_1000Hz);

  /*Configure the bandwidths to 2kHz*/
  sl_icm40627_gyro_set_bandwidth(sl_gyroBW_ODR_DIV_1);

  /* Retrieve the resolution per bit */
  sl_icm40627_gyro_get_resolution(&gyro_res);

  /* Disable the FIFO-Bypass mode */
  sl_icm40627_write_register(ICM40627_REG_FIFO_CONFIG, 0x00);

  /* Enable the accelerometer, gyro and temp sensors */
  sl_icm40627_enable_sensor(true, true, true);

  /* The accel sensor needs max 30ms, the gyro max 35ms to fully start */
  /* Experiments show that the gyro needs more time to get reliable results */
  sl_sleeptimer_delay_millisecond(50);

  /* Reset the FIFO-Stream to FIFO mode */
  sl_icm40627_write_register(ICM40627_REG_FIFO_CONFIG, ICM40627_FIFO_MODE_STREAM_TO_FIFO);

  // Enable the different sensors to write to the FIFO
  sl_icm40627_write_register(ICM40627_REG_FIFO_CONFIG1, ICM40627_MASK_FIFO_TMST_FSYNC_EN
                             | ICM40627_MASK_FIFO_TEMP_EN
                             | ICM40627_MASK_FIFO_GYRO_EN
                             | ICM40627_MASK_FIFO_ACCEL_EN);

  /* The max FIFO size is 2048 bytes, but is limited to 512. */
  fifo_count = 0;
  while ( fifo_count < FIFO_SAMPLE_COUNT ) {
    sl_sleeptimer_delay_millisecond(5);
    /* Read FIFO sample count */
    sl_icm40627_read_register(ICM40627_REG_FIFO_COUNTH, 2, &data[0]);
    /* Convert to a 16 bit value */
    fifo_count = ( (uint16_t) (data[0] << 8) | data[1]);
  }

  /* Disable accelerometer and gyro to store the data in FIFO */
  sl_icm40627_write_register(ICM40627_REG_FIFO_CONFIG1, ICM40627_FIFO_MODE_BYPASS_MODE);

  /* Read FIFO sample count */
  sl_icm40627_read_register(ICM40627_REG_FIFO_COUNTH, 2, &data[0]);

  /* Convert to a 16 bit value */
  fifo_count = ( (uint16_t) (data[0] << 8) | data[1]);

  /* Calculate the number of data sets */
  /* Each FIFO packet: [Header(1)|Accel(6)|Gyro(6)|Temp(1)|Timestamp(2)] = 16 bytes */
  packet_count = fifo_count / 16;

  /* Retrieve the data from the FIFO */
  for ( i = 0; i < packet_count; i++ ) {
    sl_icm40627_read_register(ICM40627_REG_FIFO_DATA, sizeof(data), &data[0]);
    /* Convert to 16 bit signed accel and gyro x,y and z values */
    gyro_temp[0] = (int16_t)((data[7] << 8) | data[8]);
    gyro_temp[1] = (int16_t)((data[9] << 8) | data[10]);
    gyro_temp[2] = (int16_t)((data[11] << 8) | data[12]);

    /* Sum the values */
    gyro_bias[0] += gyro_temp[0];
    gyro_bias[1] += gyro_temp[1];
    gyro_bias[2] += gyro_temp[2];
  }

  /* Divide by packet count to get the average */
  gyro_bias[0] /= packet_count;
  gyro_bias[1] /= packet_count;
  gyro_bias[2] /= packet_count;

  /* Convert the values to degrees per sec for displaying */
  gyro_bias_scaled[0] = (float) gyro_bias[0] * gyro_res;
  gyro_bias_scaled[1] = (float) gyro_bias[1] * gyro_res;
  gyro_bias_scaled[2] = (float) gyro_bias[2] * gyro_res;

  /* Read stored gyro trim values. After reset these values are all 0 */
  sl_icm40627_read_register(ICM40627_REG_OFFSET_USER0, 1, &data[0]);
  gyro_bias_stored[0] = (int16_t)data[0];
  sl_icm40627_read_register(ICM40627_REG_OFFSET_USER1, 1, &data[0]);
  uint8_t temp = data[0];
  gyro_bias_stored[0] |= (int16_t)((temp & 0x0F) << 8);
  gyro_bias_stored[1] = (int16_t)((temp & 0xF0) << 4);
  sl_icm40627_read_register(ICM40627_REG_OFFSET_USER2, 1, &data[0]);
  gyro_bias_stored[1] |= (int16_t)data[0];
  sl_icm40627_read_register(ICM40627_REG_OFFSET_USER3, 1, &data[0]);
  gyro_bias_stored[2] = (int16_t)data[0];
  sl_icm40627_read_register(ICM40627_REG_OFFSET_USER4, 1, &data[0]);
  temp = data[0];
  gyro_bias_stored[2] |= (int16_t)((temp & 0x0F) << 8);
  accel_bias_factory[0] = (int16_t)((temp & 0xF0) << 4);

  /* The gyro bias should be stored in 1000dps full scaled format. We measured in 250dps to get */
  /* the best sensitivity, so need to divide by 4 */
  /* Subtract from the stored calibration value */
  gyro_bias_stored[0] -= gyro_bias[0] / 4;
  gyro_bias_stored[1] -= gyro_bias[1] / 4;
  gyro_bias_stored[2] -= gyro_bias[2] / 4;

  /* Split the values into two bytes */
  /*Lower bits of X-gyro offset*/
  data[0] = gyro_bias_stored[0] & 0xFF;
  /*Upper bits of Y-gyro offset and Upper bits of X-gyro offset*/
  data[1] = (((gyro_bias_stored[0]) >> 8) & 0x0F) | (((gyro_bias_stored[1] >> 8) & 0x0F) << 4);
  /*Lower bits of Y-gyro offset */
  data[2] = gyro_bias_stored[1] & 0xFF;
  /*Lower bits of Z-gyro offset*/
  data[3] = gyro_bias_stored[2] & 0xFF;
  /*Upper bits of X-accel offset and Upper bits of Z-gyro offset*/
  data[4] = (((gyro_bias_stored[2]) >> 8) & 0x0F) | (((accel_bias_factory[0] >> 8) & 0x0F) << 4);

  /* Write the  gyro and accel bias values to the chip */
  sl_icm40627_write_register(ICM40627_REG_OFFSET_USER0, data[0]);
  sl_icm40627_write_register(ICM40627_REG_OFFSET_USER1, data[1]);
  sl_icm40627_write_register(ICM40627_REG_OFFSET_USER2, data[2]);
  sl_icm40627_write_register(ICM40627_REG_OFFSET_USER3, data[3]);
  sl_icm40627_write_register(ICM40627_REG_OFFSET_USER4, data[4]);

  /* Turn off FIFO */
  sl_icm40627_write_register(ICM40627_REG_FIFO_CONFIG, ICM40627_FIFO_MODE_BYPASS_MODE);

  /* Disable all sensors */
  sl_icm40627_enable_sensor(false, false, false);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Reads the temperature sensor raw value and converts to Celsius.
 ******************************************************************************/
sl_status_t sl_icm40627_read_temperature_data(float *temperature)
{
  uint8_t data[2];
  int16_t raw_temp;

  /* Read temperature registers */
  sl_icm40627_read_register(ICM40627_REG_TEMP_DATA1, 2, data);

  /* Convert to int16 */
  raw_temp = (int16_t) ( (data[0] << 8) + data[1]);

  /* Calculate the Centigrade value from the raw reading */
  *temperature = ((float) raw_temp / 132.48f) + 25.0f;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Enable or disable the accelerometer sensor in low power mode in the ICM40627 chip
 ******************************************************************************/
sl_status_t sl_icm40627_accel_enter_low_power_mode(bool enAccel)
{
  uint8_t data = 0;

  sl_icm40627_read_register(ICM40627_REG_PWR_MGMT0, 1, &data);

  /* Make sure that the chip is not in sleep */
  sl_icm40627_enable_sleep_mode(false);

  /* Enable the accelerometer and the gyroscope*/
  sl_icm40627_enable_sensor(true, false, false);

  sl_sleeptimer_delay_millisecond(50);

  /* Clear the ACCEL_MODE bits */
  data &= ~(ICM40627_PWR_MGMT0_ACCEL_MODE_MASK);

  if ( enAccel ) {
    /* Set the ACCEL_MODE bits to enable low power mode */
    data |= ICM40627_PWR_MGMT0_ACCEL_MODE_LP;

    /* Write the updated value to the PWR_MGNT_0 register */
    sl_icm40627_write_register(ICM40627_REG_PWR_MGMT0, data);

    sl_icm40627_read_register(ICM40627_REG_INTF_CONFIG1, 1, &data);

    /* Clear the ACCEL_LP_CLK_SEL bits for the Accelerometer LP mode to use Wake Up oscillator clock */
    data &= ~(ICM40627_BIT_ACCEL_LP_CLK_SEL_MASK);

    /* Write the updated value to the INTF_CONFIG1 register */
    sl_icm40627_write_register(ICM40627_REG_INTF_CONFIG1, data);

    sl_sleeptimer_delay_millisecond(10);
  } else {
    /* Set the ACCEL_MODE bits to enable low noise mode */
    data |= ICM40627_PWR_MGMT0_ACCEL_MODE_LN;

    /* Write the updated value to the PWR_MGNT_0 register */
    sl_icm40627_write_register(ICM40627_REG_PWR_MGMT0, data);

    sl_icm40627_read_register(ICM40627_REG_INTF_CONFIG1, 1, &data);

    /* Set the ACCEL_LP_CLK_SEL bits for the Accelerometer LP mode to use RC oscillator clock*/
    data |= (ICM40627_BIT_ACCEL_LP_CLK_SEL_MASK);

    /* Write the updated value to the INTF_CONFIG1 register */
    sl_icm40627_write_register(ICM40627_REG_INTF_CONFIG1, data);

    sl_sleeptimer_delay_millisecond(10);
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    Select the desired register bank
 ******************************************************************************/
sl_status_t sl_icm40627_select_register_bank(uint8_t bank)
{
  /* Enable chip select */
  sl_icm40627_chip_select_set(true);

#if defined(_SILICON_LABS_32B_SERIES_2)
  /* Select the Bank Select register */
  EUSART_Spi_TxRx(SL_ICM40627_SPI_EUSART_PERIPHERAL, ICM40627_REG_REG_BANK_SEL);

  /* Send the data */
  EUSART_Spi_TxRx(SL_ICM40627_SPI_EUSART_PERIPHERAL, bank);
#else
  /* Select the Bank Select register */
  sl_hal_eusart_spi_tx_rx(SL_ICM40627_SPI_EUSART_PERIPHERAL, ICM40627_REG_REG_BANK_SEL);

  /* Write the desired bank address 0..4 */
  sl_hal_eusart_spi_tx_rx(SL_ICM40627_SPI_EUSART_PERIPHERAL, bank);
#endif

  /* Disable chip select */
  sl_icm40627_chip_select_set(false);

  return SL_STATUS_OK;
}

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/***************************************************************************//**
 * @brief
 *    Controls the state of the ICM40627 SPI chip select pin
 *
 * @param[in] select
 *    The desired state of the select pin. True: asserted (logic L)
 *
 * @return
 *    None
 ******************************************************************************/
static void sl_icm40627_chip_select_set(bool select)
{
  sl_gpio_t spi_eusart_cs_gpio = {
    .port = SL_ICM40627_SPI_EUSART_CS_PORT,
    .pin = SL_ICM40627_SPI_EUSART_CS_PIN,
  };

  if (select) {
    sl_gpio_clear_pin(&spi_eusart_cs_gpio);
  } else {
    sl_gpio_set_pin(&spi_eusart_cs_gpio);
  }
}

/** @endcond */