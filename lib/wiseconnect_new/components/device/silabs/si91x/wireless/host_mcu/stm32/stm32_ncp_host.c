/*******************************************************************************
* @file  stm32_ncp_host.c
* @brief 
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

#include <stdbool.h>
#include <string.h>
#include "sl_wifi_constants.h"
#include "sl_si91x_host_interface.h"
#include "sl_board_configuration.h"
#include "cmsis_os2.h"
#include "sl_si91x_status.h"
#include "sl_rsi_utility.h"
#include "sl_constants.h"
#include "sl_status.h"
#include "stm32f4xx_hal.h"
#include "sl_board_configuration.h"
#include "sli_wifi_utility.h"

#define SLI_SPI_BUFFER_LENGTH 2300
#define SLI_DMA_ENABLED

static void sli_mx_gpio_init(void);
static void sli_mx_dma_init(void);
static void sli_mx_spi1_init(void);
extern void Error_Handler(void);
void gpio_interrupt(void);

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;
extern volatile int8_t init_error;
volatile uint8_t sli_dma_tx_rx_completed = 0;

uint8_t sli_spi_buffer[SLI_SPI_BUFFER_LENGTH];

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void sli_mx_spi1_init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance               = SPI1;
  hspi1.Init.Mode              = SPI_MODE_MASTER;
  hspi1.Init.Direction         = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize          = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity       = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase          = SPI_PHASE_1EDGE;
  hspi1.Init.NSS               = SPI_NSS_HARD_OUTPUT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode            = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial     = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */
}

/**
  * Enable DMA controller clock
  */
static void sli_mx_dma_init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void sli_mx_gpio_init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = { 0 };

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC7 */
  /* RESET Pin */
  GPIO_InitStruct.Pin   = GPIO_PIN_7;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA9 */
  /* Interrupts from SiWx91x */
  GPIO_InitStruct.Pin  = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB5 */
  /* sleep/wake indications from SiWx91x */
  GPIO_InitStruct.Pin  = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA10 */
  /* sleep/wake request from host */
  GPIO_InitStruct.Pin   = GPIO_PIN_10;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
}

void sl_si91x_host_hold_in_reset(void)
{
  HAL_GPIO_WritePin(RESET_PIN_GPIO_Port, RESET_PIN_Pin, GPIO_PIN_RESET);
}

void sl_si91x_host_release_from_reset(void)
{
  HAL_GPIO_WritePin(RESET_PIN_GPIO_Port, RESET_PIN_Pin, GPIO_PIN_SET);
}

sl_status_t sl_si91x_host_init(const sl_si91x_host_init_configuration_t *config)
{
  UNUSED_PARAMETER(config);
  uint32_t status = 0;

  //! Initialize the host platform GPIOs
  sli_mx_gpio_init();

  //! Initialize DMA
  sli_mx_dma_init();

  //! Initialize SPI
  sli_mx_spi1_init();
  if (init_error != 0) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t sl_si91x_host_deinit(void)
{
  return SL_STATUS_OK;
}

__WEAK void sl_si91x_host_spi_cs_assert()
{
}

__WEAK void sl_si91x_host_spi_cs_deassert()
{
}

/*==================================================================*/
/**
 * @fn         sl_status_t sl_si91x_host_spi_transfer(const void *tx_buffer, void *rx_buffer, uint16_t buffer_length)
 * @param[in]  uint8_t *tx_buff, pointer to the buffer with the data to be transferred
 * @param[in]  uint8_t *rx_buff, pointer to the buffer to store the data received
 * @param[in]  uint16_t transfer_length, Number of bytes to send and receive
 * @param[in]  uint8_t mode, To indicate mode 8 BIT/32 BIT mode transfers.
 * @param[out] None
 * @return     0, 0=success
 * @section description
 * This API is used to transfer/receive data to the Wi-Fi module through the SPI interface.
 */
sl_status_t sl_si91x_host_spi_transfer(const void *tx_buffer, void *rx_buffer, uint16_t buffer_length)
{
  if (rx_buffer == NULL) {
    rx_buffer = sli_spi_buffer;
  }
  if (tx_buffer == NULL) {
    tx_buffer = sli_spi_buffer;
  }

#ifdef SLI_DMA_ENABLED
  HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t *)tx_buffer, (uint8_t *)rx_buffer, buffer_length);
  while (!sli_dma_tx_rx_completed) {
  }
  sli_dma_tx_rx_completed = 0;
#else
  HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)tx_buffer, (uint8_t *)rx_buffer, buffer_length, 10);
#endif
  return SL_STATUS_OK;
}

void sl_si91x_host_enable_high_speed_bus()
{
  /* SPI1 parameter configuration*/
  hspi1.Instance               = SPI1;
  hspi1.Init.Mode              = SPI_MODE_MASTER;
  hspi1.Init.Direction         = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize          = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity       = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase          = SPI_PHASE_1EDGE;
  hspi1.Init.NSS               = SPI_NSS_HARD_OUTPUT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode            = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial     = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK) {
    Error_Handler();
  }
}

void sl_si91x_host_enable_bus_interrupt(void)
{
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void sl_si91x_host_disable_bus_interrupt(void)
{
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void sl_si91x_host_set_sleep_indicator(void)
{
  HAL_GPIO_WritePin(SLEEP_CONFIRM_PIN_GPIO_Port, SLEEP_CONFIRM_PIN_Pin, GPIO_PIN_SET);
}

void sl_si91x_host_clear_sleep_indicator(void)
{
  HAL_GPIO_WritePin(SLEEP_CONFIRM_PIN_GPIO_Port, SLEEP_CONFIRM_PIN_Pin, GPIO_PIN_RESET);
}

uint32_t sl_si91x_host_get_wake_indicator(void)
{
  return HAL_GPIO_ReadPin(WAKE_INDICATOR_PIN_GPIO_Port, WAKE_INDICATOR_PIN_Pin);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{

  sli_dma_tx_rx_completed = 1;
}

void gpio_interrupt(void)
{
  // Trigger SiWx91x BUS Event
  sli_wifi_set_event(SL_SI91X_NCP_HOST_BUS_RX_EVENT);
}
bool sl_si91x_host_is_in_irq_context(void)
{
  return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0U;
}