#!/usr/bin/env bash

set -e -x

./fbt TARGET_HW=20 flash


# 2025-04-09 14:03:30,536 [ERROR] Error: Command '['openocd', '-f', 'interface/cmsis-dap.cfg', '-c', 'transport select swd', '-f', '/Users/lomalkin/_repoz/bsb-firmware/scripts/debug/platforms/stm32u5/stm32u5x.cfg', '-c', 'stm32u5x.cpu configure -rtos auto', '-c', 'program "build/f20-firmware-D/firmware.elf"  reset exit']' returned non-zero exit status 1.


# ['openocd', '-f', 'interface/cmsis-dap.cfg', '-c', 'transport select swd', '-f', '/Users/lomalkin/_repoz/bsb-firmware/scripts/debug/platforms/stm32u5/stm32u5x.cfg', '-c', 'stm32u5x.cpu configure -rtos auto', '-c', 'program "build/f20-firmware-D/firmware.elf"  reset exit']

/Users/lomalkin/_repoz/bsb-firmware/toolchain/arm64-darwin/bin/openocd
openocd -f interface/cmsis-dap.cfg -c "transport select swd" -f /Users/lomalkin/_repoz/bsb-firmware/scripts/debug/platforms/stm32u5/stm32u5x.cfg -c stm32u5x.cpu configure -rtos auto -c \"program build/f20-firmware-D/firmware.elf\" reset exit

/Users/lomalkin/_repoz/bsb-firmware/toolchain/arm64-darwin/bin/openocd -f "interface/cmsis-dap.cfg" -c "transport select swd" -f "/Users/lomalkin/_repoz/bsb-firmware/scripts/debug/platforms/stm32u5/stm32u5x.cfg" -c "stm32u5x.cpu configure" -rtos "auto" -c "program build/f20-firmware-D/firmware.elf" reset exit


# . ./scripts/toolchain/fbtenv.sh 

# openocd -f interface/cmsis-dap.cfg -c transport select swd -f ./scripts/debug/platforms/stm32u5/stm32u5x.cfg -c 'stm32u5x.cpu configure -rtos auto' -c 'program "build/f20-firmware-D/firmware.elf" reset exit'








# Go inside:
# python3 scripts/fwflash.py --interface=auto --serial=auto --platform=/Users/lomalkin/_repoz/bsb-firmware/scripts/debug/platforms/stm32u595.json -ex build/f20-firmware-D/firmware.elf

# python3 ./scripts/fwflash.py --interface=auto --serial=auto --platform=/Users/lomalkin/_repoz/bsb-firmware/scripts/debug/platforms/stm32u595.json -ex build/f20-firmware-D/firmware.elf

# venvlomalkin@mbp16 ~/_repoz/bsb-firmware % git log --oneline -n 1
# c371c19 (HEAD -> dev, origin/dev, origin/HEAD) ble_per_test fix choice rate (#91)

# venvlomalkin@mbp16 ~/_repoz/bsb-firmware % ./fbt TARGET_HW=20 flash
# scons: Entering directory `/Users/lomalkin/_repoz/bsb-firmware/fbt_layers/fbtng'
#         AR      build/f20-firmware-D/targets/libflipper20.a
#         RANLIB  build/f20-firmware-D/targets/libflipper20.a
#         LINK    build/f20-firmware-D/fw_elf/firmware.elf
# python3 scripts/fwsize.py elf build/f20-firmware-D/fw_elf/firmware.elf
# .text         476660 (465.49 K)
# .rodata        96056 ( 93.80 K)
# .data           1240 (  1.21 K)
# .bss           26956 ( 26.32 K)
#         INSTALL build/f20-firmware-D/firmware.elf
#         INFO
# Loaded 49 app definitions.
# Firmware modules configuration:
# Service:
#          cli, cli_socket, usb_srv, intercom, desktop, power_popup, storage, httpd_test, input, audio, back_display, power_srv, led_display, light_sensor, loader, sockets, wifi, status_lights, gui
# System:
#          apps_menu, message
# App:
#          input_test, animation_player, gui_test, light_sensor_test, led_display_test, back_display_test, dummy, ble_per_test, wifi_per_test, power_test
# StartupHook:
#          input_start, power_srv_start, storage_start, loader_start, sl_update_start
# Package:
#          main_apps, basic_services, system_apps
# python3 scripts/fwflash.py --interface=auto --serial=auto --platform=/Users/lomalkin/_repoz/bsb-firmware/scripts/debug/platforms/stm32u595.json -ex build/f20-firmware-D/firmware.elf
# 2025-04-09 13:11:12,850 [ERROR] Error: Debug adapter not found
# scons: *** [build/20_firmware_flash.flag] Error 1

# ********** FBT ERRORS **********
# build/20_firmware_flash.flag: Error 1
# venvlomalkin@mbp16 ~/_repoz/bsb-firmware % 




# python3 scripts/fwflash.py --interface=auto --serial=auto --platform=/Users/lomalkin/_repoz/bsb-firmware/scripts/debug/platforms/stm32u595.json -ex build/f20-firmware-D/firmware.elf


# venvlomalkin@mbp16 ~/_repoz/bsb-firmware % scripts/fwflash.py
# zsh: no such file or directory: scripts/fwflash.py


# venvlomalkin@mbp16 ~/_repoz/bsb-firmware % ll scripts 
# total 120
# drwxr-xr-x  16 lomalkin  staff   512B Apr  3 14:01 .
# drwxr-xr-x  33 lomalkin  staff   1.0K Apr  8 19:01 ..
# -rw-r--r--@  1 lomalkin  staff   6.0K Apr  3 10:55 .DS_Store
# -rw-r--r--   1 lomalkin  staff     0B Jan 28 01:18 .gitkeep
# -rwxr-xr-x@  1 lomalkin  staff   1.1K Mar 14 07:41 audio.py
# -rwxr-xr-x   1 lomalkin  staff   3.0K Jan 28 01:18 bin2rps.py
# drwxr-xr-x   3 lomalkin  staff    96B Jan 28 01:18 debug
# drwxr-xr-x   4 lomalkin  staff   128B Jan 28 01:18 fbt_env_modules
# drwxr-xr-x   8 lomalkin  staff   256B Feb  7 00:08 fbt_tools
# -rwxr-xr-x@  1 lomalkin  staff    20K Mar 27 15:37 flashrps.py
# drwxr-xr-x@  5 lomalkin  staff   160B Apr  1 11:18 flipper
# -rwxr-xr-x@  1 lomalkin  staff   3.9K Mar 27 15:37 seq2anim.py
# -rwxr-xr-x@  1 lomalkin  staff   6.9K Mar 19 03:41 storage.py
# drwxr-xr-x   9 lomalkin  staff   288B Feb 19 02:48 toolchain
# -rwxr-xr-x@  1 lomalkin  staff   6.4K Apr  3 14:01 update.py
# -rwxr-xr-x@  1 lomalkin  staff   2.1K Mar 27 15:37 viewer.py
