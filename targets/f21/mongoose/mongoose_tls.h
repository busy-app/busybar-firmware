#pragma once

#include <mongoose.h>

#include <toolbox/tls_config.h>

bool mongoose_tls_init(struct mg_connection* conn, const char* url, const TlsConfig* tls_config);
