/*
 * file:		kernel/sleep_wakeup.c
 * auther:	    Jason Hu
 * time:		2019/8/8
 * copyright:	(C) 2018-2019 by Book OS developers. All rights reserved.
 */

#include <book/schedule.h>
#include <book/arch.h>
#include <book/debug.h>
#include <share/string.h>
#include <book/task.h>
#include <driver/timer.h>
#include <driver/clock.h>


/**
 * TaskWakeup - 唤醒任务
 * @task: 需要唤醒的任务
 */
PUBLIC void TaskWakeUp(struct Task *task)
{
    if (task->status == TASK_BLOCKED) {    
        /* 唤醒任务就是解除任务的阻塞 */
        TaskUnblock(task);
    }
}

/**
 * TaskSleepOn - 任务休眠
 * @task: 需要休眠的任务
 */
PUBLIC void TaskSleepOn(struct Task *task)
{
    // 先关闭中断，并且保存中断状态
    enum InterruptStatus oldStatus = InterruptDisable();
    
    //printk(PART_TIP "task %s blocked with status %d\n", current->name, state);
    task->status = TASK_BLOCKED;
    
    /* 如果是当前任务就调度 */
    if (task == CurrentTask()) {
        Schedule();
    }
    
    // 恢复之前的状态
    InterruptSetStatus(oldStatus);
}
/**
 * TaskTimeout - 任务超时，就接触任务阻塞
 * @data: 超市的数据
 */
PRIVATE void TaskTimeout(uint32_t data)
{
    /* wakeup任务 */
    TaskWakeUp((struct Task *)data);
} 

/**
 * TaskSleep - 任务休眠（ticks为单位）
 * @ticks: 休眠的ticks数
 */
PUBLIC void TaskSleep(uint32_t ticks)
{
    /* 保存状态并关闭中断 */
    enum InterruptStatus oldStatus = InterruptDisable();
    
    struct Task *current = CurrentTask();
    /* 添加一个定时器来唤醒当前任务 */
    struct Timer timer;
    /* 初始化定时器 */
    TimerInit(&timer, ticks, (uint32_t)current, TaskTimeout);
    /* 添加定时器 */
    AddTimer(&timer);

    /* 设置成阻塞状态 */
    current->status = TASK_BLOCKED;

    /* 恢复之前中断状态 */
    InterruptSetStatus(oldStatus);

    /* 调度到其他进程 */
    Schedule();

    /* 现在进程不能运行，因为它不再readyList中，只有定时器唤醒后才可以
    当定时器把它唤醒之后，他就会在这里执行
     */
}

/**
 * SysSleep - 休眠（秒为单位）
 * @second: 秒数
 */
PUBLIC int SysSleep(uint32_t second)
{
    /* 把秒转换成ticks */
    uint32_t ticks = second * HZ;
    TaskSleep(ticks);
    return 0;
}