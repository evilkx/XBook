﻿/*
 * file:		include/fs/file.h
 * auther:		Jason Hu
 * time:		2019/10/25
 * copyright:	(C) 2018-2019 by Book OS developers. All rights reserved.
 */

#ifndef _FS_FILE
#define _FS_FILE

#include <share/const.h>
#include <share/stddef.h>
#include <book/device.h>

#include <fs/flat.h>

enum {
    FILE_TYPE_DEVICE = 1,
    FILE_TYPE_NODE,
};

#define FILE_ATTR_R     0x01
#define FILE_ATTR_W     0x02
#define FILE_ATTR_X     0x04

/**
 * 文件
 * size:106
 */
struct File {
    char name[FILE_NAME_LEN];       /* 文件的名字 */
    char type;                      /* 文件的类型 */
    char attr;                      /* 文件的属性 */
    size_t size;                    /* 文件的大小 */
} __attribute__ ((packed));

#define SIZEOF_FILE sizeof(struct File)

/**
 * 设备文件
 */
struct DeviceFile {
    struct File super;  /* 继承文件 */

    struct List list;               /* 设备文件在设备目录下面的链表 */
    struct Directory *directory;    /* 设备文件对应的设备目录指针 */
    struct Device *device;          /* 设备文件对应的设备指针 */
};
#define SIZEOF_DEVICE_FILE sizeof(struct DeviceFile)

/* 文件块指针的数量 */
#define NODE_FILE_BLOCK_PRT_NR  15
#define NODE_FILE_RESERVED      (256 - 76 - SIZEOF_FILE)

/* 默认的节点文件数 */
#define DEFAULT_NODE_FILE_NR         (4096*2)

/**
 * 数据文件
 * 
 * size:19*4 = 76字节
 */
struct NodeFile {
    struct File super;      /* 继承文件 */
    dev_t devno;            /* 设备号 */
    unsigned int id;        /* id号 */
	unsigned int crttime;	/* 创建时间 */
	unsigned int mdftime;	/* 修改时间 */
	unsigned int acstime;	/* 访问时间 */

    unsigned int block[NODE_FILE_BLOCK_PRT_NR];    /* 数据区域 */
    
    unsigned char reserved[NODE_FILE_RESERVED];    /* 保留区域 */
} __attribute__ ((packed));

#define SIZEOF_NODE_FILE sizeof(struct NodeFile)

PUBLIC struct File *CreateFile(char *name, char type, char attr);
PUBLIC int MakeDeviceFile(char *dname,
	char *fname,
	char type,
	struct Device *device);
PUBLIC void DumpDeviceFile(struct DeviceFile *devfile);
PUBLIC struct DeviceFile *GetDeviceFileByName(struct List *deviceList, char *name);


/* 系统中最多可以打开的文件数量 */
#define MAX_OPEN_FILE_NR    64 

/**
 * 文件描述符
 * 用于描述一个文件打开的信息
 */
struct FileDescriptor {
    struct File *file;      /* 描述的文件 */
    unsigned int pos;	    /* 文件读写位置 */
	unsigned int flags; 	/* 文件操作标志 */
	struct Directory *dir;  /* 文件所在的目录 */
};

PUBLIC void InitFileDescriptor();

PUBLIC int FlatOpen(const char *path, int flags);
PUBLIC int FlatClose(int fd);
PUBLIC int FlatWrite(int fd, void* buf, size_t count);
PUBLIC int FlatRead(int fd, void* buf, size_t count);
PUBLIC off_t FlatLseek(int fd, off_t offset, char whence);
PUBLIC int FlatIoctl(int fd, unsigned int cmd, unsigned int arg);

void DumpFileDescriptor(int fd);

#endif  /* _FS_FILE */
