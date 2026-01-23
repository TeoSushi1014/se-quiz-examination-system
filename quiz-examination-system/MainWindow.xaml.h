#pragma once

#include "MainWindow.g.h"
#include "UserManagementWindow.xaml.h"
#include "SupabaseClient.h"
#include <vector>

namespace winrt::quiz_examination_system::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        void Login_Click(IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);
        void Logout_Click(IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);
        void ChangePassword_Click(IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);
        void ManageUsers_Click(IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);
        void ManageQuizzes_Click(IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);
        void ReviewAttempts_Click(IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);
        void TakeQuiz_Click(IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);
        void DashboardNav_SelectionChanged(IInspectable const &, IInspectable const &);
        void OnClosed(IInspectable const &, Microsoft::UI::Xaml::WindowEventArgs const &);

    private:
        void UpdateView();
        bool ConnectDatabase(hstring const &connectionString);
        void ApplyPermissions(hstring const &role);
        void OnLoginSuccess(hstring username, hstring displayRole, hstring dbRole);
        void OnLoginFailed(hstring message);
        void OnPasswordChanged(hstring message);
        void OnPasswordChangeFailed(hstring message);

        hstring m_connectionString{L"https://tuciofxdzzrzwzqsltps.supabase.co"};
        bool m_dbConnected{false};

        std::unique_ptr<::quiz_examination_system::SupabaseClient> m_supabaseClient;
        bool m_authenticated{false};
        hstring m_currentUser;
        hstring m_currentRole;
        hstring m_currentDbRole;
        bool m_isClosing{false};
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
