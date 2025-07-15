#include <furi.h>
#include <furi_hal.h>
#include "power_i.h"
#include <stm32u5xx_ll_dma.h>
#include <stm32u5xx_ll_ucpd.h>
#include <stm32u5xx_ll_pwr.h>

#define TAG "UsbPD"

// We don't support cable markers, so limit max current to 3A
#define PD_MAX_CURRENT     3000
#define PD_VOLTAGE_DEFAULT 5000

#define PD_SINK_CAP_VOLTAGE 9000

#define PD_CC_POLL_PERIOD 500
#define PD_MAX_PACKET_LEN 32

#define RX_IGNORE_HRST_COUNT 3

#define UCPD_TRANSWIN       0x09
#define UCPD_IFRGAP         0x0E
#define UCPD_HBIT_CLOCK_DIV 0x0D

typedef enum {
    PdCClineLevelNC = 0, // Not connected/pulled down
    PdCClineLevelUsb500ma = 1, // USB default (0.5A)
    PdCClineLevel1500ma = 2, // 1.5A
    PdCClineLevel3000ma = 3, // 3A
} PdCClineLevels;

typedef enum {
    PdCClineCurrentUsb500ma = 500,
    PdCClineCurrent1500ma = 1500,
    PdCClineCurrent3000ma = 3000,
} PdCClineCurrent;

typedef enum {
    PdOrdSetSOP0 = 0, // SOP
    PdOrdSetSOP1 = 1, // SOP'
    PdOrdSetSOP2 = 2, // SOP"
    PdOrdSetSOP1Dbg = 3, // SOP'_debug
    PdOrdSetSOP2Dbg = 4, // SOP"_debug
    PdOrdSetCableRst = 5, // Cable reset
    PdOrdSetSOPext1 = 6, // SOP ext 1
    PdOrdSetSOPext2 = 7, // SOP ext 2
} PdOrdSet;

typedef enum {
    PdRevision10 = 0, // Revision 1.0 (Not supported)
    PdRevision20 = 1, // Revision 2.0
    PdRevision30 = 2, // Revision 3.0
    PdRevisionReserved = 3, // Reserved (Not supported)
} PdRevision;

typedef enum {
    PdPowerRoleSnk = 0, // Sink
    PdPowerRoleSrc = 1, // Source
} PdPowerRole;

typedef enum {
    PdDataRoleUFP = 0, // UFP(Device)
    PdDataRoleDFP = 1, // DFP(Host)
} PdDataRole;

typedef enum {
    PdCablePlugFP = 0, // DFP/UFP
    PdCablePlugVPD = 1, // Cable marker / Vconn powered device
} PdCablePlug;

typedef enum {
    PdMsgGoodCrc = 1,
    PdMsgGotoMin = 2,
    PdMsgAccept = 3,
    PdMsgReject = 4,
    PdMsgPing = 5,
    PdMsgPsRdy = 6,
    PdMsgGetSourceCaps = 7,
    PdMsgGetSinkCaps = 8,
    PdMsgDrSwap = 9,
    PdMsgPrSwap = 10,
    PdMsgVconnSwap = 11,
    PdMsgWait = 12,
    PdMsgSoftReset = 13,
    PdMsgDataReset = 14,
    PdMsgDataResetComplete = 15,
    PdMsgNotSupported = 16,
    PdMsgGetSourceCapsExt = 17,
    PdMsgGetStatus = 18,
    PdMsgFrSwap = 19,
    PdMsgGetPpsStatus = 20,
    PdMsgGetCountryCodes = 21,
    PdMsgGetSinkCapsExt = 22,
    PdMsgGetSrcInfo = 23,
    PdMsgGetRevision = 24,
    PdMsgControlMax = 25,
} PdMsgControl;

typedef enum {
    PdMsgSourceCaps = 1,
    PdMsgRequest = 2,
    PdMsgBist = 3,
    PdMsgSinkCaps = 4,
    PdMsgBatteryStatus = 5,
    PdMsgAlert = 6,
    PdMsgGetCountryInfo = 7,
    PdMsgEnterUsb = 8,
    PdMsgEprRequest = 9,
    PdMsgEprMode = 10,
    PdMsgSrcInfo = 11,
    PdMsgRevision = 12,
    PdMsgDataMax = 13,
    PdMsgVendor = 0x0F,
} PdMsgData;

typedef struct FURI_PACKED {
    union {
        struct {
            uint16_t msg_type   : 5;
            uint16_t data_role  : 1;
            uint16_t revision   : 2;
            uint16_t power_role : 1;
            uint16_t msg_id     : 3;
            uint16_t nb_objects : 3;
            uint16_t extended   : 1;
        };
        uint16_t data;
    };
} PdHeaderSop0;

typedef struct FURI_PACKED {
    union {
        struct {
            uint32_t           : 28;
            uint32_t apdo_type : 2;
            uint32_t type      : 2;
        };
        struct {
            uint32_t current_max         : 10;
            uint32_t voltage             : 10;
            uint32_t peak_current_mode   : 2;
            uint32_t                     : 1;
            uint32_t epr_capable         : 1;
            uint32_t long_ext_msg        : 1;
            uint32_t drd                 : 1;
            uint32_t usb_data_support    : 1;
            uint32_t unconstrained       : 1;
            uint32_t usb_suspend_support : 1;
            uint32_t drp                 : 1;
            uint32_t                     : 2;
        } fixed;
        struct {
            uint32_t current_max   : 7;
            uint32_t               : 1;
            uint32_t voltage_min   : 8;
            uint32_t               : 1;
            uint32_t voltage_max   : 8;
            uint32_t               : 2;
            uint32_t power_limited : 1;
            uint32_t               : 2;
            uint32_t               : 2;
        } pps;
        uint32_t data;
    };
} PdPowerDataObject;

typedef struct FURI_PACKED {
    union {
        struct {
            uint32_t current          : 10;
            uint32_t voltage          : 10;
            uint32_t                  : 3;
            uint32_t frs_mode         : 2;
            uint32_t drd              : 1;
            uint32_t usb_data_support : 1;
            uint32_t unconstrained    : 1;
            uint32_t higher_cap       : 1;
            uint32_t drp              : 1;
            uint32_t type             : 2;
        };
        uint32_t data;
    };
} PdSinkPowerDataObject;

typedef struct FURI_PACKED {
    union {
        struct {
            uint32_t current_max      : 10;
            uint32_t current_target   : 10;
            uint32_t                  : 2;
            uint32_t epr_capable      : 1;
            uint32_t long_ext_msg     : 1;
            uint32_t no_usb_suspend   : 1;
            uint32_t usb_data_support : 1;
            uint32_t cap_mismatch     : 1;
            uint32_t giveback_flag    : 1;
            uint32_t pdo_id           : 4;
        };
        uint32_t data;
    };
} PdRequestFixed;

typedef struct FURI_PACKED {
    union {
        struct {
            uint32_t current          : 7;
            uint32_t                  : 2;
            uint32_t voltage          : 12;
            uint32_t                  : 1;
            uint32_t epr_capable      : 1;
            uint32_t long_ext_msg     : 1;
            uint32_t no_usb_suspend   : 1;
            uint32_t usb_data_support : 1;
            uint32_t cap_mismatch     : 1;
            uint32_t                  : 1;
            uint32_t pdo_id           : 4;
        };
        uint32_t data;
    };
} PdRequestPPS;

typedef struct {
    enum {
        PdEventIdCCState,
        PdEventIdRxDone,
        PdEventIdRxError,
        PdEventIdHrst,

        PdEventIdCCPoll,

        PdEventIdStart,
        PdEventIdRequest,
        PdEventIdPpsKeepAlive,
    } event;
    union {
        struct {
            uint8_t cc1;
            uint8_t cc2;
        } cc_state;
        struct {
            uint8_t ord_set_type;
            uint8_t data[PD_MAX_PACKET_LEN];
        } rx_done;
        struct {
            uint32_t voltage_mv;
            uint32_t current_ma;
        } request_power;
    };
} PdMessage;

struct PowerUsbPd {
    FuriMessageQueue* message_queue;

    FuriTimer* cc_poll_timer;
    FuriTimer* pps_keep_alive_timer;
    FuriSemaphore* tx_idle_sem;
    FuriMutex* cap_mutex;

    PowerUsbPdCapability caps;
    uint8_t req_cap_id;
    uint32_t req_voltage;
    uint32_t req_current;

    uint32_t rx_dma_ch;
    uint32_t tx_dma_ch;

    uint8_t cc_line;
    uint8_t cc_line_level;
    uint8_t cc_irq_last;

    PdMessage rx_msg;
    uint8_t rx_irq_ord_set;

    uint8_t tx_buf[PD_MAX_PACKET_LEN];
    uint8_t tx_len;
    uint8_t tx_ord_set;
    uint8_t tx_msg_id;

    uint8_t src_rev_id;

    uint8_t hrst_count;

    bool started;
    bool pps_keep_alive;
};

static void ucpd_irq_handler(void* context);

static void ucpd_dma_init(PowerUsbPd* pd) {
    furi_hal_dma_allocate_gpdma_channel(&pd->rx_dma_ch);
    furi_hal_dma_allocate_gpdma_channel(&pd->tx_dma_ch);

    furi_check(pd->rx_dma_ch);
    furi_check(pd->tx_dma_ch);

    LL_DMA_InitTypeDef ucpd_dma_cfg = {0};

    ucpd_dma_cfg.BlkHWRequest = LL_DMA_HWREQUEST_SINGLEBURST;
    ucpd_dma_cfg.DataAlignment = LL_DMA_DATA_ALIGN_ZEROPADD;
    ucpd_dma_cfg.SrcBurstLength = 1;
    ucpd_dma_cfg.SrcDataWidth = LL_DMA_SRC_DATAWIDTH_BYTE;
    ucpd_dma_cfg.DestBurstLength = 1;
    ucpd_dma_cfg.DestDataWidth = LL_DMA_DEST_DATAWIDTH_BYTE;
    ucpd_dma_cfg.TriggerMode = LL_DMA_TRIGM_BLK_TRANSFER;
    ucpd_dma_cfg.TriggerPolarity = LL_DMA_TRIG_POLARITY_MASKED;
    ucpd_dma_cfg.TriggerSelection = 0;
    ucpd_dma_cfg.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;
    ucpd_dma_cfg.LinkStepMode = LL_DMA_LSM_1LINK_EXECUTION;
    ucpd_dma_cfg.LinkedListBaseAddr = 0;
    ucpd_dma_cfg.LinkedListAddrOffset = 0;

    ucpd_dma_cfg.SrcAddress = (uint32_t)&UCPD1->RXDR;
    ucpd_dma_cfg.DestAddress = (uint32_t)(pd->rx_msg.rx_done.data);
    ucpd_dma_cfg.BlkDataLength = PD_MAX_PACKET_LEN;
    ucpd_dma_cfg.Request = LL_GPDMA1_REQUEST_UCPD1_RX;
    ucpd_dma_cfg.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
    ucpd_dma_cfg.SrcAllocatedPort = LL_DMA_SRC_ALLOCATED_PORT0;
    ucpd_dma_cfg.SrcIncMode = LL_DMA_SRC_FIXED;
    ucpd_dma_cfg.DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT1;
    ucpd_dma_cfg.DestIncMode = LL_DMA_DEST_INCREMENT;
    ucpd_dma_cfg.Priority = LL_DMA_HIGH_PRIORITY;
    ucpd_dma_cfg.LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT1;
    LL_DMA_Init(GPDMA1, pd->rx_dma_ch, &ucpd_dma_cfg);

    ucpd_dma_cfg.SrcAddress = 0;
    ucpd_dma_cfg.DestAddress = (uint32_t)&UCPD1->TXDR;
    ucpd_dma_cfg.BlkDataLength = 0;
    ucpd_dma_cfg.Request = LL_GPDMA1_REQUEST_UCPD1_TX;
    ucpd_dma_cfg.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    ucpd_dma_cfg.SrcAllocatedPort = LL_DMA_SRC_ALLOCATED_PORT1;
    ucpd_dma_cfg.SrcIncMode = LL_DMA_SRC_INCREMENT;
    ucpd_dma_cfg.DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT0;
    ucpd_dma_cfg.DestIncMode = LL_DMA_DEST_FIXED;
    ucpd_dma_cfg.Priority = LL_DMA_HIGH_PRIORITY;
    ucpd_dma_cfg.LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT0;
    LL_DMA_Init(GPDMA1, pd->tx_dma_ch, &ucpd_dma_cfg);
}

static void ucpd_init(PowerUsbPd* pd) {
    furi_hal_gpio_init_ex(&gpio_ucpd_cc1, GpioModeAnalog, GpioPullNo, GpioSpeedLow, 0);
    furi_hal_gpio_init_ex(&gpio_ucpd_cc2, GpioModeAnalog, GpioPullNo, GpioSpeedLow, 0);

    furi_hal_bus_enable(FuriHalBusUCPD1);
    furi_hal_bus_enable(FuriHalBusCRC);

    LL_UCPD_Disable(UCPD1);
    LL_UCPD_SetPSCClk(UCPD1, LL_UCPD_PSC_DIV2);
    LL_UCPD_SetTransWin(UCPD1, UCPD_TRANSWIN);
    LL_UCPD_SetIfrGap(UCPD1, UCPD_IFRGAP);
    LL_UCPD_SetHbitClockDiv(UCPD1, UCPD_HBIT_CLOCK_DIV);
    LL_UCPD_FRSDetectionDisable(UCPD1);

    ucpd_dma_init(pd);

    LL_UCPD_EnableIT_TypeCEventCC1(UCPD1);
    LL_UCPD_EnableIT_TypeCEventCC2(UCPD1);
    LL_UCPD_EnableIT_RxHRST(UCPD1);

    LL_UCPD_SetRxOrderSet(UCPD1, LL_UCPD_ORDERSET_SOP | LL_UCPD_ORDERSET_HARDRST);

    LL_PWR_DisableUCPDDeadBattery();

    LL_UCPD_Enable(UCPD1);
    LL_UCPD_SetccEnable(UCPD1, LL_UCPD_CCENABLE_CC1CC2);
    LL_UCPD_SetRpResistor(UCPD1, LL_UCPD_RESISTOR_DEFAULT);
    LL_UCPD_SetSNKRole(UCPD1);

    LL_UCPD_RxDMAEnable(UCPD1);
    LL_UCPD_TxDMAEnable(UCPD1);

    // Wait for initial CC1/2 event
    while((UCPD1->SR & (UCPD_SR_TYPECEVT1 | UCPD_SR_TYPECEVT2)) == 0) {
    }
    UCPD1->ICR |= (UCPD_ICR_TYPECEVT1CF | UCPD_ICR_TYPECEVT2CF);

    furi_hal_interrupt_set_isr_ex(
        FuriHalInterruptIdUCPD1, FuriHalInterruptPriorityHighest, ucpd_irq_handler, pd);
}

static void ucpd_transmit(PowerUsbPd* pd, uint8_t ord_set, uint8_t* buf, size_t len) {
    furi_assert(ord_set == PdOrdSetSOP0);

    LL_DMA_DisableChannel(GPDMA1, pd->tx_dma_ch);
    LL_DMA_ConfigAddresses(GPDMA1, pd->tx_dma_ch, (uint32_t)(buf), (uint32_t)&UCPD1->TXDR);
    LL_DMA_SetBlkDataLength(GPDMA1, pd->tx_dma_ch, len);
    LL_DMA_EnableChannel(GPDMA1, pd->tx_dma_ch);

    LL_UCPD_WriteTxOrderSet(UCPD1, LL_UCPD_ORDERED_SET_SOP);
    LL_UCPD_WriteTxPaySize(UCPD1, len);
    UCPD1->CR |= UCPD_CR_TXSEND | (LL_UCPD_TXMODE_NORMAL << UCPD_CR_TXMODE_Pos);
}

static inline void ucpd_cc_detector_control(bool enable) {
    if(enable) {
        UCPD1->CR &= ~(UCPD_CR_CC1TCDIS_Msk | UCPD_CR_CC2TCDIS_Msk);
    } else {
        UCPD1->CR |= UCPD_CR_CC1TCDIS_Msk | UCPD_CR_CC2TCDIS_Msk;
    }
}

static void pd_send_goodcrc(PowerUsbPd* pd, uint8_t msg_id) {
    furi_semaphore_acquire(pd->tx_idle_sem, 0);

    PdHeaderSop0 hdr = {
        .extended = 0,
        .power_role = PdPowerRoleSnk,
        .data_role = PdDataRoleUFP,
        .revision = pd->src_rev_id,
        .msg_id = msg_id,
        .nb_objects = 0,
        .msg_type = PdMsgGoodCrc,
    };

    memcpy(&pd->tx_buf[0], &hdr.data, 2);

    pd->tx_ord_set = PdOrdSetSOP0;
    pd->tx_len = 2;
    ucpd_transmit(pd, pd->tx_ord_set, pd->tx_buf, pd->tx_len);
}

static void pd_send_not_supported(PowerUsbPd* pd) {
    furi_semaphore_acquire(pd->tx_idle_sem, FuriWaitForever);

    PdHeaderSop0 hdr = {
        .extended = 0,
        .power_role = PdPowerRoleSnk,
        .data_role = PdDataRoleUFP,
        .revision = pd->src_rev_id,
        .msg_id = pd->tx_msg_id,
        .nb_objects = 0,
        .msg_type = (pd->src_rev_id == PdRevision30) ? PdMsgNotSupported : PdMsgReject,
    };

    memcpy(&pd->tx_buf[0], &hdr.data, 2);

    pd->tx_ord_set = PdOrdSetSOP0;
    pd->tx_len = 2;
    ucpd_transmit(pd, pd->tx_ord_set, pd->tx_buf, pd->tx_len);
}

static void pd_send_sink_caps(PowerUsbPd* pd) {
    furi_semaphore_acquire(pd->tx_idle_sem, FuriWaitForever);

    PdHeaderSop0 hdr = {
        .extended = 0,
        .power_role = PdPowerRoleSnk,
        .data_role = PdDataRoleUFP,
        .revision = pd->src_rev_id,
        .msg_id = pd->tx_msg_id,
        .nb_objects = 2,
        .msg_type = PdMsgSinkCaps,
    };

    memcpy(&pd->tx_buf[0], &hdr.data, 2);

    PdSinkPowerDataObject sink_pdo[2] = {
        {
            .type = 0,
            .drp = 0,
            .higher_cap = 0,
            .unconstrained = 0,
            .usb_data_support = 1,
            .drd = 0,
            .frs_mode = 0,
            .voltage = PD_VOLTAGE_DEFAULT / 50,
            .current = PD_MAX_CURRENT / 10,
        },
        {
            .type = 0,
            .drp = 0,
            .higher_cap = 0,
            .unconstrained = 0,
            .usb_data_support = 1,
            .drd = 0,
            .frs_mode = 0,
            .voltage = PD_SINK_CAP_VOLTAGE / 50,
            .current = PD_MAX_CURRENT / 10,
        },
    };

    memcpy(&pd->tx_buf[2], &sink_pdo[0].data, 4);
    memcpy(&pd->tx_buf[6], &sink_pdo[1].data, 4);

    pd->tx_ord_set = PdOrdSetSOP0;
    pd->tx_len = 10;
    ucpd_transmit(pd, pd->tx_ord_set, pd->tx_buf, pd->tx_len);
}

static void pd_send_request_fixed(PowerUsbPd* pd, uint32_t cap_id, uint32_t current_ma) {
    furi_timer_stop(pd->pps_keep_alive_timer);
    pd->pps_keep_alive = false;

    furi_semaphore_acquire(pd->tx_idle_sem, FuriWaitForever);

    PdHeaderSop0 hdr = {
        .extended = 0,
        .power_role = PdPowerRoleSnk,
        .data_role = PdDataRoleUFP,
        .revision = pd->src_rev_id,
        .msg_id = pd->tx_msg_id,
        .nb_objects = 1,
        .msg_type = PdMsgRequest,
    };

    memcpy(&pd->tx_buf[0], &hdr.data, 2);

    uint16_t max_current = (current_ma / 10) & 0x3FF;
    PdRequestFixed req = {
        .pdo_id = cap_id,
        .current_max = max_current,
        .current_target = max_current,
        .usb_data_support = 1,
        .no_usb_suspend = 1,
        .giveback_flag = 0,
        .cap_mismatch = 0,
        .long_ext_msg = 0,
        .epr_capable = 0,
    };
    memcpy(&pd->tx_buf[2], &req.data, 4);

    pd->tx_ord_set = PdOrdSetSOP0;
    pd->tx_len = 6;
    ucpd_transmit(pd, pd->tx_ord_set, pd->tx_buf, pd->tx_len);
}

static void
    pd_send_request_pps(PowerUsbPd* pd, uint32_t cap_id, uint32_t voltage, uint32_t current) {
    furi_semaphore_acquire(pd->tx_idle_sem, FuriWaitForever);

    PdHeaderSop0 hdr = {
        .extended = 0,
        .power_role = PdPowerRoleSnk,
        .data_role = PdDataRoleUFP,
        .revision = pd->src_rev_id,
        .msg_id = pd->tx_msg_id,
        .nb_objects = 1,
        .msg_type = PdMsgRequest,
    };

    memcpy(&pd->tx_buf[0], &hdr.data, 2);

    uint16_t max_current = (current / 50) & 0x7F;
    uint16_t voltage_set = (voltage / 20) & 0xFFF;
    PdRequestPPS req = {
        .pdo_id = cap_id,
        .voltage = voltage_set,
        .current = max_current,
        .usb_data_support = 1,
        .no_usb_suspend = 1,
        .cap_mismatch = 0,
        .long_ext_msg = 0,
        .epr_capable = 0,
    };
    memcpy(&pd->tx_buf[2], &req.data, 4);

    pd->tx_ord_set = PdOrdSetSOP0;
    pd->tx_len = 6;
    ucpd_transmit(pd, pd->tx_ord_set, pd->tx_buf, pd->tx_len);

    furi_check(furi_timer_start(pd->pps_keep_alive_timer, 10000) == FuriStatusOk);
}

static bool pd_send_request(PowerUsbPd* pd, uint32_t voltage, uint32_t current) {
    furi_mutex_acquire(pd->cap_mutex, FuriWaitForever);

    size_t cap_nb = pd->caps.cap_number;
    uint8_t cap_id = 0;
    bool is_fixed = false;
    // Find corresponding fixed capability
    for(size_t i = 0; i < cap_nb; i++) {
        if(pd->caps.cap[i].is_fixed == false) {
            continue;
        }
        if((voltage == pd->caps.cap[i].voltage_max) && (current <= pd->caps.cap[i].current_max)) {
            cap_id = pd->caps.cap[i].pdo_id;
            is_fixed = true;
        }
    }

    // No match -> try PPS
    if(cap_id == 0) {
        for(size_t i = 0; i < cap_nb; i++) {
            if(pd->caps.cap[i].is_fixed == true) {
                continue;
            }
            if((voltage >= pd->caps.cap[i].voltage_min) &&
               (voltage <= pd->caps.cap[i].voltage_max) &&
               (current <= pd->caps.cap[i].current_max)) {
                cap_id = pd->caps.cap[i].pdo_id;
                is_fixed = false;
            }
        }
    }

    if(cap_id > 0) {
        if(current == 0) {
            current = pd->caps.cap[cap_id - 1].current_max;
        }

        pd->req_voltage = voltage;
        pd->req_current = current;
        pd->req_cap_id = cap_id;
        if(is_fixed) {
            FURI_LOG_D(TAG, "Request fixed cap %u", cap_id);
            pd_send_request_fixed(pd, cap_id, current);
        } else {
            FURI_LOG_D(TAG, "Request PPS cap %u", cap_id);
            pd_send_request_pps(pd, cap_id, voltage, current);
        }
    }

    furi_mutex_release(pd->cap_mutex);
    return (cap_id != 0);
}

static bool pd_msg_parse_capabilities(PowerUsbPd* pd, uint8_t* buf, size_t nb_objects) {
    if(nb_objects == 0) {
        return false;
    }

    furi_mutex_acquire(pd->cap_mutex, FuriWaitForever);

    size_t pdo_num = 0;
    for(uint8_t i = 0; i < nb_objects; i++) {
        PdPowerDataObject pdo = {0};
        memcpy(&pdo.data, &buf[i * 4], 4);

        if(pdo.type == 0) { // Fixed PDO
            uint32_t voltage = pdo.fixed.voltage * 50;
            uint32_t current = pdo.fixed.current_max * 10;
            if(current > PD_MAX_CURRENT) {
                current = PD_MAX_CURRENT;
            }
            pd->caps.cap[pdo_num].is_fixed = true;
            pd->caps.cap[pdo_num].voltage_min = voltage;
            pd->caps.cap[pdo_num].voltage_max = voltage;
            pd->caps.cap[pdo_num].current_max = current;
            pd->caps.cap[pdo_num].pdo_id = i + 1;
            pdo_num++;
        } else if(pdo.type == 3) { // APDO
            if(pdo.apdo_type == 0) { // SPR PPS
                uint32_t vmax = pdo.pps.voltage_max * 100;
                uint32_t vmin = pdo.pps.voltage_min * 100;
                uint32_t current = pdo.pps.current_max * 50;
                if(current > PD_MAX_CURRENT) {
                    current = PD_MAX_CURRENT;
                }
                pd->caps.cap[pdo_num].is_fixed = false;
                pd->caps.cap[pdo_num].voltage_min = vmin;
                pd->caps.cap[pdo_num].voltage_max = vmax;
                pd->caps.cap[pdo_num].current_max = current;
                pd->caps.cap[pdo_num].pdo_id = i + 1;
                pdo_num++;
            } else {
                // SPR/EPR AVS
            }
        } else {
            // Battery/variable
        }
    }
    pd->caps.cap_number = pdo_num;
    furi_mutex_release(pd->cap_mutex);
    return true;
}

static void pd_msg_parse_sop0(PowerUsbPd* pd, uint8_t* buf) {
    PdHeaderSop0 hdr = {0};
    memcpy(&hdr.data, buf, 2);

    if((hdr.power_role != PdPowerRoleSrc) || (hdr.data_role != PdDataRoleDFP)) {
        return;
    }

    if(hdr.extended == 1) {
        pd_send_not_supported(pd);
        FURI_LOG_W(TAG, "Unsupported ext msg %u", hdr.msg_type);
    } else if(hdr.nb_objects == 0) {
        // Control messages
        if(hdr.msg_type == PdMsgGoodCrc) { // GoodCRC
            pd->tx_msg_id = (pd->tx_msg_id + 1) & 0x7;
        } else if(hdr.msg_type == PdMsgAccept) {
            // Accept -> update current capability
            furi_mutex_acquire(pd->cap_mutex, FuriWaitForever);
            pd->caps.cap_id_current = pd->req_cap_id;
            furi_mutex_release(pd->cap_mutex);
        } else if(hdr.msg_type == PdMsgPsRdy) {
            // PS_RDY -> mode switch done
            if(pd->pps_keep_alive == false) {
                Power* power = furi_record_open(RECORD_POWER);
                power_on_usb_pd_update(power, pd->req_voltage, pd->req_current);
                furi_record_close(RECORD_POWER);
            } else {
                pd->pps_keep_alive = false;
            }
        } else if(hdr.msg_type == PdMsgGetSinkCaps) {
            // Sink capabilities
            pd_send_sink_caps(pd);
        } else {
            pd_send_not_supported(pd);
            FURI_LOG_W(TAG, "Unsupported control msg %u", hdr.msg_type);
        }
    } else if(hdr.nb_objects > 0) {
        // Data messages
        if(hdr.msg_type == PdMsgSourceCaps) {
            // Source capabilities
            if(pd_msg_parse_capabilities(pd, &buf[2], hdr.nb_objects)) {
                PdMessage msg = {
                    .event = PdEventIdRequest,
                    .request_power = {.voltage_mv = PD_VOLTAGE_DEFAULT, .current_ma = 0},
                };
                furi_message_queue_put(pd->message_queue, &msg, FuriWaitForever);
            }
        } else {
            pd_send_not_supported(pd);
            FURI_LOG_W(TAG, "Unsupported data msg %u", hdr.msg_type);
        }
    }
}

static bool pd_is_ack_required(PowerUsbPd* pd, uint8_t ord_set, uint8_t* buf, uint8_t* msg_id) {
    if(ord_set != PdOrdSetSOP0) {
        // We only support SOP messages
        return false;
    }

    PdHeaderSop0 hdr = {0};
    memcpy(&hdr.data, buf, 2);

    *msg_id = hdr.msg_id;

    if((hdr.power_role == PdPowerRoleSrc) && (hdr.data_role == PdDataRoleDFP) &&
       (hdr.nb_objects == 0)) {
        // Command
        pd->src_rev_id = hdr.revision;
        if(hdr.msg_type == PdMsgGoodCrc) {
            // Don't respond GoodCRC to GoodCRC
            return false;
        }
        if((hdr.msg_type > 0) && (hdr.msg_type < PdMsgControlMax)) {
            return true;
        }
    } else if(
        (hdr.power_role == PdPowerRoleSrc) && (hdr.data_role == PdDataRoleDFP) &&
        (hdr.nb_objects > 0)) {
        // Data
        pd->src_rev_id = hdr.revision;
        if(hdr.msg_type == PdMsgVendor) { // VDM
            return true;
        }
        if((hdr.msg_type > 0) && (hdr.msg_type < PdMsgDataMax)) {
            return true;
        }
    }

    return false;
}

static void ucpd_irq_handler(void* context) {
    PowerUsbPd* pd = context;
    uint32_t irq_flags = UCPD1->SR;

    if(irq_flags & (UCPD_SR_TYPECEVT1 | UCPD_SR_TYPECEVT2)) {
        UCPD1->ICR |= (UCPD_ICR_TYPECEVT1CF | UCPD_ICR_TYPECEVT2CF);
        uint8_t cc1_state = (UCPD1->SR & UCPD_SR_TYPEC_VSTATE_CC1) >> UCPD_SR_TYPEC_VSTATE_CC1_Pos;
        uint8_t cc2_state = (UCPD1->SR & UCPD_SR_TYPEC_VSTATE_CC2) >> UCPD_SR_TYPEC_VSTATE_CC2_Pos;
        uint8_t cc_state = (cc1_state << 2) | cc2_state;
        ucpd_cc_detector_control(false);

        if(cc_state != pd->cc_irq_last) {
            pd->cc_irq_last = cc_state;
            PdMessage msg = {
                .event = PdEventIdCCState,
                .cc_state = {.cc1 = cc1_state, .cc2 = cc2_state},
            };
            furi_message_queue_put(pd->message_queue, &msg, 0);
        }
    }
    if(irq_flags & UCPD_SR_TXMSGSENT) {
        UCPD1->ICR |= UCPD_ICR_TXMSGSENTCF;
        furi_semaphore_release(pd->tx_idle_sem);
    }
    if(irq_flags & UCPD_SR_RXORDDET) {
        UCPD1->ICR |= UCPD_ICR_RXORDDETCF;
        pd->rx_irq_ord_set = UCPD1->RX_ORDSET & UCPD_RX_ORDSET_RXORDSET;
    }
    if(irq_flags & UCPD_SR_RXMSGEND) {
        UCPD1->ICR |= UCPD_ICR_RXMSGENDCF;
        LL_DMA_DisableChannel(GPDMA1, pd->rx_dma_ch);

        bool rx_error = (UCPD1->SR & UCPD_SR_RXERR);
        if(UCPD1->SR & UCPD_SR_RXOVR) {
            UCPD1->ICR |= UCPD_ICR_RXOVRCF;
        }

        if(rx_error == false) {
            pd->rx_msg.event = PdEventIdRxDone;
            pd->rx_msg.rx_done.ord_set_type = pd->rx_irq_ord_set;
            if(furi_message_queue_put(pd->message_queue, &(pd->rx_msg), 0) == FuriStatusOk) {
                uint8_t msg_id = 0;
                if(pd_is_ack_required(pd, pd->rx_irq_ord_set, pd->rx_msg.rx_done.data, &msg_id)) {
                    pd_send_goodcrc(pd, msg_id);
                }
            }
        } else {
            PdMessage msg = {.event = PdEventIdRxError};
            furi_message_queue_put(pd->message_queue, &msg, 0);
        }

        LL_DMA_ConfigAddresses(
            GPDMA1, pd->rx_dma_ch, (uint32_t)&UCPD1->RXDR, (uint32_t)(pd->rx_msg.rx_done.data));
        LL_DMA_SetBlkDataLength(GPDMA1, pd->rx_dma_ch, PD_MAX_PACKET_LEN);
        LL_DMA_EnableChannel(GPDMA1, pd->rx_dma_ch);
    }
    if(irq_flags & UCPD_SR_RXHRSTDET) {
        UCPD1->ICR |= UCPD_ICR_RXHRSTDETCF;
        PdMessage msg = {.event = PdEventIdHrst};
        furi_message_queue_put(pd->message_queue, &msg, 0);
    }
    if(irq_flags & UCPD_SR_TXMSGDISC) {
        UCPD1->ICR |= UCPD_ICR_TXMSGDISCCF;
        // TODO: retry send??
    }
}

static void pd_cc_poll_callback(void* context) {
    furi_check(context);
    PowerUsbPd* pd = context;

    PdMessage msg = {.event = PdEventIdCCPoll};
    furi_message_queue_put(pd->message_queue, &msg, FuriWaitForever);
}

static void pd_pps_keep_alive_callback(void* context) {
    furi_check(context);
    PowerUsbPd* pd = context;

    PdMessage msg = {.event = PdEventIdPpsKeepAlive};
    furi_message_queue_put(pd->message_queue, &msg, FuriWaitForever);
}

static void pd_cc_line_change(PowerUsbPd* pd) {
    uint32_t max_current = PdCClineCurrentUsb500ma;

    if(pd->cc_line != PdCClineLevelNC) {
        LL_UCPD_SetCCPin(UCPD1, pd->cc_line == 1 ? LL_UCPD_CCPIN_CC1 : LL_UCPD_CCPIN_CC2);

        pd->tx_msg_id = 0;
        pd->src_rev_id = 2;

        LL_DMA_ConfigAddresses(
            GPDMA1, pd->rx_dma_ch, (uint32_t)&UCPD1->RXDR, (uint32_t)(pd->rx_msg.rx_done.data));
        LL_DMA_SetBlkDataLength(GPDMA1, pd->rx_dma_ch, PD_MAX_PACKET_LEN);
        LL_DMA_EnableChannel(GPDMA1, pd->rx_dma_ch);
        LL_UCPD_EnableIT_RxOrderSet(UCPD1);
        LL_UCPD_EnableIT_RxMsgEnd(UCPD1);
        LL_UCPD_EnableIT_TxMSGSENT(UCPD1);
        LL_UCPD_EnableIT_TxMSGDISC(UCPD1);

        max_current = PdCClineCurrentUsb500ma; // USB default (0.5A)
        if(pd->cc_line_level == PdCClineLevel1500ma) {
            max_current = PdCClineCurrent1500ma; // 1.5A
        } else if(pd->cc_line_level == PdCClineLevel3000ma) {
            max_current = PdCClineCurrent3000ma; // 3A
        }

    } else {
        LL_UCPD_RxDisable(UCPD1);
        LL_DMA_DisableChannel(GPDMA1, pd->rx_dma_ch);

        LL_UCPD_DisableIT_RxOrderSet(UCPD1);
        LL_UCPD_DisableIT_RxMsgEnd(UCPD1);
        LL_UCPD_DisableIT_TxMSGSENT(UCPD1);
        LL_UCPD_DisableIT_TxMSGDISC(UCPD1);

        max_current = PdCClineCurrentUsb500ma; // No PD - use USB limit (0.5A)
    }

    furi_mutex_acquire(pd->cap_mutex, FuriWaitForever);
    pd->caps.cc_line = pd->cc_line;
    pd->caps.passive_mode_current = max_current;
    pd->caps.cap_number = 0;
    pd->caps.cap_id_current = 0;
    furi_mutex_release(pd->cap_mutex);

    Power* power = furi_record_open(RECORD_POWER);
    power_on_usb_pd_update(power, 5000, max_current);
    furi_record_close(RECORD_POWER);
}

static void pd_reset_source(PowerUsbPd* pd) {
    uint32_t cc1_state = (UCPD1->SR & UCPD_SR_TYPEC_VSTATE_CC1) >> UCPD_SR_TYPEC_VSTATE_CC1_Pos;
    uint32_t cc2_state = (UCPD1->SR & UCPD_SR_TYPEC_VSTATE_CC2) >> UCPD_SR_TYPEC_VSTATE_CC2_Pos;
    pd->cc_line = 0;
    pd->cc_line_level = PdCClineLevelNC;
    if((cc1_state == PdCClineLevelNC) && (cc2_state != PdCClineLevelNC)) {
        pd->cc_line = 2;
        pd->cc_line_level = cc2_state;
    } else if((cc2_state == PdCClineLevelNC) && (cc1_state != PdCClineLevelNC)) {
        pd->cc_line = 1;
        pd->cc_line_level = cc1_state;
    }
    pd_cc_line_change(pd);
    if(pd->cc_line > 0) {
        LL_UCPD_SendHardReset(UCPD1);
        LL_UCPD_RxEnable(UCPD1);
    }
}

static void pd_fallback(PowerUsbPd* pd) {
    LL_UCPD_RxDisable(UCPD1);
    LL_DMA_DisableChannel(GPDMA1, pd->rx_dma_ch);
    LL_UCPD_DisableIT_RxOrderSet(UCPD1);
    LL_UCPD_DisableIT_RxMsgEnd(UCPD1);
    LL_UCPD_DisableIT_TxMSGSENT(UCPD1);
    LL_UCPD_DisableIT_TxMSGDISC(UCPD1);

    uint32_t max_current = PdCClineCurrentUsb500ma; // USB default (0.5A)
    if(pd->cc_line_level == PdCClineLevel1500ma) {
        max_current = PdCClineCurrent1500ma; // 1.5A
    } else if(pd->cc_line_level == PdCClineLevel3000ma) {
        max_current = PdCClineCurrent3000ma; // 3A
    }
    furi_mutex_acquire(pd->cap_mutex, FuriWaitForever);
    pd->caps.cc_line = pd->cc_line;
    pd->caps.passive_mode_current = max_current;
    pd->caps.cap_number = 0;
    pd->caps.cap_id_current = 0;
    furi_mutex_release(pd->cap_mutex);

    Power* power = furi_record_open(RECORD_POWER);
    power_on_usb_pd_update(power, 5000, max_current);
    furi_record_close(RECORD_POWER);
}

PowerUsbPd* power_usb_pd_alloc(FuriMessageQueue** pd_queue) {
    PowerUsbPd* pd = malloc(sizeof(PowerUsbPd));
    pd->message_queue = furi_message_queue_alloc(16, sizeof(PdMessage));
    pd->cc_poll_timer = furi_timer_alloc(pd_cc_poll_callback, FuriTimerTypePeriodic, pd);
    pd->pps_keep_alive_timer = furi_timer_alloc(pd_pps_keep_alive_callback, FuriTimerTypeOnce, pd);
    pd->tx_idle_sem = furi_semaphore_alloc(1, 1);
    pd->cap_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    pd->started = false;

    *pd_queue = pd->message_queue;
    return pd;
}

void power_usb_pd_start(PowerUsbPd* pd) {
    furi_assert(pd);
    PdMessage msg = {.event = PdEventIdStart};
    furi_check(furi_message_queue_put(pd->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
}

void power_usb_pd_get_capabilities(PowerUsbPd* pd, PowerUsbPdCapability* caps) {
    furi_assert(pd);
    furi_assert(caps);

    furi_mutex_acquire(pd->cap_mutex, FuriWaitForever);
    memcpy(caps, &pd->caps, sizeof(PowerUsbPdCapability));
    furi_mutex_release(pd->cap_mutex);
}

void power_usb_pd_request_power(PowerUsbPd* pd, uint32_t voltage_mv, uint32_t current_ma) {
    furi_assert(pd);
    PdMessage msg = {
        .event = PdEventIdRequest,
        .request_power = {.voltage_mv = voltage_mv, .current_ma = current_ma},
    };
    furi_check(furi_message_queue_put(pd->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
}

void power_usb_pd_msg_handler(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    PowerUsbPd* pd = context;
    furi_assert(object == pd->message_queue);

    PdMessage msg;
    furi_check(furi_message_queue_get(pd->message_queue, &msg, 0) == FuriStatusOk);

    if(msg.event == PdEventIdCCState) {
        uint8_t cc_line_cur = 0;
        uint8_t cc_line_level = PdCClineLevelNC;
        if((msg.cc_state.cc1 == PdCClineLevelNC) && (msg.cc_state.cc2 != PdCClineLevelNC)) {
            cc_line_cur = 2;
            cc_line_level = msg.cc_state.cc2;
        } else if((msg.cc_state.cc2 == PdCClineLevelNC) && (msg.cc_state.cc1 != PdCClineLevelNC)) {
            cc_line_cur = 1;
            cc_line_level = msg.cc_state.cc1;
        }

        if(pd->cc_line != cc_line_cur) {
            pd->cc_line = cc_line_cur;
            pd->cc_line_level = cc_line_level;
            FURI_LOG_D(TAG, "CC change line:%u level:%u", pd->cc_line, pd->cc_line_level);
            pd->pps_keep_alive = false;
            pd->hrst_count = 0;
            pd->req_voltage = PD_VOLTAGE_DEFAULT;
            pd->req_current = 0;
            pd_cc_line_change(pd);
            if(pd->cc_line > 0) {
                LL_UCPD_RxEnable(UCPD1);
            }
        }
    } else if((msg.event == PdEventIdRxDone) && (pd->cc_line != 0)) {
        if(msg.rx_done.ord_set_type == PdOrdSetSOP0) {
            pd_msg_parse_sop0(pd, msg.rx_done.data);
        } else {
            FURI_LOG_W(TAG, "Unknown Ordered Set type %u", msg.rx_done.ord_set_type);
        }
    } else if(msg.event == PdEventIdRxError) {
        FURI_LOG_W(TAG, "Rx error");
    } else if(msg.event == PdEventIdHrst) {
        pd->req_voltage = PD_VOLTAGE_DEFAULT;
        pd->req_current = 0;
        pd->pps_keep_alive = false;
        furi_timer_stop(pd->pps_keep_alive_timer);
        furi_mutex_acquire(pd->cap_mutex, FuriWaitForever);
        pd->caps.cap_id_current = 0;
        furi_mutex_release(pd->cap_mutex);
        FURI_LOG_W(TAG, "HRST");
        pd->hrst_count++;
        if(pd->hrst_count >= RX_IGNORE_HRST_COUNT) {
            pd->hrst_count = 0;
            pd_fallback(pd);
            FURI_LOG_W(TAG, "Fallback to passive mode");
        }
    } else if(msg.event == PdEventIdRequest) {
        pd_send_request(pd, msg.request_power.voltage_mv, msg.request_power.current_ma);
    } else if(msg.event == PdEventIdPpsKeepAlive) {
        pd->pps_keep_alive = true;
        pd_send_request(pd, pd->req_voltage, pd->req_current);
    } else if(msg.event == PdEventIdCCPoll) {
        ucpd_cc_detector_control(true);
    } else if(msg.event == PdEventIdStart) {
        furi_check(pd->started == false);
        ucpd_init(pd);
        pd_reset_source(pd);

        UCPD1->CR |= UCPD_CR_CC1TCDIS_Msk | UCPD_CR_CC2TCDIS_Msk;
        furi_check(furi_timer_start(pd->cc_poll_timer, PD_CC_POLL_PERIOD) == FuriStatusOk);

        pd->req_voltage = PD_VOLTAGE_DEFAULT;
        pd->req_current = 0;

        pd->started = true;
    }
}
