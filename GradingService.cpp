#include "GradingService.h"

void GradingService::grade(const Quiz& quiz, Attempt& attempt) 
{
    int total = 0;
    for (int i = 0; i < quiz.size(); i++) 
    {
        if (attempt.getAnswer(i) == quiz.getQuestion(i).getCorrectOption())
            total += quiz.getQuestion(i).getPoint();
    }
    attempt.setScore(total);
}
