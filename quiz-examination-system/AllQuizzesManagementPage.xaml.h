#pragma once
#include "AllQuizzesManagementPage.g.h"
#include "QuizManagementItem.h"
#include "SupabaseClientAsync.h"

namespace winrt::quiz_examination_system::implementation
{
    struct AllQuizzesManagementPage : AllQuizzesManagementPageT<AllQuizzesManagementPage>
    {
        AllQuizzesManagementPage();

        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::QuizManagementItem> Quizzes() { return m_quizzes; }

        void OnSearchTextChanged(Microsoft::UI::Xaml::Controls::AutoSuggestBox const &sender, Microsoft::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs const &args);
        void OnRefreshClicked(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void OnCreateQuizClicked(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void OnEditClicked(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void OnAssignClicked(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void OnDeleteClicked(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);

    private:
        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::QuizManagementItem> m_quizzes;
        std::vector<quiz_examination_system::QuizManagementItem> m_allQuizzes;

        fire_and_forget LoadQuizzes();
        void FilterQuizzes(hstring const &searchText);
        fire_and_forget DeleteQuiz(hstring quizId);
        void ShowMessage(hstring const &title, hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity);
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct AllQuizzesManagementPage : AllQuizzesManagementPageT<AllQuizzesManagementPage, implementation::AllQuizzesManagementPage>
    {
    };
}
