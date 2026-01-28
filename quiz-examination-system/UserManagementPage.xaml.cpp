#include "pch.h"
#include "UserManagementPage.xaml.h"
#if __has_include("UserManagementPage.g.cpp")
#include "UserManagementPage.g.cpp"
#endif
#include "UserItem.h"
#include "BCryptPasswordHasher.h"
#include "SupabaseClientManager.h"
#include "PageHelper.h"
#include "HttpHelper.h"
#include "SupabaseConfig.h"
#include <algorithm>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Web.Http.Headers.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Foundation;
using namespace Windows::Data::Json;
using namespace Windows::Web::Http;
using namespace Windows::Web::Http::Headers;

namespace winrt::quiz_examination_system::implementation
{
    UserManagementPage::UserManagementPage()
    {
        InitializeComponent();
        m_users = single_threaded_observable_vector<quiz_examination_system::UserItem>();
        LoadUsers();
    }

    winrt::fire_and_forget UserManagementPage::LoadUsers()
    {
        auto lifetime = get_strong();

        OutputDebugStringW(L"[LoadUsers] Called - Starting user refresh\n");
        LoadingRing().IsActive(true);

        try
        {
            auto usersJson = co_await m_supabaseClient.GetAllUsersAsync();
            OutputDebugStringW((L"[LoadUsers] Received JSON: " + std::wstring(usersJson).substr(0, 100) + L"...\n").c_str());

            auto usersArray = JsonArray::Parse(usersJson);
            OutputDebugStringW((L"[LoadUsers] Parsed " + std::to_wstring(usersArray.Size()) + L" users\n").c_str());

            m_allUsers.clear();
            m_users.Clear();
            OutputDebugStringW(L"[LoadUsers] Cleared old collections\n");

            for (uint32_t i = 0; i < usersArray.Size(); i++)
            {
                auto userObj = usersArray.GetObjectAt(i);
                auto id = userObj.GetNamedString(L"id");
                auto username = userObj.GetNamedString(L"username");
                auto role = userObj.GetNamedString(L"role", L"Student");
                auto status = userObj.GetNamedString(L"status", L"Active");
                bool isActive = (status == L"Active");

                OutputDebugStringW((L"[LoadUsers] User: id='" + id + L"', username='" + username + L"'\n").c_str());

                auto userItem = make<UserItem>(id, username, role, isActive, L"");
                m_allUsers.push_back(userItem);
                m_users.Append(userItem);
            }

            OutputDebugStringW((L"[LoadUsers] SUCCESS - Added " + std::to_wstring(m_users.Size()) + L" users to UI\n").c_str());
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[LoadUsers] ERROR: " + ex.message() + L"\n").c_str());
            ShowMessage(L"Failed to load users: " + ex.message(), InfoBarSeverity::Error);
        }
        catch (...)
        {
            OutputDebugStringW(L"[LoadUsers] Unknown error\n");
        }

        LoadingRing().IsActive(false);
        OutputDebugStringW(L"[LoadUsers] Completed\n");
    }

    void UserManagementPage::OnSearchTextChanged(AutoSuggestBox const &sender, AutoSuggestBoxTextChangedEventArgs const &)
    {
        FilterUsers(sender.Text());
    }

    void UserManagementPage::FilterUsers(hstring const &searchText)
    {
        m_users.Clear();

        auto lowerSearch = to_string(searchText);
        std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);

        for (auto const &user : m_allUsers)
        {
            if (searchText.empty())
            {
                m_users.Append(user);
            }
            else
            {
                auto username = to_string(user.Username());
                std::transform(username.begin(), username.end(), username.begin(), ::tolower);

                if (username.find(lowerSearch) != std::string::npos)
                {
                    m_users.Append(user);
                }
            }
        }
    }

    void UserManagementPage::OnCreateUserClicked(IInspectable const &, RoutedEventArgs const &)
    {
        ShowCreateUserDialog();
    }

    winrt::fire_and_forget UserManagementPage::ShowCreateUserDialog()
    {
        auto lifetime = get_strong();

        ContentDialog dialog;
        dialog.XamlRoot(this->XamlRoot());
        dialog.Title(box_value(L"Create New User"));
        dialog.PrimaryButtonText(L"Create");
        dialog.CloseButtonText(L"Cancel");
        dialog.DefaultButton(ContentDialogButton::Primary);

        StackPanel panel;
        panel.Spacing(12);

        TextBox usernameBox;
        usernameBox.Header(box_value(L"Username"));
        usernameBox.PlaceholderText(L"Enter username");
        panel.Children().Append(usernameBox);

        PasswordBox passwordBox;
        passwordBox.Header(box_value(L"Password"));
        passwordBox.PlaceholderText(L"Minimum 8 characters");
        panel.Children().Append(passwordBox);

        ComboBox roleCombo;
        roleCombo.Header(box_value(L"Role"));
        roleCombo.Items().Append(box_value(L"Student"));
        roleCombo.Items().Append(box_value(L"Teacher"));
        roleCombo.Items().Append(box_value(L"Admin"));
        roleCombo.SelectedIndex(0);
        panel.Children().Append(roleCombo);

        dialog.Content(panel);

        auto result = co_await dialog.ShowAsync();

        if (result == ContentDialogResult::Primary)
        {
            auto username = usernameBox.Text();
            auto password = passwordBox.Password();
            auto role = unbox_value<hstring>(roleCombo.SelectedItem());

            if (username.empty() || password.empty())
            {
                ShowMessage(L"Username and password are required", InfoBarSeverity::Warning);
                co_return;
            }

            if (password.size() < 8)
            {
                ShowMessage(L"Password must be at least 8 characters", InfoBarSeverity::Warning);
                co_return;
            }

            LoadingRing().IsActive(true);

            try
            {
                auto adminUsername = ::quiz_examination_system::SupabaseClientManager::GetInstance().GetUsername();
                auto hashedPassword = ::quiz_examination_system::BCryptPasswordHasher::HashPassword(password);
                auto success = co_await m_supabaseClient.CreateUserAsync(adminUsername, username, hashedPassword, role);

                if (success)
                {
                    OutputDebugStringW(L"[CreateUser] User created successfully in Supabase\n");
                    ShowMessage(L"User created successfully", InfoBarSeverity::Success);

                    InsertAuditLog(L"CREATE_USER", L"users", L"", L"Created user: " + username + L" with role: " + role);

                    OutputDebugStringW(L"[CreateUser] Queueing LoadUsers refresh...\n");
                    DispatcherQueue().TryEnqueue(Microsoft::UI::Dispatching::DispatcherQueuePriority::Normal, [lifetime]()
                                                 { 
                                                     OutputDebugStringW(L"[CreateUser] DispatcherQueue executing - calling LoadUsers\n");
                                                     lifetime->LoadUsers(); });
                }
                else
                {
                    ShowMessage(L"Failed to create user", InfoBarSeverity::Error);
                }
            }
            catch (hresult_error const &ex)
            {
                ShowMessage(L"Error: " + ex.message(), InfoBarSeverity::Error);
            }

            LoadingRing().IsActive(false);
        }
    }

    void UserManagementPage::OnEditRoleClicked(IInspectable const &sender, RoutedEventArgs const &)
    {
        auto menuItem = sender.as<MenuFlyoutItem>();
        auto userId = unbox_value<hstring>(menuItem.Tag());
        ShowEditRoleDialog(userId);
    }

    void UserManagementPage::OnResetPasswordClicked(IInspectable const &sender, RoutedEventArgs const &)
    {
        OutputDebugStringW(L"[OnResetPasswordClicked] START\n");

        auto menuItem = sender.as<MenuFlyoutItem>();
        auto tagObj = menuItem.Tag();

        if (tagObj)
        {
            auto userId = unbox_value<hstring>(tagObj);
            OutputDebugStringW((L"[OnResetPasswordClicked] UserId from Tag: '" + userId + L"'\n").c_str());
            ShowResetPasswordDialog(userId);
        }
        else
        {
            OutputDebugStringW(L"[OnResetPasswordClicked] ERROR: Tag is null!\n");
            ShowMessage(L"Error: Cannot identify user", InfoBarSeverity::Error);
        }
    }

    winrt::fire_and_forget UserManagementPage::ShowEditRoleDialog(hstring userId)
    {
        auto lifetime = get_strong();

        ContentDialog dialog;
        dialog.XamlRoot(this->XamlRoot());
        dialog.Title(box_value(L"Edit User Role"));
        dialog.PrimaryButtonText(L"Save");
        dialog.CloseButtonText(L"Cancel");
        dialog.DefaultButton(ContentDialogButton::Primary);

        StackPanel panel;
        panel.Spacing(12);

        TextBlock infoText;
        infoText.Text(L"Select new role for this user");
        infoText.TextWrapping(TextWrapping::Wrap);
        panel.Children().Append(infoText);

        ComboBox roleCombo;
        roleCombo.Header(box_value(L"Role"));
        roleCombo.Items().Append(box_value(L"Student"));
        roleCombo.Items().Append(box_value(L"Teacher"));
        roleCombo.Items().Append(box_value(L"Admin"));
        roleCombo.SelectedIndex(0);
        panel.Children().Append(roleCombo);

        dialog.Content(panel);

        auto result = co_await dialog.ShowAsync();

        if (result == ContentDialogResult::Primary)
        {
            auto newRole = unbox_value<hstring>(roleCombo.SelectedItem());

            LoadingRing().IsActive(true);

            try
            {
                OutputDebugStringW((L"[EditRole] Updating role to: " + std::wstring(newRole) + L" for userId: " + std::wstring(userId) + L"\n").c_str());
                
                auto success = co_await m_supabaseClient.UpdateUserRoleAsync(userId, newRole);
                
                if (success)
                {
                    ShowMessage(L"Role updated to " + newRole, InfoBarSeverity::Success);
                    OutputDebugStringW(L"[EditRole] Refreshing user list\n");
                    LoadUsers();
                    
                    // Log the action
                    InsertAuditLog(L"CHANGE_ROLE", L"users", userId,
                        L"Changed role to " + newRole);
                }
                else
                {
                    ShowMessage(L"Failed to update role", InfoBarSeverity::Error);
                }
            }
            catch (hresult_error const &ex)
            {
                ShowMessage(L"Error: " + ex.message(), InfoBarSeverity::Error);
            }

            LoadingRing().IsActive(false);
        }
    }

    winrt::fire_and_forget UserManagementPage::ShowResetPasswordDialog(hstring userId)
    {
        auto lifetime = get_strong();

        ContentDialog dialog;
        dialog.XamlRoot(this->XamlRoot());
        dialog.Title(box_value(L"Reset Password"));
        dialog.PrimaryButtonText(L"Reset");
        dialog.CloseButtonText(L"Cancel");
        dialog.DefaultButton(ContentDialogButton::Primary);

        StackPanel panel;
        panel.Spacing(12);

        TextBlock infoText;
        infoText.Text(L"Enter new password for this user");
        infoText.TextWrapping(TextWrapping::Wrap);
        panel.Children().Append(infoText);

        PasswordBox newPasswordBox;
        newPasswordBox.Header(box_value(L"New Password"));
        newPasswordBox.PlaceholderText(L"Minimum 8 characters");
        panel.Children().Append(newPasswordBox);

        PasswordBox confirmPasswordBox;
        confirmPasswordBox.Header(box_value(L"Confirm Password"));
        confirmPasswordBox.PlaceholderText(L"Re-enter password");
        panel.Children().Append(confirmPasswordBox);

        TextBlock matchIndicator;
        matchIndicator.Foreground(Microsoft::UI::Xaml::Media::SolidColorBrush(Microsoft::UI::Colors::Red()));
        matchIndicator.FontSize(12);
        panel.Children().Append(matchIndicator);

        // Capture by value to avoid lifetime issues across co_await
        auto updateMatchIndicator = [newPasswordBox, confirmPasswordBox, matchIndicator]()
        {
            auto pwd1 = newPasswordBox.Password();
            auto pwd2 = confirmPasswordBox.Password();

            if (pwd2.empty())
            {
                matchIndicator.Text(L"");
            }
            else if (pwd1 == pwd2)
            {
                matchIndicator.Text(L"\u2713 Passwords match");
                matchIndicator.Foreground(Microsoft::UI::Xaml::Media::SolidColorBrush(Microsoft::UI::Colors::Green()));
            }
            else
            {
                matchIndicator.Text(L"\u2717 Passwords do not match");
                matchIndicator.Foreground(Microsoft::UI::Xaml::Media::SolidColorBrush(Microsoft::UI::Colors::Red()));
            }
        };

        newPasswordBox.PasswordChanged([updateMatchIndicator](auto const &, auto const &)
                                       { updateMatchIndicator(); });
        confirmPasswordBox.PasswordChanged([updateMatchIndicator](auto const &, auto const &)
                                           { updateMatchIndicator(); });

        dialog.Content(panel);

        auto result = co_await dialog.ShowAsync();

        if (result == ContentDialogResult::Primary)
        {
            auto newPassword = newPasswordBox.Password();
            auto confirmPassword = confirmPasswordBox.Password();

            if (newPassword.empty())
            {
                ShowMessage(L"Password cannot be empty", InfoBarSeverity::Warning);
                co_return;
            }

            if (newPassword.size() < 8)
            {
                ShowMessage(L"Password must be at least 8 characters", InfoBarSeverity::Warning);
                co_return;
            }

            if (newPassword != confirmPassword)
            {
                ShowMessage(L"Passwords do not match", InfoBarSeverity::Warning);
                co_return;
            }

            LoadingRing().IsActive(true);

            try
            {
                OutputDebugStringW(L"[ResetPassword] Starting password hash...\n");
                OutputDebugStringW((L"[ResetPassword] UserId: " + userId + L"\n").c_str());
                OutputDebugStringW((L"[ResetPassword] Password length: " + std::to_wstring(newPassword.size()) + L"\n").c_str());

                if (newPassword.empty())
                {
                    OutputDebugStringW(L"[ResetPassword] ERROR: Password is empty!\n");
                    ShowMessage(L"Password cannot be empty", InfoBarSeverity::Error);
                    LoadingRing().IsActive(false);
                    co_return;
                }

                hstring hashedPassword;
                try
                {
                    hashedPassword = ::quiz_examination_system::BCryptPasswordHasher::HashPassword(newPassword);
                    OutputDebugStringW(L"[ResetPassword] Hash generated successfully\n");
                    OutputDebugStringW((L"[ResetPassword] Hash length: " + std::to_wstring(hashedPassword.size()) + L"\n").c_str());

                    if (hashedPassword.empty())
                    {
                        OutputDebugStringW(L"[ResetPassword] ERROR: Hash is empty!\n");
                        ShowMessage(L"Failed to generate password hash", InfoBarSeverity::Error);
                        LoadingRing().IsActive(false);
                        co_return;
                    }
                }
                catch (std::invalid_argument const &ex)
                {
                    OutputDebugStringW((L"[ResetPassword] invalid_argument: " + winrt::to_hstring(ex.what()) + L"\n").c_str());
                    ShowMessage(L"Invalid password format", InfoBarSeverity::Error);
                    LoadingRing().IsActive(false);
                    co_return;
                }
                catch (...)
                {
                    OutputDebugStringW(L"[ResetPassword] Hash generation FAILED\n");
                    ShowMessage(L"Failed to generate password hash", InfoBarSeverity::Error);
                    LoadingRing().IsActive(false);
                    co_return;
                }

                OutputDebugStringW(L"[ResetPassword] Calling ResetUserPasswordAsync...\n");
                auto success = co_await m_supabaseClient.ResetUserPasswordAsync(userId, hashedPassword);
                OutputDebugStringW((L"[ResetPassword] Result: " + std::to_wstring(success) + L"\n").c_str());

                if (success)
                {
                    ShowMessage(L"Password reset successfully", InfoBarSeverity::Success);
                    InsertAuditLog(L"RESET_PASSWORD", L"users", userId, L"Password reset for user: " + userId);
                }
                else
                {
                    ShowMessage(L"Failed to reset password", InfoBarSeverity::Error);
                }
            }
            catch (hresult_error const &ex)
            {
                OutputDebugStringW((L"[ResetPassword] hresult_error: " + ex.message() + L"\n").c_str());
                ShowMessage(L"Error: " + ex.message(), InfoBarSeverity::Error);
            }
            catch (...)
            {
                OutputDebugStringW(L"[ResetPassword] Unknown exception\n");
                ShowMessage(L"Unknown error occurred", InfoBarSeverity::Error);
            }

            LoadingRing().IsActive(false);
        }
    }

    void UserManagementPage::OnToggleStatusClicked(IInspectable const &sender, RoutedEventArgs const &)
    {
        auto menuItem = sender.try_as<MenuFlyoutItem>();
        if (!menuItem)
        {
            OutputDebugStringW(L"[OnToggleStatusClicked] ERROR - sender is not MenuFlyoutItem\n");
            return;
        }

        auto tag = menuItem.Tag();
        if (!tag)
        {
            OutputDebugStringW(L"[OnToggleStatusClicked] ERROR - Tag is null\n");
            return;
        }

        auto userId = unbox_value<hstring>(tag);
        OutputDebugStringW((L"[OnToggleStatusClicked] userId: " + std::wstring(userId) + L"\n").c_str());

        bool currentStatus = false;
        bool foundUser = false;

        for (size_t i = 0; i < m_allUsers.size(); i++)
        {
            auto user = m_allUsers[i];
            auto id = user.Id();
            if (id == userId)
            {
                currentStatus = user.IsActive();
                foundUser = true;
                OutputDebugStringW((L"[OnToggleStatusClicked] Found user, current status: " + std::to_wstring(currentStatus) + L"\n").c_str());
                break;
            }
        }

        if (foundUser)
        {
            UpdateUserStatus(userId, !currentStatus);
        }
    }

    void UserManagementPage::OnStatusToggled(IInspectable const &sender, RoutedEventArgs const &)
    {
        auto toggle = sender.as<ToggleSwitch>();
        auto userId = unbox_value<hstring>(toggle.Tag());
        auto isActive = toggle.IsOn();

        UpdateUserStatus(userId, isActive);
    }

    winrt::fire_and_forget UserManagementPage::UpdateUserStatus(hstring userId, bool isActive)
    {
        auto lifetime = get_strong();

        try
        {
            OutputDebugStringW((L"[UpdateUserStatus] Updating status for userId: " + std::wstring(userId) + L"\n").c_str());
            auto success = co_await m_supabaseClient.UpdateUserStatusAsync(userId, isActive);

            if (success)
            {
                OutputDebugStringW(L"[UpdateUserStatus] Database updated, refreshing UI list...\n");
                ShowMessage(isActive ? L"User activated" : L"User deactivated", InfoBarSeverity::Success);

                InsertAuditLog(isActive ? L"ENABLE_USER" : L"DISABLE_USER", L"users", userId,
                               L"User status changed to " + (isActive ? hstring(L"Active") : hstring(L"Inactive")));

                LoadUsers();
            }
            else
            {
                ShowMessage(L"Failed to update status", InfoBarSeverity::Error);
            }
        }
        catch (hresult_error const &ex)
        {
            ShowMessage(L"Error: " + ex.message(), InfoBarSeverity::Error);
        }
    }

    void UserManagementPage::ShowMessage(hstring const &message, InfoBarSeverity severity)
    {
        ::quiz_examination_system::PageHelper::ShowInfoBar(MessageBar(), message, severity);
    }

    winrt::fire_and_forget UserManagementPage::InsertAuditLog(hstring action, hstring targetTable, hstring targetId, hstring details)
    {
        auto lifetime = get_strong();

        try
        {
            auto actorUsername = ::quiz_examination_system::SupabaseClientManager::GetInstance().GetUsername();
            auto actorId = ::quiz_examination_system::SupabaseClientManager::GetInstance().GetUserId();

            JsonObject logData;
            logData.SetNamedValue(L"action", JsonValue::CreateStringValue(action));
            logData.SetNamedValue(L"actor_id", JsonValue::CreateStringValue(actorId));

            if (!targetTable.empty())
            {
                logData.SetNamedValue(L"target_table", JsonValue::CreateStringValue(targetTable));
            }

            if (!targetId.empty())
            {
                logData.SetNamedValue(L"target_id", JsonValue::CreateStringValue(targetId));
            }

            JsonObject detailsObj;
            detailsObj.SetNamedValue(L"description", JsonValue::CreateStringValue(details));
            logData.SetNamedValue(L"details", detailsObj);

            hstring jsonBody = logData.Stringify();

            hstring endpoint = ::quiz_examination_system::SupabaseConfig::GetRestEndpoint(L"audit_logs");
            co_await ::quiz_examination_system::HttpHelper::SendSupabaseRequest(endpoint, jsonBody);

            OutputDebugStringW((L"[InsertAuditLog] Log inserted: " + action + L"\n").c_str());
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[InsertAuditLog] ERROR: " + ex.message() + L"\n").c_str());
        }
        catch (...)
        {
            OutputDebugStringW(L"[InsertAuditLog] Unknown error\n");
        }
    }
}
