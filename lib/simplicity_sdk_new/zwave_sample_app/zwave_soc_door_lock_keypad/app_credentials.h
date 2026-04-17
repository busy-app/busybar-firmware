/**
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: 2024 Silicon Laboratories Inc.
 */
#include "SwTimer.h"
#include "zaf_event_distributor_soc.h"

/**
 * How long it takes the user to enter their PIN code, used for simulating local
 * Credential entry during a Credential Learn operation
 */
#define PIN_CODE_ENTRY_SECONDS 7

/**
 * Initializes the User Credential Command Class related application logic upon
 * startup.
 */
void credentials_init(void);

/**
 * Initializes an empty User Credential database with a default User and a
 * default Credential.
 */
void initialize_user_credential_database(void);

/**
 * Simulates requesting a User to input a new Credential and generates a
 * "Credential Learn done" event after a set delay.
 */
void request_credential_from_user(void);

void user_credential_app_event_handler(const uint8_t event, const void *data);

void set_credential_learn_pin(const uint8_t * data, uint8_t length);
