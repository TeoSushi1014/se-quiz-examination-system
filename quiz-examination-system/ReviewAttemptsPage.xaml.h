#pragma once

#include "ReviewAttemptsPage.g.h"
#include "AttemptItem.h"
#include "SupabaseClientAsync.h"
#include "SupabaseClientManager.h"
#include <winrt/Windows.Foundation.Collections.h>

namespace winrt::quiz_examination_system::implementation
{
    struct ReviewAttemptsPage : ReviewAttemptsPageT<ReviewAttemptsPage>
    {
        ReviewAttemptsPage();

        void Page_Loaded(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void QuizSelector_SelectionChanged(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const &e);
        void Refresh_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        winrt::fire_and_forget ExportCsv_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        winrt::fire_and_forget DeleteAttempt_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        winrt::fire_and_forget ToggleRelease_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);

    private:
        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::AttemptItem> m_attempts;
        std::unique_ptr<::quiz_examination_system::SupabaseClientAsync> m_client;

        struct QuizOption
        {
            hstring QuizId;
            hstring Title;
            hstring ResultVisibility;
            bool ResultReleased;
        };
        std::vector<QuizOption> m_quizOptions;
        hstring m_currentQuizId;

        winrt::fire_and_forget LoadQuizList();
        winrt::fire_and_forget LoadAttempts(hstring const &quizId);
        void ShowMessage(hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity);
        void UpdateReleaseButton();
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct ReviewAttemptsPage : ReviewAttemptsPageT<ReviewAttemptsPage, implementation::ReviewAttemptsPage>
    {
    };
}
