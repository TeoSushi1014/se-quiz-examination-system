#include "AttemptService.h"
#include <iostream>
using namespace std;

int main() 
{
    Quiz quiz;

    string q1[4] = 
    {
        "Ngon ngu bac cao",
        "Ngon ngu danh cho tre em",
        "He dieu hanh",
        "Trinh duyet"
    };

    string q2[4] = 
    {
        "var",
        "let",
        "const",
        "define"
    };

    string q3[4] = 
    {
        "Java",
        "Python",
        "C++",
        "HTML"
    };

    string q4[4] = 
    {
        "Stack",
        "Queue",
        "Array",
        "Tree"
    };

    string q5[4] = 
    {
        "Encapsulation",
        "Inheritance",
        "Polymorphism",
        "Compilation"
    };

    quiz.addQuestion(Question("C++ la gi?", q1, 0, 5));
    quiz.addQuestion(Question("Tu khoa khai bao hang so trong C++?", q2, 2, 5));
    quiz.addQuestion(Question("Ngon ngu lap trinh nao la ngon ngu bien dich?", q3, 2, 5));
    quiz.addQuestion(Question("Cau truc du lieu LIFO la gi?", q4, 0, 5));
    quiz.addQuestion(Question("Tinh chat KHONG thuoc OOP la gi?", q5, 3, 5));

    Attempt attempt;
    attempt.startTimer(25);

    for (int i = 0; i < quiz.size(); i++) 
    {
        if (attempt.isTimeUp()) 
        {
            cout << "\n=== HET GIO ===\n";
            cout << "He thong tu dong nop bai!\n";
            AttemptService::submit(quiz, attempt);
            return 0;
        }

        quiz.getQuestion(i).display();
        cout << "Thoi gian con lai: "
            << attempt.getRemainingTime() << " giay\n";

        int ans;
        cout << "Chon dap an (0-3): ";
        cin >> ans;

        attempt.saveAnswer(i, ans);
    }

    AttemptService::submit(quiz, attempt);
    return 0;
}
