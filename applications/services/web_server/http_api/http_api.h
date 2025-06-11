#pragma once
#include "../web_server_i.h"

// Root API handlers
void* http_api_root_alloc(void);
void http_api_root_free(void* ctx);
bool http_api_root_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx);
bool http_api_root_hdr_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx);

// Assets
void* http_api_assets_alloc(void);
void http_api_assets_free(void* ctx);
bool http_api_assets_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx);
bool http_api_assets_hdr_callback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);

// Display
void* http_api_display_alloc(void);
void http_api_display_free(void* ctx);
bool http_api_display_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx);

// Audio
void* http_api_audio_alloc(void);
void http_api_audio_free(void* ctx);
bool http_api_audio_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx);

// WebSocket test
void* http_websocket_alloc(void);
void http_websocket_free(void* ctx);
bool http_websocket_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx);

// Update API
void* http_api_update_alloc(void);
void http_api_update_free(void* ctx);
bool http_api_update_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx);
bool http_api_update_hdr_callback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx);
