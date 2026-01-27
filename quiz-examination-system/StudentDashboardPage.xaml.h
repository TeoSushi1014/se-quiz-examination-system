#pragma once

#include "StudentDashboardPage.g.h"
#include "SupabaseClient.h"
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>

namespace winrt::quiz_examination_system::implementation
{
    struct StudentDashboardPage : StudentDashboardPageT<StudentDashboardPage>
    {
        StudentDashboardPage();

        void TakeQuiz_Click(winrt::Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);

        // UC04: Demo Submit Attempt
        void DemoSubmitAttempt_Click(winrt::Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);

        // UC04/UC05: Demo View Quizzes
        void DemoLoadQuizzes_Click(winrt::Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);

        // UC05: Demo View Attempt Results
        void DemoViewAttemptResults_Click(winrt::Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);
        void DemoViewAttemptDetails_Click(winrt::Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);

    private:
        std::unique_ptr<::quiz_examination_system::SupabaseClient> m_supabaseClient;
        hstring m_currentUserId;
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct StudentDashboardPage : StudentDashboardPageT<StudentDashboardPage, implementation::StudentDashboardPage>
    {
    };
}
