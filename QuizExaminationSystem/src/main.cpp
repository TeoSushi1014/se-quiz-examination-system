#include <iostream>
#include <vector>
#include <string>

#include "database.h"

using namespace std;

// ===== DB giả lập =====
vector<User> users;
vector<Attempt> attempts;
vector<Result> results;
vector<AuditLog> auditLogs;

// ===== prototype =====
void viewReport(const string&);
void exportCSV(const string&);
void deleteAttemptByAdmin(const string&, const string&);

int main() {

    // seed dữ liệu mẫu
    users.push_back({"U1","student1",Role::STUDENT});
    users.push_back({"U2","admin1",Role::ADMIN});

    attempts.push_back({"A1","Q1","U1",1});
    results.push_back({"A1",8.5,120});

    int choice;
    string quizId, attemptId, adminId;

    while (true) {
        cout << "\n==== QUIZ EXAMINATION SYSTEM ====\n";
        cout << "1. View report\n";
        cout << "2. Export CSV\n";
        cout << "3. Delete attempt (Admin)\n";
        cout << "0. Exit\n";
        cout << "Choose: ";
        cin >> choice;

        if (choice == 0) break;

        switch (choice) {
        case 1:
            cout << "Enter Quiz ID: ";
            cin >> quizId;
            viewReport(quizId);
            break;

        case 2:
            cout << "Enter Quiz ID: ";
            cin >> quizId;
            exportCSV(quizId);
            break;

        case 3:
            cout << "Enter Attempt ID: ";
            cin >> attemptId;
            cout << "Enter Admin User ID: ";
            cin >> adminId;
            deleteAttemptByAdmin(attemptId, adminId);
            break;

        default:
            cout << "Invalid choice!\n";
        }
    }

    return 0;
}
