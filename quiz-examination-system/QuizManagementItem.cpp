#include "pch.h"
#include "QuizManagementItem.h"
#if __has_include("QuizManagementItem.g.cpp")
#include "QuizManagementItem.g.cpp"
#endif

namespace winrt::quiz_examination_system::implementation
{
    QuizManagementItem::QuizManagementItem(hstring const &quizId, hstring const &title, hstring const &teacherName, hstring const &createdAt, int32_t attemptCount)
        : m_quizId(quizId), m_title(title), m_teacherName(teacherName), m_createdAt(createdAt), m_attemptCount(attemptCount)
    {
    }
}
