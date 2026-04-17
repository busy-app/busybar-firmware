/***************************************************************************//**
 * @file
 * @brief NVM3 object handling functions
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

#include "nvm3_object.h"
#include "nvm3_utils.h"
#include <string.h>

#define NVM3_OBJ_LEN_SIZE                7U          //  7 bits
#define NVM3_OBJ_LEN_MASK                ((1U << NVM3_OBJ_LEN_SIZE) - 1U)
#define NVM3_OBJ_LLEN_SIZE               26U         // 26 bits: large obj length
#define NVM3_OBJ_LLEN_MASK               ((1U << NVM3_OBJ_LLEN_SIZE) - 1U)
#define NVM3_OBJ_BCCB_SIZE               5U          //  5 bits: small obj berger code
#define NVM3_OBJ_BCCB_MASK               ((1U << NVM3_OBJ_BCCB_SIZE) - 1U)
#define NVM3_OBJ_LBCCB_SIZE              6U          //  6 bits: large obj berger code
#define NVM3_OBJ_LBCCB_MASK              ((1U << NVM3_OBJ_LBCCB_SIZE) - 1U)
#define NVM3_OBJ_F_SIZE                  2U          //  2 bits: fragment field
#define NVM3_OBJ_F_MASK                  ((1U << NVM3_OBJ_F_SIZE) - 1U)
#define NVM3_OBJ_U_SIZE                  3U          //  3 bits: unused field
#define NVM3_OBJ_U_MASK                  ((1U << NVM3_OBJ_U_SIZE) - 1U)
#define NVM3_OBJ_KEY_OFFSET              (NVM3_OBJ_LEN_SIZE)
#define NVM3_OBJ_F_OFFSET                (NVM3_OBJ_LEN_SIZE + NVM3_KEY_SIZE)
#define NVM3_OBJ_U_OFFSET                (NVM3_OBJ_LEN_SIZE + NVM3_KEY_SIZE + NVM3_OBJ_F_SIZE)
#define NVM3_OBJ_BCCB_OFFSET             (NVM3_OBJ_LEN_SIZE + NVM3_KEY_SIZE)
#define NVM3_OBJ_LBCCB_OFFSET            (NVM3_OBJ_LLEN_SIZE)

//****************************************************************************

__STATIC_INLINE uint32_t hdrSmallGetBergerCode(nvm3_ObjHdrSmall_t *oh)
{
  return ((oh->oh1 >> NVM3_OBJ_BCCB_OFFSET) & NVM3_OBJ_BCCB_MASK);
}

__STATIC_INLINE uint32_t hdrLargeGetBergerCode(nvm3_ObjHdrLarge_t *oh)
{
  return ((oh->oh2 >> NVM3_OBJ_LBCCB_OFFSET) & NVM3_OBJ_LBCCB_MASK);
}

__STATIC_INLINE nvm3_ObjType_t toObjType(uint32_t value)
{
  int type = (int)value;
  return (nvm3_ObjType_t)type;
}

__STATIC_INLINE nvm3_ObjFragType_t toObjFragType(uint32_t value)
{
  int type = (int)value;
  return (nvm3_ObjFragType_t)type;
}

__STATIC_INLINE nvm3_ObjType_t hdrGetType(nvm3_ObjHdrSmall_t *oh)
{
  return toObjType(oh->oh1 & NVM3_OBJ_LEN_MASK);
}

__STATIC_INLINE bool hdrIsLarge(nvm3_ObjHdrSmall_t *oh)
{
  nvm3_ObjType_t objTyp = hdrGetType(oh);
  return ((objTyp == objTypeCounterLarge) || (objTyp == objTypeDataLarge));
}

//****************************************************************************

/*** Initialize object header */
size_t nvm3_objHdrInit(nvm3_ObjHdrLargePtr_t oh, nvm3_ObjectKey_t key, nvm3_ObjType_t objType,
                       size_t len, bool isLarge, nvm3_ObjFragType_t fragTyp)
{
  uint8_t BCCB = 0;

  oh->oh1 = (key & NVM3_KEY_MASK) << NVM3_OBJ_KEY_OFFSET;
  oh->oh2 = (len & NVM3_OBJ_LLEN_MASK);
  switch (objType) {
    case objTypeCounterLarge:
    /* Intented fall-through */
    case objTypeDataLarge:
      oh->oh1 |= (uint32_t)objType;
      oh->oh1 |= ((uint32_t)NVM3_OBJ_U_MASK << NVM3_OBJ_U_OFFSET);
      oh->oh1 |= ((uint32_t)fragTyp & NVM3_OBJ_F_MASK) << NVM3_OBJ_F_OFFSET;
      break;
    case objTypeCounterSmall:
    /* Intented fall-through*/
    case objTypeDeleted:
      oh->oh1 |= (uint32_t)objType;
      break;
    default:
      oh->oh1 |= ((len + (uint32_t)objTypeDataSmall) & NVM3_OBJ_LEN_MASK);
      break;
  }
  if (isLarge) {
    nvm3_utilsComputeBergerCode(&BCCB, &oh->oh1, 32);
    nvm3_utilsComputeBergerCode(&BCCB, &oh->oh2, NVM3_OBJ_LLEN_SIZE);
    oh->oh2 |= ((uint32_t)BCCB & NVM3_OBJ_LBCCB_MASK) << NVM3_OBJ_LBCCB_OFFSET;
  } else {
    nvm3_utilsComputeBergerCode(&BCCB, &oh->oh1, (uint8_t)(NVM3_OBJ_LEN_SIZE + NVM3_KEY_SIZE));
    oh->oh1 |= ((uint32_t)BCCB & NVM3_OBJ_BCCB_MASK) << NVM3_OBJ_BCCB_OFFSET;
  }

  return nvm3_objHdrLen(isLarge);
}

size_t nvm3_objHdrLen(bool isLarge)
{
  return isLarge ? NVM3_OBJ_HEADER_SIZE_LARGE : NVM3_OBJ_HEADER_SIZE_SMALL;
}

bool nvm3_objHdrValidateSmall(nvm3_ObjHdrSmallPtr_t objHdrSmall)
{
  uint8_t codReq = 0;
  uint8_t codAct;
  bool res = false;

  nvm3_utilsComputeBergerCode(&codReq, &objHdrSmall->oh1, (uint8_t)(NVM3_OBJ_LEN_SIZE + NVM3_KEY_SIZE));
  codAct = (uint8_t)hdrSmallGetBergerCode(objHdrSmall);
  if (codReq == codAct) {
    res = true;
  }

  return res;
}

bool nvm3_objHdrValidateLarge(nvm3_ObjHdrLargePtr_t objHdrLarge)
{
  uint8_t codReq = 0;
  uint8_t codAct;
  bool res = false;

  nvm3_utilsComputeBergerCode(&codReq, &objHdrLarge->oh1, 32);
  nvm3_utilsComputeBergerCode(&codReq, &objHdrLarge->oh2, NVM3_OBJ_LLEN_SIZE);
  codAct = (uint8_t)hdrLargeGetBergerCode(objHdrLarge);
  if (codReq == codAct) {
    res = true;
  }

  return res;
}

bool nvm3_objHdrGetErased(nvm3_ObjHdrSmallPtr_t objHdrSmall)
{
  return objHdrSmall->oh1 == 0xFFFFFFFFU;
}

nvm3_ObjFragType_t nvm3_objHdrGetFragTyp(nvm3_ObjHdrSmallPtr_t objHdrSmall)
{
  nvm3_ObjFragType_t fragTyp = toObjFragType((objHdrSmall->oh1 >> NVM3_OBJ_F_OFFSET) & NVM3_OBJ_F_MASK);

  return hdrIsLarge(objHdrSmall) ? fragTyp : fragTypeNone;
}

nvm3_ObjectKey_t nvm3_objHdrGetKey(nvm3_ObjHdrSmallPtr_t objHdrSmall)
{
  return (nvm3_ObjectKey_t)((objHdrSmall->oh1 >> NVM3_OBJ_KEY_OFFSET) & NVM3_KEY_MASK);
}

bool nvm3_objHdrGetHdrIsLarge(nvm3_ObjHdrSmallPtr_t objHdrSmall)
{
  return hdrIsLarge(objHdrSmall);
}

size_t nvm3_objHdrGetHdrLen(nvm3_ObjHdrSmallPtr_t objHdrSmall)
{
  return nvm3_objHdrLen(hdrIsLarge(objHdrSmall));
}

size_t nvm3_objHdrGetDatLen(nvm3_ObjHdrLargePtr_t objHdrLarge)
{
  size_t len;

  if (hdrIsLarge((nvm3_ObjHdrSmallPtr_t)objHdrLarge)) {
    len = objHdrLarge->oh2 & NVM3_OBJ_LLEN_MASK;
  } else {
    len = objHdrLarge->oh1 & NVM3_OBJ_LEN_MASK;
    len = (len > (size_t)objTypeDataSmall) ? (len - (size_t)objTypeDataSmall) : 0U;
  }

  return len;
}

nvm3_ObjType_t nvm3_objHdrGetType(nvm3_ObjHdrSmallPtr_t objHdrSmall)
{
  return hdrGetType(objHdrSmall);
}

void nvm3_objInit(nvm3_ObjPtr_t obj, nvm3_ObjPtr_t objAdr)
{
  // The sizeof(nvm3_Obj_t) is dependent on the page size.
  (void)memset(obj, 0, nvm3_objHandleSize);
  obj->key = NVM3_KEY_INVALID;
  obj->objAdr = objAdr;
  obj->isValid = false;
  obj->totalLen = 0;
  obj->nextObjAdr = NVM3_OBJ_PTR_INVALID;
  obj->isFragmented = false;
  obj->frag.isFirstFragFound = false;
  obj->frag.isLastFragFound = false;
  obj->frag.idx = 0;
}

nvm3_ObjType_t nvm3_objGroupToType(nvm3_ObjGroup_t objGroup, bool hdrIsLarge)
{
  nvm3_ObjType_t objType;

  if (objGroup == objGroupDeleted) {
    objType = objTypeDeleted;
  } else if (objGroup == objGroupCounter) {
    objType = hdrIsLarge ? objTypeCounterLarge : objTypeCounterSmall;
  } else {
    objType = hdrIsLarge ? objTypeDataLarge : objTypeDataSmall;
  }

  return objType;
}

nvm3_ObjGroup_t nvm3_objTypeToGroup(nvm3_ObjType_t objType)
{
  nvm3_ObjGroup_t objGroup;

  switch (objType) {
    case objTypeCounterSmall:
      objGroup = objGroupCounter;
      break;
    case objTypeCounterLarge:
      objGroup = objGroupCounter;
      break;
    case objTypeDataLarge:
      objGroup = objGroupData;
      break;
    case objTypeRes_1:
    case objTypeRes_2:
    case objTypeRes_3:
      objGroup = objGroupUnknown;
      break;
    case objTypeDeleted:
      objGroup = objGroupDeleted;
      break;
    default:
      objGroup = objGroupData;
      break;
  }

  return objGroup;
}
