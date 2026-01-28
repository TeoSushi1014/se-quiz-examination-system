#pragma once
#include "QuizEditorPage.g.h"
#include "QuestionItem.h"

namespace winrt::quiz_examination_system::implementation
{
    struct QuizEditorPage : QuizEditorPageT<QuizEditorPage>
    {
        QuizEditorPage();

        void Page_Loaded(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void OnSearchTextChanged(Microsoft::UI::Xaml::Controls::AutoSuggestBox const &sender, Microsoft::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs const &args);
        void OnQuestionChecked(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void OnQuestionUnchecked(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void OnRemoveQuestionClicked(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void OnSaveClicked(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void OnCancelClicked(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);

    private:
        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::QuestionItem> m_availableQuestions;
        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::QuestionItem> m_selectedQuestions;
        std::vector<quiz_examination_system::QuestionItem> m_allQuestions;

        fire_and_forget LoadQuestions();
        void FilterQuestions(hstring const &searchText);
        fire_and_forget SaveQuiz();
        void UpdateSelectedCount();
        void ShowMessage(hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity);
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct QuizEditorPage : QuizEditorPageT<QuizEditorPage, implementation::QuizEditorPage>
    {
    };
}
