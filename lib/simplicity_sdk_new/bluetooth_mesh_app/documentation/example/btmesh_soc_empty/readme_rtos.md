# Bluetooth Mesh - SoC Empty RTOS

This example demonstrates the bare minimum needed for a Bluetooth mesh C application that supports Over-the-Air Device Firmware Upgrading (OTA DFU). The project is based on a Real Time Operating System (Micrium OS or FreeRTOS according to your choice). The application starts Unprovisioned Device Beaconing after boot, and waits to be provisioned to a Mesh Network. This example can be used as a starting point for an SoC project and it can be customized by adding new components using the Project Configurator, or by modifying the application. This example requires one of the Internal Storage Bootloader (single image) variants, depending on device memory.

## Getting Started

To learn Bluetooth mesh technology basics, see [Bluetooth Mesh Network - An Introduction for Developers](https://www.bluetooth.com/wp-content/uploads/2019/03/Mesh-Technology-Overview.pdf).

To get started with Silicon Labs Bluetooth Mesh and Simplicity Studio, see [QSG176: Bluetooth Mesh SDK v2.x Quick Start Guide](https://www.silabs.com/documents/public/quick-start-guides/qsg176-bluetooth-mesh-sdk-v2x-quick-start-guide.pdf).

For more information about using Real Time Operating Systems with Bluetooth, see [AN1260: Integrating v3.x Silicon Labs Bluetooth Applications with Real-Time Operating Systems](https://www.silabs.com/documents/public/application-notes/an1260-integrating-v3x-bluetooth-applications-with-rtos.pdf).

The term SoC stands for "System on Chip". In the SoC model the entire system (stack and application) resides on a single chip, whereas in the NCP model the stack processing is done in a separate coprocessor that interacts with the application’s microcontroller through an external serial interface.

This example is an (almost) empty project that has only the bare minimum with Proxy and Relay features included to make a working Bluetooth Mesh application.

To add or remove features from the example, follow this process:

- Add model and feature components to your project
- Optionally configure your Mesh node through the "Bluetooth Mesh Configurator". It is configured to have only one element supporting a minimal set of models.

## Project Architecture

With the introduction of `sl_main`, a new architecture has been implemented to streamline application development. This architecture introduces several key functions that provide a structured and consistent approach to initialization and execution. Below is an overview of these functions and their roles:

- `app_init_early`: Invoked after peripheral initialization but before kernel initialization. This function is ideal for early setup tasks that must occur before the kernel is initialized.
- `app_permanent_memory_alloc`: Called after the kernel is initialized. This function is used for permanent memory allocations and task creation.
- `app_init`: Executed after all stacks are initialized and the kernel is operational. This function serves as the main initialization point for the application.
- `app_proceed`: Signals the application to proceed with the next iteration of the main loop.
- `app_process_action`: Represents the main loop of the application. It is executed once for every call to `app_proceed`.

### RTOS Applications
For RTOS-based applications, the start task can be reused after initialization is complete. To achieve this, redefine the `sl_main_start_task_should_continue()` function. Note that if this approach is used, the application task is not created, as the start task will directly call `app_process_action`.

![Bluetooth Mesh Configurator](readme_img1.png)

To learn more about programming an SoC application, see [UG472: Silicon Labs Bluetooth ® Mesh Configurator User's guide for SDK v2.x](https://www.silabs.com/documents/public/user-guides/ug472-bluetooth-mesh-v2x-node-configuration-users-guide.pdf).

- Some components are configurable, and can be customized using the Component Editor

![Bluetooth Mesh Components](readme_img2.png)

- Respond to the events raised by the Bluetooth stack
- Implement additional application logic

[UG295: Silicon Labs Bluetooth ® Mesh C Application Developer's Guide for SDK v2.x](https://www.silabs.com/documents/public/user-guides/ug295-bluetooth-mesh-dev-guide.pdf) gives code-level information on the stack and the common pitfalls to avoid.

## Responding to Bluetooth Events

Just like in the Bluetooth Low Energy SDK, a Mesh application is event-driven. The Bluetooth Mesh stack generates events when a remote device connects or disconnects, for example, or when it publishes a message. While it is not necessary to react to all events, the ones requiring action should be handled by the application in the `sl_btmesh_on_event()` function. The prototype of this function is implemented in *app.c*. To handle more events, the switch-case statement of this function is to be extended. For the list of Bluetooth Mesh events, see the HTML documentation present in the Simplicity Studio installation directory:

* <Simplicity-Studio-installation-directory\offline\com.silabs.sdk.stack.super_4.0.1\app\btmesh\documentation<SDK-installation-location>/documentation/API_BLUETOOTH_MESH_HTML

## Implementing Application Logic

Additional application logic can be implemented in the 'app_init_early()' and 'app_process_action()' functions or by creating tasks in `app_init()`. Find the definitions of these functions in *main.c*. The 'app_init_early()' and  `app_init()` functions are called once when the device is booted and the 'app_process_action()' is called repeatedly in a `while(1)` loop from the application task. This waits for a semaphore using `app_is_process_required()` that can be triggered by calling `app_proceed()`, which can be used for example to process peripherals. Note that when creating a new task, its stack size must be set according to its resource needs.

## Features Already Added to Bluetooth Mesh - SoC Empty Application

The **Bluetooth Mesh - SoC Empty** application is ***almost*** empty. It implements a basic application to demonstrate how to emit unprovisioned beacons on both the advertising and GATT bearer.

## Testing the Bluetooth Mesh - SoC Empty Application

As described above, an empty example does nothing except broadcast unprovisioned beacons. To test this feature, do the following:

1. Clear the NVM Mesh settings, for example by performing an Erase Chip with Simplicity Commander, since the example has no button control. See [UG162: Simplicity Commander Reference Guide](https://www.silabs.com/documents/public/user-guides/ug162-simplicity-commander-reference-guide.pdf) for more information.
2. Build and flash the **Bluetooth Mesh - SoC Empty** example to your device. The flashing can be done, for example, using the Simplicity Studio internal **Flash Programmer** or external **Simplicity Commander** tools.
3. Download the Silicon Labs **Bluetooth Mesh** smartphone application available on [iOS](https://apps.apple.com/us/app/bluetooth-mesh-by-silicon-labs/id1411352948) and [Android](https://play.google.com/store/apps/details?id=com.siliconlabs.bluetoothmesh). Make sure to reset the local database by pressing the "Reset local database" button in the menu "More".

![Bluetooth Mesh App Reset local database](readme_img3.png)

4. Open the app, choose the Provision Browser, and tap **Scan**.

![Bluetooth Mesh App Scan](readme_img4.png)

5. Now you should find your device advertising. Tap **PROVISION**.

![Bluetooth Mesh App Provision](readme_img5.png)

## Troubleshooting

Before programming the radio board mounted on the mainboard, make sure the power supply switch the AEM position (right side) as shown below.

![Radio board power supply switch](readme_img0.png)

## Resources

[Bluetooth Documentation](https://docs.silabs.com/bluetooth/latest/)

[Bluetooth Mesh Network - An Introduction for Developers](https://www.bluetooth.com/wp-content/uploads/2019/03/Mesh-Technology-Overview.pdf)

[FreeRTOS Documentation](https://freertos.org/Documentation/02-Kernel/01-About-the-FreeRTOS-kernel/01-FreeRTOS-kernel)

[Micrium OS Documentation](https://docs.silabs.com/micrium/latest/micrium-general-concepts/)

[Silicon Labs RTOS Landing Page](https://www.silabs.com/developers/rtos)

[QSG176: Bluetooth Mesh SDK v2.x Quick Start Guide](https://www.silabs.com/documents/public/quick-start-guides/qsg176-bluetooth-mesh-sdk-v2x-quick-start-guide.pdf)

[AN1260: Integrating v3.x Silicon Labs Bluetooth® Applications with Real-Time Operating Systems in SDK v3.x and Higher](https://www.silabs.com/documents/public/application-notes/an1260-integrating-v3x-bluetooth-applications-with-rtos.pdf)

[AN1315: Bluetooth Mesh Device Power Consumption Measurements](https://www.silabs.com/documents/public/application-notes/an1315-bluetooth-mesh-power-consumption-measurements.pdf)

[AN1316: Bluetooth Mesh Parameter Tuning for Network Optimization](https://www.silabs.com/documents/public/application-notes/an1316-bluetooth-mesh-network-optimization.pdf)

[AN1317: Using Network Analyzer with Bluetooth Low Energy ® and Mesh](https://www.silabs.com/documents/public/application-notes/an1317-network-analyzer-with-bluetooth-mesh-le.pdf)

[AN1318: IV Update in a Bluetooth Mesh Network](https://www.silabs.com/documents/public/application-notes/an1318-bluetooth-mesh-iv-update.pdf)

[UG295: Silicon Labs Bluetooth Mesh C Application Developer's Guide for SDK v2.x](https://www.silabs.com/documents/public/user-guides/ug295-bluetooth-mesh-dev-guide.pdf)

[UG472: Silicon Labs Bluetooth ® C Application Developer's Guide for SDK v3.x](https://www.silabs.com/documents/public/user-guides/ug434-bluetooth-c-soc-dev-guide-sdk-v3x.pdf)

[Bluetooth Training](https://www.silabs.com/support/training/bluetooth)

## Report Bugs & Get Support

You are always encouraged and welcome to report any issues you found to us via [Silicon Labs Community](https://www.silabs.com/community).
