#ifndef  TINY_OS_H
#define  TINY_OS_H

#include <stdint.h>

#include "tLib.h"
#include "tConfig.h"

typedef uint32_t tTaskStack;

typedef struct _tTask {
    tTaskStack *stack;
    uint32_t    delayTicks;
    uint32_t prio;
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
#endif
