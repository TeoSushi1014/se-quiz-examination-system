#pragma once

#ifndef TIMER_H
#define TIMER_H

#include <ctime>

class Timer 
{
private:
    time_t startTime;
    int limitSeconds;
    bool started;

public:
    Timer();
    void start(int limitSeconds);
    bool isTimeUp() const;
    int getRemainingTime() const;
};

#endif
