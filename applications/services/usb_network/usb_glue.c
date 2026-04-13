#include <tusb.h>

#include "usb_network_i.h"

#define TAG "UsbGlue"

bool tud_network_recv_cb(const uint8_t* src, uint16_t size) {
    if(size != 0) {
#if(ETH_PAD_SIZE != 0)
        size += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif
        struct pbuf* p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);

        if(!p) {
            FURI_LOG_T(TAG, "cannot receive frame, pbuf_alloc failed");
            tud_network_recv_renew();
            return true;
        }

#if(ETH_PAD_SIZE != 0)
        pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

        for(struct pbuf* q = p; q != NULL && size > 0; q = q->next) {
            /* Read enough bytes to fill this pbuf in the chain.
             * The available data in the pbuf is given by the q->len variable. */
            memcpy(q->payload, src, size < q->len ? size : q->len);
            src += q->len;
            size -= q->len;
        }

#if(ETH_PAD_SIZE != 0)
        pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

        if(usb_network) { /* Check if netif is initialized */
            err_t err = usb_network->netif.input(p, &usb_network->netif);
            if(err != ERR_OK) {
                FURI_LOG_W(TAG, "netif->input failed with error: %d", err);
                pbuf_free(p); /* Free pbuf if input failed */
            }
        } else {
            FURI_LOG_E(TAG, "usb_network->netif is NULL in recv_cb");
            pbuf_free(p); /* Free pbuf as it cannot be processed */
        }
        tud_network_recv_renew();
    }

    return true;
}

uint16_t tud_network_xmit_cb(uint8_t* dst, void* ref, uint16_t arg) {
    struct pbuf* p = (struct pbuf*)ref;
    UNUSED(arg);

    uint16_t res = pbuf_copy_partial(p, dst, p->tot_len, 0);
    return res;
}

void tud_network_init_cb(void) {
}
