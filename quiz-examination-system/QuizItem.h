#pragma once

#include "QuizItem.g.h"

namespace winrt::quiz_examination_system::implementation
{
    struct QuizItem : QuizItemT<QuizItem>
    {
        QuizItem() = default;

        hstring QuizId() { return m_quizId; }
        void QuizId(hstring const &value)
        {
            if (m_quizId != value)
            {
                m_quizId = value;
                RaisePropertyChanged(L"QuizId");
            }
        }

        hstring Title() { return m_title; }
        void Title(hstring const &value)
        {
            if (m_title != value)
            {
                m_title = value;
                RaisePropertyChanged(L"Title");
            }
        }

        int32_t TimeLimitMinutes() { return m_timeLimitMinutes; }
        void TimeLimitMinutes(int32_t value)
        {
            if (m_timeLimitMinutes != value)
            {
                m_timeLimitMinutes = value;
                RaisePropertyChanged(L"TimeLimitMinutes");
            }
        }

        int32_t TotalPoints() { return m_totalPoints; }
        void TotalPoints(int32_t value)
        {
            if (m_totalPoints != value)
            {
                m_totalPoints = value;
                RaisePropertyChanged(L"TotalPoints");
            }
        }

        int32_t QuestionCount() { return m_questionCount; }
        void QuestionCount(int32_t value)
        {
            if (m_questionCount != value)
            {
                m_questionCount = value;
                RaisePropertyChanged(L"QuestionCount");
            }
        }

        hstring MaxAttempts() { return m_maxAttempts; }
        void MaxAttempts(hstring const &value)
        {
            if (m_maxAttempts != value)
            {
                m_maxAttempts = value;
                RaisePropertyChanged(L"MaxAttempts");
            }
        }

        hstring ResultVisibility() { return m_resultVisibility; }
        void ResultVisibility(hstring const &value)
        {
            if (m_resultVisibility != value)
            {
                m_resultVisibility = value;
                RaisePropertyChanged(L"ResultVisibility");
            }
        }

        bool ShuffleQuestions() { return m_shuffleQuestions; }
        void ShuffleQuestions(bool value)
        {
            if (m_shuffleQuestions != value)
            {
                m_shuffleQuestions = value;
                RaisePropertyChanged(L"ShuffleQuestions");
            }
        }

        bool ShuffleAnswers() { return m_shuffleAnswers; }
        void ShuffleAnswers(bool value)
        {
            if (m_shuffleAnswers != value)
            {
                m_shuffleAnswers = value;
                RaisePropertyChanged(L"ShuffleAnswers");
            }
        }

        hstring CreatedAt() { return m_createdAt; }
        void CreatedAt(hstring const &value)
        {
            if (m_createdAt != value)
            {
                m_createdAt = value;
                RaisePropertyChanged(L"CreatedAt");
            }
        }

        winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const &handler)
        {
            return m_propertyChanged.add(handler);
        }

        void PropertyChanged(winrt::event_token const &token) noexcept
        {
            m_propertyChanged.remove(token);
        }

    private:
        hstring m_quizId;
        hstring m_title;
        int32_t m_timeLimitMinutes = 0;
        int32_t m_totalPoints = 0;
        int32_t m_questionCount = 0;
        hstring m_maxAttempts = L"unlimited";
        hstring m_resultVisibility = L"immediate";
        bool m_shuffleQuestions = false;
        bool m_shuffleAnswers = false;
        hstring m_createdAt;

        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

        void RaisePropertyChanged(hstring const &propertyName)
        {
            m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(propertyName));
        }
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct QuizItem : QuizItemT<QuizItem, implementation::QuizItem>
    {
    };
}
