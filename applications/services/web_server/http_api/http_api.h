#pragma once
#include "../web_server_i.h"

#define API_VERSION {4, 1, 0}

// Root API handlers
void* http_api_root_alloc(void);
void http_api_root_free(void* ctx);
bool http_api_root_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);
bool http_api_root_hdr_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);
bool http_api_options_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);

// Assets
void* http_api_assets_alloc(void);
void http_api_assets_free(void* ctx);
bool http_api_assets_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);
bool http_api_assets_hdr_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);

// Storage
void* http_api_storage_alloc(void);
void http_api_storage_free(void* ctx);
bool http_api_storage_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);
bool http_api_storage_hdr_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);

// Display
void* http_api_display_alloc(void);
void http_api_display_free(void* ctx);
bool http_api_display_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);

// Audio
void* http_api_audio_alloc(void);
void http_api_audio_free(void* ctx);
bool http_api_audio_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);

// Input
void* http_api_input_alloc(void);
void http_api_input_free(void* ctx);
bool http_api_input_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);

// Status
bool http_api_status_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);

// WebSocket test
void* http_websocket_alloc(void);
void http_websocket_free(void* ctx);
bool http_websocket_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);

// Wifi
void* http_api_wifi_alloc(void);
void http_api_wifi_free(void* ctx);
bool http_api_wifi_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);

// Update API
void* http_api_update_alloc(void);
void http_api_update_free(void* ctx);
bool http_api_update_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);
bool http_api_update_hdr_callback_root(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);

// Streaming
void* http_api_streaming_ws_alloc(void);
void http_api_streaming_ws_free(void* ctx);
bool http_api_streaming_ws_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);

bool http_api_streaming_single_frame_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);

// Ble
void* http_api_ble_alloc(void);
void http_api_ble_free(void* ctx);
bool http_api_ble_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);

// Time
void* http_api_time_alloc(void);
void http_api_time_free(void* ctx);
bool http_api_time_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);

// Name
bool http_api_name_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);

// MQTT Account
void* http_api_account_alloc(void);
void http_api_account_free(void* ctx);
bool http_api_account_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);

// Busy
void* http_api_busy_alloc(void);
void http_api_busy_free(void* ctx);
bool http_api_busy_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);
