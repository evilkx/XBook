/*
 * file:		include/block/virtual/ramdisk.h
 * auther:		Jason Hu
 * time:		2019/9/22
 * copyright:	(C) 2018-2020 by Book OS developers. All rights reserved.
 */

#ifndef _DRIVER_RAMDISK_H
#define _DRIVER_RAMDISK_H

#include <lib/stdint.h>
#include <lib/types.h>

enum {
	RAMDISK_IO_CLEAN = 1,
    RAMDISK_IO_SECTORS,
    RAMDISK_IO_BLKZE,
	RAMDISK_IO_IDENTIFY,
};

#define VENDOR_CHAR_LEN 24
#define VERSION_CHAR_LEN 12

/* 磁盘信息 */
struct RamdiskInfo {
	unsigned int sectorSize;	// 扇区大小
	unsigned int sectors;		// 磁盘大小(扇区数)
	char vendor[VENDOR_CHAR_LEN];
	char version[VERSION_CHAR_LEN];
};

/*
PUBLIC int InitRamdiskDriver();
PUBLIC void ExitRamdiskdDriver();
*/

#endif  /* _DRIVER_RAMDISK_H */
