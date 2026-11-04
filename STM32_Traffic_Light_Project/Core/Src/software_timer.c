#include "software_timer.h"

uint8_t  timer_flag[MAX_TIMERS]    = {0};
uint32_t timer_counter[MAX_TIMERS] = {0};

void setTimer(uint8_t index, uint32_t duration_ms)
{
    if (index >= MAX_TIMERS)
    {
        return;
    }

    if (duration_ms >= TIMER_TICK_MS)
    {
        timer_counter[index] = duration_ms / TIMER_TICK_MS;
    }
    else if (duration_ms > 0)
    {
        timer_counter[index] = duration_ms;
    }
    else
    {
        timer_counter[index] = 0;
    }

    timer_flag[index] = 0;
}

void clearTimer(uint8_t index)
{
    if (index >= MAX_TIMERS)
    {
        return;
    }

    timer_counter[index] = 0;
    timer_flag[index]    = 0;
}

uint8_t isTimerExpired(uint8_t index)
{
    if (index >= MAX_TIMERS)
    {
        return 0;
    }

    if (timer_flag[index] == 1)
    {
        timer_flag[index] = 0;
        return 1;
    }

    return 0;
}

void timer_run(void)
{
    for (uint8_t i = 0; i < MAX_TIMERS; i++)
    {
        if (timer_counter[i] > 0)
        {
            timer_counter[i]--;
            if (timer_counter[i] == 0)
            {
                timer_flag[i] = 1;
            }
        }
    }
}

void timerRun(void)
{
    timer_run();
}
