/*******************************************************************************
 * @file  syscalls.c
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

/*
 *
 *    Atollic TrueSTUDIO Minimal System calls file
 *
 *    For more information about which c-functions
 *    need which of these lowlevel functions
 *    please consult the Newlib libc-manual
 *
 * */
#include <sys/stat.h>
#include <stdlib.h>
#include <stdarg.h>
#include "errno.h"
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include "syscalls.h"
#include "rsi_debug.h"
#include "sl_component_catalog.h"
#ifdef SL_CATALOG_SI91X_IOSTREAM_PRINTS_PRESENT
#include "sl_si91x_iostream_log_config.h"
#endif
#if defined SL_UART
#include "sl_uart.h"
#endif
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "cmsis_os2.h"
#endif
#define IO_MAXLINE 20U //maximun read length
typedef int (*PUTCHAR_FUNC)(int a);
char *stack_ptr __asm("sp");
extern char __HeapBase[];
extern char __HeapLimit[];
#ifdef DEBUG_SERIAL
extern void Serial_send(uint8_t ch);
extern char Serial_receive(void);
#endif // DEBUG_SERIAL

char *__env[1] = { 0 };
char **environ = __env;

/*! @brief Specification modifier flags for scanf. */
enum _debugconsole_scanf_flag {
  kSCANF_Suppress   = 0x2U,  /*!< Suppress Flag. */
  kSCANF_DestMask   = 0x7cU, /*!< Destination Mask. */
  kSCANF_DestChar   = 0x4U,  /*!< Destination Char Flag. */
  kSCANF_DestString = 0x8U,  /*!< Destination String FLag. */
  kSCANF_DestSet    = 0x10U, /*!< Destination Set Flag. */
  kSCANF_DestInt    = 0x20U, /*!< Destination Int Flag. */
  kSCANF_DestFloat  = 0x30U, /*!< Destination Float Flag. */
  kSCANF_LengthMask = 0x1f00U,
  /*!< Length Mask Flag. */    /*PRINTF_FLOAT_ENABLE */
  kSCANF_TypeSinged = 0x2000U, /*!< TypeSinged Flag. */
};

void initialise_monitor_handles()
{
  // This function is intentionally left empty.
  // It is required for setting up monitor handles in some environments,
  // but in this context, it does not need to perform any actions.
}

int _getpid(void)
{
  return 1;
}

int _kill(int pid, int sig)
{
  (void)pid;
  (void)sig;
  errno = EINVAL;
  return -1;
}

void _exit(int status)
{
  _kill(status, -1);
  while (1) {
    // Infinite loop to ensure the program hangs here after exit.
    // This is intentional to prevent further execution.
  }
}

#if !defined SL_UART
#ifdef SL_CATALOG_KERNEL_PRESENT
extern osMutexId_t si91x_prints_mutex;
#endif
int _write(int file, const char *ptr, int len)
{
  (void)file;
#ifdef SL_CATALOG_KERNEL_PRESENT
  if (osKernelGetState() == osKernelRunning) {
    osMutexAcquire(si91x_prints_mutex, osWaitForever);
  }
#endif
  for (int todo = 0; todo < len; todo++) {
#ifdef DEBUG_SERIAL
    Serial_send(*ptr++);

#else
#if SL_SI91X_IOSTREAM_LOG_PRINTS_ENABLE
    sl_iostream_write(SL_IOSTREAM_STDOUT, ptr, (size_t)len);
#elif DEBUG_UART
    Board_UARTPutChar(*ptr++);
#else
    (void)ptr;
#endif
#endif
  }
#ifdef SL_CATALOG_KERNEL_PRESENT
  if (osKernelGetState() == osKernelRunning) {
    osMutexRelease(si91x_prints_mutex);
  }
#endif
  return len;
}
#endif // SL_UART

int _close(int file)
{
  (void)file;
  return -1;
}

int _fstat(int file, struct stat *st)
{
  (void)file;
  st->st_mode = S_IFCHR;
  return 0;
}

int _isatty(int file)
{
  (void)file;
  return 1;
}

int _lseek(int file, int ptr, int dir)
{
  (void)file;
  (void)ptr;
  (void)dir;
  return 0;
}

static uint32_t read_exclude_space(const char **s)
{
  uint8_t count = 0;
  uint8_t c;

  c = **s;
  while ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') || (c == '\v') || (c == '\f')) {
    count++;
    (*s)++;
    c = **s;
  }
  return count;
}

static int scanf_data_format(const char *line_ptr, char *format, va_list args_ptr)
{
  uint8_t base;
  int8_t neg;
  /* Identifier for the format string. */
  char *c = format;
  char temp;
  char *buff;
  /* Flag telling the conversion specification. */
  uint32_t flag = 0;
  /* Filed width for the matching input streams. */
  uint32_t field_width;
  /* How many arguments are assigned except the suppress. */
  uint32_t nassigned = 0;
  /* How many characters are read from the input streams. */
  uint32_t n_decode = 0;

  int32_t val;

  const char *s;
  /* Identifier for the input string. */
  const char *p = line_ptr;

  /* Return EOF error before any conversion. */
  if (*p == '\0') {
    return -1;
  }

  /* Decode directives. */
  while ((*c) && (*p)) {
    /* Ignore all white-spaces in the format strings. */
    if (read_exclude_space((const char **)&c)) {
      n_decode += read_exclude_space(&p);
    } else if ((*c != '%') || ((*c == '%') && (*(c + 1) == '%'))) {
      /* Ordinary characters. */
      c++;
      if (*p == *c) {
        n_decode++;
        p++;
        c++;
      } else {
        /* Match failure. Misalignment with C99, the unmatched characters need to be pushed back to stream.
               * However, it is deserted now. */
        break;
      }
    } else {
      /* convernsion specification */
      c++;
      /* Reset. */
      flag        = 0;
      field_width = 0;
      base        = 0;

      /* Loop to get full conversion specification. */
      while ((*c) && (!(flag & kSCANF_DestMask))) {
        switch (*c) {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            if (field_width) {
              /* Match failure. */
              return (int)nassigned;
            }
            do {
              field_width = field_width * 10 + *c - '0';
              c++;
            } while ((*c >= '0') && (*c <= '9'));
            break;
          case 'd':
            base = 10;
            flag |= kSCANF_TypeSinged;
            flag |= kSCANF_DestInt;
            c++;
            break;
          case 'u':
            base = 10;
            flag |= kSCANF_DestInt;
            c++;
            break;
          case 'o':
            base = 8;
            flag |= kSCANF_DestInt;
            c++;
            break;
          case 'x':
          case 'X':
            base = 16;
            flag |= kSCANF_DestInt;
            c++;
            break;
          case 'i':
            base = 0;
            flag |= kSCANF_DestInt;
            c++;
            break;
          case 'c':
            flag |= kSCANF_DestChar;
            if (!field_width) {
              field_width = 1;
            }
            c++;
            break;
          case 's':
            flag |= kSCANF_DestString;
            c++;
            break;
          default:
            return (int)nassigned;
        }
      }

      if (!(flag & kSCANF_DestMask)) {
        /* Format strings are exhausted. */
        return (int)nassigned;
      }

      if (!field_width) {
        /* Large than length of a line. */
        field_width = 99;
      }

      /* Matching strings in input streams and assign to argument. */
      switch (flag & kSCANF_DestMask) {
        case kSCANF_DestChar:
          s    = p;
          buff = va_arg(args_ptr, char *);
          while ((field_width--) && (*p)) {
            if (!(flag & kSCANF_Suppress)) {
              *buff++ = *p++;
            } else {
              p++;
            }
            n_decode++;
          }

          if ((!(flag & kSCANF_Suppress)) && (s != p)) {
            nassigned++;
          }
          break;
        case kSCANF_DestString:
          n_decode += read_exclude_space(&p);
          s    = p;
          buff = va_arg(args_ptr, char *);
          while ((field_width--) && (*p != '\0') && (*p != ' ') && (*p != '\t') && (*p != '\n') && (*p != '\r')
                 && (*p != '\v') && (*p != '\f')) {
            if (flag & kSCANF_Suppress) {
              p++;
            } else {
              *buff++ = *p++;
            }
            n_decode++;
          }

          if ((!(flag & kSCANF_Suppress)) && (s != p)) {
            /* Add NULL to end of string. */
            *buff = '\0';
            nassigned++;
          }
          break;
        case kSCANF_DestInt:
          n_decode += read_exclude_space(&p);
          s   = p;
          val = 0;
          if ((base == 0 || base == 16) && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            base = 16;
            if (field_width >= 1) {
              p += 2;
              n_decode += 2;
              field_width -= 2;
            }
          }

          if (base == 0) {
            if (s[0] == '0') {
              base = 8;
            } else {
              base = 10;
            }
          }

          neg = 1;
          switch (*p) {
            case '-':
              neg = -1;
              n_decode++;
              p++;
              field_width--;
              break;
            case '+':
              neg = 1;
              n_decode++;
              p++;
              field_width--;
              break;
            default:
              break;
          }

          while ((*p) && field_width) {
            if ((*p <= '9') && (*p >= '0')) {
              temp = *p - '0';
            } else if ((*p <= 'f') && (*p >= 'a')) {
              temp = *p - 'a' + 10;
            } else if ((*p <= 'F') && (*p >= 'A')) {
              temp = *p - 'A' + 10;
            } else {
              temp = base;
            }

            if (temp >= base) {
              break;
            } else {
              val = base * val + temp;
            }
            p++;
            n_decode++;
            field_width--;
          }
          val *= neg;
          if (!(flag & kSCANF_Suppress)) {

            /* The default type is the type int. */
            if (flag & kSCANF_TypeSinged) {
              *va_arg(args_ptr, signed int *) = (signed int)val;
            } else {
              *va_arg(args_ptr, unsigned int *) = (unsigned int)val;
            }

            nassigned++;
          }
          break;

        default:
          return (int)nassigned;
      }
    }
  }
  return (int)nassigned;
}
int _read(char *fmt_ptr, ...)
{
  char temp_buf[IO_MAXLINE + 1];
  va_list ap;
  uint32_t i;
  char result;

  va_start(ap, fmt_ptr);
  temp_buf[0] = '\0';

  for (i = 0; i < (uint32_t)IO_MAXLINE; i++) {
#ifdef DEBUG_SERIAL
    result = Serial_receive();
#elif defined(SL_UART)
    sl_uart_rx_byte(NULL, (uint8_t *)&result);
#else
    result = (char)Board_UARTGetChar();
#endif
    temp_buf[i] = result;
    if ((result == '\r') || (result == '\n')) {
      /* End of Line. */
      break;
    }
  }

  if ((i == 0) || (i == IO_MAXLINE)) {
    temp_buf[i] = '\0';
  } else {
    temp_buf[i + 1] = '\0';
  }
  result = (char)scanf_data_format(temp_buf, fmt_ptr, ap);
  va_end(ap);

  return result;
}

int _open(const char *path, int flags, ...)
{
  (void)path;
  (void)flags;
  /* Pretend like we always fail */
  return -1;
}

int _wait(const int *status)
{
  (void)status;
  errno = ECHILD;
  return -1;
}

int _unlink(const char *name)
{
  (void)name;
  errno = ENOENT;
  return -1;
}

int _times(const struct tms *buff)
{
  (void)buff;
  return -1;
}

int _stat(const char *file, struct stat *st)
{
  (void)file;
  st->st_mode = S_IFCHR;
  return 0;
}

int _link(const char *old_link, const char *new_link)
{
  (void)old_link; //This statement is added only to resolve compilation warning, value is unchanged
  (void)new_link; //This statement is added only to resolve compilation warning, value is unchanged
  errno = EMLINK;
  return -1;
}

int _fork(void)
{
  errno = EAGAIN;
  return -1;
}

int _execve(const char *name, char **argv, char **env)
{
  (void)name;
  (void)argv;
  (void)env;
  errno = ENOMEM;
  return -1;
}

SL_WEAK void _putchar(char character)
{
  Board_UARTPutChar(character);
  return;
}
