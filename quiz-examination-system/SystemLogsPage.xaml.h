#pragma once
#include "SystemLogsPage.g.h"
#include "LogItem.h"
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Web.Http.Headers.h>

namespace winrt::quiz_examination_system::implementation
{
    struct SystemLogsPage : SystemLogsPageT<SystemLogsPage>
    {
        SystemLogsPage();

        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::LogItem> Logs() { return m_logs; }

        void OnSearchTextChanged(Microsoft::UI::Xaml::Controls::AutoSuggestBox const &sender,
                                 Microsoft::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs const &args);
        void OnActionFilterChanged(Windows::Foundation::IInspectable const &sender,
                                   Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const &args);
        void OnRefreshClicked(Windows::Foundation::IInspectable const &sender,
                              Microsoft::UI::Xaml::RoutedEventArgs const &args);

    private:
        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::LogItem> m_logs;
        std::vector<quiz_examination_system::LogItem> m_allLogs;
        hstring m_currentActionFilter = L"All";
        hstring m_currentSearchText = L"";

        winrt::fire_and_forget LoadLogs();
        void FilterLogs();
        void ShowMessage(hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity);
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct SystemLogsPage : SystemLogsPageT<SystemLogsPage, implementation::SystemLogsPage>
    {
    };
}
