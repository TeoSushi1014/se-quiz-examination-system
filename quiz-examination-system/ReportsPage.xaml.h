#pragma once

#include "ReportsPage.g.h"
#include "ReportItem.h"
#include "SupabaseClientAsync.h"
#include "SupabaseClientManager.h"
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.Streams.h>

namespace winrt::quiz_examination_system::implementation
{
    struct QuizOption
    {
        hstring QuizId;
        hstring Title;
    };

    struct ReportsPage : ReportsPageT<ReportsPage>
    {
        ReportsPage();

        void Page_Loaded(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void QuizSelector_SelectionChanged(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const &e);
        void ExportCsv_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);

    private:
        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::ReportItem> m_reports;
        std::unique_ptr<::quiz_examination_system::SupabaseClientAsync> m_client;
        std::vector<QuizOption> m_quizOptions;
        hstring m_currentQuizId;
        hstring m_currentUserId;
        hstring m_currentRole;

        winrt::fire_and_forget LoadAvailableQuizzes();
        winrt::fire_and_forget LoadReportData(hstring const &quizId);
        winrt::fire_and_forget ExportToCsv();
        void ShowMessage(hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity);
        void UpdateSummaryCards(std::vector<::quiz_examination_system::SupabaseClient::AttemptReportRow> const &data);
        hstring FormatTimeSpent(int seconds);
        hstring FormatDateTime(hstring const &timestamp);
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct ReportsPage : ReportsPageT<ReportsPage, implementation::ReportsPage>
    {
    };
}
