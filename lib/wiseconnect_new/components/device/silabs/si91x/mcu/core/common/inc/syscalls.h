/*******************************************************************************
* @file  syscalls.h
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

#ifndef __SYS_CALLS_H__
#define __SYS_CALLS_H__

#ifdef __CC_ARM
#include <stdint.h>
#else
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/times.h>
#endif // __CC_ARM
#include "sl_common.h"

#ifdef __cplusplus
extern "C" {
#endif

int _kill(int pid, int sig);
void initialise_monitor_handles();
int _getpid(void);
void _exit(int status);
int _write(int file, const char *ptr, int len);
int _close(int file);
void *_sbrk(int incr);
int _isatty(int file);
int _lseek(int file, int ptr, int dir);
int _read(char *fmt_ptr, ...);
int _open(const char *path, int flags, ...);
int _wait(const int *status);
int _unlink(const char *name);
int _link(const char *old_link, const char *new_link);
int _fork(void);
int _execve(const char *name, char **argv, char **env);
int _stat(const char *file, struct stat *st);
int _times(const struct tms *buff);
int _fstat(int file, struct stat *st);
SL_WEAK void _putchar(char character);

#ifdef __cplusplus
}
#endif

#endif // __SYS_CALLS_H__
