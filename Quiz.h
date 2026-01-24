#pragma once

#ifndef QUIZ_H
#define QUIZ_H

#include "Question.h"

class Quiz 
{
private:
    Question questions[10];
    int totalQuestions;

public:
    Quiz();

    void addQuestion(const Question& q);
    int size() const;
    Question getQuestion(int index) const;
};

#endif
