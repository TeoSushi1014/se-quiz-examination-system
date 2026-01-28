#pragma once
#include "ReportItem.g.h"

namespace winrt::quiz_examination_system::implementation
{
    struct ReportItem : ReportItemT<ReportItem>
    {
        ReportItem(hstring const &attemptId, hstring const &studentName, int32_t attemptNumber, double score, double totalPoints, hstring const &timeSpent, hstring const &submittedDate);

        hstring AttemptId() { return m_attemptId; }
        hstring StudentName() { return m_studentName; }
        int32_t AttemptNumber() { return m_attemptNumber; }
        double Score() { return m_score; }
        double TotalPoints() { return m_totalPoints; }
        hstring FormattedScore();
        hstring TimeSpent() { return m_timeSpent; }
        hstring SubmittedDate() { return m_submittedDate; }

        winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const &handler) { return m_propertyChanged.add(handler); }
        void PropertyChanged(winrt::event_token const &token) noexcept { m_propertyChanged.remove(token); }

    private:
        hstring m_attemptId;
        hstring m_studentName;
        int32_t m_attemptNumber;
        double m_score;
        double m_totalPoints;
        hstring m_timeSpent;
        hstring m_submittedDate;
        event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct ReportItem : ReportItemT<ReportItem, implementation::ReportItem>
    {
    };
}
