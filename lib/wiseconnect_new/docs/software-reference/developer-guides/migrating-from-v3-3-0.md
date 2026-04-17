# Migrating from WiSeConnect™ SDK v3.3.0 to v3.3.1

## Table of Contents

- [Overview](#overview)
- [Migration Steps](#migration-steps)
  - [Update API Calls](#update-api-calls)
  - [Update Files](#update-files)
  - [Update Macros](#update-macros)

## Overview

This is a guide for updating an existing application using the WiSeConnect™ SDK v3.3.0 to a v3.3.1 application.

There are few naming and file changes in v3.3.1 as compared to v3.3.0, mostly in order to standardize the names and improve the overall usage experience of the application programming interface (API). Migration requires the names everywhere to be updated in the existing code of an application.

## Migration Steps

In order to convert a WiSeConnect SDK v3.3.0 application to a v3.3.1 application,

1. Open your existing application project in Simplicity Studio.

2. In each source file of the project, replace the v3.3.0 names or interfaces with v3.3.1 names or interfaces. 

Refer to the tables in each of the sections that follow which map the v3.3.0 API elements to v3.3.1. In some instances, the differences between v3.3.0 and v3.3.1 are highlighted in **bold** text.
- [Update API Calls](#update-api-calls)
- [Update Files](#update-files)
- [Update Macros](#update-macros)
- [Update Types](#update-types)

### Update API Calls

| **Module**   | **v3.3.0** | **v3.3.1** |
|--------------|------------|------------|
| Wi-Fi | void data_callback(uint32_t sock_no, uint8_t *buffer, uint32_t length)|void **data\_callback** (uint32_t sock_no, uint8_t *buffer, uint32_t length, const sl_si91x_socket_metadata_t *firmware_socket_response)|
| Wi-Fi | sl_status_t sl_http_client_deinit(sl_http_client_t *client) | sl_status_t sl_http_client_deinit(**const** sl_http_client_t *client) |
| Wi-Fi | sl_status_t sl_http_client_write_chunked_data(const sl_http_client_t *client, uint8_t *data, uint32_t data_length, bool flush_now) | sl_status_t sl_http_client_write_chunked_data(const sl_http_client_t *client, **const** uint8_t *data, uint32_t data_length, bool flush_now) |

### Update Files

- The linker file has been split to separate PSRAM and NONPSRAM examples.
- Removed Hardware setup API from all power save application which uses sllib_m4_power_save.slcc component
- The clocks need to be configured in the application, if not already done, and INTF must be configured for peripheral configuration. Please refer to the respective examples for reference.

### Update Macros

| **Module** | **v3.3.0** | **v3.3.1** |
|-------------| -----------| ---------- |

### Update Types

| **Module** | **v3.3.0** | **v3.3.1** |
|------------|------------|------------|

