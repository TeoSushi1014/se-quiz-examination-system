#pragma once

#include "HistoryPage.g.h"
#include "AttemptItem.h"
#include "SupabaseClientAsync.h"
#include <memory>

namespace winrt::quiz_examination_system::implementation
{
    struct HistoryPage : HistoryPageT<HistoryPage>
    {
        HistoryPage();

        void Page_Loaded(winrt::Windows::Foundation::IInspectable const &sender,
                         Microsoft::UI::Xaml::RoutedEventArgs const &e);

    private:
        std::unique_ptr<::quiz_examination_system::SupabaseClientAsync> m_supabaseClient;
        hstring m_studentId;

        winrt::fire_and_forget LoadHistory();
        void ShowMessage(hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity);
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct HistoryPage : HistoryPageT<HistoryPage, implementation::HistoryPage>
    {
    };
}
