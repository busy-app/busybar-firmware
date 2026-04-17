/***************************************************************************//**
 * @file sl_wisun_silabs_certificates.h
 * @brief Silicon Labs Wi-SUN test certificates
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SL_WISUN_SILABS_CERTIFICATES_H
#define SL_WISUN_SILABS_CERTIFICATES_H

#include <stdint.h>

const uint8_t SILABS_ROOT_CERT[] = {
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIBojCCAUmgAwIBAgIUSOJfgI08JDWdAjuqvH3REMyjjFswCgYIKoZIzj0EAwIwHjEcMBoGA1UEAwwTV2ktU1VOIERlbW8gUm9vdCBDQTAgFw0yMTAyMjIwOTU5NDFaGA85OTk5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTV2ktU1VOIERlbW8gUm9vdCBDQTBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABG1Mn4dd9+IVJZSEcjpFKehvvRyQt9QcIBCN2ysf+BJUlFfU8TvC3w2waFrLuC+JHM+1TBEm1GLNDF7piCgqltWjYzBhMBIGA1UdEwEB/wQIMAYBAf8CAQIwCwYDVR0PBAQDAgEGMB0GA1UdDgQWBBSTZNQO92ii7l3wrPpyvMUfTU86JDAfBgNVHSMEGDAWgBSTZNQO92ii7l3wrPpyvMUfTU86JDAKBggqhkjOPQQDAgNHADBEAiAdlM3ENdd7GHHbTsTiZMc7T5DDFQ2abeUI1be+ytGaAAIgZREIYV4yhjoluqT4+snj/zQkqEqcYh/DMbx2gLKDgZ4=\r\n"
    "-----END CERTIFICATE-----"
};

const uint8_t SILABS_DEVICE_CERT[] = {
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIByDCCAW6gAwIBAgIUPRtrFcA6dw03sTpD1dArHpFi65gwCgYIKoZIzj0EAwIwHjEcMBoGA1UEAwwTV2ktU1VOIERlbW8gUm9vdCBDQTAgFw0yMTAzMDEwNzQyNDBaGA85OTk5MTIzMTIzNTk1OVowHTEbMBkGA1UEAwwSV2ktU1VOIERlbW8gRGV2aWNlMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEqOqrunh3+gAr83a6wtpB+QhRvM4o9dTdVeNCo0amXhzbUsFylw7OgbgqtoCKQguYTsbicpeLMWv4TV7JxX6Ij6OBiDCBhTAOBgNVHQ8BAf8EBAMCA4gwIQYDVR0lAQH/BBcwFQYJKwYBBAGC5CUBBggrBgEFBQcDAjAvBgNVHREBAf8EJTAjoCEGCCsGAQUFBwgEoBUwEwYJKwYBBAGCt0ECBAYxMjM0NTYwHwYDVR0jBBgwFoAUk2TUDvdoou5d8Kz6crzFH01POiQwCgYIKoZIzj0EAwIDSAAwRQIhANBxFWMzNMKyA+nMK0sbCUpqK1gVMyeoKqh0zvS3COyLAiAx8nCNB7RkW8RmZ0UMWY26g7P6TbqJiAI3zoKkSxpJPg==\r\n"
    "-----END CERTIFICATE-----"
};

const uint8_t SILABS_DEVICE_KEY[] = {
    "-----BEGIN PRIVATE KEY-----\r\n"
    "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQguF1oBuIMzOtpsOMHdf97vr2GppQfXOKDJ4RogFMk7QChRANCAASo6qu6eHf6ACvzdrrC2kH5CFG8zij11N1V40KjRqZeHNtSwXKXDs6BuCq2gIpCC5hOxuJyl4sxa/hNXsnFfoiP\r\n"
    "-----END PRIVATE KEY-----"
};

const uint8_t SILABS_BR_CERT[] = {
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIBzzCCAXWgAwIBAgIUV9rWcXwDqRGrLVU/JRipf/q5ARUwCgYIKoZIzj0EAwIwHjEcMBoGA1UEAwwTV2ktU1VOIERlbW8gUm9vdCBDQTAgFw0yMTAzMDEwNzQyMThaGA85OTk5MTIzMTIzNTk1OVowJDEiMCAGA1UEAwwZV2ktU1VOIERlbW8gQm9yZGVyIFJvdXRlcjBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABGqBAsB47GbEpZbO3iDNaXeN9S2AEjMFUGyzoYvoVg/XUNhH5Z46eoVFIt8/+tfeI9Uha2xj0KKi9Cr3Q0EmP6qjgYgwgYUwDgYDVR0PAQH/BAQDAgOIMCEGA1UdJQEB/wQXMBUGCSsGAQQBguQlAQYIKwYBBQUHAwEwLwYDVR0RAQH/BCUwI6AhBggrBgEFBQcIBKAVMBMGCSsGAQQBgrdBAQQGMTIzNDU2MB8GA1UdIwQYMBaAFJNk1A73aKLuXfCs+nK8xR9NTzokMAoGCCqGSM49BAMCA0gAMEUCIACT5SnUC+IRXrGNhX2XOursPvoGbKbpLyjtai3PwayXAiEAqxtUaEijpWJUby/RsX/yXLgD9/aATj9YFTR+ZdZ1VLo=\r\n"
    "-----END CERTIFICATE-----"
};

const uint8_t SILABS_BR_KEY[] = {
    "-----BEGIN PRIVATE KEY-----\r\n"
    "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgN0Zj70BWw1o/ZQWKfzslLkjUW4eUEMWZK5YII0aA0PahRANCAARqgQLAeOxmxKWWzt4gzWl3jfUtgBIzBVBss6GL6FYP11DYR+WeOnqFRSLfP/rX3iPVIWtsY9CiovQq90NBJj+q\r\n"
    "-----END PRIVATE KEY-----"
};

#endif /* SL_WISUN_SILABS_CERTIFICATES_H */
