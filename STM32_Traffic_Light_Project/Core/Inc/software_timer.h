#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MAX_TIMERS              4
#define TIMER_TICK_MS           10

#define TIMER_PHASE             0
#define TIMER_1SEC              1
#define TIMER_BLINK             2
#define TIMER_POPUP             3

extern uint8_t  timer_flag[MAX_TIMERS];
extern uint32_t timer_counter[MAX_TIMERS];

void setTimer(uint8_t index, uint32_t duration_ms);
void clearTimer(uint8_t index);
uint8_t isTimerExpired(uint8_t index);
void timer_run(void);
void timerRun(void);

#ifdef __cplusplus
}
#endif

#endif
