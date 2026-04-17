/*******************************************************************************
 * @file  psa_symmetric_key_storage_app.h
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef PSA_SYMMETRIC_KEY_STORAGE_APP_H
#define PSA_SYMMETRIC_KEY_STORAGE_APP_H

#include "psa/crypto.h"

void psa_its_app_process_action();

void test_import_and_export_volatile_plain_key();
void test_generate_and_export_volatile_plain_key();
void test_psa_symmetric_volatile_plain_key_storage();

void test_import_and_export_persistent_plain_key();
void test_generate_and_export_persistent_plain_key();
void test_psa_symmetric_persistent_plain_key_storage();
#endif /* PSA_SYMMETRIC_KEY_STORAGE_APP_H */