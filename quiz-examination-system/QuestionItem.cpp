#include "pch.h"
#include "QuestionItem.h"
#if __has_include("QuestionItem.g.cpp")
#include "QuestionItem.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml::Data;

namespace winrt::quiz_examination_system::implementation
{
    hstring QuestionItem::QuestionId()
    {
        return m_questionId;
    }

    void QuestionItem::QuestionId(hstring const &value)
    {
        if (m_questionId != value)
        {
            m_questionId = value;
            m_propertyChanged(*this, PropertyChangedEventArgs(L"QuestionId"));
        }
    }

    hstring QuestionItem::QuestionText()
    {
        return m_questionText;
    }

    void QuestionItem::QuestionText(hstring const &value)
    {
        if (m_questionText != value)
        {
            m_questionText = value;
            m_propertyChanged(*this, PropertyChangedEventArgs(L"QuestionText"));
        }
    }

    hstring QuestionItem::DifficultyLevel()
    {
        return m_difficultyLevel;
    }

    void QuestionItem::DifficultyLevel(hstring const &value)
    {
        if (m_difficultyLevel != value)
        {
            m_difficultyLevel = value;
            m_propertyChanged(*this, PropertyChangedEventArgs(L"DifficultyLevel"));
        }
    }

    hstring QuestionItem::Topic()
    {
        return m_topic;
    }

    void QuestionItem::Topic(hstring const &value)
    {
        if (m_topic != value)
        {
            m_topic = value;
            m_propertyChanged(*this, PropertyChangedEventArgs(L"Topic"));
        }
    }

    winrt::event_token QuestionItem::PropertyChanged(PropertyChangedEventHandler const &handler)
    {
        return m_propertyChanged.add(handler);
    }

    void QuestionItem::PropertyChanged(winrt::event_token const &token) noexcept
    {
        m_propertyChanged.remove(token);
    }
}
