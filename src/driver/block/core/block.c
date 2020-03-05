/*
 * file:	    block/core/block.c
 * auther:	    Jason Hu
 * time:	    2019/10/13
 * copyright:	(C) 2018-2020 by Book OS developers. All rights reserved.
 */

#include <book/config.h>
#include <book/debug.h>
#include <lib/string.h>
#include <book/device.h>
#include <book/task.h>
#include <clock/clock.h>
#include <block/block.h>
#include <block/blk-buffer.h>
#include <block/blk-request.h>
#include <block/blk-dev.h>
#include <block/blk-disk.h>
//#include <block/virtual/ramdisk.h>
//#include <block/ide/ide.h>


/* ----驱动程序初始化文件导入---- */
EXTERN int InitIdeDriver();
EXTERN void ExitIdeDriver();
EXTERN int InitRamdiskDriver();
EXTERN void ExitRamdiskdDriver();
/* ----驱动程序初始化文件导入完毕---- */

//#define _DEBUG_TEST

EXTERN struct List allDiskList;
EXTERN struct List allBlockDeviceList;

/**
 * BlockDiskSync - 把磁盘数据同步到磁盘上
 * 
 * 这个函数每秒被调用一次，在clock.c中的WorkForPerSecond工作中进行。
 */
PUBLIC void BlockDiskSync()
{
    /* 执行块同步操作 */
    int count = Bsync();
    if (count) {
        /* 显示同步信息，有助于调试 */
        //printk(">>>sync disk for %d count.\n", count);
    }
}

void ThreadReadTest(void *arg)
{
    char *blkbuf = kmalloc(BLOCK_SIZE, GFP_KERNEL);
    if (blkbuf == NULL) {
        Panic("alloc blkbuf failed!\n");
    }
    int i = 0;
    char dir = 1;
    struct BufferHead *wbh;

    while (1) {
        i += dir;

        if (i > 5) {
            dir = -dir;
        } 
        if (i <= 0) {
            dir = -dir;
        }

        wbh = Bread(DEV_RDA, i);
        if (wbh == NULL)
            printk("read failed!\n");
    }
}

void ThreadReadTest2(void *arg)
{
    char *blkbuf = kmalloc(BLOCK_SIZE, GFP_KERNEL);
    if (blkbuf == NULL) {
        Panic("alloc blkbuf failed!\n");
    }
    int i = 5;
    char dir = 1;
    struct BufferHead *wbh;

    while (1) {
        i += dir;

        if (i > 10) {
            dir = -dir;
        } 
        if (i <= 5) {
            dir = -dir;
        }
        memset(blkbuf, i, BLOCK_SIZE);
        wbh = Bwrite(DEV_RDA, i, blkbuf);

        wbh = Bread(DEV_RDA, i);
        if (wbh == NULL)
            printk("read failed!\n");
    }
}

void ThreadReadTest3(void *arg)
{
    char *blkbuf = kmalloc(BLOCK_SIZE, GFP_KERNEL);
    if (blkbuf == NULL) {
        Panic("alloc blkbuf failed!\n");
    }
    int i = 5;
    char dir = -1;
    struct BufferHead *wbh;

    while (1) {
        i += dir;
        
        if (i > 5) {
            dir = -dir;
        }
        if (i <= 0) {
            dir = -dir;
        }
        
        memset(blkbuf, i, BLOCK_SIZE);
        wbh = Bwrite(DEV_HDB, i, blkbuf);

        wbh = Bread(DEV_RDA, i);
        if (wbh == NULL)
            printk("read failed!\n");
    }
}

/**
 * BlockDeviceTest - 对块设备进行测试
 * 
 */
PUBLIC void BlockDeviceTest()
{
    #ifdef _DEBUG_TEST
    ThreadStart("A", 3, ThreadReadTest, "NULL");
    ThreadStart("B", 3, ThreadReadTest2, "NULL");
    ThreadStart("C", 3, ThreadReadTest3, "NULL");
    
    struct BufferHead *bh = Bread(DEV_RDA, 0);
    if (bh == NULL)
        Panic("read bh failed!\n");
    DumpBH(bh);

    printk("data:%x-%x-%x-%x\n", bh->data[0], bh->data[511], bh->data[512], bh->data[1023]);
    
    bh = Bread(DEV_RDA, 0);
    if (bh == NULL) 
        Panic("read bh failed!\n");
    DumpBH(bh);

    printk("data:%x-%x-%x-%x\n", bh->data[0], bh->data[511], bh->data[512], bh->data[1023]);
    //Panic("test\n");

    char *blkbuf = kmalloc(BLOCK_SIZE, GFP_KERNEL);
    if (blkbuf == NULL) {
        Panic("alloc blkbuf failed!\n");
    }
    
    memset(blkbuf, 0XFA, BLOCK_SIZE);

    struct BufferHead *wbh;
    wbh = Bwrite(DEV_RDB, 0, blkbuf);
    if (bh == NULL)
        Panic("write bh failed!\n");
    DumpBH(wbh);

    printk("data:%x-%x-%x-%x\n", wbh->data[0], wbh->data[511], wbh->data[512], wbh->data[1023]);

    wbh = Bwrite(DEV_RDB, 1, blkbuf);
    if (bh == NULL)
        Panic("write bh failed!\n");
    DumpBH(wbh);

    printk("data:%x-%x-%x-%x\n", wbh->data[0], wbh->data[511], wbh->data[512], wbh->data[1023]);

     char *buf = kmalloc(SECTOR_SIZE * 2, GFP_KERNEL);
    if (buf == NULL) {
        return;
    }

    buf[0] = 0x11;
    buf[511] = 0x55;
    buf[512] = 0xaa;
    buf[1023] = 0xff;
    DeviceWrite(DEV_RDA, 0, buf, 2);
    memset(buf, 0, 1024);

    DeviceRead(DEV_RDA, 0, buf, 2);

    printk("buf:%x %x %x %x\n", buf[0], buf[511], buf[512], buf[1023]);
    
    DeviceOpen(DEV_RDA, 0);

    DeviceIoctl(DEV_RDA, RAMDISK_IO_CLEAN, 10);

    #endif
}

/**
 * InitBlockDevice - 初始化块设备层
 */
PUBLIC void InitBlockDevice()
{
#ifdef CONFIG_BLOCK_DEVICE
    
    #ifdef CONFIG_DRV_RAMDISK
    /* 初始化ramdisk驱动 */
    if (InitRamdiskDriver()) {
		Panic("init ramdisk failed!\n");	
	}
    #endif
    
    #ifdef CONFIG_DRV_IDE
    /* 初始化ide驱动 */
    if (InitIdeDriver()) {
		Panic("init ide failed!\n");	
	}
    #endif
    
    /* 创建一个线程来同步磁盘 */
    //ThreadStart("dflush", TASK_PRIORITY_BEST, ThreadDiskFlush, "NULL");
    
    BlockDeviceTest();
    
    /* 打印磁盘，并打印分区 */
    /*struct Disk *disk;

    ListForEachOwner(disk, &allDiskList, list) {
        //DumpDisk(disk);
    }
    struct BlockDevice *blkdev;
    
    ListForEachOwner(blkdev, &allBlockDeviceList, list) {
        DumpBlockDevice(blkdev);
        //DumpDiskPartition(blkdev->part);
    }*/
#endif
}
