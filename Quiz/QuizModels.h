#pragma once
#include <string>
#include <vector>

namespace QuizSystem {

    struct Option {
        std::string text;
        bool isCorrect;
    };

    struct Question {
        std::string id;
        std::string text;
        std::vector<Option> options;
        int points;

        // --- BỔ SUNG CÁC TRƯỜNG CÒN THIẾU ---
        std::string difficulty; // "Easy", "Medium", "Hard"
        std::string topic;
    };

    struct Quiz {
        std::string id;
        std::string title;
        std::vector<std::string> questionIds;
        int timeLimitMinutes;
        int totalPoints;
        std::string startTime;
        std::string endTime;

        // --- BỔ SUNG CÁC TRƯỜNG ĐỂ SỬA LỖI "HAS NO MEMBER" ---
        bool shuffleQuestions = false;
        bool shuffleAnswers = false;
        std::string resultVisibility; // "IMMEDIATE", "AFTER_END", "MANUAL"
        int maxAttempts = 1;

        // Đây là biến gây ra lỗi của bạn, cần thêm vào đây:
        std::vector<std::string> assignedStudentIds;
    };

    struct StudentAnswer {
        std::string questionId;
        int selectedOptionIndex;
    };

}