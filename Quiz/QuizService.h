#pragma once
#include "QuizModels.h"
#include "QuizRepository.h"
#include <vector>
#include <string>

namespace QuizSystem {

    class QuizService {
    private:
        QuizRepository* repository; // Use pointer for dependency injection

    public:
        // Constructor Dependency Injection
        // Pass a pointer to your repository implementation
        QuizService(QuizRepository* repo) : repository(repo) {}

        // 1. Validation Logic
        bool ValidateQuestion(const Question& q);

        // 2. Creation Logic
        bool CreateQuiz(const Quiz& quiz);

        // 3. Guard Rules
        bool CanDeleteQuestion(const std::string& questionId);
        bool CanDeleteQuiz(const std::string& quizId, bool isTeacher);

        // 4. Scoring Logic (Pure Math, no DB needed usually)
        int CalculateScore(const std::vector<StudentAnswer>& answers, const std::vector<Question>& questions);
    };

}
