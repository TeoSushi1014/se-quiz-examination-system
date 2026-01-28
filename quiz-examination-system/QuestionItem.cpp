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

    hstring QuestionItem::OptionA()
    {
        return m_optionA;
    }

    void QuestionItem::OptionA(hstring const &value)
    {
        if (m_optionA != value)
        {
            m_optionA = value;
            m_propertyChanged(*this, PropertyChangedEventArgs(L"OptionA"));
        }
    }

    hstring QuestionItem::OptionB()
    {
        return m_optionB;
    }

    void QuestionItem::OptionB(hstring const &value)
    {
        if (m_optionB != value)
        {
            m_optionB = value;
            m_propertyChanged(*this, PropertyChangedEventArgs(L"OptionB"));
        }
    }

    hstring QuestionItem::OptionC()
    {
        return m_optionC;
    }

    void QuestionItem::OptionC(hstring const &value)
    {
        if (m_optionC != value)
        {
            m_optionC = value;
            m_propertyChanged(*this, PropertyChangedEventArgs(L"OptionC"));
        }
    }

    hstring QuestionItem::OptionD()
    {
        return m_optionD;
    }

    void QuestionItem::OptionD(hstring const &value)
    {
        if (m_optionD != value)
        {
            m_optionD = value;
            m_propertyChanged(*this, PropertyChangedEventArgs(L"OptionD"));
        }
    }

    hstring QuestionItem::CorrectOption()
    {
        return m_correctOption;
    }

    void QuestionItem::CorrectOption(hstring const &value)
    {
        if (m_correctOption != value)
        {
            m_correctOption = value;
            m_propertyChanged(*this, PropertyChangedEventArgs(L"CorrectOption"));
        }
    }
}
