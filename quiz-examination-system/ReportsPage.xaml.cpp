#include "pch.h"
#include "ReportsPage.xaml.h"
#if __has_include("ReportsPage.g.cpp")
#include "ReportsPage.g.cpp"
#endif
#include "ReportItem.h"
#include "SupabaseClientManager.h"
#include "HttpHelper.h"
#include "SupabaseConfig.h"
#include "PageHelper.h"
#include "DateTimeHelper.h"
#include <algorithm>
#include <sstream>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Web.Http.Headers.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Foundation;
using namespace Windows::Data::Json;
using namespace Windows::Web::Http;
using namespace Windows::Web::Http::Headers;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::Streams;

namespace winrt::quiz_examination_system::implementation
{
    ReportsPage::ReportsPage()
    {
        InitializeComponent();
        m_reports = single_threaded_observable_vector<quiz_examination_system::ReportItem>();
        m_client = std::make_unique<::quiz_examination_system::SupabaseClientAsync>();

        auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
        m_currentUserId = manager.GetUserId();
        m_currentRole = manager.GetRole();
    }

    void ReportsPage::Page_Loaded(Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        LoadAvailableQuizzes();
    }

    winrt::fire_and_forget ReportsPage::LoadAvailableQuizzes()
    {
        auto lifetime = get_strong();

        LoadingRing().IsActive(true);

        try
        {
            hstring filterByCreator = (m_currentRole == L"Teacher") ? m_currentUserId : L"";

            hstring endpoint = ::quiz_examination_system::SupabaseConfig::GetRestEndpoint(L"quizzes");

            if (!filterByCreator.empty())
            {
                endpoint = ::quiz_examination_system::SupabaseConfig::GetRestEndpoint(L"quizzes?created_by=eq." + filterByCreator);
            }

            auto responseText = co_await ::quiz_examination_system::HttpHelper::SendSupabaseRequest(
                endpoint, L"", HttpMethod::Get());

            auto quizzesArray = JsonArray::Parse(responseText);

            m_quizOptions.clear();

            for (uint32_t i = 0; i < quizzesArray.Size(); ++i)
            {
                auto qObj = quizzesArray.GetObjectAt(i);
                QuizOption option;
                option.QuizId = qObj.GetNamedString(L"id", L"");
                option.Title = qObj.GetNamedString(L"title", L"");
                m_quizOptions.push_back(option);
            }

            QuizSelector().Items().Clear();
            for (auto const &opt : m_quizOptions)
            {
                ComboBoxItem item;
                item.Content(box_value(opt.Title));
                item.Tag(box_value(opt.QuizId));
                QuizSelector().Items().Append(item);
            }

            // Empty state is self-explanatory through ComboBox placeholder
        }
        catch (hresult_error const &ex)
        {
            ShowMessage(ex.message(), InfoBarSeverity::Error);
        }

        LoadingRing().IsActive(false);
    }

    void ReportsPage::QuizSelector_SelectionChanged(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const &)
    {
        auto combo = sender.as<ComboBox>();
        if (combo.SelectedIndex() >= 0)
        {
            auto selectedItem = combo.SelectedItem().as<ComboBoxItem>();
            auto quizId = unbox_value<hstring>(selectedItem.Tag());
            m_currentQuizId = quizId;

            LoadReportData(quizId);
            ExportCsvBtn().IsEnabled(true);
        }
        else
        {
            ExportCsvBtn().IsEnabled(false);
        }
    }

    winrt::fire_and_forget ReportsPage::LoadReportData(hstring const &quizId)
    {
        auto lifetime = get_strong();

        LoadingRing().IsActive(true);
        SummaryCards().Visibility(Visibility::Collapsed);
        EmptyStateText().Visibility(Visibility::Collapsed);

        try
        {
            hstring body = hstring(L"{\"p_quiz_id:\"") + quizId + L"\"}";
            hstring rpcEndpoint = ::quiz_examination_system::SupabaseConfig::GetRpcEndpoint(L"get_quiz_attempts_report");

            auto responseText = co_await ::quiz_examination_system::HttpHelper::SendSupabaseRequest(
                rpcEndpoint, body);

            auto attemptsArray = JsonArray::Parse(responseText);

            m_reports.Clear();

            std::vector<::quiz_examination_system::SupabaseClient::AttemptReportRow> reportData;

            for (uint32_t i = 0; i < attemptsArray.Size(); ++i)
            {
                auto aObj = attemptsArray.GetObjectAt(i);

                ::quiz_examination_system::SupabaseClient::AttemptReportRow row;
                row.student_id = aObj.GetNamedString(L"student_id", L"");
                row.username = aObj.GetNamedString(L"username", L"");
                row.attempt_number = static_cast<int>(aObj.GetNamedNumber(L"attempt_number", 0));
                row.score = static_cast<int>(aObj.GetNamedNumber(L"score", 0));
                row.total_points = static_cast<int>(aObj.GetNamedNumber(L"total_points", 0));
                row.correct_count = static_cast<int>(aObj.GetNamedNumber(L"correct_count", 0));
                row.incorrect_count = static_cast<int>(aObj.GetNamedNumber(L"incorrect_count", 0));
                row.time_spent_seconds = static_cast<int>(aObj.GetNamedNumber(L"time_spent_seconds", 0));

                reportData.push_back(row);

                hstring submittedAt = aObj.GetNamedString(L"submitted_at", L"");

                auto reportItem = make<ReportItem>(
                    row.username,
                    row.attempt_number,
                    static_cast<double>(row.score),
                    static_cast<double>(row.total_points),
                    FormatTimeSpent(row.time_spent_seconds),
                    FormatDateTime(submittedAt));

                m_reports.Append(reportItem);
            }

            ReportListView().ItemsSource(m_reports);

            if (m_reports.Size() == 0)
            {
                EmptyStateText().Visibility(Visibility::Visible);
            }
            else
            {
                UpdateSummaryCards(reportData);
                SummaryCards().Visibility(Visibility::Visible);
            }
        }
        catch (hresult_error const &ex)
        {
            ShowMessage(ex.message(), InfoBarSeverity::Error);
        }

        LoadingRing().IsActive(false);
    }

    void ReportsPage::UpdateSummaryCards(std::vector<::quiz_examination_system::SupabaseClient::AttemptReportRow> const &data)
    {
        int totalAttempts = static_cast<int>(data.size());
        double totalScore = 0.0;
        int passCount = 0;

        for (auto const &row : data)
        {
            totalScore += row.score;

            double percentage = (row.total_points > 0) ? (static_cast<double>(row.score) / row.total_points * 100.0) : 0.0;
            if (percentage >= 50.0)
            {
                passCount++;
            }
        }

        double avgScore = (totalAttempts > 0) ? (totalScore / totalAttempts) : 0.0;
        double passRate = (totalAttempts > 0) ? (static_cast<double>(passCount) / totalAttempts * 100.0) : 0.0;

        TotalAttemptsText().Text(to_hstring(totalAttempts));

        wchar_t avgBuffer[32];
        swprintf_s(avgBuffer, L"%.1f", avgScore);
        AverageScoreText().Text(hstring(avgBuffer));

        wchar_t passBuffer[32];
        swprintf_s(passBuffer, L"%.1f%%", passRate);
        PassRateText().Text(hstring(passBuffer));
    }

    hstring ReportsPage::FormatTimeSpent(int seconds)
    {
        return ::quiz_examination_system::DateTimeHelper::FormatTimeSpent(seconds);
    }

    hstring ReportsPage::FormatDateTime(hstring const &timestamp)
    {
        return ::quiz_examination_system::DateTimeHelper::FormatDateTime(timestamp);
    }

    void ReportsPage::ExportCsv_Click(Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        ExportToCsv();
    }

    winrt::fire_and_forget ReportsPage::ExportToCsv()
    {
        auto lifetime = get_strong();

        if (m_reports.Size() == 0)
        {
            ShowMessage(L"No data to export", InfoBarSeverity::Warning);
            co_return;
        }

        try
        {
            FileSavePicker savePicker;
            auto hwnd = GetActiveWindow();
            winrt::check_hresult(savePicker.as<IInitializeWithWindow>()->Initialize(hwnd));

            savePicker.SuggestedStartLocation(PickerLocationId::DocumentsLibrary);
            savePicker.FileTypeChoices().Insert(L"CSV File", single_threaded_vector<hstring>({L".csv"}));
            savePicker.SuggestedFileName(L"quiz_report");

            auto file = co_await savePicker.PickSaveFileAsync();

            if (file != nullptr)
            {
                std::wstring csvContent = L"\xFEFF";
                csvContent += L"Student,Attempt #,Score,Total Points,Time Spent,Submitted At\n";

                for (uint32_t i = 0; i < m_reports.Size(); ++i)
                {
                    auto item = m_reports.GetAt(i);
                    csvContent += std::wstring(item.StudentName()) + L",";
                    csvContent += std::to_wstring(item.AttemptNumber()) + L",";
                    csvContent += std::to_wstring(static_cast<int>(item.Score())) + L",";
                    csvContent += std::to_wstring(static_cast<int>(item.TotalPoints())) + L",";
                    csvContent += std::wstring(item.TimeSpent()) + L",";
                    csvContent += std::wstring(item.SubmittedDate()) + L"\n";
                }

                co_await FileIO::WriteTextAsync(file, hstring(csvContent));

                ShowMessage(L"Report exported successfully", InfoBarSeverity::Success);
            }
        }
        catch (hresult_error const &ex)
        {
            ShowMessage(ex.message(), InfoBarSeverity::Error);
        }
    }

    void ReportsPage::ShowMessage(hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity)
    {
        ::quiz_examination_system::PageHelper::ShowInfoBar(ActionMessage(), message, severity);
    }
}
