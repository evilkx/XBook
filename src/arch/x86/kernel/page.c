/*
 * file:		arch/x86/kernel/page.c
 * auther:		Jason Hu
 * time:		2019/6/2
 * copyright:	(C) 2018-2019 by Book OS developers. All rights reserved.
 */

#include <page.h>
#include <vga.h>
#include <ards.h>
#include <x86.h>

#include <share/stdint.h>
#include <book/bitmap.h>
#include <share/string.h>
//内核和用户的内存池
memory_pool_t kernel_memory_pool, user_memory_pool;

//内核的虚拟地址，由于用户有很多个，所以用户的虚拟地址定义在进程结构体重
virtual_addr_t kernel_vir_addr;

static void init_memoryPool(uint32_t total_mem);

static uint64_t phy_memory_total;

/*
 * 功能: 初始化分页模式相关
 * 参数: 无
 * 返回: 成功返回0，失败返回-1
 * 说明: 对分页相关信息设置
 */
int init_page()
{
	printk("> init page start.\n");

	//把页目录表设置
	uint32_t *pdt = (uint32_t *)PAGE_DIR_VIR_ADDR;
	//把页目录表的第1项清空，这样，我们就只能通过高地址访问内核了
	pdt[0] = 0;
	//设置物理内存管理方式，以页框为单位
	phy_memory_total = init_ards();
	init_memoryPool(phy_memory_total);
	
	void *a = alloc_kernelPage(100);
	
	void *b = alloc_kernelPage(1000);
	void *c = alloc_kernelPage(10000);
	void *d = alloc_kernelPage(10);
	printk("%x %x %x %x\n", a, b, c, d);
	
	free_kernelPage(a,100);
	free_kernelPage(b,1000);
	free_kernelPage(c,10000);
	free_kernelPage(d,10);

	printk("< init page done.\n");

	return 0;
}

static void init_memoryPool(uint32_t total_mem)
{
	printk("> init memory pool start.\n");

	if (total_mem > X86_PHY_MEM_MAX) {
		total_mem = X86_PHY_MEM_MAX;
	}
	printk(" \\_memory size is %x bytes %d MB.\n", total_mem, total_mem/(1024*1024));

	//计算内存
	uint32_t used_mem = FREE_PHY_START;
	uint32_t free_mem = total_mem - used_mem;

	//还有多少页可以使用
	uint32_t all_free_pages = free_mem/PAGE_SIZE;

	//把空闲的一半给内核，一半给用户
	uint32_t kernel_free_pages = all_free_pages/2;
	uint32_t user_free_pages = all_free_pages - kernel_free_pages;
	
	//以字节为单位的大小
	uint32_t kernel_bitmap_len = kernel_free_pages/8;
	uint32_t user_bitmap_len = user_free_pages/8;
	
	uint32_t kernel_mem_poop_start = used_mem;
	//用户物理内存在内核后面
	uint32_t user_mem_poop_start = kernel_free_pages*PAGE_SIZE;
	
	//把数据填写到数据结构
	kernel_memory_pool.phy_addr_start = kernel_mem_poop_start;
	kernel_memory_pool.pool_size = kernel_free_pages*PAGE_SIZE;
	kernel_memory_pool.pool_bitmap.btmp_bytes_len = kernel_bitmap_len;
	
	user_memory_pool.phy_addr_start = user_mem_poop_start;
	user_memory_pool.pool_size = user_free_pages*PAGE_SIZE;
	user_memory_pool.pool_bitmap.btmp_bytes_len = user_bitmap_len;
	
	kernel_memory_pool.pool_bitmap.bits = (void *)MEM_POOL_BITMAP_VIR_ADDR_START;
	user_memory_pool.pool_bitmap.bits = (void *)(MEM_POOL_BITMAP_VIR_ADDR_START + MAX_MEM_POOL_BITMAP_SIZE);
	
	//显示信息
	printk("  \\_kernel memory pool: bitmap start:%x\n    phy addr start:%x size:%x\n", \
		kernel_memory_pool.pool_bitmap.bits, kernel_memory_pool.phy_addr_start, kernel_memory_pool.pool_size);
	printk("  \\_user memory pool: bitmap start:%x\n    phy addr start:%x size:%x\n", \
		user_memory_pool.pool_bitmap.bits, user_memory_pool.phy_addr_start, user_memory_pool.pool_size);
	
	//初始化位图
	bitmap_init(&kernel_memory_pool.pool_bitmap);
	bitmap_init(&user_memory_pool.pool_bitmap);

	//初始化内核虚拟地址，理论上内核虚拟内存的长度是小于物理内存的长度，在这里，让他们一样
	kernel_vir_addr.vaddr_bitmap.btmp_bytes_len = kernel_bitmap_len;
	//位图放到用户内存池后面
	kernel_vir_addr.vaddr_bitmap.bits = user_memory_pool.pool_bitmap.bits + MAX_MEM_POOL_BITMAP_SIZE;

	//虚拟地址的开始地址
	kernel_vir_addr.vir_addr_start = KERNEL_HEAP_START;	

	printk("  \\_kernel virtual addr: bitmap start:%x  vir addr start:%x\n", \
		kernel_vir_addr.vaddr_bitmap.bits, kernel_vir_addr.vir_addr_start);
	
	bitmap_init(&kernel_vir_addr.vaddr_bitmap);

	printk("< init memory pool done.\n");
}

uint32_t *page_ptePtr(uint32_t vaddr)
{
	uint32_t *pte = (uint32_t *)(0xffc00000 + \
	((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr)*4);
	return pte;
}

uint32_t *page_pdePtr(uint32_t vaddr)
{
	uint32_t *pde = (uint32_t *)(0xfffff000 + \
	PDE_IDX(vaddr)*4);
	return pde;
}

uint32_t page_addrV2P(uint32_t vaddr)
{
	uint32_t* pte = page_ptePtr(vaddr);
	/* 
	(*pte)的值是页表所在的物理页框地址,
	去掉其低12位的页表项属性+虚拟地址vaddr的低12位
	*/
	return ((*pte & 0xfffff000) + (vaddr & 0x00000fff));
}

void *alloc_pageVirtualAddr(pool_flags_t pf, uint32_t pages)
{
	int bit_idx_start = -1;
	uint32_t i, vaddr_start = 0;
	if (pf == PF_KERNEL) {
		//内核虚拟地址
		bit_idx_start = bitmap_scan(&kernel_vir_addr.vaddr_bitmap, pages);
		
		if (bit_idx_start == -1) {
			//分配失败
			return NULL;
		}
		//设置分配的那么多个位图位为1
		for (i = 0; i < pages; i++) {
			bitmap_set(&kernel_vir_addr.vaddr_bitmap, bit_idx_start + i, 1);
		}
		vaddr_start = kernel_vir_addr.vir_addr_start + bit_idx_start*PAGE_SIZE;
	} else {
		//用户虚拟地址

	}
	return (void *)vaddr_start;
}

void free_pageVirtualAddr(pool_flags_t pf, void *vaddr, uint32_t pages)
{
	uint32_t bit_idx_start = 0;
	uint32_t vir_addr = (uint32_t)vaddr;
	uint32_t n = 0;
	if (pf == PF_KERNEL) {
		//内核虚拟地址
		bit_idx_start = (vir_addr - kernel_vir_addr.vir_addr_start)/PAGE_SIZE;
		while (n < pages) {
			bitmap_set(&kernel_vir_addr.vaddr_bitmap, bit_idx_start + n, 0);
			n++;
		}
		
	} else {
		//用户虚拟地址

	}
}

void *pool_allocPhyMem(struct memory_pool_s *mem_pool)
{
	int bit_idx = bitmap_scan(&mem_pool->pool_bitmap, 1);
	if (bit_idx == -1) {
		return NULL;
	}
	bitmap_set(&mem_pool->pool_bitmap, bit_idx, 1);
	uint32_t page_phy_addr = mem_pool->phy_addr_start +  bit_idx*PAGE_SIZE;
	return (void *) page_phy_addr;
}

void pool_freePhyMem(uint32_t phy_addr)
{
	struct memory_pool_s *mem_pool;
	uint32_t bit_idx = 0;
	//根据地址判断是内核的地址还是用户的地址
	if (phy_addr >= user_memory_pool.phy_addr_start) {
		mem_pool = &user_memory_pool;
	} else {
		mem_pool = &kernel_memory_pool;
	}

	bit_idx = (phy_addr-mem_pool->phy_addr_start)/PAGE_SIZE;
	//把对应的位置置0，表示没有被使用
	bitmap_set(&mem_pool->pool_bitmap, bit_idx, 0);
}

/*
 * 功能: 页表的虚拟地址和一个物理地址映射
 * 参数: vir_addr 	虚拟地址
 * 		phy_addr	物理地址
 * 返回: 无
 * 说明: 直接就是虚拟地址对应物理地址，而中间的页表和页目录表是自动识别
 */
void page_tableAdd(void *vir_addr, void *phy_addr)
{
	uint32_t vaddr = (uint32_t)vir_addr, paddr = (uint32_t)phy_addr;
	uint32_t *pde = page_pdePtr(vaddr);
	uint32_t *pte = page_ptePtr(vaddr);
	
	if (*pde&PAGE_P_1) {
		//页目录项存在

		//理论上说页表是不存在的
		if (!(*pte&PAGE_P_1)) {
			//页表不存在
			//填入对应的地址
			*pte = (paddr|PAGE_US_U|PAGE_RW_W|PAGE_P_1);
		} else {
			//panic("pte exist");
			*pte = (paddr|PAGE_US_U|PAGE_RW_W|PAGE_P_1);
		}

	} else {
		//没有页目录项就创建一个
		uint32_t pde_phyaddr = (uint32_t)pool_allocPhyMem(&kernel_memory_pool);
		*pde = (pde_phyaddr|PAGE_US_U|PAGE_RW_W|PAGE_P_1);

		//把新建的页表清空
		memset((void *)((uint32_t)pte&0xfffff000), 0, PAGE_SIZE);

		*pte = (paddr|PAGE_US_U|PAGE_RW_W|PAGE_P_1);
	}
}

void page_tableRemovePTE(uint32_t vaddr)
{
	//获取页表项，并把存在位置0表明页不存在
	uint32_t *pte = page_ptePtr(vaddr);
	*pte &= ~PAGE_P_1;

	//更新tlb
	X86Invlpg(vaddr);
}

void *page_allocMemory(pool_flags_t pf, uint32_t pages)
{
	void *vaddr_start = alloc_pageVirtualAddr(pf, pages);
	if (vaddr_start == NULL) {
		return NULL;
	}
	
	uint32_t vaddr = (uint32_t)vaddr_start, n = pages;
	struct memory_pool_s *mem_pool = pf&PF_KERNEL ? &kernel_memory_pool:&user_memory_pool;

	while (n-- > 0) {
		void *page_phy_addr = pool_allocPhyMem(mem_pool);
		
		if (page_phy_addr == NULL) {
			return NULL;
		}
		
		page_tableAdd((void *)vaddr, page_phy_addr);

		vaddr += PAGE_SIZE;
	}
	return vaddr_start;
}

void page_freeMemory(pool_flags_t pf, void *vaddr, uint32_t pages)
{
	uint32_t phy_addr;
	uint32_t vir_addr = (uint32_t )vaddr;
	uint32_t n = 0;
	phy_addr = page_addrV2P(vir_addr);
	uint8_t kflags;	//是否是内核池

	if (phy_addr >= user_memory_pool.phy_addr_start) {
		//用户内存池
		kflags = 0;
	} else {
		//内核内存池
		kflags = 1;
	}

	while (n < pages) {
		phy_addr = page_addrV2P(vir_addr);

		if (kflags) {
			//对应的判断

		} else {
			//对应的判断
			
		}

		pool_freePhyMem(phy_addr);
		page_tableRemovePTE(vir_addr);

		n++;
		vir_addr += PAGE_SIZE;
	}
	free_pageVirtualAddr(pf, vaddr, pages);
}


/*
 * 功能: 分配n个内核页，返回虚拟地址
 * 参数: pages 	需要多少个页
 * 返回: NULL 失败，非NULL就是我们分配的虚拟地址
 * 说明: 有了虚拟地址分配函数后，就可以通过以页框为单位的方式编写内存管理算法
 */
void *alloc_kernelPage(uint32_t pages)
{
	void *vaddr = page_allocMemory(PF_KERNEL, pages);
	if (vaddr != NULL) {
		memset(vaddr, 0, pages*PAGE_SIZE);
	}
	return vaddr;
}

/*
 * 功能: 释放n个物理页
 * 参数: vaddr 	虚拟地址
 * 		pages 	需要多少个页
 * 返回: 无
 * 说明: 为了和alloc_kernelPage对应，这里写了一个释放内核的地址
 */
void free_kernelPage(void *vaddr, uint32_t pages)
{
	page_freeMemory(PF_KERNEL, vaddr, pages);
}
