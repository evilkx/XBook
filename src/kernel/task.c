/*
 * file:		kernel/task.c
 * auther:	    Jason Hu
 * time:		2019/7/30
 * copyright:	(C) 2018-2019 by Book OS developers. All rights reserved.
 */

#include <book/task.h>
#include <book/arch.h>
#include <book/debug.h>
#include <share/string.h>
#include <book/schedule.h>
#include <book/vmarea.h>
#include <book/sync_lock.h>
#include <book/vmspace.h>
#include <user/stdlib.h>
#include <user/conio.h>
#include <book/semaphore.h>

/**
 * SwitchToUser - 跳转到用户态执行的开关 
 * @frame: 中断栈
 * 
 * 通过准备好的中断栈，进入到用户态执行
 */
EXTERN void UpdateTssInfo(struct Task *task);

PRIVATE pid_t nextPid;

PUBLIC Task_t *mainThread;

// PUBLIC Task_t *currentTask;

/* 初始化链表头 */

// 就绪队列链表
PUBLIC LIST_HEAD(taskReadyList);
// 全局队列链表，用来查找所有存在的任务
PUBLIC LIST_HEAD(taskGlobalList);

/**
 * KernelThread - 执行内核线程
 * @function: 要执行的线程
 * @arg: 参数
 * 
 * 改变当前的执行流，去执行我们选择的内核线程
 */
PRIVATE void KernelThread(ThreadFunc *function, void *arg)
{
    EnableInterrupt();
    function(arg);
}

/**  
 * AllocatePid - 分配一个pid
 */
PRIVATE pid_t AllocatePid()
{
    return nextPid++;
}

/**  
 * ForkPid - 分配一个pid
 * 
 * 调用私有的AllocatePid获取一个pid
 */
PUBLIC pid_t ForkPid()
{
    return AllocatePid();
}
/**
 * CreatePageDir - 创建页目录   
 */
PUBLIC uint32_t *CreatePageDir()
{
    // 分配一个页来当作页目录
    uint32_t *pageDirAddr = kmalloc(PAGE_SIZE);

    if (!pageDirAddr) {
        printk(PART_WARRING "kmalloc for CreatePageDir failed!\n");
        return NULL;
    }
    memset(pageDirAddr, 0, PAGE_SIZE);
    
    /* 复制页表内容,只复制内核部分 */
    memcpy((void *)((unsigned char *)pageDirAddr + 1024*3), 
            (void *)(PAGE_DIR_VIR_ADDR + 1024*3), 1024);

    /* 更新页目录表的页目录的物理地址，因为页目录表的最后一项时页目录表的物理地址 */
    uint32_t paddr = PageAddrV2P((uint32_t )pageDirAddr);
    /* 属性是 存在，系统，可写 */
    pageDirAddr[1023] = paddr | PAGE_P_1 | PAGE_US_S | PAGE_RW_W;
    return pageDirAddr;
}

/**
 * ThreadCreate - 创建一个线程
 * @thread: 线程结构体
 * @function: 要去执行的函数
 * @arg: 参数
 */
PRIVATE void ThreadCreate(struct Task *thread, ThreadFunc function, void *arg)
{
    /* 预留中断栈 */
    thread->kstack -= sizeof(struct TrapFrame);

    /* 预留线程栈 */
    thread->kstack -= sizeof(struct ThreadStack);
    /* 填写线程栈信息 */
    struct ThreadStack *threadStack = (struct ThreadStack *)thread->kstack;

    // 在KernelThread中去改变执行流，从而可以传递一个参数
    threadStack->eip = KernelThread;
    threadStack->function = function;
    threadStack->arg = arg;
    threadStack->ebp = threadStack->ebx = \
    threadStack->esi = threadStack->edi = 0;
}
/**
 * TaskInit - 初始化线程
 * @thread: 线程结构地址
 * @name: 线程的名字
 * @priority: 线程优先级
 */
PRIVATE void TaskInit(struct Task *thread, char *name, int priority)
{
    memset(thread, 0, sizeof(struct Task));
    // 复制名字
    strcpy(thread->name, name);

    // mainThread 最开始就是运行的
    if (thread == mainThread)
        thread->status = TASK_RUNNING;
    else
        thread->status = TASK_READY;
        
    // 设置优先级
    thread->priority = priority;
    thread->ticks = priority;
    thread->elapsedTicks = 0;
    thread->pgdir = NULL;

    thread->pid = AllocatePid();
    thread->parentPid = -1;
    thread->exitStatus = -1;
    // 设置中断栈为当前线程的顶端
    thread->kstack = (uint8_t *)(((uint32_t )thread) + PAGE_SIZE);
}
/**
 * AllocTaskMemory - 初始化任务的内存管理
 * @task: 任务
 */
PUBLIC void AllocTaskMemory(struct Task *task)
{
    // 初始化内存管理器
    task->mm = (struct MemoryManager *)kmalloc(sizeof(struct MemoryManager));
    if (!task->mm)
        Panic(PART_ERROR "kmalloc for task mm failed!\n"); 
    InitMemoryManager(task->mm);

}
/**
 * TaskMemoryInit - 初始化任务的内存管理
 * @task: 任务
 */
PUBLIC void FreeTaskMemory(struct Task *task)
{
    kfree(task->mm);
}

/**
 * ThreadStart - 开始一个线程
 * @name: 线程的名字
 * @func: 线程入口
 * @arg: 线程参数
 */
PUBLIC struct Task *ThreadStart(char *name, int priority, ThreadFunc func, void *arg)
{
    // 创建一个新的线程结构体
    struct Task *thread = (struct Task *) kmalloc(PAGE_SIZE);
    if (!thread)
        return NULL;
    // 初始化线程
    TaskInit(thread, name, priority);
    
    // TaskMemoryInit(thread);

    //printk("alloc a thread at %x\n", thread);
    // 创建一个线程
    ThreadCreate(thread, func, arg);

    /* 操作链表时关闭中断，结束后恢复之前状态 */
    enum InterruptStatus oldStatus = InterruptDisable();

    // 保证不存在于链表中
    ASSERT(!ListFind(&thread->list, &taskReadyList));
    // 添加到就绪队列
    ListAddTail(&thread->list, &taskReadyList);

    // 保证不存在于链表中
    ASSERT(!ListFind(&thread->globalList, &taskGlobalList));
    // 添加到全局队列
    ListAddTail(&thread->globalList, &taskGlobalList);
    
    InterruptSetStatus(oldStatus);
    return thread;
}
/**
 * CurrentTask - 获取当前运行的任务
 * 
 * 通过esp来计算出任务结构体
 */
PUBLIC Task_t *CurrentTask()
{
    uint32_t esp;
    // 获取esp的值
    asm ("mov %%esp, %0" : "=g" (esp));
    
    /* 
    由于是在内核态，所以esp是内核态的值
    取esp整数部分并且减去一个PAGE即pcb起始地址
    内核栈，我们约定2个页的大小
    */
    return (Task_t *)(esp & (~(4095UL)));
}

/**
 * MakeMainThread - 为内核主线程设定身份
 * 
 * 内核主线程就是从boot到现在的执行流。到最后会演变成idle
 * 在这里，我们需要给与它一个身份，他才可以进行多线程调度
 */
PRIVATE void MakeMainThread()
{
    // 当前运行的就是主线程
    mainThread = CurrentTask();
    // 为线程设置信息
    TaskInit(mainThread, "main", 3);
    
    // TaskMemoryInit(mainThread);

    // 保证不存在链表中
     ASSERT(!ListFind(&mainThread->globalList, &taskGlobalList));
    // 添加到全局的队列，因为在运行，所以没有就绪
     ListAddTail(&mainThread->globalList, &taskGlobalList);
}

/**
 * TaskBlock - 把任务阻塞
 */
PUBLIC void TaskBlock(enum TaskStatus state)
{
    /*
    state有3种状态，分别是TASK_BLOCKED, TASK_WAITING, TASK_ZOMBIE
    它们不能被调度
    */
    ASSERT((state == TASK_BLOCKED) || 
            (state == TASK_WAITING) || 
            (state == TASK_ZOMBIE));

    // 先关闭中断，并且保存中断状态
    enum InterruptStatus oldStatus = InterruptDisable();
    
    // 改变状态
    struct Task *current = CurrentTask();
    //printk(PART_TIP "task %s blocked with status %d\n", current->name, state);
    current->status = state;
    
    // 调度到其它任务
    Schedule();

    // 恢复之前的状态
    InterruptSetStatus(oldStatus);
}   

/**
 * TaskUnblock - 解除任务阻塞
 * @task: 要解除的任务
 */
PUBLIC void TaskUnblock(struct Task *task)
{
    // 先关闭中断，并且保存中断状态
    enum InterruptStatus oldStatus = InterruptDisable();
    
    /*
    state有2种状态，分别是TASK_BLOCKED, TASK_WAITING
    只有它们能被唤醒, TASK_ZOMBIE只能阻塞，不能被唤醒
    */
    ASSERT((task->status == TASK_BLOCKED) || 
            (task->status == TASK_WAITING));

    // 没有就绪才能够唤醒，并且就绪
    if (task->status != TASK_READY) {
        // 保证没有在就绪队列中
        ASSERT(!ListFind(&task->list, &taskReadyList));
        
        // 已经就绪是不能再次就绪的
        if (ListFind(&task->list, &taskReadyList)) {
            Panic("TaskUnblock: task has already in ready list!\n");
        }
        // 把任务放在最前面，让它快速得到调度
        ListAdd(&task->list, &taskReadyList);
        // 处于就绪状态
        task->status = TASK_READY;
    }
    // 恢复之前的状态
    InterruptSetStatus(oldStatus);
}

/**
 * StartProcess - 开始运行一个进程
 * @fileName: 进程的文件名
 */
PRIVATE void StartProcess(void *fileName)
{
    /* 开启进程的时候，需要去执行init程序，所以这里
    调用execv来完成这个工作 */
    SysExecv("init", NULL);
    /* 如果运行失败就停在这里 */
    Panic("start init failed!\n");
}

/**
 * PageDirActive - 激活任务的页目录
 * @task: 任务
 */
PUBLIC void PageDirActive(struct Task *task)
{
    /* 进程线程都要重新设置cr3 
    但是有一个比较好一点的想法就是，
    检查前一个进程或线程，如果同属于一个页目录空间
    那么就不用重新设置cr3.这只是个猜想。^.^ 2019.8.1
    */
    uint32_t pageDirPhysicAddr = PAGE_DIR_PHY_ADDR;

    // 内核线程的页目录表是一样的，所以不会出现在pgdir里面
    if (task->pgdir != NULL) {
        // 获取转换后的地址
        pageDirPhysicAddr = PageAddrV2P((uint32_t )task->pgdir);
        //printk("task %s active pgdir\n", task->name);
    }
    // 修改cr3的值，切换页目录表    
    WriteCR3(pageDirPhysicAddr);
}

/**
 * TaskActivate - 激活任务
 * @task: 要激活的任务
 */
PUBLIC void TaskActivate(struct Task *task)
{
    /* 任务不能为空 */
    ASSERT(task != NULL);

    /* 激活任务的页目录表 */
    PageDirActive(task);

    /* 内核线程特权级为0，处理器进入中断时不会从tss中获取esp0，
    所以不需要更新tss的esp0，用户进程则需要*/
    if (task->pgdir) {
        UpdateTssInfo(task);
    }
}

/**
 * TaskExecute - 执行一个任务
 * @fileName: 任务的文件名
 * @name: 任务的名字
 */
PUBLIC struct Task *TaskExecute(void *fileName, char *name)
{
    // 创建一个新的线程结构体
    struct Task *thread = (struct Task *) kmalloc(PAGE_SIZE);
    if (!thread)
        return NULL;
    // 初始化线程
    TaskInit(thread, name, TASK_DEFAULT_PRIO);
    
    AllocTaskMemory(thread);
    // 创建页目录
    thread->pgdir = CreatePageDir();
    if (!thread->pgdir)
        return NULL;
    
    //printk("alloc a thread at %x\n", thread);
    // 创建一个线程
    ThreadCreate(thread, StartProcess, fileName);
    
    /* 操作链表时关闭中断，结束后恢复之前状态 */
    enum InterruptStatus oldStatus = InterruptDisable();

    // 保证不存在于链表中
    ASSERT(!ListFind(&thread->list, &taskReadyList));
    // 添加到就绪队列
    ListAddTail(&thread->list, &taskReadyList);

    // 保证不存在于链表中
    ASSERT(!ListFind(&thread->globalList, &taskGlobalList));
    // 添加到全局队列
    ListAddTail(&thread->globalList, &taskGlobalList);
    
    InterruptSetStatus(oldStatus);
    return thread;
}

/**
 * TaskYield - 任务让出当前cpu占用
 * 
 * 相当于任务在这个调度轮回中不运行，下一个轮回才运行
 */
PUBLIC void TaskYield()
{
    struct Task *current = CurrentTask();
    /* 保存状态并关闭中断 */
    enum InterruptStatus oldStatus = InterruptDisable();
    // 保证不存在于链表中
    ASSERT(!ListFind(&current->list, &taskReadyList));
    // 添加到就绪队列
    ListAddTail(&current->list, &taskReadyList);

    /* 设置状态为就绪 */
    current->status = TASK_READY;

    /* 调度到其它任务，自己不运行 */
    Schedule();

    /* 当再次运行到自己的时候就恢复之前的状态 */
    InterruptSetStatus(oldStatus);
}

/**
 * SysGetPid - 获取任务id
 */
PUBLIC uint32_t SysGetPid()
{
    return CurrentTask()->pid;
}

PRIVATE struct SyncLock consoleLock;

PRIVATE void lockPrintk(char *buf)
{
    SyncLockAcquire(&consoleLock);
    printk(buf);

    SyncLockRelease(&consoleLock);
}

int testA = 0, testB = 0;
void ThreadA(void *arg)
{
    char *par = arg;
    int i = 0;
    while (1) {
        i++;
        if (i%0xf00000 == 0) {
            //lockPrintk("%x ", testA);
            //lockPrintk(par);
            
        }
    }
}
void ThreadB(void *arg)
{
    char *par = arg;
    int i = 0;
    // log("hello\n");
    while (1) {
        i++;
        if (i%0xf00000 == 0) {
            //lockPrintk(par);
            //SysMSleep(3000);
            //printk("%x ", testB);
        }
    }
}

void TaskInitEntry()
{
    printf("in task init pid %d\n", getpid());
    
    while (1) {
        testA++;
    }
}

void TaskTestEntry()
{
    printf("in task init pid %d\n", getpid());
    
    while (1) {
        testA++;
    }
}
/**
 * InitTasks - 初始化多任务环境
 */
PUBLIC void InitTasks()
{
    PART_START("Task");
    nextPid = 0;

    /* 最开始初始化init进程，让它的pid为0 */
    TaskExecute(0, "init");
    
    SyncLockInit(&consoleLock);
    
    MakeMainThread();
    
    ThreadStart("testa", 1, ThreadA, "<11111111111111111111111111111111111111111111111111111> ");
    ThreadStart("testb", 1, ThreadB, "<22222222222222222222222222222222222222222222222222222> ");

    //TaskExecute(TaskTestEntry, "test");
    
    PART_END();
}
