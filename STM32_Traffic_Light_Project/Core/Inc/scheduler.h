#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define SCH_MAX_TASKS           20
#define NO_TASK_ID              0

typedef struct {
    void     (*pTask)(void);
    uint32_t Delay;
    uint32_t Period;
    uint8_t  RunMe;
    uint32_t TaskID;
} sTask;

void SCH_Init(void);
void SCH_Update(void);
uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD);
uint8_t SCH_Delete_Task(uint32_t taskID);
void SCH_Dispatch_Tasks(void);

#ifdef __cplusplus
}
#endif

#endif
