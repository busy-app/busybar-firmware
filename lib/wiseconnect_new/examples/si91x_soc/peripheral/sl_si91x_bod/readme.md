# Brown Out Detection (BOD)

## Table of Contents

- [BOD \[Brown Out Detection\]](#bod-brown-out-detection)
  - [Table of Contents](#table-of-contents)
  - [Purpose/Scope](#purposescope)
  - [Prerequisites/Setup Requirements](#prerequisitessetup-requirements)
    - [Hardware Requirements](#hardware-requirements)
    - [Software Requirements](#software-requirements)
    - [Setup Diagram](#setup-diagram)
  - [Getting Started](#getting-started)
  - [Application Build Environment](#application-build-environment)
    - [Application Configuration Parameters](#application-configuration-parameters)
  - [Test the Application](#test-the-application)

## Purpose/Scope

- This application demonstrates how to set up and use the Brown Out Detection (BOD) feature. It allows users to configure the threshold value for BOD and monitor the voltage levels on VMCU. When the voltage drops below the configured threshold, a BOD interrupt is triggered, and the application handles the event accordingly.

- **Black-out:** When Black-out is enabled, the reset line will be pulled down automatically if the battery voltage (Vbatt) falls below ~1.65V.

## Prerequisites/Setup Requirements

To use this application, the following hardware, software, and project setup are required.

### Hardware Requirements

- Windows PC
- SiWx917 Radio Board

### Software Requirements

- Simplicity Studio
- Serial console setup
  - For Serial Console setup instructions, refer to [link name](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#console-input-and-output).

### Setup Diagram

> ![Figure: setupdiagram](resources/readme/setupdiagram.png)

## Getting Started

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

- [Install Simplicity Studio](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#install-simplicity-studio)
- [Install WiSeConnect extension](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#install-the-wiseconnect-3-extension)
- [Connect your device to the computer](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#connect-siwx91x-to-computer)
- [Upgrade your connectivity firmware](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#update-siwx91x-connectivity-firmware)
- [Create a Studio project](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/using-the-simplicity-studio-ide#create-a-project)

For details on the project folder structure, see the [WiSeConnect Examples](https://docs.silabs.com/wiseconnect/latest/wiseconnect-examples/#example-folder-structure) page.

## Application Build Environment

### Application Configuration Parameters
  - The threshold and slot values can be configured in [`bod_example.h`](https://github.com/SiliconLabs/wiseconnect/blob/v4.0.1-content-for-docs/examples/si91x_soc/peripheral/sl_si91x_bod/bod_example.h). Alternatively, the slot value and blackout feature can be updated through the UC configuration.
    ```
    #define SL_BOD_DEFAULT_SLOT_VALUE 2  ///< Default BOD Slot value
    #define SL_BOD_DEFAULT_THRESHOLD 2.7f ///< Default BOD threshold value
    ```
  - Battery voltage ranges can be customized for improved battery status monitoring by modifying the configurations in [`sl_si91x_bod.h`](https://github.com/SiliconLabs/wiseconnect/blob/v4.0.1-content-for-docs/components/device/silabs/si91x/mcu/drivers/unified_api/inc/sl_si91x_bod.h`).

    ```
    #define SL_BOD_MAX_BATTERY_VOLTAGE 3.3f ///< Maximum battery voltage for Brown-Out Detector (BOD)
    #define SL_BOD_MIN_BATTERY_VOLTAGE 1.6f ///< Minimum battery voltage for Brown-Out Detector (BOD)
    ```

- Configure UC from the slcp component.

  ![Figure: bod_uc_screen](resources/uc_screen/bod_uc_screen.png)

- Open  the **sl_si91x_bod.slcp** project file, select **Software Component** tab and search for **BOD** in the search bar.
- You can use the configuration wizard to configure different parameters. The configuration screen is below, with options for the user to pick based on need.

  - **Enable BOD UC Configuration:** This needs to be enabled to apply these configurations in the application.
    - **Slot Value:** Sets how often the comparator checks the BOD value. In automatic mode, up to six voltage comparisons (cmp_[1-5]_en and button_wakeup_en) are performed in sequence during each slot interval. Adjust this option to control the comparison frequency and interval.
      - Slot value effects are only observable in low-power (sleep) modes. In active mode, power consumption differences are minimal. For noticeable results, test with higher slot values to further reduce sleep current.

    - **Enable Black-Out Monitor:** When enabled, the system will automatically reset if the VMCU voltage falls below 1.65V.

    - **Automatic Mode:** In this mode, up to six voltage comparisons can be performed consecutively within each slot interval. All enabled comparisons (cmp_[1-5]_en and button_wakeup_en) are checked in sequence during each slot.

    - **Manual Mode:** In manual mode, only a single comparison is performed repeatedly. You can select which comparison to use by setting the `manual_cmp_mux_sel` parameter and enabling the corresponding `cmp_en` signal.

> **Note**: For recommended settings, please refer the [recommendations guide](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-prog-recommended-settings/).

## Test the Application
- Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

  - Compile and run the application.
  - After successful program execution, the prints in serial console looks as shown below.

- Expected Results
   - If the user decreases the voltage less than the threshold voltage, then a BOD interrupt occurs.
  - The console output will appear as shown below. (Interrupt operations are not included in the Console Output section.)

  - Console Output:

    ![Figure: bod_result_console](resources/readme/bod_result_console.png)

  - When an interrupt occurs, the following behavior can be observed based on the configured threshold value:

    - When Blackout is enabled:

      ![Figure: bod_interrupt_result_console](resources/readme/bod_interrupt_result_console.png)
    
    - When Blackout is Disabled:

      ![Figure: bod_interrupt_result_console_DisableBlkot](resources/readme/bod_interrupt_result_console_DisableBlkot.png)

> **Note:** Interrupt handlers are implemented in the driver layer, and user callbacks are provided for custom code. If you want to write your own interrupt handler instead of using the default one, make the driver interrupt handler a weak handler. Then, copy the necessary code from the driver handler to your custom interrupt handler.
