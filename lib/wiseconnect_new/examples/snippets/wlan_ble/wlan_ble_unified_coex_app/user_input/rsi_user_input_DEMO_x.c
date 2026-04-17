
#include <rsi_common_app.h>
#include <stdio.h>
#include <string.h>
#ifdef M4_UART
#include "USART.h"
#endif
#include "cmsis_os2.h"
#ifdef MXRT_595s
#include "sl_hal_mcu_uart.h"
extern sl_flexcomm_uart_cb_t g_sl_flexcomm_uart_cb[];
#endif
extern osSemaphoreId_t ui_task_sem;

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_RING_BUFFER_SIZE        100
#define MAX_NO_OF_SEGREGATE_COMMANDS 10
#ifdef M4_UART
#define BAUD_VALUE       115200
#define BUFFER_SIZE      100
#define BYTES_TO_BE_READ 2
typedef unsigned long TickType_t;
#define ULONG_MAX     4294967295UL
#define portMAX_DELAY (TickType_t) ULONG_MAX
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void prepare_cmd_args(uint8_t *uart_buff);

/*******************************************************************************
 * extern functions 
 ******************************************************************************/
extern void command_parser(uint8_t argc, uint8_t **argv);

/*******************************************************************************
 * Variables
 ******************************************************************************/
osSemaphoreId_t ui_task_response_semaphore;
#ifdef M4_UART
extern ARM_DRIVER_USART Driver_USART0;
static ARM_DRIVER_USART *USARTdrv = &Driver_USART0;

ARM_USART_CAPABILITIES drv_capabilities;

//ARM_USART_STATUS stats;
uint8_t rx_buffer[BUFFER_SIZE];
uint8_t stats;
extern osEventFlagsId_t ui_task_handle;
#endif
/*******************************************************************************
 * UART UI Task/Function
 ******************************************************************************/
#ifdef M4_UART
void ARM_USART_SignalEvent(uint32_t event);
void Read_Capabilities(void)
{
  drv_capabilities = USARTdrv->GetCapabilities();
}
void display(uint8_t *disp)
{
  uint8_t *rx_ptr = disp;
  printf("Entered Value: ");
  while (*(rx_ptr) != '\r')
    printf("%c", *(rx_ptr++));
  printf("\n");
}

void ARM_USART_SignalEvent(uint32_t event)
{
  // Mask to get only the usart events,
  // bits 30-31 gives the instance from which this event occurred
  event &= (USART_EVENT_MASK);

  volatile uint32_t read_rx_cnt = 0;
  volatile uint32_t read_tx_cnt = 0;
  //BaseType_t xHigherPriorityTaskWoken = 0;
  switch (event) {
    case ARM_USART_EVENT_SEND_COMPLETE:
      read_tx_cnt = USARTdrv->GetTxCount();
      break;
    case ARM_USART_EVENT_RECEIVE_COMPLETE:
      read_rx_cnt = USARTdrv->GetRxCount();
      //rsi_task_notify_from_isr(ui_task_handle, 0, 0, 0);
      osEventFlagsSet(ui_task_handle, event);
      break;
    case ARM_USART_EVENT_TRANSFER_COMPLETE:
      break;
    case ARM_USART_EVENT_TX_COMPLETE:
      break;
    case ARM_USART_EVENT_TX_UNDERFLOW:
      break;
    case ARM_USART_EVENT_RX_OVERFLOW:
      break;
    case ARM_USART_EVENT_RX_TIMEOUT:
      break;
    case ARM_USART_EVENT_RX_BREAK:
      break;
    case ARM_USART_EVENT_RX_FRAMING_ERROR:
      break;
    case ARM_USART_EVENT_RX_PARITY_ERROR:
      break;
    case ARM_USART_EVENT_CTS:
      break;
    case ARM_USART_EVENT_DSR:
      break;
    case ARM_USART_EVENT_DCD:
      break;
    case ARM_USART_EVENT_RI:
      break;
  }
}
void configure_m4_uart(void)
{
  // Configures the system default clock and power configurations
  SystemCoreClockUpdate();

  // Read capabilities of UART
  Read_Capabilities();

  // Initialized board UART
  DEBUGINIT();

  // Initialize UART(Enable Clock)
  stats = USARTdrv->Initialize(ARM_USART_SignalEvent);

  if (stats != ARM_DRIVER_OK) {
    printf("\r\n UART Initialization Failed, Error Code : %d\r\n", stats);
    return;
  } else {
    printf("\r\n UART Initialization Success\r\n");
  }

  // Power up the UART peripheral
  stats = USARTdrv->PowerControl(ARM_POWER_FULL);
  if (stats != ARM_DRIVER_OK) {
    printf("\r\n Failed to Set Power to UART, Error Code : %d\r\n", stats);
    return;
  } else {
    printf("\r\n Configured Power to UART \r\n");
  }
  // Enable Receiver and Transmitter lines
  stats = USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
  if (stats != ARM_DRIVER_OK) {
    printf("\r\n Failed to Set  Transmitter lines to UART, Error Code : %d\r\n", stats);
    return;
  } else {
    printf("\r\n Set  Transmitter lines to UART is success \r\n");
  }

  stats = USARTdrv->Control(ARM_USART_CONTROL_RX, 1);
  if (stats != ARM_DRIVER_OK) {
    printf("\r\n Failed to Set  Receiver lines to UART, Error Code : %d \r\n", stats);
    return;
  } else {
    printf("\r\n Set  Receiver lines to UART\r\n");
  }

  // Configure the UART to 9600 Bits/sec
  stats = USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 | ARM_USART_PARITY_NONE
                              | ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE,
                            BAUD_VALUE);
  if (stats != ARM_DRIVER_OK) {
    printf("\r\n Failed to set  UART control Configuration , Error Code : %d \r\n", stats);
    return;
  } else {
    printf("\r\n Successfully Configured UART control  \r\n");
  }
}
#endif
/*==============================================*/
/**
 * @fn          void rsi_ui_app_task(void)
 * @brief       function/task will handle uart commands
 * @param[in]   None
 * @return      None
 *
 * @section description
 * It will handle uart commands
 *
 */
void rsi_ui_app_task(void)
{
  //! creating a semaphore for handling command
  ui_task_response_semaphore = osSemaphoreNew(1, 0, NULL);
#ifdef MXRT_595s
  sl_flexcomm_uart_cb_t *sl_flexcomm_uart_cb = &g_sl_flexcomm_uart_cb[0];
#endif
  //! enable uart interrupt
  //rsi_hal_enable_uartn_irq(0);
  osSemaphoreAcquire(ui_task_sem, osWaitForever);
#ifdef M4_UART
  uint8_t *rx_ptr = rx_buffer;
  uint8_t length  = 0;
  uint32_t ulInterruptStatus;
  configure_m4_uart();
#endif
  while (1) {
#ifdef M4_UART
    // Receives command length
    memset(rx_buffer, 0, BUFFER_SIZE);
    rx_ptr = rx_buffer;
    stats  = USARTdrv->Receive(rx_ptr, BYTES_TO_BE_READ);
    if (stats != ARM_DRIVER_OK) {
      printf("\r\n Failed to Receive data , Error Code : %d \r\n", stats);
      return;
    } else {
      //DEBUGOUT("\r\n Receives data success  \r\n");
    }
#endif
    //! Wait for uart interrupt reception
#ifdef M4_UART
    osEventFlagsWait(ui_task_handle, ulInterruptStatus, (osFlagsWaitAny | osFlagsNoClear), osWaitForever);
#if 1
    rx_buffer[BYTES_TO_BE_READ] = '\0';
    rx_ptr                      = rx_buffer;
    length                      = ascii_to_dec(rx_ptr);
    memset(rx_buffer, 0, BYTES_TO_BE_READ);
    // Receives command based on the length
    stats = USARTdrv->Receive(rx_ptr, length);
    if (stats != ARM_DRIVER_OK) {
      printf("\r\n Failed to Receive data , Error Code : %d \r\n", stats);
      return;
    } else {
      //DEBUGOUT("\r\n Receives data success  \r\n");
    }
    osEventFlagsWait(ui_task_handle, ulInterruptStatus, (osFlagsWaitAny | osFlagsNoClear), osWaitForever);
#endif
#endif
#ifdef M4_UART
    //! segregate the commands and pass to command parser
    // prepare_cmd_args(rx_buffer);
    prepare_cmd_args(rx_buffer + 1);
    //! clear the uart buffer
#endif
#ifdef MXRT_595s
    //! segregate the commands and pass to command parser
    prepare_cmd_args(sl_flexcomm_uart_cb->demoRingBuffer);
    //! clear the uart buffer
    memset(sl_flexcomm_uart_cb->demoRingBuffer, 0, sizeof(sl_flexcomm_uart_cb->demoRingBuffer));
#endif
  }
}

/*******************************************************************************
 * Local Functions
 ******************************************************************************/
unsigned int is_delim(char c, const char *delim)
{
  while (*delim != '\0') {
    if (c == *delim)
      return 1;
    delim++;
  }
  return 0;
}
char *my_strtok(char *srcString, const char *delim)
{
  static char *backup_string; // start of the next search
  if (!srcString) {
    srcString = backup_string;
  }
  if (!srcString) {
    // user is bad user
    return NULL;
  }
  // handle beginning of the string containing delims
  while (1) {
    if (is_delim(*srcString, delim)) {
      srcString++;
      continue;
    }
    if (*srcString == '\0') {
      // we've reached the end of the string
      return NULL;
    }
    break;
  }
  char *ret = srcString;
  while (1) {
    if (*srcString == '\0') {
      /*end of the input string and
            next exec will return NULL*/
      backup_string = srcString;
      return ret;
    }
    if (is_delim(*srcString, delim)) {
      *srcString    = '\0';
      backup_string = srcString + 1;
      return ret;
    }
    srcString++;
  }
}
/*==============================================*/
/**
 * @fn          void prepare_cmd_args(uint8_t *uart_buff)
 * @brief       function will segregate each command from the uart frame
 * @param[in]   uint8_t, *uart_buff
 * @return      None
 *
 * @section description
 * function will segregate each command from the uart frame
 *
 */
void prepare_cmd_args(uint8_t *uart_buff)
{
  //display(uart_buff);
  uint8_t num_args = 0;
  uint8_t *strs_ptr[MAX_NO_OF_SEGREGATE_COMMANDS];

  //! delimiter "  "
  const char *delimiter_str = " ";
  const char *end_of_frame  = "\r";
  char *token;
  char *status_EOF;

  //!strtok is used to break given string into a series of tokens using the delimiter
  token = my_strtok((char *)uart_buff, delimiter_str);

  while (token != NULL) {
    //! assigning segregate commands address into strs_ptr(pointers to strings)
    strs_ptr[num_args] = (uint8_t *)token;
    //! counting no of args after strtok operation
    num_args++;
    //! finding for end of frame character
    status_EOF = strstr(token, end_of_frame);
    if (status_EOF != NULL) {
      memset(status_EOF, 0, strlen(end_of_frame));
      break;
    }

    //! traversing to the end of string
    token = my_strtok(NULL, delimiter_str);
  }

  //! calling command_parser func with params as no of args and strs_ptr(pointers to strings)
  command_parser(num_args, strs_ptr);
}
