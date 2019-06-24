/*
 * file:		arch/x86/kernel/arch.c
 * auther:		Jason Hu
 * time:		2019/6/2
 * copyright:	(C) 2018-2019 by Book OS developers. All rights reserved.
 */

#include <arch.h>
#include <page.h>
#include <segment.h>
#include <gate.h>
#include <book/debug.h>

/*
 * 功能: 平台架构初始化入口
 * 参数: 无
 * 返回: 成功返回0，失败返回-1
 * 说明: 初始化平台的所有信息，初始化完成后再跳入到内核执行
 */
int init_arch()
{
	//初始化内核调试功能，基于硬件抽象层
	InitKernelDebugHal();
	InitSegmentDescriptor();
	InitGateDescriptor();
	
	init_page();


	return 0;
}

