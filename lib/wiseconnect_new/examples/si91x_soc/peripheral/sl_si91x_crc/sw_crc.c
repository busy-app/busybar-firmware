#include "sw_crc.h"

#define CRC_ALGO_BIT_BY_BIT 1
#define DUMMY_CRC           0xFFFFFFFF
unsigned int esw_crc = 0;

unsigned char input[BUFF_LEN_8BIT_TYPE] = {
  0x6e, 0X95, 0Xc1, 0X6f, 0x26, 0X03, 0X25, 0X26, 0x45, 0X3e, 0Xa6, 0X33, 0x33, 0X30, 0X1c, 0X04, 0xa5,
  0X31, 0Xe3, 0Xde, 0xb0, 0X40, 0Xd1, 0X30, 0xab, 0Xac, 0X09, 0Xc0, 0x53, 0x9d, 0x67, 0X54, 0x30, 0X36,
  0Xbe, 0Xdf, 0xa2, 0Xc9, 0X98, 0Xdf, 0xa7, 0Xd3, 0xbc, 0xed, 0xb9, 0x3e, 0x26, 0x2e, 0xd0, 0x96, 0xb2,
  0xa7, 0xf2, 0xb3, 0x2b, 0x1c, 0x51, 0x92, 0xff, 0x8d, 0x6d, 0xc0, 0x46, 0xe6, 0x2d, 0x32, 0x56, 0xfe,
  0x73, 0xdd, 0xaa, 0x7c, 0x17, 0xe7, 0x3a, 0xb6, 0xc3, 0xca, 0x12, 0x2a, 0xb0, 0x3f, 0x5b, 0xa4, 0xb3,
  0x4e, 0x87, 0xa3, 0x98, 0x54, 0xdb, 0x27, 0x3c, 0x5c, 0x17, 0x6a, 0x67, 0xa3, 0x67, 0xac
};
crc_t output = 0;

uint32_t sw_crc_compute(uint32_t data_len)
{
  static unsigned int cal_crc;
  crc_calc(input, data_len, &cal_crc);
  return cal_crc;
}
int crc_calc(unsigned char *data_in, unsigned int num, crc_t *crc_out)
{
  crc_cfg_t cfg;
  crc_t crc;

  cfg.width = DATA_WIDTH_32;
  cfg.poly  = CRC32_POLYNOMIAL;
  //cfg.poly = 0x7;
  cfg.reflect_in  = INIT_VAL_1;
  cfg.xor_in      = INIT_VAL_0;
  cfg.reflect_out = INIT_VAL_1;
  cfg.xor_out     = INIT_VAL_0;

  cfg.msb_mask  = INIT_VAL_1 << (cfg.width - INIT_VAL_1);
  cfg.crc_mask  = (cfg.msb_mask - INIT_VAL_1) | cfg.msb_mask;
  cfg.crc_shift = cfg.width < INIT_VAL_8 ? INIT_VAL_8 - cfg.width : INIT_VAL_0;

  cfg.poly &= cfg.crc_mask;
  cfg.xor_in &= cfg.crc_mask;
  cfg.xor_out &= cfg.crc_mask;

  crc      = crc_init(&cfg);
  crc      = crc_update(&cfg, crc, (unsigned char *)data_in, num);
  crc      = crc_finalize(&cfg, crc);
  *crc_out = crc;
  return 0;
}

/**
 * @brief Reflects the bits of a data word.
 *
 * @details This function reflects the bits of a data word.
 *
 * @param data     Data word to be reflected.
 * @param data_len Length of the data word in bits.
 *
 * @return Reflected data word.
 *
 *****************************************************************************/
crc_t crc_reflect(crc_t data, size_t data_len)
{
  unsigned int loop_cnt;
  crc_t ret;

  ret = data & INIT_VAL_1;
  for (loop_cnt = INIT_VAL_1; loop_cnt < data_len; loop_cnt++) {
    data >>= INIT_VAL_1;
    ret = (ret << INIT_VAL_1) | (data & INIT_VAL_1);
  }
  return ret;
}

/**
 * Calculate the initial crc value.
 *
 * \param cfg  A pointer to a initialised crc_cfg_t structure.
 * \return     The initial crc value.
 *****************************************************************************/
crc_t crc_init(const crc_cfg_t *cfg)
{
  unsigned int loop_cnt;
  bool bit;
  crc_t crc = cfg->xor_in;
  for (loop_cnt = 0; loop_cnt < cfg->width; loop_cnt++) {
    bit = crc & INIT_VAL_1;
    if (bit) {
      crc = ((crc ^ cfg->poly) >> INIT_VAL_1) | cfg->msb_mask;
    } else {
      crc >>= 1;
    }
  }
  return crc & cfg->crc_mask;
}

/**
 * Update the crc value with new data.
 *
 * \param crc      The current crc value.
 * \param cfg      A pointer to a initialised crc_cfg_t structure.
 * \param data     Pointer to a buffer of \a data_len bytes.
 * \param data_len Number of bytes in the \a data buffer.
 * \return         The updated crc value.
 *****************************************************************************/
crc_t crc_update(const crc_cfg_t *cfg, crc_t crc, const unsigned char *data, size_t data_len)
{
  unsigned int loop_cnt;
  bool bit;
  unsigned char ch_data;

  //printf("data_len : %x\n",data_len);
  while (data_len--) {
    if (cfg->reflect_in) {
      ch_data = crc_reflect(*data++, DATA_LEN_8);
    } else {
      ch_data = *data++;
    }
    //printf("data-in : %x\n",c);
    for (loop_cnt = INIT_VAL_0; loop_cnt < INIT_VAL_8; loop_cnt++) {
      bit = crc & cfg->msb_mask;
      crc = (crc << INIT_VAL_1) | ((ch_data >> (INIT_VAL_7 - loop_cnt)) & INIT_VAL_1);
      if (bit) {
        crc ^= cfg->poly;
      }
    }
    crc &= cfg->crc_mask;
  }
  return crc & cfg->crc_mask;
}

/**
 * Calculate the final crc value.
 *
 * \param cfg  A pointer to a initialised crc_cfg_t structure.
 * \param crc  The current crc value.
 * \return     The final crc value.
 *****************************************************************************/
crc_t crc_finalize(const crc_cfg_t *cfg, crc_t crc)
{
  unsigned int loop_cnt;
  bool bit;

  for (loop_cnt = INIT_VAL_0; loop_cnt < cfg->width; loop_cnt++) {
    bit = crc & cfg->msb_mask;
    crc = (crc << INIT_VAL_1) | INIT_VAL_0;
    if (bit) {
      crc ^= cfg->poly;
    }
  }
  if (cfg->reflect_out) {
    crc = crc_reflect(crc, cfg->width);
  }
  return (crc ^ cfg->xor_out) & cfg->crc_mask;
}
