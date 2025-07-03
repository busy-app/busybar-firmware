#include "fatfs.h"

/** logical drive path */
char fatfs_path[4];
/** File system object */
FATFS fatfs_object;

PARTITION VolToPart[_VOLUMES] = {
    {0, 1}, /* "0:" ==> 1st partition on the physical drive 0 */
    {0, 2}, /* "1:" ==> 2nd partition on the physical drive 0 */
};

void fatfs_init(void) {
    FATFS_LinkDriver(&sd_fatfs_driver, fatfs_path);
}
