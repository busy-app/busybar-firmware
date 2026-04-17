/***************************************************************************//**
 * @file
 * @brief NVM3 object cache
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

#include "nvm3_cache.h"
#include "nvm3_trace.h"

#if defined(__ICCARM__)
#define SPEED_OPT _Pragma("optimize=speed")
#elif defined(__GNUC__) && defined(__CORTEX_M)
#define SPEED_OPT _Pragma("GCC optimize(\"O3\")")
#else
#define SPEED_OPT
#endif

#define TRACE_LEVEL                 NVM3_TRACE_LEVEL_LOW

#if defined(NVM3_OPTIMIZATION) && (NVM3_OPTIMIZATION == 1)
#include "sl_memory_manager.h"
#endif

//****************************************************************************

// isValid is implemented as a macro as this significantly improves
// speed when using compiler settings that do not inline these functions.
#define isValid(h, idx) (h->entryPtr[idx].key != NVM3_KEY_INVALID)

#if defined(NVM3_OPTIMIZATION) && (NVM3_OPTIMIZATION == 1)
uint32_t *L;
uint32_t *H;
uint32_t *P1;
uint32_t *P2;
uint32_t *K1;
uint32_t *K2;
#endif

static inline nvm3_ObjectKey_t entryGetKey(nvm3_Cache_t *h, size_t idx)
{
  uint32_t tmp = h->entryPtr[idx].key;
  return (tmp & NVM3_KEY_MASK);
}

static inline nvm3_ObjGroup_t entryGetGroup(nvm3_Cache_t *h, size_t idx)
{
  uint32_t tmp = h->entryPtr[idx].key;
  return (nvm3_ObjGroup_t)(tmp >> NVM3_KEY_SIZE);
}

static inline nvm3_ObjPtr_t entryGetPtr(nvm3_Cache_t *h, size_t idx)
{
  return h->entryPtr[idx].ptr;
}

static inline void entrySetKey(nvm3_Cache_t *h, size_t idx, nvm3_ObjectKey_t key)
{
  uint32_t tmp = h->entryPtr[idx].key;
  tmp &= ~NVM3_KEY_MASK;
  tmp |= key;
  h->entryPtr[idx].key = (nvm3_ObjectKey_t)tmp;
}

static inline void entrySetGroup(nvm3_Cache_t *h, size_t idx, nvm3_ObjGroup_t group)
{
  uint32_t tmp = h->entryPtr[idx].key;
  tmp &= NVM3_KEY_MASK;
  tmp |= (group << NVM3_KEY_SIZE);
  h->entryPtr[idx].key = (nvm3_ObjectKey_t)tmp;
}

static inline void entrySetPtr(nvm3_Cache_t *h, size_t idx, nvm3_ObjPtr_t obj)
{
  h->entryPtr[idx].ptr = obj;
}

static inline void setInvalid(nvm3_Cache_t *h, size_t idx)
{
  h->entryPtr[idx].key = NVM3_KEY_INVALID;
  h->entryPtr[idx].ptr = NVM3_OBJ_PTR_INVALID;
}

//****************************************************************************

void nvm3_cacheOpen(nvm3_Cache_t *h, nvm3_CacheEntry_t *ptr, size_t count)
{
  h->entryPtr = ptr;
  h->entryCount = count;
  nvm3_cacheClear(h);
}

void nvm3_cacheClear(nvm3_Cache_t *h)
{
  nvm3_tracePrint(TRACE_LEVEL, "nvm3_cacheClear.\n");

  for (size_t idx = 0; idx < h->entryCount; idx++) {
    setInvalid(h, idx);
  }
  h->overflow = false;
#if defined(NVM3_OPTIMIZATION) && (NVM3_OPTIMIZATION == 1)
  h->usedCount = 0U;
#endif
}

#if defined(NVM3_OPTIMIZATION) && (NVM3_OPTIMIZATION == 1)
/******************************************************************************************************//**
 * Find index of the key in cache using binary search.
 *
 * @param[in]  h      A pointer to NVM3 cache data.
 *
 * @param[in]  key    A 20-bit object identifier.
 *
 * @param[out] idx    A pointer to the location where key idx will be placed.
 *
 * @return            Returns SL_STATUS_OK on success or SL_STATUS_NOT_FOUND on failure.
 *********************************************************************************************************/
static sl_status_t cacheSearch(nvm3_Cache_t *h, nvm3_ObjectKey_t key, size_t *idx)
{
  if (h->usedCount <= 0U) {
    return SL_STATUS_NOT_FOUND;
  }

  size_t low = 0U;
  size_t high = h->usedCount - 1;
  size_t mid = 0U;
  while (low <= high) {
    mid = (low + high) / 2;
    if (entryGetKey(h, mid) == key) {
      *idx = mid;
      return SL_STATUS_OK;
    }
    if (entryGetKey(h, mid) < key) {
      low = mid + 1;
    } else {
      if (mid != 0U) {
        high = mid - 1;
      } else {
        return SL_STATUS_NOT_FOUND;
      }
    }
  }
  return SL_STATUS_NOT_FOUND;
}

/******************************************************************************************************//**
 * Merge two cache subarrays. Merges the two haves arr[low..mid] and arr[mid+1..high] of cache subarrays.
 *
 * @param[in]  h       A pointer to NVM3 cache data.
 *
 * @param[in]  low     Left index of cache subarray.
 *
 * @param[in]  mid     Mid index of cache subarray.
 *
 * @param[in]  high    Right index of cache subarray.
 *
 * @return             Returns SL_STATUS_OK on success or SL_STATUS_ALLOCATION_FAILED on failure.
 *********************************************************************************************************/
static sl_status_t cacheMerge(nvm3_Cache_t *h, uint32_t low, uint32_t mid, uint32_t high)
{
  sl_status_t status = SL_STATUS_OK;

  if ((low > mid) || (mid >= high)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  uint32_t i, j, k;
  uint32_t a1 = mid - low + 1;
  uint32_t a2 = high - mid;

  if ((L != NULL) && (H != NULL) && (P1 != NULL) && (P2 != NULL) && (K1 != NULL) && (K2 != NULL)) {
    // Copy cache key and pointer data into subarrays
    for (i = 0; i < a1; i++) {
      L[i] = entryGetKey(h, low + i);
      P1[i] = (uint32_t)h->entryPtr[low + i].ptr;
      K1[i] = h->entryPtr[low + i].key;
    }
    for (j = 0; j < a2; j++) {
      H[j] = entryGetKey(h, mid + 1 + j);
      P2[j] = (uint32_t)h->entryPtr[mid + 1 + j].ptr;
      K2[j] = h->entryPtr[mid + 1 + j].key;
    }

    // Index of first, second and merged subarrays
    i = 0;
    j = 0;
    k = low;

    // Merge cache subarrays
    while (i < a1 && j < a2) {
      if (L[i] <= H[j]) {
        h->entryPtr[k].key = K1[i];
        h->entryPtr[k].ptr = (uint32_t*)P1[i];
        i++;
      } else {
        h->entryPtr[k].key = K2[j];
        h->entryPtr[k].ptr = (uint32_t*)P2[j];
        j++;
      }
      k++;
    }

    // Copy remaining cache elements of L subarray
    while (i < a1) {
      h->entryPtr[k].key = K1[i];
      h->entryPtr[k].ptr = (uint32_t*)P1[i];
      i++;
      k++;
    }

    // Copy remaining cache elements of H subarray
    while (j < a2) {
      h->entryPtr[k].key = K2[j];
      h->entryPtr[k].ptr = (uint32_t*)P2[j];
      j++;
      k++;
    }
  } else {
    status = SL_STATUS_ALLOCATION_FAILED;
  }

  return status;
}

/******************************************************************************************************//**
 * Sort cache entries using merge sort.
 *
 * @param[in]  h   A pointer to NVM3 cache data.
 *
 * @return         Returns SL_STATUS_OK on success or SL_STATUS_ALLOCATION_FAILED on failure.
 *********************************************************************************************************/
sl_status_t nvm3_cacheSort(nvm3_Cache_t *h)
{
  sl_status_t status = SL_STATUS_OK;
  uint32_t cacheSize = h->usedCount;  // size of cache array
  uint32_t currSize;                  // current size of subarray
  uint32_t leftStart;                 // starting index of left subarray

  // Allocate memory for cache subarrays
  L = sl_malloc(cacheSize * sizeof(uint32_t));
  H = sl_malloc(cacheSize * sizeof(uint32_t));
  P1 = sl_malloc(cacheSize * sizeof(uint32_t));
  P2 = sl_malloc(cacheSize * sizeof(uint32_t));
  K1 = sl_malloc(cacheSize * sizeof(uint32_t));
  K2 = sl_malloc(cacheSize * sizeof(uint32_t));

  // Merge cache subarrays using bottom up approach.
  for (currSize = 1; currSize <= cacheSize - 1; currSize = (currSize * 2U)) {
    // Pick starting point of different subarrays of current size
    for (leftStart = 0; leftStart < cacheSize - 1; leftStart += (currSize * 2U)) {
      // Find ending point of left subarray. mid+1 is starting point of right
      uint32_t mid = SL_MIN(leftStart + currSize - 1, cacheSize - 1);
      uint32_t rightEnd = SL_MIN(leftStart + (2U * currSize - 1), cacheSize - 1);
      // Merge cache subarrays arr[leftStart...mid] & arr[mid+1...rightEnd]
      status = cacheMerge(h, leftStart, mid, rightEnd);
    }
  }

  sl_free(L);
  sl_free(H);
  sl_free(K1);
  sl_free(P1);
  sl_free(K2);
  sl_free(P2);

  return status;
}

/******************************************************************************************************//**
 * Update existing cache entry.
 *
 * @param[in]  h       A pointer to NVM3 cache data.
 *
 * @param[in]  key     A 20-bit object identifier.
 *
 * @param[in]  obj     A pointer to object struct.
 *
 * @param[in]  group   Object group.
 *
 * @return             Returns true on success or false on failure.
 *********************************************************************************************************/
bool nvm3_cacheUpdateEntry(nvm3_Cache_t *h, nvm3_ObjectKey_t key, nvm3_ObjPtr_t obj, nvm3_ObjGroup_t group)
{
  size_t idx = 0;
  bool res = false;

  if ((h->usedCount > 0U) && (h->usedCount <= h->entryCount)) {
    if (cacheSearch(h, key, &idx) == SL_STATUS_OK) {
      if (isValid(h, idx)) {
        entrySetGroup(h, idx, group);
        entrySetPtr(h, idx, obj);
        res = true;
      }
    }
  }
  nvm3_tracePrint(TRACE_LEVEL, "nvm3_cacheUpdateEntry, key=%lu, grp=%u, obj=%p, idx=%u.\n", key, group, obj, idx);
  return res;
}

/******************************************************************************************************//**
 * Add new cache entry
 *
 * @param[in]  h       A pointer to NVM3 cache data.
 *
 * @param[in]  key     A 20-bit object identifier.
 *
 * @param[in]  obj     A pointer to object struct.
 *
 * @param[in]  group   Object type.
 *
 * @return             Returns SL_STATUS_OK on success or SL_STATUS_ALLOCATION_FAILED on failure.
 *********************************************************************************************************/
sl_status_t nvm3_cacheAddEntry(nvm3_Cache_t *h, nvm3_ObjectKey_t key, nvm3_ObjPtr_t obj, nvm3_ObjGroup_t group)
{
  sl_status_t status = SL_STATUS_OK;
  bool cacheSet = false;

  if (h->usedCount < h->entryCount) {
    size_t idx = 0;
    if (h->usedCount > 0U) {
      if (nvm3_cacheGetIdx(h, key, 0U, h->usedCount - 1, &idx) == SL_STATUS_OK) {
        nvm3_cacheOrganize(h, idx);
      }
    }
    setInvalid(h, idx);
    entrySetKey(h, idx, key);
    entrySetGroup(h, idx, group);
    entrySetPtr(h, idx, obj);
    cacheSet = true;
    h->usedCount++;
    nvm3_tracePrint(TRACE_LEVEL, "nvm3_cacheAddEntry(1), key=%lu, grp=%u, obj=%p, idx=%u.\n", key, group, obj, idx);
  }

  // Prioritize data over deleted objects, force an overwrite if possible
  if ((!cacheSet) && (group != objGroupDeleted)) {
    for (size_t idx1 = 0; idx1 < h->entryCount; idx1++) {
      nvm3_ObjGroup_t cacheGroup = entryGetGroup(h, idx1);
      if (cacheGroup == objGroupDeleted) {
        entrySetKey(h, idx1, key);
        entrySetGroup(h, idx1, group);
        entrySetPtr(h, idx1, obj);
        cacheSet = true;
        if (h->usedCount > 1U) {
          status = nvm3_cacheSort(h);
        }
        nvm3_tracePrint(TRACE_LEVEL, "nvm3_cacheAddEntry(2), cache overflow for key=%lu, grp=%u, obj=%p, inserted at idx=%u.\n", key, group, obj, idx1);
        break;
      }
    }
  }

  if (!cacheSet) {
    h->overflow = true;
    nvm3_tracePrint(TRACE_LEVEL, "nvm3_cacheAddEntry(3), cache overflow for key=%lu, grp=%u, obj=%p.\n", key, group, obj);
  }

  return status;
}

/******************************************************************************************************//**
 * Get cache idx to add new entry
 *
 * @param[in]  h       A pointer to NVM3 cache data.
 *
 * @param[in]  key     A 20-bit object identifier.
 *
 * @param[in]  low     Left index of cache subarray.
 *
 * @param[in]  high    Right index of cache subarray.
 *
 * @param[out] idx     A pointer to the location where key idx will be placed.
 *
 * @return             Returns SL_STATUS_OK on success or SL_STATUS_NOT_FOUND on failure.
 *********************************************************************************************************/
sl_status_t nvm3_cacheGetIdx(nvm3_Cache_t *h, nvm3_ObjectKey_t key, size_t low, size_t high, size_t *idx)
{
  if (high <= low) {
    *idx = (key > entryGetKey(h, low)) ? (low + 1) : low;
    return SL_STATUS_OK;
  }
  if (key > entryGetKey(h, high)) {
    *idx = high + 1;
    return SL_STATUS_OK;
  }
  if (key < entryGetKey(h, low)) {
    *idx = low;
    return SL_STATUS_OK;
  }
  size_t mid = (low + high) / 2;
  if (key == entryGetKey(h, mid)) {
    *idx = mid + 1;
    return SL_STATUS_OK;
  }
  if (key > entryGetKey(h, mid)) {
    return nvm3_cacheGetIdx(h, key, mid + 1, high, idx);
  }
  if (mid != 0U) {
    return nvm3_cacheGetIdx(h, key, low, mid - 1, idx);
  } else {
    return SL_STATUS_NOT_FOUND;
  }
}

/******************************************************************************************************//**
 * Arrange cache entries
 *
 * @param[in]  h       A pointer to NVM3 cache data.
 *
 * @param[in]  idx     Index of the key to arrange the cache entries.
 *
 *********************************************************************************************************/
void nvm3_cacheOrganize(nvm3_Cache_t *h, size_t idx)
{
  size_t lastIdx = h->usedCount - 1;
  // Move all cache entries from idx to end
  for (; (lastIdx >= idx) && (lastIdx != 0); lastIdx--) {
    h->entryPtr[lastIdx + 1].key = h->entryPtr[lastIdx].key;
    h->entryPtr[lastIdx + 1].ptr = h->entryPtr[lastIdx].ptr;
  }
  if ((lastIdx == 0) && (idx == 0)) {
    h->entryPtr[lastIdx + 1].key = h->entryPtr[lastIdx].key;
    h->entryPtr[lastIdx + 1].ptr = h->entryPtr[lastIdx].ptr;
  }
}
#endif

#if defined(NVM3_OPTIMIZATION) && (NVM3_OPTIMIZATION == 1)
void nvm3_cacheDelete(nvm3_Cache_t *h, nvm3_ObjectKey_t key)
{
  bool found = false;
  size_t idx = 0;

  if ((h->usedCount > 0U) && (h->usedCount <= h->entryCount)) {
    if (cacheSearch(h, key, &idx) == SL_STATUS_OK) {
      if (isValid(h, idx)) {
        if (entryGetKey(h, idx) == key) {
          setInvalid(h, idx);
          found = true;
          size_t lastIdx = h->usedCount - 1;
          // Arrange cache entries after cache obj deletion
          for (; idx < lastIdx; idx++) {
            h->entryPtr[idx].key = h->entryPtr[idx + 1].key;
            h->entryPtr[idx].ptr = h->entryPtr[idx + 1].ptr;
          }
          if (h->usedCount > 0) {
            h->usedCount--;
          }
        }
      }
    }
  }

  nvm3_tracePrint(TRACE_LEVEL, "nvm3_cacheDelete, key=%lu, found=%d.\n", key, found ? 1 : 0);
  (void)found;
}
#else
void nvm3_cacheDelete(nvm3_Cache_t *h, nvm3_ObjectKey_t key)
{
  bool found = false;

  for (size_t idx = 0; idx < h->entryCount; idx++) {
    if (isValid(h, idx)) {
      if (entryGetKey(h, idx) == key) {
        setInvalid(h, idx);
        found = true;
        break;
      }
    }
  }

  nvm3_tracePrint(TRACE_LEVEL, "nvm3_cacheDelete, key=%lu, found=%d.\n", key, found ? 1 : 0);
  (void)found;
}
#endif

#if defined(NVM3_OPTIMIZATION) && (NVM3_OPTIMIZATION == 1)
nvm3_ObjPtr_t nvm3_cacheGet(nvm3_Cache_t *h, nvm3_ObjectKey_t key, nvm3_ObjGroup_t *group)
{
  nvm3_ObjPtr_t obj = NVM3_OBJ_PTR_INVALID;
#if NVM3_TRACE_PORT
  int tmp = -1;
#endif

  size_t idx = 0;
  if ((h->usedCount > 0U) && (h->usedCount <= h->entryCount)) {
    if (cacheSearch(h, key, &idx) == SL_STATUS_OK) {
      if (isValid(h, idx)) {
        *group = entryGetGroup(h, idx);
        obj = entryGetPtr(h, idx);
#if NVM3_TRACE_PORT
        tmp = (int)idx;
#endif
      }
    }
  }

  nvm3_tracePrint(TRACE_LEVEL, "nvm3_cacheGet, key=%lu, grp=%d, obj=%p, idx=%d.\n", key, (obj != NVM3_OBJ_PTR_INVALID) ? *group : -1, obj, tmp);

  return obj;
}
#else
nvm3_ObjPtr_t nvm3_cacheGet(nvm3_Cache_t *h, nvm3_ObjectKey_t key, nvm3_ObjGroup_t *group)
{
  nvm3_ObjPtr_t obj = NVM3_OBJ_PTR_INVALID;
#if NVM3_TRACE_PORT
  int tmp = -1;
#endif

  for (size_t idx = 0; idx < h->entryCount; idx++) {
    if (isValid(h, idx)) {
      if (entryGetKey(h, idx) == key) {
        *group = entryGetGroup(h, idx);
        obj = entryGetPtr(h, idx);
#if NVM3_TRACE_PORT
        tmp = (int)idx;
#endif
        break;
      }
    }
  }

  nvm3_tracePrint(TRACE_LEVEL, "nvm3_cacheGet, key=%lu, grp=%d, obj=%p, idx=%d.\n", key, (obj != NVM3_OBJ_PTR_INVALID) ? *group : -1, obj, tmp);

  return obj;
}
#endif

#if defined(NVM3_OPTIMIZATION) && (NVM3_OPTIMIZATION == 1)
SPEED_OPT
void nvm3_cacheSet(nvm3_Cache_t *h, nvm3_ObjectKey_t key, nvm3_ObjPtr_t obj, nvm3_ObjGroup_t group)
{
  bool bSet = false;

  // Update existing entry
  size_t idx = 0;
  if ((h->usedCount > 0U) && (h->usedCount <= h->entryCount)) {
    for (idx = 0; idx < h->usedCount; idx++) {
      if (isValid(h, idx)) {
        if (entryGetKey(h, idx) == key) {
          entrySetGroup(h, idx, group);
          entrySetPtr(h, idx, obj);
          nvm3_tracePrint(TRACE_LEVEL, "nvm3_cacheSet(1), key=%lu, grp=%u, obj=%p, idx=%u.\n", key, group, obj, idx);
          return;
        }
      }
    }
  }

  // Add new Entry
  if (h->usedCount < h->entryCount) {
    idx = h->usedCount;
    if (!isValid(h, idx)) {
      entrySetKey(h, idx, key);
      entrySetGroup(h, idx, group);
      entrySetPtr(h, idx, obj);
      bSet = true;
      h->usedCount++;
      nvm3_tracePrint(TRACE_LEVEL, "nvm3_cacheSet(2), key=%lu, grp=%u, obj=%p, idx=%u.\n", key, group, obj, idx);
    }
  }

  // Full, prioritize data over deleted objects, force an overwrite if possible
  if ((!bSet) && (group != objGroupDeleted)) {
    for (size_t idx1 = 0; idx1 < h->entryCount; idx1++) {
      nvm3_ObjGroup_t cacheGroup = entryGetGroup(h, idx1);
      if (cacheGroup == objGroupDeleted) {
        entrySetKey(h, idx1, key);
        entrySetGroup(h, idx1, group);
        entrySetPtr(h, idx1, obj);
        bSet = true;
        nvm3_tracePrint(TRACE_LEVEL, "nvm3_cacheSet(3), cache overflow for key=%lu, grp=%u, obj=%p, inserted at idx=%u.\n", key, group, obj, idx1);
        break;
      }
    }
  }

  if (!bSet) {
    h->overflow = true;
    nvm3_tracePrint(TRACE_LEVEL, "nvm3_cacheSet(4), cache overflow for key=%lu, grp=%u, obj=%p.\n", key, group, obj);
  }
}
#else
SPEED_OPT
void nvm3_cacheSet(nvm3_Cache_t *h, nvm3_ObjectKey_t key, nvm3_ObjPtr_t obj, nvm3_ObjGroup_t group)
{
  bool bSet = false;

  // Update existing entry
  for (size_t idx = 0; idx < h->entryCount; idx++) {
    if (isValid(h, idx)) {
      if (entryGetKey(h, idx) == key) {
        entrySetGroup(h, idx, group);
        entrySetPtr(h, idx, obj);
        nvm3_tracePrint(TRACE_LEVEL, "nvm3_cacheSet(1), key=%lu, grp=%u, obj=%p, idx=%u.\n", key, group, obj, idx);
        return;
      }
    }
  }

  // Add new Entry
  for (size_t idx = 0; idx < h->entryCount; idx++) {
    if (!isValid(h, idx)) {
      entrySetKey(h, idx, key);
      entrySetGroup(h, idx, group);
      entrySetPtr(h, idx, obj);
      bSet = true;
      nvm3_tracePrint(TRACE_LEVEL, "nvm3_cacheSet(2), key=%lu, grp=%u, obj=%p, idx=%u.\n", key, group, obj, idx);
      break;
    }
  }

  // Full, prioritize data over deleted objects, force an overwrite if possible
  if ((!bSet) && (group != objGroupDeleted)) {
    for (size_t idx = 0; idx < h->entryCount; idx++) {
      nvm3_ObjGroup_t cacheGroup = entryGetGroup(h, idx);
      if (cacheGroup == objGroupDeleted) {
        entrySetKey(h, idx, key);
        entrySetGroup(h, idx, group);
        entrySetPtr(h, idx, obj);
        bSet = true;
        nvm3_tracePrint(TRACE_LEVEL, "nvm3_cacheSet(3), cache overflow for key=%lu, grp=%u, obj=%p, inserted at idx=%u.\n", key, group, obj, idx);
        break;
      }
    }
  }

  if (!bSet) {
    h->overflow = true;
    nvm3_tracePrint(TRACE_LEVEL, "nvm3_cacheSet(4), cache overflow for key=%lu, grp=%u, obj=%p.\n", key, group, obj);
  }
}
#endif

void nvm3_cacheScan(nvm3_Cache_t *h, nvm3_CacheScanCallback_t cacheScanCallback, void *user)
{
  bool keepGoing;
  for (size_t idx = 0; idx < h->entryCount; idx++) {
    if (isValid(h, idx)) {
      // Found an object.
      nvm3_ObjectKey_t key = entryGetKey(h, idx);
      nvm3_ObjGroup_t group = entryGetGroup(h, idx);
      nvm3_ObjPtr_t obj = entryGetPtr(h, idx);
      keepGoing = cacheScanCallback(h, key, group, obj, user);
      if (!keepGoing) {
        return;
      }
    }
  }
}
