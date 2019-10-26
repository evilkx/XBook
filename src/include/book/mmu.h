/*
 * file:		include/book/slab.h
 * auther:		Jason Hu
 * time:		2019/7/11
 * copyright:	(C) 2018-2019 by Book OS developers. All rights reserved.
 */

#ifndef _BOOK_MMU_H
#define _BOOK_MMU_H

/**
 * 内存管理单元
 */
#include <share/types.h>

PUBLIC void MmuMemoryInfo();

/* 导入内存管理相关头问题 */
#include <book/vmspace.h>

#endif   /*_BOOK_MMU_H*/
