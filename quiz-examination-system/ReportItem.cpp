#include "pch.h"
#include "ReportItem.h"
#if __has_include("ReportItem.g.cpp")
#include "ReportItem.g.cpp"
#endif

namespace winrt::quiz_examination_system::implementation
{
    ReportItem::ReportItem(hstring const &attemptId, hstring const &studentName, int32_t attemptNumber, double score, double totalPoints, hstring const &timeSpent, hstring const &submittedDate)
        : m_attemptId(attemptId), m_studentName(studentName), m_attemptNumber(attemptNumber), m_score(score), m_totalPoints(totalPoints), m_timeSpent(timeSpent), m_submittedDate(submittedDate)
    {
    }

    hstring ReportItem::FormattedScore()
    {
        wchar_t buffer[64];
        swprintf_s(buffer, L"%.1f/%.0f", m_score, m_totalPoints);
        return hstring(buffer);
    }
}
