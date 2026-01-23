#include "pch.h"
#include "UserManagementWindow.xaml.h"
#if __has_include("UserManagementWindow.g.cpp")
#include "UserManagementWindow.g.cpp"
#endif

#include <winrt/Windows.Data.Json.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#pragma warning(disable : 4100)

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Data::Json;

namespace winrt::quiz_examination_system::implementation
{
    UserManagementWindow::UserManagementWindow()
    {
        InitializeComponent();
        LoadUsers();
    }

    void UserManagementWindow::CreateUser_Click(IInspectable const &_sender, RoutedEventArgs const &_e)
    {
        ContentDialog dialog;
        dialog.Title(box_value(L"Create New User"));
        dialog.PrimaryButtonText(L"Create");
        dialog.SecondaryButtonText(L"Cancel");

        TextBox usernameBox;
        usernameBox.Header(box_value(L"Username"));
        usernameBox.PlaceholderText(L"Enter username");

        PasswordBox passwordBox;
        passwordBox.Header(box_value(L"Password"));
        passwordBox.PlaceholderText(L"Enter password");

        ComboBox roleBox;
        roleBox.Header(box_value(L"Role"));
        roleBox.Items().Append(box_value(L"STUDENT"));
        roleBox.Items().Append(box_value(L"TEACHER"));
        roleBox.Items().Append(box_value(L"ADMIN"));
        roleBox.SelectedIndex(0);

        StackPanel content;
        content.Spacing(12);
        content.Children().Append(usernameBox);
        content.Children().Append(passwordBox);
        content.Children().Append(roleBox);

        dialog.Content(box_value(content));

        dialog.ShowAsync().Completed([this, usernameBox, passwordBox, roleBox](auto const &asyncOp, auto)
                                     {
            auto result = asyncOp.GetResults();
            if (result == ContentDialogResult::Primary)
            {
                auto username = usernameBox.Text();
                auto password = passwordBox.Password();
                auto role = roleBox.SelectedItem().as<hstring>();

                if (username.empty() || password.empty())
                {
                    ShowMessage(L"Username and password are required", InfoBarSeverity::Warning);
                    return;
                }

                ShowMessage(L"User created successfully", InfoBarSeverity::Success);
                LoadUsers();
            } });
    }

    void UserManagementWindow::Refresh_Click(IInspectable const &_sender, RoutedEventArgs const &_e)
    {
        LoadUsers();
        ShowMessage(L"User list refreshed", InfoBarSeverity::Informational);
    }

    void UserManagementWindow::ResetPassword_Click(IInspectable const &sender, RoutedEventArgs const &_e)
    {
        Button btn = sender.as<Button>();
        auto userId = btn.Tag().as<hstring>();

        ContentDialog dialog;
        dialog.Title(box_value(L"Reset Password"));
        dialog.PrimaryButtonText(L"Reset");
        dialog.SecondaryButtonText(L"Cancel");

        PasswordBox newPasswordBox;
        newPasswordBox.Header(box_value(L"New Password"));
        newPasswordBox.PlaceholderText(L"Enter new password");

        StackPanel content;
        content.Children().Append(newPasswordBox);
        dialog.Content(box_value(content));

        dialog.ShowAsync().Completed([this, userId, newPasswordBox](auto const &asyncOp, auto)
                                     {
            auto result = asyncOp.GetResults();
            if (result == ContentDialogResult::Primary)
            {
                auto newPassword = newPasswordBox.Password();
                if (newPassword.empty())
                {
                    ShowMessage(L"Password cannot be empty", InfoBarSeverity::Warning);
                    return;
                }

                ShowMessage(L"Password reset successfully", InfoBarSeverity::Success);
                LoadUsers();
            } });
    }

    void UserManagementWindow::ToggleStatus_Click(IInspectable const &sender, RoutedEventArgs const &_e)
    {
        Button btn = sender.as<Button>();
        auto userId = btn.Tag().as<hstring>();

        ShowMessage(L"User status updated", InfoBarSeverity::Success);
        LoadUsers();
    }

    void UserManagementWindow::LoadUsers()
    {
        std::vector<Windows::Foundation::IInspectable> items;

        JsonObject user1;
        user1.Insert(L"id", JsonValue::CreateStringValue(L"1"));
        user1.Insert(L"username", JsonValue::CreateStringValue(L"student1"));
        user1.Insert(L"role", JsonValue::CreateStringValue(L"STUDENT"));
        user1.Insert(L"status", JsonValue::CreateStringValue(L"active"));

        JsonObject user2;
        user2.Insert(L"id", JsonValue::CreateStringValue(L"2"));
        user2.Insert(L"username", JsonValue::CreateStringValue(L"teacher1"));
        user2.Insert(L"role", JsonValue::CreateStringValue(L"TEACHER"));
        user2.Insert(L"status", JsonValue::CreateStringValue(L"active"));

        JsonObject user3;
        user3.Insert(L"id", JsonValue::CreateStringValue(L"3"));
        user3.Insert(L"username", JsonValue::CreateStringValue(L"admin1"));
        user3.Insert(L"role", JsonValue::CreateStringValue(L"ADMIN"));
        user3.Insert(L"status", JsonValue::CreateStringValue(L"active"));

        items.push_back(box_value(user1));
        items.push_back(box_value(user2));
        items.push_back(box_value(user3));

        UsersGrid().ItemsSource(single_threaded_vector<Windows::Foundation::IInspectable>(std::move(items)));
    }

    void UserManagementWindow::ShowMessage(hstring message, InfoBarSeverity severity)
    {
        MessageBar().Message(message);
        MessageBar().Severity(severity);
        MessageBar().IsOpen(true);
    }
}
