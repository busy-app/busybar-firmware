#pragma once

#include "sl_device_registers.h"
#include "sl_uart.h"
#include "sl_sdio_types.h"
#include "sl_ethernet_types.h"

// Use HFRCO directly with no DPLL
//#define PLATFORM_CLOCK_USE_HFRCO
#define PLATFORM_CLOCK_USE_HFXO
#define PLATFORM_CLOCK_USE_LFXO

#define PLATFORM_CPU_CLOCK             72000000UL
#define SL_CYCLE_COUNT_PER_MILLISECOND (PLATFORM_CPU_CLOCK / 1000)

#define PLATFORM_SDIO_USE_AUXHFRCO
#define PLATFORM_SDIO_CLOCK_DIVIDER (1)
//#define PLATFORM_SDIO_NEEDS_PULLUPS
#define PLATFORM_AUXHFRCO_CLOCK 16000000U

#define BSP_CLK_LFXO_PRESENT (1)
#define BSP_CLK_LFXO_INIT    CMU_LFXOINIT_DEFAULT
#define BSP_CLK_LFXO_CTUNE   (32U)
#define BSP_CLK_LFXO_FREQ    (32768U)

#define BSP_CLK_HFXO_PRESENT     (1)
#define BSP_CLK_HFXO_FREQ        (50000000UL)
#define BSP_CLK_HFXO_CTUNE       (342)
#define BSP_CLK_HFXO_INIT        CMU_HFXOINIT_DEFAULT
#define BSP_CLK_HFXO_CTUNE_TOKEN (0)

#define DEFAULT_UART USART4

#define SL_WIFI_RESET_PORT SL_GPIO_PORT_C
#define SL_WIFI_RESET_PIN  4
//
#define SL_WIFI_WAKE_PORT SL_GPIO_PORT_A
#define SL_WIFI_WAKE_PIN  12

#define SL_UART_VCOM_PORT SL_GPIO_PORT_E
#define SL_UART_VCOM_PIN  1
static const sl_gpio_t vcom_enable_pin = { SL_UART_VCOM_PORT, SL_UART_VCOM_PIN };

#define DEFAULT_UART_PIN_CONFIG &default_uart_pin_configuration

// XXX: HACK to get things working
#define UART_CLOCK  cmuClock_USART4
#define UART_RX_IRQ USART4_RX_IRQn

/* Note: This creates a static instance for each C file that includes this header and references the variable */
static const sl_uart_pin_configuration_t default_uart_pin_configuration = {
  .tx_port   = SL_GPIO_PORT_H,
  .tx_pin    = 4,
  .rx_port   = SL_GPIO_PORT_H,
  .rx_pin    = 5,
  .route_loc = USART_ROUTELOC0_RXLOC_LOC4 | USART_ROUTELOC0_TXLOC_LOC4,
};

static const sl_sdio_pin_configuration_t sdio_pin_configuration = {
  .clock_pin   = { SL_GPIO_PORT_E, 13u },
  .command_pin = { SL_GPIO_PORT_E, 12u },
  .d0_pin      = { SL_GPIO_PORT_E, 11u },
  .d1_pin      = { SL_GPIO_PORT_E, 10u },
  .d2_pin      = { SL_GPIO_PORT_E, 9u },
  .d3_pin      = { SL_GPIO_PORT_E, 8u },
};

static const sl_gpio_t button_1 = { SL_GPIO_PORT_C, 8 };
static const sl_gpio_t button_2 = { SL_GPIO_PORT_C, 9 };

static const sl_gpio_t led_1 = { SL_GPIO_PORT_H, 4 };
static const sl_gpio_t led_2 = { SL_GPIO_PORT_H, 5 };

static const sl_ethernet_pin_configuration_t ethernet_pin_configuration =
{
    .management =
    {
        .ETH_MDC  = {SL_GPIO_PORT_D, 14},
        .ETH_MDIO = {SL_GPIO_PORT_D, 13},
    },
    .rmii =
    {
        .ETH_RMIICRSDV  = {SL_GPIO_PORT_D, 11},
        .ETH_RMIIREFCLK = {SL_GPIO_PORT_D, 10},
        .ETH_RMIIRXD0   = {SL_GPIO_PORT_D,  9},
        .ETH_RMIIRXD1   = {SL_GPIO_PORT_F,  9},
        .ETH_RMIIRXER   = {SL_GPIO_PORT_D, 12},
        .ETH_RMIITXD0   = {SL_GPIO_PORT_F,  7},
        .ETH_RMIITXD1   = {SL_GPIO_PORT_F,  6},
        .ETH_RMIITXEN   = {SL_GPIO_PORT_F,  8},
    },
    .ETH_ROUTELOC0 = 0,
    .ETH_ROUTELOC1 = (1 << _ETH_ROUTELOC1_RMIILOC_SHIFT) | (1 << _ETH_ROUTELOC1_MDIOLOC_SHIFT),
    .ETH_ROUTEPEN  = ETH_ROUTEPEN_RMIIPEN | ETH_ROUTEPEN_MDIOPEN,
};

static const sl_gpio_t ethernet_enable_pin = { SL_GPIO_PORT_I, 10 };
static const sl_gpio_t ethernet_reset_pin  = { SL_GPIO_PORT_H, 7 };
static const sl_gpio_t ethernet_irq_pin    = { SL_GPIO_PORT_G, 15 };

#define SL_ETHERNET_ENABLE_PIN ethernet_enable_pin
#define SL_ETHERNET_RESET_PIN  ethernet_reset_pin
#define SL_ETHERNET_IRQ_PIN    ethernet_irq_pin

#define RS_WIFI_SPI_CLOCK_PORT SL_GPIO_PORT_E
#define RS_WIFI_SPI_CLOCK_PIN  12
#define RS_WIFI_SPI_CS_PORT    SL_GPIO_PORT_E
#define RS_WIFI_SPI_CS_PIN     13
#define RS_WIFI_SPI_MOSI_PORT  SL_GPIO_PORT_E
#define RS_WIFI_SPI_MOSI_PIN   10
#define RS_WIFI_SPI_MISO_PORT  SL_GPIO_PORT_E
#define RS_WIFI_SPI_MISO_PIN   11
