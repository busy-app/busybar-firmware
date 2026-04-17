/**************************************************************************//**
 * @file
 * @brief Device Manager Clock API Definition
 ******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories, Inc. www.silabs.com</b>
 ******************************************************************************
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
 *****************************************************************************/

#include "sl_device_gpio.h"
#include "cmsis_compiler.h"

/***************************************************************************//**
 * @addtogroup device_gpio Device Manager GPIO
 * @{
 ******************************************************************************/

/// Declarations for port and pins
const sl_gpio_t pa0 = { .port = SL_GPIO_PORT_A, .pin = 0 };
const sl_gpio_t pa1 = { .port = SL_GPIO_PORT_A, .pin = 1 };
const sl_gpio_t pa2 = { .port = SL_GPIO_PORT_A, .pin = 2 };
const sl_gpio_t pa3 = { .port = SL_GPIO_PORT_A, .pin = 3 };
const sl_gpio_t pa4 = { .port = SL_GPIO_PORT_A, .pin = 4 };
const sl_gpio_t pa5 = { .port = SL_GPIO_PORT_A, .pin = 5 };
const sl_gpio_t pa6 = { .port = SL_GPIO_PORT_A, .pin = 6 };
const sl_gpio_t pa7 = { .port = SL_GPIO_PORT_A, .pin = 7 };
const sl_gpio_t pa8 = { .port = SL_GPIO_PORT_A, .pin = 8 };
const sl_gpio_t pa9 = { .port = SL_GPIO_PORT_A, .pin = 9 };
const sl_gpio_t pa10 = { .port = SL_GPIO_PORT_A, .pin = 10 };
const sl_gpio_t pa11 = { .port = SL_GPIO_PORT_A, .pin = 11 };
const sl_gpio_t pa12 = { .port = SL_GPIO_PORT_A, .pin = 12 };
const sl_gpio_t pa13 = { .port = SL_GPIO_PORT_A, .pin = 13 };
const sl_gpio_t pa14 = { .port = SL_GPIO_PORT_A, .pin = 14 };
const sl_gpio_t pa15 = { .port = SL_GPIO_PORT_A, .pin = 15 };
const sl_gpio_t pa16 = { .port = SL_GPIO_PORT_A, .pin = 16 };
const sl_gpio_t pa17 = { .port = SL_GPIO_PORT_A, .pin = 17 };
const sl_gpio_t pa18 = { .port = SL_GPIO_PORT_A, .pin = 18 };
const sl_gpio_t pa19 = { .port = SL_GPIO_PORT_A, .pin = 19 };
const sl_gpio_t pa20 = { .port = SL_GPIO_PORT_A, .pin = 20 };
const sl_gpio_t pa21 = { .port = SL_GPIO_PORT_A, .pin = 21 };
const sl_gpio_t pa22 = { .port = SL_GPIO_PORT_A, .pin = 22 };
const sl_gpio_t pa23 = { .port = SL_GPIO_PORT_A, .pin = 23 };
const sl_gpio_t pa24 = { .port = SL_GPIO_PORT_A, .pin = 24 };
const sl_gpio_t pa25 = { .port = SL_GPIO_PORT_A, .pin = 25 };
const sl_gpio_t pa26 = { .port = SL_GPIO_PORT_A, .pin = 26 };
const sl_gpio_t pa27 = { .port = SL_GPIO_PORT_A, .pin = 27 };
const sl_gpio_t pa28 = { .port = SL_GPIO_PORT_A, .pin = 28 };
const sl_gpio_t pa29 = { .port = SL_GPIO_PORT_A, .pin = 29 };
const sl_gpio_t pa30 = { .port = SL_GPIO_PORT_A, .pin = 30 };
const sl_gpio_t pa31 = { .port = SL_GPIO_PORT_A, .pin = 31 };

const sl_gpio_t pb0 = { .port = SL_GPIO_PORT_B, .pin = 0 };
const sl_gpio_t pb1 = { .port = SL_GPIO_PORT_B, .pin = 1 };
const sl_gpio_t pb2 = { .port = SL_GPIO_PORT_B, .pin = 2 };
const sl_gpio_t pb3 = { .port = SL_GPIO_PORT_B, .pin = 3 };
const sl_gpio_t pb4 = { .port = SL_GPIO_PORT_B, .pin = 4 };
const sl_gpio_t pb5 = { .port = SL_GPIO_PORT_B, .pin = 5 };
const sl_gpio_t pb6 = { .port = SL_GPIO_PORT_B, .pin = 6 };
const sl_gpio_t pb7 = { .port = SL_GPIO_PORT_B, .pin = 7 };
const sl_gpio_t pb8 = { .port = SL_GPIO_PORT_B, .pin = 8 };
const sl_gpio_t pb9 = { .port = SL_GPIO_PORT_B, .pin = 9 };
const sl_gpio_t pb10 = { .port = SL_GPIO_PORT_B, .pin = 10 };
const sl_gpio_t pb11 = { .port = SL_GPIO_PORT_B, .pin = 11 };
const sl_gpio_t pb12 = { .port = SL_GPIO_PORT_B, .pin = 12 };
const sl_gpio_t pb13 = { .port = SL_GPIO_PORT_B, .pin = 13 };
const sl_gpio_t pb14 = { .port = SL_GPIO_PORT_B, .pin = 14 };
const sl_gpio_t pb15 = { .port = SL_GPIO_PORT_B, .pin = 15 };
const sl_gpio_t pb16 = { .port = SL_GPIO_PORT_B, .pin = 16 };
const sl_gpio_t pb17 = { .port = SL_GPIO_PORT_B, .pin = 17 };
const sl_gpio_t pb18 = { .port = SL_GPIO_PORT_B, .pin = 18 };
const sl_gpio_t pb19 = { .port = SL_GPIO_PORT_B, .pin = 19 };
const sl_gpio_t pb20 = { .port = SL_GPIO_PORT_B, .pin = 20 };
const sl_gpio_t pb21 = { .port = SL_GPIO_PORT_B, .pin = 21 };
const sl_gpio_t pb22 = { .port = SL_GPIO_PORT_B, .pin = 22 };
const sl_gpio_t pb23 = { .port = SL_GPIO_PORT_B, .pin = 23 };
const sl_gpio_t pb24 = { .port = SL_GPIO_PORT_B, .pin = 24 };
const sl_gpio_t pb25 = { .port = SL_GPIO_PORT_B, .pin = 25 };
const sl_gpio_t pb26 = { .port = SL_GPIO_PORT_B, .pin = 26 };
const sl_gpio_t pb27 = { .port = SL_GPIO_PORT_B, .pin = 27 };
const sl_gpio_t pb28 = { .port = SL_GPIO_PORT_B, .pin = 28 };
const sl_gpio_t pb29 = { .port = SL_GPIO_PORT_B, .pin = 29 };
const sl_gpio_t pb30 = { .port = SL_GPIO_PORT_B, .pin = 30 };
const sl_gpio_t pb31 = { .port = SL_GPIO_PORT_B, .pin = 31 };

const sl_gpio_t pc0 = { .port = SL_GPIO_PORT_C, .pin = 0 };
const sl_gpio_t pc1 = { .port = SL_GPIO_PORT_C, .pin = 1 };
const sl_gpio_t pc2 = { .port = SL_GPIO_PORT_C, .pin = 2 };
const sl_gpio_t pc3 = { .port = SL_GPIO_PORT_C, .pin = 3 };
const sl_gpio_t pc4 = { .port = SL_GPIO_PORT_C, .pin = 4 };
const sl_gpio_t pc5 = { .port = SL_GPIO_PORT_C, .pin = 5 };
const sl_gpio_t pc6 = { .port = SL_GPIO_PORT_C, .pin = 6 };
const sl_gpio_t pc7 = { .port = SL_GPIO_PORT_C, .pin = 7 };
const sl_gpio_t pc8 = { .port = SL_GPIO_PORT_C, .pin = 8 };
const sl_gpio_t pc9 = { .port = SL_GPIO_PORT_C, .pin = 9 };
const sl_gpio_t pc10 = { .port = SL_GPIO_PORT_C, .pin = 10 };
const sl_gpio_t pc11 = { .port = SL_GPIO_PORT_C, .pin = 11 };
const sl_gpio_t pc12 = { .port = SL_GPIO_PORT_C, .pin = 12 };
const sl_gpio_t pc13 = { .port = SL_GPIO_PORT_C, .pin = 13 };
const sl_gpio_t pc14 = { .port = SL_GPIO_PORT_C, .pin = 14 };
const sl_gpio_t pc15 = { .port = SL_GPIO_PORT_C, .pin = 15 };
const sl_gpio_t pc16 = { .port = SL_GPIO_PORT_C, .pin = 16 };
const sl_gpio_t pc17 = { .port = SL_GPIO_PORT_C, .pin = 17 };
const sl_gpio_t pc18 = { .port = SL_GPIO_PORT_C, .pin = 18 };
const sl_gpio_t pc19 = { .port = SL_GPIO_PORT_C, .pin = 19 };
const sl_gpio_t pc20 = { .port = SL_GPIO_PORT_C, .pin = 20 };
const sl_gpio_t pc21 = { .port = SL_GPIO_PORT_C, .pin = 21 };
const sl_gpio_t pc22 = { .port = SL_GPIO_PORT_C, .pin = 22 };
const sl_gpio_t pc23 = { .port = SL_GPIO_PORT_C, .pin = 23 };
const sl_gpio_t pc24 = { .port = SL_GPIO_PORT_C, .pin = 24 };
const sl_gpio_t pc25 = { .port = SL_GPIO_PORT_C, .pin = 25 };
const sl_gpio_t pc26 = { .port = SL_GPIO_PORT_C, .pin = 26 };
const sl_gpio_t pc27 = { .port = SL_GPIO_PORT_C, .pin = 27 };
const sl_gpio_t pc28 = { .port = SL_GPIO_PORT_C, .pin = 28 };
const sl_gpio_t pc29 = { .port = SL_GPIO_PORT_C, .pin = 29 };
const sl_gpio_t pc30 = { .port = SL_GPIO_PORT_C, .pin = 30 };
const sl_gpio_t pc31 = { .port = SL_GPIO_PORT_C, .pin = 31 };

const sl_gpio_t pd0 = { .port = SL_GPIO_PORT_D, .pin = 0 };
const sl_gpio_t pd1 = { .port = SL_GPIO_PORT_D, .pin = 1 };
const sl_gpio_t pd2 = { .port = SL_GPIO_PORT_D, .pin = 2 };
const sl_gpio_t pd3 = { .port = SL_GPIO_PORT_D, .pin = 3 };
const sl_gpio_t pd4 = { .port = SL_GPIO_PORT_D, .pin = 4 };
const sl_gpio_t pd5 = { .port = SL_GPIO_PORT_D, .pin = 5 };
const sl_gpio_t pd6 = { .port = SL_GPIO_PORT_D, .pin = 6 };
const sl_gpio_t pd7 = { .port = SL_GPIO_PORT_D, .pin = 7 };
const sl_gpio_t pd8 = { .port = SL_GPIO_PORT_D, .pin = 8 };
const sl_gpio_t pd9 = { .port = SL_GPIO_PORT_D, .pin = 9 };
const sl_gpio_t pd10 = { .port = SL_GPIO_PORT_D, .pin = 10 };
const sl_gpio_t pd11 = { .port = SL_GPIO_PORT_D, .pin = 11 };
const sl_gpio_t pd12 = { .port = SL_GPIO_PORT_D, .pin = 12 };
const sl_gpio_t pd13 = { .port = SL_GPIO_PORT_D, .pin = 13 };
const sl_gpio_t pd14 = { .port = SL_GPIO_PORT_D, .pin = 14 };
const sl_gpio_t pd15 = { .port = SL_GPIO_PORT_D, .pin = 15 };
const sl_gpio_t pd16 = { .port = SL_GPIO_PORT_D, .pin = 16 };
const sl_gpio_t pd17 = { .port = SL_GPIO_PORT_D, .pin = 17 };
const sl_gpio_t pd18 = { .port = SL_GPIO_PORT_D, .pin = 18 };
const sl_gpio_t pd19 = { .port = SL_GPIO_PORT_D, .pin = 19 };
const sl_gpio_t pd20 = { .port = SL_GPIO_PORT_D, .pin = 20 };
const sl_gpio_t pd21 = { .port = SL_GPIO_PORT_D, .pin = 21 };
const sl_gpio_t pd22 = { .port = SL_GPIO_PORT_D, .pin = 22 };
const sl_gpio_t pd23 = { .port = SL_GPIO_PORT_D, .pin = 23 };
const sl_gpio_t pd24 = { .port = SL_GPIO_PORT_D, .pin = 24 };
const sl_gpio_t pd25 = { .port = SL_GPIO_PORT_D, .pin = 25 };
const sl_gpio_t pd26 = { .port = SL_GPIO_PORT_D, .pin = 26 };
const sl_gpio_t pd27 = { .port = SL_GPIO_PORT_D, .pin = 27 };
const sl_gpio_t pd28 = { .port = SL_GPIO_PORT_D, .pin = 28 };
const sl_gpio_t pd29 = { .port = SL_GPIO_PORT_D, .pin = 29 };
const sl_gpio_t pd30 = { .port = SL_GPIO_PORT_D, .pin = 30 };
const sl_gpio_t pd31 = { .port = SL_GPIO_PORT_D, .pin = 31 };

const sl_gpio_t pe0 = { .port = SL_GPIO_PORT_E, .pin = 0 };
const sl_gpio_t pe1 = { .port = SL_GPIO_PORT_E, .pin = 1 };
const sl_gpio_t pe2 = { .port = SL_GPIO_PORT_E, .pin = 2 };
const sl_gpio_t pe3 = { .port = SL_GPIO_PORT_E, .pin = 3 };
const sl_gpio_t pe4 = { .port = SL_GPIO_PORT_E, .pin = 4 };
const sl_gpio_t pe5 = { .port = SL_GPIO_PORT_E, .pin = 5 };
const sl_gpio_t pe6 = { .port = SL_GPIO_PORT_E, .pin = 6 };
const sl_gpio_t pe7 = { .port = SL_GPIO_PORT_E, .pin = 7 };
const sl_gpio_t pe8 = { .port = SL_GPIO_PORT_E, .pin = 8 };
const sl_gpio_t pe9 = { .port = SL_GPIO_PORT_E, .pin = 9 };
const sl_gpio_t pe10 = { .port = SL_GPIO_PORT_E, .pin = 10 };
const sl_gpio_t pe11 = { .port = SL_GPIO_PORT_E, .pin = 11 };
const sl_gpio_t pe12 = { .port = SL_GPIO_PORT_E, .pin = 12 };
const sl_gpio_t pe13 = { .port = SL_GPIO_PORT_E, .pin = 13 };
const sl_gpio_t pe14 = { .port = SL_GPIO_PORT_E, .pin = 14 };
const sl_gpio_t pe15 = { .port = SL_GPIO_PORT_E, .pin = 15 };
const sl_gpio_t pe16 = { .port = SL_GPIO_PORT_E, .pin = 16 };
const sl_gpio_t pe17 = { .port = SL_GPIO_PORT_E, .pin = 17 };
const sl_gpio_t pe18 = { .port = SL_GPIO_PORT_E, .pin = 18 };
const sl_gpio_t pe19 = { .port = SL_GPIO_PORT_E, .pin = 19 };
const sl_gpio_t pe20 = { .port = SL_GPIO_PORT_E, .pin = 20 };
const sl_gpio_t pe21 = { .port = SL_GPIO_PORT_E, .pin = 21 };
const sl_gpio_t pe22 = { .port = SL_GPIO_PORT_E, .pin = 22 };
const sl_gpio_t pe23 = { .port = SL_GPIO_PORT_E, .pin = 23 };
const sl_gpio_t pe24 = { .port = SL_GPIO_PORT_E, .pin = 24 };
const sl_gpio_t pe25 = { .port = SL_GPIO_PORT_E, .pin = 25 };
const sl_gpio_t pe26 = { .port = SL_GPIO_PORT_E, .pin = 26 };
const sl_gpio_t pe27 = { .port = SL_GPIO_PORT_E, .pin = 27 };
const sl_gpio_t pe28 = { .port = SL_GPIO_PORT_E, .pin = 28 };
const sl_gpio_t pe29 = { .port = SL_GPIO_PORT_E, .pin = 29 };
const sl_gpio_t pe30 = { .port = SL_GPIO_PORT_E, .pin = 30 };
const sl_gpio_t pe31 = { .port = SL_GPIO_PORT_E, .pin = 31 };

const sl_gpio_t pf0 = { .port = SL_GPIO_PORT_F, .pin = 0 };
const sl_gpio_t pf1 = { .port = SL_GPIO_PORT_F, .pin = 1 };
const sl_gpio_t pf2 = { .port = SL_GPIO_PORT_F, .pin = 2 };
const sl_gpio_t pf3 = { .port = SL_GPIO_PORT_F, .pin = 3 };
const sl_gpio_t pf4 = { .port = SL_GPIO_PORT_F, .pin = 4 };
const sl_gpio_t pf5 = { .port = SL_GPIO_PORT_F, .pin = 5 };
const sl_gpio_t pf6 = { .port = SL_GPIO_PORT_F, .pin = 6 };
const sl_gpio_t pf7 = { .port = SL_GPIO_PORT_F, .pin = 7 };
const sl_gpio_t pf8 = { .port = SL_GPIO_PORT_F, .pin = 8 };
const sl_gpio_t pf9 = { .port = SL_GPIO_PORT_F, .pin = 9 };
const sl_gpio_t pf10 = { .port = SL_GPIO_PORT_F, .pin = 10 };
const sl_gpio_t pf11 = { .port = SL_GPIO_PORT_F, .pin = 11 };
const sl_gpio_t pf12 = { .port = SL_GPIO_PORT_F, .pin = 12 };
const sl_gpio_t pf13 = { .port = SL_GPIO_PORT_F, .pin = 13 };
const sl_gpio_t pf14 = { .port = SL_GPIO_PORT_F, .pin = 14 };
const sl_gpio_t pf15 = { .port = SL_GPIO_PORT_F, .pin = 15 };
const sl_gpio_t pf16 = { .port = SL_GPIO_PORT_F, .pin = 16 };
const sl_gpio_t pf17 = { .port = SL_GPIO_PORT_F, .pin = 17 };
const sl_gpio_t pf18 = { .port = SL_GPIO_PORT_F, .pin = 18 };
const sl_gpio_t pf19 = { .port = SL_GPIO_PORT_F, .pin = 19 };
const sl_gpio_t pf20 = { .port = SL_GPIO_PORT_F, .pin = 20 };
const sl_gpio_t pf21 = { .port = SL_GPIO_PORT_F, .pin = 21 };
const sl_gpio_t pf22 = { .port = SL_GPIO_PORT_F, .pin = 22 };
const sl_gpio_t pf23 = { .port = SL_GPIO_PORT_F, .pin = 23 };
const sl_gpio_t pf24 = { .port = SL_GPIO_PORT_F, .pin = 24 };
const sl_gpio_t pf25 = { .port = SL_GPIO_PORT_F, .pin = 25 };
const sl_gpio_t pf26 = { .port = SL_GPIO_PORT_F, .pin = 26 };
const sl_gpio_t pf27 = { .port = SL_GPIO_PORT_F, .pin = 27 };
const sl_gpio_t pf28 = { .port = SL_GPIO_PORT_F, .pin = 28 };
const sl_gpio_t pf29 = { .port = SL_GPIO_PORT_F, .pin = 29 };
const sl_gpio_t pf30 = { .port = SL_GPIO_PORT_F, .pin = 30 };
const sl_gpio_t pf31 = { .port = SL_GPIO_PORT_F, .pin = 31 };

const sl_gpio_t pg0 = { .port = SL_GPIO_PORT_G, .pin = 0 };
const sl_gpio_t pg1 = { .port = SL_GPIO_PORT_G, .pin = 1 };
const sl_gpio_t pg2 = { .port = SL_GPIO_PORT_G, .pin = 2 };
const sl_gpio_t pg3 = { .port = SL_GPIO_PORT_G, .pin = 3 };
const sl_gpio_t pg4 = { .port = SL_GPIO_PORT_G, .pin = 4 };
const sl_gpio_t pg5 = { .port = SL_GPIO_PORT_G, .pin = 5 };
const sl_gpio_t pg6 = { .port = SL_GPIO_PORT_G, .pin = 6 };
const sl_gpio_t pg7 = { .port = SL_GPIO_PORT_G, .pin = 7 };
const sl_gpio_t pg8 = { .port = SL_GPIO_PORT_G, .pin = 8 };
const sl_gpio_t pg9 = { .port = SL_GPIO_PORT_G, .pin = 9 };
const sl_gpio_t pg10 = { .port = SL_GPIO_PORT_G, .pin = 10 };
const sl_gpio_t pg11 = { .port = SL_GPIO_PORT_G, .pin = 11 };
const sl_gpio_t pg12 = { .port = SL_GPIO_PORT_G, .pin = 12 };
const sl_gpio_t pg13 = { .port = SL_GPIO_PORT_G, .pin = 13 };
const sl_gpio_t pg14 = { .port = SL_GPIO_PORT_G, .pin = 14 };
const sl_gpio_t pg15 = { .port = SL_GPIO_PORT_G, .pin = 15 };
const sl_gpio_t pg16 = { .port = SL_GPIO_PORT_G, .pin = 16 };
const sl_gpio_t pg17 = { .port = SL_GPIO_PORT_G, .pin = 17 };
const sl_gpio_t pg18 = { .port = SL_GPIO_PORT_G, .pin = 18 };
const sl_gpio_t pg19 = { .port = SL_GPIO_PORT_G, .pin = 19 };
const sl_gpio_t pg20 = { .port = SL_GPIO_PORT_G, .pin = 20 };
const sl_gpio_t pg21 = { .port = SL_GPIO_PORT_G, .pin = 21 };
const sl_gpio_t pg22 = { .port = SL_GPIO_PORT_G, .pin = 22 };
const sl_gpio_t pg23 = { .port = SL_GPIO_PORT_G, .pin = 23 };
const sl_gpio_t pg24 = { .port = SL_GPIO_PORT_G, .pin = 24 };
const sl_gpio_t pg25 = { .port = SL_GPIO_PORT_G, .pin = 25 };
const sl_gpio_t pg26 = { .port = SL_GPIO_PORT_G, .pin = 26 };
const sl_gpio_t pg27 = { .port = SL_GPIO_PORT_G, .pin = 27 };
const sl_gpio_t pg28 = { .port = SL_GPIO_PORT_G, .pin = 28 };
const sl_gpio_t pg29 = { .port = SL_GPIO_PORT_G, .pin = 29 };
const sl_gpio_t pg30 = { .port = SL_GPIO_PORT_G, .pin = 30 };
const sl_gpio_t pg31 = { .port = SL_GPIO_PORT_G, .pin = 31 };

const sl_gpio_t ph0 = { .port = SL_GPIO_PORT_H, .pin = 0 };
const sl_gpio_t ph1 = { .port = SL_GPIO_PORT_H, .pin = 1 };
const sl_gpio_t ph2 = { .port = SL_GPIO_PORT_H, .pin = 2 };
const sl_gpio_t ph3 = { .port = SL_GPIO_PORT_H, .pin = 3 };
const sl_gpio_t ph4 = { .port = SL_GPIO_PORT_H, .pin = 4 };
const sl_gpio_t ph5 = { .port = SL_GPIO_PORT_H, .pin = 5 };
const sl_gpio_t ph6 = { .port = SL_GPIO_PORT_H, .pin = 6 };
const sl_gpio_t ph7 = { .port = SL_GPIO_PORT_H, .pin = 7 };
const sl_gpio_t ph8 = { .port = SL_GPIO_PORT_H, .pin = 8 };
const sl_gpio_t ph9 = { .port = SL_GPIO_PORT_H, .pin = 9 };
const sl_gpio_t ph10 = { .port = SL_GPIO_PORT_H, .pin = 10 };
const sl_gpio_t ph11 = { .port = SL_GPIO_PORT_H, .pin = 11 };
const sl_gpio_t ph12 = { .port = SL_GPIO_PORT_H, .pin = 12 };
const sl_gpio_t ph13 = { .port = SL_GPIO_PORT_H, .pin = 13 };
const sl_gpio_t ph14 = { .port = SL_GPIO_PORT_H, .pin = 14 };
const sl_gpio_t ph15 = { .port = SL_GPIO_PORT_H, .pin = 15 };
const sl_gpio_t ph16 = { .port = SL_GPIO_PORT_H, .pin = 16 };
const sl_gpio_t ph17 = { .port = SL_GPIO_PORT_H, .pin = 17 };
const sl_gpio_t ph18 = { .port = SL_GPIO_PORT_H, .pin = 18 };
const sl_gpio_t ph19 = { .port = SL_GPIO_PORT_H, .pin = 19 };
const sl_gpio_t ph20 = { .port = SL_GPIO_PORT_H, .pin = 20 };
const sl_gpio_t ph21 = { .port = SL_GPIO_PORT_H, .pin = 21 };
const sl_gpio_t ph22 = { .port = SL_GPIO_PORT_H, .pin = 22 };
const sl_gpio_t ph23 = { .port = SL_GPIO_PORT_H, .pin = 23 };
const sl_gpio_t ph24 = { .port = SL_GPIO_PORT_H, .pin = 24 };
const sl_gpio_t ph25 = { .port = SL_GPIO_PORT_H, .pin = 25 };
const sl_gpio_t ph26 = { .port = SL_GPIO_PORT_H, .pin = 26 };
const sl_gpio_t ph27 = { .port = SL_GPIO_PORT_H, .pin = 27 };
const sl_gpio_t ph28 = { .port = SL_GPIO_PORT_H, .pin = 28 };
const sl_gpio_t ph29 = { .port = SL_GPIO_PORT_H, .pin = 29 };
const sl_gpio_t ph30 = { .port = SL_GPIO_PORT_H, .pin = 30 };
const sl_gpio_t ph31 = { .port = SL_GPIO_PORT_H, .pin = 31 };

const sl_gpio_t pi0 = { .port = SL_GPIO_PORT_I, .pin = 0 };
const sl_gpio_t pi1 = { .port = SL_GPIO_PORT_I, .pin = 1 };
const sl_gpio_t pi2 = { .port = SL_GPIO_PORT_I, .pin = 2 };
const sl_gpio_t pi3 = { .port = SL_GPIO_PORT_I, .pin = 3 };
const sl_gpio_t pi4 = { .port = SL_GPIO_PORT_I, .pin = 4 };
const sl_gpio_t pi5 = { .port = SL_GPIO_PORT_I, .pin = 5 };
const sl_gpio_t pi6 = { .port = SL_GPIO_PORT_I, .pin = 6 };
const sl_gpio_t pi7 = { .port = SL_GPIO_PORT_I, .pin = 7 };
const sl_gpio_t pi8 = { .port = SL_GPIO_PORT_I, .pin = 8 };
const sl_gpio_t pi9 = { .port = SL_GPIO_PORT_I, .pin = 9 };
const sl_gpio_t pi10 = { .port = SL_GPIO_PORT_I, .pin = 10 };
const sl_gpio_t pi11 = { .port = SL_GPIO_PORT_I, .pin = 11 };
const sl_gpio_t pi12 = { .port = SL_GPIO_PORT_I, .pin = 12 };
const sl_gpio_t pi13 = { .port = SL_GPIO_PORT_I, .pin = 13 };
const sl_gpio_t pi14 = { .port = SL_GPIO_PORT_I, .pin = 14 };
const sl_gpio_t pi15 = { .port = SL_GPIO_PORT_I, .pin = 15 };
const sl_gpio_t pi16 = { .port = SL_GPIO_PORT_I, .pin = 16 };
const sl_gpio_t pi17 = { .port = SL_GPIO_PORT_I, .pin = 17 };
const sl_gpio_t pi18 = { .port = SL_GPIO_PORT_I, .pin = 18 };
const sl_gpio_t pi19 = { .port = SL_GPIO_PORT_I, .pin = 19 };
const sl_gpio_t pi20 = { .port = SL_GPIO_PORT_I, .pin = 20 };
const sl_gpio_t pi21 = { .port = SL_GPIO_PORT_I, .pin = 21 };
const sl_gpio_t pi22 = { .port = SL_GPIO_PORT_I, .pin = 22 };
const sl_gpio_t pi23 = { .port = SL_GPIO_PORT_I, .pin = 23 };
const sl_gpio_t pi24 = { .port = SL_GPIO_PORT_I, .pin = 24 };
const sl_gpio_t pi25 = { .port = SL_GPIO_PORT_I, .pin = 25 };
const sl_gpio_t pi26 = { .port = SL_GPIO_PORT_I, .pin = 26 };
const sl_gpio_t pi27 = { .port = SL_GPIO_PORT_I, .pin = 27 };
const sl_gpio_t pi28 = { .port = SL_GPIO_PORT_I, .pin = 28 };
const sl_gpio_t pi29 = { .port = SL_GPIO_PORT_I, .pin = 29 };
const sl_gpio_t pi30 = { .port = SL_GPIO_PORT_I, .pin = 30 };
const sl_gpio_t pi31 = { .port = SL_GPIO_PORT_I, .pin = 31 };

const sl_gpio_t pj0 = { .port = SL_GPIO_PORT_J, .pin = 0 };
const sl_gpio_t pj1 = { .port = SL_GPIO_PORT_J, .pin = 1 };
const sl_gpio_t pj2 = { .port = SL_GPIO_PORT_J, .pin = 2 };
const sl_gpio_t pj3 = { .port = SL_GPIO_PORT_J, .pin = 3 };
const sl_gpio_t pj4 = { .port = SL_GPIO_PORT_J, .pin = 4 };
const sl_gpio_t pj5 = { .port = SL_GPIO_PORT_J, .pin = 5 };
const sl_gpio_t pj6 = { .port = SL_GPIO_PORT_J, .pin = 6 };
const sl_gpio_t pj7 = { .port = SL_GPIO_PORT_J, .pin = 7 };
const sl_gpio_t pj8 = { .port = SL_GPIO_PORT_J, .pin = 8 };
const sl_gpio_t pj9 = { .port = SL_GPIO_PORT_J, .pin = 9 };
const sl_gpio_t pj10 = { .port = SL_GPIO_PORT_J, .pin = 10 };
const sl_gpio_t pj11 = { .port = SL_GPIO_PORT_J, .pin = 11 };
const sl_gpio_t pj12 = { .port = SL_GPIO_PORT_J, .pin = 12 };
const sl_gpio_t pj13 = { .port = SL_GPIO_PORT_J, .pin = 13 };
const sl_gpio_t pj14 = { .port = SL_GPIO_PORT_J, .pin = 14 };
const sl_gpio_t pj15 = { .port = SL_GPIO_PORT_J, .pin = 15 };
const sl_gpio_t pj16 = { .port = SL_GPIO_PORT_J, .pin = 16 };
const sl_gpio_t pj17 = { .port = SL_GPIO_PORT_J, .pin = 17 };
const sl_gpio_t pj18 = { .port = SL_GPIO_PORT_J, .pin = 18 };
const sl_gpio_t pj19 = { .port = SL_GPIO_PORT_J, .pin = 19 };
const sl_gpio_t pj20 = { .port = SL_GPIO_PORT_J, .pin = 20 };
const sl_gpio_t pj21 = { .port = SL_GPIO_PORT_J, .pin = 21 };
const sl_gpio_t pj22 = { .port = SL_GPIO_PORT_J, .pin = 22 };
const sl_gpio_t pj23 = { .port = SL_GPIO_PORT_J, .pin = 23 };
const sl_gpio_t pj24 = { .port = SL_GPIO_PORT_J, .pin = 24 };
const sl_gpio_t pj25 = { .port = SL_GPIO_PORT_J, .pin = 25 };
const sl_gpio_t pj26 = { .port = SL_GPIO_PORT_J, .pin = 26 };
const sl_gpio_t pj27 = { .port = SL_GPIO_PORT_J, .pin = 27 };
const sl_gpio_t pj28 = { .port = SL_GPIO_PORT_J, .pin = 28 };
const sl_gpio_t pj29 = { .port = SL_GPIO_PORT_J, .pin = 29 };
const sl_gpio_t pj30 = { .port = SL_GPIO_PORT_J, .pin = 30 };
const sl_gpio_t pj31 = { .port = SL_GPIO_PORT_J, .pin = 31 };

const sl_gpio_t pk0 = { .port = SL_GPIO_PORT_K, .pin = 0 };
const sl_gpio_t pk1 = { .port = SL_GPIO_PORT_K, .pin = 1 };
const sl_gpio_t pk2 = { .port = SL_GPIO_PORT_K, .pin = 2 };
const sl_gpio_t pk3 = { .port = SL_GPIO_PORT_K, .pin = 3 };
const sl_gpio_t pk4 = { .port = SL_GPIO_PORT_K, .pin = 4 };
const sl_gpio_t pk5 = { .port = SL_GPIO_PORT_K, .pin = 5 };
const sl_gpio_t pk6 = { .port = SL_GPIO_PORT_K, .pin = 6 };
const sl_gpio_t pk7 = { .port = SL_GPIO_PORT_K, .pin = 7 };
const sl_gpio_t pk8 = { .port = SL_GPIO_PORT_K, .pin = 8 };
const sl_gpio_t pk9 = { .port = SL_GPIO_PORT_K, .pin = 9 };
const sl_gpio_t pk10 = { .port = SL_GPIO_PORT_K, .pin = 10 };
const sl_gpio_t pk11 = { .port = SL_GPIO_PORT_K, .pin = 11 };
const sl_gpio_t pk12 = { .port = SL_GPIO_PORT_K, .pin = 12 };
const sl_gpio_t pk13 = { .port = SL_GPIO_PORT_K, .pin = 13 };
const sl_gpio_t pk14 = { .port = SL_GPIO_PORT_K, .pin = 14 };
const sl_gpio_t pk15 = { .port = SL_GPIO_PORT_K, .pin = 15 };
const sl_gpio_t pk16 = { .port = SL_GPIO_PORT_K, .pin = 16 };
const sl_gpio_t pk17 = { .port = SL_GPIO_PORT_K, .pin = 17 };
const sl_gpio_t pk18 = { .port = SL_GPIO_PORT_K, .pin = 18 };
const sl_gpio_t pk19 = { .port = SL_GPIO_PORT_K, .pin = 19 };
const sl_gpio_t pk20 = { .port = SL_GPIO_PORT_K, .pin = 20 };
const sl_gpio_t pk21 = { .port = SL_GPIO_PORT_K, .pin = 21 };
const sl_gpio_t pk22 = { .port = SL_GPIO_PORT_K, .pin = 22 };
const sl_gpio_t pk23 = { .port = SL_GPIO_PORT_K, .pin = 23 };
const sl_gpio_t pk24 = { .port = SL_GPIO_PORT_K, .pin = 24 };
const sl_gpio_t pk25 = { .port = SL_GPIO_PORT_K, .pin = 25 };
const sl_gpio_t pk26 = { .port = SL_GPIO_PORT_K, .pin = 26 };
const sl_gpio_t pk27 = { .port = SL_GPIO_PORT_K, .pin = 27 };
const sl_gpio_t pk28 = { .port = SL_GPIO_PORT_K, .pin = 28 };
const sl_gpio_t pk29 = { .port = SL_GPIO_PORT_K, .pin = 29 };
const sl_gpio_t pk30 = { .port = SL_GPIO_PORT_K, .pin = 30 };
const sl_gpio_t pk31 = { .port = SL_GPIO_PORT_K, .pin = 31 };

/** @} (end addtogroup device_gpio) */
