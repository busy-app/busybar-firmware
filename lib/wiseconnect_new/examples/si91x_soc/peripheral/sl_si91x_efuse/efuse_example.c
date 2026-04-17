/***************************************************************************/ /**
* @file efuse_example.c
* @brief EFUSE examples functions
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
#include "efuse_example.h"
#include "rsi_debug.h"

#include "sl_si91x_efuse.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define WRITE_ADD_1 0x00001 // efuse address 1
#define WRITE_ADD_2 0x00002 // efuse address 2
#define CLOCK       100     // Clock
#define BIT_POS_0   0       // Bit position 0
#define BIT_POS_1   1       // Bit position 1
#define BIT_POS_3   3       // Bit position 3
#define BIT_POS_5   5       // Bit position 5
#define MATCH_BYTE  0x2A    // 1 byte to be read
#define MATCH_WORD  0x032A  // 1 word to be read

/* Since efuse is a one-time programmable, WRITE_ENABLE macro must not be enabled unless user wants to use efuse write */
#define WRITE_ENABLE 0 // Enables efuse write if set to '1'
/*******************************************************************************
 *************************** LOCAL VARIABLES   *********************************
 ******************************************************************************/
uint8_t fsm_read_byte            = 0;
uint8_t memory_mapped_read_byte  = 0;
uint16_t memory_mapped_read_word = 0;
uint16_t address_value_1         = 0;
uint16_t address_value_2         = 0;

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * EFUSE example initialization function
 ******************************************************************************/
void efuse_example_init(void)
{
  sl_efuse_version_t version;
  sl_status_t status;

  do {
    /* Version information of EFUSE */
    version = sl_si91x_efuse_get_version();
    DEBUGOUT("EFUSE version is fetched successfully \n");
    DEBUGOUT("API version is %d.%d.%d\n", version.release, version.major, version.minor);

    /*  Initialize the efuse */
    status = sl_si91x_efuse_init();
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_efuse_init: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Efuse initialization is successful \n");
    /* Set the eFUSE address WRITE_ADD_1 for read and write operations */
    status = sl_si91x_efuse_set_address(WRITE_ADD_1);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_efuse_set_address: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Setting the eFUSE address WRITE_ADD_1 for read and write "
             "operations is successful \n");
    /* Get the eFUSE address WRITE_ADD_1 for read and write operations */
    status = sl_si91x_efuse_get_address(&address_value_1);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_efuse_get_address: Error Code : %lu \n", status);
      break;
    }
    if (address_value_1 == WRITE_ADD_1) {
      DEBUGOUT("MATCH\n");
    } else {
      DEBUGOUT("UNMATCH\n");
    }
    DEBUGOUT("Getting the eFUSE address WRITE_ADD_1 for read and write "
             "operations is successful \n");

#if defined(WRITE_ENABLE) && (WRITE_ENABLE == 1)
    /*Writing 0X2A Data at address 0x00001*/
    /*Address :0x00001 -> Data : 0010 1010  */
    /* Write WRITE_ADD_1 , bit position 1*/
    status = sl_si91x_efuse_write_bit_v2(WRITE_ADD_1, BIT_POS_1);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_efuse_write_bit_v2: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Writing on bit position 1 of address WRITE_ADD_1 is "
             "successful \n");
    /* Write WRITE_ADD_1 , bit position 3*/
    status = sl_si91x_efuse_write_bit_v2(WRITE_ADD_1, BIT_POS_3);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_efuse_write_bit_v2: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Writing on bit position 3 of address WRITE_ADD_1 is "
             "successful \n");

    /* Write WRITE_ADD_1 , bit position 5*/
    status = sl_si91x_efuse_write_bit_v2(WRITE_ADD_1, BIT_POS_5);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_efuse_write_bit_v2: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Writing on bit position 5 of address WRITE_ADD_1 is "
             "successful \n");
#endif
    /* Set the eFUSE address WRITE_ADD_2 for read and write operations */
    status = sl_si91x_efuse_set_address(WRITE_ADD_2);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_efuse_set_address: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Setting the eFUSE address WRITE_ADD_2 for read and write "
             "operations is successful \n");

    /* Get the eFUSE address WRITE_ADD_2 for read and write operations */
    status = sl_si91x_efuse_get_address(&address_value_2);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_efuse_get_address: Error Code : %lu \n", status);
      break;
    }
    if (address_value_2 == WRITE_ADD_2) {
      DEBUGOUT("MATCH\n");
    } else {
      DEBUGOUT("UNMATCH\n");
    }
    DEBUGOUT("Getting the eFUSE address WRITE_ADD_2 for read and write "
             "operations is successful \n");

#if defined(WRITE_ENABLE) && (WRITE_ENABLE == 1)
    /*Writing 0X02 Data at address 0x00002*/
    /*Address :0x00002 -> Data : 0000 0011  */
    /* Write WRITE_ADD_2 , bit position 0*/
    status = sl_si91x_efuse_write_bit_v2(WRITE_ADD_2, BIT_POS_0);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_efuse_write_bit_v2: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Writing on bit position 0 of address WRITE_ADD_2 is "
             "successful \n");

    /* Write WRITE_ADD_2 , bit position 1*/
    status = sl_si91x_efuse_write_bit_v2(WRITE_ADD_2, BIT_POS_1);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_efuse_write_bit_v2: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Writing on bit position 1 of address WRITE_ADD_2 is "
             "successful \n");
#endif
    /* Read byte from address WRITE_ADD_1 in FSM read mode */
    status = sl_si91x_efuse_fsm_read_byte(WRITE_ADD_1, &fsm_read_byte, CLOCK);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_efuse_fsm_read_byte: Error Code : %lu \n", status);
      break;
    }
    if (fsm_read_byte == MATCH_BYTE) {
      DEBUGOUT("MATCH\n");
    } else {
      DEBUGOUT("UNMATCH\n");
    }
    DEBUGOUT("Reading byte from address WRITE_ADD_1 in FSM read mode is "
             "successful \n");

    /* Read byte from address WRITE_ADD_1 in memory mapped read byte mode */
    status = sl_si91x_efuse_memory_mapped_read_byte(WRITE_ADD_1, &memory_mapped_read_byte, CLOCK);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_efuse_memory_mapped_read_byte: Error Code : %lu \n", status);
      break;
    }
    if (memory_mapped_read_byte == MATCH_BYTE) {
      DEBUGOUT("MATCH\n");
    } else {
      DEBUGOUT("UNMATCH\n");
    }
    DEBUGOUT("Reading byte from address WRITE_ADD_1 in memory mapped read mode "
             "is successful \n");

    /* Read word from addresses WRITE_ADD_1 and WRITE_ADD_2 in memory mapped
     * read word mode */
    status = sl_si91x_efuse_memory_mapped_read_word(WRITE_ADD_1, &memory_mapped_read_word, CLOCK);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_efuse_memory_mapped_read_word: Error Code : %lu \n", status);
      break;
    }
    if (memory_mapped_read_word == MATCH_WORD) {
      DEBUGOUT("MATCH\n");
    } else {
      DEBUGOUT("UNMATCH\n");
    }
    DEBUGOUT("Reading word from addresses WRITE_ADD_1 and WRITE_ADD_2 in "
             "memory mapped read mode is successful \n");

    /*  Un-Initialize the efuse */
    status = sl_si91x_efuse_deinit();
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_efuse_deinit: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Efuse un-initialization is successful \n");
  } while (false);
}
/*******************************************************************************
 * Function will run continuously in while loop
 ******************************************************************************/
void efuse_example_process_action(void)
{
  // Function will run continuously in while loop
}
