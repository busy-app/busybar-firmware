/***************************************************************************//**
 * @file
 * @brief BT Mesh Common Manufacturing Token Configuration Header
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_BTMESH_CMT_CONFIG_H
#define SL_BTMESH_CMT_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <q SL_BTMESH_CMT_LOGGING_CFG_VAL> Enable logging
// <d> 0
#define SL_BTMESH_CMT_LOGGING_CFG_VAL 0

// <h> Enable the use of settings via Common Manufacturing Token
// <q SL_BTMESH_CMT_USE_UUID> Use UUID provided as a CMT
// <d> 1
#define SL_BTMESH_CMT_USE_UUID 1

// <q SL_BTMESH_CMT_USE_OOB> Use OOB authentication settings provided as a CMT
// <d> 1
#define SL_BTMESH_CMT_USE_OOB 1

// <a.65 SL_BTMESH_CMT_TOKEN_PUBLIC_KEY> The public key of the signed token as a uint8_t array
// <d> {0}
#define SL_BTMESH_CMT_TOKEN_PUBLIC_KEY {0}

// <h> Mesh configuration data settings

// <o SL_BTMESH_CMT_MESH_TOKEN_OFFSET> Offset of Mesh Manufacturing Token
// This value is defined in token_definition.json
// <d> 0x480
#define SL_BTMESH_CMT_MESH_TOKEN_OFFSET 0x480

// <o SL_BTMESH_CMT_MESH_CONFIG_DATA_SIZE> Size of Mesh Configuration Data token in bytes
// <d> 256
#define SL_BTMESH_CMT_MESH_CONFIG_DATA_SIZE  (256u)

// <h> Token data settings
// <o SL_BTMESH_CMT_TAG_TOKEN_VERSION> Mesh token version tag
// Version of the mesh token data format.
// <d> 0x01
#define SL_BTMESH_CMT_TAG_TOKEN_VERSION 0x01

// <o SL_BTMESH_CMT_TAG_UUID> Mesh token UUID tag
// This tag indicates the start of the UUID data field.
// <d> 0xD7
#define SL_BTMESH_CMT_TAG_UUID          0xD7

// <o SL_BTMESH_CMT_TAG_OOB_CONFIG> OOB Capabilities configuration tag
// This tag indicates the start of the OOB capabilities data field.
// <d> 0xD8
#define SL_BTMESH_CMT_TAG_OOB_CONFIG    0xD8

// <o SL_BTMESH_CMT_TAG_PADDING> Padding tag
// This tag indicates the start of padding bytes.
// <d> 0xFF
#define SL_BTMESH_CMT_TAG_PADDING       0xFF

// <o SL_BTMESH_CMT_TAG_END> Marks the end of data
// The terminator, a length-tag-value triplet with zero length.
// <d> 0x00
#define SL_BTMESH_CMT_TAG_END           0x00

// <<< end of configuration section >>>

#endif // SL_BTMESH_CMT_CONFIG_H
