# Internal images

Internal images are compiled to specially formatted `.c` source files and are linked together with the firmware.

Use them for critical images that need to be displayed regardless of the internal storage state.

## Usage

1. Include the generated header file:
```c
#include <assets_images.h>
```
2. The images can be referenced by their respective `I_image_name` variables. 

**NOTE**: Internal images may only be used in internal LVGL functions like `lv_image_set_src()` for now.
