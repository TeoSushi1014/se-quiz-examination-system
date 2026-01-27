#pragma once
#include "UserItem.g.h"

namespace winrt::quiz_examination_system::implementation
{
    struct UserItem : UserItemT<UserItem>
    {
        UserItem(hstring const &id, hstring const &username, hstring const &role, bool isActive, hstring const &email);

        hstring Id() { return m_id; }
        hstring Username() { return m_username; }
        hstring Role() { return m_role; }
        hstring Email() { return m_email; }
        bool IsActive() { return m_isActive; }
        void IsActive(bool value);
        hstring StatusColor();
        hstring StatusText();
        hstring RoleBadgeColor();
        hstring BlockButtonText();

        winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const &handler);
        void PropertyChanged(winrt::event_token const &token) noexcept;

    private:
        hstring m_id;
        hstring m_username;
        hstring m_role;
        hstring m_email;
        bool m_isActive;
        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct UserItem : UserItemT<UserItem, implementation::UserItem>
    {
    };
}
