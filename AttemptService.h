#pragma once

#ifndef ATTEMPT_SERVICE_H
#define ATTEMPT_SERVICE_H

#include "GradingService.h"

class AttemptService 
{
public:
    static void submit(const Quiz& quiz, Attempt& attempt);
};

#endif
