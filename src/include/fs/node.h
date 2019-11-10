/*
 * file:		include/fs/node.h
 * auther:		Jason Hu
 * time:		2019/11/3
 * copyright:	(C) 2018-2019 by Book OS developers. All rights reserved.
 */

#ifndef _FS_NODE_H
#define _FS_NODE_H

#include <share/stdint.h>
#include <share/types.h>
#include <book/device.h>

#include <fs/file.h>
#include <fs/super_block.h>

#define NODE_FILE_BLOCK_PTR_NR  15     /* 文件块指针的数量 */

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

    unsigned int blocks[NODE_FILE_BLOCK_PTR_NR];    /* 数据区域 */
    
    unsigned char reserved[NODE_FILE_RESERVED];    /* 保留区域 */
} __attribute__ ((packed));

#define SIZEOF_NODE_FILE sizeof(struct NodeFile)

void NodeFileInit(struct NodeFile *node,
	dev_t devno,
    unsigned int id);
int CleanNodeFile(struct NodeFile *node, struct SuperBlock *sb);
PUBLIC int SyncNodeFile(struct NodeFile *node, struct SuperBlock *sb);
void DumpNodeFile(struct NodeFile *node);
struct NodeFile *CreateNodeFile(char *name,
	char attr, struct SuperBlock *sb);
struct NodeFile *GetNodeFileByName(struct Directory *dir, char *name);
PUBLIC int LoseNodeFile(struct NodeFile *node, struct SuperBlock *sb, char depth);
PUBLIC void CloseNodeFile(struct NodeFile *node);

PUBLIC int NodeFileWrite(struct FileDescriptor *pfd, void *buffer, size_t count);
PUBLIC int NodeFileRead(struct FileDescriptor *pfd, void *buffer, size_t count);

PUBLIC int GetBlockByBlockIndex(struct NodeFile *node,
	unsigned int index,
	unsigned int *block,
	struct SuperBlock *sb);
PUBLIC int PutBlockByBlockIndex(struct NodeFile *node,
	unsigned int index,
	struct SuperBlock *sb);
PUBLIC int ReleaseNodeFileData(struct SuperBlock *sb,
	struct NodeFile *node);

#endif	/* _FS_NODE_H */

