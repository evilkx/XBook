/*
 * file:		arch/x86/include/kernel/segment.h
 * auther:		Jason Hu
 * time:		2019/6/23
 * copyright:	(C) 2018-2019 by Book OS developers. All rights reserved.
 */

#ifndef _ARCH_SEGMENT_H
#define _ARCH_SEGMENT_H

#include <share/stdint.h>

/*
 * 段的相关信息会出现在这个文件中
 */

/* 描述符类型值说明 */
#define	DA_32			0x4000	/* 32 位段				*/
#define	DA_G			0x8000	/* 段界限粒度为 4K 字节			*/
#define	DA_DPL0			0x00	/* DPL = 0				*/
#define	DA_DPL1			0x20	/* DPL = 1				*/
#define	DA_DPL2			0x40	/* DPL = 2				*/
#define	DA_DPL3			0x60	/* DPL = 3				*/
/* 存储段描述符类型值说明 */
#define	DA_DR			0x90	/* 存在的只读数据段类型值		*/
#define	DA_DRW			0x92	/* 存在的可读写数据段属性值		*/
#define	DA_DRWA			0x93	/* 存在的已访问可读写数据段类型值	*/
#define	DA_C			0x98	/* 存在的只执行代码段属性值		*/
#define	DA_CR			0x9A	/* 存在的可执行可读代码段属性值		*/
#define	DA_CCO			0x9C	/* 存在的只执行一致代码段属性值		*/
#define	DA_CCOR			0x9E	/* 存在的可执行可读一致代码段属性值	*/
/* 系统段描述符类型值说明 */
#define	DA_LDT			0x82	/* 局部描述符表段类型值			*/
#define	DA_TaskGate		0x85	/* 任务门类型值				*/
#define	DA_386TSS		0x89	/* 可用 386 任务状态段类型值		*/
#define	DA_386CGate		0x8C	/* 386 调用门类型值			*/
#define	DA_386IGate		0x8E	/* 386 中断门类型值			*/
#define	DA_386TGate		0x8F	/* 386 陷阱门类型值			*/

/* 选择子类型值说明 */
/* 其中, SA_ : Selector Attribute */
#define	SA_RPL0		0
#define	SA_RPL1		1
#define	SA_RPL2		2
#define	SA_RPL3		3

#define	SA_TIG		0
#define	SA_TIL		4

//index of descriptor
#define	INDEX_DUMMY 0
#define	INDEX_KERNEL_C 1
#define	INDEX_KERNEL_RW 2
#define	INDEX_TSS 3
#define	INDEX_USER_C 4
#define	INDEX_USER_RW 5

//选择子...
//内核代码，数据，栈，视频

#define KERNEL_CODE_SEL ((INDEX_KERNEL_C << 3) + (SA_TIG << 2) + SA_RPL0)
#define KERNEL_DATA_SEL ((INDEX_KERNEL_RW << 3) + (SA_TIG << 2) + SA_RPL0)
#define KERNEL_STACK_SEL KERNEL_DATA_SEL 

//用户代码，数据，栈
#define USER_CODE_SEL ((INDEX_USER_C << 3) + (SA_TIG << 2) + SA_RPL3)
#define USER_DATA_SEL ((INDEX_USER_RW << 3) + (SA_TIG << 2) + SA_RPL3)
#define USER_STACK_SEL USER_DATA_SEL 

//TSS
#define KERNEL_TSS_SEL ((INDEX_TSS << 3) + (SA_TIG << 2) + SA_RPL0)

/* GDT 的虚拟地址 */
#define GDT_VADDR			0x80200000
#define GDT_LIMIT		0x000007ff

/*
段描述符结构
*/
struct SegmentDescriptor {
	unsigned short limitLow, baseLow;
	unsigned char baseMid, accessRight;
	unsigned char limitHigh, baseHigh;
};

//初始化段描述符
void InitSegmentDescriptor();

//设置段描述符
void SetSegmentDescriptor(struct SegmentDescriptor *descriptor, unsigned int limit, \
        unsigned int base, unsigned int attributes);

#endif	/*_ARCH_SEGMENT_H*/