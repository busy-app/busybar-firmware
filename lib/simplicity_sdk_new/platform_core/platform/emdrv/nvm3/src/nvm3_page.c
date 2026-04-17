/***************************************************************************//**
 * @file
 * @brief NVM3 page handling functions
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

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "nvm3_page.h"
#include "nvm3_object.h"
#include "nvm3_utils.h"
#include "nvm3_trace.h"

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

//============================================================================

#define H1_MAGIC_SHIFT          16          // The magic shift
#define H1_MAGIC_V1             0xB29AU     // The magic marker
#define H1_VERSION_V1           0x01U       // The first version of NVM3
#define H4_BAD_MASK             0xffff0000U // The BAD mask
#define H4_BAD_SHIFT            16          // The BAD shift
#define H4_BAD_GOOD             0xFFFFU     // The page is good
#define H4_BAD_NOTGOOD          0x0000U     // The page is not good
#define H4_EIP_MASK             0xffffU     // The EIP mask
#define H4_EIP_SET              0xA5A5U     // The page is scheduled for erase
#define H4_EIP_CLEAR            0xFFFFU     // The page is not sceduled for erase
#define H5_DEVICE_FAMILY_MASK   0x07FFU
#define H5_WRITESIZE_SHIFT      11
#define H5_WRITESIZE_MASK       0x0800U
#define H5_MEMORYMAPPED_SHIFT   12
#define H5_MEMORYMAPPED_MASK    0x1000U
#define H5_PAGESIZE_SHIFT       13
#define H5_PAGESIZE_MASK        0xE000U
#define H5_DEVINFO_MASK         (H5_DEVICE_FAMILY_MASK | H5_WRITESIZE_MASK | H5_MEMORYMAPPED_MASK | H5_PAGESIZE_MASK)
#if defined(NVM3_SECURITY)
#define H5_SECTYPE_AEAD_SHIFT   16          // The AEAD shift
#define H5_SECTYPE_AEAD_MASK    0x10000U    // The AEAD mask
#define H5_SECTYPE_ENC_SHIFT    17          // The encryption shift
#define H5_SECTYPE_ENC_MASK     0x20000U    // The encryption mask
#endif

//============================================================================

typedef enum {
  eraseCntNormal,
  eraseCntInverted
} EraseCnt_t;

//============================================================================

__STATIC_INLINE bool pageHdrErased(nvm3_PageHdr_t *pageHdr)
{
  return (pageHdr->data[0] == 0xFFFFFFFFU);
}

__STATIC_INLINE bool pageHdrMagicAndVersion(nvm3_PageHdr_t *pageHdr)
{
  uint16_t magic;
  uint16_t version;

  magic = (uint16_t)(pageHdr->data[0] >> H1_MAGIC_SHIFT);
  version = (uint16_t)pageHdr->data[0];
  return (magic == H1_MAGIC_V1) && (version == H1_VERSION_V1);
}

__STATIC_INLINE bool pageHdrIsPageBad(nvm3_PageHdr_t *pageHdr)
{
  uint32_t badPage = (pageHdr->data[3] & H4_BAD_MASK) >> H4_BAD_SHIFT;

  return badPage != H4_BAD_GOOD;
}

__STATIC_INLINE bool pageHdrIsEraseInProgress(nvm3_PageHdr_t *pageHdr)
{
  uint16_t EIP = (pageHdr->data[3]);

  return EIP != H4_EIP_CLEAR;
}

__STATIC_INLINE uint32_t pageHdrGetCounterBcod(uint32_t h)
{
  return (h >> NVM3_PAGE_COUNTER_SIZE) & NVM3_PAGE_BCCB_MASK;
}

__STATIC_INLINE uint32_t pageHdrGetCounterVal(uint32_t h, EraseCnt_t level)
{
  uint32_t cnt = (level == eraseCntInverted) ? ~h : h;
  return cnt & NVM3_PAGE_COUNTER_MASK;
}

__STATIC_INLINE uint32_t pageHdrCounterMake(uint32_t cnt, EraseCnt_t level)
{
  uint8_t  BCCB;

  if (level == eraseCntInverted) {
    cnt = ~cnt;
  }
  cnt &= NVM3_PAGE_COUNTER_MASK;
  BCCB = 0;
  nvm3_utilsComputeBergerCode(&BCCB, &cnt, NVM3_PAGE_COUNTER_SIZE);
  return ((uint32_t)BCCB << NVM3_PAGE_COUNTER_SIZE) | cnt;
}

// The page size field has a value from 0 to 7 describing page sizes from 512
// to 65536 bytes. Note: The page size must be a power of 2.
static uint16_t pageSizeToField(size_t pageSize)
{
  uint16_t field = 0U;

  pageSize /= 512U;
  for (;; ) {
    pageSize /= 2U;
    if (pageSize == 0U) {
      break;
    }
    field++;
  }

  return field;
}

//============================================================================
#if defined(NVM3_SECURITY)
sl_status_t nvm3_pageHeaderWrite(const nvm3_HalHandle_t *hal, nvm3_HalPtr_t pageAdr, uint32_t eraseCnt, nvm3_HalInfo_t *halInfo, nvm3_SecurityType_t secType)
#else
sl_status_t nvm3_pageHeaderWrite(const nvm3_HalHandle_t *hal, nvm3_HalPtr_t pageAdr, uint32_t eraseCnt, nvm3_HalInfo_t *halInfo)
#endif
{
  nvm3_PageHdr_t pageHdr;
  uint32_t devInfo;
  uint32_t formatInfo;
  size_t ofs;
  nvm3_HalPtr_t adr;
  sl_status_t staWrite;
  sl_status_t sta = SL_STATUS_OK;

  // Create header
  devInfo = ((pageSizeToField(halInfo->pageSize) << H5_PAGESIZE_SHIFT) & H5_PAGESIZE_MASK)
            + (((halInfo->memoryMapped) << H5_MEMORYMAPPED_SHIFT) & H5_MEMORYMAPPED_MASK)
            + (((halInfo->writeSize) << H5_WRITESIZE_SHIFT) & H5_WRITESIZE_MASK)
            + ((halInfo->deviceFamilyPartNumber) & H5_DEVICE_FAMILY_MASK);
#if defined(NVM3_SECURITY)
  formatInfo = ((secType == NVM3_SECURITY_AEAD) ? 0 : 1) + 0xFFFEU;
#else
  formatInfo = 0xFFFFU;
#endif
  pageHdr.data[0] = (H1_MAGIC_V1 << H1_MAGIC_SHIFT) | H1_VERSION_V1;
  pageHdr.data[1] = pageHdrCounterMake(eraseCnt, eraseCntNormal);
  pageHdr.data[2] = pageHdrCounterMake(eraseCnt, eraseCntInverted);
  pageHdr.data[3] = 0xffffffffU;
  pageHdr.data[4] = (formatInfo << 16) | devInfo;
  //printf("-> hdrWr: adr=%p, cnt=%u, 1=0x%08x, 2=0x%08x\n", pageAdr, eraseCnt, pageHdr.data[1], pageHdr.data[2]);

  // Write header in the following order: H5 -> H3 -> H2 -> H1
  // Write H5
  ofs = 4 * sizeof(uint32_t);
  adr = (nvm3_HalPtr_t)((size_t)pageAdr + ofs);
  staWrite = nvm3_halWriteWords(hal, adr, &(pageHdr.data[4]), 1);
  if (staWrite != SL_STATUS_OK) {
    sta = staWrite;
  }

  // Write H4
  // Do not write 0xffffffff

  // Write H3
  ofs = 2 * sizeof(uint32_t);
  adr = (nvm3_HalPtr_t)((size_t)pageAdr + ofs);
  staWrite = nvm3_halWriteWords(hal, adr, &(pageHdr.data[2]), 1);
  if (staWrite != SL_STATUS_OK) {
    sta = staWrite;
  }

  // Write H2
  ofs = 1 * sizeof(uint32_t);
  adr = (nvm3_HalPtr_t)((size_t)pageAdr + ofs);
  staWrite = nvm3_halWriteWords(hal, adr, &(pageHdr.data[1]), 1);
  if (staWrite != SL_STATUS_OK) {
    sta = staWrite;
  }

  // Write H1
  ofs = 0;
  adr = (nvm3_HalPtr_t)((size_t)pageAdr + ofs);
  staWrite = nvm3_halWriteWords(hal, adr, &(pageHdr.data[0]), 1);
  if (staWrite != SL_STATUS_OK) {
    sta = staWrite;
  }

  return sta;
}

/*** Mark page as bad page */
void nvm3_pageSetBad(const nvm3_HalHandle_t *hal, nvm3_HalPtr_t pageAdr)
{
  uint32_t  h4Rd;
  uint32_t  h4Wr;
  nvm3_HalPtr_t adr;

  nvm3_tracePrint(NVM3_TRACE_LEVEL_WARNING, "nvm3_pageSetBad, pageAdr=0x%p.\n", pageAdr);

  adr = (nvm3_HalPtr_t)((size_t)pageAdr + NVM3_PAGE_H4_OFFSET);
  nvm3_halReadWords(hal, adr, &h4Rd, 1);

  h4Wr = h4Rd & ~H4_BAD_MASK;
  h4Wr |= (H4_BAD_NOTGOOD << H4_BAD_SHIFT);
  (void)nvm3_halWriteWords(hal, adr, &h4Wr, 1);

  // There is no recovery from a write error at this point.
}

sl_status_t nvm3_pageSetEip(const nvm3_HalHandle_t *hal, nvm3_HalPtr_t pageAdr)
{
  uint32_t  h4Rd;
  uint32_t  h4Wr;
  nvm3_HalPtr_t adr;
  sl_status_t sta = SL_STATUS_OK;

  nvm3_tracePrint(NVM3_TRACE_LEVEL_LOW, "nvm3_pageSetEip, pageAdr=0x%p.\n", pageAdr);

  adr = (nvm3_HalPtr_t)((size_t)pageAdr + NVM3_PAGE_H4_OFFSET);
  nvm3_halReadWords(hal, adr, &h4Rd, 1);
  if ((h4Rd & H4_EIP_MASK) == H4_EIP_CLEAR) {
    h4Wr = (h4Rd & ~H4_EIP_MASK);
    h4Wr |= H4_EIP_SET;
    sta = nvm3_halWriteWords(hal, adr, &h4Wr, 1);
  }

  return sta;
}

uint32_t nvm3_pageGetEraseCnt(nvm3_PageHdr_t *pageHdr)
{
  uint8_t BCCB;

  BCCB = 0;
  nvm3_utilsComputeBergerCode(&BCCB, &(pageHdr->data[1]), NVM3_PAGE_COUNTER_SIZE);
  if (BCCB == pageHdrGetCounterBcod(pageHdr->data[1])) {
    return pageHdrGetCounterVal(pageHdr->data[1], eraseCntNormal);
  }

  BCCB = 0;
  nvm3_utilsComputeBergerCode(&BCCB, &(pageHdr->data[2]), NVM3_PAGE_COUNTER_SIZE);
  if (BCCB == pageHdrGetCounterBcod(pageHdr->data[2])) {
    return pageHdrGetCounterVal(pageHdr->data[2], eraseCntInverted);
  }

  //printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ERROR: getEraseCount failed!\n");
  return NVM3_ERASE_COUNT_INVALID;
}

nvm3_PageState_t nvm3_pageGetState(nvm3_PageHdr_t *pageHdr)
{
  nvm3_PageState_t pageState;
  uint32_t eraseCnt = NVM3_ERASE_COUNT_INVALID;
  bool pageHdrValid;

  pageHdrValid = pageHdrMagicAndVersion(pageHdr);

  if (pageHdrValid) {
    eraseCnt = nvm3_pageGetEraseCnt(pageHdr);
  }

  if (eraseCnt != NVM3_ERASE_COUNT_INVALID) {
    if (pageHdrIsPageBad(pageHdr)) {
      pageState = nvm3_PageStateBad;
    } else if (pageHdrIsEraseInProgress(pageHdr)) {
      pageState = nvm3_PageStateGoodEip;
    } else {
      pageState = nvm3_PageStateGood;
    }
  } else {
    if (pageHdrErased(pageHdr)) {
      pageState = nvm3_PageStateInvalidErased;
    } else {
      pageState = nvm3_PageStateInvalidUnknown;
    }
  }

  return pageState;
}

bool nvm3_pageStateIsGood(nvm3_PageState_t pageState)
{
  return ((pageState == nvm3_PageStateGood) || (pageState == nvm3_PageStateGoodEip));
}

bool nvm3_pageStateIsInvalid(nvm3_PageState_t pageState)
{
  return ((pageState == nvm3_PageStateInvalidErased) || (pageState == nvm3_PageStateInvalidUnknown));
}

nvm3_ObjPtr_t nvm3_pageGetFirstObj(nvm3_HalPtr_t pageAdr)
{
  return (nvm3_ObjPtr_t)((size_t)pageAdr + NVM3_PAGE_HEADER_SIZE);
}

#if defined(NVM3_SECURITY)
sl_status_t nvm3_pageErase(const nvm3_HalHandle_t *hal, nvm3_HalPtr_t pageAdr, uint32_t eraseCnt, nvm3_HalInfo_t *halInfo, nvm3_SecurityType_t secType)
#else
sl_status_t nvm3_pageErase(const nvm3_HalHandle_t *hal, nvm3_HalPtr_t pageAdr, uint32_t eraseCnt, nvm3_HalInfo_t *halInfo)
#endif
{
  sl_status_t sta;

  nvm3_tracePrint(NVM3_TRACE_LEVEL_LOW, "nvm3_pageErase: adr=0x%p, eraseCnt=%lu.\n", pageAdr, eraseCnt);

  // Erase
  sta = nvm3_halPageErase(hal, pageAdr);
  if (sta == SL_STATUS_OK) {
    // Create new page header
#if defined(NVM3_SECURITY)
    sta = nvm3_pageHeaderWrite(hal, pageAdr, eraseCnt, halInfo, secType);
#else
    sta = nvm3_pageHeaderWrite(hal, pageAdr, eraseCnt, halInfo);
#endif
    if (sta != SL_STATUS_OK) {
      nvm3_tracePrint(NVM3_TRACE_LEVEL_WARNING, "erasePage: adr=0x%p, Write hdr ERROR, mark page as BAD.\n", pageAdr);
      nvm3_pageSetBad(hal, pageAdr);
    }
  } else {
    // Erasure failed, mark page as BAD
    nvm3_tracePrint(NVM3_TRACE_LEVEL_WARNING, "erasePage: adr=0x%p, Erase ERROR, page is marked as BAD.\n", pageAdr);
    nvm3_pageSetBad(hal, pageAdr);
  }

  return sta;
}

#if defined(NVM3_SECURITY)
/***************************************************************************//**
 * Get NVM3 security type.
 *
 * @param[in]  pageHdr     Page header of a valid page.
 *
 * @return                 Returns NVM3 security type.
 ******************************************************************************/
nvm3_SecurityType_t nvm3_pageGetSecType(nvm3_PageHdr_t *pageHdr)
{
  nvm3_SecurityType_t secType = NVM3_SECURITY_INVALID;

  if (((pageHdr->data[4] & H5_SECTYPE_AEAD_MASK) >> H5_SECTYPE_AEAD_SHIFT) == 0) {
    secType = NVM3_SECURITY_AEAD;
  }

  return secType;
}
#endif
/** @endcond */
