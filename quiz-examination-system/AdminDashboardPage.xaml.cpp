#include "pch.h"
#include "AdminDashboardPage.xaml.h"
#if __has_include("AdminDashboardPage.g.cpp")
#include "AdminDashboardPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::quiz_examination_system::implementation
{
    AdminDashboardPage::AdminDashboardPage()
    {
        InitializeComponent();
    }

    void AdminDashboardPage::ManageUsers_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);
        ActionMessage().Message(L"Open user management window");
        ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Informational);
        ActionMessage().IsOpen(true);
    }

    void AdminDashboardPage::ShowMessage(hstring const &title, hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity)
    {
        ActionMessage().Title(title);
        ActionMessage().Message(message);
        ActionMessage().Severity(severity);
        ActionMessage().IsOpen(true);
    }

    // =====================================================
    // UC08: Admin Quiz Management - Purge Demo
    // =====================================================

    void AdminDashboardPage::DemoPurgeQuiz_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ShowMessage(L"Admin Purge", L"Calling RPC: purge_quiz_admin (CASCADE delete)...", Microsoft::UI::Xaml::Controls::InfoBarSeverity::Warning);

        if (!m_supabaseClient)
        {
            m_supabaseClient = std::make_unique<::quiz_examination_system::SupabaseClient>();
        }

        // Set callback
        m_supabaseClient->OnQuizPurgeResult = [this](bool success, int attemptsDeleted, hstring message)
        {
            if (success)
            {
                hstring msg = message + L" - Deleted " + winrt::to_hstring(attemptsDeleted) + L" attempt(s). Action logged to audit_logs.";
                ShowMessage(L"Purge Complete", msg, Microsoft::UI::Xaml::Controls::InfoBarSeverity::Success);
            }
            else
            {
                ShowMessage(L"Purge Failed", message, Microsoft::UI::Xaml::Controls::InfoBarSeverity::Error);
            }
        };

        // WARNING: This will permanently delete quiz and all attempts!
        // In production, should show ContentDialog confirmation first
        m_supabaseClient->PurgeQuizAsAdmin(L"Q01", L"001");
    }
}
