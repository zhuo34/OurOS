#ifndef DRIVER_SD_H
#define DRIVER_SD_H

#include <os/type.h>

#define SECSIZE 	(1 << 9)
#define SECSHIFT 	9

bool sd_read_block(Byte *buf, uint addr, uint count);
bool sd_write_block(Byte *buf, uint addr, uint count);

#endif  // DRIVER_SD_H