#pragma once

#include "TeacherDashboardPage.g.h"
#include "SupabaseClient.h"
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>

namespace winrt::quiz_examination_system::implementation
{
    struct TeacherDashboardPage : TeacherDashboardPageT<TeacherDashboardPage>
    {
        TeacherDashboardPage();

        void TeacherNav_SelectionChanged(Microsoft::UI::Xaml::Controls::NavigationView const &, Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs const &);
        void QuestionBank_Click(winrt::Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);
        void ManageQuizzes_Click(winrt::Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);
        void ReviewAttempts_Click(winrt::Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);

    private:
        std::unique_ptr<::quiz_examination_system::SupabaseClient> m_supabaseClient;
        hstring m_currentUserId;
        void UpdateContent(hstring const &section);
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct TeacherDashboardPage : TeacherDashboardPageT<TeacherDashboardPage, implementation::TeacherDashboardPage>
    {
    };
}
