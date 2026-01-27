#pragma once
#include "QuizManagementItem.g.h"

namespace winrt::quiz_examination_system::implementation
{
    struct QuizManagementItem : QuizManagementItemT<QuizManagementItem>
    {
        QuizManagementItem(hstring const &quizId, hstring const &title, hstring const &teacherName, hstring const &createdAt, int32_t attemptCount);

        hstring QuizId() { return m_quizId; }
        hstring Title() { return m_title; }
        hstring TeacherName() { return m_teacherName; }
        hstring CreatedAt() { return m_createdAt; }
        int32_t AttemptCount() { return m_attemptCount; }

        winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const &handler) { return m_propertyChanged.add(handler); }
        void PropertyChanged(winrt::event_token const &token) noexcept { m_propertyChanged.remove(token); }

    private:
        hstring m_quizId;
        hstring m_title;
        hstring m_teacherName;
        hstring m_createdAt;
        int32_t m_attemptCount;
        event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct QuizManagementItem : QuizManagementItemT<QuizManagementItem, implementation::QuizManagementItem>
    {
    };
}
