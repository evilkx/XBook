/*
 * file:		include/book/config.h
 * auther:		Jason Hu
 * time:		2019/6/3
 * copyright:	(C) 2018-2019 by Book OS developers. All rights reserved.
 */

/**
 * 操作系统配置文件，有些可以配置的内容可以在这里进行配置，配置后重新编译内核即可
 */

#ifndef _BOOK_CONFIG_H
#define _BOOK_CONFIG_H

/**
 * ------------------------
 * 系统宏
 * ------------------------
 */
#define _BOOK32 /* 配置为BOOK32位 */



/**
 * ------------------------
 * 系统平台
 * ------------------------
 */
#define CONFIG_ARCH_X86 /* 配置为X86平台 */

/**
 * ------------------------
 * CPU的数据宽度
 * ------------------------
 */
#define CONFIG_CPU_32   /* 32位CPU */
//#define CONFIG_CPU_64 /* 64位CPU */

/**
 * ------------------------
 * 内核配置
 * ------------------------
 */

#define CONFIG_SEMAPHORE_M /* 配置多元信号量（Multivariate semaphore） */
//#define CONFIG_SEMAPHORE_B /* 配置二元信号量（Binary semaphore） */

/**
 * ------------------------
 * 内存管理配置
 * ------------------------
 */
//#define CONFIG_LARGE_ALLOCS /* 如果想要用kmalloc分配128KB~4MB之间大小的内存，就需要配置此项 */

/**
 * ------------------------
 * 配置驱动
 * ------------------------
 */
#define CONFIG_DRV_KEYBOARD /* 键盘驱动配置 */
//#define CONFIG_DRV_MOUSE  /* 鼠标驱动配置 */
#define CONFIG_DRV_IDE      /* IDE驱动配置 */
#define CONFIG_DRV_RAMDISK  /* RAMDISK驱动配置 */

#endif   /*_BOOK_CONFIG_H*/
