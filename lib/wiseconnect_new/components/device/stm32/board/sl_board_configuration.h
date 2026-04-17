#ifndef RSI_BOARD_CONFIG_H
#define RSI_BOARD_CONFIG_H

#define SPI1_CLOCK_PIN_GPIO_Port     GPIOA
#define SPI1_CLOCK_PIN_Pin           GPIO_PIN_5

#define SPI1_MOSI_PIN_GPIO_Port      GPIOA
#define SPI1_MOSI_PIN_Pin            GPIO_PIN_7

#define SPI1_MISO_PIN_GPIO_Port      GPIOA
#define SPI1_MISO_PIN_Pin            GPIO_PIN_6

#define SPI1_CS_PIN_GPIO_Port        GPIOA
#define SPI1_CS_PIN_Pin              GPIO_PIN_4

#define RESET_PIN_GPIO_Port          GPIOC
#define RESET_PIN_Pin                GPIO_PIN_7

#define INTERRUPT_PIN_GPIO_Port      GPIOA
#define INTERRUPT_PIN_Pin            GPIO_PIN_9

#define WAKE_INDICATOR_PIN_GPIO_Port GPIOB
#define WAKE_INDICATOR_PIN_Pin       GPIO_PIN_5

#define SLEEP_CONFIRM_PIN_GPIO_Port  GPIOA
#define SLEEP_CONFIRM_PIN_Pin        GPIO_PIN_10

#endif