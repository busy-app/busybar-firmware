/***************************************************************************//**
 * @file sli_wisun_certificate.h
 * @brief Wi-SUN Certificate API
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef SLI_WISUN_CERTIFICATE_H_
#define SLI_WISUN_CERTIFICATE_H_

#include <stdint.h>
#include <stddef.h>
#include "sl_status.h"

/// Forward declaration for a Wi-SUN certificate instance
typedef struct sli_wisun_certificate sli_wisun_certificate_t;

/// Forward declaration for a Wi-SUN certificate chain instance
typedef struct sli_wisun_certificate_chain sli_wisun_certificate_chain_t;

/**************************************************************************//**
 * Create a Wi-SUN certificate instance.
 *
 * @returns Pointer to the certificate instance. Ownership is transferred
 *          to the caller.
 *****************************************************************************/
sli_wisun_certificate_t *sli_wisun_certificate_init();

/**************************************************************************//**
 * Delete a Wi-SUN certificate instance.
 *
 * @param[in] cert Certificate instance to delete. Ownership is transferred
 *                 to the function.
 *****************************************************************************/
void sli_wisun_certificate_free(sli_wisun_certificate_t *cert);

/**************************************************************************//**
 * Create a Wi-SUN certificate chain instance.
 *
 * @returns Pointer to the certificate chain instance. Ownership is transferred
 *          to the caller.
 *****************************************************************************/
sli_wisun_certificate_chain_t *sli_wisun_certificate_chain_init();

/**************************************************************************//**
 * Delete a Wi-SUN certificate chain instance.
 *
 * @param[in] cert Certificate chain instance to delete. Ownership is
 *                 transferred to the function.
 *****************************************************************************/
void sli_wisun_certificate_chain_free(sli_wisun_certificate_chain_t *chain);

/**************************************************************************//**
 * Import a Wi-SUN device certificate into a certificate instance.
 *
 * @param[in] cert Certificate instance
 * @param[in] data Pointer to certificate data
 * @param[in] data_length Length of certificate data
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function parses the given data buffer for a Wi-SUN device certificate
 * and imports it into the certificate instance. If the instance already
 * contains a certificate, the previous certificate is replaced.
 *****************************************************************************/
sl_status_t sli_wisun_certificate_import_devid(sli_wisun_certificate_t *cert,
                                               const uint8_t *data,
                                               size_t data_length);

/**************************************************************************//**
 * Import a Wi-SUN authenticator certificate into a certificate instance.
 *
 * @param[in] cert Certificate instance
 * @param[in] data Pointer to certificate data
 * @param[in] data_length Length of certificate data
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function parses the given data buffer for a Wi-SUN authenticator
 * certificate and imports it into the certificate instance. If the instance
 * already contains a certificate, the previous certificate is replaced.
 *****************************************************************************/
sl_status_t sli_wisun_certificate_import_auth(sli_wisun_certificate_t *cert,
                                              const uint8_t *data,
                                              size_t data_length);

/**************************************************************************//**
 * Import a Wi-SUN trusted certificate into a certificate chain instance.
 *
 * @param[in] chain Certificate chain instance
 * @param[in] data Pointer to certificate data
 * @param[in] data_length Length of certificate data
 * @return SL_STATUS_OK if successful, an error code otherwise
 *
 * This function parses the given data buffer for a Wi-SUN trusted certificate
 * and imports it into the certificate chain instance.
 *****************************************************************************/
sl_status_t sli_wisun_certificate_import_trusted(sli_wisun_certificate_chain_t *chain,
                                                 const uint8_t *data,
                                                 size_t data_length);

#endif /* SLI_WISUN_CERTIFICATE_H_ */
