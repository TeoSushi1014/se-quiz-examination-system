#pragma once

#include "QuizItemStudent.g.h"

namespace winrt::quiz_examination_system::implementation
{
    struct QuizItemStudent : QuizItemStudentT<QuizItemStudent>
    {
        QuizItemStudent() = default;

        hstring QuizId();
        void QuizId(hstring const &value);

        hstring Title();
        void Title(hstring const &value);

        int32_t TimeLimit();
        void TimeLimit(int32_t value);

        int32_t TotalPoints();
        void TotalPoints(int32_t value);

        int32_t AttemptsUsed();
        void AttemptsUsed(int32_t value);

        hstring MaxAttempts();
        void MaxAttempts(hstring const &value);

        hstring Status();
        void Status(hstring const &value);

        Microsoft::UI::Xaml::Media::Brush StatusBadgeColor();
        void StatusBadgeColor(Microsoft::UI::Xaml::Media::Brush const &value);

        winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const &handler);
        void PropertyChanged(winrt::event_token const &token) noexcept;

    private:
        hstring m_quizId;
        hstring m_title;
        int32_t m_timeLimit{0};
        int32_t m_totalPoints{0};
        int32_t m_attemptsUsed{0};
        hstring m_maxAttempts;
        hstring m_status;
        Microsoft::UI::Xaml::Media::Brush m_statusBadgeColor{nullptr};

        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct QuizItemStudent : QuizItemStudentT<QuizItemStudent, implementation::QuizItemStudent>
    {
    };
}
