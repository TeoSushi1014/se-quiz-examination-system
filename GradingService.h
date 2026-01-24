#pragma once

#ifndef GRADING_SERVICE_H
#define GRADING_SERVICE_H

#include "Quiz.h"
#include "Attempt.h"

class GradingService 
{
public:
    static void grade(const Quiz& quiz, Attempt& attempt);
};

#endif
