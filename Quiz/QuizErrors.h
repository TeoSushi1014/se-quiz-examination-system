#pragma once
#include <string>

namespace QuizSystem {
    namespace Errors {
        const std::string INVALID_QUESTION = "Error: Invalid Question Data";
        const std::string DUPLICATE_QUESTION = "Error: Question Already Exists";
        const std::string INVALID_QUIZ_CONFIG = "Error: Invalid Quiz Configuration";
        const std::string QUESTION_IN_USE = "Error: Cannot Delete - Question In Use";
        const std::string QUIZ_HAS_ATTEMPTS = "Error: Cannot Delete - Quiz Has Student Attempts";
    }
}
