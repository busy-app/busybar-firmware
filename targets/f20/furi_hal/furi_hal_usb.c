#include <furi_hal.h>
#include "furi_hal_usb_i.h"
#include <toolbox/api_lock.h>
#include <stm32u5xx_ll_pwr.h>

#define TAG "USB"

#define USB_RECONNECT_DELAY     500
#define USB_DESC_STRING_LEN_MAX (32)
#define USB_LANGID_EN           ((const char[]){0x09, 0x04})

typedef enum {
    UsbApiEventTypeSetConfig,
    UsbApiEventTypeEnable,
    UsbApiEventTypeDisable,
} UsbApiEventType;

typedef struct {
    FuriHalUsbInterface* interface;
    void* context;
} UsbApiEventDataInterface;

typedef union {
    UsbApiEventDataInterface interface;
} UsbApiEventData;

typedef union {
    bool bool_value;
    void* void_value;
} UsbApiEventReturnData;

typedef struct {
    FuriApiLock lock;
    UsbApiEventType type;
    UsbApiEventData data;
    UsbApiEventReturnData* return_data;
} UsbApiEventMessage;

static struct FuriHalUsbCfg {
    FuriThread* thread;
    FuriMessageQueue* queue;
    FuriHalUsbInterface* cfg;
    void* cfg_context;
    void* cfg_inst; //TODO: pass to all callbacks

    // TinyUsb driver struct
    usbd_class_driver_t tu_driver;

    // Qualifier descriptor lives here and is copied from device descriptor
    tusb_desc_device_qualifier_t desc_qualifier;
    uint16_t desc_string_temp[USB_DESC_STRING_LEN_MAX + 1];
    bool enabled;
    bool connected;
} usb_service = {0};

static tusb_desc_device_t const desc_device_dummy = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = VERSION_BCD(2, 0, 0),

    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = 0,
    .idProduct = 0,
    .bcdDevice = VERSION_BCD(1, 0, 0),

    .iManufacturer = 0,
    .iProduct = 0,
    .iSerialNumber = 0,

    .bNumConfigurations = 0x01,
};

static uint8_t const desc_cfg_dummy[] = {
    TUD_CONFIG_DESCRIPTOR(1, 0, 0, TUD_CONFIG_DESC_LEN, 0x00, 100),
};

static void tud_drv_dummy_init(void);
static void tud_drv_dummy_reset(uint8_t rhport);
static uint16_t
    tud_drv_dummy_open(uint8_t rhport, tusb_desc_interface_t const* desc_intf, uint16_t max_len);
static bool tud_drv_dummy_control_xfer_cb(
    uint8_t rhport,
    uint8_t stage,
    tusb_control_request_t const* request);
static bool tud_drv_dummy_xfer_cb(
    uint8_t rhport,
    uint8_t ep_addr,
    xfer_result_t result,
    uint32_t xferred_bytes);

static FuriHalUsbInterface cfg_dummy = {
    .init = NULL,
    .deinit = NULL,
    .reset = tud_drv_dummy_reset,
    .open = tud_drv_dummy_open,
    .control_xfer_cb = tud_drv_dummy_control_xfer_cb,
    .xfer_cb = tud_drv_dummy_xfer_cb,
    .sof = NULL,
    .connect_state = NULL,
    .dev_descr = (tusb_desc_device_t*)&desc_device_dummy,
    .str_manuf_descr = NULL,
    .str_prod_descr = NULL,
    .str_serial_descr = NULL,
    .cfg_fs_descr = (uint8_t*)desc_cfg_dummy,
    .cfg_hs_descr = (uint8_t*)desc_cfg_dummy,
};

usbd_class_driver_t const* usbd_app_driver_get_cb(uint8_t* driver_count) {
    *driver_count = 1;
    return &usb_service.tu_driver;
}

uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const*)(usb_service.cfg->dev_descr);
}

uint8_t const* tud_descriptor_device_qualifier_cb(void) {
    return (uint8_t const*)&(usb_service.desc_qualifier);
}

uint8_t const* tud_descriptor_other_speed_configuration_cb(uint8_t index) {
    UNUSED(index); // for multiple configurations

    // if link speed is high return fullspeed config, and vice versa
    return (tud_speed_get() == TUSB_SPEED_HIGH) ? usb_service.cfg->cfg_fs_descr :
                                                  usb_service.cfg->cfg_hs_descr;
}

uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    UNUSED(index); // for multiple configurations

    // Although we are highspeed, host may be fullspeed.
    return (tud_speed_get() == TUSB_SPEED_HIGH) ? usb_service.cfg->cfg_hs_descr :
                                                  usb_service.cfg->cfg_fs_descr;
}

void tud_mount_cb(void) {
    usb_service.connected = true;

    if(usb_service.cfg) {
        if(usb_service.cfg->connect_state) {
            usb_service.cfg->connect_state(true);
        }
    }
}

void tud_suspend_cb(bool remote_wakeup_en) {
    UNUSED(remote_wakeup_en);
    if(usb_service.connected) {
        usb_service.connected = false;
        if(usb_service.cfg) {
            if(usb_service.cfg->connect_state) {
                usb_service.cfg->connect_state(false);
            }
        }
    }
}

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    UNUSED(langid);
    size_t desc_len = 0;

    if(index == UsbDevLang) {
        memcpy(&usb_service.desc_string_temp[1], USB_LANGID_EN, 2);
        desc_len = 2;

    } else {
        char* string_src = NULL;
        if(index == UsbDevManuf) {
            string_src = usb_service.cfg->str_manuf_descr;
        } else if(index == UsbDevProduct) {
            string_src = usb_service.cfg->str_prod_descr;
        } else if(index == UsbDevSerial) {
            string_src = usb_service.cfg->str_serial_descr;
        }

        if((string_src == NULL) && (usb_service.cfg->str_desc_custom)) {
            string_src = usb_service.cfg->str_desc_custom(BOARD_TUD_RHPORT, index);
        }

        do {
            if(string_src == NULL) break;

            size_t string_len = strlen(string_src);
            if(string_len == 0) break;

            if(string_len > USB_DESC_STRING_LEN_MAX) {
                string_len = USB_DESC_STRING_LEN_MAX;
            }

            // Convert ASCII string into UTF-16
            for(size_t i = 0; i < string_len; i++) {
                usb_service.desc_string_temp[1 + i] = string_src[i];
            }

            desc_len = string_len * 2;

        } while(0);
    }

    if(desc_len > 0) {
        usb_service.desc_string_temp[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (desc_len + 2));
        return usb_service.desc_string_temp;
    } else {
        return NULL;
    }
}

static void tud_drv_dummy_init(void) {
    // Not used
}

static void tud_drv_dummy_reset(uint8_t rhport) {
    UNUSED(rhport);
}

static uint16_t
    tud_drv_dummy_open(uint8_t rhport, tusb_desc_interface_t const* desc_intf, uint16_t max_len) {
    UNUSED(rhport);
    UNUSED(desc_intf);
    UNUSED(max_len);
    return 0;
}

static bool tud_drv_dummy_control_xfer_cb(
    uint8_t rhport,
    uint8_t stage,
    tusb_control_request_t const* request) {
    UNUSED(rhport);
    UNUSED(stage);
    UNUSED(request);
    return false;
}

static bool tud_drv_dummy_xfer_cb(
    uint8_t rhport,
    uint8_t ep_addr,
    xfer_result_t result,
    uint32_t xferred_bytes) {
    UNUSED(rhport);
    UNUSED(ep_addr);
    UNUSED(result);
    UNUSED(xferred_bytes);
    return true;
}

void OTG_HS_IRQHandler(void) {
    tud_int_handler(BOARD_TUD_RHPORT); // TODO: move to furi_hal_interrupt
}

static void usb_make_qualifier_desc(tusb_desc_device_t* dev) {
    furi_check(dev);
    usb_service.desc_qualifier.bLength = sizeof(tusb_desc_device_qualifier_t);
    usb_service.desc_qualifier.bDescriptorType = TUSB_DESC_DEVICE_QUALIFIER;
    usb_service.desc_qualifier.bcdUSB = dev->bcdUSB;
    usb_service.desc_qualifier.bDeviceClass = dev->bDeviceClass;
    usb_service.desc_qualifier.bDeviceSubClass = dev->bDeviceSubClass;
    usb_service.desc_qualifier.bDeviceProtocol = dev->bDeviceProtocol;
    usb_service.desc_qualifier.bMaxPacketSize0 = dev->bMaxPacketSize0;
    usb_service.desc_qualifier.bNumConfigurations = dev->bNumConfigurations;
    usb_service.desc_qualifier.bReserved = 0;
}

static void usb_process_set_config(FuriHalUsbInterface* cfg_new, void* context) {
    furi_check(cfg_new);
    furi_check(cfg_new->dev_descr);
    furi_check(cfg_new->cfg_fs_descr);
    furi_check(cfg_new->cfg_hs_descr);
    furi_check(cfg_new->reset);
    furi_check(cfg_new->open);
    furi_check(cfg_new->control_xfer_cb);
    furi_check(cfg_new->xfer_cb);

    dcd_int_disable(BOARD_TUD_RHPORT);

    if(usb_service.cfg) {
        if(usb_service.cfg->deinit) {
            usb_service.cfg->deinit(usb_service.cfg_inst);
        }
    }

    usb_service.cfg = cfg_new;
    usb_service.cfg_context = context;

    // Fill Qualifier Descriptor from Device Descriptor
    usb_make_qualifier_desc(usb_service.cfg->dev_descr);

    // Fill TinyUsb driver struct
    usb_service.tu_driver.reset = usb_service.cfg->reset;
    usb_service.tu_driver.open = usb_service.cfg->open;
    usb_service.tu_driver.control_xfer_cb = usb_service.cfg->control_xfer_cb;
    usb_service.tu_driver.xfer_cb = usb_service.cfg->xfer_cb;
    usb_service.tu_driver.sof = usb_service.cfg->sof;

    if(usb_service.cfg->init) {
        usb_service.cfg_inst = usb_service.cfg->init(context);
    } else {
        usb_service.cfg_inst = NULL;
    }

    dcd_int_enable(BOARD_TUD_RHPORT);
}

static bool usb_process_change_mode(FuriHalUsbInterface* cfg_new, void* context) {
    if((cfg_new != usb_service.cfg) || (context != usb_service.cfg_context)) {
        if(usb_service.enabled) {
            tud_suspend_cb(false);
            tud_disconnect();
            usb_service.enabled = false;
            furi_delay_ms(USB_RECONNECT_DELAY);
        }
        if(cfg_new) {
            usb_process_set_config(cfg_new, context);
            tud_connect();
            usb_service.enabled = true;
        } else {
            usb_process_set_config(&cfg_dummy, NULL);
        }
    }
    return true;
}

static void usb_process_enable(bool enable) {
    if(enable) {
        if((!usb_service.enabled) && (usb_service.cfg != NULL)) {
            tud_connect();
            usb_service.enabled = true;
        }
    } else {
        if(usb_service.enabled) {
            tud_suspend_cb(false);
            tud_disconnect();
            usb_service.enabled = false;
        }
    }
}

static void usb_process_message(UsbApiEventMessage* message) {
    switch(message->type) {
    case UsbApiEventTypeSetConfig:
        message->return_data->bool_value = usb_process_change_mode(
            message->data.interface.interface, message->data.interface.context);
        break;
    case UsbApiEventTypeDisable:
        usb_process_enable(false);
        break;
    case UsbApiEventTypeEnable:
        usb_process_enable(true);
        break;
    }

    api_lock_unlock(message->lock);
}

static void furi_hal_usb_send_message(UsbApiEventMessage* message) {
    furi_message_queue_put(usb_service.queue, message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message->lock);
}

static int32_t furi_hal_usb_thread(void* context) {
    UNUSED(context);

    usb_service.tu_driver.init = tud_drv_dummy_init;
    usb_service.enabled = false;
    usb_service.connected = false;

    tud_init(BOARD_TUD_RHPORT);
    tud_disconnect();
    usb_process_set_config(&cfg_dummy, NULL);

    while(1) {
        tud_task_ext(10, false);

        UsbApiEventMessage message;
        if(furi_message_queue_get(usb_service.queue, &message, 0) == FuriStatusOk) {
            usb_process_message(&message);
        }
    }
    return 0;
}

void furi_hal_usb_init(void) {
    // USB Clock
    LL_RCC_SetUSBPHYClockSource(LL_RCC_USBPHYCLKSOURCE_HSE);
    MODIFY_REG(
        SYSCFG->OTGHSPHYCR,
        SYSCFG_OTGHSPHYCR_CLKSEL,
        SYSCFG_OTGHSPHYCR_CLKSEL_0 | SYSCFG_OTGHSPHYCR_CLKSEL_1); // TODO: HSE value

    furi_hal_gpio_init_ex(
        &gpio_usb_dm, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedHigh, GpioAltFn10USB_HS);
    furi_hal_gpio_init_ex(
        &gpio_usb_dp, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedHigh, GpioAltFn10USB_HS);

    NVIC_SetPriority(OTG_HS_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

    furi_hal_bus_enable(FuriHalBusOTG_HS);
    furi_hal_bus_enable(FuriHalBusUSBPHY);

    LL_PWR_EnableVddUSB();
    LL_PWR_EnableUSBPowerSupply();
    LL_PWR_EnableUSBEPODBooster();

    // Configuring the SYSCFG registers OTG_HS PHY
    SYSCFG->OTGHSPHYCR |= SYSCFG_OTGHSPHYCR_EN;

    // Disable VBUS sense (B device)
    USB_OTG_HS->GCCFG &= ~USB_OTG_GCCFG_VBDEN;

    // B-peripheral session valid override enable
    USB_OTG_HS->GCCFG |= USB_OTG_GCCFG_VBVALEXTOEN;
    USB_OTG_HS->GCCFG |= USB_OTG_GCCFG_VBVALOVAL;

    usb_service.queue = furi_message_queue_alloc(1, sizeof(UsbApiEventMessage));
    usb_service.thread = furi_thread_alloc_service("UsbDriver", 1024, furi_hal_usb_thread, NULL);
    furi_thread_start(usb_service.thread);

    FURI_LOG_I(TAG, "Init OK");
}

bool furi_hal_usb_set_config(FuriHalUsbInterface* new_if, void* ctx) {
    UsbApiEventReturnData return_data = {
        .bool_value = false,
    };

    UsbApiEventMessage msg = {
        .lock = api_lock_alloc_locked(),
        .type = UsbApiEventTypeSetConfig,
        .data.interface =
            {
                .interface = new_if,
                .context = ctx,
            },
        .return_data = &return_data,
    };

    furi_hal_usb_send_message(&msg);
    return return_data.bool_value;
}

void furi_hal_usb_enable(bool state) {
    UsbApiEventMessage msg = {
        .lock = api_lock_alloc_locked(),
        .type = state ? UsbApiEventTypeEnable : UsbApiEventTypeDisable,
    };

    furi_hal_usb_send_message(&msg);
}
