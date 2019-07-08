/*
 * file:		   include/book/hal.h
 * auther:		Jason Hu
 * time:		   2019/6/22
 * copyright:	(C) 2018-2019 by Book OS developers. All rights reserved.
 */

#ifndef _BOOK_HAL_H
#define _BOOK_HAL_H

#include <share/stdint.h>
#include <share/types.h>
#include <book/list.h>

/* 
 * 硬件抽象层 （Hardware Abstraction Layer）
 * 对硬件进行抽象管理，抽象出一套通用的方法。
 */

#define HAL_NAME_LEN 32
/*
 * 用名字来访问每一个hal，所以初始化的时候需要注册到队列中 
 * 但是每次访问都要检测并获取那个hal，所以，我想到了缓存，
 * 就是写成 键：值 的形式，这样就可以通过halName快速访问到对应的hal了
 * 
 * 约定：通过hal抽象的对象都要以对象名开头
 */

struct Hal  {
   struct HalOperate *halOperate;
   char *halName;
   char halType;
   char isOpened;
   //用链表把所有的hal链接起来。
   struct List halList;
};

struct HalOperate  {
   void (*Init)();   //初始化函数
   void (*Open)();   //打开函数
   int (*Read)(unsigned char *, unsigned int ); //读取函数
   int (*Write)(unsigned char *, unsigned int ); //写入函数
   void (*Close)();    //关闭函数
   void (*Ioctl)(unsigned int , unsigned int ); //控制设置
   void (*Destruct)(); //析构函数
   
};

PUBLIC void InitHalEnvironment();
PUBLIC void InitHalEarly();
PUBLIC void InitHalKernel();

PUBLIC int HalCreate(struct Hal *hal);
PUBLIC int HalDestory(char *name);

PUBLIC INLINE void HalOpen(char *name);
PUBLIC INLINE int HalRead(char *name,unsigned char *buffer, unsigned int count);
PUBLIC INLINE int HalWrite(char *name,unsigned char *buffer, unsigned int count);
PUBLIC INLINE void HalClose(char *name);
PUBLIC INLINE void HalIoctl(char *name,unsigned int cmd, unsigned int param);

/* 为一个hal结构赋值 */
#define HAL_INIT(op, name) {&(op), name, }

/* 创建一个hal并为他赋值 */
#define HAL(hal, op, name) \
      struct Hal hal = HAL_INIT(op, name)

#define HAL_EXTERN(hal) extern struct Hal hal


/* 导入hal头文件 */
#include <hal/char/clock.h>
#include <hal/char/cpu.h>
#include <hal/char/display.h>
#include <hal/block/ram.h>



#endif   /*_BOOK_HAL_H*/
