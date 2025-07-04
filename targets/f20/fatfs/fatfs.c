#include "fatfs.h"

PARTITION VolToPart[_VOLUMES] = {
    {0, 1}, /* "0:" ==> 1st partition on the physical drive 0 */
    {0, 2}, /* "1:" ==> 2nd partition on the physical drive 0 */
};

void fatfs_init(char* path, size_t logical_unit_number) {
    FATFS_LinkDriverEx(&sd_fatfs_driver, path, logical_unit_number);
}
