#pragma once
#include <string>

enum class Role {
    STUDENT,
    ADMIN
};

struct User {
    std::string id;
    std::string username;
    Role role;
};

struct Attempt {
    std::string attemptId;
    std::string quizId;
    std::string studentId;
    int attemptNumber;
};

struct Result {
    std::string attemptId;
    double score;
    int timeSpentSeconds;
};

struct AuditLog {
    std::string adminId;
    std::string action;
    std::string targetId;
};
