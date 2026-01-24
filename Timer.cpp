#include "Timer.h"

Timer::Timer() 
{
    started = false;
    limitSeconds = 0;
}

void Timer::start(int seconds) 
{
    limitSeconds = seconds;
    startTime = time(NULL);
    started = true;
}

bool Timer::isTimeUp() const 
{
    if (!started) return false;
    return difftime(time(NULL), startTime) >= limitSeconds;
}

int Timer::getRemainingTime() const 
{
    if (!started) return limitSeconds;
    int passed = (int)difftime(time(NULL), startTime);
    int remain = limitSeconds - passed;
    return remain > 0 ? remain : 0;
}
