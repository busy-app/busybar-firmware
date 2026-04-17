# Deprecation notice for Device Information GATT Service (Legacy) component

This component provides backward compatibility for projects upgraded from Bluetooth stack version
9.x.x and earlier. Please review the scenarios below for guidance.

## Scenario 1: Upgrading from Bluetooth Stack Version 9.x.x or Earlier

The [sl_gatt_service_device_information.c](sl_gatt_service_device_information.c) that comes with
this component derives the value of the firmware revision string characteristic from the Bluetooth
stack version. The Bluetooth version is expected to fit in 5 characters like this: `X.Y.Z`.
Starting from version 10.0.0, this rule breaks, that needs to be fixed manually.
Since the sl_gatt_service_device_information.c is owned by the project, an SDK upgrade would break the build. To prevent this, a placeholder
Bluetooth stack version is injected in the sl_gatt_service_device_information.h header.

The [sl_gatt_service_device_information_warning.c](sl_gatt_service_device_information_warning.c) is
also added to the project. Its sole purpose is to ensure that replacing the actual Bluetooth stack
version with a placeholder does not go unnoticed, by generating a compiler warning.

Depending on your needs, you have several options to fix this:

### If you want the Firmware Revision String to show the Bluetooth stack version

**WARNING:** Following these steps will remove `sl_gatt_service_device_information.c` from your project.  
If you have made any custom changes to this file, be sure to create a backup before proceeding.

Replace the 'Device Information GATT Service (Legacy)' component with the 'Device Information GATT Service Override' component:
- Open the 'Software Components' view in your project.
- Navigate to `Bluetooth` > `Application` > `GATT Services` > `Device Information GATT Service Override`.
- When prompted about conflicting components, choose to replace Device Information GATT Service (Legacy) with Device Information GATT Service Override.
- Adjust the length of the Firmware Revision String using the GATT Configurator tool.

### If you have your own implementation for the Firmware Revision String and want to remove the compiler warning

Simply remove `sl_gatt_service_device_information_warning.c` from your project.

### If you do not care about the Firmware Revision String value and want to remove the compiler warning

Simply remove `sl_gatt_service_device_information_warning.c` from your project.

## Scenario 2: Adding This Component to a Project Manually

If you added this component by mistake, remove it from your project and add the 'Device Information GATT Service Override' component instead.
