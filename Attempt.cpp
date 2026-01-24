#include "Attempt.h"

Attempt::Attempt() 
{
    submitted = false;
    score = 0;
    for (int i = 0; i < 10; i++)
        answers[i] = -1;
}

void Attempt::startTimer(int seconds) 
{
    timer.start(seconds);
}

bool Attempt::isTimeUp() const 
{    
    return timer.isTimeUp();
}

int Attempt::getRemainingTime() const 
{
    return timer.getRemainingTime();
}

void Attempt::saveAnswer(int index, int answer) 
{
    if (!submitted)
        answers[index] = answer;
}

int Attempt::getAnswer(int index) const 
{
    return answers[index];
}

void Attempt::setScore(int s)
{
    score = s;
}

int Attempt::getScore() const 
{
    return score;
}

bool Attempt::isSubmitted() const 
{
    return submitted;
}

void Attempt::markSubmitted() 
{
    submitted = true;
}
