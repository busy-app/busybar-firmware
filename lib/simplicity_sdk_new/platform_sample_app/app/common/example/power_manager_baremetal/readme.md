# Power Manager Bare Metal Application


This example project demonstrates use of the Power Manager module
in a bare metal application.


The application starts in awake mode (EM0). To cycle through desired energy modes, click Button 1.
 * 1 click -> EM1
 * 2 clicks -> EM2, etc.
Button 0 is used to confirm selection.


Once a mode is selected, the device will remain in that mode until the following conditions are met:
 * EM1, EM2: the sleep timer expires
 * EM3: a button is pushed
 * EM4: the device resets 

#### Note:
>* DCDC Boost devices will not enter EM4 mode. 
>* Enabling SL_POWER_MANAGER_INIT_EMU_EM2_DEBUG_ENABLE forces PD0B/PD0D power domain to remain active in EM2, ensuring debugger connectivity in both EM2 and EM3. Consequently, if either power domain is active, PD0E is also powered on, resulting in increased power consumption in EM2 and EM3.
