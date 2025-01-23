#include <furi_hal.h>
#include <tusb.h>
#include "furi_hal_usb_i.h"
#include "furi_hal_usb_interface_i.h"
#include "class/net/net_device.h"
#include "device/usbd.h"
#include "device/usbd_pvt.h"
#include "class/net/ncm.h"

#define TAG "USB ETH"

#define NCM_ALIGNMENT             4
#define NCM_IN_NTB_MAX_SIZE       3200
#define NCM_OUT_NTB_MAX_SIZE      3200
#define NCM_MAX_DATAGRAMS_PER_NTB 8

#define XMIT_ALIGN_OFFSET(x) ((NCM_ALIGNMENT - ((x) & (NCM_ALIGNMENT - 1))) & (NCM_ALIGNMENT - 1))

#define NCM_XMIT_NTB_N 1 // Number of NTB buffers for transmission side
#define NCM_RECV_NTB_N 1 // Number of NTB buffers for reception side

#define NTH16_SIGNATURE      0x484D434E
#define NDP16_SIGNATURE_NCM0 0x304D434E
#define NDP16_SIGNATURE_NCM1 0x314D434E

typedef struct {
    uint8_t itf_num;
    uint8_t itf_data_alt;

    uint8_t ep_notif;
    uint8_t ep_in;
    uint8_t ep_out;

    recv_ntb_t* recv_free_ntb[NCM_RECV_NTB_N];
    recv_ntb_t* recv_ready_ntb[NCM_RECV_NTB_N];
    recv_ntb_t* recv_tinyusb_ntb;
    recv_ntb_t* recv_glue_ntb;
    uint16_t recv_glue_ntb_datagram_ndx;

    xmit_ntb_t* xmit_free_ntb[NCM_XMIT_NTB_N];
    xmit_ntb_t* xmit_ready_ntb[NCM_XMIT_NTB_N];
    xmit_ntb_t* xmit_tinyusb_ntb;
    xmit_ntb_t* xmit_glue_ntb;
    uint16_t xmit_sequence;
    uint16_t xmit_glue_ntb_datagram_ndx;

    enum {
        NOTIFICATION_SPEED,
        NOTIFICATION_CONNECTED,
        NOTIFICATION_DONE
    } notification_xmit_state;
    bool notification_xmit_is_running;

    bool tud_network_recv_renew_active;
    bool tud_network_recv_renew_process_again;
} ncm_interface_t;

typedef struct {
    struct {
        TUD_EPBUF_TYPE_DEF(recv_ntb_t, ntb);
    } recv[NCM_RECV_NTB_N];

    struct {
        TUD_EPBUF_TYPE_DEF(xmit_ntb_t, ntb);
    } xmit[NCM_XMIT_NTB_N];

    TUD_EPBUF_TYPE_DEF(ncm_notify_t, epnotif);
} ncm_epbuf_t;

static ncm_interface_t ncm_interface;
CFG_TUD_MEM_SECTION static ncm_epbuf_t ncm_epbuf;

TU_ATTR_ALIGNED(4)
static const ntb_parameters_t ntb_parameters = {
    .wLength = sizeof(ntb_parameters_t),
    .bmNtbFormatsSupported = 0x01, // 16-bit NTB supported
    .dwNtbInMaxSize = CFG_TUD_NCM_IN_NTB_MAX_SIZE,
    .wNdbInDivisor = 1,
    .wNdbInPayloadRemainder = 0,
    .wNdbInAlignment = NCM_ALIGNMENT,
    .wReserved = 0,
    .dwNtbOutMaxSize = CFG_TUD_NCM_OUT_NTB_MAX_SIZE,
    .wNdbOutDivisor = 1,
    .wNdbOutPayloadRemainder = 0,
    .wNdbOutAlignment = NCM_ALIGNMENT,
    .wNtbOutMaxDatagrams = CFG_TUD_NCM_OUT_MAX_DATAGRAMS_PER_NTB,
};

static void notification_xmit(uint8_t rhport, bool force_next) {
    if(!force_next && ncm_interface.notification_xmit_is_running) {
        return;
    }

    if(ncm_interface.notification_xmit_state == NOTIFICATION_SPEED) {
        ncm_notify_t notify_speed_change = {
            .header = {
                .bmRequestType_bit =
                    {.recipient = TUSB_REQ_RCPT_INTERFACE,
                     .type = TUSB_REQ_TYPE_CLASS,
                     .direction = TUSB_DIR_IN},
                .bRequest = CDC_NOTIF_CONNECTION_SPEED_CHANGE,
                .wValue = 0,
                .wIndex = ncm_interface.itf_num,
                .wLength = 8}};
        if(tud_speed_get() == TUSB_SPEED_HIGH) {
            notify_speed_change.downlink = 480000000;
            notify_speed_change.uplink = 480000000;
        } else {
            notify_speed_change.downlink = 12000000;
            notify_speed_change.uplink = 12000000;
        }

        uint16_t notif_len =
            sizeof(notify_speed_change.header) + notify_speed_change.header.wLength;
        ncm_epbuf.epnotif = notify_speed_change;
        usbd_edpt_xfer(rhport, ncm_interface.ep_notif, (uint8_t*)&ncm_epbuf.epnotif, notif_len);

        ncm_interface.notification_xmit_state = NOTIFICATION_CONNECTED;
        ncm_interface.notification_xmit_is_running = true;
    } else if(ncm_interface.notification_xmit_state == NOTIFICATION_CONNECTED) {
        ncm_notify_t notify_connected = {
            .header =
                {
                    .bmRequestType_bit =
                        {.recipient = TUSB_REQ_RCPT_INTERFACE,
                         .type = TUSB_REQ_TYPE_CLASS,
                         .direction = TUSB_DIR_IN},
                    .bRequest = CDC_NOTIF_NETWORK_CONNECTION,
                    .wValue = 1 /* Connected */,
                    .wIndex = ncm_interface.itf_num,
                    .wLength = 0,
                },
        };

        uint16_t notif_len = sizeof(notify_connected.header) + notify_connected.header.wLength;
        ncm_epbuf.epnotif = notify_connected;
        usbd_edpt_xfer(rhport, ncm_interface.ep_notif, (uint8_t*)&ncm_epbuf.epnotif, notif_len);

        ncm_interface.notification_xmit_state = NOTIFICATION_DONE;
        ncm_interface.notification_xmit_is_running = true;
    }
}

//-----------------------------------------------------------------------------
//
// everything about packet transmission (driver -> TinyUSB)
//

/**
 * Put NTB into the transmitter free list.
 */
static void xmit_put_ntb_into_free_list(xmit_ntb_t* free_ntb) {
    if(free_ntb == NULL) { // can happen due to ZLPs
        return;
    }

    for(int i = 0; i < NCM_XMIT_NTB_N; ++i) {
        if(ncm_interface.xmit_free_ntb[i] == NULL) {
            ncm_interface.xmit_free_ntb[i] = free_ntb;
            return;
        }
    }
}

/**
 * Get an NTB from the free list
 */
static xmit_ntb_t* xmit_get_free_ntb(void) {
    for(int i = 0; i < NCM_XMIT_NTB_N; ++i) {
        if(ncm_interface.xmit_free_ntb[i] != NULL) {
            xmit_ntb_t* free = ncm_interface.xmit_free_ntb[i];
            ncm_interface.xmit_free_ntb[i] = NULL;
            return free;
        }
    }
    return NULL;
} // xmit_get_free_ntb

/**
 * Put a filled NTB into the ready list
 */
static void xmit_put_ntb_into_ready_list(xmit_ntb_t* ready_ntb) {
    for(int i = 0; i < NCM_XMIT_NTB_N; ++i) {
        if(ncm_interface.xmit_ready_ntb[i] == NULL) {
            ncm_interface.xmit_ready_ntb[i] = ready_ntb;
            return;
        }
    }
}

/**
 * Get the next NTB from the ready list (and remove it from the list).
 * If the ready list is empty, return NULL.
 */
static xmit_ntb_t* xmit_get_next_ready_ntb(void) {
    xmit_ntb_t* r = NULL;

    r = ncm_interface.xmit_ready_ntb[0];
    memmove(
        ncm_interface.xmit_ready_ntb + 0,
        ncm_interface.xmit_ready_ntb + 1,
        sizeof(ncm_interface.xmit_ready_ntb) - sizeof(ncm_interface.xmit_ready_ntb[0]));
    ncm_interface.xmit_ready_ntb[NCM_XMIT_NTB_N - 1] = NULL;

    return r;
} // xmit_get_next_ready_ntb

/**
 * Transmit a ZLP if required
 *
 * \note
 *    Insertion of the ZLPs is a little bit different then described in the spec.
 *    But the below implementation actually works.  Don't know if this is a spec
 *    or TinyUSB issue.
 *
 * \pre
 *    This must be called from netd_xfer_cb() so that ep_in is ready
 */
static bool xmit_insert_required_zlp(uint8_t rhport, uint32_t xferred_bytes) {
    if(xferred_bytes == 0 || xferred_bytes % CFG_TUD_NET_ENDPOINT_SIZE != 0) {
        return false;
    }

    TU_ASSERT(ncm_interface.itf_data_alt == 1, false);
    TU_ASSERT(!usbd_edpt_busy(rhport, ncm_interface.ep_in), false);

    // start transmission of the ZLP
    usbd_edpt_xfer(rhport, ncm_interface.ep_in, NULL, 0);

    return true;
} // xmit_insert_required_zlp

/**
 * Start transmission if it there is a waiting packet and if can be done from interface side.
 */
static void xmit_start_if_possible(uint8_t rhport) {
    if(ncm_interface.xmit_tinyusb_ntb != NULL) {
        return;
    }
    if(ncm_interface.itf_data_alt != 1) {
        return;
    }
    if(usbd_edpt_busy(rhport, ncm_interface.ep_in)) {
        return;
    }

    ncm_interface.xmit_tinyusb_ntb = xmit_get_next_ready_ntb();
    if(ncm_interface.xmit_tinyusb_ntb == NULL) {
        if(ncm_interface.xmit_glue_ntb == NULL || ncm_interface.xmit_glue_ntb_datagram_ndx == 0) {
            // -> really nothing is waiting
            return;
        }
        ncm_interface.xmit_tinyusb_ntb = ncm_interface.xmit_glue_ntb;
        ncm_interface.xmit_glue_ntb = NULL;
    }

    if(ncm_interface.xmit_glue_ntb_datagram_ndx != 1) {
    }

    // Kick off an endpoint transfer
    usbd_edpt_xfer(
        0,
        ncm_interface.ep_in,
        ncm_interface.xmit_tinyusb_ntb->data,
        ncm_interface.xmit_tinyusb_ntb->nth.wBlockLength);
}

/**
 * check if a new datagram fits into the current NTB
 */
static bool xmit_requested_datagram_fits_into_current_ntb(uint16_t datagram_size) {
    if(ncm_interface.xmit_glue_ntb == NULL) {
        return false;
    }
    if(ncm_interface.xmit_glue_ntb_datagram_ndx >= CFG_TUD_NCM_IN_MAX_DATAGRAMS_PER_NTB) {
        return false;
    }
    if(ncm_interface.xmit_glue_ntb->nth.wBlockLength + datagram_size +
           XMIT_ALIGN_OFFSET(datagram_size) >
       CFG_TUD_NCM_IN_NTB_MAX_SIZE) {
        return false;
    }
    return true;
} // xmit_requested_datagram_fits_into_current_ntb

/**
 * Setup an NTB for the glue logic
 */
static bool xmit_setup_next_glue_ntb(void) {
    if(ncm_interface.xmit_glue_ntb != NULL) {
        // put NTB into waiting list (the new datagram did not fit in)
        xmit_put_ntb_into_ready_list(ncm_interface.xmit_glue_ntb);
    }

    ncm_interface.xmit_glue_ntb = xmit_get_free_ntb(); // get next buffer (if any)
    if(ncm_interface.xmit_glue_ntb == NULL) {
        return false;
    }

    ncm_interface.xmit_glue_ntb_datagram_ndx = 0;

    xmit_ntb_t* ntb = ncm_interface.xmit_glue_ntb;

    // Fill in NTB header
    ntb->nth.dwSignature = NTH16_SIGNATURE;
    ntb->nth.wHeaderLength = sizeof(ntb->nth);
    ntb->nth.wSequence = ncm_interface.xmit_sequence++;
    ntb->nth.wBlockLength = sizeof(ntb->nth) + sizeof(ntb->ndp) + sizeof(ntb->ndp_datagram);
    ntb->nth.wNdpIndex = sizeof(ntb->nth);

    // Fill in NDP16 header and terminator
    ntb->ndp.dwSignature = NDP16_SIGNATURE_NCM0;
    ntb->ndp.wLength = sizeof(ntb->ndp) + sizeof(ntb->ndp_datagram);
    ntb->ndp.wNextNdpIndex = 0;

    memset(ntb->ndp_datagram, 0, sizeof(ntb->ndp_datagram));
    return true;
} // xmit_setup_next_glue_ntb

//-----------------------------------------------------------------------------
//
// all the recv_*() stuff (TinyUSB -> driver -> glue logic)
//

/**
 * Return pointer to an available receive buffer or NULL.
 * Returned buffer (if any) has the size \a CFG_TUD_NCM_OUT_NTB_MAX_SIZE.
 */
static recv_ntb_t* recv_get_free_ntb(void) {
    for(int i = 0; i < NCM_RECV_NTB_N; ++i) {
        if(ncm_interface.recv_free_ntb[i] != NULL) {
            recv_ntb_t* free = ncm_interface.recv_free_ntb[i];
            ncm_interface.recv_free_ntb[i] = NULL;
            return free;
        }
    }
    return NULL;
}

/**
 * Get the next NTB from the ready list (and remove it from the list).
 * If the ready list is empty, return NULL.
 */
static recv_ntb_t* recv_get_next_ready_ntb(void) {
    recv_ntb_t* r = NULL;

    r = ncm_interface.recv_ready_ntb[0];
    memmove(
        ncm_interface.recv_ready_ntb + 0,
        ncm_interface.recv_ready_ntb + 1,
        sizeof(ncm_interface.recv_ready_ntb) - sizeof(ncm_interface.recv_ready_ntb[0]));
    ncm_interface.recv_ready_ntb[NCM_RECV_NTB_N - 1] = NULL;

    return r;
}

/**
 * Put NTB into the receiver free list.
 */
static void recv_put_ntb_into_free_list(recv_ntb_t* free_ntb) {
    for(int i = 0; i < NCM_RECV_NTB_N; ++i) {
        if(ncm_interface.recv_free_ntb[i] == NULL) {
            ncm_interface.recv_free_ntb[i] = free_ntb;
            return;
        }
    }
}

/**
 * \a ready_ntb holds a validated NTB,
 * put this buffer into the waiting list.
 */
static void recv_put_ntb_into_ready_list(recv_ntb_t* ready_ntb) {
    for(int i = 0; i < NCM_RECV_NTB_N; ++i) {
        if(ncm_interface.recv_ready_ntb[i] == NULL) {
            ncm_interface.recv_ready_ntb[i] = ready_ntb;
            return;
        }
    }
} // recv_put_ntb_into_ready_list

/**
 * If possible, start a new reception TinyUSB -> driver.
 */
static void recv_try_to_start_new_reception(uint8_t rhport) {
    if(ncm_interface.itf_data_alt != 1) {
        return;
    }
    if(ncm_interface.recv_tinyusb_ntb != NULL) {
        return;
    }
    if(usbd_edpt_busy(rhport, ncm_interface.ep_out)) {
        return;
    }

    ncm_interface.recv_tinyusb_ntb = recv_get_free_ntb();
    if(ncm_interface.recv_tinyusb_ntb == NULL) {
        return;
    }

    // initiate transfer
    bool r = usbd_edpt_xfer(
        rhport,
        ncm_interface.ep_out,
        ncm_interface.recv_tinyusb_ntb->data,
        CFG_TUD_NCM_OUT_NTB_MAX_SIZE);
    if(!r) {
        recv_put_ntb_into_free_list(ncm_interface.recv_tinyusb_ntb);
        ncm_interface.recv_tinyusb_ntb = NULL;
    }
}

/**
 * Validate incoming datagram.
 * \return true if valid
 *
 * \note
 *    \a ndp16->wNextNdpIndex != 0 is not supported
 */
static bool recv_validate_datagram(const recv_ntb_t* ntb, uint32_t len) {
    const nth16_t* nth16 = &(ntb->nth);

    // check header
    if(nth16->wHeaderLength != sizeof(nth16_t)) {
        return false;
    }
    if(nth16->dwSignature != NTH16_SIGNATURE) {
        return false;
    }
    if(len < sizeof(nth16_t) + sizeof(ndp16_t) + 2 * sizeof(ndp16_datagram_t)) {
        return false;
    }
    if(nth16->wBlockLength > len) {
        return false;
    }
    if(nth16->wBlockLength > CFG_TUD_NCM_OUT_NTB_MAX_SIZE) {
        return false;
    }
    if(nth16->wNdpIndex < sizeof(nth16) ||
       nth16->wNdpIndex > len - (sizeof(ndp16_t) + 2 * sizeof(ndp16_datagram_t))) {
        return false;
    }

    // check (first) NDP(16)
    const ndp16_t* ndp16 = (const ndp16_t*)(ntb->data + nth16->wNdpIndex);

    if(ndp16->wLength < sizeof(ndp16_t) + 2 * sizeof(ndp16_datagram_t)) {
        return false;
    }
    if(ndp16->dwSignature != NDP16_SIGNATURE_NCM0 && ndp16->dwSignature != NDP16_SIGNATURE_NCM1) {
        return false;
    }
    if(ndp16->wNextNdpIndex != 0) {
        return false;
    }

    const ndp16_datagram_t* ndp16_datagram =
        (const ndp16_datagram_t*)(ntb->data + nth16->wNdpIndex + sizeof(ndp16_t));
    int ndx = 0;
    uint16_t max_ndx = (uint16_t)((ndp16->wLength - sizeof(ndp16_t)) / sizeof(ndp16_datagram_t));

    if(max_ndx > 2) { // number of datagrams in NTB > 1
    }
    if(ndp16_datagram[max_ndx - 1].wDatagramIndex != 0 ||
       ndp16_datagram[max_ndx - 1].wDatagramLength != 0) {
        return false;
    }
    while(ndp16_datagram[ndx].wDatagramIndex != 0 && ndp16_datagram[ndx].wDatagramLength != 0) {
        if(ndp16_datagram[ndx].wDatagramIndex > len) {
            return false;
        }
        if(ndp16_datagram[ndx].wDatagramIndex + ndp16_datagram[ndx].wDatagramLength > len) {
            return false;
        }
        ++ndx;
    }

    // -> ntb contains a valid packet structure
    //    ok... I did not check for garbage within the datagram indices...
    return true;
} // recv_validate_datagram

/**
 * Transfer the next (pending) datagram to the glue logic and return receive buffer if empty.
 */
static void recv_transfer_datagram_to_glue_logic(void) {
    if(ncm_interface.recv_glue_ntb == NULL) {
        ncm_interface.recv_glue_ntb = recv_get_next_ready_ntb();
        ncm_interface.recv_glue_ntb_datagram_ndx = 0;
    }

    if(ncm_interface.recv_glue_ntb != NULL) {
        const ndp16_datagram_t* ndp16_datagram =
            (ndp16_datagram_t*)(ncm_interface.recv_glue_ntb->data +
                                ncm_interface.recv_glue_ntb->nth.wNdpIndex + sizeof(ndp16_t));

        if(ndp16_datagram[ncm_interface.recv_glue_ntb_datagram_ndx].wDatagramIndex == 0) {
        } else if(ndp16_datagram[ncm_interface.recv_glue_ntb_datagram_ndx].wDatagramLength == 0) {
        } else {
            uint16_t datagramIndex =
                ndp16_datagram[ncm_interface.recv_glue_ntb_datagram_ndx].wDatagramIndex;
            uint16_t datagramLength =
                ndp16_datagram[ncm_interface.recv_glue_ntb_datagram_ndx].wDatagramLength;

            if(tud_network_recv_cb(
                   ncm_interface.recv_glue_ntb->data + datagramIndex, datagramLength)) {
                // send datagram successfully to glue logic
                datagramIndex =
                    ndp16_datagram[ncm_interface.recv_glue_ntb_datagram_ndx + 1].wDatagramIndex;
                datagramLength =
                    ndp16_datagram[ncm_interface.recv_glue_ntb_datagram_ndx + 1].wDatagramLength;

                if(datagramIndex != 0 && datagramLength != 0) {
                    // -> next datagram
                    ++ncm_interface.recv_glue_ntb_datagram_ndx;
                } else {
                    // end of datagrams reached
                    recv_put_ntb_into_free_list(ncm_interface.recv_glue_ntb);
                    ncm_interface.recv_glue_ntb = NULL;
                }
            }
        }
    }
}

bool furi_hal_usb_eth_can_xmit(uint16_t size) {
    TU_ASSERT(
        size <= CFG_TUD_NCM_IN_NTB_MAX_SIZE -
                    (sizeof(nth16_t) + sizeof(ndp16_t) + 2 * sizeof(ndp16_datagram_t)),
        false);

    if(xmit_requested_datagram_fits_into_current_ntb(size) || xmit_setup_next_glue_ntb()) {
        // -> everything is fine
        return true;
    }
    xmit_start_if_possible(0);
    return false;
}

void furi_hal_usb_eth_xmit(void* ref, uint16_t arg) {
    if(ncm_interface.xmit_glue_ntb == NULL) {
        return;
    }

    xmit_ntb_t* ntb = ncm_interface.xmit_glue_ntb;

    // copy new datagram to the end of the current NTB
    uint16_t size = tud_network_xmit_cb(ntb->data + ntb->nth.wBlockLength, ref, arg);

    // correct NTB internals
    ntb->ndp_datagram[ncm_interface.xmit_glue_ntb_datagram_ndx].wDatagramIndex =
        ntb->nth.wBlockLength;
    ntb->ndp_datagram[ncm_interface.xmit_glue_ntb_datagram_ndx].wDatagramLength = size;
    ncm_interface.xmit_glue_ntb_datagram_ndx += 1;

    ntb->nth.wBlockLength += (uint16_t)(size + XMIT_ALIGN_OFFSET(size));

    if(ntb->nth.wBlockLength > CFG_TUD_NCM_IN_NTB_MAX_SIZE) {
        return;
    }

    xmit_start_if_possible(0);
}

void furi_hal_usb_eth_recv_renew(void) {
    ncm_interface.tud_network_recv_renew_process_again = true;

    if(ncm_interface.tud_network_recv_renew_active) {
        return;
    }

    while(ncm_interface.tud_network_recv_renew_process_again) {
        ncm_interface.tud_network_recv_renew_process_again = false;

        // If the current function is called within recv_transfer_datagram_to_glue_logic,
        // tud_network_recv_renew_process_again will become true, and the loop will run again
        // Otherwise the loop will not run again
        ncm_interface.tud_network_recv_renew_active = true;
        recv_transfer_datagram_to_glue_logic();
        ncm_interface.tud_network_recv_renew_active = false;
    }
    recv_try_to_start_new_reception(0);
}

void* usbd_eth_init(void* settings) {
    UNUSED(settings);

    memset(&ncm_interface, 0, sizeof(ncm_interface));

    for(int i = 0; i < NCM_XMIT_NTB_N; ++i) {
        ncm_interface.xmit_free_ntb[i] = &ncm_epbuf.xmit[i].ntb;
    }
    for(int i = 0; i < NCM_RECV_NTB_N; ++i) {
        ncm_interface.recv_free_ntb[i] = &ncm_epbuf.recv[i].ntb;
    }
    return NULL;
}

void usbd_eth_deinit(void) {
}

void usbd_eth_reset(uint8_t rhport) {
    (void)rhport;

    usbd_eth_init(NULL);
}

uint16_t usbd_eth_open(uint8_t rhport, tusb_desc_interface_t const* itf_desc, uint16_t max_len) {
    TU_ASSERT(ncm_interface.ep_notif == 0, 0); // assure that the interface is only opened once

    ncm_interface.itf_num = itf_desc->bInterfaceNumber; // management interface

    // skip the two first entries and the following TUSB_DESC_CS_INTERFACE entries
    uint16_t drv_len = sizeof(tusb_desc_interface_t);
    uint8_t const* p_desc = tu_desc_next(itf_desc);
    while(tu_desc_type(p_desc) == TUSB_DESC_CS_INTERFACE && drv_len <= max_len) {
        drv_len += tu_desc_len(p_desc);
        p_desc = tu_desc_next(p_desc);
    }

    // get notification endpoint
    TU_ASSERT(tu_desc_type(p_desc) == TUSB_DESC_ENDPOINT, 0);
    TU_ASSERT(usbd_edpt_open(rhport, (tusb_desc_endpoint_t const*)p_desc), 0);
    ncm_interface.ep_notif = ((tusb_desc_endpoint_t const*)p_desc)->bEndpointAddress;
    drv_len += tu_desc_len(p_desc);
    p_desc = tu_desc_next(p_desc);

    // skip the following TUSB_DESC_INTERFACE entries (which must be TUSB_CLASS_CDC_DATA)
    while(tu_desc_type(p_desc) == TUSB_DESC_INTERFACE && drv_len <= max_len) {
        tusb_desc_interface_t const* data_itf_desc = (tusb_desc_interface_t const*)p_desc;
        TU_ASSERT(data_itf_desc->bInterfaceClass == TUSB_CLASS_CDC_DATA, 0);

        drv_len += tu_desc_len(p_desc);
        p_desc = tu_desc_next(p_desc);
    }

    // a TUSB_DESC_ENDPOINT (actually two) must follow, open these endpoints
    TU_ASSERT(tu_desc_type(p_desc) == TUSB_DESC_ENDPOINT, 0);
    TU_ASSERT(usbd_open_edpt_pair(
        rhport, p_desc, 2, TUSB_XFER_BULK, &ncm_interface.ep_out, &ncm_interface.ep_in));
    drv_len += 2 * sizeof(tusb_desc_endpoint_t);

    return drv_len;
}

bool usbd_eth_xfer_cb(
    uint8_t rhport,
    uint8_t ep_addr,
    xfer_result_t result,
    uint32_t xferred_bytes) {
    (void)result;

    if(ep_addr == ncm_interface.ep_out) {
        // new NTB received
        // - make the NTB valid
        // - if ready transfer datagrams to the glue logic for further processing
        // - if there is a free receive buffer, initiate reception
        if(!recv_validate_datagram(ncm_interface.recv_tinyusb_ntb, xferred_bytes)) {
            // verification failed: ignore NTB and return it to free
            recv_put_ntb_into_free_list(ncm_interface.recv_tinyusb_ntb);
        } else {
            // packet ok -> put it into ready list
            recv_put_ntb_into_ready_list(ncm_interface.recv_tinyusb_ntb);
        }
        ncm_interface.recv_tinyusb_ntb = NULL;
        furi_hal_usb_eth_recv_renew();
    } else if(ep_addr == ncm_interface.ep_in) {
        // transmission of an NTB finished
        // - free the transmitted NTB buffer
        // - insert ZLPs when necessary
        // - if there is another transmit NTB waiting, try to start transmission
        xmit_put_ntb_into_free_list(ncm_interface.xmit_tinyusb_ntb);
        ncm_interface.xmit_tinyusb_ntb = NULL;
        if(!xmit_insert_required_zlp(rhport, xferred_bytes)) {
            xmit_start_if_possible(rhport);
        }
    } else if(ep_addr == ncm_interface.ep_notif) {
        // next transfer on notification channel
        notification_xmit(rhport, true);
    }

    return true;
}

bool usbd_eth_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const* request) {
    if(stage != CONTROL_STAGE_SETUP) {
        return true;
    }

    switch(request->bmRequestType_bit.type) {
    case TUSB_REQ_TYPE_STANDARD:

        switch(request->bRequest) {
        case TUSB_REQ_GET_INTERFACE: {
            TU_VERIFY(ncm_interface.itf_num + 1 == request->wIndex, false);

            tud_control_xfer(rhport, request, &ncm_interface.itf_data_alt, 1);
        } break;

        case TUSB_REQ_SET_INTERFACE: {
            TU_VERIFY(ncm_interface.itf_num + 1 == request->wIndex && request->wValue < 2, false);

            ncm_interface.itf_data_alt = (uint8_t)request->wValue;

            if(ncm_interface.itf_data_alt == 1) {
                furi_hal_usb_eth_recv_renew();
                notification_xmit(rhport, false);
            }
            tud_control_status(rhport, request);
        } break;

        // unsupported request
        default:
            return false;
        }
        break;

    case TUSB_REQ_TYPE_CLASS:
        TU_VERIFY(ncm_interface.itf_num == request->wIndex, false);
        switch(request->bRequest) {
        case NCM_GET_NTB_PARAMETERS: {
            // transfer NTB parameters to host.
            tud_control_xfer(
                rhport, request, (void*)(uintptr_t)&ntb_parameters, sizeof(ntb_parameters));
        } break;
            // unsupported request
        default:
            return false;
        }
        break;
        // unsupported request
    default:
        return false;
    }

    return true;
}

char* usbd_eth_get_mac_str(void) {
    return "0CFA22012345"; // TODO: furi_hal_version_get_mac_str
}
