#pragma once

#include "AttemptItem.g.h"

namespace winrt::quiz_examination_system::implementation
{
    struct AttemptItem : AttemptItemT<AttemptItem>
    {
        AttemptItem() = default;

        hstring AttemptId() { return m_attemptId; }
        void AttemptId(hstring const &value)
        {
            m_attemptId = value;
            RaisePropertyChanged(L"AttemptId");
        }

        hstring StudentId() { return m_studentId; }
        void StudentId(hstring const &value)
        {
            m_studentId = value;
            RaisePropertyChanged(L"StudentId");
        }

        hstring StudentUsername() { return m_studentUsername; }
        void StudentUsername(hstring const &value)
        {
            m_studentUsername = value;
            RaisePropertyChanged(L"StudentUsername");
        }

        int32_t AttemptNumber() { return m_attemptNumber; }
        void AttemptNumber(int32_t value)
        {
            m_attemptNumber = value;
            RaisePropertyChanged(L"AttemptNumber");
        }

        int32_t Score() { return m_score; }
        void Score(int32_t value)
        {
            m_score = value;
            RaisePropertyChanged(L"Score");
        }

        int32_t TotalPoints() { return m_totalPoints; }
        void TotalPoints(int32_t value)
        {
            m_totalPoints = value;
            RaisePropertyChanged(L"TotalPoints");
        }

        int32_t CorrectCount() { return m_correctCount; }
        void CorrectCount(int32_t value)
        {
            m_correctCount = value;
            RaisePropertyChanged(L"CorrectCount");
        }

        int32_t IncorrectCount() { return m_incorrectCount; }
        void IncorrectCount(int32_t value)
        {
            m_incorrectCount = value;
            RaisePropertyChanged(L"IncorrectCount");
        }

        int32_t TimeSpentSeconds() { return m_timeSpentSeconds; }
        void TimeSpentSeconds(int32_t value)
        {
            m_timeSpentSeconds = value;
            RaisePropertyChanged(L"TimeSpentSeconds");
        }

        hstring Status() { return m_status; }
        void Status(hstring const &value)
        {
            m_status = value;
            RaisePropertyChanged(L"Status");
        }

        hstring StartedAt() { return m_startedAt; }
        void StartedAt(hstring const &value)
        {
            m_startedAt = value;
            RaisePropertyChanged(L"StartedAt");
        }

        hstring SubmittedAt() { return m_submittedAt; }
        void SubmittedAt(hstring const &value)
        {
            m_submittedAt = value;
            RaisePropertyChanged(L"SubmittedAt");
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
        hstring m_attemptId;
        hstring m_studentId;
        hstring m_studentUsername;
        int32_t m_attemptNumber{0};
        int32_t m_score{0};
        int32_t m_totalPoints{0};
        int32_t m_correctCount{0};
        int32_t m_incorrectCount{0};
        int32_t m_timeSpentSeconds{0};
        hstring m_status;
        hstring m_startedAt;
        hstring m_submittedAt;

        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

        void RaisePropertyChanged(hstring const &propertyName)
        {
            m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(propertyName));
        }
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct AttemptItem : AttemptItemT<AttemptItem, implementation::AttemptItem>
    {
    };
}
