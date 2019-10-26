/*
 * file:		arch/x86/kernel/arch.c
 * auther:		Jason Hu
 * time:		2019/6/2
 * copyright:	(C) 2018-2019 by Book OS developers. All rights reserved.
 */

#include <arch.h>
#include <segment.h>
#include <gate.h>
#include <cpu.h>
#include <phymem.h>
#include <bootmem.h>
#include <pci.h>
#include <book/debug.h>
#include <book/hal.h>
#include <hal/cpu.h>

/*
 * 功能: 平台架构初始化入口
 * 参数: 无
 * 返回: 成功返回0，失败返回-1
 * 说明: 初始化平台的所有信息，初始化完成后再跳入到内核执行
 */
int InitArch()
{	
	// 初始化硬件抽象层的环境
	InitHalEnvironment();
	
	// 初始化早期硬件抽象
	InitHalEarly();
	
	/*
	 * 在硬件抽象中初始化了显示器，后面才可以显示字符
	 */
	PART_START("Arch");

	// 查看cpu信息
	InitCpu();
	// 初始化内核段描述符
	InitSegmentDescriptor();
	// 初始化内核门描述符
	InitGateDescriptor();
	
	// 初始化任务状态段
	InitTss();

	// 初始化物理内存管理
	InitPhysicMemory();

	/* 初始化平台总线 */
	InitPci();

	//Spin("a");

	PART_END();

	return 0;
}
