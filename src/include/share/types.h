/*
 * file:		   include/share/types.h
 * auther:		Jason Hu
 * time:		2019/6/2
 * copyright:	(C) 2018-2019 by Book OS developers. All rights reserved.
 */

#ifndef _SHARE_TYPES_H
#define _SHARE_TYPES_H

#ifndef NULL
#ifdef __cplusplus
        #define NULL 0
#else
        #define NULL ((void *)0)
#endif
#endif

#ifndef __cplusplus
#define bool _Bool      //C语言下实现Bool
#define true 1
#define false 0
#endif

#endif  /*_SHARE_TYPES_H*/
