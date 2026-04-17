/***************************************************************************//**
 * @file
 * @brief SPIDRV API implementation.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#define CURRENT_MODULE_NAME    "SPIDRV"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#include "em_device.h"
#include "sl_clock_manager.h"
#include "sl_core.h"
#include "sl_interrupt_manager.h"
#include "sl_code_classification.h"

#if  defined(_SILICON_LABS_32B_SERIES_2)
#include "em_bus.h"
#if defined (USART_PRESENT)
#include "em_usart.h"
#endif
#if defined(EUSART_PRESENT)
#include "em_eusart.h"
#endif
#elif defined(_SILICON_LABS_32B_SERIES_3)
#include "sl_hal_eusart.h"
#include "sl_hal_bus.h"
#endif

#include "dmadrv.h"
#include "spidrv.h"
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#if defined(EUSART_PRESENT)
#include "sl_slist.h"
#endif
#endif
#include <string.h>

/// @cond DO_NOT_INCLUDE_WITH_DOXYGEN

//****************************************************************************

#if defined(DMA_PRESENT) && (DMA_COUNT == 1)
#define SPI_DMA_IRQ   DMA_IRQn

#elif defined(LDMA_PRESENT) && (LDMA_COUNT == 1)
#if defined(_SILICON_LABS_32B_SERIES_2)
#define SPI_DMA_IRQ   LDMA_IRQn
#else
#define SPI_DMA_IRQ   LDMA0_CHNL0_IRQn
#endif

#else
#error "No valid SPIDRV DMA engine defined."
#endif

#define EMDRV_SPIDRV_USART_FRAMELENGTH_REGVALUE_OFFSET  (3U)
#define EMDRV_SPIDRV_EUSART_FRAMELENGTH_REGVALUE_OFFSET (6U)

//****************************************************************************

/**
 * @brief SPI Pins structure used when mapping from location to gpio port+pin.
 */
typedef struct {
  uint8_t mosiPort;
  uint8_t mosiPin;
  uint8_t misoPort;
  uint8_t misoPin;
  uint8_t clkPort;
  uint8_t clkPin;
  uint8_t csPort;
  uint8_t csPin;
} SPI_Pins_t;

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && defined(EUSART_PRESENT)
static sl_power_manager_em_transition_event_handle_t on_power_manager_event_handle;

static void on_power_manager_event(sl_power_manager_em_t from,
                                   sl_power_manager_em_t to);

static sl_power_manager_em_transition_event_info_t on_power_manager_event_info =
{
  .event_mask = (SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_EM2 | SL_POWER_MANAGER_EVENT_TRANSITION_LEAVING_EM2),
  .on_event = on_power_manager_event,
};

// List of EUSART handles
sl_slist_node_t *eusart_handle_list = NULL;
#endif

static bool     spidrvIsInitialized = false;

#if defined(USART_PRESENT)
static Ecode_t SPIDRV_InitUsart(SPIDRV_Handle_t handle, SPIDRV_Init_t *initData);
#endif

#if defined(EUSART_PRESENT)
static Ecode_t SPIDRV_InitEusart(SPIDRV_Handle_t handle, SPIDRV_Init_t *initData);
#endif

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SPIDRV, SL_CODE_CLASS_TIME_CRITICAL)
static void     BlockingComplete(SPIDRV_Handle_t handle,
                                 Ecode_t transferStatus,
                                 int itemsTransferred);

static Ecode_t  ConfigGPIO(SPIDRV_Handle_t handle, bool enable);

static bool     RxDMAComplete(unsigned int channel,
                              unsigned int sequenceNo,
                              void *userParam);

#if defined(EMDRV_SPIDRV_INCLUDE_SLAVE)
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SPIDRV, SL_CODE_CLASS_TIME_CRITICAL)
static void     SlaveTimeout(sl_sleeptimer_timer_handle_t *handle, void *data);
#endif

static void     StartReceiveDMA(SPIDRV_Handle_t handle,
                                void *buffer,
                                int count,
                                SPIDRV_Callback_t callback);

static void     StartTransferDMA(SPIDRV_Handle_t handle,
                                 const void *txBuffer,
                                 void *rxBuffer,
                                 int count,
                                 SPIDRV_Callback_t callback);

static void     StartTransmitDMA(SPIDRV_Handle_t handle,
                                 const void *buffer,
                                 int count,
                                 SPIDRV_Callback_t callback);

static Ecode_t  TransferApiPrologue(SPIDRV_Handle_t handle,
                                    void *buffer,
                                    int count);

static Ecode_t  TransferApiBlockingPrologue(SPIDRV_Handle_t handle,
                                            void *buffer,
                                            int count);

static void     WaitForTransferCompletion(SPIDRV_Handle_t handle);

#if defined(EMDRV_SPIDRV_INCLUDE_SLAVE)
static Ecode_t  WaitForIdleLine(SPIDRV_Handle_t handle);
#endif

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && defined(EUSART_PRESENT)
static Ecode_t sli_spidrv_exit_em23(SPIDRV_Handle_t handle);
static Ecode_t sli_spidrv_enter_em23(SPIDRV_Handle_t handle);
#endif

/// @endcond

/***************************************************************************//**
 * @brief Power management functions.
 ******************************************************************************/
static void em1RequestAdd(SPIDRV_Handle_t handle)
{
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  if (handle->em1RequestCount == 0) {
    sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
  }
  handle->em1RequestCount++;
  CORE_EXIT_ATOMIC();
#else
  handle->em1RequestCount++;
#endif
}

static void em1RequestRemove(SPIDRV_Handle_t handle)
{
  EFM_ASSERT(handle->em1RequestCount > 0);
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  handle->em1RequestCount--;
  if (handle->em1RequestCount == 0) {
    sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
  }
  CORE_EXIT_ATOMIC();
#else
  handle->em1RequestCount--;
#endif
}

static void emRequestInit(SPIDRV_Handle_t handle)
{
  handle->em1RequestCount = 0;
}

static void emRequestDeinit(SPIDRV_Handle_t handle)
{
  if (handle->em1RequestCount > 0) {
    handle->em1RequestCount = 1;
    em1RequestRemove(handle);
  }
}

/***************************************************************************//**
 * @brief
 *    Initialize an SPI driver instance.
 *
 * @param[out] handle  Pointer to an SPI driver handle; refer to @ref
 *                     SPIDRV_Handle_t.
 *
 * @param[in] initData Pointer to an initialization data structure;
 *                     refer to @ref SPIDRV_Init.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success. On failure, an appropriate
 *    SPIDRV @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_Init(SPIDRV_Handle_t handle, SPIDRV_Init_t *initData)
{
#if defined (EUSART_PRESENT)
  Ecode_t result = ECODE_EMDRV_SPIDRV_PARAM_ERROR;

  if (EUSART_NUM((EUSART_TypeDef*)initData->port) != -1) {
    result = SPIDRV_InitEusart(handle, initData);
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    // Subscribe to notification to re-enable eusart after deepsleep.
    if (eusart_handle_list == NULL) {
      sl_power_manager_subscribe_em_transition_event(&on_power_manager_event_handle, &on_power_manager_event_info);
    }
    sl_slist_push(&eusart_handle_list, &handle->node);
#endif

    return result;
  }
#endif

#if defined (USART_PRESENT)
  return SPIDRV_InitUsart(handle, initData);
#else
  return ECODE_EMDRV_SPIDRV_ILLEGAL_HANDLE;
#endif
}

#if defined (USART_PRESENT)
/***************************************************************************//**
 * @brief
 *    Initialize an SPI driver usart instance.
 *
 * @param[out] handle  Pointer to an SPI driver handle; refer to @ref
 *                     SPIDRV_Handle_t.
 *
 * @param[in] initData Pointer to an initialization data structure;
 *                     refer to @ref SPIDRV_Init.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success. On failure, an appropriate
 *    SPIDRV @ref Ecode_t is returned.
 ******************************************************************************/
static Ecode_t SPIDRV_InitUsart(SPIDRV_Handle_t handle, SPIDRV_Init_t *initData)
{
  Ecode_t retVal;
  USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;
  int8_t spiPortNum = -1;
  sl_bus_clock_t bus_clock;

  if (handle == NULL) {
    return ECODE_EMDRV_SPIDRV_ILLEGAL_HANDLE;
  }

  if (initData == NULL) {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }

  memset(handle, 0, sizeof(SPIDRV_HandleData_t));
  emRequestInit(handle);

  if (0) {
#if defined(USART0)
  } else if ((USART_TypeDef*)initData->port == USART0) {
    handle->usartPeripheral  = SL_PERIPHERAL_USART0;
    handle->txDMASignal = dmadrvPeripheralSignal_USART0_TXBL;
    handle->rxDMASignal = dmadrvPeripheralSignal_USART0_RXDATAV;
    spiPortNum = 0;
#endif
#if defined(USART1)
  } else if ((USART_TypeDef*)initData->port == USART1) {
    handle->usartPeripheral  = SL_PERIPHERAL_USART1;
    handle->txDMASignal = dmadrvPeripheralSignal_USART1_TXBL;
    handle->rxDMASignal = dmadrvPeripheralSignal_USART1_RXDATAV;
    spiPortNum = 1;
#endif
#if defined(USART2)
  } else if ((USART_TypeDef*)initData->port == USART2) {
    handle->usartPeripheral  = SL_PERIPHERAL_USART2;
    handle->txDMASignal = dmadrvPeripheralSignal_USART2_TXBL;
    handle->rxDMASignal = dmadrvPeripheralSignal_USART2_RXDATAV;
    spiPortNum = 2;
#endif
  } else {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }

  handle->peripheral.usartPort  = initData->port;
  handle->peripheralType        = spidrvPeripheralTypeUsart;
  handle->initData              = *initData;

  if (initData->bitOrder == spidrvBitOrderMsbFirst) {
    usartInit.msbf = true;
  }

  if (initData->clockMode == spidrvClockMode0) {
    usartInit.clockMode = usartClockMode0;
  } else if (initData->clockMode == spidrvClockMode1) {
    usartInit.clockMode = usartClockMode1;
  } else if (initData->clockMode == spidrvClockMode2) {
    usartInit.clockMode = usartClockMode2;
  } else if (initData->clockMode == spidrvClockMode3) {
    usartInit.clockMode = usartClockMode3;
  } else {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }

  if (initData->type == spidrvSlave) {
    usartInit.master = false;
    usartInit.baudrate = 115200;      // Dummy value needed by USART_InitSync()
  } else {
    usartInit.baudrate = initData->bitRate;
  }

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
  bus_clock = sl_device_peripheral_get_bus_clock(handle->usartPeripheral);
  sl_clock_manager_enable_bus_clock(bus_clock);

  if ((initData->frameLength < 4U) || (initData->frameLength > 16U)) {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }
  uint32_t databits = initData->frameLength - 4U + _USART_FRAME_DATABITS_FOUR;
  usartInit.databits = (USART_Databits_TypeDef)databits;

  USART_InitSync(initData->port, &usartInit);

  if ((initData->type == spidrvMaster)
      && (initData->csControl == spidrvCsControlAuto)) {
    #if defined(USART_HAS_SET_CLEAR)
    handle->peripheral.usartPort->CTRL_SET = USART_CTRL_AUTOCS;
    #else
    handle->peripheral.usartPort->CTRL |= USART_CTRL_AUTOCS;
    #endif
  }

  if (initData->csControl == spidrvCsControlAuto) {
    // SPI 4 wire mode
#if defined(USART_ROUTEPEN_TXPEN)
    handle->peripheral.usartPort->ROUTELOC0 = (handle->peripheral.usartPort->ROUTELOC0
                                               & ~(_USART_ROUTELOC0_TXLOC_MASK
                                                   | _USART_ROUTELOC0_RXLOC_MASK
                                                   | _USART_ROUTELOC0_CLKLOC_MASK
                                                   | _USART_ROUTELOC0_CSLOC_MASK))
                                              | (initData->portLocationTx  << _USART_ROUTELOC0_TXLOC_SHIFT)
                                              | (initData->portLocationRx  << _USART_ROUTELOC0_RXLOC_SHIFT)
                                              | (initData->portLocationClk << _USART_ROUTELOC0_CLKLOC_SHIFT)
                                              | (initData->portLocationCs  << _USART_ROUTELOC0_CSLOC_SHIFT);

    handle->peripheral.usartPort->ROUTEPEN = USART_ROUTEPEN_TXPEN
                                             | USART_ROUTEPEN_RXPEN
                                             | USART_ROUTEPEN_CLKPEN
                                             | USART_ROUTEPEN_CSPEN;
#elif defined (_GPIO_USART_ROUTEEN_MASK)
    GPIO->USARTROUTE[spiPortNum].TXROUTE = ((uint32_t)initData->portTx
                                            << _GPIO_USART_TXROUTE_PORT_SHIFT)
                                           | ((uint32_t)initData->pinTx
                                              << _GPIO_USART_TXROUTE_PIN_SHIFT);

    GPIO->USARTROUTE[spiPortNum].RXROUTE = ((uint32_t)initData->portRx
                                            << _GPIO_USART_RXROUTE_PORT_SHIFT)
                                           | ((uint32_t)initData->pinRx
                                              << _GPIO_USART_RXROUTE_PIN_SHIFT);

    GPIO->USARTROUTE[spiPortNum].CLKROUTE = ((uint32_t)initData->portClk
                                             << _GPIO_USART_CLKROUTE_PORT_SHIFT)
                                            | ((uint32_t)initData->pinClk
                                               << _GPIO_USART_CLKROUTE_PIN_SHIFT);

    GPIO->USARTROUTE[spiPortNum].CSROUTE = ((uint32_t)initData->portCs
                                            << _GPIO_USART_CSROUTE_PORT_SHIFT)
                                           | ((uint32_t)initData->pinCs
                                              << _GPIO_USART_CSROUTE_PIN_SHIFT);

    GPIO->USARTROUTE[spiPortNum].ROUTEEN = GPIO_USART_ROUTEEN_TXPEN
                                           | GPIO_USART_ROUTEEN_RXPEN
                                           | GPIO_USART_ROUTEEN_CLKPEN
                                           | GPIO_USART_ROUTEEN_CSPEN;
#else
    handle->peripheral.usartPort->ROUTE = USART_ROUTE_TXPEN
                                          | USART_ROUTE_RXPEN
                                          | USART_ROUTE_CLKPEN
                                          | USART_ROUTE_CSPEN
                                          | (initData->portLocation
                                             << _USART_ROUTE_LOCATION_SHIFT);
#endif
  } else {
    // SPI 3 wire mode
#if defined(USART_ROUTEPEN_TXPEN)
    handle->peripheral.usartPort->ROUTELOC0 = (handle->peripheral.usartPort->ROUTELOC0
                                               & ~(_USART_ROUTELOC0_TXLOC_MASK
                                                   | _USART_ROUTELOC0_RXLOC_MASK
                                                   | _USART_ROUTELOC0_CLKLOC_MASK))
                                              | (initData->portLocationTx  << _USART_ROUTELOC0_TXLOC_SHIFT)
                                              | (initData->portLocationRx  << _USART_ROUTELOC0_RXLOC_SHIFT)
                                              | (initData->portLocationClk << _USART_ROUTELOC0_CLKLOC_SHIFT);

    handle->peripheral.usartPort->ROUTEPEN = USART_ROUTEPEN_TXPEN
                                             | USART_ROUTEPEN_RXPEN
                                             | USART_ROUTEPEN_CLKPEN;
#elif defined (GPIO_USART_ROUTEEN_TXPEN)
    GPIO->USARTROUTE[spiPortNum].TXROUTE = ((uint32_t)initData->portTx
                                            << _GPIO_USART_TXROUTE_PORT_SHIFT)
                                           | ((uint32_t)initData->pinTx
                                              << _GPIO_USART_TXROUTE_PIN_SHIFT);

    GPIO->USARTROUTE[spiPortNum].RXROUTE = ((uint32_t)initData->portRx
                                            << _GPIO_USART_RXROUTE_PORT_SHIFT)
                                           | ((uint32_t)initData->pinRx
                                              << _GPIO_USART_RXROUTE_PIN_SHIFT);

    GPIO->USARTROUTE[spiPortNum].CLKROUTE = ((uint32_t)initData->portClk
                                             << _GPIO_USART_CLKROUTE_PORT_SHIFT)
                                            | ((uint32_t)initData->pinClk
                                               << _GPIO_USART_CLKROUTE_PIN_SHIFT);

    GPIO->USARTROUTE[spiPortNum].ROUTEEN = GPIO_USART_ROUTEEN_TXPEN
                                           | GPIO_USART_ROUTEEN_RXPEN
                                           | GPIO_USART_ROUTEEN_CLKPEN;
#else
    handle->peripheral.usartPort->ROUTE = USART_ROUTE_TXPEN
                                          | USART_ROUTE_RXPEN
                                          | USART_ROUTE_CLKPEN
                                          | (initData->portLocation
                                             << _USART_ROUTE_LOCATION_SHIFT);
#endif
  }

  if ((retVal = ConfigGPIO(handle, true)) != ECODE_EMDRV_SPIDRV_OK) {
    return retVal;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  if (!spidrvIsInitialized) {
    spidrvIsInitialized = true;
    CORE_EXIT_ATOMIC();

#if defined(EMDRV_SPIDRV_INCLUDE_SLAVE)
    sl_sleeptimer_init();
#endif
  } else {
    CORE_EXIT_ATOMIC();
  }

  // Initialize DMA.
  DMADRV_Init();

  if (DMADRV_AllocateChannel(&handle->txDMACh, NULL) != ECODE_EMDRV_DMADRV_OK) {
    return ECODE_EMDRV_SPIDRV_DMA_ALLOC_ERROR;
  }

  if (DMADRV_AllocateChannel(&handle->rxDMACh, NULL) != ECODE_EMDRV_DMADRV_OK) {
    return ECODE_EMDRV_SPIDRV_DMA_ALLOC_ERROR;
  }

  return ECODE_EMDRV_SPIDRV_OK;
}
#endif // defined USART_PRESENT

#if defined(EUSART_PRESENT)
/***************************************************************************//**
 * @brief
 *    Initialize an SPI driver eusart instance.
 *
 * @param[out] handle  Pointer to an SPI driver handle; refer to @ref
 *                     SPIDRV_Handle_t.
 *
 * @param[in] initData Pointer to an initialization data structure;
 *                     refer to @ref SPIDRV_Init.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success. On failure, an appropriate
 *    SPIDRV @ref Ecode_t is returned.
 ******************************************************************************/
static Ecode_t SPIDRV_InitEusart(SPIDRV_Handle_t handle, SPIDRV_Init_t *initData)
{
  Ecode_t retVal;
  int8_t spiPortNum = -1;
  sl_bus_clock_t bus_clock;
#if defined(_SILICON_LABS_32B_SERIES_2)
  EUSART_SpiAdvancedInit_TypeDef eusartAdvancedSpiInit = EUSART_SPI_ADVANCED_INIT_DEFAULT;
  EUSART_SpiInit_TypeDef eusartSpiInit = EUSART_SPI_MASTER_INIT_DEFAULT_HF;
  eusartSpiInit.advancedSettings = &eusartAdvancedSpiInit;
#else
  sl_clock_branch_t clock_branch;
  sl_hal_eusart_spi_advanced_config_t eusartAdvancedSpiInit = SL_HAL_EUSART_SPI_ADVANCED_INIT_DEFAULT;
  sl_hal_eusart_spi_config_t eusartSpiInit = SL_HAL_EUSART_SPI_MASTER_INIT_DEFAULT_HF;
  eusartSpiInit.advanced_config = &eusartAdvancedSpiInit;
#endif

  if (handle == NULL) {
    return ECODE_EMDRV_SPIDRV_ILLEGAL_HANDLE;
  }

  if (initData == NULL) {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }

  memset(handle, 0, sizeof(SPIDRV_HandleData_t));
  emRequestInit(handle);

  if (0) {
#if defined(EUSART0)
  } else if (initData->port == EUSART0) {
    handle->usartPeripheral  = SL_PERIPHERAL_EUSART0;
    handle->txDMASignal = dmadrvPeripheralSignal_EUSART0_TXBL;
    handle->rxDMASignal = dmadrvPeripheralSignal_EUSART0_RXDATAV;
    spiPortNum = 0;
#endif
#if defined(EUSART1)
  } else if (initData->port == EUSART1) {
    handle->usartPeripheral  = SL_PERIPHERAL_EUSART1;
    handle->txDMASignal = dmadrvPeripheralSignal_EUSART1_TXBL;
    handle->rxDMASignal = dmadrvPeripheralSignal_EUSART1_RXDATAV;
    spiPortNum = 1;
#endif
#if defined(EUSART2)
  } else if (initData->port == EUSART2) {
    handle->usartPeripheral  = SL_PERIPHERAL_EUSART2;
    handle->txDMASignal = dmadrvPeripheralSignal_EUSART2_TXBL;
    handle->rxDMASignal = dmadrvPeripheralSignal_EUSART2_RXDATAV;
    spiPortNum = 2;
#endif
#if defined(EUSART3)
  } else if (initData->port == EUSART3) {
    handle->usartPeripheral  = SL_PERIPHERAL_EUSART3;
    handle->txDMASignal = dmadrvPeripheralSignal_EUSART3_TXBL;
    handle->rxDMASignal = dmadrvPeripheralSignal_EUSART3_RXDATAV;
    spiPortNum = 3;
#endif
#if defined(EUSART4)
  } else if (initData->port == EUSART4) {
    handle->usartPeripheral  = SL_PERIPHERAL_EUSART4;
    handle->txDMASignal = dmadrvPeripheralSignal_EUSART4_TXBL;
    handle->rxDMASignal = dmadrvPeripheralSignal_EUSART4_RXDATAV;
    spiPortNum = 4;
#endif
  } else {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }

  handle->peripheral.eusartPort = initData->port;
  handle->peripheralType        = spidrvPeripheralTypeEusart;
  handle->initData              = *initData;
#if defined(_SILICON_LABS_32B_SERIES_2)
  if (initData->bitOrder == spidrvBitOrderMsbFirst) {
    eusartAdvancedSpiInit.msbFirst = true;
  }

  if (initData->clockMode == spidrvClockMode0) {
    eusartSpiInit.clockMode = eusartClockMode0;
  } else if (initData->clockMode == spidrvClockMode1) {
    eusartSpiInit.clockMode = eusartClockMode1;
  } else if (initData->clockMode == spidrvClockMode2) {
    eusartSpiInit.clockMode = eusartClockMode2;
  } else if (initData->clockMode == spidrvClockMode3) {
    eusartSpiInit.clockMode = eusartClockMode3;
  } else {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }

  if (initData->type == spidrvSlave) {
    if (initData->bitRate >= 5000000) {
      // If baud-rate is more than 5MHz, a value of 4 is required
      eusartSpiInit.advancedSettings->setupWindow = 4;
    } else {
      // If baud-rate is less than 5MHz, a value of 5 is required
      eusartSpiInit.advancedSettings->setupWindow = 5;
    }
    eusartSpiInit.master  = false;
    eusartSpiInit.bitRate = 1000000;
  } else {
    eusartAdvancedSpiInit.forceLoad = false;
    eusartSpiInit.bitRate = initData->bitRate;
  }
#else /* Series 3 and above */
  if (initData->bitOrder == spidrvBitOrderMsbFirst) {
    eusartAdvancedSpiInit.msb_first = true;
  }

  if (initData->clockMode == spidrvClockMode0) {
    eusartSpiInit.clock_mode = SL_HAL_EUSART_CLOCK_MODE_0;
  } else if (initData->clockMode == spidrvClockMode1) {
    eusartSpiInit.clock_mode = SL_HAL_EUSART_CLOCK_MODE_1;
  } else if (initData->clockMode == spidrvClockMode2) {
    eusartSpiInit.clock_mode = SL_HAL_EUSART_CLOCK_MODE_2;
  } else if (initData->clockMode == spidrvClockMode3) {
    eusartSpiInit.clock_mode = SL_HAL_EUSART_CLOCK_MODE_3;
  } else {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }

  uint32_t ref_freq;
  if (initData->type == spidrvSlave) {
    if (initData->bitRate >= 5000000) {
      // If baud-rate is more than 5MHz, a value of 4 is required
      eusartSpiInit.advanced_config->setup_window = 4;
    } else {
      // If baud-rate is less than 5MHz, a value of 5 is required
      eusartSpiInit.advanced_config->setup_window = 5;
    }
    eusartSpiInit.master  = false;
    clock_branch = sl_device_peripheral_get_clock_branch(handle->usartPeripheral);
    sl_clock_manager_get_clock_branch_frequency(clock_branch, &ref_freq);
    eusartSpiInit.clock_div = sl_hal_eusart_spi_calculate_clock_div(ref_freq, 1000000);
  } else {
    eusartAdvancedSpiInit.force_load = false;
    clock_branch = sl_device_peripheral_get_clock_branch(handle->usartPeripheral);
    sl_clock_manager_get_clock_branch_frequency(clock_branch, &ref_freq);
    eusartSpiInit.clock_div = sl_hal_eusart_spi_calculate_clock_div(ref_freq, initData->bitRate);
  }
#endif
  // Leave eusartAdvancedSpiInit.defaultTxData = 0, not initData->dummyTxValue
  // for EUSART compatibility with USART behavior.

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
  bus_clock = sl_device_peripheral_get_bus_clock(handle->usartPeripheral);
  sl_clock_manager_enable_bus_clock(bus_clock);

  if ((initData->frameLength < 7U) || (initData->frameLength > 16U)) {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }

  uint32_t databits = initData->frameLength - 7U + EUSART_FRAMECFG_DATABITS_SEVEN;
#if defined(_SILICON_LABS_32B_SERIES_2)
  eusartSpiInit.databits = (EUSART_Databits_TypeDef)databits;
  if (initData->type == spidrvMaster) {
    // Don't assume EUSART_SPI_ADVANCED_INIT_DEFAULT has desired autoCsEnable
    eusartAdvancedSpiInit.autoCsEnable = (initData->csControl == spidrvCsControlAuto);
  }
  if ((retVal = ConfigGPIO(handle, true)) != ECODE_EMDRV_SPIDRV_OK) {
    return retVal;
  }
  EUSART_SpiInit(initData->port, &eusartSpiInit);
#else /* Series 3 and above */
  eusartSpiInit.data_bits = (sl_hal_eusart_data_bits_t)databits;
  if (initData->type == spidrvMaster) {
    // Don't assume EUSART_SPI_ADVANCED_INIT_DEFAULT has desired autoCsEnable
    eusartAdvancedSpiInit.auto_cs_enable = (initData->csControl == spidrvCsControlAuto);
  }
  if ((retVal = ConfigGPIO(handle, true)) != ECODE_EMDRV_SPIDRV_OK) {
    return retVal;
  }
  sl_hal_eusart_init_spi(initData->port, &eusartSpiInit);
  sl_hal_eusart_enable(initData->port);
  sl_hal_eusart_enable_rx(initData->port);
  sl_hal_eusart_enable_tx(initData->port);
  sl_hal_eusart_wait_sync(initData->port, _EUSART_SYNCBUSY_MASK);
#endif

  // Common GPIO EUSARTROUTE configuration for both series
  GPIO->EUSARTROUTE[spiPortNum].TXROUTE = ((uint32_t)initData->portTx
                                           << _GPIO_EUSART_TXROUTE_PORT_SHIFT)
                                          | ((uint32_t)initData->pinTx
                                             << _GPIO_EUSART_TXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[spiPortNum].RXROUTE = ((uint32_t)initData->portRx
                                           << _GPIO_EUSART_RXROUTE_PORT_SHIFT)
                                          | ((uint32_t)initData->pinRx
                                             << _GPIO_EUSART_RXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[spiPortNum].SCLKROUTE = ((uint32_t)initData->portClk
                                             << _GPIO_EUSART_SCLKROUTE_PORT_SHIFT)
                                            | ((uint32_t)initData->pinClk
                                               << _GPIO_EUSART_SCLKROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[spiPortNum].ROUTEEN = GPIO_EUSART_ROUTEEN_TXPEN
                                          | GPIO_EUSART_ROUTEEN_RXPEN
                                          | GPIO_EUSART_ROUTEEN_SCLKPEN;
  if (initData->csControl == spidrvCsControlAuto) {
    // SPI 4 wire mode, Chip Select controlled by the peripheral
    GPIO->EUSARTROUTE[spiPortNum].CSROUTE = ((uint32_t)initData->portCs << _GPIO_EUSART_CSROUTE_PORT_SHIFT)
                                            | ((uint32_t)initData->pinCs << _GPIO_EUSART_CSROUTE_PIN_SHIFT);
    GPIO->EUSARTROUTE_SET[spiPortNum].ROUTEEN = GPIO_EUSART_ROUTEEN_CSPEN;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  if (!spidrvIsInitialized) {
    spidrvIsInitialized = true;
    CORE_EXIT_ATOMIC();

#if defined(EMDRV_SPIDRV_INCLUDE_SLAVE)
    sl_sleeptimer_init();
#endif
  } else {
    CORE_EXIT_ATOMIC();
  }

  // Initialize DMA.
  DMADRV_Init();

  if (DMADRV_AllocateChannel(&handle->txDMACh, NULL) != ECODE_EMDRV_DMADRV_OK) {
    return ECODE_EMDRV_SPIDRV_DMA_ALLOC_ERROR;
  }

  if (DMADRV_AllocateChannel(&handle->rxDMACh, NULL) != ECODE_EMDRV_DMADRV_OK) {
    return ECODE_EMDRV_SPIDRV_DMA_ALLOC_ERROR;
  }

  return ECODE_EMDRV_SPIDRV_OK;
}
#endif  // defined(EUSART_PRESENT)

/***************************************************************************//**
 * @brief
 *    Deinitialize an SPI driver instance.
 *
 * @warning
 *  This function should only be called with an initialized spidrv instance handle.
 *
 * @param[in] handle Pointer to an SPI driver handle.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success. On failure, an appropriate
 *    SPIDRV @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_DeInit(SPIDRV_Handle_t handle)
{
  sl_bus_clock_t bus_clock;
#if defined(USART_PRESENT)
  int8_t USART_spiPortNum = -1;
#endif

#if defined(EUSART_PRESENT)
  int8_t EUSART_spiPortNum = -1;
#endif

  if (0) {
  }
#if defined(USART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeUsart) {
#if (USART_COUNT == 1) && !defined(USART_NUM)
    USART_spiPortNum = 0;
#else
    USART_spiPortNum = USART_NUM((USART_TypeDef*)handle->peripheral.usartPort);
#endif
    if (USART_spiPortNum == -1) {
      return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
    }
  }
#endif
#if defined (EUSART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeEusart) {
#if (EUSART_COUNT == 1) && !defined(EUSART_NUM)
    EUSART_spiPortNum = 0;
#else
    EUSART_spiPortNum = EUSART_NUM((EUSART_TypeDef*)handle->peripheral.eusartPort);
#endif
    if (EUSART_spiPortNum == -1) {
      return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
    }
  }
#endif

  if (handle == NULL) {
    return ECODE_EMDRV_SPIDRV_ILLEGAL_HANDLE;
  }

  // Stop DMAs.
  DMADRV_StopTransfer(handle->rxDMACh);
  DMADRV_StopTransfer(handle->txDMACh);

  ConfigGPIO(handle, false);

#if defined(EMDRV_SPIDRV_INCLUDE_SLAVE)
  if (handle->initData.type == spidrvSlave) {
    sl_sleeptimer_stop_timer(&handle->timer);
  }
#endif

  if (0) {
  }
#if defined(USART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeUsart) {
    USART_Reset(handle->peripheral.usartPort);
  }
#endif
#if defined(EUSART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeEusart) {
#if defined(_SILICON_LABS_32B_SERIES_2)
    EUSART_Reset(handle->peripheral.eusartPort);
#else
    sl_hal_eusart_reset(handle->peripheral.eusartPort);
#endif
  }
#endif

  if (0) {
  }
#if defined(USART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeUsart) {
#if defined(USART_ROUTEPEN_TXPEN)
    handle->peripheral.usartPort->ROUTELOC0 = _USART_ROUTELOC0_RESETVALUE;
    handle->peripheral.usartPort->ROUTEPEN = _USART_ROUTEPEN_RESETVALUE;
#elif defined(GPIO_USART_ROUTEEN_TXPEN)
    GPIO->USARTROUTE[USART_spiPortNum].ROUTEEN = _GPIO_USART_ROUTEEN_RESETVALUE;
    GPIO->USARTROUTE[USART_spiPortNum].TXROUTE = _GPIO_USART_TXROUTE_RESETVALUE;
    GPIO->USARTROUTE[USART_spiPortNum].RXROUTE = _GPIO_USART_RXROUTE_RESETVALUE;
    GPIO->USARTROUTE[USART_spiPortNum].CLKROUTE = _GPIO_USART_CLKROUTE_RESETVALUE;
    if (handle->initData.csControl == spidrvCsControlAuto) {
      // SPI 4 wire mode
      GPIO->USARTROUTE[USART_spiPortNum].CSROUTE = _GPIO_USART_CSROUTE_RESETVALUE;
    }
#else
    handle->peripheral.usartPort->ROUTE = _USART_ROUTE_RESETVALUE;
#endif
  }
#endif
#if defined(EUSART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeEusart) {
    GPIO->EUSARTROUTE[EUSART_spiPortNum].TXROUTE = _GPIO_EUSART_TXROUTE_RESETVALUE;
    GPIO->EUSARTROUTE[EUSART_spiPortNum].RXROUTE = _GPIO_EUSART_RXROUTE_RESETVALUE;
    GPIO->EUSARTROUTE[EUSART_spiPortNum].SCLKROUTE = _GPIO_EUSART_SCLKROUTE_RESETVALUE;
    if (handle->initData.csControl == spidrvCsControlAuto) {
      // SPI 4 wire mode, Chip Select controled by the peripheral
      GPIO->EUSARTROUTE[EUSART_spiPortNum].CSROUTE = _GPIO_EUSART_CSROUTE_RESETVALUE;
    }
    GPIO->EUSARTROUTE[EUSART_spiPortNum].ROUTEEN = _GPIO_EUSART_ROUTEEN_RESETVALUE;
  }
#endif

  bus_clock = sl_device_peripheral_get_bus_clock(handle->usartPeripheral);
  sl_clock_manager_disable_bus_clock(bus_clock);

  DMADRV_FreeChannel(handle->txDMACh);
  DMADRV_FreeChannel(handle->rxDMACh);
  DMADRV_DeInit();
  emRequestDeinit(handle);

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && defined(EUSART_PRESENT)
  // Unsubscribe to notification to re-enable eusart after deepsleep.
  sl_slist_remove(&eusart_handle_list, &handle->node);
  if (eusart_handle_list == NULL) {
    sl_power_manager_unsubscribe_em_transition_event(&on_power_manager_event_handle);
  }
#endif

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Abort an ongoing SPI transfer.
 *
 * @param[in] handle Pointer to an SPI driver handle.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success, @ref ECODE_EMDRV_SPIDRV_IDLE if
 *    SPI is idle. On failure, an appropriate SPIDRV @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_AbortTransfer(SPIDRV_Handle_t handle)
{
  if (handle == NULL) {
    return ECODE_EMDRV_SPIDRV_ILLEGAL_HANDLE;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  if (handle->state == spidrvStateIdle) {
    CORE_EXIT_ATOMIC();
    return ECODE_EMDRV_SPIDRV_IDLE;
  }

#if defined(EMDRV_SPIDRV_INCLUDE_SLAVE)
  if (handle->initData.type == spidrvSlave) {
    sl_sleeptimer_stop_timer(&handle->timer);
  }
#endif

  // Stop DMA's.
  DMADRV_StopTransfer(handle->rxDMACh);
  DMADRV_StopTransfer(handle->txDMACh);
  DMADRV_TransferRemainingCount(handle->rxDMACh, &handle->remaining);
  handle->transferStatus    = ECODE_EMDRV_SPIDRV_ABORTED;
  handle->state             = spidrvStateIdle;
  handle->transferStatus    = ECODE_EMDRV_SPIDRV_ABORTED;
  handle->blockingCompleted = true;

  em1RequestRemove(handle);

  if (handle->userCallback != NULL) {
    handle->userCallback(handle,
                         ECODE_EMDRV_SPIDRV_ABORTED,
                         handle->transferCount - handle->remaining);
  }
  CORE_EXIT_ATOMIC();

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Get current SPI bus bitrate.
 *
 * @param[in] handle Pointer to an SPI driver handle.
 *
 * @param[out] bitRate Current SPI bus bitrate.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success. On failure, an appropriate SPIDRV
 *    @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_GetBitrate(SPIDRV_Handle_t handle, uint32_t *bitRate)
{
#if defined(EUSART_PRESENT) && !defined(_SILICON_LABS_32B_SERIES_2)
  sl_clock_branch_t clock_branch;
#endif
  if (handle == NULL) {
    return ECODE_EMDRV_SPIDRV_ILLEGAL_HANDLE;
  }

  if (bitRate == NULL) {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }

  if (0) {
  }
#if defined(USART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeUsart) {
    *bitRate = USART_BaudrateGet(handle->peripheral.usartPort);
  }
#endif
#if defined(EUSART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeEusart) {
#if defined(_SILICON_LABS_32B_SERIES_2)
    *bitRate = EUSART_BaudrateGet(handle->peripheral.eusartPort);
#else
    uint32_t freq;
    clock_branch = sl_device_peripheral_get_clock_branch(handle->usartPeripheral);
    sl_clock_manager_get_clock_branch_frequency(clock_branch, &freq);
    uint32_t div = (handle->peripheral.eusartPort->CFG2 & _EUSART_CFG2_SDIV_MASK) >> _EUSART_CFG2_SDIV_SHIFT;
    *bitRate = freq / (div + 1);
#endif
  }
#endif

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Get current SPI framelength.
 *
 * @param[in] handle Pointer to an SPI driver handle.
 *
 * @param[out] frameLength Current SPI bus framelength.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success. On failure, an appropriate SPIDRV
 *    @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_GetFramelength(SPIDRV_Handle_t handle, uint32_t *frameLength)
{
  if (handle == NULL) {
    return ECODE_EMDRV_SPIDRV_ILLEGAL_HANDLE;
  }

  if (frameLength == NULL) {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }

  *frameLength = handle->initData.frameLength;

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Get the status of an SPI transfer.
 *
 * @details
 *    Returns status of an ongoing transfer. If no transfer is in progress,
 *    the status of the last transfer is reported.
 *
 * @param[in] handle Pointer to an SPI driver handle.
 *
 * @param[out] itemsTransferred Number of items (frames) transferred.
 *
 * @param[out] itemsRemaining Number of items (frames) remaining.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success. On failure, an appropriate SPIDRV
 *    @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_GetTransferStatus(SPIDRV_Handle_t handle,
                                 int *itemsTransferred,
                                 int *itemsRemaining)
{
  int remaining;

  if (handle == NULL) {
    return ECODE_EMDRV_SPIDRV_ILLEGAL_HANDLE;
  }

  if ((itemsTransferred == NULL) || (itemsRemaining == NULL)) {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  if (handle->state == spidrvStateIdle) {
    remaining = handle->remaining;
  } else {
    DMADRV_TransferRemainingCount(handle->rxDMACh, &remaining);
  }
  CORE_EXIT_ATOMIC();

  *itemsTransferred = (handle->transferCount - remaining);
  *itemsRemaining = remaining;

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Start an SPI master receive transfer.
 *
 * @note
 *    The MOSI wire will transmit @ref SPIDRV_Init.dummyTxValue.
 *
 * @param[in]  handle Pointer to an SPI driver handle.
 *
 * @param[out] buffer Receive data buffer.
 *
 * @param[in]  count Number of bytes in transfer.
 *
 * @param[in]  callback Transfer completion callback.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success. On failure, an appropriate SPIDRV
 *    @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_MReceive(SPIDRV_Handle_t handle,
                        void *buffer,
                        int count,
                        SPIDRV_Callback_t callback)
{
  Ecode_t retVal;

  if (handle->initData.type == spidrvSlave) {
    return ECODE_EMDRV_SPIDRV_MODE_ERROR;
  }

  if ((retVal = TransferApiPrologue(handle, buffer, count))
      != ECODE_EMDRV_SPIDRV_OK) {
    return retVal;
  }

  StartReceiveDMA(handle, buffer, count, callback);

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Start an SPI master blocking receive transfer.
 *
 * @note
 *    The MOSI wire will transmit @ref SPIDRV_Init.dummyTxValue.
 *    @n This function is blocking and returns when the transfer is complete
 *    or when @ref SPIDRV_AbortTransfer() is called.
 *
 * @param[in]  handle Pointer to an SPI driver handle.
 *
 * @param[out] buffer Receive data buffer.
 *
 * @param[in]  count Number of bytes in transfer.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success or @ref ECODE_EMDRV_SPIDRV_ABORTED
 *    if @ref SPIDRV_AbortTransfer() has been called. On failure, an appropriate
 *    SPIDRV @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_MReceiveB(SPIDRV_Handle_t handle,
                         void *buffer,
                         int count)
{
  Ecode_t retVal;

  if (handle->initData.type == spidrvSlave) {
    return ECODE_EMDRV_SPIDRV_MODE_ERROR;
  }

  if ((retVal = TransferApiBlockingPrologue(handle, buffer, count))
      != ECODE_EMDRV_SPIDRV_OK) {
    return retVal;
  }

  StartReceiveDMA(handle, buffer, count, BlockingComplete);

  WaitForTransferCompletion(handle);

  return handle->transferStatus;
}

/***************************************************************************//**
 * @brief
 *    Start an SPI master transfer.
 *
 * @param[in]  handle Pointer to an SPI driver handle.
 *
 * @param[in]  txBuffer Transmit data buffer.
 *
 * @param[out] rxBuffer Receive data buffer.
 *
 * @param[in]  count Number of bytes in transfer.
 *
 * @param[in]  callback Transfer completion callback.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success. On failure, an appropriate SPIDRV
 *    @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_MTransfer(SPIDRV_Handle_t handle,
                         const void *txBuffer,
                         void *rxBuffer,
                         int count,
                         SPIDRV_Callback_t callback)
{
  Ecode_t retVal;

  if (handle->initData.type == spidrvSlave) {
    return ECODE_EMDRV_SPIDRV_MODE_ERROR;
  }

  if ((retVal = TransferApiPrologue(handle, (void*)txBuffer, count))
      != ECODE_EMDRV_SPIDRV_OK) {
    return retVal;
  }

  if (rxBuffer == NULL) {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }

  StartTransferDMA(handle, txBuffer, rxBuffer, count, callback);

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Start an SPI master blocking transfer.
 *
 * @note
 *    This function is blocking and returns when the transfer is complete
 *    or when @ref SPIDRV_AbortTransfer() is called.
 *
 * @param[in]  handle Pointer to an SPI driver handle.
 *
 * @param[in]  txBuffer Transmit data buffer.
 *
 * @param[out] rxBuffer Receive data buffer.
 *
 * @param[in]  count Number of bytes in transfer.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success or @ref ECODE_EMDRV_SPIDRV_ABORTED
 *    if @ref SPIDRV_AbortTransfer() has been called. On failure, an appropriate
 *    SPIDRV @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_MTransferB(SPIDRV_Handle_t handle,
                          const void *txBuffer,
                          void *rxBuffer,
                          int count)
{
  Ecode_t retVal;

  if (handle->initData.type == spidrvSlave) {
    return ECODE_EMDRV_SPIDRV_MODE_ERROR;
  }

  if ((retVal = TransferApiBlockingPrologue(handle, (void*)txBuffer, count))
      != ECODE_EMDRV_SPIDRV_OK) {
    return retVal;
  }

  if (rxBuffer == NULL) {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }

  StartTransferDMA(handle, txBuffer, rxBuffer, count, BlockingComplete);

  WaitForTransferCompletion(handle);

  return handle->transferStatus;
}

/***************************************************************************//**
 * @brief
 *    Start an SPI master blocking single item (frame) transfer.
 *
 * @note
 *    This function is blocking and returns when the transfer is complete
 *    or when @ref SPIDRV_AbortTransfer() is called.
 *
 * @param[in] handle Pointer to an SPI driver handle.
 *
 * @param[in] txValue Value to transmit.
 *
 * @param[out] rxValue Value received.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success or @ref ECODE_EMDRV_SPIDRV_ABORTED
 *    if @ref SPIDRV_AbortTransfer() has been called. On failure, an appropriate
 *    SPIDRV @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_MTransferSingleItemB(SPIDRV_Handle_t handle,
                                    uint32_t txValue,
                                    void *rxValue)
{
  void *pRx;
  uint32_t rxBuffer;

  if (handle == NULL) {
    return ECODE_EMDRV_SPIDRV_ILLEGAL_HANDLE;
  }

  if (handle->initData.type == spidrvSlave) {
    return ECODE_EMDRV_SPIDRV_MODE_ERROR;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  if (handle->state != spidrvStateIdle) {
    CORE_EXIT_ATOMIC();
    return ECODE_EMDRV_SPIDRV_BUSY;
  }
  handle->state = spidrvStateTransferring;
  CORE_EXIT_ATOMIC();

  if ((pRx = rxValue) == NULL) {
    pRx = &rxBuffer;
  }

  StartTransferDMA(handle, &txValue, pRx, 1, BlockingComplete);

  WaitForTransferCompletion(handle);

  return handle->transferStatus;
}

/***************************************************************************//**
 * @brief
 *    Start an SPI master transmit transfer.
 *
 * @note
 *    The data received on the MISO wire is discarded.
 *
 * @param[in] handle Pointer to an SPI driver handle.
 *
 * @param[in] buffer Transmit data buffer.
 *
 * @param[in] count Number of bytes in transfer.
 *
 * @param[in] callback Transfer completion callback.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success. On failure, an appropriate SPIDRV
 *    @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_MTransmit(SPIDRV_Handle_t handle,
                         const void *buffer,
                         int count,
                         SPIDRV_Callback_t callback)
{
  Ecode_t retVal;

  if (handle->initData.type == spidrvSlave) {
    return ECODE_EMDRV_SPIDRV_MODE_ERROR;
  }

  if ((retVal = TransferApiPrologue(handle, (void*)buffer, count))
      != ECODE_EMDRV_SPIDRV_OK) {
    return retVal;
  }

  StartTransmitDMA(handle, buffer, count, callback);

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Start an SPI master blocking transmit transfer.
 *
 * @note
 *    The data received on the MISO wire is discarded.
 *    @n This function is blocking and returns when the transfer is complete.
 *
 * @param[in] handle Pointer to an SPI driver handle.
 *
 * @param[in] buffer Transmit data buffer.
 *
 * @param[in] count Number of bytes in transfer.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success or @ref ECODE_EMDRV_SPIDRV_ABORTED
 *    if @ref SPIDRV_AbortTransfer() has been called. On failure, an appropriate
 *    SPIDRV @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_MTransmitB(SPIDRV_Handle_t handle,
                          const void *buffer,
                          int count)
{
  Ecode_t retVal;

  if (handle->initData.type == spidrvSlave) {
    return ECODE_EMDRV_SPIDRV_MODE_ERROR;
  }

  if ((retVal = TransferApiBlockingPrologue(handle, (void*)buffer, count))
      != ECODE_EMDRV_SPIDRV_OK) {
    return retVal;
  }

  StartTransmitDMA(handle, buffer, count, BlockingComplete);

  WaitForTransferCompletion(handle);

  return handle->transferStatus;
}

/***************************************************************************//**
 * @brief
 *    Set SPI bus bitrate.
 *
 * @param[in] handle Pointer to an SPI driver handle.
 *
 * @param[in] bitRate New SPI bus bitrate.
 *
 * @details
 *    The configuration will be set to use a bitrate less than or equal to the desired bitrate
 *    to ensure that the bitrate does not exceed the requested value.
 *
 *    For USART peripherals, fractional clock division is suppressed
 *    in synchronous mode by this function, even though the hardware allows it.
 *    This restriction prevents timing violations causing framing errors and data corruption at high frequencies.
 *    The fractional clock division modifies the clock cycles to average out around the desired bitrate,
 *    adjusting some half periods clock cycle count.
 *    Fractional divider affects the duty cycle of the SCLK, impact is greater at high bitrates where there are few clock cycle count per
 *    half period, and can cause timing issues like MISO setup time violations.
 *    The clock divider will be set to an integer value, so the reference clock
 *    will only be divided by 2N (where N is a positive integer).
 *
 *    For example, if the reference clock is 39 MHz, the maximum baudrate would be 19.5 MHz (RefClock/2).
 *    If the baudrate is set to 19.5 MHz, the clock divider will be set to 0 and the baudrate will be 19.5 MHz.
 *    If the baudrate is [9.75 - 19.5[ MHz, the clock divider will be set to 1, and the actual baudrate will be (RefClock/4) = 9.75 MHz.
 *    If the baudrate is [6.5 - 9.75[ MHz, the clock divider will be set to 2, and the actual baudrate will be (RefClock/6) = 6.5 MHz.
 *    If the baudrate is [4.875 - 6.5[ MHz, the clock divider will be set to 3, and the actual baudrate will be (RefClock/8) = 4.875 MHz.
 *
 *    SPIDRV_GetBitrate will return the actual baudrate according to the USART peripheral configuration.
 *
 * @warning
 *    If you require fractional clock division, you may manually modify the CLKDIV register.
 *    Users should ensure compliance with USART synchronous mode timing requirements
 *    and consider using integer division ratios of the reference clock for reliable
 *    operation at high bitrates.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success. On failure, an appropriate SPIDRV
 *    @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_SetBitrate(SPIDRV_Handle_t handle, uint32_t bitRate)
{
#if defined(EUSART_PRESENT) && !defined(_SILICON_LABS_32B_SERIES_2)
  sl_clock_branch_t clock_branch;
#endif
  if (handle == NULL) {
    return ECODE_EMDRV_SPIDRV_ILLEGAL_HANDLE;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  if (handle->state != spidrvStateIdle) {
    CORE_EXIT_ATOMIC();
    return ECODE_EMDRV_SPIDRV_BUSY;
  }

  handle->initData.bitRate = bitRate;

  if (0) {
  }
#if defined(USART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeUsart) {
    USART_BaudrateSyncSet(handle->peripheral.usartPort, 0, bitRate);
  }
#endif
#if defined(EUSART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeEusart) {
#if defined(_SILICON_LABS_32B_SERIES_2)
    EUSART_BaudrateSet(handle->peripheral.eusartPort, 0, bitRate);
#else
    uint32_t ref_freq;
    clock_branch = sl_device_peripheral_get_clock_branch(handle->usartPeripheral);
    sl_clock_manager_get_clock_branch_frequency(clock_branch, &ref_freq);
    uint32_t div = sl_hal_eusart_spi_calculate_clock_div(ref_freq, bitRate);

    sl_hal_eusart_disable_rx(handle->peripheral.eusartPort);
    sl_hal_eusart_disable_tx(handle->peripheral.eusartPort);
    sl_hal_eusart_disable(handle->peripheral.eusartPort);
    while (handle->peripheral.eusartPort->EN & _EUSART_EN_DISABLING_MASK) ;

    handle->peripheral.eusartPort->CFG2 = (handle->peripheral.eusartPort->CFG2 & ~(_EUSART_CFG2_SDIV_MASK))
                                          | ((div << _EUSART_CFG2_SDIV_SHIFT) & _EUSART_CFG2_SDIV_MASK);

    sl_hal_eusart_enable(handle->peripheral.eusartPort);
    sl_hal_eusart_enable_rx(handle->peripheral.eusartPort);
    sl_hal_eusart_enable_tx(handle->peripheral.eusartPort);
    sl_hal_eusart_wait_sync(handle->peripheral.eusartPort, _EUSART_SYNCBUSY_MASK);
#endif
  }
#endif
  CORE_EXIT_ATOMIC();

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Set SPI framelength.
 *
 * @param[in] handle Pointer to an SPI driver handle.
 *
 * @param[in] frameLength New SPI bus framelength.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success. On failure, an appropriate SPIDRV
 *    @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_SetFramelength(SPIDRV_Handle_t handle, uint32_t frameLength)
{
  if (handle == NULL) {
    return ECODE_EMDRV_SPIDRV_ILLEGAL_HANDLE;
  }

  if (0) {
  }
#if defined(USART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeUsart) {
    frameLength -= EMDRV_SPIDRV_USART_FRAMELENGTH_REGVALUE_OFFSET;

    if ((frameLength < _USART_FRAME_DATABITS_FOUR) || (frameLength > _USART_FRAME_DATABITS_SIXTEEN)) {
      return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
    }
  }
#endif
#if defined(EUSART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeEusart) {
    frameLength -= EMDRV_SPIDRV_EUSART_FRAMELENGTH_REGVALUE_OFFSET;

    if ((frameLength < _EUSART_FRAMECFG_DATABITS_SEVEN) || (frameLength > _EUSART_FRAMECFG_DATABITS_SIXTEEN)) {
      return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
    }
  }
#endif

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  if (handle->state != spidrvStateIdle) {
    CORE_EXIT_ATOMIC();
    return ECODE_EMDRV_SPIDRV_BUSY;
  }

  if (0) {
  }
#if defined(USART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeUsart) {
    handle->initData.frameLength = frameLength + EMDRV_SPIDRV_USART_FRAMELENGTH_REGVALUE_OFFSET;

    handle->peripheral.usartPort->FRAME = (handle->peripheral.usartPort->FRAME
                                           & ~_USART_FRAME_DATABITS_MASK)
                                          | (frameLength
                                             << _USART_FRAME_DATABITS_SHIFT);
  }
#endif
#if defined(EUSART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeEusart) {
    handle->initData.frameLength = frameLength + EMDRV_SPIDRV_EUSART_FRAMELENGTH_REGVALUE_OFFSET;

#if defined(_SILICON_LABS_32B_SERIES_2)
    EUSART_Enable(handle->peripheral.eusartPort, eusartDisable);
    handle->peripheral.eusartPort->FRAMECFG = (handle->peripheral.eusartPort->FRAMECFG
                                               & ~_EUSART_FRAMECFG_DATABITS_MASK)
                                              | (frameLength
                                                 << _EUSART_FRAMECFG_DATABITS_SHIFT);
    EUSART_Enable(handle->peripheral.eusartPort, eusartEnable);
#else
    sl_hal_eusart_disable_rx(handle->peripheral.eusartPort);
    sl_hal_eusart_disable_tx(handle->peripheral.eusartPort);
    sl_hal_eusart_disable(handle->peripheral.eusartPort);
    while (handle->peripheral.eusartPort->EN & _EUSART_EN_DISABLING_MASK) ;

    handle->peripheral.eusartPort->FRAMECFG = (handle->peripheral.eusartPort->FRAMECFG
                                               & ~_EUSART_FRAMECFG_DATABITS_MASK)
                                              | (frameLength
                                                 << _EUSART_FRAMECFG_DATABITS_SHIFT);

    sl_hal_eusart_enable(handle->peripheral.eusartPort);
    sl_hal_eusart_enable_rx(handle->peripheral.eusartPort);
    sl_hal_eusart_enable_tx(handle->peripheral.eusartPort);
    sl_hal_eusart_wait_sync(handle->peripheral.eusartPort, _EUSART_SYNCBUSY_MASK);
#endif
  }
#endif
  CORE_EXIT_ATOMIC();

  return ECODE_EMDRV_SPIDRV_OK;
}

#if defined(EMDRV_SPIDRV_INCLUDE_SLAVE)
/***************************************************************************//**
 * @brief
 *    Start an SPI slave receive transfer.
 *
 * @note
 *    The MISO wire will transmit @ref SPIDRV_Init.dummyTxValue.
 *
 * @param[in]  handle Pointer to an SPI driver handle.
 *
 * @param[out] buffer Receive data buffer.
 *
 * @param[in]  count Number of bytes in transfer.
 *
 * @param[in]  callback Transfer completion callback.
 *
 * @param[in]  timeoutMs Transfer timeout in milliseconds.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success. On failure, an appropriate SPIDRV
 *    @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_SReceive(SPIDRV_Handle_t handle,
                        void *buffer,
                        int count,
                        SPIDRV_Callback_t callback,
                        int timeoutMs)
{
  Ecode_t retVal;

  if (handle->initData.type == spidrvMaster) {
    return ECODE_EMDRV_SPIDRV_MODE_ERROR;
  }

  if ((retVal = TransferApiPrologue(handle, buffer, count))
      != ECODE_EMDRV_SPIDRV_OK) {
    return retVal;
  }

  if (timeoutMs) {
    sl_sleeptimer_start_timer_ms(&handle->timer, timeoutMs, SlaveTimeout, handle, 0, 0);
  }

  if (handle->initData.slaveStartMode == spidrvSlaveStartDelayed) {
    if ((retVal = WaitForIdleLine(handle)) != ECODE_EMDRV_SPIDRV_OK) {
      return retVal;
    }
  }

  StartReceiveDMA(handle, buffer, count, callback);

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Start an SPI slave blocking receive transfer.
 *
 * @note
 *    The MISO wire will transmit @ref SPIDRV_Init.dummyTxValue.
 *    @n This function is blocking and returns when the transfer is complete,
 *    on timeout, or when @ref SPIDRV_AbortTransfer() is called.
 *
 * @param[in]  handle Pointer to an SPI driver handle.
 *
 * @param[out] buffer Receive data buffer.
 *
 * @param[in]  count Number of bytes in transfer.
 *
 * @param[in]  timeoutMs Transfer timeout in milliseconds.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success, @ref ECODE_EMDRV_SPIDRV_TIMEOUT on
 *    timeout or @ref ECODE_EMDRV_SPIDRV_ABORTED if @ref SPIDRV_AbortTransfer()
 *    has been called. On failure, an appropriate SPIDRV @ref Ecode_t is
 *    returned.
 ******************************************************************************/
Ecode_t SPIDRV_SReceiveB(SPIDRV_Handle_t handle,
                         void *buffer,
                         int count,
                         int timeoutMs)
{
  Ecode_t retVal;

  if (handle->initData.type == spidrvMaster) {
    return ECODE_EMDRV_SPIDRV_MODE_ERROR;
  }

  if ((retVal = TransferApiBlockingPrologue(handle, buffer, count))
      != ECODE_EMDRV_SPIDRV_OK) {
    return retVal;
  }

  if (timeoutMs) {
    sl_sleeptimer_start_timer_ms(&handle->timer, timeoutMs, SlaveTimeout, handle, 0, 0);
  }

  if (handle->initData.slaveStartMode == spidrvSlaveStartDelayed) {
    if ((retVal = WaitForIdleLine(handle)) != ECODE_EMDRV_SPIDRV_OK) {
      return retVal;
    }
  }

  StartReceiveDMA(handle, buffer, count, BlockingComplete);

  WaitForTransferCompletion(handle);

  return handle->transferStatus;
}

/***************************************************************************//**
 * @brief
 *    Start an SPI slave transfer.
 *
 * @param[in]  handle Pointer to an SPI driver handle.
 *
 * @param[in]  txBuffer Transmit data buffer.
 *
 * @param[out] rxBuffer Receive data buffer.
 *
 * @param[in]  count Number of bytes in transfer.
 *
 * @param[in]  callback Transfer completion callback.
 *
 * @param[in]  timeoutMs Transfer timeout in milliseconds.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success. On failure, an appropriate SPIDRV
 *    @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_STransfer(SPIDRV_Handle_t handle,
                         const void *txBuffer,
                         void *rxBuffer,
                         int count,
                         SPIDRV_Callback_t callback,
                         int timeoutMs)
{
  Ecode_t retVal;

  if (handle->initData.type == spidrvMaster) {
    return ECODE_EMDRV_SPIDRV_MODE_ERROR;
  }

  if ((retVal = TransferApiPrologue(handle, (void*)txBuffer, count))
      != ECODE_EMDRV_SPIDRV_OK) {
    return retVal;
  }

  if (rxBuffer == NULL) {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }

  if (timeoutMs) {
    sl_sleeptimer_start_timer_ms(&handle->timer, timeoutMs, SlaveTimeout, handle, 0, 0);
  }

  if (handle->initData.slaveStartMode == spidrvSlaveStartDelayed) {
    if ((retVal = WaitForIdleLine(handle)) != ECODE_EMDRV_SPIDRV_OK) {
      return retVal;
    }
  }

  StartTransferDMA(handle, txBuffer, rxBuffer, count, callback);

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Start an SPI slave blocking transfer.
 *
 * @note
 *    @n This function is blocking and returns when the transfer is complete,
 *    on timeout, or when @ref SPIDRV_AbortTransfer() is called.
 *
 * @param[in]  handle Pointer to an SPI driver handle.
 *
 * @param[in]  txBuffer Transmit data buffer.
 *
 * @param[out] rxBuffer Receive data buffer.
 *
 * @param[in]  count Number of bytes in transfer.
 *
 * @param[in]  timeoutMs Transfer timeout in milliseconds.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success, @ref ECODE_EMDRV_SPIDRV_TIMEOUT on
 *    timeout or @ref ECODE_EMDRV_SPIDRV_ABORTED if @ref SPIDRV_AbortTransfer()
 *    has been called. On failure, an appropriate SPIDRV @ref Ecode_t is
 *    returned.
 ******************************************************************************/
Ecode_t SPIDRV_STransferB(SPIDRV_Handle_t handle,
                          const void *txBuffer,
                          void *rxBuffer,
                          int count,
                          int timeoutMs)
{
  Ecode_t retVal;

  if (handle->initData.type == spidrvMaster) {
    return ECODE_EMDRV_SPIDRV_MODE_ERROR;
  }

  if ((retVal = TransferApiBlockingPrologue(handle, (void*)txBuffer, count))
      != ECODE_EMDRV_SPIDRV_OK) {
    return retVal;
  }

  if (rxBuffer == NULL) {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }

  if (timeoutMs) {
    sl_sleeptimer_start_timer_ms(&handle->timer, timeoutMs, SlaveTimeout, handle, 0, 0);
  }

  if (handle->initData.slaveStartMode == spidrvSlaveStartDelayed) {
    if ((retVal = WaitForIdleLine(handle)) != ECODE_EMDRV_SPIDRV_OK) {
      return retVal;
    }
  }

  StartTransferDMA(handle, txBuffer, rxBuffer, count, BlockingComplete);

  WaitForTransferCompletion(handle);

  return handle->transferStatus;
}

/***************************************************************************//**
 * @brief
 *    Start an SPI slave transmit transfer.
 *
 * @note
 *    The data received on the MOSI wire is discarded.
 *
 * @param[in]  handle Pointer to an SPI driver handle.
 *
 * @param[in]  buffer Transmit data buffer.
 *
 * @param[in]  count Number of bytes in transfer.
 *
 * @param[in]  callback Transfer completion callback.
 *
 * @param[in]  timeoutMs Transfer timeout in milliseconds.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success. On failure, an appropriate SPIDRV
 *    @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t SPIDRV_STransmit(SPIDRV_Handle_t handle,
                         const void *buffer,
                         int count,
                         SPIDRV_Callback_t callback,
                         int timeoutMs)
{
  Ecode_t retVal;

  if (handle->initData.type == spidrvMaster) {
    return ECODE_EMDRV_SPIDRV_MODE_ERROR;
  }

  if ((retVal = TransferApiPrologue(handle, (void*)buffer, count))
      != ECODE_EMDRV_SPIDRV_OK) {
    return retVal;
  }

  if (timeoutMs) {
    sl_sleeptimer_start_timer_ms(&handle->timer, timeoutMs, SlaveTimeout, handle, 0, 0);
  }

  if (handle->initData.slaveStartMode == spidrvSlaveStartDelayed) {
    if ((retVal = WaitForIdleLine(handle)) != ECODE_EMDRV_SPIDRV_OK) {
      return retVal;
    }
  }

  StartTransmitDMA(handle, buffer, count, callback);

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Start an SPI slave blocking transmit transfer.
 *
 * @note
 *    The data received on the MOSI wire is discarded.
 *    @n This function is blocking and returns when the transfer is complete,
 *    on timeout, or when @ref SPIDRV_AbortTransfer() is called.
 *
 * @param[in]  handle Pointer to an SPI driver handle.
 *
 * @param[in]  buffer Transmit data buffer.
 *
 * @param[in]  count Number of bytes in transfer.
 *
 * @param[in]  timeoutMs Transfer timeout in milliseconds.
 *
 * @return
 *    @ref ECODE_EMDRV_SPIDRV_OK on success, @ref ECODE_EMDRV_SPIDRV_TIMEOUT on
 *    timeout or @ref ECODE_EMDRV_SPIDRV_ABORTED if @ref SPIDRV_AbortTransfer()
 *    has been called. On failure, an appropriate SPIDRV @ref Ecode_t is
 *    returned.
 ******************************************************************************/
Ecode_t SPIDRV_STransmitB(SPIDRV_Handle_t handle,
                          const void *buffer,
                          int count,
                          int timeoutMs)
{
  Ecode_t retVal;

  if (handle->initData.type == spidrvMaster) {
    return ECODE_EMDRV_SPIDRV_MODE_ERROR;
  }

  if ((retVal = TransferApiBlockingPrologue(handle, (void*)buffer, count))
      != ECODE_EMDRV_SPIDRV_OK) {
    return retVal;
  }

  if (timeoutMs) {
    sl_sleeptimer_start_timer_ms(&handle->timer, timeoutMs, SlaveTimeout, handle, 0, 0);
  }

  if (handle->initData.slaveStartMode == spidrvSlaveStartDelayed) {
    if ((retVal = WaitForIdleLine(handle)) != ECODE_EMDRV_SPIDRV_OK) {
      return retVal;
    }
  }

  StartTransmitDMA(handle, buffer, count, BlockingComplete);

  WaitForTransferCompletion(handle);

  return handle->transferStatus;
}
#endif // defined(EMDRV_SPIDRV_INCLUDE_SLAVE)

/// @cond DO_NOT_INCLUDE_WITH_DOXYGEN

/***************************************************************************//**
 * @brief
 *    Transfer complete callback function used by blocking transfer API
 *    functions. Called by DMA interrupt handler, timer timeout handler
 *    or @ref SPIDRV_AbortTransfer() function.
 ******************************************************************************/
static void BlockingComplete(SPIDRV_Handle_t handle,
                             Ecode_t transferStatus,
                             int itemsTransferred)
{
  (void)itemsTransferred;

  handle->transferStatus    = transferStatus;
  handle->blockingCompleted = true;
}

/***************************************************************************//**
 * @brief Get SPI pins.
 ******************************************************************************/
static Ecode_t GetSpiPins(SPIDRV_Handle_t handle, SPI_Pins_t * pins)
{
  pins->mosiPort = handle->initData.portTx;
  pins->misoPort = handle->initData.portRx;
  pins->clkPort  = handle->initData.portClk;
  pins->csPort   = handle->initData.portCs;
  pins->mosiPin  = handle->initData.pinTx;
  pins->misoPin  = handle->initData.pinRx;
  pins->clkPin   = handle->initData.pinClk;
  pins->csPin    = handle->initData.pinCs;

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * @brief Configure/deconfigure SPI GPIO pins.
 ******************************************************************************/
static Ecode_t ConfigGPIO(SPIDRV_Handle_t handle, bool enable)
{
  SPI_Pins_t pins;
  Ecode_t ret;

  ret = GetSpiPins(handle, &pins);
  if (ret != ECODE_EMDRV_SPIDRV_OK) {
    return ret;
  }

  handle->portCs = pins.csPort;
  handle->pinCs  = pins.csPin;

  sl_gpio_t gpio_mosi;
  sl_gpio_t gpio_miso;
  sl_gpio_t gpio_clk;
  sl_gpio_t gpio_cs;
  gpio_mosi.port = pins.mosiPort;
  gpio_mosi.pin = pins.mosiPin;
  gpio_miso.port = pins.misoPort;
  gpio_miso.pin = pins.misoPin;
  gpio_clk.port = pins.clkPort;
  gpio_clk.pin = pins.clkPin;
  gpio_cs.port = handle->portCs;
  gpio_cs.pin = handle->pinCs;

  if (enable) {
    if (handle->initData.type == spidrvMaster) {
      sl_gpio_set_pin_mode(&gpio_mosi, SL_GPIO_MODE_PUSH_PULL, 0);
      sl_gpio_set_pin_mode(&gpio_miso, SL_GPIO_MODE_INPUT, 0);

      if ((handle->initData.clockMode == spidrvClockMode0)
          || (handle->initData.clockMode == spidrvClockMode1)) {
        sl_gpio_set_pin_mode(&gpio_clk, SL_GPIO_MODE_PUSH_PULL, 0);
      } else {
        sl_gpio_set_pin_mode(&gpio_clk, SL_GPIO_MODE_PUSH_PULL, 1);
      }
      if (handle->initData.csControl == spidrvCsControlAuto) {
        sl_gpio_set_pin_mode(&gpio_cs, SL_GPIO_MODE_PUSH_PULL, 1);
      }
    } else {
      sl_gpio_set_pin_mode(&gpio_mosi, SL_GPIO_MODE_INPUT, 0);
      sl_gpio_set_pin_mode(&gpio_miso, SL_GPIO_MODE_PUSH_PULL, 0);

      if ((handle->initData.clockMode == spidrvClockMode0)
          || (handle->initData.clockMode == spidrvClockMode1)) {
        sl_gpio_set_pin_mode(&gpio_clk, SL_GPIO_MODE_INPUT_PULL, 0);
      } else {
        sl_gpio_set_pin_mode(&gpio_clk, SL_GPIO_MODE_INPUT_PULL, 1);
      }

      if (handle->initData.csControl == spidrvCsControlAuto) {
        sl_gpio_set_pin_mode(&gpio_cs, SL_GPIO_MODE_INPUT_PULL, 1);
      }
    }
  } else {
    sl_gpio_set_pin_mode(&gpio_mosi, SL_GPIO_MODE_INPUT_PULL, 0);
    sl_gpio_set_pin_mode(&gpio_miso, SL_GPIO_MODE_INPUT_PULL, 0);

    if ((handle->initData.clockMode == spidrvClockMode0)
        || (handle->initData.clockMode == spidrvClockMode1)) {
      sl_gpio_set_pin_mode(&gpio_clk, SL_GPIO_MODE_INPUT_PULL, 0);
    } else {
      sl_gpio_set_pin_mode(&gpio_clk, SL_GPIO_MODE_INPUT_PULL, 1);
    }

    if (handle->initData.csControl == spidrvCsControlAuto) {
      sl_gpio_set_pin_mode(&gpio_cs, SL_GPIO_MODE_DISABLED, 0);
    }
  }

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * @brief DMA transfer completion callback. Called by the DMA interrupt handler.
 ******************************************************************************/
static bool RxDMAComplete(unsigned int channel,
                          unsigned int sequenceNo,
                          void *userParam)
{
  SPIDRV_Handle_t handle;
  (void)channel;
  (void)sequenceNo;

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  handle = (SPIDRV_Handle_t)userParam;

  handle->transferStatus = ECODE_EMDRV_SPIDRV_OK;
  handle->state          = spidrvStateIdle;
  handle->remaining      = 0;

#if defined(EMDRV_SPIDRV_INCLUDE_SLAVE)
  if (handle->initData.type == spidrvSlave) {
    sl_sleeptimer_stop_timer(&handle->timer);
  }
#endif

  if (handle->userCallback != NULL) {
    handle->userCallback(handle, ECODE_EMDRV_SPIDRV_OK, handle->transferCount);
  }

  CORE_EXIT_ATOMIC();
  em1RequestRemove(handle);

  return true;
}

#if defined(EMDRV_SPIDRV_INCLUDE_SLAVE)
/***************************************************************************//**
 * @brief Slave transfer timeout callback function.
 ******************************************************************************/
static void SlaveTimeout(sl_sleeptimer_timer_handle_t *sleepdriver_handle, void *user)
{
  bool active;
  bool pending;
  SPIDRV_Handle_t handle;
  (void)sleepdriver_handle;

  handle = (SPIDRV_Handle_t)user;

  if (handle->state == spidrvStateTransferring) {
    DMADRV_TransferActive(handle->rxDMACh, &active);
    if (active) {
      // Stop running DMAs
      DMADRV_StopTransfer(handle->rxDMACh);
      DMADRV_StopTransfer(handle->txDMACh);
      DMADRV_TransferRemainingCount(handle->rxDMACh, &handle->remaining);
    } else {
      // DMA is either completed or not yet started
      DMADRV_TransferCompletePending(handle->txDMACh, &pending);
      if (pending) {
        // A DMA interrupt is pending; let the DMA handler do the rest
        return;
      }
      handle->remaining = handle->transferCount;
    }
    handle->transferStatus = ECODE_EMDRV_SPIDRV_TIMEOUT;
    handle->state          = spidrvStateIdle;

    if (handle->userCallback != NULL) {
      handle->userCallback(handle,
                           ECODE_EMDRV_SPIDRV_TIMEOUT,
                           handle->transferCount - handle->remaining);
    }
  }
}
#endif

#if defined(EUSART_PRESENT)
/***************************************************************************//**
 * @brief Manually clear eusart Tx fifo.
 ******************************************************************************/
static void clearEusartFifos(EUSART_TypeDef *eusart)
{
  // EUSART_CMD_CLEARTX reportedly only affects UART, not SPI mode,
  // and there is no EUSART_CMD_CLEARRX. Only way to clear the
  // FIFOs is via the big hammer of disabling then reenabling it.
#if defined(_SILICON_LABS_32B_SERIES_2)
  EUSART_Enable(eusart, eusartDisable);
  EUSART_Enable(eusart, eusartEnable);
#else
  sl_hal_eusart_disable_rx(eusart);
  sl_hal_eusart_disable_tx(eusart);
  sl_hal_eusart_disable(eusart);
  while (eusart->EN & _EUSART_EN_DISABLING_MASK) ;

  sl_hal_eusart_enable(eusart);
  sl_hal_eusart_enable_rx(eusart);
  sl_hal_eusart_enable_tx(eusart);
  sl_hal_eusart_wait_sync(eusart, _EUSART_SYNCBUSY_MASK);
#endif
}
#endif

/***************************************************************************//**
 * @brief Start an SPI receive DMA.
 ******************************************************************************/
static void StartReceiveDMA(SPIDRV_Handle_t handle,
                            void *buffer,
                            int count,
                            SPIDRV_Callback_t callback)
{
  void *rxPort;
  void *txPort;
  DMADRV_DataSize_t size;

  handle->blockingCompleted  = false;
  handle->transferCount      = count;
  handle->userCallback       = callback;

  if (0) {
  }
#if defined(USART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeUsart) {
    handle->peripheral.usartPort->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;

    if (handle->initData.frameLength > 9) {
      rxPort = (void *)&(handle->peripheral.usartPort->RXDOUBLE);
      txPort = (void *)&(handle->peripheral.usartPort->TXDOUBLE);
    } else if (handle->initData.frameLength == 9) {
      rxPort = (void *)&(handle->peripheral.usartPort->RXDATAX);
      txPort = (void *)&(handle->peripheral.usartPort->TXDATAX);
    } else {
      rxPort = (void *)&(handle->peripheral.usartPort->RXDATA);
      txPort = (void *)&(handle->peripheral.usartPort->TXDATA);
    }
  }
#endif
#if defined(EUSART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeEusart) {
    clearEusartFifos(handle->peripheral.eusartPort);

    rxPort = (void *)&(handle->peripheral.eusartPort->RXDATA);
    txPort = (void *)&(handle->peripheral.eusartPort->TXDATA);
  }
#endif
  else {
    return;
  }

  if (handle->initData.frameLength > 8) {
    size = dmadrvDataSize2;
  } else {
    size = dmadrvDataSize1;
  }

  em1RequestAdd(handle);

  // Start receive DMA.
  DMADRV_PeripheralMemory(handle->rxDMACh,
                          handle->rxDMASignal,
                          buffer,
                          rxPort,
                          true,
                          count,
                          size,
                          RxDMAComplete,
                          handle);

  // Start transmit DMA.
  DMADRV_MemoryPeripheral(handle->txDMACh,
                          handle->txDMASignal,
                          txPort,
                          (void *)&(handle->initData.dummyTxValue),
                          false,
                          count,
                          size,
                          NULL,
                          NULL);
}

/***************************************************************************//**
 * @brief Start an SPI transmit/receive DMA.
 ******************************************************************************/
static void StartTransferDMA(SPIDRV_Handle_t handle,
                             const void *txBuffer,
                             void *rxBuffer,
                             int count,
                             SPIDRV_Callback_t callback)
{
  void *rxPort;
  void *txPort;
  DMADRV_DataSize_t size;

  handle->blockingCompleted  = false;
  handle->transferCount      = count;
  handle->userCallback       = callback;

  if (0) {
  }
#if defined(USART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeUsart) {
    handle->peripheral.usartPort->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;

    if (handle->initData.frameLength > 9) {
      rxPort = (void *)&(handle->peripheral.usartPort->RXDOUBLE);
      txPort = (void *)&(handle->peripheral.usartPort->TXDOUBLE);
    } else if (handle->initData.frameLength == 9) {
      rxPort = (void *)&(handle->peripheral.usartPort->RXDATAX);
      txPort = (void *)&(handle->peripheral.usartPort->TXDATAX);
    } else {
      rxPort = (void *)&(handle->peripheral.usartPort->RXDATA);
      txPort = (void *)&(handle->peripheral.usartPort->TXDATA);
    }
  }
#endif
#if defined(EUSART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeEusart) {
    clearEusartFifos(handle->peripheral.eusartPort);

    rxPort = (void *)&(handle->peripheral.eusartPort->RXDATA);
    txPort = (void *)&(handle->peripheral.eusartPort->TXDATA);
  }
#endif
  else {
    return;
  }

  if (handle->initData.frameLength > 8) {
    size = dmadrvDataSize2;
  } else {
    size = dmadrvDataSize1;
  }

  em1RequestAdd(handle);

  // Start receive DMA.
  DMADRV_PeripheralMemory(handle->rxDMACh,
                          handle->rxDMASignal,
                          rxBuffer,
                          rxPort,
                          true,
                          count,
                          size,
                          RxDMAComplete,
                          handle);

  // Start transmit DMA.
  DMADRV_MemoryPeripheral(handle->txDMACh,
                          handle->txDMASignal,
                          txPort,
                          (void*)txBuffer,
                          true,
                          count,
                          size,
                          NULL,
                          NULL);
}

/***************************************************************************//**
 * @brief Start an SPI transmit DMA.
 ******************************************************************************/
static void StartTransmitDMA(SPIDRV_Handle_t handle,
                             const void *buffer,
                             int count,
                             SPIDRV_Callback_t callback)
{
  void *rxPort;
  void *txPort;
  DMADRV_DataSize_t size;

  handle->blockingCompleted  = false;
  handle->transferCount      = count;
  handle->userCallback       = callback;

  if (0) {
  }
#if defined(USART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeUsart) {
    handle->peripheral.usartPort->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;

    if (handle->initData.frameLength > 9) {
      rxPort = (void *)&(handle->peripheral.usartPort->RXDOUBLE);
      txPort = (void *)&(handle->peripheral.usartPort->TXDOUBLE);
    } else if (handle->initData.frameLength == 9) {
      rxPort = (void *)&(handle->peripheral.usartPort->RXDATAX);
      txPort = (void *)&(handle->peripheral.usartPort->TXDATAX);
    } else {
      rxPort = (void *)&(handle->peripheral.usartPort->RXDATA);
      txPort = (void *)&(handle->peripheral.usartPort->TXDATA);
    }
  }
#endif
#if defined(EUSART_PRESENT)
  else if (handle->peripheralType == spidrvPeripheralTypeEusart) {
    clearEusartFifos(handle->peripheral.eusartPort);

    rxPort = (void *)&(handle->peripheral.eusartPort->RXDATA);
    txPort = (void *)&(handle->peripheral.eusartPort->TXDATA);
  }
#endif
  else {
    return;
  }

  if (handle->initData.frameLength > 8) {
    size = dmadrvDataSize2;
  } else {
    size = dmadrvDataSize1;
  }

  em1RequestAdd(handle);

  // Receive DMA runs only to get precise numbers for SPIDRV_GetTransferStatus()
  // Start receive DMA.
  DMADRV_PeripheralMemory(handle->rxDMACh,
                          handle->rxDMASignal,
                          &(handle->dummyRx),
                          rxPort,
                          false,
                          count,
                          size,
                          RxDMAComplete,
                          handle);

  // Start transmit DMA.
  DMADRV_MemoryPeripheral(handle->txDMACh,
                          handle->txDMASignal,
                          txPort,
                          (void*)buffer,
                          true,
                          count,
                          size,
                          NULL,
                          NULL);
}

/***************************************************************************//**
 * @brief Parameter checking function for blocking transfer API functions.
 ******************************************************************************/
static Ecode_t TransferApiBlockingPrologue(SPIDRV_Handle_t handle,
                                           void *buffer,
                                           int count)
{
  if (handle == NULL) {
    return ECODE_EMDRV_SPIDRV_ILLEGAL_HANDLE;
  }

  if ((buffer == NULL) || (count == 0) || (count > DMADRV_MAX_XFER_COUNT)) {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  if (handle->state != spidrvStateIdle) {
    CORE_EXIT_ATOMIC();
    return ECODE_EMDRV_SPIDRV_BUSY;
  }
  handle->state = spidrvStateTransferring;
  CORE_EXIT_ATOMIC();

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * @brief Parameter checking function for non-blocking transfer API functions.
 ******************************************************************************/
static Ecode_t TransferApiPrologue(SPIDRV_Handle_t handle,
                                   void *buffer,
                                   int count)
{
  if (handle == NULL) {
    return ECODE_EMDRV_SPIDRV_ILLEGAL_HANDLE;
  }

  if ((buffer == NULL) || (count == 0) || (count > DMADRV_MAX_XFER_COUNT)) {
    return ECODE_EMDRV_SPIDRV_PARAM_ERROR;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  if (handle->state != spidrvStateIdle) {
    CORE_EXIT_ATOMIC();
    return ECODE_EMDRV_SPIDRV_BUSY;
  }
  handle->state = spidrvStateTransferring;
  CORE_EXIT_ATOMIC();

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * @brief Wait for transfer completion.
 ******************************************************************************/
static void WaitForTransferCompletion(SPIDRV_Handle_t handle)
{
  if (sl_interrupt_manager_is_irq_blocked(SPI_DMA_IRQ)) {
    // Poll for completion by calling IRQ handler.
    while (handle->blockingCompleted == false) {
#if defined(DMA_PRESENT) && (DMA_COUNT == 1)
      DMA_IRQHandler();
#elif defined(LDMA_PRESENT) && (LDMA_COUNT == 1)
#if defined(_SILICON_LABS_32B_SERIES_2)
      LDMA_IRQHandler();
#else
      switch (handle->rxDMACh) {
        case 0:
          LDMA0_CHNL0_IRQHandler();
          break;
        case 1:
          LDMA0_CHNL1_IRQHandler();
          break;
        case 2:
          LDMA0_CHNL2_IRQHandler();
          break;
        case 3:
          LDMA0_CHNL3_IRQHandler();
          break;
        case 4:
          LDMA0_CHNL4_IRQHandler();
          break;
        case 5:
          LDMA0_CHNL5_IRQHandler();
          break;
        case 6:
          LDMA0_CHNL6_IRQHandler();
          break;
        case 7:
          LDMA0_CHNL7_IRQHandler();
          break;
        default:
          break;
      }
#endif
#else
#error "No valid SPIDRV DMA engine defined."
#endif
    }
  } else {
    while (handle->blockingCompleted == false) ;
  }
}

#if defined(EMDRV_SPIDRV_INCLUDE_SLAVE)
/***************************************************************************//**
 * @brief Wait for CS deassertion. Used by slave transfer API functions.
 ******************************************************************************/
static Ecode_t WaitForIdleLine(SPIDRV_Handle_t handle)
{
  sl_gpio_t gpio_cs;
  gpio_cs.port = handle->portCs;
  gpio_cs.pin = handle->pinCs;
  bool value;
  do {
    sl_gpio_get_pin_input(&gpio_cs, &value);
  } while (!value && (handle->state != spidrvStateIdle));

  if (handle->state == spidrvStateIdle) {
    return handle->transferStatus;
  }

  return ECODE_EMDRV_SPIDRV_OK;
}
#endif

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && defined(EUSART_PRESENT)
/***************************************************************************//**
 * @brief Enable EUSART and SPI IOs after deepsleep
 ******************************************************************************/
static Ecode_t sli_spidrv_exit_em23(SPIDRV_Handle_t handle)
{
  EUSART_TypeDef *eusart = handle->peripheral.eusartPort;
#if defined(_SILICON_LABS_32B_SERIES_2)
  EUSART_Enable(eusart, eusartEnable);
  BUS_RegMaskedWrite(&GPIO->EUSARTROUTE[EUSART_NUM(eusart)].ROUTEEN,
                     _GPIO_EUSART_ROUTEEN_TXPEN_MASK | _GPIO_EUSART_ROUTEEN_SCLKPEN_MASK,
                     GPIO_EUSART_ROUTEEN_TXPEN | GPIO_EUSART_ROUTEEN_SCLKPEN);
#else
  sl_hal_eusart_enable(eusart);
  sl_hal_eusart_enable_rx(eusart);
  sl_hal_eusart_enable_tx(eusart);
  sl_hal_eusart_wait_sync(eusart, _EUSART_SYNCBUSY_MASK);
  sl_hal_bus_reg_write_mask(&GPIO->EUSARTROUTE[EUSART_NUM(eusart)].ROUTEEN,
                            _GPIO_EUSART_ROUTEEN_TXPEN_MASK | _GPIO_EUSART_ROUTEEN_SCLKPEN_MASK,
                            GPIO_EUSART_ROUTEEN_TXPEN | GPIO_EUSART_ROUTEEN_SCLKPEN);
#endif

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * @brief Disable SPIO IOs before deepsleep
 ******************************************************************************/
static Ecode_t sli_spidrv_enter_em23(SPIDRV_Handle_t handle)
{
  EUSART_TypeDef *eusart = handle->peripheral.eusartPort;
#if defined(_SILICON_LABS_32B_SERIES_2)
  BUS_RegMaskedWrite(&GPIO->EUSARTROUTE[EUSART_NUM(eusart)].ROUTEEN,
                     _GPIO_EUSART_ROUTEEN_TXPEN_MASK | _GPIO_EUSART_ROUTEEN_SCLKPEN_MASK,
                     0);
#else
  sl_hal_bus_reg_write_mask(&GPIO->EUSARTROUTE[EUSART_NUM(eusart)].ROUTEEN,
                            _GPIO_EUSART_ROUTEEN_TXPEN_MASK | _GPIO_EUSART_ROUTEEN_SCLKPEN_MASK,
                            0);
#endif

  return ECODE_EMDRV_SPIDRV_OK;
}

/***************************************************************************//**
 * Power Manager callback notification for EUSART.
 * It is used to prepare EUSART module before/after deepsleeping.
 ******************************************************************************/
static void on_power_manager_event(sl_power_manager_em_t from,
                                   sl_power_manager_em_t to)
{
  (void)from;
  SPIDRV_Handle_t handle;

  if (to == SL_POWER_MANAGER_EM1
      || to == SL_POWER_MANAGER_EM0) {
    SL_SLIST_FOR_EACH_ENTRY(eusart_handle_list, handle, SPIDRV_HandleData_t, node) {
      sli_spidrv_exit_em23(handle);
    }
  }

  if (to == SL_POWER_MANAGER_EM2
      || to == SL_POWER_MANAGER_EM3) {
    SL_SLIST_FOR_EACH_ENTRY(eusart_handle_list, handle, SPIDRV_HandleData_t, node) {
      sli_spidrv_enter_em23(handle);
    }
  }
}
#endif

/// @endcond

/* *INDENT-OFF* */
/******** THE REST OF THE FILE IS DOCUMENTATION ONLY !**********************//**
 * @addtogroup spidrv SPIDRV - SPI Driver
 * @brief Serial Peripheral Interface Driver
 * @{

   @details
   The spidrv.c and spidrv.h source files for the SPI driver library are in the
   emdrv/spidrv folder.

   @n @section spidrv_intro Introduction
   The SPI driver supports the SPI capabilities of EFM32/EZR32/EFR32 USARTs.
   The driver is fully reentrant, supports several driver instances, and
   does not buffer or queue data. Both synchronous and asynchronous transfer
   functions are included for both master and slave SPI mode. Synchronous
   transfer functions are blocking and do
   not return before the transfer is complete. Asynchronous transfer
   functions report transfer completion with callback functions. Transfers are
   handled using DMA.

   @note Transfer completion callback functions are called from within the DMA
   interrupt handler with interrupts disabled.

   @note When doing USART synchronous transfers, fractional clock
   division of the reference clock is suppressed at @ref SPIDRV_SetBitrate.
   This restriction prevents timing violations causing framing errors and data corruption at high frequencies.

   @n @section spidrv_conf Configuration Options

   Some properties of the SPIDRV driver are compile-time configurable. These
   properties are stored in a file named spidrv_config.h. A template for
   this file, containing default values, is in the emdrv/config folder.
   Currently the configuration options are as follows:
   @li Inclusion of slave API transfer functions.

   To configure SPIDRV, provide a custom configuration file. This is a
   sample spidrv_config.h file:
   @verbatim
#ifndef __SILICON_LABS_SPIDRV_CONFIG_H__
#define __SILICON_LABS_SPIDRV_CONFIG_H__

// SPIDRV configuration option. Use this define to include the
// slave part of the SPIDRV API.
#define EMDRV_SPIDRV_INCLUDE_SLAVE

#endif
   @endverbatim

   The properties of each SPI driver instance are set at run-time using the
   @ref SPIDRV_Init data structure input parameter to the @ref SPIDRV_Init()
   function.

   @n @section spidrv_api The API

   This section contains brief descriptions of the API functions. For
   detailed information on input and output parameters and return values,
   click on the hyperlinked function names. Most functions return an error
   code, @ref ECODE_EMDRV_SPIDRV_OK is returned on success,
   see ecode.h and spidrv.h for other error codes.

   The application code must include @em spidrv.h.

   @ref SPIDRV_Init(), @ref SPIDRV_DeInit() @n
    These functions initialize or deinitializes the SPIDRV driver. Typically,
    SPIDRV_Init() is called once in the startup code.

   @ref SPIDRV_GetTransferStatus() @n
    Query the status of a transfer. Reports number of items (frames) transmitted
    and remaining.

   @ref SPIDRV_AbortTransfer() @n
    Stop an ongoing transfer.

   @ref SPIDRV_SetBitrate(), @ref SPIDRV_GetBitrate() @n
    Set or query the SPI bus bitrate.

   @ref SPIDRV_SetFramelength(), @ref SPIDRV_GetFramelength() @n
    Set or query SPI the bus frame length.

   SPIDRV_MReceive(), SPIDRV_MReceiveB() @n
   SPIDRV_MTransfer(), SPIDRV_MTransferB(), SPIDRV_MTransferSingleItemB() @n
   SPIDRV_MTransmit(), SPIDRV_MTransmitB() @n
   SPIDRV_SReceive(), SPIDRV_SReceiveB() @n
   SPIDRV_STransfer(), SPIDRV_STransferB() @n
   SPIDRV_STransmit(), SPIDRV_STransmitB() @n
    SPI transfer functions for SPI masters have an uppercase M in their name,
    the slave counterparts have an S.

    As previously mentioned, transfer functions are synchronous and asynchronous.
    The synchronous versions have an uppercase B (for Blocking) at the end of
    their function name.

    @em Transmit functions discard received data, @em receive functions transmit
    a fixed data pattern set when the driver is initialized
    (@ref SPIDRV_Init.dummyTxValue). @em Transfer functions both receive and
    transmit data.

    All slave transfer functions have a millisecond timeout parameter. Use 0
    for no (infinite) timeout.

   @n @section spidrv_example Example
   @verbatim
#include "spidrv.h"

SPIDRV_HandleData_t handleData;
SPIDRV_Handle_t handle = &handleData;

void TransferComplete(SPIDRV_Handle_t handle,
                      Ecode_t transferStatus,
                      int itemsTransferred)
{
  if (transferStatus == ECODE_EMDRV_SPIDRV_OK) {
   // Success !
  }
}

int main(void)
{
  uint8_t buffer[10];
  SPIDRV_Init_t initData = SPIDRV_MASTER_USART2;

  // Initialize an SPI driver instance.
  SPIDRV_Init(handle, &initData);

  // Transmit data using a blocking transmit function.
  SPIDRV_MTransmitB(handle, buffer, 10);

  // Transmit data using a callback to catch transfer completion.
  SPIDRV_MTransmit(handle, buffer, 10, TransferComplete);
}
   @endverbatim

 * @} end group spidrv ****************************************************/
