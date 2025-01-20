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

#define NTH16_SIGNATURE      0x484D434E
#define NDP16_SIGNATURE_NCM0 0x304D434E
#define NDP16_SIGNATURE_NCM1 0x314D434E

// NCM Transfer Block (NTB) parameters
typedef struct {
    uint16_t wLength;
    uint16_t bmNtbFormatsSupported;
    uint32_t dwNtbInMaxSize;
    uint16_t wNdbInDivisor;
    uint16_t wNdbInPayloadRemainder;
    uint16_t wNdbInAlignment;
    uint16_t wReserved;
    uint32_t dwNtbOutMaxSize;
    uint16_t wNdbOutDivisor;
    uint16_t wNdbOutPayloadRemainder;
    uint16_t wNdbOutAlignment;
    uint16_t wNtbOutMaxDatagrams;
} FURI_PACKED NtbParameters;

// NCM Transfer Header (NTH)
typedef struct {
    uint32_t dwSignature;
    uint16_t wHeaderLength;
    uint16_t wSequence;
    uint16_t wBlockLength;
    uint16_t wNdpIndex;
} FURI_PACKED Nth16;

// NCM single Datagram Pointer
typedef struct {
    uint16_t wDatagramIndex;
    uint16_t wDatagramLength;
} FURI_PACKED Ndp16Datagram;

// NCM Datagram Pointers (NDP)
typedef struct {
    uint32_t dwSignature;
    uint16_t wLength;
    uint16_t wNextNdpIndex;
    Ndp16Datagram datagram[];
} FURI_PACKED Ndp16;

typedef union {
    struct {
        Nth16 nth;
        Ndp16 ndp;
    };
    uint8_t data[NCM_IN_NTB_MAX_SIZE];
} FURI_PACKED TransmitNtb;

typedef struct {
    tusb_control_request_t header;
    uint32_t downlink;
    uint32_t uplink;
} FURI_PACKED NcmNotify;

typedef struct {
    uint8_t itf_num; // Index number of Management Interface, +1 for Data Interface
    uint8_t itf_data_alt; // Alternate setting of Data Interface. 0 : inactive, 1 : active

    uint8_t ep_notif;
    uint8_t ep_in;
    uint8_t ep_out;

    const Ndp16* ndp;
    uint8_t num_datagrams, current_datagram_index;

    enum {
        REPORT_SPEED,
        REPORT_CONNECTED,
        REPORT_DONE
    } report_state;
    bool report_pending;

    uint8_t current_ntb;
    uint8_t datagram_count;
    uint16_t next_datagram_offset;
    uint16_t ntb_in_size;
    uint8_t max_datagrams_per_ntb;

    uint16_t nth_sequence;

    bool transferring;

} ncm_interface_t;

static const NtbParameters ntb_parameters __attribute__((aligned(4))) = {
    .wLength = sizeof(NtbParameters),
    .bmNtbFormatsSupported = 0x01,
    .dwNtbInMaxSize = NCM_IN_NTB_MAX_SIZE,
    .wNdbInDivisor = 4,
    .wNdbInPayloadRemainder = 0,
    .wNdbInAlignment = NCM_ALIGNMENT,
    .wReserved = 0,
    .dwNtbOutMaxSize = NCM_OUT_NTB_MAX_SIZE,
    .wNdbOutDivisor = 4,
    .wNdbOutPayloadRemainder = 0,
    .wNdbOutAlignment = NCM_ALIGNMENT,
    .wNtbOutMaxDatagrams = 0,
};

static TransmitNtb transmit_ntb[2] __attribute__((aligned(4)));
static uint8_t receive_ntb[NCM_OUT_NTB_MAX_SIZE] __attribute__((aligned(4)));
static ncm_interface_t ncm_interface;

static void ncm_prepare_for_tx(void) {
    ncm_interface.datagram_count = 0;
    ncm_interface.next_datagram_offset =
        sizeof(Nth16) + sizeof(Ndp16) + ((NCM_MAX_DATAGRAMS_PER_NTB + 1) * sizeof(Ndp16Datagram));
}

static void ncm_start_tx(void) {
    if(ncm_interface.transferring) {
        return;
    }

    TransmitNtb* ntb = &transmit_ntb[ncm_interface.current_ntb];
    size_t ntb_length = ncm_interface.next_datagram_offset;

    // Fill in NTB header
    ntb->nth.dwSignature = NTH16_SIGNATURE;
    ntb->nth.wHeaderLength = sizeof(Nth16);
    ntb->nth.wSequence = ncm_interface.nth_sequence++;
    ntb->nth.wBlockLength = ntb_length;
    ntb->nth.wNdpIndex = sizeof(Nth16);

    // Fill in NDP16 header and terminator
    ntb->ndp.dwSignature = NDP16_SIGNATURE_NCM0;
    ntb->ndp.wLength = sizeof(Ndp16) + (ncm_interface.datagram_count + 1) * sizeof(Ndp16Datagram);
    ntb->ndp.wNextNdpIndex = 0;
    ntb->ndp.datagram[ncm_interface.datagram_count].wDatagramIndex = 0;
    ntb->ndp.datagram[ncm_interface.datagram_count].wDatagramLength = 0;

    // Kick off an endpoint transfer
    usbd_edpt_xfer(0, ncm_interface.ep_in, ntb->data, ntb_length);
    ncm_interface.transferring = true;

    // Swap to the other NTB and clear it out
    ncm_interface.current_ntb = 1 - ncm_interface.current_ntb;
    ncm_prepare_for_tx();
}

static NcmNotify ncm_notify_connected = {
    .header =
        {
            .bmRequestType_bit =
                {
                    .recipient = TUSB_REQ_RCPT_INTERFACE,
                    .type = TUSB_REQ_TYPE_CLASS,
                    .direction = TUSB_DIR_IN,
                },
            .bRequest = CDC_NOTIF_NETWORK_CONNECTION,
            .wValue = 1 /* Connected */,
            .wLength = 0,
        },
};

static NcmNotify ncm_notify_speed_change = {
    .header =
        {
            .bmRequestType_bit =
                {
                    .recipient = TUSB_REQ_RCPT_INTERFACE,
                    .type = TUSB_REQ_TYPE_CLASS,
                    .direction = TUSB_DIR_IN,
                },
            .bRequest = CDC_NOTIF_CONNECTION_SPEED_CHANGE,
            .wLength = 8,
        },
    .downlink = 10000000,
    .uplink = 10000000,
};

void furi_hal_usb_eth_recv_renew(void) {
    if(!ncm_interface.num_datagrams) {
        usbd_edpt_xfer(0, ncm_interface.ep_out, receive_ntb, sizeof(receive_ntb));
        return;
    }

    const Ndp16* ndp = ncm_interface.ndp;
    const int i = ncm_interface.current_datagram_index;
    ncm_interface.current_datagram_index++;
    ncm_interface.num_datagrams--;

    tud_network_recv_cb(
        receive_ntb + ndp->datagram[i].wDatagramIndex, ndp->datagram[i].wDatagramLength);
}

void* usbd_eth_init(void* settings) {
    UNUSED(settings);
    tu_memclr(&ncm_interface, sizeof(ncm_interface));
    ncm_interface.ntb_in_size = NCM_IN_NTB_MAX_SIZE;
    ncm_interface.max_datagrams_per_ntb = NCM_MAX_DATAGRAMS_PER_NTB;
    ncm_prepare_for_tx();
    return NULL;
}

void usbd_eth_deinit(void) {
}

void usbd_eth_reset(uint8_t rhport) {
    (void)rhport;
    usbd_eth_init(NULL);
}

uint16_t usbd_eth_open(uint8_t rhport, tusb_desc_interface_t const* itf_desc, uint16_t max_len) {
    // confirm interface hasn't already been allocated
    TU_ASSERT(0 == ncm_interface.ep_notif, 0);

    ncm_interface.itf_num = itf_desc->bInterfaceNumber;

    uint16_t drv_len = sizeof(tusb_desc_interface_t);
    uint8_t const* p_desc = tu_desc_next(itf_desc);

    // Communication Functional Descriptors
    while(TUSB_DESC_CS_INTERFACE == tu_desc_type(p_desc) && drv_len <= max_len) {
        drv_len += tu_desc_len(p_desc);
        p_desc = tu_desc_next(p_desc);
    }

    // notification endpoint (if any)
    if(TUSB_DESC_ENDPOINT == tu_desc_type(p_desc)) {
        TU_ASSERT(usbd_edpt_open(rhport, (tusb_desc_endpoint_t const*)p_desc), 0);

        ncm_interface.ep_notif = ((tusb_desc_endpoint_t const*)p_desc)->bEndpointAddress;

        drv_len += tu_desc_len(p_desc);
        p_desc = tu_desc_next(p_desc);
    }

    TU_ASSERT(TUSB_DESC_INTERFACE == tu_desc_type(p_desc), 0);

    do {
        tusb_desc_interface_t const* data_itf_desc = (tusb_desc_interface_t const*)p_desc;
        TU_ASSERT(TUSB_CLASS_CDC_DATA == data_itf_desc->bInterfaceClass, 0);

        drv_len += tu_desc_len(p_desc);
        p_desc = tu_desc_next(p_desc);
    } while((TUSB_DESC_INTERFACE == tu_desc_type(p_desc)) && (drv_len <= max_len));

    // Pair of endpoints
    TU_ASSERT(TUSB_DESC_ENDPOINT == tu_desc_type(p_desc), 0);

    TU_ASSERT(usbd_open_edpt_pair(
        rhport, p_desc, 2, TUSB_XFER_BULK, &ncm_interface.ep_out, &ncm_interface.ep_in));

    drv_len += 2 * sizeof(tusb_desc_endpoint_t);

    return drv_len;
}

static void ncm_report(void) {
    uint8_t const rhport = 0;
    if(ncm_interface.report_state == REPORT_SPEED) {
        ncm_notify_speed_change.header.wIndex = ncm_interface.itf_num;
        usbd_edpt_xfer(
            rhport,
            ncm_interface.ep_notif,
            (uint8_t*)&ncm_notify_speed_change,
            sizeof(ncm_notify_speed_change));
        ncm_interface.report_state = REPORT_CONNECTED;
        ncm_interface.report_pending = true;
    } else if(ncm_interface.report_state == REPORT_CONNECTED) {
        ncm_notify_connected.header.wIndex = ncm_interface.itf_num;
        usbd_edpt_xfer(
            rhport,
            ncm_interface.ep_notif,
            (uint8_t*)&ncm_notify_connected,
            sizeof(ncm_notify_connected));
        ncm_interface.report_state = REPORT_DONE;
        ncm_interface.report_pending = true;
    }
}

void tud_network_link_state_cb(bool state) {
    (void)state;
}

bool usbd_eth_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const* request) {
    if(stage == CONTROL_STAGE_SETUP) {
        switch(request->bmRequestType_bit.type) {
        case TUSB_REQ_TYPE_STANDARD:
            switch(request->bRequest) {
            case TUSB_REQ_GET_INTERFACE: {
                uint8_t const req_itfnum = (uint8_t)request->wIndex;
                TU_VERIFY(ncm_interface.itf_num + 1 == req_itfnum);

                tud_control_xfer(rhport, request, &ncm_interface.itf_data_alt, 1);
            } break;

            case TUSB_REQ_SET_INTERFACE: {
                uint8_t const req_itfnum = (uint8_t)request->wIndex;
                uint8_t const req_alt = (uint8_t)request->wValue;

                // Only valid for Data Interface with Alternate is either 0 or 1
                TU_VERIFY(ncm_interface.itf_num + 1 == req_itfnum && req_alt < 2);

                if(req_alt != ncm_interface.itf_data_alt) {
                    ncm_interface.itf_data_alt = req_alt;

                    if(ncm_interface.itf_data_alt) {
                        if(!usbd_edpt_busy(rhport, ncm_interface.ep_out)) {
                            furi_hal_usb_eth_recv_renew(); // prepare for incoming datagrams
                        }
                        if(!ncm_interface.report_pending) {
                            ncm_report();
                        }
                    }

                    tud_network_link_state_cb(ncm_interface.itf_data_alt);
                }

                tud_control_status(rhport, request);
            } break;

            default:
                return false;
            }
            break;

        case TUSB_REQ_TYPE_CLASS:
            TU_VERIFY(ncm_interface.itf_num == request->wIndex);

            if(request->bRequest == NCM_GET_NTB_PARAMETERS) {
                tud_control_xfer(
                    rhport, request, (void*)(uintptr_t)&ntb_parameters, sizeof(ntb_parameters));
            }

            break;

            // unsupported request
        default:
            return false;
        }
    }

    return true;
}

static void ncm_handle_incoming_datagram(uint32_t len) {
    uint32_t size = len;

    if(len == 0) {
        return;
    }

    TU_ASSERT(size >= sizeof(Nth16), );

    const Nth16* hdr = (const Nth16*)receive_ntb;
    TU_ASSERT(hdr->dwSignature == NTH16_SIGNATURE, );
    TU_ASSERT(hdr->wNdpIndex >= sizeof(Nth16) && (hdr->wNdpIndex + sizeof(Ndp16)) <= len, );

    const Ndp16* ndp = (const Ndp16*)(receive_ntb + hdr->wNdpIndex);
    TU_ASSERT(
        ndp->dwSignature == NDP16_SIGNATURE_NCM0 || ndp->dwSignature == NDP16_SIGNATURE_NCM1, );
    TU_ASSERT(hdr->wNdpIndex + ndp->wLength <= len, );

    int num_datagrams = (ndp->wLength - 12) / 4;
    ncm_interface.current_datagram_index = 0;
    ncm_interface.num_datagrams = 0;
    ncm_interface.ndp = ndp;
    for(int i = 0;
        i < num_datagrams && ndp->datagram[i].wDatagramIndex && ndp->datagram[i].wDatagramLength;
        i++) {
        ncm_interface.num_datagrams++;
    }

    furi_hal_usb_eth_recv_renew();
}

bool usbd_eth_xfer_cb(
    uint8_t rhport,
    uint8_t ep_addr,
    xfer_result_t result,
    uint32_t xferred_bytes) {
    (void)rhport;
    (void)result;

    /* new datagram receive_ntb */
    if(ep_addr == ncm_interface.ep_out) {
        ncm_handle_incoming_datagram(xferred_bytes);
    }

    /* data transmission finished */
    if(ep_addr == ncm_interface.ep_in) {
        if(ncm_interface.transferring) {
            ncm_interface.transferring = false;
        }

        // If there are datagrams queued up that we tried to send while this NTB was being emitted, send them now
        if(ncm_interface.datagram_count && ncm_interface.itf_data_alt == 1) {
            ncm_start_tx();
        }
    }

    if(ep_addr == ncm_interface.ep_notif) {
        ncm_interface.report_pending = false;
        ncm_report();
    }

    return true;
}

char* usbd_eth_get_mac_str(void) {
    return "0CFA22012345"; // TODO: furi_hal_version_get_mac_str
}

bool furi_hal_usb_eth_can_xmit(uint16_t size) {
    TU_VERIFY(ncm_interface.itf_data_alt == 1);

    if(ncm_interface.datagram_count >= ncm_interface.max_datagrams_per_ntb) {
        return false;
    }

    size_t next_datagram_offset = ncm_interface.next_datagram_offset;
    if(next_datagram_offset + size > ncm_interface.ntb_in_size) {
        return false;
    }

    return true;
}

void furi_hal_usb_eth_xmit(void* ref, uint16_t arg) {
    TransmitNtb* ntb = &transmit_ntb[ncm_interface.current_ntb];
    size_t next_datagram_offset = ncm_interface.next_datagram_offset;

    uint16_t size = tud_network_xmit_cb(ntb->data + next_datagram_offset, ref, arg);

    ntb->ndp.datagram[ncm_interface.datagram_count].wDatagramIndex =
        ncm_interface.next_datagram_offset;
    ntb->ndp.datagram[ncm_interface.datagram_count].wDatagramLength = size;

    ncm_interface.datagram_count++;
    next_datagram_offset += size;

    // round up so the next datagram is aligned correctly
    next_datagram_offset += (NCM_ALIGNMENT - 1);
    next_datagram_offset -= (next_datagram_offset % NCM_ALIGNMENT);

    ncm_interface.next_datagram_offset = next_datagram_offset;

    ncm_start_tx();
}
