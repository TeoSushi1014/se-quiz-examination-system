#pragma once
#include <string>
#include <vector>
#include "QuizModels.h"

namespace QuizSystem {

    class QuizRepository {
    public:

        virtual bool IsQuestionTextDuplicate(const std::string& text) {
            return false;
        }

        virtual bool IsQuestionInActiveQuiz(const std::string& questionId) {
            return false;
        }

        virtual int GetStudentAttemptCount(const std::string& quizId) {
            return 0;
        }

        virtual void SaveQuiz(const Quiz& quiz) {
        }
    };

}
