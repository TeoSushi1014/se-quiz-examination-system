#include "Quiz.h"

Quiz::Quiz() 
{
    totalQuestions = 0;
}

void Quiz::addQuestion(const Question& q) 
{
    questions[totalQuestions++] = q;
}

int Quiz::size() const 
{
    return totalQuestions;
}

Question Quiz::getQuestion(int index) const 
{
    return questions[index];
}
