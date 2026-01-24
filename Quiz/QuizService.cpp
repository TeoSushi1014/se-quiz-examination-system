#include "QuizService.h"
#include "QuizErrors.h"
#include <iostream>

namespace QuizSystem {

    
    bool QuizService::ValidateQuestion(const Question& q) {
        
        if (q.text.empty()) return false;

        
        if (q.options.size() != 4) return false;

        
        int correctCount = 0;
        for (const auto& opt : q.options) {
            if (opt.isCorrect) correctCount++;
        }
        if (correctCount != 1) return false;

        
        if (repository->IsQuestionTextDuplicate(q.text)) {
            std::cout << Errors::DUPLICATE_QUESTION << std::endl;
            return false;
        }

        return true;
    }

   
    bool QuizService::CreateQuiz(const Quiz& quiz) {
        
        if (quiz.questionIds.empty()) return false;

        
        if (quiz.timeLimitMinutes <= 0) return false;

        
        if (quiz.totalPoints <= 0) return false;

        
        if (quiz.assignedStudentIds.empty()) return false;

       
        if (quiz.startTime >= quiz.endTime) return false;

        
        repository->SaveQuiz(quiz);
        return true;
    }

    bool QuizService::CanDeleteQuestion(const std::string& questionId) {
        if (repository->IsQuestionInActiveQuiz(questionId)) {
            std::cout << Errors::QUESTION_IN_USE << std::endl;
            return false;
        }
        return true;
    }

    bool QuizService::CanDeleteQuiz(const std::string& quizId, bool isTeacher) {
        if (isTeacher) {
            int attempts = repository->GetStudentAttemptCount(quizId);
            if (attempts > 0) {
                std::cout << Errors::QUIZ_HAS_ATTEMPTS << std::endl;
                return false;
            }
        }
        return true;
    }


    int QuizService::CalculateScore(const std::vector<StudentAnswer>& answers, const std::vector<Question>& questions) {
        int totalScore = 0;
        for (const auto& ans : answers) {
            for (const auto& q : questions) {
                if (q.id == ans.questionId) {
                    if (ans.selectedOptionIndex >= 0 && ans.selectedOptionIndex < (int)q.options.size()) {
                        if (q.options[ans.selectedOptionIndex].isCorrect) {
                            totalScore += q.points;
                        }
                    }
                    break; 
                }
            }
        }
        return totalScore;
    }

}