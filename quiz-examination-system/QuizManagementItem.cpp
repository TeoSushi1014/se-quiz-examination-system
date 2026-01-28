#include "pch.h"
#include "QuizManagementItem.h"
#if __has_include("QuizManagementItem.g.cpp")
#include "QuizManagementItem.g.cpp"
#endif

namespace winrt::quiz_examination_system::implementation
{
    QuizManagementItem::QuizManagementItem(hstring const &quizId, hstring const &title, hstring const &createdBy, int32_t timeLimitMinutes, int32_t totalPoints, int32_t questionCount, hstring const &createdAt)
        : m_quizId(quizId), m_title(title), m_createdBy(createdBy), m_timeLimitMinutes(timeLimitMinutes), m_totalPoints(totalPoints), m_questionCount(questionCount), m_createdAt(createdAt)
    {
    }

    hstring QuizManagementItem::FormattedTime()
    {
        if (m_timeLimitMinutes < 60)
        {
            return winrt::to_hstring(m_timeLimitMinutes) + L" min";
        }
        else
        {
            int32_t hours = m_timeLimitMinutes / 60;
            int32_t minutes = m_timeLimitMinutes % 60;
            if (minutes == 0)
            {
                return winrt::to_hstring(hours) + L" hr";
            }
            return winrt::to_hstring(hours) + L" hr " + winrt::to_hstring(minutes) + L" min";
        }
    }

    hstring QuizManagementItem::QuestionCountText()
    {
        return winrt::to_hstring(m_questionCount) + L" question" + (m_questionCount > 1 ? L"s" : L"");
    }
}
