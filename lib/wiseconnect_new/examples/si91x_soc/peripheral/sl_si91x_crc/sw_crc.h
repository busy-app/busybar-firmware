#ifndef CRC_H
#define CRC_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/**
 * @brief Length of the buffer for 8-bit type data.
 *
 * This constant defines the length of the buffer for 8-bit type data.
 * It is set to 100 bytes, which corresponds to 25 elements of uint32_t type.
 */
#define BUFF_LEN_8BIT_TYPE 100 //25 Bytes for uint32 type

/**
 * @brief Polynomial used in CRC-32 calculation.
 *
 * This constant defines the polynomial used in CRC-32 calculation.
 * The value is set to 0x04C11DB7.
 */
#define CRC32_POLYNOMIAL 0x04C11DB7

/**
 * @brief Initial value for CRC-32 calculation.
 *
 * This constant defines the initial value for CRC-32 calculation.
 * The value is set to 1.
 */
#define INIT_VAL_1 1

/**
 * @brief Initial value for CRC-32 calculation.
 *
 * This constant defines the initial value for CRC-32 calculation.
 * The value is set to 0.
 */
#define INIT_VAL_0 0

/**
 * @brief Length of data in bits.
 *
 * This constant defines the length of data in bits.
 * The value is set to 8 bits.
 */
#define DATA_LEN_8 8

/**
 * @brief Initial value for CRC-32 calculation.
 *
 * This constant defines the initial value for CRC-32 calculation.
 * The value is set to 8.
 */
#define INIT_VAL_8 8

/**
 * @brief Initial value for CRC-32 calculation.
 *
 * This constant defines the initial value for CRC-32 calculation.
 * The value is set to 7.
 */
#define INIT_VAL_7 7

/**
 * @brief Width of data in bits.
 *
 * This constant defines the width of data in bits.
 * The value is set to 32 bits.
 */
#define DATA_WIDTH_32 32

/**
 * @brief Type definition for CRC value.
 *
 * This typedef defines the type for CRC value.
 * It is defined as an unsigned integer.
 */
typedef unsigned int crc_t;

typedef struct {
  unsigned int width;       /*!< The width of the polynomial */
  crc_t poly;               /*!< The CRC polynomial */
  unsigned int reflect_in;  /*!< Whether the input shall be reflected or not */
  crc_t xor_in;             /*!< The initial value of the algorithm */
  unsigned int reflect_out; /*!< Wether the output shall be reflected or not */
  crc_t xor_out;            /*!< The value which shall be XOR-ed to the final CRC value */

  /* internal parameters */
  crc_t msb_mask;         /*!< a bitmask with the Most Significant Bit set to 1
                                     initialise as 1UL << (width - 1) */
  crc_t crc_mask;         /*!< a bitmask with all width bits set to 1
                                     initialise as (cfg->msb_mask - 1) | cfg->msb_mask */
  unsigned int crc_shift; /*!< a shift count that is used when width < 8
                                     initialise as cfg->width < 8 ? 8 - cfg->width : 0 */
} crc_cfg_t;

/**
 * @brief Reflects the bits of a data word.
 *
 * This function reflects the bits of a data word.
 *
 * @param data     Data word to be reflected.
 * @param data_len Length of the data word in bits.
 *
 * @return Reflected data word.
 */
crc_t crc_reflect(crc_t data, size_t data_len);

/**
 * @brief Initializes the CRC calculation.
 *
 * This function initializes the CRC calculation with the given configuration.
 *
 * @param cfg Configuration for CRC calculation.
 *
 * @return Initialized CRC value.
 */
crc_t crc_init(const crc_cfg_t *cfg);

/**
 * @brief Updates the CRC calculation with new data.
 *
 * This function updates the CRC calculation with new data.
 *
 * @param cfg      Configuration for CRC calculation.
 * @param crc      Current CRC value.
 * @param data     Pointer to the data array.
 * @param data_len Length of the data array in bytes.
 *
 * @return Updated CRC value.
 */
crc_t crc_update(const crc_cfg_t *cfg, crc_t crc, const unsigned char *data, size_t data_len);

/**
 * @brief Finalizes the CRC calculation.
 *
 * This function finalizes the CRC calculation and applies the final XOR value.
 *
 * @param cfg Configuration for CRC calculation.
 * @param crc Current CRC value.
 *
 * @return Finalized CRC value.
 */
crc_t crc_finalize(const crc_cfg_t *cfg, crc_t crc);

/**
 * @brief Calculates the CRC for the given data.
 *
 * This function calculates the CRC for the given data.
 *
 * @param data_in  Pointer to the input data array.
 * @param num      Number of bytes in the input data array.
 * @param crc_out  Pointer to store the calculated CRC value.
 *
 * @return 0 on success, -1 on failure.
 */
int crc_calc(unsigned char *data_in, unsigned int num, crc_t *crc_out);

/**
 * @brief Calculates the CRC using software implementation.
 *
 * This function calculates the CRC using software implementation.
 *
 * @param data_len  Length of the input data array in bytes.
 *
 * @return Calculated CRC value.
 */
uint32_t sw_crc_compute(uint32_t data_len);

#endif /* CRC_H */
