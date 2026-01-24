#pragma once
#ifndef QUESTION_H
#define QUESTION_H
#pragma once

#include <iostream>
using namespace std;

class Question 
{
private:
    string content;
    string options[4];
    int correctOption;
    int point;

public:
    Question();
    Question(string content, string options[], int correctOption, int point);

    void display() const;
    int getCorrectOption() const;
    int getPoint() const;
};

#endif
