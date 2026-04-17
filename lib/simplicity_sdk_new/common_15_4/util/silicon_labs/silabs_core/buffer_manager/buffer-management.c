/***************************************************************************//**
 * @file
 * @brief Buffer allocation and management routines.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

// Questions:
//  - How much RAM does the pointer compression actually save?

// Provides two links in order to allow queues of header+payload.

// Issues:
//  Buffered output
//    Add a layer on top of the FIFO that allows the FIFO code
//    to pull more data when needed.  This requires a lock of
//    some kind because the FIFO operates as an ISR.  If the
//    lock is set the FIFO sets a flag indicating that data should
//    be pushed when the lock is released.  This allows the
//    existing buffered output code to be left as-is, and only
//    requires minor changes to the FIFO code.  Post-compaction
//    cleanup needs to check the flags and push data as needed.
//----------------------------------------------------------------

#ifdef SL_ZIGBEE_TEST

// on 64 bit macos, we will need to undefine a bunch of HTONS that are redefined in base/../endian.h
// so the ordering of this include matters
#include <stdio.h>       // for fprintf
#include <stdlib.h>      // for malloc() and free()

#endif

#include <string.h>      // for memmove()
#include PLATFORM_HEADER
#include STACK_CORE_HEADER
#include "sl_status.h"
#include "sl_code_classification.h"
#include "buffer-management.h"
#include "buffer-queue.h"

uint16_t *emHeapLimit;
#if defined(SL_ZIGBEE_TEST) || defined(EMBER_TEST)
uint16_t *emHeapBase;
uint16_t *heapPointer;             // points to next free word
sli_buffer_manager_buffer_t phyToMacQueue = NULL_BUFFER;
#ifdef MAC_DUAL_PRESENT
sli_buffer_manager_buffer_t phyToMacQueue2 = NULL_BUFFER;
#endif
#else
static uint16_t *emHeapBase;
static uint16_t *heapPointer;             // points to next free word
static sli_buffer_manager_buffer_t phyToMacQueue = NULL_BUFFER;
#ifdef MAC_DUAL_PRESENT
static sli_buffer_manager_buffer_t phyToMacQueue2 = NULL_BUFFER;
#endif
#endif
#ifdef SL_ZIGBEE_MBEDTLS_STACK
char *emMallocHeap = NULL;
#endif

// The following two lines can be enable to use SEGGER_RTT print for debugging.
// For that, include the segger_rtt and segger_rtt_printf componenents
// to the ncp project and uncomment last two lines to define DEBUG to RTT printf,
// while comment out the last two lines.
//#include "SEGGER_RTT.h"
//#define DEBUG(...) SEGGER_RTT_printf(0, __VA_ARGS__)
//#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#undef DEBUG
#define DEBUG(...)

uint16_t sli_legacy_buffer_manager_save_heap_state(void)
{
  return heapPointer - emHeapBase;
}

void sli_legacy_buffer_manager_restore_heap_state(uint16_t state)
{
  heapPointer = emHeapBase + state;
}

//----------------------------------------------------------------

#ifdef SL_ZIGBEE_TEST
// These are used when running under Unix to keep track of which
// Buffer values are legitimate.  They parallel the heap array:
// emMallocedContents[x] is true iff emHeapBase + x is the beginning of
// a buffer.  emNewMallocedContents[] is used during temporarily during
// the GC.
static uint16_t **emMallocedContents;
static uint16_t **emNewMallocedContents;

#define bufferTest(x) do { x } while (0)

#define NOT_NULL ((uint16_t *) 16)

// During the final phase of the GC we are partly using the old heap
// and partly using the new heap, so the emMallocedContents[] checking doesn't
// work properly.  This disables the checking and is set to true during
// that final GC phase.
static bool inGcCleanup;

// If this is true we use malloc() to allocated the contents of each buffer.
// The buffer management code runs as normal, but sli_legacy_buffer_manager_get_buffer_pointer()
// returns the malloc'ed contents rather than a pointer into the heap.
// This allows Valgrind to detect attempts to read or write outside the
// buffer's actual contents.
bool sli_legacy_buffer_manager_use_malloc = false;

static uint16_t *testMalloc(size_t size)
{
  if (sli_legacy_buffer_manager_use_malloc) {
    return (uint16_t *) malloc(size);
  } else {
    return NOT_NULL;
  }
}

static void testFree(void *thing)
{
  if (sli_legacy_buffer_manager_use_malloc) {
    free(thing);
  }
}

// These are used in conjuction with tool/simulator/child/child-main.c to
// find for allocations that don't check for a NULL_BUFFER return.  See
// child-main.c for how these are used.
uint32_t allocationCount = 0;             // incremented for each new allocation
uint32_t allocationToFail = 0;            // return NULL_BUFFER for this call

// To allow simulated nodes to modify the heap size at run time.

#if defined(SL_ZIGBEE_TEST) && !defined(BUFFER_DUAL_TEST)
static uint16_t *newHeapBase;
static uint16_t *newHeapLimit;
#endif

#ifdef BUFFER_DUAL_TEST
// This is to simulate 4 ISR and 4 packets are added into phyToMac queues
uint8_t array[4][25] = {
  { "This is Pkt1 from SUBGig" },
  { "This is Pkt1 from 2.4GH" },
  { "This is Pkt2 from SUBGig" },
  { "This is Pkt2 from 2.4GH" },
};

// Allocat buffers and fillthe above in order Q2-Q-Q2-Q
// When we do above the buffers will get allocated from the heap outwords.
// When copy call in invoked, it would need to copy the buffers based on the their allocated pointer - whic are incrementing
// so the following copy will check the if the the pointer is a bigger than the next one
// Copy contents to buffer and add to queue 1/2

#define SL_ZIGBEE_TEST_INJECT_ISR_PACKETS() do {                                           \
    for (uint8_t i = 0; i < 4; i++) {                                                      \
      sli_buffer_manager_buffer_t element = sli_legacy_buffer_manager_allocate_buffer(25); \
      if (element != NULL_BUFFER) {                                                        \
        uint8_t *finger = sli_legacy_buffer_manager_get_buffer_pointer(element);           \
        memcpy(finger, array[i], 25);                                                      \
        if (i % 2) {                                                                       \
          sli_legacy_buffer_manager_buffer_queue_add(&phyToMacQueue, element);             \
        } else {                                                                           \
          sli_legacy_buffer_manager_buffer_queue_add(&phyToMacQueue2, element);            \
        }                                                                                  \
      }                                                                                    \
    }                                                                                      \
} while (0)

// If the above copy of the test packets are correctly done to their respective queues
// phyToMacQueue will hold
//  { "This is Pkt1 from 2.4GH"}
//  { "This is Pkt2 from 2.4GH"}
// And phyToMacQueue2 will hold
//  { "This is Pkt1 from SUBGig"}
//  { "This is Pkt2 from SUBGig"}
#define SL_ZIGBEE_TEST_VERIFY_ISR_PACKETS() do {                        \
    uint8_t *fing = NULL;                                               \
    sli_buffer_manager_buffer_t queue, tmp;                             \
    for (uint8_t i = 0; i < 4; i++) {                                   \
      bool match = false;                                               \
      if (i % 2) {                                                      \
        queue = phyToMacQueue;                                          \
      } else {                                                          \
        queue = phyToMacQueue2;                                         \
      }                                                                 \
      tmp = sli_legacy_buffer_manager_buffer_queue_head(&queue);        \
      while (tmp != NULL_BUFFER) {                                      \
        fing = sli_legacy_buffer_manager_get_buffer_pointer(tmp);       \
        if (memcmp(array[i], fing, 25) == 0) {                          \
          match = true;                                                 \
        }                                                               \
        tmp = sli_legacy_buffer_manager_buffer_queue_next(&queue, tmp); \
      }                                                                 \
      assert(match == true);                                            \
    }                                                                   \
} while (0)

#endif // BUFFER_DUAL_TEST
#else
#define bufferTest(x) do {} while (0)
#define sli_legacy_buffer_manager_use_malloc false
#endif

sli_buffer_manager_buffer_t sli_legacy_buffer_manager_malloc_free_list = NULL_BUFFER;

static uint16_t *heapMemory = NULL;
static uint32_t heapMemorySize = 0;

void sli_legacy_buffer_manager_initialize_buffers(void)
{
  sli_legacy_buffer_manager_acquire_heap((void **) &heapMemory, (size_t *) &heapMemorySize);
  assert(heapMemorySize > 0 && heapMemory != NULL);
    #if defined(CORTEXM3)
  // Get the heap base and limit for this chip. The base is assumed to be
  // inclusive and the limit is assumed to be exclusive.  We also assume
  // 4-byte alignemnt on the heap base.
  emHeapBase = heapMemory;
  emHeapLimit = heapMemory + (heapMemorySize / 2);
    #elif SL_ZIGBEE_TEST
  // Allow the simulator to set the heap size if it wants to.
  if (emHeapBase == NULL) {
    emHeapBase = (uint16_t *) ((((unsigned long) heapMemory) + 3) & - 4);
    emHeapLimit = heapMemory + (heapMemorySize / 2);
  }
    #else
  // Align the heap base on 4-byte boundary.
  emHeapBase = (uint16_t *) ((((unsigned long) heapMemory) + 3) & - 4);
  emHeapLimit = heapMemory + (heapMemorySize / 2);
    #endif
    #ifdef SL_ZIGBEE_MBEDTLS_STACK
  // This breaks if EMBER_MALLOC_HEAP_SIZE_BYTES is not divisible by two.
  if (emMallocHeap == NULL ) {
    emMallocHeap = ((char *) emHeapLimit) - EMBER_MALLOC_HEAP_SIZE_BYTES;
    emHeapLimit = (uint16_t *) emMallocHeap;
  }
    #endif
  heapPointer = emHeapBase;
  assert(emHeapBase != NULL && sli_legacy_buffer_manager_buffer_bytes_remaining() > 100u);
  bufferTest(size_t size = (emHeapLimit - emHeapBase) * sizeof(uint16_t *);
             if (size < 256 * 32 * 4) {
    // force heap size increase so zigbee can test it that way
    size = 256 * 32 * 4;
  }
             emMallocedContents = (uint16_t **) malloc(size);
             emNewMallocedContents = (uint16_t **) malloc(size);
             memset(emMallocedContents,
                    0,
                    (emHeapLimit - emHeapBase) * sizeof(uint16_t *));
             inGcCleanup = false; );
}

uint16_t sli_legacy_buffer_manager_buffer_bytes_used(void)
{
  assert(heapPointer >= emHeapBase);
  return ((heapPointer - emHeapBase) << 1);
}

uint16_t sli_legacy_buffer_manager_buffer_bytes_remaining(void)
{
  assert(heapPointer <= emHeapLimit);
  return ((emHeapLimit - heapPointer) << 1);
}

uint16_t sli_legacy_buffer_manager_buffer_bytes_total(void)
{
  assert(emHeapBase < emHeapLimit);
  return ((emHeapLimit - emHeapBase) << 1);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BUFFER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
bool sli_legacy_buffer_manager_points_into_heap(void *pointer)
{
  return (emHeapBase <= ((uint16_t *) pointer)
          && ((uint16_t *) pointer) < heapPointer);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BUFFER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
static bool isHeapPointer(uint16_t *thing)
{
  return (thing == NULL
          || sli_legacy_buffer_manager_points_into_heap(thing));
}

// Hack for testing the code that deals with allocation by the receive
// ISR during compaction.
#ifdef SL_ZIGBEE_TEST
static void doNothing(void)
{
}
void (*emCompactionInterruptProc)(void) = doNothing;
#endif // SL_ZIGBEE_TEST

// Biasing by emHeapBase allows us to store pointers in 16 bits.
// Biasing by emHeapBase + 1 allows use to use 0 as NULL_BUFFER,
// which avoids the need to initialize roots.
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BUFFER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
static uint16_t compressPointer(uint16_t *pointer)
{
  if (pointer == NULL) {
    return NULL_BUFFER;
  } else {
    return (uint16_t) (((pointer - emHeapBase) + 1u) & 0xFFFFu);
  }
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BUFFER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
static uint16_t * expandPointerNoCheck(uint16_t buffer)
{
  if (buffer == NULL_BUFFER) {
    return NULL;
  } else {
    return emHeapBase + (buffer - 1u);
  }
}

#ifdef SL_ZIGBEE_TEST

static uint16_t *expandPointer(uint16_t buffer)
{
  if (buffer == NULL_BUFFER) {
    return NULL;
  } else {
    // NOTE failure here usually indicates an unmarked buffer
    assert(inGcCleanup || emMallocedContents[buffer - 1] != NULL);
    return emHeapBase + (buffer - 1);
  }
}

#else

#define expandPointer expandPointerNoCheck

#endif

//----------------------------------------------------------------

#define NEW_LOCATION_INDEX 0u    // Must be first because first value cannot
// be 0xFFFF, which is used to mark unused
// memory.
#define SIZE_INDEX         1u
#define LINK0_INDEX        2u
#define LINK1_INDEX        3u
#define NUM_LINKS          2u

#define OVERHEAD_IN_WORDS 4u
#define OVERHEAD_IN_BYTES (WORDS_TO_BYTES(OVERHEAD_IN_WORDS))

// Indirect buffers contain a pointer to memory elsewhere, rather
// than containing the storage themselves.  This allows buffers to
// refer to data in read-only memory, such as certificates.

#define INDIRECT_BUFFER_LENGTH_INDEX  4u
#define INDIRECT_BUFFER_POINTER_INDEX 5u
#define INDIRECT_BUFFER_OBJ_REF_INDEX (INDIRECT_BUFFER_POINTER_INDEX + BYTES_TO_WORDS(sizeof(uint8_t *)))
#define INDIRECT_BUFFER_DATA_SIZE_IN_BYTES \
  (WORDS_TO_BYTES(1u) + 2u * sizeof(uint8_t *))

// The compactor uses two flags, LIVE and TRACED.  LIVE is set if a
// buffer is reachable from a root.  TRACED is set if the buffer is
// live and its links have had their LIVE flags set.
//
// These flags are at the start of the size word.
#define LIVE_BIT         0x8000u
#define TRACED_BIT       0x4000u
#define PHY_FREELIST_BIT 0x2000u
#define SIZE_MASK        0x1FFFu

// This flag is at the start of the new location mask.
#define INDIRECT_BIT            0x8000u
#define NEW_LOCATION_MASK       0x7FFFu

#define sizeWord(thing)    ((thing)[SIZE_INDEX])

#define BOOLEAN(x) ((x) != 0u ? true : false)

#define getDataSize(thing)    (sizeWord(thing) & SIZE_MASK)
#define clearFlags(thing)     (sizeWord(thing) &= ~(LIVE_BIT | TRACED_BIT))
#define setLive(thing)        (sizeWord(thing) |= LIVE_BIT)
#define isLive(thing)         BOOLEAN((sizeWord(thing) & LIVE_BIT))
#define setTraced(thing)      (sizeWord(thing) |= TRACED_BIT)
#define clearTraced(thing)    (sizeWord(thing) &= ~TRACED_BIT)
#define isTraced(thing)       BOOLEAN((sizeWord(thing) & TRACED_BIT))

static uint16_t newLocation(uint16_t *bufferPointer)
{
  return bufferPointer[NEW_LOCATION_INDEX] & NEW_LOCATION_MASK;
}

static void setNewLocation(uint16_t *bufferPointer,
                           uint16_t theNewLocation)
{
  bufferPointer[NEW_LOCATION_INDEX] =
    (bufferPointer[NEW_LOCATION_INDEX] & INDIRECT_BIT)
    | theNewLocation;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BUFFER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
static bool isIndirect(uint16_t *bufferPointer)
{
  return BOOLEAN(bufferPointer[NEW_LOCATION_INDEX] & INDIRECT_BIT);
}

// This returns an even number so that the buffers (and their contents) are
// aligned on four-byte boundaries, assuming the emHeapBase is on a four-byte
// boundary.
#define BYTES_TO_WORDS(b) ((((b) + 3u) >> 2) << 1)
#define WORDS_TO_BYTES(w) ((w) << 1)

static uint16_t getSizeInWords(uint16_t *thing)
{
  return OVERHEAD_IN_WORDS + BYTES_TO_WORDS(getDataSize(thing));
}

// We use 0xFFFF to mark dead space created when a buffer is
// shortened by a word or two.  For larger shortenings we can
// insert a full object.

#define UNUSED_MEMORY 0xFFFFu

static uint16_t *nextBuffer(uint16_t *bufferPointer)
{
  uint16_t *next = bufferPointer + getSizeInWords(bufferPointer);
  while (next < heapPointer
         && *next == UNUSED_MEMORY) {
    next += 1;
  }
  return next;
}

#define getSizeInBytes(bufferPointer) (getSizeInWords(bufferPointer) << 1)

static const char *currentUser = "unclaimed";

void sli_legacy_buffer_manager_buffer_usage(const char *tag)
{
  currentUser = tag;
}

void sli_legacy_buffer_manager_end_buffer_usage(void)
{
  currentUser = "unclaimed";
}

typedef struct {
  const char *user;
  uint16_t count;
  uint16_t size;
} UserData;

static UserData *userData;
static uint16_t userDataCount;

static void noteUse(uint16_t *object)
{
  if (userDataCount > 0u
      && !isLive(object)) {
    uint16_t i;
    assert(strcmp(currentUser, "unclaimed") != 0);
    for (i = 0;; i++) {
      assert(i < userDataCount);
      if (userData[i].user == NULL) {
        userData[i].user = currentUser;
        break;
      } else if (strcmp(userData[i].user, currentUser) == 0) {
        break;
      }
    }
    userData[i].count += 1u;
    userData[i].size += getSizeInBytes(object);
  }
}

// TODO: switch to use the standard I/O stream APIs.
#define reportPrint(...)

#if 0
  #ifdef SL_ZIGBEE_SCRIPTED_TEST
    #define reportPrint(...)
  #else
    #ifdef SL_ZIGBEE_TEST
      #define reportPrint(p, f1, f2, ...)                 \
  if (port == 0xFF) { fprintf(stderr, f1, __VA_ARGS__); } \
  else { sli_legacy_serial_printf_line(p, f2, __VA_ARGS__); }
    #else
      #define reportPrint(p, f1, f2, ...) \
  sli_legacy_serial_printf_line(p, f2, __VA_ARGS__);
    #endif
  #endif
#endif

static void reportUsage(uint8_t port)
{
  (void)port;

  uint16_t count = 0;
  uint16_t size = 0;
  uint16_t i;
  for (i = 0; i < userDataCount && userData[i].user != NULL; i++) {
    uint16_t oh = userData[i].count * OVERHEAD_IN_BYTES;
    uint16_t sz = userData[i].size - oh;
    count += userData[i].count;
    size += sz;
    reportPrint(port, " %4d %d+%d=%d %s\n", " %d %d+%d=%d %s",
                userData[i].count,
                oh, sz, oh + sz,
                userData[i].user);
  }
  uint16_t total = count * OVERHEAD_IN_BYTES + size;
  reportPrint(port, " %4d %d+%d=%d total\n", " %d %d+%d=%d total",
              count, count * OVERHEAD_IN_BYTES, size, total);
  assert(total == WORDS_TO_BYTES(heapPointer - emHeapBase));
}

void sli_legacy_buffer_manager_print_buffers(uint8_t port, const BufferMarker *markers)
{
  UserData data[20];
  userData = data;
  userDataCount = COUNTOF(data);
  sli_legacy_buffer_manager_reclaim_unused_buffers(markers);
  reportUsage(port);
  userData = NULL;
  userDataCount = 0;
}

#ifdef SL_ZIGBEE_TEST

// Consistency check.  The main purpose of this is to detect buffer overruns,
// so we check the next buffer, not this one.  That way an error gets signalled
// for the offending buffer and not for its innocent victim.

static bool nextBufferIsOkay(uint16_t *bufferPointer)
{
  uint16_t i;

  bufferPointer = nextBuffer(bufferPointer);

  if (heapPointer <= bufferPointer) {
    return true;
  }

  if (!((newLocation(bufferPointer) == 0
         || newLocation(bufferPointer) == compressPointer(bufferPointer))
        && ((getDataSize(bufferPointer)
             < (uint16_t)((uint8_t *) heapPointer - (uint8_t *) emHeapBase))
            || isIndirect(bufferPointer)))) {
    return false;
  }

  for (i = 0; i < NUM_LINKS; i++) {
    if (!isHeapPointer(expandPointer(bufferPointer[LINK0_INDEX + i]))) {
      return false;
    }
  }

  return true;
}

//----------------------------------------------------------------
// Keeping track of who uses how much of the heap.

#define bufferUsage(x) do { x } while (0)

// The way to use this is run a simulator trace under GDB.  Print out
// gcCount at the time of interest.  Then set printUsageGcCount to that
// value and restart the trace.  The buffer usage at the previous buffer
// reclamation will be printed out, as will every subsequent allocation.

static uint32_t gcCount = 0;
static uint32_t printUsageGcCount = -1;

bool emTraceBufferUsage = false;
UserData simUserData[1000];

#if 0

#include <execinfo.h>
#include <dlfcn.h>

static void printBacktrace(void)
{
  void *callstack[6];
  int frames = backtrace(callstack, 6);
  int i;
  for (i = 2; i < frames; i++) {
    Dl_info info;
    assert(dladdr(callstack[i], &info) != 0);
    if (i == 2) {
      assert(strcmp(info.dli_sname, "sli_legacy_buffer_manager_really_allocate_buffer") == 0);
    } else {
      fprintf(stderr, " > %s", info.dli_sname);
    }
  }
  fprintf(stderr, "\n");
}

#endif  // if 0

static void noteAllocation(uint16_t bytes)
{
  if (emTraceBufferUsage) {
    fprintf(stderr, "allocating %d+%d=%d bytes\n",
            OVERHEAD_IN_BYTES, bytes, OVERHEAD_IN_BYTES + bytes);
    //printBacktrace();
  }
}

#else   // ifdef(SL_ZIGBEE_TEST)
#define bufferUsage(x) do {} while (0)
#endif  // ifdef(SL_ZIGBEE_TEST)

//----------------------------------------------------------------

// The stack/done/isTraced mechanism would be unnecessary if buffers had
// only one link.  We want two to allow queues of header+payload.
//
// It is unlikely that any actual heap will fill up the untraced
// stack.  Doing so requires a long chain through the second link
// field with each buffer having both first and second links.

#define UNTRACED_STACK_SIZE 16

enum {
  NOT_TRACING = 0,
  MARK_LIVE,
  UPDATE
};

static uint8_t tracePhase;

// For roots we walk down the link0 and link1 fields before doing the
// full tracing.  This greatly reduces the number of times we have to
// walk through the heap for the typical case where a root contains a
// list or queue of buffers.

void sli_legacy_buffer_manager_mark_buffer(sli_buffer_manager_buffer_t *root)
{
  if (tracePhase == MARK_LIVE) {
    uint16_t *object = expandPointer(*root);
    uint8_t linkIndex;
    if (object != NULL) {
      for (linkIndex = LINK0_INDEX;
           linkIndex < LINK0_INDEX + NUM_LINKS;
           linkIndex++) {
        uint16_t *linkedObject = object;
        do {
          ATOMIC(
            bufferTest(assert(nextBufferIsOkay(linkedObject)); );
            noteUse(linkedObject);
            setLive(linkedObject);
            linkedObject = expandPointer(linkedObject[linkIndex]);
            );
        } while (linkedObject != NULL
                 && !isLive(linkedObject));
      }
    }
  } else if (tracePhase == UPDATE) {
    uint16_t *object;
    ATOMIC(
      object = expandPointerNoCheck(*root);
      if (object != NULL) {
      *root = newLocation(object);
    }
      );
  }
}

#ifdef BUFFER_SUPPORT_AMALGAMATE
void sli_legacy_buffer_manager_reclaim_unused_buffers(const BufferMarker *markers)
{
  uint8_t scratchpad[1200];
  sli_legacy_buffer_manager_reclaim_unused_buffers_and_amalgamate(markers,
                                                                  scratchpad,
                                                                  sizeof(scratchpad));
}
#endif // BUFFER_SUPPORT_AMALGAMATE

//------------------------------------------------------------------------------
// Buffer amalgamation.

// Utility - copy the buffer from its old location to its new location.

static void moveToNewLocation(uint16_t *old, uint16_t *new)
{
  uint16_t size = WORDS_TO_BYTES(getSizeInWords(old));
  clearFlags(old);
  DEBUG("[copy %04X -> %04X Size = %02X]\n",
        compressPointer(old), compressPointer(new), BYTES_TO_WORDS(size));
  (void) memmove(new, old, size);
}

// Advance finger over any buffers that are no longer live.

static uint16_t *skipOverNotLive(uint16_t *finger, uint16_t *heapEnd)
{
  while (finger < heapEnd
         && !isLive(finger)) {
    finger = nextBuffer(finger);
  }
  return finger;
}

#ifdef BUFFER_SUPPORT_AMALGAMATE
static sli_buffer_manager_buffer_t *amalgamateQueue;

static bool noPayloadLinks(sli_buffer_manager_buffer_t *queue)
{
  sli_buffer_manager_buffer_t finger = sli_legacy_buffer_manager_buffer_queue_head(queue);
  while (finger != NULL_BUFFER) {
    if (sli_legacy_buffer_manager_get_payload_link(finger) != NULL_BUFFER
        || isIndirect(expandPointer(finger))) {
      return false;
    }
    finger = sli_legacy_buffer_manager_buffer_queue_next(queue, finger);
  }
  return true;
}

bool sli_legacy_buffer_manager_mark_amalgamate_queue(sli_buffer_manager_buffer_t *queue)
{
  if (tracePhase == MARK_LIVE
      && amalgamateQueue == NULL                // first come, first served
      && 1 < sli_legacy_buffer_manager_buffer_queue_length(queue)
      && noPayloadLinks(queue)) {
    amalgamateQueue = queue;
    return true;
  }
  sli_legacy_buffer_manager_mark_buffer(queue);
  return false;
}

// If amalgamateQueue contains 2 or more buffers, the buffer at the head of
// the queue (amalgamateHead) is merged with the second buffer in the queue
// (amalgamateNext). The contents of amalgamateNext are temporarily moved to a
// scratchpad while the heap is being compacted. amalgamateHead then increases
// in size to provide a new home for the contents on the scratchpad.
//
// Usually amalgamateHead will come first on the heap because it was allocated
// before amalgamateNext. Therefore to make amalgamateHead bigger, any buffers
// between amalgamateHead and amalgamateNext must be moved forward on the heap
// during compaction. Normally buffers only move down on the heap during
// compaction.
//
// Before: Q1 A B Q2 C X D
// After:  Q1+Q2 A B C D
//
// Once the compaction is complete, the contents of amalgamateNext are moved
// from the scratchpad to the space at the end of amalgamateHead.
//
// Limitations:
// - The scratchpad must be large enough to hold all of amalgamateNext.
// - There must be no other references to the buffers in amalgamateQueue.

void sli_legacy_buffer_manager_reclaim_unused_buffers_and_amalgamate(const BufferMarker *markers,
                                                                     uint8_t* scratchpad,
                                                                     uint16_t scratchpadSize)
#else // BUFFER_SUPPORT_AMALGAMATE
void sli_legacy_buffer_manager_reclaim_unused_buffers(const BufferMarker *markers)
#endif // BUFFER_SUPPORT_AMALGAMATE
{
  DEBUG("-- GC Start --\n");
  uint16_t *finger;
  uint16_t *heap;
  uint16_t *heapEnd;
  uint16_t *liveHeapEnd;
  sli_buffer_manager_buffer_t phyToMacQueueTemp;
#ifdef MAC_DUAL_PRESENT
  sli_buffer_manager_buffer_t phyToMacQueueTemp2;
#endif
  bool done;

  sli_legacy_buffer_manager_malloc_free_list = NULL_BUFFER;
#ifdef BUFFER_SUPPORT_AMALGAMATE
  amalgamateQueue = NULL;
#endif

  ATOMIC(
    tracePhase = MARK_LIVE;
    phyToMacQueueTemp = phyToMacQueue;
    phyToMacQueue = NULL_BUFFER;
#ifdef MAC_DUAL_PRESENT
    phyToMacQueueTemp2 = phyToMacQueue2;
    phyToMacQueue2 = NULL_BUFFER;
#endif
    heapEnd = heapPointer;
    );

  bufferUsage(gcCount += 1;
              if (gcCount == printUsageGcCount) {
    emTraceBufferUsage = true;
    userData = simUserData;
    userDataCount = COUNTOF(simUserData);
  }
              );
  (void) memset(userData, 0, userDataCount * sizeof(UserData));

  // trace the roots, including the PHY->MAC queue

  bufferUsage(sli_legacy_buffer_manager_buffer_usage("PHY->MAC queue"); );
  sli_legacy_buffer_manager_mark_buffer(&phyToMacQueueTemp);
#ifdef MAC_DUAL_PRESENT
  sli_legacy_buffer_manager_mark_buffer(&phyToMacQueueTemp2);
#endif

  bufferUsage(sli_legacy_buffer_manager_buffer_usage("marker array"); );
  if (markers != NULL) {
    uint8_t i;
    for (i = 0; markers[i] != NULL; i++) {
      markers[i]();
    }
  }

  // trace the links
  bufferUsage(sli_legacy_buffer_manager_buffer_usage("tracing"); );
  do {
    uint16_t *stack[UNTRACED_STACK_SIZE]; // live buffers that need to be traced
    uint8_t top = 0;                      // top of the stack

    finger = emHeapBase;                // walk the heap finding live buffers
    done = true;                        // set to false if an untraced buffer
                                        //   could not be put on the stack

    while (finger < heapEnd
           || 0u < top) {
      uint16_t *next;

      if (0u < top) {
        top -= 1u;
        next = stack[top];
      } else {
        next = finger;
        finger = nextBuffer(finger);
      }

      if (isLive(next)
          && !isTraced(next)) {
        setTraced(next);
        uint8_t i;

        for (i = LINK0_INDEX; i <= LINK1_INDEX; i++) {
          uint16_t *link = expandPointer(next[i]);

          if (link != NULL
              && !isLive(link)) {
            setLive(link);
            noteUse(link);
            if (top < UNTRACED_STACK_SIZE) {
              stack[top] = link;
              top += 1u;
            } else {
              done = false;
            }
          }
        }
      }
    }
  } while (!done);

  // decide what to amalgamate

#ifdef BUFFER_SUPPORT_AMALGAMATE
  if (amalgamateQueue != NULL) {
    // Disable amalgamation if any buffers in the queue were already marked.
    // Because queues are circular, we only need to check one.
    expandPointer(*amalgamateQueue);
    if (amalgamateQueue != NULL) {
      if (isLive(amalgamateQueue)) {
        amalgamateQueue = NULL;
      } else {
        sli_legacy_buffer_manager_mark_buffer(amalgamateQueue);
      }
    }
  }

  sli_buffer_manager_buffer_t amalgamateHead = NULL_BUFFER;
  uint16_t amalgamateHeadLength = 0;
  uint16_t amalgamateNextLength = 0;

  if (amalgamateQueue != NULL) {
    amalgamateHead = sli_legacy_buffer_manager_get_queue_link(*amalgamateQueue);
    sli_buffer_manager_buffer_t amalgamateNext = sli_legacy_buffer_manager_get_queue_link(amalgamateHead);
    amalgamateHeadLength = sli_legacy_buffer_manager_get_buffer_length(amalgamateHead);
    amalgamateNextLength = sli_legacy_buffer_manager_get_buffer_length(amalgamateNext);
    if (amalgamateNextLength <= scratchpadSize
        && amalgamateNext != NULL_BUFFER) {
      memmove(scratchpad,
              sli_legacy_buffer_manager_get_buffer_pointer(amalgamateNext),
              sli_legacy_buffer_manager_get_buffer_length(amalgamateNext));
      if (amalgamateNext != NULL_BUFFER) {
        uint16_t *ptr = expandPointer(amalgamateNext);
        if (ptr != NULL) {
          clearFlags(ptr);
        }
        sli_legacy_buffer_manager_buffer_queue_remove(amalgamateQueue, amalgamateNext);
      }
    } else {
      amalgamateHead = NULL_BUFFER;
    }
    DEBUG("head = %04X (%u)\n", amalgamateHead, amalgamateHeadLength);
    DEBUG("next = %04X (%u)\n", amalgamateNext, amalgamateNextLength);
  }
#endif //BUFFER_SUPPORT_AMALGAMATE

  // set new locations

  heap = emHeapBase;

  bufferTest(memset(emNewMallocedContents,
                    0,
                    (emHeapLimit - emHeapBase) * sizeof(uint16_t *)); );

  liveHeapEnd = emHeapBase;

  for (finger = emHeapBase; finger < heapEnd; finger = nextBuffer(finger)) {
    uint16_t sizeInWords = getSizeInWords(finger);
    DEBUG("sizeInWords for %04X is %u\n", compressPointer(finger), sizeInWords);

#ifdef BUFFER_SUPPORT_AMALGAMATE
    if (compressPointer(finger) == amalgamateHead) {
      sizeInWords = (OVERHEAD_IN_WORDS
                     + BYTES_TO_WORDS(amalgamateHeadLength
                                      + amalgamateNextLength));
      DEBUG("increasing sizeInWords to %u\n", sizeInWords);
    }
#endif // BUFFER_SUPPORT_AMALGAMATE
    if (isLive(finger)) {
      setNewLocation(finger, compressPointer(heap));
      bufferTest(emNewMallocedContents[heap - emHeapBase]
                   = (sli_legacy_buffer_manager_use_malloc
                      ? emMallocedContents[finger - emHeapBase]
                      : NOT_NULL); );
      DEBUG("[%04X -> %04X]\n", compressPointer(finger), compressPointer(heap));
      heap += sizeInWords;
      liveHeapEnd = finger + sizeInWords;
    } else {
      bufferTest(testFree(emMallocedContents[finger - emHeapBase]);
                 emMallocedContents[finger - emHeapBase] = NULL; );
      if (isIndirect(finger)) {
        void* objectRef = *((void **) (finger + INDIRECT_BUFFER_OBJ_REF_INDEX));
        if (objectRef != NULL) {
          sl_legacy_buffer_manager_free_memory_for_packet_handler(objectRef);
        }
      }
      setNewLocation(finger, NULL_BUFFER);  //change any weak mark references to this to NULL_BUFFER
    }
  }

  // When copying, don't scan past end of live objects.  Amalgamation may
  // copy live objects forward, overwriting dead objects at the end of the heap.
  heapEnd = liveHeapEnd;

#if defined(SL_ZIGBEE_TEST) && !defined(BUFFER_DUAL_TEST)
  // This only works in simulation.  On real hardware an incoming message ISR
  // that allocates a buffer will break this.
  // We simulate ISR in buffer dual test
  // so only do this when that test was done
  if (newHeapBase != NULL) {
    uint16_t liveSize = liveHeapEnd - emHeapBase;
    memcpy(newHeapBase, emHeapBase, liveSize * sizeof(uint16_t));
    heap = newHeapBase + (heap - emHeapBase);
    heapEnd = newHeapBase + liveSize;
    emHeapBase = newHeapBase;
    emHeapLimit = newHeapLimit;
    newHeapBase = NULL;
    newHeapLimit = NULL;
    size_t size = (emHeapLimit - emHeapBase) * sizeof(uint16_t *);
    emMallocedContents = (uint16_t **) realloc(emMallocedContents, size);
    emNewMallocedContents = (uint16_t **) realloc(emNewMallocedContents, size);
  }
#endif //defined(SL_ZIGBEE_TEST) && !defined(BUFFER_DUAL_TEST)

  // update the roots

  tracePhase = UPDATE;
  sli_legacy_buffer_manager_mark_buffer(&phyToMacQueueTemp);
#ifdef MAC_DUAL_PRESENT
  sli_legacy_buffer_manager_mark_buffer(&phyToMacQueueTemp2);
#endif
#ifdef BUFFER_SUPPORT_AMALGAMATE
  sli_legacy_buffer_manager_mark_buffer(&amalgamateHead);
#endif

  if (markers != NULL) {
    uint8_t i;
    for (i = 0; markers[i] != NULL; i++) {
      markers[i]();
    }
  }

  // update the links

  for (finger = emHeapBase; finger < heapEnd; finger = nextBuffer(finger)) {
    uint8_t i;
    for (i = LINK0_INDEX; i < LINK0_INDEX + NUM_LINKS; i++) {
      uint16_t *old = expandPointerNoCheck(finger[i]);
      if (old != NULL) {
        finger[i] = newLocation(old);
      }
    }
  }

  // do the actual move

  for (finger = emHeapBase; finger < heapEnd; ) {
    uint16_t *next = nextBuffer(finger);
    if (finger != NULL) {
      if (isLive(finger)) {
        uint16_t *dest = expandPointerNoCheck(newLocation(finger));
        if (dest == finger) {
          DEBUG("[copy %04X -> %04X Size = %02X]\n",
                compressPointer(finger),
                compressPointer(finger), getSizeInWords(finger));
          clearFlags(finger);
        } else if (dest < finger) {
          moveToNewLocation(finger, dest);
        } else {
          // We are moving buffers forward, which means they must get moved in
          // in reverse order.  Scan forward to find the end of the block of
          // buffers that move forward.

          uint16_t *forwardBlockStart = finger;
          while (next < heapEnd
                 && next < expandPointerNoCheck(newLocation(next))) {
            finger = next;
            next = skipOverNotLive(nextBuffer(next), heapEnd);
          }
          DEBUG("[scan forward to %04X -> %04X]\n",
                compressPointer(finger), compressPointer(next));

          // At this point next == nextBuffer(finger), next moves
          // backward, and finger moves forward.  After the forward
          // moves are done, the outer loop will continue with next.

          // Do the forward moves, starting with finger and working backwards
          // to forwardBlockStart.
          uint16_t *lastMoved;
          do {
            if (finger != NULL) {
              uint16_t fingerLoc = newLocation(finger);
              uint16_t *fingerPtr = expandPointerNoCheck(fingerLoc);
              if (fingerPtr != NULL) {
                moveToNewLocation(finger, fingerPtr);
              }
            }
            lastMoved = finger;
            // Scan forward to find the buffer before lastMoved.
            uint16_t *scanNext = forwardBlockStart;
            while (scanNext < lastMoved) {
              finger = scanNext;
              scanNext = skipOverNotLive(nextBuffer(scanNext), heapEnd);
            }
            DEBUG("[scan forward2 to %04X -> %04X]\n",
                  compressPointer(finger), compressPointer(scanNext));
            // Quit when we have worked back to the start of the block.
          } while (lastMoved != forwardBlockStart);
        }
      }
    }
    finger = next;
  }

  // The receive ISR may have been adding to the PHY_TO_MAC_QUEUE while
  // we were busy.  This moves any new buffers to the new heap base and
  // puts them on phyToMacQueue.  If PHY_TO_MAC_QUEUE is empty we are done.

//  fprintf(stderr, "[start]\n");

  // During this next phase we are operating partly in the old heap
  // and partly in the new heap, so the flag checking doesn't work.
  bufferTest(inGcCleanup = true; );

#ifndef MAC_DUAL_PRESENT
  do {
    ATOMIC(
      finger = expandPointer(sli_legacy_buffer_manager_buffer_queue_remove_head(&phyToMacQueue));
      if (finger == NULL) {
//        fprintf(stderr, "[done]\n");
      phyToMacQueue = phyToMacQueueTemp;
      heapPointer = heap;
      tracePhase = NOT_TRACING;
    }
      )
    if (finger != NULL) {
      uint16_t size = getSizeInWords(finger);
      uint16_t newTail = compressPointer(heap);
      bufferTest(emNewMallocedContents[heap - emHeapBase]
                   = (sli_legacy_buffer_manager_use_malloc
                      ? emMallocedContents[finger - emHeapBase]
                      : NOT_NULL); );
      (void) memmove(heap, finger, WORDS_TO_BYTES(size));
      // fprintf(stderr, "[ISR copy %04X -> %04X]\n",
      //         compressPointer(finger), compressPointer(heap));
      sli_legacy_buffer_manager_buffer_queue_add(&phyToMacQueueTemp, newTail);
      heap += size;
    }
  } while (finger != NULL);

#else
  uint8_t whichQueue = 0;
#if defined(SL_ZIGBEE_TEST) && defined (BUFFER_DUAL_TEST)
  SL_ZIGBEE_TEST_INJECT_ISR_PACKETS();
#endif //defined(SL_ZIGBEE_TEST) && defined (BUFFER_DUAL_TEST)
  do {
    ATOMIC(
      finger = NULL;
      uint16_t *finger1 = expandPointer(sli_legacy_buffer_manager_buffer_queue_head(&phyToMacQueue));
      uint16_t *finger2 = expandPointer(sli_legacy_buffer_manager_buffer_queue_head(&phyToMacQueue2));
      if (finger1 != NULL
          && finger2 != NULL) {
      if (finger1 < finger2) {
        finger = finger1;
        sli_legacy_buffer_manager_buffer_queue_remove_head(&phyToMacQueue);
        whichQueue = 1;
      } else {
        finger = finger2;
        sli_legacy_buffer_manager_buffer_queue_remove_head(&phyToMacQueue2);
        whichQueue = 2;
      }
    } else if (finger1 != NULL) {
      finger = finger1;
      whichQueue = 1;
      sli_legacy_buffer_manager_buffer_queue_remove_head(&phyToMacQueue);
    } else if (finger2 != NULL) {
      sli_legacy_buffer_manager_buffer_queue_remove_head(&phyToMacQueue2);
      finger = finger2;
      whichQueue = 2;
    }

      if (finger == NULL) {
      // both are done
      // fprintf(stderr, "[done]\n");
      phyToMacQueue = phyToMacQueueTemp;
      phyToMacQueue2 = phyToMacQueueTemp2;
      heapPointer = heap;
      tracePhase = NOT_TRACING;
    }
      )
    if (finger != NULL) {
      uint16_t size = getSizeInWords(finger);
      uint16_t newTail = compressPointer(heap);
      bufferTest(emNewMallocedContents[heap - emHeapBase]
                   = (sli_legacy_buffer_manager_use_malloc
                      ? emMallocedContents[finger - emHeapBase]
                      : NOT_NULL); );
      memmove(heap, finger, WORDS_TO_BYTES(size));
      DEBUG("[Q%d ISR copy %04X -> %04X Size = %02X]\n",
            (whichQueue - 1), compressPointer(finger), compressPointer(heap), size);
      if (whichQueue != 2) {
        sli_legacy_buffer_manager_buffer_queue_add(&phyToMacQueueTemp, newTail);
      } else {
        sli_legacy_buffer_manager_buffer_queue_add(&phyToMacQueueTemp2, newTail);
      }
      heap += size;
    }
  } while (finger != NULL);
#if defined(SL_ZIGBEE_TEST) && defined (BUFFER_DUAL_TEST)
  SL_ZIGBEE_TEST_VERIFY_ISR_PACKETS();
#endif //defined(SL_ZIGBEE_TEST) && defined (BUFFER_DUAL_TEST)
#endif

  bufferTest(memmove(emMallocedContents,
                     emNewMallocedContents,
                     (emHeapLimit - emHeapBase) * sizeof(uint16_t *));
             inGcCleanup = false; );

  // finish the amalgamation

#ifdef BUFFER_SUPPORT_AMALGAMATE
  if (amalgamateHead != NULL_BUFFER) {
    sli_buffer_manager_buffer_t b = amalgamateHead;
    DEBUG("head = %04X\n", b);
    finger = expandPointer(b);
    uint16_t newSize = amalgamateHeadLength + amalgamateNextLength;
    bufferTest(uint16_t * newMalloc = testMalloc(newSize);
               if (sli_legacy_buffer_manager_use_malloc) {
      memmove(newMalloc,
              emMallocedContents[finger - emHeapBase],
              sli_legacy_buffer_manager_get_buffer_length(b));
    }
               testFree(emMallocedContents[finger - emHeapBase]);
               emMallocedContents[finger - emHeapBase] = newMalloc; );
    DEBUG("[adjusting size of %04X", compressPointer(finger));
    if (finger != NULL) {
      DEBUG(" from %u to %u]\n", sizeWord(finger), newSize);
      sizeWord(finger) = newSize;
    }
    uint8_t *contents = sli_legacy_buffer_manager_get_buffer_pointer(b);
    // Buffer is old head, but bigger. Old next is on the scratchpad. Move
    // scratchpad to end of buffer.
    memmove(contents + amalgamateHeadLength,
            scratchpad,
            amalgamateNextLength);
  }
#endif //BUFFER_SUPPORT_AMALGAMATE

  bufferUsage(if (emTraceBufferUsage) {
    reportUsage(0xFF);
  }
              );
  DEBUG("-- GC End --\n");
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BUFFER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
bool sli_legacy_buffer_manager_is_valid_buffer(sli_buffer_manager_buffer_t buffer)
{
  uint16_t *bufferPointer = expandPointer(buffer);
  return isHeapPointer(bufferPointer);
}

//----------------------------------------------------------------
// External Interface

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BUFFER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
uint8_t * sli_legacy_buffer_manager_get_buffer_pointer(sli_buffer_manager_buffer_t buffer)
{
  if (buffer == NULL_BUFFER) {
    return NULL;
  }

  assert(sli_legacy_buffer_manager_is_valid_buffer(buffer));
  uint16_t *bufferPointer = expandPointer(buffer);

  if (bufferPointer != NULL) {
    if (isIndirect(bufferPointer)) {
      return *((uint8_t **) (bufferPointer + INDIRECT_BUFFER_POINTER_INDEX));

  #ifdef SL_ZIGBEE_TEST
    } else if (sli_legacy_buffer_manager_use_malloc) {
      return (uint8_t *) emMallocedContents[bufferPointer - emHeapBase];
  #endif
    } else {
      return (uint8_t *) (bufferPointer + OVERHEAD_IN_WORDS);
    }
  }
  // We will never get here because of the assert, CSTAT null checks
  return NULL;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BUFFER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
uint16_t sli_legacy_buffer_manager_get_buffer_length(sli_buffer_manager_buffer_t buffer)
{
  if (buffer == NULL_BUFFER) {
    return 0;
  }

  uint16_t *bufferPointer = expandPointer(buffer);
  assert(isHeapPointer(bufferPointer));
  if (bufferPointer != NULL) {
    if (isIndirect(bufferPointer)) {
      return bufferPointer[INDIRECT_BUFFER_LENGTH_INDEX];
    } else {
      return getDataSize(bufferPointer);
    }
  }
  // Will never get here because of the asserts, CSTAT null checks
  return 0;
}

void sli_legacy_buffer_manager_set_buffer_length(sli_buffer_manager_buffer_t buffer, uint16_t newLength)
{
  uint16_t *bufferPointer = expandPointer(buffer);
  uint16_t oldLength;
  uint16_t *next;

  if (bufferPointer != NULL) {
    assert(isHeapPointer(bufferPointer));
    oldLength = sli_legacy_buffer_manager_get_buffer_length(buffer);
    next = nextBuffer(bufferPointer);

    if (newLength == oldLength) {
      return;
    }

    assert(newLength < oldLength);

    if (isIndirect(bufferPointer)) {
      bufferPointer[INDIRECT_BUFFER_LENGTH_INDEX] = newLength;
    } else {
      // If there is enough space we turn the leftover space into a new,
      // unreferenced buffer.  If not, we fill it with the UNUSED_MEMORY
      // marker.
      uint16_t *leftoverPointer;
      uint16_t leftoverBytes;

      sizeWord(bufferPointer) = newLength;

      leftoverPointer = bufferPointer + getSizeInWords(bufferPointer);
      leftoverBytes = ((uint8_t *) next) - ((uint8_t *) leftoverPointer);

      ATOMIC(
        if (next == heapPointer) {
        heapPointer = leftoverPointer;
        leftoverBytes = 0;
      }
        )

      if (leftoverBytes < OVERHEAD_IN_BYTES) {
        (void) memset(leftoverPointer, (uint8_t)UNUSED_MEMORY, leftoverBytes);
      } else {
        (void) memset(leftoverPointer, 0, OVERHEAD_IN_BYTES);
        sizeWord(leftoverPointer) = leftoverBytes - OVERHEAD_IN_BYTES;
      }
    }
  }
}

void sli_legacy_buffer_manager_set_buffer_length_from_end(sli_buffer_manager_buffer_t buffer, uint16_t newLength)
{
  uint16_t *bufferPointer = expandPointer(buffer);
  uint16_t oldLength = sli_legacy_buffer_manager_get_buffer_length(buffer);
  uint16_t remove = oldLength - newLength;

  if (newLength == oldLength) {
    return;
  }
  assert(newLength < oldLength);

  if (bufferPointer != NULL) {
    if (isIndirect(bufferPointer)) {
      *((const uint8_t **) (bufferPointer + INDIRECT_BUFFER_POINTER_INDEX))
        += remove;
      bufferPointer[INDIRECT_BUFFER_LENGTH_INDEX] = newLength;
    } else {
      uint8_t *contents = sli_legacy_buffer_manager_get_buffer_pointer(buffer);
      (void) memmove(contents, contents + remove, newLength);
      sli_legacy_buffer_manager_set_buffer_length(buffer, newLength);
    }
  }
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BUFFER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sli_buffer_manager_buffer_t sli_legacy_buffer_manager_get_buffer_link(sli_buffer_manager_buffer_t buffer, uint8_t i)
{
  uint16_t *bufferPointer = expandPointer(buffer);
  if (bufferPointer != NULL) {
    assert(isHeapPointer(bufferPointer));
    return (sli_buffer_manager_buffer_t) bufferPointer[LINK0_INDEX + i];
  } else {
    return NULL_BUFFER;
  }
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BUFFER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
void sli_legacy_buffer_manager_set_buffer_link(sli_buffer_manager_buffer_t buffer, uint8_t i, sli_buffer_manager_buffer_t newLink)
{
  uint16_t *bufferPointer = expandPointer(buffer);
  if (bufferPointer != NULL) {
    assert(isHeapPointer(bufferPointer));
    assert(isHeapPointer(expandPointer(newLink)));
    bufferPointer[LINK0_INDEX + i] = newLink;
  } else {
    DEBUG("ASSERT : %04X %d %04X\n", buffer, i, newLink);
    assert(false);
  }
}

uint16_t sli_legacy_buffer_manager_buffer_chain_byte_length(sli_buffer_manager_buffer_t buffer, uint8_t link)
{
  uint16_t length = 0;

  while (buffer != NULL_BUFFER) {
    length += sli_legacy_buffer_manager_get_buffer_length(buffer);
    buffer = sli_legacy_buffer_manager_get_buffer_link(buffer, link);
  }

  return length;
}

static uint16_t *asynchronousHeapLimit = NULL;

bool sli_legacy_buffer_manager_set_reserved_buffer_space(uint16_t dataSizeInBytes)
{
  uint16_t totalWords = BYTES_TO_WORDS(dataSizeInBytes);
  assert(asynchronousHeapLimit == NULL);

  if (totalWords < (uint16_t)(emHeapLimit - heapPointer)) {
    asynchronousHeapLimit = emHeapLimit - totalWords;
    return true;
  }
  return false;
}

void sli_legacy_buffer_manager_end_buffer_space_reservation(void)
{
  asynchronousHeapLimit = NULL;
}

// The receive ISR calls this from interrupt context, so we need the ATOMIC
// to protect emHeapPointer.

// Function to check if current execution context permits buffer operation
// Well ... this blows up a lot of our code in AF that call this function.
// These ABSOLUTELY need to be fixed, but some cause more problems than others
// We are replacing buffers to pointer / len pairs piecemeal.

#if defined(SL_ZIGBEE_SCRIPTED_TEST) && !defined(EMBER_STACK_CONNECT) && !defined(MAC_TEST_STACK)
bool sli_zigbee_is_stack_task_or_isr_current_context(void)
{
  return true;
}
#endif //SL_ZIGBEE_SCRIPTED_TEST
#define  BUFFER_ISSUE_DEBUG
#ifdef BUFFER_ISSUE_DEBUG
extern bool sli_zigbee_is_stack_task_or_isr_current_context(void);
#endif // BUFFER_ISSUE_DEBUG

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BUFFER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sli_buffer_manager_buffer_t sli_legacy_buffer_manager_really_allocate_buffer(uint16_t dataSizeInBytes, bool asynchronous)
{
  // We cannot call buffer functions from any other context besides the stack task or ISR
  #if defined(BUFFER_ISSUE_DEBUG) && !defined(EMBER_STACK_CONNECT) && !defined(MAC_TEST_STACK)
  if (!sli_zigbee_is_stack_task_or_isr_current_context()) {
    assert(false);
  }
  #endif //BUFFER_ISSUE_DEBUG
  uint16_t sizeInWords = (OVERHEAD_IN_WORDS + BYTES_TO_WORDS(dataSizeInBytes));
  uint16_t *result = NULL;
  uint16_t *heapLimit = emHeapLimit;

  assert(dataSizeInBytes <= SIZE_MASK);

  bufferTest(
    allocationCount += 1;
    if (allocationCount == allocationToFail) {
    return NULL_BUFFER;
  }
    );

  ATOMIC(
    uint16_t available;
    if (asynchronousHeapLimit == NULL) {
    available = heapLimit - heapPointer;
  } else if (asynchronous) {
    available = asynchronousHeapLimit - heapPointer;
  } else {
    available = heapLimit - asynchronousHeapLimit;
    assert(available != 0u);          // check that heap reservation was large enough
  }

    if (sizeInWords <= available) {
    result = heapPointer;

    heapPointer += sizeInWords;

    if (asynchronousHeapLimit != NULL
        && !asynchronous) {
      asynchronousHeapLimit += sizeInWords;
    }

    result[SIZE_INDEX] = dataSizeInBytes;
    result[LINK0_INDEX] = NULL_BUFFER;
    result[LINK1_INDEX] = NULL_BUFFER;
    // anything other than the UNUSED_MEMORY is OK
    result[NEW_LOCATION_INDEX] = (~UNUSED_MEMORY) & 0xFFFFu;
  }
    )

  bufferTest(
    if (result != NULL) {
    emMallocedContents[result - emHeapBase] = testMalloc(dataSizeInBytes);
  }
    );
  bufferUsage(noteAllocation(dataSizeInBytes); );
#ifndef SL_ZIGBEE_SCRIPTED_TEST
  if (result == NULL) {
    #ifdef SL_ZIGBEE_STACK_IP
    emApiCounterHandler(SL_ZIGBEE_COUNTER_BUFFER_ALLOCATION_FAIL, 1);
    #endif //#ifdef SL_ZIGBEE_STACK_IP

    #ifdef SL_ZIGBEE_STACK_ZIGBEE
    emApiCounterHandler(SL_ZIGBEE_COUNTER_ALLOCATE_PACKET_BUFFER_FAILURE, 1);
    #endif //#ifdef SL_ZIGBEE_STACK_ZIGBEE
  }
#endif
  return compressPointer(result);
}

sli_buffer_manager_buffer_t sli_legacy_buffer_manager_allocate_indirect_buffer(uint8_t *contents,
                                                                               void    *freePtr,
                                                                               uint16_t length)
{
  sli_buffer_manager_buffer_t buffer =
    sli_legacy_buffer_manager_really_allocate_buffer(INDIRECT_BUFFER_DATA_SIZE_IN_BYTES, false);
  uint16_t *bufferPointer = expandPointer(buffer);
  if (bufferPointer != NULL) {
    bufferPointer[INDIRECT_BUFFER_LENGTH_INDEX] = length;
    *((uint8_t **) (bufferPointer + INDIRECT_BUFFER_POINTER_INDEX))
      = contents;
    bufferPointer[NEW_LOCATION_INDEX] |= INDIRECT_BIT;
    *((void **) (bufferPointer + INDIRECT_BUFFER_OBJ_REF_INDEX)) = freePtr;
    return buffer;
  } else {
    return NULL_BUFFER;
  }
}

void* sl_legacy_buffer_manager_get_object_ref_from_buffer(sli_buffer_manager_buffer_t b)
{
  uint16_t *bufferPointer = expandPointer(b);

  if (bufferPointer != NULL) {
    if (!isIndirect(bufferPointer)) {
      return NULL;
    }
    return *((void **) (bufferPointer + INDIRECT_BUFFER_OBJ_REF_INDEX));
  }
  return NULL;
}

sli_buffer_manager_buffer_t sli_legacy_buffer_manager_really_fill_buffer(const uint8_t *contents, uint16_t length, bool async)
{
  sli_buffer_manager_buffer_t buffer = sli_legacy_buffer_manager_really_allocate_buffer(length, async);

  if (contents != NULL
      && buffer != NULL_BUFFER) {
    (void) memmove(sli_legacy_buffer_manager_get_buffer_pointer(buffer),
                   contents,
                   length);
  }
  return buffer;
}

//----------------------------------------------------------------

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BUFFER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
void sli_802154phy_phy_to_mac_queue_add(sli_buffer_manager_buffer_t newTail)
{
  ATOMIC(sli_legacy_buffer_manager_buffer_queue_add(&phyToMacQueue, newTail); )
}

sli_buffer_manager_buffer_t sli_802154phy_phy_to_mac_queue_remove_head(void)
{
  sli_buffer_manager_buffer_t result;
  ATOMIC(result = sli_legacy_buffer_manager_buffer_queue_remove_head(&phyToMacQueue); )
  return result;
}

bool sli_802154phy_phy_to_mac_queue_is_empty(void)
{
  return phyToMacQueue == NULL_BUFFER;
}

void sli_legacy_buffer_manager_empty_phy_to_mac_queue(void)
{
  phyToMacQueue = NULL_BUFFER;
}

#ifdef MAC_DUAL_PRESENT
void sli_legacy_buffer_manager_multi_phy_to_mac_queue_add(uint8_t mac_index, sli_buffer_manager_buffer_t newTail)
{
  if (mac_index) {
    ATOMIC(sli_legacy_buffer_manager_buffer_queue_add(&phyToMacQueue2, newTail); )
  } else {
    ATOMIC(sli_legacy_buffer_manager_buffer_queue_add(&phyToMacQueue, newTail); )
  }
}

sli_buffer_manager_buffer_t sli_legacy_buffer_manager_multi_phy_to_mac_queue_remove_head(uint8_t mac_index)
{
  sli_buffer_manager_buffer_t result;
  if (mac_index) {
    ATOMIC(result = sli_legacy_buffer_manager_buffer_queue_remove_head(&phyToMacQueue2); )
  } else {
    ATOMIC(result = sli_legacy_buffer_manager_buffer_queue_remove_head(&phyToMacQueue); )
  }
  return result;
}

bool sli_legacy_buffer_manager_multi_phy_to_mac_queue_is_empty(uint8_t mac_index)
{
  if (mac_index) {
    return phyToMacQueue2 == NULL_BUFFER;
  } else {
    return phyToMacQueue == NULL_BUFFER;
  }
}

void sli_legacy_buffer_manager_multi_empty_phy_to_mac_queue(uint8_t mac_index)
{
  if (mac_index) {
    phyToMacQueue2 = NULL_BUFFER;
  } else {
    phyToMacQueue = NULL_BUFFER;
  }
}
#endif

sli_buffer_manager_buffer_t sli_legacy_buffer_manager_fill_string_buffer(const uint8_t *contents)
{
  return (contents == NULL
          ? NULL_BUFFER
          : sli_legacy_buffer_manager_fill_buffer(contents, strlen((char const *)contents) + 1u));
}

#ifdef SL_ZIGBEE_TEST
uint8_t sli_legacy_buffer_manager_packet_buffer_count(void)
{
  return 0;
}

void printPacketBuffers(sli_buffer_manager_buffer_t buffer)
{
  uint8_t i;
  uint8_t j = 0;
  uint8_t length = sli_legacy_buffer_manager_get_buffer_length(buffer);

  for (i = 0; i < length; i++) {
    fprintf(stderr, "%c%02X",
            (i = 0
                 ? '['
                 : ' '),
            sli_legacy_buffer_manager_get_buffer_pointer(buffer)[j]);
  }

  fprintf(stderr, "]\n");
}

void simPrintBytes(char *prefix, uint8_t *bytes, uint16_t count)
{
  uint16_t i;

  fprintf(stderr, "%s", prefix);
  for (i = 0; i < count; i++) {
    fprintf(stderr, " %02X", bytes[i]);
  }
  fprintf(stderr, "\n");
}

void simPrintBuffer(char *prefix, sli_buffer_manager_buffer_t buffer)
{
  simPrintBytes(prefix, sli_legacy_buffer_manager_get_buffer_pointer(buffer), sli_legacy_buffer_manager_get_buffer_length(buffer));
}

#endif // SL_ZIGBEE_TEST

//----------------------------------------------------------------
// Utilities used by the buffer malloc code.

// Given a pointer, return the buffer whose data pointer it is.

sli_buffer_manager_buffer_t sli_legacy_buffer_manager_buffer_pointer_to_buffer(uint16_t *bufferPointer)
{
#ifdef SL_ZIGBEE_TEST
  if (sli_legacy_buffer_manager_use_malloc) {
    if (bufferPointer == NULL) {
      return NULL_BUFFER;
    } else {
      uint16_t i;
      uint16_t limit = emHeapLimit - emHeapBase;
      for (i = 0; i < limit; i++) {
        if (emMallocedContents[i] == bufferPointer) {
          expandPointer(i + 1);
          return i + 1;
        }
      }
      return NULL_BUFFER;
    }
  }
#endif
  if (sli_legacy_buffer_manager_points_into_heap(bufferPointer)) {
    return compressPointer(bufferPointer - OVERHEAD_IN_WORDS);
  } else {
    return NULL_BUFFER;
  }
}

// Returns the buffer following the given one, which may in fact be
// no buffer at all.

sli_buffer_manager_buffer_t sli_legacy_buffer_manager_following_buffer(sli_buffer_manager_buffer_t buffer)
{
  uint16_t *bufferPointer = expandPointer(buffer);
  if (bufferPointer != NULL) {
    uint16_t *next = bufferPointer + getSizeInWords(bufferPointer);

    // This would allow the current buffer to be reclaimed immediately.
    // Having this be done as a side effect of finding the following
    // buffer would likely lead to obscure bugs.
    //
    //  ATOMIC(
    //   if (next == heapPointer) {
    //      heapPointer = bufferPointer;
    //      next = NULL;
    //   }
    //         )

    return compressPointer(next);
  }
  return NULL_BUFFER;
}

// Set the length of the first buffer so that it includes the second,
// which must immediately follow the first in the heap.

void sli_legacy_buffer_manager_merge_buffers(sli_buffer_manager_buffer_t first, sli_buffer_manager_buffer_t second)
{
  uint16_t *bufferPointer = expandPointer(first);
  if (bufferPointer != NULL) {
    uint16_t oldLength = getDataSize(bufferPointer);
    uint16_t addedLength = getSizeInBytes(expandPointer(second));
    uint16_t newLength = (oldLength
                          + (oldLength & 1u)
                          + addedLength);
    assert(sli_legacy_buffer_manager_following_buffer(first) == second);
    bufferTest(testFree(emMallocedContents[bufferPointer - emHeapBase]);
               emMallocedContents[bufferPointer - emHeapBase]
                 = testMalloc(newLength);
               testFree(emMallocedContents[expandPointer(second) - emHeapBase]);
               emMallocedContents[expandPointer(second) - emHeapBase]
                 = NULL; );
    sizeWord(bufferPointer) = newLength;
  }
}

// Split 'buffer' in two, leaving the first buffer with 'newLength'
// bytes.  Returns the new buffer, or NULL_BUFFER if there is not
// enough space left over to create a buffer.

sli_buffer_manager_buffer_t sli_legacy_buffer_manager_split_buffer(sli_buffer_manager_buffer_t buffer, uint16_t newLength)
{
  uint16_t *bufferPointer = expandPointer(buffer);
  if (bufferPointer != NULL) {
    uint16_t oldLength = getDataSize(bufferPointer);
    uint16_t oldLengthEven = oldLength + (oldLength & 1u);
    uint16_t newLengthEven = newLength + (newLength & 1u);
    uint16_t leftoverBytes = oldLengthEven - newLengthEven;
    uint16_t *leftoverPointer = (bufferPointer
                                 + OVERHEAD_IN_WORDS
                                 + BYTES_TO_WORDS(newLength));

    assert(isHeapPointer(bufferPointer)
           && newLength <= oldLength);

    if (newLength == oldLength
        || leftoverBytes <= OVERHEAD_IN_BYTES) {
      return NULL_BUFFER;
    } else {
      sizeWord(bufferPointer) = newLength;
      (void) memset(leftoverPointer, 0, OVERHEAD_IN_BYTES);
      sizeWord(leftoverPointer) = leftoverBytes - OVERHEAD_IN_BYTES;
      bufferTest(testFree(emMallocedContents[bufferPointer - emHeapBase]);
                 emMallocedContents[bufferPointer - emHeapBase]
                   = testMalloc(newLength);
                 emMallocedContents[leftoverPointer - emHeapBase]
                   = testMalloc(leftoverBytes); );
      return compressPointer(leftoverPointer);
    }
  }
  return NULL_BUFFER;
}

// A utility for marking a buffer via a pointer to its contents.
void sli_legacy_buffer_manager_mark_buffer_pointer(void **pointerLoc)
{
  sli_buffer_manager_buffer_t buffer = sli_legacy_buffer_manager_buffer_pointer_to_buffer(*pointerLoc);
  if (buffer != NULL_BUFFER) {
    uint16_t *bufferPointer = expandPointer(buffer);
    if (bufferPointer != NULL) {
      assert(!isIndirect(bufferPointer));
      sli_legacy_buffer_manager_mark_buffer(&buffer);
      if (!sli_legacy_buffer_manager_use_malloc) {
        // Can't use sli_legacy_buffer_manager_get_buffer_pointer() to update *pointerLoc because it checks
        // INDIRECT_BIT and this may not have been copied to the new location yet.
        bufferPointer = expandPointerNoCheck(buffer);
        assert(isHeapPointer(bufferPointer));
        *pointerLoc = (uint8_t *) (bufferPointer + OVERHEAD_IN_WORDS);
      }
    }
  }
}

sl_status_t sl_legacy_buffer_manager_really_append_to_linked_buffers(sli_buffer_manager_buffer_t *buffer,
                                                                     uint8_t *contents,
                                                                     uint16_t length,
                                                                     bool reallyAppend)
{
  uint16_t *bufferPointer = expandPointer(*buffer);
  uint16_t oldLength;
  uint16_t *next;

  assert(isHeapPointer(bufferPointer));
  if (isIndirect(bufferPointer)) {
    bufferPointer[INDIRECT_BUFFER_LENGTH_INDEX] += length;
    return SL_STATUS_OK;
  }
  oldLength = sli_legacy_buffer_manager_get_buffer_length(*buffer);
  next = nextBuffer(bufferPointer);
  DECLARE_INTERRUPT_STATE;
  DISABLE_INTERRUPTS();

  if (next == heapPointer) {
    if (sli_legacy_buffer_manager_buffer_bytes_remaining() < length) {
      RESTORE_INTERRUPTS();
      return SL_STATUS_NO_MORE_RESOURCE;
    }

    bufferTest(
      uint8_t * old_pointer = (uint8_t *)emMallocedContents[bufferPointer - emHeapBase];
      uint8_t *new_pointer = (uint8_t *)testMalloc(oldLength + length);
      if (sli_legacy_buffer_manager_use_malloc) {
      memcpy(new_pointer, old_pointer, oldLength);
    }
      testFree(old_pointer);
      emMallocedContents[bufferPointer - emHeapBase] = (uint16_t *) new_pointer;
      );

    //We're at the end
    sizeWord(bufferPointer) = oldLength + length;
    if (reallyAppend) {
      (void) memcpy(sli_legacy_buffer_manager_get_buffer_pointer(*buffer) + oldLength, contents, length);
    } else {
      //memset(sli_legacy_buffer_manager_get_buffer_pointer(*buffer)+oldLength,0,length);
    }
    heapPointer = nextBuffer(bufferPointer);
    RESTORE_INTERRUPTS();
    return SL_STATUS_OK;
  } else {
    RESTORE_INTERRUPTS();
    sli_buffer_manager_buffer_t newBuffer = sli_legacy_buffer_manager_allocate_buffer(sli_legacy_buffer_manager_get_buffer_length(*buffer) + length);
    if (newBuffer == NULL_BUFFER) {
      return SL_STATUS_NO_MORE_RESOURCE;
    }

    (void) memcpy(sli_legacy_buffer_manager_get_buffer_pointer(newBuffer), sli_legacy_buffer_manager_get_buffer_pointer(*buffer), sli_legacy_buffer_manager_get_buffer_length(*buffer));
    if (reallyAppend) {
      (void) memcpy(sli_legacy_buffer_manager_get_buffer_pointer(newBuffer) + sli_legacy_buffer_manager_get_buffer_length(*buffer), contents, length);
    } else {
      (void) memset(sli_legacy_buffer_manager_get_buffer_pointer(newBuffer) + sli_legacy_buffer_manager_get_buffer_length(*buffer), 0, length);
    }

    sli_legacy_buffer_manager_set_payload_link(newBuffer, sli_legacy_buffer_manager_get_payload_link(*buffer));

    *buffer = newBuffer;
    return SL_STATUS_OK;
  }
}

//------------------------------------------------------------------------------
// These are wrappers for the subset of functionality we currently expose to the
// application layer.

#if defined(EMBER_STACK_CONNECT) || defined(MAC_TEST_STACK)
EmberBuffer emberAllocateBuffer(uint16_t dataSizeInBytes)
{
  return sli_legacy_buffer_manager_allocate_buffer(dataSizeInBytes);
}

void emberMarkBuffer(EmberBuffer *buffer)
{
  sli_legacy_buffer_manager_mark_buffer(buffer);
}

uint8_t *emberGetBufferPointer(EmberBuffer buffer)
{
  return sli_legacy_buffer_manager_get_buffer_pointer(buffer);
}

uint16_t emberGetBufferLength(EmberBuffer buffer)
{
  return sli_legacy_buffer_manager_get_buffer_length(buffer);
}

uint16_t emberGetAvailableBufferMemory(void)
{
  return sli_legacy_buffer_manager_buffer_bytes_remaining();
}
#endif

//------------------------------------------------------------------------------
// Stubs

#ifndef SL_ZIGBEE_STACK_IP
void sl_legacy_buffer_manager_free_memory_for_packet_handler(void *objectRef)
{
  (void)objectRef;
}
#endif
