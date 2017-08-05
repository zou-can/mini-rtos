#include "tinyOS.h"
#include "ARMCM3.h"


void tTaskSched(void);


tTask *currentTask;
tTask *nextTask;
tTask *taskTable[2];
tTask *idleTask;

uint8_t schedLockCount;
void tTaskInit (tTask *task, void (*entry)(void *), void * param, tTaskStack * stack)
{
    *(--stack) = (unsigned long)(1 << 24);
    *(--stack) = (unsigned long)entry;
    *(--stack) = (unsigned long)0x14;
    *(--stack) = (unsigned long)0x12;
    *(--stack) = (unsigned long)0x3;
    *(--stack) = (unsigned long)0x2;
    *(--stack) = (unsigned long)0x1;
    *(--stack) = (unsigned long)param;

    *(--stack) = (unsigned long)0x11;
    *(--stack) = (unsigned long)0x10;
    *(--stack) = (unsigned long)0x9;
    *(--stack) = (unsigned long)0x8;
    *(--stack) = (unsigned long)0x7;
    *(--stack) = (unsigned long)0x6;
    *(--stack) = (unsigned long)0x5;
    *(--stack) = (unsigned long)0x4;

    task->stack = stack; 
    task->delayTicks = 0;
}

void tTaskSchedInit(void)
{
    schedLockCount = 0;
}

void tTaskSchedDisable(void)
{
    uint32_t key = tTaskEnterCritical();
    
    if (schedLockCount < 255) {
        schedLockCount++;
    }
    
    tTaskExitCritical(key);
}


void tTaskSchedEnable(void)
{
        uint32_t key = tTaskEnterCritical();
    
    if (schedLockCount > 0) {
        if (--schedLockCount == 0) {
            tTaskSched();
        }
    }
    
    tTaskExitCritical(key);
}

void tTaskSched(void)
{
    uint32_t key = tTaskEnterCritical();
    
    if (schedLockCount > 0) {
        tTaskExitCritical(key);
        return;
    }
    if (currentTask == idleTask) {
        if (taskTable[0]->delayTicks == 0) {
            nextTask = taskTable[0];
        }else if (taskTable[1]->delayTicks == 0) {
            nextTask = taskTable[1];
        } else {
            tTaskExitCritical(key);
            return;
        }
    } else {
        if (currentTask == taskTable[0]) {
            if (taskTable[1]->delayTicks == 0) {
                nextTask = taskTable[1];
            } else if (taskTable[0]->delayTicks != 0) {
                nextTask = idleTask;
            } else {
                tTaskExitCritical(key);
                return;
            }  
        } else if (currentTask == taskTable[1]){
            if (taskTable[0]->delayTicks == 0) {
                nextTask = taskTable[0];
            } else if (taskTable[1]->delayTicks != 0) {
                nextTask = idleTask;
            } else {
                tTaskExitCritical(key);
                return;
            }
        } else {
            tTaskExitCritical(key);
            return;
        }
    } 
  
    tTaskSwitch();
    tTaskExitCritical(key);  
}

void tTaskSystemTickHandler()
{
    uint32_t key = tTaskEnterCritical();
    int i;
    for(i = 0; i < 2; i++) {
        if (taskTable[i]->delayTicks > 0) {
            taskTable[i]->delayTicks--;
        }
    }
    tTaskExitCritical(key);
    tTaskSched();
    
}

void tTaskDelay(uint32_t delay)
{
    uint32_t key = tTaskEnterCritical();
    currentTask->delayTicks = delay;
    tTaskSched();
    tTaskExitCritical(key);
}
void tSetSusTickPeriod(uint32_t ms)
{
    SysTick->LOAD = ms * SystemCoreClock / 1000 - 1;
    NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | 
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler()
{
    tTaskSystemTickHandler();
}

void delay(int count)
{
    while(--count > 0);
}


int fitstSet;

int task1flag;
void task1Entry (void *param)
{

    tBitmap bitmap;
    int i = 0;
    
    tBitmapInit(&bitmap);
    
    for (i = tBitmaposCount() - 1; i>=0;i--) {
        tBitmapSet(&bitmap,i);
        
        fitstSet = tBitmapGetFirstSet(&bitmap);
    }
    
        for (i = 0; i < tBitmaposCount();i++) {
        tBitmapClear(&bitmap,i);
        
        fitstSet = tBitmapGetFirstSet(&bitmap);
    }
    
    for (;;) {
        
        task1flag = 0;
        tTaskDelay(1);
        task1flag = 1;
        tTaskDelay(1);
    }
}


int task2flag;
void task2Entry (void *param)
{
    for (;;) {
        task2flag = 0;
        tTaskDelay(1);
        task2flag = 1;
        tTaskDelay(1);
    }
}

tTask tTask1;
tTask tTask2;


tTaskStack task1Env[1024];
tTaskStack task2Env[1024];

tTask tTaskIdle;
tTaskStack idleTaskEnv[1024];

void idleTaskEntry(void *param)
{
    for (;;)
    {
    }
}

int main ()
{
    tTaskSchedInit();
    tTaskInit (&tTask1, task1Entry, (void *)(0x11111111), &task1Env[1024]);
    tTaskInit (&tTask2, task2Entry, (void *)(0x22222222), &task2Env[1024]);
    tTaskInit (&tTaskIdle, idleTaskEntry, (void *)(0), &idleTaskEnv[1024]);

    taskTable[0] = &tTask1;
    taskTable[1] = &tTask2;
    idleTask     = &tTaskIdle;
    
    nextTask  = taskTable[0];
    
    tTaskRunFrist();

    return 0;
}
