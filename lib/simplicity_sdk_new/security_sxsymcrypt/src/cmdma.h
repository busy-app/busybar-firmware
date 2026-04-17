/** Cryptomaster DMA (descriptors and control) and hardware detection
 *
 * @copyright Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#ifndef CMDMA_HEADER_FILE
#define CMDMA_HEADER_FILE

#ifndef SX_COMPATIBILTY_STORAGE
#define SX_COMPATIBILTY_STORAGE
#endif


#include "../include/sxsymcrypt/internal.h"
#include "sl_code_classification.h"

#define DMATAG_BYPASS (0)
#define DMATAG_BA411 (1)
#define DMATAG_BA412 (2)
#define DMATAG_BA413 (3)
#define DMATAG_BA417 (4)
#define DMATAG_BA418 (5)
#define DMATAG_BA415 (6)
#define DMATAG_BA416 (7)
#define DMATAG_BA421 (0x0A)
#define DMATAG_BA419 (0x0B)
#define DMATAG_BA423 (0x0D)
#define DMATAG_BA422 (0x0E)
#define DMATAG_BA424 (0x0F)
#define DMATAG_CONFIG(offset) ((1<<4) | (offset << 8))

//can be 0, 1 or 2
#define DMATAG_DATATYPE(x) (x << 6)

#define DMATAG_DATATYPE_HEADER (1<<6)
#define DMATAG_DATATYPE_REFERENCE (3<<6)
#define DMATAG_LAST (1<<5)
#define DMATAG_INVALID_BYTES_MASK 0x1F
#define DMATAG_INVALID_BYTES_OFFSET 8
#define DMATAG_IGN(sz) ((sz) << DMATAG_INVALID_BYTES_OFFSET)

#define DMA_LAST_DESCRIPTOR ((struct sxdesc *)1)
#define DMA_CONST_ADDR (1 << 28)
#define DMA_REALIGN (1 << 29)
#define DMA_DISCARD (1 << 30)

#define DMA_BUS_FETCHER_ERROR_MASK (1 << 2)
#define DMA_BUS_PUSHER_ERROR_MASK  (1 << 5)


/** A cryptomaster DMA descriptor */
struct sxdesc {
    char *addr;
    struct sxdesc *next;
    uint32_t sz;
    uint32_t dmatag;
};

#define UPDATE_LASTDESC_TAG(dmactl, tag)\
    do {\
        struct sxdesc *d = (struct sxdesc*)(dmactl).d - 1;\
        while(d->next) {\
            d--;\
        }\
        d->dmatag |= tag;\
    } while (0)

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
void sx_add_indesc(struct sx_dmactl *dma, char *baddr, size_t bsz, uint32_t tag);

#define ADD_CFGDESC(dmactl, baddr, bsz, tag)\
    do {\
        sx_add_indesc(&(dmactl), sx_map_usrdatain(((char*)baddr), bsz), \
            (bsz) | DMA_REALIGN, tag);\
    } while (0)

#define ADD_INDESC_PRIV(dmactl, offset, bsz, tag)\
    do {\
        sx_add_indesc(&(dmactl), (dmactl).mapped + (offset), \
            (bsz) | DMA_REALIGN, tag);\
    } while (0)

#define ADD_INDESC_PRIV_RAW(dmactl, offset, bsz, tag)\
    do {\
        sx_add_indesc(&(dmactl), (dmactl).mapped + (offset), bsz, tag);\
    } while (0)


#define ADD_RAW_INDESC(dmactl, baddr, bsz, tag)\
    do {\
        sx_add_indesc(&(dmactl), sx_map_usrdatain(((char*)baddr), bsz), \
            bsz, tag);\
    } while (0)


#define ADD_INDESC_IGN(dmactl, baddr, bsz, ignsz, tag)\
    do {\
        sx_add_indesc(&(dmactl), sx_map_usrdatain(((char*)baddr), bsz), \
            (bsz) | DMA_REALIGN, (tag) | DMATAG_IGN(ignsz));\
    } while (0)

#define ALIGN_SZA(sz, msk) (((sz)+(msk)) & ~(msk))
#define ADD_EMPTY_INDESC(dmactl, ignb, tag)\
    do {\
        sx_add_indesc(&(dmactl), (dmactl).mapped, \
            (ignb) | DMA_REALIGN, \
            (tag) | DMATAG_IGN(ignb & DMATAG_INVALID_BYTES_MASK));\
    } while (0)

#define ADD_INDESCA(dmactl, baddr, bsz, tag, msk)\
    do {\
        uint32_t asz = ALIGN_SZA(bsz, msk);\
        sx_add_indesc(&(dmactl), sx_map_usrdatain(((char*)baddr), bsz), \
            asz | DMA_REALIGN, tag | DMATAG_IGN(asz - (bsz)));\
    } while (0)

#define ADD_INDESC_BITS(dmactl, baddr, bsz, tag, msk, bitsz)\
    do {\
        size_t bitmask = (msk << 3) | 0x7;\
        size_t validbitsz = bitsz & bitmask;\
        if (validbitsz == 0)\
            validbitsz = bitmask + 1;\
        uint32_t asz = ALIGN_SZA(bsz, msk);\
        sx_add_indesc(&(dmactl), sx_map_usrdatain(((char*)baddr), bsz), \
            asz | DMA_REALIGN, (tag) | DMATAG_IGN(validbitsz - 1));\
    } while (0)

#define SET_LAST_DESC_IGN(dmactl, bsz, msk) \
    do {\
        size_t ign =  ALIGN_SZA(bsz, msk) - bsz;\
        struct sxdesc *d = (struct sxdesc*)(dmactl).d;\
        (d - 1)->dmatag |= DMATAG_IGN(ign);\
        (d - 1)->sz = ((d-1)->sz + ign) | DMA_REALIGN;\
    } while (0)

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
void sx_add_outdesc(struct sx_dmactl *dmactl, char *p, size_t bsz);

#define ADD_DISCARDDESC(dmactl, bsz) sx_add_outdesc(&(dmactl), 0, (bsz) | DMA_DISCARD)

#define ADD_OUTDESCA(dmactl, baddr, bsz, msk)\
    do {\
        uint32_t asz = ALIGN_SZA(bsz, msk);\
        sx_add_outdesc(&(dmactl), sx_map_usrdataout((char*)(baddr), bsz), bsz);\
        if (asz - (bsz)) ADD_DISCARDDESC(dmactl, (asz - (bsz)));\
    } while (0)

#define ADD_RAW_OUTDESC(dmactl, baddr, bsz)\
    do {\
        sx_add_outdesc(&(dmactl), sx_map_usrdataout((char*)(baddr), bsz), bsz);\
    } while (0)

#define ADD_OUTDESC_PRIV(dmactl, offset, bsz, msk)\
    do {\
        uint32_t asz = ALIGN_SZA(bsz, msk);\
        sx_add_outdesc(&(dmactl), (dmactl).mapped + offset, bsz);\
        if (asz - (bsz)) ADD_DISCARDDESC(dmactl, (asz - (bsz)));\
    } while (0)


#define DMA_MAX_SZ (1u << 24)


struct sx_dmactl;

/** Prepare for a new command/operation over DMA */
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
void sx_cmdma_newcmd(struct sx_dmactl *dma, union sxdescmem *d, size_t dsz,
    uint32_t cmd, uint32_t tag);


/** Prepare for a new DMA operation */
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
void sx_cmdma_newdma(struct sx_dmactl *dma, union sxdescmem *d, size_t dsz);


/** Start input/fetcher DMA at indescs and output/pusher DMA at outdescs */
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
void sx_cmdma_start(struct sx_dmactl *dma, size_t privsz, union sxdescmem *d);


/** Return how the DMA is doing.
 *
 * Possible return values are:
 *  - SX_ERR_HW_PROCESSING: The DMA transfers are going on.
 *  - SX_OK: The DMA transfers finished successfully.
 *  - SX_ERR_DMA_FAILED: The DMA engine failed.
 */
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
int sx_cmdma_check(struct sx_dmactl *dma);


/** Return a bitmask of instances which include hardware from 'mask'
 *
 * The mask is compared to the register listing all included hardware
 * offload modules.
 *
 * Each bit of the returned compatibility bitmask represents a
 * cryptomaster instance with its index matching the bit position.
 * The compatibility bitmask often cached by the caller and used with
 * sx_cmdma_find_available() or sx_cmdma_filter_compatible().
 */
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
unsigned int sx_cmdma_list_compatible(unsigned int mask);


/** Return a bitmask of filtered cryptomaster instances that meet requirements.
 *
 * This function will filter, based on 'capsmask', the cryptomaster instances
 * defined by the bitmask 'compatible'.
 * 'capsaddr' is the register offset in each cryptomaster instance that needs
 * to be read in order to get the hardware capabilities.
 * 'capsmask' is the bitmask of the capabilities that need to be found in the
 * hardware capabilities read at 'capsaddr'.
 *
 * @remark - 'compatible' should be generated with sx_cmdma_list_compatible()
 */
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
unsigned int sx_cmdma_filter_compatible(unsigned int compatible,
    uint32_t capsaddr, uint32_t capsmask);


static inline
int sx_cmdma_has_room(struct sx_dmactl *dma, unsigned int indescs,
    unsigned int outdescs)
{
    return sizeof(struct sxdesc) * (outdescs + indescs) <= dma->dsz;
}

/** Soft-reset of the DMA
 */
void sx_cmdma_reset(struct sx_regs *regs);
#endif
