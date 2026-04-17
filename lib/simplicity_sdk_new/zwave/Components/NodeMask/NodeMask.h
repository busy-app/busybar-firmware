/**
 * @file
 * @brief Functions used to manipulate bits (Node ID) in a NodeMask array.
 * @copyright 2019 Silicon Laboratories Inc.
 */
#ifndef _ZW_NODEMASK_API_H_
#define _ZW_NODEMASK_API_H_

#include <ZW_transport_api.h>

/**
 * @addtogroup Components
 * @{
 * @addtogroup NodeMask
 * @{
 */

/// Number of nodes per node mask byte
#define NODEMASK_NODES_PER_BYTE   8

/// Max length of a node mask
#define MAX_NODEMASK_LENGTH       (ZW_MAX_NODES / NODEMASK_NODES_PER_BYTE)
/// Max length of Long Range node mask
#define MAX_LR_NODEMASK_LENGTH    (ZW_MAX_NODES_LR / NODEMASK_NODES_PER_BYTE)

typedef uint8_t NODE_MASK_TYPE[MAX_NODEMASK_LENGTH];
typedef uint8_t LR_NODE_MASK_TYPE[MAX_LR_NODEMASK_LENGTH];

/***
 * Set the node bit in a node bitmask
 * @param pMask Pointer to Nodemask list that should be set
 * @param bNodeID Node ID that should be set in the mask
 */
extern void
ZW_NodeMaskSetBit(
  uint8_t* pMask,
  node_id_t bNodeID);

/**
 * Clear the node bit in a node bitmask
 * @param pMask Pointer to Nodemask list that should be set
 * @param bNodeID Node to set in nodemask
 */
extern void
ZW_NodeMaskClearBit(
  uint8_t* pMask,
  node_id_t bNodeID);

/**
 * Clear all bits in a nodemask
 * @param pMask Pointer to Nodemask list that should be cleared
 * @param bLength Length of nodemask to be cleared
 */
extern void
ZW_NodeMaskClear(
  uint8_t* pMask,
  uint8_t bLength);

/**
 * Check and count number of bit that is set in a nodemask
 * @param pMask Pointer to nodemask that should be counted
 * @param bLength Length of nodemask to count
 * @return Number of bits set in nodemask
 */
extern uint8_t
ZW_NodeMaskBitsIn(
  const uint8_t* pMask,
  uint8_t bLength);

/**
 * Check if a node is in a nodemask
 * @param pMask Pointer to nodemask to check for bNode
 * @param bNode Bit number that should be checked
 * @return 0 if not in nodemask, non-zero if found in nodemask
 */
extern uint8_t
ZW_NodeMaskNodeIn(
  const uint8_t* pMask,
  node_id_t bNode);

/**
 * Find the next NodeIndex that is set in a nodemask
 * WARNING: this function is the only one working with indexes and not node IDs
 * @param currentNodeIndex Last NodeId found (0 for first call)
 * @param pMask Pointer to Nodemask list that should be searched
 * @return Next NodeId from the nodemask if found, or 0 if not found.
 */
extern node_id_t
ZW_NodeMaskGetNextNodeIndex(
  node_id_t currentNodeIndex,
  uint8_t* pMask);

/**
 * @} // addtogroup NodeMask
 * @} // addtogroup Components
 */
#endif /* _ZW_NODEMASK_API_H_ */
