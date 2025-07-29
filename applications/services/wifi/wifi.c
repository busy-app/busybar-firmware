#include "wifi_i.h"

#include <lwip/tcpip.h>
#include <lwip/etharp.h>
#include <lwip/dhcp.h>

#include <usb_network/usb_network.h>

#define STARTUP_THREAD_STACK_SIZE (1536UL)

static err_t wifi_link_output(struct netif* netif, struct pbuf* p) {
    Wifi* instance = netif->state;
    furi_assert(instance);

    // FURI_LOG_W(TAG, "%s", __PRETTY_FUNCTION__);

#if(ETH_PAD_SIZE != 0)
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    const size_t tx_size = intercom_tx(
        instance->intercom, IntercomChannelSockets, p->payload, p->len, FuriWaitForever);
    furi_check(tx_size == p->len);

    // FURI_LOG_I(TAG, "Link output: %zu bytes", tx_size);

#if(ETH_PAD_SIZE != 0)
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    return ERR_OK;
}

static err_t wifi_init_netif(struct netif* netif) {
    FURI_LOG_I(TAG, "%s", __PRETTY_FUNCTION__);
    furi_assert(netif);

    netif->mtu = 1019 - 14;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP;
    netif->name[0] = 'W';
    netif->name[1] = 'L';

    netif->output = etharp_output;
    netif->linkoutput = wifi_link_output;

    return ERR_OK;
}

static void wifi_enable_netif(void* arg) {
    FURI_LOG_I(TAG, "%s", __PRETTY_FUNCTION__);
    furi_assert(arg);
    Wifi* instance = arg;

    struct netif* netif = &instance->netif;

    netif_set_link_up(netif);
    netif_set_up(netif);

    const err_t err = dhcp_start(netif);
    if(err != ERR_OK) {
        FURI_LOG_E(TAG, "DHCP error: %d", err);
    }
}

static void wifi_add_netif(void* arg) {
    FURI_LOG_I(TAG, "%s", __PRETTY_FUNCTION__);
    furi_assert(arg);
    Wifi* instance = arg;

    struct netif* netif = &instance->netif;

    // TODO: get hwaddr dynamically
    static const uint8_t hwaddr[6] = {0x8C, 0x8B, 0x48, 0x33, 0xE0, 0x44};

    netif->hwaddr_len = sizeof(netif->hwaddr);
    memcpy(netif->hwaddr, hwaddr, sizeof(netif->hwaddr));

    ip_addr_t ip, netmask, gateway;

    ip_addr_set_zero_ip4(&ip);
    ip_addr_set_zero_ip4(&netmask);
    ip_addr_set_zero_ip4(&gateway);

    netif_add(netif, &ip, &netmask, &gateway, instance, wifi_init_netif, tcpip_input);

    // netif_set_default(netif);
}

static void wifi_net_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(context);
    Wifi* instance = context;

    // FURI_LOG_D(TAG, "RX: %zu bytes", data_size);

    size_t size = data_size;
    const void* src = data;

#if(ETH_PAD_SIZE != 0)
    size += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif
    struct pbuf* p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);

    if(!p) {
        FURI_LOG_E(TAG, "cannot receive frame, pbuf_alloc failed");
        return;
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

    struct netif* netif = &instance->netif;
    const err_t err = netif->input(p, netif);

    if(err != ERR_OK) {
        FURI_LOG_W(TAG, "netif->input failed with error: %d", err);
        pbuf_free(p); /* Free pbuf if input failed */
    }
}

static void wifi_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data_size == sizeof(WifiResponse));
    furi_assert(context);

    Wifi* instance = context;

    memcpy(&instance->response, data, data_size);
    furi_event_loop_set_custom_event(instance->event_loop, WifiEventResponse);
}

static void wifi_publish_state(const Wifi* instance, WifiState state) {
    furi_pubsub_publish(instance->pubsub, &state);
}

static void wifi_update_enabled(Wifi* instance, bool enabled) {
    if(instance->settings_applied) {
        WifiSettings* settings = &instance->settings;

        if(settings->enabled != enabled) {
            settings->enabled = enabled;
            wifi_settings_save(settings);
        }
    }
}

static void wifi_update_connection_params(
    Wifi* instance,
    const WifiCredentials* credentials,
    const WifiIpConfig* ip_config) {
    if(instance->settings_applied) {
        WifiSettings* settings = &instance->settings;

        bool save_file = false;

        if(memcmp(&settings->credentials, credentials, sizeof(WifiCredentials)) != 0) {
            settings->credentials = *credentials;
            save_file = true;
        }

        if(memcmp(&settings->ip_config, ip_config, sizeof(WifiIpConfig)) != 0) {
            settings->ip_config = *ip_config;
            save_file = true;
        }

        if(save_file) {
            wifi_settings_save(settings);
        }
    }
}

static void wifi_process_request(Wifi* instance) {
    const WifiMessage* message = instance->current_message;
    WifiRequest* request = &instance->request;

    const WifiRequestType request_type = message->request_type;
    request->type = request_type;

    if(request_type == WifiRequestTypeConnect) {
        const WifiConnectMessage* connect_message = &message->connect_message;
        WifiConnectRequest* connect_request = &request->connect_request;

        connect_request->credentials = *connect_message->credentials;
        connect_request->ip = *connect_message->ip_config;
    }

    intercom_tx(
        instance->intercom, IntercomChannelWifi, request, sizeof(WifiRequest), FuriWaitForever);
}

static void wifi_process_response(Wifi* instance) {
    WifiMessage* message = instance->current_message;

    if(message == NULL) {
        // BUG: Figure out where the rogue responses come from
        FURI_LOG_W(TAG, "BUG: Rogue response");
        return;
    }

    const WifiResponse* response = &instance->response;
    const WifiRequestType request_type = message->request_type;
    furi_assert(request_type == response->type);

    const WifiStatus status = response->status;

    if(status == WifiStatusOk) {
        if(request_type == WifiRequestTypeInit) {
            wifi_update_enabled(instance, true);

        } else if(request_type == WifiRequestTypeDeinit) {
            wifi_update_enabled(instance, false);

        } else if(request_type == WifiRequestTypeScan) {
            const uint8_t results_count =
                MIN(message->scan_message.max_count, response->scan_results.count);

            const WifiScanResult* results_in = response->scan_results.data;
            WifiScanResult* results_out = message->scan_message.data;

            memcpy(results_out, results_in, results_count * sizeof(WifiScanResult));
            *message->scan_message.count = results_count;

        } else if(request_type == WifiRequestTypeConnect) {
            const WifiConnectMessage* connect_message = &message->connect_message;
            wifi_update_connection_params(
                instance, connect_message->credentials, connect_message->ip_config);

            tcpip_callback(wifi_enable_netif, instance);

            struct netif* netif = &instance->netif;

            while(!dhcp_supplied_address(netif)) {
                FURI_LOG_I(TAG, "Waiting for IP address...");
                furi_delay_ms(1000);
            }

            const uint8_t* addr = (void*)&netif->ip_addr;

            FURI_LOG_I(TAG, "IP address: %hhu.%hhu.%hhu.%hhu", addr[0], addr[1], addr[2], addr[3]);

        } else if(request_type == WifiRequestTypeGetInfo) {
            *message->get_info_message.info = response->info;
        }
    }

    message->status = status;
    api_lock_unlock(message->lock);

    instance->current_message = NULL;
    furi_semaphore_release(instance->access_semaphore);
}

static void wifi_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    Wifi* instance = context;

    if(events == WifiEventRequest) {
        wifi_process_request(instance);
    } else if(events == WifiEventResponse) {
        wifi_process_response(instance);
    } else {
        furi_crash("Multiple Wifi events");
    }
}

static int32_t wifi_startup_thread_callback(void* arg) {
    furi_assert(arg);
    Wifi* instance = arg;

    do {
        wifi_publish_state(instance, WifiStateDeinit);

        WifiInfo info;
        if(wifi_get_info(instance, &info) != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to get info");
            break;
        }

        if(info.state != WifiStateDeinit) {
            if(wifi_deinit(instance) != WifiStatusOk) {
                FURI_LOG_E(TAG, "Failed to deinit");
                break;
            }
        }

        const WifiSettings* settings = &instance->settings;

        if(!settings->enabled) {
            FURI_LOG_I(TAG, "Disabled in settings");
            break;
        }

        if(wifi_init(instance) != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to init");
            break;
        }

        wifi_publish_state(instance, WifiStateDown);

        const char* ssid = settings->credentials.ssid;

        if(strlen(ssid) == 0) {
            FURI_LOG_W(TAG, "No SSID specified");
            break;
        }

        FURI_LOG_I(TAG, "Connecting to \"%s\"", ssid);

        if(wifi_connect(instance, &settings->credentials, &settings->ip_config) != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to connect");
            break;
        }

        FURI_LOG_I(TAG, "Connected to \"%s\"", ssid);

        if(wifi_get_info(instance, &info) != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to get info");
            break;
        }

        const WifiIpConfig* ip_config = &info.ip_config;
        if(ip_config->type == WifiIpTypeV4) {
            const WifiIpv4* addr = &ip_config->ip4.address;
            FURI_LOG_I(
                TAG,
                "IP: %hhu.%hhu.%hhu.%hhu",
                addr->bytes[0],
                addr->bytes[1],
                addr->bytes[2],
                addr->bytes[3]);
        } else {
            FURI_LOG_W(TAG, "IPv6 not implemented");
        }

        wifi_publish_state(instance, info.state);

    } while(false);

    instance->settings_applied = true;

    return 0;
}

static void
    wifi_startup_thread_state_callback(FuriThread* thread, FuriThreadState state, void* context) {
    furi_assert(thread);
    UNUSED(context);

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
    }
}

static void wifi_apply_settings(Wifi* instance) {
    FuriThread* startup_thread = furi_thread_alloc_ex(
        "WifiStartup", STARTUP_THREAD_STACK_SIZE, wifi_startup_thread_callback, instance);

    furi_thread_set_state_callback(startup_thread, wifi_startup_thread_state_callback);
    furi_thread_start(startup_thread);
}

static void wifi_load_settings(Wifi* instance) {
    WifiSettings* settings = &instance->settings;

    if(!wifi_settings_load(settings)) {
        FURI_LOG_W(TAG, "Failed to load settings, using defaults");

        wifi_settings_init_defaults(settings);
        wifi_settings_save(settings);
    }
}

static Wifi* wifi_alloc(void) {
    Wifi* instance = malloc(sizeof(Wifi));

    instance->event_loop = furi_event_loop_alloc();
    instance->access_semaphore = furi_semaphore_alloc(1, 1);
    instance->pubsub = furi_pubsub_alloc();
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    furi_record_open(RECORD_USB_NETWORK);
    tcpip_callback(wifi_add_netif, instance);
    // TODO: semaphore or something
    furi_delay_ms(1000);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, wifi_custom_event_callback, instance);

    intercom_set_rx_callback(
        instance->intercom, IntercomChannelWifi, wifi_intercom_rx_callback, instance);

    intercom_set_rx_callback(
        instance->intercom, IntercomChannelSockets, wifi_net_intercom_rx_callback, instance);

    wifi_load_settings(instance);

    furi_record_create(RECORD_WIFI, instance);

    wifi_apply_settings(instance);

    return instance;
}

FuriPubSub* wifi_get_pubsub(const Wifi* instance) {
    furi_check(instance);
    return instance->pubsub;
}

int32_t wifi_srv(void* arg) {
    UNUSED(arg);

    Wifi* instance = wifi_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
