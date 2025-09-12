/**
 * @file qr_code.h
 * @brief Displays a QR code generated from textual data.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** QRCode opaque structure. */
typedef struct QRCode QRCode;

/**
 * @brief Create a new QRCode instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created QRCode instance
 */
QRCode* qr_code_alloc(Widget* parent);

/**
 * @brief Delete an QRCode instance.
 *
 * @param[in,out] instance pointer to the QRCode instance to be deleted
 */
void qr_code_free(QRCode* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the QRCode instance to be queried
 * @returns pointer to the base class instance
 */
Widget* qr_code_get_base(QRCode* instance);

/**
 * @brief Sets the size of the QR code
 * 
 * @param[in,out] instance pointer to the QRCode instance to be modified
 * @param[in] size Size of one side of the code (width and height)
 */
void qr_code_set_size(QRCode* instance, int32_t size);

/**
 * @brief Generate and show a QR code.
 *
 * @param[in,out] instance pointer to the QRCode instance to be modified
 * @param[in] file_path zero-terminated string containing the text to be shown
 */
void qr_code_set_data(QRCode* instance, const char* data);

#ifdef __cplusplus
}
#endif
