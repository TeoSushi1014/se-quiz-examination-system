#include "database.h"
#include <algorithm>
#include <iostream>

using namespace std;

void deleteAttemptByAdmin(
        const string& attemptId,
        const string& adminId) {

    bool isAdmin = false;

    for (auto &u : users)
        if (u.id == adminId && u.role == Role::ADMIN)
            isAdmin = true;

    if (!isAdmin) {
        cout << "Permission denied!\n";
        return;
    }

    // xoá attempt
    attempts.erase(remove_if(attempts.begin(), attempts.end(),
        [&](Attempt a) {
            return a.attemptId == attemptId;
        }), attempts.end());

    // xoá result
    results.erase(remove_if(results.begin(), results.end(),
        [&](Result r) {
            return r.attemptId == attemptId;
        }), results.end());

    // audit
    auditLogs.push_back(
        {adminId, "DELETE_ATTEMPT", attemptId}
    );
}
