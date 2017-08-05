#ifndef  TINY_OS_H
#define  TINY_OS_H

#include <stdint.h>

#include "tLib.h"
#include "tConfig.h"

#define TINYOS_TASK_STATE_RDY        0
#define TINYOS_TASK_STATE_DISTORYED (1 << 1)
#define TINYOS_TASK_STATE_DELAYED   (1 << 2)
#define TINYOS_TASK_STATE_SUSPEND   (1 << 3)
 
typedef uint32_t tTaskStack;

typedef struct _tTask {
    tTaskStack *stack;
    tNode linkNode;
    uint32_t    delayTicks;
    tNode    delayNode;
    uint32_t prio;
    uint32_t state;
    uint32_t slice;
    uint32_t suspendCount;
    
    void (*clean) (void *param);
    void *clearParam;
    uint8_t requestDeleteFlag;
}tTask;


extern tTask *currentTask;
extern tTask *nextTask;

void tTaskRunFrist(void);
void tTaskSwitch(void);

uint32_t tTaskEnterCritical(void);
void tTaskExitCritical(uint32_t key);

void tTaskSchedInit(void);
void tTaskSchedDisable(void);
void tTaskSchedEnable(void);
void tTaskSchedRdy (tTask *task);
void tTaskSchedUnRdy (tTask *task);

void tTimeTaskWait(tTask *task,uint32_t ticks);
void tTimeTaskWakeUp(tTask *task);
void tTaskSchedRemove(tTask *task);
void tTimeTaskRemove (tTask *task);
void tTaskDelay(uint32_t delay);
void tTaskSched(void);
void tTaskSystemTickHandler(void);

typedef struct _tTaskInfo{
    uint32_t delayTicks;
    uint32_t prio;
    uint32_t state;
    uint32_t slice;
    uint32_t suspendCount;
}tTaskInfo;

void tTaskInit (tTask *task, void (*entry)(void *), void * param, uint32_t prio, tTaskStack * stack);
void tTaskSetClearFunc (tTask *task, void (*clean) (void *param), void *param);
void tTaskForceDelete (tTask *task);
void tTaskRequestDelete (tTask *task);
uint8_t tTaskIsRequestDeleted (void);
void tTaskDeleteSelf (void);
void tTaskSuspend(tTask *task);
void tTaskWakeUp(tTask *task);
void tTaskGetInfo (tTask *task, tTaskInfo *info);
    
void tSetSysTickPeriod(uint32_t ms);
#endif
