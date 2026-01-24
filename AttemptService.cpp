#include "AttemptService.h"
#include <iostream>
using namespace std;

void AttemptService::submit(const Quiz& quiz, Attempt& attempt) 
{
    if (attempt.isSubmitted()) 
        return;

    GradingService::grade(quiz, attempt);
    attempt.markSubmitted();

    cout << "\n=== DA NOP BAI ===\n";
    cout << "Tong diem: " << attempt.getScore() << endl;
}
