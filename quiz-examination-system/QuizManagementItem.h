#pragma once
#include "QuizManagementItem.g.h"

namespace winrt::quiz_examination_system::implementation
{
    struct QuizManagementItem : QuizManagementItemT<QuizManagementItem>
    {
        QuizManagementItem(hstring const &quizId, hstring const &title, hstring const &createdBy, int32_t timeLimitMinutes, int32_t totalPoints, int32_t questionCount, hstring const &createdAt);

        hstring QuizId() { return m_quizId; }
        hstring Title() { return m_title; }
        hstring CreatedBy() { return m_createdBy; }
        int32_t TimeLimitMinutes() { return m_timeLimitMinutes; }
        int32_t TotalPoints() { return m_totalPoints; }
        int32_t QuestionCount() { return m_questionCount; }
        hstring CreatedAt() { return m_createdAt; }
        hstring FormattedTime();
        hstring QuestionCountText();

        winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const &handler) { return m_propertyChanged.add(handler); }
        void PropertyChanged(winrt::event_token const &token) noexcept { m_propertyChanged.remove(token); }

    private:
        hstring m_quizId;
        hstring m_title;
        hstring m_createdBy;
        int32_t m_timeLimitMinutes;
        int32_t m_totalPoints;
        int32_t m_questionCount;
        hstring m_createdAt;
        event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct QuizManagementItem : QuizManagementItemT<QuizManagementItem, implementation::QuizManagementItem>
    {
    };
}
