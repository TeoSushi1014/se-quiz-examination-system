#pragma once
#include "LogItem.g.h"

namespace winrt::quiz_examination_system::implementation
{
    struct LogItem : LogItemT<LogItem>
    {
        LogItem(hstring const &id, hstring const &action, hstring const &actorId,
                hstring const &actorUsername, hstring const &targetTable,
                hstring const &targetId, hstring const &details, hstring const &timestamp);

        hstring Id() const { return m_id; }
        hstring Action() const { return m_action; }
        hstring ActorId() const { return m_actorId; }
        hstring ActorUsername() const { return m_actorUsername; }
        hstring TargetTable() const { return m_targetTable; }
        hstring TargetId() const { return m_targetId; }
        hstring Details() const { return m_details; }
        hstring Timestamp() const { return m_timestamp; }
        Microsoft::UI::Xaml::Media::SolidColorBrush ActionColor() const;
        Microsoft::UI::Xaml::Media::SolidColorBrush ActionBadgeColor() const;
        hstring FormattedTimestamp() const;

        winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const &handler);
        void PropertyChanged(winrt::event_token const &token) noexcept;

    private:
        hstring m_id;
        hstring m_action;
        hstring m_actorId;
        hstring m_actorUsername;
        hstring m_targetTable;
        hstring m_targetId;
        hstring m_details;
        hstring m_timestamp;
        mutable Microsoft::UI::Xaml::Media::SolidColorBrush m_actionColor{nullptr};
        mutable Microsoft::UI::Xaml::Media::SolidColorBrush m_actionBadgeColor{nullptr};
        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

        void RaisePropertyChanged(hstring const &propertyName);
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct LogItem : LogItemT<LogItem, implementation::LogItem>
    {
    };
}
