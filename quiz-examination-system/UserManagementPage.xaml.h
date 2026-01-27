#pragma once
#include "UserManagementPage.g.h"
#include "UserItem.h"
#include "SupabaseClientAsync.h"

namespace winrt::quiz_examination_system::implementation
{
    struct UserManagementPage : UserManagementPageT<UserManagementPage>
    {
        UserManagementPage();

        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::UserItem> Users() { return m_users; }

        void OnSearchTextChanged(Microsoft::UI::Xaml::Controls::AutoSuggestBox const &sender, Microsoft::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs const &args);
        void OnCreateUserClicked(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void OnEditRoleClicked(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void OnResetPasswordClicked(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void OnToggleStatusClicked(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void OnStatusToggled(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);

    private:
        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::UserItem> m_users;
        ::quiz_examination_system::SupabaseClientAsync m_supabaseClient;
        std::vector<quiz_examination_system::UserItem> m_allUsers;

        fire_and_forget LoadUsers();
        void FilterUsers(hstring const &searchText);
        fire_and_forget ShowCreateUserDialog();
        fire_and_forget ShowEditRoleDialog(hstring userId);
        fire_and_forget ShowResetPasswordDialog(hstring userId);
        fire_and_forget UpdateUserStatus(hstring userId, bool isActive);
        void ShowMessage(hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity);
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct UserManagementPage : UserManagementPageT<UserManagementPage, implementation::UserManagementPage>
    {
    };
}
