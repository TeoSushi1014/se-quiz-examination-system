#include "pch.h"
#include "UserItem.h"
#if __has_include("UserItem.g.cpp")
#include "UserItem.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml::Data;

namespace winrt::quiz_examination_system::implementation
{
    UserItem::UserItem(hstring const &id, hstring const &username, hstring const &role, bool isActive, hstring const &email)
        : m_id(id), m_username(username), m_role(role), m_isActive(isActive), m_email(email)
    {
    }

    void UserItem::IsActive(bool value)
    {
        if (m_isActive != value)
        {
            m_isActive = value;
            m_propertyChanged(*this, PropertyChangedEventArgs(L"IsActive"));
            m_propertyChanged(*this, PropertyChangedEventArgs(L"StatusColor"));
            m_propertyChanged(*this, PropertyChangedEventArgs(L"StatusText"));
            m_propertyChanged(*this, PropertyChangedEventArgs(L"BlockButtonText"));
        }
    }

    hstring UserItem::StatusColor()
    {
        return m_isActive ? L"#107C10" : L"#605E5C";
    }

    hstring UserItem::StatusText()
    {
        return m_isActive ? L"Active" : L"Inactive";
    }

    hstring UserItem::RoleBadgeColor()
    {
        if (m_role == L"Admin")
            return L"#8B5CF6";
        if (m_role == L"Teacher")
            return L"#0078D4";
        return L"#00B7C3";
    }

    hstring UserItem::BlockButtonText()
    {
        return m_isActive ? L"Block Account" : L"Unblock Account";
    }

    winrt::event_token UserItem::PropertyChanged(PropertyChangedEventHandler const &handler)
    {
        return m_propertyChanged.add(handler);
    }

    void UserItem::PropertyChanged(winrt::event_token const &token) noexcept
    {
        m_propertyChanged.remove(token);
    }
}
