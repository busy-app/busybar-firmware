# Wi-Fi - Calibration App

- [Wi-Fi - Calibration App](#wi-fi---calibration-app)
  - [Purpose/Scope](#purposescope)
  - [Prerequisites/Setup Requirements](#prerequisitessetup-requirements)
    - [Hardware Requirements](#hardware-requirements)
    - [Software Requirements](#software-requirements)
    - [Setup Diagram](#setup-diagram)
  - [Getting Started](#getting-started)
  - [Application Build Environment](#application-build-environment)
  - [Test the Application](#test-the-application)
    - [Gain Offset Correction](#gain-offset-correction)
      - [Update XO Ctune and Gain Offset](#update-xo-ctune-and-gain-offset)
    - [Spectrum Analyzer Settings](#spectrum-analyzer-settings)
    - [Acronyms and Abbreviations](#acronyms-and-abbreviations)

## Purpose/Scope

This application demonstrates the procedure to calibrate the carrier frequency offset, Tx gain offset, and a few more parameters and update them to the Flash/Efuse. Customers need to calibrate these two parameters on their platforms after chip integration.

## Prerequisites/Setup Requirements

### Hardware Requirements

- Windows PC
- SiWx91x Wi-Fi Evaluation Kit. The SiWx91x supports multiple operating modes. See [Operating Modes](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-driver/sl-si91-x-constants#sl-si91x-operation-mode-t) for details.
- **SoC Mode**:
  - Standalone
    - BRD4002A Wireless Pro Kit Mainboard [SI-MB4002A]
    - Radio Boards 
  	  - BRD4338A [SiWx917-RB4338A]
  	  - BRD4343A [SiWx917-RB4343A]
  - Kits
  	- SiWx917 Pro Kit [Si917-PK6031A](https://www.silabs.com/development-tools/wireless/wi-fi/siwx917-pro-kit?tab=overview)
  	- SiWx917 Pro Kit [Si917-PK6032A]
    - SiWx917 AC1 Module Explorer Kit (BRD2708A)
  	
- **NCP Mode**:
  - Standalone
    - BRD4002A Wireless Pro Kit Mainboard [SI-MB4002A]
    - EFR32xG24 Wireless 2.4 GHz +10 dBm Radio Board [xG24-RB4186C](https://www.silabs.com/development-tools/wireless/xg24-rb4186c-efr32xg24-wireless-gecko-radio-board?tab=overview)
    - NCP Expansion Kit with NCP Radio Boards
      - (BRD4346A + BRD8045A) [SiWx917-EB4346A]
      - (BRD4357A + BRD8045A) [SiWx917-EB4357A]
  - Kits
  	- EFR32xG24 Pro Kit +10 dBm [xG24-PK6009A](https://www.silabs.com/development-tools/wireless/efr32xg24-pro-kit-10-dbm?tab=overview)
  - Spectrum Analyzer
  - RF Cable connects between EVK and Spectrum Analyzer.
  - Interface and Host MCU Supported
    - SPI - EFR32 
    - UART - EFR32

### Software Requirements

- Simplicity Studio

### Setup Diagram

![Figure: Setup Diagram SoC Mode for Calibration Example](resources/readme/calibration_soc_ncp.png)

## Getting Started

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

- [Install Simplicity Studio](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#install-simplicity-studio)
- [Install WiSeConnect extension](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#install-the-wi-se-connect-extension)
- [Connect your device to the computer](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#connect-si-wx91x-to-computer)
- [Upgrade your connectivity firmware ](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#update-si-wx91x-connectivity-firmware)
- [Create a Studio project ](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-developing-for-silabs-hosts/#create-a-project)

For details on the project folder structure, see the [WiSeConnect Examples](https://docs.silabs.com/wiseconnect/latest/wiseconnect-examples/#example-folder-structure) page.

## Application Build Environment

The application can be configured to suit user requirements and development environment. Read through the following sections and make any changes needed. 

1. Ensure that SL_SI91X_EXT_FEAT_DISABLE_XTAL_CORRECTION (BIT(18)) is enabled in the sl_wifi_device_configuration_t.sl_wifi_system_boot_configuration_t.ext_custom_feature_bit_map structure variable. This setting will disable the automatic compensation for frequency offsets, thereby ensuring error-free calibration.

2. Configure the following parameters in **app.c** to test calibration app as per requirements:

```c
  sl_wifi_data_rate_t rate = SL_WIFI_DATA_RATE_1;
  sl_wifi_request_tx_test_info_t tx_test_info = {
    .enable      = 1,          // Enable/disable tx test mode
    .power       = 18,         // Tx RF power in the range [2:18] dBm
    .rate        = rate,      // WLAN data rate of 6 Mbps
    .length      = 30,        // Tx packet length in the range [24:1500] bytes in burst mode 
    .mode        = 0,         // Selects burst mode or continuous mode
    .channel     = 1,         // Channel number in 2.4 or 5 GHz
    // Other configurable parameters
  }
```

    The different values of mode:
      0 - Burst Mode
      1 - Continuous Mode
      2 - Continuous Wave Mode (non modulation) in DC mode
      3 - Continuous Wave Mode (non modulation) in single tone mode (center frequency -2.5 MHz)
      4 - Continuous Wave Mode (non modulation) in single tone mode (center frequency + 5MHz)

> **Note**: For recommended settings, please refer the [recommendations guide](https://docs.silabs.com/wiseconnect/latest/wiseconnect-developers-guide-prog-recommended-settings/).

## Test the Application

Refer to the instructions [here](https://docs.silabs.com/wiseconnect/latest/wiseconnect-getting-started/) to:

- Build the application.
- Flash, run, and debug the application.

1. Make connections as per setup diagram (above) and set the appropriate settings on the Spectrum Analyzer by using the steps mentioned in the **Spectrum Analyzer Settings** section below.

2. After the program gets executed, the SiWx91x device will start the transmit test with the given configuration.

3. Refer to the following image which shows when the SiWx91x device transmits packets in Burst mode with different Tx power and different transmission rates in channel 1 with length 30 bytes.

 ```sh
    sl_wifi_data_rate_t rate = SL_WIFI_DATA_RATE_1;
    sl_wifi_request_tx_test_info_t tx_test_info = {
      .enable      = 1,          // Enable/disable tx test mode
      .power       = 18,         // Tx RF power in the range [2:18] dBm
      .rate        = rate,      // WLAN data rate of 6 Mbps
      .length      = 30,        // Tx packet length in the range [24:1500] bytes in burst mode and TX packet length in the range  [24:260 ] in continuous mode 
      .mode        = 0,         // Selects burst mode or continuous mode
      .channel     = 1,         // Channel number in 2.4 or 5 GHz
      // Other configurable parameters
  }
   ```

4. Observe the Avg Freq Error (highlighted) on the screen and now try to adjust the frequency offset by using CLI commands with serial terminal (Docklight or Teraterm).
   
    ![Avg Freq Error (highlighted) on the screen](resources/readme/image_c7.png)

    ### Frequency Offset Correction

    Frequency offset correction will be done by using the sl_si91x_frequency_offset command. This command is used during the RF calibration process and requires PER mode transmissions to be initiated prior. This command sends freq_offset (deviation) as observed on the signal analyzer against the expected channel frequency.
 
    Frequency offset correction command syntax:  
      > `sl_freq_offset = freq_offset_in_khz <CR><LF>`  
      >
      > Here freq_offset_in_khz means Frequency deviation in KHz
 
    Examples:
 
      If an error of 10 KHz is observed on the instrument, the command should be used as follows:
      > `sl_freq_offset=10<CR><LF>`
 
      If an error of -10 KHz is observed on the instrument, the command should be used as follows:
      > `sl_freq_offset=-10<CR><LF>`
 
  > **Note:** You can use the above command any number of times until the device gets tuned to the desired frequency offset.


  Open the serial terminal (Docklight/TeraTerm tool) and enter the following commands. You can provide input to correct frequency offset by sending the commands on console. This should lead towards a correction in the frequency offset as observed earlier and will repeat till the error is within the tolerance limits (+/- 2 KHz tolerance limits).

  See the picture below after frequency offset correction.

  ![After frequency offset correction](resources/readme/image_c9.png)

  > **Note:** freq_offset_in_khz can be either +ve or -ve. When you enter the freq offset as observed on signal analyzer (+ve/-ve), a small freq offset correction is done. You need to iterate this till the freq offset is within tolerance limits.

  The corrected value of frequency offset is stored within a hardware register given to the sl_freq_offset command. For the correction to reflect upon reset, it is required that we copy the corrected value to flash. The value can be written to flash using the sl_calib_write command. The command should be used as follows: sl_calib_write = 1,2,0,0,0. Further details regarding the syntax of the sl_calib_write command are provided in the subsequent sections.

### Gain Offset Correction

#### Update XO Ctune and Gain Offset

Using sl_calib_write command, the calibrated XO Ctune and calculated gain offset can be updated to target memory (Flash/Efuse).
```
 Prototype :
 sl_calib_write= <target>,<flags>,<gain_offset_low>,<gain_offset_mid>,<gain_offset_high>,<xo_ctune>,<gain_offset_ch14><CR><LF>
 ```

**Structure Members**

   | Parameter  | Description |                 |          |
   | :---       | :---        | :---            | :---     |
   | target     | Value       | Macro         | Description |
   |            |   0         | BURN_INTO_EFUSE | Burns calibration data to EFuse |
   |            |   1         | BURN_INTO_FLASH | Burns calibration data to Flash |
   | flags      |  BIT        | Macro                  | Description |
   |            |   0         |                       |  Reserved for future use
   |            |   1         | BURN_FREQ_OFFSET      | 1 - Update XO Ctune to calibration data 
   |||| 0 - Skip XO Ctune update
   |            |   2         | SW_XO_CTUNE_VALID     | 1 - Use XO Ctune provided as argument to update calibration data  
   |||| 0 - Use XO Ctune value as read from hardware register
   |            |   3         | BURN_XO_FAST_DISABLE  |     Used to apply patch for cold temperature issue (host interface detection) observed on CC0/CC1 modules. \ref appendix
   |            |   4         |  BURN_GAIN_OFFSET_LOW  | 1 - Update gain offset for low sub-band (2 GHz)  
   |||| 0 - Skip low sub-band gain-offset update
   |            |   5         |  BURN_GAIN_OFFSET_MID  | 1 - Update gain offset for mid sub-band (2 GHz)  
   |||| 0 - Skip mid sub-band gain-offset update
   |            |   6         |  BURN_GAIN_OFFSET_HIGH | 1 - Update gain offset for high sub-band (2 GHz)  
   |||| 0 - Skip high sub-band gain-offset update
   |            |   8         |  ENABLE_DPD_CALIB      | 1 - Collect dpd coefficients data   
   |||| 0 - Skip dpd coeffiecients calibration
   |            |   9         |  BURN_DPD_COEFFICIENTS | 1 - Burn dpd coefficients data   
   |||| 0 - Skip dpd coeffiecients calibration
   |            |   10         |  BURN_GAIN_OFFSET_CHANNEL-14  | 1 - Update gain offset for low sub-band (2 GHz) 
   |||| 0 - Skip channel-14 sub-band gain-offset update
   |            |   31-4      |                        | Reserved for future use
   |gain_offset_low | gain_offset as observed in dBm in channel-1 | |
   |gain_offset_mid | gain_offset as observed in dBm in channel-6 | |
   |gain_offset_high| gain_offset as observed in dBm in channel-11 | |
   | xo_ctune   | This field allows the user to directly update xo_ctune value to calibration data, bypassing the freq offset loop. Valid only when BURN_FREQ_OFFSET & SW_XO_CTUNE_VALID of flags is set. |   | The range of xo_ctune is [0, 255], and the typical value is 80 |
   |
   |gain_offset_ch14 | gain_offset as observed in dBm in channel-14 | |   

> **Note:** For SiWx917, you need to calibrate gain-offset for low sub-band (channel-1), mid sub-band (channel-6), and high sub-band (channel-11). You also need to input the three gain-offsets to this API and set the corresponding flags to validate it. 

> **Precondition:** 
sl_freq_offset command needs to be called before this command when xo ctune value from hardware register is to be used. 

Gain offset can be calculated using the following equation :

```text
gain_offset = observed_power_level + cable_loss - configured_power_level
```

Example :

  gain_offset = 14.3 + 1.7 (assuming) - 18 = -2 dBm

The corresponding value to be passed via the command would be -4 (2*calculated value).

Example usage:

sl_calib_write=1,16,-4,0,0,0,0
```
Target = 1 (data to be written to flash)
Flags  = BIT(4) => 2^4 = 16 (BURN_GAIN_OFFSET_LOW => Gain offset in channel 1 is to be updated in the flash)
gain_offset_low  = -4 (Updates the channel 1 gain offset such that the average tx power in channel 1 is incremented by 2 dbm)
gain_offset_mid  = 0 (Not updating the gain offsets of channel 6, the corresponding bit is disabled in the flags)
gain_offset_high = 0 (Not updating the gain offsets of channel 11, the corresponding bit is disabled in the flags)
xo_ctune         = 0 (Not updating the xo_ctune, the corresponding bit is disabled in the flags)
gain_offset_ch14 = 0 (Not updating the gain offsets of channel 14, the corresponding bit is disabled in the flags)
```

For 2 GHz band, the gain offset has to calibrated for three channels: viz. channel-1, channel-6, channel-11. After the three gain offsets are calculated, you can write these to Flash using the `sl_calib_write` command.

> NOTE : 
> The gain_offset can be negative but not a floating value.
> Once the frequency offset is corrected after multiple tries, sl_calib_write commands has to be given once for all to write the values to flash.

The application reads the updated calibration values for verification by using `sl_calib_read` command and displays.

### Spectrum Analyzer Settings

**Below are the necessary settings to see Polar Graph Spectrum Analyzer settings**

1. Frequency channel → center frequency→ 2412 MHz for channel1.

2. SpanX scale → span→ 50 MHz

3. Mode → WLAN → Mode setup → Radio Std → 802.11a/b/g → 802.11 b/g

4. Trigger → RF Burst

The frequency error section shows the error that needs to be adjusted. You can adjust the frequency error by using the `sl_si91x_frequency_offset` and `sl_calib_write` commands.

### Acronyms and Abbreviations

| Acronym   | Description |
| :----  | :----  | 
| TX      | Transmit       |
|RX | Receive |
|RF | Radio Frequency |
|WLAN | Wireless Local Area Network |
|XO | Crystal Oscillator |
|Ctune| Captune |
|Q7 | Single band SiWx91x EVK |
|A7 | Dual band SiWx91x EVK |

  ![Calibration Output](resources/readme/output.png)
