/******************************************************************************
* @file  rsi_rom_fim.h
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

//Includes

#ifndef __RSI_ROM_FIM_H__
#define __RSI_ROM_FIM_H__

/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 * \defgroup FIM_DRIVERS
 *  @{
 *
 */
#include "rsi_fim.h"
#include "rsi_packing.h"
#if defined(A11_ROM)
#include "rsi_rom_table_si91x.h"
#else
#include "rsi_rom_table_RS1xxxx.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 *@brief        This API is used to Adds a constant offset to a floating-point vector.
 *@param[in]	pSrc  :	pointer of the input vector
 *@param[in]	scale  :	is the offset to be added
 *@param[out]	pDst  :	pointer of the output vector
 *@param[in]	length :	number of samples in the vector
 *@return       none
 * */
STATIC INLINE void arm_offset_f32_opt(int32_t *pSrc, int32_t scale, int32_t *pDst, uint32_t length)
{
  rsi_arm_offset_f32_opt(pSrc, scale, pDst, length, 0x00, BANK1);
}

/**
 *
 *@fn           STATIC INLINE STATIC INLINE void   arm_offset_q7_opt( q7_t *pSrc, q7_t scale, q7_t *pDst, uint32_t length)
 *@brief        The function uses saturating arithmetic. Results outside of the allowable Q7 range [0x80 0x7F] are saturated
 *@param[in]	pSrc  :	pointer of the input vector
 *@param[in]	scale  :	is the offset to be added
 *@param[out]	pDst  :	pointer of the output vector
 *@param[in]	length :	number of samples in the vector
 *@return        none
 */
STATIC INLINE void arm_offset_q7_opt(q7_t *pSrc, q7_t scale, q7_t *pDst, uint32_t length)
{
  rsi_arm_offset_q7_opt(pSrc, scale, pDst, length, 0x00, BANK1);
}

/**
 *@fn           STATIC INLINE void  arm_offset_q15_opt( q15_t *pSrc, q15_t scale, q15_t *pDst, uint32_t length)
 *@brief        The function uses saturating arithmetic. Results outside of the allowable Q15 range [0x8000 0x7FFF] are saturated.
 *@param[in]	pSrc  :	pointer of the input vector
 *@param[in]	scale  :	is the offset to be added
 *@param[out]	*pDst  :	points to the output vector
 *@param[in]	length :	number of samples in the vector
 *@return        none
 *
 */
STATIC INLINE void arm_offset_q15_opt(q15_t *pSrc, q15_t scale, q15_t *pDst, uint32_t length)
{
  rsi_arm_offset_q15_opt(pSrc, scale, pDst, length, 0x00, BANK1);
}

/**
 *@fn           STATIC INLINE void  arm_offset_q31_opt( q31_t *pSrc, q31_t scale, q31_t *pDst, uint32_t length)
 *@brief        The function uses saturating arithmetic. Results outside of the allowable Q31 range [0x80000000 0x7FFFFFFF] are saturated.
 *@param[in]	pSrc  :	pointer of the input vector
 *@param[in]	scale  :	is the offset to be added
 *@param[out]	*pDst  :	points to the output vector
 *@param[in]	length :	number of samples in the vector
 *@return       none
 **/
STATIC INLINE void arm_offset_q31_opt(q31_t *pSrc, q31_t scale, q31_t *pDst, uint32_t length)
{
  rsi_arm_offset_q31_opt(pSrc, scale, pDst, length, 0x00, BANK1);
}

/**
 *@brief      This API is used to set the FIM Scalar Subtraction
 *@param[in]  *pSrc : points to input vector
 *@param[out] *pDst : points to output vector
 *@param[in]  scale : is constant value that need to be subtracted from each elements of vector array.
 *@param[in]  blockSize : is size of the input array
 *@return       none
 */
STATIC INLINE void fim_scalar_sub_q7(q7_t *pSrc, q7_t scale, q7_t *pDst, uint32_t blockSize)
{
  rsi_fim_scalar_sub_q7(pSrc, scale, pDst, blockSize, 0x00, BANK1);
}

/**
 *@brief      This API is used to set the FIM Scalar Subtraction
 *@param[in]  *pSrc  : points to input vector
 *@param[out] *pDst  : points to output vector
 *@param[in]  scale  :  is constant value that need to be subtracted from each elements of vector array.
 *@param[in]  blockSize : is size of the input array
 *@return       none
 */
STATIC INLINE void fim_scalar_sub_q31(q31_t *pSrc, q31_t scale, q31_t *pDst, uint32_t blockSize)
{

  rsi_fim_scalar_sub_q31(pSrc, scale, pDst, blockSize, 0x00, BANK1);
}

/**
 *@brief      This API is used to set the FIM Scalar Subtraction
 *@param[in]  *pSrc :points to input vector
 *@param[out] *pDst : points to output vector
 *@param[in]  scale : is constant value that need to be subtracted from each elements of vector array.
 *@param[in]  length :is size of the input array
 *@return       none
 */
STATIC INLINE void fim_scalar_sub_f32(int32_t *pSrc, int32_t scale, int32_t *pDst, uint32_t length)
{

  rsi_fim_scalar_sub_f32(pSrc, scale, pDst, length, 0x00, BANK1);
}

/**
 *@brief      This API is used to set the FIM Scalar Multiplication for real data
 *@param[in]  *pSrc : points to input vector
 *@param[out] *pDst : points to output vector
 *@param[in]  scale : is constant value that need to be multiplied for each elements of vector array.
 *@param[in]  length : is size of the input array
 *return      none
 *
 */
STATIC INLINE void arm_scale_f32_opt(int32_t *pSrc, int32_t scale, int32_t *pDst, uint32_t length)
{

  rsi_arm_scale_f32_opt(pSrc, scale, pDst, length, 0x00, BANK1);
}

/**
 *@brief        This API is used to set the FIM Scalar Multiplication
 *@param[in]	*pSrc :	   points to the input vector
 *@param[in]	scaleFract : fractional portion of the scale value
 *@param[in]	shift :	   number of bits to shift the result by
 *@param[out]	*pDst :	   points to the output vector
 *@param[in]	blockSize :  number of samples in the vector
 *return        none
 */
STATIC INLINE void arm_scale_q7_opt(q7_t *pSrc, q7_t scaleFract, int8_t shift, q7_t *pDst, uint32_t blockSize)
{

  rsi_arm_scale_q7_opt(pSrc, scaleFract, shift, pDst, blockSize, 0x00, BANK1);
}
/**
 *@brief      This API is used to set the FIM Scalar Multiplication
 *@param[in]	*pSrc :	points to the input vector
 *@param[in]	scaleFract :	fractional portion of the scale value
 *@param[in]	shift :	number of bits to shift the result by
 *@param[out]	*pDst :	points to the output vector
 *@param[in]	blockSize :	number of samples in the vector
 *return      none
 */
STATIC INLINE void arm_scale_q15_opt(q15_t *pSrc, q15_t scaleFract, int8_t shift, q15_t *pDst, uint32_t blockSize)
{

  rsi_arm_scale_q15_opt(pSrc, scaleFract, shift, pDst, blockSize, 0x00, BANK1);
}

/**
 *@brief       This API is used to set the FIM Scalar Multiplication
 *@param[in]   *pSrc :	points to the input vector
 *@param[in]   scaleFract :	fractional portion of the scale value
 *@param[in]   shift :	number of bits to shift the result by
 *@param[out]  *pDst :	points to the output vector
 *@param[in]   blockSize : is size of the input array
 *return       none
 */
STATIC INLINE void arm_scale_q31_opt(q31_t *pSrc, q31_t scaleFract, int8_t shift, q31_t *pDst, uint32_t blockSize)
{

  rsi_arm_scale_q31_opt(pSrc, scaleFract, shift, pDst, blockSize, 0x00, BANK1);
}

/**
 *@brief      This API is used to set the FIM Scalar Multiplication
 *@param[in]  pSrc : is input vector A
 *@param[out]  pDst : points to output vector
 *@param[in]  scale : is constant value that need to be multiplied for each elements of vector array.
 *@param[in]  blockSize : is size of the input array
 *@param[in]  typ_data : is to select which type of data is given as input
 *            \n i.e. real complex , complex real and complex complex
 *            \n real complex   real vector, complex  scalar
 *            \n complex real   real scalar, complex vector
 *@return      none
 */
STATIC INLINE void fim_scalar_mul_q15(q15_t *pSrc, q15_t *pDst, q15_t *scale, uint32_t blockSize, typ_data_t typ_data)
{

  rsi_fim_scalar_mul_q15(pSrc, pDst, scale, blockSize, typ_data, 0x00, BANK1);
}

/**
 *@brief      This API is used to set the FIM Vector Addition for real data
 *@param[in]  pSrcA : is input vector A
 *@param[out]  pDst : points to output vector
 *@param[in]  pSrcB : is input vector B
 *@param[in]  blockSize : is size of the input array
 *@return      none
 */
STATIC INLINE void arm_add_f32_opt(int32_t *pSrcA, int32_t *pDst, int32_t *pSrcB, uint32_t blockSize)
{

  rsi_arm_add_f32_opt(pSrcA, pDst, pSrcB, blockSize, 0x00, BANK1, BANK2);
}

/**
 *@brief      This API is used to set the FIM Vector Addition for real data
 *@param[in]  pSrcA : points to input vector A
 *@param[in]  pSrcB : points to input vector B
 *@param[out] pDst :  points to output vector
 *@param[in]  blockSize : is size of the input array
 *@return      none
 */
STATIC INLINE void arm_add_q7_opt(q7_t *pSrcA, q7_t *pSrcB, q7_t *pDst, uint32_t blockSize)
{

  rsi_arm_add_q7_opt(pSrcA, pSrcB, pDst, blockSize, 0x00, BANK1, BANK2);
}
/**
 *@brief     This API is used to set the FIM Vector Addition for real data
 *@param[in]  pSrcA : points to input vector A
 *@param[in]  pSrcB : points to input vector B
 *@param[out] pDst  : points to output vector
 *@param[in]  blockSize : is size of the input array
 *@return      none
 */

STATIC INLINE void arm_add_q15_opt(q15_t *pSrcA, q15_t *pSrcB, q15_t *pDst, uint32_t blockSize)
{

  rsi_arm_add_q15_opt(pSrcA, pSrcB, pDst, blockSize, 0x00, BANK1, BANK2);
}
/**
 *@brief     This API is used to set the FIM Vector Addition for real data
 *@param[in]  pSrcA : points to input vector A
 *@param[in]  pSrcB : points to input vector B
 *@param[out] pDst :  points to output vector
 *@param[in]  blockSize : is size of the input array
 *@return      none
 */

STATIC INLINE void arm_add_q31_opt(q31_t *pSrcA, q31_t *pSrcB, q31_t *pDst, uint32_t blockSize)
{

  rsi_arm_add_q31_opt(pSrcA, pSrcB, pDst, blockSize, 0x00, BANK1, BANK2);
}
/**
 *@brief      This API is used to set the FIM Vector Addition
 *@param[in]  pIn1 : points to input vector A
 *@param[in]  pIn2 : points to input vector B
 *@param[in]  blockSize : is size of the input array
 *@param[out] pDst :  points to output vector
 *@param[in]  typ_data : is to select which type of data is given as input
 *            \n i.e. real complex , complex real and complex complex
 *            \n real complex   1st vector is real vector i.e pIn1, 2nd vector is complex i.e pIn2
 *            \n complex real   1st vector is complex vector i.e pIn1, 2nd vector is real i.e pIn2
 *@return     none 
 */

STATIC INLINE void fim_vector_add_q15(q15_t *pIn1, q15_t *pIn2, q15_t *pDst, uint32_t blockSize, typ_data_t typ_data)
{

  rsi_fim_vector_add_q15(pIn1, pIn2, pDst, blockSize, typ_data, 0x00, BANK3, BANK2);
}

/**
 *@brief      This API is used to set the FIM Vector Subtraction for real data
 *@param[in]  pSrcA : points to input vector A
 *@param[in]  pSrcB : points to input vector B
 *@param[out] pDst :  points to output vector
 *@param[in]  blockSize : is size of the input array
 *@return     none
 *
 */
STATIC INLINE void arm_sub_f32_opt(int32_t *pSrcA, int32_t *pSrcB, int32_t *pDst, uint32_t blockSize)
{

  rsi_arm_sub_f32_opt(pSrcA, pSrcB, pDst, blockSize, 0x00, BANK1, BANK2);
}
/**
 *@brief      This API is used to set the FIM Vector Subtraction for real data
 *@param[in]  pSrcA : points to input vector A
 *@param[in]  pSrcB : points to input vector B
 *@param[out] pDst :  points to output vector
 *@param[in]  blockSize : is size of the input array
 *@return     none
 */
STATIC INLINE void arm_sub_q7_opt(q7_t *pSrcA, q7_t *pSrcB, q7_t *pDst, uint32_t blockSize)
{

  rsi_arm_sub_q7_opt(pSrcA, pSrcB, pDst, blockSize, 0x00, BANK1, BANK2);
}

/**
 *@brief      This API is used to set the FIM Vector Subtraction for real data
 *@param[in]  pSrcA : points to input vector A
 *@param[in]  pSrcB : points to input vector B
 *@param[out] pDst :  points to output vector
 *@param[in]  blockSize : is size of the input array
 *@return     none
 *
 */
STATIC INLINE void arm_sub_q15_opt(q15_t *pSrcA, q15_t *pSrcB, q15_t *pDst, uint32_t blockSize)
{

  rsi_arm_sub_q15_opt(pSrcA, pSrcB, pDst, blockSize, 0x00, BANK1, BANK2);
}

/**
 *@brief      This API is used to set the FIM Vector Subtraction for real data
 *@param[in]  pSrcA : points to input vector A
 *@param[in]  pSrcB : points to input vector B
 *@param[out] pDst :  points to output vector
 *@param[in]  blockSize : is size of the input array
 *@return      none
 */
STATIC INLINE void arm_sub_q31_opt(q31_t *pSrcA, q31_t *pSrcB, q31_t *pDst, uint32_t blockSize)
{

  rsi_arm_sub_q31_opt(pSrcA, pSrcB, pDst, blockSize, 0x00, BANK1, BANK2);
}

/**
 *@brief      This API is used to set the FIM to read the output
 *@param[in]  bank : is output bank address
 *@param[out] pDst : is required output array
 *@param[in]  data_type : specifies q7,q15 , q31 formats
 *@param[in]  type_data : is to specify real-complex , complex-real or complex-complex data
 *@param[in]  length : is size of the input array
 *@return     none
 */
STATIC INLINE void fim_read_data(uint32_t bank,
                                 uint32_t length,
                                 volatile void *pDst,
                                 uint8_t data_type,
                                 typ_data_t type_data)
{

  rsi_fim_read_data(bank, length, pDst, data_type, type_data); //Skip the API if ENHANCED_FIM
}
/**
 *@brief      This API is used to set the FIM Vector Subtraction
 *@param[in]  pIn1 : is input vector A
 *@param[in]  pIn2 : is input vector B
 *@param[out] pDst : is required output array
 *@param[in]  typ_data : is to specify real-complex , complex-real or complex-complex data
 *@param[in]  blockSize : is size of the input array
 *@return     none
 */
STATIC INLINE void fim_vector_sub_q15(q15_t *pIn1, q15_t *pIn2, q15_t *pDst, uint32_t blockSize, typ_data_t typ_data)
{

  rsi_fim_vector_sub_q15(pIn1, pIn2, pDst, blockSize, typ_data, 0x00, BANK1, BANK2);
}
/**
 *@brief     This API is used to set the FIM Vector Multiplication for real data
 *@param[in]  pIn1 : is input vector A
 *@param[in]  pIn2 : is input vector B
 *@param[out]  pDst : points to output vector
 *@param[in]  length : is size of the input array
 *@param[in]  SatTruncRound : is used to limit the output as required
 *@return     none
 */
STATIC INLINE void arm_mult_f32_opt(int32_t *pIn1,
                                    int32_t *pIn2,
                                    int32_t *pDst,
                                    uint32_t SatTruncRound,
                                    uint32_t length)
{

  rsi_arm_mult_f32_opt(pIn1, pIn2, pDst, SatTruncRound, length, 0x00, BANK1, BANK2);
}
/**
 *@brief        This API is used for Q7 vector multiplication
 *@param[in]    pSrcA : points to the first input vector
 *@param[in]    pSrcB : points to the second input vector
 *@param[out]   pDst : points to the output vector
 *@param[in]    blockSize : number of samples in each vector
 *@return       none
 */
STATIC INLINE void arm_mult_q7_opt(q7_t *pSrcA, q7_t *pSrcB, q7_t *pDst, uint32_t blockSize)
{
  rsi_arm_mult_q7_opt(pSrcA, pSrcB, pDst, blockSize, 0x00, BANK1, BANK2);
}
/**
 *@brief       This API is used for Q15 vector multiplication
 *@param[in]   pSrcA : points to the first input vector
 *@param[in]   pSrcB : points to the second input vector
 *@param[out]  pDst : points to the output vector
 *@param[in]   blockSize : number of samples in each vector
 *@return      none
 */
STATIC INLINE void arm_mult_q15_opt(q15_t *pSrcA, q15_t *pSrcB, q15_t *pDst, uint32_t blockSize)
{

  rsi_arm_mult_q15_opt(pSrcA, pSrcB, pDst, blockSize, 0x00, BANK1, BANK2);
}

/**
 *@brief        This API is used for Q31 vector multiplication.
 *@param[in]    pSrcA : points to the first input vector
 *@param[in]    pSrcB : points to the second input vector
 *@param[out]   pDst : points to the output vector
 *@param[in]    blockSize : number of samples in each vector
 *@return       none
 */
STATIC INLINE void arm_mult_q31_opt(q31_t *pSrcA, q31_t *pSrcB, q31_t *pDst, uint32_t blockSize)
{

  rsi_arm_mult_q31_opt(pSrcA, pSrcB, pDst, blockSize, 0x00, BANK1, BANK2);
}

/**
 *@brief       This API is used for Vector Multiplication for complex-real data
 *@param[in]   pIn1 : is input vector A
 *@param[in]   pIn2 : is input vector B
 *@param[out]   pDst : points to output vector
 *@param[in]   blockSize : is size of the input array
 *return       none
 */
STATIC INLINE void fim_vector_mul_q15(q15_t *pIn1, q15_t *pIn2, q15_t *pDst, uint32_t blockSize)
{

  rsi_fim_vector_mul_q15(pIn1, pIn2, pDst, blockSize, 0x00, BANK1, BANK2);
}

/**
 *@brief     This API is used to set the FIM Vector Multiplication for complex-real data
 *@param[in]  pSrcCmplx : is input vector A
 *@param[in]  pSrcReal : is input vector B
 *@param[out]     pDst : points to the real output vector
 *@param[in]     numSamples : number of complex samples in the input vector
 *@return    none
 */
STATIC INLINE void arm_cmplx_mult_real_q15_opt(q15_t *pSrcCmplx, q15_t *pSrcReal, q15_t *pDst, uint32_t numSamples)
{

  rsi_arm_cmplx_mult_real_q15_opt(pSrcCmplx, pSrcReal, pDst, numSamples, 0x00, BANK1, BANK2);
}

/**
 *@brief       This API is used for Q15 complex-by-complex multiplication
 *@param[in]   pSrcA : points to the first input vector
 *@param[in]   pSrcB : points to the second input vector
 *@param[out]  pDst :  points to the output vector
 *@param[in]   numSamples : number of complex samples in each vector
 *return       none
 */
STATIC INLINE void arm_cmplx_mult_cmplx_q15_opt(q15_t *pSrcA, q15_t *pSrcB, q15_t *pDst, uint32_t numSamples)
{

  rsi_arm_cmplx_mult_cmplx_q15_opt(pSrcA, pSrcB, pDst, numSamples, 0x00, BANK1, BANK2);
}

/**
 *@brief     This API is used for Q15 complex magnitude squared
 *@param[in]     pSrc : points to the complex input vector
 *@param[out]     pDst : points to the real output vector
 *@param[in]     numSamples : number of complex samples in the input vector
 *@return    none
 */
STATIC INLINE void arm_cmplx_mag_squared_q15_opt(q15_t *pSrc, q15_t *pDst, uint32_t numSamples)
{

  rsi_arm_cmplx_mag_squared_q15_opt(pSrc, pDst, numSamples, 0x00, BANK1);
}

/**
 *@brief     This API is used to set the FIM Absolute Squaring for real number
 *@param[in]  pSrc : is input for squaring a number
 *@param[out]  pDst : points to output vector
 *@param[in]  length  :is size of the input array
 *@return     none
 */
STATIC INLINE void fim_absSqr_q7(q7_t *pSrc, q7_t *pDst, uint32_t length)
{

  rsi_fim_absSqr_q7(pSrc, pDst, length, 0x00, BANK1);
}

/**
 *@brief      This API is used to set the FIM Absolute Squaring for real number
 *@param[in]  pSrc : is input for squaring a number
 *@param[out]  pDst : points to output vector
 *@param[in]  length : is size of the input array
 *@return     none
 */
STATIC INLINE void fim_absSqr_q15(q15_t *pSrc, q15_t *pDst, uint32_t length)
{

  rsi_fim_absSqr_q15(pSrc, pDst, length, 0x00, BANK1);
}

/**
 *@brief     This API is used to set the FIM Absolute Squaring for real number
 *@param[in]  pSrc : is input for squaring a number
 *@param[in]  length : is size of the input array
 *@param[out]  pDst : points to output vector
 *@return      none
 */
STATIC INLINE void fim_absSqr_q31(q31_t *pSrc, q31_t *pDst, uint32_t length)
{

  rsi_fim_absSqr_q31(pSrc, pDst, length, 0x00, BANK1);
}

/**
 *@brief      This API is used to set the FIM Absolute Squaring for real number
 *@param[in]  pSrc : is input for squaring a number
 *@param[out]  pDst : points to output vector
 *@param[in]  length : is size of the input array
 *@return     none
 */
STATIC INLINE void fim_absSqr_f32(int32_t *pSrc, int32_t *pDst, uint32_t length)
{

  rsi_fim_absSqr_f32(pSrc, pDst, length, 0x00, BANK1);
}

/**
 *@brief        This API is used to set the FIM Matrix Multiplication for real numbers
 *@param[in]	pSrcA :	points to the first input matrix structure
 *@param[in]	pSrcB :	points to the second input matrix structure
 *@param[out]	pDst :	points to output matrix structure
 *@return       none
 *
 */
STATIC INLINE rsi_error_t arm_mat_mult_f32_opt(const arm_matrix_instance_f32_opt *pSrcA,
                                               const arm_matrix_instance_f32_opt *pSrcB,
                                               arm_matrix_instance_f32_opt *pDst)
{

  return rsi_arm_mat_mult_f32_opt(pSrcA, pSrcB, pDst, 0x00, BANK1, BANK2);
}

/**
 *@brief       This API is used to set the FIM Matrix Multiplication for real numbers
 *@param[in]  pSrcA :	points to the first input matrix structure
 *@param[in]  pSrcB :	points to the second input matrix structure
 *@param[out] pDst :	points to output matrix structure
 *@return     none
 */
STATIC INLINE rsi_error_t arm_mat_mult_q31_opt(const arm_matrix_instance_q31_opt *pSrcA,
                                               const arm_matrix_instance_q31_opt *pSrcB,
                                               arm_matrix_instance_q31_opt *pDst)
{

  return rsi_arm_mat_mult_q31_opt(pSrcA, pSrcB, pDst, 0x00, BANK1, BANK2);
}

/**
 *@brief        This API is used to set the FIM Matrix Multiplication for real numbers
 *@param[in]	pSrcA :	points to the first input matrix structure
 *@param[in]	pSrcB :	points to the second input matrix structure
 *@param[out]	pDst :	points to output matrix structure
 *@param[in]	pState :	points to the array for storing intermediate results (Unused)
 *@return        none
 */
STATIC INLINE rsi_error_t arm_mat_mult_q15_opt(const arm_matrix_instance_q15_opt *pSrcA,
                                               const arm_matrix_instance_q15_opt *pSrcB,
                                               arm_matrix_instance_q15_opt *pDst,
                                               q15_t *pState)
{

  return rsi_arm_mat_mult_q15_opt(pSrcA, pSrcB, pDst, pState, 0x00, BANK1, BANK2);
}

/**
 *@brief        This API is used to initialize the FIM  filters
 *@param[in,out]	S :	points to an instance of the floating-point FIR filter structure.
 *@param[in]	numTaps :	Number of filter coefficients in the filter.
 *@param[in]	pCoeffs :	points to the filter coefficients buffer.
 *@param[in]	pState :	points to the state buffer.
 *@param[in]	blockSize :	number of samples that are processed per call.
 *@return        none
 */
STATIC INLINE void arm_fir_init_f32_opt(arm_fir_instance_f32_opt *S,
                                        uint16_t numTaps,
                                        int32_t *pCoeffs,
                                        int32_t *pState,
                                        uint32_t blockSize)
{

  rsi_arm_fir_init_f32_opt(S, numTaps, pCoeffs, pState, blockSize);
}

/**
 *@brief     This API is used to set the FIM FIR Filter
 *@param[in]	S :	points to an instance of the floating-point FIR filter structure.
 *@param[in]	pSrc :	points to the block of input data.
 *@param[out]	pDst :	points to the block of output data.
 *@param[in]	blockSize :	number of samples to process per call.
 *@return     none
 */
STATIC INLINE void arm_fir_f32_opt(arm_fir_instance_f32_opt *S, int32_t *pSrc, int32_t *pDst, uint32_t blockSize)
{

  rsi_arm_fir_f32_opt(S, pSrc, pDst, blockSize, 0x00, BANK1, BANK2);
}
/**
 *@brief         This API is used to initialize the FIM  filters
 *@param[in,out] S :	points to an instance of the Q31 FIR filter structure.
 *@param[in]	 numTaps :	Number of filter coefficients in the filter.
 *@param[in]	 pCoeffs :	points to the filter coefficients buffer.
 *@param[in]	 pState :	points to the state buffer.
 *@param[in]	 blockSize :	number of samples that are processed per call.
 *return         none
 */
STATIC INLINE void arm_fir_init_q31_opt(arm_fir_instance_q31_opt *S,
                                        uint16_t numTaps,
                                        q31_t *pCoeffs,
                                        q31_t *pState,
                                        uint32_t blockSize)
{

  rsi_arm_fir_init_q31_opt(S, numTaps, pCoeffs, pState, blockSize);
}
/**
 *@brief        This API is used to set the FIM FIR Filter
 *@param[in]	S :	points to an instance of the Q31 FIR filter structure.
 *@param[in]	pSrc :	points to the block of input data.
 *@param[out]	pDst :	points to the block of output data.
 *@param[in]	blockSize :	number of samples to process per call
 *@return     none
 */

STATIC INLINE void arm_fir_q31_opt(arm_fir_instance_q31_opt *S, q31_t *pSrc, q31_t *pDst, uint32_t blockSize)
{

  rsi_arm_fir_q31_opt(S, pSrc, pDst, blockSize, 0x00, BANK1, BANK2);
}
/**
 *@brief         This API is used to initialise the FIM  filters
 *@param[in,out] S :	points to an instance of the Q15 FIR filter structure.
 *@param[in]	 numTaps :	Number of filter coefficients in the filter. Must be even and greater than or equal to 4.
 *@param[in]	 pCoeffs :	points to the filter coefficients buffer.
 *@param[in]	 pState :	points to the state buffer.
 *@param[in]	 blockSize :	is number of samples processed per call.
 *@return        none
 *
 */
STATIC INLINE void arm_fir_init_q15_opt(arm_fir_instance_q15_opt *S,
                                        uint16_t numTaps,
                                        q15_t *pCoeffs,
                                        q15_t *pState,
                                        uint32_t blockSize)
{

  rsi_arm_fir_init_q15_opt(S, numTaps, pCoeffs, pState, blockSize);
}
/**
 *@brief        This API is used to set the FIM FIR Filter
 *@param[in]	S :	points to an instance of the Q15 FIR structure.
 *@param[in]	pSrc :	points to the block of input data.
 *@param[out]	pDst :	points to the block of output data.
 *@param[in]	blockSize :	number of samples to process per call.
 *@return       none
 */
STATIC INLINE void arm_fir_q15_opt(arm_fir_instance_q15_opt *S, q15_t *pSrc, q15_t *pDst, uint32_t blockSize)
{

  rsi_arm_fir_q15_opt(S, pSrc, pDst, blockSize, 0x00, BANK1, BANK2);
}

/**
 *@brief         This API is used to initialise the FIM  filters
 *@param[in,out] S :	points to an instance of the Q7 FIR filter structure.
 *@param[in]	 numTaps :	Number of filter coefficients in the filter.
 *@param[in]	 pCoeffs :	points to the filter coefficients buffer.
 *@param[in]	 pState :	points to the state buffer.
 *@param[in]	 blockSize :	number of samples that are processed per call.
 *return         none
 */
STATIC INLINE void arm_fir_init_q7_opt(arm_fir_instance_q7_opt *S,
                                       uint16_t numTaps,
                                       q7_t *pCoeffs,
                                       q7_t *pState,
                                       uint32_t blockSize)
{

  rsi_arm_fir_init_q7_opt(S, numTaps, pCoeffs, pState, blockSize);
}
/**
 *@brief        This API is used to set the FIM FIR Filter
 *@param[in]	S :	points to an instance of the Q7 FIR filter structure.
 *@param[in]	pSrc :	points to the block of input data.
 *@param[out]	pDst :	points to the block of output data.
 *@param[in]	blockSize :	number of samples to process per call.
 *return        none
 */
STATIC INLINE void arm_fir_q7_opt(arm_fir_instance_q7_opt *S, q7_t *pSrc, q7_t *pDst, uint32_t blockSize)
{
  rsi_arm_fir_q7_opt(S, pSrc, pDst, blockSize, 0x00, BANK1, BANK2);
}

/**
 *@brief     This API Clears interrupt status of fim
 *@param[in]  ptFim : is pointer to the FIM register instance
 *@return    none
 */
STATIC INLINE void fim_interrupt_handler(volatile FIM_Type *ptFim)
{

  rsi_fim_interrupt_handler(ptFim);
}

/**
 *@brief        This API is used Processing function for the floating-point FIR interpolator.
 *@param[in]	S :	points to an instance of the floating-point FIR interpolator structure.
 *@param[in]	pSrc :	points to the block of input data.
 *@param[out]   pDst :	points to the block of output data.
 *@param[in]	blockSize :	number of input samples to process per call.
 *@return       none
 */
STATIC INLINE void arm_fir_interpolate_f32_opt(const arm_fir_interpolate_instance_f32_opt *S,
                                               int32_t *pSrc,
                                               int32_t *pDst,
                                               uint32_t blockSize)
{

  rsi_arm_fir_interpolate_f32_opt(S, pSrc, pDst, blockSize, 0x00, BANK1, BANK2);
}

/**
 *@brief             This API is used to initialization function for the floating-point FIR interpolator.
 *@param[in,out]     S :	points to an instance of the floating-point FIR interpolator structure.
 *@param[in]	     L	 :upsample factor.
 *@param[in]	     numTaps :	number of filter coefficients in the filter.
 *@param[in]	     pCoeffs :	points to the filter coefficient buffer.
 *@param[in]	     pState :	points to the state buffer.
 *@param[in]	     blockSize :	number of input samples to process per call.
 *
 *@return            The function returns \ref ARM_MATH_SUCCESS if initialization was successful or \ref ARM_MATH_LENGTH_ERROR if the filter length numTaps is not a multiple of the interpolation factor L.
 */
STATIC INLINE arm_status arm_fir_interpolate_init_f32_opt(arm_fir_interpolate_instance_f32_opt *S,
                                                          uint8_t L,
                                                          uint16_t numTaps,
                                                          int32_t *pCoeffs,
                                                          int32_t *pState,
                                                          uint32_t blockSize)
{

  return rsi_arm_fir_interpolate_init_f32_opt(S, L, numTaps, pCoeffs, pState, blockSize);
}

/**
 *@brief          This API is used to initialization function for the Q15 FIR interpolator.
 *@param[in,out]  S :	points to an instance of the Q15 FIR interpolator structure.
 *@param[in]	  L :	upsample factor.
 *@param[in]	  numTaps :	number of filter coefficients in the filter.
 *@param[in]	  pCoeffs :	points to the filter coefficient buffer.
 *@param[in]	  pState :	points to the state buffer.
 *@param[in]	  blockSize :	number of input samples to process per call
 *@return         The function returns \ref ARM_MATH_SUCCESS if initialization was successful or \ref ARM_MATH_LENGTH_ERROR if the filter length numTaps is not a multiple of the interpolation factor L.
 *
 */
STATIC INLINE arm_status arm_fir_interpolate_init_q15_opt(arm_fir_interpolate_instance_q15_opt *S,
                                                          uint8_t L,
                                                          uint16_t numTaps,
                                                          q15_t *pCoeffs,
                                                          q15_t *pState,
                                                          uint32_t blockSize)
{

  return rsi_arm_fir_interpolate_init_q15_opt(S, L, numTaps, pCoeffs, pState, blockSize);
}

/**
 *@brief            This API  is used to  initialization function for the Q31 FIR interpolator.
 *@param[in,out]    S :	points to an instance of the Q31 FIR interpolator structure.
 *@param[in]	    L :	upsample factor.
 *@param[in]	    numTaps :	number of filter coefficients in the filter.
 *@param[in]	    pCoeffs :	points to the filter coefficient buffer.
 *@param[in]	    pState :	points to the state buffer.
 *@param[in]	    blockSize :	number of input samples to process per call.
 *@return           The function returns \ref ARM_MATH_SUCCESS if initialization was successful or \ref ARM_MATH_LENGTH_ERROR if the filter length numTaps is not a multiple of the interpolation factor L.
 *
 */
STATIC INLINE arm_status arm_fir_interpolate_init_q31_opt(arm_fir_interpolate_instance_q31_opt *S,
                                                          uint8_t L,
                                                          uint16_t numTaps,
                                                          q31_t *pCoeffs,
                                                          q31_t *pState,
                                                          uint32_t blockSize)
{

  return rsi_arm_fir_interpolate_init_q31_opt(S, L, numTaps, pCoeffs, pState, blockSize);
}

/**
 *@brief        This API is used to Processing function for the Q15 FIR interpolator
 *@param[in]	S :	points to an instance of the Q15 FIR interpolator structure.
 *@param[in]	pSrc :	points to the block of input data.
 *@param[out]	pDst :	points to the block of output data.
 *@param[in]	blockSize :	number of input samples to process per call.
 *@return       none
 */
STATIC INLINE void arm_fir_interpolate_q15_opt(arm_fir_interpolate_instance_q15_opt *S,
                                               q15_t *pSrc,
                                               q15_t *pDst,
                                               uint32_t blockSize)
{

  rsi_arm_fir_interpolate_q15_opt(S, pSrc, pDst, blockSize, 0x00, BANK1, BANK2);
}

/**
 *@brief        This  API is used  to Processing function for the Q31 FIR interpolator.
 *@param[in]	S :	points to an instance of the Q31 FIR interpolator structure.
 *@param[in]	pSrc :	points to the block of input data.
 *@param[out]	pDst :	points to the block of output data.
 *@param[in]	blockSize :	number of input samples to process per call.
 *@return       none
 */
STATIC INLINE void arm_fir_interpolate_q31_opt(const arm_fir_interpolate_instance_q31_opt *S,
                                               q31_t *pSrc,
                                               q31_t *pDst,
                                               uint32_t blockSize)
{

  rsi_arm_fir_interpolate_q31_opt(S, pSrc, pDst, blockSize, 0x00, BANK1, BANK2);
}
/* function prototypes */
void arm_offset_f32_opt(int32_t *pSrc, int32_t scale, int32_t *pDst, uint32_t length);
void arm_offset_q7_opt(q7_t *pSrc, q7_t scale, q7_t *pDst, uint32_t length);

void arm_offset_q15_opt(q15_t *pSrc, q15_t scale, q15_t *pDst, uint32_t length);

void arm_offset_q31_opt(q31_t *pSrc, q31_t scale, q31_t *pDst, uint32_t length);

void fim_scalar_sub_q7(q7_t *pSrc, q7_t scale, q7_t *pDst, uint32_t blockSize);

void fim_scalar_sub_q31(q31_t *pSrc, q31_t scale, q31_t *pDst, uint32_t blockSize);
void fim_scalar_sub_f32(int32_t *pSrc, int32_t scale, int32_t *pDst, uint32_t length);

void arm_scale_f32_opt(int32_t *pSrc, int32_t scale, int32_t *pDst, uint32_t length);

void arm_scale_q7_opt(q7_t *pSrc, q7_t scaleFract, int8_t shift, q7_t *pDst, uint32_t blockSize);

void arm_scale_q15_opt(q15_t *pSrc, q15_t scaleFract, int8_t shift, q15_t *pDst, uint32_t blockSize);

void arm_scale_q31_opt(q31_t *pSrc, q31_t scaleFract, int8_t shift, q31_t *pDst, uint32_t blockSize);

void fim_scalar_mul_q15(q15_t *pSrc, q15_t *pDst, q15_t *scale, uint32_t blockSize, typ_data_t typ_data);

void arm_add_f32_opt(int32_t *pSrcA, int32_t *pDst, int32_t *pSrcB, uint32_t blockSize);

void arm_add_q7_opt(q7_t *pSrcA, q7_t *pSrcB, q7_t *pDst, uint32_t blockSize);

void arm_add_q15_opt(q15_t *pSrcA, q15_t *pSrcB, q15_t *pDst, uint32_t blockSize);

void arm_add_q31_opt(q31_t *pSrcA, q31_t *pSrcB, q31_t *pDst, uint32_t blockSize);

void fim_vector_add_q15(q15_t *pIn1, q15_t *pIn2, q15_t *pDst, uint32_t blockSize, typ_data_t typ_data);

void arm_sub_f32_opt(int32_t *pSrcA, int32_t *pSrcB, int32_t *pDst, uint32_t blockSize);

void arm_sub_q7_opt(q7_t *pSrcA, q7_t *pSrcB, q7_t *pDst, uint32_t blockSize);

void arm_sub_q15_opt(q15_t *pSrcA, q15_t *pSrcB, q15_t *pDst, uint32_t blockSize);

void arm_sub_q31_opt(q31_t *pSrcA, q31_t *pSrcB, q31_t *pDst, uint32_t blockSize);

void fim_read_data(uint32_t bank, uint32_t length, volatile void *pDst, uint8_t data_type, typ_data_t type_data);

void fim_vector_sub_q15(q15_t *pIn1, q15_t *pIn2, q15_t *pDst, uint32_t blockSize, typ_data_t typ_data);

void arm_mult_f32_opt(int32_t *pIn1, int32_t *pIn2, int32_t *pDst, uint32_t SatTruncRound, uint32_t length);

void arm_mult_q7_opt(q7_t *pSrcA, q7_t *pSrcB, q7_t *pDst, uint32_t blockSize);

void arm_mult_q15_opt(q15_t *pSrcA, q15_t *pSrcB, q15_t *pDst, uint32_t blockSize);

void arm_mult_q31_opt(q31_t *pSrcA, q31_t *pSrcB, q31_t *pDst, uint32_t blockSize);

void fim_vector_mul_q15(q15_t *pIn1, q15_t *pIn2, q15_t *pDst, uint32_t blockSize);

void arm_cmplx_mult_real_q15_opt(q15_t *pSrcCmplx, q15_t *pSrcReal, q15_t *pDst, uint32_t numSamples);

void arm_cmplx_mult_cmplx_q15_opt(q15_t *pSrcA, q15_t *pSrcB, q15_t *pDst, uint32_t numSamples);

void arm_cmplx_mag_squared_q15_opt(q15_t *pSrc, q15_t *pDst, uint32_t numSamples);

void fim_absSqr_q7(q7_t *pSrc, q7_t *pDst, uint32_t length);

void fim_absSqr_q15(q15_t *pSrc, q15_t *pDst, uint32_t length);

void fim_absSqr_q31(q31_t *pSrc, q31_t *pDst, uint32_t length);

void fim_absSqr_f32(int32_t *pSrc, int32_t *pDst, uint32_t length);

rsi_error_t arm_mat_mult_f32_opt(const arm_matrix_instance_f32_opt *pSrcA,
                                 const arm_matrix_instance_f32_opt *pSrcB,
                                 arm_matrix_instance_f32_opt *pDst);

rsi_error_t arm_mat_mult_q31_opt(const arm_matrix_instance_q31_opt *pSrcA,
                                 const arm_matrix_instance_q31_opt *pSrcB,
                                 arm_matrix_instance_q31_opt *pDst);

rsi_error_t arm_mat_mult_q15_opt(const arm_matrix_instance_q15_opt *pSrcA,
                                 const arm_matrix_instance_q15_opt *pSrcB,
                                 arm_matrix_instance_q15_opt *pDst,
                                 q15_t *pState);

void arm_fir_init_f32_opt(arm_fir_instance_f32_opt *S,
                          uint16_t numTaps,
                          int32_t *pCoeffs,
                          int32_t *pState,
                          uint32_t blockSize);

void arm_fir_f32_opt(arm_fir_instance_f32_opt *S, int32_t *pSrc, int32_t *pDst, uint32_t blockSize);

void arm_fir_init_q31_opt(arm_fir_instance_q31_opt *S,
                          uint16_t numTaps,
                          q31_t *pCoeffs,
                          q31_t *pState,
                          uint32_t blockSize);

void arm_fir_q31_opt(arm_fir_instance_q31_opt *S, q31_t *pSrc, q31_t *pDst, uint32_t blockSize);

void arm_fir_init_q15_opt(arm_fir_instance_q15_opt *S,
                          uint16_t numTaps,
                          q15_t *pCoeffs,
                          q15_t *pState,
                          uint32_t blockSize);

void arm_fir_q15_opt(arm_fir_instance_q15_opt *S, q15_t *pSrc, q15_t *pDst, uint32_t blockSize);

void arm_fir_init_q7_opt(arm_fir_instance_q7_opt *S, uint16_t numTaps, q7_t *pCoeffs, q7_t *pState, uint32_t blockSize);

void arm_fir_q7_opt(arm_fir_instance_q7_opt *S, q7_t *pSrc, q7_t *pDst, uint32_t blockSize);

void fim_interrupt_handler(volatile FIM_Type *ptFim);

void arm_fir_interpolate_f32_opt(const arm_fir_interpolate_instance_f32_opt *S,
                                 int32_t *pSrc,
                                 int32_t *pDst,
                                 uint32_t blockSize);

arm_status arm_fir_interpolate_init_f32_opt(arm_fir_interpolate_instance_f32_opt *S,
                                            uint8_t L,
                                            uint16_t numTaps,
                                            int32_t *pCoeffs,
                                            int32_t *pState,
                                            uint32_t blockSize);

arm_status arm_fir_interpolate_init_q15_opt(arm_fir_interpolate_instance_q15_opt *S,
                                            uint8_t L,
                                            uint16_t numTaps,
                                            q15_t *pCoeffs,
                                            q15_t *pState,
                                            uint32_t blockSize);

arm_status arm_fir_interpolate_init_q31_opt(arm_fir_interpolate_instance_q31_opt *S,
                                            uint8_t L,
                                            uint16_t numTaps,
                                            q31_t *pCoeffs,
                                            q31_t *pState,
                                            uint32_t blockSize);

void arm_fir_interpolate_q15_opt(arm_fir_interpolate_instance_q15_opt *S, q15_t *pSrc, q15_t *pDst, uint32_t blockSize);

void arm_fir_interpolate_q31_opt(const arm_fir_interpolate_instance_q31_opt *S,
                                 q31_t *pSrc,
                                 q31_t *pDst,
                                 uint32_t blockSize);

#ifdef __cplusplus
}
#endif
#endif /*__RSI_ROM_FIM_H__*/

/* @}end of RSI_FIM_DRIVERS */
