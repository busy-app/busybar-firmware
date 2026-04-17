/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#pragma once

//C1 Register Bit Field Defines
#define SLI_C1_INIT_CMD 0x15
// sent to spi interface after reset/powerup to init the spi interface

//STM 32 Init Sequence
#define SLI_SI91X_INIT_CMD 0x005c4a12

// SPI Start Token
#define SLI_SPI_START_TOKEN 0x55

#define SLI_C1_INIT_RESP 0x55
// response from spi interface to successful init

#define SLI_C176_INIT      0b00111111 //@ and
#define SLI_C176_RW        0b01000000 //@ or
#define SLI_C15_RD         0b11011111 //@ and
#define SLI_C15_WR         0b00100000 //@ or
#define SLI_C14_SPISLV     0b11101111 //@ and
#define SLI_C14_AHBBUS     0b00010000 //@ or
#define SLI_C13_AHBMEM     0b11110111 //@ and
#define SLI_C13_AHBFRM     0b00001000 //@ or
#define SLI_C12_02BITXFR   0b11111011 //@ and
#define SLI_C12_16BITXFR   0b00000100 //@ or
#define SLI_C110_4BYTESLEN 0b11111100
// and number of C bytes transferred, usually 4
#define SLI_C110_1BYTESLEN 0b00000001
// or/~and number of C bytes transferred
#define SLI_C110_2BYTESLEN 0b00000010
// or/~and number of C bytes transferred
#define SLI_C110_3BYTESLEN 0b00000011
// or number of C bytes transferred

/*=========================================================*/
// C2 Register Defines
#define SLI_C276_08BIT      0b00111111 //@ and
#define SLI_C276_32BIT      0b01000000 //@ or
#define SLI_C250_SPIADDR    0b00111111 //@ and
#define SLI_C250_DATAPACKET 0b00000010 //@ or
#define SLI_C250_MGMTPACKET 0b00000100 //@ or

//#define SLI_C3_XFERLENLSB  0x00                 //@ set value
//#define SLI_C4_XFERLENMSB  0x00                 //@ set value

/*==========================================================*/
// C1 Register Defines
// Internal Read
#define SLI_C1INTREAD2BYTES 0x42
// (((((0x00 & C176_INT) & C15_RD) & C14_SPISLV) & C13_AHBMEM) & C12_02BITXFR) | C110_2BYTESLEN // 01000010
#define SLI_C1INTREAD1BYTES 0x41

// Memory ReadWrite (AHB Master Read/Write, Internal Legacy Name)
// Memory read/write is normally done using 16-bit transfer length with 4 C bytes transferred
#define SLI_C1MEMWR16BIT1BYTE 0x75
// (((0x00 | C176_RW | C15_WR | C14_AHBBUS) & C13_AHBMEM) & C12_16BITXFR) | C110_1BYTESLEN // 01110101
#define SLI_C1MEMRD16BIT1BYTE 0x55
// (((0x00 | C176_RW & C15_RD | C14_AHBBUS) & C13_AHBMEM) & C12_16BITXFR) | C110_1BYTESLEN // 01010101

#define SLI_C1MEMWR16BIT4BYTE 0x74
// (((0x00 | C176_RW | C15_WR | C14_AHBBUS) & C13_AHBMEM) & C12_16BITXFR) & C110_4BYTESLEN // 01110100
#define SLI_C1MEMRD16BIT4BYTE 0x54
// ((((0x00 | C176_RW & C15_RD) | C14_AHBBUS) & C13_AHBMEM) & C12_16BITXFR) & C110_4BYTESLEN // 01010100

// Normally, 2-bit transfer length is not used for memory read/write
#define SLI_C1MEMWR02BIT1BYTE 0x71
// (((0x00 | C176_RW | C15_WR | C14_AHBBUS) & C13_AHBMEM) & C12_02BITXFR) | C110_1BYTESLEN // 01110001
#define SLI_C1MEMRD02BIT1BYTE 0x51
// (((0x00 | C176_RW & C15_RD | C14_AHBBUS) & C13_AHBMEM) & C12_02BITXFR) | C110_1BYTESLEN // 01010001
#define SLI_C1MEMWR02BIT4BYTE 0x70
// (((0x00 | C176_RW | C15_WR | C14_AHBBUS) & C13_AHBMEM) & C12_02BITXFR) & C110_4BYTESLEN // 01110000
#define SLI_C1MEMRD02BIT4BYTE 0x50
// ((((0x00 | C176_RW & C15_RD) | C14_AHBBUS) & C13_AHBMEM) & C12_02BITXFR) & C110_4BYTESLEN // 01010000

// Frame ReadWrite
// Frame read/writes normally  use 16-bit transfer length
#define SLI_C1FRMWR16BIT1BYTE 0x7d
// ((C176_RW | C15_WR | C14_AHBBUS | C13_AHBFRM) & C12_16BITXFR) | C110_1BYTESLEN // 01111101
#define SLI_C1FRMRD16BIT1BYTE 0x5d
// ((C176_RW & C15_RD | C14_AHBBUS | C13_AHBFRM) & C12_16BITXFR) | C110_1BYTESLEN // 01011101

#define SLI_C1FRMWR16BIT4BYTE 0x7c
// ((C176_RW | C15_WR | C14_AHBBUS | C13_AHBFRM) & C12_16BITXFR) | C110_4BYTESLEN // 01111100
#define SLI_C1FRMRD16BIT4BYTE 0x5c
// ((C176_RW & C15_RD | C14_AHBBUS | C13_AHBFRM) & C12_16BITXFR) | C110_4BYTESLEN // 01011100

// Frame read/writes normally do not use 2-bit transfer length
#define SLI_C1FRMWR02BIT1BYTE 0x79
// ((C176_RW | C15_WR | C14_AHBBUS | C13_AHBFRM) & C12_02BITXFR) | C110_1BYTESLEN // 01111001
#define SLI_C1FRMRD02BIT1BYTE 0x59
// ((C176_RW & C15_RD | C14_AHBBUS | C13_AHBFRM) & C12_02BITXFR) | C110_1BYTESLEN // 01011001

#define SLI_C1FRMWR02BIT4BYTE 0x78
// ((C176_RW | C15_WR | C14_AHBBUS | C13_AHBFRM) & C12_02BITXFR) | C110_4BYTESLEN // 01111000
#define SLI_C1FRMRD02BIT4BYTE 0x58
// ((C176_RW & C15_RD | C14_AHBBUS | C13_AHBFRM) & C12_02BITXFR) | C110_4BYTESLEN // 01011000

// SPI Register ReadWrite
#define SLI_C1SPIREGWR16BIT4BYTE 0x64
// ((((C176_RW | C15_WR) & C14_SPISLV) & C13_AHBMEM) & C12_16BITXFR) | C110_4BYTESLEN // 01100100
#define SLI_C1SPIREGRD16BIT4BYTE 0x44
// ((((C176_RW & C15_RD) & C14_SPISLV) & C13_AHBMEM) & C12_16BITXFR) | C110_4BYTESLEN // 01000100

#define SLI_C1SPIREGWR02BIT4BYTE 0x60
// ((((C176_RW | C15_WR) & C14_SPISLV) & C13_AHBMEM) & C12_02BITXFR) | C110_4BYTESLEN // 01100000
#define SLI_C1SPIREGRD02BIT4BYTE 0x40
// ((((C176_RW & C15_RD) & C14_SPISLV) & C13_AHBMEM) & C12_02BITXFR) | C110_4BYTESLEN // 01000000

#define SLI_C1SPIREGWR02BIT1BYTE 0x61
// ((((C176_RW | C15_WR) & C14_SPISLV) & C13_AHBMEM) & C12_02BITXFR) | C110_1BYTESLEN // 01100001
#define SLI_C1SPIREGRD02BIT1BYTE 0x41
// ((((C176_RW & C15_RD) & C14_SPISLV) & C13_AHBMEM) & C12_02BITXFR) | C110_1BYTESLEN // 01000001

//C2 Register Defines
#define SLI_C2RDWR4BYTE 0x40 //@ 0x00 | C276_32BIT | C250_DATAPACKET   // 01000010
#define SLI_C2RDWR4BYTE 0x40 //@ 0x00 | C276_32BIT | C250_MGMTPACKET   // 01000100

#define SLI_C2RDWR1BYTE 0x00 //@ (0x00 & C276_08BIT) | C250_DATAPACKET // 00000010
#define SLI_C2RDWR1BYTE 0x00 //@ (0x00 & C276_08BIT) | C250_MGMTPACKET
//@ 00000100
#define SLI_C2MGMT 0x04
#define SLI_C2DATA 0x02
#define SLI_C2WPS  0x08

#define SLI_C2SPIADDR1BYTE  0x00 //@(0x00 & C276_08BIT) | C250_SPIADDR   // 00xxxxxx
#define SLI_C2MEMRDWRNOCARE 0x00 //@0x00 or ANYTHING                     // 00000000
#define SLI_C2SPIADDR4BYTE  0x40 //@(0x00 | C276_32BIT) | C250_SPIADDR   // 01xxxxxx

#define SLI_C1INTWRITE1BYTES 0x61
#define SLI_C1INTWRITE2BYTES 0x62
