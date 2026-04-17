/***************************************************************************//**
 * @file
 * @brief Memory Manager Driver API definition.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_MEMORY_MANAGER_H_
#define SLI_MEMORY_MANAGER_H_

#include <stdbool.h>

#include "em_device.h"
#include "sl_memory_manager.h"
#include "sl_memory_manager_config.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

// Memory Manager integration to SystemView is enabled on GCC builds of
// applications that include the SystemView component
#if defined(SL_CATALOG_SYSTEMVIEW_TRACE_PRESENT) && defined(__GNUC__)
#define SLI_MEMORY_MANAGER_ENABLE_SYSTEMVIEW
#endif

// Minimum block alignment in bytes. 8 bytes is the minimum alignment to account for largest CPU data type
// that can be used in some block allocation scenarios. 64-bit data type may be used to manipulate the
// allocated block. The ARM processor ABI defines data types and byte alignment, and 8-byte alignment
// can be seen for the largest data object type.
#define SLI_BLOCK_ALLOC_MIN_ALIGN   SL_MEMORY_BLOCK_ALIGN_8_BYTES

// Minimum block allocation size to avoid creating a block too small while splitting up an allocated block.
// Minimum size is formed from (metadata + payload) size. Size expressed in bytes.
#define SLI_BLOCK_ALLOCATION_MIN_SIZE   (SLI_BLOCK_METADATA_SIZE_BYTE + SL_MEMORY_MANAGER_BLOCK_ALLOCATION_MIN_SIZE)

// Minimum block reservation size to avoid creating a block too small while splitting up a reserved block.
// Contrary to block allocations, reservations don't have metadata.
#define SLI_BLOCK_RESERVATION_MIN_SIZE_BYTE   SL_MEMORY_MANAGER_BLOCK_ALLOCATION_MIN_SIZE
#define SLI_BLOCK_RESERVATION_MIN_SIZE_DWORD  SLI_BLOCK_LEN_BYTE_TO_DWORD(SL_MEMORY_MANAGER_BLOCK_ALLOCATION_MIN_SIZE)

// 64-bit word size (in octets).
#define SLI_WORD_SIZE_64                8u
// 32-bit word size (in octets).
#define SLI_WORD_SIZE_32                4u
#define SLI_DEF_INT_32_NBR_BITS         32u

// 1-byte size (in bits).
#define SLI_DEF_INT_08_NBR_BITS         8u

// Size of block metadata area in different units.
#define SLI_BLOCK_METADATA_SIZE_BYTE    sizeof(sli_block_metadata_t)
#define SLI_BLOCK_METADATA_SIZE_DWORD   SLI_BLOCK_LEN_BYTE_TO_DWORD(SLI_BLOCK_METADATA_SIZE_BYTE)

// Size of reservation handle area in different units.
#define SLI_RESERVATION_HANDLE_SIZE_BYTE    sizeof(sl_memory_reservation_t)
#define SLI_RESERVATION_HANDLE_SIZE_DWORD   SLI_BLOCK_LEN_BYTE_TO_DWORD(SLI_RESERVATION_HANDLE_SIZE_BYTE)

// Size of pool handle area in different units.
#define SLI_POOL_HANDLE_SIZE_BYTE    sizeof(sl_memory_pool_t)
#define SLI_POOL_HANDLE_SIZE_DWORD   SLI_BLOCK_LEN_BYTE_TO_DWORD(SLI_POOL_HANDLE_SIZE_BYTE)

// Block size limit to determine large blocks (limit is 512KB).
// 512KB = 512 * 1024 bytes = 524288 bytes is used as the limit to determine large block.
#define SLI_LARGE_BLOCK_SIZE_LIMIT_BYTE     524288u

// Internal define for large block support if the device has a large DMEM memory (at least 512KB).
#if (defined(DMEM_MEM_SIZE) && (DMEM_MEM_SIZE > SLI_LARGE_BLOCK_SIZE_LIMIT_BYTE)) \
  || (defined(HOSTDMEM_MEM_SIZE) && (HOSTDMEM_MEM_SIZE > SLI_LARGE_BLOCK_SIZE_LIMIT_BYTE))
#define SLI_LARGE_BLOCK_SUPPORT
#endif

#if ((defined(SL_CATALOG_MEMORY_MANAGER_DTCM_PRESENT) || defined(SL_CATALOG_MEMORY_MANAGER_PSRAM_PRESENT)) \
  && (defined(SL_MEMORY_MANAGER_HEAP_FALLBACK_EN) && (SL_MEMORY_MANAGER_HEAP_FALLBACK_EN == 1)))
#define SLI_MEMORY_MANAGER_SUPPORT_ALLOCATION_FALLBACK
#endif

// Masks for extracting block type and attributes from allocation parameters
#define SLI_MEMORY_BLOCK_TYPE_MASK         0x00000001U
#define SLI_MEMORY_BLOCK_ATTRIBUTE_MASK    0xFFFFFFFEU

// Size of pool block metadata.
#define SLI_MEMORY_POOL_BLOCK_METADATA_SIZE_BYTE   sizeof(sli_memory_pool_block_t)

#ifdef SLI_MEMORY_MANAGER_ENABLE_TEST_UTILITIES
#define SLI_MAX_RESERVATION_COUNT 32
#endif

#if !defined(_SILICON_LABS_32B_SERIES_2)             \
  && !defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301) \
  && !defined(SL_RAM_LINKER)
// Internal define to indicate that the memory manager stack is in the heap.
#define SLI_MEMORY_MANAGER_STACK_IN_HEAP 1
#endif

/*******************************************************************************
 **********************************   MACROS   *********************************
 ******************************************************************************/

// Macros to align a value to the nearest value multiple of the specified alignment
// (rounded up or down). These macros are used for memory addresses requiring an alignment.
#define SLI_ALIGN_ROUND_UP(num, align) (((num) + ((align) - 1)) & ~((align) - 1))
#define SLI_ALIGN_ROUND_DOWN(num, align) ((num) & ~((align) - 1))

// Macros to convert block length in different units (bytes, double words).
// Byte to word will round up to account for extra bytes.
#define SLI_BLOCK_LEN_DWORD_TO_BYTE(len) ((len) * SLI_WORD_SIZE_64)
#define SLI_BLOCK_LEN_BYTE_TO_DWORD(len) ((len + SLI_WORD_SIZE_64 - 1) / SLI_WORD_SIZE_64)

// Macro to test address given a specified data alignment.
#define SLI_ADDR_IS_ALIGNED(ptr, align_byte)   (((uintptr_t)(const void *)(ptr)) % (align_byte) == 0)

// Macro to convert from bits to byte.
#define SLI_POOL_BITS_TO_BYTE(bits) (((bits) + 7u) / SLI_DEF_INT_08_NBR_BITS)

#if defined(SL_CATALOG_BANK_RETENTION_CONTROL_PRESENT)
#define INCREMENT_BANK_COUNTER(heap, start_addr, end_addr) sli_memory_manager_increment_bank_counter(heap,                                                     \
                                                                                                     sli_memory_manager_get_bank_id_by_addr(heap, start_addr), \
                                                                                                     sli_memory_manager_get_bank_id_by_addr(heap, end_addr))

#define DECREMENT_BANK_COUNTER(heap, start_addr, end_addr) sli_memory_manager_decrement_bank_counter(heap,                                                     \
                                                                                                     sli_memory_manager_get_bank_id_by_addr(heap, start_addr), \
                                                                                                     sli_memory_manager_get_bank_id_by_addr(heap, end_addr))
#else
#define INCREMENT_BANK_COUNTER(heap, start_addr, end_addr) (void)heap
#define DECREMENT_BANK_COUNTER(heap, start_addr, end_addr) (void)heap
#endif

#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)
#define SLI_MEMORY_STAT_HEAP_INCREASE(_heap, _inc)          \
  do {                                                      \
    (_heap)->used_size += (size_t)(_inc);                   \
    if ((_heap)->used_size > (_heap)->high_watermark) {     \
      (_heap)->high_watermark = (_heap)->used_size;         \
    }                                                       \
  } while (0)

#define SLI_MEMORY_STAT_HEAP_DECREASE(_heap, _dec)          \
  do {                                                      \
    (_heap)->used_size -= (size_t)(_dec);                   \
  } while (0)
#else
#define SLI_MEMORY_STAT_HEAP_INCREASE(_heap, _inc) do { (void)(_heap); (void)(_inc); } while (0)
#define SLI_MEMORY_STAT_HEAP_DECREASE(_heap, _dec) do { (void)(_heap); (void)(_dec); } while (0)
#endif

/*******************************************************************************
 *********************************   TYPEDEF   *********************************
 ******************************************************************************/

// Block metadata containing information about allocated block.
// This metadata allows to implement explicit free blocks list.
// NOTE: The metadata size should ideally be a multiple of 8 bytes (see description of
// SLI_BLOCK_ALLOC_MIN_ALIGN for other details) or at least multiple of CPU data size
// (e.g. 4 bytes for 32-bit CPU).
// 'length' is expressed in double words unit. It can described a block up to 512 KB (65535 * 8 bytes).
// When large block (over 512 KB) supported, new fields are used 'xxxx_msb' to encode the full block
// length, offset previous and offset next, still in double words unit. The large block support can
// describe blocks up to 8MB using a 20-bit encoding ((4 + 16 bits) so 1,048,575 * 8 bytes = 8,388,600 bytes)
typedef struct {
  uint16_t block_in_use : 1;              // Flag indicating if block allocated or not.
  uint16_t heap_start_align : 1;          // Flag indicating if first block at heap start undergone a data payload adjustment.
  uint16_t block_type : 1;                // Block type (LT or ST). Used only with SLI_MEMORY_MANAGER_ENABLE_SYSTEMVIEW.
  uint16_t reserved : 1;                  // Unallocated for future usage.
  uint16_t length_msb : 4;                // MSBs of field "length" for blocks larger than 512 KB.
  uint16_t offset_neighbour_prev_msb : 4; // MSBs of field "offset_neighbour_prev" for offsets larger than 512 KB.
  uint16_t offset_neighbour_next_msb : 4; // MSBs of field "offset_neighbour_next" for offsets larger than 512 KB.
  uint16_t length;                        // Block size (metadata not included just data payload), in double words (64 bit).
  uint16_t offset_neighbour_prev;         // Offset to previous neighbor, in double words. It includes metadata/payload sizes.
  uint16_t offset_neighbour_next;         // Offset to next neighbor, in double words.
} sli_block_metadata_t;

/// @brief Pool free count list structure.
struct sli_memory_pool_free_cnt_entry {
  uint16_t free_cnt;                      ///< The number of free blocks available in this free count entry.
  sli_memory_pool_free_cnt_entry_t *prev; ///< Pointer to the previous free count entry in the memory pool.
  sli_memory_pool_free_cnt_entry_t *next; ///< Pointer to the next entry in the free count list.
  sli_memory_pool_bank_t *bank_head;      ///< Pointer to the first bank in the list that has free_cnt free blocks.
};

/// @brief Pool bank list structure.
struct sli_memory_pool_bank {
  sli_memory_pool_bank_t *prev;                     ///< Pointer to the previous memory pool bank in the list.
  sli_memory_pool_bank_t *next;                     ///< Pointer to the next memory pool bank.
  sli_memory_pool_free_cnt_entry_t *free_cnt_entry; ///< Pointer to the entry in the free count list.
  sli_memory_pool_block_t *block_head;              ///< Pointer to the head of the free block list.
};

/// @brief Pool block list structure.
struct sli_memory_pool_block {
  void *block_addr;              ///< Represents the address of a pool block aligned to 2 bytes.
  sli_memory_pool_block_t *next; ///< Pointer to the next block in the memory pool's block list.
};

/*******************************************************************************
 ****************************   GLOBAL VARIABLES   *****************************
 ******************************************************************************/

extern sl_memory_heap_t sli_general_purpose_heap SL_FAST_DATA;

#if defined(SL_CATALOG_MEMORY_MANAGER_DTCM_PRESENT)
extern sl_memory_heap_t sli_dtcm_heap SL_FAST_DATA;
#endif

#if defined(SL_CATALOG_MEMORY_MANAGER_PSRAM_PRESENT)
extern sl_memory_heap_t sli_psram_heap SL_FAST_DATA;
#endif

#if defined(DEBUG_EFM) || defined(DEBUG_EFM_USER)
extern bool reserve_no_retention_first;
extern uint32_t reserve_no_retention_size;
#endif

#if defined(SLI_MEMORY_MANAGER_ENABLE_TEST_UTILITIES)
extern sl_memory_reservation_t* sli_reservation_handle_ptr_table[];
extern uint32_t sli_reservation_alignment_table[];

extern sl_memory_reservation_t sli_reservation_no_retention_table[];
#endif

// The heap name is also used as the Memory Profiler tracker handle for the heap
// pool managed by the Memory Manager
extern const char sli_mm_heap_name[];
extern const char sli_mm_heap_reservation_name[];

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * Initializes a memory block metadata to some reset values.
 *
 * @param[in] block_metadata  Pointer to block metadata.
 ******************************************************************************/
void sli_memory_metadata_init(sli_block_metadata_t *block_metadata);

/***************************************************************************//**
 * Gets pointer to the first free block of adequate size.
 *
 * @param[in]  heap               Heap handle.
 * @param[in]  size               Size of the block, in bytes.
 * @param[in]  align              Required alignment for the block, in bytes.
 * @param[in]  type               Type of block (long-term or short term).
 *                                  BLOCK_TYPE_LONG_TERM
 *                                  BLOCK_TYPE_SHORT_TERM
 * @param[in]  block_reservation  Indicates if the free block is for a dynamic
 *                                reservation.
 * @param[out] block              Pointer to variable that will receive the
 *                                start address of the free block.
 *
 * @return    Size of the block adjusted with the alignment.
 ******************************************************************************/
size_t sli_memory_find_free_block(sl_memory_heap_t *heap,
                                  size_t size,
                                  size_t align,
                                  sl_memory_block_type_t type,
                                  bool block_reservation,
                                  sli_block_metadata_t **block);

/***************************************************************************//**
 * Finds the next free block that will become the long-term or short-term head
 * pointer in a specific heap instance.
 *
 * @param[in]  heap               Heap handle.
 * @param[in]  type               Type of block (long-term or short term).
 *                                  BLOCK_TYPE_LONG_TERM
 *                                  BLOCK_TYPE_SHORT_TERM
 * @param[in]  block_start_from   Pointer to block where to start searching.
 *                                NULL pointer means start from one of heap
 *                                ends according to the block type.
 *
 * @return     Pointer to the new free block.
 ******************************************************************************/
sli_block_metadata_t *sli_memory_find_head_free_block(sl_memory_heap_t *heap,
                                                      sl_memory_block_type_t type,
                                                      sli_block_metadata_t *block_start_from);

/***************************************************************************//**
 * Gets long-term head pointer to the first free block.
 *
 * @return    Pointer to first free long-term block.
 ******************************************************************************/
void *sli_memory_get_longterm_head_ptr(void);

/***************************************************************************//**
 * Gets short-term head pointer to the first free block.
 *
 * @return    Pointer to first free short-term block.
 ******************************************************************************/
void *sli_memory_get_shortterm_head_ptr(void);

/***************************************************************************//**
 * Update free lists heads (short and long terms) for a specific heap instance.
 *
 * @param[in]  heap             Heap handle.
 * @param[in]  free_head        Block from where to start searching or next free block.
 * @param[in]  condition_block  Block condition to check if update is necessary
 *                              or not.
 * @param[in]  search           Boolean condition to check if searching the heap
 *                              for a free block is necessary.
 ******************************************************************************/
void sli_update_free_list_heads(sl_memory_heap_t *heap,
                                sli_block_metadata_t *free_head,
                                const sli_block_metadata_t *condition_block,
                                bool search);

/***************************************************************************//**
 * Creates a new heap instance.
 *
 * @param[in]  base_addr  Base address of the heap memory.
 * @param[in]  size       Size of the heap memory, in bytes.
 * @param[in]  attrib     Heap attributes
 *                          SL_MEMORY_HEAP_ALLOC_CPU_RAM
 *                          SL_MEMORY_HEAP_ALLOC_EXTERNAL_RAM
 * @param[out] heap       Pointer to the initialized heap handle.
 *
 * @return  SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
sl_status_t sli_memory_create_heap(void *base_addr,
                                   size_t size,
                                   sl_memory_block_attrib_t attrib,
                                   sl_memory_heap_t *heap);

/***************************************************************************//**
 * Get the heap in which a block is allocated.
 *
 * @param[in] block  A block's address.
 *
 * @return     Pointer to the block's Heap Handle.
 ******************************************************************************/
sl_memory_heap_t *sli_memory_get_heap_handle(const void *block);

/***************************************************************************//**
 * Creates the Stack at the end of the Heap.
 ******************************************************************************/
void sli_memory_create_stack(void);

#if defined(SLI_MEMORY_MANAGER_ENABLE_TEST_UTILITIES)
/***************************************************************************//**
 * Get an index of sli_reservation_handle_ptr_table that is free.
 *
 * @return    Index of an empty entry in sli_reservation_handle_ptr_table.
 ******************************************************************************/
uint32_t sli_get_available_reservation_handle_ix(void);

/***************************************************************************//**
 * Gets the pointer to sl_memory_reservation_t{} by block address.
 *
 * @param[in]  addr  Pointer to the block reservation.
 *
 * @return    Pointer to reservation handle.
 ******************************************************************************/
sl_memory_reservation_t *sli_memory_get_reservation_handle_by_addr(void *addr);

/***************************************************************************//**
 * Gets the size of a reservation by block address.
 *
 * @param[in]  addr  Pointer to the block reservation.
 *
 * @return    Size of the reservation in bytes.
 ******************************************************************************/
uint32_t sli_memory_get_reservation_size_by_addr(void *addr);

/***************************************************************************//**
 * Get the alignment of a reservation by block address.
 *
 * @param[in]  addr  Pointer to the block reservation.
 *
 * @return    Alignment of the reservation in bytes.
 ******************************************************************************/
uint32_t sli_memory_get_reservation_align_by_addr(void *addr);

/***************************************************************************//**
 * Bookkeeps a reservation for profiling purposes.
 *
 * @param[in]  reservation_handle_ptr  Pointer to the reservation handle.
 *
 * @return    SL_STATUS_FULL if record is full.
 ******************************************************************************/
sl_status_t sli_memory_save_reservation_handle(sl_memory_reservation_t *reservation_handle_ptr);

/***************************************************************************//**
 * Removes a reservation from records.
 *
 * @param[in]  reservation_handle_ptr  Pointer to the reservation handle.
 *
 * @return    SL_STATUS_NOT_FOUND if reservation is does not exist in records.
 ******************************************************************************/
sl_status_t sli_memory_remove_reservation_handle(sl_memory_reservation_t *reservation_handle_ptr);

/***************************************************************************//**
 * Bookkeeps a reservation (no retention) for profiling purposes.
 *
 * @param[in]  block_address  Pointer to the block reservation.
 * @param[in]  block_size     Size of the reservation.
 *
 * @return    SL_STATUS_NOT_FOUND if reservation is does not exist in records.
 ******************************************************************************/
sl_status_t sli_memory_save_reservation_no_retention(void *block_address,
                                                     uint32_t block_size);

/***************************************************************************//**
 * Gets the size of a reservation (no retention) by block address.
 *
 * @param[in]  addr  Pointer to the block reservation.
 *
 * @return    Size of the reservation (no retention) in bytes.
 ******************************************************************************/
uint32_t sli_memory_get_reservation_no_retention_size(void *addr);

/***************************************************************************//**
 * Does a heap integrity check forwards from the general-purpose heap free_lt_list_head
 * and return the pointer to the corrupted sli_block_metadata_t{} (if applicable).
 * This could go past reservations so there are checks.
 *
 * @return    Pointer to the corrupted sli_block_metadata_t{}.
 ******************************************************************************/
sli_block_metadata_t *sli_memory_check_general_purpose_heap_integrity_forwards(void);

/***************************************************************************//**
 * Does a heap integrity check forwards from free_lt_list_head and return
 * the pointer to the corrupted sli_block_metadata_t{} (if applicable).
 * This could go past reservations so there are checks.
 *
 * @return    Pointer to the corrupted sli_block_metadata_t{}.
 ******************************************************************************/
sli_block_metadata_t *sli_memory_check_heap_integrity_forwards(sl_memory_heap_t *heap);

/***************************************************************************//**
 * Does a heap integrity check backwards from the general-purpose heap free_st_list_head
 * and return the pointer to the corrupted sli_block_metadata_t{} (if applicable).
 * This should not go past any reservations, hence there are no checks.
 *
 * @return    Pointer to the corrupted sli_block_metadata_t{}.
 ******************************************************************************/
sli_block_metadata_t *sli_memory_check_general_purpose_heap_integrity_backwards(void);

/***************************************************************************//**
 * Does a heap integrity check backwards from free_st_list_head and return
 * the pointer to the corrupted sli_block_metadata_t{} (if applicable).
 * This should not go past any reservations, hence there are no checks.
 *
 * @return    Pointer to the corrupted sli_block_metadata_t{}.
 ******************************************************************************/
sli_block_metadata_t *sli_memory_check_heap_integrity_backwards(sl_memory_heap_t *heap);

/***************************************************************************//**
 * Go through the pool metadata lists and check some facts. For example check that
 * the free count entries are correct and that the total block count is correct.
 * Also check that the free block count is correct.
 *
 * @param[in] pool_handle   Pointer to a pool handle.
 * @param[in] allocated_count   The block count of already allocated blocks.
 *
 * @return true if the heap integrity check is successful. False otherwise.
 ******************************************************************************/
bool sli_check_pool_integrity(sl_memory_pool_t *pool_handle,
                              uint32_t allocated_count);
#endif /* SLI_MEMORY_MANAGER_ENABLE_TEST_UTILITIES */

// Inline functions to encode and decode different fields of sli_block_metadata_t{} when the block size
// is larger than 512 KB.
// - Full block length = .length_msb + .length = 20 bits to represent large length
// - Full block offset previous = .offset_neighbour_prev_msb + .offset_neighbour_prev = 20 bits to represent large offset
// - Full block offset next = .offset_neighbour_prev_msb + .offset_neighbour_prev = 20 bits to represent large offset
/***************************************************************************//**
 * Encodes the length of a memory block in its metadata.
 *
 * @param[in,out] meta Pointer to memory block metadata.
 * @param[in]     len  Length of the memory block in double words.
 ******************************************************************************/
__INLINE void sli_block_len_dword_encode(sli_block_metadata_t *meta,
                                         uint32_t len)
{
#if defined(SLI_LARGE_BLOCK_SUPPORT)
  meta->length_msb = (len >> 16) & 0xF;
  meta->length = len & 0xFFFF;
#else
  meta->length = (uint16_t)len;
#endif
}

/***************************************************************************//**
 * Encodes the previous offset of a memory block in its metadata.
 *
 * @param[in,out] meta Pointer to memory block metadata.
 * @param[in]     len  Previous offset in double words.
 ******************************************************************************/
__INLINE void sli_block_offset_prev_dword_encode(sli_block_metadata_t *meta,
                                                 uint32_t len)
{
#if defined(SLI_LARGE_BLOCK_SUPPORT)
  meta->offset_neighbour_prev_msb = (len >> 16) & 0xF;
  meta->offset_neighbour_prev = len & 0xFFFF;
#else
  meta->offset_neighbour_prev = (uint16_t)len;
#endif
}

/***************************************************************************//**
 * Encodes the next offset of a memory block in its metadata.
 *
 * @param[in,out] meta Pointer to memory block metadata.
 * @param[in]     len  Next offset in double words.
 ******************************************************************************/
__INLINE void sli_block_offset_next_dword_encode(sli_block_metadata_t *meta,
                                                 uint32_t len)
{
#if defined(SLI_LARGE_BLOCK_SUPPORT)
  meta->offset_neighbour_next_msb = (len >> 16) & 0xF;
  meta->offset_neighbour_next = len & 0xFFFF;
#else
  meta->offset_neighbour_next = (uint16_t)len;
#endif
}

/***************************************************************************//**
 * Decodes the length of a memory block.
 *
 * @param[in] meta Pointer to memory block metadata.
 *
 * @return The length of the memory block in double words.
 ******************************************************************************/
__INLINE uint32_t sli_block_len_dword_decode(const sli_block_metadata_t *meta)
{
#if defined(SLI_LARGE_BLOCK_SUPPORT)
  return ((meta->length_msb << 16) | meta->length);
#else
  return meta->length;
#endif
}

/***************************************************************************//**
 * Decodes the previous offset of a memory block.
 *
 * @param[in] meta Pointer to memory block metadata.
 *
 * @return The previous offset of the memory block in double words.
 ******************************************************************************/
__INLINE uint32_t sli_block_offset_prev_dword_decode(const sli_block_metadata_t *meta)
{
#if defined(SLI_LARGE_BLOCK_SUPPORT)
  return ((meta->offset_neighbour_prev_msb << 16) | meta->offset_neighbour_prev);
#else
  return meta->offset_neighbour_prev;
#endif
}

/***************************************************************************//**
 * Decodes the next offset of a memory block.
 *
 * @param[in] meta Pointer to memory block metadata.
 *
 * @return The next offset of the memory block in double words.
 ******************************************************************************/
__INLINE uint32_t sli_block_offset_next_dword_decode(const sli_block_metadata_t *meta)
{
#if defined(SLI_LARGE_BLOCK_SUPPORT)
  return ((meta->offset_neighbour_next_msb << 16) | meta->offset_neighbour_next);
#else
  return meta->offset_neighbour_next;
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* SLI_MEMORY_MANAGER_H_ */
