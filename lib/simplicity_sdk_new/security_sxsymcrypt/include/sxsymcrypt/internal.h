/*
 * @copyright Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 *
 * SPDX-License-Identifier: Zlib
 */
#ifndef INTERNAL_HEADER_FILE
#define INTERNAL_HEADER_FILE

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/* Extra descriptors needed for multifeed */

/* Ensure backward compatibility */
#ifdef SX_EXTRA_IN_DESCS
#ifdef SX_HASH_EXTRA_DESC_IN
#error "SX_EXTRA_IN_DESCS and SX_HASH_EXTRA_DESC_IN cannot be set at same time"
#else
#define SX_HASH_EXTRA_DESC_IN SX_EXTRA_IN_DESCS
#endif
#endif

#ifndef SX_HASH_EXTRA_DESC_IN
#define SX_HASH_EXTRA_DESC_IN 0
#endif

#ifndef SX_MAC_EXTRA_DESC_IN
#define SX_MAC_EXTRA_DESC_IN 0
#endif

#ifndef SX_BLKCIPHER_EXTRA_DESC_IN
#define SX_BLKCIPHER_EXTRA_DESC_IN 0
#endif

#ifndef SX_BLKCIPHER_EXTRA_DESC_OUT
#define SX_BLKCIPHER_EXTRA_DESC_OUT 0
#endif

#ifndef SX_AEAD_DATA_EXTRA_DESC_IN
#define SX_AEAD_DATA_EXTRA_DESC_IN 0
#endif

#ifndef SX_AEAD_DATA_EXTRA_DESC_OUT
#define SX_AEAD_DATA_EXTRA_DESC_OUT 0
#endif

#ifndef SX_AEAD_AAD_EXTRA_DESC_IN
#define SX_AEAD_AAD_EXTRA_DESC_IN 0
#endif

#define SX_AEAD_EXTRA_DESC_IN (SX_AEAD_DATA_EXTRA_DESC_IN + \
            SX_AEAD_AAD_EXTRA_DESC_IN)
#define SX_AEAD_EXTRA_DESC_OUT (SX_AEAD_DATA_EXTRA_DESC_OUT)

/* Private memory sizes */
#ifndef SX_HASH_PRIV_SZ
#define SX_HASH_PRIV_SZ 344
#endif

#define SX_BLKCIPHER_PRIV_SZ (16)
#define SX_AEAD_PRIV_SZ (70)


struct sx_regs;
struct sx3gppalg;
struct sx_aead_cmdma_cfg;
struct sxhashalg;

union sxdescmem {
    uintptr_t addr;
    uint32_t v;
};

#define SX_DESCMEMSZ(incnt, outcnt) (((incnt)+(outcnt)) * (2 + (2 * sizeof(uint32_t)) / sizeof(union sxdescmem)))


/** Input and output descriptors and related state for cmdma */
struct sx_dmaslot {
    uint32_t cfg;
    uint32_t pad;
};


/** DMA controller
 *
 * For internal use only. Don't access directly.
 */
struct sx_dmactl {
    struct sx_regs *regs;
    size_t dsz;
    union sxdescmem *d;
    char *mapped;
    struct sx_dmaslot dmamem;
};

/** Key reference
 *
 * Used for making a reference to a key stored in memory or to a key selected
 * by an identifier.
 * Created by sx_keyref_load_material() or sx_keyref_load_by_id(). Used by blkcipher and
 * aead creation functions.
 *
 * All members should be considered INTERNAL and may not be accessed directly.
 */
struct sxkeyref {
    const char *key;
    size_t sz;
    uint32_t cfg;
};

/** Data structure
 *
 * Used for allowing users to create arrays with input/output data.
 * Used in the context of multifeed for MAC and block cipher operations.
 */
struct sxdataref {
    const char *data;
    size_t sz;
};

/** An AEAD operation
 *
 * To be used with sx_aead_*() functions.
 *
 * All members should be considered INTERNAL and may not be accessed
 * directly.
 */
struct sxaead {
    const struct sx_aead_cmdma_cfg *cfg;
    const char *expectedtag;
    uint8_t tagsz;
    uint32_t discardaadsz;
    uint32_t datainsz;
    uint64_t dataintotalsz;
    uint64_t totalaadsz;
    uint32_t extraaadsz;
    uint32_t no_exec;
    uint8_t *extraaadptr;
    uint8_t extraaadmem[2 * SX_BLKCIPHER_PRIV_SZ];
    unsigned int compatible;
    const struct sxkeyref *key;
    struct sx_dmactl dma;
    union sxdescmem descs[SX_DESCMEMSZ(9 + SX_AEAD_EXTRA_DESC_IN,
        5 + SX_AEAD_EXTRA_DESC_OUT)];
    uint8_t extramem[SX_AEAD_PRIV_SZ];
};


/** A simple block cipher operation
 *
 * To be used with sx_blkcipher_*() functions.
 *
 * All members should be considered INTERNAL and may not be accessed
 * directly.
 */
struct sxblkcipher {
    const struct sx_blkcipher_cmdma_cfg *cfg;
    unsigned int compatible;
    const struct sxkeyref *key;
    uint32_t textsz;
    uint16_t is_multifeed;
    uint16_t iv_loaded;
    const char *datain_bkp;
    const char *dataout_bkp;
    char state_bkp[SX_BLKCIPHER_PRIV_SZ];
    char output_bkp[SX_BLKCIPHER_PRIV_SZ];
    uint32_t outputsz_bkp;
    char *extradataptr;
    char extradata[2 * SX_BLKCIPHER_PRIV_SZ];
    uint32_t extradatasz;
    uint32_t offset;
    struct sx_dmactl dma;
    union sxdescmem descs[SX_DESCMEMSZ(7 + SX_BLKCIPHER_EXTRA_DESC_IN,
        4 + SX_BLKCIPHER_EXTRA_DESC_OUT)];
    char extramem[SX_BLKCIPHER_PRIV_SZ];
};


/** A simple 3gpp operation
 *
 * To be used with sx_3gpp_*() functions.
 *
 * All members should be considered INTERNAL and may not be accessed
 * directly.
 */
struct sx3gpp {
    const struct sx3gppalg *cfg;
    struct sx_dmactl dma;
    union sxdescmem descs[SX_DESCMEMSZ(5, 2)];
    uint32_t params[2];
    size_t insz;
    size_t outsz;
};


/** A hash operation.
 *
 * To be used with sx_hash_*() functions.
 *
 * All members should be considered INTERNAL and may not be accessed
 * directly.
 */
struct sxhash {
    const struct sxhashalg *algo;
    const struct sx_digesttags *dmatags;
    uint32_t cntindescs;
    size_t totalfeedsz;
    uint32_t feedsz;
    void(*digest)(struct sxhash* c, char *digest);
    struct sx_dmactl dma;
    union sxdescmem descs[SX_DESCMEMSZ(7 + SX_HASH_EXTRA_DESC_IN, 2)];
    uint8_t extramem[SX_HASH_PRIV_SZ];
};


/** A simple MAC(message authentication code) operation
 *
 * To be used with sx_mac_*() functions.
 *
 * All members should be considered INTERNAL and may not be accessed
 * directly.
 */
struct sxmac {
    const struct sx_mac_cmdma_cfg *cfg;
    uint32_t cntindescs;
    uint32_t feedsz;
    int macsz;
    unsigned int compatible;
    const struct sxkeyref *key;
    struct sx_dmactl dma;
    union sxdescmem descs[SX_DESCMEMSZ(5 + SX_MAC_EXTRA_DESC_IN, 2)];
    uint8_t extramem[16];
};


struct sxchannel {
    struct sx_dmactl dma;
    union sxdescmem descs[SX_DESCMEMSZ(1, 2)];
};


/** A operation to load a countermeasures mask into the hardware.
 * This operation is based on channel.
 *
 * To be used with sx_cm_*() functions.
 *
 * All members should be considered INTERNAL and may not be accessed
 * directly.
 */
struct sxcmmask {
    struct sxchannel channel;
};

#ifdef __cplusplus
}
#endif

#endif
