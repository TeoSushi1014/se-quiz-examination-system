#pragma once
#include "AllQuizzesManagementPage.g.h"
#include "QuizManagementItem.h"
#include "SupabaseClientAsync.h"

namespace winrt::quiz_examination_system::implementation
{
    struct AllQuizzesManagementPage : AllQuizzesManagementPageT<AllQuizzesManagementPage>
    {
        AllQuizzesManagementPage();
        ~AllQuizzesManagementPage();

        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::QuizManagementItem> Quizzes() { return m_quizzes; }

        void OnSearchTextChanged(Microsoft::UI::Xaml::Controls::AutoSuggestBox const &sender, Microsoft::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs const &args);
        void OnPurgeClicked(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);

    private:
        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::QuizManagementItem> m_quizzes;
        ::quiz_examination_system::SupabaseClientAsync m_supabaseClient;
        std::vector<quiz_examination_system::QuizManagementItem> m_allQuizzes;
        Microsoft::UI::Dispatching::DispatcherQueueTimer m_messageTimer{nullptr};

        fire_and_forget LoadQuizzes();
        void FilterQuizzes(hstring const &searchText);
        fire_and_forget PurgeQuiz(hstring quizId, int32_t attemptCount);
        void ShowMessage(hstring const &title, hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity);
        fire_and_forget InsertAuditLog(hstring action, hstring targetTable, hstring targetId, hstring details);
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct AllQuizzesManagementPage : AllQuizzesManagementPageT<AllQuizzesManagementPage, implementation::AllQuizzesManagementPage>
    {
    };
}
