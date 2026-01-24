#pragma once

#ifndef ATTEMPT_H
#define ATTEMPT_H

#include "Timer.h"

class Attempt 
{
private:
    int answers[10];
    bool submitted;
    int score;
    Timer timer;

public:
    Attempt();

    void startTimer(int seconds);
    bool isTimeUp() const;
    int getRemainingTime() const;

    void saveAnswer(int index, int answer);
    int getAnswer(int index) const;

    void setScore(int score);
    int getScore() const;

    bool isSubmitted() const;
    void markSubmitted();
};

#endif
