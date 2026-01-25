#include "database.h"
#include <iostream>
#include <fstream>

using namespace std;

void viewReport(const string& quizId) {

    cout << "=== REPORT FOR QUIZ " << quizId << " ===\n";

    for (auto &a : attempts) {
        if (a.quizId == quizId) {

            string username;
            for (auto &u : users)
                if (u.id == a.studentId)
                    username = u.username;

            for (auto &r : results)
                if (r.attemptId == a.attemptId)
                    cout << username
                         << " | Attempt " << a.attemptNumber
                         << " | Score " << r.score
                         << " | Time " << r.timeSpentSeconds
                         << "s\n";
        }
    }
}

void exportCSV(const string& quizId) {

    ofstream file("report_" + quizId + ".csv");

    file << "Username,Attempt,Score,TimeSpent\n";

    for (auto &a : attempts) {
        if (a.quizId == quizId) {

            string username;
            for (auto &u : users)
                if (u.id == a.studentId)
                    username = u.username;

            for (auto &r : results)
                if (r.attemptId == a.attemptId)
                    file << username << ","
                         << a.attemptNumber << ","
                         << r.score << ","
                         << r.timeSpentSeconds << "\n";
        }
    }

    file.close();
}
