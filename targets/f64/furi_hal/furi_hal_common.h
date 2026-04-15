#pragma once

#ifndef FURI_HAL_NODISCARD
/* C++17 and later */
#if defined(__cplusplus) && (__cplusplus >= 201703L)
#define FURI_HAL_NODISCARD [[nodiscard]]

/* C23 and later */
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L)
#define FURI_HAL_NODISCARD [[nodiscard]]

/* GCC / Clang */
#elif defined(__GNUC__) || defined(__clang__)
#define FURI_HAL_NODISCARD __attribute__((warn_unused_result))

/* fallback */
#else
#define FURI_HAL_NODISCARD
#endif
#endif
