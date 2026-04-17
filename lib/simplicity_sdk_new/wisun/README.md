<table border="0">
  <tr>
    <td align="left" valign="middle">
    <h1>Wi-SUN FAN Stack</h1>
  </td>
  <td align="left" valign="middle">
    <a href="https://wi-sun.org/">
      <img src="https://wi-sun.org/wp-content/uploads/Wi-SUN-Main-Logo.png"  title="Silicon Labs Gecko and Wireless Gecko MCUs" alt="EFM32 32-bit Microcontrollers" width="300"/>
    </a>
  </td>
  </tr>
</table>

# Silicon Labs Wi-SUN FAN Stack

This repository contains Silicon Labs Wi-SUN FAN stack and the Wi-SUN CLI application source code. It allows a user to recompile the Wi-SUN stack and the Wi-SUN CLI application and start a Wi-SUN application development. The document below describes the steps to get started with the Wi-SUN resources and Simplicity Studio 5.

To go through the Wi-SUN CLI application and Wi-SUN border router demonstration, refer to the [**dedicated Wi-SUN Application Examples repository**](https://github.com/SiliconLabs/proprietary_wisun_applications).

For Silicon Labs Technical Support, create a ticket on [**Silicon Labs Support Platform**](https://siliconlabs.force.com/s/contactsupport).

## Content

- **doxygen**: contains the Doxygen documentation resources. A generated HTML Doxygen documentation is available in the [**Wi-SUN Application Examples repository**](https://github.com/SiliconLabs/proprietary_wisun_applications).
- **gsdk-integration**: contains the resources and script to add the Wi-SUN stack and Wi-SUN CLI application to Simplicity Studio 5.
- **wisun**: contains the Wi-SUN FAN stack source code.
- **wisuncli**: contains the Wi-SUN CLI application source code.

## Build the Wi-SUN CLI Application from the Source Code

### Install the Required Software

- Install [**Simplicity Studio 5**](https://www.silabs.com/products/development-tools/software/simplicity-studio/simplicity-studio-5) and [**Git**](https://git-scm.com/) on your machine
- Open Simplicity Studio 5
- When prompted by the Installation Manager, select **"Install by technology type"**
- In the **"Select Technology Type"** panel, select **"Proprietary"** and click on **"Next"**
- In the **"Package Installation Options"** panel, directly click on **"Next"**
- Accept the License Agreement
- Simplicity Studio downloads the Proprietary Gecko SDK (takes several minutes)
- When done, click **"Close"**
- Click **"Restart"**
- Once Simplicity Studio is reopened, close it again

### Retrieve and Include the Wi-SUN Stack and Applications to the Gecko SDK

- Under the path *SiliconLabs\SimplicityStudio\v5\developer\sdks\gecko_sdk_suite\v3.0\protocol* (default path), clone the [**Wi-SUN stack GitHub repository**](https://github.com/SiliconLabs/proprietary_wisun_stack) using the command below:

`git clone https://github.com/SiliconLabs/proprietary_wisun_stack.git wisun`

The command creates a **"wisun"** folder containing the Wi-SUN stack and Wi-SUN CLI application.
- Under *wisun/gsdk-integration*, run *gsdk-setup.sh*

This script adds the Wi-SUN stack and CLI application to the Gecko SDK.

> **The script has to be reapplied on each Gecko SDK update** to continue having access to the Wi-SUN stack and CLI integration in Simplicity Studio 5.

### Create a Wi-SUN CLI Application in your Workspace

- Restart Simplicity Studio
- In the **"Launcher"** context, select the board you want to use as a Wi-SUN CLI router in the **"Debug Adapters"** panel
- In the main window, make sure the Gecko SDK v3.0.0 is selected in the **"Preferred SDK"** field
- By clicking on the arrow to extend the SDK list, make sure Gecko SDK v3.0 lists the Wi-SUN component, i.e., **"Wi-SUN x.x.x.x"**
- In the main window, click on **"Example Projects"**
- A list of supported examples appears
- Find the **"Wi-SUN CLI"** example in the list and click on **"CREATE"**
- In the **"Project Configuration"** window, click on **"FINISH"**

Simplicity Studio loads the Wi-SUN CLI example in the workspace.

### Build and Load the Wi-SUN CLI Application

- Connect a WSTK board with the appropriate EFR32xG12 radio board
- Select the **"wisun_cli"** project (default name)
- Click on **"Run"** in the toolbar
- Select **"Debug"**
- Make sure the build is successful
- Once in the **"Debug"** view, click on the **"Run"** in the toolbar
- Select **"Resume"**

The Wi-SUN CLI application is now running. You can use this application as a starting point for your Wi-SUN developments.
