#pragma once

#include "UserManagementWindow.g.h"
#include <memory>

namespace winrt::quiz_examination_system::implementation
{
    struct UserManagementWindow : UserManagementWindowT<UserManagementWindow>
    {
    public:
        UserManagementWindow();

        void CreateUser_Click(IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void Refresh_Click(IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void ResetPassword_Click(IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void ToggleStatus_Click(IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);

    private:
        void LoadUsers();
        void ShowMessage(hstring message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity);

        hstring m_currentUsername;
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct UserManagementWindow : UserManagementWindowT<UserManagementWindow, implementation::UserManagementWindow>
    {
    };
}
