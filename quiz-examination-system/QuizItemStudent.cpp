#include "pch.h"
#include "QuizItemStudent.h"
#include "QuizItemStudent.g.cpp"

namespace winrt::quiz_examination_system::implementation
{
    hstring QuizItemStudent::QuizId()
    {
        return m_quizId;
    }

    void QuizItemStudent::QuizId(hstring const &value)
    {
        if (m_quizId != value)
        {
            m_quizId = value;
            m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{L"QuizId"});
        }
    }

    hstring QuizItemStudent::Title()
    {
        return m_title;
    }

    void QuizItemStudent::Title(hstring const &value)
    {
        if (m_title != value)
        {
            m_title = value;
            m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{L"Title"});
        }
    }

    int32_t QuizItemStudent::TimeLimit()
    {
        return m_timeLimit;
    }

    void QuizItemStudent::TimeLimit(int32_t value)
    {
        if (m_timeLimit != value)
        {
            m_timeLimit = value;
            m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{L"TimeLimit"});
        }
    }

    int32_t QuizItemStudent::TotalPoints()
    {
        return m_totalPoints;
    }

    void QuizItemStudent::TotalPoints(int32_t value)
    {
        if (m_totalPoints != value)
        {
            m_totalPoints = value;
            m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{L"TotalPoints"});
        }
    }

    int32_t QuizItemStudent::AttemptsUsed()
    {
        return m_attemptsUsed;
    }

    void QuizItemStudent::AttemptsUsed(int32_t value)
    {
        if (m_attemptsUsed != value)
        {
            m_attemptsUsed = value;
            m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{L"AttemptsUsed"});
        }
    }

    hstring QuizItemStudent::MaxAttempts()
    {
        return m_maxAttempts;
    }

    void QuizItemStudent::MaxAttempts(hstring const &value)
    {
        if (m_maxAttempts != value)
        {
            m_maxAttempts = value;
            m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{L"MaxAttempts"});
        }
    }

    hstring QuizItemStudent::Status()
    {
        return m_status;
    }

    void QuizItemStudent::Status(hstring const &value)
    {
        if (m_status != value)
        {
            m_status = value;
            m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{L"Status"});
        }
    }

    Microsoft::UI::Xaml::Media::Brush QuizItemStudent::StatusBadgeColor()
    {
        return m_statusBadgeColor;
    }

    void QuizItemStudent::StatusBadgeColor(Microsoft::UI::Xaml::Media::Brush const &value)
    {
        if (m_statusBadgeColor != value)
        {
            m_statusBadgeColor = value;
            m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{L"StatusBadgeColor"});
        }
    }

    winrt::event_token QuizItemStudent::PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const &handler)
    {
        return m_propertyChanged.add(handler);
    }

    void QuizItemStudent::PropertyChanged(winrt::event_token const &token) noexcept
    {
        m_propertyChanged.remove(token);
    }
}
