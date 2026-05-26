#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct QrDocs QrDocs;

QrDocs* qr_docs_alloc(Widget* parent);

void qr_docs_set_url(QrDocs* instance, const char* url);

void qr_docs_set_text(QrDocs* instance, const char* text);

void qr_docs_set_image(QrDocs* instance, const char* path);

void qr_docs_free(QrDocs* instance);

#ifdef __cplusplus
}
#endif
