/**
 * @file cli_intercom.h
 * Lets f20 spawn a CLI session on f64 and talk to it over intercom.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <containers/pipe.h>

#define RECORD_CLI_INTERCOM "cli_intercom"

typedef struct CliIntercom CliIntercom;

typedef enum {
    CliIntercomSpawnStatusOk,
    CliIntercomSpawnStatusChannelTaken, //<! Shell is already spawned and hasn't exited yet
} CliIntercomSpawnStatus;

#if defined(STM32U595xx)
#define TARGET_F20
#elif defined(SI917)
#define TARGET_F64
#else
#error "Unsupported MCU"
#endif

// This service has no public API on f64.
#ifdef TARGET_F20

/**
 * @brief Spawns a shell on f64. Relays data and status information in both
 *        directions using the given pipe.
 * 
 * @param[in] cli_intercom CliIntercom instance
 * @param[in] pipe         Local PipeSide instance
 * 
 * @returns Spawn status: OK or error.
 */
CliIntercomSpawnStatus cli_intercom_spawn(CliIntercom* cli_intercom, PipeSide* pipe);

/**
 * @brief Blocks until the shell on f64 exits.
 * 
 * @param[in] cli_intercom CliIntercom instance
 */
void cli_intercom_join(CliIntercom* cli_intercom);

#endif

#ifdef __cplusplus
}
#endif
