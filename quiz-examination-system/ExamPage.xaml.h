#pragma once

#include "ExamPage.g.h"
#include "SupabaseClient.h"
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Dispatching.h>

namespace winrt::quiz_examination_system::implementation
{
    struct ExamPage : ExamPageT<ExamPage>
    {
        ExamPage();
        ExamPage(hstring const &quizId, hstring const &studentId);

        void SetQuizData(hstring const &quizId, hstring const &studentId)
        {
            m_quizId = quizId;
            m_studentId = studentId;
        }

        void Page_Loaded(winrt::Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void OnNavigatedTo(Microsoft::UI::Xaml::Navigation::NavigationEventArgs const &e);
        void QuestionListView_SelectionChanged(winrt::Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const &e);
        void PrevButton_Click(winrt::Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void NextButton_Click(winrt::Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        winrt::fire_and_forget SubmitButton_Click(winrt::Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        winrt::fire_and_forget ExitButton_Click(winrt::Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);

    private:
        std::unique_ptr<::quiz_examination_system::SupabaseClient> m_supabaseClient;
        hstring m_quizId;
        hstring m_studentId;
        std::vector<::quiz_examination_system::SupabaseClient::QuestionData> m_questions;
        std::map<hstring, hstring> m_answers;
        int m_currentQuestionIndex{0};
        int m_timeRemainingSeconds{0};
        int m_timeLimitSeconds{0};
        winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer m_timer{nullptr};

        void LoadQuizData();
        void LoadQuestions();
        winrt::fire_and_forget ValidateAccessAndLoad();
        void DisplayQuestion(int index);
        void SaveCurrentAnswer();
        void StartTimer();
        void UpdateTimerDisplay();
        void SubmitAttempt();
        void ShowMessage(hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity);
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct ExamPage : ExamPageT<ExamPage, implementation::ExamPage>
    {
    };
}
