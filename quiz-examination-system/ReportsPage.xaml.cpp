#include "pch.h"
#include "ReportsPage.xaml.h"
#if __has_include("ReportsPage.g.cpp")
#include "ReportsPage.g.cpp"
#endif
#include "ReportItem.h"
#include "ReviewAttemptsPage.xaml.h"
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

            if (!filterByCreator.empty())
            {
                std::wstring trimmed(filterByCreator);
                auto pos = trimmed.find_last_not_of(L' ');
                if (pos != std::wstring::npos)
                {
                    trimmed.erase(pos + 1);
                }
                filterByCreator = hstring(trimmed);
            }

            hstring endpoint = ::quiz_examination_system::SupabaseConfig::GetRestEndpoint(L"quizzes?select=*");

            if (!filterByCreator.empty())
            {
                endpoint = endpoint + L"&created_by=eq." + filterByCreator;
            }

            OutputDebugStringW(L"[ReportsPage] Fetching quizzes...\n");
            OutputDebugStringW((L"[ReportsPage] Endpoint: " + std::wstring(endpoint) + L"\n").c_str());
            auto responseText = co_await ::quiz_examination_system::HttpHelper::SendSupabaseRequest(
                endpoint, L"", HttpMethod::Get());

            OutputDebugStringW((L"[ReportsPage] Response length: " + std::to_wstring(responseText.size()) + L"\n").c_str());
            if (responseText.size() > 0 && responseText.size() < 500)
            {
                OutputDebugStringW((L"[ReportsPage] Response text: " + std::wstring(responseText) + L"\n").c_str());
            }

            if (responseText.empty())
            {
                OutputDebugStringW(L"[ReportsPage] Empty quiz response\n");
                m_quizOptions.clear();
                QuizSelector().Items().Clear();
                LoadingRing().IsActive(false);
                co_return;
            }

            std::wstring jsonStr(responseText);
            if (jsonStr == L"null" || jsonStr == L"[]")
            {
                OutputDebugStringW(L"[ReportsPage] No quizzes available\n");
                m_quizOptions.clear();
                QuizSelector().Items().Clear();
                LoadingRing().IsActive(false);
                co_return;
            }

            if (jsonStr.size() < 2)
            {
                OutputDebugStringW(L"[ReportsPage] Invalid JSON - too short\n");
                m_quizOptions.clear();
                QuizSelector().Items().Clear();
                LoadingRing().IsActive(false);
                co_return;
            }

            if (jsonStr[0] == L'{' && jsonStr.find(L"\"code\":") != std::wstring::npos)
            {
                OutputDebugStringW(L"[ReportsPage] Server returned error response\n");
                try
                {
                    auto errorObj = JsonObject::Parse(responseText);
                    auto errorMsg = errorObj.GetNamedString(L"message", L"Unknown error");
                    OutputDebugStringW((L"[ReportsPage] Error: " + std::wstring(errorMsg) + L"\n").c_str());
                    ShowMessage(L"Failed to load quizzes: " + errorMsg, InfoBarSeverity::Error);
                }
                catch (...)
                {
                    ShowMessage(L"Failed to load quizzes from server", InfoBarSeverity::Error);
                }
                m_quizOptions.clear();
                QuizSelector().Items().Clear();
                LoadingRing().IsActive(false);
                co_return;
            }

            JsonArray quizzesArray{nullptr};
            try
            {
                OutputDebugStringW(L"[ReportsPage] Parsing quiz JSON...\n");
                quizzesArray = JsonArray::Parse(responseText);
                OutputDebugStringW(L"[ReportsPage] JSON parsed successfully\n");
            }
            catch (hresult_error const &parseEx)
            {
                OutputDebugStringW(L"[ReportsPage] JSON Parse Error in LoadAvailableQuizzes\n");
                ShowMessage(L"Invalid data format from server", InfoBarSeverity::Error);
                LoadingRing().IsActive(false);
                co_return;
            }

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
        }
        catch (hresult_error const &)
        {
            OutputDebugStringW(L"[ReportsPage] Error in LoadAvailableQuizzes\n");
            ShowMessage(L"Failed to load quizzes", InfoBarSeverity::Error);
        }
        catch (...)
        {
            OutputDebugStringW(L"[ReportsPage] Unknown error in LoadAvailableQuizzes\n");
            ShowMessage(L"An unexpected error occurred", InfoBarSeverity::Error);
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
            OutputDebugStringW(L"[ReportsPage] Loading report data...\n");
            auto responseText = co_await m_client->GetQuizAttemptsForReviewAsync(quizId);

            if (responseText.empty())
            {
                OutputDebugStringW(L"[ReportsPage] Empty report response\n");
                m_reports.Clear();
                EmptyStateText().Text(L"No results available to display or export");
                EmptyStateText().Visibility(Visibility::Visible);
                SummaryCards().Visibility(Visibility::Collapsed);
                LoadingRing().IsActive(false);
                co_return;
            }

            std::wstring jsonStr(responseText);
            if (jsonStr == L"null" || jsonStr == L"[]")
            {
                OutputDebugStringW(L"[ReportsPage] No report data available\n");
                m_reports.Clear();
                EmptyStateText().Text(L"No results available to display or export");
                EmptyStateText().Visibility(Visibility::Visible);
                SummaryCards().Visibility(Visibility::Collapsed);
                LoadingRing().IsActive(false);
                co_return;
            }

            if (jsonStr.size() < 2)
            {
                OutputDebugStringW(L"[ReportsPage] Invalid report JSON - too short\n");
                m_reports.Clear();
                EmptyStateText().Text(L"No results available to display or export");
                EmptyStateText().Visibility(Visibility::Visible);
                SummaryCards().Visibility(Visibility::Collapsed);
                LoadingRing().IsActive(false);
                co_return;
            }

            JsonArray attemptsArray{nullptr};
            try
            {
                OutputDebugStringW(L"[ReportsPage] Parsing report JSON...\n");
                attemptsArray = JsonArray::Parse(responseText);
                OutputDebugStringW(L"[ReportsPage] Report JSON parsed successfully\n");
            }
            catch (hresult_error const &)
            {
                OutputDebugStringW(L"[ReportsPage] JSON Parse Error in LoadReportData\n");
                m_reports.Clear();
                ShowMessage(L"Invalid report data format from server", InfoBarSeverity::Error);
                LoadingRing().IsActive(false);
                co_return;
            }

            m_reports.Clear();

            std::vector<::quiz_examination_system::SupabaseClient::AttemptReportRow> reportData;

            for (uint32_t i = 0; i < attemptsArray.Size(); ++i)
            {
                auto aObj = attemptsArray.GetObjectAt(i);

                ::quiz_examination_system::SupabaseClient::AttemptReportRow row;
                row.student_id = aObj.GetNamedString(L"student_id", L"");
                row.username = aObj.GetNamedString(L"student_username", L"");
                row.attempt_number = static_cast<int>(aObj.GetNamedNumber(L"attempt_number", 0));
                row.score = static_cast<int>(aObj.GetNamedNumber(L"score", 0));
                row.total_points = static_cast<int>(aObj.GetNamedNumber(L"total_points", 0));
                row.correct_count = static_cast<int>(aObj.GetNamedNumber(L"correct_count", 0));
                row.incorrect_count = static_cast<int>(aObj.GetNamedNumber(L"incorrect_count", 0));
                row.time_spent_seconds = static_cast<int>(aObj.GetNamedNumber(L"time_spent_seconds", 0));

                reportData.push_back(row);

                hstring attemptId = aObj.GetNamedString(L"attempt_id", L"");
                hstring submittedAt = aObj.GetNamedString(L"submitted_at", L"");

                auto reportItem = make<ReportItem>(
                    attemptId,
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
        catch (hresult_error const &)
        {
            OutputDebugStringW(L"[ReportsPage] Error in LoadReportData\n");
            m_reports.Clear();
            ShowMessage(L"Failed to load report data", InfoBarSeverity::Error);
        }
        catch (...)
        {
            OutputDebugStringW(L"[ReportsPage] Unknown error in LoadReportData\n");
            m_reports.Clear();
            ShowMessage(L"An unexpected error occurred while loading report", InfoBarSeverity::Error);
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

    void ReportsPage::ReportListView_ItemClick(Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::Controls::ItemClickEventArgs const &e)
    {
        auto clickedItem = e.ClickedItem().as<quiz_examination_system::ReportItem>();
        if (clickedItem)
        {
            // Navigate to ReviewAttemptsPage with the selected quiz
            // The teacher can then see all attempts and perform actions (delete, view details, etc.)
            this->Frame().Navigate(xaml_typename<quiz_examination_system::ReviewAttemptsPage>());
        }
    }
}
