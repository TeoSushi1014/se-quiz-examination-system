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
