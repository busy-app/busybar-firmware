// static void fetch_client_update_on_data_cb(struct mg_connection* conn, struct mg_iobuf* io) {
//     ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
//     FetchClient* instance = (FetchClient*)conn_ctx->context;
//     furi_assert(instance);
//     size_t data_len = io->len;

//     if(instance->is_downloading) {
//         if(!instance->temp_file_handle || !storage_file_is_open(instance->temp_file_handle)) {
//             FURI_LOG_E(TAG, "on_data: Context or file handle invalid/closed. Draining.");
//             mg_iobuf_del(io, 0, io->len); // Consume data to prevent further calls
//             conn->is_draining = 1; // Mark connection to be closed
//             return;
//         }

//         FURI_LOG_D(
//             TAG,
//             "on_data: Received %zu bytes. Total received: %zu / %zu",
//             data_len,
//             instance->received_file_size,
//             instance->total_file_size);

//         if(data_len > 0) {
//             if(instance->received_file_size + data_len > instance->total_file_size) {
//                 FURI_LOG_E(
//                     TAG,
//                     "on_data: Received more data than expected. Expected %zu, got %zu more.",
//                     instance->total_file_size,
//                     (instance->received_file_size + data_len) - instance->total_file_size);
//                 MG_REPLY_PAYLOAD_TOO_LARGE(conn);
//                 storage_file_close(instance->temp_file_handle); // Close file on error
//                 conn->is_draining = 1;
//                 mg_iobuf_del(io, 0, io->len);
//                 return;
//             }

//             size_t written = storage_file_write(instance->temp_file_handle, io->buf, data_len);
//             if(written != data_len) {
//                 FURI_LOG_E(
//                     TAG,
//                     "on_data: Failed to write data to temp file. Wrote %zu of %zu.",
//                     written,
//                     data_len);
//                 MG_REPLY_INTERNAL_ERROR(conn, "Failed to save update package (write error).");
//                 storage_file_close(instance->temp_file_handle); // Close file on error
//                 conn->is_draining = 1;
//                 mg_iobuf_del(io, 0, io->len);
//                 return;
//             }
//             instance->received_file_size += written;
//         }
//         if(instance->callback) {
//             FetchClientData data_progress;
//             data_progress.progress.total_file_size = instance->total_file_size;
//             data_progress.progress.received_file_size = instance->received_file_size;
//             instance->callback(FetchClientEventProgress, &data_progress, instance->context);
//         }

//         mg_iobuf_del(io, 0, io->len); // Consume all data from buffer

//         if(instance->received_file_size >= instance->total_file_size) {
//             FURI_LOG_I(
//                 TAG, "on_data: All data received (%zu bytes)", instance->received_file_size);
//             if(storage_file_is_open(instance->temp_file_handle)) {
//                 storage_file_close(instance->temp_file_handle);
//             }

//             instance->file_fully_received = true;
//             if(instance->callback)
//                 instance->callback(FetchClientEventDone, NULL, instance->context);

//             // if(!handle_completed_upload_and_reboot(instance, conn)) {
//             //     // Error response already sent by handle_completed_upload_and_reboot
//             //     FURI_LOG_E(TAG, "on_data: package handling failed.");
//             //     conn->is_draining = 1;
//             // }
//         }
//     } else {
//         // Not downloading, just consume
//         FETCH_CLIENT_INFO(TAG, "on_data: Received %zu bytes", data_len);
// #ifdef FETCH_CLIENT_DEBUG
//         if(data_len) {
//             for(size_t i = 0; i < data_len; i++) {
//                 if(!io->buf[i]) {
//                     FURI_LOG_RAW_I(" [00] ");
//                 } else {
//                     FURI_LOG_RAW_I("%c", io->buf[i]);
//                 }
//             }
//             FURI_LOG_RAW_I("\r\n");
//         }

// #endif
//         if(instance->callback) {
//             FetchClientData data_raw;
//             data_raw.raw.data = (uint8_t*)io->buf;
//             data_raw.raw.size = data_len;
//             instance->callback(FetchClientEventRawData, &data_raw, instance->context);
//         }

//         mg_iobuf_del(io, 0, io->len); // Consume all data from buffer
//     }
// }

// void fetch_client_switching_to_raw_protocol(
//     struct mg_connection* conn,
//     struct mg_http_message* msg) {
//     ConnectionContext* conn_ctx = (ConnectionContext*)conn->data;
//     FetchClient* instance = (FetchClient*)conn->fn_data;
//     conn_ctx->context = instance;

//     FETCH_CLIENT_INFO(TAG, "body size: %d", (int)msg->body.len);

//     if((int)msg->body.len != -1) instance->total_file_size = msg->body.len;

//     if(instance->is_downloading) {
//         if(instance->total_file_size == 0) {
//             FURI_LOG_W(
//                 TAG, "on_headers: Content-Length is 0 or missing/invalid. No file to upload?");
//             MG_REPLY_BAD_REQUEST(conn);
//             conn->is_draining = 1;
//             return;
//         }

//         if(instance->total_file_size > MAX_UPLOAD_FILE_SIZE) {
//             FURI_LOG_E(
//                 TAG,
//                 "on_headers: File size %zu exceeds max %u.",
//                 instance->total_file_size,
//                 MAX_UPLOAD_FILE_SIZE);
//             MG_REPLY_PAYLOAD_TOO_LARGE(conn);
//             conn->is_draining = 1;
//             return;
//         }

//         FURI_LOG_I(
//             TAG, "on_headers: Expecting file of size: %zu bytes", instance->total_file_size);

//         // // Ensure staging root /ext/update exists
//         // if(!storage_dir_exists(instance->storage, STORAGE_EXT_PATH_PREFIX)) {
//         //     if(storage_common_mkdir(instance->storage, STORAGE_EXT_PATH_PREFIX) != FSE_OK) {
//         //         FURI_LOG_E(
//         //             TAG,
//         //             "on_headers: Failed to create staging root directory: %s",
//         //             STORAGE_EXT_PATH_PREFIX);
//         //         MG_REPLY_INTERNAL_ERROR(conn, "Failed to create update staging directory.");
//         //         conn->is_draining = 1;
//         //         return true;
//         //     }
//         // }

//         if(storage_file_exists(instance->storage, furi_string_get_cstr(instance->temp_file_path))) {
//             storage_simply_remove(
//                 instance->storage, furi_string_get_cstr(instance->temp_file_path));
//         }

//         if(!storage_file_open(
//                instance->temp_file_handle,
//                furi_string_get_cstr(instance->temp_file_path),
//                FSAM_WRITE,
//                FSOM_CREATE_ALWAYS)) {
//             FURI_LOG_E(
//                 TAG,
//                 "on_headers: Failed to open temp file for writing: %s",
//                 furi_string_get_cstr(instance->temp_file_path));
//             MG_REPLY_INTERNAL_ERROR(conn, "Failed to save update package (file open error).");
//             conn->is_draining = 1;
//             return;
//         }
//         FURI_LOG_I(
//             TAG,
//             "on_headers: Opened temp a file for writing: %s",
//             furi_string_get_cstr(instance->temp_file_path));
//     }
//     // Set up raw data handlers
//     conn_ctx->raw.on_data = fetch_client_update_on_data_cb;
//     conn_ctx->on_close = fetch_client_on_close;

//     if(instance->is_downloading) {
//         mg_iobuf_del(&conn->recv, 0, msg->head.len); // Delete HTTP headers
//     }
//     conn->pfn = NULL; // Silence HTTP protocol handler, we'll use MG_EV_READ
// }

// FetchClient* fetch_client_alloc(FuriString* url, FuriString* file_path) {
//     FetchClient* instance = malloc(sizeof(FetchClient));
//     instance->url = furi_string_alloc_printf("%s", furi_string_get_cstr(url));
//     instance->response = furi_string_alloc();

//     if(furi_string_size(file_path)) {
//         instance->storage = furi_record_open(RECORD_STORAGE);
//         instance->temp_file_path = furi_string_alloc_printf(
//             "%s/%s", STORAGE_EXT_PATH_PREFIX, furi_string_get_cstr(file_path));
//         instance->temp_file_handle = storage_file_alloc(instance->storage);
//         instance->is_downloading = true;
//     }

//     instance->done = false;
//     instance->total_file_size = 0;
//     instance->received_file_size = 0;
//     instance->file_fully_received = false;
//     instance->is_working = true;

//     return instance;
// }

// void fetch_client_free(FetchClient* instance) {
//     furi_check(instance);

//     furi_string_free(instance->url);
//     furi_string_free(instance->response);

//     if(instance->is_downloading) {
//         if(instance->temp_file_handle) {
//             if(storage_file_is_open(instance->temp_file_handle)) {
//                 storage_file_close(instance->temp_file_handle);
//             }
//             storage_file_free(instance->temp_file_handle);
//             instance->temp_file_handle = NULL; // Nullify after free
//         }

//         furi_string_free(instance->temp_file_path);

//         if(instance->storage) {
//             furi_record_close(RECORD_STORAGE);
//             instance->storage = NULL;
//         }
//     }

//     if(instance->data_body) {
//         free(instance->data_body);
//         instance->data_body = NULL;
//     }
//     free(instance);
// }
