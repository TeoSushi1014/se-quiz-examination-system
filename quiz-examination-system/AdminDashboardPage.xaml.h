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

        void ManageUsers_Click(winrt::Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);

    private:
        std::unique_ptr<::quiz_examination_system::SupabaseClient> m_supabaseClient;
        hstring m_currentUserId;
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct AdminDashboardPage : AdminDashboardPageT<AdminDashboardPage, implementation::AdminDashboardPage>
    {
    };
}
