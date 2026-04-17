/***************************************************************************//**
 * @file
 * @brief PA power conversion curves used by Silicon Labs PA power conversion
 *   functions.
 * @details This file contains the curves needed convert PA power levels to
 *   dBm powers.
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

#ifndef __PA_DBM_POWERSETTING_MAPPING_H_
#define __PA_DBM_POWERSETTING_MAPPING_H_

#ifdef __cplusplus
extern "C" {
#endif

#define RAIL_PA_CURVES_OFDM_CURVES_NUM_VALUES  (136U)
#define RAIL_PA_CURVES_OFDM_CURVES_STEP_DDBM   (1U)
#define RAIL_PA_CURVES_OFDM_MAX_POWER_DDBM     (310U)
#define RAIL_PA_CURVES_OFDM_MIN_POWER_DDBM     (175)
#define RAIL_PA_CURVES_OFDM_CURVES \
  {                                \
    0x21124 /* 175 decidBm */,     \
    0x21124 /* 176 decidBm */,     \
    0x21124 /* 177 decidBm */,     \
    0x21124 /* 178 decidBm */,     \
    0x21224 /* 179 decidBm */,     \
    0x21224 /* 180 decidBm */,     \
    0x21224 /* 181 decidBm */,     \
    0x21224 /* 182 decidBm */,     \
    0x21224 /* 183 decidBm */,     \
    0x21324 /* 184 decidBm */,     \
    0x21324 /* 185 decidBm */,     \
    0x21324 /* 186 decidBm */,     \
    0x21324 /* 187 decidBm */,     \
    0x21424 /* 188 decidBm */,     \
    0x21424 /* 189 decidBm */,     \
    0x21424 /* 190 decidBm */,     \
    0x21424 /* 191 decidBm */,     \
    0x21424 /* 192 decidBm */,     \
    0x21524 /* 193 decidBm */,     \
    0x21524 /* 194 decidBm */,     \
    0x21524 /* 195 decidBm */,     \
    0x21524 /* 196 decidBm */,     \
    0x21624 /* 197 decidBm */,     \
    0x21624 /* 198 decidBm */,     \
    0x21624 /* 199 decidBm */,     \
    0x21624 /* 200 decidBm */,     \
    0x21724 /* 201 decidBm */,     \
    0x21724 /* 202 decidBm */,     \
    0x21724 /* 203 decidBm */,     \
    0x21824 /* 204 decidBm */,     \
    0x21824 /* 205 decidBm */,     \
    0x21824 /* 206 decidBm */,     \
    0x21824 /* 207 decidBm */,     \
    0x21924 /* 208 decidBm */,     \
    0x21924 /* 209 decidBm */,     \
    0x21924 /* 210 decidBm */,     \
    0x21A24 /* 211 decidBm */,     \
    0x21A24 /* 212 decidBm */,     \
    0x21A24 /* 213 decidBm */,     \
    0x21B24 /* 214 decidBm */,     \
    0x21B24 /* 215 decidBm */,     \
    0x21B24 /* 216 decidBm */,     \
    0x21C24 /* 217 decidBm */,     \
    0x21C24 /* 218 decidBm */,     \
    0x21D24 /* 219 decidBm */,     \
    0x21D24 /* 220 decidBm */,     \
    0x21D24 /* 221 decidBm */,     \
    0x21E24 /* 222 decidBm */,     \
    0x21E24 /* 223 decidBm */,     \
    0x21E24 /* 224 decidBm */,     \
    0x21F24 /* 225 decidBm */,     \
    0x21F24 /* 226 decidBm */,     \
    0x21F25 /* 227 decidBm */,     \
    0x21F25 /* 228 decidBm */,     \
    0x21F25 /* 229 decidBm */,     \
    0x22025 /* 230 decidBm */,     \
    0x22025 /* 231 decidBm */,     \
    0x22026 /* 232 decidBm */,     \
    0x22026 /* 233 decidBm */,     \
    0x22026 /* 234 decidBm */,     \
    0x22027 /* 235 decidBm */,     \
    0x22027 /* 236 decidBm */,     \
    0x22028 /* 237 decidBm */,     \
    0x22028 /* 238 decidBm */,     \
    0x22029 /* 239 decidBm */,     \
    0x22029 /* 240 decidBm */,     \
    0x2202A /* 241 decidBm */,     \
    0x2202A /* 242 decidBm */,     \
    0x2202B /* 243 decidBm */,     \
    0x2202B /* 244 decidBm */,     \
    0x2202C /* 245 decidBm */,     \
    0x2202D /* 246 decidBm */,     \
    0x2202D /* 247 decidBm */,     \
    0x2202E /* 248 decidBm */,     \
    0x2202E /* 249 decidBm */,     \
    0x2202F /* 250 decidBm */,     \
    0x2202F /* 251 decidBm */,     \
    0x22030 /* 252 decidBm */,     \
    0x22031 /* 253 decidBm */,     \
    0x22031 /* 254 decidBm */,     \
    0x22032 /* 255 decidBm */,     \
    0x22033 /* 256 decidBm */,     \
    0x22034 /* 257 decidBm */,     \
    0x22034 /* 258 decidBm */,     \
    0x22035 /* 259 decidBm */,     \
    0x22036 /* 260 decidBm */,     \
    0x22037 /* 261 decidBm */,     \
    0x22037 /* 262 decidBm */,     \
    0x22038 /* 263 decidBm */,     \
    0x22039 /* 264 decidBm */,     \
    0x2203A /* 265 decidBm */,     \
    0x2203B /* 266 decidBm */,     \
    0x2203C /* 267 decidBm */,     \
    0x2203D /* 268 decidBm */,     \
    0x2203D /* 269 decidBm */,     \
    0x2203F /* 270 decidBm */,     \
    0x2203F /* 271 decidBm */,     \
    0x2213F /* 272 decidBm */,     \
    0x2213F /* 273 decidBm */,     \
    0x2223F /* 274 decidBm */,     \
    0x2233F /* 275 decidBm */,     \
    0x2243F /* 276 decidBm */,     \
    0x2243F /* 277 decidBm */,     \
    0x2253F /* 278 decidBm */,     \
    0x2263F /* 279 decidBm */,     \
    0x2273F /* 280 decidBm */,     \
    0x2283F /* 281 decidBm */,     \
    0x2293F /* 282 decidBm */,     \
    0x22A3F /* 283 decidBm */,     \
    0x22B3F /* 284 decidBm */,     \
    0x22C3F /* 285 decidBm */,     \
    0x22D3F /* 286 decidBm */,     \
    0x22D3F /* 287 decidBm */,     \
    0x22F3F /* 288 decidBm */,     \
    0x2303F /* 289 decidBm */,     \
    0x2323F /* 290 decidBm */,     \
    0x2333F /* 291 decidBm */,     \
    0x2353F /* 292 decidBm */,     \
    0x2363F /* 293 decidBm */,     \
    0x2383F /* 294 decidBm */,     \
    0x2393F /* 295 decidBm */,     \
    0x23B3F /* 296 decidBm */,     \
    0x23D3F /* 297 decidBm */,     \
    0x23F3F /* 298 decidBm */,     \
    0x24732 /* 299 decidBm */,     \
    0x24932 /* 300 decidBm */,     \
    0x24B32 /* 301 decidBm */,     \
    0x24E32 /* 302 decidBm */,     \
    0x25132 /* 303 decidBm */,     \
    0x25432 /* 304 decidBm */,     \
    0x25832 /* 305 decidBm */,     \
    0x25B32 /* 306 decidBm */,     \
    0x25E32 /* 307 decidBm */,     \
    0x26332 /* 308 decidBm */,     \
    0x26732 /* 309 decidBm */,     \
    0x26E32 /* 310 decidBm */,     \
  }

#define RAIL_PA_CURVES_SUBGIG_CURVES_NUM_VALUES  (146U)
#define RAIL_PA_CURVES_SUBGIG_CURVES_STEP_DDBM   (1U)
#define RAIL_PA_CURVES_SUBGIG_MAX_POWER_DDBM     (324U)
#define RAIL_PA_CURVES_SUBGIG_MIN_POWER_DDBM     (179)
#define RAIL_PA_CURVES_SUBGIG_CURVES \
  {                                  \
    0x41F0018 /* 179 decidBm */,     \
    0x41F0018 /* 180 decidBm */,     \
    0x41F0019 /* 181 decidBm */,     \
    0x41F0019 /* 182 decidBm */,     \
    0x41F0019 /* 183 decidBm */,     \
    0x41F001A /* 184 decidBm */,     \
    0x41F001A /* 185 decidBm */,     \
    0x41F001A /* 186 decidBm */,     \
    0x41F001B /* 187 decidBm */,     \
    0x41F001B /* 188 decidBm */,     \
    0x41F001B /* 189 decidBm */,     \
    0x41F001B /* 190 decidBm */,     \
    0x41F001C /* 191 decidBm */,     \
    0x41F001C /* 192 decidBm */,     \
    0x41F001C /* 193 decidBm */,     \
    0x41F001C /* 194 decidBm */,     \
    0x41F001D /* 195 decidBm */,     \
    0x41F001D /* 196 decidBm */,     \
    0x41F001E /* 197 decidBm */,     \
    0x41F001E /* 198 decidBm */,     \
    0x41F001E /* 199 decidBm */,     \
    0x41F001F /* 200 decidBm */,     \
    0x41F001F /* 201 decidBm */,     \
    0x41F001F /* 202 decidBm */,     \
    0x41F0100 /* 203 decidBm */,     \
    0x41F0100 /* 204 decidBm */,     \
    0x41F0101 /* 205 decidBm */,     \
    0x41F0101 /* 206 decidBm */,     \
    0x41F0101 /* 207 decidBm */,     \
    0x41F0102 /* 208 decidBm */,     \
    0x41F0102 /* 209 decidBm */,     \
    0x41F0103 /* 210 decidBm */,     \
    0x41F0103 /* 211 decidBm */,     \
    0x41F0104 /* 212 decidBm */,     \
    0x41F0104 /* 213 decidBm */,     \
    0x41F0104 /* 214 decidBm */,     \
    0x41F0105 /* 215 decidBm */,     \
    0x41F0105 /* 216 decidBm */,     \
    0x41F0106 /* 217 decidBm */,     \
    0x41F0106 /* 218 decidBm */,     \
    0x41F0107 /* 219 decidBm */,     \
    0x41F0107 /* 220 decidBm */,     \
    0x41F0107 /* 221 decidBm */,     \
    0x41F0108 /* 222 decidBm */,     \
    0x41F0108 /* 223 decidBm */,     \
    0x41F0108 /* 224 decidBm */,     \
    0x41F0109 /* 225 decidBm */,     \
    0x41F0109 /* 226 decidBm */,     \
    0x41F010A /* 227 decidBm */,     \
    0x41F010B /* 228 decidBm */,     \
    0x41F010B /* 229 decidBm */,     \
    0x41F010C /* 230 decidBm */,     \
    0x41F010C /* 231 decidBm */,     \
    0x41F010C /* 232 decidBm */,     \
    0x41F010D /* 233 decidBm */,     \
    0x41F010D /* 234 decidBm */,     \
    0x41F010E /* 235 decidBm */,     \
    0x41F010F /* 236 decidBm */,     \
    0x41F010F /* 237 decidBm */,     \
    0x41F010F /* 238 decidBm */,     \
    0x41F0110 /* 239 decidBm */,     \
    0x41F0110 /* 240 decidBm */,     \
    0x41F0110 /* 241 decidBm */,     \
    0x41F0111 /* 242 decidBm */,     \
    0x41F0111 /* 243 decidBm */,     \
    0x41F0112 /* 244 decidBm */,     \
    0x41F0113 /* 245 decidBm */,     \
    0x41F0113 /* 246 decidBm */,     \
    0x41F0114 /* 247 decidBm */,     \
    0x41F0115 /* 248 decidBm */,     \
    0x41F0115 /* 249 decidBm */,     \
    0x41F0116 /* 250 decidBm */,     \
    0x41F0117 /* 251 decidBm */,     \
    0x41F0117 /* 252 decidBm */,     \
    0x41F0118 /* 253 decidBm */,     \
    0x41F0118 /* 254 decidBm */,     \
    0x41F0119 /* 255 decidBm */,     \
    0x41F011A /* 256 decidBm */,     \
    0x41F011A /* 257 decidBm */,     \
    0x41F011B /* 258 decidBm */,     \
    0x41F011B /* 259 decidBm */,     \
    0x41F011C /* 260 decidBm */,     \
    0x41F011D /* 261 decidBm */,     \
    0x41F011E /* 262 decidBm */,     \
    0x41F011F /* 263 decidBm */,     \
    0x41F0201 /* 264 decidBm */,     \
    0x41F0203 /* 265 decidBm */,     \
    0x41F0205 /* 266 decidBm */,     \
    0x41F0207 /* 267 decidBm */,     \
    0x41F0209 /* 268 decidBm */,     \
    0x41F020B /* 269 decidBm */,     \
    0x41F020D /* 270 decidBm */,     \
    0x41F020F /* 271 decidBm */,     \
    0x41F0211 /* 272 decidBm */,     \
    0x41F0213 /* 273 decidBm */,     \
    0x41F0217 /* 274 decidBm */,     \
    0x41F0217 /* 275 decidBm */,     \
    0x41F021B /* 276 decidBm */,     \
    0x41F021D /* 277 decidBm */,     \
    0x41F021F /* 278 decidBm */,     \
    0x41F021F /* 279 decidBm */,     \
    0x41F021F /* 280 decidBm */,     \
    0x40B0104 /* 281 decidBm */,     \
    0x40B0104 /* 282 decidBm */,     \
    0x40B0104 /* 283 decidBm */,     \
    0x40B0104 /* 284 decidBm */,     \
    0x40B0105 /* 285 decidBm */,     \
    0x40B0106 /* 286 decidBm */,     \
    0x40B0106 /* 287 decidBm */,     \
    0x40B0107 /* 288 decidBm */,     \
    0x40B0107 /* 289 decidBm */,     \
    0x40B0108 /* 290 decidBm */,     \
    0x40B0109 /* 291 decidBm */,     \
    0x40B010A /* 292 decidBm */,     \
    0x40B010B /* 293 decidBm */,     \
    0x40B010B /* 294 decidBm */,     \
    0x40B010C /* 295 decidBm */,     \
    0x40B010D /* 296 decidBm */,     \
    0x40B010E /* 297 decidBm */,     \
    0x40B010F /* 298 decidBm */,     \
    0x40B010F /* 299 decidBm */,     \
    0x40B0111 /* 300 decidBm */,     \
    0x40B0111 /* 301 decidBm */,     \
    0x40B0112 /* 302 decidBm */,     \
    0x40B0113 /* 303 decidBm */,     \
    0x40B0114 /* 304 decidBm */,     \
    0x40B0115 /* 305 decidBm */,     \
    0x40B0117 /* 306 decidBm */,     \
    0x40B0117 /* 307 decidBm */,     \
    0x40B0118 /* 308 decidBm */,     \
    0x40B011A /* 309 decidBm */,     \
    0x40B011B /* 310 decidBm */,     \
    0x40B011D /* 311 decidBm */,     \
    0x40B011F /* 312 decidBm */,     \
    0x40B0203 /* 313 decidBm */,     \
    0x40B0207 /* 314 decidBm */,     \
    0x40B020B /* 315 decidBm */,     \
    0x40B020F /* 316 decidBm */,     \
    0x40B0215 /* 317 decidBm */,     \
    0x40B0219 /* 318 decidBm */,     \
    0x40B021F /* 319 decidBm */,     \
    0x4030105 /* 320 decidBm */,     \
    0x4030107 /* 321 decidBm */,     \
    0x4030108 /* 322 decidBm */,     \
    0x403010A /* 323 decidBm */,     \
    0x403010C /* 324 decidBm */,     \
  }

#endif
// *INDENT-OFF*

#ifdef __cplusplus
}
#endif
