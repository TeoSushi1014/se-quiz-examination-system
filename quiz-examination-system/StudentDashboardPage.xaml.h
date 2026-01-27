#pragma once

#include "StudentDashboardPage.g.h"
#include "QuizItemStudent.h"
#include "SupabaseClient.h"
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>

namespace winrt::quiz_examination_system::implementation
{
    struct StudentDashboardPage : StudentDashboardPageT<StudentDashboardPage>
    {
        StudentDashboardPage();

        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::QuizItemStudent> Quizzes();

        void SetStudentId(hstring const &studentId) { m_currentUserId = studentId; }

        void Page_Loaded(winrt::Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void QuizzesGridView_SelectionChanged(winrt::Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const &e);
        void StartExam_Click(winrt::Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);

    private:
        std::unique_ptr<::quiz_examination_system::SupabaseClient> m_supabaseClient;
        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::QuizItemStudent> m_quizzes{nullptr};
        quiz_examination_system::QuizItemStudent m_selectedQuiz{nullptr};
        hstring m_currentUserId;

        void LoadQuizzes();
        void ShowMessage(hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity);
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct StudentDashboardPage : StudentDashboardPageT<StudentDashboardPage, implementation::StudentDashboardPage>
    {
    };
}
