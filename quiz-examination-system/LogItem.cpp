#include "pch.h"
#include "LogItem.h"
#if __has_include("LogItem.g.cpp")
#include "LogItem.g.cpp"
#endif
#include <winrt/Windows.UI.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Media;
using namespace Windows::UI;

namespace winrt::quiz_examination_system::implementation
{
    LogItem::LogItem(hstring const &id, hstring const &action, hstring const &actorId,
                     hstring const &actorUsername, hstring const &targetTable,
                     hstring const &targetId, hstring const &details, hstring const &timestamp)
        : m_id(id), m_action(action), m_actorId(actorId), m_actorUsername(actorUsername),
          m_targetTable(targetTable), m_targetId(targetId), m_details(details), m_timestamp(timestamp)
    {
        OutputDebugStringW((L"[LogItem] Created: action=" + std::wstring(action) + L", actor=" + std::wstring(actorUsername) + L"\n").c_str());
    }

    Microsoft::UI::Xaml::Media::SolidColorBrush LogItem::ActionColor() const
    {
        OutputDebugStringW((L"[LogItem] ActionColor called for action: " + std::wstring(m_action) + L"\n").c_str());

        if (!m_actionColor)
        {
            OutputDebugStringW(L"[LogItem] Creating ActionColor brush\n");
            Color color;
            if (m_action.starts_with(L"DELETE_") || m_action.starts_with(L"PURGE_"))
            {
                color = ColorHelper::FromArgb(255, 209, 52, 56);
            }
            else if (m_action.starts_with(L"UPDATE_") || m_action.starts_with(L"DISABLE_"))
            {
                color = ColorHelper::FromArgb(255, 202, 80, 16);
            }
            else if (m_action.starts_with(L"CREATE_"))
            {
                color = ColorHelper::FromArgb(255, 16, 124, 16);
            }
            else if (m_action.starts_with(L"LOGIN_"))
            {
                color = ColorHelper::FromArgb(255, 0, 120, 212);
            }
            else
            {
                color = ColorHelper::FromArgb(255, 96, 94, 92);
            }
            m_actionColor = SolidColorBrush(color);
            OutputDebugStringW(L"[LogItem] ActionColor brush created\n");
        }
        else
        {
            OutputDebugStringW(L"[LogItem] Returning cached ActionColor brush\n");
        }
        return m_actionColor;
    }

    Microsoft::UI::Xaml::Media::SolidColorBrush LogItem::ActionBadgeColor() const
    {
        OutputDebugStringW((L"[LogItem] ActionBadgeColor called for action: " + std::wstring(m_action) + L"\n").c_str());

        if (!m_actionBadgeColor)
        {
            OutputDebugStringW(L"[LogItem] Creating ActionBadgeColor brush\n");
            Color color;
            if (m_action.starts_with(L"DELETE_") || m_action.starts_with(L"PURGE_"))
            {
                color = ColorHelper::FromArgb(255, 253, 231, 233);
            }
            else if (m_action.starts_with(L"UPDATE_") || m_action.starts_with(L"DISABLE_"))
            {
                color = ColorHelper::FromArgb(255, 255, 244, 206);
            }
            else if (m_action.starts_with(L"CREATE_"))
            {
                color = ColorHelper::FromArgb(255, 223, 246, 221);
            }
            else if (m_action.starts_with(L"LOGIN_"))
            {
                color = ColorHelper::FromArgb(255, 225, 245, 254);
            }
            else
            {
                color = ColorHelper::FromArgb(255, 243, 242, 241);
            }
            m_actionBadgeColor = SolidColorBrush(color);
            OutputDebugStringW(L"[LogItem] ActionBadgeColor brush created\n");
        }
        else
        {
            OutputDebugStringW(L"[LogItem] Returning cached ActionBadgeColor brush\n");
        }
        return m_actionBadgeColor;
    }

    hstring LogItem::FormattedTimestamp() const
    {
        std::wstring ts(m_timestamp);
        if (ts.length() >= 19)
        {
            std::wstring date = ts.substr(0, 10);
            std::wstring time = ts.substr(11, 8);
            return hstring(date + L" " + time);
        }
        return m_timestamp;
    }

    winrt::event_token LogItem::PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const &handler)
    {
        return m_propertyChanged.add(handler);
    }

    void LogItem::PropertyChanged(winrt::event_token const &token) noexcept
    {
        m_propertyChanged.remove(token);
    }

    void LogItem::RaisePropertyChanged(hstring const &propertyName)
    {
        m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(propertyName));
    }
}
