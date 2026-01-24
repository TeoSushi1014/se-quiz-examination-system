#include "Question.h"

Question::Question() {}

Question::Question(string c, string ops[], int correct, int p) 
{
    content = c;
    correctOption = correct;
    point = p;
    for (int i = 0; i < 4; i++)
        options[i] = ops[i];
}

void Question::display() const 
{
    cout << "\n" << content << endl;
    for (int i = 0; i < 4; i++)
        cout << i << ". " << options[i] << endl;
}

int Question::getCorrectOption() const 
{
    return correctOption;
}

int Question::getPoint() const 
{
    return point;
}
