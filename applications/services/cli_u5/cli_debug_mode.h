#pragma once

/**
 * Re-evaluates the FuriHalNvmFlagDebug state and registers or unregisters
 * the debug-only CLI commands (gpio, factory_reset, otp) accordingly.
 * Called after changing the debug NVM flag.
 */
void cli_command_update_debug_mode(void);
