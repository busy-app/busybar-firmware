#ifndef SL_SI91X_WDT_MANAGER_H
#define SL_SI91X_WDT_MANAGER_H

/***************************************************************************/ /**
 * Initialize WDT Manager example
 * 
 * This function initializes the WDT manager example with LED toggling functionality.
 * The WDT manager is automatically initialized and started by the WDT Manager
 * component to ensure system reliability.
 ******************************************************************************/
void wdt_manager_init(void);

/***************************************************************************/ /**
 * WDT Manager process action function
 * 
 * This function handles the LED toggling based on timer timeout events.
 * It should be called continuously in the main loop.
 ******************************************************************************/
void wdt_manager_process_action(void);

#endif // SL_SI91X_WDT_MANAGER_H
