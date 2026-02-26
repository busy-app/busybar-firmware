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
    CliIntercomSpawnStatusTimeout,
} CliIntercomSpawnStatus;

#if defined(STM32U595xx)
#define CLI_INTERCOM_MASTER
#elif defined(SI917)
#define CLI_INTERCOM_SLAVE
#else
#error "Unsupported MCU"
#endif

// This service has no public API on f64.
#ifdef CLI_INTERCOM_MASTER

/**
 * @brief Spawns a shell on f64. Relays data and status information in both
 *        directions using the given pipe.
 * 
 * @param[in] cli_intercom         CliIntercom instance
 * @param[in] pipe                 Local PipeSide instance
 * @param[in] close_pipe_on_detach If true, call pipe_close() on the pipe when the intercom
 *                                 session ends. Set to true for programmatic callers that may be
 *                                 blocked in pipe operations on the other side of a temp pipe;
 *                                 set to false for interactive callers that pass an outer shell
 *                                 pipe that must survive the session.
 * 
 * @returns Spawn status: OK or error.
 */
CliIntercomSpawnStatus
    cli_intercom_spawn(CliIntercom* cli_intercom, PipeSide* pipe, bool close_pipe_on_detach);

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
