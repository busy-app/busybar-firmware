#include <furi_hal.h>
#include <furi_hal_interrupt.h>
#include "furi_hal_usb_i.h"
#include <toolbox/api_lock.h>
#include <stm32u5xx_ll_pwr.h>
#include <class/net/net_device.h>
#include <class/net/ncm.h>

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

    FuriEventLoop* event_loop;
    FuriSemaphore* usb_semaphore;
    FuriSemaphore* net_semaphore;

    // TinyUsb driver struct
    usbd_class_driver_t tu_driver;

    // Qualifier descriptor lives here and is copied from device descriptor
    tusb_desc_device_qualifier_t desc_qualifier;
    uint16_t desc_string_temp[USB_DESC_STRING_LEN_MAX + 1];
    bool enabled;
    bool connected;
} usb_service = {0};

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

void tud_event_hook_cb(uint8_t rhport, uint32_t eventid, bool in_isr) {
    UNUSED(rhport);
    UNUSED(eventid);
    UNUSED(in_isr);

    furi_semaphore_release(usb_service.usb_semaphore);
}

void furi_hal_usb_irq(void* ctx) {
    UNUSED(ctx);
    tusb_int_handler(BOARD_TUD_RHPORT, true);
}

static void furi_hal_usb_handler(FuriEventLoopObject* object, void* context) {
    UNUSED(context);
    // Power* instance = context;

    // furi_assert(instance);
    furi_assert(usb_service.usb_semaphore == object);
    furi_check(furi_semaphore_acquire(object, 0) == FuriStatusOk);

    do {
        tud_task_ext(0, false);
    } while(tud_task_event_ready());
}

void network_handle(void);
void network_start(FuriSemaphore* usb_sem);

static void furi_hal_usbnet_handler(FuriEventLoopObject* object, void* context) {
    UNUSED(context);
    // Power* instance = context;

    // furi_assert(instance);
    furi_assert(usb_service.net_semaphore == object);
    furi_check(furi_semaphore_acquire(object, 0) == FuriStatusOk);

    network_handle();
}

static int32_t furi_hal_usb_thread(void* context) {
    UNUSED(context);

    usb_service.event_loop = furi_event_loop_alloc();
    usb_service.usb_semaphore = furi_semaphore_alloc(1, 1);
    furi_event_loop_subscribe_semaphore(
        usb_service.event_loop,
        usb_service.usb_semaphore,
        FuriEventLoopEventIn,
        furi_hal_usb_handler,
        &usb_service);

    usb_service.net_semaphore = furi_semaphore_alloc(1, 0);
    furi_event_loop_subscribe_semaphore(
        usb_service.event_loop,
        usb_service.net_semaphore,
        FuriEventLoopEventIn,
        furi_hal_usbnet_handler,
        &usb_service);

    network_start(usb_service.net_semaphore);

    usb_service.enabled = false;
    usb_service.connected = false;

    furi_delay_ms(1000);

    tusb_rhport_init_t dev_init = {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO,
    };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);

    furi_event_loop_run(usb_service.event_loop);

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

    furi_hal_interrupt_set_isr_ex(
        FuriHalInterruptIdUSBHS, FuriHalInterruptPriorityHighest, furi_hal_usb_irq, NULL);

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
    usb_service.thread = furi_thread_alloc_service("UsbDriver", 4096, furi_hal_usb_thread, NULL);
    furi_thread_start(usb_service.thread);

    FURI_LOG_I(TAG, "Init OK");
}

bool furi_hal_usb_set_config(FuriHalUsbInterface* new_if, void* ctx) {
    UNUSED(new_if);
    UNUSED(ctx);

    return false;
}

enum {
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_SERIAL,
    STRID_INTERFACE,
    STRID_MAC
};

enum {
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
    ITF_NUM_TOTAL
};

enum {
    CONFIG_ID_NCM = 0,
    CONFIG_ID_COUNT
};

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,

    // Use Interface Association Descriptor (IAD) device class
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,

    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = 0x37C1,
    .idProduct = 0x0001,
    .bcdDevice = 0x0101,

    .iManufacturer = STRID_MANUFACTURER,
    .iProduct = STRID_PRODUCT,
    .iSerialNumber = STRID_SERIAL,

    .bNumConfigurations = CONFIG_ID_COUNT // multiple configurations
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const*)&desc_device;
}

#define NCM_CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_NCM_DESC_LEN)

#define EPNUM_NET_NOTIF 0x81
#define EPNUM_NET_OUT   0x02
#define EPNUM_NET_IN    0x82

static uint8_t const ncm_configuration[] = {
    // Config number (index+1), interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(CONFIG_ID_NCM + 1, ITF_NUM_TOTAL, 0, NCM_CONFIG_TOTAL_LEN, 0, 100),

    // Interface number, description string index, MAC address string index, EP notification address and size, EP data address (out, in), and size, max segment size.
    TUD_CDC_NCM_DESCRIPTOR(
        ITF_NUM_CDC,
        STRID_INTERFACE,
        STRID_MAC,
        EPNUM_NET_NOTIF,
        64,
        EPNUM_NET_OUT,
        EPNUM_NET_IN,
        CFG_TUD_NET_ENDPOINT_SIZE,
        CFG_TUD_NET_MTU),
};

static uint8_t const* const configuration_arr[] = {[CONFIG_ID_NCM] = ncm_configuration};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    return (index < CONFIG_ID_COUNT) ? configuration_arr[index] : NULL;
}

// array of pointer to string descriptors
static char const* string_desc_arr[] = {
    [STRID_LANGID] = (const char[]){0x09, 0x04}, // supported language is English (0x0409)
    [STRID_MANUFACTURER] = "TinyUSB", // Manufacturer
    [STRID_PRODUCT] = "TinyUSB Device", // Product
    [STRID_SERIAL] = "0", // Serials will use unique ID if possible
    [STRID_INTERFACE] = "TinyUSB Network Interface" // Interface Description

};

static uint16_t _desc_str[32 + 1];

uint8_t tud_network_mac_address[6] = {0x0C, 0xFA, 0x22, 0x01, 0x23, 0x45};

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    unsigned int chr_count = 0;

    switch(index) {
    case STRID_LANGID:
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
        break;

    case STRID_MAC:
        // Convert MAC address into UTF-16
        for(unsigned i = 0; i < sizeof(tud_network_mac_address); i++) {
            _desc_str[1 + chr_count++] =
                "0123456789ABCDEF"[(tud_network_mac_address[i] >> 4) & 0xf];
            _desc_str[1 + chr_count++] =
                "0123456789ABCDEF"[(tud_network_mac_address[i] >> 0) & 0xf];
        }
        break;

    default:
        // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

        if(!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))) return NULL;

        const char* str = string_desc_arr[index];

        // Cap at max char
        chr_count = strlen(str);
        size_t const max_count =
            sizeof(_desc_str) / sizeof(_desc_str[0]) - 1; // -1 for string type
        if(chr_count > max_count) chr_count = max_count;

        // Convert ASCII string into UTF-16
        for(size_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
        break;
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

    return _desc_str;
}
