#pragma once

#include "QuestionItem.g.h"

namespace winrt::quiz_examination_system::implementation
{
    struct QuestionItem : QuestionItemT<QuestionItem>
    {
        QuestionItem() = default;

        hstring QuestionId();
        void QuestionId(hstring const &value);

        hstring QuestionText();
        void QuestionText(hstring const &value);

        hstring DifficultyLevel();
        void DifficultyLevel(hstring const &value);

        hstring Topic();
        void Topic(hstring const &value);

        winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const &handler);
        void PropertyChanged(winrt::event_token const &token) noexcept;

    private:
        hstring m_questionId;
        hstring m_questionText;
        hstring m_difficultyLevel;
        hstring m_topic;

        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct QuestionItem : QuestionItemT<QuestionItem, implementation::QuestionItem>
    {
    };
}
