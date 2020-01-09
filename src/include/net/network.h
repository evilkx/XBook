/*
 * file:		include/net/network.h
 * auther:		Jason Hu
 * time:		2019/12/31
 * copyright:	(C) 2018-2019 by Book OS developers. All rights reserved.
 */

/*
网络层之间传输数据的缓冲区
*/

#ifndef _NET_NETWORK_H
#define _NET_NETWORK_H

#include <share/stdint.h>
#include <share/types.h>

/***** 配置区域 *****/

/* 回环测试 */
//#define _LOOPBACL_DEBUG

/* 网卡配置 */
#define _NIC_RTL8139
//#define _NIC_AMD79C973

/* 虚拟机配置 */
//#define _VM_VMWARE
//#define _VM_VBOX
#define _VM_QEMU

/* 网络协议 */
#define PROTO_IP            0x0800
#define PROTO_ARP           0x0806


/*备注以太网的情况*/
#define ETH_ALEN 6 /*以太网地址，即MAC地址，6字节*/
#define ETH_HLEN 14 /*以太网头部的总长度*/
#define ETH_ZLEN 60 /*不含CRC校验的数据最小长度*/
#define ETH_DATA_LEN 1500 /*帧内数据的最大长度*/
#define ETH_FRAME_LEN 1514 /*不含CRC校验和的最大以太网数据长度*/




PUBLIC unsigned int NetworkMakeIpAddress(
    unsigned char ip0,
    unsigned char ip1,
    unsigned char ip2, 
    unsigned char ip3);

PUBLIC int InitNetwork();
PUBLIC unsigned int NetworkMakeIpAddress(
    unsigned char ip0,
    unsigned char ip1,
    unsigned char ip2, 
    unsigned char ip3);

PUBLIC void NetworkSetAddress(unsigned int ip);
PUBLIC unsigned int NetworkGetAddress();
PUBLIC void DumpIpAddress(unsigned int ip);


#endif   /* _NET_NETWORK_H */
