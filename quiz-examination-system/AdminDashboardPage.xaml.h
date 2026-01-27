#pragma once

#include "AdminDashboardPage.g.h"
#include "SupabaseClient.h"
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>

namespace winrt::quiz_examination_system::implementation
{
    struct AdminDashboardPage : AdminDashboardPageT<AdminDashboardPage>
    {
        AdminDashboardPage();

        void AdminNav_SelectionChanged(winrt::Microsoft::UI::Xaml::Controls::NavigationView const &, winrt::Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs const &);

    private:
        std::unique_ptr<::quiz_examination_system::SupabaseClient> m_supabaseClient;
        hstring m_currentUserId;
        void ShowMessage(hstring const &title, hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity);
        void NavigateToDashboard();
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct AdminDashboardPage : AdminDashboardPageT<AdminDashboardPage, implementation::AdminDashboardPage>
    {
    };
}
