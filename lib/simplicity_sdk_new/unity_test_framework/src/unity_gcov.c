/***************************************************************************//**
 * # License
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is Third Party Software licensed by Silicon Labs from a third party
 * and is governed by the sections of the MSLA applicable to Third Party
 * Software and the additional terms set forth below.
 *
 ******************************************************************************/
/**
 * @file unity_gcov.c
 *
 *  Output gcov coverage information through Unity.
 */

#if defined(ENABLE_TEST_COVERAGE)

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <errno.h>

#include "unity.h"

#if !defined(COV_FILE_HANDLE)
#define COV_FILE_HANDLE          17
#endif

/**
 * Dummy implementation of _read to make gcov believe that there is no
 * pre-existing .gcda files
 */
int _read(int file, char *ptr, int len)
{
  (void)file;
  (void)ptr;
  (void)len;
  return 0;
}

/**
 * Retarget file IO to Unity output. Write as escaped binary data.
 */
int _write(int file, char *pch, int len)
{
  (void)file;
  int size = len;

  while (size > 0)
  {
    if (file == COV_FILE_HANDLE) {
      UNITY_OUTPUT_CHAR('\\');
      UNITY_OUTPUT_CHAR('0');
      UNITY_OUTPUT_CHAR('x');
      UnityPrintNumberHex((UNITY_UINT)*pch, 2);
    } else {
      UNITY_OUTPUT_CHAR(*pch);
    }
    pch++;
    size--;
  }

  return len;
}

/**
 * Retarget file IO to Unity output
 */
int _open(const char *ptr, int mode) {
  (void)mode;
  char *coverage_start_text = "{{coverage begin;";
  char *coverage_end_text = "}}";

  UnityPrintLen(coverage_start_text, strlen(coverage_start_text));
  UnityPrintLen(ptr, strlen(ptr));
  UnityPrintLen(coverage_end_text, strlen(coverage_end_text));
  
  return COV_FILE_HANDLE;
}

/**
 * Write end marker to Unity output when closing file
 */
int _close(int file) {
  (void)file;
  UnityPrint("{{coverage end}}");
  UNITY_OUTPUT_CHAR('\n');
  return 0;
}

int _fstat(int file, struct stat *st)
{
  (void)file;
  st->st_mode = S_IFCHR;
  return 0;
}

int _getpid(void)
{
  return 1;
}

int _isatty(int file)
{
  switch (file) {
  case STDOUT_FILENO:
  case STDERR_FILENO:
  case STDIN_FILENO:
    return 1;
  default:
    errno = EBADF;
    return 0;
  }
}

int _kill(int pid, int sig)
{
  (void)pid;
  (void)sig;
  errno = EINVAL;
  return (-1);
}

int _lseek(int file, int ptr, int dir)
{
  (void)file;
  (void)dir;
  return ptr;
}

/**
 * __gcov_flush was removed in GCC 12. Therefore, we will use __gcov_dump() to get coverage information.
 *
 * Note that it seems that GCC 12 toolchain have included the gcov library with empty hooks/implementation,
 * so not writing data files at all. So we will use another method to get the `.gcda` file information.
 *
 * https://github.com/zephyrproject-rtos/zephyr/tree/main/subsys/testsuite/coverage
 *
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#if defined(__GNUC__) && __GNUC__ == 12

typedef uint64_t gcov_type;

#define GCOV_COUNTERS               8U
#define GCOV_TAG_FUNCTION_LENGTH    12

#define GCOV_DATA_MAGIC             (0x67636461)
#define GCOV_TAG_FUNCTION           (0x01000000)
#define GCOV_TAG_COUNTER_BASE       (0x01a10000)
#define GCOV_TAG_FOR_COUNTER(count) (GCOV_TAG_COUNTER_BASE + ((uint32_t)(count) << 17))

/**
 * Information about counters for a single function
 */
struct gcov_ctr_info {
	unsigned int num;    /* number of counter values for this type */
	gcov_type *values;   /* array of counter values for this type */
};

/**
 * Profiling meta data per function
 */
struct gcov_fn_info {
	const struct gcov_info *key;     /* comdat key */
	unsigned int ident;              /* unique ident of function */
	unsigned int lineno_checksum;    /* function lineno_checksum */
	unsigned int cfg_checksum;       /* function cfg checksum */
	struct gcov_ctr_info ctrs[1];    /* instrumented counters */
};

/**
 * Profiling data per object file
 */
struct gcov_info {
	unsigned int version;                                     /* Gcov version (same as GCC version) */
	struct gcov_info *next;                                   /* List head for a singly-linked list */
	unsigned int stamp;                                       /* Uniquifying time stamp */
	unsigned int checksum;	                                  /* unique object checksum */
	const char *filename;                                     /* Name of the associated gcda data file */
	void (*merge[GCOV_COUNTERS])(gcov_type *, unsigned int);  /* merge functions, null for unused*/
	unsigned int n_functions;                                 /* number of instrumented functions */
	struct gcov_fn_info **functions;                          /* function information */
};

static struct gcov_info *gcov_info_head = NULL;

/**
 * Is called by gcc-generated constructor code for each object file compiled
 * with -fprofile-arcs.
 */
void __gcov_init(struct gcov_info *info)
{
	info->next = gcov_info_head;
	gcov_info_head = info;
}

void __gcov_merge_add(gcov_type *counters, unsigned int n_counters)
{
	/* Unused. */
  (void)counters;   
  (void)n_counters;
}

void __gcov_exit(void)
{
	/* Unused. */
}

/**
 * Store 64 bit data on a buffer
 */
static inline void write_u64(void *buffer, size_t *off, uint64_t v)
{
	memcpy((uint8_t *)buffer + *off, (uint8_t *)&v, sizeof(v));
	*off = *off + sizeof(uint64_t);
}

/**
 * Store 32 bit data on a buffer
 */
static inline void write_u32(void *buffer, size_t *off, uint32_t v)
{
	memcpy((uint8_t *)buffer + *off, (uint8_t *)&v, sizeof(v));
	*off = *off + sizeof(uint32_t);
}

size_t gcov_calculate_buff_size(struct gcov_info *info)
{
	uint32_t iter;
	uint32_t iter_1;
	uint32_t iter_2;
	uint32_t size = sizeof(uint32_t) * 4;

	for (iter = 0U; iter < info->n_functions; iter++) {
		/* space for TAG_FUNCTION and FUNCTION_LENGTH
		 * ident
		 * lineno_checksum
		 * cfg_checksum
		 */
		size += (sizeof(uint32_t) * 5);

		for (iter_1 = 0U; iter_1 < GCOV_COUNTERS; iter_1++) {
			if (!info->merge[iter_1]) {
				continue;
			}

			/*  for function counter and number of values  */
			size += (sizeof(uint32_t) * 2);

			for (iter_2 = 0U;
			     iter_2 < info->functions[iter]->ctrs->num;
			     iter_2++) {

				/* Iter for values which is uint64_t */
				size += (sizeof(uint64_t));
			}
		}
	}
	return size;
}

/**
 * Convert from gcov data set (info) to .gcda file format.
 */
void gcov_to_gcda(uint8_t *buffer, struct gcov_info *info)
{
	struct gcov_fn_info *functions;
	struct gcov_ctr_info *counters_per_func;
	uint32_t iter_functions;
	uint32_t iter_counts;
	uint32_t iter_counter_values;
	size_t buffer_write_position = 0;

	/* File header. */
	write_u32(buffer, &buffer_write_position, GCOV_DATA_MAGIC);
	write_u32(buffer, &buffer_write_position, info->version);
	write_u32(buffer, &buffer_write_position, info->stamp);
	write_u32(buffer, &buffer_write_position, info->checksum);

	for (iter_functions = 0U;
	     iter_functions < info->n_functions;
	     iter_functions++) {

		functions = info->functions[iter_functions];

		write_u32(buffer, &buffer_write_position, GCOV_TAG_FUNCTION);
		write_u32(buffer, &buffer_write_position, GCOV_TAG_FUNCTION_LENGTH);
		write_u32(buffer, &buffer_write_position, functions->ident);
		write_u32(buffer, &buffer_write_position, functions->lineno_checksum);
		write_u32(buffer, &buffer_write_position, functions->cfg_checksum);

		counters_per_func = functions->ctrs;

		for (iter_counts = 0U; iter_counts < GCOV_COUNTERS; iter_counts++) {
			if (!info->merge[iter_counts]) {
				continue;
			}

			write_u32(buffer, &buffer_write_position, GCOV_TAG_FOR_COUNTER(iter_counts));
			write_u32(buffer, &buffer_write_position, counters_per_func->num * 2U * 4);

			for (iter_counter_values = 0U;
			     iter_counter_values < counters_per_func->num;
			     iter_counter_values++) {

				write_u64(buffer, &buffer_write_position, counters_per_func->values[iter_counter_values]);
			}
			counters_per_func++;
		}
	}
}

/**
 * Retrieves gcov coverage data and sends it over the given interface.
 */
void gcov_coverage_dump(void)
{
	uint8_t *buffer;
	size_t size;
	struct gcov_info *gcov_list = gcov_info_head;

	while (gcov_list) {
		_open(gcov_list->filename, 0);

		size = gcov_calculate_buff_size(gcov_list);
		buffer = calloc(size, sizeof(uint8_t));
    
		gcov_to_gcda(buffer, gcov_list);
    	_write(COV_FILE_HANDLE, (char *)buffer, (int)size);
		free(buffer);

		gcov_list = gcov_list->next;
		if (gcov_list == NULL) {
			_close(0);
		}
	}
}


void __gcov_flush(void)
{
  gcov_coverage_dump();
}
#endif

#else
// An empty translation unit is disallowed; add a dummy extern int
extern int dummy;
#endif // ENABLE_TEST_COVERAGE
