/*
 * SPDX-License-Identifier: LicenseRef-MSLA
 * Copyright (c) 2024 Silicon Laboratories Inc. (www.silabs.com)
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of the Silicon Labs Master Software License
 * Agreement (MSLA) available at [1].  This software is distributed to you in
 * Object Code format and/or Source Code format and is governed by the sections
 * of the MSLA applicable to Object Code, Source Code and Modified Open Source
 * Code. By using this software, you agree to the terms of the MSLA.
 *
 * [1]: https://www.silabs.com/about-us/legal/master-software-license-agreement
 */
#ifndef PKTBUF_H
#define PKTBUF_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common/int24.h"

/*
 * This module aims to provide a structure used to mutate buffers in place
 * during packet processing. I can been seen as a simplified sk_buff (from the
 * Linux kernel), or as a combination of iobuf_read and iobuf_write.
 */

struct pktbuf {
    uint8_t *buf;
    size_t buf_len;
    size_t offset_head;
    size_t offset_tail;
    bool   err;
};

// WARN: calls to pktbuf_push_*() can invalidate the returned pointer
static inline uint8_t *pktbuf_head(const struct pktbuf *pktbuf)
{
    return pktbuf->buf + pktbuf->offset_head;
}

static inline size_t pktbuf_len(const struct pktbuf *pktbuf)
{
    return pktbuf->offset_tail - pktbuf->offset_head;
}

void pktbuf_init(struct pktbuf *pktbuf, const void *buf, size_t buf_len);
void pktbuf_free(struct pktbuf *pktbuf);

// Use buf = NULL to reserve bytes (0-init)
void pktbuf_push_head(struct pktbuf *pktbuf, const void *buf, size_t buf_len);
void pktbuf_push_head_u8(struct pktbuf *pktbuf, uint8_t val);
void pktbuf_push_head_be16(struct pktbuf *pktbuf, uint16_t val);
void pktbuf_push_head_le16(struct pktbuf *pktbuf, uint16_t val);
void pktbuf_push_head_be24(struct pktbuf *pktbuf, uint24_t val);
void pktbuf_push_head_le24(struct pktbuf *pktbuf, uint24_t val);
void pktbuf_push_head_be32(struct pktbuf *pktbuf, uint32_t val);
void pktbuf_push_head_le32(struct pktbuf *pktbuf, uint32_t val);
void pktbuf_push_head_be64(struct pktbuf *pktbuf, uint64_t val);
void pktbuf_push_head_le64(struct pktbuf *pktbuf, uint64_t val);

// Use buf = NULL to reserve bytes (0-init)
void pktbuf_push_tail(struct pktbuf *pktbuf, const void *buf, size_t buf_len);
void pktbuf_push_tail_u8(struct pktbuf *pktbuf, uint8_t val);
void pktbuf_push_tail_be16(struct pktbuf *pktbuf, uint16_t val);
void pktbuf_push_tail_le16(struct pktbuf *pktbuf, uint16_t val);
void pktbuf_push_tail_be24(struct pktbuf *pktbuf, uint24_t val);
void pktbuf_push_tail_le24(struct pktbuf *pktbuf, uint24_t val);
void pktbuf_push_tail_be32(struct pktbuf *pktbuf, uint32_t val);
void pktbuf_push_tail_le32(struct pktbuf *pktbuf, uint32_t val);
void pktbuf_push_tail_be64(struct pktbuf *pktbuf, uint64_t val);
void pktbuf_push_tail_le64(struct pktbuf *pktbuf, uint64_t val);

void pktbuf_pop_head(struct pktbuf *pktbuf, void *buf, size_t buf_len);
uint8_t  pktbuf_pop_head_u8(struct pktbuf *pktbuf);
uint16_t pktbuf_pop_head_be16(struct pktbuf *pktbuf);
uint16_t pktbuf_pop_head_le16(struct pktbuf *pktbuf);
uint24_t pktbuf_pop_head_be24(struct pktbuf *pktbuf);
uint24_t pktbuf_pop_head_le24(struct pktbuf *pktbuf);
uint32_t pktbuf_pop_head_be32(struct pktbuf *pktbuf);
uint32_t pktbuf_pop_head_le32(struct pktbuf *pktbuf);
uint64_t pktbuf_pop_head_be64(struct pktbuf *pktbuf);
uint64_t pktbuf_pop_head_le64(struct pktbuf *pktbuf);

void pktbuf_pop_tail(struct pktbuf *pktbuf, void *buf, size_t buf_len);
uint8_t  pktbuf_pop_tail_u8(struct pktbuf *pktbuf);
uint16_t pktbuf_pop_tail_be16(struct pktbuf *pktbuf);
uint16_t pktbuf_pop_tail_le16(struct pktbuf *pktbuf);
uint24_t pktbuf_pop_tail_be24(struct pktbuf *pktbuf);
uint24_t pktbuf_pop_tail_le24(struct pktbuf *pktbuf);
uint32_t pktbuf_pop_tail_be32(struct pktbuf *pktbuf);
uint32_t pktbuf_pop_tail_le32(struct pktbuf *pktbuf);
uint64_t pktbuf_pop_tail_be64(struct pktbuf *pktbuf);
uint64_t pktbuf_pop_tail_le64(struct pktbuf *pktbuf);

#endif
