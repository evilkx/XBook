/*
 * file:		arch/x86/include/mm/zone.h
 * auther:		Jason Hu
 * time:		2019/7/1
 * copyright:	(C) 2018-2019 by Book OS developers. All rights reserved.
 */

#ifndef _X86_MM_ZONE_H
#define _X86_MM_ZONE_H


#include "area.h"
#include <share/stddef.h>
#include <share/const.h>
#include <book/bitmap.h>


/* 
 * 物理空间的划分
 */

// 0MB~1MB是体系结构相关的内存分布
#define ZONE_PHY_ARCH_ADDR          0X000000000
#define ZONE_ARCH_SIZE             0X000100000     // 1MB

// 1MB~2MB是内核镜像
#define ZONE_PHY_KERNEL_ADDR        (ZONE_PHY_ARCH_ADDR+ZONE_ARCH_SIZE)
#define ZONE_KERNEL_SIZE               0X000100000     // 1MB

// 2MB~8MB是系统重要信息的存放地址
#define ZONE_PHY_MATERIAL_ADDR      (ZONE_PHY_KERNEL_ADDR+ZONE_KERNEL_SIZE)
#define ZONE_MATERIAL_SIZE          0X000600000   // 6MB

// 8MB~16MB是设备DMA的地址
#define ZONE_PHY_DMA_ADDR           (ZONE_PHY_MATERIAL_ADDR+ZONE_MATERIAL_SIZE)
#define ZONE_DMA_SIZE               0X000800000   // 8MB

// ---- 以上是必须的基本内存使用情况 ----

// 16M以上是静态映射开始地址
#define ZONE_PHY_STATIC_ADDR           (ZONE_PHY_DMA_ADDR+ZONE_DMA_SIZE)

/* 
    我们会把16M以上的物理内存分为3部分，静态，动态，持久态。
    静态：在内核运行时就让虚拟地址和物理地址做一对一映射，
可以直接通过虚拟地址访问对应的物理地址。
分配和释放页，都是直接对地址进行操作，不需要再次做映射。
实现kmalloc和kfree部分

    动态：保留物理地址，当需要使用虚拟地址时，必须做映射，
才可以访问物理地址。可以对物理地址进行
分配和释放，这个过程都需要做页的映射操作。
实现vmalloc和vfree部分

    持久态：相对于动态的来说，没有那么频繁，一块区域分配后，
几乎就是用到系统结束，除非把某个设备卸载，
这样，会把设备对应的地址映射取消掉。
分配和释放，这个过程都需要做页的映射操作。
实现kmap和kunmap部分

    他们的位置会根据物理内存的大小做出一定的调整。
持久态的变化应该不大，因为大多数设备的内存地址映射都是已经设定好了的。
所以，我们给他一个大致的范围就好了。但是对于静态和动态这两部分，
最好做到公平起见，但是感觉静态的用的更多，而动态的也就是几个少数地方会使用，
所以，把物理内存分成2份，1份给静态，1份给动态。持久态不需要，因为他是和
设备内存映射一对一映射
*/

/*
 * 内核空间的划分，以下都是虚拟地址
 */

// 该地址是3GB虚拟地址的偏移
#define ZONE_VIR_ADDR_OFFSET        0Xc0000000

/* 
因为在我们的分页机制中，最后一个页目录表项存放的是页目录表自己，
所以不能使用，因此少了4MB的内存空间。
我把它称作黑洞（black hole）区域，就是不能访问，访问就系统死机
*/
#define ZONE_VIR_BLACK_HOLE_SIZE     (4*MB)    // 4MB内存大小

/* 内核固定内存的虚拟地址 */
#define ZONE_VIR_BLACK_HOLE_ADDR     (0XFFFFFFFF-ZONE_VIR_BLACK_HOLE_SIZE)

/* 内核动态内存最大128MB */
#define ZONE_VIR_DYNAMIC_SIZE           (128*MB)
/* 动态内存的虚拟地址 */
#define ZONE_VIR_DYNAMIC_ADDR           (ZONE_VIR_BLACK_HOLE_ADDR - ZONE_VIR_DYNAMIC_SIZE)

#define ZONE_VIR_DYNAMIC_END           (ZONE_VIR_DYNAMIC_ADDR + ZONE_VIR_DYNAMIC_SIZE)

/* 默认内核静态空间地址时在DMA之后*/
#define ZONE_VIR_STATIC_ADDR            (ZONE_VIR_ADDR_OFFSET + ZONE_PHY_STATIC_ADDR)

/* 规划1GB虚拟地址给静态内存 */
#define ZONE_VIR_STATIC_SIZE            (ZONE_VIR_DYNAMIC_ADDR-ZONE_VIR_STATIC_ADDR)

/* 一共有多少个空间 */
#define MAX_ZONE_NR     2


/* 静态空间是内核自己运行需要的空间 */
#define ZONE_TYPE_STATIC     0

/* 静态空间是内核和用户共同拥有的空间 */
#define ZONE_TYPE_DYNAMIC    1

/* 
静态空间最大的页数 
1GB的页数-16MB的页数
然后在于512对齐
*/

#define ZONE_STATIC_MAX_PAGES      (ZONE_VIR_STATIC_SIZE / PAGE_SIZE)

#define ZONE_SEPARATE_MAX_PAGES      (PAGE_NR_IN_1GB - ZONE_VIR_BLACK_HOLE_SIZE / PAGE_SIZE)

#define ZONE_BAD_RANGE(zone, page) \
        (!(zone->pageArray <= page && \
        page <= zone->pageArray + zone->pageTotalCount))

/* 用于描述内核内存地址空间的结构体 */
struct Zone 
{
    address_t virtualStart;     //虚拟空间的开始地址
    address_t virtualEnd;       //虚拟空间的结束地址
    size_t virtualLength;       //虚拟空间的大小
    
    address_t physicStart;      //物理空间的开始地址
    address_t physicEnd;        //虚拟空间的结束地址
    size_t physicLength;        //物理空间的大小

    flags_t flags;  //空间的状态标志

    //与页相关的部分
    /* 页的数组,避免和页表混淆，这里用array*/
    struct Page *pageArray;
    /* 管理页的使用状态的位图 */
    struct Bitmap pageMap;
    /* 指向伙伴算法对应的area的地址 */
    struct FreeArea freeArea[MAX_ORDER];

    //对页的使用情况的统计
    unsigned int pageTotalCount;    //总共有多少个页
    unsigned int pageUsingCount;    //使用中的页数
    unsigned int pageFreeCount;     //空闲的页数

};

PUBLIC void InitZone();

EXTERN struct Zone zoneOfMemory;

/* 获取空间的方法 */
PUBLIC INLINE struct Zone *ZoneGetByType(int type);
PUBLIC INLINE struct Zone *ZoneGetByPhysicAddress(unsigned int paddr);
PUBLIC INLINE struct Zone *ZoneGetByVirtualAddress(unsigned int vaddr);
PUBLIC INLINE struct Zone *ZoneGetByPage(struct Page *page);
PUBLIC INLINE unsigned int ZoneGetTotalPages(int type);
PUBLIC INLINE unsigned int ZoneGetAllUsingPages();
PUBLIC INLINE unsigned int ZoneGetAllTotalPages();
PUBLIC INLINE unsigned int ZoneGetInitMemorySize();

/* 页和物理地址的转换 */
PUBLIC INLINE struct Page *PhysicAddressToPage(unsigned int addr);
PUBLIC INLINE address_t PageToPhysicAddress(struct Page *page);

/* 物理地址和虚拟地址的转换 */
PUBLIC INLINE address_t PhysicToVirtual(unsigned int paddr);
PUBLIC INLINE address_t VirtualToPhysic(unsigned int vaddr);

#endif   /*_X86_MM_ZONE_H */
